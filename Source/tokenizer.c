
#include <assert.h>
#include <string.h> // for 'memmove'

#include "Report_Error.h"
#include "tokenizer.h"

#define local static

local inline bool32 is_space(char c) {
    if (c == ' ')  return True;
    if (c == '\f') return True;
    if (c == '\n') return True;
    if (c == '\r') return True;
    if (c == '\t') return True;
    if (c == '\v') return True;
    return False;
}

local inline bool32 is_alpha(char c) {
    if ('a' <= c && c <= 'z') return True;
    if ('A' <= c && c <= 'Z') return True;
    return False;
}

local inline bool32 is_digit(char c) {
    if ('0' <= c && c <= '9') return True;
    return False;
}

// what a identifier is made of
local inline bool32 is_ident_char(char c) {
    if (is_alpha(c)) return True;
    if (is_digit(c)) return True;
    if (c == '_')    return True;
    return False;
}


local inline Token default_token(void) {
    Token token = {
        .kind = TK_ERROR,
        .text = {0},
        .line_number = 0,
        .col_number = 0,
    };
    return token;
}



local SV get_tokenizer_current_line(Tokenizer *t) {
    return get_single_line(t->original, t->parseing.data - t->original.data);
}

// constant, do not touch
const SV multiline_end = {
    .data = "*/",
    .size = 2,
};



// advance the parser by 'count' characters, also dose line num counting and stuff
local inline void tk_advance(Tokenizer *t, s64 count) {
    assert(t->parseing.size >= count); // dont do anything stupid!

    // count white spaces
    // TODO put in SV? <<< what dose this mean?
    s64 last_i_at_line = -1;
    for (s64 i = 0; i < count; i++) {
        if (t->parseing.data[i] == '\n') {
            t->line_num += 1;
            last_i_at_line = i;
        }
    }

    if (last_i_at_line == -1) {
        // we didn't cross a line boundary
        t->col_num += count;
    } else {
        t->col_num = count - last_i_at_line;
    }

    t->parseing.data += count;
    t->parseing.size -= count;
}

local void chop_whitespace(Tokenizer *t) {
    while (True) {
        // advance cursor until non whitespace
        while (t->parseing.size > 0 && is_space(t->parseing.data[0])) {
            tk_advance(t, 1);
        }

        // return on eof
        if (t->parseing.size == 0) return;

        // check if the char is comment char
        if (t->parseing.size < 2) return;

        // comments need 2 char's
        if (t->parseing.data[0] == '/' && t->parseing.data[1] == '/') {
            // its a line comment
            tk_advance(t, 2);
            s64 index = find_index_of_char(t->parseing, '\n');
            if (index == -1) {
                tk_advance(t, t->parseing.size);
                return;
            }

            // advance past the line.
            tk_advance(t, index + 1);
            continue;
        }

        if (t->parseing.data[0] == '/' && t->parseing.data[1] == '*') {
            // is a multiline comment
            // TODO support multiline layers.

            // forward is for makeing the error thing a bit better
            SV forward = {.data = t->parseing.data + 2, .size = t->parseing.size - 2};

            s64 index = find_index_of(forward, multiline_end);
            if (index == -1) {
                SV current_line = get_tokenizer_current_line(t);
                report_Tokenizer_error(t,
                    current_line,
                    "Unexpected EOF when tokenizing multiline comment.",
                    "Try adding a '*/' to close the multiline comment, note that multiple levels of /**/ are currently not supported.");
            }

            tk_advance(t, index + multiline_end.size + 2);
            continue;
        }


        return;
    }
}


