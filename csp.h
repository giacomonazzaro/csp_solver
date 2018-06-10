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
    array<int> variables;
    // std::function<bool(const array<Domain>&)> eval;
    constraint_type type = UNKNOWN;

    virtual bool eval(const array<Domain>&) const = 0;
};


struct CSP {
    std::string name;
    array<Domain> domains;
    array<const Constraint*> constraints;
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
//     cp->variables = c.variables;
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




struct all_different : Constraint {
     all_different(const array<int>& vars, const std::string& n = "") {
        variables = vars;
        name = n;
        type = ALL_DIFFERENT;
    }

    bool eval(const array<Domain>& D) const {
        for (int i = 0; i < variables.size()-1; ++i) {
            int v = variables[i];
            if(D[v].size() != 1) continue;
            for (int k = i+1; k < variables.size(); ++k) {
                int w = variables[k];
                if(D[w].size() == 1 and D[v][0] == D[w][0])
                    return false;
            }
        }
        
        return true;
    }
};

struct binary : Constraint {
    std::function<bool(int, int)> rel;

    binary(int i, int k, std::function<bool(int, int)> r, const std::string& n = "") {
        variables = {i, k};
        name = n;
        rel = r;
        type = BINARY;
    }

    bool eval(const array<Domain>& D) const {
        int i = variables[0];
        int k = variables[1];
        if(D[i].size() == 1 and D[k].size() == 1) {
            if(not rel(D[i][0], D[k][0])) return false;
        }

        return true;
    };
};