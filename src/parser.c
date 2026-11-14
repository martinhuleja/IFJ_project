/**
 * @file   parser.c
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-09
 *
 * @brief  Implementation of parser functions as recursive descent
 */

#include "parser.h"
#include "error.h"
#include "precedence-analysis.h"
#include "token.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/********************************************************************
 ************************ SUPPORT FUNCTIONS *************************
 *******************************************************************/

/**
 * @brief Checks return value of given function
 *
 * If return value isn't ERROR_OK, returns with the error code.
 *
 * @param foo function returning an ErrorCode value
 */
#define RETURN_IF_ERROR(foo)                                                                                           \
  do {                                                                                                                 \
    ErrorCode error_code = (foo);                                                                                      \
    if (error_code != ERROR_OK) {                                                                                      \
      return error_code;                                                                                               \
    }                                                                                                                  \
  } while (0)

/**
 * @brief Prints error msg and return with syntactic ErrorCode
 * @param Ptr to struct parser
 */
#define RETURN_SYNTACTIC_ERROR(parser)                                                                                 \
  do {                                                                                                                 \
    RETURN_IF_ERROR(check_current_token(parser));                                                                      \
    if (parser->print_error_msg_and_return) {                                                                          \
      print_err(ERROR_SYNTACTIC, "Expected token doesn't match the current one (%s, line %u, column %u).",             \
                parser->current_token->value != NULL ? parser->current_token->value : "<no value>",                    \
                parser->current_token->line, parser->current_token->column);                                           \
    }                                                                                                                  \
    return ERROR_SYNTACTIC;                                                                                            \
  } while (0)

/**
 * @brief Returns ERROR_INTERNAL if the given pointer is NULL
 *
 * Prints the error message if it isn't NULL.
 *
 * @param ptr Ptr to check
 * @param message Error message, or NULL to return without printing
 */
#define RETURN_INTERNAL_ERROR_IF_NULL(ptr, message)                                                                    \
  do {                                                                                                                 \
    if ((ptr) == NULL) {                                                                                               \
      const char *error_msg = (message);                                                                               \
      if (error_msg != NULL) {                                                                                         \
        print_err(ERROR_INTERNAL, "%s", error_msg);                                                                    \
      }                                                                                                                \
      return ERROR_INTERNAL;                                                                                           \
    }                                                                                                                  \
  } while (0)

Parser *parser_init() {
  Parser *parser = (Parser *)malloc(sizeof(Parser));
  if (parser == NULL) {
    print_err(ERROR_INTERNAL, "Internal malloc error.");
    return NULL;
  }

  parser->current_token = NULL;
  parser->next_token = NULL;
  parser->print_error_msg_and_return = true;
  return parser;
} // parser_init()

void parser_free(Parser *parser) {
  if (parser != NULL) {
    if (parser->current_token != NULL) {
      free(parser->current_token->value);
      free(parser->current_token);
    }

    if (parser->next_token != NULL) {
      free(parser->next_token->value);
      free(parser->next_token);
    }

    free(parser);
  }
} // parser_free()

ErrorCode check_current_token(Parser *parser) {
  RETURN_INTERNAL_ERROR_IF_NULL(parser, "Parser is NULL.");
  RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token, "current_token in Parser is NULL.");

  return ERROR_OK;
} // check_current_token()

