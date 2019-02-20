#pragma once
#include <cassert>
#include <initializer_list>
#include <memory>

template <typename Type>
struct array {
    Type* data  = nullptr;
    int   count = 0;

    // array() {}
    // array(Type* d, int c) : data(d), count(c) {}

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
        // memcpy(data + index + 1, data + index, (count - index) *
        // sizeof(Type));
        for (int i = count; i > index; i--) {
            data[i] = data[i - 1];
        }
        data[index] = element;
        count += 1;
    }

    inline void insert(const array<Type>& arr, int index) {
        memcpy(data + index + arr.count, data + index,
               (count - index - arr.count) * sizeof(Type));
        memcpy(data + index, arr.data, arr.count);
        count += arr.count;
    }

    void operator=(const std::initializer_list<Type>& list) {
        count = (int)list.size();
        memcpy(data, list.begin(), list.size() * sizeof(Type));
    }

    array<Type> operator()(int from, int to) {
        assert(from >= 0 and from <= count);
        assert(to >= 0 and to <= count);
        assert(from <= to);
        return {data + from, to - from};
    }

    const array<const Type> operator()(int from, int to) const {
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

   private:
    // array<Type>(const array<Type>&);
    // array<Type>& operator=(const array<Type>&);
};

template <typename Container>
inline void print(const char* name, Container&& a, int line_size = 32,
                  int max_elems = 300) {
    int count = a.count;
    printf("%s (count: %d)\n", name, count);  // capacity(a));
    if (count)
        printf(" ");
    else {
        printf("\n");
        return;
    }
    for (int i = 0; i < count; ++i) {
        printf("%d", a[i]);
        if (i == count - 1)
            printf("\n\n");

        else if (i % line_size == line_size - 1)
            printf("\n ");
        else
            printf(", ");

        if (i > max_elems) {
            printf("...\n\n");
            return;
        }
    }
}
