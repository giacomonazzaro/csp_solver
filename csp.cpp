#include "csp.h"

#define GAC3
// #define PRINT_SEARCH
// #define PRINT_SEARCH_GAC

bool satisfies(const Assignment& assignment, const vector<Constraint>& C) {
    for(auto& kv : assignment) {
        int variable = kv.first;
        int value = kv.second;
       
        for(auto& constraint : C) {
            if(not constraint.eval(assignment)) {
                return false;
            }
        }
    }

    return true;
}


Assignment search(const vector<Constraint>& C, vector<Domain> D, Assignment A, int depth) {
    #ifdef PRINT_SEARCH
    print_state(A, D, depth);
    #endif
    num_search += 1;

    // If assignment is complete, return.
    if(A.size() == D.size()) {
        return A;
    }

    int variable = choose_variable(D, A, C); // Minimum Remaining Value, then Max Degree (fail first)
    assert(variable<D.size());
    assert(D[variable].size() > 0);

    for(int val : D[variable]) {
        A[variable] = val;
        // printf("new_assign %d := %d\n", variable, val);

        // If new assignment does not satisfies constraints, continue.
        if(not satisfies(A, C)) {
            // printf("not satisfy!\n");
            A.erase(variable);
            continue;
        }

        #ifdef GAC3
            vector<Domain> D_new;
            D_new = gac3(C, A, D);
            if(D_new.size() == 0) {
                // printf("GAC fail!\n");    
                A.erase(variable);
                continue;
            }
        #else
            vector<Domain>& D_new = D;
        #endif
       
        Assignment A_new = search(C, D_new, A, depth+1);
        if(A_new.size() == 0) {
            A.erase(variable);
            #ifdef PRINT_SEARCH 
            printf("BACKTRACK\n");
            print_state(A, D, depth);
            #endif
        }
        else {
            return A_new;
        }
    }

    // Return empty assignment.
    return {};
}


Assignment search(const CSP& csp, Assignment A = {}) {
    return search(csp.constraints, csp.domains, A, 0);
}


int choose_variable(const vector<Domain>& D, const Assignment& asg, const vector<Constraint>& C) {
    // Choose following minimun remaining values heuristic.
    vector<int> remaining_values (D.size(), 9999); //@Hack: 9999???
    for (int i = 0; i < D.size(); ++i) {
        if(asg.count(i) > 0) continue;
        remaining_values[i] = D[i].size();
    }

    // Choose as candidates all varibale which have minimum remaining values.
    int min_val = min(remaining_values);
    vector<int> candidates;
    candidates.reserve(D.size());
    for (int i = 0; i < remaining_values.size(); ++i) {
        if(remaining_values[i] == min_val)
            candidates.push_back(i);
    }

    // If no tie, return the variable.
    if(candidates.size() == 1)
        return candidates[0];


    // If there's a tie, use Max Degree heuristic.
    // Start with computing degrees. We can cache that, but it
    // is probably unexpensive to compute on the fly (@Profile it).
    vector<int> degrees (D.size(), 0);
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
    return max_degree_idx;
}


bool remove_values(int variable, const Constraint& constraint, vector<Domain>& D, Assignment A) {
    bool removed_value = false;
    int i = 0;
    while(true) {
        int val = D[variable][i];
        A[variable] = val;
        bool exist = search_small(constraint, D, A, 0);

        if(exist == false) {
            D[variable].erase(D[variable].begin() + i);
            removed_value = true;
        }
        else {
            i += 1;
        }

        A.erase(variable);

        if(i >= D[variable].size())
            return removed_value;
    }
}


vector<Domain> gac3(const vector<Constraint>& C, const Assignment& asg, vector<Domain> D) {
    vector<int> var_queue;
    vector<int> const_queue;

    // For each constraint c, for each variable v in the scope of c,
    // add the pair (v, c) to the queue.
    for (int i = 0; i < C.size(); ++i) {
        const Constraint& c = C[i];
        for(int v : c.variables) {
            if(asg.count(v)) continue;
            assert(D[v].size() > 0);
            var_queue.push_back(v);
            const_queue.push_back(i);
        }
    }

    // Consume the queue until it is empty.
    // For each pair (v, c), look it there exist a possible assignment
    // of the other varibles in the scope of c.
    while(var_queue.size() > 0) {
        int v = var_queue.back();
        int c = const_queue.back();
        var_queue.pop_back();
        const_queue.pop_back();

        bool removed_value_from_domain = remove_values(v, C.at(c), D, asg);
        if(removed_value_from_domain) {
            // If the domain was left empty, this assignment cannot
            // be made complete. search() will read {} as failure.
            if(D[v].size() == 0) {
                return {};
            }

            // If we shrinked its domain, we add to the queue all
            // the variables that are neighbors of v through other constraints.
            for (int i = 0; i < C.size(); ++i) {
                if(i == c) continue;

                if(not contains(C[i].variables, v))
                    continue;

                for(int w : C[i].variables) {
                    if(w == v) continue;
                    
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
    return D;
}


bool search_small(const Constraint& c, vector<Domain> D, Assignment A, int depth) {  
    // Naive search that just check if there's a possible assignment that
    // satisfy only ONE constraint. Used by remove_values()
    #ifdef PRINT_SEARCH_GAC
    print(A, depth);
    #endif

    // If assignment is complete, return true.
    const vector<int>& vars = c.variables;
    bool complete = true;
    for(int v : vars) {
        if(A.count(v) == 0) {
            complete = false;
            break;
        }
    }
    if(complete) return true;


    // @Speed: We're not unsing any heuristic to choose the variable!! (MRV, Max Degree)
    int variable = -1;
    for(int v : vars) {
        if(A.count(v) == 0) {
            variable = v;
            break;
        }
    }

    assert(D[variable].size() > 0);

    for(int val : D[variable]) {
        A[variable] = val;

        // If new assignment does not satisfies constraints, continue.
        if(not c.eval(A)) {
            A.erase(variable);
            continue;
        }

        auto ass = search_small(c, D, A, depth+1);
        if(not ass) {
            A.erase(variable);
        }
        else {
            return true;
        }
    }

    return false;
}

