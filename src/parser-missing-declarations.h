/**
 * @file   parser-missing-declarations.h
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-09
 *
 * DELETE THIS FILE BEFORE SUBMISSION.
 *
 * This file..
 * - is used just for declaration of functions which are currently missing,
 * - should be deleted after implementation of these function in other modules,
 * - is necessary for compiling the program.
 */

#ifndef PARSER_MISSING_DECLARATIONS_H
#define PARSER_MISSING_DECLARATIONS_H

#include "error.h"
#include "parser.h"
#include "token.h"

// Parser gets next token from scanner using this function
Token *get_next_token();

// Parser calls this function for evaluating expressions (will be implemented in precedence syntactic analysis)
ErrorCode parse_expr(Parser *parser, ASTNode *parent);

// Parser calls this function for evaluating expressions (will be implemented in precedence syntactic analysis)
ErrorCode parse_expr_tail(Parser *parser, ASTNode *expression);

#endif // PARSER_MISSING_DECLARATIONS_H
/* End of file parser-missing-declarations.h */
