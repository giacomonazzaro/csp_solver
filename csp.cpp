#include "csp.h"

// #define PRINT_SEARCH
// #define PRINT_SEARCH_LOG
// #define PRINT_SEARCH_GAC

void comment(const std::string& c) {
    #ifdef PRINT_SEARCH_LOG
    printf("%s\n", c.c_str());
    #endif
}


bool satisfies(const array<Constraint>& C, const array<Domain>& D) {
    for(auto& constraint : C) {
        if(not constraint.eval(D)) {
            return false;
        }
    }
    return true;
}


bool is_assignment_complete(const array<Domain>& D) {
    for (int i = 0; i < D.size(); ++i)
        if(D[i].size() != 1) return false;

    return true;
}


bool do_inferences(const array<Constraint>& C, array<Domain>& D) {
    // Forward propagation.
    if(not constraint_propagation(C, D)) {
        comment("Propagation failure");
        return false;
    }
    
    // Generalized arc consistency.
    if(not gac3(C, D)) {
        comment("GAC3 failure");
        return false;
    }

    return true;
}


bool search(const array<Constraint>& C, array<Domain>& D, int depth) {
    #ifdef PRINT_SEARCH
    print_state(D, depth);
    #endif

    // If assignment is complete, return success.
    if(is_assignment_complete(D)) {
        return true;
    }

    int variable = choose_variable(D, C);

    for(int val : D[variable]) {
        // Copying the domains to make a temp version.
        array<Domain> D_attempt = D; 
        D_attempt[variable] = {val};

        // Check if assignment satisfies constraints.
        if(not satisfies(C, D))
            return false;


        // Propagate assignment and eventually reduce domains.
        if(not do_inferences(C, D_attempt))
            continue;
        
        // Recursive call.
        bool success = search(C, D_attempt, depth+1);
        if(success) {
            D = D_attempt; // update the domains.
            return true;
        }
    }

    // Return failure. Backtrack.
    return false;
}


Assignment search(const CSP& csp, Assignment A = {}) {
    auto D = csp.domains;
    for(auto& kv : A)
        D[kv.first] = {kv.second};

    if(A.size() > 0) {
        constraint_propagation(csp.constraints, D);
        gac3(csp.constraints, D);
    }

    if(search(csp.constraints, D, 0)) {
        return make_assignment(D);
    }
    else
        return {};
}


int choose_variable(const array<Domain>& D, const array<Constraint>& C) {
    // Choose following minimun remaining values heuristic.
    array<int> remaining_values (D.size(), 9999); //@Hack: 9999???
    for (int i = 0; i < D.size(); ++i) {
        if(D[i].size() <= 1) continue;
        remaining_values[i] = D[i].size();
    }

    // Choose as candidates all varibale which have minimum remaining values.
    int min_val = min(remaining_values);
    array<int> candidates;
    candidates.reserve(D.size());
    for (int i = 0; i < remaining_values.size(); ++i) {
        if(remaining_values[i] == min_val)
            candidates.push_back(i);
    }

    // If no ties, return the variable.
    if(candidates.size() == 1)
        return candidates[0];


    // If there's a tie, use Max Degree heuristic.
    // Start with computing degrees. We can cache that, but it
    // is probably unexpensive to compute on the fly (@Profile it).
    array<int> degrees (D.size(), 0);
    for(auto& c : C)
        for(int v : c.variables) 
            degrees[v] += 1;

    int max_degree_idx = candidates[0];
    int max_degree = degrees[max_degree_idx];
    for (int i = 1; i < candidates.size(); ++i) {
        int v = candidates[i];
        if(degrees[v] > max_degree) {
            max_degree_idx = v;
            max_degree = degrees[v];
        }
    }
    assert(max_degree_idx < D.size());
    assert(D[max_degree_idx].size() > 1);
    return max_degree_idx;
}


bool constraint_propagation(const array<Constraint>& C, array<Domain>& D) {
    for(auto& c : C) {
        if(c.type == ALL_DIFFERENT) {
            for(int v : c.variables) {
                if(D[v].size() != 1) continue;
                for(int w : c.variables) {
                    if(w == v) continue;
                    for (int i = 0; i < D[w].size(); ++i)
                        if(D[w][i] == D[v][0]) {
                            remove(D[w], i);
                            if(D[w].size() == 0) return false;
                            break;
                        }
                }
            }
        }

        // if(c.variables.size() == 2) {
        //     for(int k : {0,1}) {
        //         int v0 = c.variables[k];
        //         int v1 = c.variables[(k+1)%2];
        //         if(D[v0].size() != 1) continue;
        //         Domain old_domain = D[v1];
        //         for (int i = 0; i < D[v1].size(); ) {
        //             D[v1] = {old_domain[]}
        //         }
        //     }
        // }
    }

    return true;
}


