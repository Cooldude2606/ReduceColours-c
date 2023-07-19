#include "./octTree.h"
#include "./hashMap.h"
#include <stdlib.h>

OctTree octTree_new(Vector3 pos, int size) {
    OctTree tree;
    tree.pos = pos;
    tree.size = size;
    tree.children = NULL;
    tree.value = NULL;
    return tree;
}

void octTree_destroy(OctTree* tree) {
    if (tree->children) {
        for (int i = 0; i < 8; i++) octTree_destroy(tree->children+i);
        free(tree->children);
    }
}

int _octTree_getRegion(OctTree* tree, Vector3* key) {
    int rtn = 0;
    if (key->x > tree->pos.x) rtn += 1;
    if (key->y > tree->pos.y) rtn += 2;
    if (key->z > tree->pos.z) rtn += 4;
    return rtn;
}

int octTree_getChildren(OctTree* tree, OctTree* children[]) {
    if (!tree->children) return 0;
    int nextIndex = 0;
    for (int i = 0; i < 8; i++) {
        if (tree->children[i].value || tree->children[i].children)
            children[nextIndex++] = tree->children+i;
    }
    return nextIndex;
}

void _octTree_createChildren(OctTree* tree) {
    int halfSize = tree->size/2;
    Vector3 pos = tree->pos;
    tree->children = malloc(sizeof(OctTree)*8);
    tree->children[0] = octTree_new(vector3_new(pos.x-halfSize, pos.y-halfSize, pos.z-halfSize), halfSize);
    tree->children[1] = octTree_new(vector3_new(pos.x+halfSize, pos.y-halfSize, pos.z-halfSize), halfSize);
    tree->children[2] = octTree_new(vector3_new(pos.x-halfSize, pos.y+halfSize, pos.z-halfSize), halfSize);
    tree->children[3] = octTree_new(vector3_new(pos.x+halfSize, pos.y+halfSize, pos.z-halfSize), halfSize);
    tree->children[4] = octTree_new(vector3_new(pos.x-halfSize, pos.y-halfSize, pos.z+halfSize), halfSize);
    tree->children[5] = octTree_new(vector3_new(pos.x+halfSize, pos.y-halfSize, pos.z+halfSize), halfSize);
    tree->children[6] = octTree_new(vector3_new(pos.x-halfSize, pos.y+halfSize, pos.z+halfSize), halfSize);
    tree->children[7] = octTree_new(vector3_new(pos.x+halfSize, pos.y+halfSize, pos.z+halfSize), halfSize);
    #ifdef _DEBUG
    for (int i = 0; i < 8; i++) tree->children[i].parent = tree;
    #endif // _DEBUG
}

void octTree_setValue(OctTree* tree, Vector3* key, void* value) {
    if (tree->value == NULL) {
        // This node contains no key, so it can be inserted here
        tree->key = *key;
        tree->value = value;
        return;
    } else if (vector3_equals(&tree->key, key)) {
        // This node contains the key, so set the value
        tree->value = value;
        return;
    } else if (tree->children == NULL) {
        // This node has a key, but no children, so children need to be created
        _octTree_createChildren(tree);
    }
    
    if (vector3_sqDistance(&tree->pos, key) < vector3_sqDistance(&tree->pos, &tree->key)) {
        // The new key is closer to this node, so insert the current value into the best child
        int region = _octTree_getRegion(tree, &tree->key);
        octTree_setValue(tree->children+region, &tree->key, tree->value);
        tree->key = *key; tree->value = value;
    } else {
        // Insert the key into the best child
        int region = _octTree_getRegion(tree, key);
        octTree_setValue(tree->children+region, key, value);
    }    
}

void* octTree_getValue(OctTree* tree, Vector3* key) {
    if (vector3_equals(&tree->key, key)) return tree->value;
    if (tree->children == NULL) return NULL;
    int region = _octTree_getRegion(tree, key);
    return octTree_getValue(tree->children+region, key);
}

#ifdef _DEBUG
OctTree* octTree_getSubTree(OctTree* tree, Vector3* key) {
    if (vector3_equals(&tree->key, key)) return tree;
    if (tree->children == NULL) return NULL;
    int region = _octTree_getRegion(tree, key);
    return octTree_getSubTree(tree->children+region, key);
}
#endif

void** octTree_values(OctTree* tree, void* values[], int* valuesLength, int* valuesSize) {
    if (tree->value) {
        if (*valuesLength > *valuesSize) {
            *valuesSize *= 2;
            values = realloc(values, sizeof(void*)*(*valuesSize));
        }
        values[(*valuesLength)++] = tree->value;
    }
    if (tree->children) {
        for (int i = 0; i < 8; i++) values = octTree_values(tree->children+i, values, valuesLength, valuesSize);
    }
    return values;
}

void** octTree_valuesExcluding(OctTree* tree, HashMap* exclude, void* values[], int* valuesLength, int* valuesSize) {
    if (*valuesLength > 0 && hashMap_includes(exclude, octTree_pointerHash(tree))) return values;
    if (tree->value) {
        if (*valuesLength > *valuesSize) {
            *valuesSize *= 2;
            values = realloc(values, sizeof(void*)*(*valuesSize));
        }
        values[(*valuesLength)++] = tree->value;
    }
    if (tree->children) {
        for (int i = 0; i < 8; i++) values = octTree_valuesExcluding(tree->children+i, exclude, values, valuesLength, valuesSize);
    }
    return values;
}