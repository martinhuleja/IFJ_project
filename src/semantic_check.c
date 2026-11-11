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
// returns NULL if called with invalid scope
// returns NULL if function didnt find symbol
// returns data
data *lookup_symbol(scope *current_scope,char *key)
{
    if (current_scope == NULL)
    {
        //called with invalid scope (99)
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
// takes current scope, key of symbol and data of symbol
// returns 99 if malloc error/called with invalid scope
// returns 4 if symbol already exists within scope
// returns 0
int insert_symbol(scope *current_scope,char *key,data data)
{
    if (current_scope == NULL)
    {
        //called with invalid scope, so returning 99 
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

// checks wether value can be assigned to varriable
// takes current scope, key of variable, and Token Type of right side (value)
// returns 3 if variable doesnt exist
// returns 7 if val
// returns 99 if called with invalid scope, or with function instaed of variable
// returns 6 if uncompabitibletypes
// returns 0
int check_assign_variable(scope *current_scope, char *key,TokenType right_side)
{

    if (current_scope == NULL)
    {
        //called with invalid scope, so returning 99 
        return 99;
    }
    data *temp_data = lookup_symbol(current_scope,key);
    if(temp_data == NULL)
    {
        //variable doesnt exist
        return 3;
    }
    if(temp_data->kind == SYMBOL_FUNCTION)
    {
        // function is unchangeable 
        return 7;
    }
    if(temp_data->changeable == false)
    { 
        // val is unchangeable
        return 7;
    }
    return check_type(temp_data->type,right_side);

}

// function checks if right and left side have compatible types
// doesnt check for invalid scopes/variables -> is only meant to be called as a helper function from other funcitons that already checked
// takes tokentype of left side and rightside
// returns 6 if uncompabitible types
// returns 0
int check_type(TokenType left_side, TokenType right_side)
{
    if(left_side == right_side)
        return 0;
    if(left_side == TOKEN_DOUBLE && right_side == TOKEN_INT)
        return 0;
    else
        return 6;
}

// checks wether variable can be initialized
// takes current scope, key of variable, data of variable, Token Type of right side (value), bool if variable has a declared type or not
// returns 4 if variable already exists within scope
// returns 99 if called with invalid scope, or with function instaed of variable or malloc error
// returns 6 if uncompatibile types or cant deduct type
// returns 0
int check_init_variable(scope *current_scope, char *key,data data,TokenType right_side, bool has_declared_type)
{

    if (current_scope == NULL)
    {
        //called with invalid scope, so returning 99 
        return 99;
    }

    if(data.kind == SYMBOL_FUNCTION)
    {
        return 99;
    }

    if(has_declared_type == true)
    {
        if(check_type(data.type,right_side) == 6)
            return 6;
    

    }
    // if type isnt declared we deduct type from right side
    else
    {
        data.type = right_side;
    }
    int insert_symbol_result = insert_symbol(current_scope,key,data);
    if( insert_symbol_result == 99)
    {
        return 99;
    }
    if( insert_symbol_result == 4)
    {
        return 4;
    }
    return 0;
} 

void free_scope(scope *current_scope)
{

    free_table(current_scope->hash_tab);
    free(current_scope); 
}