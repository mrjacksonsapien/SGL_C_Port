#ifndef SGL_HashMap_h
#define SGL_HashMap_h

#include <SDL3/SDL.h>

#include "SGL_List.h"

typedef struct {
    void *key;
    void *value;
} SGL_MapEntry;

typedef int (*SGL_KeyEquals)(void *a, void *b);
typedef size_t (*SGL_KeyHash)(void *key);

typedef struct {
    SGL_List **buckets;
    size_t capacity;
    size_t size;
    SGL_KeyEquals equals_fn;
    SGL_KeyHash hash_fn;
} SGL_HashMap;

SGL_HashMap* SGL_CreateHashMap(SGL_KeyEquals equals_fn, SGL_KeyHash hash_fn);
void SGL_HashMapPut(SGL_HashMap *map, void *key, void *value);
void* SGL_HashMapGet(SGL_HashMap *map, void *key);
void SGL_HashMapRemove(SGL_HashMap *map, void *key);
void SGL_FreeHashMap(SGL_HashMap *map);

#endif