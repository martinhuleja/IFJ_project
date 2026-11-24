/*
 * IFJ + IAL projekt 2026/2027
 * Adam Havier (xhaviea00)
*/

#include "codegen.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct CodegenVariable {
    const char *source_name;
    char target_name[40];
    ASTDataType type;
    struct CodegenVariable *next;
} CodegenVariable;

typedef struct {
    CodegenVariable *variables;
    unsigned long next_variable_id;
    size_t block_depth;
} CodegenContext;

static CodegenContext codegen_context = {0};

static const CodegenVariable *codegen_find_variable(const CodegenVariable *variables, const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    while (variables != NULL) {
        if (strcmp(variables->source_name, name) == 0) {
            return variables;
        }
        variables = variables->next;
    }
    return NULL;
}

static CodegenVariable *codegen_create_variable(const char *name, unsigned long id, ASTDataType type)
{
    if (name == NULL) {
        return NULL;
    }
    CodegenVariable *variable = malloc(sizeof *variable);
    if (variable == NULL) {
        return NULL;
    }
    variable->source_name = name;
    snprintf(variable->target_name, sizeof variable->target_name, "%%var_%lu", id);
    variable->next = NULL;
    variable->type = type;
    return variable;
}

static void codegen_free_variables_until(CodegenVariable *variables, const CodegenVariable *stop)
{
    while (variables != stop) {
        CodegenVariable *next = variables->next;
        free(variables);
        variables = next;
    }
}

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

static bool codegen_operand(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL) {
        return false;
    }
    if (node->type == AST_IDENTIFIER && node->text != NULL) {
        const CodegenVariable *variable = codegen_find_variable(codegen_context.variables, node->text);
        if (variable == NULL) {
            return false;
        }
        codegen_variable(frame, variable->target_name);
    }
    else if (node->type == AST_INT_LITERAL) {
        codegen_int(node->int_value);
    }
    else if (node->type == AST_DOUBLE_LITERAL) {
        codegen_float(node->double_value);
    }
    else if (node->type == AST_STRING_LITERAL && node->text != NULL) {
        codegen_string((const unsigned char*)node->text, strlen(node->text));
    }
    else {
        return false;
    }
    return true;
}

static bool codegen_push_expression(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL) {
        return false;
    }
    if (node->type == AST_IDENTIFIER || node->type == AST_INT_LITERAL || node->type == AST_DOUBLE_LITERAL || node->type == AST_STRING_LITERAL) {
        printf("PUSHS ");
        if (!codegen_operand(frame, node)) {
            return false;
        }
        putchar('\n');
        return true;
    }
    else if (node->type == AST_BINARY_OP) {
        if (node->children == NULL || node->children_count != 2) {
            return false;
        }
        const char *instruction;
        switch (node->operator) {
        case AST_OP_ADD:
            instruction = "ADDS";
            break;
        case AST_OP_SUB:
            instruction = "SUBS";
            break;
        case AST_OP_MUL:
            instruction = "MULS";
            break;
        case AST_OP_DIV:
            if (node->resolved_type == AST_DATA_DOUBLE) {
                instruction = "DIVS";
            }
            else {
                instruction = "IDIVS";
            }
            break;
        default:
            return false;
        }
        const ASTNode *left = node->children[0];
        const ASTNode *right = node->children[1];
        if (!codegen_push_expression(frame, left)) {
            return false;
        }
        if (node->resolved_type == AST_DATA_DOUBLE && left->resolved_type == AST_DATA_INT) {
            puts("INT2FLOATS");
        }
        if (!codegen_push_expression(frame, right)) {
            return false;
        }
        if (node->resolved_type == AST_DATA_DOUBLE && right->resolved_type == AST_DATA_INT) {
            puts("INT2FLOATS");
        }
        puts(instruction);
        return true;
    }
    else if (node->type == AST_UNARY_OP) {
        if (node->operator != AST_OP_NEGATION || node->children == NULL || node->children_count != 1) {
            return false;
        }
        if (node->resolved_type == AST_DATA_DOUBLE) {
            puts("PUSHS float@0x0p+0");
        }
        else{
            puts("PUSHS int@0");
        }
        if (!codegen_push_expression(frame, node->children[0])) {
            return false;
        }
        puts("SUBS");
        return true;
    }
    return false;
}

static bool codegen_expression(CodegenFrame frame, const char *dest, const ASTNode *node, ASTDataType target_type)
{
    if (dest == NULL) {
        return false;
    }
    if (!codegen_push_expression(frame, node)) {
        return false;
    }
    if (target_type == AST_DATA_DOUBLE && node->resolved_type == AST_DATA_INT) {
        puts("INT2FLOATS");
    }
    printf("POPS ");
    codegen_variable(frame, dest);
    putchar('\n');
    return true;
}

