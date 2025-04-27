
#ifndef AST_H_
#define AST_H_


#include "ints.h"
#include "String_View.h"

#include "tokenizer.h"


// This file is all over the place...
// TODO think harder about all this stuff


typedef enum AST_Node_Kind {
    AST_NULL = 0,
    AST_FUNCTION,
    AST_CALL_FUNCTION,
    AST_CONST_ASSIGNMENT,
    AST_ARGUMENT,
    AST_STRING_LIT,
} AST_Node_Kind;

// the base node, cast this to whatever kind you find inside.
typedef struct AST_Node {
    AST_Node_Kind kind;
} AST_Node;

typedef struct AST_Node_ptr_Array {
    AST_Node **items;
    s64 count;
    s64 capacity;
} AST_Node_ptr_Array;



typedef struct AST_Node_Function {
    AST_Node_Kind kind;

    // TODO arguments
    // AST_Node_ptr_Array arguments;
    // TODO return values
    // AST_Node *return_value;

    // a function takes arguments, and runs them through an array of expressions.
    // TODO could this just be a block or something?
    AST_Node_ptr_Array expressions;
} AST_Node_Function;

typedef struct AST_Node_Const_Assignment {
    AST_Node_Kind kind;

    // the ident were assigning to, (TODO something about array indexing)
    SV name;

    // the expression on the othe side, could be a function, or something else.
    AST_Node *expression;
} AST_Node_Const_Assignment;


typedef struct AST_Node_Call_Function {
    AST_Node_Kind kind;
    // just the name? really?
    SV function_name;

    AST_Node_ptr_Array args;
} AST_Node_Call_Function;


typedef struct AST_Node_String_Lit {
    AST_Node_Kind kind;

    SV literal;
} AST_Node_String_Lit;



AST_Node_ptr_Array tokenizer_to_AST(Tokenizer *t);

void print_program(AST_Node_ptr_Array array);


#endif // AST_H_
