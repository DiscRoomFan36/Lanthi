
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "arena.h"
#include "context.h"

#include "AST.h"
#include "Report_Error.h"


#define local static

// ----------------
// function headers
// ----------------

local s64 get_operator_precedence(Token token);
local bool32 check_is_function_declaration(Tokenizer *t, s64 ahead);
local AST_Node_Function *parse_function(Tokenizer *t);
local AST_Node_ptr_Array parse_arguments(Tokenizer *t);
local AST_Node *parse_expression(Tokenizer *t);
local AST_Node *parse_subexpression(Tokenizer *t);



// TODO put this somewhere smarter
// TODO put this somewhere smarter
// TODO put this somewhere smarter
// TODO put this somewhere smarter
// TODO put this somewhere smarter
local SV arena_SV_dup(Arena *a, SV s) {
    SV result;
    result.data = Arena_alloc(a, s.size);
    result.size = s.size;

    for (s64 i = 0; i < s.size; i++) result.data[i] = s.data[i];

    return result;
}


// ------------------
// new node functions
// ------------------

AST_Node_Ident *new_node_ident(Token ident_token) {
    Context *context = get_context();

    AST_Node_Ident *node = Arena_alloc(&context->ast_arena, sizeof(AST_Node_Ident));
    node->kind = AST_IDENT;
    node->ident = arena_SV_dup(&context->string_arena, ident_token.text);

    return node;
}

AST_Node_String_Lit *new_node_string_lit(Token string_lit_token) {
    Context *context = get_context();

    AST_Node_String_Lit *node = Arena_alloc(&context->ast_arena, sizeof(AST_Node_String_Lit));
    node->kind = AST_STRING_LIT;
    node->literal = arena_SV_dup(&context->string_arena, string_lit_token.text);

    return node;
}

AST_Node_Const_Assignment *new_node_decl(void) {
    Context *context = get_context();

    AST_Node_Const_Assignment *node = Arena_alloc(&context->ast_arena, sizeof(AST_Node_Const_Assignment));
    node->kind = AST_CONST_ASSIGNMENT;
    // we dont do any other setup here

    return node;
}

AST_Node_Function *new_node_function(void) {
    Context *context = get_context();

    AST_Node_Function *node = Arena_calloc(&context->ast_arena, sizeof(AST_Node_Function));
    node->kind = AST_FUNCTION;
    // node->expressions = {0}; set to zero

    return node;
}

AST_Node_Call_Function *new_node_call_function(void) {
    Context *context = get_context();

    AST_Node_Call_Function *node = Arena_calloc(&context->ast_arena, sizeof(AST_Node_Call_Function));
    node->kind = AST_CALL_FUNCTION;
    // node->left = Null;
    // node->args = {0}; set to zero

    return node;
}



// TODO do this parseing
// https://www.youtube.com/watch?v=MnctEW1oL-E
// 1:13:55

local s64 get_operator_precedence(Token token) {
    switch ((int)token.kind) {
        case '+':
        case '-':
            return 1;

        case '*':
        case '/':
            return 2;

        case '[': assert(False && "[ dose not have precedence");
        case '(': assert(False && "( dose not have precedence");

        // less than the others, this should happen last.
        case '=':
            return 0;
    }

    // TODO use token_to_name here,. 
    printf("got token %s\n", token_to_name(token));
    assert(False && "unknown token");
}



// TODO make all the 'new' functions
// AST_Node_Const_Assignment



// start from ahead, check if its a function declaration.
local bool32 check_is_function_declaration(Tokenizer *t, s64 ahead) {

    // functions start with (...)
    if (peek_token(t, ahead).kind != '(') return False;

    ahead += 1;
    // find the other side of the (
    while (True) {
        Token token = peek_token(t, ahead);
        if (token.kind == ')') break;
        if (token.kind == TK_Eof) {
            report_AST_error(t, "expected ')' found EOF", "close your parens");
        }
        ahead += 1;
    }

    assert(peek_token(t, ahead).kind == ')');

    ahead += 1;

    // TODO check for returns

    // if its a brace, its a function
    if (peek_token(t, ahead).kind != '{') return False;

    return True;
}

local AST_Node_Function *parse_function(Tokenizer *t) {
    Context *context = get_context();

    AST_Node_Function *func = Arena_calloc(&context->ast_arena, sizeof(AST_Node_Function));
    func->kind = AST_FUNCTION;
    // init expressions to zero.
    // func->expressions = {0};

    assert(peek_token(t, 0).kind == '(');

    Token token;

    token = peek_token(t, 1);
    if (token.kind != ')') report_AST_error(t, "we currently do not handle function arguments", NULL);


    token = peek_token(t, 2);
    if (token.kind != '{') report_AST_error(t, "we currently do not handle function return values.", NULL);

    // setup expression parseing
    take_token(t, 3);

    while (peek_next_token(t).kind != '}') {
        AST_Node *expr = parse_expression(t);
        if (peek_next_token(t).kind != ';') {
            report_AST_error(t, "statement must end with ';'", "end lines with ';', TODO this might break functions in functions");
        }
        take_next_token(t);
        arena_da_append(&context->ast_arena, &func->expressions, expr);
    }

    return func;
}


