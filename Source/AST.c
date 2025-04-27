
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "arena.h"
#include "context.h"

#include "AST.h"
#include "Report_Error.h"


#define local static

// TODO put this somewhere smarter
// local SV arena_SV_dup(Arena *a, SV s) {
//     SV result;
//     result.data = Arena_alloc(a, s.size);
//     result.size = s.size;

//     for (s64 i = 0; i < s.size; i++) result.data[i] = s.data[i];

//     return result;
// }


// TODO make all the 'new' functions
// AST_Node_Const_Assignment

// TODO should this accept a node? so we can handle array assigns, <- later
local AST_Node *parse_declaration_or_assignment(Tokenizer *t, SV name_to_assign_to) {
    (void) t;
    (void) name_to_assign_to;

    assert(False && "TODO: 'parse_declaration_or_assignment'");
}


local AST_Node *parse_expression(Tokenizer *t) {
    Token token = peek_next_token(t);

    if (token.kind == TK_Ident) {

        take_token(t); // go past the ident token

        Token next = peek_next_token(t);

        if (next.kind == ':') {
            // ok, its a definition, hand the job to someone else.
            return parse_declaration_or_assignment(t, token.text);
        }

        report_AST_unexpected_token(t, "parse expression when parseing token", next, "we only handle ':' after Ident for now");
    }


    report_AST_unexpected_token(t, "parse expression", token, "expected an ident or something.");
}

AST_Node_ptr_Array tokenizer_to_AST(Tokenizer *t) {
    Context *context = get_context();

    AST_Node_ptr_Array nodes = {0};

    // we are in global scope, loop over all expressions and only stop when we hit EOF.
    while (True) {
        Token tok = peek_next_token(t);
        if (tok.kind == TK_Eof) break;

        AST_Node *node = parse_expression(t);
        arena_da_append(&context->ast_arena, &nodes, node);
    }

    return nodes;
}




void print_program(AST_Node_ptr_Array array) {
    (void) array;

    assert(False && "TODO: 'print_program'");
}
