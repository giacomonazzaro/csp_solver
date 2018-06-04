#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>
#define vector std::vector

static int num_search = 0;

template <typename Type>
inline bool contains(const vector<Type>& v, const Type& x){
    return std::find(v.begin(), v.end(), x) != v.end();
}

template <typename Type>
inline Type min(const vector<Type>& v){
    Type m = v[0];
    for (int i = 1; i < v.size(); ++i)
        if(v[i] < m) m = v[i];

    return m;
}

using Domain = vector<int>;
using Assignment = std::unordered_map<int, int>;

struct Constraint {
    std::string name;
    vector<int> variables;
    std::function<bool(const Assignment&)> eval;
};

struct CSP {
    std::string name;
    vector<Domain> domains;
    vector<Constraint> constraints;
    vector<int> degrees;
};

CSP make_csp(const std::string& s, const vector<Domain>& d, const vector<Constraint>& c = {}) {
    CSP csp;
    csp.name = s;
    csp.domains = d;
    csp.constraints = c;
    csp.degrees = vector<int>(csp.domains.size(), 0);
    return csp;
}

bool satisfies(const CSP& csp, const Assignment& assignment) {
    for(auto& kv : assignment) {
        int variable = kv.first;
        int value = kv.second;
       
        assert(contains(csp.domains[variable], value));

        for(int i=0; i<csp.constraints.size(); i++) {
            auto constraint = csp.constraints[i];
            if(not constraint.eval(assignment)) {
                return false;
            }
        }
    }

    return true;
}

void add_contstraint(CSP& csp, const Constraint& c) {
    for(int v : c.variables)
        csp.degrees[v] += 1;

    csp.constraints.push_back(c);
}

void print(const CSP& csp);
void print(const Assignment& asg, int);

Constraint all_different(const vector<int>& vars, const std::string& name = "") {
    Constraint c;
    c.variables = vars;
    c.name = name;
    c.eval = [vars](const Assignment& asg) {
        for (int i = 0; i < vars.size()-1; ++i)
            for (int k = i+1; k < vars.size(); ++k) {
                int v0 = vars[i];
                int v1 = vars[k];
                if(asg.count(v0) and asg.count(v1))
                    if(asg.at(v0) == asg.at(v1))
                        return false;
            }
        
        return true;
    };
    return c;
}

Constraint binary(int i, int k, std::function<bool(int, int)> rel, const std::string& name = "") {
    Constraint c;
    c.variables = {i, k};
    c.name = name;
    c.eval = [=](const Assignment& asg) {
        if(asg.count(i) and asg.count(k)) {
            if(not rel(asg.at(i), asg.at(k))) return false;
        }

        return true;
    };
    return c;
}

Constraint equal(int v0, int v1, const std::string& name = "") {
    Constraint c;
    c.name = name;
    c.variables = {v0, v1};
    c.eval = [v0, v1](const Assignment& asg) {
        if(asg.count(v0) and asg.count(v1))
            if(asg.at(v0) != asg.at(v1))
                return false;

        return true;
    };
    return c;
}

// Constraint equal_constant(int v0, int val) {
//     Constraint c;
//     c.variables = {v0};
//     c.eval = [v0, val](const Assignment& asg) {
//         if(asg.count(v0) and asg.at(v0) != val)
//             return false;

//         return true;
//     };
//     return c;
// }


bool remove_values(CSP& csp, int v, const Constraint& c) {
    bool removed = false;
    Assignment asg;

    for(int vidx=0; vidx < csp.domains[v].size(); vidx++) {
        int value = csp.domains[v][vidx];
        asg[v] = value;

        int possible_assignments = 1;
        for(int w : c.variables) {
            if(w == v) continue;
            possible_assignments *= csp.domains[w].size();
        }
        
        bool exist_assignment = false;
        for (int i = 0; i<possible_assignments; ++i) {
            for(int w : c.variables) {
                if(w == v) continue;
                asg[w] = i / csp.domains[w].size();
            }
            
            if(satisfies(csp, asg)) {
                exist_assignment = true;
                break;
            }

            for(int w : c.variables) {
                if(w == v) continue;
                asg.erase(w);
            }
        }

        if(not exist_assignment) {
            csp.domains[v].erase(csp.domains[v].begin() + vidx);
            vidx -= 1;
            removed = true;
        }
    }

    return removed;
}

