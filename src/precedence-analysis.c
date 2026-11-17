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
  case TOKEN_DEDENT:
  case TOKEN_EOF:
  case TOKEN_SEMICOLON:
  case TOKEN_THEN:
  case TOKEN_DO:
  case TOKEN_COMMA:
    return PREC_END;
  default:
    return PREC_SYMBOL_COUNT;
  }
} // get_prec_symbol()

ASTOperator get_ast_operator(TokenType type) {
  switch (type) {
  case TOKEN_PLUS:
    return AST_OP_ADD;
  case TOKEN_MINUS:
    return AST_OP_SUB;
  case TOKEN_MULTIPLY:
    return AST_OP_MUL;
  case TOKEN_DIVIDE:
    return AST_OP_DIV;
  case TOKEN_LESS:
    return AST_OP_LESS;
  case TOKEN_LESS_EQUAL:
    return AST_OP_LESS_EQUAL;
  case TOKEN_GREATER:
    return AST_OP_GREATER;
  case TOKEN_GREATER_EQUAL:
    return AST_OP_GREATER_EQUAL;
  case TOKEN_EQUAL:
    return AST_OP_EQUAL;
  case TOKEN_NOT_EQUAL:
    return AST_OP_NOT_EQUAL;
  default:
    return AST_OP_NONE;
  }
} // get_ast_operator()

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

ErrorCode reduce_binary(PrecStack *stack) {
  RETURN_INTERNAL_ERROR_IF_NULL(stack, "PrecStack is NULL.");

  // Stack links run from the right operand through the operator and left operand to the marker
  PrecStackItem *right = stack->top;
  if (right == NULL || right->type != PREC_STACK_EXPRESSION) {
    return ERROR_SYNTACTIC;
  }
  RETURN_INTERNAL_ERROR_IF_NULL(right->node, "Right operand node is NULL.");

  PrecStackItem *operator_item = right->next;
  if (operator_item == NULL || operator_item->type != PREC_STACK_TERMINAL ||
      operator_item->symbol < PREC_MULTIPLICATIVE || operator_item->symbol > PREC_EQUALITY ||
      operator_item->operator == AST_OP_NONE) {
    return ERROR_SYNTACTIC;
  }

  PrecStackItem *left = operator_item->next;
  if (left == NULL || left->type != PREC_STACK_EXPRESSION) {
    return ERROR_SYNTACTIC;
  }
  RETURN_INTERNAL_ERROR_IF_NULL(left->node, "Left operand node is NULL.");

  PrecStackItem *marker = left->next;
  if (marker == NULL || marker->type != PREC_STACK_MARK) {
    return ERROR_SYNTACTIC;
  }

  // Attach operands in source order, ast_add_child() consumes the child even on failure
  ASTNode *node = ast_create_node(AST_BINARY_OP);
  RETURN_INTERNAL_ERROR_IF_NULL(node, NULL);
  node->operator = operator_item->operator;

  ErrorCode return_value = ast_add_child(node, left->node);
  left->node = NULL;

  if (return_value == ERROR_OK) {
    return_value = ast_add_child(node, right->node);
    right->node = NULL;
  }

  // Catch error
  if (return_value != ERROR_OK) {
    ast_node_free(node);
    return return_value;
  }

  // Replace the complete handle with one E owning the new subtree
  right->node = node;
  right->next = marker->next;
  free(operator_item);
  free(left);
  free(marker);
  return ERROR_OK;
} // reduce_binary()

ErrorCode reduce_unary(PrecStack *stack) {
  RETURN_INTERNAL_ERROR_IF_NULL(stack, "PrecStack is NULL.");

  // Check the complete handle - E, following links from the operand to the marker
  PrecStackItem *operand = stack->top;
  if (operand == NULL || operand->type != PREC_STACK_EXPRESSION) {
    return ERROR_SYNTACTIC;
  }
  RETURN_INTERNAL_ERROR_IF_NULL(operand->node, "Unary operand node is NULL.");

  PrecStackItem *operator_item = operand->next;
  if (operator_item == NULL || operator_item->type != PREC_STACK_TERMINAL ||
      operator_item->symbol != PREC_UNARY_MINUS || operator_item->operator != AST_OP_NEGATION) {
    return ERROR_SYNTACTIC;
  }

  PrecStackItem *marker = operator_item->next;
  if (marker == NULL || marker->type != PREC_STACK_MARK) {
    return ERROR_SYNTACTIC;
  }

  // Transfer the operand to the unary node; ast_add_child() also consumes it on failure
  ASTNode *node = ast_create_node(AST_UNARY_OP);
  RETURN_INTERNAL_ERROR_IF_NULL(node, NULL);
  node->operator = AST_OP_NEGATION;

  ErrorCode return_value = ast_add_child(node, operand->node);
  operand->node = NULL;

  if (return_value != ERROR_OK) {
    ast_node_free(node);
    return return_value;
  }

  // Replace the marked handle with one E owning the unary subtree
  operand->node = node;
  operand->next = marker->next;
  free(operator_item);
  free(marker);
  return ERROR_OK;
} // reduce_unary()

