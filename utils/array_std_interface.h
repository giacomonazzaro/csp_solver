#pragma once
#include <cassert>
#include <vector>

namespace giacomo {

template <typename Type>
struct array : std::vector<Type> {
    array() : std::vector<Type>() {}
    array(size_t capacity) : std::vector<Type>(capacity) {}
    array(size_t capacity, const Type& value)
        : std::vector<Type>(capacity, value) {}
    ~array() {}

    inline void add(const Type& val) { push_back(val); }

    operator array<Type>() const {
        return array<Type>((Type*)this->data(), this->size());
    }

    inline void insert(const Type& element, int index) {
        std::vector<Type>::insert(this->begin() + index, element);
    }

    inline void insert(const array<Type>& arr, int index) {
        std::vector<Type>::insert(this->begin() + index, arr.data,
                                  arr.data + arr.size());
    }

    inline void operator+=(const array<Type>& arr) {
        std::vector<Type>::insert(this->end(), arr.data, arr.data + arr.size());
    }

    inline void remove(int index) { erase(this->begin() + index); }

    void operator=(const std::initializer_list<Type>& list) {
        *this = std::vector<Type>(list);
    }

    inline Type pop() {
        assert(this->size() > 0);
        Type val = this->back();
        this->pop_back();
        return val;
    }
};

template <typename Type>
inline array<Type> allocate(int count) {
    return array<Type>(count);
}

template <typename Type>
inline array<Type> allocate(int count, const Type& value) {
    return array<Type>(count, value);
}

template <typename Type>
inline array<Type> allocate(const std::initializer_list<Type>& list) {
    auto result = array<Type>(list);
    return result;
}

// fill array with a constant value
template <typename Type>
inline void fill(array<Type>& arr, const Type& val) {
    for (auto& x : arr) x = val;
}

// fill array with values returned by a function f: (int) -> Type
template <typename Type, typename Function>
inline void fill(array<Type>& arr, Function f) {
    for (int i = 0; i < arr.size(); ++i) arr[i] = f(i);
}

// copy content of array to another array
template <typename Type>
inline void copy_to(const array<Type>& from, array<Type>& to) {
    assert(from.size() <= to.size());
    for (int i = 0; i < from.size(); ++i) to[i] = from[i];
    to.resize(from.size());
}

// copy content of array to another array
template <typename Type>
inline array<Type> copy(const array<Type>& from) {
    auto result = from;
    return result;
}

// copy content of array of array to another array of arrays
template <typename Type>
inline void copy_to(const array<array<Type>>& from, array<array<Type>>& to) {
    assert(from.size() <= to.size());
    for (int i = 0; i < from.size(); ++i) copy_to(from[i], to[i]);
    to.size() = from.size();
}

// get index of fisrt occurrence of some value
template <typename Type>
inline int find(const array<Type>& arr, const Type& value) {
    for (int i = 0; i < arr.size(); ++i)
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
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

inline int argmin(const array<int>& arr) {
    assert(arr.size() > 0);
    int min_index = 0;
    for (int i = 1; i < arr.size(); ++i) {
        if (arr[i] < arr[min_index]) {
            min_index = i;
        }
    }
    return min_index;
}

inline int argmax(const array<int>& arr) {
    assert(arr.size() > 0);
    int max_index = 0;
    for (int i = 1; i < arr.size(); ++i) {
        if (arr[i] > arr[max_index]) {
            max_index = i;
        }
    }
    return max_index;
}

inline void shuffle(array<int>& arr) {
    for (int i = arr.size() - 1; i > 0; --i) {
        int j   = rand() % (i + 1);
        int tmp = arr[i];
        arr[i]  = arr[j];
        arr[j]  = tmp;
    }
}

inline void write(const char* s) { printf("%s\n", s); }
}  // namespace giacomo