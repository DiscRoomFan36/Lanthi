
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ints.h"
#include "String_View.h"
#include "dynamic_array.h"
#include "arena.h"
#include "context.h"

#include "tokenizer.h"
#include "AST.h"


SV read_entire_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    SV result = {0};

    if (!file) {
        perror("Error when opening file");
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


int main(int argc, char const *argv[]) {
    const char *program_name = argv[0];
    if (argc != 2) {
        fprintf(stderr, "No input file was provided\n");
        printf("USAGE: %s file\n", program_name);
        exit(1);
    }

    const char *filename = argv[1];
    // TODO who owns this data? the context?
    SV file = read_entire_file(filename);
    if (!file.data) {
        fprintf(stderr, "could not open file %s\n", filename);
        exit(1);
    }

    Context *context = get_context();
    // memset these to 0, even though the context grantees this.
    // just to make sure we know what were doing.
    memset(&context->string_arena, 0, sizeof(Arena));
    memset(&context->ast_arena,    0, sizeof(Arena));

    Tokenizer tokenizer = new_tokenizer(filename, file);
    AST_Node_ptr_Array nodes = tokenizer_to_AST(&tokenizer);

    printf("finished converting into AST hurray!\n");
    printf("probably should do something with it...\n");

    for (s64 i = 0; i < nodes.count; i++) {
        print_node(nodes.items[i], 0);
    }


    #define OUTPUT_C_FILENAME "./output.c"

    FILE *f = fopen(OUTPUT_C_FILENAME, "wb");
    if (!f) {
        perror("Cannot open "OUTPUT_C_FILENAME);
        assert(False && "TODO figure out error reporting");
    }


    fprintf(f, "#include <stdio.h>\n");

    // pre define all functions for C backend.
    for (size_t i = 0; i < nodes.count; i++) {
        AST_Node_Const_Assignment *node = nodes.items[i];
        // assert that all the top nodes are const assignments. TODO
        assert(node->kind == AST_CONST_ASSIGNMENT);

        // assumes that all top const assignments are functions
        // also assumes no duplicate names.
        fprintf(f, "void "SV_Fmt"(void);\n", SV_Arg(node->name));

        // at this point also recur deeper into the structures to find more functions.
    }



    for (size_t i = 0; i < nodes.count; i++) {
        AST_Node_Const_Assignment *node = nodes.items[i];
        // assert that all the top nodes are const assignments. TODO
        assert(node->kind == AST_CONST_ASSIGNMENT);

        // assumes that all top const assignments are functions
        // also assumes no duplicate names.
        fprintf(f, "void "SV_Fmt"(void) {\n", SV_Arg(node->name));

        for (size_t j = 0; j < node->function.expressions.count; j++) {
            AST_Node_Expression *expr = node->function.expressions.items[j];

            AST_Node_Call_Function *call = expr->sub_expression;
            // assume this is a function call.
            assert(call->kind == AST_CALL_FUNCTION);

            fprintf(f, "    "SV_Fmt"(", call->function_name);
            for (size_t k = 0; k < call->arguments->args.count; k++) {
                AST_Node_String_Lit *str_lit = call->arguments->args.items[k];
                // assert the argument is a string lit
                assert(str_lit->kind == AST_STRING_LIT);

                if (k != 0) fprintf(f, ", ");
                fprintf(f, SV_Fmt, SV_Arg(str_lit->literal));
            }
            fprintf(f, ");\n", call->function_name);
        }

        fprintf(f, "}\n");
    }

    fclose(f);


    int res = system("gcc -o output "OUTPUT_C_FILENAME);
    if (res) {
        perror("error in compiling output");
        assert(False && "TODO figure out error reporting");
    }

    printf("Successfully compiled file!\n");

    assert(context == get_context() && "Make sure the context is fine");
    Arena_free(&context->ast_arena);
    Arena_free(&context->string_arena);
    free(file.data);

    return 0;
}
