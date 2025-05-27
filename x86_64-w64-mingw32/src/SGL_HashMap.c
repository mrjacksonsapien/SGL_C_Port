#include "SGL_HashMap.h"

static const size_t INITIAL_MAP_CAPACITY = 16;

SGL_HashMap* SGL_CreateHashMap(SGL_KeyEquals equals_fn, SGL_KeyHash hash_fn) {
    SGL_HashMap *map = malloc(sizeof(SGL_HashMap));
    map->capacity = INITIAL_MAP_CAPACITY;
    map->size = 0;
    map->equals_fn = equals_fn;
    map->hash_fn = hash_fn;
    map->buckets = calloc(INITIAL_MAP_CAPACITY, sizeof(SGL_List*));
    return map;
}

void SGL_HashMapResize(SGL_HashMap *map, size_t new_capacity) {
    SGL_List **old_buckets = map->buckets;
    size_t old_capacity = map->capacity;

    map->buckets = calloc(new_capacity, sizeof(SGL_List*));
    map->capacity = new_capacity;
    map->size = 0; // Will be recomputed as we re-insert

    for (size_t i = 0; i < old_capacity; ++i) {
        SGL_List *bucket = old_buckets[i];
        if (bucket != NULL) {
            for (size_t j = 0; j < bucket->size; ++j) {
                SGL_MapEntry *entry = (SGL_MapEntry*)SGL_ListGet(bucket, j);
                SGL_MapPut(map, entry->key, entry->value); // Rehash & insert
                free(entry); // Free old entry (SGL_MapPut allocates new one)
            }
            SGL_FreeList(bucket);
        }
    }

    free(old_buckets);
}

void SGL_HashMapPut(SGL_HashMap *map, void *key, void *value) {
    if ((float)map->size / map->capacity > 0.75f) { // update capacity
        SGL_MapResize(map, map->capacity * 2);
    }

    size_t index = map->hash_fn(key) % map->capacity;

    if (map->buckets[index] == NULL) {
        map->buckets[index] = SGL_CreateList();
    }

    SGL_List *bucket = map->buckets[index];

    for (size_t i = 0; i < bucket->size; i++)
    {
        SGL_MapEntry *entry = (SGL_MapEntry*)SGL_ListGet(bucket, i);
        if (map->equals_fn(entry->key, key)) {
            entry->value = value; // update existing
            return;
        }
    }
    
    // Key not found
    SGL_MapEntry *new_entry = malloc(sizeof(SGL_MapEntry));
    new_entry->key = key;
    new_entry->value = value;
    SGL_ListAdd(bucket, new_entry);
    map->size++;
}

void* SGL_HashMapGet(SGL_HashMap *map, void *key) {
    size_t index = map->hash_fn(key) % map->capacity;
    SGL_List *bucket = map->buckets[index];
    if (bucket == NULL) return NULL;

    for (size_t i = 0; i < bucket->size; i++)
    {
        SGL_MapEntry *entry = (SGL_MapEntry*)SGL_ListGet(bucket, i);
        if (map->equals_fn(entry->key, key)) {
            return entry->value;
        }
    }

    return NULL;
}

void SGL_HashMapRemove(SGL_HashMap *map, void *key) {
    size_t index = map->hash_fn(key) % map->capacity;
    SGL_List *bucket = map->buckets[index];
    if (bucket == NULL) return;

    for (size_t i = 0; i < bucket->size; i++)
    {
        SGL_MapEntry *entry = (SGL_MapEntry*)SGL_ListGet(bucket, i);
        if (map->equals_fn(entry->key, key)) {
            SGL_ListRemove(bucket, i);
            free(entry);
            map->size--;
            return;
        }
    }
}

void SGL_HashFreeMap(SGL_HashMap *map) {
    for (size_t i = 0; i < map->capacity; ++i) {
        SGL_List *bucket = map->buckets[i];
        if (bucket != NULL) {
            for (size_t j = 0; j < bucket->size; ++j) {
                SGL_MapEntry *entry = (SGL_MapEntry*)SGL_ListGet(bucket, j);
                free(entry);
            }
            SGL_FreeList(bucket);
        }
    }
    free(map->buckets);
    free(map);
}