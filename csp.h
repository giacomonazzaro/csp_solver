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

static int num_search;

struct Constraint {
    std::string name;
    vector<int> variables;
    std::function<bool(const vector<Domain>&)> eval;
};


struct CSP {
    std::string name;
    vector<Domain> domains;
    vector<Constraint> constraints;
};

// Check if assignment satisfies the constraints.
bool satisfies(const vector<Domain>& assignment, const vector<Constraint>& C);

// Search satisfying assignment.
bool search(const vector<Constraint>& C, vector<Domain>& D, int depth);
Assignment search(const CSP& csp, Assignment A);

// Choose next variable (MRV & MaxDegree heuristics).
int choose_variable(const vector<Domain>& D, const vector<Constraint>& C);

// Make inferences afeter assignment (Genrealized Arc Consistency).
vector<Domain> gac3(const vector<Constraint>& C, vector<Domain> D);
bool remove_values(int variable, const Constraint& constraint, vector<Domain>& D, vector<Domain> A);
bool search_small(const Constraint& c, vector<Domain> D, int depth);


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
    c.eval = [vars](const vector<Domain>& D) {
        for (int i = 0; i < vars.size()-1; ++i)
            for (int k = i+1; k < vars.size(); ++k) {
                int v0 = vars[i];
                int v1 = vars[k];
                if(D[v0].size() == 1 and D[v1].size() == 1)
                    if(D[v0][0] == D[v1][0])
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
    c.eval = [=](const vector<Domain>& D) {
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
    c.eval = [v0, v1](const vector<Domain>& D) {
        if(D[v0].size() == 1 and D[v1].size() == 1)
            if(D[v0][0] != D[v1][0])
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

inline void print_state(const vector<Domain>& D, int depth = 0) {
    for (int i = 0; i < D.size(); ++i) {
        print_times("-", depth);
        printf(" %d = ", i);
        //if(A.count(i) > 0) {
        //    printf("%d\n", A.at(i));
        //}
        //else
            print_domain(D[i]);
    }
}


inline Assignment make_assignment(const vector<Domain>& D) {
    Assignment A = {};
    for(int i=0; i<D.size(); i++) {
        if(D[i].size() == 1)
            A[i] = D[i][0];
    }
    return A;
}


inline void print_sudoku(const Assignment& A) {
    for(int i = 0; i<81; i++) {
        if(i%9 == 0) printf("\n");
        if(A.count(i) == 1)
            printf(" %d", A.at(i));
        else
            printf(" -");
    }
    printf("\n");
}

inline void print_sudoku(const vector<Domain>& D) {
    print_sudoku(make_assignment(D));
}



#endif

