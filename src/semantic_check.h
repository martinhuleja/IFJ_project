/**
 * @author Richard Komora (xkomorr00), FIT VUT
 * @note   IFJ + IAL project 2026/2027
 */
#ifndef SEMANTIC_CHECK_H
#define SEMANTIC_CHECK_H
#include "error.h"
#include "symtable.h"

/**
 * @brief Single linked list scope
 */
typedef struct Sscope scope;

ErrorCode semantic_analysis(ASTree *tree);
ErrorCode first_run(scope *global_scope, ASTree *tree);
ErrorCode built_in(scope *global_scope);
ErrorCode allocate_params(data *function_data);
ErrorCode second_run(scope *global_scope, ASTree *tree);
ErrorCode insert_params(scope *function_scope, ASTNode *params);
ErrorCode insert_block(scope *function_scope, ASTNode *block, ASTNode *function);
ErrorCode calculate_allocated(scope *current_scope, ASTNode *allocated, ASTNode *caller_function);
ErrorCode check_function(scope *global_scope, ASTNode *node, ASTNode *caller_function);

/**
 * @brief Creates a new scope and puts it on top of stack
 *
 * @param current_scope
 * @return PTR to new_scope on success, or NULL on malloc error
 */
scope *new_scope(scope *current_scope);

/**
 * @brief pops current scope from stack
 *
 * @param current_scope PTR to current_scope
 * @return PTR to previous_scope on success, or NULL if called with invalid scope
 */
scope *pop_scope(scope *current_scope);

/**
 * @brief checks if symbol exists across current and all parent scopes
 *
 * @param current_scope PTR to current_scope
 * @param *key pointer to symbol key
 * @return ptr to data of symbol on success, NULL if symbol not found, NULL If called with invalid scope
 */
data *lookup_symbol(scope *current_scope, char *key);

/**
 * @brief checks if symbol already exists in scope, then enters it into hash_table
 *
 * @param current_scope PTR to current_scope
 * @param key pointer to symbol key
 * @param data data of symbol to be inserted
 * @return ERROR_OK on success, ERROR_INTERNAL on malloc error,
 * ERROR_INTERNAL if called with invalid scope, ERROR_REDEFINITION if symbol already exists in scope
 */
ErrorCode insert_symbol(scope *current_scope, char *key, data data);

void free_scope(scope *current_scope);

/**
 * @brief takes current scope, key of variable, and ASTDataType of right side (value)
 *
 * @param current_scope PTR to current_scope
 * @param key pointer to varaible key
 * @param right_side ASTDataType of right side (value)
 * @return ERROR_OK on success, ERROR_IMMUTABLE_ASSIGNEMENT if not called with variable,
 * ERROR_INTERNAL if called with invalid scope, ERROR_TYPE_COMPATIBILITY if uncompabitible types,
 * ERROR_IMMUTABLE_ASSIGNMENT if leftisde is val, ERROR_UNDEFINED if  symbol doesnt exist
 */
ErrorCode check_assign_variable(scope *current_scope, char *key, ASTDataType right_side);

/**
 * @brief checks if two data types have compatible types
 *
 * doesnt check for invalid scopes/variables -> is only meant to be called as a helper function from other funcitons
 * that already checked
 *
 * @param left_side ASTDataType of first value
 * @param right_side ASTDataType of second value
 * @return ERROR_OK on success, ERROR_TYPE_COMPATIBILITY if uncompabitible types,
 */
ErrorCode check_type(ASTDataType left_side, ASTDataType right_side);

/**
 * @brief checks wether variable can be initialized and if yes, initializes it
 *
 * checks if variable hasnt been repeatedly declard, if right and left side are compatibile, if yes initializes it
 *
 * @param current_scope PTR to current_scope
 * @param key pointer to varaible key
 * @param data data of variable, that value should be assigned to
 * @param right_side ASTDataType of right side (value)
 * @return ERROR_OK on success, ERROR_INTERNAL on malloc_error, ERROR_INTERNAL if not valled with variable,
 * ERROR_INTERNAL if called with invalid scope, ERROR_TYPE_COMPATIBILITY if uncompabitible types,
 * ERROR_REDEFINITION if variable already exists within scope
 */
ErrorCode check_init_variable(scope *current_scope, char *key, data data, ASTDataType right_side);

/**
 * @brief registering functions
 *
 * registers functions, used during first run through of semantic analysis
 *
 * @param global_scope PTR to global scope
 * @param key pointer to varaible key
 * @param data data of function ought to be registred
 * @return ERROR_OK on sucess, ERROR_INTERNAL if called with invalid scope,
 * ERROR_REDEFINITION if funciton already defined, ERROR_INTERNAL on malloc error,
 * ERROR_INTERNAL if not called with function, ERROR_INTERNAL if function has internal data discrepencies.
 */
ErrorCode register_function(scope *global_scope, char *key, data data);

/**
 * @brief checks if a function has been called correctly
 *
 * checks if function exists and compares argument count and types
 *
 * @param current_scope PTR to current scope
 * @param key pointer to function key
 * @param arguments  AST_ARG_LIST Node
 * @return ERROR_OK on success, ERROR_INTERNAL if called with invalid scope,
 * ERROR_INTERNAL if argument array is invalid, ERROR_UNDEFINED if function
 * doesnt exist or symbol isnt a function, ERROR_FUNCTION_ARGS if argument
 * count or types
 */
ErrorCode check_function_call(scope *current_scope, char *key, ASTNode *arguments);
#endif
