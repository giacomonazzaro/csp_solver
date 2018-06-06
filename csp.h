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
using Assignment = std::unordered_map<int, int>;

/***** Data definitions *****/
enum constraint_type {
    CUSTOM, ALL_DIFFERENT, EQUALITY, BINARY
};

struct Constraint {
    std::string name;
    array<int> variables;
    std::function<bool(const array<Domain>&)> eval;
    constraint_type type = CUSTOM;
};


struct CSP {
    std::string name;
    array<Domain> domains;
    array<Constraint> constraints;
};


/***** Solving functions *****/
// Check if assignment satisfies the constraints.
bool satisfies(const array<Domain>& assignment, const array<Constraint>& C);

// Search satisfying assignment.
bool search(const array<Constraint>& C, array<Domain>& D, int depth);
Assignment search(const CSP& csp, Assignment A);

// Choose next variable (MRV & MaxDegree heuristics).
int choose_variable(const array<Domain>& D, const array<Constraint>& C);

// Make inferences after assignment (Genrealized Arc Consistency).
bool constraint_propagation(const array<Constraint>& C, array<Domain>& D);
bool gac3(const array<Constraint>& C, array<Domain>& D);
bool remove_values(int variable, const Constraint& constraint, array<Domain>& D, array<Domain> A);
bool search_small(const Constraint& c, array<Domain> D, int depth);



/***** CSP intialization functions. *****/
inline CSP make_csp(const std::string& s, const array<Domain>& d, const array<Constraint>& c = {}) {
    CSP csp;
    csp.name = s;
    csp.domains = d;
    csp.constraints = c;
    return csp;
}


inline void add_contstraint(CSP& csp, const Constraint& c) {
    csp.constraints.push_back(c);
}


static Constraint all_different(const array<int>& vars, const std::string& name = "") {
    Constraint c;
    c.variables = vars;
    c.name = name;
    c.type = ALL_DIFFERENT;
    c.eval = [vars](const array<Domain>& D) {
        // std::set<int> buckets; // Piccioni
        // for (int i : vars)
        //     for (int val : D[i])
        //         buckets.insert(val);
        // if(buckets.size() < vars.size()) return false;
            
        
        // auto D_ = D;
        // for(int v : vars) {
        //     if(D_[v].size() == 1)
        //         for(int w : vars) {
        //             if(w == v) continue;
        //                 for (int i = 0; i < D_[w].size(); ++i)
        //                     if(D_[w][i] == D_[v][0]) {
        //                         D_[w].erase(D_[w].begin() + i);
        //                         if(D[w].size() == 0) return false;
        //                         break;
        //                     }

        //         }
        // }

        for (int i = 0; i < vars.size()-1; ++i) {
            int v = vars[i];
            if(D[v].size() != 1) continue;
            for (int k = i+1; k < vars.size(); ++k) {
                int w = vars[k];
                if(D[w].size() == 1 and D[v][0] == D[w][0])
                    return false;
            }
        }
        
        return true;
    };
    return c;
}


static Constraint binary(int i, int k, const std::function<bool(int, int)>& rel, const std::string& name = "") {
    Constraint c;
    c.variables = {i, k};
    c.name = name;
    c.eval = [=](const array<Domain>& D) {
        if(D[i].size() == 1 and D[k].size() == 1) {
            if(not rel(D[i][0], D[k][0])) return false;
        }

        return true;
    };
    return c;
}


static Constraint equal(int v0, int v1, const std::string& name = "") {
    Constraint c;
    c.name = name;
    c.variables = {v0, v1};
    c.eval = [v0, v1](const array<Domain>& D) {
        if(D[v0].size() == 1 and D[v1].size() == 1)
            if(D[v0][0] != D[v1][0])
                return false;

        return true;
    };
    return c;
}




// Utilities functions.
#define add(v, x) v.push_back(x)
#define remove(v, i) v.erase(v.begin() + i)
#define contains(v, x) (std::find(v.begin(),v.end(),x) !=v.end())
#define min(v) *std::min_element(v.begin(),v.end());

template <typename Type>
inline array<Type> make_range(int from, int to) {
    array<Type> result (to - from);
    for(int i = 0; i < to-from; i++) result[i] = from + i;
    return result;
}

template <typename Type>
inline array<Type> make_range(int to) { return make_range<Type>(0, to); }





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
