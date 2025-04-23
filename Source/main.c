
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "ints.h"
#include "String_View.h"
#include "dynamic_array.h"
#include "arena.h"

#include "tokenizer.h"
#include "AST_Node.h"



SV arena_SV_dup(Arena *a, SV s) {
    SV result;
    result.data = Arena_alloc(a, s.size);
    result.size = s.size;

    for (s64 i = 0; i < s.size; i++) result.data[i] = s.data[i];

    return result;
}

SV read_entire_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    SV result = {0};

    if (!file) {
        fprintf(stderr, "ERROR when opening the file: %s\n", strerror(errno));
        return result;
    }

    fseek(file, 0, SEEK_END);
    result.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    result.data = malloc(result.size * sizeof(char));
    assert(result.data != NULL);

    s64 read_bytes = fread(result.data, sizeof(char), result.size, file);
    assert(read_bytes == result.size);

    if (fclose(file)) {} // error, we dont care

    return result;
}


AST_Node_Argument *parse_argument(Arena *a, Tokenizer *t) {
    AST_Node_Argument *argument = Arena_calloc(a, sizeof(AST_Node_Argument));
    // TODO this is error prone, make functions that return the struct *, or something
    argument->kind = AST_ARGUMENT;

    while (True) {
        Token token = peek_next_token(t);

        // end of args
        if (token.kind == ')') { return argument; }

        // parse expression until ,
        if (token.kind == TK_String_Lit) {
            AST_Node_String_Lit *str_lit = Arena_alloc(a, sizeof(AST_Node_String_Lit));
            str_lit->kind = AST_STRING_LIT;
            str_lit->literal = arena_SV_dup(a, token.text);

            arena_da_append(a, &argument->args, (AST_Node *) str_lit);

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


// TODO what dose this return
AST_Node *parse_expression(Arena *a, Tokenizer *t) {
    Token token = get_next_token(t);

    if (token.kind == TK_Ident) {

        // got an ident, now what are we doing with it?
        Token next = get_next_token(t);
        if (next.kind == '(') {
            // were calling a function!

            AST_Node_Argument *args = parse_argument(a, t);
            if (!expect_next_token(t, ')', NULL)) {
                assert(False && "parse argument should put us in the right place");
            }

            assert(False && "TODO: handle this");
        }

        assert(False && "TODO: figure out error reporting here");
    }


    assert(False && "TODO: parse statement, non ident");
}



#define return_defer(res) do { result = res; goto defer; } while(0)

void usage(const char *prog_name) {
    printf("USAGE: %s file\n", prog_name);
}

int main(int argc, char const *argv[]) {
    const char *program_name = argv[0];
    if (argc != 2) {
        fprintf(stderr, "No input file was provided\n");
        usage(program_name);
        exit(1);
    }

    const char *filename = argv[1];
    SV file = read_entire_file(filename);
    if (!file.data) {
        fprintf(stderr, "could not open file %s\n", filename);
        exit(1);
    }

    int result = 0;

    // an arena to just dump things in.
    // in the future we might separate different alloc's into different arena's.
    Arena arena = {0};

    AST_Node_Array nodes = {0};


    Tokenizer tokenizer = new_tokenizer(file);
    Tokenizer *t = &tokenizer;

    while (True) {
        Token token = get_next_token(t);

        if (token.kind == TK_Eof) {
            // end of file where done here
            break;
        }

        if (token.kind == TK_Ident) {
            printf("got function ["SV_Fmt"]\n", SV_Arg(token.text));

            // start building AST, must be a const assignment for now
            AST_Node_Const_Assignment *assignment = Arena_alloc(&arena, sizeof(AST_Node_Const_Assignment));
            assignment->name = arena_SV_dup(&arena, token.text);

            // TODO fprintf, turn into 'report_expected'

            { // parse the start of a function.
                Token next;
                if (!expect_next_token(t, ':', &next)) {
                    fprintf(stderr, "%s:%ld: expected ':' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.text));
                    return_defer(1);
                }

                // its always constant.
                if (!expect_next_token(t, ':', &next)) {
                    fprintf(stderr, "%s:%ld: expected ':' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.text));
                    return_defer(1);
                }

                if (!expect_next_token(t, '(', &next)) {
                    fprintf(stderr, "%s:%ld: expected '(' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.text));
                    return_defer(1);
                }

                // dont handle arguments yet
                if (!expect_next_token(t, ')', &next)) {
                    fprintf(stderr, "%s:%ld: expected ')' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.text));
                    return_defer(1);
                }

                // dont handle return arguments yet
                if (!expect_next_token(t, '{', &next)) {
                    fprintf(stderr, "%s:%ld: expected '{' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.text));
                    return_defer(1);
                }
            }

            AST_Node_Function func = {0};

            // now parse statements until '}'
            while (True) {
                Token token = peek_next_token(t);

                // end of function
                if (token.kind == '}') {
                    get_next_token(t);
                    break;
                }

                AST_Node *thing = parse_expression(&arena, t);
                assert(False && "TODO: use the result");

                fprintf(stderr, "%s:%ld: unexpected token: |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(token.text));
                return_defer(1);
            }


            assignment->function = func;

            AST_Node new_node = {0};
            new_node.kind = AST_CONST_ASSIGNMENT;
            new_node.rest = assignment;

            arena_da_append(&arena, &nodes, new_node);
            continue;
        }

        fprintf(stderr, "Unknown token %d (%c)\n", token.kind, token.kind);
        return_defer(1);
    }


    printf("finished converting into AST hurray!\n");
    printf("probably should do something with it...\n");


defer:
    Arena_free(&arena);
    free(file.data);
    return result;
}
