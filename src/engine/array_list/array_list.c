#include <stdlib.h>
#include <strings.h>
#include "src/engine/util.h"
#include "src/engine/array_list.h"


Array_List *array_list_create(usize item_size, usize initial_capacity)
{
    Array_List *list = malloc(sizeof(Array_List));
    if (!list)
        ERROR_RETURN(NULL, "could not allocate mem for Array_List\n");

    list -> item_size = item_size;
    list -> capacity = initial_capacity;
    list -> len = 0;
    list -> items = malloc(item_size * initial_capacity);

    if(!list->items)
            ERROR_RETURN(NULL, "Could not allocate mem for Array_List\n");

    return list;
}


usize array_list_append(Array_List *list, void *item)
{
    if (list->len == list->capacity)
    {
        usize new_capacity = list->capacity ? list->capacity * 2 : 1;

        void *new_items = realloc(list->items,
                                  new_capacity * list->item_size);

        if (!new_items)
            ERROR_RETURN((usize)-1, "Realloc failed\n");

        list->items = new_items;
        list->capacity = new_capacity;
    }

    usize index = list->len++;

    memcpy((u8*)list->items + index * list->item_size,
           item,
           list->item_size);

    return index;
}

void *array_list_get(Array_List *list, usize index)
{
    if (index >= list->len)
        ERROR_RETURN(NULL, "Index out of bounds\n");

    return (u8*)list->items + index * list->item_size;
}


u8 *array_list_remove(Array_List *list, usize index)
{
    if (list->len == 0)
        ERROR_RETURN(NULL, "List Empty\n");

    if (index >= list->len)
        ERROR_RETURN(NULL, "Index out of bounds\n");

    u8 *item_ptr = (u8*)list->items + index * list->item_size;

    --list->len;

    if (index != list->len)
    {
        u8 *end_ptr = (u8*)list->items + list->len * list->item_size;
        memcpy(item_ptr, end_ptr, list->item_size);
    }

    return item_ptr;
}
