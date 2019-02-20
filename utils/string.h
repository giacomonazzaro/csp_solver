#include "array.h"

#define STRING_SIZE (4096)

struct string : array<char> {
    char buffer[STRING_SIZE];

    string() : array<char>() {
        for (int i = 0; i < STRING_SIZE; i++) buffer[i] = '\0';
        data = &buffer[0];
    }

    string(const char* literal) : string() {
        count = (int)strlen(literal);
        for (int i = 0; i < count; ++i) buffer[i] = literal[i];
        buffer[count] = '\0';
    }
    void operator+=(const string& s) { this->append(s); }

    const char* c_str() const {
        data[count] = '\0';
        return data;
    }
};

inline string operator+(const string& a, const string& b) {
    auto result = a;
    result += b;
    return result;
}

inline void write(const string& s) { printf("%.*s\n", s.count, s.data); }

inline void write_inline(const string& s) { printf("%.*s", s.count, s.data); }

inline string to_string(int i) {
    auto result  = string();
    result.count = 100;
    sprintf(result.data, "%d", i);
    result.count = (int)strlen(result.data);
    return result;
}
