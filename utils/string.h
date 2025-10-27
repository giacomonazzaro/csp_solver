#ifndef GIACOMO_STRING
#define GIACOMO_STRING

#include <stdio.h>
#include "array.h"

namespace giacomo {

#define STRING_SIZE (4096)

struct string : array<char> {
    char buffer[STRING_SIZE];

    string() {
        for (int i = 0; i < STRING_SIZE; i++) buffer[i] = '\0';
        data  = &buffer[0];
        count = 0;
    }

    string(const string& s) : string() {
        for (int i = 0; i < s.size(); i++) buffer[i] = s.buffer[i];
        count = s.size();
    }

    string(const char* literal) : string() {
        while (literal[count] != '\0') {
            buffer[count] = literal[count];
            count += 1;
        }
        buffer[count] = '\0';
    }

    string(char c) : string() { add(c); }

    string(char* literal) : string() {
        while (literal[count] != '\0') {
            buffer[count] = literal[count];
            count += 1;
        }
        buffer[count] = '\0';
    }

    void operator+=(const string& s) {
        for (int i = 0; i < s.size(); ++i) buffer[count + i] = s[i];
        count += s.size();
    }

    void operator+=(char c) { add(c); }

    operator const char*() const { return &buffer[0]; }
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

// inline const char* get_format(unsigned char) { return "%X"; }
// inline const char* get_format(char) { return "%s"; }
// inline const char* get_format(int) { return "%d"; }
// inline const char* get_format(long int) { return "%ld"; }
// inline const char* get_format(float) { return "%f"; }
// inline const char* get_format(double) { return "%lf"; }
// inline const char* get_format(void*) { return "%p"; }
// inline const char* get_format(const char*) { return "%s"; }

// template <typename Type>
// inline string to_string(const Type& val) {
//     auto result  = string();
//     auto format  = get_format(val);
//     result.size() = sprintf(result.buffer, format, val);
//     return result;
// }

inline string to_string(unsigned char val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%X", val);
    return result;
}

inline string to_string(char val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%c", val);
    return result;
}
inline string to_string(int val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%d", val);
    return result;
}
inline string to_string(long int val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%ld", val);
    return result;
}
inline string to_string(size_t val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%lu", val);
    return result;
}
inline string to_string(float val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%f", val);
    return result;
}
inline string to_string(double val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%lf", val);
    return result;
}
inline string to_string(void* val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%p", val);
    return result;
}
inline string to_string(const char* val) {
    auto result   = string();
    result.size() = sprintf(result.buffer, "%s", val);
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

inline void write_inline(const string& s, FILE* file) {
    fprintf(file, "%.*s", s.size(), s.buffer);
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
#endif
