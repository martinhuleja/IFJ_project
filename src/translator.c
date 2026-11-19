/**
 * @file   translator.c
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-10
 *
 * @brief  Main file for IFJ26 translator
 *
 * Controls whole program and allocated memory for ASTree.
 */
#include "semantic_check.h"
#include "abstract-syntactic-tree.h"
#include "error.h"
#include "parser.h"
#include "codegen.h"
#include <stdlib.h>

int main() {
  ASTree *tree = ast_init();
  if (tree == NULL) {
    return ERROR_INTERNAL;
  }

  ErrorCode return_value = ERROR_OK;

  return_value = parse_program(tree);

  if (return_value == ERROR_OK) {
    return_value = semantic_analysis(tree);
  }
  if (return_value == ERROR_OK && !codegen_program(tree)) {
    return_value = ERROR_INTERNAL;
  }

  ast_free(tree);

  return return_value;
}
