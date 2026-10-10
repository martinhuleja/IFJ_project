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
    table *table = malloc(sizeof(table));
    if (table == NULL)
    {
        fprintf(stderr,"99");
        return NULL;
    }   
    table->items = malloc(sizeof(item));
    if (table->items == NULL)
    {
        free(table);
        fprintf(stderr,"99");
        return NULL;
    }
    table->items[0].taken = false;
    table->table_capacity =1;
    table->table_size =0;


    return table;
}
table *insert_item(table *table, char* key,data data)
{   
    item *temp;
    if(table->table_capacity <= table->table_size)
    {
        temp = realloc(table->items, sizeof(item)*(table->table_capacity)*2);
        if(temp == NULL)
        {
            free(table->items);
            free(table);
            fprintf(stderr,"99");
            return NULL;
        } 
        else
        {
            table->items = temp;
            table->table_capacity  *= 2;
        }
    }

    table->table_size++;
    table->items[table->table_size-1].taken = true;
    table->items[table->table_size].key = key;
    table->items[table->table_size].data =data;
    return table;
}
