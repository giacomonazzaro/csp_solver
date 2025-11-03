#include "../csp.h"
// #include "../utils/allocate.h"
// #include "../utils/memory_arena.h"

CSP make_map_coloring() {
    const int NUM_REGIONS = 7;
    const int NUM_COLORS = 4;

    // Define regions as named integer constants
    const int western_australia = 0;
    const int northern_territory = 1;
    const int south_australia = 2;
    const int queensland = 3;
    const int new_south_wales = 4;
    const int victoria = 5;
    const int tasmania = 6;

    auto domains = allocate<array<int>>(NUM_REGIONS);
    for (auto& d : domains) {
        d = make_range(NUM_COLORS);
    }

    const int NUM_CONSTRAINTS = 10;
    CSP csp = make_csp("Map Coloring", domains, NUM_CONSTRAINTS);

    auto different_colors = [](const Constraint& c, const array<int>& values) {
        return values[0] != values[1];
    };

    auto add_different_constraint = [&](int region1, int region2) {
        auto scope = allocate({region1, region2});
        auto constraint = Constraint(Constraint::BINARY, scope, "different");
        constraint.eval_custom = different_colors;
        add_constraint(csp, constraint);
    };

    // Western Australia (0) is adjacent to Northern Territory (1) and South Australia (2)
    add_different_constraint(western_australia, northern_territory);
    add_different_constraint(western_australia, south_australia);

    // Northern Territory (1) is adjacent to Western Australia (0), South Australia (2), and Queensland (3)
    add_different_constraint(northern_territory, south_australia);
    add_different_constraint(northern_territory, queensland);

    // South Australia (2) is adjacent to Western Australia (0), Northern Territory (1), Queensland (3), New South Wales (4), and Victoria (5)
    add_different_constraint(south_australia, queensland);
    add_different_constraint(south_australia, new_south_wales);
    add_different_constraint(south_australia, victoria);

    // Queensland (3) is adjacent to Northern Territory (1), South Australia (2), and New South Wales (4)
    add_different_constraint(queensland, new_south_wales);

    // New South Wales (4) is adjacent to South Australia (2), Queensland (3), and Victoria (5)
    add_different_constraint(new_south_wales, victoria);

    // Victoria (5) is adjacent to South Australia (2) and New South Wales (4)
    // Tasmania (6) is not adjacent to any other region

    return csp;
}

void print_solution(const Assignment& solution) {
    const char* region_names[] = {
        "Western Australia",
        "Northern Territory",
        "South Australia",
        "Queensland",
        "New South Wales",
        "Victoria",
        "Tasmania"
    };

    const char* colors[] = {"Red", "Green", "Blue", "Yellow"};

    for (const auto& assignment : solution) {
        printf("%s: %s\n", region_names[assignment.variable], colors[assignment.value]);
    }
}

int main() {
    auto arena = memory_arena(1e8);
    default_allocator() = stack_allocator{&arena, 0};

    CSP csp = make_map_coloring();
    search_stats stats;
    auto solution = search(csp, {}, stats);

    if (solution.size() > 0) {
        printf("Solution found:\n");
        print_solution(solution);
    } else {
        printf("No solution found.\n");
    }

    print_stats(stats);

    return 0;
}
