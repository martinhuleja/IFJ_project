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
    //second run
    result = second_run(global_scope,tree);
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
            tmp_data.return_type = node->declared_type;
            tmp_data.function_node = node;
            tmp_data.state = FUNCTION_UNCHECKED;
            ASTNode *params = NULL;


            //find param child
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
//cycling thru each function each function has own scope
//in function taking each node
// puting it into check depending on what typa node it is

ErrorCode second_run(scope *global_scope, ASTree *tree)
{
    ErrorCode result = ERROR_OK;
    ASTNode *root = tree->root;
    for (size_t i = 0; i < root->children_count; i++)
    {
        ASTNode *node = root->children[i];

        // calling check function
        if (node->type == AST_FUNCTION)
            result = check_function(global_scope,node);
        if (result != ERROR_OK)
            return result;
        
    }

    return ERROR_OK;
}

ErrorCode check_function(scope *global_scope,ASTNode *node)
{
    data* function_data = lookup_symbol(global_scope,node->text);
    //function already checked
    if(function_data->state == FUNCTION_CHECKED)
        return ERROR_OK;
    if (function_data->state == FUNCTION_IN_PROCESS)
    {
        //recursion happening 
        if(node->declared_type == AST_DATA_UNKNOWN)
            return ERROR_TYPE_COMPATIBILITY;
        else
            return ERROR_OK;
    }
    if(function_data->state == FUNCTION_UNCHECKED)
        function_data->state = FUNCTION_IN_PROCESS;

    ErrorCode result = ERROR_OK;

    ASTNode *params = NULL;
    ASTNode *block = NULL;

    // each function gets own scope
    scope *function_scope = new_scope(global_scope);
    if (function_scope == NULL)
        return ERROR_INTERNAL;
    
    // finding params of the function
    for ( size_t j = 0; j < node->children_count; j++)
    {
        if(node->children[j]->type == AST_PARAM_LIST)
        {
            params = node->children[j];
            break;
        }
    }
    // inserting params into local hashtable first
    if(params!= NULL)
    {
        result = insert_params(function_scope,params);
        if (result != ERROR_OK)
        {
            free_scope(function_scope);
            return result;
        }              
    }
    //find block
    for (size_t j = 0; j < node->children_count; j++)
    {
        if (node->children[j]->type == AST_BLOCK)
        {
            block = node->children[j];
            break;
        }
    }
    // going through block    


    if (block != NULL)
    {

        result = insert_block(function_scope, block,node);
        if (result != ERROR_OK)
        {
            free_scope(function_scope);
            return result;
        }
        
        //undeclared return type
        if(node->declared_type == AST_DATA_UNKNOWN)
        {
            node->resolved_type = block->resolved_type;
            // changing to its new type
            function_data->return_type = node->resolved_type;
        }

        // check if last line of function aligns with return type of function that doesnt always return
        if (block->always_returns == false && node->declared_type != AST_DATA_UNKNOWN && node->declared_type != AST_DATA_UNIT)
        {

            result = check_type(node->declared_type, block->resolved_type);
            if (result != ERROR_OK)
            {
                free_scope(function_scope);
                return result;
            }
        }
    }
    //main has to be unit
    if(node->is_main)
    {
        result = check_type(AST_DATA_UNIT,function_data->return_type);
        if (result != ERROR_OK)
        {
            free_scope(function_scope);
            return result;
        }
    }
    //unknown type and type hasnt been resolved
    if(node->declared_type == AST_DATA_UNKNOWN && node->resolved_type == AST_DATA_UNKNOWN)
    {
        free_scope(function_scope);
        return ERROR_TYPE_COMPATIBILITY;
    }
    function_data->state = FUNCTION_CHECKED;
    free_scope(function_scope);
    return ERROR_OK;
}

// inserts params from head of function into functions scope/hashtable
ErrorCode insert_params(scope *function_scope,ASTNode *params)
{
    ErrorCode result = ERROR_OK;
    int position = 0;

    for( size_t j = 0; j< params->children_count;j++)
    {
        ASTNode *param = params->children[j];
        data param_data = {0};

        param_data.position = position++;
        param_data.kind = SYMBOL_VARIABLE;
        param_data.type = param->declared_type;
        param_data.changeable = false;
        result = insert_symbol(function_scope,params->children[j]->text,param_data);
        if (result != ERROR_OK)
        {
            return result;
        }
    }
    
    return ERROR_OK;
}

