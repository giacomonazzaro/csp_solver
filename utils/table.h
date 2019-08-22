#pragma once
#include "array.h"

template <typename Type>
struct table_entry {
    Type value;
    int  index;
};

/*
 * Keys never change. The key of an entry is its initial position in the data
 * array when it is inserted. With follwing deletions and insetions, values are
 * moved such that they are at the beginning of data and one indirection through
 * indices does the rest.
 */

template <typename Type>
struct table {
    array<table_entry<Type>> data;
    array<int>               removed_keys;
    int                      num_indices = 0;
    int                      num_values  = 0;

    const Type& operator[](int key) const {
        assert(contains(key));
        return data[data[key].index].value;
    }

    Type& operator[](int key) {
        assert(contains(key));
        return data[data[key].index].value;
    }

    int insert(const Type& value) {
        int key = num_indices;
        if (num_indices != num_values) {
            assert(removed_keys.count > 0);
            key = removed_keys.back();
            removed_keys.count -= 1;
        } else {
            num_indices += 1;
        }

        data[key].index        = num_values;
        data[num_values].value = value;

        num_values += 1;
        return key;
    }

    void remove(int key) {
        // Always keep beginning of data full of values
        data[key].index = -1;
        if (key != num_values - 1) {
            data[key].value            = data[num_values - 1].value;
            data[num_values - 1].index = key;
        }
        num_values -= 1;
        removed_keys.add(key);
    }

    // void reorganize() {}

    inline bool find(int key) { return data[key].index; }
    inline bool contains(int key) { return find(key) != -1; }

    // Iterators
    struct const_iterator {
        struct entry {
            int         key;
            const Type& value;
        };
        const array<table_entry<Type>> data;
        int                            i;

        const_iterator& operator++() {
            i += 1;
            while (i < data.count and data[i].index == -1) i += 1;
            return *this;
        }
        bool operator!=(const const_iterator& other) const {
            return i != other.i;
        }
        entry operator*() const { return {i, data[data[i].index].value}; }
    };
    inline const_iterator begin() const { return const_iterator{data, 0}; }
    inline const_iterator end() const {
        return const_iterator{data, data.count};
    }

    struct nonconst_iterator {
        struct entry {
            int   key;
            Type& value;
        };
        array<table_entry<Type>> data;
        int                      i;

        nonconst_iterator& operator++() {
            i += 1;
            while (i < data.count and data[i].index == -1) i += 1;
            return *this;
        }
        bool operator!=(const nonconst_iterator& other) const {
            return i != other.i;
        }
        entry operator*() { return {i, data[data[i].index].value}; }
    };
    inline nonconst_iterator begin() { return nonconst_iterator{data, 0}; }
    inline nonconst_iterator end() {
        return nonconst_iterator{data, data.count};
    }
};