local AST_Node_ptr_Array parse_arguments(Tokenizer *t) {
    Context *context = get_context();

    AST_Node_ptr_Array results = {0};

    assert(take_next_token(t).kind == '(' && "this is where i should be");

    AST_Node *argument = parse_expression(t);
    arena_da_append(&context->ast_arena, &results, argument);

    Token token = take_next_token(t);

    if (token.kind == ',') {
        report_AST_unexpected_token(t, "parse arguments", token, "we dont currently handle multiple arguments");
    }

    if (token.kind != ')') {
        report_AST_unexpected_token(t, "parse arguments", token, "must finish arguments with ')'");
    }

    return results;
}


local AST_Node *parse_subexpression(Tokenizer *t) {
    Token token = peek_next_token(t);

    if (token.kind == TK_String_Lit) {
        take_next_token(t);
        return (AST_Node*) new_node_string_lit(token);
    }

    if (token.kind == TK_Ident) {
        // check the next, it could be a function like...
        take_next_token(t);
        Token next = peek_next_token(t);

        if (next.kind == '(') {
            // its a function call

            AST_Node_Call_Function *call_func = new_node_call_function();
            call_func->left = (AST_Node*) new_node_ident(token);
            // call_func->args = {0};


            AST_Node_ptr_Array args = parse_arguments(t);
            call_func->args = args;

            assert(peek_next_token(t).kind != '(' && "no double functions or what not. maybe make call function a recursive thing??? tree down to the right.");
            return (AST_Node*) call_func;
        }

        // if were here, the node was just a token.
        return (AST_Node*) new_node_ident(token);
    }

    if (token.kind == '(') {
        // it might be a function.
        if (check_is_function_declaration(t, 0)) {
            // it must be a function
            return (AST_Node*) parse_function(t);
        }

        assert(False && "subexpr dose not parse '(' for now, maybe parse expression should handle this...");
    }


    report_AST_unexpected_token(t, "subexpression", token, NULL);
}


local AST_Node *parse_expression(Tokenizer *t) {
    AST_Node *left = parse_subexpression(t);
    assert(left != NULL && "i dont know whats happening");

    // if the left was a function. we stop. no ';' after function
    if (left->kind == AST_FUNCTION) {
        if (peek_next_token(t).kind == ';') {
            report_AST_unexpected_token(t, "parse expression after a function was parsed", peek_next_token(t), "no ';' after functions.");
        }
        return left;
    }

    Token token = peek_next_token(t);


    // end of parse expression chars
    if (token.kind == ';') return left;
    if (token.kind == ')') return left;
    if (token.kind == ',') return left;
    if (token.kind == ']') return left;
    // if (token.kind == '') return left;

    if (token.kind == ':') {
        take_next_token(t);
        // its a declaration.
        // TODO parse more thoroughly
        Token after_dec = peek_next_token(t);

        assert(after_dec.kind == ':' && "we only handle const assign");
        take_next_token(t);

        AST_Node_Const_Assignment *decl = new_node_decl();
        decl->left = left;

        AST_Node *right = parse_expression(t);

        assert(right != NULL && "must have a right side...");

        decl->right = right;
    }


    report_AST_unexpected_token(t, "parse expression next character", token, NULL);

    // Token token = peek_token(t, 0);

    // if (token.kind == TK_Ident) {
    //     Token next = peek_token(t, 1);

    //     if (next.kind == ':') {
    //         Token next_next_token = peek_token(t, 2);
    //         if (next_next_token.kind == ':') {

    //             // bool32 is_function_dec = check_is_function_declaration(t, 3);

    //             AST_Node_Const_Assignment *assign = Arena_alloc(&context->ast_arena, sizeof(AST_Node_Const_Assignment));

    //             // take the next tokens, to get parse_expression in the right place
    //             take_token(t, 3);

    //             // TODO imdeietly
    //             // TODO imdeietly
    //             // TODO imdeietly
    //             // TODO imdeietly
    //             // TODO imdeietly
    //             assign->kind = AST_CONST_ASSIGNMENT;
    //             assign->name = token.text;
    //             assign->expression = parse_expression(t);
    //             return (AST_Node*) assign;
    //         }

    //         report_AST_error(t, "expected ':'", "we only handle :: for now");

    //         // ok, its a definition, hand the job to someone else.
    //         // return parse_declaration_or_assignment(t);
    //     }

    //     report_AST_unexpected_token(t, "parse expression when parseing token", next, "we only handle ':' after Ident for now");
    // }

    // if (token.kind == '(') {
    //     bool32 is_function = check_is_function_declaration(t, 0);

    //     if (is_function) { return (AST_Node*) parse_function(t); }

    //     report_AST_error(t, "saw ( but it wasn't a function", "TODO parens better");
    // }


    // report_AST_unexpected_token(t, "parse expression", token, "expected an ident or something.");
}

AST_Node_ptr_Array tokenizer_to_AST(Tokenizer *t) {
    Context *context = get_context();

    AST_Node_ptr_Array nodes = {0};

    // we are in global scope, loop over all expressions and only stop when we hit EOF.
    while (True) {
        Token tok = peek_next_token(t);
        if (tok.kind == TK_Eof) break;

        AST_Node *node = parse_expression(t);
        arena_da_append(&context->ast_arena, &nodes, node);
    }

    return nodes;
}




void print_program(AST_Node_ptr_Array array) {
    (void) array;

    assert(False && "TODO: 'print_program'");
}
