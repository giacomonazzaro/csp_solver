#pragma once
#include "csp.h"

// Search satisfying assignment.
Assignment search(const vector<Constraint>& C, vector<Domain> D, Assignment A, int depth);
Assignment search(const CSP& csp, Assignment A);

// Choose next vatiable. MRV & MaxDegree heuristics.
int choose_variable(const vector<Domain>& D, const Assignment& asg, const vector<Constraint>& C);

// Make infernces afeter assignment. Genrealized Arc Consistency.
vector<Domain> gac3(const vector<Constraint> C, const Assignment& asg, vector<Domain> D);
bool remove_values(int variable, const Constraint& constraint, vector<Domain>& D, Assignment A);
bool search_small(const Constraint& c, vector<Domain> D, Assignment A, int depth);


Assignment search(const vector<Constraint>& C, vector<Domain> D, Assignment A, int depth) {
    #ifdef PRINT_SEARCH
    print_state(A, D, depth);
    // print_domains(D);
    // print(A, depth);
    // // printf("A size %d\n", A.size());
    #endif
    num_search += 1;


    // If assignment is complete, return.
    if(A.size() == D.size()) {
        return A;
    }

    for(int i=0; i<D.size(); i++) assert(D[i].size() > 0);

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

        for(int i=0; i<D.size(); i++) assert(D[i].size() > 0);

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

        for(int i=0; i<D.size(); i++) assert(D[i].size() > 0);
       
        Assignment A_new = search(C, D_new, A, depth+1);
        if(A_new.size() == 0) {
            A.erase(variable);
            #ifdef PRINT_SEARCH 
            printf("^\n");
            printf("\n BACKTRACK \n\n");
            print_state(A, D, depth);
            // print_domains(D);
            // print(A, depth);
            // printf("A size %d\n", A.size());
            #endif
        }
        else {
            return A_new;
        }
    }

    if(depth == 0)
    printf("\n***** Failure! (%d) *****\n", depth);
    
    // Return incomplete assignment.
    return {};
}


Assignment search(const CSP& csp, Assignment A = {}) {
    auto C = csp.constraints;
    auto D = csp.domains;
    
    return search(C, D, A, 0);
}


int choose_variable(const vector<Domain>& D, const Assignment& asg, const vector<Constraint>& C) {
    // Choose following minimun remaining values heuristic.
    vector<int> remaining_values (D.size(), 9999); //@Hack: 9999???
    for (int i = 0; i < D.size(); ++i) {
        if(asg.count(i) > 0) continue;
        remaining_values[i] = D[i].size();
    }

    int min_val = min(remaining_values);
    vector<int> candidates;
    candidates.reserve(D.size());
    for (int i = 0; i < remaining_values.size(); ++i) {
        if(remaining_values[i] == min_val)
            candidates.push_back(i);
    }

    if(candidates.size() == 1)
        return candidates[0];

    vector<int> degrees (D.size(), 0);
    for(auto& c : C)
        for(int v : c.variables) 
            degrees[v] += 1;

    // Max degree heuristic.
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


bool search_small(const Constraint& c, vector<Domain> D, Assignment A, int depth) {  
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
            #ifdef PRINT_SEARCH_GAC
            printf("^\n");
            #endif
        }
        else {
            return true;
        }
    }

    return false;
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


vector<Domain> gac3(const vector<Constraint> C, const Assignment& asg, vector<Domain> D) {
    vector<int> var_queue;
    vector<int> const_queue;
    for (int i = 0; i < C.size(); ++i) {
        const Constraint& c = C[i];
        for(int v : c.variables) {
            if(asg.count(v)) continue;
            assert(D[v].size() > 0);
            var_queue.push_back(v);
            const_queue.push_back(i);
        }
     }

    while(var_queue.size() > 0) {
        int v = var_queue.back();
        int c = const_queue.back();
        var_queue.pop_back();
        const_queue.pop_back();

        if(remove_values(v, C.at(c), D, asg)) {
            // printf("(%s) smaller %d: ", C[c].name.c_str(), v);
            // print_domain(D[v]);

            if(D[v].size() == 0) {
                return {};
            }

            for (int i = 0; i < C.size(); ++i) {
                if(i == c) continue;

                if(not contains(C[i].variables, v))
                    continue;

                for(int w : C[i].variables) {
                    if(w == v) continue;
                    
                    // Check if it is already in queue
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

    return D;
}
