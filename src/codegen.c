/*
 * IFJ + IAL projekt 2026/2027
 * Adam Havier (xhaviea00)
*/

#include "codegen.h"
#include <stdio.h>
#include <string.h>

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

void codegen_idiv_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value)
{
    printf("IDIV ");
    codegen_variable(dest_frame, dest_name);
    printf(" ");
    codegen_variable(source_frame, source_name);
    printf(" ");
    codegen_int(value);
    printf("\n");
}

void codegen_add_vars(CodegenFrame dest_frame, const char *dest_name, CodegenFrame lsource_frame, const char *lsource_name, CodegenFrame rsource_frame, const char *rsource_name)
{
    printf("ADD ");
    codegen_variable(dest_frame, dest_name);
    printf(" ");
    codegen_variable(lsource_frame, lsource_name);
    printf(" ");
    codegen_variable(rsource_frame, rsource_name);
    printf("\n");
}

void codegen_lt_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value)
{
    printf("LT ");
    codegen_variable(dest_frame, dest_name);
    printf(" ");
    codegen_variable(source_frame, source_name);
    printf(" ");
    codegen_int(value);
    printf("\n");
}

void codegen_gt_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value)
{
    printf("GT ");
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

void codegen_jumpifeq_bool(const char *label, CodegenFrame frame, const char *name, bool value)
{
    printf("JUMPIFEQ %s ", label);
    codegen_variable(frame, name);
    printf(" ");
    codegen_bool(value);
    printf("\n");
}

void codegen_jumpifneq_int(const char *label, CodegenFrame frame, const char *name, long long value)
{
    printf("JUMPIFNEQ %s ", label);
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

bool codegen_move_literal(CodegenFrame frame, const char *name, const ASTNode *node)
{
    if (node == NULL || node->type != AST_INT_LITERAL) {
        return false;
    }
    codegen_move_int(frame, name, node->int_value);
    return true;
}

static bool codegen_arithmetic_expression(CodegenFrame frame, const char *dest, const ASTNode *expr)
{
    if (expr == NULL || expr->type != AST_BINARY_OP || (expr->operator != AST_OP_ADD && expr->operator != AST_OP_SUB && expr->operator != AST_OP_MUL && expr->operator != AST_OP_DIV) || expr->children_count != 2 || expr->children == NULL) {
        return false;
    }
    const ASTNode *left = expr->children[0];
    const ASTNode *right = expr->children[1];
    if (left == NULL || right == NULL) {
        return false;
    }
    const char *source_name;
    if (right->type == AST_IDENTIFIER) {
        if (left->type != AST_IDENTIFIER || left->text == NULL || right->text == NULL || expr->operator != AST_OP_ADD) {
            return false;
        }
        codegen_add_vars(frame, dest, frame, left->text, frame, right->text);
        return true;
    }
    if (right->type != AST_INT_LITERAL) {
        return false;
    }
    if (left->type == AST_IDENTIFIER) {
        if (left->text == NULL) {
            return false;
        }
        source_name = left->text;
    }
    else if (left->type == AST_INT_LITERAL) {
        codegen_move_int(frame, dest, left->int_value);
        source_name = dest;
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
    case AST_OP_DIV:
        codegen_idiv_int(frame, dest, frame, source_name, right->int_value);
        break;
    default:
        return false;
    }
    return true;
}

bool codegen_var_declaration(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL || node->type != AST_VAR_DECLARATION || node->text == NULL || node->children_count != 1 || node->children == NULL){
        return false;
    }
    const ASTNode *initializer = node->children[0];
    if (initializer == NULL || (initializer->type != AST_INT_LITERAL && initializer->type != AST_BINARY_OP)) {
        return false;
    }
    codegen_defvar(frame, node->text);
    if (initializer->type == AST_INT_LITERAL) {
        return codegen_move_literal(frame, node->text, initializer);
    }
    else if (initializer->type == AST_BINARY_OP) {
        return codegen_arithmetic_expression(frame, node->text, initializer);
    }
    else {
        return false;
    }
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

bool codegen_print(CodegenFrame frame, const ASTNode *node){
    if (node == NULL || node->type != AST_CALL || node->text == NULL || strcmp(node->text, "print") != 0) {
        return false;
    }
    if (node->children == NULL || node->children_count != 1) {
        return false;
    }
    const ASTNode *args = node->children[0];
    if (args == NULL || args->type != AST_ARG_LIST || args->children_count != 1 || args->children == NULL) {
        return false;
    }
    const ASTNode *arg = args->children[0];
    if (arg == NULL) {
        return false;
    }
    if (arg->type == AST_IDENTIFIER) {
        if (arg->text == NULL) {
            return false;
        }
        codegen_write_var(frame, arg->text);
    }
    else if (arg->type == AST_STRING_LITERAL) {
        if (arg->text == NULL) {
            return false;
        }
        printf("WRITE ");
        codegen_string((const unsigned char *)arg->text, strlen(arg->text));
        putchar('\n');
    }
    else if (arg->type == AST_INT_LITERAL) {
        printf("WRITE ");
        codegen_int(arg->int_value);
        putchar('\n');
    }
    else {
        return false;
    }
    return true;
}

static bool codegen_int_operand(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL) {
        return false;
    }
    if (node->type == AST_IDENTIFIER && node->text != NULL) {
        codegen_variable(frame, node->text);
    }
    else if (node->type == AST_INT_LITERAL) {
        codegen_int(node->int_value);
    }
    else {
        return false;
    }
    return true;
}

static const ASTNode *codegen_int_comparison(const ASTNode *condition) {
    if (condition == NULL || condition->type != AST_EXPR) {
        return NULL;
    }
    if (condition->children == NULL || condition->children_count != 1) {
        return NULL;
    }
    const ASTNode *expression = condition->children[0];
    if (expression == NULL || expression->type != AST_BINARY_OP || (expression->operator != AST_OP_EQUAL && expression->operator != AST_OP_NOT_EQUAL && expression->operator != AST_OP_LESS && expression->operator != AST_OP_LESS_EQUAL && expression->operator != AST_OP_GREATER && expression->operator != AST_OP_GREATER_EQUAL)) {
        return NULL;
    }
    if (expression->children == NULL || expression->children_count != 2) {
        return NULL;
    }
    const ASTNode *left = expression->children[0];
    const ASTNode *right = expression->children[1];
    if (left == NULL || (left->type != AST_IDENTIFIER && left->type != AST_INT_LITERAL) || right == NULL || (right->type != AST_IDENTIFIER && right->type != AST_INT_LITERAL)) {
        return NULL;
    }
    if ((right->type == AST_IDENTIFIER && right->text == NULL) || (left->type == AST_IDENTIFIER && left->text == NULL)) {
        return NULL;
    }
    return expression;
}

static bool codegen_jump_if_true(CodegenFrame frame, const ASTNode *condition, const char *label)
{
    const ASTNode *expression = codegen_int_comparison(condition);
    if (expression == NULL || label == NULL) {
        return false;
    }
    const ASTNode *left = expression->children[0];
    const ASTNode *right = expression->children[1];
    if (expression->operator == AST_OP_EQUAL) {
        printf("JUMPIFEQ %s ", label);
        if (!codegen_int_operand(frame, left)) {
            return false;
        }
        printf(" ");
        if (!codegen_int_operand(frame, right)) {
            return false;
        }
        putchar('\n');
    }
    else if (expression->operator == AST_OP_NOT_EQUAL) {
        printf("JUMPIFNEQ %s ", label);
        if (!codegen_int_operand(frame, left)) {
            return false;
        }
        printf(" ");
        if (!codegen_int_operand(frame, right)) {
            return false;
        }
        putchar('\n');
    }
    else if (expression->operator == AST_OP_LESS) {
        printf("LT ");
        codegen_variable(frame, "%condition");
        printf(" ");
        if (!codegen_int_operand(frame, left)) {
            return false;
        }
        printf(" ");
        if (!codegen_int_operand(frame, right)) {
            return false;
        }
        putchar('\n');
        codegen_jumpifeq_bool(label, frame, "%condition", true);
    }
    else if (expression->operator == AST_OP_LESS_EQUAL) {
        printf("GT ");
        codegen_variable(frame, "%condition");
        printf(" ");
        if (!codegen_int_operand(frame, left)) {
            return false;
        }
        printf(" ");
        if (!codegen_int_operand(frame, right)) {
            return false;
        }
        putchar('\n');
        codegen_jumpifeq_bool(label, frame, "%condition", false);
    }
    else if (expression->operator == AST_OP_GREATER) {
        printf("GT ");
        codegen_variable(frame, "%condition");
        printf(" ");
        if (!codegen_int_operand(frame, left)) {
            return false;
        }
        printf(" ");
        if (!codegen_int_operand(frame, right)) {
            return false;
        }
        putchar('\n');
        codegen_jumpifeq_bool(label, frame, "%condition", true);
    }
    else if (expression->operator == AST_OP_GREATER_EQUAL) {
        printf("LT ");
        codegen_variable(frame, "%condition");
        printf(" ");
        if (!codegen_int_operand(frame, left)) {
            return false;
        }
        printf(" ");
        if (!codegen_int_operand(frame, right)) {
            return false;
        }
        putchar('\n');
        codegen_jumpifeq_bool(label, frame, "%condition", false);
    }
    else {
        return false;
    }
    return true;
}

bool codegen_if(CodegenFrame frame, const ASTNode *node) {
    if (node == NULL || node->type != AST_IF || node->children == NULL || node->children_count != 3) {
        return false;
    }
    const ASTNode *condition = node->children[0];
    const ASTNode *then_block = node->children[1];
    const ASTNode *else_block = node->children[2];
    const ASTNode *expression = codegen_int_comparison(condition);
    if (expression == NULL || then_block == NULL || then_block->type != AST_BLOCK || else_block == NULL || else_block->type != AST_BLOCK) {
        return false;
    }
    static unsigned long next_if_id = 0;
    const unsigned long id = next_if_id++;
    char then_label[40];
    char end_label[40];
    snprintf(then_label, sizeof then_label, "if_then_%lu", id);
    snprintf(end_label, sizeof end_label, "if_end_%lu", id);
    if (!codegen_jump_if_true(frame, condition, then_label)) {
        return false;
    }
    if (!codegen_block(frame, else_block)) {
        return false;
    }
    codegen_jump(end_label);
    codegen_label(then_label);
    if (!codegen_block(frame, then_block)) {
        return false;
    }
    codegen_label(end_label);
    return true;
}

bool codegen_while(CodegenFrame frame, const ASTNode *node){
    if (node == NULL || node->type != AST_WHILE || node->children == NULL || node->children_count != 2) {
        return false;
    }
    const ASTNode *condition = node->children[0];
    const ASTNode *block = node->children[1];
    const ASTNode *expression = codegen_int_comparison(condition);
    if (expression == NULL || block == NULL || block->type != AST_BLOCK) {
        return false;
    }
    static unsigned long next_while_id = 0;
    const unsigned long id = next_while_id++;
    char start_label[40];
    char body_label[40];
    char end_label[40];
    snprintf(start_label, sizeof start_label, "while_start_%lu", id);
    snprintf(body_label, sizeof body_label, "while_body_%lu", id);
    snprintf(end_label, sizeof end_label, "while_end_%lu", id);
    codegen_label(start_label);
    if (!codegen_jump_if_true(frame, condition, body_label)) {
        return false;
    }
    codegen_jump(end_label);
    codegen_label(body_label);
    if (!codegen_block(frame, block)) {
        return false;
    }
    codegen_jump(start_label);
    codegen_label(end_label);
    return true;
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
        case AST_CALL:
            if (!codegen_print(frame, statement)) {
                return false;
            }
            break;
        case AST_IF:
            if (!codegen_if(frame, statement)) {
                return false;
            }
            break;
        case AST_WHILE:
            if (!codegen_while(frame, statement)) {
                return false;
            }
            break;
        default:
            return false;
        }
    }
    return true;
}

bool codegen_program(const ASTree *tree)
{
    if (tree == NULL || tree->root == NULL) {
        return false;
    }
    const ASTNode *program = tree->root;
    for (size_t i = 0; i < program->children_count; i++)
    {
        const ASTNode *function = program->children[i];
        if (function != NULL && function->type == AST_FUNCTION && function->is_main) {
            if (function->children == NULL || function->children_count != 2) {
                return false;
            }
            const ASTNode *body = function->children[1];
            if (body != NULL && body->type == AST_BLOCK) {
                puts(".IFJcode26");
                puts("CREATEFRAME");
                puts("PUSHFRAME");
                codegen_defvar(CODEGEN_LF, "%condition");
                if (!codegen_block(CODEGEN_LF, body)) {
                    return false;
                }
                puts("POPFRAME");
                puts("EXIT int@0");
                return true;
            }
        }
    }
    return false;
}