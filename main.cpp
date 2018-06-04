#define PRINT_SEARCH
#include "csp.h"

CSP make_sudoku(const std::map<int,int> init) {
    auto domains = std::vector<Domain>(81, {1,2,3,4,5,6,7,8,9}); // Small sudoku, just one block.
    auto constraints = std::vector<Constraint>();
    CSP sudoku = make_csp("Sudoku", domains, constraints);
    
    for(auto kv : init) {
        add_contstraint(sudoku, equal_constant(kv.first, kv.second));
    }

    for (int k = 0; k < 9; ++k) {
        std::vector<int> row (9);
        std::vector<int> col (9);
        std::vector<int> block (9);
        int block_start = (k/3) * 27 + (k%3)*3;
        for (int i = 0; i < 9; ++i) {
            row[i] = k * 9 + i;
            col[i] = i * 9 + k;
            block[i] = block_start + (i/3) * 9 + i%3;
        }
        add_contstraint(sudoku, all_different(row) );
        add_contstraint(sudoku, all_different(col) );
        add_contstraint(sudoku, all_different(block) );
    }

    return sudoku;
}


CSP make_nqueens() {
    auto domains = std::vector<std::vector<int>>(8, {1,2,3,4,5,6,7,8} );
    CSP csp = make_csp("N-Queens", domains, {});
    add_contstraint(csp, all_different({0,1,2,3,4,5,6,7,8}, "alldiff"));
    
    for (int i = 0; i < 8-1; ++i) {
        for (int j = i+1; j < 8; ++j) {
            add_contstraint(csp, binary(i, j, [=](int x, int y) { return x - y != j - i; }, "+diag"));
            add_contstraint(csp, binary(i, j, [=](int x, int y) { return x - y != i - j; }, "-diag"));
        }
    }

    return csp;
}

int main(int argc, char const *argv[])
{
    int N = 9;
    auto domains = std::vector<std::vector<int>>(8, {1,2,3,4,5,6,7,8} );
    // CSP csp = make_sudoku({{0,1}, {10,2}, {20,3}, {30,4}, {40,5}, {50,6}, {60,7}, {70,8}, {80,9}});
    CSP csp = make_nqueens();
    print(csp);
    auto asg = search(csp);
    printf("\nsolution\n");
    print(asg);
    printf("\nnum_search: %d\n", num_search);

    // for(auto kv : asg) {
    //     if(kv.first%9 == 0) printf("\n");
    //     printf("%d  ", kv.second);
    // }
}
