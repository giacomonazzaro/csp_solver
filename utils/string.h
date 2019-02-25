#pragma once
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
        count = literal_length(literal);
        for (int i = 0; i < count; ++i) buffer[i] = literal[i];
        buffer[count] = '\0';
    }

    string(int i) : string() {
        count = 100;
        sprintf(buffer, "%d", i);
        count = literal_length(buffer);
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

inline void write(const string& s) { printf("%.*s\n", s.count, s.buffer); }

inline void write_inline(const string& s) { printf("%.*s", s.count, s.buffer); }
