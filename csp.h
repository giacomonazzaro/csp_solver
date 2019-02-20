#pragma once
#include <string>
#include <unordered_map>
#include "array_algorithms.h"
#include "stack_allocator.h"

template <typename Type>
void remove(array<Type>& arr, int index) {
    for (int i = index; i < arr.count - 1; ++i) {
        arr[i] = arr[i + 1];
    }
    arr.count -= 1;
}

using Domain     = array<int>;
using Assignment = std::unordered_map<int, int>;  // Used only to interface with
                                                  // outside world.

/***** Data definitions *****/
enum constraint_type { RELATION, ALL_DIFFERENT, EQUAL, BINARY, UNKNOWN };

struct Constraint {
    std::string name;
    array<int>  scope;

    Constraint() {}

    Constraint(const array<int>& vars, std::string s) {
        scope = copy(vars);
        name  = s;
        name += "(";
        for (int i = 0; i < scope.size() - 1; ++i)
            name += std::to_string(scope[i]) + ", ";
        name += std::to_string(scope.back()) + ")";
    }

    virtual bool eval(const array<Domain>&) const = 0;
    virtual bool propagate(array<Domain>&) const  = 0;
};

struct CSP {
    std::string              name;
    array<Domain>            domains;
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
bool constraints_propagation(const array<const Constraint*>& C,
                             array<Domain>&                  D);
bool gac3(const array<const Constraint*>& C, array<Domain>& D);
bool remove_values(int variable, const Constraint& constraint, array<Domain>& D,
                   array<Domain> A);
bool search_small(const Constraint* c, const array<Domain>& D_, int depth);

/***** CSP intialization functions. *****/
inline CSP make_csp(const std::string& s, const array<Domain>& d,
                    int num_constraints) {
    CSP csp;
    csp.name              = s;
    csp.domains           = d;
    csp.constraints       = allocate_array<const Constraint*>(num_constraints);
    csp.constraints.count = 0;
    return csp;
}

inline void add_constraint(CSP& csp, Constraint* c) {
    csp.constraints.push_back(c);
}

// Utilities functions.
#define add(v, x) v.push_back(x)
// #define remove(v, i) v.erase(v.begin() + i)
// #define contains(v, x) (v.contains(x));
// #define min(v) *std::min_element(v.begin(), v.end());
// #define append(v, w)                \
    // v.reserve(v.size() + w.size()); \
    v.insert(v.end(), w.begin(), w.end())

inline array<int> make_range(int from, int to) {
    auto result = allocate_array<int>(to - from);
    for (int i = 0; i < to - from; i++) result[i] = from + i;
    return result;
}

inline array<int> make_range(int to) { return make_range(0, to); }

// Printing functions.
inline void print_array(const array<int>& d) {
    printf("[");
    for (int i = 0; i < d.count - 1; ++i) printf("%d, ", d[i]);
    printf("%d]\n", d.back());
}

inline void print_domains(const array<Domain>& domains) {
    printf("\ndomains:\n");
    for (int i = 0; i < domains.count; ++i) {
        printf("%d: ", i);
        print_array(domains[i]);
    }
}

inline void print_times(const char* s, int times) {
    for (int i = 0; i < times; ++i) printf("%s", s);
}

inline void print_state(const array<Domain>& D, int depth = 0) {
    for (int i = 0; i < D.size(); ++i) {
        print_times("-", depth);
        printf(" %d = ", i);
        print_array(D[i]);
    }
}

inline void print_constraints(const CSP& csp) {
    for (auto c : csp.constraints) printf("%s\n", c->name.c_str());
}

inline Assignment make_assignment(const array<Domain>& D) {
    Assignment A = {};
    for (int i = 0; i < D.size(); i++) {
        if (D[i].size() == 1) A[i] = D[i][0];
    }
    return A;
}

inline array<Domain> make_domains(const Assignment& A) {
    auto D = allocate_array<array<int>>((int)A.size(), {nullptr, 0});
    for (auto& kv : A) D[kv.first] = allocate_array<int>(1, kv.second);
    return D;
}

inline void apply_assignment(Domain& D, const Assignment& A) {
    for (auto& kv : A) D[kv.first] = kv.second;
}

inline void print_stats(const search_stats& stats) {
    printf("\nSearch statistics:\n");
    printf("   num_backtracks = %d\n", stats.backtracks);
    printf("   num_expansions = %d\n\n", stats.expansions);
}

inline void print_unsatisfied(const array<Domain>&            D,
                              const array<const Constraint*>& C) {
    printf("unsatisfied constraints: ");
    // for(auto& d : D) assert(d.size() == 1);
    bool found = false;
    for (int i = 0; i < C.size(); ++i) {
        if (not C[i]->eval(D)) {
            found = true;
            printf("\n%d: %s\n", i, C[i]->name.c_str());
        }
    }
    if (not found) printf("nothing\n");
    printf("\n");
}

struct AllDifferent : Constraint {
    AllDifferent(const array<int>& vars, std::string n = "all_different")
        : Constraint(vars, n) {}

