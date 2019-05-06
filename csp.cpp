#include "csp.h"
stack_allocator giacomo::default_allocator;

bool satisfies(const array<Constraint>& C, const array<Domain>& D) {
    for (auto& constraint : C) {
        if (not eval(constraint, D)) {
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

bool do_inferences(const array<Constraint>& C, array<Domain>& D) {
    // Forward propagation.
    if (not constraints_propagation(C, D)) {
        return false;
    }

    // Generalized arc consistency.
    // if (not gac3(C, D)) {
    //     comment("GAC3 failure");
    // return false;
    // }

    return true;
}

bool search(const array<Constraint>& C, array<Domain>& D, int depth,
            search_stats& stats) {
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
        auto D_attempt      = copy(D);
        D_attempt[variable] = {val};

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

Assignment search(const CSP& csp, const Assignment& assignment,
                  search_stats& stats) {
    stack_frame();
    auto D = copy(csp.domains);

    apply_assignment(D, assignment);
    constraints_propagation(csp.constraints, D);

    if (is_assignment_complete(D)) {
        if (not satisfies(csp.constraints, D)) {
            printf("No solution found! (search not needed)\n");
            print_unsatisfied(D, csp.constraints);
        }
        return make_assignment(D);
    }

    bool success  = search(csp.constraints, D, 0, stats);
    auto solution = make_assignment(D);
    if (success) {
        bool check = satisfies(csp.constraints, D);
        if (not check) {
            printf("\n***** Search found a solution, but it's wrong! *****\n");
            print_unsatisfied(D, csp.constraints);
        }
        return solution;
    } else {
        printf("No solution found!\n");
        return solution;
    }
}

int choose_variable(const array<Domain>& D, const array<Constraint>& C) {
    // Choose following minimun remaining values heuristic.
    // Gradually update min_size and populate candidates with all
    // the variables that have domain size == min_size.
    stack_frame();
    auto candidates  = allocate<int>(D.size());
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
    auto degrees = allocate<int>(D.size(), 0);
    for (auto& c : C)
        for (int v : c.scope) degrees[v] += c.scope.size() - 1;

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

bool constraints_propagation(const array<Constraint>& C, array<Domain>& D) {
    for (auto& c : C)
        if (not propagate(c, D)) return false;

    return true;
}

bool remove_values(int variable, const Constraint& constraint,
                   array<Domain>& D) {
    stack_frame();
    bool removed_value = false;
    int  i             = 0;
    auto domain_tmp    = copy(D[variable]);  // copying the domain.
    auto ones          = allocate<int>(D.size(), 1);

    while (true) {
        stack_frame();
        // Make a fake copy of the domain. Will set the just interesting
        // variables.
        // array<Domain> Dfake = array<Domain>(D.size(), {-1});

        auto Dfake = allocate<array<int>>(D.size());
        for (int k = 0; k < Dfake.count; ++k) {
            Dfake[k]       = allocate<int>(D[k].count);
            Dfake[k].count = 0;
        }

        Dfake[variable].push_back(domain_tmp[i]);
        for (auto v : constraint.scope) {
            Dfake[v].count = D[v].count;
            if (v != variable) copy_to(D[v], Dfake[v]);  // copying the domains.
        }

        bool exists = search_single_constraint(constraint, Dfake, 0);

        if (exists == false) {
            domain_tmp.remove(i);
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

bool gac3(const array<Constraint>& C, array<Domain>& D_result) {
    stack_frame();
    auto D = copy(D_result);  // copying the domains.

    int size = 0;
    for (auto& c : C) size += c.scope.count;
    auto var_queue    = allocate<int>(size);
    auto const_queue  = allocate<int>(size);
    var_queue.count   = 0;
    const_queue.count = 0;

    // For each constraint c, for each variable v in the scope of c,
    // add the pair (v, c) to the queue.
    for (int i = 0; i < C.size(); ++i) {
        for (int v : C[i].scope) {
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

                if (not contains(C[i].scope, v)) continue;

                for (int w : C[i].scope) {
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
    copy_to(D, D_result);  // copying the domains.
    return true;
}

bool search_single_constraint(const Constraint& c, const array<Domain>& D_,
                              int depth) {
    stack_frame();
    auto D = copy(D_);

    // Naive search that just check if there's a possible assignment that
    // satisfy only ONE constraint. Used by remove_values().

    // If assignment is complete, return true. Only admissible assignments
    // arrive here.
    bool complete = true;
    for (int v : c.scope) {
        if (D[v].size() != 1) {
            complete = false;
            break;
        }
    }
    if (complete) return true;

    // Still using MRV & Max Degree.
    auto cc = allocate<Constraint>(1, c);
    // auto cc       = array<Constraint>{&c, 1};
    int variable = choose_variable(D, cc);

    const auto domain = copy(D[variable]);
    for (int val : domain) {
        D[variable] = {val};

        // If new assignment does not satisfies constraints, continue.
        if (not eval(c, D)) continue;

        // @Speed: We should propagate also in search_single_constraint, but
        // copying D seems to slow down. array<Domain> D_new = D; if(not
        // c->propagate(D_new)) continue;

        if (search_single_constraint(c, D, depth + 1)) {
            return true;
        }
    }

    return false;
}
