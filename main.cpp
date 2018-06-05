#include "csp.h"

CSP make_sudoku() {
    auto domains = vector<Domain>(81, {1,2,3,4,5,6,7,8,9});
    CSP sudoku = make_csp("Sudoku", domains);

    for (int k = 0; k < 9; ++k) {
        vector<int> row (9);
        vector<int> col (9);
        vector<int> block (9);
        int block_start = (k/3) * 27 + (k%3)*3;
        for (int i = 0; i < 9; ++i) {
            row[i] = k * 9 + i;
            col[i] = i * 9 + k;
            block[i] = block_start + (i/3) * 9 + i%3;
        }
        add_contstraint(sudoku, all_different(row, "row_diff") );
        add_contstraint(sudoku, all_different(col, "col_diff") );
        add_contstraint(sudoku, all_different(block, "block_diff") );
    }

    return sudoku;
}

void print_sudoku(const Assignment& asg) {
    for(int i = 0; i<81; i++) {
        if(i%9 == 0) printf("\n");
        if(asg.count(i))
            printf(" %d", asg.at(i));
        else
            printf(" -");
    }
    printf("\n");
}

CSP make_nqueens(int N = 8) {
    auto domains = vector<vector<int>>(8, {1,2,3,4,5,6,7,8} );
    CSP csp = make_csp("N-Queens", domains);
    add_contstraint(csp, all_different({0,1,2,3,4,5,6,7}, "alldiff"));
    
    for (int i = 0; i < 8-1; ++i) {
        for (int j = i+1; j < 8; ++j) {
            std::string name = "diag("+ std::to_string(i) + ", " + std::to_string(j) +")";
            add_contstraint(csp, binary(i, j, [=](int x, int y) { return x - y != j - i; }, "+"+name));
            add_contstraint(csp, binary(i, j, [=](int x, int y) { return x - y != i - j; }, "-"+name));
        }
    }

    return csp;
}

void print_nqueens(const Assignment& asg) {
    for(int i = 0; i<8; i++) {
        for(int k = 0; k<8; k++) {
            if(asg.at(i)-1 == k)
                printf(" X");
            else
            printf(" •");
        }
        printf("\n");
    }
    printf("\n");
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

int main(int argc, char const *argv[])
{
    // CSP csp = make_nqueens();
    // Assignment init = {};
    
    CSP csp = make_sudoku();
    Assignment init = sudoku_hard();
    print_sudoku(init);

    auto solution = search(csp, init);
    printf("\nnum_search: %d\n", num_search);
    printf("solution\n");

    if(solution.size())
        // print_nqueens(solution);
        print_sudoku(solution);
}
