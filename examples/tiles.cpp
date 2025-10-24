#include <time.h>  // for rand() and srand()

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

CSP make_tiles(int N, bool tileable = true) {
    auto domains = allocate<array<int>>(N * N);
    auto domain  = allocate<int>(17);
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
            auto tile = tile_as_ascii(x);
        }
    }
    domain.push_back(15 + 16);
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

            if (tileable) {
                if (y == N - 1) adj[0] -= N;      // right
                if (x == N - 1) adj[1] -= N * N;  // down
                if (y == 0) adj[2] += N;          // left
                if (x == 0) adj[3] += N * N;      // up
            } else {
                if (y == N - 1) adj[0] = -1;
                if (x == N - 1) adj[1] = -1;
                if (y == 0) adj[2] = -1;
                if (x == 0) adj[3] = -1;
            }

            for (int k = 0; k < 4; ++k) {
                if (!tileable && adj[k] == -1) {
                    auto scope = allocate<int>({var});
                    auto c = Constraint(Constraint::UNARY, scope, "boundary");
                    c.constants   = allocate<int>({k});
                    c.eval_custom = [](const Constraint& c,
                                       const array<int>& values) {
                        auto x = values[0];
                        int  k = c.constants[0];
                        return get_bit(x, k) == 0;
                    };
                    csp.constraints.push_back(c);
                } else {
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
    }

    csp.domains[(N * N) / 2]         = {15};
    csp.domains[(N * N) / 2 - N - 1] = {15 + 16};
    csp.domains[(N * 2) - 2]         = {6};
    csp.domains[(N * 5) + 1]         = {8 + 1};
    // csp.domains[(N * 5) + 5] = {2 + 8};
    // csp.domains[10] = {3};
    // csp.domains[14] = {3};
    return csp;
}

inline void save_tiles_as_image(const array<int>& tiles, int N,
                                const std::string& filename) {
    // each tile is 20x20 pixels
    const int tile_size = 20;
    const int img_size  = N * tile_size;

    // create a blank image
    auto image = allocate<byte>(img_size * img_size * 3, (byte)255);

    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            int tile   = tiles[x * N + y];
            int rgb[3] = {rand(), rand(), rand()};
            for (int i = 0; i < tile_size; ++i) {
                for (int j = 0; j < tile_size; ++j) {
                    int img_y = y * tile_size + j;  // column
                    int img_x = x * tile_size + i;  // row
                    int idx   = (img_x * img_size + img_y) * 3;

                    // image[idx + 0] = 220 + rgb[0] % 30;
                    // image[idx + 1] = 220 + rgb[0] % 30;
                    // image[idx + 2] = 220 + rgb[0] % 30;

                    int thickness = 2;

                    // draw borders based on tile bits
                    auto down  = i >= tile_size / 2 - thickness;
                    auto up    = i < tile_size / 2 + thickness;
                    auto right = j >= tile_size / 2 - thickness;
                    auto left  = j < tile_size / 2 + thickness;

                    auto vertical_line   = right && left;
                    auto horizontal_line = up & down;

                    right &= horizontal_line;
                    left &= horizontal_line;
                    up &= vertical_line;
                    down &= vertical_line;
                    if (right && get_bit(tile, 0)) {  // right
                        image[idx + 0] = 0;
                        image[idx + 1] = 0;
                        image[idx + 2] = 0;
                    }
                    if (down && get_bit(tile, 1)) {  // down
                        image[idx + 0] = 0;
                        image[idx + 1] = 0;
                        image[idx + 2] = 0;
                    }
                    if (left && get_bit(tile, 2)) {  // left
                        image[idx + 0] = 0;
                        image[idx + 1] = 0;
                        image[idx + 2] = 0;
                    }
                    if (up && get_bit(tile, 3)) {  // up
                        image[idx + 0] = 0;
                        image[idx + 1] = 0;
                        image[idx + 2] = 0;
                    }
                    if (tile == 15) {
                        if (i == tile_size / 2 + (thickness / 2 + 1) ||
                            i == tile_size / 2 - (thickness / 2 + 2)) {
                            image[idx + 0] = 255;
                            image[idx + 1] = 255;
                            image[idx + 2] = 255;
                        }
                    }

                    if (tile == 15 + 16) {
                        if (j == tile_size / 2 + (thickness / 2 + 1) ||
                            j == tile_size / 2 - (thickness / 2 + 2)) {
                            image[idx + 0] = 255;
                            image[idx + 1] = 255;
                            image[idx + 2] = 255;
                        }
                    }
                }
            }
        }
    }

    // save the image as PPM
    FILE* file = fopen(filename.c_str(), "wb");
    fprintf(file, "P6\n%d %d\n255\n", img_size, img_size);
    fwrite(image.data, 1, img_size * img_size * 3, file);
    fclose(file);
    printf("Saved image to %s\n", filename.c_str());
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

int main(int argc, char const* argv[]) {
    srand((unsigned int)time(0));

    int N = 9;

    srand(time(nullptr));

    auto arena = memory_arena(1e8);

    default_allocator() = stack_allocator{&arena, 0};

    CSP csp = make_tiles(N, true);

    auto tiles_init = allocate<int>(N * N, 0);
    for (int i = 0; i < N * N; ++i) {
        if (csp.domains[i].size() == 1) {
            tiles_init[i] = csp.domains[i][0];
        }
    }
    save_tiles_as_image(tiles_init, N, "tiles_initial.ppm");

    search_stats stats;
    auto         assignment = search(csp, {}, stats);
    auto         tiles      = allocate<int>(N * N);
    for (auto& t : assignment) tiles[t.variable] = t.value;

    print_tiles(tiles, N);
    save_tiles_as_image(tiles, N, "tiles_output.ppm");
    print_stats(stats);
}
