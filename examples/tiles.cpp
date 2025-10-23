#include <string>

#include "../csp.h"

// int& at(array<array<int>>& grid, int row, int col) {
//     return gridgrid[0].count * col + row;
// }

inline int get_bit(int number, int i) { return (number >> i) & 1; }

struct Tile_Ascii {
    std::string top    = "   ";
    std::string mid    = "   ";
    std::string bottom = "   ";
};
inline Tile_Ascii tile_as_ascii(int tile) {
    auto result = Tile_Ascii{};
    if (tile != 0) result.mid[1] = '+';
    if (get_bit(tile, 0)) result.mid[2] = '-';
    if (get_bit(tile, 1)) result.bottom[1] = '|';
    if (get_bit(tile, 2)) result.mid[0] = '-';
    if (get_bit(tile, 3)) result.top[1] = '|';
    return result;
}

inline bool are_tiles_compatible(int tile_a, int tile_b, int direction) {
    // direction: 0 = right, 1 = down, 2 = left, 3 = up
    int bit_a = get_bit(tile_a, direction);
    int bit_b = get_bit(tile_b, (direction + 2) % 4);
    return bit_a == bit_b;
}

CSP make_tiles(int N) {
    auto domains = allocate<array<int>>(N * N);
    auto domain  = allocate<int>(16);
    domain.count = 0;
    for (int x = 0; x < 16; ++x) {
        auto count = 0;
        for (int k = 0; k < 4; ++k) {
            if (get_bit(x, k)) {
                count += 1;
            }
        }
        if (count != 1) {
            domain.push_back(x);
            // printf("Added tile %d\n", x);
            auto tile = tile_as_ascii(x);
            printf("\n %d:\n", x);
            printf("%s\n%s\n%s\n\n", tile.top.c_str(), tile.mid.c_str(),
                   tile.bottom.c_str());
        }
    }
    for (auto& d : domains) {
        d = copy(domain);
    }

    auto csp = make_csp("tiles", domains, N * N * 4);

    for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
            int  var = N * x + y;
            auto adj = allocate<int>({
                var + 1,  // right
                var + N,  // down
                var - 1,  // left
                var - N   // up
            });

            for (int k = 0; k < 4; ++k) {
                if (adj[k] < 0 or adj[k] >= N * N) continue;
                auto scope  = allocate<int>({var, adj[k]});
                auto c      = Constraint(Constraint::BINARY, scope, "adj");
                c.constants = allocate<int>({k});

                c.eval_custom = [](const Constraint& c,
                                   const array<int>& values) {
                    auto x = values[0];
                    auto y = values[1];
                    int  k = c.constants[0];
                    return are_tiles_compatible(x, y, k);
                };
                csp.constraints.push_back(c);
            }
        }
    }
    // Set {0} as domain for all tiles at the boundary of the image grid.
    for (int x = 0; x < N; ++x) {
        csp.domains[x]               = {0};
        csp.domains[x * N]           = {0};
        csp.domains[x * N + (N - 1)] = {0};
        csp.domains[x + (N - 1) * N] = {0};
    }

    csp.domains[21] = {15};
    return csp;
}
inline void print_tiles(const array<int>& tiles, int N) {
    for (int x = 0; x < N; x++) {
        for (int k = 0; k < N; k++) {
            int  i    = x * N + k;
            auto tile = tile_as_ascii(tiles[i]);
            printf("%s", tile.top.c_str());
        }
        printf("\n");
        for (int k = 0; k < N; k++) {
            int  i    = x * N + k;
            auto tile = tile_as_ascii(tiles[i]);
            printf("%s", tile.mid.c_str());
        }
        printf("\n");
        for (int k = 0; k < N; k++) {
            int  i    = x * N + k;
            auto tile = tile_as_ascii(tiles[i]);
            printf("%s", tile.bottom.c_str());
        }
        printf("\n");
    }
    printf("\n");
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

// Assignment make_assignment_from_list(const array<int>& D,
//                                      bool              starts_at_one = false)
//                                      {
//     Assignment A;
//     for (int i = 0; i < D.size(); i++) A[i] = D[i] - int(starts_at_one);
//     return A;
// }

int main(int argc, char const* argv[]) {
    int N = 6;
    // if (argc == 2) N = atoi(argv[1]);

    // init_default_stack_allocator(10e8);

    // CSP csp = make_tiles(N);

    // // csp.domains[8] = {3};
    // assert(satisfies(csp.constraints, csp.domains));
    // search_stats stats;
    // auto         solution = search(csp, {}, stats);
    // print_tiles(N, solution);
    // print_stats(stats);

    // destroy_default_stack_allocator();

    auto arena = memory_arena(1e8);

    default_allocator() = stack_allocator{&arena, 0};

    CSP          csp = make_tiles(N);
    search_stats stats;
    auto         assignment = search(csp, {}, stats);
    auto         tiles      = allocate<int>(N * N);
    for (auto& t : assignment) tiles[t.variable] = t.value;

    print_tiles(tiles, N);
    print_stats(stats);
}
