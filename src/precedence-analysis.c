/**
 * @file   precedence-analysis.c
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-16
 *
 * @brief  Implementation of bottom-up precedence syntactic analysis
 */

#include "precedence-analysis.h"
#include "abstract-syntactic-tree.h"
#include "error.h"
#include "parser.h"
#include "token.h"
#include <errno.h>
#include <math.h>
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

PrecStack *stack_init() {
  // Allocate the stack and prepare its bottom terminal $
  PrecStack *stack = (PrecStack *)malloc(sizeof(PrecStack));
  if (stack == NULL) {
    print_err(ERROR_INTERNAL, "Couldn't allocate PrecStack.");
    return NULL;
  }

  stack->top = create_stack_item(PREC_STACK_TERMINAL, PREC_END);
  if (stack->top == NULL) {
    free(stack);
    return NULL;
  }
  return stack;
} // stack_init()

PrecStackItem *create_stack_item(PrecStackItemType type, PrecSymbol symbol) {
  PrecStackItem *new_item = (PrecStackItem *)malloc(sizeof(PrecStackItem));
  if (new_item == NULL) {
    print_err(ERROR_INTERNAL, "Couldn't allocate PrecStackItem.");
    return NULL;
  }
  *new_item = (PrecStackItem){.type = type, .symbol = symbol};
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
    free(stack);
  }
} // stack_free()

PrecSymbol get_prec_symbol(TokenType type) {
  switch (type) {
  case TOKEN_IDENTIFIER:
  case TOKEN_INT_LITERAL:
  case TOKEN_DOUBLE_LITERAL:
  case TOKEN_STRING_LITERAL:
    return PREC_OPERAND;
  case TOKEN_MULTIPLY:
  case TOKEN_DIVIDE:
    return PREC_MULTIPLICATIVE;
  case TOKEN_PLUS:
  case TOKEN_MINUS:
    return PREC_ADDITIVE;
  case TOKEN_LESS:
  case TOKEN_LESS_EQUAL:
  case TOKEN_GREATER:
  case TOKEN_GREATER_EQUAL:
    return PREC_ORDERING;
  case TOKEN_EQUAL:
  case TOKEN_NOT_EQUAL:
    return PREC_EQUALITY;
  case TOKEN_LPAREN:
    return PREC_LPAREN;
  case TOKEN_RPAREN:
    return PREC_RPAREN;
  case TOKEN_NEWLINE:
  case TOKEN_EOF:
  case TOKEN_SEMICOLON:
  case TOKEN_THEN:
  case TOKEN_DO:
  case TOKEN_COMMA:
  // case TOKEN_RPAREN:
    return PREC_END;
  default:
    return PREC_SYMBOL_COUNT;
  }
} // get_prec_symbol()

ErrorCode fill_operand_node(Parser *parser, ASTNode *node) {
  RETURN_IF_ERROR(check_current_token(parser));
  RETURN_INTERNAL_ERROR_IF_NULL(node, "Operand node is NULL.");

  switch (parser->current_token->type) {
  case TOKEN_IDENTIFIER:
    RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token->value, "Identifier token value is NULL.");
    node->type = AST_IDENTIFIER;
    RETURN_IF_ERROR(ast_set_text(node, parser->current_token->value));
    break;

  case TOKEN_STRING_LITERAL:
    RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token->value, "StringLiteral value is NULL.");
    node->type = AST_STRING_LITERAL;
    RETURN_IF_ERROR(ast_set_text(node, parser->current_token->value));
    break;

  case TOKEN_INT_LITERAL: {
    node->type = AST_INT_LITERAL;
    RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token->value, "IntLiteral value is NULL.");

    char *end; // pointer to end of strtol() reading
    errno = 0;
    long value = strtol(parser->current_token->value, &end, 10);
    if (errno == ERANGE || end == parser->current_token->value || *end != '\0' || value < INT32_MIN ||
        value > INT32_MAX) {
      print_err(ERROR_LEXICAL, "Invalid Int literal.");
      return ERROR_LEXICAL;
    }
    node->int_value = (int32_t)value;
    break;
  }
  case TOKEN_DOUBLE_LITERAL: {
    node->type = AST_DOUBLE_LITERAL;
    RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token->value, "DoubleLiteral value is NULL.");

    char *end; // pointer to end of strtod() reading
    errno = 0;
    double value = strtod(parser->current_token->value, &end);
    if (errno == ERANGE || end == parser->current_token->value || *end != '\0' || !isfinite(value)) {
      print_err(ERROR_LEXICAL, "Invalid Double literal.");
      return ERROR_LEXICAL;
    }
    node->double_value = value;
    break;
  }
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // fill_operand_node()

