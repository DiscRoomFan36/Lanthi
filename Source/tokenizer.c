
#include <assert.h>

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


// constant, do not touch
const SV multiline_end = {
    .data = "*/",
    .size = 2,
};


// make a tokenizer
Tokenizer new_tokenizer(const char *filename, SV file) {
    Tokenizer result = {
        .filename = filename,
        .original = file,

        .parseing = file,

        .line_num = 1,
        .col_num = 1,
    };

    return result;
}

// returns the current line, everything before the current token, up to the next newline, (newline omitted)
local SV get_current_line(Tokenizer *t) {
    return get_single_line(t->original, t->parseing.data - t->original.data);
}

// advance the parser by 'count' characters, also dose line num counting and stuff
local inline void advance(Tokenizer *t, s64 count) {
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
            advance(t, 1);
        }

        // return on eof
        if (t->parseing.size == 0) return;

        // check if the char is comment char
        if (t->parseing.size < 2) return;

        // comments need 2 char's
        if (t->parseing.data[0] == '/' && t->parseing.data[1] == '/') {
            // its a line comment
            advance(t, 2);
            s64 index = find_index_of_char(t->parseing, '\n');
            if (index == -1) {
                advance(t, t->parseing.size);
                return;
            }

            // advance past the line.
            advance(t, index + 1);
            continue;
        }

        if (t->parseing.data[0] == '/' && t->parseing.data[1] == '*') {
            // is a multiline comment
            // TODO support multiline layers.

            // forward is for makeing the error thing a bit better
            SV forward = {.data = t->parseing.data + 2, .size = t->parseing.size - 2};

            s64 index = find_index_of(forward, multiline_end);
            if (index == -1) {
                SV cur_line = get_current_line(t);
                report_Tokenizer_error(t,
                    cur_line,
                    "Unexpected EOF when tokenizing multiline comment.",
                    "Try adding a '*/' to close the multiline comment, note that multiple levels of /**/ are currently not supported.");
            }

            advance(t, index + multiline_end.size + 2);
            continue;
        }


        return;
    }
}

Token peek_next_token(Tokenizer *t) {
    assert(t->parseing.size >= 0);

    chop_whitespace(t);

    if (t->parseing.size == 0) return (Token) { .kind = TK_Eof, .text = {0} };

    if (is_alpha(t->parseing.data[0])) {
        SV ident = {.data = t->parseing.data, .size = 1};
        for (s64 i = 1; i < t->parseing.size; i++) {
            if (!is_ident_char(t->parseing.data[i])) break;
            ident.size += 1;
        }
        return (Token){ .kind = TK_Ident, .text = ident };
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
                    // we have a problem, decide the better error message
                    SV cur_line = get_current_line(t);
                    // else report that we hit EOF.
                    report_Tokenizer_error(t,
                        cur_line,
                        "Missing closeing quote when trying to parse 'String Literal'",
                        "Try adding '\"' to close a String Literal, be carful to make sure you didn't escape the '\"' character with the '\\' character");
                }

                s64 new_line_index = find_index_of_char(thing, '\n');
                if (new_line_index != -1 && new_line_index < index) {
                    // we hit a newline before the next '"' character, not multiline strings
                    SV cur_line = get_current_line(t);
                    // copy-pasta this error.
                    report_Tokenizer_error(t,
                        cur_line,
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

        // TODO: dose this insert the special chars?
        return (Token) {
            .kind = TK_String_Lit,
            .text = {
                .data = t->parseing.data,
                .size = t->parseing.size - thing.size,
            },
        };
    }

    // just return the single char
    Token next_token = {
        .kind = t->parseing.data[0],
        .text = { .data = t->parseing.data, .size = 1 },
    };
    return next_token;
}

Token get_next_token(Tokenizer *t) {
    assert(t->parseing.size >= 0);
    Token token = peek_next_token(t);
    advance(t, token.text.size);
    return token;
}

bool32 expect_next_token(Tokenizer *t, TokenKind expect, Token *out_token) {
    assert(t->parseing.size >= 0);
    Token token = get_next_token(t);
    if (out_token) { *out_token = token; }
    return token.kind == expect;
}

