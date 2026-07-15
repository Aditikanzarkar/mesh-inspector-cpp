#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace detail {

class LockFreeQueue {
public:
    LockFreeQueue() : head_(new Node()), tail_(head_.load(std::memory_order_relaxed)) {
    }

    ~LockFreeQueue() {
        std::function<void()> ignored;
        while (tryPop(ignored)) {
        }
        delete head_.load(std::memory_order_relaxed);
    }

    template <typename F>
    void push(F&& task) {
        auto* node = new Node{std::forward<F>(task), nullptr};

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
                        delete head;
                        return true;
                    }
                }
            }
        }
    }

private:
    struct Node {
        std::function<void()> task;
        std::atomic<Node*> next;

        Node() : next(nullptr) {}
        explicit Node(std::function<void()>&& taskValue, Node* nextValue)
            : task(std::move(taskValue)), next(nextValue) {
        }
    };

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
};

}  // namespace detail

class ThreadPool {
public:
    explicit ThreadPool(std::size_t threadCount = 1)
        : stop_(false), pendingTasks_(0) {
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
    detail::LockFreeQueue tasks_;
    std::atomic<bool> stop_;
    std::atomic<std::size_t> pendingTasks_;
};
