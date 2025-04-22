
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



Tokenizer new_tokenizer(SV file);

Token get_next_token(Tokenizer *t);
bool32 expect_next_token(Tokenizer *t, TokenKind expect, Token *out_token);

