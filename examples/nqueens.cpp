#include "../csp.h"

CSP make_nqueens(int N = 8) {
    const array<int> range = make_range(N);
    auto domains = array<Domain>(N, range);
    CSP csp = make_csp("N-Queens", domains);
    add_constraint(csp, new all_different(range, "alldiff"));
    
    // Diagonal attack constraints.
    for (int i = 0; i < N-1; ++i) {
        for (int j = i+1; j < N; ++j) {
            std::string name = "diag("+ std::to_string(i) + ", " + std::to_string(j) +")";
            add_constraint(csp, new binary(i, j, [i,j](int x, int y) { return x + i != y + j; }, "+"+name));
            add_constraint(csp, new binary(i, j, [i,j](int x, int y) { return x - i != y - j; }, "-"+name));
        }
    }

    return csp;
}

inline void print_nqueens(const array<Domain>& D) {
    int N = D.size();\
    for(int i = 0; i<N; i++) {
        for(int k = 0; k<N; k++) {
            if(not contains(D[i], k)) printf(" -"); // Unassigned and cannot be a queen.
            else {
                if(D[i].size() == 1)  printf(" Q"); // There's a queen.
                else                  printf(" o"); // Unassigned, but can be a queen.
            }
        }
        printf("\n");
    }
    printf("\n");
}

inline void print_nqueens(const Assignment& A) {
    int N = A.size();
    for(int i = 0; i<N; i++) {
        for(int k = 0; k<N; k++) {
            if(A.at(i) == k) printf(" Q"); // There's a queen.
            else             printf(" -"); // Empty cell.
        }
        printf("\n");
    }
    printf("\n");
}

Assignment make_assignment_from_list(array<int> D, bool starts_at_one = false) {
    Assignment A;
    for(int i=0; i<D.size(); i++)
        A[i] = D[i]-starts_at_one;
    return A;
}


int main(int argc, char const *argv[]) {
    int N = 13;
    if(argc == 2)
        N = atoi(argv[2]);

    CSP csp = make_nqueens(N);
    search_stats stats;
    auto solution = search(csp, {}, stats);
    print_nqueens(solution);
    print_stats(stats);
}
