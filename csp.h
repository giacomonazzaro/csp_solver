#pragma once
#include "utils/stack_allocator.h"
#include "utils/string.h"
using namespace giacomo;

struct Constraint {
    enum type { ALL_DIFFERENT, BINARY, NARY, UNARY };

    // string     name;
    string     name      = {};
    array<int> scope     = {};
    array<int> constants = {};
    type       type      = UNARY;

    bool (*eval_custom)(const Constraint&, const array<int>&) = nullptr;

    inline Constraint() {}
    inline Constraint(enum type t, const array<int>& vars, string s);
};

using Domain = array<int>;

inline bool eval(const Constraint& constraint, const array<Domain>& domains);
inline bool propagate(const Constraint& constraint, array<Domain>& domains);

struct CSP {
    string            name;
    array<Domain>     domains;
    array<Constraint> constraints;
    array<array<int>> variable_to_constraints;
};

struct search_stats {
    int backtracks = 0;
    int expansions = 0;
};

struct assignment {
    int variable;
    int value;
};
using Assignment = array<assignment>;

void add_constraint(CSP& csp, const Constraint& constraint);

// Check if assignment satisfies the constraints.
bool satisfies(const array<Constraint>& C, const array<Domain>& A);

// Search satisfying assignment.
bool search(const array<Constraint>& C, array<Domain>& D, int depth,
            search_stats& stats);

Assignment search(const CSP& csp, const Assignment& assignment,
                  search_stats& stats);

bool search_single_constraint(const Constraint& c, const array<Domain>& D,
                              int depth);

// Choose next variable to assign (MRV & MaxDegree heuristics).
int choose_variable(const array<Domain>& D, const array<Constraint>& C);

// Propagate consequences after assignment in order to reduce domains.
bool constraints_propagation(const array<Constraint>& C, array<Domain>& D);
bool gac3(const array<Constraint>& C, array<Domain>& D);
bool remove_values(int variable, const Constraint& constraint, array<Domain>& D,
                   array<Domain> A);

// Initialize CSP.
inline CSP make_csp(const string& name, const array<Domain>& domains,
                    int num_constraints) {
    CSP csp;
    csp.name        = name;
    csp.domains     = domains;
    csp.constraints = allocate<Constraint>(num_constraints);
    csp.constraints.resize(0);

    csp.variable_to_constraints = allocate<array<int>>(domains.size());
    for (int i = 0; i < domains.size(); ++i) {
        csp.variable_to_constraints[i] = allocate<int>(num_constraints);
        csp.variable_to_constraints[i].resize(0);
    }
    return csp;
}

// Useful functions to initialize domains.
inline array<int> make_range(int from, int to) {
    auto result = allocate<int>(to - from);
    for (int i = 0; i < to - from; i++) result[i] = from + i;
    return result;
}

inline array<int> make_range(int to) { return make_range(0, to); }

// Printing functions.
inline void print_array(const array<int>& d) {
    printf("[");
    for (int i = 0; i < d.size() - 1; ++i) printf("%d, ", d[i]);
    printf("%d]\n", d.back());
}

inline void print_domains(const array<Domain>& domains) {
    printf("\ndomains:\n");
    for (int i = 0; i < domains.size(); ++i) {
        printf("%d: ", i);
        print_array(domains[i]);
    }
}

inline void print_state(const array<Domain>& D, int depth = 0) {
    for (int i = 0; i < D.size(); ++i) {
        for (int k = 0; k < depth; ++k) printf("-");
        printf(" %d = ", i);
        print_array(D[i]);
    }
}

inline void print_constraints(const array<Constraint>& constraints) {
    for (auto& c : constraints) {
        write(c.name);
        // print_array(c.scope);
        write("");
    }
    write("\n");
}

inline Assignment make_assignment(const array<Domain>& D) {
    auto A = allocate<assignment>(D.size());
    A.resize(0);
    for (int i = 0; i < D.size(); i++) {
        if (D[i].size() == 1) A.push_back({i, D[i][0]});
    }
    return A;
}

