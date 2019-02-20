#pragma once
#include "array.h"

struct stack_allocator {
    unsigned char* data     = nullptr;
    int            capacity = 0;
    int            offset   = 0;

    ~stack_allocator() {
        assert(data == nullptr && "call destroy_stack_allocator()!");
    }
};

extern stack_allocator default_allocator;

inline void init_stack_allocator(stack_allocator& stack, int size) {
    assert(stack.data == nullptr);
    stack.data     = new unsigned char[size];
    stack.capacity = size;
    stack.offset   = 0;
}

inline void init_default_stack_allocator(int size) {
    init_stack_allocator(default_allocator, size);
}

inline void destroy_stack_allocator(stack_allocator& stack) {
    assert(stack.data != nullptr);
    delete[] stack.data;
    stack.data     = nullptr;
    stack.capacity = 0;
    stack.offset   = 0;
}

inline void destroy_default_stack_allocator() {
    destroy_stack_allocator(default_allocator);
}

inline unsigned char* allocate(stack_allocator& stack, int bytes) {
    assert(stack.data != nullptr);
    auto ptr = (stack.offset + bytes >= stack.capacity) ?
                   nullptr :
                   stack.data + stack.offset;

    assert(ptr != nullptr);  // Resize stack? Not for now.
    stack.offset += bytes;
    printf("stack size: %d\n", stack.offset);
    return ptr;
}

inline unsigned char* allocate(int bytes) {
    return allocate(default_allocator, bytes);
}

template <typename Type>
inline Type& allocate(stack_allocator& stack) {
    return *((Type*)allocate(stack, sizeof(Type)));
}

template <typename Type>
inline Type& allocate() {
    return allocate<Type>(default_allocator);
}

template <typename Type>
inline array<Type> allocate_array(stack_allocator& stack, int count) {
    int  bytes  = sizeof(Type) * count;
    auto result = array<Type>{nullptr, count};
    result.data = (Type*)allocate(stack, bytes);
    return result;
}

template <typename Type>
inline array<Type> allocate_array(int count) {
    return allocate_array<Type>(default_allocator, count);
}

template <typename Type>
inline array<Type> allocate_array(stack_allocator& stack, int count,
                                  const Type& def) {
    auto result = allocate_array<Type>(stack, count);
    for (int i = 0; i < count; ++i) result[i] = def;
    return result;
}

template <typename Type>
inline array<Type> allocate_array(int count, const Type& def) {
    return allocate_array(default_allocator, count, def);
}

template <typename Type>
inline array<array<Type>> allocate_arrays(stack_allocator&  stack,
                                          const array<int>& counts) {
    auto result  = allocate_array<array<Type>>(stack, counts.count);
    result.count = counts.count;
    for (int i = 0; i < counts.count; ++i) {
        result[i] = allocate_array<Type>(stack, counts[i]);
    }
    return result;
}

template <typename Type>
inline array<array<Type>> allocate_arrays(const array<int>& counts) {
    return allocate_arrays<Type>(counts);
}

template <typename Type>
inline array<array<Type>> allocate_arrays(stack_allocator&   stack,
                                          const array<int>&  counts,
                                          const array<Type>& def) {
    auto result = allocate_arrays<Type>(stack, counts);
    for (int i = 0; i < result.count; ++i) {
        copy_to(def, result[i]);
    }
    return result;
}

template <typename Type>
inline array<array<Type>> allocate_arrays(const array<int>&  counts,
                                          const array<Type>& def) {
    auto result = allocate_arrays<Type>(default_allocator, counts);
    for (int i = 0; i < result.count; ++i) {
        copy_to(def, result[i]);
    }
    return allocate_arrays<Type>(default_allocator, counts, def);
}

struct _stack_frame {
    stack_allocator* stack = nullptr;
    int              start = 0;

    _stack_frame(stack_allocator* stack) {
        this->stack = stack;
        start       = stack->offset;
    }

    ~_stack_frame() {
        printf("free %d\n", stack->offset-start);
        stack->offset = start;
        printf("stack size: %d\n", stack->offset);

    }
};

// Used to temporarly allocate local data in stack frames.
#define stack_frame() auto _frame = _stack_frame(&default_allocator);

template <typename Type>
inline array<Type> copy(stack_allocator& stack, const array<Type>& arr) {
    auto result = allocate_array<Type>(stack, arr.count);
    copy_to(arr, result);
    return result;
}

template <typename Type>
inline array<array<Type>> copy(stack_allocator&          stack,
                               const array<array<Type>>& arr) {
    auto result = allocate_array<array<Type>>(stack, arr.count);
    for (int i = 0; i < result.count; i++) result[i] = copy(stack, arr[i]);
    return result;
}

#define copy(a) copy(default_allocator, a);
