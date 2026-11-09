#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "token.h"

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
    unsigned int parenthesis_depth;
    unsigned int comment_depth;
} LexerState;

void token_free(Token *token) {
    if (token == NULL) {
        return;
    }

    free(token->value);
    free(token);
}

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

typedef enum {
    ST_START,

    ST_B, ST_Bo, ST_Boo, ST_Bool, ST_Boole, ST_Boolea, ST_Boolean,
    ST_D,
    ST_Do,
    ST_Dou,
    ST_Doub,
    ST_Doubl,
    ST_Double,
    ST_I, ST_In, ST_Int,
    ST_S, ST_St, ST_Str, ST_Stri, ST_Strin, ST_String,
    ST_U, ST_Un, ST_Uni, ST_Unit,
    ST_d, ST_de, ST_def, ST_do,
    ST_e, ST_el, ST_els, ST_else,
    ST_f, ST_fo, ST_for,
    ST_i, ST_if, ST_im, ST_imp, ST_impo, ST_impor, ST_import,
    ST_r, ST_re, ST_ret, ST_retu, ST_retur, ST_return,
    ST_t, ST_th, ST_the, ST_then,
    ST_v, ST_va, ST_val, ST_var,
    ST_w, ST_wh, ST_whi, ST_whil, ST_while,

    ST_IDENTIFIER,
    ST_LEADING_UNDERSCORE,
    ST_AT, ST_AT_M, ST_AT_MA, ST_AT_MAI, ST_AT_MAIN,

    ST_GREATER, ST_GREATER_EQUAL,
    ST_LESS, ST_LESS_EQUAL,
    ST_BANG, ST_NOT_EQUAL,
    ST_MULTIPLY, ST_DIVIDE,
    ST_PLUS, ST_MINUS,
    ST_ASSIGN, ST_EQUAL,
    ST_DOT,
    ST_NEWLINE,
    ST_LPAREN, ST_RPAREN,
    ST_COLON, ST_COMMA, ST_SEMICOLON,

    ST_COMMENT,
    ST_BLOCK_COMMENT, ST_BLOCK_COMMENT_STAR, ST_BLOCK_COMMENT_END,

    ST_INT_ZERO, ST_INT_DIGITS, ST_INT_SEPARATOR,
    ST_DOUBLE_DOT, ST_DOUBLE_FRACTION, ST_DOUBLE_FRACTION_SEPARATOR,
    ST_DOUBLE_EXPONENT, ST_DOUBLE_EXPONENT_SIGN,
    ST_DOUBLE_EXPONENT_DIGITS, ST_DOUBLE_EXPONENT_SEPARATOR,
    ST_DOUBLE_SUFFIX,

    ST_STRING_OPEN, ST_STRING_BODY, ST_STRING_ESCAPE,
    ST_STRING_AFTER_ESCAPE, ST_STRING,
    ST_STRING_EMPTY_OR_TRIPLE_PREFIX, ST_STRING_TRIPLE_BODY,
    ST_STRING_TRIPLE_QUOTE_1, ST_STRING_TRIPLE_QUOTE_2,
    ST_STRING_TRIPLE_END
} FSMState;

Token *get_next_token(void) {
    FSMState state = ST_START;
    int c;

    while (true) {
        c = getchar();

        switch (state) {
            case ST_START:
                if (c == 'D') {
                    state = ST_D;
                    break;
                }
                break;
            case ST_D:
                if (c == 'o') {
                    state = ST_Do;
                    break;
                }
                break;
            case ST_Do:
                if (c == 'u') {
                    state = ST_Dou;
                    break;
                }

                break;
            case ST_Dou:
                break;

            case ST_IDENTIFIER:
                break;
            case ST_STRING:
                break;
        }
    }
}
