#pragma once
#include "array.h"
#include "memory_arena.h"

namespace giacomo {
/* stack_allocator handles memory allocation. It allows to allocate data
 * incrementally on a stack, which is a pre-allocated memory arena. By using the
 * helper struct stack_frame, memory deallocation is automatic */

struct stack_allocator {
    memory_arena* arena;
    size_t        head;
};

struct stack_frame {
    stack_allocator& stack;
    size_t           start;

    stack_frame(stack_allocator& s) : stack(s), start(stack.head) {}
    ~stack_frame() { stack.head = start; }
};

extern stack_allocator default_allocator;

// allocate chosen amount of bytes
inline byte* allocate_bytes(size_t bytes, stack_allocator& stack) {
    assert(stack.arena->data != nullptr);

    auto capacity = stack.arena->capacity;
    while (stack.head + bytes > capacity) {
        capacity = 2 * (capacity + 8);
    }
    auto success = grow_memory_arena(*stack.arena, capacity);
    assert(success);

    auto pointer = stack.arena->data + stack.head;
    stack.head += bytes;
    return pointer;
}

// allocate struct
template <typename Type>
inline Type& allocate(stack_allocator& = default_allocator);

// allocate array
template <typename Type>
inline array<Type> allocate(int count, stack_allocator& = default_allocator);

// allocate array with initialized elements
template <typename Type, typename Filler>
inline array<Type> allocate(int count, const Filler& filler,
                            stack_allocator& = default_allocator);

// allocate array with initialized elements by initializer_list
template <typename Type>
inline array<Type> allocate(const std::initializer_list<Type>& list,
                            stack_allocator& = default_allocator);

// allocate array with elements copied from another array
template <typename Type>
inline array<Type> copy(const array<Type>& arr,
                        stack_allocator& = default_allocator);

// allocate array of array with elements copied from another array of array
template <typename Type>
inline array<array<Type>> copy(const array<array<Type>>& arr,
                               stack_allocator& = default_allocator);

/*
 * IMPLEMENTATION
 */

template <typename Type>
inline Type& allocate(stack_allocator& stack) {
    return *(Type*)allocate_bytes(sizeof(Type), stack);
}

template <typename Type>
inline array<Type> allocate(int count, stack_allocator& stack) {
    int         bytes = sizeof(Type) * count;
    array<Type> result;
    result.count = count;
    result.data  = (Type*)allocate_bytes(bytes, stack);
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
//     for (int i = 0; i < result.count; i++) result[i] = copy(stack,
//     arr[i]); return result;
// }

}  // namespace giacomo

#define stack_frame() auto _frame = stack_frame(giacomo::default_allocator);
