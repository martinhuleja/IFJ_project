/**
 * @file parser.h
 * @author Martin Huleja (xhulejm00)
 * @date 2026-11-09
 *
 * @brief Header file for parser functions
 */

#ifndef PARSER_H
#define PARSER_H

#include "abstract-syntactic-tree.h"
#include "error.h"
#include "token.h"
#include <stdbool.h>

typedef struct parser {
  Token *current_token;            // Holds current token across all parser functions
  Token *next_token;               // Holds next token (used when look up a token after the current one is necessarily)
  bool print_error_msg_and_return; // Prints an error message and returns according to this value
} Parser;

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

/**
 * @brief Initialize struct parser
 *
 * @return ptr to initialized struct parser
 */
Parser *parser_init();

/**
 * @brief Frees the memory allocated for struct parser
 *
 * In case current token or next token isn't NULL, frees memory allocated for these Tokens too.
 *
 * @param parser Ptr to struct parser
 */
void parser_free(Parser *parser);

/**
 * @brief Checks if struct parser or current token isn't NULL
 *
 * If it is, print error message and returns with ERROR_INTERNAL.
 *
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode check_current_token(Parser *parser);

/**
 * @brief Compares current token with the expected token
 *
 * Also checks that parser or current token isn't NULL using check_current_token().
 *
 * @param parser Ptr to struct parser
 * @param expected_type Expected token according to LL grammar rules
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode compare_tokens(Parser *parser, TokenType expected_type);

/**
 * @brief Compares next token with the expected token
 *
 * Also checks that parser or next token isn't NULL.
 *
 * @param parser Ptr to struct parser
 * @param expected_type Expected token according to LL grammar rules
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode compare_tokens_next_token(Parser *parser, TokenType expected_type);

/**
 * @brief Checks if current token matches the expected one and returns bool
 *
 * Compares tokens using compare_tokens(). Doesn't print error msg.
 * Used for comparing tokens, where failure doesn't mean an error.
 * Returns bool, so the code would be easier to read.
 *
 * @param Ptr to struct parser
 * @param expected_token Expected token according to LL grammar rules
 * @return True, if tokens matches, othervise false
 */
bool tokens_matches(Parser *parser, TokenType expected_token);

/**
 * @brief Checks if next token matches the expected one and returns bool
 *
 * Compares tokens using compare_tokens_next_token(). Doesn't print error msg.
 * Used for comparing tokens, where failure doesn't mean an error.
 * Returns bool, so the code would be easier to read.
 *
 * @param Ptr to struct parser
 * @param expected_token Expected token according to LL grammar rules
 * @return True, if tokens match, othervise false
 */
bool token_matches_next_token(Parser *parser, TokenType expected_token);

/**
 * @brief If it's possible, gets next token from scanner
 *
 * Deallocates memory used for current token.
 * If look_up_next_token() was used before, sets next token from struct parser as the current one
 * and sets next token to NULL.
 * Checks if calling scanner was succesfull.
 *
 * @param parser Ptr to struct parser
 * @param expected_type Expected token according to LL grammar rules
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode new_token(Parser *parser);

/**
 * @brief Looks up a token after the current one
 *
 * Saves the given token as next token to struct parser. Checks if calling scanner was succesfull.
 * If parser is NULL or next token isn't NULL, prints error message and returns with ERROR_INTERNAL.
 *
 * @param parser Ptr to struct parser
 * @param expected_type Expected token according to LL grammar rules
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode look_up_next_token(Parser *parser);

/**
 * @brief Compares current token with the expected token and gets next one
 *
 * Compares token types according to LL grammar rules.
 * Then gets next token.
 *
 * @param parser Ptr to struct parser
 * @param expected_type Expected token according to LL grammar rules
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode expect(Parser *parser, TokenType expected_type);

/********************************************************************
 ************** PARSING ACCORDING TO LL GRAMMAR RULES ***************
 *******************************************************************/

