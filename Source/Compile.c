

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "Compile.h"
#include "context.h"

// TODO common.h
#define local static

// ----------------
// function headers
// ----------------

local void compile_expression(FILE *f, AST_Node *nokind_node, int depth);
local void compile_statement(FILE *f, AST_Node *nokind_node, int depth);



// TODO put this somewhere smarter
void maybe_make_folder(const char *folder_name) {
    char buffer[512] = {0};
    sprintf(buffer, "mkdir -p %s", folder_name);

    if (system(buffer)) {
        perror("couldn't maybe make folder.");
        exit(1);
    }
}

// TODO put this somewhere smarter
#define BUILD_FOLDER "./lanthi_build"
#define OUTPUT_C_FILENAME   BUILD_FOLDER "/intermediate_c_generated.c"


local void print_spaces(FILE *f, int n) {
    for (int i = 0; i < n; i++) fprintf(f, "    ");
}





local void compile_expression(FILE *f, AST_Node *nokind_node, int depth) {

    switch (nokind_node->kind) {
    case AST_CONST_ASSIGNMENT: {
        AST_Node_Const_Assignment *node = (AST_Node_Const_Assignment*)nokind_node;

        if (node->right->kind == AST_FUNCTION) {
            // TODO we can probably skip this, its already handled maybe...
            // TODO should probably just pull out all the functions that we find in some step...
            assert(depth == 0);

            AST_Node_Function *right_node = (AST_Node_Function*)node->right;

            // make a function, and compile all the statements
            // TODO typechecking will get this, or do we make some 'get_compiled_name()' function?
            assert(node->left->kind == AST_IDENT);

            // assumes no returns an others, (TODO maybe that thing in compile_ast can come here?, prob not, dont factor to hard)
            fprintf(f, "void ");
            compile_expression(f, node->left, depth);
            fprintf(f, "(void) {\n");

            // recursively compile sub expressions, 
            for (s64 i = 0; i < right_node->expressions.count; i++) {
                compile_statement(f, right_node->expressions.items[i], depth+1);
            }

            print_spaces(f, depth);
            fprintf(f, "}");

            return;
        }
    }

    case AST_CALL_FUNCTION: {
        AST_Node_Call_Function *node = (AST_Node_Call_Function*)nokind_node;

        // we dont have to know whats on either side, just compile them, this only works with c,
        // TODO IR

        compile_expression(f, node->left, depth);
        fprintf(f, "(");
        for (s64 i = 0; i < node->args.count; i++) {
            if (i != 0) fprintf(f, ", ");
            compile_expression(f, node->args.items[i], depth);
        }
        fprintf(f, ")");
        return;
    }

    // TODO these are the same...? do something?
    case AST_IDENT: {
        AST_Node_Ident *node = (AST_Node_Ident*)nokind_node;
        fprintf(f, SV_Fmt, SV_Arg(node->ident));
        return;
    }
    case AST_STRING_LIT: {
        AST_Node_String_Lit *node = (AST_Node_String_Lit*)nokind_node;
        fprintf(f, SV_Fmt, SV_Arg(node->literal));
        return;
    }

    case AST_NULL: {
        assert(False && "AST node was null");
    }

    case AST_FUNCTION: {
        assert(False && "found Function in compile expression, this should be handled somewhere else.");
    }
    }

    assert(False && "Unknown kind in compile expression.");
}

// compile a single statement, must end with ';' if its not a function
local void compile_statement(FILE *f, AST_Node *nokind_node, int depth) {
    print_spaces(f, depth);

    compile_expression(f, nokind_node, depth);

    // if its a binop,
    if (nokind_node->kind == AST_CONST_ASSIGNMENT) {
        // and the right side is a function
        AST_Node_Const_Assignment *node = (AST_Node_Const_Assignment*)nokind_node;

        if (node->right->kind == AST_FUNCTION) {
            // dont print anything
        } else {
            // else do print ';'
            fprintf(f, ";");
        }
    } else {
        // the statement node cannot be a function, so we good. hopefully
        fprintf(f, ";");
    }

    // ending newline, always here.
    fprintf(f, "\n");
}



void compile_ast(AST_Node_ptr_Array ast, SV output_name) {
    maybe_make_folder(BUILD_FOLDER);

    // setup output file.
    // TODO use a String builder.
    FILE *f = fopen(OUTPUT_C_FILENAME, "wb");
    if (!f) {
        perror("Cannot open "OUTPUT_C_FILENAME);
        assert(False && "TODO figure out error reporting in Compile.c");
    }

    fprintf(f, "\n// START OF FILE\n\n");

    // prelude include stdio.h
    fprintf(f, "#include <stdio.h>\n\n");

    // predef all function for C backend.
    for (s64 i = 0; i < ast.count; i++) {
        AST_Node *nokind_node = ast.items[i];

        // top level is only for assignments, and macros, and maybe other things later
        // TODO have a 'type-checking phase' that dose error reporting on stuff that doesn't fit.
        // TODO just have an IR
        assert(nokind_node->kind == AST_CONST_ASSIGNMENT);

        AST_Node_Const_Assignment *node = (AST_Node_Const_Assignment*)nokind_node;

        // left must be a literal, also must be semi unique,
        // TODO typechecking
        assert(node->left->kind == AST_IDENT);
        AST_Node_Ident *left_node = (AST_Node_Ident*)node->left;

        if (node->right->kind == AST_FUNCTION) {
            fprintf(f, "void "SV_Fmt"(void);\n", SV_Arg(left_node->ident));

            // TODO recur deeper into the function to find more functions.
        } else {
            assert("TODO, top level decl is not a function");
        }
    }

    fprintf(f, "\n"); // just some extra spaceing

    // recursively compile all statements.
    for (s64 i = 0; i < ast.count; i++) {
        compile_statement(f, ast.items[i], 0);
        fprintf(f, "\n"); // just some extra spaceing
    }

    // finish the file.
    fprintf(f, "// END OF FILE\n");
    fclose(f);

    { // compile the c with gcc
        char buffer[512] = {0};

        // setup the command
        int n = snprintf(buffer, 512, "gcc -o "BUILD_FOLDER"/"SV_Fmt" "OUTPUT_C_FILENAME, SV_Arg(output_name));
        assert(n != 512);

        // call the compiler.
        int res = system(buffer);
        if (res) {
            perror("error in compiling output");
            assert(False && "TODO figure out error reporting");
        }
    }
}


