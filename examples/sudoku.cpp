#include "../csp.h"

CSP make_sudoku(int N) {
    auto range = make_range<int>(1, N*N+1);
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


Assignment sudoku_hard() {
    return {
        {1,4}, {4,3}, {6,2}, {7,5},
        {9+3, 7}, {9+4, 4},
        {18+0, 6}, {18+6, 4}, {18+8, 3},
        {27+0, 1}, {27+8, 6},
        {36+2, 2}, {36+5, 7},
        {45+0, 9}, {45+5, 5}, {45+6, 7},
        {56+3, 2},
        {63+1, 3}, {63+7, 9}, {63+8, 1},
        {72+0, 4}, {72+3, 5}, {72+4, 1}, {72+6, 8}, {72+8, 2},
    };
}


Assignment parse_sudoku(const std::string s) {
    Assignment A;
    for(int i = 0; i<80; i++) 
        if(s[i] != '-')
            A[i] = s[i] - '0';

    return A;
}

Assignment sudoku_impossible() {
    return parse_sudoku(
        "8--------"
        "--36-----"
        "-7--9-2--"
        "-5---7---"
        "----457--"
        "---1---3-"
        "--1----68"
        "--85---1-"
        "-9----4--"
    );
}

Assignment sudoku_easy() {
    return {
        {0+1, 2}, 
        {9+0, 7}, {9+4, 8}, {9+5, 3}, {9+6, 6}, {9+7, 4},
        {18+0, 8}, {18+1, 1}, {18+5, 5}, {18+7, 3},
        {27+1, 9}, {27+2, 2}, {27+4, 3}, {27+7, 6}, {27+8, 8},
        {36+3, 8}, {36+5, 9},
        {45+0, 4}, {45+1, 8}, {45+4, 6}, {45+6, 1}, {45+7, 7},
        {54+1, 6}, {54+3, 2}, {54+7, 8}, {54+8, 3},
        {63+1, 4}, {63+2, 7}, {63+3, 3}, {63+4, 1}, {63+8, 6},
        {72+7, 1}
    };
}

void do_sudoku(int N = 3) {
    CSP csp = make_sudoku(N);
    Assignment init = sudoku_impossible();
    // Assignment init = sudoku_hard();
    print_sudoku(init, N);

    auto solution = search(csp, init);
    printf("solution\n");

    if(solution.size())
        print_sudoku(solution, N);
}

int main(int argc, char const *argv[]) {
    do_sudoku();
}
