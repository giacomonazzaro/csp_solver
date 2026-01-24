#include <stdio.h>
#include <stdlib.h>

#include "../csp.h"

// #include "../utils/allocate.h"
// #include "../utils/memory_arena.h"

CSP make_map_coloring() {
    const int NUM_REGIONS = 7;
    const int NUM_COLORS  = 4;

    // Define regions as named integer constants
    const int western_australia  = 0;
    const int northern_territory = 1;
    const int south_australia    = 2;
    const int queensland         = 3;
    const int new_south_wales    = 4;
    const int victoria           = 5;
    const int tasmania           = 6;

    auto domains = allocate<array<int>>(NUM_REGIONS);
    for (auto& d : domains) {
        d = make_range(NUM_COLORS);
    }

    const int NUM_CONSTRAINTS = 10;
    CSP       csp = make_csp("Map Coloring", domains, NUM_CONSTRAINTS);

    auto different_colors = [](const Constraint& c, const array<int>& values) {
        return values[0] != values[1];
    };

    auto add_different_constraint = [&](int region1, int region2) {
        auto scope      = allocate({region1, region2});
        auto constraint = Constraint(Constraint::BINARY, scope, "different");
        constraint.eval_custom = different_colors;
        add_constraint(csp, constraint);
    };

    // Western Australia (0) is adjacent to Northern Territory (1) and South
    // Australia (2)
    add_different_constraint(western_australia, northern_territory);
    add_different_constraint(western_australia, south_australia);

    // Northern Territory (1) is adjacent to Western Australia (0), South
    // Australia (2), and Queensland (3)
    add_different_constraint(northern_territory, south_australia);
    add_different_constraint(northern_territory, queensland);

    // South Australia (2) is adjacent to Western Australia (0), Northern
    // Territory (1), Queensland (3), New South Wales (4), and Victoria (5)
    add_different_constraint(south_australia, queensland);
    add_different_constraint(south_australia, new_south_wales);
    add_different_constraint(south_australia, victoria);

    // Queensland (3) is adjacent to Northern Territory (1), South Australia
    // (2), and New South Wales (4)
    add_different_constraint(queensland, new_south_wales);

    // New South Wales (4) is adjacent to South Australia (2), Queensland (3),
    // and Victoria (5)
    add_different_constraint(new_south_wales, victoria);

    // Victoria (5) is adjacent to South Australia (2) and New South Wales (4)
    // Tasmania (6) is not adjacent to any other region

    // Impose initial color for Western Australia to Blue
    csp.domains[western_australia] = {2};

    return csp;
}

void print_solution(const Assignment& solution) {
    const char* region_names[] = {"Western Australia", "Northern Territory",
                                  "South Australia",   "Queensland",
                                  "New South Wales",   "Victoria",
                                  "Tasmania"};

    const char* colors[] = {"Red", "Green", "Blue", "Yellow"};

    for (const auto& assignment : solution) {
        printf("%s: %s\n", region_names[assignment.variable],
               colors[assignment.value]);
    }
}

void save_map_graph(const CSP& csp, const char* filename) {
    FILE* dot_file = fopen(filename, "w");
    if (!dot_file) {
        printf("Error opening %s\n", filename);
        return;
    }

    const char* region_names[] = {"Western_Australia", "Northern_Territory",
                                  "South_Australia",   "Queensland",
                                  "New_South_Wales",   "Victoria",
                                  "Tasmania"};

    const char* colors[] = {"red", "green", "blue", "yellow"};

    fprintf(dot_file, "graph Map {\n");
    fprintf(dot_file, "  node [style=filled];\n");

    for (int i = 0; i < csp.domains.size(); ++i) {
        if (csp.domains[i].size() == 1) {
            int color_idx = csp.domains[i][0];
            fprintf(dot_file, "  %s [fillcolor=%s];\n", region_names[i],
                    colors[color_idx]);
        } else {
            fprintf(dot_file, "  %s [fillcolor=white];\n", region_names[i]);
        }
    }

    for (const auto& constraint : csp.constraints) {
        if (constraint.type == Constraint::BINARY) {
            int region1_idx = constraint.scope[0];
            int region2_idx = constraint.scope[1];
            fprintf(dot_file, "  %s -- %s;\n", region_names[region1_idx],
                    region_names[region2_idx]);
        }
    }

    fprintf(dot_file, "}\n");
    fclose(dot_file);
    printf("Saved Graphviz file to %s\n", filename);
}