ErrorCode compare_tokens(Parser *parser, TokenType expected_type) {
  RETURN_IF_ERROR(check_current_token(parser));

  // Compare current token type to the expected one
  if (parser->current_token->type != expected_type) {
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // compare_tokens()

ErrorCode compare_tokens_next_token(Parser *parser, TokenType expected_type) {
  RETURN_INTERNAL_ERROR_IF_NULL(parser, "Parser is NULL.");
  RETURN_INTERNAL_ERROR_IF_NULL(parser->next_token, "next_token in Parser is NULL.");

  // Compare current token type to the expected one
  if (parser->next_token->type != expected_type) {
    return ERROR_SYNTACTIC;
  }

  return ERROR_OK;
} // compare_tokens_next_token()

bool tokens_matches(Parser *parser, TokenType expected_token) {
  if (parser == NULL) {
    return false;
  }

  parser->print_error_msg_and_return = false;
  bool return_value = (compare_tokens(parser, expected_token) == ERROR_OK);
  parser->print_error_msg_and_return = true; // Reset value
  return return_value;
} // tokens_matches()

bool token_matches_next_token(Parser *parser, TokenType expected_token) {
  if (parser == NULL) {
    return false;
  }

  parser->print_error_msg_and_return = false;
  bool return_value = (compare_tokens_next_token(parser, expected_token) == ERROR_OK);
  parser->print_error_msg_and_return = true; // Reset value
  return return_value;
} // token_matches_next_token()

ErrorCode new_token(Parser *parser) {
  RETURN_INTERNAL_ERROR_IF_NULL(parser, "Parser is NULL.");

  // Deallocate memory used for current token
  if (parser->current_token != NULL) {
    free(parser->current_token->value);
    free(parser->current_token);
  }

  if (parser->next_token != NULL) {
    parser->current_token = parser->next_token;
    parser->next_token = NULL;
  } else {
    Token *token = (Token *)malloc(sizeof(Token));
    RETURN_INTERNAL_ERROR_IF_NULL(token, "Couldn't allocate memory for token.");

    ErrorCode lexer_return_value = get_next_token(token);
    if (lexer_return_value != ERROR_OK) {
      free(token);
      return lexer_return_value;
    }
    parser->current_token = token;
  }

  return ERROR_OK;
} // new_token()

ErrorCode look_up_next_token(Parser *parser) {
  RETURN_INTERNAL_ERROR_IF_NULL(parser, "Parser is NULL.");

  if (parser->next_token == NULL) {
    Token *token = (Token *)malloc(sizeof(Token));
    RETURN_INTERNAL_ERROR_IF_NULL(token, "Couldn't allocate memory for token.");

    ErrorCode lexer_return_value = get_next_token(token);
    if (lexer_return_value != ERROR_OK) {
      free(token);
      return lexer_return_value;
    }
    parser->next_token = token;
  }

  return ERROR_OK;
} // look_up_next_token()

ErrorCode expect(Parser *parser, TokenType expected_type) {
  RETURN_IF_ERROR(compare_tokens(parser, expected_type));
  RETURN_IF_ERROR(new_token(parser));

  return ERROR_OK;
} // expect()

/********************************************************************
 ************** PARSING ACCORDING TO LL GRAMMAR RULES ***************
 *******************************************************************/

ErrorCode parse_program(ASTree *tree) {
  RETURN_INTERNAL_ERROR_IF_NULL(tree, "ASTree is NULL.");
  RETURN_INTERNAL_ERROR_IF_NULL(tree->root, "Root node in ASTree is NULL.");

  Parser *parser = parser_init();
  RETURN_INTERNAL_ERROR_IF_NULL(parser, NULL);

  ErrorCode return_value = ERROR_OK;

  return_value = new_token(parser);
  if (return_value == ERROR_OK) {
    return_value = parse_prologue(parser);
  }
  if (return_value == ERROR_OK) {
    return_value = parse_function_list(parser, tree->root);
  }
  if (return_value == ERROR_OK) {
    return_value = compare_tokens(parser, TOKEN_EOF);
  }

  parser_free(parser);

  return return_value;
} // parse_program()

ErrorCode parse_prologue(Parser *parser) {
  RETURN_IF_ERROR(expect(parser, TOKEN_IMPORT));

  // Manually execute expect()
  RETURN_IF_ERROR(compare_tokens(parser, TOKEN_IDENTIFIER));
  // Compare current token value with "ifj26"
  RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token->value, "Identifier token value is NULL.");
  if (strcmp(parser->current_token->value, "ifj26") != 0) {
    RETURN_SYNTACTIC_ERROR(parser);
  }
  RETURN_IF_ERROR(new_token(parser));

  RETURN_IF_ERROR(expect(parser, TOKEN_DOT));
  RETURN_IF_ERROR(expect(parser, TOKEN_MULTIPLY));
  RETURN_IF_ERROR(expect(parser, TOKEN_NEWLINE));

  return ERROR_OK;
} // parse_prologue()

ErrorCode parse_function_list(Parser *parser, ASTNode *program) {
  while (tokens_matches(parser, TOKEN_NEWLINE)) {
    RETURN_IF_ERROR(new_token(parser));
  }
  RETURN_IF_ERROR(parse_function_def(parser, program));
  RETURN_IF_ERROR(parse_function_list_rest(parser, program));

  return ERROR_OK;
} // parse_function_list()

ErrorCode parse_function_list_rest(Parser *parser, ASTNode *program) {
  // FunctionListRest -> FunctionDef FunctionListRest
  if (tokens_matches(parser, TOKEN_MAIN_ANNOTATION) || tokens_matches(parser, TOKEN_DEF)) {
    RETURN_IF_ERROR(parse_function_def(parser, program));
    RETURN_IF_ERROR(parse_function_list_rest(parser, program));
  } else { // FunctionListRest -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_EOF));
  }

  return ERROR_OK;
} // parse_function_list_rest()

ErrorCode parse_function_def(Parser *parser, ASTNode *program) {
  ASTNode *function = ast_create_node(AST_FUNCTION);
  RETURN_INTERNAL_ERROR_IF_NULL(function, NULL);
  RETURN_IF_ERROR(ast_add_child(program, function));

  RETURN_IF_ERROR(parse_main_opt(parser, function));
  RETURN_IF_ERROR(expect(parser, TOKEN_DEF));
  RETURN_IF_ERROR(compare_tokens(parser, TOKEN_IDENTIFIER));
  RETURN_IF_ERROR(ast_set_text(function, parser->current_token->value));
  RETURN_IF_ERROR(new_token(parser));
  RETURN_IF_ERROR(parse_param_group(parser, function));
  RETURN_IF_ERROR(parse_type_opt(parser, function));
  RETURN_IF_ERROR(expect(parser, TOKEN_ASSIGN));
  RETURN_IF_ERROR(parse_function_body(parser, function));

  return ERROR_OK;
} // parse_function_def()

ErrorCode parse_main_opt(Parser *parser, ASTNode *function) {
  RETURN_INTERNAL_ERROR_IF_NULL(function, "Function node in ASTree is NULL.");

  // MainOpt -> AtMain
  if (tokens_matches(parser, TOKEN_MAIN_ANNOTATION)) {
    function->is_main = true;
    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_eol_opt(parser));
  }

  return ERROR_OK;
} // parse_main_opt()

