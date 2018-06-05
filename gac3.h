#pragma once
// #include "csp.h"

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
