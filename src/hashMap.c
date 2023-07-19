#include "./hashMap.h"

#include <math.h>
#include <stdio.h>

// Internal - Allocate a new hashmap instance, mapSize must be within PRIME_SIZES
static HashMap* _hashMap_new(size_t mapSize) {
    HashMap* hashMap = malloc(sizeof(HashMap));
    hashMap->map = calloc(mapSize, sizeof(HashMapElement));
    hashMap->used = 0; hashMap->max = mapSize;
    #ifdef _INSPECT_hashMap
    printf("new hash map %p %li\n", hashMap, mapSize);
    #endif // _INSPECT_hashMap
    return hashMap;
}

// Destroy a hashmap instance, this only frees the internal map and elements, not the values within them
void hashMap_destroy(HashMap* hashMap) {
    #ifdef _INSPECT_hashMap
    printf("destroy mash map %p\n", hashMap);
    #endif // _INSPECT_hashMap
    free(hashMap->map);
    free(hashMap);
}

// https://planetmath.org/goodhashtableprimes
static const size_t hashMap_PRIME_SIZES[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433, 1572869};
static const size_t hashMap_PRIME_SIZES_LENGTH = 16;

// Get the next allowed map size, only primes are allowed because otherwise the probing hash might not visit all indexes
static size_t _hashMap_getAllowedSize(size_t mapSize) {
    size_t i = 0;
    while (i < hashMap_PRIME_SIZES_LENGTH && hashMap_PRIME_SIZES[i] < mapSize) i++;
    if (i < hashMap_PRIME_SIZES_LENGTH) return hashMap_PRIME_SIZES[i];
    printf("error: hash map exceeded maximum allowed size. Desired: %li Max: %li\n", mapSize, hashMap_PRIME_SIZES[i-1]);
    exit(1);
}

// Internal - Reorganise the internal map of a hashMap to be twice its current size
static void _hashMap_reorganise(HashMap* hashMap) {
    #ifdef _INSPECT_hashMap
    printf("reorganise mash map %p %li %li\n", hashMap, hashMap->used, hashMap->max);
    #endif // _INSPECT_hashMap

    // Reference the old map and allocate the new one
    size_t mapSize = hashMap->max;
    HashMapElement* map = hashMap->map;
    hashMap->used = 0; hashMap->max = _hashMap_getAllowedSize(mapSize+1);
    hashMap->map = calloc(hashMap->max, sizeof(HashMapElement));

    // Insert all the elements from the old map
    for (size_t i = 0; i < mapSize; i++) {
        hashMap_setValue(hashMap, map[i].key, map[i].value);
    }

    // Free the old map
    free(map);
}

// Create a new hash map of the minimum allowed size, use prealloc if you can estimate the number of elements required
HashMap* hashMap_new() {
    return _hashMap_new(hashMap_PRIME_SIZES[0]); // Default: Allocate the minimum allowed
}

// Create a new hash map, will allocate the internal map to beable to contain at least the amount specified
HashMap* hashMap_preAlloc(size_t mapSize) {
    return _hashMap_new(_hashMap_getAllowedSize(mapSize));
}

// Hash1 is the inital hash, Hash2 is used as the offset for probing
#define _hashMap_Hash1(k) ((size_t)abs(k*3141)) // Any largish prime is good here to produce large gaps between similar keys
#define _hashMap_Hash2(k) (1+k%37) // Must be less than hashMap_PRIME_SIZES[0] to avoid infinite loops