ErrorCode parse_param_group(Parser *parser, ASTNode *function) {
  ASTNode *param_list = ast_create_node(AST_PARAM_LIST);
  RETURN_INTERNAL_ERROR_IF_NULL(param_list, NULL);
  RETURN_IF_ERROR(ast_add_child(function, param_list));

  RETURN_IF_ERROR(expect(parser, TOKEN_LPAREN));
  RETURN_IF_ERROR(parse_param_list_opt(parser, param_list));
  RETURN_IF_ERROR(expect(parser, TOKEN_RPAREN));

  return ERROR_OK;
} // parse_param_group()

ErrorCode parse_param_list_opt(Parser *parser, ASTNode *param_list) {
  // ParamListOpt -> ParamList
  if (tokens_matches(parser, TOKEN_IDENTIFIER)) {
    RETURN_IF_ERROR(parse_param_list(parser, param_list));
  } else { // ParamListOpt -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_RPAREN));
  }

  return ERROR_OK;
} // parse_param_list_opt()

ErrorCode parse_param_list(Parser *parser, ASTNode *param_list) {
  RETURN_IF_ERROR(parse_param(parser, param_list));
  RETURN_IF_ERROR(parse_param_list_rest(parser, param_list));

  return ERROR_OK;
} // parse_param_list()

ErrorCode parse_param_list_rest(Parser *parser, ASTNode *param_list) {
  // ParamListRest -> Comma ParamListRestTail
  if (tokens_matches(parser, TOKEN_COMMA)) {
    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_param_list_rest_tail(parser, param_list));
  } else { // ParamListRest -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_RPAREN));
  }

  return ERROR_OK;
} // parse_param_list_rest()

