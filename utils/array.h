#ifndef GIACOMO_ARRAY
#define GIACOMO_ARRAY

#include <stdio.h>
#include <cassert>
#include <initializer_list>

namespace giacomo {

template <typename Type>
struct array {
    Type* data;
    int   count;

    inline const Type& operator[](int i) const { return data[i]; }
    inline Type&       operator[](int i) { return data[i]; }
    inline const Type& back() const { return data[count - 1]; }
    inline Type&       back() { return data[count - 1]; }
    inline void        add(const Type& val) { data[count++] = val; }

    inline int  size() const { return count; }
    inline void push_back(const Type& val) { add(val); }

    array() {
        data  = nullptr;
        count = 0;
    }

    // array(const Type* p, int c) : data(p), count(c) {}
    array(Type* p, int c) : data(p), count(c) {}

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

    inline void operator+=(const array<Type>& arr) { insert(arr, count); }

    inline void remove(int index) {
        for (int i = index; i < count - 1; ++i) {
            data[i] = data[i + 1];
        }
        count -= 1;
    }

    void operator=(const std::initializer_list<Type>& list) {
        count = (int)list.size();
        int i = 0;
        for (auto& v : list) {
            data[i++] = v;
        }
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

    // iterators
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

    struct mutable_iterator {
        Type* data;
        int   i;

        mutable_iterator& operator++() {
            i += 1;
            return *this;
        }
        bool  operator!=(mutable_iterator& other) const { return i != other.i; }
        Type& operator*() const { return *(data + i); }
    };
    inline mutable_iterator begin() { return mutable_iterator{data, 0}; }
    inline mutable_iterator end() { return mutable_iterator{data, count}; }
};

// fill array with a constant value
template <typename Type>
inline void fill(array<Type>& arr, const Type& val) {
    for (auto& x : arr) x = val;
}

// fill array with values returned by a function f: (int) -> Type
template <typename Type, typename Function>
inline void fill(array<Type>& arr, Function f) {
    for (int i = 0; i < arr.count; ++i) arr[i] = f(i);
}

// copy content of array to another array
template <typename Type>
inline void copy_to(const array<Type>& from, array<Type>& to) {
    assert(from.count <= to.count);
    for (int i = 0; i < from.count; ++i) to[i] = from[i];
    to.count = from.count;
}

// copy content of array of array to another array of arrays
template <typename Type>
inline void copy_to(const array<array<Type>>& from, array<array<Type>>& to) {
    assert(from.count <= to.count);
    for (int i = 0; i < from.count; ++i) copy_to(from[i], to[i]);
    to.count = from.count;
}

// get index of fisrt occurrence of some value
template <typename Type>
inline int find(const array<Type>& arr, const Type& value) {
    for (int i = 0; i < arr.count; ++i)
        if (arr[i] == value) return i;
    return -1;
}

// check if array contains some value
template <typename Type>
inline bool contains(const array<Type>& arr, const Type& value) {
    return find(arr, value) != -1;
}

template <typename Type>
inline bool operator==(const array<Type>& a, const array<Type>& b) {
    if (a.count != b.count) return false;
    for (int i = 0; i < a.count; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

}  // namespace giacomo

#endif
