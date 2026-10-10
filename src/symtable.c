#include "symtable.h"
#include <stdlib.h> //malloc

typedef struct Sdata
{
    char type;
    int position;
    bool changeable;
}data;

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
