#include "./priorityQueue.h"
#include "./hashMap.h"
#include "./octTree.h"
#include "./colour3.h"
#include "./vector3.h"
#include "./images.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct Node {
    Colour3 color;
    Colour3* replacement;
    int frequency;
    int recursiveFrequency;
    #ifdef _DEBUG
    OctTree* tree;
    #endif // _DEBUG
} Node;

void scanImage(Image image, HashMap* colours, OctTree* tree) {
    for (unsigned i = 0; i < image.height*image.width; i++) {
        Colour3 color = colour3_fromBuffer(image.buffer, i*4);
        int key = colour3_hash(color);

        Node* node = hashMap_getValue(colours, key);
        if (node) {
            node->frequency++;
        } else {
            node = malloc(sizeof(Node));
            node->color = color; node->replacement = NULL;
            node->frequency = 1; node->recursiveFrequency = 0;
            Vector3 vec3 = colour3_toVector3(color);
            hashMap_setValue(colours, key, node);
            octTree_setValue(tree, &vec3, node);
            #ifdef _DEBUG
            node->tree = octTree_getSubTree(tree, &vec3);
            #endif
        }
    }   
}

int recursiveFrequency(OctTree* tree) {
    if (!tree->value) return 0;
    Node* node = tree->value;
    if (node->recursiveFrequency) return node->recursiveFrequency;

    int frequency = 0, valuesLength = 0, valuesSize = 256;
    Node** values = malloc(sizeof(Node*)*256);
    values = (Node**) octTree_values(tree, (void**)values, &valuesLength, &valuesSize);
    for (int i = 0; i < valuesLength; i++) {
        frequency += values[i]->frequency;
    }

    node->recursiveFrequency = frequency;
    return frequency;
}

void selectNodes(OctTree* tree, OctTree* selected[], int* selectedLength, int selectedSize) {
    PriorityQueue* queue = priorityQueue_preAlloc(selectedSize*8);
    priorityQueue_push(queue, recursiveFrequency(tree), tree);

    OctTree* children[8];
    int childrenLength = 0;
    while (*selectedLength < selectedSize && priorityQueue_hasNext(queue)) {
        OctTree* nextTree = priorityQueue_pop(queue);
        childrenLength = octTree_getChildren(nextTree, children);
        for (int i = 0; i < childrenLength; i++) {
            priorityQueue_push(queue, recursiveFrequency(children[i]), children[i]);
        }
        selected[(*selectedLength)++] = nextTree;
    }
}

#ifndef main
#define PALLET_SCALE 4

