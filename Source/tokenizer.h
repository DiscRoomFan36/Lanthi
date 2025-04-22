
#include "ints.h"
#include "String_View.h"

typedef struct Tokenizer {
    SV parseing;

    s64 line_num;
} Tokenizer;

typedef enum TokenKind {
    TK_Eof = 0,
    TK_Ident,

    TK_Colon      = ':',
    TK_SemiColon  = ';',
    TK_OpenParen  = '(',
    TK_CloseParen = ')',
    TK_OpenCurly  = '{',
    TK_CloseCurly = '}',
} TokenKind;

typedef struct Token {
    TokenKind kind;
    // if its an ident.
    SV name;
} Token;


// dose not allocate. no need to free.
Tokenizer new_tokenizer(SV file);

// peeks at the next token.
Token peek_next_token(Tokenizer *t);
// gets the next token, and advances the tokenizer
Token get_next_token(Tokenizer *t);

// calls get_next_token, and returns (expect == token.kind)
// out_token is the returned token, I wish there were multiple return values.
bool32 expect_next_token(Tokenizer *t, TokenKind expect, Token *out_token);

