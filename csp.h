#pragma once
#include "utils/stack_allocator.h"
#include "utils/string.h"

using Domain = array<int>;
struct assignment {
    int variable;
    int value;
};

using Assignment = array<assignment>;

enum constraint_type {
    ALL_DIFFERENT,
    BINARY,
    NARY,
    // DIFFERENT,
    // DIFFERENT_CONST,
    // RELATION,
    // CUSTOM
};

struct Constraint {
    string     name;
    array<int> scope;
    array<int> constants;
    bool (*eval_custom)(const Constraint&, const array<int>&) = nullptr;
    constraint_type type;

    Constraint(constraint_type t, const array<int>& vars, const string& s) {
        type  = t;
        scope = copy(vars);
        name  = s;
        name += "(";

        for (int i = 0; i < scope.size() - 1; ++i) {
            name += string(scope[i]);
            name += ", ";
        }
        name += string(scope.back());
        name += ")";
    }
};

inline bool eval(const Constraint& constraint, const array<Domain>& domains);
inline bool propagate(const Constraint& constraint, array<Domain>& domains);

struct CSP {
    string            name;
    array<Domain>     domains;
    array<Constraint> constraints;
};

struct search_stats {
    int backtracks = 0;
    int expansions = 0;
};

/***** Solving functions *****/
// Check if assignment satisfies the constraints.
bool satisfies(const array<Constraint>& C, const array<Domain>& A);
bool satisfies(const array<Constraint>& C, const Assignment& A);

// Search satisfying assignment.
bool       search(const array<Constraint>& C, array<Domain>& D, int depth);
Assignment search(const CSP& csp, const array<Domain>& assignment,
                  search_stats& stats);
inline Assignment search(const CSP& csp, search_stats& stats) {
    return search(csp, csp.domains, stats);
}

// Choose next variable (MRV & MaxDegree heuristics).
int choose_variable(const array<Domain>& D, const array<Constraint>& C);

// Make inferences after assignment (Genrealized Arc Consistency).
bool constraints_propagation(const array<Constraint>& C, array<Domain>& D);
bool gac3(const array<Constraint>& C, array<Domain>& D);
bool remove_values(int variable, const Constraint& constraint, array<Domain>& D,
                   array<Domain> A);
bool search_small(Constraint c, const array<Domain>& D_, int depth);

/***** CSP intialization functions. *****/
inline CSP make_csp(const string& s, const array<Domain>& d,
                    int num_constraints) {
    CSP csp;
    csp.name              = s;
    csp.domains           = d;
    csp.constraints       = allocate_array<Constraint>(num_constraints);
    csp.constraints.count = 0;
    return csp;
}

inline void add_constraint(CSP& csp, const Constraint& c) {
    csp.constraints.push_back(c);
}

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

inline void print_constraints(const array<Constraint>& constraints) {
    for (auto& c : constraints) {
        write(c.name);
        print_array(c.scope);
        write("");
    }
    write("\n");
}

inline Assignment make_assignment(const array<Domain>& D) {
    auto A = allocate_array<assignment>(D.count);
    A.count = 0;
    for (int i = 0; i < D.size(); i++) {
        if (D[i].size() == 1) A.push_back({i, D[i][0]});
    }
    return A;
}

/*
inline array<Domain> make_domains(const Assignment& A) {
    auto D = allocate_arrays<int>((int)A.size(), );
    for (auto& a : A) {
        D[a.variable] = allocate_array<assi>(1, A[i]);
    }
    return D;
}*/

inline void apply_assignment(array<Domain>& D, const Assignment& A) {
    for (auto& a : A)
        D[a.variable] = {a.value};
}

inline void print_stats(const search_stats& stats) {
    printf("\nSearch statistics:\n");
    printf("   num_backtracks = %d\n", stats.backtracks);
    printf("   num_expansions = %d\n\n", stats.expansions);
}

// Constraint all_different
inline Constraint all_different(const array<int>& scope,
                                const string&     name = "all_different") {
    auto result = Constraint(ALL_DIFFERENT, scope, name);
    return result;
}

inline bool eval_all_different(const Constraint&    constraint,
                               const array<Domain>& D) {
    for (int i = 0; i < constraint.scope.size() - 1; ++i) {
        int v = constraint.scope[i];
        if (D[v].size() != 1) continue;
        for (int k = i + 1; k < constraint.scope.size(); ++k) {
            int w = constraint.scope[k];
            if (D[w].size() == 1)
                if (D[v][0] == D[w][0]) return false;
        }
    }
    return true;
}

inline bool propagate_all_different(const Constraint& constraint,
                                    array<Domain>&    D) {
    for (int v : constraint.scope) {
        if (D[v].size() != 1) continue;
        for (int w : constraint.scope) {
            if (w == v) continue;
            for (int i = 0; i < D[w].size(); ++i)
                if (D[w][i] == D[v][0]) {
                    D[w].remove(i);
                    if (D[w].size() == 0) return false;
                    break;
                }
        }
    }
    return true;
}

inline bool eval_binary(const Constraint&    constraint,
                        const array<Domain>& domains) {
    int x = constraint.scope[0];
    int y = constraint.scope[1];
    if (domains[x].count == 1 and domains[y].count == 1) {
        stack_frame();
        auto xy = allocate_array({domains[x][0], domains[y][0]});
        if (not constraint.eval_custom(constraint, xy)) return false;
    }
    return true;
}

