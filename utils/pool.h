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
struct pool_entry {
    Type value;
    int  generation = -1;  // -1 means unoccupied
};

template <typename Type>
struct Pool {
    array<pool_entry<Type>> data;
    array<int>              free_list;
    int                     num_entries = 0;
    int                     capacity    = 0;

    bool contains(pool_key key) const {
        if (key.index < 0 or key.index >= capacity) return false;
        if (data[key.index].generation < 0) return false;
        return data[key.index].generation == key.generation;
    }

    const Type& operator[](pool_key key) const {
        assert(contains(key));
        return data[key.index].value;
    }

    Type& operator[](pool_key key) {
        assert(contains(key));
        return data[key.index].value;
    }

    pool_key insert(const Type& value) {
        int index;
        if (free_list.count > 0) {
            index = free_list.back();
            free_list.count -= 1;
            // Flip generation back to positive (was negated when removed)
            data[index].generation = -data[index].generation;
        } else {
            index = capacity;
            capacity += 1;
            data[index].generation = 0;
        }

        data[index].value = value;
        num_entries += 1;

        return {index, data[index].generation};
    }

    void remove(pool_key key) {
        assert(contains(key));
        // Increment and negate to mark as unoccupied
        data[key.index].generation = -(data[key.index].generation + 1);
        free_list.add(key.index);
        num_entries -= 1;
    }

    int size() const { return num_entries; }

    // Iterators - skip unoccupied entries (negative generation)
    struct const_iterator {
        struct entry {
            pool_key    key;
            const Type& value;
        };
        const array<pool_entry<Type>>& data;
        int                            i;
        int                            cap;

        const_iterator& operator++() {
            i += 1;
            while (i < cap and data[i].generation < 0) i += 1;
            return *this;
        }
        bool operator!=(const const_iterator& other) const {
            return i != other.i;
        }
        entry operator*() const {
            return {{i, data[i].generation}, data[i].value};
        }
    };

    inline const_iterator begin() const {
        int start = 0;
        while (start < capacity and data[start].generation < 0) start += 1;
        return const_iterator{data, start, capacity};
    }
    inline const_iterator end() const {
        return const_iterator{data, capacity, capacity};
    }

    struct mutable_iterator {
        struct entry {
            pool_key key;
            Type&    value;
        };
        array<pool_entry<Type>>& data;
        int                      i;
        int                      cap;

        mutable_iterator& operator++() {
            i += 1;
            while (i < cap and data[i].generation < 0) i += 1;
            return *this;
        }
        bool operator!=(const mutable_iterator& other) const {
            return i != other.i;
        }
        entry operator*() { return {{i, data[i].generation}, data[i].value}; }
    };

    inline mutable_iterator begin() {
        int start = 0;
        while (start < capacity and data[start].generation < 0) start += 1;
        return mutable_iterator{data, start, capacity};
    }
    inline mutable_iterator end() {
        return mutable_iterator{data, capacity, capacity};
    }
};

template <typename Type>
inline Pool<Type> allocate_pool(Allocator& allocator, size_t max_capacity) {
    Pool<Type> pool;
    pool.data      = allocator.allocate<pool_entry<Type>>(max_capacity);
    pool.free_list = allocator.allocate<int>(max_capacity);
    pool.free_list.count = 0;
    pool.num_entries     = 0;
    pool.capacity        = 0;
    // Initialize all entries as unoccupied
    for (size_t i = 0; i < max_capacity; i++) {
        pool.data[i].generation = -1;
    }
    return pool;
}

template <typename Type>
inline Pool<Type> allocate_pool(size_t max_capacity) {
    return allocate_pool<Type>(*default_allocator(), max_capacity);
}