ErrorCode parse_param_list_rest_tail(Parser *parser, ASTNode *param_list) {
  // ParamListRestTail -> ParamList
  if (tokens_matches(parser, TOKEN_IDENTIFIER)) {
    RETURN_IF_ERROR(parse_param_list(parser, param_list));
  } else { // ParamListRestTail -> ε
    // ParamListRestTail -> ε {checkLF(RParen)}
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_RPAREN));
    if (!parser->current_token->lf_before_rparen) {
      RETURN_SYNTACTIC_ERROR(parser);
    }
  }

  return ERROR_OK;
} // parse_param_list_rest_tail()

ErrorCode parse_param(Parser *parser, ASTNode *param_list) {
  ASTNode *param = ast_create_node(AST_PARAM);
  RETURN_INTERNAL_ERROR_IF_NULL(param, NULL);
  RETURN_IF_ERROR(ast_add_child(param_list, param));

  RETURN_IF_ERROR(compare_tokens(parser, TOKEN_IDENTIFIER));
  RETURN_IF_ERROR(ast_set_text(param, parser->current_token->value));
  RETURN_IF_ERROR(new_token(parser));
  RETURN_IF_ERROR(expect(parser, TOKEN_COLON));
  RETURN_IF_ERROR(parse_type(parser, param));

  return ERROR_OK;
} // parse_param()

ErrorCode parse_type_opt(Parser *parser, ASTNode *declaration) {
  RETURN_INTERNAL_ERROR_IF_NULL(declaration, "Declaration node in ASTree is NULL.");

  // TypeOpt -> Colon Type
  if (tokens_matches(parser, TOKEN_COLON)) {
    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_type(parser, declaration));
  } else { // TypeOpt -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_ASSIGN));
    declaration->declared_type = AST_DATA_UNKNOWN;
  }

  return ERROR_OK;
} // parse_type_opt()

ErrorCode parse_type(Parser *parser, ASTNode *declaration) {
  RETURN_INTERNAL_ERROR_IF_NULL(declaration, "Declaration node in ASTree is NULL.");

  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_INT:
    declaration->declared_type = AST_DATA_INT;
    break;
  case TOKEN_DOUBLE:
    declaration->declared_type = AST_DATA_DOUBLE;
    break;
  case TOKEN_STRING:
    declaration->declared_type = AST_DATA_STRING;
    break;
  case TOKEN_UNIT:
    declaration->declared_type = AST_DATA_UNIT;
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  RETURN_IF_ERROR(new_token(parser));

  return ERROR_OK;
} // parse_type()

ErrorCode parse_function_body(Parser *parser, ASTNode *function) {
  RETURN_INTERNAL_ERROR_IF_NULL(function, "Function node in ASTree is NULL.");

  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  // One line function definition
  case TOKEN_IDENTIFIER:
    // fall through
  case TOKEN_INT_LITERAL:
    // fall through
  case TOKEN_DOUBLE_LITERAL:
    // fall through
  case TOKEN_STRING_LITERAL:
    // fall through
  case TOKEN_LPAREN: {
    // Represent one line function body in AST similar to multi line function body
    // Function -> Block -> Return -> ExprOrCall
    ASTNode *block = ast_create_node(AST_BLOCK);
    RETURN_INTERNAL_ERROR_IF_NULL(block, NULL);
    RETURN_IF_ERROR(ast_add_child(function, block));

    ASTNode *return_node = ast_create_node(AST_RETURN);
    RETURN_INTERNAL_ERROR_IF_NULL(return_node, NULL);
    RETURN_IF_ERROR(ast_add_child(block, return_node));

    RETURN_IF_ERROR(parse_expr_or_call(parser, return_node));
    RETURN_IF_ERROR(parse_eol_or_eof(parser));
    break;
  }
  // Multi line function definition
  case TOKEN_NEWLINE:
    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_block(parser, function));
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_function_body()

ErrorCode parse_eol_or_eof(Parser *parser) {
  if (tokens_matches(parser, TOKEN_NEWLINE)) {
    RETURN_IF_ERROR(new_token(parser));
    return ERROR_OK;
  } else if (tokens_matches(parser, TOKEN_EOF)) {
    return ERROR_OK;
  } else {
    RETURN_SYNTACTIC_ERROR(parser);
  }
} // parse_eol_or_eof()

