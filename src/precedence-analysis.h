/**
 * @file   precedence-analysis.h
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-17
 *
 * @brief  Declaration of bottom-up precedence analysis functions
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
 * @brief Precedence table classes; operator identity is stored separately
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
 * @brief Allocates PrecStackItem
 *
 * Initializes type and symbol, other values sets to 0 or NULL
 *
 * @param type Type of allocated item
 * @param symbol PrecSymbol of allocated item
 * @return Ptr to allocated PrecStackItem, or NULL
 */
PrecStackItem *create_stack_item(PrecStackItemType type, PrecSymbol symbol);

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
 * Returns PREC_ADDITIVE for TOKEN_MINUS; parse_expr() resolves unary minus from its position.
 *
 * @param type Type of the current token
 * @return Corresponding precedence class, or PREC_SYMBOL_COUNT for an unsupported token
 */
PrecSymbol get_prec_symbol(TokenType type);

/**
 * @brief Maps a binary operator token to its specific AST operation
 *
 * @param type Type of the current token
 * @return Corresponding ASTOperator, or AST_OP_NONE for an unsupported token
 */
ASTOperator get_ast_operator(TokenType type);

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

/**
 * @brief Transfers a stack item's subtree to the parent, clearing item->node even on failure
 *
 * @param parent Ptr to the AST node receiving the subtree
 * @param item Ptr to the stack item owning the subtree
 * @return ERROR_OK on success, or ERROR_INTERNAL
 */
ErrorCode attach_stack_node(ASTNode *parent, PrecStackItem *item);

/**
 * @brief Replaces a validated handle with one expression, keeping its top item
 *
 * Removed items must no longer own subtrees; this function frees only their stack items.
 *
 * @param top Ptr to the handle's top item, whose node must be NULL or equal to node
 * @param marker Ptr to the handle marker reachable from top->next
 * @param node Ptr to the resulting subtree, transferred to top
 */
void replace_handle(PrecStackItem *top, PrecStackItem *marker, ASTNode *node);

/**
 * @brief Reduces the marked handle E op E to one expression owning an AST_BINARY_OP subtree
 *
 * Preserves operand order. On failure the caller still owns the stack and must free it.
 *
 * @param stack Ptr to the initialized stack
 * @return ERROR_OK on success, ERROR_SYNTACTIC for an invalid handle, or ERROR_INTERNAL for invalid pointers/allocation
 */
ErrorCode reduce_binary(PrecStack *stack);

/**
 * @brief Reduces the marked handle - E to one expression owning an AST_UNARY_OP subtree
 *
 * On failure the caller still owns the stack and must free it.
 *
 * @param stack Ptr to the initialized stack
 * @return ERROR_OK on success, ERROR_SYNTACTIC for an invalid handle, or ERROR_INTERNAL for invalid pointers/allocation
 */
ErrorCode reduce_unary(PrecStack *stack);

/**
 * @brief Reduces ( E ) by reusing its subtree, or ( ) by creating an AST_UNIT_LITERAL
 *
 * Removes the parentheses and their handle marker. On failure the caller must free the stack.
 *
 * @param stack Ptr to the initialized stack with a closing parenthesis at its top
 * @return ERROR_OK on success, ERROR_SYNTACTIC for an invalid handle, or ERROR_INTERNAL for invalid pointers/allocation
 */
ErrorCode reduce_parentheses(PrecStack *stack);

/**
 * @brief Executes the token shift shared by PREC_EQUAL and PREC_SHIFT
 *
 * The caller owns the stack even on failure. Advances the token only after a successful push.
 *
 * @param parser Ptr to struct parser
 * @param stack Ptr to the initialized stack receiving the current token
 * @param input_symbol Precedence class of the current token, with unary minus already resolved
 * @param expect_operand Ptr to the operand expectation, updated after a successful push
 * @param parenthesis_depth Ptr to this expression's parenthesis depth, updated after a successful push
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode execute_prec_equal(Parser *parser, PrecStack *stack, PrecSymbol input_symbol, bool *expect_operand,
                             size_t *parenthesis_depth);

/**
 * @brief Executes PREC_REDUCE by selecting and reducing the current handle
 *
 * Does not consume an input token. The caller owns the stack even on failure.
 *
 * @param stack Ptr to the initialized nonempty stack
 * @return ERROR_OK on success, ERROR_SYNTACTIC for an invalid handle, or ERROR_INTERNAL on failure
 */
ErrorCode execute_prec_reduce(PrecStack *stack);

/********************************************************************
 ****************** PRECEDENCE ANALYSIS FUNCTIONS *******************
 *******************************************************************/

/**
 * @brief Parses expression by bottom-up syntactic analysis
 *
 * Parses the complete expression from the current token according to the precedence grammar and precedence table.
 * Appends one subtree on success, consumes matching parentheses and leaves the expression terminator unconsumed.
 * On failure releases partial subtrees and leaves parent unchanged.
 *
 * @param parser Ptr to struct parser
 * @param parent Ptr to the parent node receiving expression
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr(Parser *parser, ASTNode *parent);

#endif // PRECEDENCE_ANALYSIS_H
/* End of file precedence-analysis.h */