ErrorCode reduce_parentheses(PrecStack *stack) {
  RETURN_INTERNAL_ERROR_IF_NULL(stack, "PrecStack is NULL.");

  // Find the left parenthesis, with an optional expression between the parentheses
  PrecStackItem *right = stack->top;
  if (right == NULL || right->type != PREC_STACK_TERMINAL || right->symbol != PREC_RPAREN) {
    return ERROR_SYNTACTIC;
  }

  PrecStackItem *middle = right->next;
  if (middle == NULL) {
    return ERROR_SYNTACTIC;
  }

  PrecStackItem *left = middle->type == PREC_STACK_EXPRESSION ? middle->next : middle;
  if (left == NULL || left->type != PREC_STACK_TERMINAL || left->symbol != PREC_LPAREN) {
    return ERROR_SYNTACTIC;
  }

  PrecStackItem *marker = left->next;
  if (marker == NULL || marker->type != PREC_STACK_MARK) {
    return ERROR_SYNTACTIC;
  }

  ASTNode *node;
  if (middle == left) {
    // Empty parentheses represent the UnitLiteral
    node = ast_create_node(AST_UNIT_LITERAL);
    RETURN_INTERNAL_ERROR_IF_NULL(node, NULL);
  } else {
    // Grouping keeps the inner subtree without adding an AST wrapper
    RETURN_INTERNAL_ERROR_IF_NULL(middle->node, "Parenthesized expression node is NULL.");
    node = middle->node;
    middle->node = NULL;
    free(middle);
  }

  // Replace ( E ) or ( ) with one expression
  right->type = PREC_STACK_EXPRESSION;
  right->node = node;
  right->next = marker->next;
  free(left);
  free(marker);
  return ERROR_OK;
} // reduce_parentheses()

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
  bool expect_operand = true;
  size_t parenthesis_depth = 0;

  while (return_value == ERROR_OK) {
    // Compare the nearest stack terminal with the current input symbol
    PrecSymbol input_symbol = get_prec_symbol(parser->current_token->type);

    // A parenthesis not opened by this call belongs to the calling parser
    if (input_symbol == PREC_RPAREN && parenthesis_depth == 0) {
      input_symbol = PREC_END;
    }

    // A minus before an operand is unary; after an operand it is binary
    if (parser->current_token->type == TOKEN_MINUS && expect_operand) {
      input_symbol = PREC_UNARY_MINUS;
    }

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
      // Create an AST leaf for an operand; operators retain only their specific operation
      PrecStackItem *new_item = create_stack_item(PREC_STACK_TERMINAL, input_symbol);
      if (new_item == NULL) {
        return_value = ERROR_INTERNAL;
        break;
      }

      if (input_symbol == PREC_OPERAND) {
        new_item->node = ast_create_node(AST_EXPR); // fill_operand_node() sets the specific operand type
        if (new_item->node == NULL) {
          free(new_item);
          return_value = ERROR_INTERNAL;
          break;
        }
        return_value = fill_operand_node(parser, new_item->node);
      } else if (input_symbol != PREC_LPAREN && input_symbol != PREC_RPAREN) {
        if (input_symbol == PREC_UNARY_MINUS) {
          new_item->operator = AST_OP_NEGATION;
        } else {
          new_item->operator = get_ast_operator(parser->current_token->type);
        }

        if (new_item->operator == AST_OP_NONE) {
          free(new_item);
          stack_free(stack);
          RETURN_SYNTACTIC_ERROR(parser);
        }
      }

      if (return_value == ERROR_OK) {
        // Transfer ownership to the stack only after the item is filled successfully
        return_value = stack_push(stack, new_item);
      }

      if (return_value == ERROR_OK) {
        // Track only parentheses shifted by this call
        if (input_symbol == PREC_LPAREN) {
          parenthesis_depth++;
        } else if (input_symbol == PREC_RPAREN) {
          parenthesis_depth--;
        }

        // Only shifted tokens change whether the next token should be an operand
        expect_operand = input_symbol != PREC_OPERAND && input_symbol != PREC_RPAREN;
        return_value = new_token(parser);
      } else {
        ast_node_free(new_item->node);
        free(new_item);
      }
      break;
    }
    case PREC_REDUCE: {
      PrecStackItem *second_item = stack->top->next;

      // Reduce parentheses before handling operands and operators
      if (stack->top->type == PREC_STACK_TERMINAL && stack->top->symbol == PREC_RPAREN) {
        return_value = reduce_parentheses(stack);
      } else if (stack->top->type == PREC_STACK_TERMINAL && stack->top->symbol == PREC_OPERAND) {
        // Check the complete handle before reducing i to E
        if (second_item == NULL || second_item->type != PREC_STACK_MARK) {
          return_value = ERROR_SYNTACTIC;
        } else {
          stack->top->type = PREC_STACK_EXPRESSION;
          stack->top->next = second_item->next;
          free(second_item);
        }
      } else if (second_item != NULL && second_item->type == PREC_STACK_TERMINAL &&
                 second_item->symbol == PREC_UNARY_MINUS) {
        return_value = reduce_unary(stack);
      } else { // Reduce complete binary handle E op E
        return_value = reduce_binary(stack);
      }

      if (return_value != ERROR_SYNTACTIC) {
        // Keep the current input token for the next table lookup
        break;
      }
    }
    // fall through
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

/* End of file precedence-analysis.c */
