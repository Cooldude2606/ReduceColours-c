#ifndef __H_priorityQueue
#define __H_priorityQueue

//#define _INSPECT_priorityQueue
//#define _TESTS

typedef struct PriorityQueueElement {
    int key;
    void* value;
} PriorityQueueElement;

typedef struct PriorityQueue {
    PriorityQueueElement* heap;
    int nextIndex;
    int heapSize;
} PriorityQueue;

PriorityQueue* priorityQueue_new();
PriorityQueue* priorityQueue_preAlloc(int size);
void priorityQueue_destroy(PriorityQueue* queue);

int priorityQueue_hasNext(PriorityQueue* queue);
void priorityQueue_push(PriorityQueue* queue, int key, void* value);
void* priorityQueue_pop(PriorityQueue* queue);

#ifdef _TESTS
void _test_priorityQueue();
void _test_stress_priorityQueue();
#endif // _INSPECT_priorityQueue

#endif // __H_priorityQueue