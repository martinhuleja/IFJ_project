/**
 * @file   parser.c
 * @author Martin Huleja (xhulejm00)
 * @date   2026-10-28
 *
 * @brief  Implementation of parser functions
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "token.h"
#include "error.h"

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
#define RETURN_IF_ERROR(foo) do { \
  ErrorCode error_code = (foo); \
  if (error_code != ERROR_OK) { \
    return error_code; \
  } \
} while(0)

/**
 * @brief Prints error msg and return with syntactic ErrorCode
 * @param Ptr to struct parser
 */
#define RETURN_SYNTACTIC_ERROR(parser) do { \
  if (parser->print_error_msg) { \
    print_err(ERROR_SYNTACTIC, "Expected token doesn't match the current one."); \
  } \
  return ERROR_SYNTACTIC; \
} while(0)

Parser *parser_init() {
  Parser *parser = (Parser *)malloc(sizeof(Parser));
  if (parser == NULL) {
    print_err(ERROR_INTERNAL, "Internal malloc error.");
    return NULL;
  }

  parser->current_token = NULL;
  parser->print_error_msg = true;
  return parser;
} // parser_init()

void parser_free(Parser *parser) {
  if (parser != NULL) {
    free(parser);
  }
} // parser_free()

ErrorCode check_current_token(Parser *parser) {
  if (parser == NULL || parser->current_token == NULL) {
    print_err(ERROR_INTERNAL, "current_token is empty.");
    return ERROR_INTERNAL;
  }

  return ERROR_OK;
}

ErrorCode compare_tokens(Parser *parser, TokenType expected_type) {
  RETURN_IF_ERROR(check_current_token(parser));

  // Compare current token type to the expected one
  if (parser->current_token->type != expected_type) {
    RETURN_SYNTACTIC_ERROR(parser);
  }

  return ERROR_OK;
} // compare_tokens()

bool tokens_matches(Parser *parser, TokenType expected_token) {
  parser->print_error_msg = false;
  bool return_value = (compare_tokens(parser, expected_token) == ERROR_OK);
  parser->print_error_msg = true; // Reset print_error_msg value
  return return_value;
} // tokens_matches()

ErrorCode new_token(Parser *parser, TokenType expected_type) {
  // If parser haven't expected EOF, get another token
  if (expected_type != TOKEN_EOF) {
    parser->current_token = get_next_token();
    if (parser->current_token == NULL) {
      print_err(ERROR_LEXICAL, "No more tokens to return.");
      return ERROR_LEXICAL;
    }
  }

  return ERROR_OK;
} // new_token()

ErrorCode expect(Parser *parser, TokenType expected_type) {
  RETURN_IF_ERROR(compare_tokens(parser, expected_type));
  RETURN_IF_ERROR(new_token(parser, expected_type));

  return ERROR_OK;
} // expect()

/********************************************************************
 ************** PARSING ACCORDING TO LL GRAMMAR RULES ***************
 *******************************************************************/

ErrorCode parse_program() {
  Parser *parser = parser_init();
  if (parser == NULL) {
    return ERROR_INTERNAL;
  }

  RETURN_IF_ERROR(new_token(parser, TOKEN_IMPORT));
  RETURN_IF_ERROR(parse_prologue(parser));
  RETURN_IF_ERROR(parse_function_list(parser));
  RETURN_IF_ERROR(expect(parser, TOKEN_EOF));

  return ERROR_OK;
} // parse_program()

ErrorCode parse_prologue(Parser *parser) {
  RETURN_IF_ERROR(expect(parser, TOKEN_IMPORT));

  // Manually execute expect()
  RETURN_IF_ERROR(compare_tokens(parser, TOKEN_IDENTIFIER));
  // Compare current token value with "ifj26"
  if (strcmp(parser->current_token->value, "ifj26") != 0) {
    RETURN_SYNTACTIC_ERROR(parser);
  }
  RETURN_IF_ERROR(new_token(parser, TOKEN_IDENTIFIER));

  RETURN_IF_ERROR(expect(parser, TOKEN_DOT));
  RETURN_IF_ERROR(expect(parser, TOKEN_STAR));
  RETURN_IF_ERROR(expect(parser, TOKEN_NEWLINE));

  return ERROR_OK;
} // parse_prologue()

ErrorCode parse_function_list(Parser *parser) {
  RETURN_IF_ERROR(parse_function_def(parser));
  RETURN_IF_ERROR(parse_function_list_rest(parser));

  return ERROR_OK;
} // parse_function_list()

ErrorCode parse_function_list_rest(Parser *parser) {
  // FunctionListRest -> FunctionDef FunctionListRest
  if (tokens_matches(parser, TOKEN_MAIN_ANNOTATION) || tokens_matches(parser, TOKEN_DEF)) {
    RETURN_IF_ERROR(parse_function_def(parser));
    RETURN_IF_ERROR(parse_function_list_rest(parser));
  } else { // FunctionListRest -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_EOF));
  }

  return ERROR_OK;
} // parse_function_list_rest()

