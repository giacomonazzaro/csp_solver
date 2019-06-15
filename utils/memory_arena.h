#ifndef GIACOMO_MEMORY_ARENA
#define GIACOMO_MEMORY_ARENA
namespace giacomo {

enum class byte : unsigned char {};

struct memory_arena {
    byte*  data;
    size_t capacity;

    memory_arena(size_t n) {
        // allocate memory on initialization
        data = new byte[n];
        if (data == nullptr)
            capacity = 0;
        else
            capacity = n;
    }

    ~memory_arena() {
        // free memory on destruction
        if (data != nullptr) delete[] data;
        data     = nullptr;
        capacity = 0;
    }

    // implicit conversion to raw pointer
    operator void*() const { return data; }
};

inline bool grow_memory_arena(memory_arena& arena, size_t capacity) {
    // If there is no need to grow, return success
    if (capacity <= arena.capacity) return true;

    // allocate new memory, if something goes wrong return failure
    auto data = new byte[capacity];
    if (data == nullptr) return false;

    // copy data from old location to new one
    for (int i = 0; i < arena.capacity; ++i) data[i] = arena.data[i];

    // free old memory
    if (arena.data) delete[] arena.data;

    // update members
    arena.data     = data;
    arena.capacity = capacity;

    // return success
    return true;
}

}  // namespace giacomo

#endif
