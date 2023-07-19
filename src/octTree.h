#ifndef __H_octTree
#define __H_octTree

#include "./vector3.h"
#include "./hashMap.h"
#include <limits.h>

//#define _INSPECT_octTree
//#define _TESTS

typedef struct OctTree {
    #ifdef _DEBUG
    struct OctTree* parent;
    #endif // _DEBUG
    struct OctTree* children;
    void* value;
    Vector3 key;
    Vector3 pos;
    int size;
} OctTree;

OctTree octTree_new(Vector3 pos, int size);
void octTree_destroy(OctTree* tree);

int octTree_getChildren(OctTree* tree, OctTree* children[]);

void octTree_setValue(OctTree* tree, Vector3* key, void* value);
void* octTree_getValue(OctTree* tree, Vector3* key);

void** octTree_values(OctTree* tree, void* values[], int* valuesLength, int* valuesSize);
void** octTree_valuesExcluding(OctTree* tree, HashMap* exclude, void* values[], int* valuesLength, int* valueSize);

#define octTree_pointerHash(p) (int)((size_t)p&INT_MAX)

#ifdef _DEBUG
OctTree* octTree_getSubTree(OctTree* tree, Vector3* key);
#endif // _DEBUG

#endif // __H_octTree