ErrorCode parse_function_def(Parser *parser) {
  if (tokens_matches(parser, TOKEN_MAIN_ANNOTATION)) {
    RETURN_IF_ERROR(parse_main_opt(parser));
  }
  RETURN_IF_ERROR(parse_eol_opt(parser));
  RETURN_IF_ERROR(expect(parser, TOKEN_DEF));
  RETURN_IF_ERROR(expect(parser, TOKEN_IDENTIFIER));
  RETURN_IF_ERROR(parse_param_group(parser));
  RETURN_IF_ERROR(parse_type_opt(parser));
  RETURN_IF_ERROR(expect(parser, TOKEN_ASSIGN));
  RETURN_IF_ERROR(parse_function_body(parser));

  return ERROR_OK;
} // parse_function_def()

ErrorCode parse_main_opt(Parser *parser) {
  // MainOpt -> AtMain
  if (tokens_matches(parser, TOKEN_MAIN_ANNOTATION)) {
    RETURN_IF_ERROR(new_token(parser, TOKEN_MAIN_ANNOTATION));
  } else { // MainOpt -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_DEF));
  }

  return ERROR_OK;
} // parse_main_opt()

ErrorCode parse_param_group(Parser *parser) {
  RETURN_IF_ERROR(expect(parser, TOKEN_LPAREN));
  RETURN_IF_ERROR(parse_param_list_opt(parser));
  RETURN_IF_ERROR(expect(parser, TOKEN_RPAREN));

  return ERROR_OK;
} // parse_param_group()

ErrorCode parse_param_list_opt(Parser *parser) {
  // ParamListOpt -> ParamList
  if (tokens_matches(parser, TOKEN_IDENTIFIER)) {
    RETURN_IF_ERROR(parse_param_list(parser));
  } else { // ParamListOpt -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_RPAREN));
  }

  return ERROR_OK;
} // parse_param_list_opt()

ErrorCode parse_param_list(Parser *parser) {
  RETURN_IF_ERROR(parse_param(parser));
  RETURN_IF_ERROR(parse_param_list_rest(parser));

  return ERROR_OK;
} // parse_param_list()

ErrorCode parse_param_list_rest(Parser *parser) {
  // ParamListRest -> Comma Param ParamListRest
  if (tokens_matches(parser, TOKEN_COMMA)) {
    RETURN_IF_ERROR(expect(parser, TOKEN_COMMA));
    RETURN_IF_ERROR(parse_param(parser));
    RETURN_IF_ERROR(parse_param_list_rest(parser));
  } else { // ParamListRest -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_RPAREN));
  }

  return ERROR_OK;
} // parse_param_list_rest()

ErrorCode parse_param(Parser *parser) {
  RETURN_IF_ERROR(expect(parser, TOKEN_IDENTIFIER));
  RETURN_IF_ERROR(expect(parser, TOKEN_COLON));
  RETURN_IF_ERROR(parse_type(parser));

  return ERROR_OK;
} // parse_param()

ErrorCode parse_type_opt(Parser *parser) {
  // TypeOpt -> Colon Type
  if (tokens_matches(parser, TOKEN_COLON)) {
    RETURN_IF_ERROR(expect(parser, TOKEN_COLON));
    RETURN_IF_ERROR(parse_type(parser));
  } else { // TypeOpt -> ε
    RETURN_IF_ERROR(expect(parser, TOKEN_ASSIGN));
  }

  return ERROR_OK;
} // parse_type_opt()

ErrorCode parse_type(Parser *parser) {
  RETURN_IF_ERROR(check_current_token(parser));

  switch (parser->current_token->type) {
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_STRING:
    case TOKEN_UNIT:
      RETURN_IF_ERROR(new_token(parser, TOKEN_ASSIGN));
      return ERROR_OK;
    default:
      RETURN_SYNTACTIC_ERROR(parser);
  }
} // parse_type()

ErrorCode parse_function_body(Parser *parser);

ErrorCode parse_eol_or_eof(Parser *parser);

ErrorCode parse_block(Parser *parser);

ErrorCode parse_statement_list(Parser *parser);

ErrorCode parse_statement_list_rest(Parser *parser);

ErrorCode parse_statement_end(Parser *parser);

ErrorCode parse_statement(Parser *parser);

ErrorCode parse_statement_after_id(Parser *parser);

ErrorCode parse_expr_non_id(Parser *parser);

ErrorCode parse_var_def(Parser *parser);

ErrorCode parse_var_kind(Parser *parser);

ErrorCode parse_if_statement(Parser *parser);

ErrorCode parse_while_statement(Parser *parser);

ErrorCode parse_return_statement(Parser *parser);

ErrorCode parse_return_expr_opt(Parser *parser);

ErrorCode parse_expr_or_call(Parser *parser);

ErrorCode parse_call_opt(Parser *parser);

ErrorCode parse_arg_group(Parser *parser);

ErrorCode parse_arg_list_opt(Parser *parser);

ErrorCode parse_arg_list(Parser *parser);

ErrorCode parse_arg_list_rest(Parser *parser);

ErrorCode parse_expr_tail(Parser *parser);

ErrorCode parse_eol_opt(Parser *parser) {
  // EOLOpt -> EOL
  if (tokens_matches(parser, TOKEN_NEWLINE)) {
    RETURN_IF_ERROR(new_token(parser, TOKEN_NEWLINE));
  } else { // EOLOpt -> ε
    RETURN_IF_ERROR(compare_tokens(parser, TOKEN_DEF));
  }

  return ERROR_OK;
} // parse_eol_opt()

/* End of file parser.c */
