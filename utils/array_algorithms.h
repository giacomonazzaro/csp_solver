#pragma once
#include "array.h"

template <typename Type>
bool equal(const array<Type>& a, const array<Type>& b) {
    if (a.count != b.count) return false;
    if (a.data == b.data) return true;
    return memcmp(a.data, b.data, a.count) == 0;
}

template <typename Type>
bool equal(const array<Type>& a, const array<Type>& b,
           bool (*eq)(const Type&, const Type&)) {
    if (a.count != b.count) return false;
    if (a.data == b.data) return true;
    for (int i = 0; i < a.count; ++i) {
        if (not eq(a[i], b[i])) return false;
    }
    return true;
}

template <typename Type>
int find(const array<Type>& vec, const Type& val) {
    for (int i = 0; i < vec.count; ++i)
        if (vec[i] == val) return i;
    return -1;
}

template <typename Type>
int contains(const array<Type>& vec, const Type& val) {
    return find(vec, val) != -1;
}

template <typename Type>
void apply(array<Type>& vec, bool (*f)(Type&)) {
    for (int i = 0; i < vec.count; ++i) {
        f(vec[i]);
    }
}

template <typename Type>
inline int argmax(const array<Type>& vec) {
    int max_idx = 0, i = 1;
    for (; i < vec.count; ++i) {
        if (vec[i] > vec[max_idx]) max_idx = i;
    }
    return i;
}

template <typename Type>
inline int argmin(const array<Type>& vec) {
    int min_idx = 0, i = 1;
    for (; i < vec.count; ++i) {
        if (vec[i] < vec[min_idx]) min_idx = i;
    }
    return i;
}

template <typename Type>
inline int min(const array<Type>& vec) {
    return vec[argmin(vec)];
}

template <typename Type>
inline int max(const array<Type>& vec) {
    return vec[argmax(vec)];
}

// @Todo!!!
// template <typename Container>
// void merge_sort(Container&& range);
// #import <vector>

template <typename Type>
void quick_sort(array<Type>&& arr) {
    int n = arr.count;
    if (n < 2) return;

    const auto& pivot = arr[n / 2];

    int i = 0, j = n - 1;
    while (true) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;

        if (i >= j) break;

        std::swap(arr[i], arr[j]);
        i++;
        j--;
    }

    quick_sort(arr(0, i));
    quick_sort(arr(i, n));
}

template <typename Type>
void bubble_sort(array<Type>& arr) {
    for (int i = 0; i < arr.count - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < arr.count - i - 1; j++)
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        if (not swapped) return;
    }
}

template <typename Type>
void heapify_shift_down(array<Type>& heap, int i, int max) {
    int i_big, c1, c2;
    while (i < max) {
        i_big = i;
        c1    = (2 * i) + 1;
        c2    = c1 + 1;
        if (c1 < max && heap[c1] > heap[i_big]) i_big = c1;
        if (c2 < max && heap[c2] > heap[i_big]) i_big = c2;
        if (i_big == i) return;
        std::swap(heap[i], heap[i_big]);
        i = i_big;
    }
}

template <typename Type>
void heapify(array<Type>& arr) {
    int i = (arr.count / 2) - 1;
    while (i >= 0) {
        heapify_shift_down(arr, i, arr.count);
        i -= 1;
    }
}

template <typename Type>
void heap_sort(array<Type>& arr) {
    heapify(arr);
    int end = arr.count - 1;
    while (end > 0) {
        std::swap(arr[0], arr[end]);
        heapify_shift_down(arr, 0, end);
        end -= 1;
    }
}

template <typename Type>
void reverse(array<Type>& arr) {
    int n = arr.count;
    for (int i = n / 2 - 1; i >= 0; i--) std::swap(arr[i], arr[n - i - 1]);
}
