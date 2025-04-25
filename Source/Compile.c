

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "Compile.h"
#include "context.h"



void maybe_make_folder(const char *folder_name) {
    char buffer[512] = {0};
    sprintf(buffer, "mkdir -p %s", folder_name);

    if (system(buffer)) {
        perror("couldn't maybe make folder.");
        exit(1);
    }
}

#define BUILD_FOLDER "./lanthi_build"
#define OUTPUT_C_FILENAME   BUILD_FOLDER "/intermediate_c_generated.c"

void compile_ast(AST_Node_ptr_Array ast, SV output_name) {
    maybe_make_folder(BUILD_FOLDER);

    FILE *f = fopen(OUTPUT_C_FILENAME, "wb");
    if (!f) {
        perror("Cannot open "OUTPUT_C_FILENAME);
        assert(False && "TODO figure out error reporting");
    }


    fprintf(f, "#include <stdio.h>\n");

    // pre define all functions for C backend.
    for (s64 i = 0; i < ast.count; i++) {
        AST_Node_Const_Assignment *node = (AST_Node_Const_Assignment*) ast.items[i];
        // assert that all the top nodes are const assignments. TODO
        assert(node->kind == AST_CONST_ASSIGNMENT);

        // assumes that all top const assignments are functions
        // also assumes no duplicate names.
        fprintf(f, "void "SV_Fmt"(void);\n", SV_Arg(node->name));

        // at this point also recur deeper into the structures to find more functions.
    }


    for (s64 i = 0; i < ast.count; i++) {
        AST_Node_Const_Assignment *node = (AST_Node_Const_Assignment*) ast.items[i];
        // assert that all the top nodes are const assignments. TODO
        assert(node->kind == AST_CONST_ASSIGNMENT);

        // assumes that all top const assignments are functions
        // also assumes no duplicate names.
        fprintf(f, "void "SV_Fmt"(void) {\n", SV_Arg(node->name));

        for (s64 j = 0; j < node->function.expressions.count; j++) {
            AST_Node_Expression *expr = node->function.expressions.items[j];

            AST_Node_Call_Function *call = (AST_Node_Call_Function*) expr->sub_expression;
            // assume this is a function call.
            assert(call->kind == AST_CALL_FUNCTION);

            fprintf(f, "    "SV_Fmt"(", SV_Arg(call->function_name));
            for (s64 k = 0; k < call->arguments->args.count; k++) {
                AST_Node_String_Lit *str_lit = (AST_Node_String_Lit*) call->arguments->args.items[k];
                // assert the argument is a string lit
                assert(str_lit->kind == AST_STRING_LIT);

                if (k != 0) fprintf(f, ", ");
                fprintf(f, SV_Fmt, SV_Arg(str_lit->literal));
            }
            fprintf(f, ");\n");
        }

        fprintf(f, "}\n");
    }

    fclose(f);

    char buffer[512] = {0};
    int n = snprintf(buffer, 512, "gcc -o "BUILD_FOLDER"/"SV_Fmt" "OUTPUT_C_FILENAME, SV_Arg(output_name));
    assert(n != 512);
    int res = system(buffer);
    if (res) {
        perror("error in compiling output");
        assert(False && "TODO figure out error reporting");
    }
}