bool gac3(CSP& csp, const Assignment& asg) {
    vector<int> var_queue;
    vector<Constraint> const_queue;
    for(auto c : csp.constraints) {
        for(int v : c.variables) {
            if(asg.count(v)) continue;
            var_queue.push_back(v);
            const_queue.push_back(c);
        }
    }

    while(var_queue.size()>0) {
        int v = var_queue.back();
        auto c = const_queue.back();
        var_queue.pop_back();
        const_queue.pop_back();

        if(remove_values(csp, v, c)) {
            if(csp.domains[v].size() == 0) {
                return false;
            }

            for(auto& cc : csp.constraints) {
                if(not contains(cc.variables, v))
                    continue;

                for(int w : cc.variables) {
                    if(w == v) continue;
                    var_queue.push_back(w);
                    const_queue.push_back(cc);
                }
            }
        }
    }

    return true;
}


int choose_variable(const CSP& csp, const Assignment& asg) {
    // Try to choose following minimun remaining values heuristic.
    vector<int> remaining_values (csp.domains.size(), 9999);
    for (int i = 0; i < csp.domains.size(); ++i) {
        if(asg.count(i) != 0) continue;
        remaining_values[i] = csp.domains[i].size();
    }

    int min_val = min(remaining_values);
    vector<int> candidates;
    candidates.reserve(csp.domains.size());
    for (int i = 0; i < remaining_values.size(); ++i) {
        if(remaining_values[i] == min_val)
            candidates.push_back(i);
    }

    // if(candidates.size() == 1)
        return candidates[0];

    // Min degree heuristic.
    // int min_degree_idx = candidates[0];
    // int min_degree = csp.degrees[min_degree_idx];
    // for (int i = 1; i < candidates.size(); ++i) {
    //     if(csp.degrees[candidates[i]] < min_degree) {
    //         min_degree_idx = candidates[i];
    //         min_degree = csp.degrees[min_degree_idx];
    //     }
    // }
    // return candidates[min_degree_idx];
}

void print_times(const char* s, int times) { for (int i = 0; i < times; ++i) printf("%s", s); }

bool search(const CSP& csp, Assignment& A, const vector<Domain>& D, int depth) {
    #ifdef PRINT_SEARCH
    print(A, depth);
    // printf("A size %d\n", A.size());
    #endif
    num_search += 1;


    // If assignment is complete, return.
    if(A.size() == csp.domains.size()) {
        return true;
    }

    int variable = choose_variable(csp, A); // Minimum Remaining Value, then Max Degree (fail first)
    assert(D[variable].size() > 0);

    for(int val : D[variable]) {
        A[variable] = val;

        // If new assignment does not satisfies constraints, continue.
        if(not satisfies(csp, A)) {
            A.erase(variable);
            continue;
        }

        /*if(not gac3(csp, A)) {
            A.erase(variable);
            continue;
        }*/
       
        Assignment A_copy = A;
        if(not search(csp, A_copy, D, depth+1)) {
            A.erase(variable);
            #ifdef PRINT_SEARCH 
            printf("^\n");
            #endif
        }
        else {
            A = A_copy;
            return true;
        }
    }

    if(depth == 0)
    printf("\n***** Failure! (%d) *****\n", depth);
    return false;
}


Assignment search(const CSP& csp, Assignment asg = {}) {
    search(csp, asg, csp.domains, 0);
    return asg;
}


void print(const CSP& csp) {
    printf("csp %s\n", csp.name.c_str());
    printf("  vars deg   domain\n");
    for (int i = 0; i < csp.domains.size(); ++i) {
        printf("    %d: ", i);
        printf(" %d  { ", csp.degrees[i]);
        for(int val : csp.domains[i]) printf("%d ", val);
        printf("}\n");
    }
}

void print(const Assignment& asg, int b = 0) {
    print_times("-", b);
    printf("assignment:\n");
    for (auto kv : asg) {
        print_times("-", b);
        printf("var %d := %d\n", kv.first, kv.second);
    }
}
