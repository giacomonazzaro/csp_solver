#include <vector>
#include "array.h"
using namespace giacomo;

template <typename Type>
struct Array : std::vector<Type> {
    Array(size_t capacity) : std::vector<Type>(capacity) {}
    Array(size_t capacity, const Type& value)
        : std::vector<Type>(capacity, value) {}

    inline void add(const Type& val) { push_back(val); }

    operator array<Type>() const {
        return array<Type>((Type*)this->data(), this->size());
    }

    inline void insert(const Type& element, int index) {
        std::vector<Type>::insert(this->begin() + index, element);
    }

    inline void insert(const array<Type>& arr, int index) {
        std::vector<Type>::insert(this->begin() + index, arr.data,
                                  arr.data + arr.count);
    }

    inline void operator+=(const array<Type>& arr) {
        std::vector<Type>::insert(this->end(), arr.data, arr.data + arr.count);
    }

    inline void remove(int index) { erase(this->begin() + index); }
};