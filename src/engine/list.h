#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include "types.h"

typedef struct list {
    uint64 len, capacity, item_size;
    void *items;
} List;

List *list_create(uint64 capacity, uint64 item_size);
uint64 list_append(List *list, void *data);
void *list_get(List *list, uint64 index);
bool list_remove(List *list, uint64 index);
void list_insert(List *list, uint64 index);
void list_delete(List *list);

#endif // !LIST_H
