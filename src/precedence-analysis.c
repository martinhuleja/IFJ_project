/**
 * @file   precedence-analysis.c
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-14
 *
 * @brief  Implementation of bottom-up precedence syntactic analysis
 */

#include "precedence-analysis.h"
#include "abstract-syntactic-tree.h"
#include "error.h"
#include "parser.h"
#include "token.h"

/********************************************************************
 ************************ SUPPORT FUNCTIONS *************************
 *******************************************************************/

PrecedenceRelation precedence_relation(PrecedenceSymbol stack_symbol, PrecedenceSymbol input_symbol) {
  if (stack_symbol >= PREC_SYMBOL_COUNT || input_symbol >= PREC_SYMBOL_COUNT) {
    return PREC_ERROR;
  }
  return (PrecedenceRelation)precedence_table[stack_symbol][input_symbol];
} // precedence_relation()

/********************************************************************
 ****************** PRECEDENCE ANALYSIS FUNCTIONS *******************
 *******************************************************************/

ErrorCode parse_expr(Parser *parser, ASTNode *parent) {
  (void)parser;
  (void)parent;
  return ERROR_OK;
}

ErrorCode parse_expr_tail(Parser *parser, ASTNode *expression) {
  (void)parser;
  (void)expression;
  return ERROR_OK;
}

/* End of file precedence-analysis.c */