int desiredColourSortCmp(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

int main(int argc, char** argv) {
    // Check that exactly two arguments were given
    if (argc != 3) {
        printf("error: wrong number of arguments, 2 expect got %i\n", argc);
        return 1;
    }

    int maxDesired = 0;
    int desiredColours[16];
    int desiredColoursLength = 0;
    // Split the second argument into a list of ints
    char* token = strtok(argv[2], ",");
    while (token != NULL && desiredColoursLength < 16) {
        int value = atoi(token);
        desiredColours[desiredColoursLength++] = value;
        if (value <= 0) {
            printf("error invalid argument 2: must be integer greater than 0, got: '%s'\n", token);
            return 1;
        } else if (value > maxDesired) {
            maxDesired = value; // New maximum value
        }
        token = strtok(NULL, ",");
    }

    // If token is not null then we ran out of room in the colours array
    if (token) {
        printf("error invalid argument 2: can only contain a maximum of 16 different reductions at once\n");
        return 1;
    }

    char* inputPath = argv[1];
    // Get the file type so we can read in the image correctly
    Image image = readImage(inputPath);
    if (!image.bufferSize) {
        printf("error invalid argument 1: file type can not be decoded, must be one of: png, jpeg, jpg\n");
        return 1;
    }

    /*
        All arguments have been validated beyond this section
    */

    // Allocate space for the colour map and colour tree
    HashMap* colours = hashMap_preAlloc(image.width*image.height/9);
    OctTree tree = octTree_new(vector3_new(128, 128, 128), 128);

    // Create a pallet with an internal buffer large enough for largest pallet
    int maxPalletSize = PALLET_SCALE*(int)ceil(sqrt(maxDesired));
    Image pallet = newImage(maxPalletSize, maxPalletSize);

    // Copy the input path and image so it can be modified
    char outputPath[256];
    strcpy(outputPath, inputPath);
    Image output = newImage(image.height, image.width);
    char* outputFileExtension = strrchr(outputPath, '.');

    // Scan the image for all colours, inserting them into the map and tree
    scanImage(image, colours, &tree);
    printf("Read %ix%i pixels containing %li unique colours\n", image.height, image.width, colours->used);

    // Select the nodes to be used, these nodes will later be used to generate the pallet
    int selectedLength = 0;
    HashMap* excludeMap = hashMap_preAlloc(maxDesired);
    OctTree** selected = malloc(sizeof(OctTree*)*maxDesired);
    selectNodes(&tree, selected, &selectedLength, maxDesired);

    // For optimisation reasons, allocate values outside the loops and sort the desired colours array
    int previousDesired = 0;
    int valuesLength = 0, valuesSize = 256;
    Node** values = malloc(sizeof(Node*)*256);
    qsort(desiredColours, desiredColoursLength, sizeof(int), desiredColourSortCmp);
    for (int desiredColourIndex = 0; desiredColourIndex < desiredColoursLength; desiredColourIndex++) {
        int desired = desiredColours[desiredColourIndex];
        if (desired > selectedLength) desired = selectedLength; // Only trigged if desired is greater than the total number of colours in the image

        // Create a hash map set of those selected nodes, using a set here greatly improves performance
        for (int i = previousDesired; i < desired; i++) hashMap_setValue(excludeMap, octTree_pointerHash(selected[i]), HashMap_SetElementExists);

        // Create an image for the pallet output
        int palletSize = PALLET_SCALE*(int)ceil(sqrt(desired));
        resizeImage(&pallet, palletSize, palletSize);

        // Find the replacement colour for every node
        for (int index = 0; index < desired; index++) {
            OctTree* next = selected[index];
            Node* node = next->value;

            // Allocate the replacement colour
            if (index >= previousDesired)
                node->replacement = malloc(sizeof(Colour3));
            Colour3* replacement = node->replacement;

            // Initiate the count and sum from the root node
            int count = node->frequency;
            Vector3 sum = colour3_toVector3(node->color);
            vector3_scale(&sum, count);

            // For all descendants, set their replacement colour and add their frequencies
            valuesLength = 0;
            values = (Node**) octTree_valuesExcluding(next, excludeMap, (void**)values, &valuesLength, &valuesSize);
            for (int i = 0; i < valuesLength; i++) {
                Node* childNode = values[i];
                childNode->replacement = replacement;
                Vector3 vec3 = colour3_toVector3(childNode->color);
                vector3_add_scaled(&sum, &vec3, childNode->frequency);
                count += childNode->frequency;
            }

            // Calculate the weighted average
            vector3_divide(&sum, count);
            Colour3 colour = colour3_fromVector3(sum);
            *replacement = colour;

            // Add the colour to the pallet image
            int row = PALLET_SCALE*((index*PALLET_SCALE) / pallet.width);
            int column = (index*PALLET_SCALE) % pallet.width;
            for (int ri = 0; ri < PALLET_SCALE; ri++) for (int ci = 0; ci < PALLET_SCALE; ci++) {
                int idx = ((row+ri)*pallet.width+column+ci)*4;
                colour3_toBufferWithAlpha(colour, pallet.buffer, idx, 255);
            }
        }

        // For each pixel in the input, copy the replacement colour into the output
        for (unsigned i = 0; i < image.height*image.width; i++) {
            Colour3 color = colour3_fromBuffer(image.buffer, i*4);
            Node* node = hashMap_getValue(colours, colour3_hash(color));
            
            Colour3 replacement = *node->replacement;
            colour3_toBufferWithAlpha(replacement, output.buffer, i*4, 255);
        }

        // Edit the file name to end in "_reduced" followed by the colour count
        // desiredColours[desiredColourIndex] is used in case selectedLength is smaller
        sprintf(outputFileExtension, "_reduced_%i.png", desiredColours[desiredColourIndex]);
        writeImage(output, outputPath);
        printf("Wrote %s\n", outputPath);

        // Edit the file name to end in "_pallet" followed by the colour count
        sprintf(outputFileExtension, "_pallet_%i.png", desiredColours[desiredColourIndex]);
        writeImage(pallet, outputPath);
        printf("Wrote %s\n", outputPath);

        previousDesired = desiredColours[desiredColourIndex];
    }

    return 0;
}
#endif // main