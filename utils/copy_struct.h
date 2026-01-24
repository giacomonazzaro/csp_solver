#pragma once
#include <cstring>

#include "allocator.h"
#include "array.h"
#include "memory.h"
#include "print_struct.h"

template <typename T>
struct Custom_Copy;

template <typename T, typename = void>
struct has_custom_copy : std::false_type {};

template <typename T>
struct has_custom_copy<T, std::void_t<decltype(sizeof(Custom_Copy<T>))>>
    : std::true_type {};

// Helper variable template
template <typename T>
inline constexpr bool has_custom_copy_v = has_custom_copy<T>::value;

// inline void* copy_bytes(Allocator_Linear* allocator, const void* data,
//                         size_t size) {
//     auto ptr = allocator->allocate_bytes(size);
//     memcpy(ptr, data, size);
//     return ptr;
// }

template <typename T>
inline void copy_value(Allocator_Linear* allocator, const T& x) {
    auto& ptr = allocator->allocate<T>();
    memcpy(&ptr, &x, sizeof(T));
}

template <typename T>
inline void copy_struct(T& output, const T& input,
                        Allocator_Linear* allocator) {
    if constexpr (has_custom_copy_v<T>) {
        Custom_Copy<T>::copy_struct(output, input, allocator);
    } else {
        memcpy(&output, &input, sizeof(T));
    }
}

template <typename T>
inline T& copy(const T& x, Allocator_Linear* allocator) {
    auto& output = allocator->allocate<T>();
    copy_struct(output, x, allocator);
    return output;
}

template <typename T>
inline T& copy(const T& x) {
    return copy(x, (Allocator_Linear*)default_allocator());
}

template <typename T>
struct Custom_Copy<array<T>> {
    static void copy_struct(array<T>& w, const array<T>& v,
                            Allocator_Linear* allocator) {
        w.count = v.size();
        if (v.size() == 0) {
            w.data = nullptr;
            return;
        }
        w.data = (T*)allocator->allocate_bytes(sizeof(T) * v.count);

        // copy array elements
        if constexpr (has_custom_copy_v<T>) {
            // Now fill in each header by copying element contents
            for (size_t i = 0; i < v.size(); i++) {
                Custom_Copy<T>::copy_struct(w[i], v[i], allocator);
            }
        } else {
            // Primitive types: just copy directly
            memcpy(w.data, v.data, v.size() * sizeof(T));
        }
    }
};

#define COPY_BY_MEMBER(T)                                                    \
    template <>                                                              \
    struct Custom_Copy<T> {                                                  \
        static void copy_struct(T& result, const T& t,                       \
                                Allocator_Linear* allocator) {               \
            memcpy(&result, &t, sizeof(T));                                  \
            visit_struct::for_each(t, [&](const char* name,                  \
                                          const auto& value) {               \
                using MemberType   = std::decay_t<decltype(value)>;          \
                auto member_offset = (byte*)&value - (byte*)&t;              \
                if constexpr (has_custom_copy_v<MemberType>) {               \
                    auto& dest_member = *(MemberType*)((byte*)&result +      \
                                                       member_offset);       \
                    Custom_Copy<MemberType>::copy_struct(dest_member, value, \
                                                         allocator);         \
                }                                                            \
            });                                                              \
        }                                                                    \
    };
