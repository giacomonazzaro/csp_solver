#ifndef GIACOMO_MEMORY_ARENA
#define GIACOMO_MEMORY_ARENA

struct memory_arena {
    unsigned char* data     = nullptr;
    size_t         capacity = 0;

    ~memory_arena() {
        if (data != nullptr) delete[] data;

        data     = nullptr;
        capacity = 0;
    }
};

inline memory_arena make_memory_arena(size_t capacity) {
    auto arena     = memory_arena();
    arena.data     = new unsigned char[capacity];
    arena.capacity = capacity;
    return arena;
}

#endif