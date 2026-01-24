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
inline void copy_struct(Allocator_Linear* allocator, const T& x) {
    if constexpr (has_custom_copy_v<T>) {
        Custom_Copy<T>::copy_struct(allocator, x);
    } else {
        copy_value(allocator, x);
    }
}

template <typename T>
inline T& copy_struct(const T& x, Allocator_Linear* allocator) {
    auto result = allocator->data();
    copy_struct(allocator, x);
    return *(T*)result;
}

template <typename T>
inline T& copy(const T& x) {
    return copy_struct(x, (Allocator_Linear*)default_allocator());
}

template <typename T>
struct Custom_Copy<array<T>> {
    static void copy_struct(Allocator_Linear* allocator, const array<T>& v) {
        auto& w = allocator->allocate<array<T>>(v.count);
        // w.count = v.size();

        if (v.size() == 0) {
            w.data = nullptr;
            return;
        }

        //w.data = (T*)allocator->back();

        // copy array elements
        if constexpr (has_custom_copy_v<T>) {
            // Allocate space for all element headers
//            auto headers_start = allocator->back();
//            for (size_t i = 0; i < v.size(); i++) {
//                allocator->allocate<T>();
//            }
            // Now fill in each header by copying element contents
            auto headers = w.data;
            for (size_t i = 0; i < v.size(); i++) {
                auto element_start = allocator->back();
                Custom_Copy<T>::copy_struct(allocator, v[i]);
                headers[i] = *(T*)element_start;
            }
        } else {
            // Primitive types: just copy directly
            for (size_t i = 0; i < v.size(); i++) {
                ::copy_struct(allocator, v[i]);
            }
        }
    }
};

#define COPY_BY_MEMBER(T)                                                   \
    template <>                                                             \
    struct Custom_Copy<T> {                                                 \
        static void copy_struct(Allocator_Linear* allocator, const T& t) {  \
            /* First allocate all contiguous space for all members */       \
            auto& result = allocator->allocate<T>();                        \
                                                                            \
            visit_struct::for_each(t, [&](const char* name,                 \
                                          const auto& value) {              \
                using MemberType    = std::decay_t<decltype(value)>;        \
                auto  member_offset = (byte*)&value - (byte*)&t;            \
                auto& dest_member   = *(MemberType*)((byte*)&result +       \
                                                   member_offset);        \
                if constexpr (has_custom_copy_v<MemberType>) {              \
                    auto data_start = allocator->back();                    \
                    Custom_Copy<MemberType>::copy_struct(allocator, value); \
                    dest_member = *(MemberType*)data_start;                 \
                } else {                                                    \
                    /* Just copy the value */                               \
                    dest_member = value;                                    \
                }                                                           \
            });                                                             \
        }                                                                   \
    };