bool remove_values(int variable, const Constraint& constraint, array<Domain>& D) {
    bool removed_value = false;
    int i = 0;
    Domain domain_tmp = D[variable]; // copying the domain.
    
    while(true) {
        // Make a fake copy of the domain. Will set the just interesting variables.
        array<Domain> Dfake = array<Domain>(D.size(), {-1});
        Dfake[variable] = {domain_tmp[i]};
        for(auto v : constraint.variables)
            if(v != variable) Dfake[v] = D[v]; // copying the domains.
        
        
        bool exists = search_small(constraint, Dfake, 0);

        if(exists == false) {
            remove(domain_tmp, i);
            // removed_values.push_back(domain_tmp, i);
            removed_value = true;
        }
        else {
            i += 1;
        }


        if(i >= domain_tmp.size()) {
            D[variable] = domain_tmp;
            return removed_value;
        }
    }
}


bool gac3(const array<Constraint>& C, array<Domain>& D_result) {
    array<Domain> D = D_result; // copying the domains.
    array<int> var_queue;
    array<int> const_queue;

    // For each constraint c, for each variable v in the scope of c,
    // add the pair (v, c) to the queue.
    for (int i = 0; i < C.size(); ++i) {
        for(int v : C[i].variables) {
            if(D[v].size() == 1) continue;
            var_queue.push_back(v);
            const_queue.push_back(i);
        }
    }

    // Consume the queue until it is empty.
    // For each pair (v, c), look if there exist a possible assignment
    // of the other varibles in the scope of c.
    while(var_queue.size() > 0) {
        int v = var_queue.back();
        int c = const_queue.back();
        var_queue.pop_back();
        const_queue.pop_back();

        bool removed_value_from_domain = remove_values(v, C[c], D);
        if(removed_value_from_domain) {
            // If the domain was left empty, this assignment cannot
            // be made complete. search() will read {} as failure.
            if(D[v].size() == 0) {
                return false;
            }

            // If we shrinked its domain, we add to the queue all
            // the variables that are neighbors of v through other constraints.
            for (int i = 0; i < C.size(); ++i) {
                if(i == c) continue;

                if(not contains(C[i].variables, v))
                    continue;

                for(int w : C[i].variables) {
                    if(w == v) continue;
                    if(D[w].size() == 1) continue;

                    // Check if it is already in queue.
                    bool already_in_queue = false;
                    for(int k = 0; k < var_queue.size(); k++) {
                        if(var_queue[k] == w and const_queue[k] == i) {
                            already_in_queue = true;
                            break;
                        }
                    }
                    
                    if(not already_in_queue) {
                        var_queue.push_back(w);
                        const_queue.push_back(i);
                    }
                }
            }
        }
    }

    // Return the updated domain.
    D_result = D; // copying the domains.
    return true;
}


bool search_small(const Constraint& c, array<Domain> D, int depth) {  
    // Naive search that just check if there's a possible assignment that
    // satisfy only ONE constraint. Used by remove_values()
    #ifdef PRINT_SEARCH_GAC
    print_state(D, depth);
    #endif

    // If assignment is complete, return true.
    const array<int>& vars = c.variables;
    bool complete = true;
    for(int v : vars) {
        if(D[v].size() != 1) {
            complete = false;
            break;
        }
    }
    if(complete) return true;


    // Still using MRV & Max Degree.
    int variable = choose_variable(D, {c});

    const Domain domain = D[variable];
    for(int val : domain) {
        D[variable] = {val};

        // If new assignment does not satisfies constraints, continue.
        if(not c.eval(D)) continue;
        
        // @Speed: We should propagate also in search_small, but copying D seems to slow down.
        // array<Domain> D_new = D;
        // if(not constraint_propagation({c}, D_new)) continue;

        if(search_small(c, D, depth+1)) {
            return true;
        }
    }

    return false;
}

