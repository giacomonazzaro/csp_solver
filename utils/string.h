#include "array.h"

#define STRING_SIZE (4096)

struct string : array<char> {
    char buffer[STRING_SIZE];

    string() : array<char>() {
        for (int i = 0; i < STRING_SIZE; i++) buffer[i] = '\0';
        data = &buffer[0];
    }

    string(const string& s) : string() {
        for (int i = 0; i < s.count; i++) buffer[i] = s.buffer[i];
        count = s.count;
    }

    string(const char* literal) : string() {
        count = (int)strlen(literal);
        for (int i = 0; i < count; ++i) buffer[i] = literal[i];
        buffer[count] = '\0';
    }

    string(int i) : string() {
        count = 100;
        sprintf(buffer, "%d", i);
        count = (int)strlen(buffer);
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
