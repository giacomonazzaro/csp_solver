#include "allocator.h"

static Allocator_Linear* _allocator = nullptr;

Allocator_Linear* default_allocator() {
    if (_allocator == nullptr) {
        auto capacity = (1024 * 1024 * 10);  // 10 MB
        _allocator    = new Allocator_Linear(capacity);
    }
    return _allocator;
}

void set_default_allocator(Allocator_Linear* allocator) {
    if (_allocator != nullptr) {
        delete _allocator;
    }
    _allocator = allocator;
}

// Allocator_Linear implementation
byte* Allocator_Linear::allocate_bytes(size_t bytes) {
    assert(this->memory.data != nullptr);

    auto capacity = this->memory.capacity;
    while (this->head + bytes > capacity) {
        capacity = 2 * (capacity + 8);
    }
    auto success = grow_memory(this->memory, capacity);
    assert(success);

    auto pointer          = this->memory.data + this->head;
    this->last_allocation = pointer;
    this->head += bytes;
    return pointer;
}

byte* Allocator_Linear::reallocate_bytes(byte* ptr, size_t bytes) {
    if (ptr == this->last_allocation) {
        this->head = (ptr - this->memory.data) + bytes;
        return ptr;
    } else {
        return this->allocate_bytes(bytes);
    }
}

// Allocator_Heap implementation
byte* Allocator_Heap::allocate_bytes(size_t bytes) {
    auto pointer = (byte*)malloc(bytes);
    assert(pointer != nullptr);
    allocations.push_back(pointer);
    return pointer;
}

byte* Allocator_Heap::reallocate_bytes(byte* ptr, size_t bytes) {
    auto new_ptr = (byte*)realloc(ptr, bytes);
    assert(new_ptr != nullptr);
    // update allocation list
    for (size_t i = allocations.size() - 1; i >= 0; i--) {
        if (allocations[i] == ptr) {
            allocations[i] = new_ptr;
            return new_ptr;
        }
    }
    // if we reach here, ptr was not found
    assert(false && "Pointer not found in allocations");
    return nullptr;
}