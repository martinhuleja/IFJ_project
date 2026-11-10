/**
 * @file   abstract-syntactic-tree.c
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-10
 *
 * @brief  Implementation of abstract syntactic tree
 */

#include "abstract-syntactic-tree.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

ASTree *ast_init() {
  ASTree *tree = (ASTree *)malloc(sizeof(ASTree));
  if (tree == NULL) {
    print_err(ERROR_INTERNAL, "Couldn't allocate struct ASTree.");
    return NULL;
  }

  ASTNode *program = ast_create_node(AST_PROGRAM);
  if (program == NULL) {
    free(tree);
    return NULL;
  }

  tree->root = program;

  return tree;
} // ast_init()

ASTNode *ast_create_node(ASTNodeType type) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (node == NULL) {
    print_err(ERROR_INTERNAL, "Couldn't allocate struct ASTNode.");
    return NULL;
  }

  // Set node->type to 'type', other values in node to NULL, false, 0
  *node = (ASTNode){.type = type};

  return node;
} // ast_create_node()

ErrorCode ast_add_child(ASTNode *parent, ASTNode *child) {
  if (parent == NULL || child == NULL) {
    print_err(ERROR_INTERNAL, "Parent or child node is NULL.");
    ast_node_free(child);
    return ERROR_INTERNAL;
  }

  // If capacity isn't big enough, double the parent->children_capacity
  if (parent->children_count >= parent->children_capacity) {
    if (parent->children_capacity > SIZE_MAX / sizeof(ASTNode *) / 2) {
      print_err(ERROR_INTERNAL, "AST children capacity is too large.");
      ast_node_free(child);
      return ERROR_INTERNAL;
    }

    size_t capacity = parent->children_capacity == 0 ? 2 : parent->children_capacity * 2;
    ASTNode **children = (ASTNode **)realloc(parent->children, capacity * sizeof(ASTNode *));
    if (children == NULL) {
      print_err(ERROR_INTERNAL, "Couldn't allocate AST children array.");
      ast_node_free(child);
      return ERROR_INTERNAL;
    }

    parent->children = children;
    parent->children_capacity = capacity;
  } // if (count >= capacity)

  parent->children[parent->children_count++] = child;

  return ERROR_OK;
} // ast_add_child()

ErrorCode ast_set_text(ASTNode *node, const char *text) {
  if (node == NULL || text == NULL) {
    print_err(ERROR_INTERNAL, "AST node or text is NULL.");
    return ERROR_INTERNAL;
  }

  size_t text_length = strlen(text) + 1;
  char *copy_of_the_text = (char *)malloc(text_length * sizeof(char));
  if (copy_of_the_text == NULL) {
    print_err(ERROR_INTERNAL, "Couldn't allocate AST node text.");
    return ERROR_INTERNAL;
  }

  memcpy(copy_of_the_text, text, text_length);
  free(node->text);
  node->text = copy_of_the_text;

  return ERROR_OK;
} // ast_set_text()

void ast_node_free(ASTNode *node) {
  if (node == NULL) {
    return;
  }

  for (size_t i = 0; i < node->children_count; i++) {
    ast_node_free(node->children[i]);
  }

  free(node->text);
  free(node->children);
  free(node);
} // ast_node_free()

void ast_free(ASTree *tree) {
  if (tree == NULL) {
    return;
  }

  ast_node_free(tree->root);
  free(tree);
} // ast_free()

/* End of file abstract-syntactic-tree.c */
