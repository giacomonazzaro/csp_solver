#include "../csp.h"

CSP make_nqueens(int N = 8) {
    auto range   = make_range(N);
    auto counts  = allocate_array(N, N);
    auto domains = allocate_arrays<int>(counts, range);

    auto num_constraints = N * N;
    CSP  csp             = make_csp("N-Queens", domains, num_constraints);

    auto one_per_column = all_different(range, "one_per_column");
    csp.constraints.push_back(one_per_column);

    // Diagonal attack constraints.
    for (int i = 0; i < N - 1; ++i) {
        for (int j = i + 1; j < N; ++j) {
            auto scope       = allocate_array({i, j});
            auto diag        = Constraint(BINARY, scope, "diag+");
            diag.constants   = allocate_array({j - i});
            diag.eval_custom = [](const Constraint& c,
                                  const array<int>& values) {
                auto x = values[0];
                auto y = values[1];
                return abs(x - y) != abs(c.constants[0]);
            };

            csp.constraints.push_back(diag);
        }
    }
    return csp;
}

inline void print_nqueens(const array<Domain>& D) {
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
    auto N = A.size();
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

Assignment make_assignment_from_list(const array<int>& D,
                                     bool              starts_at_one = false) {
    Assignment A;
    for (int i = 0; i < D.size(); i++) A[i] = D[i] - starts_at_one;
    return A;
}

int main(int argc, char const* argv[]) {
    int N = 80;
    if (argc == 2) N = atoi(argv[1]);

    init_default_stack_allocator(10e8);

    CSP          csp = make_nqueens(N);
    search_stats stats;
    auto         solution = search(csp, {}, stats);
    print_nqueens(solution);
    print_stats(stats);

    destroy_default_stack_allocator();
}
