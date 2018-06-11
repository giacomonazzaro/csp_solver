#include "../csp.h"

inline void print_edge_coloring(const Assignment& A) {
    int N = int(std::sqrt(A.size()/2));
    for(int i = 0; i<N; i++) {
        for(int k = 0; k<N; k++) {
            int edge = A.at(i * N + k);
            printf(" %d", edge);
        }
        printf("\n");
    }
    printf("\n");


    for(int i = 0; i<N; i++) {
        for(int k = 0; k<N; k++) {
            int color = A.at(N*N + i * N + k);
            printf(" %d", color);
        }
        printf("\n");
    }
    printf("\n");
}

CSP make_edge_coloring(int N) {
    array<Domain> D;
    auto M = array<Domain>(N*N, make_range(N));
    auto colors = array<Domain>(N*N, make_range(3));
    append(D, M); // Adjacency matrix.
    append(D, colors); // Colors.
    CSP csp;
    for (int i = 0; i < N*N; ++i) {
        int row = i / N;
        int col = i % N;
        int v0 = row * N + col; 
        int v1 = col * N + row; 
        add_constraint(csp, equal(v0, v1));
    }

    for (int i = 0; i < N*N-2; ++i) {
        for (int k = i+1; k < N*N-1; ++k) {
            for (int j = k+1; j < N*N; ++j) {
                std::function<bool(const array<Domain>& D)> rel = [](const array<Domain>& D) {
                    for(int i : vars) if(D[i].size() != 1) return true;
                    int edge_ik = 

                    if(edge_ik and edge_ij and edge_kj) {
                        if(color_ik == color_ij and color_ij == color_kj)
                            return false;
                    }

            }
            add_constraint(csp, equal(v0, v1));
    }

    csp.domains = D;
    return csp;
}

int main(int argc, char const *argv[])
{
    int N = 4;
    CSP csp = make_edge_coloring(N);
    printf("%d\n", csp.domains.size());
    search_stats stats;
    auto solution = search(csp, {{2, 1}}, stats);
    print_edge_coloring(solution);
}