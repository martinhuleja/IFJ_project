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

void lookup_symbol_test(void) //zmena
{
    scope *outer_scope = new_scope(NULL);
    assert(outer_scope != NULL);

    scope *inner_scope = new_scope(outer_scope);
    assert(inner_scope != NULL);

    char *key = "test_key";
    data test_data = {0};
    test_data.kind = SYMBOL_VARIABLE; 
    test_data.type = TOKEN_INT;

    //if not present
    assert(lookup_symbol(outer_scope,key) == NULL);
    assert(lookup_symbol(inner_scope,key) == NULL);

    //only in outer
    assert(insert_symbol(outer_scope,key,test_data) == 0);
    data *result = lookup_symbol(inner_scope,key); 
    assert(result != NULL); 
    assert(result->type == TOKEN_INT); 

    //shadowing - same key different type
    test_data.type = TOKEN_STRING;
    assert(insert_symbol(inner_scope,key,test_data) == 0);

    result = lookup_symbol(inner_scope,key); 
    assert(result != NULL); 
    assert(result->type == TOKEN_STRING);

    result = lookup_symbol(outer_scope,key); 
    assert(result != NULL);
    assert(result->type == TOKEN_INT); 

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
    assert(insert_symbol(outer_scope,key,test_data) ==0);
    assert(insert_symbol(inner_scope,key,test_data) ==0);
    //duplicit item
    assert(insert_symbol(inner_scope,key,test_data) ==4);
    
    //wrong call of function
    assert(insert_symbol(NULL,key,test_data) == 99);


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
    test_data.type = TOKEN_INT;
    test_data.changeable = true;

    //calling with invalid scope
    assert(check_assign_variable(NULL,key,TOKEN_INT) == 99);
    
    //calling with variable not yet inserted
    assert(check_assign_variable(outer_scope,key,TOKEN_INT)==3);

    //calling with valid value
    assert(insert_symbol(outer_scope,key,test_data) ==0);
    assert(check_assign_variable(outer_scope,key,TOKEN_INT)==0);

    // incompatible type
    assert(check_assign_variable(outer_scope, key, TOKEN_STRING) == 6);
    assert(check_assign_variable(outer_scope, key, TOKEN_DOUBLE) == 6);

    //calling with function 
    test_data.kind = SYMBOL_FUNCTION; 
    char *key_2 = "function_test";
    assert(insert_symbol(outer_scope,key_2,test_data) ==0);
    assert(check_assign_variable(outer_scope,key_2,TOKEN_INT)==7);

    //calling with val
    test_data.kind = SYMBOL_VARIABLE; 
    test_data.changeable = false; 
    char *key_3 = "val_test";
    assert(insert_symbol(outer_scope,key_3,test_data) ==0);
    assert(check_assign_variable(outer_scope,key_3,TOKEN_INT)==7);

    // inserting int into double
    test_data.changeable = true;
    test_data.type = TOKEN_DOUBLE;
    char *key_4 = "double_test";
    assert(insert_symbol(outer_scope, key_4, test_data) == 0);
    assert(check_assign_variable(outer_scope, key_4, TOKEN_INT) == 0);
    assert(check_assign_variable(outer_scope, key_4, TOKEN_DOUBLE) == 0);
    assert(check_assign_variable(outer_scope, key_4, TOKEN_STRING) == 6);
    free_scope(outer_scope);
}

int main(void)
{
    new_scope_test();
    pop_scope_test();
    lookup_symbol_test();
    insert_symbol_test();
    check_assign_variable_test();
}