/********************************************************************
 ****************** PRECEDENCE ANALYSIS FUNCTIONS *******************
 *******************************************************************/

ErrorCode parse_expr(Parser *parser, ASTNode *parent) {
  RETURN_IF_ERROR(check_current_token(parser));
  RETURN_INTERNAL_ERROR_IF_NULL(parent, "Expression parent node is NULL.");

  // Allocate the stack with its bottom terminal $ before processing operands
  PrecStack *stack = stack_init();
  RETURN_INTERNAL_ERROR_IF_NULL(stack, NULL);
  ErrorCode return_value = ERROR_OK;

  while (return_value == ERROR_OK) {
    // Compare the nearest stack terminal with the current input symbol
    PrecSymbol input_symbol = get_prec_symbol(parser->current_token->type);
    PrecStackItem *terminal = stack_top_terminal(stack);
    if (terminal == NULL) {
      return_value = ERROR_INTERNAL;
      break;
    }

    // Accept only $ E and leave the expression terminator unconsumed
    if (input_symbol == PREC_END && stack->top->type == PREC_STACK_EXPRESSION && stack->top->next == terminal &&
        terminal->symbol == PREC_END && terminal->next == NULL) {
      ASTNode *expression = stack->top->node;
      stack->top->node = NULL;
      return_value = ast_add_child(parent, expression);
      break;
    }

    PrecRelation relation = get_relation(terminal->symbol, input_symbol);
    switch (relation) {
    case PREC_SHIFT:
      // Insert a handle marker only for '<'
      return_value = stack_mark(stack);
      if (return_value != ERROR_OK) {
        break;
      }
      // fall through
    case PREC_EQUAL: {
      // Create the operand item only when it is shifted onto the stack
      PrecStackItem *new_item = create_stack_item(PREC_STACK_TERMINAL, input_symbol);
      if (new_item == NULL) {
        return_value = ERROR_INTERNAL;
        break;
      }
      new_item->node = ast_create_node(AST_EXPR); // fill_operand_node() sets the concrete operand type
      if (new_item->node == NULL) {
        free(new_item);
        return_value = ERROR_INTERNAL;
        break;
      }

      // Transfer ownership to the stack only after the operand is filled successfully
      return_value = fill_operand_node(parser, new_item->node);
      if (return_value == ERROR_OK) {
        return_value = stack_push(stack, new_item);
      }
      if (return_value != ERROR_OK) {
        ast_node_free(new_item->node);
        free(new_item);
      } else {
        return_value = new_token(parser);
      }
      break;
    }
    case PREC_REDUCE: {
      // Check the complete handle before reducing i to E
      PrecStackItem *marker = stack->top->next;
      if (stack->top->type != PREC_STACK_TERMINAL || stack->top->symbol != PREC_OPERAND || marker == NULL ||
          marker->type != PREC_STACK_MARK) {
        stack_free(stack);
        RETURN_SYNTACTIC_ERROR(parser);
      }
      stack->top->type = PREC_STACK_EXPRESSION;
      stack->top->next = marker->next;
      free(marker);
      // Keep the current input token for the next table lookup
      break;
    }
    default:
      // Free the stack's items before returning ERROR_SYNTACTIC
      stack_free(stack);
      RETURN_SYNTACTIC_ERROR(parser);
    }
  }

  // Free the remaining stack items and the allocated stack object
  stack_free(stack);
  return return_value;
} // parse_expr()

ErrorCode parse_expr_tail(Parser *parser, ASTNode *expression) {
  (void)parser;
  (void)expression;
  return ERROR_OK;
}

/* End of file precedence-analysis.c */
