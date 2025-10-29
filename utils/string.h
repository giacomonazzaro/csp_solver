#pragma once

#if STD_ARRAY
#include <string>
#else
#include "array.h"
#endif

namespace giacomo {
#if STD_ARRAY
using string = std::string;
using std::to_string;
#else

struct string : array<char> {
    string() {
        this->data  = nullptr;
        this->count = 0;
    }

    explicit string(size_t size) {
        this->data  = new char[size];
        this->count = size;
    }

    string(const char* literal) {
        size_t len = 0;
        while (literal[len] != '\0') len++;
        this->data  = new char[len];
        this->count = len;
        for (size_t i = 0; i < len; ++i) this->data[i] = literal[i];
    }

    // Copy constructor
    string(const string& s) {
        this->data  = new char[s.count];
        this->count = s.count;
        for (size_t i = 0; i < s.count; ++i) this->data[i] = s.data[i];
    }

    // Move constructor
    string(string&& s) noexcept {
        this->data  = s.data;
        this->count = s.count;
        s.data      = nullptr;
        s.count     = 0;
    }

    // Copy assignment
    string& operator=(const string& s) {
        if (this != &s) {
            delete[] this->data;
            this->data  = new char[s.count];
            this->count = s.count;
            for (size_t i = 0; i < s.count; ++i) this->data[i] = s.data[i];
        }
        return *this;
    }

    // Move assignment
    string& operator=(string&& s) noexcept {
        if (this != &s) {
            delete[] this->data;
            this->data  = s.data;
            this->count = s.count;
            s.data      = nullptr;
            s.count     = 0;
        }
        return *this;
    }

    ~string() {
        delete[] this->data;
        this->data  = nullptr;
        this->count = 0;
    }

    void operator+=(const string& s) {
        char* data_new = new char[this->count + s.count];
        for (size_t i = 0; i < this->count; ++i) data_new[i] = this->data[i];
        for (size_t i = 0; i < s.count; ++i)
            data_new[this->count + i] = s.data[i];
        delete[] this->data;
        this->data = data_new;
        this->count += s.count;
    }

    operator const char*() const { return this->data; }

    size_t size() const { return this->count; }
};

inline string operator+(const string& a, const string& b) {
    auto result = a;
    result += b;
    return result;
}

inline int find(const array<char>& text, const array<char>& token) {
    auto index = -1;

here:
    for (int i = index + 1; i < text.size(); ++i) {
        if (text[i] == token[0]) {
            index = i;
            break;
        }
    }
    if (index == -1) return -1;
    for (int i = 1; i < token.size(); ++i) {
        if (text[index + i] != token[i]) goto here;
    }
    return index;
}

inline const char* get_format(unsigned char) { return "%X"; }
inline const char* get_format(char) { return "%s"; }
inline const char* get_format(int) { return "%d"; }
inline const char* get_format(long int) { return "%ld"; }
inline const char* get_format(float) { return "%f"; }
inline const char* get_format(double) { return "%lf"; }
inline const char* get_format(void*) { return "%p"; }
inline const char* get_format(const char*) { return "%s"; }

template <typename Type>
inline string to_string(const Type& val) {
    auto format = get_format(val);
    auto count  = snprintf(nullptr, 0, format, val);
    auto result = string(count + 1);
    sprintf(result.data, format, val);
    result.resize(count);
    return result;
}

inline string to_string(const string& val) { return val; }

template <typename Type>
inline string to_string(const array<Type>& val) {
    auto result = string("[");
    if (val.size() == 0) return result;
    for (int i = 0; i < val.size() - 1; i++) {
        result += to_string(val[i]) + ", ";
    }
    result += to_string(val.back()) + "]";
    return result;
}

#endif
inline void write_inline(const string& s, FILE* file) {
    // fprintf(file, "%.*s", s.size(), s.data);
    fprintf(file, "%.*s", s.size(), data(s));
}

template <typename Type>
inline void write_inline(const Type& x, FILE* file = stdout) {
    auto s = to_string(x);
    write_inline(s, file);
}

template <typename Type>
inline void write(const Type& s, FILE* file = stdout) {
    write_inline(s, file);
    fprintf(file, "\n");
}

}  // namespace giacomo
