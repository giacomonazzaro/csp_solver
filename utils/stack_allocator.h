#pragma once
#include "array.h"
#include "memory_arena.h"

namespace giacomo {
/* stack_allocator handles memory allocation. It allows to allocate data
 * incrementally on a stack (a pre-allocated memory arena). By using the helper
 * struct stack_frame_cleaner, memory deallocation is automatic */

struct stack_allocator {
    memory_arena* arena;
    size_t        head;
};

extern stack_allocator default_allocator;

// allocate chosen amount of bytes
inline byte* allocate_bytes(int bytes, stack_allocator& stack) {
    assert(stack.arena->data != nullptr);
    if (stack.head + bytes >= stack.arena->capacity) {
        auto capacity = stack.head + bytes;
        if (capacity < (stack.arena->capacity + 8) * 2)
            capacity = (stack.arena->capacity + 8) * 2;

        auto success = grow_memory_arena(*stack.arena, capacity);
        if (not success) return nullptr;
    }

    auto ptr = stack.arena->data + stack.head;
    stack.head += bytes;
    return ptr;
}

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
//     for (int i = 0; i < result.count; i++) result[i] = copy(stack, arr[i]);
//     return result;
// }

struct stack_frame_cleaner {
    stack_allocator* stack = nullptr;
    size_t           start = 0;

    ~stack_frame_cleaner() { stack->head = start; }
};

inline stack_frame_cleaner make_stack_frame(stack_allocator* stack) {
    return stack_frame_cleaner{stack, stack->head};
}

}  // namespace giacomo

#define stack_frame() \
    auto _frame = make_stack_frame(&giacomo::default_allocator);
