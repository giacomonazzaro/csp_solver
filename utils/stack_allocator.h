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

#if 1
extern stack_allocator default_allocator;

template <typename Type>
inline array<Type> allocate(int count, stack_allocator& = default_allocator);

template <typename Type, typename Filler>
inline array<Type> allocate(int count, const Filler& filler,
                            stack_allocator& = default_allocator);

template <typename Type>
inline array<Type> allocate(const std::initializer_list<Type>& list,
                            stack_allocator& = default_allocator);

template <typename Type>
inline array<Type> copy(const array<Type>& arr,
                        stack_allocator& = default_allocator);

template <typename Type>
inline array<array<Type>> copy(const array<array<Type>>& arr,
                               stack_allocator& = default_allocator);

#define stack_frame() auto _frame = make_stack_frame(default_allocator);
#endif

inline unsigned char* allocate_bytes(int bytes, stack_allocator& stack) {
    assert(stack.data != nullptr);
    auto ptr = (stack.offset + bytes >= stack.capacity) ?
                   nullptr :
                   stack.data + stack.offset;

    assert(ptr != nullptr);  // Resize stack? Not for now.
    stack.offset += bytes;
    return ptr;
}

// template <typename Type>
// inline Type* allocate(stack_allocator& stack) {
//     return (Type*)allocate(stack, sizeof(Type));
// }

template <typename Type>
inline array<Type> allocate(int count, stack_allocator& stack) {
    int         bytes = sizeof(Type) * count;
    array<Type> result;
    result.data = (Type*)allocate_bytes(bytes, stack);
    return result;
}

template <typename Type, typename Filler>
inline array<Type> allocate(int count, const Filler& filler,
                            stack_allocator& stack) {
    auto result = allocate<Type>(count, stack);
    fill(result, filler);
    return result;
}

template <typename Type>
inline array<Type> allocate(const std::initializer_list<Type>& list,
                            stack_allocator&                   stack) {
    auto result = allocate<Type>((int)list.size(), stack);
    int  i      = 0;
    for (auto& v : list) result[i++] = v;
    return result;
}

template <typename Type>
inline array<Type> copy(const array<Type>& arr, stack_allocator& stack) {
    auto result = allocate<Type>(arr.count, stack);
    copy_to(arr, result);
    return result;
}

template <typename Type>
inline array<array<Type>> copy(const array<array<Type>>& arr,
                               stack_allocator&          stack) {
    auto result = allocate<array<Type>>(arr.count, stack);
    for (int i = 0; i < result.count; i++) result[i] = copy(arr[i], stack);
    return result;
}

// template <typename Type, typename CopyableBuffer>
// inline array<array<Type>> copy(stack_allocator&      stack,
//                                const CopyableBuffer& arr) {
//     auto result = allocate<array<Type>>(stack, arr.count);
//     for (int i = 0; i < result.count; i++) result[i] = copy(stack, arr[i]);
//     return result;
// }

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

#if 0
// Using default_allocator as hidden parameter (@Design: namespace?)
namespace default_stack_allocator {

extern stack_allocator default_allocator;

inline void init_default_stack_allocator(int size) {
    init_stack_allocator(default_allocator, size);
}

inline void destroy_default_stack_allocator() {
    destroy_stack_allocator(default_allocator);
}

inline unsigned char* allocate_bytes(int bytes) {
    return allocate_bytes(bytes, default_allocator);
}

template <typename Type>
inline Type& allocate() {
    return allocate<Type>(default_allocator);
}

template <typename Type>
inline array<Type> allocate(int count) {
    return allocate<Type>(count, default_allocator);
}

template <typename Type, typename Filler>
inline array<Type> allocate(int count, const Filler& filler) {
    auto result = allocate<Type>(count);
    fill(result, filler);
    return result;
}

template <typename Type>
inline array<Type> allocate(const std::initializer_list<Type>& list) {
    return allocate(list, default_allocator);
}

template <typename Type>
inline array<Type> copy(const array<Type>& arr) {
    return copy(arr, default_allocator);
}

template <typename Type>
inline array<array<Type>> copy(const array<array<Type>>& arr) {
    return copy(arr, default_allocator);
}
// Used to temporarly allocate local data in stack frames.
#define stack_frame() \
    auto _frame = make_stack_frame(&default_stack_allocator::default_allocator);

// #define copy(a) copy(default_allocator, a);

}  // namespace default_stack_allocator

// #else

stack_allocator default_allocator;

template <typename Type>
inline array<Type> allocate(int count, stack_allocator& = default_allocator);

template <typename Type, typename Filler>
inline array<Type> allocate(int count, const Filler& filler,
                            stack_allocator& stack);

template <typename Type>
inline array<Type> allocate(const std::initializer_list<Type>& list,
                            stack_allocator& = default_allocator);

template <typename Type>
inline array<Type> copy(const array<Type>& arr,
                        stack_allocator& = default_allocator);

template <typename Type>
inline array<array<Type>> copy(const array<array<Type>>& arr,
                               stack_allocator& = default_allocator);

#define stack_frame() auto _frame = make_stack_frame(default_allocator);
#endif
