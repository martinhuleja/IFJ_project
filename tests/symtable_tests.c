#include <assert.h> //assert
#include <symtable.h>
#include <stddef.h> // def null

//init table tests
void init_table_test()
{
    table *hash_tab = init_table();
    assert(hash_tab != NULL);
}
void insert_item_test()
{
    table *hash_tab = init_table();
    data hash_data;
    hash_data.changeable = false;
    hash_data.position = 1;
    hash_data.type = 'i';
    char* key = "test";

    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) != false);
}
void get_data_test()
{
    table *hash_tab = init_table();
    data hash_data;
    hash_data.changeable = false;
    hash_data.position = 1;
    hash_data.type = 'i';
    char* key = "test";

    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) != false);

    //TODO: get data test na empty table doplnit ked bude implementovana funkcia;


    //non empty table
    data test_data = get_data(hash_tab,key);
    assert(test_data.changeable == hash_data.changeable);
    assert(test_data.position == hash_data.position);
    assert(test_data.type == hash_data.type);
}
void delete_item_test()
{
    //TODO

}
void search_table_test()
{
    table *hash_tab = init_table();
    data hash_data;
    hash_data.changeable = false;
    hash_data.position = 1;
    hash_data.type = 'i';
    char* key = "test";

    // testing empty table 
    assert(search_table(hash_tab,key) == false);

    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) != false);

    // testing non empty table
    assert(search_table(hash_tab,key) == true);
    assert(search_table(hash_tab,"not_key") == false);


}
void resize_table_test()
{

    //TODO
}

int main()
{

    init_table_test();
    insert_item_test();
    //get_data_test();
    delete_item_test();
    search_table_test();

}