inline bool propagate_binary(const Constraint& constraint, array<Domain>& D) {
    stack_frame();
    int x0 = constraint.scope[0];
    int x1 = constraint.scope[1];
    // Domain d0, d1;
    auto d0  = allocate_array<int>(D[x0].size() * D[x1].size());
    auto d1  = allocate_array<int>(D[x0].size() * D[x1].size());
    d0.count = 0;
    d1.count = 0;
    // std::set d0, d1; // @Try with std::set, code will be simpler.
    for (int v0 : D[x0]) {
        bool found = false;
        for (int v1 : D[x1]) {
            stack_frame();
            auto xy = allocate_array({v0, v1});
            if (constraint.eval_custom(constraint, xy)) {
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

inline bool eval_nary(const Constraint&    constraint,
                      const array<Domain>& domains) {
    for (auto var : constraint.scope)
        if (domains[var].count != 1) return true;

    stack_frame();
    auto values = allocate_array<int>(constraint.scope.count);
    for (int i = 0; i < constraint.scope.count; ++i) {
        values[i] = domains[constraint.scope[i]][0];
    }

    return constraint.eval_custom(constraint, values);
}

inline bool propagate_nary(const Constraint& constraint, array<Domain>& D) {
    return true;
}

// Constraint equal(int x, int y, const string& name = "equal") {
//     auto result  = Constraint(EQUAL, name);
//     result.scope = allocate_array({x, y});
//     return result;
// }

// bool eval_equal(const Constraint& constraint, const array<Domain>& D) {
//     int i = constraint.scope[0];
//     int k = constraint.scope[1];
//     if (D[i].size() == 1 and D[k].size() == 1) {
//         if (D[i][0] != D[k][0]) return false;
//     }
//     return true;
// }

// bool propagate_equal(const Constraint& constraint, array<Domain>& D) {
//     stack_frame();
//     auto& scope        = constraint.scope;
//     auto  intersection = allocate_array<int>(D[scope[0]].size());
//     intersection.count = 0;
//     for (int v0 : D[scope[0]]) {
//         if (contains(D[scope[1]], v0)) intersection.push_back(v0);
//     }
//     if (intersection.size() == 0) return false;
//     copy_to(intersection, D[scope[0]]);
//     copy_to(intersection, D[scope[1]]);
//     return true;
// }

// Constraint different(int x, int y, const string& name = "different") {
//     auto result  = Constraint(DIFFERENT, name);
//     result.scope = allocate_array({x, y});
//     return result;
// }

// bool eval_different(const Constraint& constraint, const array<Domain>& D) {
//     int i = constraint.scope[0];
//     int k = constraint.scope[1];
//     if (D[i].size() == 1 and D[k].size() == 1) {
//         if (D[i][0] == D[k][0]) return false;
//     }
//     return true;
// }

// bool propagate_different(const Constraint& constraint, array<Domain>& D) {
//     return true;
// }

// Constraint equal_const(int x, int val, const string& name = "equal_const") {
//     auto result      = Constraint(EQUAL_CONST, name);
//     result.scope     = allocate_array({x});
//     result.constants = allocate_array({val});
//     return result;
// }

// bool eval_equal_const(const Constraint& constraint, const array<Domain>& D) {
//     int i = constraint.scope[0];
//     if (D[i].size() == 1) {
//         if (D[i][0] != constraint.constants[0]) return false;
//     }
//     return true;
// }

// bool propagate_equal_const(const Constraint& constraint, array<Domain>& D) {
//     stack_frame();
//     auto& scope        = constraint.scope;
//     auto  intersection = allocate_array<int>(D[scope[0]].size());
//     intersection.count = 0;
//     for (int v0 : D[scope[0]]) {
//         if (contains(D[scope[1]], v0)) intersection.push_back(v0);
//     }
//     if (intersection.size() == 0) return false;
//     copy_to(intersection, D[scope[0]]);
//     copy_to(intersection, D[scope[1]]);
//     return true;
// }

inline bool eval(const Constraint& constraint, const array<Domain>& domains) {
    auto type = constraint.type;
    // if (type == RELATION)assert(0);  // return eval_relation(constraint,
    // domains);
    if (type == ALL_DIFFERENT) return eval_all_different(constraint, domains);
    // if (type == EQUAL) return eval_equal(constraint, domains);
    if (type == BINARY) return eval_binary(constraint, domains);
    if (type == NARY) return eval_nary(constraint, domains);
    // if (type == CUSTOM) return eval_custom(constraint, domains);
    return false;
}

inline bool propagate(const Constraint& constraint, array<Domain>& domains) {
    // if (type == RELATION) return propagate_relation(constraint, domains);
    if (constraint.type == ALL_DIFFERENT)
        return propagate_all_different(constraint, domains);
    // if (type == EQUAL) return propagate_equal(constraint, domains);
    if (constraint.type == BINARY) return propagate_binary(constraint, domains);
    if (constraint.type == NARY) return propagate_nary(constraint, domains);
    return false;
}

inline void print_unsatisfied(const array<Domain>&     D,
                              const array<Constraint>& C) {
    printf("unsatisfied constraints: ");
    // for(auto& d : D) assert(d.size() == 1);
    bool found = false;
    for (int i = 0; i < C.size(); ++i) {
        if (not eval(C[i], D)) {
            found = true;
            printf("\n%d: %s\n", i, (const char*)C[i].name);
        }
    }
    if (not found) printf("nothing\n");
    printf("\n");
}