/**
 * @brief Parses Program according to LL grammar rules
 * @param tree Ptr to the AST of the entire program
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_program(ASTree *tree);

/**
 * @brief Parses Prologue according to LL grammar rules
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_prologue(Parser *parser);

/**
 * @brief Parses FunctionList according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param program Ptr to the program node receiving function definitions
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_function_list(Parser *parser, ASTNode *program);

/**
 * @brief Parses FunctionListRest according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param program Ptr to the program node receiving function definitions
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_function_list_rest(Parser *parser, ASTNode *program);

/**
 * @brief Parses FunctionDef according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param program Ptr to the program node receiving the function definition
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_function_def(Parser *parser, ASTNode *program);

/**
 * @brief Parses MainOpt according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param function Ptr to the function node whose main annotation is set
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_main_opt(Parser *parser, ASTNode *function);

/**
 * @brief Parses ParamGroup according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param function Ptr to the function node receiving the parameter list
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_group(Parser *parser, ASTNode *function);

/**
 * @brief Parses ParamListOpt according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param param_list Ptr to the parameter list node receiving parameters
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_list_opt(Parser *parser, ASTNode *param_list);

/**
 * @brief Parses ParamList according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param param_list Ptr to the parameter list node receiving parameters
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_list(Parser *parser, ASTNode *param_list);

/**
 * @brief Parses ParamListRest according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param param_list Ptr to the parameter list node receiving parameters
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_list_rest(Parser *parser, ASTNode *param_list);

/**
 * @brief Parses ParamListRestTail according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param param_list Ptr to the parameter list node receiving parameters
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_list_rest_tail(Parser *parser, ASTNode *param_list);

/**
 * @brief Parses Param according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param param_list Ptr to the parameter list node receiving the parameter
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param(Parser *parser, ASTNode *param_list);

/**
 * @brief Parses TypeOpt according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param declaration Ptr to the declaration node whose declared type is set
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_type_opt(Parser *parser, ASTNode *declaration);

/**
 * @brief Parses Type according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param declaration Ptr to the declaration node whose declared type is set
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_type(Parser *parser, ASTNode *declaration);

/**
 * @brief Parses FunctionBody according to LL grammar rules
 *
 * Stores the body in AST as AST_BLOCK. One-line bodies contain one expression or call directly.
 *
 * @param parser Ptr to struct parser
 * @param function Ptr to the function node receiving its body
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_function_body(Parser *parser, ASTNode *function);

/**
 * @brief Parses EOLOrEOF according to LL grammar rules
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_eol_or_eof(Parser *parser);

/**
 * @brief Parses Block according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param parent Ptr to the parent node receiving the block
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_block(Parser *parser, ASTNode *parent);

/**
 * @brief Parses StatementList according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param block Ptr to the block node receiving statements
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement_list(Parser *parser, ASTNode *block);

/**
 * @brief Parses StatementListRest according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param block Ptr to the block node receiving statements
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement_list_rest(Parser *parser, ASTNode *block);

/**
 * @brief Parses StatementEnd according to LL grammar rules
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement_end(Parser *parser);

/**
 * @brief Parses Statement using the next token to distinguish an assignment from an expression or call
 * @param parser Ptr to struct parser
 * @param block Ptr to the block node receiving the statement
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement(Parser *parser, ASTNode *block);

/**
 * @brief Parses Assignment according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param block Ptr to the block node receiving the assignment
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_assignment(Parser *parser, ASTNode *block);

/**
 * @brief Parses VarDef according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param block Ptr to the block node receiving the variable declaration
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_var_def(Parser *parser, ASTNode *block);

/**
 * @brief Parses VarKind according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param declaration Ptr to the variable declaration node whose mutability is set
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_var_kind(Parser *parser, ASTNode *declaration);

/**
 * @brief Parses IfStatement according to LL grammar rules
 *
 * Stores the condition as AST_EXPR, followed by the then block and the else block.
 *
 * @param parser Ptr to struct parser
 * @param block Ptr to the block node receiving the if statement
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_if_statement(Parser *parser, ASTNode *block);

/**
 * @brief Parses WhileStatement according to LL grammar rules
 *
 * Stores the condition as AST_EXPR, followed by the loop body block.
 *
 * @param parser Ptr to struct parser
 * @param block Ptr to the block node receiving the while statement
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_while_statement(Parser *parser, ASTNode *block);

/**
 * @brief Parses ReturnStatement according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param block Ptr to the block node receiving the return statement
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_return_statement(Parser *parser, ASTNode *block);

/**
 * @brief Parses ReturnExprOpt according to LL grammar rules
 *
 * Leaves the return node without children if no expression is present.
 *
 * @param parser Ptr to struct parser
 * @param return_node Ptr to the return statement node receiving its expression
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_return_expr_opt(Parser *parser, ASTNode *return_node);

/**
 * @brief Parses ExprOrCall using the next token to distinguish a call from an expression
 * @param parser Ptr to struct parser
 * @param parent Ptr to the parent node receiving the expression or function call
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr_or_call(Parser *parser, ASTNode *parent);

/**
 * @brief Parses Call according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param parent Ptr to the parent node receiving the function call
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_call(Parser *parser, ASTNode *parent);

/**
 * @brief Parses ArgGroup according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param call Ptr to the function call node receiving the argument list
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_group(Parser *parser, ASTNode *call);

/**
 * @brief Parses ArgListOpt according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param arg_list Ptr to the argument list node receiving arguments
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_list_opt(Parser *parser, ASTNode *arg_list);

/**
 * @brief Parses ArgList according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param arg_list Ptr to the argument list node receiving arguments
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_list(Parser *parser, ASTNode *arg_list);

/**
 * @brief Parses ArgListRest according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param arg_list Ptr to the argument list node receiving arguments
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_list_rest(Parser *parser, ASTNode *arg_list);

/**
 * @brief Parses ArgListRestTail according to LL grammar rules
 * @param parser Ptr to struct parser
 * @param arg_list Ptr to the argument list node receiving arguments
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_list_rest_tail(Parser *parser, ASTNode *arg_list);

/**
 * @brief Parses EOLOpt according to LL grammar rules
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_eol_opt(Parser *parser);

#endif // PARSER_H
/* End of file parser.h */
