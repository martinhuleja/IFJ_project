#include "symtable.h"
#include <stdlib.h> //malloc
#include <stdbool.h>
#include <stdio.h>
#include <string.h> //strlen


// [DELETED ITEM] cannot be used as a key 

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
    size_t table_capacity;
    size_t table_size;

}table;
table *resize_table(table *hash_tab);

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


    // checking if key already in use 
    bool key_in_use = false;
    if(search_table(hash_tab,key) == true)
    {
        key_in_use = true;
    }

    //realloc if table is full 
    if(hash_tab->table_capacity <= hash_tab->table_size && key_in_use == false)
    {
        if(resize_table(hash_tab) == NULL)
            return NULL;
    }


    // hashing and normalizing position
    unsigned position = BKDRhash(strlen(key),key);
    position = position % hash_tab->table_capacity;

    //linear probing if duplicit item
    if(key_in_use == false)
    {
        while( hash_tab->items[position].taken == true && strcmp(hash_tab->items[position].key,"[DELETED ITEM]") != 0 )
        {
            position++;
            if (position >= hash_tab->table_capacity)
                position = position % hash_tab->table_capacity;
        }
    }
    else
    {
        while(strcmp(hash_tab->items[position].key,key) !=0)
        {

            position++;
            if (position >= hash_tab->table_capacity)
                position = position % hash_tab->table_capacity;  
        }
    }
    //linear probing if original item
    
    //allocating
    hash_tab->items[position].taken = true;
    hash_tab->items[position].data =data;
    if(key_in_use == false)
    {
        hash_tab->table_size++;
        hash_tab->items[position].key = key;

    }
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

table *resize_table(table *hash_tab)
{   
    item *new_tab;
    new_tab = malloc(sizeof(item)*(hash_tab->table_capacity)*2);
    if(new_tab == NULL)
    {
        free_table(hash_tab);
        fprintf(stderr,"99");
        return NULL;
    } 
    else
    {

        //rehash from old table -> new table
        size_t old_cap = hash_tab->table_capacity;
        size_t new_cap = hash_tab->table_capacity *2 ;
        size_t new_pos = 0;
        for(size_t i =0;i<new_cap;i++)
        {
            new_tab[i].taken = false;
        }
        for(size_t i = 0; i<old_cap;i++)
        {
            if(hash_tab->items[i].taken == true && strcmp(hash_tab->items[i].key,"[DELETED ITEM]") != 0)
            {
                new_pos = BKDRhash(strlen(hash_tab->items[i].key),hash_tab->items[i].key);
                new_pos = new_pos % new_cap;

                while( new_tab[new_pos].taken == true )
                {
                    new_pos++;
                    if (new_pos >= new_cap)
                        new_pos = new_pos % new_cap;
                }
                new_tab[new_pos].taken = true;
                new_tab[new_pos].key = hash_tab->items[i].key;
                new_tab[new_pos].data= hash_tab->items[i].data;
            }
        }

        // move to new table & init new table
        free(hash_tab->items);
        hash_tab->items = new_tab;
        hash_tab->table_capacity  = new_cap;
    }
    return hash_tab;

}

bool search_table(table *hash_tab, char* key)
{
    size_t capacity = hash_tab->table_capacity;
    size_t i = 0;
    size_t position = BKDRhash(strlen(key),key);
    position = position % capacity;

    //checking adjusted for linear probing
    if (hash_tab->items[position].taken == false )
        return false;
    else
    {
        while( hash_tab->items[position].taken == true && i < capacity) 
        {
            if(strcmp(hash_tab->items[position].key,key) == 0)
            {
                return true;
            }
            else
            {
                position++;
                i++;
                if (position >= capacity)
                    position = position % capacity;
            }
        }
    }
    return false;
}
data *get_data(table* hash_tab,char* key)
{

    size_t capacity = hash_tab->table_capacity;
    size_t i = 0;
    size_t position = BKDRhash(strlen(key),key);
    position = position % capacity;

    //checking adjusted for linear probing
    if (hash_tab->items[position].taken == false )
        return NULL;
    else
    {
        while( hash_tab->items[position].taken == true && i < capacity) 
        {
            if(strcmp(hash_tab->items[position].key,key) == 0)
            {
                return &(hash_tab->items[position].data);
            }
            else
            {
                position++;
                i++;
                if (position >= capacity)
                    position = position % capacity;
            }
        }
    }
    return NULL;
}
table *delete_item(table *hash_tab, char* key)
{

    size_t capacity = hash_tab->table_capacity;
    size_t i = 0;
    size_t position = BKDRhash(strlen(key),key);
    position = position % capacity;

    //checking adjusted for linear probing
    if (hash_tab->items[position].taken == false )
        return hash_tab;
    else
    {
        while( hash_tab->items[position].taken == true && i < capacity) 
        {
            if(strcmp(hash_tab->items[position].key,key) == 0)
            {
                hash_tab->items[position].key= "[DELETED ITEM]";
                hash_tab->table_size--;
                return hash_tab;
            }
            else
            {
                position++;
                i++;
                if (position >= capacity)
                    position = position % capacity;
            }
        }
    }
    return hash_tab;
}

void free_table(table* hash_tab)
{
    if(hash_tab != NULL)
    {
        if(hash_tab->items != NULL)
            free(hash_tab->items);
            free(hash_tab);
    }        
}
