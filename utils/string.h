#ifndef GIACOMO_STRING
#define GIACOMO_STRING

#include <stdio.h>
#include "array.h"

#define STRING_SIZE (4096)

inline int literal_length(const char* literal) {
    int len = 0;
    while (literal[len] != '\0') len += 1;
    return len;
}

struct string : array<char> {
    char buffer[STRING_SIZE];

    string() {
        for (int i = 0; i < STRING_SIZE; i++) buffer[i] = '\0';
        data  = &buffer[0];
        count = 0;
    }

    string(const string& s) : string() {
        for (int i = 0; i < s.count; i++) buffer[i] = s.buffer[i];
        count = s.count;
    }

    string(const char* literal) : string() {
        while (literal[count] != '\0') {
            buffer[count] = literal[count];
            count += 1;
        }
        buffer[count] = '\0';
    }

    void operator+=(const string& s) {
        for (int i = 0; i < s.count; ++i) buffer[count + i] = s[i];
        count += s.count;
    }

    operator const char*() const { return &buffer[0]; }
};

inline string operator+(const string& a, const string& b) {
    auto result = a;
    result += b;
    return result;
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
inline string to_string(Type val) {
    auto result  = string();
    auto format  = get_format(val);
    result.count = sprintf(result.buffer, format, val);
    return result;
}

template <typename Type>
inline void write(const Type& s, FILE* file = stdout) {
    write_inline(s, file);
    fprintf(file, "\n");
}

template <typename Type>
inline void write_inline(const Type& x) {
    auto s = to_string(x);
    write_inline(s);
}

inline void write_inline(const string& s, FILE* file) {
    fprintf(file, "%.*s", s.count, s.buffer);
}

#endif
