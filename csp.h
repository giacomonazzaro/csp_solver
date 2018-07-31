#pragma once
#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <set>

// @Warining: std::array is a different thing! (It's a statically sized array)
//            But 'vector' is a really bad name...
#define array std::vector
using Domain = array<int>;
using Assignment = std::unordered_map<int, int>; // Used only to interface with outside world.

/***** Data definitions *****/
enum constraint_type {
    RELATION, ALL_DIFFERENT, EQUAL, BINARY, UNKNOWN
};

struct Constraint {
    std::string name;
    array<int> scope;
    
    Constraint(array<int> vars, std::string s): scope(vars), name(s) {
        name += "(";
        for (int i = 0; i < scope.size()-1; ++i)
            name += std::to_string(scope[i]) + ", ";
        name += std::to_string(scope.back()) + ")";
    }

    virtual bool eval(const array<Domain>&) const = 0;
    virtual bool propagate(array<Domain>&) const = 0;
};


struct CSP {
    std::string name;
    array<Domain> domains;
    array<const Constraint*> constraints;

    void all_different(const array<int>& scope, std::string name);
    void binary(int i, int k, std::function<bool(int, int)> r, std::string name);
    void equal(int i, int k, std::string name);
};

struct search_stats {
    int backtracks = 0;
    int expansions = 0;
};

/***** Solving functions *****/
// Check if assignment satisfies the constraints.
bool satisfies(const array<const Constraint*>& C, const array<Domain>& A);
bool satisfies(const array<const Constraint*>& C, const Assignment& A);

// Search satisfying assignment.
bool search(const array<const Constraint*>& C, array<Domain>& D, int depth);
Assignment search(const CSP& csp, Assignment A, search_stats& stats);

// Choose next variable (MRV & MaxDegree heuristics).
int choose_variable(const array<Domain>& D, const array<const Constraint*>& C);

// Make inferences after assignment (Genrealized Arc Consistency).
bool constraints_propagation(const array<const Constraint*>& C, array<Domain>& D);
bool gac3(const array<const Constraint*>& C, array<Domain>& D);
bool remove_values(int variable, const Constraint& constraint, array<Domain>& D, array<Domain> A);
bool search_small(const Constraint* c, array<Domain> D, int depth);

/***** CSP intialization functions. *****/
inline CSP make_csp(const std::string& s, const array<Domain>& d, const array<const Constraint*>& c = {}) {
    CSP csp;
    csp.name = s;
    csp.domains = d;
    csp.constraints = c;
    return csp;
}

inline void add_constraint(CSP& csp, Constraint* c) {
    csp.constraints.push_back(c);
}

// inline void add_constraint(CSP& csp, Constraint c) {
//     Constraint* cp = new Constraint();
//     cp->name = c.name;
//     cp->scope = c.scope;
//     cp->eval = c.eval;
//     cp->type = c.type;
//     csp.constraints.push_back(cp);
// }

// Constraint definitions.
// Constraint all_different(const array<int>& vars, const std::string& name);
// Constraint binary(int i, int k, const std::function<bool(int, int)>& rel, const std::string& name);
// Constraint relation(const array<int>, const std::function<bool(const array<Domain>&)>, const std::string&);
// Constraint equal(int i, int k, const std::string& name);


// Utilities functions.
#define add(v, x) v.push_back(x)
#define remove(v, i) v.erase(v.begin() + i)
#define contains(v, x) (std::find(v.begin(),v.end(),x) != v.end())
#define min(v) *std::min_element(v.begin(),v.end());
#define append(v, w) v.reserve(v.size()+w.size()); v.insert( v.end(), w.begin(), w.end() )

inline array<int> make_range(int from, int to) {
    array<int> result (to - from);
    for(int i = 0; i < to-from; i++) result[i] = from + i;
    return result;
}

inline array<int> make_range(int to) { return make_range(0, to); }





// Printing functions.
inline void print_array(const Domain& d) {
    printf("{ "); for(int val : d) printf("%d ", val); printf("}\n");
}

inline void print_times(const char* s, int times) { for (int i = 0; i < times; ++i) printf("%s", s); }

inline void print_state(const array<Domain>& D, int depth = 0) {
    for (int i = 0; i < D.size(); ++i) {
        print_times("-", depth);
        printf(" %d = ", i);
        print_array(D[i]);
    }
}

