#include "csp.h"

void add_constraint(CSP& csp, const Constraint& constraint) {
    csp.constraints.push_back(constraint);
    for (auto var : constraint.scope) {
        csp.variable_to_constraints[var].push_back(csp.constraints.size() - 1);
    }
}

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

    auto values = copy(D[variable]);
    shuffle(values);
    for (int val : values) {
        stack_frame();
        stats.expansions += 1;

        // Copying the domains to make a temp version.
        auto D_attempt = copy(D);

        D_attempt[variable] = {val};

        // Check if assignment satisfies constraints.
        if (not satisfies(C, D_attempt)) continue;

        // Propagate assignment and eventually reduce domains.
        // if (not do_inferences(C, D_attempt)) continue;
        if (not constraints_propagation(C, D_attempt)) {
            continue;
        }

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
    auto candidates = allocate<int>(D.size());
    candidates.resize(0);
    int min_size = 9999999;  // @Hack.
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
    stack_frame();
    auto was_variable_updated = allocate<bool>(D.size(), false);

    for (auto& c : C) {
        stack_frame();
        if (not propagate(c, D, was_variable_updated)) {
            return false;
        }
    }
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
        for (int k = 0; k < Dfake.size(); ++k) {
            Dfake[k] = allocate<int>(D[k].size());
            Dfake[k].resize(0);
        }

        Dfake[variable].push_back(domain_tmp[i]);
        for (auto v : constraint.scope) {
            Dfake[v].resize(D[v].size());
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

// bool gac3(const array<Constraint>& C, array<Domain>& D_result) {
//     auto total_result = Propagation_Result(D_result.size());

//     stack_frame();
//     auto D = copy(D_result);  // copying the domains.

//     int size = 0;
//     for (auto& c : C) size += c.scope.size();
//     auto var_queue   = allocate<int>(size);
//     auto const_queue = allocate<int>(size);
//     var_queue.resize(0);
//     const_queue.resize(0);

//     // For each constraint c, for each variable v in the scope of c,
//     // add the pair (v, c) to the queue.
//     for (int i = 0; i < C.size(); ++i) {
//         for (int v : C[i].scope) {
//             if (D[v].size() == 1) continue;
//             var_queue.push_back(v);
//             const_queue.push_back(i);
//         }
//     }

//     // Consume the queue until it is empty.
//     // For each pair (v, c), look if there exist a possible assignment
//     // of the other varibles in the scope of c.
//     while (var_queue.size() > 0) {
//         int v = var_queue.back();
//         int c = const_queue.back();
//         var_queue.pop();
//         const_queue.pop();

//         int  original_domain_size_v    = D[v].size();
//         bool removed_value_from_domain = remove_values(v, C[c], D);

//         if (removed_value_from_domain) {
//             if (D[v].size() < original_domain_size_v) {
//                 total_result.was_variable_updated[v] = true;
//             }

//             // If the domain was left empty, this assignment cannot
//             // be made complete. search() will read {} as failure.
//             if (D[v].size() == 0) {
//                 total_result.valid = false;
//                 return total_result;  // Domain wipeout
//             }

//             // If we shrinked its domain, we add to the queue all
//             // the variables that are neighbors of v through other
//             // constraints. for (int i = 0; i < C.size(); ++i) {
//             //     if (i == c) continue;

//             //     if (not contains(C[i].scope, v)) continue;

//             //     for (int w : C[i].scope) {
//             //         if (w == v) continue;
//             //         if (D[w].size() == 1) continue;

//             //         // Check if it is already in queue.
//             //         bool already_in_queue = false;
//             //         for (int k = 0; k < var_queue.size(); k++) {
//             //             if (var_queue[k] == w and const_queue[k] == i) {
//             //                 already_in_queue = true;
//             //                 break;
//             //             }
//             //         }

//             //         if (not already_in_queue) {
//             //             var_queue.push_back(w);
//             //             const_queue.push_back(i);
//             //         }
//             //     }
//             // }
//         }
//     }

//     // Return the updated domain.
//     copy_to(D, D_result);  // copying the domains.
//     return total_result;
// }

bool search_single_constraint(const Constraint& c, const array<Domain>& D_,
                              int depth) {
    // Naive search that just check if there's a possible assignment that
    // satisfy only ONE constraint. Used by remove_values().

    stack_frame();
    auto D = copy(D_);

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
    auto cc       = allocate<Constraint>(1, c);
    int  variable = choose_variable(D, cc);

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

Constraint::Constraint() {}
Constraint::Constraint(enum type t, const array<int>& vars, string s)
    : type(t), scope(copy(vars)), name(s), constants({}) {
    type  = t;
    scope = copy(vars);
    name  = s;
    name += "(";

    for (int i = 0; i < scope.size() - 1; ++i) {
        name += to_string(scope[i]);
        name += ", ";
    }
    name += to_string(scope.back());
    name += ")";
}

CSP make_csp(const string& name, const array<Domain>& domains,
             int num_constraints) {
    CSP csp;
    csp.name        = name;
    csp.domains     = domains;
    csp.constraints = allocate<Constraint>(num_constraints);
    csp.constraints.resize(0);

    csp.variable_to_constraints = allocate<array<int>>(domains.size());
    for (int i = 0; i < domains.size(); ++i) {
        csp.variable_to_constraints[i] = allocate<int>(num_constraints);
        csp.variable_to_constraints[i].resize(0);
    }
    return csp;
}

array<int> make_range(int from, int to) {
    auto result = allocate<int>(to - from);
    for (int i = 0; i < to - from; i++) result[i] = from + i;
    return result;
}

array<int> make_range(int to) { return make_range(0, to); }

Assignment make_assignment(const array<Domain>& D) {
    auto A = allocate<assignment>(D.size());
    A.resize(0);
    for (int i = 0; i < D.size(); i++) {
        if (D[i].size() == 1) A.push_back({i, D[i][0]});
    }
    return A;
}

void apply_assignment(array<Domain>& D, const Assignment& A) {
    for (auto& a : A) D[a.variable] = {a.value};
}

void print_stats(const search_stats& stats) {
    printf("\nSearch statistics:\n");
    printf("   num_backtracks = %d\n", stats.backtracks);
    printf("   num_expansions = %d\n\n", stats.expansions);
}

Constraint all_different(const array<int>& scope,
                         const string&     name = "all_different") {
    auto result = Constraint(Constraint::ALL_DIFFERENT, scope, name);
    return result;
}

bool eval_all_different(const Constraint& constraint, const array<Domain>& D) {
    for (int i = 0; i < constraint.scope.size() - 1; ++i) {
        int v = constraint.scope[i];
        if (D[v].size() != 1) continue;
        for (int k = i + 1; k < constraint.scope.size(); ++k) {
            int w = constraint.scope[k];
            if (D[w].size() == 1)
                if (D[v][0] == D[w][0]) return false;
        }
    }
    return true;
}

bool propagate_all_different(const Constraint& constraint, array<Domain>& D,
                             array<bool>& was_variable_updated) {
    for (int v : constraint.scope) {
        if (D[v].size() != 1) continue;
        for (int w : constraint.scope) {
            if (w == v) continue;
            for (int i = 0; i < D[w].size(); ++i) {
                if (D[w][i] == D[v][0]) {
                    D[w].remove(i);
                    was_variable_updated[w] = true;
                    if (D[w].size() == 0) {
                        return false;  // Domain wipeout.
                    }
                    break;
                }
            }
        }
    }
    return true;
}

bool eval_unary(const Constraint& constraint, const array<Domain>& domains) {
    int x = constraint.scope[0];
    if (domains[x].size() == 1) {
        stack_frame();
        auto value = allocate<int>({domains[x][0]});
        if (not constraint.eval_custom(constraint, value)) return false;
    }
    return true;
}

bool eval_binary(const Constraint& constraint, const array<Domain>& domains) {
    int x = constraint.scope[0];
    int y = constraint.scope[1];
    if (domains[x].size() == 1 and domains[y].size() == 1) {
        stack_frame();
        auto xy = allocate({domains[x][0], domains[y][0]});
        if (not constraint.eval_custom(constraint, xy)) return false;
    }
    return true;
}

bool propagate_unary(const Constraint& constraint, array<Domain>& D,
                     array<bool>& was_variable_updated) {
    int x                    = constraint.scope[0];
    int original_domain_size = D[x].size();

    stack_frame();
    auto domain_new = allocate<int>(D[x].size());
    domain_new.resize(0);
    for (auto value : D[x]) {
        stack_frame();
        auto v = allocate({value});
        if (constraint.eval_custom(constraint, v)) {
            domain_new.push_back(value);
        }
    }
    if (domain_new.size() == 0) {
        return false;  // Domain wipeout.
    }
    copy_to(domain_new, D[x]);

    if (D[x].size() < original_domain_size) {
        was_variable_updated[x] = true;
    }
    return true;
}

inline bool propagate_binary(const Constraint& constraint, array<Domain>& D,
                             array<bool>& was_variable_updated) {
    int x0 = constraint.scope[0];
    int x1 = constraint.scope[1];

    auto& d0 = D[x0];
    auto& d1 = D[x1];

    stack_frame();
    // auto d0_new      = allocate<int>(d0.size());
    auto d0_has_pair = allocate<bool>(d0.size(), false);
    auto d1_has_pair = allocate<bool>(d1.size(), false);

    for (size_t i = 0; i < d0.size(); i++) {
        auto v0 = d0[i];
        for (size_t k = 0; k < d1.size(); k++) {
            auto v1 = d1[k];
            stack_frame();
            auto xy = allocate({v0, v1});
            if (constraint.eval_custom(constraint, xy)) {
                d0_has_pair[i] = true;
                d1_has_pair[k] = true;
                break;
            }
        }
    }

    auto d0_new = allocate<int>(d0.size());
    d0_new.resize(0);
    for (size_t i = 0; i < d0.size(); i++) {
        if (d0_has_pair[i]) {
            d0_new.push_back(d0[i]);
        }
    }
    if (d0_new.size() == 0) return false;
    if (d0_new.size() < d0.size()) {
        was_variable_updated[x0] = true;
        copy_to(d0_new, d0);
    }

    for (size_t k = 0; k < d1.size(); k++) {
        if (d1_has_pair[k]) continue;
        auto v1 = d1[k];
        for (size_t i = 0; i < d0.size(); i++) {
            auto v0 = d0[i];
            stack_frame();
            auto xy = allocate({v0, v1});
            if (constraint.eval_custom(constraint, xy)) {
                d1_has_pair[k] = true;
                break;
            }
        }
    }

    auto d1_new = allocate<int>(d1.size());
    d1_new.resize(0);
    for (size_t i = 0; i < d1.size(); i++) {
        if (d1_has_pair[i]) {
            d1_new.push_back(d1[i]);
        }
    }
    if (d1_new.size() == 0) return false;
    if (d1_new.size() < d1.size()) {
        was_variable_updated[x1] = true;
        copy_to(d1_new, d1);
    }

    return true;
}

bool eval_nary(const Constraint& constraint, const array<Domain>& domains) {
    for (auto var : constraint.scope)
        if (domains[var].size() != 1) return true;

    stack_frame();
    auto values = allocate<int>(constraint.scope.size());
    for (int i = 0; i < constraint.scope.size(); ++i) {
        values[i] = domains[constraint.scope[i]][0];
    }

    return constraint.eval_custom(constraint, values);
}

bool propagate_nary(const Constraint& constraint, array<Domain>& D,
                    array<bool>& was_variable_updated) {
    return true;
}

bool eval(const Constraint& constraint, const array<Domain>& domains) {
    auto type = constraint.type;
    if (type == Constraint::ALL_DIFFERENT)
        return eval_all_different(constraint, domains);
    if (type == Constraint::BINARY) return eval_binary(constraint, domains);
    if (type == Constraint::NARY) return eval_nary(constraint, domains);
    if (type == Constraint::UNARY) return eval_unary(constraint, domains);
    return false;
}

bool propagate(const Constraint& constraint, array<Domain>& domains,
               array<bool>& was_variable_updated) {
    if (constraint.type == Constraint::ALL_DIFFERENT) {
        return propagate_all_different(constraint, domains,
                                       was_variable_updated);
    } else if (constraint.type == Constraint::BINARY) {
        return propagate_binary(constraint, domains, was_variable_updated);
    } else if (constraint.type == Constraint::NARY) {
        return propagate_nary(constraint, domains, was_variable_updated);
    } else if (constraint.type == Constraint::UNARY) {
        return propagate_unary(constraint, domains, was_variable_updated);
    } else {
        return false;
    }
}

void print_unsatisfied(const array<Domain>& D, const array<Constraint>& C) {
    printf("unsatisfied constraints: ");
    bool found = false;
    for (int i = 0; i < C.size(); ++i) {
        if (not eval(C[i], D)) {
            found = true;
            printf("\n%d: %s\n", i, data(C[i].name));
        }
    }
    if (not found) printf("nothing\n");
    printf("\n");
}