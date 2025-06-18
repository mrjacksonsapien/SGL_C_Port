#ifndef SGL_List_h
#define SGL_List_h

#include <SDL3/SDL.h>
#include <stdlib.h>

/**
 * Resizable array of pointers used for any type. Warning: Putting different types of data can work
 * but it is recommended to stick with only one (depending of the context) but if you know
 * what you're doing, feel free to mess around. I tried to be as close to the JavaScript arrays
 * who are dynamic and you can add anything you want in them (mix different types in the same array).
 * Again, be careful, you must know exactly the size of every data you put in and if using different types,
 * know the layout of the list.
 */
typedef struct {
    void **items;
    size_t size;
    size_t capacity;
} SGL_List;

SGL_List* SGL_CreateList();
/**
 * Copies all the values in the array passed.
 */
SGL_List* SGL_CreateListFromArray(void *array, size_t count, size_t element_size);
void SGL_ListAdd(SGL_List *list, void *item);
void* SGL_ListGet(SGL_List *list, size_t index);
size_t SGL_ListIndexOf(SGL_List *list, void *item);
void SGL_ListRemove(SGL_List *list, size_t index, bool free_item);
void** SGL_ListToArray(SGL_List *list);
void SGL_FreeList(SGL_List *list, bool free_items);

#endif