ErrorCode parse_block(Parser *parser, ASTNode *parent) {
  if (tokens_matches(parser, TOKEN_INDENT)) {
    ASTNode *block = ast_create_node(AST_BLOCK);
    RETURN_INTERNAL_ERROR_IF_NULL(block, NULL);
    RETURN_IF_ERROR(ast_add_child(parent, block));

    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_statement_list(parser, block));
    RETURN_IF_ERROR(expect(parser, TOKEN_DEDENT));
    return ERROR_OK;
  } else {
    RETURN_SYNTACTIC_ERROR(parser);
  }
} // parse_block()

ErrorCode parse_statement_list(Parser *parser, ASTNode *block) {
  RETURN_IF_ERROR(parse_statement(parser, block));
  RETURN_IF_ERROR(parse_statement_end(parser));
  RETURN_IF_ERROR(parse_statement_list_rest(parser, block));

  return ERROR_OK;
} // parse_statement_list()

ErrorCode parse_statement_list_rest(Parser *parser, ASTNode *block) {
  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_VAL:
    // fall through
  case TOKEN_VAR:
    // fall through
  case TOKEN_IF:
    // fall through
  case TOKEN_WHILE:
    // fall through
  case TOKEN_RETURN:
    // fall through
  case TOKEN_IDENTIFIER:
    // fall through
  case TOKEN_INT_LITERAL:
    // fall through
  case TOKEN_DOUBLE_LITERAL:
    // fall through
  case TOKEN_STRING_LITERAL:
    // fall through
  case TOKEN_LPAREN:
    RETURN_IF_ERROR(parse_statement(parser, block));
    RETURN_IF_ERROR(parse_statement_end(parser));
    RETURN_IF_ERROR(parse_statement_list_rest(parser, block));
    break;
  case TOKEN_DEDENT:
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_statement_list_rest()

ErrorCode parse_statement_end(Parser *parser) {
  if (tokens_matches(parser, TOKEN_NEWLINE) || tokens_matches(parser, TOKEN_SEMICOLON)) {
    RETURN_IF_ERROR(new_token(parser));
    return ERROR_OK;
  } else {
    RETURN_SYNTACTIC_ERROR(parser);
  }
} // parse_statement_end()

ErrorCode parse_statement(Parser *parser, ASTNode *block) {
  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_VAL:
    // fall through
  case TOKEN_VAR:
    RETURN_IF_ERROR(parse_var_def(parser, block));
    break;
  case TOKEN_IF:
    RETURN_IF_ERROR(parse_if_statement(parser, block));
    break;
  case TOKEN_WHILE:
    RETURN_IF_ERROR(parse_while_statement(parser, block));
    break;
  case TOKEN_RETURN:
    RETURN_IF_ERROR(parse_return_statement(parser, block));
    break;
  case TOKEN_IDENTIFIER: {
    ASTNode *statement = ast_create_node(AST_IDENTIFIER);
    RETURN_INTERNAL_ERROR_IF_NULL(statement, NULL);
    RETURN_IF_ERROR(ast_add_child(block, statement));
    RETURN_IF_ERROR(ast_set_text(statement, parser->current_token->value));

    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_statement_after_id(parser, statement));
    break;
  }
  case TOKEN_INT_LITERAL:
    // fall through
  case TOKEN_DOUBLE_LITERAL:
    // fall through
  case TOKEN_STRING_LITERAL:
    // fall through
  case TOKEN_LPAREN:
    RETURN_IF_ERROR(parse_expr_non_id(parser, block));
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_statement()

ErrorCode parse_statement_after_id(Parser *parser, ASTNode *statement) {
  RETURN_INTERNAL_ERROR_IF_NULL(statement, "Statement node in ASTree is NULL.");

  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_ASSIGN:
    statement->type = AST_ASSIGN;
    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_expr_or_call(parser, statement));
    break;
  case TOKEN_LPAREN:
    statement->type = AST_CALL;
    RETURN_IF_ERROR(parse_arg_group(parser, statement));
    break;
  case TOKEN_NEWLINE:
    // fall through
  case TOKEN_SEMICOLON:
    RETURN_IF_ERROR(parse_expr_tail(parser, statement));
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_statement_after_id()

ErrorCode parse_expr_non_id(Parser *parser, ASTNode *parent) {
  RETURN_IF_ERROR(check_current_token(parser));

  ASTNode *expression = ast_create_node(AST_EXPR);
  RETURN_INTERNAL_ERROR_IF_NULL(expression, NULL);
  RETURN_IF_ERROR(ast_add_child(parent, expression));

  switch (parser->current_token->type) {
  case TOKEN_LPAREN:
    RETURN_IF_ERROR(new_token(parser));
    if (!tokens_matches(parser, TOKEN_RPAREN)) {
      // ExprNonId -> LParen Expr RParen ExprTail
      RETURN_IF_ERROR(parse_expr(parser, expression));
      RETURN_IF_ERROR(expect(parser, TOKEN_RPAREN));
      RETURN_IF_ERROR(parse_expr_tail(parser, expression));
      return ERROR_OK;
    }
    expression->type = AST_UNIT_LITERAL;
    break;

  case TOKEN_INT_LITERAL: {
    expression->type = AST_INT_LITERAL;
    RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token->value, "Literal token value is NULL.");

    char *end; // pointer to end of strtol() reading
    errno = 0;
    long value = strtol(parser->current_token->value, &end, 10);
    if (errno == ERANGE || end == parser->current_token->value || *end != '\0' || value < INT32_MIN ||
        value > INT32_MAX) {
      print_err(ERROR_LEXICAL, "Invalid Int literal.");
      return ERROR_LEXICAL;
    }
    expression->int_value = (int32_t)value;
    break;
  }
  case TOKEN_DOUBLE_LITERAL: {
    expression->type = AST_DOUBLE_LITERAL;
    RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token->value, "Literal token value is NULL.");

    char *end; // pointer to end of strtod() reading
    errno = 0;
    double value = strtod(parser->current_token->value, &end);
    if (errno == ERANGE || end == parser->current_token->value || *end != '\0' || !isfinite(value)) {
      print_err(ERROR_LEXICAL, "Invalid Double literal.");
      return ERROR_LEXICAL;
    }
    expression->double_value = value;
    break;
  }
  case TOKEN_STRING_LITERAL:
    expression->type = AST_STRING_LITERAL;
    RETURN_INTERNAL_ERROR_IF_NULL(parser->current_token->value, "Literal token value is NULL.");

    RETURN_IF_ERROR(ast_set_text(expression, parser->current_token->value));
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  RETURN_IF_ERROR(new_token(parser));
  RETURN_IF_ERROR(parse_expr_tail(parser, expression));

  return ERROR_OK;
} // parse_expr_non_id()

