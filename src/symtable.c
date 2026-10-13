#include "symtable.h"
#include <stdlib.h> //malloc
#include <stdbool.h>
#include <stdio.h>
#include <string.h> //strlen




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
table *insert_item(table *hash_tab, char* key,data data)
{   
    item *temp;
    //realloc if table is full 
    if(hash_tab->table_capacity <= hash_tab->table_size)
    {
        temp = realloc(hash_tab->items, sizeof(item)*(hash_tab->table_capacity)*2);
        if(temp == NULL)
        {
            free(hash_tab->items);
            free(hash_tab);
            fprintf(stderr,"99");
            return NULL;
        } 
        else
        {

            hash_tab->items = temp;
            int old_cap;
            int new_cap;
            old_cap = hash_tab->table_capacity;
            new_cap = hash_tab->table_capacity *2 ;
            hash_tab->table_capacity  = new_cap;
            for(int i = old_cap; i< new_cap;i++)
            {
                hash_tab->items[i].taken = false;
            }
        }
    }

    // hashing and normalizing position
    int position = BKDRhash(strlen(key),key);
    position = position % hash_tab->table_capacity;

    while( hash_tab->items[position].taken == true )
    {
        //linear probing 
        position++;
        if (position >= hash_tab->table_capacity)
            position = position % hash_tab->table_capacity;
    }
    hash_tab->items[position].taken = true;
    hash_tab->items[position].key = key;
    hash_tab->items[position].data =data;
    hash_tab->table_size++;

    return hash_tab;
}
//BKDR HASH FUNCTION
unsigned int BKDRhash(int length, char* key)
{
    unsigned int seed = 131;
    unsigned int hash = 0;
    for(int i = 0; i<length;key++,i++)
    {
        hash = (hash*seed) + (*key);
    }
    return hash;
}
