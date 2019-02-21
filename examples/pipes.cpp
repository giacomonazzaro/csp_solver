#include "../csp.h"

// int& at(array<array<int>>& grid, int row, int col) {
//     return gridgrid[0].count * col + row;
// }

inline int get_bit(int number, int i) { return (number >> i) & 1; }

// L = 3
// ⅃ = 6
// ⎤= 12
// Γ = 9
// | = 10
// - = 5
// X = 15
//   = 0

CSP make_pipes(int N) {
    auto range = allocate_array({3, 6, 12, 9, 10, 5, 15, 0});
    // auto counts  = allocate_array(N * N, range.count);
    auto domains = allocate_arrays<int>(N * N, range);

    auto num_constraints = N * N * 4;
    CSP  csp             = make_csp("pipes", domains, num_constraints);

    // empty borders
    for (int x = 0; x < N; ++x) {
        csp.domains[x]               = {0};
        csp.domains[x * N]           = {0};
        csp.domains[x * N + (N - 1)] = {0};
        csp.domains[x + (N - 1) * N] = {0};
    }
    csp.domains[N * N / 2] = {15};

    for (int x = 1; x < N - 1; ++x) {
        for (int y = 1; y < N - 1; ++y) {
            int var     = N * x + y;
            int right   = var + 1;
            int up      = var - N;
            int left    = var - 1;
            int down    = var + N;
            int ruld[4] = {right, up, left, down};
            for (int k = 0; k < 4; ++k) {
                /* code */
                auto scope  = allocate_array({var, ruld[k]});
                auto c      = Constraint(BINARY, scope, "adj");
                c.constants = allocate_array({k});

                c.eval_custom = [](const Constraint& c,
                                   const array<int>& values) {
                    auto x = values[0];
                    auto y = values[1];
                    int  k = c.constants[0];
                    int  a = get_bit(x, k);
                    int  b = get_bit(y, (k + 2) % 4);
                    return a == b;
                };
                csp.constraints.push_back(c);
            }
        }
    }

    // auto one_per_column = all_different(range, "one_per_column");
    // csp.constraints.push_back(one_per_column);

    // Diagonal attack constraints.
    // for (int i = 0; i < N - 1; ++i) {
    //     for (int j = i + 1; j < N; ++j) {
    //         auto scope       = allocate_array({i, j});
    //         auto diag        = Constraint(BINARY, scope, "diag+");
    //         diag.constants   = allocate_array({j - i});
    //         diag.eval_custom = [](const Constraint& c,
    //                               const array<int>& values) {
    //             auto x = values[0];
    //             auto y = values[1];
    //             return abs(x - y) != abs(c.constants[0]);
    //         };

    //         csp.constraints.push_back(diag);
    //     }
    // }
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

inline void print_pipes(int N, const Assignment& A) {
    for (int x = 0; x < N; x++) {
        for (int k = 0; k < N; k++) {
            int i = x * N + k;

            if (A[i] == 3)
                printf(" L");
            else if (A[i] == 6)
                printf(" ⅃");
            else if (A[i] == 12)
                printf(" ⎤");
            else if (A[i] == 9)
                printf(" Γ");

            // | = 10
            // - = 5
            // X = 15
            //   = 0
            else if (A[i] == 10)
                printf(" |");
            else if (A[i] == 5)
                printf(" -");
            else if (A[i] == 15)
                printf(" X");
            else if (A[i] == 0)
                printf(" •");
            else
                printf("(%d)", A[i]);
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
    int N = 6;
    if (argc == 2) N = atoi(argv[1]);

    init_default_stack_allocator(10e8);

    CSP csp = make_pipes(N);

    // csp.domains[8] = {3};
    assert(satisfies(csp.constraints, csp.domains));
    search_stats stats;
    auto         solution = search(csp, {}, stats);
    print_pipes(N, solution);
    print_stats(stats);

    destroy_default_stack_allocator();
}
