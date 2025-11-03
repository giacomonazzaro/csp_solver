#pragma once

#include "array.h"

namespace giacomo {

// A simple hash function for types that can be cast to size_t.
// For production use, it's recommended to provide a more robust hash
// function for your specific type.
template <typename T>
struct hash {
    size_t operator()(const T& key) const { return static_cast<size_t>(key); }
};

// A simple hash set implementation using open addressing and linear probing.
// - The capacity is fixed on creation and it does not automatically resize.
// - It is the user's responsibility to allocate memory for the underlying
// arrays
//   (buckets and states) and initialize the hash_set with them. This is
//   consistent with the memory management style of this project.
template <typename Type, typename Hasher = hash<Type>>
struct hash_set {
    enum class bucket_state : unsigned char { empty, occupied, deleted };

    array<Type>         buckets = {};
    array<bucket_state> states  = {};
    size_t              count   = 0;
    Hasher              hasher;

    hash_set() : buckets(), states(), count(0), hasher() {}

    hash_set(const array<Type>& b, const array<bucket_state>& s)
        : buckets(b), states(s), count(0), hasher() {
        fill(states, bucket_state::empty);
    }

    void init(array<Type> b, array<bucket_state> s) {
        buckets = b;
        states  = s;
        count   = 0;
        fill(states, bucket_state::empty);
    }

    // Returns the index of the bucket with the given key, or -1 if not found.
    int find_key(const Type& key) const {
        if (buckets.count == 0) {
            return -1;
        }
        auto hash_value  = hasher(key);
        int  start_index = hash_value % buckets.count;

        for (int i = 0; i < buckets.count; ++i) {
            int current_index = (start_index + i) % buckets.count;
            if (states[current_index] == bucket_state::empty) {
                return -1;  // Key not found
            }
            if (states[current_index] == bucket_state::occupied &&
                buckets[current_index] == key) {
                return current_index;  // Key found
            }
            // If deleted, continue searching
        }
        return -1;  // Table searched completely
    }

    bool contains(const Type& key) const { return find_key(key) != -1; }

    // Inserts a key. Returns true if inserted, false if it already exists or
    // the set is full.
    bool insert(const Type& key) {
        if (count >= buckets.count) {
            return false;  // Table is full
        }

        unsigned int hash_value   = hasher(key);
        int          start_index  = hash_value % buckets.count;
        int          insert_index = -1;

        // Perform linear probing to find an empty or deleted slot.
        for (int i = 0; i < buckets.count; ++i) {
            int current_index = (start_index + i) % buckets.count;
            if (states[current_index] == bucket_state::occupied) {
                if (buckets[current_index] == key) {
                    return false;  // Key already exists
                }
            } else {  // empty or deleted
                if (insert_index == -1) {
                    insert_index = current_index;
                }
                if (states[current_index] == bucket_state::empty) {
                    break;  // Found empty slot, can stop searching for
                            // duplicates
                }
            }
        }

        if (insert_index != -1) {
            buckets[insert_index] = key;
            states[insert_index]  = bucket_state::occupied;
            count++;
            return true;
        }

        return false;  // Should not happen if count < buckets.count
    }

    // Removes a key. Returns true if the key was found and removed.
    bool remove(const Type& key) {
        int index = find_key(key);
        if (index != -1) {
            states[index] = bucket_state::deleted;
            count--;
            return true;
        }
        return false;
    }

    void clear() {
        count = 0;
        fill(states, bucket_state::empty);
    }

    int size() const { return count; }
    int capacity() const { return buckets.count; }

    // Const iterator to go through the elements in the set.
    struct iterator {
        const hash_set* set;
        int             index;

        void advance() {
            while (index < set->buckets.count &&
                   set->states[index] != bucket_state::occupied) {
                index++;
            }
        }

        iterator(const hash_set* s, int i) : set(s), index(i) { advance(); }

        iterator& operator++() {
            index++;
            advance();
            return *this;
        }

        bool operator!=(const iterator& other) const {
            return index != other.index;
        }

        const Type& operator*() const { return set->buckets[index]; }
    };

    typename hash_set<Type, Hasher>::iterator begin() const {
        return typename hash_set<Type, Hasher>::iterator(this, 0);
    }
    typename hash_set<Type, Hasher>::iterator end() const {
        return typename hash_set<Type, Hasher>::iterator(this, buckets.count);
    }
};

template <typename Type>
inline void copy_to(const hash_set<Type>& from, array<Type>& to) {
    to.resize(0);
    for (auto& v : from) to.push_back(v);
}
}  // namespace giacomo