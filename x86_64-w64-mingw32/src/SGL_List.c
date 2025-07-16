#include "SGL_List.h"
#define INITIAL_LIST_CAPACITY 4

SGL_List* SGL_CreateList() {
    SGL_List *list = malloc(sizeof(SGL_List));
    list->size = 0;
    list->capacity = INITIAL_LIST_CAPACITY;
    list->items = malloc(sizeof(void*) * list->capacity);
    return list;
}

SGL_List* SGL_CreateListFromArray(void *array, float_safe_index_t count, float_safe_index_t element_size) {
    SGL_List *list = SGL_CreateList();
    const char *data = (const char *)array;
    for (float_safe_index_t i = 0; i < count; i++) {
        void *copy = malloc(element_size);
        memcpy(copy, data + i * element_size, element_size);
        SGL_ListAdd(list, copy);
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

void* SGL_ListGet(SGL_List *list, float_safe_index_t index) {
    if (index >= list->size) return NULL;
    return list->items[index];
}

float_safe_index_t SGL_ListIndexOf(SGL_List *list, void *item) {
    for (float_safe_index_t i = 0; i < list->size; i++) {
        if (list->items[i] == item) {
            return i;
        }
    }

    return -1;
}

void SGL_ListRemove(SGL_List *list, float_safe_index_t index, bool free_item) {
    if (index >= list->size) return;
    if (free_item) {
        free(list->items[index]);
    }
    for (float_safe_index_t i = index; i < list->size - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    list->size--;
}

void** SGL_ListToArray(SGL_List *list) {
    if (list == NULL || list->size == 0) return NULL;

    void **array = malloc(sizeof(void*) * list->size);
    if (!array) return NULL;

    for (float_safe_index_t i = 0; i < list->size; i++) {
        array[i] = list->items[i];
    }

    return array;
}

void SGL_FreeList(SGL_List *list, bool free_items) {
    if (free_items) {
        for (float_safe_index_t i = 0; i < list->size; i++) {
            free(list->items[i]);
        }
    }
    free(list->items);
    free(list);
}