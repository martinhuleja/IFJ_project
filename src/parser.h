/**
 * @file parser.h
 * @author Martin Huleja (xhulejm00)
 * @date 2026-10-28
 *
 * @brief Header file for parser functions
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "error.h"
#include "token.h"

typedef struct {
  Token *current_token; // Holds current token across all parser functions
  bool print_error_msg; // Program prints an error message according to this value
} Parser;

/********************************************************************
 ************************ SUPPORT FUNCTIONS *************************
 *******************************************************************/

/**
 * @brief Initialize struct parser
 *
 * @return ptr to initialized struct parser
 */
Parser *parser_init();

/**
 * @brief Frees the memory allocated for struct parser
 *
 * @param parser Ptr to struct parser
 */
void parser_free(Parser *parser);

/**
 * @brief Checks if struct parser or current token isn't NULL
 *
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode check_current_token(Parser *parser);

/**
 * @brief Compares current token with the expected token
 *
 * @param parser Ptr to struct parser
 * @param expected_type Expected token according to LL grammar rules
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode compare_tokens(Parser *parser, TokenType expected_type);

/**
 * @brief Checks if current token match the expected one and returns bool
 *
 * Compares tokens using compare_tokens(). Doesn't print error msg.
 * Used for comparing tokens, where failure doesn't mean an error.
 * Returns bool, so the code would be easier to read.
 *
 * @param Ptr to struct parser
 * @param expected_token Expected token according to LL grammar rules
 * @return True, if tokens match, othervise false
 */
bool tokens_matches(Parser *parser, TokenType expected_token);

/**
 * @brief If it's possible, gets next token from scanner
 *
 * Also checks if the action was succesfull.
 *
 * @param parser Ptr to struct parser
 * @param expected_type Expected token according to LL grammar rules
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode new_token(Parser *parser, TokenType expected_type);

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
 * @brief Parses Program according to 1st rule of LL grammar
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_program();

/**
 * @brief Parses Prologue according to 2nd rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_prologue(Parser *parser);

/**
 * @brief Parses FunctionList according to 3rd rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_function_list(Parser *parser);

/**
 * @brief Parses FunctionListRest according to 4th and 5th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_function_list_rest(Parser *parser);

/**
 * @brief Parses FunctionDef according to 6th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_function_def(Parser *parser);

/**
 * @brief Parses MainOpt according to 7th and 8th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_main_opt(Parser *parser);

/**
 * @brief Parses ParamGroup according to 9th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_group(Parser *parser);

/**
 * @brief Parses ParamListOpt according to 10th and 11th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_list_opt(Parser *parser);

/**
 * @brief Parses ParamList according to 12th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_list(Parser *parser);

/**
 * @brief Parses ParamListRest according to 13th and 14th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param_list_rest(Parser *parser);

/**
 * @brief Parses Param according to 15th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_param(Parser *parser);

/**
 * @brief Parses TypeOpt according to 16th and 17th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_type_opt(Parser *parser);

/**
 * @brief Parses Type according to 18th - 20th and 21st rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_type(Parser *parser);

/**
 * @brief Parses FunctionBody according to 22nd and 25th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_function_body(Parser *parser);

/**
 * @brief Parses EOLOrEOF according to 23rd and 24th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_eol_or_eof(Parser *parser);

/**
 * @brief Parses Block according to 26th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_block(Parser *parser);

/**
 * @brief Parses StatementList according to 27th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement_list(Parser *parser);

/**
 * @brief Parses StatementListRest according to 28th and 29th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement_list_rest(Parser *parser);

/**
 * @brief Parses StatementEnd according to 30th and 31st rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement_end(Parser *parser);

/**
 * @brief Parses Statement according to 32nd - 37th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement(Parser *parser);

/**
 * @brief Parses StatementAfterID according to 38th - 40th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_statement_after_id(Parser *parser);

/**
 * @brief Parses ExprNonId according to 41st - 45th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr_non_id(Parser *parser);

/**
 * @brief Parses VarDef according to 46th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_var_def(Parser *parser);

/**
 * @brief Parses VarKind according to 47th and 48th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_var_kind(Parser *parser);

/**
 * @brief Parses IfStatement according to 49th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_if_statement(Parser *parser);

/**
 * @brief Parses WhileStatement according to 50th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_while_statement(Parser *parser);

/**
 * @brief Parses ReturnStatement according to 51st rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_return_statement(Parser *parser);

/**
 * @brief Parses ReturnExprOpt according to 52nd and 53th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_return_expr_opt(Parser *parser);

/**
 * @brief Parses ExprOrCall according to 54th and 55th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr_or_call(Parser *parser);

/**
 * @brief Parses CallOpt according to 56th and 57th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_call_opt(Parser *parser);

/**
 * @brief Parses ArgGroup according to 58th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_group(Parser *parser);

/**
 * @brief Parses ArgListOpt according to 59th and 60th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_list_opt(Parser *parser);

/**
 * @brief Parses ArgList according to 61st rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_list(Parser *parser);

/**
 * @brief Parses ArgListRest according to 62nd and 63rd rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_arg_list_rest(Parser *parser);

/**
 * @brief Parses ExprTail according to 64th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr_tail(Parser *parser);

/**
 * @brief Parses EOLOpt according to 65th and 66th rule of LL grammar
 * @param parser Ptr to struct parser
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_eol_opt(Parser *parser);

#endif // PARSER_H
/* End of file parser.h */
