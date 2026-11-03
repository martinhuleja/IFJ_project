#include <assert.h> //assert
#include <symtable.h>
#include <stddef.h> // def null
#include <stdbool.h>

//init table tests
void init_table_test()
{
    table *hash_tab = init_table();
    assert(hash_tab != NULL);

    free_table(hash_tab);
}
void insert_item_test()
{
    table *hash_tab = init_table();
    data hash_data;
    hash_data.changeable = false;
    hash_data.position = 1;
    hash_data.type = 'i';
    char* key = "test";
    data *tmp_data;

    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) != false);
    tmp_data = get_data(hash_tab,key);
    assert(tmp_data != NULL);
    assert(tmp_data->changeable == false);
    assert(tmp_data->position == 1 );
    assert(tmp_data->type == 'i');


    // duplicit key insert, should replace old key
    hash_data.changeable = true;
    hash_data.position = 10;
    hash_data.type ='d';
    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) != false);
    tmp_data = get_data(hash_tab,key);
    assert(tmp_data != NULL);
    assert(tmp_data->changeable == true);
    assert(tmp_data->position == 10 );
    assert(tmp_data->type == 'd');

    free_table(hash_tab);
}
void get_data_test()
{
    table *hash_tab = init_table();
    assert(hash_tab != NULL);

    data hash_data;
    hash_data.changeable = false;
    hash_data.position = 1;
    hash_data.type = 'i';
    char* key = "test";

    data *tmp_data;
    //empty table
    tmp_data = get_data(hash_tab,key);
    assert(tmp_data == NULL);


    //filling table
    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) != false);

    //non empty table
    tmp_data = get_data(hash_tab,key);
    assert(tmp_data != NULL);
    assert(tmp_data->changeable == hash_data.changeable);
    assert(tmp_data->position == hash_data.position);
    assert(tmp_data->type == hash_data.type);

    free_table(hash_tab);
}
void delete_item_test()
{
    //deleted items should have key [DELETED ITEM]
    //returns true if item found & deleted, otherwise false

    table *hash_tab = init_table();
    assert(hash_tab != NULL);

    data hash_data;
    hash_data.changeable = false;
    hash_data.position = 1;
    hash_data.type = 'i';

    char* key = "test";

    //empty talbe
    assert(delete_item(hash_tab,key) == false);

    //non empty table
    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) == true);

    assert(delete_item(hash_tab,key) == true);
    assert(search_table(hash_tab,key) == false);
    assert(get_data(hash_tab, key) == NULL);

        // inserting a deleted item
    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) == true);


    free_table(hash_tab);

    // 2 coliding items
    // with capacity 2  a and c collide because they are both uneven (97, 99) % capacity (2) = 1
    table *hash_tab_2 = init_table();
    assert(hash_tab_2 != NULL);

    hash_tab_2 = (insert_item(hash_tab_2,"a",hash_data));
    assert(hash_tab_2 != NULL);
    assert(search_table(hash_tab_2,"a") == true);

    hash_tab_2 = (insert_item(hash_tab_2,"c",hash_data));
    assert(hash_tab_2 != NULL);
    assert(search_table(hash_tab_2,"c") == true);

    assert(delete_item(hash_tab_2,"a") == true);
    assert(search_table(hash_tab_2,"a") == false);
    assert(search_table(hash_tab_2,"c") == true);

    free_table(hash_tab_2);

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

    free_table(hash_tab);
}
void resize_table_test()
{
    table *hash_tab = init_table();
    assert(hash_tab != NULL);

    data hash_data;
    hash_data.changeable = false;
    hash_data.position = 1;
    hash_data.type = 'i';
    char* key = "test";

    hash_tab = insert_item(hash_tab,key,hash_data);
    assert(hash_tab != NULL);
    assert(search_table(hash_tab,key) != false);

    //checked that 1 item works

    //creating 20 extra keys
    char keys[20][2];
    for(int i = 0;i<20;i++)
    {
        keys[i][0] = 'a' + i;
        keys[i][1] = '\0';
    }
    //triggering resize_table
    for(int i = 0;i<20;i++)
    {
        hash_data.position++;
        hash_tab = insert_item(hash_tab,keys[i],hash_data);
        assert(hash_tab != NULL);
    }
    //confirming it works
    data *temp_data;
    for(int i = 19;i>=0;i--)
    {
        assert(search_table(hash_tab,keys[i]) == true);
        temp_data = get_data(hash_tab,keys[i]);
        assert(temp_data != NULL);
        assert(temp_data->changeable == false);
        assert(temp_data->position == hash_data.position);
        assert(temp_data->type == 'i');
        hash_data.position--;

    }

    //checking the first item too
    assert(search_table(hash_tab,key) != false);

    temp_data = get_data(hash_tab,key);
    assert(temp_data != NULL);
    assert(temp_data->changeable == false);
    assert(temp_data->position == hash_data.position);
    assert(temp_data->type == 'i');

    free_table(hash_tab);
}

int main()
{

    init_table_test();
    insert_item_test();
    get_data_test();
    delete_item_test();
    search_table_test();
    resize_table_test();

}
