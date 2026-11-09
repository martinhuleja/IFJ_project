/**
 * @file   parser-missing-declarations.c
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-09
 *
 * DELETE THIS FILE BEFORE SUBMISSION.
 *
 * This file..
 * - is used just for definition of functions which are currently missing,
 * - should be deleted after implementation of these function in other modules,
 * - is necessary for compiling the program.
 */

#include "token.h"
#include "error.h"
#include "parser.h"
#include "parser-missing-declarations.h"
#include <stdlib.h>

Token *get_next_token() {
  Token *token = (Token *)malloc(sizeof(Token));
  return token;
}

ErrorCode parse_expr(Parser *parser) {
  parser = parser;
  return ERROR_OK;
}

ErrorCode parse_expr_tail(Parser *parser) {
  parser = parser;
  return ERROR_OK;
}

/* End of file parser-missing-declarations.c */
