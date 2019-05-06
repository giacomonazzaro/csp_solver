#ifndef GIACOMO_MEMORY_ARENA
#define GIACOMO_MEMORY_ARENA

struct memory_arena {
    unsigned char* data;
    size_t         capacity;

    memory_arena(size_t n) {
        this->data = new unsigned char[n];
        if (data == nullptr)
            this->capacity = 0;
        else
            this->capacity = n;
    }
    ~memory_arena() {
        if (data != nullptr) delete[] data;
        data     = nullptr;
        capacity = 0;
    }
};

inline bool grow_memory_arena(memory_arena& arena, size_t capacity) {
    if (capacity <= arena.capacity) return true;

    // allocate new memory
    auto new_data = new unsigned char[capacity];
    if (new_data == nullptr) return false;

    // copy from old location to new one
    for (int i = 0; i < arena.capacity; ++i) new_data[i] = arena.data[i];

    // free old memory
    if (arena.data) delete[] arena.data;

    // update stack allocator
    arena.data     = new_data;
    arena.capacity = capacity;
    return true;
}

#endif
