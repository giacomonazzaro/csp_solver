#ifndef _CSP_H_
#define _CSP_H_

#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <map>
#define vector std::vector
using Domain = vector<int>;
using Assignment = std::unordered_map<int, int>;

static int num_search = 0;

struct Constraint {
    std::string name;
    vector<int> variables;
    std::function<bool(const Assignment&)> eval;
};


struct CSP {
    std::string name;
    vector<Domain> domains;
    vector<Constraint> constraints;
};

// Check if assignment satisfies the constraints.
bool satisfies(const Assignment& assignment, const vector<Constraint>& C);

// Search satisfying assignment.
Assignment search(const vector<Constraint>& C, vector<Domain> D, Assignment A, int depth);
Assignment search(const CSP& csp, Assignment A);

// Choose next variable (MRV & MaxDegree heuristics).
int choose_variable(const vector<Domain>& D, const Assignment& asg, const vector<Constraint>& C);

// Make inferences afeter assignment (Genrealized Arc Consistency).
vector<Domain> gac3(const vector<Constraint>& C, const Assignment& asg, vector<Domain> D);
bool remove_values(int variable, const Constraint& constraint, vector<Domain>& D, Assignment A);
bool search_small(const Constraint& c, vector<Domain> D, Assignment A, int depth);


// Csp intialization functions.
inline CSP make_csp(const std::string& s, const vector<Domain>& d, const vector<Constraint>& c = {}) {
    CSP csp;
    csp.name = s;
    csp.domains = d;
    csp.constraints = c;
    return csp;
}


inline void add_contstraint(CSP& csp, const Constraint& c) {
    csp.constraints.push_back(c);
}


static Constraint all_different(const vector<int>& vars, const std::string& name = "") {
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


static Constraint binary(int i, int k, const std::function<bool(int, int)>& rel, const std::string& name = "") {
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


static Constraint equal(int v0, int v1, const std::string& name = "") {
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







// Utilities functions.
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


// Printing functions.
inline void print_domain(const Domain& d) {
    printf("{ ");
    for(int val : d) printf("%d ", val);
    printf("}\n");
}

inline void print_domains(const vector<Domain>& D) {
    printf("domains:\n");
    for (int i = 0; i < D.size(); ++i) {
        printf("    %d: ", i);
        print_domain(D[i]);
    }
}

inline void print_times(const char* s, int times) { for (int i = 0; i < times; ++i) printf("%s", s); }

inline void print(const Assignment& asg, int b = 0) {
    print_times("-", b);
    printf("assignment:\n");
    for (auto kv : asg) {
        print_times("-", b);
        printf("var %d := %d\n", kv.first, kv.second);
    }
}

inline void print_state(const Assignment& A, const vector<Domain>& D, int depth = 0) {
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


#endif

