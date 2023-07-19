#include "./priorityQueue.h"
#include "./hashMap.h"

int main() {
    
    _test_priorityQueue();
    _test_stress_priorityQueue();

    _test_hashMap();
    _test_stress_hashMap();

    return 0;
}