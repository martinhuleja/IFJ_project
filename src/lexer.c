#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct IndentNode {
    unsigned int spaces;
    struct IndentNode *next;
} IndentNode;

typedef struct {
    IndentNode *top;
} IndentStack;

typedef struct {
    IndentStack indent_stack;
    bool at_line_start;
    unsigned int leading_spaces;
} LexerState;

void indent_stack_init(IndentStack *stack) {
    stack->top = NULL;
}

bool indent_stack_push(IndentStack *stack, unsigned int spaces) {
    IndentNode *node = malloc(sizeof(IndentNode));

    if (node == NULL) {
        return false;
    }

    node->spaces = spaces;
    node->next = stack->top;
    stack->top = node;

    return true;
}

unsigned int indent_stack_top(const IndentStack *stack) {
    return stack->top->spaces;
}

void indent_stack_pop(IndentStack *stack) {
    if (stack->top == NULL) {
        return;
    }

    IndentNode *old = stack->top;
    stack->top = old->next;

    free(old);
}

void indent_stack_free(IndentStack *stack) {
    while (stack->top != NULL) {
        indent_stack_pop(stack);
    }
}