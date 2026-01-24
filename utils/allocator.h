#pragma once
#define STD_ARRAY 0

#if STD_ARRAY
#include "array_std_interface.h"
#else
#include "array.h"
#endif

#include "memory.h"

struct Allocator {
    virtual ~Allocator() = default;

    virtual inline byte* allocate_bytes(size_t bytes) {
        assert(0 && "Not implemented");
        return nullptr;
    }

    virtual inline byte* reallocate_bytes(byte* ptr, size_t size) {
        assert(0 && "Not implemented");
        return nullptr;
    }

    virtual inline void align(size_t size) {}

    struct Frame;
    virtual inline void end_frame(Frame*) {}

    struct Frame {
        Allocator* allocator;
        size_t     start;
        ~Frame() { allocator->end_frame(this); }
    };
    virtual inline Frame begin_frame() {};

    template <typename T>
    inline void align_for() {
        align(alignof(T));
    }

    template <typename Type>
    inline Type& allocate() {
        align_for<Type>();
        return *(Type*)allocate_bytes(sizeof(Type));
    }

    template <typename Type>
    inline array<Type> allocate(size_t count) {
        size_t      bytes = sizeof(Type) * count;
        array<Type> result;
        result.count = count;
        align_for<Type>();
        result.data = (Type*)allocate_bytes(bytes);
        return result;
    }

    template <typename Type>
    inline void resize(array<Type>& arr, size_t count) {
        arr.count = count;
        arr.data  = reallocate_bytes(arr.data, count);
    }

    template <typename Type>
    inline array<Type> allocate(size_t count, const Type& value) {
        auto result = allocate<Type>(count);
        for (size_t i = 0; i < count; i++) {
            result[i] = value;
        }
        return result;
    }

    template <typename Type>
    inline array<Type> allocate(const std::initializer_list<Type>& list) {
        auto   result = allocate<Type>((size_t)list.size());
        size_t i      = 0;
        for (auto& v : list) result[i++] = v;
        return result;
    }

    // template <typename Type>
    // inline hash_set<Type> allocate_set(size_t capacity)
    // {
    //     auto result    = hash_set<Type>{};
    //     result.buckets = allocate<Type>(capacity, stack);
    //     result.states  = allocate<typename hash_set<Type>::bucket_state>(
    //         capacity, hash_set<Type>::bucket_state::empty, stack);
    //     return result;
    // }

    // template <typename Type>
    // inline hash_set<Type>
    // allocate_set(const array<Type>& values, size_t capacity = -1,
    //              Allocator_Linear& stack = default_allocator()) {
    //     if (capacity == -1) capacity = values.size();
    //     auto result = allocate_set<Type>(capacity, stack);
    //     for (auto& v : values) result.insert(v);
    //     return result;
    // }

    template <typename Type>
    inline array<Type> copy(const array<Type>& arr) {
        auto result = allocate<Type>(arr.size());
        copy_to(arr, result);
        return result;
    }
};

struct Allocator_Linear;
Allocator_Linear* default_allocator();

/* Allocator_Linear handles memory allocation. It allows to allocate data
 * incrementally on a stack, which is a pre-allocated memory memory. By using
 * the helper struct stack_frame, memory deallocation is automatic */

struct Allocator_Linear : Allocator {
    Memory memory;
    size_t head;
    byte*  last_allocation = nullptr;

    Allocator_Linear(size_t capacity)
        : memory(capacity), head(0), last_allocation(nullptr) {}

    byte*       data() { return memory.data; }
    byte*       back() { return memory.data + head; }
    const byte* data() const { return memory.data; }
    const byte* back() const { return memory.data + head; }

    inline void align(size_t alignment) override {
        auto modulo = (uintptr_t)back() % alignment;
        if (modulo != 0) {
            allocate_bytes(alignment - modulo);
        }
    }

    inline void end_frame(Frame* frame) override { this->head = frame->start; }

    inline Frame begin_frame() override {
        auto frame      = Frame{};
        frame.allocator = this;
        frame.start     = this->head;
        return frame;
    }

    // allocate chosen amount of bytes
    byte* allocate_bytes(size_t bytes) override;
    byte* reallocate_bytes(byte* ptr, size_t bytes) override;
};

#define stack_frame() auto _frame = default_allocator()->begin_frame()

// Wrapper functions using default_allocator
template <typename T>
inline array<T> allocate(size_t size) {
    return default_allocator()->allocate<T>(size);
}

template <typename T>
inline array<T> allocate(size_t count, const T& value) {
    return default_allocator()->allocate<T>(count, value);
}

template <typename T>
inline array<T> allocate(const std::initializer_list<T>& list) {
    return default_allocator()->allocate<T>(list);
}

// template <typename T>
// inline array<T> copy(const array<T>& arr) {
//     return default_allocator()->copy<T>(arr);
// }

// // Deep copy for array of arrays
// template <typename T>
// inline T copy(const T& arr) {
//     auto result = allocate<array<T>>(arr.size());
//     for (size_t i = 0; i < arr.size(); i++) {
//         result[i] = copy(arr[i]);
//     }
//     return result;
// }

template <typename T>
inline void resize(array<T>& arr, size_t count) {
    default_allocator()->resize<T>(arr, count);
}

inline void align(size_t size) { default_allocator()->align(size); }

template <typename T>
inline void align_for() {
    default_allocator()->align_for<T>();
}

#include <cstdlib>
#include <vector>

struct Allocator_Heap : Allocator {
    std::vector<byte*> allocations;

    Allocator_Heap() = default;

    ~Allocator_Heap() {
        for (auto ptr : allocations) {
            free(ptr);
        }
        allocations.clear();
    }

    byte* allocate_bytes(size_t bytes) override;
    byte* reallocate_bytes(byte* ptr, size_t bytes) override;

    inline void end_frame(Frame* frame) override {
        for (size_t i = frame->start; i < this->allocations.size(); i++) {
            free(this->allocations[i]);
        }

        this->allocations.resize(frame->start);
    }

    inline Frame begin_frame() override {
        auto frame      = Frame{};
        frame.allocator = this;
        frame.start     = this->allocations.size();
        return frame;
    }
};