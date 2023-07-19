#include "priorityQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Internal - Allocate a new priority queue instance
PriorityQueue* _priorityQueue_new(int heapSize) {
    PriorityQueue* queue = malloc(sizeof(PriorityQueue)+(sizeof(PriorityQueueElement)*heapSize));
    queue->nextIndex = 0; queue->heapSize = heapSize;
    queue->prev = NULL; queue->next = NULL;
    queue->heap = (PriorityQueueElement*)(queue+1);
    #ifdef _INSPECT_priorityQueue
    printf("new priority queue %p %i\n", queue, heapSize);
    #endif // _INSPECT_priorityQueue
    return queue;
}

// Destroy a priority queue instance, all memory of its self and children are freed
void priorityQueue_destroy(PriorityQueue* queue) {
    if (queue->next) priorityQueue_destroy(queue->next);
    #ifdef _INSPECT_priorityQueue
    printf("destroy priority queue %p\n", queue);
    #endif // _INSPECT_priorityQueue
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
};

// Internal - Allocate a new priority queue instance and parent it to the provided queue
void _priorityQueue_createChild(PriorityQueue* queue) {
    PriorityQueue* child = _priorityQueue_new(queue->prev ? queue->heapSize*2 : queue->heapSize+1);
    child->prev = queue;
    queue->next = child;
}

// Push a new value into the queue with the given key / priority, greater is better
void priorityQueue_push(PriorityQueue* queue, int key, void* value) {
    // Check if the element can be inserted into this heap
    if (queue->nextIndex == queue->heapSize) {
        // There is no room in the heap, so it must go into the next heap
        if (!queue->next) _priorityQueue_createChild(queue);
        return priorityQueue_push(queue->next, key, value);        
    }

    // There is room in this heap, so insert it
    PriorityQueueElement* element = queue->heap+queue->nextIndex++;
    element->key = key; element->value = value;

    // Attempt to swap the element into a higher heap layer
    PriorityQueue* rootLayer = queue;
    PriorityQueue* nextLayer = queue->prev;
    int currentIndex = queue->nextIndex-1;
    while (nextLayer) {
        currentIndex = (currentIndex)/2;
        if (!nextLayer->prev) currentIndex += nextLayer->heapSize/2;
        PriorityQueueElement* nextElement = nextLayer->heap+currentIndex;
        if (nextElement->key > key) return; // The element can not be pushed higher

        // Swap the elements around
        element->key = nextElement->key; element->value = nextElement->value;
        nextElement->key = key; nextElement->value = value;

        // Iterate the variables
        element = nextElement;
        rootLayer = nextLayer; 
        nextLayer = nextLayer->prev;
    }

    // Assert the root layer is actually the top layer, ensured by the while condition
    if (rootLayer->prev) {
        printf("FATAL: Assert failed, root layer of priority queue heap is not the top layer");
        exit(1);
    }

    // Attempt to swap the element with elements in the root layer
    while (currentIndex > 0) {
        currentIndex = (currentIndex-1)/2;
        PriorityQueueElement* nextElement = rootLayer->heap+currentIndex;
        if (nextElement->key > key) return; // The element can not be pushed higher

        // Swap the elements around
        element->key = nextElement->key; element->value = nextElement->value;
        nextElement->key = key; nextElement->value = value;

        // Iterate the variables
        element = nextElement;
    }
};

// Pop the top element from the queue
void* priorityQueue_pop(PriorityQueue* queue) {
    if (queue->prev) return priorityQueue_pop(queue->prev);
    if (queue->nextIndex == 0) return NULL;
    void* rtn = queue->heap[0].value;

    // Assert this layer is actually the top layer, ensured by the recursive calls
    if (queue->prev) {
        printf("FATAL: Assert failed, root layer of priority queue heap is not the top layer");
        exit(1);
    }

    // Swap elements within the current layer to find the new largest element
    int previousIndex = 0;
    for (int index = 1; index < queue->nextIndex; index = index*2+1) {
        if (index+1 < queue->nextIndex && queue->heap[index].key < queue->heap[index+1].key) index++;
        queue->heap[previousIndex].value = queue->heap[index].value;
        queue->heap[previousIndex].key = queue->heap[index].key;
        previousIndex = index;
    }

    // Swap to deeper layers to maintain heap integrity
    PriorityQueue* prevLayer = queue;
    PriorityQueue* nextLayer = queue->next;
    if (nextLayer && previousIndex >= queue->heapSize/2) {
        for (int index = (previousIndex-queue->heapSize/2)*2; index < nextLayer->nextIndex; index *= 2) {
            if (index+1 < nextLayer->nextIndex && nextLayer->heap[index].key < nextLayer->heap[index+1].key) index++;
            prevLayer->heap[previousIndex].value = nextLayer->heap[index].value;
            prevLayer->heap[previousIndex].key = nextLayer->heap[index].key;
            previousIndex = index;
            prevLayer = nextLayer;
            nextLayer = nextLayer->next;
            if (!nextLayer) break;
        }
    }

    // Swap the last element with the empty space which was created
    int finalIndex = --prevLayer->nextIndex;
    prevLayer->heap[previousIndex].value = prevLayer->heap[finalIndex].value;
    prevLayer->heap[previousIndex].key = prevLayer->heap[finalIndex].key;

    return rtn;
}

#ifdef _INSPECT_priorityQueue
// Debug - Print to stdout the contents of a queue
void _priorityQueue_inspect(PriorityQueue* queue) {
    printf("%p %i %i %i :: ", queue, queue->heapSize, queue->nextIndex, queue->next != NULL);
    for (int i = 0; i < queue->nextIndex; i++) {
        printf("%i '%s' | ", queue->heap[i].key, (char*)queue->heap[i].value);
    }
    printf("\n");
    for (PriorityQueue* next = queue->next; next != NULL; next = next->next) {
        printf("> %p %i %i %i :: ", next, next->heapSize, next->nextIndex, next->next != NULL);
        for (int i = 0; i < next->nextIndex; i++) {
            printf("%i '%s' | ", next->heap[i].key, (char*)next->heap[i].value);
        }
        printf("\n");
    }
    printf("\n");
}
#endif // _INSPECT_priorityQueue

#ifdef _TESTS
void _test_priorityQueue() {
    printf("_test_priorityQueue\n");

    PriorityQueue* queue = priorityQueue_preAlloc(2);
    int ary[] = {5,3,4,7,10,15,3,5,11};
    for (int i = 0; i < 9; i++) {
        printf("## Insert %i - %i\n", i, ary[i]);
        priorityQueue_push(queue, ary[i], "");
        #ifdef _INSPECT_priorityQueue
        _priorityQueue_inspect(queue);
        #endif // _INSPECT_priorityQueue
    }

    for (int i = 0; i < 9; i++) {
        printf("## Remove %i\n", i);
        priorityQueue_pop(queue);
        #ifdef _INSPECT_priorityQueue
        _priorityQueue_inspect(queue);
        #endif // _INSPECT_priorityQueue
    }

    priorityQueue_destroy(queue);
}

#define priorityQueueArraySize 200000
void _test_stress_priorityQueue() {
    printf("_test_stress_priorityQueue\n");

    int ary[priorityQueueArraySize];
    for (int i = 0; i < priorityQueueArraySize; i++) ary[i] = i;
    for (int i = 0; i < priorityQueueArraySize; i++) ary[i] = ary[(int)pow(1.1,rand()*rand())%priorityQueueArraySize];
    char* value = "";

    for (int i = 0; i < 250; i++) {
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