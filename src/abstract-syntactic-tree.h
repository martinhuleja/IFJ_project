/**
 * @file   abstract-syntactic-tree.h
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-17
 *
 * @brief  Declaration of abstract syntactic tree functions
 */

#ifndef ABSTRACT_SYNTACTIC_TREE_H
#define ABSTRACT_SYNTACTIC_TREE_H

#include "error.h"
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
  AST_DATA_ANY,
} ASTDataType;

/**
 * @brief A single node of the abstract syntax tree
 *
 * @note Payload and ordered children produced by parser:
 * - AST_PROGRAM: function definitions.
 * - AST_FUNCTION: text = name, is_main, declared_type; [AST_PARAM_LIST, AST_BLOCK].
 * - AST_PARAM_LIST: parameters in declaration order.
 * - AST_PARAM: text = name, declared_type; no children.
 * - AST_BLOCK: statements in source order; expressions and calls are stored directly, also in a one-line body.
 * - AST_VAR_DECLARATION: text = name, is_changeable, declared_type; [initializer].
 * - AST_ASSIGN: text = target name (no identifier child); [assigned expression or call].
 * - AST_IF: [AST_EXPR condition, AST_BLOCK then, AST_BLOCK else].
 * - AST_WHILE: [AST_EXPR condition, AST_BLOCK body].
 * - AST_RETURN: explicit return only; [expression or call], or no children for a bare return.
 * - AST_CALL: text = function name; [AST_ARG_LIST], including for an empty call.
 * - AST_ARG_LIST: expressions in argument order.
 * - AST_EXPR: [expression], used for conditions, parentheses in expressions do not create a wrapper.
 * - AST_IDENTIFIER: text = name; no children.
 * - Literals: int_value, double_value or text according to type; no children.
 *   AST_UNIT_LITERAL is created by PSA for () and has no payload.
 *
 * Expression subtrees are recursive, not flat lists of operands and operators.
 * When an AST_EXPR wrapper is used, children[0] is the single root of the entire expression:
 * - A single operand is an AST_IDENTIFIER or a literal leaf, without children.
 * - A unary operation is an AST_UNARY_OP root with operator set to AST_OP_NEGATION;
 *   children[0] is the operand subtree.
 * - A binary operation is an AST_BINARY_OP root with operator set to the specific operation;
 *   children[0] is the left operand subtree and children[1] is the right operand subtree.
 * Operand subtrees may themselves contain operations. Their nesting reflects precedence, associativity and grouping.
 * For example, for a + b * c, the root is addition and its right child is multiplication.
 * Grouping parentheses do not create nodes. Precedence syntactic analysis appends the expression root
 * to its supplied parent. Initializers, return values, arguments and expression statements store this root
 * directly, without an additional AST_EXPR wrapper.
 * resolved_type is reserved for semantic analysis.
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

/**
 * @brief Initializes struct ast_tree
 *
 * Creates an empty root node of type AST_PROGRAM.
 * If allocation fails, frees any allocated memory and returns NULL.
 *
 * @return Ptr to initialized struct ast_tree, or NULL on allocation failure
 */
ASTree *ast_init();

/**
 * @brief Creates and initializes an AST node of the given type
 *
 * @note The caller owns the node until it is passed to ast_add_child().
 *
 * @param type Type of the created node
 * @return Ptr to initialized struct ast_node, or NULL on allocation failure
 */
ASTNode *ast_create_node(ASTNodeType type);

/**
 * @brief Adds a child node at the end of the parent's children
 *
 * Takes ownership of the child, which must not belong to another parent.
 * If adding the child fails, frees its entire subtree using ast_node_free()
 * and leaves the parent unchanged. The caller must not use the freed child.
 *
 * @param parent Ptr to the parent node
 * @param child Ptr to the child node
 * @return ERROR_OK on success, or ERROR_INTERNAL on failure
 */
ErrorCode ast_add_child(ASTNode *parent, ASTNode *child);

/**
 * @brief Sets the text stored in an AST node
 *
 * Stores its own copy of the text and frees any previously stored text.
 * If allocation fails, leaves the original text unchanged.
 *
 * @param node Ptr to the AST node
 * @param text Text to copy into the node
 * @return ERROR_OK on success, or ERROR_INTERNAL on failure
 */
ErrorCode ast_set_text(ASTNode *node, const char *text);

/**
 * @brief Frees the memory allocated for an AST node and its subtree
 *
 * Frees all child nodes, the stored text and the children array too.
 * If node is NULL, does nothing.
 *
 * @param node Ptr to the root node of the subtree to free
 */
void ast_node_free(ASTNode *node);

/**
 * @brief Frees the memory allocated for struct ast_tree
 *
 * Frees the entire program subtree using ast_node_free().
 * If tree is NULL, does nothing.
 *
 * @param tree Ptr to struct ast_tree
 */
void ast_free(ASTree *tree);

#endif // ABSTRACT_SYNTACTIC_TREE_H
/* End of file abstract-syntactic-tree.h */