bool codegen_var_declaration(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL || node->type != AST_VAR_DECLARATION || node->text == NULL || node->children_count != 1 || node->children == NULL){
        return false;
    }
    const ASTNode *initializer = node->children[0];
    if (initializer == NULL || (initializer->type != AST_IDENTIFIER && initializer->type != AST_INT_LITERAL && initializer->type != AST_DOUBLE_LITERAL && initializer->type != AST_BINARY_OP && initializer->type != AST_UNARY_OP && initializer->type != AST_STRING_LITERAL)) {
        return false;
    }
    ASTDataType variable_type = node->declared_type;
    if (variable_type == AST_DATA_UNKNOWN) {
        variable_type = initializer->resolved_type;
    }
    CodegenVariable *variable = codegen_create_variable(node->text, codegen_context.next_variable_id++, variable_type);
    if (variable == NULL) {
        return false;
    }
    if (!codegen_expression(frame, variable->target_name, initializer, variable->type)) {
        free(variable);
        return false;
    }
    variable->next = codegen_context.variables;
    codegen_context.variables = variable;
    return true;
}

bool codegen_assignment(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL || node->type != AST_ASSIGN || node->text == NULL || node->children_count != 1 || node->children == NULL) {
        return false;
    }
    const CodegenVariable *variable = codegen_find_variable(codegen_context.variables, node->text);
    if (variable == NULL) {
        return false;
    }
    return codegen_expression(frame, variable->target_name, node->children[0], variable->type);
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
        const CodegenVariable *variable = codegen_find_variable(codegen_context.variables, arg->text);
        if (variable == NULL) {
            return false;
        }
        codegen_write_var(frame, variable->target_name);
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
    else if (arg->type == AST_DOUBLE_LITERAL) {
        printf("WRITE ");
        codegen_float(arg->double_value);
        putchar('\n');
    }
    else if (arg->type == AST_UNIT_LITERAL) {
        puts("WRITE string@()");
    }
    else {
        return false;
    }
    return true;
}

static const ASTNode *codegen_comparison(const ASTNode *condition) {
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
    if (left == NULL || right == NULL) {
        return NULL;
    }
    return expression;
}

static bool codegen_jump_if_true(CodegenFrame frame, const ASTNode *condition, const char *label)
{
    const ASTNode *expression = codegen_comparison(condition);
    if (expression == NULL || label == NULL) {
        return false;
    }
    const ASTNode *left = expression->children[0];
    const ASTNode *right = expression->children[1];
    const bool use_double = left->resolved_type == AST_DATA_DOUBLE || right->resolved_type == AST_DATA_DOUBLE;
    const char *instruction;
    bool negate = false;
    switch (expression->operator) {
    case AST_OP_EQUAL:
        instruction = "EQS";
        negate = false;
        break;
    case AST_OP_NOT_EQUAL:
        instruction = "EQS";
        negate = true;
        break;
    case AST_OP_LESS:
        instruction = "LTS";
        negate = false;
        break;
    case AST_OP_LESS_EQUAL:
        instruction = "GTS";
        negate = true;
        break;
    case AST_OP_GREATER:
        instruction = "GTS";
        negate = false;
        break;
    case AST_OP_GREATER_EQUAL:
        instruction = "LTS";
        negate = true;
        break;
    default:
        return false;
    }
    if (!codegen_push_expression(frame, left)) {
        return false;
    }
    if (use_double && left->resolved_type == AST_DATA_INT) {
        puts("INT2FLOATS");
    }
    if (!codegen_push_expression(frame, right)) {
        return false;
    }
    if (use_double && right->resolved_type == AST_DATA_INT) {
        puts("INT2FLOATS");
    }
    puts(instruction);
    if (negate) {
        puts("NOTS");
    }
    printf("POPS ");
    codegen_variable(frame, "%condition");
    putchar('\n');
    codegen_jumpifeq_bool(label, frame, "%condition", true);
    return true;
}

bool codegen_if(CodegenFrame frame, const ASTNode *node) {
    if (node == NULL || node->type != AST_IF || node->children == NULL || node->children_count != 3) {
        return false;
    }
    const ASTNode *condition = node->children[0];
    const ASTNode *then_block = node->children[1];
    const ASTNode *else_block = node->children[2];
    const ASTNode *expression = codegen_comparison(condition);
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
    const ASTNode *expression = codegen_comparison(condition);
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

static bool codegen_declare_variables(CodegenFrame frame, const ASTNode *node)
{
    if (node == NULL || (node->children_count > 0 && node->children == NULL)) {
        return false;
    }
    if (node->type == AST_VAR_DECLARATION) {
        char target_name[40];
        snprintf(target_name, sizeof(target_name), "%%var_%lu", codegen_context.next_variable_id++);
        codegen_defvar(frame, target_name);
    }
    for (size_t i = 0; i < node->children_count; i++) {
        if (!codegen_declare_variables(frame, node->children[i])) {
            return false;
        }
    }
    return true;
}

static bool codegen_block_statements(CodegenFrame frame, const ASTNode *node)
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
        case AST_UNIT_LITERAL:
            break;
        default:
            return false;
        }
    }
    return true;
}

bool codegen_block(CodegenFrame frame, const ASTNode *node)
{
    if (codegen_context.block_depth == 0) {
        codegen_context.next_variable_id = 0;
        if (!codegen_declare_variables(frame, node)) {
            return false;
        }
        codegen_context.next_variable_id = 0;
    }
    CodegenVariable *saved_variables = codegen_context.variables;
    codegen_context.block_depth++;

    bool result = codegen_block_statements(frame, node);
    codegen_free_variables_until(codegen_context.variables, saved_variables);
    codegen_context.variables = saved_variables;
    codegen_context.block_depth--;

    return result;
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