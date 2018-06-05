#pragma once
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


CSP make_csp(const std::string& s, const vector<Domain>& d, const vector<Constraint>& c = {}) {
    CSP csp;
    csp.name = s;
    csp.domains = d;
    csp.constraints = c;
    return csp;
}


bool satisfies(const Assignment& assignment, const vector<Constraint>& C) {
    for(auto& kv : assignment) {
        int variable = kv.first;
        int value = kv.second;
       
        for(auto& constraint : C) {
            if(not constraint.eval(assignment)) {
                return false;
            }
        }
    }

    return true;
}


void add_contstraint(CSP& csp, const Constraint& c) {
    csp.constraints.push_back(c);
}


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


Constraint binary(int i, int k, const std::function<bool(int, int)>& rel, const std::string& name = "") {
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


// Utilies functions.
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
void print_domain(const Domain& d) {
    printf("{ ");
    for(int val : d) printf("%d ", val);
    printf("}\n");
}

void print_domains(const vector<Domain>& D) {
    printf("domains:\n");
    for (int i = 0; i < D.size(); ++i) {
        printf("    %d: ", i);
        print_domain(D[i]);
    }
}

void print(const CSP& csp) {
    printf("csp %s\n", csp.name.c_str());
    printf("  vars deg   domain\n");
    for (int i = 0; i < csp.domains.size(); ++i) {
        printf("    %d: ", i);
        // printf(" %d  { ", csp.degrees[i]);
        for(int val : csp.domains[i]) printf("%d ", val);
        printf("}\n");
    }
}

void print_times(const char* s, int times) { for (int i = 0; i < times; ++i) printf("%s", s); }

void print(const Assignment& asg, int b = 0) {
    print_times("-", b);
    printf("assignment:\n");
    for (auto kv : asg) {
        print_times("-", b);
        printf("var %d := %d\n", kv.first, kv.second);
    }
}

void print_state(const Assignment& A, const vector<Domain>& D, int depth = 0) {
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



