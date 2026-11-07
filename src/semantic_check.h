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
// returns NULL if function called wrong
// returns NULL if function didnt find symbol
// returns data
data *lookup_symbol(scope *current_scope,char *key);

// checks if symbol already exists in scope, then enters it into hash_table
// returns 99 if malloc error/wrong call of funciton
// returns 4 if symbol already exists within scope
// returns 0
int insert_symbol(scope *current_scope,char *key,data data);

void free_scope(scope *current_scope);
#endif