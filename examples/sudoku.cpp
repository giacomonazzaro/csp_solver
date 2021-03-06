#include "../csp.h"

CSP make_sudoku(int N) {
    auto domains = allocate<array<int>>(N * N * N * N);
    for (auto& d : domains) d = make_range(1, N * N + 1);

    auto num_constraints = 3 * N * N;
    CSP  sudoku          = make_csp("Sudoku", domains, num_constraints);

    auto row   = allocate<int>(N * N);
    auto col   = allocate<int>(N * N);
    auto block = allocate<int>(N * N);
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

array<Domain> parse_sudoku(const string& s, int N) {
    auto A = allocate<array<int>>(N * N * N * N);
    for (int i = 0; i < N * N * N * N; i++) {
        int idx = i * 2 + 1;
        if (s[idx] != '-')
            A[i] = allocate({s[idx] - '0'});
        else
            A[i] = make_range(1, N * N + 1);
    }

    return A;
}

inline void print_sudoku(const array<Domain>& D, int N) {
    for (int i = 0; i < N * N * N * N; i++) {
        if (i % (N * N) == 0) printf("\n");
        if (D[i].count == 1)
            printf(" %d", D[i][0]);
        else
            printf(" -");
    }
    printf("\n");
}

array<Domain> make_sudoku_hard() {
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
    int  N              = 3;
    auto arena          = memory_arena(1e6);
    default_allocator() = stack_allocator{&arena, 0};

    CSP  csp  = make_sudoku(N);
    auto init = make_sudoku_hard();
    print_sudoku(init, N);

    search_stats stats;
    auto         success = search(csp.constraints, init, 0, stats);
    print_sudoku(init, N);
    print_stats(stats);
}
