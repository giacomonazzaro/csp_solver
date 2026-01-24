#pragma once

#include <cstring>  // for memcpy

using byte = unsigned char;

struct Memory {
    byte*  data;
    size_t capacity;

    Memory(size_t n) {
        // allocate memory on initialization
        if (n > 0) {
            data = new byte[n];
            if (data != nullptr) {
                capacity = n;
            }
        }
    }

    explicit Memory(const Memory& from) : Memory(from.capacity) {
        memcpy(this->data, from.data, from.capacity);
    }

    ~Memory() {
        // free memory on destruction
        if (data != nullptr) delete[] data;
        data     = nullptr;
        capacity = 0;
    }

    Memory& operator=(const Memory&) = delete;

    // implicit conversion to raw pointer
    operator void*() const { return data; }
};

inline bool grow_memory(Memory& memory, size_t new_capacity) {
    // If there is no need to grow, return success
    if (new_capacity <= memory.capacity) return true;

    // Allocate new memory
    byte* new_data = new byte[new_capacity];
    if (new_data == nullptr) return false;

    // Copy data from old location to new one
    if (memory.data != nullptr && memory.capacity > 0) {
        memcpy(new_data, memory.data, memory.capacity);
    }

    // Free old memory
    if (memory.data != nullptr) {
        delete[] memory.data;
    }

    // Update members
    memory.data     = new_data;
    memory.capacity = new_capacity;

    return true;
}
