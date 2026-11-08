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

// checks if symbol already exists in scope, if not then enters it into hash_table
// returns 99 if malloc error/called with invalid scope
// returns 4 if symbol already exists within scope
// returns 0
int insert_symbol(scope *current_scope,char *key,data data);

void free_scope(scope *current_scope);

// checks wether value can be assigned to varriable
// returns 3 if variable doesnt exist
// returns 7 if val
// returns 99 if called with invalid scope, or with function instaed of variable
// returns 0
int check_assign_variable(scope *current_scope, char *key,TokenType right_side);

// function checks if right and left side have compatible types
// doesnt check for invalid scopes/variables -> is only meant to be called as a helper function from other funcitons that already checked
// returns 6 if uncompabite types
// returns 0 
int check_type(TokenType left_side, TokenType right_side);
#endif