/**
 * @file   precedence-analysis.h
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-16
 *
 * @brief  Header file for precedence analysis functions
 *
 * Precedence analysis creates AST subtree for expressions as bottom-up syntactic analysis.
 * Allocates subtree, links in to given ASTNode pointer as children and fills it with data.
 */

#ifndef PRECEDENCE_ANALYSIS_H
#define PRECEDENCE_ANALYSIS_H

#include "abstract-syntactic-tree.h"
#include "error.h"
#include "parser.h"

/**
 * @brief Precedence table classes; stack entries also retain the concrete token or operator
 */
typedef enum precedence_symbol {
  PREC_OPERAND,
  PREC_UNARY_MINUS,
  PREC_MULTIPLICATIVE,
  PREC_ADDITIVE,
  PREC_ORDERING,
  PREC_EQUALITY,
  PREC_LPAREN,
  PREC_RPAREN,
  PREC_END,
  PREC_SYMBOL_COUNT,
} PrecSymbol;

/**
 * @brief Action selected by the precedence table
 */
typedef enum precedence_relation {
  PREC_ERROR = ' ',
  PREC_SHIFT = '<',
  PREC_EQUAL = '=',
  PREC_REDUCE = '>',
} PrecRelation;

/* Columns: operand, unary -, multiplicative, additive, ordering, equality, (, ), end. */
static const char precedence_table[PREC_SYMBOL_COUNT][PREC_SYMBOL_COUNT] = {
    [PREC_OPERAND]        = {' ', ' ', '>', '>', '>', '>', ' ', '>', '>'},
    [PREC_UNARY_MINUS]    = {'<', '<', '>', '>', '>', '>', '<', '>', '>'},
    [PREC_MULTIPLICATIVE] = {'<', '<', '>', '>', '>', '>', '<', '>', '>'},
    [PREC_ADDITIVE]       = {'<', '<', '<', '>', '>', '>', '<', '>', '>'},
    [PREC_ORDERING]       = {'<', '<', '<', '<', ' ', ' ', '<', '>', '>'},
    [PREC_EQUALITY]       = {'<', '<', '<', '<', ' ', ' ', '<', '>', '>'},
    [PREC_LPAREN]         = {'<', '<', '<', '<', '<', '<', '<', '=', ' '},
    [PREC_RPAREN]         = {' ', ' ', '>', '>', '>', '>', ' ', '>', '>'},
    [PREC_END]            = {'<', '<', '<', '<', '<', '<', '<', ' ', ' '},
};

/**
 * @brief Kind of item stored on the precedence stack
 */
typedef enum precedence_stack_item_type {
  PREC_STACK_TERMINAL,
  PREC_STACK_EXPRESSION,
  PREC_STACK_MARK,
} PrecStackItemType;

/**
 * @brief Stack item containing a terminal, expression or handle marker
 */
typedef struct precedence_stack_item {
  PrecStackItemType type;
  PrecSymbol symbol;
  ASTOperator operator;
  ASTNode *node; // Owned subtree
  struct precedence_stack_item *next;
} PrecStackItem;

/**
 * @brief Precedence stack with a pointer to its first item
 */
typedef struct precedence_stack {
  PrecStackItem *top;
} PrecStack;

/********************************************************************
 ************************ SUPPORT FUNCTIONS *************************
 *******************************************************************/

/**
 * @brief Compares the nearest stack terminal with the current input class
 *
 * @param stack_symbol Precedence class of the nearest terminal on the stack
 * @param input_symbol Precedence class of the current input token
 * @return Precedence relation, or PREC_ERROR for invalid classes
 */
PrecRelation get_relation(PrecSymbol stack_symbol, PrecSymbol input_symbol);

/**
 * @brief Allocates a precedence stack and inserts its bottom terminal $
 *
 * @return Ptr to the initialized stack, or NULL on allocation failure
 */
PrecStack *stack_init();

/**
 * @brief Allocates and zero-initializes a PrecStackItem
 *
 * @return Ptr to allocated PrecStackItem, or NULL
 */
PrecStackItem *create_stack_item();

/**
 * @brief Pushes an item onto the precedence stack
 *
 * @param stack Ptr to the initialized stack
 * @param item Ptr to an item whose ownership is transferred to the stack on success
 * @return ERROR_OK on success, or ERROR_INTERNAL
 */
ErrorCode stack_push(PrecStack *stack, PrecStackItem *item);

/**
 * @brief Returns the nearest terminal from the stack top, or NULL if none exists
 *
 * Does not modify the stack. The returned item remains owned by the stack.
 *
 * @param stack Ptr to the initialized stack
 * @return Ptr to the nearest terminal, or NULL
 */
PrecStackItem *stack_top_terminal(PrecStack *stack);

/**
 * @brief Inserts a handle marker immediately above the nearest terminal, below any trailing expression
 *
 * Leaves the stack unchanged on failure.
 *
 * @param stack Ptr to the initialized stack
 * @return ERROR_OK on success, or ERROR_INTERNAL
 */
ErrorCode stack_mark(PrecStack *stack);

/**
 * @brief Frees the allocated stack, all its items and their owned subtrees
 *
 * @param stack Ptr to a stack allocated by stack_init()
 */
void stack_free(PrecStack *stack);

/**
 * @brief Maps the current token type to a precedence table class
 *
 * @param type Type of the current token
 * @return PREC_OPERAND, PREC_END, or PREC_SYMBOL_COUNT for an unsupported token
 */
PrecSymbol get_prec_symbol(TokenType type);

/**
 * @brief Fills an AST node from the current identifier or literal
 *
 * Copies identifier and string text.
 *
 * @param parser Ptr to struct parser
 * @param node Ptr to an empty AST node receiving the operand's type and value
 * @return ERROR_OK on success, or other ErrorCode on failure
 */
ErrorCode fill_operand_node(Parser *parser, ASTNode *node);

/********************************************************************
 ****************** PRECEDENCE ANALYSIS FUNCTIONS *******************
 *******************************************************************/

/**
 * @brief Parses expression by bottom-up syntactic analysis
 *
 * @param parser Ptr to struct parser
 * @param parent Ptr to the parent node receiving expression
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr(Parser *parser, ASTNode *parent);

/**
 * @brief Parses expression tail by bottom-up syntactic analysis
 *
 * @param parser Ptr to struct parser
 * @param parent Ptr to the parent node receiving expression
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr_tail(Parser *parser, ASTNode *expression);

#endif // PRECEDENCE_ANALYSIS_H
/* End of file precedence-analysis.h */
