#pragma once
#include "array.h"

struct stack_allocator {
    unsigned char* data     = nullptr;
    int            capacity = 0;
    int            offset   = 0;
};

inline void init_stack_allocator(stack_allocator& stack, int size) {
    assert(stack.data == nullptr);
    stack.data     = new unsigned char[size];
    stack.capacity = size;
    stack.offset   = 0;
}

inline void destroy_stack_allocator(stack_allocator& stack) {
    assert(stack.data != nullptr);
    delete[] stack.data;
    stack.data     = nullptr;
    stack.capacity = 0;
    stack.offset   = 0;
}

inline unsigned char* allocate(stack_allocator& stack, int bytes) {
    assert(stack.data != nullptr);
    auto ptr = (stack.offset + bytes >= stack.capacity) ?
                   nullptr :
                   stack.data + stack.offset;

    assert(ptr != nullptr);  // Resize stack? Not for now.
    stack.offset += bytes;
    return ptr;
}

template <typename Type>
inline Type& allocate(stack_allocator& stack) {
    return *((Type*)allocate(stack, sizeof(Type)));
}

template <typename Type>
inline array<Type> allocate_array(stack_allocator& stack, int count) {
    int  bytes  = sizeof(Type) * count;
    auto result = array<Type>{nullptr, count};
    result.data = (Type*)allocate(stack, bytes);
    return result;
}

template <typename Type>
inline array<Type> allocate_array(stack_allocator& stack, int count,
                                  const Type& def) {
    auto result = allocate_array<Type>(stack, count);
    for (int i = 0; i < count; ++i) result[i] = def;
    return result;
}

template <typename Type>
inline array<Type> allocate_array(stack_allocator&                   stack,
                                  const std::initializer_list<Type>& list) {
    auto result = allocate_array<Type>(stack, (int)list.size());
    int  i      = 0;
    for (auto& v : list) result[i++] = v;
    return result;
}

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

struct stack_frame_cleaner {
    stack_allocator* stack = nullptr;
    int              start = 0;

    ~stack_frame_cleaner() { stack->offset = start; }
};

inline stack_frame_cleaner make_stack_frame(stack_allocator* stack) {
    auto result  = stack_frame_cleaner{};
    result.stack = stack;
    result.start = stack->offset;
    return result;
}

// Using default_allocator as hidden parameter (@Design: namespace?)
namespace default_stack_allocator {

extern stack_allocator default_allocator;

inline void init_default_stack_allocator(int size) {
    init_stack_allocator(default_allocator, size);
}

inline void destroy_default_stack_allocator() {
    destroy_stack_allocator(default_allocator);
}

inline unsigned char* allocate(int bytes) {
    return allocate(default_allocator, bytes);
}

template <typename Type>
inline Type& allocate() {
    return allocate<Type>(default_allocator);
}

template <typename Type>
inline array<Type> allocate_array(int count) {
    return allocate_array<Type>(default_allocator, count);
}

template <typename Type>
inline array<Type> allocate_array(int count, const Type& def) {
    return allocate_array(default_allocator, count, def);
}

template <typename Type>
inline array<Type> allocate_array(const std::initializer_list<Type>& list) {
    return allocate_array(default_allocator, list);
}

template <typename Type>
inline array<Type> copy(const array<Type>& arr) {
    return copy(default_allocator, arr);
}

template <typename Type>
inline array<array<Type>> copy(const array<array<Type>>& arr) {
    return copy(default_allocator, arr);
}

// Used to temporarly allocate local data in stack frames.
#define stack_frame() \
    auto _frame = make_stack_frame(&default_stack_allocator::default_allocator);

// #define copy(a) copy(default_allocator, a);

}  // namespace default_stack_allocator
