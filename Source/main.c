
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ints.h"
#include "String_View.h"
#include "dynamic_array.h"
#include "arena.h"
#include "context.h"

#include "Report_Error.h"
#include "tokenizer.h"
#include "AST.h"
#include "Compile.h"


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
    // should this assert? or maybe report an error,
    // this could happen if you try and read a really big file.
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
        fprintf(stderr, "USAGE: %s file\n", program_name);
        cleanup_and_exit(1);
    }

    const char *filename = argv[1];

    Context *context = get_context();

    // memset these to 0, even though the context grantees this.
    // just to make sure we know what were doing.
    memset(&context->string_arena, 0, sizeof(Arena));
    memset(&context->ast_arena,    0, sizeof(Arena));

    context->file = read_entire_file(filename);

    if (!context->file.data) {
        fprintf(stderr, "could not open file %s\n", filename);
        cleanup_and_exit(1);
    }


    Tokenizer tokenizer = new_tokenizer(filename, context->file);
    AST_Node_ptr_Array nodes = tokenizer_to_AST(&tokenizer);


    printf("finished converting into AST hurray!\n");
    printf("probably should do something with it...\n");

    for (s64 i = 0; i < nodes.count; i++) {
        print_node(nodes.items[i], 0);
    }

    printf("--------- Start compiling file ---------\n");
    compile_ast(nodes, SV_from_C_Str("output"));
    printf("--------- Successfully compiled file! ---------\n");


    assert(context == get_context() && "Make sure the context is fine");

    cleanup_and_exit(0);
}
