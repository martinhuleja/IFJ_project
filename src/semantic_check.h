#ifndef SEMANTIC_CHECK_H
#define SEMANTIC_CHECK_H
#include "symtable.h"

// single linked list
typedef struct Sscope scope;

// returns NULL if malloc errorr
// returns new scope
scope* new_scope(scope *current_scope);

// returns previous scope
scope* pop_scope(scope *current_scope);

// checks if symbol exists through all scopes
// returns NULL if called with invalid scope
// returns NULL if function didnt find symbol
// returns data
data *lookup_symbol(scope *current_scope,char *key);

// checks if symbol already exists in scope, then enters it into hash_table
// takes current scope, key of symbol and data of symbol
// returns 99 if malloc error/called with invalid scope
// returns 4 if symbol already exists within scope
// returns 0
int insert_symbol(scope *current_scope,char *key,data data);

void free_scope(scope *current_scope);

// checks wether value can be assigned to varriable
// takes current scope, key of variable, and Token Type of right side (value)
// returns 3 if variable doesnt exist
// returns 7 if val
// returns 99 if called with invalid scope, or with function instaed of variable
// returns 6 if uncompabitibletypes
// returns 0
int check_assign_variable(scope *current_scope, char *key,TokenType right_side);

// function checks if right and left side have compatible types
// doesnt check for invalid scopes/variables -> is only meant to be called as a helper function from other funcitons that already checked
// takes tokentype of left side and rightside
// returns 6 if uncompabite types
// returns 0 
int check_type(TokenType left_side, TokenType right_side);

// checks wether variable can be initialized
// takes current scope, key of variable, data of variable, Token Type of right side (value), bool if variable has a declared type or not
// returns 4 if variable already exists within scope
// returns 99 if called with invalid scope, or with function instaed of variable or malloc error
// returns 6 if uncompatibile types or cant deduct type
// returns 0
int check_init_variable(scope *current_scope, char *key,data data,TokenType right_side, bool has_declared_type);
#endif