#include "../csp.h"

CSP make_knight_tour(int N = 6) {
    int  num_squares = N * N;
    auto domains      = allocate<array<int>>(num_squares);
    for (auto& d : domains) d = make_range(num_squares);

    // variable i = step i of the tour; value = square visited at that step.
    auto num_constraints = num_squares;
    CSP  csp = make_csp("Knight's Tour", domains, num_constraints);

    // constraint: every square is visited exactly once.
    add_constraint(csp, all_different(make_range(num_squares), "visit_all"));

    // constraint: consecutive steps must be a valid knight move apart.
    for (int i = 0; i < num_squares - 1; ++i) {
        auto scope        = allocate({i, i + 1});
        auto move         = Constraint(Constraint::BINARY, scope, "knight_move");
        move.constants    = allocate({N});
        move.eval_custom  = [](const Constraint& c, const array<int>& values) {
            int n  = c.constants[0];
            int dr = values[0] / n - values[1] / n;
            int dc = values[0] % n - values[1] % n;
            if (dr < 0) dr = -dr;
            if (dc < 0) dc = -dc;
            return (dr == 1 and dc == 2) or (dr == 2 and dc == 1);
        };
        add_constraint(csp, move);
    }

    // Start from a corner.
    csp.domains[0] = {0};

    return csp;
}

inline void print_knight_tour(int N, const Assignment& A) {
    auto step_at_square = allocate<int>(N * N, -1);
    for (auto& a : A) step_at_square[a.value] = a.variable;

    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) printf("%3d", step_at_square[row * N + col]);
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char const* argv[]) {
    int N = 6;
    if (argc == 2) N = atoi(argv[1]);

    CSP          csp = make_knight_tour(N);
    search_stats stats;
    auto         solution = search(csp, {}, stats);
    print_knight_tour(N, solution);
    print_stats(stats);
}
