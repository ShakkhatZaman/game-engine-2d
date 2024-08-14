#include "utils.h"
#include <string.h>
#include "list.h"

List *list_create(uint64 initial_capacity, uint64 item_size) {
    List *list = malloc(sizeof(List));
    if (!list) {
        ERROR_RETURN(NULL, "Unable to allocate memory for list\n");
    }
    list->item_size = item_size;
    list->capacity = initial_capacity;
    list->len = 0;
    list->items = malloc(initial_capacity * item_size);
    if (!list->items) {
        free(list);
        ERROR_RETURN(NULL, "Unable to allocate space for memory\n");
    }
    return list;
}

uint64 list_append(List *list, void *data) {
    if (list->len >= list->capacity) {
        list->capacity = (list->capacity > 0) ? list->capacity * 2 : 1;
        void *items = realloc(list->items, list->capacity * list->item_size);
        if (!items) {
            ERROR_RETURN(-1, "Unable to allocate memory to append item\n");
        }
        list->items = items;
    }
    memcpy((uint8 *)list->items + list->len * list->item_size, data, list->item_size);
    return list->len++;
}

void *list_get(List *list, uint64 index) {
    if (index >= list->len) {
        ERROR_RETURN(NULL, "List index out of bounds");
    }
    return (uint8 *)list->items + list->item_size * index;
}

bool list_remove(List *list, uint64 index) {
    if (index >= list->len) {
        ERROR_RETURN(false, "List index out of bounds\n");
    }
    if (list->len == 0) {
        ERROR_RETURN(false, "List is empty\n");
    }
    if (list->len == 1) {
        list->len = 0;
        return true;
    }
    --list->len;
    uint8 *index_ptr = (uint8 *)list->items + index * list->item_size;
    uint8 *end_ptr = (uint8 *)list->items + list->len * list->item_size;
    memcpy(index_ptr, end_ptr, list->item_size);
    return true;
}

void list_delete(List *list) {
    if (list) {
        if (list->items)
            free(list->items);
        free(list);
    }
    else {
        ERROR_EXIT_PROGRAM("Illegal pointer\n");
    }
}

void list_insert(List *list, uint64 index);
