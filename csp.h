#pragma once
#include "utils/allocator.h"
#include "utils/copy_struct.h"
#include "utils/hash_set.h"
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

    Constraint();
    Constraint(enum type t, const array<int>& vars, string s);
};

using Domain = array<int>;

bool eval(const Constraint& constraint, const array<Domain>& domains);
bool propagate(const Constraint& constraint, array<Domain>& domains,
               array<bool>& was_variable_updated);

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
// Propagation_Result gac3(const array<Constraint>& C, array<Domain>& D);
bool remove_values(int variable, const Constraint& constraint, array<Domain>& D,
                   array<Domain> A);

// Initialize CSP.
CSP make_csp(const string& name, const array<Domain>& domains,
             int num_constraints);

// Useful functions to initialize domains.
array<int> make_range(int from, int to);
array<int> make_range(int to);

// Printing functions.
void print_stats(const search_stats& stats);
void print_unsatisfied(const array<Domain>& D, const array<Constraint>& C);

Assignment make_assignment(const array<Domain>& D);
void       apply_assignment(array<Domain>& D, const Assignment& A);

// Constraint all_different
Constraint all_different(const array<int>& scope, const string& name);

bool eval_all_different(const Constraint& constraint, const array<Domain>& D);
bool propagate_all_different(const Constraint& constraint, array<Domain>& D,
                             array<bool>& was_variable_updated);

bool eval_unary(const Constraint& constraint, const array<Domain>& domains);
bool eval_binary(const Constraint& constraint, const array<Domain>& domains);
bool propagate_unary(const Constraint& constraint, array<Domain>& D,
                     array<bool>& was_variable_updated);
bool propagate_binary(const Constraint& constraint, array<Domain>& D,
                      array<bool>& was_variable_updated);
bool eval_nary(const Constraint& constraint, const array<Domain>& domains);
bool propagate_nary(const Constraint& constraint, array<Domain>& D,
                    array<bool>& was_variable_updated);
