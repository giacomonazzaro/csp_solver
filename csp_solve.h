#pragma once
#include "csp.h"
#include "gac3.h"

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

struct search_params {
    bool gac3 = false;
    int num_search = 0;
};


void print_state(const Assignment& A, const vector<Domain>& D, int depth = 0) {
    for (int i = 0; i < D.size(); ++i) {
        print_times("-", depth);
        printf(" %d := ", i);
        if(A.count(i) > 0) {
            printf("%d\n", A.at(i));
        }
        else
            print_domain(D[i]);
    }
}

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