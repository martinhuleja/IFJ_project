/**
 * @file   abstract-syntactic-tree.h
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-10
 *
 * @brief  Header file for abstract syntactic tree
 */

#ifndef ABSTRACT_SYNTACTIC_TREE_H
#define ABSTRACT_SYNTACTIC_TREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum ast_node_type {
  AST_PROGRAM,
  AST_FUNCTION,
  AST_PARAM_LIST,
  AST_PARAM,
  AST_BLOCK,
  AST_VAR_DECLARATION,
  AST_ASSIGN,
  AST_IF,
  AST_WHILE,
  AST_RETURN,
  AST_EXPR,
  AST_CALL,
  AST_ARG_LIST,
  AST_BINARY_OP,
  AST_UNARY_OP,
  AST_IDENTIFIER,
  AST_INT_LITERAL,
  AST_DOUBLE_LITERAL,
  AST_STRING_LITERAL,
  AST_UNIT_LITERAL,
} ASTNodeType;

typedef enum ast_operator {
  AST_OP_NONE,
  AST_OP_ADD,
  AST_OP_SUB,
  AST_OP_MUL,
  AST_OP_DIV,
  AST_OP_EQUAL,
  AST_OP_NOT_EQUAL,
  AST_OP_LESS,
  AST_OP_LESS_EQUAL,
  AST_OP_GREATER,
  AST_OP_GREATER_EQUAL,
  AST_OP_NEGATION,
} ASTOperator;

typedef enum ast_data_type {
  AST_DATA_UNKNOWN,
  AST_DATA_INT,
  AST_DATA_DOUBLE,
  AST_DATA_STRING,
  AST_DATA_UNIT,
} ASTDataType;

/**
 * @brief A single node of the abstract syntax tree
 */
typedef struct ast_node {
  ASTNodeType type;

  // Node atributes
  char *text; // Identifier or StringLiteral value
  int32_t int_value;
  double double_value;
  bool is_changeable; // var (true), val (false)
  bool is_main;       // @main (true)
  ASTOperator operator;
  ASTDataType declared_type; // Type declared in code
  ASTDataType resolved_type; // Type resolved by semantic analysis

  // Children nodes
  struct ast_node **children; // Owned nodes in the order of the source code
  size_t children_count;
  size_t children_capacity;
} ASTNode;

/**
 * @brief Abstract syntax tree representing the entire source program
 */
typedef struct ast_tree {
  ASTNode *root; // Root node of the program
} ASTree;

#endif // ABSTRACT_SYNTACTIC_TREE_H
/* End of file abstract-syntactic-tree.h */