// peeks the next token, dose not advance the t->parseing pointer past the next token, only skips whitespace
local Token get_next_token(Tokenizer *t) {
    assert(t->parseing.size >= 0);

    chop_whitespace(t);

    Token result = default_token();
    result.line_number = t->line_num;
    result.col_number = t->col_num;

    if (t->parseing.size == 0) {
        result.kind = TK_Eof;
        result.text = (SV){0};
        return result;
    }

    if (is_alpha(t->parseing.data[0])) {
        SV ident = {.data = t->parseing.data, .size = 1};
        for (s64 i = 1; i < t->parseing.size; i++) {
            if (!is_ident_char(t->parseing.data[i])) break;
            ident.size += 1;
        }

        result.kind = TK_Ident;
        result.text = ident;
        return result;
    }

    if (is_digit(t->parseing.data[0])) {
        assert(False && "TODO: get_next_token: parse int literal");
    }

    if (t->parseing.data[0] == '"') {
        SV thing = { .data = t->parseing.data + 1, .size = t->parseing.size - 1 };

        while (True) {
            s64 index = find_index_of_char(thing, '"');

            { // do error checks
                if (index == -1) {
                    SV current_line = get_tokenizer_current_line(t);
                    report_Tokenizer_error(t,
                        current_line,
                        "Missing closeing quote when trying to parse 'String Literal'",
                        "Try adding '\"' to close a String Literal, be carful to make sure you didn't escape the '\"' character with the '\\' character");
                }

                s64 new_line_index = find_index_of_char(thing, '\n');
                if (new_line_index != -1 && new_line_index < index) {
                    // we hit a newline before the next '"' character, not multiline strings
                    SV current_line = get_tokenizer_current_line(t);
                    report_Tokenizer_error(t,
                        current_line,
                        "Missing closeing quote when trying to parse 'String Literal', (String Literals cannot currently cross new line boundaries)",
                        "Try adding '\"' to close a String Literal, be carful to make sure you didn't escape the '\"' character with the '\\' character");
                }
            }

            // count number of prev /
            s64 n = 0;
            for (s64 i = index-1; i > 0; i--) {
                if (thing.data[i] != '\\') break;
                n += 1;
            }
            thing.data += index + 1;
            thing.size -= index + 1;
            if (n % 2 == 0) {
                break;
            }
        }

        // TODO: dose this insert the special chars? no
        result.kind = TK_String_Lit;
        result.text = (SV) {
            .data = t->parseing.data,
            .size = t->parseing.size - thing.size,
        };
        return result;
    }

    // just return the single char
    result.kind = t->parseing.data[0];
    result.text = (SV){ .data = t->parseing.data, .size = 1 };
    return result;
}

local void next_token_into_peek_buffer(Tokenizer *t) {
    assert(t->current_peek_count < MAX_PEEK_COUNT && "Cannot get more than 'MAX_PEEK_COUNT' peeks ahead");

    if (t->current_peek_count > 0) {
        Token last_token = t->peek_buffer[t->current_peek_count-1];
        assert(last_token.kind != TK_Eof && "Cannot get next token when the last token currently is EOF");
    }

    // get the next token from peek.
    Token token = get_next_token(t);
    // advance past the token
    tk_advance(t, token.text.size);

    // put the token into the peek buffer
    t->peek_buffer[t->current_peek_count] = token;
    // inc the buffer
    t->current_peek_count += 1;
}


// ----------------------
// User faceing functions
// ----------------------


Tokenizer new_tokenizer(const char *filename, SV file) {
    Tokenizer result = {
        .filename = filename,
        .original = file,

        .parseing = file,

        .line_num = 1,
        .col_num = 1,

        .current_peek_count = 0,
        // init this to zero, even though we probably dont have to.
        .peek_buffer = {0},
    };

    return result;
}


Token peek_next_token(Tokenizer *t) { return peek_token(t, 0); }

Token peek_token(Tokenizer *t, s64 peek_index) {
    // generate until we have enough of a buffer
    while (peek_index >= t->current_peek_count) {
        next_token_into_peek_buffer(t);
    }

    return t->peek_buffer[peek_index];
}

Token take_next_token(Tokenizer *t) { return take_token(t, 1); }

Token take_token(Tokenizer *t, s64 count) {
    assert(count > 0 && "Cannot take 0 tokens, maybe we could, but what would we return?");
    assert(count < MAX_PEEK_COUNT && "Cannot take more than the max buffer count, why would you do this anyway? how do you know where your going? we could support this...");

    // get the token
    Token token = peek_token(t, count-1);

    // shift the buffer over, might be expensive
    // TODO circular buffer?
    t->current_peek_count -= count;
    memmove(t->peek_buffer, &t->peek_buffer[count], t->current_peek_count * sizeof(Token));

    // return the token
    return token;
}


const char *token_to_name(Token token) {
    switch ((int)token.kind) {
        case TK_Eof:        return "EOF";
        case TK_Ident:      return "Ident";
        case TK_String_Lit: return "String Literal";

        case ':': return "Colon";
        case ';': return "Semi Colon";
        case '(': return "Open Paren";
        case ')': return "Close Paren";
        case '{': return "Open Curly";
        case '}': return "Close Curly";

        default:            return "(Unknown Token)";
    }
}


// TODO is this useful? maybe for other modules...
SV get_line_around_token(Tokenizer *t, Token token) {
    s64 index = token.text.data - t->original.data;
    assert(0 <= index && index < t->original.size);
    return get_single_line(t->original, index);
}

