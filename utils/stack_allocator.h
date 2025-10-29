#pragma once
#define STD_ARRAY 0

#if STD_ARRAY
#include "array_std_interface.h"
#else
#include "array.h"
#endif

#include "memory_arena.h"

#if STD_ARRAY
#include "array_std_interface.h"
#endif

namespace giacomo {
/* stack_allocator handles memory allocation. It allows to allocate data
 * incrementally on a stack, which is a pre-allocated memory arena. By using the
 * helper struct stack_frame, memory deallocation is automatic */

struct stack_allocator {
    memory_arena* arena;
    size_t        head;

    byte*       data() { return arena->data; }
    const byte* data() const { return arena->data; }
};

struct stack_frame {
    stack_allocator& stack;
    size_t           start;

    stack_frame(stack_allocator& s) : stack(s), start(stack.head) {}
    ~stack_frame() { stack.head = start; }
};

inline stack_allocator& default_allocator() {
    static stack_allocator _default_allocator;
    return _default_allocator;
}

#if !STD_ARRAY
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
inline Type& allocate(stack_allocator& = default_allocator());

// allocate array
template <typename Type>
inline array<Type> allocate(int count, stack_allocator& = default_allocator());

// allocate array with initialized elements
template <typename Type>
inline array<Type> allocate(int count, const Type& value,
                            stack_allocator& = default_allocator());

// allocate array with initialized elements by initializer_list
template <typename Type>
inline array<Type> allocate(const std::initializer_list<Type>& list,
                            stack_allocator& = default_allocator());

// allocate array with elements copied from another array
template <typename Type>
inline array<Type> copy(const array<Type>& arr,
                        stack_allocator& = default_allocator());

// allocate array of array with elements copied from another array of array
template <typename Type>
inline array<array<Type>> copy(const array<array<Type>>& arr,
                               stack_allocator& = default_allocator());

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

template <typename Type>
inline array<Type> allocate(int count, const Type& value,
                            stack_allocator& stack) {
    auto result = allocate<Type>(count, stack);
    for (size_t i = 0; i < count; i++) {
        result[i] = value;
    }
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
    auto result = allocate<Type>(arr.size(), stack);
    copy_to(arr, result);
    return result;
}

template <typename Type>
inline array<array<Type>> copy(const array<array<Type>>& arr,
                               stack_allocator&          stack) {
    auto result = allocate<array<Type>>(arr.size(), stack);
    for (int i = 0; i < result.size(); i++) result[i] = copy(arr[i], stack);
    return result;
}

// template <typename Type, typename CopyableBuffer>
// inline array<array<Type>> copy(stack_allocator&      stack,
//                                const CopyableBuffer& arr) {
//     auto result = allocate<array<Type>>(stack, arr.size());
//     for (int i = 0; i < result.size(); i++) result[i] = copy(stack,
//     arr[i]); return result;
// }

#endif

}  // namespace giacomo

#define stack_frame() auto _frame = stack_frame(default_allocator());