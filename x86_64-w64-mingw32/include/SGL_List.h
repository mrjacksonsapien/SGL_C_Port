#ifndef SGL_List_h
#define SGL_List_h

#include <SDL3/SDL.h>
#include <stdlib.h>

typedef uint32_t float_safe_index_t;

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
    float_safe_index_t size;
    float_safe_index_t capacity;
} SGL_List;

SGL_List* SGL_CreateList();
/**
 * Copies all the values in the array passed.
 */
SGL_List* SGL_CreateListFromArray(void *array, float_safe_index_t count, float_safe_index_t element_size);
void SGL_ListAdd(SGL_List *list, void *item);
void* SGL_ListGet(SGL_List *list, float_safe_index_t index);
float_safe_index_t SGL_ListIndexOf(SGL_List *list, void *item);
void SGL_ListRemove(SGL_List *list, float_safe_index_t index, bool free_item);
void** SGL_ListToArray(SGL_List *list);
void SGL_FreeList(SGL_List *list, bool free_items);

#endif