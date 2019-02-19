#include "../csp.h"

CSP make_nqueens(int N = 8) {
    const Array<int> range   = make_range(N);
    auto             domains = Array<Domain>(N, range);
    CSP              csp     = make_csp("N-Queens", domains);
    add_constraint(csp, new AllDifferent(range, "one_per_column"));

    // Diagonal attack constraints.
    for (int i = 0; i < N - 1; ++i) {
        for (int j = i + 1; j < N; ++j) {
            add_constraint(csp, new Binary(i, j,
                                           [i, j](int x, int y) {
                                               return x + i != y + j;
                                           },
                                           "diag+"));
            add_constraint(csp, new Binary(i, j,
                                           [i, j](int x, int y) {
                                               return x - i != y - j;
                                           },
                                           "diag-"));
        }
    }
    return csp;
}

inline void print_nqueens(const Array<Domain>& D) {
    int N = D.size();
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < N; k++) {
            if (not contains(D[i], k))
                printf(" -");  // Unassigned and cannot be a queen.
            else {
                if (D[i].size() == 1)
                    printf(" X");  // There's a queen.
                else
                    printf(" o");  // Unassigned, but can be a queen.
            }
        }
        printf("\n");
    }
    printf("\n");
}

inline void print_nqueens(const Assignment& A) {
    int N = A.size();
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < N; k++) {
            if (A.at(i) == k)
                printf(" Q");  // There's a queen.
            else
                printf(" -");  // Empty cell.
        }
        printf("\n");
    }
    printf("\n");
}

Assignment make_assignment_from_list(Array<int> D, bool starts_at_one = false) {
    Assignment A;
    for (int i = 0; i < D.size(); i++) A[i] = D[i] - starts_at_one;
    return A;
}

int main(int argc, char const* argv[]) {
    int N = 20;
    if (argc == 2) N = atoi(argv[1]);

    CSP          csp = make_nqueens(N);
    search_stats stats;
    auto         solution = search(csp, {}, stats);
    print_nqueens(solution);
    print_stats(stats);
}
