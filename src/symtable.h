#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <stdbool.h>

typedef struct Sdata
{
    char type;
    int position;
    bool changeable;
}data;

typedef struct Sitem item;

typedef struct Stable table;


table *init_table();
table *insert_item(table *table, char* key,data data);
data get_data(table *table, char* key);
table *delete_item(table *table, char* key);
bool search_table(table *table,char* key);
unsigned int BKDRhash(int length, char* key);
table *resize_table(table *hash_tab);
bool search_table(table *hash_tab, char* key);
#endif


