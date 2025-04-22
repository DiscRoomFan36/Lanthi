
#include "ints.h"
#include "String_View.h"


// This file is all over the place...
// TODO think harder about all this stuff


typedef enum AST_Node_Kind {
    AST_NULL = 0,
    AST_CONST_ASSIGNMENT,
    // AST_ASSIGNMENT,
} AST_Node_Kind;

typedef struct AST_Node {
    AST_Node_Kind kind;
    void *rest;
} AST_Node;

typedef struct AST_Node_Array {
    AST_Node *items;
    s64 count;
    s64 capacity;
} AST_Node_Array;

typedef struct AST_Node_Function {
    // TODO make this a statement array?
    AST_Node_Array statements;
} AST_Node_Function;

typedef struct AST_Node_Const_Assignment {
    SV name;
    // can only const assign functions
    // TODO make a union or something
    AST_Node_Function function;
} AST_Node_Const_Assignment;


