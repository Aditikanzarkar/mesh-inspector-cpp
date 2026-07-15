#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>
#include "core/MemoryPool.h"
#include <type_traits>
#include <utility>
#include <vector>

namespace detail {

class LockFreeQueue {
public:
    struct Node {
        std::function<void()> task;
        std::atomic<Node*> next;

        Node() : next(nullptr) {}
    };

    static std::size_t nodeSize() {
        return sizeof(Node);
    }

    explicit LockFreeQueue(MemoryPool* pool = nullptr)
        : pool_(pool), head_(allocateNode()), tail_(head_.load(std::memory_order_relaxed)) {
    }

    ~LockFreeQueue() {
        std::function<void()> ignored;
        while (tryPop(ignored)) {
        }
    }

    template <typename F>
    void push(F&& task) {
        Node* node = allocateNode();
        node->task = std::forward<F>(task);
        node->next.store(nullptr, std::memory_order_relaxed);

        for (;;) {
            Node* tail = tail_.load(std::memory_order_relaxed);
            Node* next = tail->next.load(std::memory_order_acquire);
            if (tail == tail_.load(std::memory_order_relaxed)) {
                if (next == nullptr) {
                    if (tail->next.compare_exchange_weak(next, node, std::memory_order_release, std::memory_order_relaxed)) {
                        break;
                    }
                } else {
                    tail_.compare_exchange_weak(tail, next, std::memory_order_release, std::memory_order_relaxed);
                }
            }
        }

        Node* expectedTail = tail_.load(std::memory_order_relaxed);
        while (!tail_.compare_exchange_weak(expectedTail, node, std::memory_order_release, std::memory_order_relaxed)) {
            expectedTail = tail_.load(std::memory_order_relaxed);
        }
    }

    bool tryPop(std::function<void()>& outTask) {
        Node* head = head_.load(std::memory_order_relaxed);
        for (;;) {
            Node* tail = tail_.load(std::memory_order_acquire);
            Node* next = head->next.load(std::memory_order_acquire);
            if (head == head_.load(std::memory_order_relaxed)) {
                if (head == tail) {
                    if (next == nullptr) {
                        return false;
                    }
                    tail_.compare_exchange_weak(tail, next, std::memory_order_release, std::memory_order_relaxed);
                } else {
                    if (head_.compare_exchange_weak(head, next, std::memory_order_release, std::memory_order_relaxed)) {
                        outTask = std::move(next->task);
                        next->task = {};
                        return true;
                    }
                }
            }
        }
    }

private:
    Node* allocateNode() {
        if (pool_ != nullptr) {
            void* storage = pool_->allocate();
            if (storage != nullptr) {
                return new (storage) Node();
            }
        }
        return new Node();
    }

    void releaseNode(Node* node) {
        if (node == nullptr) {
            return;
        }
        if (pool_ != nullptr) {
            node->~Node();
            pool_->deallocate(node);
            return;
        }
        delete node;
    }

    MemoryPool* pool_;
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
};

}  // namespace detail

class ThreadPool {
public:
    explicit ThreadPool(std::size_t threadCount = 1)
        : stop_(false), pendingTasks_(0), taskPool_(detail::LockFreeQueue::nodeSize(), 4096) {
        const std::size_t workerCount = std::max<std::size_t>(1, threadCount);
        workers_.reserve(workerCount);
        for (std::size_t i = 0; i < workerCount; ++i) {
            workers_.emplace_back([this]() {
                for (;;) {
                    std::function<void()> task;
                    if (tasks_.tryPop(task)) {
                        task();
                        pendingTasks_.fetch_sub(1, std::memory_order_acq_rel);
                    } else if (stop_.load(std::memory_order_acquire) && pendingTasks_.load(std::memory_order_acquire) == 0) {
                        break;
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        stop_.store(true, std::memory_order_release);
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        if (stop_.load(std::memory_order_acquire)) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> result = task->get_future();
        pendingTasks_.fetch_add(1, std::memory_order_relaxed);
        tasks_.push([task]() {
            (*task)();
        });
        return result;
    }

private:
    std::vector<std::thread> workers_;
    MemoryPool taskPool_;
    detail::LockFreeQueue tasks_{&taskPool_};
    std::atomic<bool> stop_;
    std::atomic<std::size_t> pendingTasks_;
};