ErrorCode parse_var_def(Parser *parser, ASTNode *block) {
  ASTNode *declaration = ast_create_node(AST_VAR_DECLARATION);
  RETURN_INTERNAL_ERROR_IF_NULL(declaration, NULL);
  RETURN_IF_ERROR(ast_add_child(block, declaration));

  RETURN_IF_ERROR(parse_var_kind(parser, declaration));
  RETURN_IF_ERROR(compare_tokens(parser, TOKEN_IDENTIFIER));
  RETURN_IF_ERROR(ast_set_text(declaration, parser->current_token->value));
  RETURN_IF_ERROR(new_token(parser));
  RETURN_IF_ERROR(parse_type_opt(parser, declaration));
  RETURN_IF_ERROR(expect(parser, TOKEN_ASSIGN));
  RETURN_IF_ERROR(parse_expr_or_call(parser, declaration));

  return ERROR_OK;
} // parse_var_def()

ErrorCode parse_var_kind(Parser *parser, ASTNode *declaration) {
  RETURN_INTERNAL_ERROR_IF_NULL(declaration, "Declaration node in ASTree is NULL.");

  RETURN_IF_ERROR(check_current_token(parser));

  if (tokens_matches(parser, TOKEN_VAL) || tokens_matches(parser, TOKEN_VAR)) {
    declaration->is_changeable = parser->current_token->type == TOKEN_VAR;
    RETURN_IF_ERROR(new_token(parser));
    return ERROR_OK;
  } else {
    RETURN_SYNTACTIC_ERROR(parser);
  }
} // parse_var_kind()

