#include "semantic_check.h"
#include <stddef.h> // def null
#include <assert.h> //asser
void new_scope_test()
{
    scope *outer_scope = new_scope(NULL);
    assert(outer_scope != NULL);

    scope *inner_scope = new_scope(outer_scope);
    assert(inner_scope != NULL);

    free_scope(outer_scope);
    free_scope(inner_scope);
}
void pop_scope_test()
{
    scope *outer_scope = new_scope(NULL);
    assert(outer_scope != NULL);

    scope *inner_scope = new_scope(outer_scope);
    assert(inner_scope != NULL);

    assert(pop_scope(inner_scope) == outer_scope);
    assert(pop_scope(outer_scope) == NULL);
    assert(pop_scope(NULL) == NULL);
}

void lookup_symbol_test(void) 
{
    scope *outer_scope = new_scope(NULL);
    assert(outer_scope != NULL);

    scope *inner_scope = new_scope(outer_scope);
    assert(inner_scope != NULL);

    char *key = "test_key";
    data test_data = {0};
    test_data.kind = SYMBOL_VARIABLE; 
    test_data.type = AST_DATA_INT;

    //if not present
    assert(lookup_symbol(outer_scope,key) == NULL);
    assert(lookup_symbol(inner_scope,key) == NULL);

    //only in outer
    assert(insert_symbol(outer_scope,key,test_data) == ERROR_OK);
    data *result = lookup_symbol(inner_scope,key); 
    assert(result != NULL); 
    assert(result->type == AST_DATA_INT);

    //shadowing - same key different type
    test_data.type = AST_DATA_STRING;
    assert(insert_symbol(inner_scope,key,test_data) == ERROR_OK);

    result = lookup_symbol(inner_scope,key); 
    assert(result != NULL); 
    assert(result->type == AST_DATA_STRING);

    result = lookup_symbol(outer_scope,key); 
    assert(result != NULL);
    assert(result->type == AST_DATA_INT);

    //non existing key
    assert(lookup_symbol(inner_scope,"fake") == NULL); 
    //wrong call of function
    assert(lookup_symbol(NULL,key) == NULL);


    free_scope(outer_scope);
    free_scope(inner_scope);
}
void insert_symbol_test()
{
    scope *outer_scope = new_scope(NULL);
    assert(outer_scope != NULL);

    scope *inner_scope = new_scope(outer_scope);
    assert(inner_scope != NULL);

    char *key = "test_key";
    data test_data = {0};

    //testing for if function only looks within one scope
    assert(insert_symbol(outer_scope,key,test_data) ==ERROR_OK);
    assert(insert_symbol(inner_scope,key,test_data) ==ERROR_OK);
    //duplicit item
    assert(insert_symbol(inner_scope,key,test_data) ==ERROR_REDEFINITION);
    
    //wrong call of function
    assert(insert_symbol(NULL,key,test_data) == ERROR_INTERNAL);


    free_scope(outer_scope);
    free_scope(inner_scope);
}
void check_assign_variable_test()
{
    scope *outer_scope = new_scope(NULL);
    assert(outer_scope != NULL);

    char *key = "test_key";
    data test_data = {0};
    test_data.kind = SYMBOL_VARIABLE; 
    test_data.type = AST_DATA_INT;
    test_data.changeable = true;

    //calling with invalid scope
    assert(check_assign_variable(NULL,key,AST_DATA_INT) == ERROR_INTERNAL);
    
    //calling with variable not yet inserted
    assert(check_assign_variable(outer_scope,key,AST_DATA_INT)==ERROR_UNDEFINED);

    //calling with valid value
    assert(insert_symbol(outer_scope,key,test_data) ==ERROR_OK);
    assert(check_assign_variable(outer_scope,key,AST_DATA_INT)==ERROR_OK);

    // incompatible type
    assert(check_assign_variable(outer_scope, key, AST_DATA_STRING) == ERROR_TYPE_COMPATIBILITY);
    assert(check_assign_variable(outer_scope, key, AST_DATA_DOUBLE) == ERROR_TYPE_COMPATIBILITY);

    //calling with function 
    test_data.kind = SYMBOL_FUNCTION; 
    char *key_2 = "function_test";
    assert(insert_symbol(outer_scope,key_2,test_data) ==ERROR_OK);
    assert(check_assign_variable(outer_scope,key_2,AST_DATA_INT)==ERROR_IMMUTABLE_ASSIGNMENT);

    //calling with val
    test_data.kind = SYMBOL_VARIABLE; 
    test_data.changeable = false; 
    char *key_3 = "val_test";
    assert(insert_symbol(outer_scope,key_3,test_data) ==ERROR_OK);
    assert(check_assign_variable(outer_scope,key_3,AST_DATA_INT)==ERROR_IMMUTABLE_ASSIGNMENT);

    // inserting int into double
    test_data.changeable = true;
    test_data.type = AST_DATA_DOUBLE;
    char *key_4 = "double_test";
    assert(insert_symbol(outer_scope, key_4, test_data) == ERROR_OK);
    assert(check_assign_variable(outer_scope, key_4, AST_DATA_INT) == ERROR_OK);
    assert(check_assign_variable(outer_scope, key_4, AST_DATA_DOUBLE) == ERROR_OK);
    assert(check_assign_variable(outer_scope, key_4, AST_DATA_STRING) == ERROR_TYPE_COMPATIBILITY);
    free_scope(outer_scope);
}
void check_init_variable_test(void)
{
    scope *outer_scope = new_scope(NULL);
    assert(outer_scope != NULL);

    char *key = "test_key";
    data test_data = {0};
    test_data.kind = SYMBOL_VARIABLE;
    test_data.type = AST_DATA_INT;
    test_data.changeable = true;

    ErrorCode result;
    data *stored_data;

    // calling with invalid scope
    result = check_init_variable(NULL, key, test_data, AST_DATA_INT);
    assert(result == ERROR_INTERNAL);

    // calling with function instead of variable
    test_data.kind = SYMBOL_FUNCTION;
    char *key_2 = "function_test";
    result = check_init_variable(outer_scope, key_2, test_data, AST_DATA_INT);
    assert(result == ERROR_INTERNAL);

    // valid declaration with explicit type
    test_data.kind = SYMBOL_VARIABLE;
    result = check_init_variable(outer_scope, key, test_data, AST_DATA_INT);
    assert(result == ERROR_OK);

    stored_data = lookup_symbol(outer_scope, key);
    assert(stored_data != NULL);
    assert(stored_data->type == AST_DATA_INT);

    // duplicate declaration within same scope
    result = check_init_variable(outer_scope, key, test_data, AST_DATA_INT);
    assert(result == ERROR_REDEFINITION);

    // incompatible types
    char *key_3 = "wrong_type";
    result = check_init_variable(outer_scope, key_3, test_data, AST_DATA_STRING);
    assert(result == ERROR_TYPE_COMPATIBILITY);

    // invalid declaration must not be inserted
    assert(lookup_symbol(outer_scope, key_3) == NULL);

    // int into double
    test_data.type = AST_DATA_DOUBLE;
    char *key_4 = "double_test";
    result = check_init_variable(outer_scope, key_4, test_data, AST_DATA_INT);
    assert(result == ERROR_OK);

    stored_data = lookup_symbol(outer_scope, key_4);
    assert(stored_data != NULL);
    assert(stored_data->type == AST_DATA_DOUBLE);

    // type deduction
    test_data.type = AST_DATA_UNKNOWN;
    char *key_5 = "deduct_test";
    result = check_init_variable(outer_scope, key_5, test_data, AST_DATA_STRING);
    assert(result == ERROR_OK);

    stored_data = lookup_symbol(outer_scope, key_5);
    assert(stored_data != NULL);
    assert(stored_data->type == AST_DATA_STRING);

    // val can be initialized during declaration
    test_data.changeable = false;
    test_data.type = AST_DATA_INT;
    char *key_6 = "val_test";
    result = check_init_variable(outer_scope, key_6, test_data, AST_DATA_INT);
    assert(result == ERROR_OK);

    stored_data = lookup_symbol(outer_scope, key_6);
    assert(stored_data != NULL);
    assert(stored_data->changeable == false);

    free_scope(outer_scope);
}
void register_function_test(void)
{
    scope *global_scope = new_scope(NULL);
    assert(global_scope != NULL);

    ASTDataType params[] = {AST_DATA_INT, AST_DATA_DOUBLE};

    data function_data = {0};
    function_data.kind = SYMBOL_FUNCTION;
    function_data.return_type = AST_DATA_INT;
    function_data.parameter_count = 2;
    function_data.parameter_types = params;
    function_data.defined = true;

    char *key = "test_function";

    // invalid scope
    assert(register_function(NULL, key, function_data) == ERROR_INTERNAL);

    // valid function
    assert(register_function(global_scope, key, function_data) == ERROR_OK);

    // duplicate function
    assert(register_function(global_scope, key, function_data) == ERROR_REDEFINITION);

    // variable instead of function
    char *key_2 = "test_variable";
    function_data.kind = SYMBOL_VARIABLE;
    assert(register_function(global_scope, key_2, function_data) == ERROR_INTERNAL);

    free_scope(global_scope);
}
void check_function_call_test(void)
{
    scope *global_scope = new_scope(NULL);
    assert(global_scope != NULL);

    ASTDataType params[] = {AST_DATA_INT, AST_DATA_DOUBLE};

    data function_data = {0};
    function_data.kind = SYMBOL_FUNCTION;
    function_data.return_type = AST_DATA_INT;
    function_data.parameter_count = 2;
    function_data.parameter_types = params;
    char *key = "test_function";

    assert(register_function(global_scope, key, function_data) == ERROR_OK);

    ASTDataType correct_args[] = {AST_DATA_INT, AST_DATA_DOUBLE};
    ASTDataType wrong_args[] = {AST_DATA_INT, AST_DATA_STRING};

    // correct function call
    assert(check_function_call(global_scope, key, correct_args, 2) == ERROR_OK);

    // wrong argument count
    assert(check_function_call(global_scope, key, correct_args, 1) == ERROR_FUNCTION_ARGS);

    // wrong argument type
    assert(check_function_call(global_scope, key, wrong_args, 2) == ERROR_FUNCTION_ARGS);

    // undefined function
    assert(check_function_call(global_scope, "fake", correct_args, 2) == ERROR_UNDEFINED);

    // invalid scope
    assert(check_function_call(NULL, key, correct_args, 2) == ERROR_INTERNAL);

    // variable called as function
    data variable_data = {0};
    variable_data.kind = SYMBOL_VARIABLE;
    char *key_2 = "test_variable";
    assert(insert_symbol(global_scope, key_2, variable_data) == ERROR_OK);
    assert(check_function_call(global_scope, key_2, correct_args, 2) == ERROR_UNDEFINED);

    free_scope(global_scope);
}
int main(void)
{
    new_scope_test();
    pop_scope_test();
    lookup_symbol_test();
    insert_symbol_test();
    check_assign_variable_test();
    check_init_variable_test();
    register_function_test();
    check_function_call_test();
}