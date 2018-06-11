#include "../csp.h"

CSP make_nqueens(int N = 8) {
    const array<int> range = make_range(N);
    auto domains = array<Domain>(N, range);
    CSP csp = make_csp("N-Queens", domains);
    add_constraint(csp, all_different(range, "alldiff"));
    
    // Diagonal attack constraints.
    for (int i = 0; i < N-1; ++i) {
        for (int j = i+1; j < N; ++j) {
            std::string name = "diag("+ std::to_string(i) + ", " + std::to_string(j) +")";
            add_constraint(csp, binary(i, j, [i,j](int x, int y) { return x + i != y + j; }, "+"+name));
            add_constraint(csp, binary(i, j, [i,j](int x, int y) { return x - i != y - j; }, "-"+name));
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


template <typename Node, typename Problem>
Node steepest_descent(Problem prob, int max_sides) {
    printf("start!\n");
    Node node = get_start(prob);
    float cost = get_cost(node, prob);
    if(cost == 0)
        return node;

    int sides = 0;
    while (true) {
        bool found_better = false;
        bool side_move = false;
        
        // printf("conflicts: %d\n", (int)cost);
        // print_nqueens(node);
        
        array<Node> neighbors = get_neighbors(node, prob);

        for (Node neighbor : neighbors) {
            
            float new_cost = get_cost(neighbor, prob);

            if (new_cost == 0)
                return neighbor;
            
            if (new_cost < cost) {
                cost = new_cost;
                node = neighbor;
                found_better = true;
            }

            if (new_cost == cost)
                side_move = true;

        }

        if (found_better) continue;
        if (side_move and sides < max_sides) {
            sides++;
            continue;
        }

        // print_unsatisfied(make_domains(node), prob.constraints);
        return steepest_descent<Node, Problem>(prob, max_sides);
    }
    return node;
}


int num_conflicts(const Assignment& A, const array<Constraint>& C) {
    array<Domain> D (A.size());
    for(auto& kv : A) {
        Domain dom = {kv.second};
        D[kv.first] = dom;
    }
    int conflicts = 0;
    for(Constraint c : C) {
        if(not c.eval(D)) {
            //printf("%s\n", c.name.c_str());
            conflicts++;
        }
    }
    return conflicts;
}

float get_cost(const Assignment& A, const CSP& csp) {
    return (float) num_conflicts(A, csp.constraints);
}

Assignment get_start(const CSP& csp) {
    Assignment assignment;
    array<Domain> domains = csp.domains;
    for(int i=0; i < domains.size(); i++) {
        int index = rand() % domains[i].size();
        assignment[i] = domains[i][index];
    }
    return assignment;
}

array<Assignment> get_neighbors(const Assignment& A, const CSP& csp) {
    array<Assignment> result;
    for (int i=0; i < csp.domains.size(); ++i) {
        for (int val : csp.domains[i]) {
            Assignment new_assignment = A;
            new_assignment[i] = val;
            result.push_back(new_assignment);
        }
    }
    return result;
}

int main(int argc, char const *argv[]) {
    int N = 12;
    if(argc == 2)
        N = atoi(argv[2]);

    CSP csp = make_nqueens(N);
    auto A = steepest_descent<Assignment, CSP>(csp, 10);
    print_nqueens(A);
    print_unsatisfied(make_domains(A), csp.constraints);
}
