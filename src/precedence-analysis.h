/**
 * @file   precedence-analysis.h
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-14
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
} PrecedenceSymbol;

typedef enum precedence_relation {
  PREC_ERROR = ' ',
  PREC_SHIFT = '<',
  PREC_EQUAL = '=',
  PREC_REDUCE = '>',
} PrecedenceRelation;

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
PrecedenceRelation precedence_relation(PrecedenceSymbol stack_symbol, PrecedenceSymbol input_symbol);

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
