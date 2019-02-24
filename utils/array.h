#pragma once
#include <cassert>
#include <initializer_list>

template <typename Type>
struct array {
    Type* data  = nullptr;
    int   count = 0;

    inline             operator bool() const { return data == nullptr; }
    inline const Type& operator[](int i) const { return data[i]; }
    inline Type&       operator[](int i) { return data[i]; }
    inline const Type& back() const { return data[count - 1]; }
    inline Type&       back() { return data[count - 1]; }
    inline int         size() const { return count; }
    inline void        push_back(const Type& val) { data[count++] = val; }
    inline void        add(const Type& val) { data[count++] = val; }

    inline void fill(const Type& val) {
        for (int i = 0; i < count; ++i) data[i] = val;
    }

    inline void insert(const Type& element, int index) {
        for (int i = count; i > index; i--) {
            data[i] = data[i - 1];
        }
        data[index] = element;
        count += 1;
    }

    inline void insert(const array<Type>& arr, int index) {
        for (int i = count; i > index; i--) {
            data[i] = data[i - arr.count];
        }
        for (int i = 0; i < arr.count; ++i) {
            data[index + i] = arr[i];
        }
        count += arr.count;
    }

    inline void append(const array<Type>& arr) { insert(arr, count); }

    inline void remove(int index) {
        for (int i = index; i < count - 1; ++i) {
            data[i] = data[i + 1];
        }
        count -= 1;
    }

    void operator=(const std::initializer_list<Type>& list) {
        count = (int)list.size();
        int i = 0;
        for (auto& v : list) data[i++] = v;
    }

    array<Type> slice(int from, int to) {
        assert(from >= 0 and from <= count);
        assert(to >= 0 and to <= count);
        assert(from <= to);
        return {data + from, to - from};
    }

    const array<const Type> slice(int from, int to) const {
        assert(from >= 0 and from <= count);
        assert(to >= 0 and to <= count);
        assert(from <= to);
        return {data + from, to - from};
    }

    // Iterators
    struct const_iterator {
        const Type* data;
        int         i;

        const_iterator& operator++() {
            i += 1;
            return *this;
        }
        bool operator!=(const const_iterator& other) const {
            return i != other.i;
        }
        const Type& operator*() const { return *(data + i); }
    };
    inline const_iterator begin() const { return const_iterator{data, 0}; }
    inline const_iterator end() const { return const_iterator{data, count}; }

    struct nonconst_iterator {
        Type* data;
        int   i;

        nonconst_iterator& operator++() {
            i += 1;
            return *this;
        }
        bool operator!=(nonconst_iterator& other) const { return i != other.i; }
        Type& operator*() const { return *(data + i); }
    };
    inline nonconst_iterator begin() { return nonconst_iterator{data, 0}; }
    inline nonconst_iterator end() { return nonconst_iterator{data, count}; }
};

template <typename Type>
inline void copy_to(const array<Type>& from, array<Type>& to) {
    assert(from.count <= to.count);
    for (int i = 0; i < from.count; ++i) to[i] = from[i];
    to.count = from.count;
}

template <typename Type>
inline void copy_to(const array<array<Type>>& from, array<array<Type>>& to) {
    assert(from.count <= to.count);
    for (int i = 0; i < from.count; ++i) copy_to(from[i], to[i]);
    to.count = from.count;
}

template <typename Type>
inline int find(const array<Type>& vec, const Type& val) {
    for (int i = 0; i < vec.count; ++i)
        if (vec[i] == val) return i;
    return -1;
}

template <typename Type>
inline bool contains(const array<Type>& vec, const Type& val) {
    return find(vec, val) != -1;
}

// template <typename Container>
// inline void print(const char* name, Container&& a, int line_size = 32,
//                   int max_elems = 300) {
//     int count = a.count;
//     printf("%s (count: %d)\n", name, count);  // capacity(a));
//     if (count)
//         printf(" ");
//     else {
//         printf("\n");
//         return;
//     }
//     for (int i = 0; i < count; ++i) {
//         printf("%d", a[i]);
//         if (i == count - 1)
//             printf("\n\n");

//         else if (i % line_size == line_size - 1)
//             printf("\n ");
//         else
//             printf(", ");

//         if (i > max_elems) {
//             printf("...\n\n");
//             return;
//         }
//     }
// }