// Set the value of a key in a hash map, uses A La Brent hashing improvement, value can not be NULL because removal is not supported
void hashMap_setValue(HashMap* hashMap, int key, void* value) {
    // If the load factor is 95% or more, then rebuild the map
    if (100*hashMap->used/hashMap->max > 95) _hashMap_reorganise(hashMap); // Not using doubles proved to give more consistent performance

    // Get the inital index for the key
    size_t index = _hashMap_Hash1(key) % hashMap->max;
    HashMapElement* map = hashMap->map;

    // Loop until an empty element is found
    while (map[index].value && map[index].key != key) {
        int collisionKey = map[index].key;
        size_t nextIndex = (index + _hashMap_Hash2(key)) % hashMap->max;
        size_t collisionIndex = (index + _hashMap_Hash2(collisionKey)) % hashMap->max;
        if (!map[nextIndex].value || map[collisionIndex].value || map[nextIndex].key == key) {
            // An empty element was found or the collision could not be moved or this is the key we are setting
            index = nextIndex;
        } else {
            // No empty element was found but the collision can be moved
            // Move the collision to its next element to make this an empty element
            map[collisionIndex].key = collisionKey;
            map[collisionIndex].value = map[index].value;;
            break;
        }
    }

    // Increment the used counter if the key is new
    if (map[index].key != key) {
        map[index].key = key;
        hashMap->used++;
    }

    // Empty element found, insert the key and value
    map[index].value = value;
}

// Get the value at a given key in a hashmap
void* hashMap_getValue(const HashMap* hashMap, int key) {
    // Get the inital index for the key
    size_t index = _hashMap_Hash1(key) % hashMap->max;
    HashMapElement* map = hashMap->map;

    // Loop until an empty element is found
    while (map[index].value) {
        if (map[index].key == key) {
            return map[index].value;
        } else {
            index = (index + _hashMap_Hash2(key)) % hashMap->max;
        }
    }

    // No value was found
    return NULL;
}

#ifdef _INSPECT_hashMap
// Debug - Print to stdout the contents of a hash map
static void _hashMap_inspect(HashMap* hashMap) {
    printf("%p %i %i :: ", hashMap, hashMap->max, hashMap->used);
    for (int i = 0; i < hashMap->max; i++) {
        if (hashMap->map[i].value) printf("%i %i | ", i, hashMap->map[i].key);
    }
    printf("\n");
}
#endif // _INSPECT_hashMap

#ifdef _TESTS
// Test a small set of adds and removes from the queue
void _test_hashMap() {
    printf("\n_test_hashmap\n");

    HashMap* hashMap = hashMap_new();
    int ary[] = {5,3,11,7,10,3,15,5,11};

    // Insert all the keys into the map
    for (size_t i = 0; i < 9; i++) {
        printf("# Insert %li - %i\n", i, ary[i]);
        hashMap_setValue(hashMap, ary[i], (void*)(i+1));
        #ifdef _INSPECT_hashMap
        _hashMap_inspect(hashMap);
        #endif // _INSPECT_hashMap
    }

    // Get all the keys from the map
    for (int i = 0; i < 9; i++) {
        size_t value = (size_t)hashMap_getValue(hashMap, ary[i]);
        printf("# Get %i - %i %li\n", i, ary[i], value-1);
        #ifdef _INSPECT_hashMap
        _hashMap_inspect(hashMap);
        #endif // _INSPECT_hashMap
    }

    hashMap_destroy(hashMap);
}

// Test with a very large set of add and removes, multiple times
#define hashMapArraySize 200000
void _test_stress_hashMap() {
    printf("\n_test_stress_hashMap\n");

    // Generate an array of non-duplicate random numbers in a single pass
    int ary[hashMapArraySize] = {0,1};
    for (int i = 2; i < hashMapArraySize; i++) {
        int index = (abs(rand()*rand()))%i;
        ary[i] = ary[index]; ary[index] = i;
    }
    char* value = "";

    // Do 250 setups and teardowns to better time the result
    for (int i = 0; i < 250; i++) {
        if (i % 25 == 24) printf("# Iteration %i / 250\n", i+1);
        HashMap* hashMap = hashMap_new();

        for (int i = 0; i < 50000; i++) {
            hashMap_setValue(hashMap, ary[i], value);
        }

        for (int i = 0; i < 25000; i++) {
            hashMap_getValue(hashMap, ary[i]);
        }

        for (int i = 50000; i < hashMapArraySize; i++) {
            hashMap_setValue(hashMap, ary[i], value);
        }

        for (int i = 25000; i < hashMapArraySize; i++) {
            hashMap_getValue(hashMap, ary[i]);
        }

        hashMap_destroy(hashMap);
    }
}
#endif // _TESTS