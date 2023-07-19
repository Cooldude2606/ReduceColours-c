#include "priorityQueue.h"
#include <stdlib.h>
#include <math.h>

#ifdef _INSPECT_priorityQueue
#include <stdio.h>
#endif // _INSPECT_priorityQueue

#ifdef _TESTS
#include <stdio.h>
#endif // _TESTS

// Internal - Allocate a new priority queue instance
PriorityQueue* _priorityQueue_new(int heapSize) {
    PriorityQueue* queue = malloc(sizeof(PriorityQueue));
    queue->heap = malloc(sizeof(PriorityQueueElement)*heapSize);
    queue->nextIndex = 0; queue->heapSize = heapSize;
    #ifdef _INSPECT_priorityQueue
    printf("new priority queue %p %i\n", queue, heapSize);
    #endif // _INSPECT_priorityQueue
    return queue;
}

// Destroy a priority queue instance, all memory of its self and children are freed
void priorityQueue_destroy(PriorityQueue* queue) {
    #ifdef _INSPECT_priorityQueue
    printf("destroy priority queue %p\n", queue);
    #endif // _INSPECT_priorityQueue
    free(queue->heap);
    free(queue);
}

// Create a new priority queue, will pre allocate 64 elements to aid with performance
PriorityQueue* priorityQueue_new() {
    return _priorityQueue_new(64); // Default: Allocate 64 elements
}

// Create a new priority queue, select the amount of elements to allocate, rounded up to next power of 2 minus 1
PriorityQueue* priorityQueue_preAlloc(int size) {
    return _priorityQueue_new((int)pow(2, 1+(int)log2(size))-1);
}

// Returns true if the queue has at least one element
int priorityQueue_hasNext(PriorityQueue* queue) {
    return queue->nextIndex > 0;
}

// Push a new value into the queue with the given key / priority, greater is better
void priorityQueue_push(PriorityQueue* queue, int key, void* value) {
    // Check if the element can be inserted into this heap
    if (queue->nextIndex == queue->heapSize) {
        // There is no room in the heap, so make it bigger, keeps it aligned to (2^n)-1
        queue->heapSize = 2*(queue->heapSize+1)-1;
        queue->heap = realloc(queue->heap, sizeof(PriorityQueueElement)*queue->heapSize);
    }

    // There is room in this heap, so insert it
    PriorityQueueElement* element = queue->heap+queue->nextIndex++;
    element->key = key; element->value = value;

    // Attempt to swap the element with others to move it up the heap
    int index = queue->nextIndex-1;
    while (index > 0) {
        index = (index-1)/2;
        PriorityQueueElement* nextElement = queue->heap+index;
        if (nextElement->key > key) return; // The element can not be pushed higher

        // Swap the elements around
        *element = *nextElement;
        element = nextElement;
        element->value = value;
        element->key = key;
    }
}

// Pop the top element from the queue
void* priorityQueue_pop(PriorityQueue* queue) {
    if (queue->nextIndex == 0) return NULL;
    void* rtn = queue->heap[0].value;

    // Swap elements in layers under the current to fill the empty space
    int previousIndex = 0;
    for (int index = 1; index < queue->nextIndex; index = index*2+1) {
        if (index+1 < queue->nextIndex && queue->heap[index].key < queue->heap[index+1].key) index++;
        queue->heap[previousIndex] = queue->heap[index];
        previousIndex = index;
    }

    // Swap the last element with the empty space which was created
    int finalIndex = --queue->nextIndex;
    queue->heap[previousIndex] = queue->heap[finalIndex];

    return rtn;
}

#ifdef _INSPECT_priorityQueue
// Debug - Print to stdout the contents of a queue
void _priorityQueue_inspect(PriorityQueue* queue) {
    printf("%p %i %i :: ", queue, queue->heapSize, queue->nextIndex);
    for (int i = 0; i < queue->nextIndex; i++) {
        printf("%i | ", queue->heap[i].key);
    }
    printf("\n");
}
#endif // _INSPECT_priorityQueue

#ifdef _TESTS
// Test a small set of adds and removes from the queue
void _test_priorityQueue() {
    printf("\n_test_priorityQueue\n");

    PriorityQueue* queue = priorityQueue_preAlloc(2);
    int ary[] = {5,3,4,7,10,15,3,5,11};

    // Insert all the keys into the queue
    for (int i = 0; i < 9; i++) {
        printf("# Insert %i - %i\n", i, ary[i]);
        priorityQueue_push(queue, ary[i], ary+i);
        #ifdef _INSPECT_priorityQueue
        _priorityQueue_inspect(queue);
        #endif // _INSPECT_priorityQueue
    }

    // Remove all the keys from the queue
    for (int i = 0; i < 9; i++) {
        printf("# Remove %i - ", i);
        int* rtn = priorityQueue_pop(queue);
        printf("%i\n", *rtn);
        #ifdef _INSPECT_priorityQueue
        _priorityQueue_inspect(queue);
        #endif // _INSPECT_priorityQueue
    }

    priorityQueue_destroy(queue);
}

// Test with a very large set of add and removes, multiple times
#define priorityQueueArraySize 200000
void _test_stress_priorityQueue() {
    printf("\n_test_stress_priorityQueue\n");

    // Generate an array of non-duplicate random numbers in a single pass
    int ary[priorityQueueArraySize] = {0,1};
    for (int i = 2; i < priorityQueueArraySize; i++) {
        int index = (abs(rand()*rand()))%i;
        ary[i] = ary[index]; ary[index] = i;
    }
    char* value = "";

    // Do 250 setups and teardowns to better time the result
    for (int i = 0; i < 250; i++) {
        if (i % 25 == 24) printf("# Iteration %i / 250\n", i+1);
        PriorityQueue* queue = priorityQueue_preAlloc(2);

        for (int i = 0; i < 50000; i++) {
            priorityQueue_push(queue, ary[i], value);
        }

        for (int i = 0; i < 25000; i++) {
            priorityQueue_pop(queue);
        }

        for (int i = 50000; i < priorityQueueArraySize; i++) {
            priorityQueue_push(queue, ary[i], value);
        }

        for (int i = 25000; i < priorityQueueArraySize; i++) {
            priorityQueue_pop(queue);
        }

        priorityQueue_destroy(queue);
    }
}
#endif // _TESTS