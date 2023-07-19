#ifndef __H_hashMap
#define __H_hashMap

#include <stdlib.h>

//#define _INSPECT_hashMap

// An element of the hashmap which contains a key value pair
typedef struct HashMapElement {
    int key;
    void* value;
} HashMapElement;

// The hashmap struct containing its internal map and allocation size, as well as the number of elements it contains
typedef struct HashMap {
    HashMapElement* map;
    size_t used;
    size_t max;
} HashMap;

// Value is used to represent a key is present but not valid, useful for implementing Sets where the value does not matter
#define HashMap_SetElementExists (void*)1

// Allocate a new hashmap instance, if you can estimate the number of elements to be inserted then preAlloc is more efficient
HashMap* hashMap_new();
HashMap* hashMap_preAlloc(size_t mapSize);

// Destroy a hashmap instance, this does not free the values stored only the internal map and its elements
void hashMap_destroy(HashMap* hashMap);

// Set the value of a key to a certain value; note, NULL can not be used as the value because element removal is not supported by this implementation
void hashMap_setValue(HashMap* hashMap, int key, void* value);

// Get the value of a key, if the value does not exist then NULL is returned, only use includes if you dont need the return value
void* hashMap_getValue(const HashMap* hashMap, int key);
#define hashMap_includes(hm, k) hashMap_getValue(hm, k) != NULL

#ifdef _TESTS
void _test_hashMap();
void _test_stress_hashMap();
#endif // _TESTS

#endif // __H_hashMap