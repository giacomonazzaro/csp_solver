#include "csp.h"
// extern stack_allocator* default_allocator;
stack_allocator default_allocator;

// #include "csp_constraints.cpp"

// #define PRINT_SEARCH
// #define PRINT_SEARCH_LOG
// #define PRINT_SEARCH_GAC

void comment(const std::string& c) {
#ifdef PRINT_SEARCH_LOG
    printf("%s\n", c.c_str());
#endif
}

bool satisfies(const array<const Constraint*>& C, const array<Domain>& D) {
    for (auto& constraint : C) {
        if (not constraint->eval(D)) {
            return false;
        }
    }
    return true;
}

bool satisfies(const array<const Constraint*>& C, const Assignment& A) {
    stack_frame();
    auto D = make_domains(A);
    for (auto& c : C) {
        if (not c->eval(D)) {
            return false;
        }
    }
    return true;
}

bool is_assignment_complete(const array<Domain>& D) {
    for (int i = 0; i < D.size(); ++i)
        if (D[i].size() != 1) return false;

    return true;
}

bool do_inferences(const array<const Constraint*>& C, array<Domain>& D) {
    // Forward propagation.
    if (not constraints_propagation(C, D)) {
        comment("Propagation failure");
        return false;
    }

    // Generalized arc consistency.
    // if(not gac3(C, D)) {
    //     comment("GAC3 failure");
    //     return false;
    // }

    return true;
}

bool search(const array<const Constraint*>& C, array<Domain>& D, int depth,
            search_stats& stats) {
#ifdef PRINT_SEARCH
    print_state(D, depth);
#endif

    // If assignment is complete, just check if it satisfies contraints.
    if (is_assignment_complete(D)) {
        if (satisfies(C, D))
            return true;
        else
            return false;
    }

    int variable = choose_variable(D, C);

    for (int val : D[variable]) {
        stack_frame();

        stats.expansions += 1;
        // Copying the domains to make a temp version.
        auto D_attempt            = copy(D);
        D_attempt[variable][0]    = val;
        D_attempt[variable].count = 1;

        // Check if assignment satisfies constraints.
        if (not satisfies(C, D_attempt)) continue;

        // Propagate assignment and eventually reduce domains.
        if (not do_inferences(C, D_attempt)) continue;

        // Recursive call.
        bool success = search(C, D_attempt, depth + 1, stats);
        if (success) {
            copy_to(D_attempt, D);  // update the domains.
            return true;
        }
    }

    // Return failure. Backtrack.
    stats.backtracks += 1;
    return false;
}

Assignment search(const CSP& csp, Assignment A, search_stats& stats) {
    stack_frame();
    auto D = copy(csp.domains);
    
    for (auto& kv : A) D[kv.first] = {kv.second};

    if (A.size() > 0) {
        constraints_propagation(csp.constraints, D);
        // gac3(csp.constraints, D);
    }

    if (is_assignment_complete(D)) {
        if (not satisfies(csp.constraints, D)) {
            printf("No solution found! (search not needed)\n");
            print_unsatisfied(D, csp.constraints);
        }
        return make_assignment(D);
    }

    bool success = search(csp.constraints, D, 0, stats);
    A            = make_assignment(D);
    if (success) {
        bool check = satisfies(csp.constraints, D);
        if (not check) {
            printf("\n***** Search found a solution, but it's wrong! *****\n");
            print_unsatisfied(D, csp.constraints);
        }
        return A;
    } else {
        printf("No solution found!\n");
        return A;
    }
}

int choose_variable(const array<Domain>& D, const array<const Constraint*>& C) {
    // Choose following minimun remaining values heuristic.
    // Gradually update min_size and populate candidates with all
    // the variables that have domain size == min_size.
    stack_frame();
    auto candidates  = allocate_array<int>(D.size());
    candidates.count = 0;
    int min_size     = 9999999;  // @Hack.
    for (int i = 0; i < D.size(); i++) {
        auto size = D[i].size();
        if (size == 1) continue;
        if (size == min_size) candidates.push_back(i);
        if (size < min_size) {
            min_size   = size;
            candidates = {i};
        }
    }
    assert(candidates.size() > 0);

    // If no ties, return the variable.
    if (candidates.size() == 1) return candidates[0];

    // If there's a tie, use Max Degree heuristic.
    // Start with computing degrees. We could cache that, but it
    // is probably unexpensive to compute them on the fly (@Profile it).
    auto degrees = allocate_array(D.size(), 0);
    for (auto& c : C)
        for (int v : c->scope) degrees[v] += c->scope.size() - 1;

    int max_degree_idx = candidates[0];
    int max_degree     = degrees[max_degree_idx];
    for (int i = 1; i < candidates.size(); ++i) {
        int v = candidates[i];
        if (degrees[v] > max_degree) {
            max_degree_idx = v;
            max_degree     = degrees[v];
        }
    }
    assert(max_degree_idx < D.size());
    assert(D[max_degree_idx].size() > 1);
    return max_degree_idx;
}

