/*
 * IFJ + IAL projekt 2026/2027
 * Adam Havier (xhaviea00)
*/

#include "codegen.h"
#include <stdio.h>

void codegen_string(const unsigned char *text, size_t length)
{
    printf("string@");

    for (size_t i = 0; i < length; i++)
    {
        unsigned char znak = text[i];

        if (znak <= 32 || znak == '#' || znak == '\\')
        {
            printf("\\%03u", (unsigned int)znak);
        }
        else
        {
            putchar(znak);
        }
    }
}

void codegen_int(long long value)
{
    printf("int@%lld", value);
}

void codegen_float(double value)
{
    printf("float@%a", value);
}

void codegen_bool(bool value)
{
    if (value)
    {
        printf("bool@true");
    }
    else
    {
        printf("bool@false");
    }
}

void codegen_variable(CodegenFrame frame, const char *name)
{
    const char *prefix;
    switch(frame)
    {
        case CODEGEN_GF:
            prefix = "GF";
            break;
        case CODEGEN_LF:
            prefix = "LF";
            break;
        case CODEGEN_TF:
            prefix = "TF";
            break;
        default:
            return;
    }
    printf("%s@%s", prefix, name);
}

void codegen_defvar(CodegenFrame frame, const char *name)
{
    printf("DEFVAR ");
    codegen_variable(frame, name);
    printf("\n");
}

void codegen_move_int(CodegenFrame frame, const char *name, long long value)
{
    printf("MOVE ");
    codegen_variable(frame, name);
    printf(" ");
    codegen_int(value);
    printf("\n");
}

void codegen_move_var(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name)
{
    printf("MOVE ");
    codegen_variable(dest_frame, dest_name);
    printf(" ");
    codegen_variable(source_frame, source_name);
    printf("\n");
}

bool codegen_move_literal(CodegenFrame frame, const char *name, const ASTNode *node)
{
    if (node == NULL || node->type != AST_INT_LITERAL) {
        return false;
    }
    codegen_move_int(frame, name, node->int_value);
    return true;
}

bool codegen_var_declaration(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL || node->type != AST_VAR_DECLARATION || node->text == NULL || node->children_count != 1 || node->children == NULL){
        return false;
    }
    const ASTNode *initializer = node->children[0];
    if (initializer == NULL || initializer->type != AST_INT_LITERAL) {
        return false;
    }
    codegen_defvar(frame, node->text);
    return codegen_move_literal(frame, node->text, initializer);
}

static bool codegen_arithmetic_expression(CodegenFrame frame, const char *dest, const ASTNode *expr)
{
    if (expr == NULL || expr->type != AST_BINARY_OP || (expr->operator != AST_OP_ADD && expr->operator != AST_OP_SUB && expr->operator != AST_OP_MUL) || expr->children_count != 2 || expr->children == NULL) {
        return false;
    }
    const ASTNode *left = expr->children[0];
    const ASTNode *right = expr->children[1];
    if (left == NULL || right == NULL || right->type != AST_INT_LITERAL) {
        return false;
    }
    const char *source_name;
    if (left->type == AST_IDENTIFIER) {
        if (left->text == NULL) {
            return false;
        }
        source_name = left->text;
    }
    else {
        if (!codegen_arithmetic_expression(frame, dest, left)) {
            return false;
        }
        source_name = dest;
    }
    switch (expr->operator) {
    case AST_OP_ADD:
        codegen_add_int(frame, dest, frame, source_name, right->int_value);
        break;
    case AST_OP_SUB:
        codegen_sub_int(frame, dest, frame, source_name, right->int_value);
        break;
    case AST_OP_MUL:
        codegen_mul_int(frame, dest, frame, source_name, right->int_value);
        break;
    default:
        return false;
    }
    return true;
}

bool codegen_assignment(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL || node->type != AST_ASSIGN || node->text == NULL || node->children_count != 1 || node->children == NULL) {
        return false;
    }
    const ASTNode *value = node->children[0];
    if (value != NULL && value->type == AST_IDENTIFIER) {
        if (value->text == NULL) {
            return false;
        }
        codegen_move_var(frame, node->text, frame, value->text);
        return true;
    }
    if (value != NULL && value->type == AST_BINARY_OP) {
        return codegen_arithmetic_expression(frame, node->text, value);
    }
    return codegen_move_literal(frame, node->text, value);
}

bool codegen_block(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL || node->type != AST_BLOCK || (node->children_count > 0 && node->children == NULL)) {
        return false;
    }
    for (size_t i = 0; i < node->children_count; i++)
    {
        const ASTNode *statement = node->children[i];
        if (statement == NULL){
            return false;
        }
        switch (statement->type) {
        case AST_VAR_DECLARATION:
            if (!codegen_var_declaration(frame, statement)) {
                return false;
            }
            break;
        case AST_ASSIGN:
            if (!codegen_assignment(frame, statement)) {
                return false;
            }
            break;
        default:
            return false;
        }
    }
    return true;
}

void codegen_add_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value)
{
    printf("ADD ");
    codegen_variable(dest_frame, dest_name);
    printf(" ");
    codegen_variable(source_frame, source_name);
    printf(" ");
    codegen_int(value);
    printf("\n");
}

void codegen_sub_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value)
{
    printf("SUB ");
    codegen_variable(dest_frame, dest_name);
    printf(" ");
    codegen_variable(source_frame, source_name);
    printf(" ");
    codegen_int(value);
    printf("\n");
}

void codegen_mul_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value)
{
    printf("MUL ");
    codegen_variable(dest_frame, dest_name);
    printf(" ");
    codegen_variable(source_frame, source_name);
    printf(" ");
    codegen_int(value);
    printf("\n");
}

void codegen_write_var(CodegenFrame frame, const char *name)
{
    printf("WRITE ");
    codegen_variable(frame, name);
    printf("\n");
}

void codegen_label(const char *name)
{
    printf("LABEL %s\n", name);
}

void codegen_jump(const char *name)
{
    printf("JUMP %s\n", name);
}

void codegen_jumpifeq_int(const char *label, CodegenFrame frame, const char *name, long long value)
{
    printf("JUMPIFEQ %s ", label);
    codegen_variable(frame, name);
    printf(" ");
    codegen_int(value);
    printf("\n");
}

void codegen_call(const char *label)
{
    printf("CALL %s\n", label);
}

void codegen_return()
{
    printf("RETURN\n");
}