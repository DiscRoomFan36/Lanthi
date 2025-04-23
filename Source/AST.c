
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "arena.h"
#include "context.h"

#include "AST.h"


#define local static

// TODO put this somewhere smarter
local SV arena_SV_dup(Arena *a, SV s) {
    SV result;
    result.data = Arena_alloc(a, s.size);
    result.size = s.size;

    for (s64 i = 0; i < s.size; i++) result.data[i] = s.data[i];

    return result;
}


local AST_Node_Argument *parse_argument(Tokenizer *t) {
    Context *context = get_context();

    AST_Node_Argument *argument = Arena_calloc(&context->ast_arena, sizeof(AST_Node_Argument));
    // TODO this is error prone, make functions that return the struct *, or something
    argument->kind = AST_ARGUMENT;

    while (True) {
        Token token = peek_next_token(t);

        // end of args
        if (token.kind == ')') { return argument; }

        // parse expression until ,
        if (token.kind == TK_String_Lit) {
            AST_Node_String_Lit *str_lit = Arena_alloc(&context->ast_arena, sizeof(AST_Node_String_Lit));
            str_lit->kind = AST_STRING_LIT;
            str_lit->literal = arena_SV_dup(&context->string_arena, token.text);

            arena_da_append(&context->ast_arena, &argument->args, (AST_Node *) str_lit);

            // skip the token
            get_next_token(t);

            continue;
        }


        // hmm how do we handle ","?
        // if (token.kind == ',') {}
        // then check is there has already been an arg?

        fprintf(stderr, "%s:%ld: unexpected token in argument: |"SV_Fmt"|\n", "TODO: add context so we can get the filename", t->line_num, SV_Arg(token.text));
        // TODO we must get better error handling.
        // maybe call a function that exits for us? but also dose the cleanup
        exit(1);
    }
}


local AST_Node_Expression *parse_expression(Tokenizer *t) {
    Context *context = get_context();
    AST_Node_Expression *expr = Arena_alloc(&context->ast_arena, sizeof(AST_Node_Expression));
    expr->kind = AST_EXPRESSION;
    expr->sub_expression = NULL;

    Token token = get_next_token(t);

    if (token.kind == TK_Ident) {

        // got an ident, now what are we doing with it?
        Token next = get_next_token(t);
        if (next.kind == '(') {
            // were calling a function!
            AST_Node_Call_Function *call = Arena_alloc(&context->ast_arena, sizeof(AST_Node_Call_Function));
            call->kind = AST_CALL_FUNCTION;
            call->function_name = arena_SV_dup(&context->string_arena, token.text);
            call->arguments = parse_argument(t);

            if (!expect_next_token(t, ')', NULL)) {
                // this assert is real.
                assert(False && "parse argument should put us in the right place");
            }

            Token semi = get_next_token(t);

            if (semi.kind != ';') {
                // error, we dont know how to do more than 1 thing...
                assert(False && "TODO: figure out error reporting");
            }

            // end of expression
            expr->sub_expression = (AST_Node*) call;
            return expr;
        }

        assert(False && "TODO: figure out error reporting here");
    }

    // this could happen with the following line.
    // -47;
    // which might be fine? just make it possible.
    assert(False && "TODO: parse statement, non ident");
}


