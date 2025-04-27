
#ifndef TOKENIZER_H_
#define TOKENIZER_H_


#include "ints.h"
#include "String_View.h"

// how many peeks ahead you can do
// this could be way higher.
#define MAX_PEEK_COUNT 64


typedef enum TokenKind {
    TK_Eof = 0,
    TK_Ident,
    TK_String_Lit,

    TK_Colon      = ':',
    TK_SemiColon  = ';',
    TK_OpenParen  = '(',
    TK_CloseParen = ')',
    TK_OpenCurly  = '{',
    TK_CloseCurly = '}',
} TokenKind;

typedef struct Token {
    TokenKind kind;
    // what the text that was parsed was, good for lit's and ident's
    SV text;

    s64 line_number;
    s64 col_number;
} Token;


// keeps track of the tokenization process,
// will never do more work than necessary to parse the same tokens more than once
// no allocations
typedef struct Tokenizer {
    // make this a SV? or is it easier like this.
    const char *filename;
    // contains the original SV for error handling
    SV original;

    SV parseing;

    s64 line_num;
    s64 col_num;

    s64 current_peek_count;
    Token peek_buffer[MAX_PEEK_COUNT];
} Tokenizer;



// dose not allocate. no need to free.
Tokenizer new_tokenizer(const char *filename, SV file);


// just peeks the next token.
// wrapper for peek_token, equivlenent to peek_token(t, 0);
Token peek_next_token(Tokenizer *t);

// peek 'peek_index' tokens ahead,
// will assert if you ran past EOF, or you overran the buffer
// peek_index = 0   =is equal to=   next token
Token peek_token(Tokenizer *t, s64 peek_index);


// take a token, and shift the peek buffer. works like a 'get_next_token'
Token take_token(Tokenizer *t);
// takes count tokens, returns the last
// returns the same as peeking the 'count-1' th token
Token take_tokens(Tokenizer *t, s64 count);

// TODO add take_and_expect back, would be easy, only when needed
// bool32 take_and_expect(Tokenizer *t, TokenKind expect, Token *out_token);


const char *token_to_name(Token token);

#endif // TOKENIZER_H_