inline void print_constraints(const CSP& csp) {
    for(auto c : csp.constraints)
        printf("%s\n", c->name.c_str());
}

inline Assignment make_assignment(const array<Domain>& D) {
    Assignment A = {};
    for(int i=0; i<D.size(); i++) {
        if(D[i].size() == 1)
            A[i] = D[i][0];
    }
    return A;
}

inline array<Domain> make_domains(const Assignment& A) {
    array<Domain> D (A.size());
    for(auto& kv : A)  D[kv.first] = {kv.second};
    return D;
}

inline void apply_assignment(Domain& D, const Assignment& A) {
    for(auto& kv : A)  D[kv.first] = kv.second;
}

inline void print_stats(const search_stats& stats) {
    printf("\nSearch statistics:\n");
    printf("   num_backtracks = %d\n", stats.backtracks);
    printf("   num_expansions = %d\n\n", stats.expansions);
}

inline void print_unsatisfied(const array<Domain> D, const array<const Constraint*>& C) {
    printf("unsatisfied constraints: ");
    //for(auto& d : D) assert(d.size() == 1);
    bool found = false;
    for (int i = 0; i < C.size(); ++i)
    {
        if(not C[i]->eval(D)) {
            found = true;
            printf("\n%d: %s\n", i, C[i]->name.c_str());
        }
    }
    if(not found) printf("nothing\n");
    printf("\n");
}




struct AllDifferent : Constraint {
     AllDifferent(const array<int>& vars, std::string n = "all_different"):
        Constraint(vars, n) {}

    bool eval(const array<Domain>& D) const {
        for (int i = 0; i < scope.size()-1; ++i) {
            int v = scope[i];
            if(D[v].size() != 1) continue;
            for (int k = i+1; k < scope.size(); ++k) {
                int w = scope[k];
                if(D[w].size() == 1 and D[v][0] == D[w][0])
                    return false;
            }
        }
        
        return true;
    }

    bool propagate(array<Domain>& D) const {
        for(int v : scope) {
            if(D[v].size() != 1) continue;
            for(int w : scope) {
                if(w == v) continue;
                for (int i = 0; i < D[w].size(); ++i)
                    if(D[w][i] == D[v][0]) {
                        remove(D[w], i);
                        if(D[w].size() == 0) return false;
                        break;
                    }
            }
        }
        return true;
    }
};

struct Binary : Constraint {
    std::function<bool(int, int)> rel;

    Binary(int i, int k, std::function<bool(int, int)> r, std::string n = "binary") :
        Constraint({i,k}, n), rel(r) {}

    bool eval(const array<Domain>& D) const {
        int i = scope[0];
        int k = scope[1];
        if(D[i].size() == 1 and D[k].size() == 1) {
            if(not rel(D[i][0], D[k][0]))
                return false;
        }

        return true;
    };

    bool propagate(array<Domain>& D) const {
        int x0 = scope[0];
        int x1 = scope[1];
        Domain d0, d1;
        // std::set d0, d1; // @Try with std::set, code will be simpler.
        for(int v0 : D[x0]) {
            bool found = false;
            for(int v1 : D[x1]) {
                if(rel(v0, v1)) {
                    if(not contains(d1, v1))
                        d1.push_back(v1);
                    found = true;
                }
            }
            if(found) d0.push_back(v0);
        }
        if(d0.size() == 0) return false;
        if(d1.size() == 0) return false;
        D[x0] = d0;
        D[x1] = d1;
        return true;
    }
};


struct Equal : Constraint {
    Equal(int i, int k, std::string name = "equal"):
        Constraint({i,k}, name) {}

    bool eval(const array<Domain>& D) const {
        int i = scope[0];
        int k = scope[1];
        if(D[i].size() == 1 and D[k].size() == 1) {
            if(D[i][0] != D[k][0]) return false;
        }
        return true;
    }
    
    bool propagate(array<Domain>& D) const {
        Domain intersection;
        intersection.reserve(D[scope[0]].size());
        for(int v0 : D[scope[0]]) {
            if(contains(D[scope[1]], v0))
                intersection.push_back(v0); 
        }
        if(intersection.size() == 0) return false;
        D[scope[0]] = intersection;
        D[scope[1]] = intersection;
        return true;
    }
};
