#ifndef SGL_List_h
#define SGL_List_h

#include <SDL3/SDL.h>

/**
 * Resizable array used for any type. Warning: Putting different types of data can work
 * but it is recommended to stick with only one.
 */
typedef struct {
    void **items;
    size_t size;
    size_t capacity;
} SGL_List;

SGL_List* SGL_CreateList();
SGL_List* SGL_CreateListFromArray(void **array, size_t count);
void SGL_ListAdd(SGL_List *list, void *item);
void* SGL_ListGet(SGL_List *list, size_t index);
size_t SGL_ListIndexOf(SGL_List *list, void *item);
void SGL_ListRemove(SGL_List *list, size_t index);
void** SGL_ListToArray(SGL_List *list);
void SGL_FreeList(SGL_List *list);

#endif