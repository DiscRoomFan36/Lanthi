
#ifndef COMPILER_H_
#define COMPILER_H_


#include "String_View.h"
#include "AST.h"

void maybe_make_folder(const char *folder_name);
void compile_ast(AST_Node_ptr_Array ast, SV output_name);


#endif // COMPILER_H_
