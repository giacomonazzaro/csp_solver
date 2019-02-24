#include "../csp.h"

CSP make_nqueens(int N = 8) {
    auto domains = allocate_array<array<int>>(N);
    for (auto& d : domains) d = make_range(N);

    auto num_constraints = N * N;
    CSP  csp             = make_csp("N-Queens", domains, num_constraints);

    // constraint: One queen per column.
    auto one_per_column = all_different(make_range(N), "one_per_column");
    csp.constraints.push_back(one_per_column);

    // constraint: No diagonal threats.
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

inline void print_nqueens(int N, const Assignment& A) {
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < N; k++) {
            if (k == A[i].value)
                printf(" Q");  // There's a queen.
            else
                printf(" -");  // Empty cell.
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char const* argv[]) {
    int N = 8;
    if (argc == 2) N = atoi(argv[1]);

    init_default_stack_allocator(10e8);

    CSP          csp = make_nqueens(N);
    search_stats stats;
    auto         solution = search(csp, {}, stats);
    print_nqueens(N, solution);
    print_stats(stats);

    destroy_default_stack_allocator();
}
