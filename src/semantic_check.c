#include "semantic_check.h"
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
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

ErrorCode insert_symbol(scope *current_scope,char *key,data data)
{
    if (current_scope == NULL)
    {
        //called with invalid scope, so returning 99 
        return ERROR_INTERNAL;
    }
    if(search_table(current_scope->hash_tab,key) == true)
    {
        // returns error code, for 2nd declaration of variable within same scope
        return ERROR_REDEFINITION;
    }
    
    if(insert_item(current_scope->hash_tab,key,data) == NULL)
    {        
        // malloc failed, so returning 99
        return ERROR_INTERNAL;
    }
    return 0;
}

ErrorCode check_assign_variable(scope *current_scope, char *key,ASTDataType right_side)
{

    if (current_scope == NULL)
    {
        //called with invalid scope, so returning 99 
        return ERROR_INTERNAL;
    }
    data *temp_data = lookup_symbol(current_scope,key);
    if(temp_data == NULL)
    {
        //variable doesnt exist
        return ERROR_UNDEFINED;
    }
    if(temp_data->kind == SYMBOL_FUNCTION)
    {
        // function is unchangeable 
        return ERROR_IMMUTABLE_ASSIGNMENT;
    }
    if(temp_data->changeable == false)
    { 
        // val is unchangeable
        return ERROR_IMMUTABLE_ASSIGNMENT;
    }
    return check_type(temp_data->type,right_side);

}

ErrorCode check_type(ASTDataType left_side, ASTDataType right_side)
{
    if(left_side == right_side && left_side != AST_DATA_UNKNOWN)
        return ERROR_OK;
    if(left_side == AST_DATA_DOUBLE && right_side == AST_DATA_INT)
        return ERROR_OK;
    else
        return ERROR_TYPE_COMPATIBILITY;
}

ErrorCode check_init_variable(scope *current_scope, char *key,data data,ASTDataType right_side)
{

    if (current_scope == NULL)
    {
        //called with invalid scope, so returning 99 
        return ERROR_INTERNAL;
    }

    if(data.kind == SYMBOL_FUNCTION)
    {
        return ERROR_INTERNAL;
    }

    if(data.type != AST_DATA_UNKNOWN)
    {
        if(check_type(data.type,right_side) == ERROR_TYPE_COMPATIBILITY)
            return ERROR_TYPE_COMPATIBILITY;
    

    }
    // if type isnt declared we deduct type from right side
    else
    {
        data.type = right_side;
    }
    int insert_symbol_result = insert_symbol(current_scope,key,data);
    if( insert_symbol_result == ERROR_INTERNAL)
    {
        return ERROR_INTERNAL;
    }
    if( insert_symbol_result == ERROR_REDEFINITION)
    {
        return ERROR_REDEFINITION;
    }
    return ERROR_OK;
} 

// first run-through registering functions
ErrorCode register_function(scope *global_scope, char *key,data data)
{
    if (global_scope == NULL)
    {
        //called with invalid scope, so returning 99
        return ERROR_INTERNAL;
    }
    if(data.kind != SYMBOL_FUNCTION)
    {
        //NOT CALED WITH FUNCTION SO RETURNING 99
        return ERROR_INTERNAL;
    }
    if(data.parameter_count >0 && data.parameter_types == NULL)
        return ERROR_INTERNAL;
    return(insert_symbol(global_scope,key,data));
}
// checks if function called correctly
ErrorCode check_function_call(scope *current_scope, char *key, ASTDataType *argument_types,int argument_count)
{
    if (current_scope == NULL)
    {
        //called with invalid scope, so returning 99
        return ERROR_INTERNAL;
    }
    data *function_data = lookup_symbol(current_scope,key);
    if (function_data == NULL)
        return ERROR_UNDEFINED;
    if (function_data->kind != SYMBOL_FUNCTION)
        return ERROR_UNDEFINED;
    //comparasioning
    if( function_data->parameter_count != argument_count)
        return ERROR_FUNCTION_ARGS;
    if(argument_count >0 && argument_types == NULL)
    //caller has wrong data
        return ERROR_INTERNAL;
    for(int i = 0;i<argument_count;i++)
    {
        if(check_type(function_data->parameter_types[i],argument_types[i]) != ERROR_OK)
            return ERROR_FUNCTION_ARGS;
    }
    return ERROR_OK;
}

void free_scope(scope *current_scope)
{

    if(current_scope != NULL)
    {
        free_table(current_scope->hash_tab);
        free(current_scope);
    }
}