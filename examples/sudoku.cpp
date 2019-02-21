#include "../csp.h"

CSP make_sudoku(int N) {
    auto range   = make_range(1, N * N + 1);
    auto counts  = allocate_array(N * N * N * N, N * N);  // N=3 -> 81 times 9
    auto domains = allocate_arrays<int>(counts, range);
    CSP  sudoku  = make_csp("Sudoku", domains, N * N * 3);

    auto row   = allocate_array<int>(N * N);
    auto col   = allocate_array<int>(N * N);
    auto block = allocate_array<int>(N * N);
    for (int k = 0; k < N * N; ++k) {
        int block_start = (k / N) * (N * N * N) + (k % N) * N;
        for (int i = 0; i < N * N; ++i) {
            row[i]   = k * N * N + i;
            col[i]   = i * N * N + k;
            block[i] = block_start + (i / N) * N * N + i % N;
        }
        sudoku.constraints.push_back(all_different(row, "row_diff"));
        sudoku.constraints.push_back(all_different(col, "col_diff"));
        sudoku.constraints.push_back(all_different(block, "block_diff"));
    }

    return sudoku;
}

Assignment parse_sudoku(const string& s, int N) {
    auto A = allocate_array<int>(N * N * N * N);
    for (int i = 0; i < N * N * N * N; i++) {
        int idx = i * 2 + 1;
        if (s[idx] != '-') A[i] = s[idx] - '0';
    }

    return A;
}

inline void print_sudoku(const Assignment& A, int N) {
    for (int i = 0; i < N * N * N * N; i++) {
        if (i % (N * N) == 0) printf("\n");
        if (A[i] > 0)
            printf(" %d", A[i]);
        else
            printf(" -");
    }
    printf("\n");
}

Assignment make_sudoku_hard() {
    return parse_sudoku(
        " 8 - - - - - - - -"
        " - - 3 6 - - - - -"
        " - 7 - - 9 - 2 - -"
        " - 5 - - - 7 - - -"
        " - - - - 4 5 7 - -"
        " - - - 1 - - - 3 -"
        " - - 1 - - - - 6 8"
        " - - 8 5 - - - 1 -"
        " - 9 - - - - 4 - -",
        3);
}

int main(int argc, char const* argv[]) {
    int N = 3;
    init_default_stack_allocator(10e7);
    CSP        csp  = make_sudoku(N);
    Assignment init = make_sudoku_hard();
    print_sudoku(init, N);

    search_stats stats;
    auto         solution = search(csp, init, stats);
    print_sudoku(solution, N);
    print_stats(stats);

    destroy_default_stack_allocator();
}
