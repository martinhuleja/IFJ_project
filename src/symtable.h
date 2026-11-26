/**
 * @author Richard Komora (xkomorr00), FIT VUT
 * @note   IFJ + IAL project 2026/2027
 */
#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <stdbool.h>
#include "abstract-syntactic-tree.h"


typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION
} SymbolKind;
typedef enum {
    FUNCTION_UNCHECKED,
    FUNCTION_CHECKED,
    FUNCTION_IN_PROCESS
} FunctionState;
typedef struct Sdata
{
    SymbolKind kind;
    int position;
    //VARIABLE/VALUE ONLY
    ASTDataType type;
    bool changeable;
    //FUNCTION ONLY
    ASTDataType return_type;
    ASTDataType *parameter_types;
    int parameter_count;
    bool defined;
    // saving function node to deduct its type later
    ASTNode *function_node;
    // processing state of function to detect recursion
    FunctionState state;
    //caller of function to prevent cycels
    ASTNode *caller;
}data;

typedef struct Sitem item;

typedef struct Stable table;

// creates table, returns table, returns NULL if initialization fails
table *init_table();
// inserts item or replaces item, returns table, returns NULL if allocation fails
// on failure ownership remains unchanged
// on success table owns dynamic list parameter_types  funciton
// key isnt freed by the table, it must stay available  and unchanged until table is freed or item is removed
table *insert_item(table *table, char* key,data data);
// returns data of an item or NULL if item not found
// funciton can return NULL, indicating a fault
data *get_data(table *table, char* key);
// deletes a selected item from table, true if deletion sucessfull and sets item to '[DELETED ITEM]', otherwise false
bool delete_item(table *table, char* key);
// searches table, true if table contains symbol, otherwise false
bool search_table(table *table,char* key);
// BKDR hash function
unsigned int BKDRhash(int length, char* key);
// frees memory
void free_table(table *table);
#endif


