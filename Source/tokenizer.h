
#ifndef TOKENIZER_H_
#define TOKENIZER_H_


#include "ints.h"
#include "String_View.h"

typedef struct Tokenizer {
    // make this a SV? or is it easier like this.
    const char *filename;
    // contains the original SV for error handling
    SV original;

    SV parseing;

    s64 line_num;
    s64 col_num;
} Tokenizer;

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
} Token;


// dose not allocate. no need to free.
Tokenizer new_tokenizer(const char *filename, SV file);

// peeks at the next token.
Token peek_next_token(Tokenizer *t);
// gets the next token, and advances the tokenizer
Token get_next_token(Tokenizer *t);

// calls get_next_token, and returns (expect == token.kind)
// out_token is the returned token, I wish there were multiple return values.
bool32 expect_next_token(Tokenizer *t, TokenKind expect, Token *out_token);


const char *token_to_name(Token token);

#endif // TOKENIZER_H_