// inserts body of function into functions scope/hashtable
// gets called once per funciton
ErrorCode insert_block(scope *function_scope,ASTNode *block,ASTNode *function)
{
    ErrorCode result = ERROR_OK;
    int position = 0; 
    bool return_happened = false;

    for( size_t j = 0; j< block->children_count;j++)
    {
        ASTNode *block_symbol = block->children[j];
        data block_symbol_data = {0};

        //variable declaration
        
        if (block_symbol->type == AST_VAR_DECLARATION)
        {
            //init values
            block_symbol_data.position = position++;
            block_symbol_data.kind = SYMBOL_VARIABLE;
            block_symbol_data.type = block_symbol->declared_type;   
            block_symbol_data.changeable = block_symbol->is_changeable;
        
        
            //right side gathering 
            result = calculate_allocated(function_scope,block_symbol->children[0]);
            if (result != ERROR_OK)
            {
                return result;
            }
            result = check_init_variable(function_scope,block_symbol->text,block_symbol_data,block_symbol->children[0]->resolved_type);
            if (result != ERROR_OK)
            {
                return result;
            }
        }
        if (block_symbol->type == AST_ASSIGN)
        {
        
            //right side gathering 
            result = calculate_allocated(function_scope,block_symbol->children[0]);
            if (result != ERROR_OK)
            {
                return result;
            }

            result = check_assign_variable(function_scope,block_symbol->text,block_symbol->children[0]->resolved_type);
            if (result != ERROR_OK)
            {
                return result;
            }

        }
        if (block_symbol->type == AST_CALL)
        {
            result = calculate_allocated(function_scope,block_symbol);
            if (result != ERROR_OK)
            {
                return result;
            }
        }
        if(block_symbol->type == AST_EXPR)
        {
            result = calculate_allocated(function_scope,block_symbol);
            if (result != ERROR_OK)
            {
                return result;
            }
        }
        if(block_symbol->type == AST_IF)
        {
            // if 
            result = calculate_allocated(function_scope,block_symbol->children[0]);
            if (result != ERROR_OK)
            {
                return result;
            }
            if (block_symbol->children[0]->resolved_type != AST_DATA_BOOL)
                return ERROR_TYPE_COMPATIBILITY;
            //then
            scope *inner_scope = new_scope(function_scope);
            if (inner_scope == NULL)
            {
                return ERROR_INTERNAL;
            }
            result = insert_block(inner_scope,block_symbol->children[1],function);
            if (result != ERROR_OK)
            {
                pop_scope(inner_scope);
                return result;
            }

            pop_scope(inner_scope);


            //else
            inner_scope = new_scope(function_scope);
            if (inner_scope == NULL)
            {
                return ERROR_INTERNAL;
            }
            result = insert_block(inner_scope,block_symbol->children[2],function);
            if (result != ERROR_OK)
            {
                pop_scope(inner_scope);
                return result;
            }
            pop_scope(inner_scope);
            
            if (block_symbol->children[1]->always_returns && block_symbol->children[2]->always_returns)
            {
                return_happened = true;
            }
            //if both if and else are UNIT block_symbol is unit
            if (block_symbol->children[1]->resolved_type == AST_DATA_UNIT && block_symbol->children[2]->resolved_type == AST_DATA_UNIT )
            {
                block_symbol->resolved_type = AST_DATA_UNIT;
            }
        
        }

        if(block_symbol->type == AST_WHILE)
        {
             
            //while
            result = calculate_allocated(function_scope,block_symbol->children[0]);
            if (result != ERROR_OK)
            {
                return result;
            }
            if (block_symbol->children[0]->resolved_type != AST_DATA_BOOL)
                return ERROR_TYPE_COMPATIBILITY;
            //do
            scope *inner_scope = new_scope(function_scope);
            if (inner_scope == NULL)
            {
                return ERROR_INTERNAL;
            }
            result = insert_block(inner_scope,block_symbol->children[1],function);
            if (result != ERROR_OK)
            {
                pop_scope(inner_scope);
                return result;
            }
            pop_scope(inner_scope);
        }
        // lone symbol in block
        if(block_symbol->type == AST_BINARY_OP || block_symbol->type == AST_UNARY_OP || block_symbol->type == AST_IDENTIFIER || block_symbol->type == AST_INT_LITERAL|| block_symbol->type == AST_DOUBLE_LITERAL || block_symbol->type == AST_STRING_LITERAL || block_symbol->type ==AST_UNIT_LITERAL)
        {
            result = calculate_allocated(function_scope,block_symbol);
            if (result != ERROR_OK)
            {
                return result;
            }

            // cant be a comaprasion
            if (block_symbol->resolved_type == AST_DATA_BOOL)
                return ERROR_TYPE_COMPATIBILITY;
            
        }

        if(block_symbol->type == AST_RETURN)
        {
            if (function->declared_type == AST_DATA_UNKNOWN)
                return ERROR_TYPE_COMPATIBILITY;
            //naked returm
            if(block_symbol->children_count == 0)
            {
        
                if(function->declared_type != AST_DATA_UNIT)
            
                        return ERROR_TYPE_COMPATIBILITY;

                block_symbol->resolved_type= AST_DATA_UNIT;
            }
            else
            {
                result = calculate_allocated(function_scope,block_symbol->children[0]);
                if (result != ERROR_OK)
                {
                    return result;
                }
                block_symbol->resolved_type = block_symbol->children[0]->resolved_type;

                if(block_symbol->children[0]->resolved_type == AST_DATA_BOOL)
                    return ERROR_TYPE_COMPATIBILITY;

                // check if corect:
                if(function->declared_type != AST_DATA_UNIT)
                {
                    result = check_type(function->declared_type, block_symbol->resolved_type);
                    if (result != ERROR_OK)
                    {
                        return result;
                    }
                }
            }
            return_happened = true;
        }
    }
    // end of function
    if (block->children_count == 0)
    return ERROR_INTERNAL;

    if (return_happened == false)
    {
        ASTNode *last = block->children[block->children_count - 1];
        //unit when declaration,while, assign
        if (last->type == AST_VAR_DECLARATION || last->type == AST_ASSIGN || last->type == AST_WHILE)
        {
            block->resolved_type = AST_DATA_UNIT;
        }
        else
        //keep true to type
        {
            block->resolved_type = last->resolved_type;
        }
    }

    block->always_returns = return_happened;

    return ERROR_OK;
}
// checks type of expression assigned to a variable
ErrorCode calculate_allocated(scope *current_scope, ASTNode *allocated)
{
    ErrorCode result;
    if (current_scope == NULL || allocated == NULL)
        return ERROR_INTERNAL;

    switch (allocated->type)
    {
        case AST_INT_LITERAL:
            allocated->resolved_type = AST_DATA_INT;
            break;

        case AST_DOUBLE_LITERAL:
            allocated->resolved_type = AST_DATA_DOUBLE;
            break;

        case AST_STRING_LITERAL:
            allocated->resolved_type = AST_DATA_STRING;
            break;

        case AST_UNIT_LITERAL:
            allocated->resolved_type = AST_DATA_UNIT;
            break;

        // cases such as int x = y
        case AST_IDENTIFIER:
        {
            data *symbol = lookup_symbol(current_scope, allocated->text);
            if (symbol == NULL || symbol->kind != SYMBOL_VARIABLE)
                return ERROR_UNDEFINED;

            if (symbol->type == AST_DATA_UNKNOWN || symbol->type == AST_DATA_ANY)
                return ERROR_TYPE_COMPATIBILITY;

            allocated->resolved_type = symbol->type;
            break;        
        }
        // unary operator going recursively
        case AST_UNARY_OP:
        {
            result = calculate_allocated(current_scope, allocated->children[0]);
            if (result != ERROR_OK)
                return result;

            
            if (allocated->children[0]->resolved_type != AST_DATA_INT && allocated->children[0]->resolved_type != AST_DATA_DOUBLE)
                return ERROR_TYPE_COMPATIBILITY;
            allocated->resolved_type = allocated->children[0]->resolved_type;
            break;
        }
        // binary operator going recursively
        case AST_BINARY_OP:
        {
            result = calculate_allocated(current_scope, allocated->children[0]);
            if (result != ERROR_OK)
                return result;

            result = calculate_allocated(current_scope, allocated->children[1]);
            if (result != ERROR_OK)
                return result;
            
            // figuring out final type:
            // + includes strings, others  error  
            if(allocated->operator == AST_OP_ADD)
            {


                if(allocated->children[0]->resolved_type == AST_DATA_INT)
                {
                    if(allocated->children[1]->resolved_type == AST_DATA_DOUBLE)
                        allocated->resolved_type = AST_DATA_DOUBLE; 
                    else if (allocated->children[1]->resolved_type == AST_DATA_INT)
                        allocated->resolved_type = AST_DATA_INT;
                    else 
                        return ERROR_TYPE_COMPATIBILITY;
                }
                else if(allocated->children[0]->resolved_type == AST_DATA_DOUBLE) 
                {
                    if(allocated->children[1]->resolved_type == AST_DATA_DOUBLE)
                        allocated->resolved_type = AST_DATA_DOUBLE; 
                    else if (allocated->children[1]->resolved_type == AST_DATA_INT)
                        allocated->resolved_type = AST_DATA_DOUBLE;
                    else 
                        return ERROR_TYPE_COMPATIBILITY;
                }
                else if(allocated->children[0]->resolved_type == AST_DATA_STRING) 
                {
                    if(allocated->children[1]->resolved_type == AST_DATA_STRING) 
                        allocated->resolved_type = AST_DATA_STRING;
                    else
                        return ERROR_TYPE_COMPATIBILITY;
                }
                else
                    return ERROR_TYPE_COMPATIBILITY;
            }
            // - / *  doesnt suport strings
            else if(allocated->operator == AST_OP_SUB || allocated->operator == AST_OP_DIV || allocated->operator == AST_OP_MUL)
            {
                if(allocated->children[0]->resolved_type == AST_DATA_INT)
                {
                    if(allocated->children[1]->resolved_type == AST_DATA_DOUBLE)
                        allocated->resolved_type = AST_DATA_DOUBLE; 
                    else if (allocated->children[1]->resolved_type == AST_DATA_INT)
                        allocated->resolved_type = AST_DATA_INT;
                    else 
                        return ERROR_TYPE_COMPATIBILITY;
                }
                else if(allocated->children[0]->resolved_type == AST_DATA_DOUBLE) 
                {
                    if(allocated->children[1]->resolved_type == AST_DATA_DOUBLE)
                        allocated->resolved_type = AST_DATA_DOUBLE; 
                    else if (allocated->children[1]->resolved_type == AST_DATA_INT)
                        allocated->resolved_type = AST_DATA_DOUBLE;
                    else
                        return ERROR_TYPE_COMPATIBILITY;
                }
                else
                    return ERROR_TYPE_COMPATIBILITY;
                
            }
            //comparasions (< > <= >=) doesnt  include UNIT
            if(allocated->operator == AST_OP_LESS || allocated->operator ==  AST_OP_GREATER || allocated->operator ==  AST_OP_LESS_EQUAL || allocated->operator == AST_OP_GREATER_EQUAL)
            {
                if(allocated->children[1]->resolved_type == AST_DATA_DOUBLE || allocated->children[1]->resolved_type == AST_DATA_STRING || allocated->children[1]->resolved_type == AST_DATA_INT)
                {
                    if(allocated->children[0]->resolved_type == AST_DATA_DOUBLE || allocated->children[0]->resolved_type == AST_DATA_STRING || allocated->children[0]->resolved_type == AST_DATA_INT)
                    {                    
                        if (check_type(allocated->children[0]->resolved_type,allocated->children[1]->resolved_type) != ERROR_OK && check_type(allocated->children[1]->resolved_type,allocated->children[0]->resolved_type) != ERROR_OK)
                            return ERROR_TYPE_COMPATIBILITY;
                    }
                    else
                    {
                        return ERROR_TYPE_COMPATIBILITY;
                    }
                }
                else
                {
                    return ERROR_TYPE_COMPATIBILITY;
                }
                allocated->resolved_type = AST_DATA_BOOL;
            }

            //comparasion ( == , != )
            if(allocated->operator == AST_OP_EQUAL || allocated->operator == AST_OP_NOT_EQUAL)
            {
                if(allocated->children[1]->resolved_type == AST_DATA_DOUBLE || allocated->children[1]->resolved_type == AST_DATA_STRING || allocated->children[1]->resolved_type == AST_DATA_INT || allocated->children[1]->resolved_type == AST_DATA_UNIT)
                {
                    if(allocated->children[0]->resolved_type == AST_DATA_DOUBLE || allocated->children[0]->resolved_type == AST_DATA_STRING || allocated->children[0]->resolved_type == AST_DATA_INT || allocated->children[0]->resolved_type == AST_DATA_UNIT )
                    {                    
                        if (check_type(allocated->children[0]->resolved_type,allocated->children[1]->resolved_type) != ERROR_OK && check_type(allocated->children[1]->resolved_type,allocated->children[0]->resolved_type) != ERROR_OK)
                            return ERROR_TYPE_COMPATIBILITY;
                    }
                    else
                    {
                        return ERROR_TYPE_COMPATIBILITY;
                    }
                }
                else
                {
                    return ERROR_TYPE_COMPATIBILITY;
                }
                allocated->resolved_type = AST_DATA_BOOL;

            }
            break;
        }

        case AST_CALL:
        {
            ASTNode *arguments = allocated->children[0];

            for(size_t i = 0; i<arguments->children_count;i++)
            {
                result = calculate_allocated(current_scope,arguments->children[i]);
                if (result != ERROR_OK)
                    return result;
            }

            result = check_function_call(current_scope,allocated->text,arguments);
            if (result != ERROR_OK)
                return result;

            data *function_data = lookup_symbol(current_scope, allocated->text);

            //figuring out global scope for check_function 
            scope *global_scope = current_scope;
            while (global_scope->parent != NULL)
            {
                global_scope = global_scope->parent;
            }
            // if first = second(), we have to first figure out second() return type
            if(function_data->return_type == AST_DATA_UNKNOWN)
            {
                result = check_function(global_scope,function_data->function_node);
                if (result != ERROR_OK)
                    return result;
            }
            allocated->resolved_type = function_data->return_type;
            break;

        }
        case AST_EXPR:
        {
            result = calculate_allocated(current_scope, allocated->children[0]);
            if (result != ERROR_OK)
                return result;

            allocated->resolved_type = allocated->children[0]->resolved_type;
            break;
        
        }
        default:
            return ERROR_INTERNAL;
        



    }
    return ERROR_OK;
}
// implementing built in functions first of all things
// readInt() -> I ,readDouble() -> D, readLine -> S, print(Any) -> U, toStr(Any) -> S, toInt(D) -> I
// strLength(S) -> I, subStr(S,I,I) -> S, ord   (S,I) -> I, chr(I) -> S
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

    // without extension we cant init variable with bool type
    if(right_side == AST_DATA_BOOL || right_side == AST_DATA_ANY || right_side == AST_DATA_UNKNOWN) 
    {
        return ERROR_TYPE_COMPATIBILITY;
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
ErrorCode check_function_call(scope *current_scope, char *key, ASTNode *arguments)
{
    if (current_scope == NULL || arguments == NULL)
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
    if( (size_t)function_data->parameter_count != arguments->children_count)
        return ERROR_FUNCTION_ARGS;
    for(size_t i = 0;i<arguments->children_count;i++)
    {
        if (arguments->children[i]->resolved_type == AST_DATA_UNKNOWN || arguments->children[i]->resolved_type == AST_DATA_ANY ||  arguments->children[i]->resolved_type  == AST_DATA_BOOL)
            return ERROR_FUNCTION_ARGS;

        if (function_data->parameter_types[i] == AST_DATA_ANY)
            continue;
        if(check_type(function_data->parameter_types[i],arguments->children[i]->resolved_type ) != ERROR_OK)
            return ERROR_FUNCTION_ARGS;
    }
    return ERROR_OK;
}

// allocates parameters of a function into its data
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