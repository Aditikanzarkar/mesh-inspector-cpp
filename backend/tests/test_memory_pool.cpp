#include <iostream>

#include "core/MemoryPool.h"

int main() {
    MemoryPool pool(sizeof(int), 4);
    int* first = static_cast<int*>(pool.allocate());
    int* second = static_cast<int*>(pool.allocate());

    if (first == nullptr || second == nullptr) {
        std::cerr << "memory pool allocation failed\n";
        return 1;
    }

    *first = 42;
    *second = 7;
    pool.deallocate(second);

    int* third = static_cast<int*>(pool.allocate());
    if (third != second) {
        std::cerr << "memory pool recycle failed\n";
        return 1;
    }

    std::cout << "test_memory_pool passed\n";
    return 0;
}
