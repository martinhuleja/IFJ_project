#include "symtable.h"
#include <stdlib.h> //malloc
#include <stdbool.h>



typedef struct Sitem
{
    char* key;
    data data;
}item;

typedef struct Stable
{
    item* items;
    int table_capacity;
    int table_size;

}table;

void symtable(void)
{

}
