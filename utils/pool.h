#pragma once
#include "allocator.h"

struct pool_key {
    int index      = -1;
    int generation = -1;

    bool operator==(const pool_key& other) const {
        return index == other.index and generation == other.generation;
    }
    bool operator!=(const pool_key& other) const { return !(*this == other); }
};

inline bool is_valid(pool_key key) { return key.index >= 0; }

template <typename Type>
struct Pool {
    array<Type> values;
    array<int>  generations;
    int         num_entries = 0;
    int         capacity    = 0;
    int         first_free  = 0;

    // In the memory of empty slots, we write the index to the next empty slot.
    int& next_free(int index) { return *(int*)&values[index]; }

    bool contains(pool_key key) const {
        if (key.index < 0 or key.index >= capacity) return false;
        if (generations[key.index] < 0) return false;
        return generations[key.index] == key.generation;
    }

    const Type& operator[](pool_key key) const {
        assert(contains(key));
        return values[key.index];
    }

    Type& operator[](pool_key key) {
        assert(contains(key));
        return values[key.index];
    }

    pool_key insert(const Type& value) {
        assert(first_free >= 0);
        int index          = first_free;
        first_free         = next_free(index);
        generations[index] = -generations[index];

        values[index] = value;
        num_entries += 1;

        return {index, generations[index]};
    }

    void remove(pool_key key) {
        assert(contains(key));
        generations[key.index] = -(generations[key.index] + 1);
        next_free(key.index)   = first_free;
        first_free             = key.index;
        num_entries -= 1;
    }

    int size() const { return num_entries; }

    // Iterators - skip unoccupied entries (negative generation)
    struct const_iterator {
        struct entry {
            pool_key    key;
            const Type& value;
        };
        const array<Type>& values;
        const array<int>&  generations;
        int                i;
        int                cap;

        const_iterator& operator++() {
            i += 1;
            while (i < cap and generations[i] < 0) i += 1;
            return *this;
        }
        bool operator!=(const const_iterator& other) const {
            return i != other.i;
        }
        entry operator*() const { return {{i, generations[i]}, values[i]}; }
    };

    inline const_iterator begin() const {
        int start = 0;
        while (start < capacity and generations[start] < 0) start += 1;
        return const_iterator{values, generations, start, capacity};
    }
    inline const_iterator end() const {
        return const_iterator{values, generations, capacity, capacity};
    }

    struct mutable_iterator {
        struct entry {
            pool_key key;
            Type&    value;
        };
        array<Type>&      values;
        const array<int>& generations;
        int               i;
        int               cap;

        mutable_iterator& operator++() {
            i += 1;
            while (i < cap and generations[i] < 0) i += 1;
            return *this;
        }
        bool operator!=(const mutable_iterator& other) const {
            return i != other.i;
        }
        entry operator*() { return {{i, generations[i]}, values[i]}; }
    };

    inline mutable_iterator begin() {
        int start = 0;
        while (start < capacity and generations[start] < 0) start += 1;
        return mutable_iterator{values, generations, start, capacity};
    }
    inline mutable_iterator end() {
        return mutable_iterator{values, generations, capacity, capacity};
    }
};

template <typename Type>
inline Pool<Type> allocate_pool(Allocator& allocator, size_t capacity) {
    Pool<Type> pool;
    pool.values      = allocator.allocate<Type>(capacity);
    pool.generations = allocator.allocate<int>(capacity, -1);
    pool.num_entries = 0;
    pool.capacity    = (int)capacity;
    pool.first_free  = 0;
    // Chain all slots: 0 -> 1 -> 2 -> ... -> -1
    for (size_t i = 0; i < capacity - 1; i++) {
        pool.next_free(i) = i + 1;
    }
    pool.next_free(capacity - 1) = -1;
    return pool;
}

template <typename Type>
inline Pool<Type> allocate_pool(size_t capacity) {
    return allocate_pool<Type>(*default_allocator(), capacity);
}
