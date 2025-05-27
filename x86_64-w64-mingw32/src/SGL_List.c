#include "SGL_List.h"

static const size_t INITIAL_LIST_CAPACITY = 4;

SGL_List* SGL_CreateList() {
    SGL_List *list = malloc(sizeof(SGL_List));
    list->size = 0;
    list->capacity = INITIAL_LIST_CAPACITY;
    list->items = malloc(sizeof(void*) * list->capacity);
    return list;
}

SGL_List* SGL_CreateListFromArray(void **array, size_t count) {
    SGL_List *list = SGL_CreateList();
    for (size_t i = 0; i < count; i++) {
        SGL_ListAdd(list, array[i]);
    }
    return list;
}

void SGL_ListAdd(SGL_List *list, void *item) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, sizeof(void*) * list->capacity);
    }
    list->items[list->size++] = item;
}

void* SGL_ListGet(SGL_List *list, size_t index) {
    if (index >= list->size) return NULL;
    return list->items[index];
}

size_t SGL_ListIndexOf(SGL_List *list, void *item) {
    for (size_t i = 0; i < list->size; i++) {
        if (list->items[i] == item) {
            return i;
        }
    }
}

void SGL_ListRemove(SGL_List *list, size_t index) {
    if (index >= list->size) return;
    for (size_t i = index; i < list->size - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    list->size--;
}

void** SGL_ListToArray(SGL_List *list) {
    if (list == NULL || list->size == 0) return NULL;

    void **array = malloc(sizeof(void*) * list->size);
    if (!array) return NULL;

    for (size_t i = 0; i < list->size; i++) {
        array[i] = list->items[i];
    }

    return array;
}

void SGL_FreeList(SGL_List *list) {
    free(list->items);
    free(list);
}