
#include "ints.h"
#include "String_View.h"


// This file is all over the place...
// TODO think harder about all this stuff


typedef enum AST_Node_Kind {
    AST_NULL = 0,
    AST_EXPRESSION,
    AST_FUNCTION,
    AST_CALL_FUNCTION,
    AST_CONST_ASSIGNMENT,
    AST_ARGUMENT,
    AST_STRING_LIT,
} AST_Node_Kind;

typedef struct AST_Node {
    AST_Node_Kind kind;
} AST_Node;

typedef struct AST_Node_ptr_Array {
    AST_Node **items;
    s64 count;
    s64 capacity;
} AST_Node_ptr_Array;

typedef struct AST_Node_Expression {
    AST_Node_Kind kind;

    AST_Node *sub_expression;
} AST_Node_Expression;



typedef struct AST_Node_Function {
    AST_Node_Kind kind;

    // TODO express arguments somehow
    // AST_Node_Arguments_Kind?
    // something different from AST_Node_Argument, 

    // a function takes arguments, and runs them through an array of expressions.
    struct {
        // we know all the nodes are going to be of type
        AST_Node_Expression **items;
        s64 count;
        s64 capacity;
    } expressions;
} AST_Node_Function;

typedef struct AST_Node_Const_Assignment {
    AST_Node_Kind kind;

    SV name;
    // can only const assign functions
    // TODO make a union or something
    // is this an expression?
    AST_Node_Function function;
} AST_Node_Const_Assignment;


// The arguments a function is currently taking to be called.
// not the proper arguments for that function.
typedef struct AST_Node_Argument {
    AST_Node_Kind kind;

    struct {
        AST_Node **items;
        s64 count;
        s64 capacity;
    } args;

} AST_Node_Argument;


typedef struct AST_Node_Call_Function {
    AST_Node_Kind kind;
    // just the name? really?
    SV function_name;
    AST_Node_Argument *arguments;
} AST_Node_Call_Function;


typedef struct AST_Node_String_Lit {
    // i kinda want this system to be like
    /*
        if (node.kind == AST_String_Lit) {
            AST_Node_String_Lit *node = (AST_Node_String_Lit*) node;
            // use the node
        }
    */

    AST_Node_Kind kind;
    SV literal;
} AST_Node_String_Lit;
