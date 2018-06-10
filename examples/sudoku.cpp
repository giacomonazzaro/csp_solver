#include "../csp.h"

CSP make_sudoku(int N) {
    auto range = make_range(1, N*N+1);
    auto domains = array<Domain>(N*N*N*N, range);
    CSP sudoku = make_csp("Sudoku", domains);

    for (int k = 0; k < N*N; ++k) {
        array<int> row (N*N);
        array<int> col (N*N);
        array<int> block (N*N);
        int block_start = (k/N) * (N*N*N) + (k%N)*N;
        for (int i = 0; i < N*N; ++i) {
            row[i] = k * N*N + i;
            col[i] = i * N*N + k;
            block[i] = block_start + (i/N) * N*N + i%N;
        }
        add(sudoku.constraints, all_different(row, "row_diff") );
        add(sudoku.constraints, all_different(col, "col_diff") );
        add(sudoku.constraints, all_different(block, "block_diff") );
    }

    return sudoku;
}


Assignment parse_sudoku(const std::string& s, int N) {
    Assignment A;
    for(int i = 0; i<N*N*N*N; i++) {
        int idx = i*2 + 1;
        if(s[idx] != '-')
            A[i] = s[idx] - '0';
    }

    return A;
}

inline void print_sudoku(const Assignment& A, int N) {
    for(int i = 0; i<N*N*N*N; i++) {
        if(i%(N*N) == 0) printf("\n");
        if(A.count(i) == 1)
            printf(" %d", A.at(i));
        else
            printf(" -");
    }
    printf("\n");
}

inline void print_sudoku(const array<Domain>& D) {
    print_sudoku(make_assignment(D), round(std::sqrt(std::sqrt(D.size()))));
}


Assignment sudoku_hard = parse_sudoku(
        " 8 - - - - - - - -"
        " - - 3 6 - - - - -"
        " - 7 - - 9 - 2 - -"
        " - 5 - - - 7 - - -"
        " - - - - 4 5 7 - -"
        " - - - 1 - - - 3 -"
        " - - 1 - - - - 6 8"
        " - - 8 5 - - - 1 -"
        " - 9 - - - - 4 - -", 3);


int main(int argc, char const *argv[]) {
    int N = 3;
    CSP csp = make_sudoku(N);
    Assignment init = sudoku_hard;
    print_sudoku(init, N);

    search_stats stats;
    auto solution = search(csp, init, stats);
    print_sudoku(solution, N);
    print_stats(stats);
}