inline void apply_assignment(array<Domain>& D, const Assignment& A) {
    for (auto& a : A) D[a.variable] = {a.value};
}

inline void print_stats(const search_stats& stats) {
    printf("\nSearch statistics:\n");
    printf("   num_backtracks = %d\n", stats.backtracks);
    printf("   num_expansions = %d\n\n", stats.expansions);
}

inline Constraint::Constraint(enum type t, const array<int>& vars, string s)
    : type(t), scope(copy(vars)), name(s), constants({}) {
    type  = t;
    scope = copy(vars);
    name  = s;
    name += "(";

    for (int i = 0; i < scope.size() - 1; ++i) {
        name += to_string(scope[i]);
        name += ", ";
    }
    name += to_string(scope.back());
    name += ")";
}

// Constraint all_different
inline Constraint all_different(const array<int>& scope,
                                const string&     name = "all_different") {
    auto result = Constraint(Constraint::ALL_DIFFERENT, scope, name);
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
            for (int i = 0; i < D[w].size(); ++i) {
                if (D[w][i] == D[v][0]) {
                    D[w].remove(i);
                    if (D[w].size() == 0) return false;
                    break;
                }
            }
        }
    }

    // Enhanced propagation: find naked pairs.
    // If two variables in scope have the same domain of size 2,
    // remove those two values from all other variables in scope.
    // stack_frame();
    // auto& scope = constraint.scope;
    // for (int i = 0; i < scope.size(); ++i) {
    //     int v1 = scope[i];
    //     if (D[v1].size() == 2) {
    //         for (int j = i + 1; j < scope.size(); ++j) {
    //             int v2 = scope[j];
    //             if (D[v2].size() == 2 && contains(D[v1], D[v2][0]) &&
    //                 contains(D[v1], D[v2][1])) {
    //                 // Naked pair (v1, v2) found.
    //                 int val1 = D[v1][0];
    //                 int val2 = D[v1][1];
    //                 for (int k = 0; k < scope.size(); ++k) {
    //                     int v_other = scope[k];
    //                     if (v_other == v1 || v_other == v2) continue;

    //                     auto& d_other = D[v_other];
    //                     auto  d_new   = allocate<int>(d_other.size());
    //                     d_new.resize(0);
    //                     bool changed = false;
    //                     for (int val : d_other) {
    //                         if (val != val1 && val != val2) {
    //                             d_new.push_back(val);
    //                         } else {
    //                             changed = true;
    //                         }
    //                     }

    //                     if (changed) {
    //                         if (d_new.size() == 0) return false;
    //                         copy_to(d_new, d_other);
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    return true;
}

inline bool eval_unary(const Constraint&    constraint,
                       const array<Domain>& domains) {
    int x = constraint.scope[0];
    if (domains[x].size() == 1) {
        stack_frame();
        auto value = allocate<int>({domains[x][0]});
        if (not constraint.eval_custom(constraint, value)) return false;
    }
    return true;
}

inline bool eval_binary(const Constraint&    constraint,
                        const array<Domain>& domains) {
    int x = constraint.scope[0];
    int y = constraint.scope[1];
    if (domains[x].size() == 1 and domains[y].size() == 1) {
        stack_frame();
        auto xy = allocate({domains[x][0], domains[y][0]});
        if (not constraint.eval_custom(constraint, xy)) return false;
    }
    return true;
}

inline bool propagate_unary(const Constraint& constraint, array<Domain>& D) {
    stack_frame();
    int x = constraint.scope[0];

    auto domain_new = allocate<int>(D[x].size());
    domain_new.resize(0);
    for (auto value : D[x]) {
        stack_frame();
        auto v = allocate({value});
        if (constraint.eval_custom(constraint, v)) {
            domain_new.push_back(value);
        }
    }
    if (domain_new.size() == 0) return false;
    copy_to(domain_new, D[x]);
    return true;
}

