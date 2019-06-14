#ifndef GIACOMO_MEMORY_ARENA
#define GIACOMO_MEMORY_ARENA
namespace giacomo {

enum class byte : unsigned char {};

struct memory_arena {
    byte*  data;
    size_t capacity;

    memory_arena(size_t n) {
        this->data = new byte[n];
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

    operator void*() const { return data; }
};

inline bool grow_memory_arena(memory_arena& arena, size_t capacity) {
    if (capacity <= arena.capacity) return true;

    // allocate new memory
    auto data = new byte[capacity];
    if (data == nullptr) return false;

    // copy from old location to new one
    for (int i = 0; i < arena.capacity; ++i) data[i] = arena.data[i];

    // free old memory
    if (arena.data) delete[] arena.data;

    // update stack allocator
    arena.data     = data;
    arena.capacity = capacity;
    return true;
}

}  // namespace giacomo

#endif
