#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <stack>
#include <stdexcept>

class MemoryPool {
public:
    explicit MemoryPool(std::size_t blockSize, std::size_t capacity)
        : blockSize_(blockSize), capacity_(capacity), storage_(nullptr) {
        if (blockSize == 0 || capacity == 0) {
            throw std::invalid_argument("blockSize and capacity must be > 0");
        }
        storage_ = static_cast<std::byte*>(::operator new(capacity_ * blockSize_));
        for (std::size_t i = 0; i < capacity_; ++i) {
            freeList_.push(storage_ + i * blockSize_);
        }
    }

    ~MemoryPool() {
        ::operator delete(storage_);
    }

    void* allocate() {
        if (freeList_.empty()) {
            return nullptr;
        }
        void* p = freeList_.top();
        freeList_.pop();
        return p;
    }

    void deallocate(void* p) {
        if (p != nullptr) {
            freeList_.push(static_cast<std::byte*>(p));
        }
    }

private:
    std::size_t blockSize_;
    std::size_t capacity_;
    std::byte* storage_;
    std::stack<std::byte*> freeList_;
};