CSP make_random_graph_coloring() {
    const int    NUM_NODES        = 50;
    const int    NUM_COLORS       = 4;
    const double EDGE_PROBABILITY = 0.15;

    auto domains = allocate<array<int>>(NUM_NODES);
    for (auto& d : domains) {
        d = make_range(NUM_COLORS);
    }

    // First, count the number of constraints we'll have
    int num_constraints = 0;
    srand(0);  // Use a fixed seed for reproducibility
    for (int i = 0; i < NUM_NODES; ++i) {
        for (int j = i + 1; j < NUM_NODES; ++j) {
            if ((double)rand() / RAND_MAX < EDGE_PROBABILITY) {
                num_constraints++;
            }
        }
    }

    CSP csp = make_csp("Random Graph Coloring", domains, num_constraints);

    auto add_different_constraint = [&](int node1, int node2) {
        auto scope      = allocate({node1, node2});
        auto constraint = Constraint(Constraint::BINARY, scope, "different");
        constraint.eval_custom = [](const Constraint& c,
                                    const array<int>& values) {
            return values[0] != values[1];
        };
        add_constraint(csp, constraint);
    };

    // Reset seed to generate the same graph
    srand(0);
    for (int i = 0; i < NUM_NODES; ++i) {
        for (int j = i + 1; j < NUM_NODES; ++j) {
            if ((double)rand() / RAND_MAX < EDGE_PROBABILITY) {
                add_different_constraint(i, j);
            }
        }
    }

    return csp;
}

void print_random_graph_solution(const Assignment& solution) {
    const char* colors[] = {"Red", "Green", "Blue", "Yellow"};

    for (const auto& assignment : solution) {
        printf("Node %d: %s\n", assignment.variable, colors[assignment.value]);
    }
}

void save_random_graph(const CSP& csp, const char* filename) {
    FILE* dot_file = fopen(filename, "w");
    if (!dot_file) {
        printf("Error opening %s\n", filename);
        return;
    }

    const char* colors[] = {"red", "green", "blue", "cyan"};

    fprintf(dot_file, "graph RandomGraph {\n");
    fprintf(dot_file, "  node [style=filled];\n");

    for (int i = 0; i < csp.domains.size(); ++i) {
        char node_name[16];
        sprintf(node_name, "Node_%d", i);
        if (csp.domains[i].size() == 1) {
            int color_idx = csp.domains[i][0];
            fprintf(dot_file, "  %s [fillcolor=%s];\n", node_name,
                    colors[color_idx]);
        } else {
            fprintf(dot_file, "  %s [fillcolor=white];\n", node_name);
        }
    }

    for (const auto& constraint : csp.constraints) {
        if (constraint.type == Constraint::BINARY) {
            int  node1_idx = constraint.scope[0];
            int  node2_idx = constraint.scope[1];
            char node1_name[16];
            char node2_name[16];
            sprintf(node1_name, "Node_%d", node1_idx);
            sprintf(node2_name, "Node_%d", node2_idx);
            fprintf(dot_file, "  %s -- %s;\n", node1_name, node2_name);
        }
    }

    fprintf(dot_file, "}\n");
    fclose(dot_file);
    printf("Saved Graphviz file to %s\n", filename);
}

int main() {
    // auto arena          = memory_arena(1e8);
    // default_allocator() = stack_allocator{&arena, 0};

    CSP csp = make_random_graph_coloring();
    save_random_graph(csp, "map_initial.dot");

    search_stats stats;
    auto         solution = search(csp, {}, stats);

    if (solution.size() > 0) {
        printf("Solution found:\n");
        print_random_graph_solution(solution);
        // Update domains with solution for drawing
        for (const auto& assignment : solution) {
            csp.domains[assignment.variable] = {assignment.value};
        }
        save_random_graph(csp, "map_final.dot");
    } else {
        printf("No solution found.\n");
    }

    print_stats(stats);

    return 0;
}