bool constraints_propagation(const array<const Constraint*>& C,
                             array<Domain>&                  D) {
    for (auto& c : C) {
        if (not c->propagate(D)) return false;
    }
    return true;
}

bool remove_values(int variable, const Constraint* constraint,
                   array<Domain>& D) {
    stack_frame();
    bool removed_value = false;
    int  i             = 0;
    auto domain_tmp    = copy(D[variable]);  // copying the domain.
    auto ones          = allocate_array<int>(D.size(), 1);

    while (true) {
        stack_frame();
        // Make a fake copy of the domain. Will set the just interesting
        // variables.
        // array<Domain> Dfake = array<Domain>(D.size(), {-1});

        auto Dfake = allocate_arrays<int>(ones);
        for (auto& d : Dfake) d.count = 0;
        Dfake[variable] = {domain_tmp[i]};
        for (auto v : constraint->scope)
            if (v != variable) copy_to(D[v], Dfake[v]);  // copying the domains.

        bool exists = search_small(constraint, Dfake, 0);

        if (exists == false) {
            remove(domain_tmp, i);
            // removed_values.push_back(domain_tmp, i);
            removed_value = true;
        } else {
            i += 1;
        }

        if (i >= domain_tmp.size()) {
            copy_to(domain_tmp, D[variable]);
            return removed_value;
        }
    }
}

bool gac3(const array<const Constraint*>& C, array<Domain>& D_result) {
    array<Domain> D = D_result;  // copying the domains.
    array<int>    var_queue;
    array<int>    const_queue;

    // For each constraint c, for each variable v in the scope of c,
    // add the pair (v, c) to the queue.
    for (int i = 0; i < C.size(); ++i) {
        for (int v : C[i]->scope) {
            if (D[v].size() == 1) continue;
            var_queue.push_back(v);
            const_queue.push_back(i);
        }
    }

    // Consume the queue until it is empty.
    // For each pair (v, c), look if there exist a possible assignment
    // of the other varibles in the scope of c.
    while (var_queue.size() > 0) {
        int v = var_queue.back();
        int c = const_queue.back();
        var_queue.count -= 1;
        const_queue.count -= 1;

        bool removed_value_from_domain = remove_values(v, C[c], D);
        if (removed_value_from_domain) {
            // If the domain was left empty, this assignment cannot
            // be made complete. search() will read {} as failure.
            if (D[v].size() == 0) {
                return false;
            }

            // If we shrinked its domain, we add to the queue all
            // the variables that are neighbors of v through other constraints.
            for (int i = 0; i < C.size(); ++i) {
                if (i == c) continue;

                if (not contains(C[i]->scope, v)) continue;

                for (int w : C[i]->scope) {
                    if (w == v) continue;
                    if (D[w].size() == 1) continue;

                    // Check if it is already in queue.
                    bool already_in_queue = false;
                    for (int k = 0; k < var_queue.size(); k++) {
                        if (var_queue[k] == w and const_queue[k] == i) {
                            already_in_queue = true;
                            break;
                        }
                    }

                    if (not already_in_queue) {
                        var_queue.push_back(w);
                        const_queue.push_back(i);
                    }
                }
            }
        }
    }

    // Return the updated domain.
    D_result = D;  // copying the domains.
    return true;
}

bool search_small(const Constraint* c, const array<Domain>& D_, int depth) {
    stack_frame();

    auto D = copy(D_);
// Naive search that just check if there's a possible assignment that
// satisfy only ONE constraint-> Used by remove_values()
#ifdef PRINT_SEARCH_GAC
    print_state(D, depth);
#endif

    // If assignment is complete, return true.
    const array<int>& vars     = c->scope;
    bool              complete = true;
    for (int v : vars) {
        if (D[v].size() != 1) {
            complete = false;
            break;
        }
    }
    if (complete) return true;

    // Still using MRV & Max Degree.
    auto cc       = allocate_array<const Constraint*>(1, c);
    int  variable = choose_variable(D, cc);

    const auto domain = copy(D[variable]);
    for (int val : domain) {
        D[variable] = {val};

        // If new assignment does not satisfies constraints, continue.
        if (not c->eval(D)) continue;

        // @Speed: We should propagate also in search_small, but copying D seems
        // to slow down. array<Domain> D_new = D; if(not c->propagate(D_new))
        // continue;

        if (search_small(c, D, depth + 1)) {
            return true;
        }
    }

    return false;
}