ErrorCode parse_if_statement(Parser *parser, ASTNode *block) {
  ASTNode *if_node = ast_create_node(AST_IF);
  RETURN_INTERNAL_ERROR_IF_NULL(if_node, NULL);
  RETURN_IF_ERROR(ast_add_child(block, if_node));

  RETURN_IF_ERROR(expect(parser, TOKEN_IF));

  ASTNode *condition = ast_create_node(AST_EXPR);
  RETURN_INTERNAL_ERROR_IF_NULL(condition, NULL);
  RETURN_IF_ERROR(ast_add_child(if_node, condition));

  RETURN_IF_ERROR(parse_expr(parser, condition));
  RETURN_IF_ERROR(expect(parser, TOKEN_THEN));
  RETURN_IF_ERROR(expect(parser, TOKEN_NEWLINE));
  RETURN_IF_ERROR(parse_block(parser, if_node));
  RETURN_IF_ERROR(expect(parser, TOKEN_NEWLINE));
  RETURN_IF_ERROR(expect(parser, TOKEN_ELSE));
  RETURN_IF_ERROR(expect(parser, TOKEN_NEWLINE));
  RETURN_IF_ERROR(parse_block(parser, if_node));

  return ERROR_OK;
} // parse_if_statement()

ErrorCode parse_while_statement(Parser *parser, ASTNode *block) {
  ASTNode *while_node = ast_create_node(AST_WHILE);
  RETURN_INTERNAL_ERROR_IF_NULL(while_node, NULL);
  RETURN_IF_ERROR(ast_add_child(block, while_node));

  RETURN_IF_ERROR(expect(parser, TOKEN_WHILE));

  ASTNode *condition = ast_create_node(AST_EXPR);
  RETURN_INTERNAL_ERROR_IF_NULL(condition, NULL);
  RETURN_IF_ERROR(ast_add_child(while_node, condition));

  RETURN_IF_ERROR(parse_expr(parser, condition));
  RETURN_IF_ERROR(expect(parser, TOKEN_DO));
  RETURN_IF_ERROR(expect(parser, TOKEN_NEWLINE));
  RETURN_IF_ERROR(parse_block(parser, while_node));

  return ERROR_OK;
} // parse_while_statement()

ErrorCode parse_return_statement(Parser *parser, ASTNode *block) {
  ASTNode *return_node = ast_create_node(AST_RETURN);
  RETURN_INTERNAL_ERROR_IF_NULL(return_node, NULL);
  RETURN_IF_ERROR(ast_add_child(block, return_node));

  RETURN_IF_ERROR(expect(parser, TOKEN_RETURN));
  RETURN_IF_ERROR(parse_return_expr_opt(parser, return_node));

  return ERROR_OK;
} // parse_return_statement()

