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
#include <stdlib.h>

/********************************************************************
 ************************ SUPPORT FUNCTIONS *************************
 *******************************************************************/

PrecRelation get_relation(PrecSymbol stack_symbol, PrecSymbol input_symbol) {
  if (stack_symbol >= PREC_SYMBOL_COUNT || input_symbol >= PREC_SYMBOL_COUNT) {
    return PREC_ERROR;
  }
  return (PrecRelation)precedence_table[stack_symbol][input_symbol];
} // get_relation()

PrecStackItem *create_stack_item() {
  PrecStackItem *new_item = (PrecStackItem *)malloc(sizeof(PrecStackItem));
  RETURN_INTERNAL_ERROR_IF_NULL(new_item, "Couldn't allocate precedence stack item.");
  return new_item;
} // create_stack_item()

ErrorCode stack_push(PrecStack *stack, PrecStackItem *item) {
  RETURN_INTERNAL_ERROR_IF_NULL(stack, "PrecStack is NULL.");
  RETURN_INTERNAL_ERROR_IF_NULL(item, "PrecStackItem is NULL.");

  // Connect the item to the stack
  item->next = stack->top;
  stack->top = item;
  return ERROR_OK;
} // stack_push()

PrecStackItem *stack_top_terminal(PrecStack *stack) {
  if (stack == NULL) {
    print_err(ERROR_INTERNAL, "PrecStack is NULL.");
    return NULL;
  }

  // Get top terminal
  PrecStackItem *item = stack->top;
  while (item != NULL && item->type != PREC_STACK_TERMINAL) {
    item = item->next;
  }
  return item;
} // stack_top_terminal()

ErrorCode stack_mark(PrecStack *stack) {
  RETURN_INTERNAL_ERROR_IF_NULL(stack, "PrecStack is NULL.");

  PrecStackItem *previous = NULL;
  PrecStackItem *terminal = stack->top;

  // Find the closes terminal to top of the stack
  while (terminal != NULL && terminal->type != PREC_STACK_TERMINAL) {
    previous = terminal;
    terminal = terminal->next;
  }

  RETURN_INTERNAL_ERROR_IF_NULL(terminal, NULL);

  // Alocate memory for a marker
  PrecStackItem *marker = (PrecStackItem *)malloc(sizeof(PrecStackItem));
  RETURN_INTERNAL_ERROR_IF_NULL(marker, "Couldn't allocate PrecStackItem");

  // Set marker's atributes
  marker->type = PREC_STACK_MARK;
  marker->node = NULL;
  marker->next = terminal;

  // Connect the marker to the stack
  if (previous == NULL) {
    stack->top = marker;
  } else {
    previous->next = marker;
  }

  return ERROR_OK;
} // stack_mark()

void stack_free(PrecStack *stack) {
  if (stack != NULL) {
    while (stack->top != NULL) {
      PrecStackItem *item = stack->top;
      stack->top = item->next;
      ast_node_free(item->node);
      free(item);
    }
  }
} // stack_free()

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
