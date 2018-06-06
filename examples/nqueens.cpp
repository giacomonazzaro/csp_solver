#include "../csp.h"

CSP make_nqueens(int N = 8) {
    const array<int> range = make_range<int>(N);
    auto domains = array<Domain>(N, range);
    CSP csp = make_csp("N-Queens", domains);
    add(csp.constraints, all_different(range, "alldiff"));
    
    for (int i = 0; i < N-1; ++i) {
        for (int j = i+1; j < N; ++j) {
            std::string name = "diag("+ std::to_string(i) + ", " + std::to_string(j) +")";
            add(csp.constraints, binary(i, j, [=](int x, int y) { return x - y != j - i; }, "+"+name));
            add(csp.constraints, binary(i, j, [=](int x, int y) { return x - y != i - j; }, "-"+name));
        }
    }

    return csp;
}

inline void print_nqueens(const array<Domain>& D) {
    int N = D.size();
    for(int i = 0; i<N; i++) {
        for(int k = 0; k<N; k++) {
            if(not contains(D[i], k)) printf(" -");
            else {
                if(D[i].size() == 1)
                    printf(" Q");
                else
                    printf(" o");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void do_nqueens(int N) {
    CSP csp = make_nqueens(N);

    auto solution = search(csp, {});
    printf("solution\n");

    // if(solution.size())
        // print_nqueens(solution);
}

Assignment make_assignment_from_minizinc(array<int> D) {
    Assignment A;
    for(int i=0; i<D.size(); i++)
        A[i] = D[i]-1;
    return A;
}

int main(int argc, char const *argv[])
{
    do_nqueens(15);
}