inline bool propagate_binary(const Constraint& constraint, array<Domain>& D) {
    stack_frame();
    int x0 = constraint.scope[0];
    int x1 = constraint.scope[1];

    // Revise domain of x0 based on x1
    auto d0_new = allocate<int>(D[x0].size());
    d0_new.resize(0);
    for (int v0 : D[x0]) {
        bool found_support = false;
        for (int v1 : D[x1]) {
            stack_frame();
            auto xy = allocate({v0, v1});
            if (constraint.eval_custom(constraint, xy)) {
                found_support = true;
                break;
            }
        }
        if (found_support) {
            d0_new.push_back(v0);
        }
    }

    if (d0_new.size() < D[x0].size()) {
        if (d0_new.size() == 0) return false;
        copy_to(d0_new, D[x0]);
    }

    // Revise domain of x1 based on x0 (the potentially pruned one)
    auto d1_new = allocate<int>(D[x1].size());
    d1_new.resize(0);
    for (int v1 : D[x1]) {
        bool found_support = false;
        for (int v0 : D[x0]) {
            stack_frame();
            auto xy = allocate({v0, v1});
            if (constraint.eval_custom(constraint, xy)) {
                found_support = true;
                break;
            }
        }
        if (found_support) {
            d1_new.push_back(v1);
        }
    }

    if (d1_new.size() < D[x1].size()) {
        if (d1_new.size() == 0) return false;
        copy_to(d1_new, D[x1]);
    }

    return true;
}

inline bool eval_nary(const Constraint&    constraint,
                      const array<Domain>& domains) {
    for (auto var : constraint.scope)
        if (domains[var].size() != 1) return true;

    stack_frame();
    auto values = allocate<int>(constraint.scope.size());
    for (int i = 0; i < constraint.scope.size(); ++i) {
        values[i] = domains[constraint.scope[i]][0];
    }

    return constraint.eval_custom(constraint, values);
}

inline bool propagate_nary(const Constraint& constraint, array<Domain>& D) {
    return true;
}

// Constraint equal(int x, int y, const string& name = "equal") {
//     auto result  = Constraint(Constraint::EQUAL, name);
//     result.scope = allocate({x, y});
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
//     auto  intersection = allocate<int>(D[scope[0]].size());
//     intersection.size() = 0;
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
//     result.scope = allocate({x, y});
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
//     result.scope     = allocate({x});
//     result.constants = allocate({val});
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
//     auto  intersection = allocate<int>(D[scope[0]].size());
//     intersection.size() = 0;
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
    if (type == Constraint::ALL_DIFFERENT)
        return eval_all_different(constraint, domains);
    // if (type == Constraint::EQUAL) return eval_equal(constraint, domains);
    if (type == Constraint::BINARY) return eval_binary(constraint, domains);
    if (type == Constraint::NARY) return eval_nary(constraint, domains);
    if (type == Constraint::UNARY) return eval_unary(constraint, domains);
    // if (type == Constraint::CUSTOM) return eval_custom(constraint, domains);
    return false;
}

inline bool propagate(const Constraint& constraint, array<Domain>& domains) {
    // if (type == RELATION) return propagate_relation(constraint, domains);
    if (constraint.type == Constraint::ALL_DIFFERENT)
        return propagate_all_different(constraint, domains);
    // if (type == Constraint::EQUAL) return propagate_equal(constraint,
    // domains);
    if (constraint.type == Constraint::BINARY)
        return propagate_binary(constraint, domains);
    if (constraint.type == Constraint::NARY)
        return propagate_nary(constraint, domains);
    if (constraint.type == Constraint::UNARY)
        return propagate_unary(constraint, domains);
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
            printf("\n%d: %s\n", i, data(C[i].name));
        }
    }
    if (not found) printf("nothing\n");
    printf("\n");
}
