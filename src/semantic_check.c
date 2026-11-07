#include "semantic_check.h"
#include <stdlib.h> 
#include <stdio.h>
struct Sscope
{
    scope *parent; 
    table *hash_tab;
};


// creates new hashtab puts it into linked list and returns it
scope* new_scope(scope *current_scope)
{
    scope *new_scope = malloc(sizeof(scope));
    if (new_scope == NULL)
    {
        fprintf(stderr,"99");
        return NULL;
    }
    new_scope->parent = current_scope;
    new_scope->hash_tab = init_table();

    if (new_scope->hash_tab == NULL)
    {
        free(new_scope);
        return NULL;
    }
    return new_scope;
}

scope* pop_scope(scope *current_scope)
{
    if (current_scope == NULL)
    {
        return NULL;
    }
    scope *previous_scope = current_scope->parent; 
    free_scope(current_scope);
    return previous_scope;
}

// checks if symbol exists through all scopes
// returns NULL if function called wrong
// returns NULL if function didnt find symbol
// returns data
data *lookup_symbol(scope *current_scope,char *key)
{
    if (current_scope == NULL)
    {
        //wrong call of function (99)
        return NULL;
    }

    // looking through allscopes
    while(current_scope != NULL)
    {
        if(search_table(current_scope->hash_tab,key) == true)
        {
            //sucess
            return(get_data(current_scope->hash_tab,key));
        }
        current_scope = current_scope->parent;
    }
    //didnt find it  ( -1)
    return NULL;
}


// checks if symbol already exists in scope, then enters it into hash_table
// returns 99 if malloc error/wrong call of funciton
// returns 4 if symbol already exists within scope
// returns 0
int insert_symbol(scope *current_scope,char *key,data data)
{
    if (current_scope == NULL)
    {
        //wrong call of function, so returning 99 
        return 99;
    }
    if(search_table(current_scope->hash_tab,key) == true)
    {
        // returns error code, for 2nd declaration of variable within same scope
        return(4);
    }
    
    if(insert_item(current_scope->hash_tab,key,data) == NULL)
    {        
        // malloc failed, so returning 99
        return(99);
    }
    return 0;
}


void free_scope(scope *current_scope)
{

    free_table(current_scope->hash_tab);
    free(current_scope); 
}