    bool eval(const array<Domain>& D) const {
        for (int i = 0; i < scope.size() - 1; ++i) {
            int v = scope[i];
            if (D[v].size() != 1) continue;
            for (int k = i + 1; k < scope.size(); ++k) {
                int w = scope[k];
                if (D[w].size() == 1)
                    if (D[v][0] == D[w][0]) return false;
            }
        }

        return true;
    }

    bool propagate(array<Domain>& D) const {
        for (int v : scope) {
            if (D[v].size() != 1) continue;
            for (int w : scope) {
                if (w == v) continue;
                for (int i = 0; i < D[w].size(); ++i)
                    if (D[w][i] == D[v][0]) {
                        remove(D[w], i);
                        if (D[w].size() == 0) return false;
                        break;
                    }
            }
        }
        return true;
    }
};

struct Binary : Constraint {
    std::function<bool(int, int)> rel;

    Binary(int i, int k, std::function<bool(int, int)> r,
           std::string n = "binary") {
        scope = allocate_array<int>(2);
        scope = {i, k};
        rel   = r;
        name  = n;
    }

    bool eval(const array<Domain>& D) const {
        int i = scope[0];
        int k = scope[1];
        if (D[i].size() == 1 and D[k].size() == 1) {
            if (not rel(D[i][0], D[k][0])) return false;
        }

        return true;
    };

    bool propagate(array<Domain>& D) const {
        stack_frame();
        int x0 = scope[0];
        int x1 = scope[1];
        // Domain d0, d1;
        auto d0  = allocate_array<int>(D[x0].size() * D[x1].size());
        auto d1  = allocate_array<int>(D[x0].size() * D[x1].size());
        d0.count = 0;
        d1.count = 0;
        // std::set d0, d1; // @Try with std::set, code will be simpler.
        for (int v0 : D[x0]) {
            bool found = false;
            for (int v1 : D[x1]) {
                if (rel(v0, v1)) {
                    if (not contains(d1, v1)) d1.push_back(v1);
                    found = true;
                }
            }
            if (found) d0.push_back(v0);
        }
        if (d0.size() == 0) return false;
        if (d1.size() == 0) return false;
        copy_to(d0, D[x0]);
        copy_to(d1, D[x1]);
        return true;
    }
};

struct Equal : Constraint {
    Equal(int i, int k, const std::string name = "equal") {
        scope      = allocate_array<int>(2);
        scope      = {i, k};
        this->name = name;
    }

    bool eval(const array<Domain>& D) const {
        int i = scope[0];
        int k = scope[1];
        if (D[i].size() == 1 and D[k].size() == 1) {
            if (D[i][0] != D[k][0]) return false;
        }
        return true;
    }

    bool propagate(array<Domain>& D) const {
        stack_frame();
        auto intersection  = allocate_array<int>(D[scope[0]].size());
        intersection.count = 0;
        for (int v0 : D[scope[0]]) {
            if (contains(D[scope[1]], v0)) intersection.push_back(v0);
        }
        if (intersection.size() == 0) return false;
        copy_to(intersection, D[scope[0]]);
        copy_to(intersection, D[scope[1]]);
        return true;
    }
};