ErrorCode parse_return_expr_opt(Parser *parser, ASTNode *return_node) {
  RETURN_INTERNAL_ERROR_IF_NULL(return_node, "Return node in ASTree is NULL.");

  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_IDENTIFIER:
    // fall through
  case TOKEN_INT_LITERAL:
    // fall through
  case TOKEN_DOUBLE_LITERAL:
    // fall through
  case TOKEN_STRING_LITERAL:
    // fall through
  case TOKEN_LPAREN:
    RETURN_IF_ERROR(parse_expr_or_call(parser, return_node));
    break;
  case TOKEN_NEWLINE:
    // fall through
  case TOKEN_SEMICOLON:
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_return_expr_opt()

ErrorCode parse_expr_or_call(Parser *parser, ASTNode *parent) {
  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_IDENTIFIER: {
    ASTNode *expression = ast_create_node(AST_IDENTIFIER);
    RETURN_INTERNAL_ERROR_IF_NULL(expression, NULL);
    RETURN_IF_ERROR(ast_add_child(parent, expression));
    RETURN_IF_ERROR(ast_set_text(expression, parser->current_token->value));

    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_call_opt(parser, expression));
    break;
  }
  case TOKEN_INT_LITERAL:
    // fall through
  case TOKEN_DOUBLE_LITERAL:
    // fall through
  case TOKEN_STRING_LITERAL:
    // fall through
  case TOKEN_LPAREN:
    RETURN_IF_ERROR(parse_expr(parser, parent));
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_expr_or_call()

ErrorCode parse_call_opt(Parser *parser, ASTNode *expression) {
  RETURN_INTERNAL_ERROR_IF_NULL(expression, "Expression node in ASTree is NULL.");

  if (tokens_matches(parser, TOKEN_LPAREN)) {
    expression->type = AST_CALL;
    RETURN_IF_ERROR(parse_arg_group(parser, expression));
    return ERROR_OK;
  } else if (tokens_matches(parser, TOKEN_NEWLINE) || tokens_matches(parser, TOKEN_EOF) ||
             tokens_matches(parser, TOKEN_SEMICOLON)) {
    return ERROR_OK;
  } else {
    RETURN_SYNTACTIC_ERROR(parser);
  }
} // parse_call_opt()

ErrorCode parse_arg_group(Parser *parser, ASTNode *call) {
  ASTNode *arg_list = ast_create_node(AST_ARG_LIST);
  RETURN_INTERNAL_ERROR_IF_NULL(arg_list, NULL);
  RETURN_IF_ERROR(ast_add_child(call, arg_list));

  RETURN_IF_ERROR(expect(parser, TOKEN_LPAREN));
  RETURN_IF_ERROR(parse_arg_list_opt(parser, arg_list));
  RETURN_IF_ERROR(expect(parser, TOKEN_RPAREN));

  return ERROR_OK;
} // parse_arg_group()

ErrorCode parse_arg_list_opt(Parser *parser, ASTNode *arg_list) {
  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_IDENTIFIER:
    // fall through
  case TOKEN_INT_LITERAL:
    // fall through
  case TOKEN_DOUBLE_LITERAL:
    // fall through
  case TOKEN_STRING_LITERAL:
    // fall through
  case TOKEN_LPAREN:
    RETURN_IF_ERROR(parse_arg_list(parser, arg_list));
    break;
  case TOKEN_RPAREN:
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_arg_list_opt()

ErrorCode parse_arg_list(Parser *parser, ASTNode *arg_list) {
  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_IDENTIFIER:
    // fall through
  case TOKEN_INT_LITERAL:
    // fall through
  case TOKEN_DOUBLE_LITERAL:
    // fall through
  case TOKEN_STRING_LITERAL:
    // fall through
  case TOKEN_LPAREN:
    RETURN_IF_ERROR(parse_expr(parser, arg_list));
    RETURN_IF_ERROR(parse_arg_list_rest(parser, arg_list));
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_arg_list()

ErrorCode parse_arg_list_rest(Parser *parser, ASTNode *arg_list) {
  if (tokens_matches(parser, TOKEN_COMMA)) {
    RETURN_IF_ERROR(new_token(parser));
    RETURN_IF_ERROR(parse_arg_list_rest_tail(parser, arg_list));

    return ERROR_OK;
  } else if (tokens_matches(parser, TOKEN_RPAREN)) {
    return ERROR_OK;
  } else {
    RETURN_SYNTACTIC_ERROR(parser);
  }
} // parse_arg_list_rest()

ErrorCode parse_arg_list_rest_tail(Parser *parser, ASTNode *arg_list) {
  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
  case TOKEN_IDENTIFIER:
    // fall through
  case TOKEN_INT_LITERAL:
    // fall through
  case TOKEN_DOUBLE_LITERAL:
    // fall through
  case TOKEN_STRING_LITERAL:
    // fall through
  case TOKEN_LPAREN:
    RETURN_IF_ERROR(parse_arg_list(parser, arg_list));
    break;
  case TOKEN_RPAREN:
    // ArgListRestTail -> ε {checkLF(RParen)}
    if (!parser->current_token->lf_before_rparen) {
      RETURN_SYNTACTIC_ERROR(parser);
    }
    break;
  default:
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // parse_arg_list_rest_tail()

ErrorCode parse_eol_opt(Parser *parser) {
  // EOLOpt -> EOL
  if (tokens_matches(parser, TOKEN_NEWLINE)) {
    RETURN_IF_ERROR(new_token(parser));
  } else { // EOLOpt -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_DEF));
  }

  return ERROR_OK;
} // parse_eol_opt()

/* End of file parser.c */
