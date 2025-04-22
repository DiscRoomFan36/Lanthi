
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "ints.h"
#include "String_View.h"
#include "dynamic_array.h"

#include "tokenizer.h"


// TODO put all this stuff into a AST file
typedef enum AST_Node_Kind {
    AST_NONE = 0,
    AST_STATEMENT,
    AST_FUNCTION,
} AST_Node_Kind;

typedef struct AST_Node {
    AST_Node_Kind kind;
} AST_Node;

typedef struct AST_Node_Statement {
    AST_Node base;
} AST_Node_Statement;

typedef struct AST_Node_Statement_Array {
    AST_Node_Statement *items;
    u64 count;
    u64 capacity;
} AST_Node_Statement_Array;

typedef struct AST_Node_Function {
    AST_Node base;
    SV name;
    AST_Node_Statement_Array statements;
} AST_Node_Function;

typedef struct AST_Node_Function_Array {
    AST_Node_Function *items;
    u64 count;
    u64 capacity;
} AST_Node_Function_Array;




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

    AST_Node_Function_Array functions = {0};

    Tokenizer tokenizer = new_tokenizer(file);
    Tokenizer *t = &tokenizer;

    while (True) {
        Token token = get_next_token(t);

        if (token.kind == TK_Eof) {
            // end of file where done here
            break;
        }

        if (token.kind == TK_Ident) {
            printf("got function ["SV_Fmt"]\n", SV_Arg(token.name));

            // start building AST
            AST_Node_Function func = {0};
            func.base.kind = AST_FUNCTION;
            func.name = SV_dup(token.name);

            // TODO fprintf, turn into 'report_expected'

            { // parse the start of a function.
                Token next;
                if (!expect_next_token(t, ':', &next)) {
                    fprintf(stderr, "%s:%ld: expected ':' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.name));
                    return_defer(1);
                }

                // its always constant.
                if (!expect_next_token(t, ':', &next)) {
                    fprintf(stderr, "%s:%ld: expected ':' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.name));
                    return_defer(1);
                }

                if (!expect_next_token(t, '(', &next)) {
                    fprintf(stderr, "%s:%ld: expected '(' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.name));
                    return_defer(1);
                }

                // dont handle arguments yet
                if (!expect_next_token(t, ')', &next)) {
                    fprintf(stderr, "%s:%ld: expected ')' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.name));
                    return_defer(1);
                }

                // dont handle return arguments yet
                if (!expect_next_token(t, '{', &next)) {
                    fprintf(stderr, "%s:%ld: expected '{' got |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(next.name));
                    return_defer(1);
                }
            }


            // now parse statements until '}'
            while (True) {
                Token token = get_next_token(t);

                if (token.kind == '}') {
                    break;
                }


                fprintf(stderr, "%s:%ld: unexpected token: |"SV_Fmt"|\n", filename, t->line_num, SV_Arg(token.name));
                return_defer(1);
            }

            da_append(&functions, func);
            continue;
        }

        fprintf(stderr, "Unknown token %d (%c)\n", token.kind, token.kind);
        return_defer(1);
    }


    printf("finished converting into AST hurray!\n");
    printf("probably should do something with it...\n");


defer:
    // TODO use an arena, this is tedious.
    for (size_t i = 0; i < functions.count; i++) {
        AST_Node_Function func = functions.items[i];
        SV_free(&func.name);
        da_free(&func.statements);
    }

    da_free(&functions);
    free(file.data);
    return result;
}
