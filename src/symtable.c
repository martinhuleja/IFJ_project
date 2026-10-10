#include "symtable.h"
#include <stdlib.h> //malloc
#include <stdbool.h>
#include <stdio.h>



typedef struct Sitem
{
    char* key;
    data data;
    bool taken;
}item;

typedef struct Stable
{
    //pointer to an array, so that Stable doesnt have to be dynamic
    item* items;
    int table_capacity;
    int table_size;

}table;

table *init_table()
{
    table *hash_tab = malloc(sizeof(table));
    if (hash_tab == NULL)
    {
        fprintf(stderr,"99");
        return NULL;
    }   
    hash_tab->items = malloc(sizeof(item));
    if (hash_tab->items == NULL)
    {
        free(hash_tab);
        fprintf(stderr,"99");
        return NULL;
    }
    hash_tab->items[0].taken = false;
    hash_tab->table_capacity =1;
    hash_tab->table_size =0;


    return hash_tab;
}
