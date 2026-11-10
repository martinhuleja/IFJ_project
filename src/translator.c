#include "abstract-syntactic-tree.h"
#include "error.h"
#include "parser.h"
#include <stdlib.h>

int main() {
  ASTree *tree = ast_init();
  if (tree == NULL) {
    return ERROR_INTERNAL;
  }

  ErrorCode return_value = ERROR_OK;

  return_value = parse_program();

  if (return_value == ERROR_OK) {
    // semantic analysis
  }
  if (return_value == ERROR_OK) {
    // codegen
  }

  ast_free(tree);

  return return_value;
}
