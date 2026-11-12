#include "semantic_check.h"
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
struct Sscope
{
    scope *parent; 
    table *hash_tab;
};

ErrorCode semantic_analysis(ASTree *tree)
{
    ErrorCode result = ERROR_OK;
    scope *global_scope = new_scope(NULL);
    if (global_scope == NULL)
    {
        free_scope(global_scope);
        return ERROR_INTERNAL;
    }

    //built in functions
    result = built_in(global_scope);
    if (result != ERROR_OK)
    {
        free_scope(global_scope);
        return result;
    }
    // first run
    result = first_run(global_scope,tree);
    if (result != ERROR_OK)
    {
        free_scope(global_scope);
        return result;
    }



    //cleaning up
    free_scope(global_scope);

    return ERROR_OK;
}

// fu no parametre sa tu dynamicky alokuju
// data sa tu vytvoria pre tu funkciu
// prejde sa cely kod funkcie iba
// error redef if 2 mains, error undef if 0 mains, error internal if malloc fail, returns registerfunction sometimes
ErrorCode first_run(scope *global_scope,ASTree *tree)
{
    int position = 0;
    int main_found = 0;
    ErrorCode result = ERROR_OK;
    ASTNode *root = tree->root;
    for (size_t i = 0; i < root->children_count; i++)
    {
        data tmp_data = {0};

        ASTNode *node = root->children[i];

        if (node->type == AST_FUNCTION)
        {
            tmp_data.kind = SYMBOL_FUNCTION;
            tmp_data.position = position++;
            tmp_data.return_type = node->declared_type;
            ASTNode *params = NULL;


                //find params and param counts
                for (size_t j = 0; j < node->children_count; j++)
                {
                    if (node->children[j]->type == AST_PARAM_LIST)
                    {
                        params = node->children[j];
                        break;
                    }
                }

            if (params != NULL)
                tmp_data.parameter_count = (int)params->children_count;
            //checking for main
            if(node->is_main == true)
            {
                main_found++;
                if(main_found > 1)
                    return ERROR_REDEFINITION;
                //main has to have undeclared or unit
                if(node->declared_type != AST_DATA_UNKNOWN && node->declared_type != AST_DATA_UNIT)
                {
                    return ERROR_TYPE_COMPATIBILITY;
                }
                if(tmp_data.parameter_count != 0)
                    return ERROR_FUNCTION_ARGS;

            }
            //allocating params
            result = allocate_params(&tmp_data);
            if (result != ERROR_OK)
                return result;
            for (int j = 0; j < tmp_data.parameter_count; j++)
                tmp_data.parameter_types[j] = params->children[j]->declared_type;


            // registering function
            result = register_function(global_scope,node->text,tmp_data);
            if (result != ERROR_OK)
            {
                free(tmp_data.parameter_types);
                return result;

            }
        }
    }
    if(main_found == 0)
        return ERROR_UNDEFINED;
    return ERROR_OK;
}
// implementing built in functions first of all things
// readInt() -> I ,readDouble() -> D, readLine -> S, print(Any) -> U, toStr(Any) -> S, toInt(D) -> I
// strLength(S) -> I, subStr(S,I,I) -> S, ord(S,I) -> I, chr(I) -> S
ErrorCode built_in(scope *global_scope)
{
    ErrorCode result = ERROR_OK;

    //readInt() -> I
    data readInt = {0};
    readInt.kind = SYMBOL_FUNCTION;
    readInt.return_type = AST_DATA_INT;
    readInt.parameter_types = NULL;
    readInt.parameter_count = 0;

        // registering function
    result = register_function(global_scope,"readInt",readInt);
    if (result != ERROR_OK)
    {
        return result;
    }
    //readDouble() -> D
    data readDouble = {0};
    readDouble.kind = SYMBOL_FUNCTION;
    readDouble.return_type = AST_DATA_DOUBLE;
    readDouble.parameter_types = NULL;
    readDouble.parameter_count = 0;

        // registering function
    result = register_function(global_scope,"readDouble",readDouble);
    if (result != ERROR_OK)
    {
        return result;
    }
    //readLine -> S
    data readLine = {0};
    readLine.kind = SYMBOL_FUNCTION;
    readLine.return_type = AST_DATA_STRING;
    readLine.parameter_types = NULL;
    readLine.parameter_count = 0;

        // registering function
    result = register_function(global_scope,"readLine",readLine);
    if (result != ERROR_OK)
    {
        return result;
    }
    //print(Any) -> U
    data print = {0};
    print.kind = SYMBOL_FUNCTION;
    print.return_type = AST_DATA_UNIT;
    print.parameter_count = 1;

        //registering params
    result = allocate_params(&print);
    if (result != ERROR_OK)
        return result;
    print.parameter_types[0] = AST_DATA_ANY;
        // registering function
    result = register_function(global_scope,"print",print);
    if (result != ERROR_OK)
    {
        free(print.parameter_types);
        return result;
    }
    //toStr(Any) -> S
    data toStr = {0};
    toStr.kind = SYMBOL_FUNCTION;
    toStr.return_type = AST_DATA_STRING;
    toStr.parameter_count = 1;
        //registering params
    result = allocate_params(&toStr);
    if (result != ERROR_OK)
        return result;
    toStr.parameter_types[0] = AST_DATA_ANY;

        // registering function
    result = register_function(global_scope,"toStr",toStr);
    if (result != ERROR_OK)
    {
        free(toStr.parameter_types);
        return result;
    }
    //toInt(D) -> I
    data toInt = {0};
    toInt.kind = SYMBOL_FUNCTION;
    toInt.return_type = AST_DATA_INT;
    toInt.parameter_count = 1;
        //registering params
    result = allocate_params(&toInt);
    if (result != ERROR_OK)
        return result;
    toInt.parameter_types[0] = AST_DATA_DOUBLE;

        // registering function
    result = register_function(global_scope,"toInt",toInt);
    if (result != ERROR_OK)
    {
        free(toInt.parameter_types);
        return result;
    }

    //strLength(S) -> I
    data strLength = {0};
    strLength.kind = SYMBOL_FUNCTION;
    strLength.return_type = AST_DATA_INT;
    strLength.parameter_count = 1;
        //registering params
    result = allocate_params(&strLength);
    if (result != ERROR_OK)
        return result;
    strLength.parameter_types[0] = AST_DATA_STRING;
        // registering function
    result = register_function(global_scope,"strLength",strLength);
    if (result != ERROR_OK)
    {
        free(strLength.parameter_types);
        return result;
    }
    //subStr(S,I,I) -> S
    data subStr = {0};
    subStr.kind = SYMBOL_FUNCTION;
    subStr.return_type = AST_DATA_STRING;
    subStr.parameter_count = 3;
        //registering params
    result = allocate_params(&subStr);
    if (result != ERROR_OK)
        return result;
    subStr.parameter_types[0] = AST_DATA_STRING;
    subStr.parameter_types[1] = AST_DATA_INT;
    subStr.parameter_types[2] = AST_DATA_INT;

        // registering function
    result = register_function(global_scope,"subStr",subStr);
    if (result != ERROR_OK)
    {
        free(subStr.parameter_types);
        return result;
    }
    //ord(S,I) -> I
    data ord = {0};
    ord.kind = SYMBOL_FUNCTION;
    ord.return_type = AST_DATA_INT;
    ord.parameter_count = 2;
        //registering params
    result = allocate_params(&ord);
    if (result != ERROR_OK)
        return result;
    ord.parameter_types[0] = AST_DATA_STRING;
    ord.parameter_types[1] = AST_DATA_INT;

        // registering function
    result = register_function(global_scope,"ord",ord);
    if (result != ERROR_OK)
    {
        free(ord.parameter_types);
        return result;
    }
    //chr(I) -> S
    data chr = {0};
    chr.kind = SYMBOL_FUNCTION;
    chr.return_type = AST_DATA_STRING;
    chr.parameter_count = 1;
        //registering params
    result = allocate_params(&chr);
    if (result != ERROR_OK)
        return result;
     chr.parameter_types[0] = AST_DATA_INT;

        // registering function
    result = register_function(global_scope,"chr",chr);
    if (result != ERROR_OK)
    {
        free(chr.parameter_types);
        return result;
    }
    return ERROR_OK;
}
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
        if (argument_types[i] == AST_DATA_UNKNOWN || argument_types[i] == AST_DATA_ANY)
            return ERROR_FUNCTION_ARGS;

        if (function_data->parameter_types[i] == AST_DATA_ANY)
            continue;
        if(check_type(function_data->parameter_types[i],argument_types[i]) != ERROR_OK)
            return ERROR_FUNCTION_ARGS;
    }
    return ERROR_OK;
}

ErrorCode allocate_params(data *function_data)
{
    if (function_data == NULL || function_data->parameter_count < 0)
        return ERROR_INTERNAL;

    function_data->parameter_types = NULL;

    if (function_data->parameter_count == 0)
        return ERROR_OK;

    function_data->parameter_types = malloc(function_data->parameter_count * sizeof(*function_data->parameter_types));

    if (function_data->parameter_types == NULL)
        return ERROR_INTERNAL;

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