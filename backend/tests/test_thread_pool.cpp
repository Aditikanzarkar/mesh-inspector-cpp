#include <future>
#include <iostream>
#include <vector>

#include "core/ThreadPool.h"

int main() {
    ThreadPool pool(2);
    std::vector<std::future<int>> futures;
    futures.reserve(8);

    for (int i = 0; i < 8; ++i) {
        futures.emplace_back(pool.enqueue([i]() { return i * i; }));
    }

    int sum = 0;
    for (std::future<int>& future : futures) {
        sum += future.get();
    }

    if (sum != 140) {
        std::cerr << "thread pool dispatch failed. expected 140 got " << sum << '\n';
        return 1;
    }

    std::cout << "test_thread_pool passed\n";
    return 0;
}