AST_Node_ptr_Array tokenizer_to_AST(Tokenizer *t) {
    AST_Node_ptr_Array nodes = {0};


    while (True) {
        Token token = get_next_token(t);

        if (token.kind == TK_Eof) {
            // end of file where done here
            break;
        }

        if (token.kind == TK_Ident) {
            printf("got function ["SV_Fmt"]\n", SV_Arg(token.text));

            // start building AST, must be a const assignment for now
            Context *context = get_context();
            AST_Node_Const_Assignment *assignment = Arena_alloc(&context->ast_arena, sizeof(AST_Node_Const_Assignment));
            assignment->kind = AST_CONST_ASSIGNMENT;
            assignment->name = arena_SV_dup(&context->string_arena, token.text);

            // TODO fprintf, turn into 'report_expected'

            { // parse the start of a function.
                Token next;
                if (!expect_next_token(t, ':', &next)) {
                    fprintf(stderr, "%s:%ld: expected ':' got |"SV_Fmt"|\n", t->filename, t->line_num, SV_Arg(next.text));
                    assert(False && "TODO: replace return_defer");
                }

                // its always constant.
                if (!expect_next_token(t, ':', &next)) {
                    fprintf(stderr, "%s:%ld: expected ':' got |"SV_Fmt"|\n", t->filename, t->line_num, SV_Arg(next.text));
                    assert(False && "TODO: replace return_defer");
                }

                if (!expect_next_token(t, '(', &next)) {
                    fprintf(stderr, "%s:%ld: expected '(' got |"SV_Fmt"|\n", t->filename, t->line_num, SV_Arg(next.text));
                    assert(False && "TODO: replace return_defer");
                }

                // dont handle arguments yet
                if (!expect_next_token(t, ')', &next)) {
                    fprintf(stderr, "%s:%ld: expected ')' got |"SV_Fmt"|\n", t->filename, t->line_num, SV_Arg(next.text));
                    assert(False && "TODO: replace return_defer");
                }

                // dont handle return arguments yet
                if (!expect_next_token(t, '{', &next)) {
                    fprintf(stderr, "%s:%ld: expected '{' got |"SV_Fmt"|\n", t->filename, t->line_num, SV_Arg(next.text));
                    assert(False && "TODO: replace return_defer");
                }
            }

            AST_Node_Function func = {0};
            func.kind = AST_FUNCTION;

            // now parse statements until '}'
            while (True) {
                Token token = peek_next_token(t);

                // end of function
                if (token.kind == '}') {
                    // skip the '}'
                    get_next_token(t);
                    break;
                }

                AST_Node_Expression *thing = parse_expression(t);
                arena_da_append(&context->ast_arena, &func.expressions, thing);
            }

            // do we allow semi after functions? and in what context?
            /*
                Token could_be_semi = peek_next_token(t);
                if (could_be_semi.kind == ';') {
                    // skip it.
                    get_next_token(t);
                }
            */

            assignment->function = func;
            arena_da_append(&context->ast_arena, &nodes, (AST_Node*) assignment);
            continue;
        }

        fprintf(stderr, "Unknown token %d (%c)\n", token.kind, token.kind);
        assert(False && "TODO: replace return_defer");
    }

    return nodes;
}



local void print_spaces(int num) {
    for (int i = 0; i < num; i++) {
        printf("    ");
    }
}

void print_node(AST_Node *node, s64 indent) {
    if (node->kind == AST_CONST_ASSIGNMENT) {
        AST_Node_Const_Assignment *node_const_assign = (AST_Node_Const_Assignment*) node;

        print_spaces(indent);
        printf(""SV_Fmt" :: () {\n", SV_Arg(node_const_assign->name));

        for (s64 i = 0; i < node_const_assign->function.expressions.count; i++) {
            AST_Node *expr_node = node_const_assign->function.expressions.items[i]->sub_expression;
            print_node(expr_node, indent+1);
        }

        print_spaces(indent);
        printf("}\n");

        return;
    }

    if (node->kind == AST_CALL_FUNCTION) {
        AST_Node_Call_Function *call = (AST_Node_Call_Function*)node;

        print_spaces(indent);
        printf(SV_Fmt"(\n", SV_Arg(call->function_name));


        // TODO just print it here
        print_node((AST_Node*)call->arguments, indent+1);

        print_spaces(indent);
        printf(");\n");
        return;
    }

    if (node->kind == AST_ARGUMENT) {
        AST_Node_Argument *args = (AST_Node_Argument*) node;

        // TODO this is dumb? put in call function
        for (s64 i = 0; i < args->args.count; i++) {
            print_node(args->args.items[i], indent);
        }

        return;
    }

    if (node->kind == AST_STRING_LIT) {
        AST_Node_String_Lit *str_lit = (AST_Node_String_Lit*)node;
        print_spaces(indent);
        printf(SV_Fmt"\n", SV_Arg(str_lit->literal));

        return;
    }

    fprintf(stderr, "unknown ident %d\n", node->kind);
    exit(1);
}

