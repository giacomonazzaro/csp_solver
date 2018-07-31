COMPILE = g++ -std=c++11 -O3
N = 8

all: local_nqueens

nqueens: build/csp.o
	$(COMPILE) -o bin/nqueens examples/nqueens.cpp build/csp.o 
	time ./bin/nqueens $(N)

sudoku: build/csp.o
	$(COMPILE) -o bin/sudoku examples/sudoku.cpp build/csp.o
	time ./bin/sudoku

edge_coloring: build/csp.o
	$(COMPILE) -o bin/edge_coloring examples/edge_coloring.cpp build/csp.o
	time ./bin/edge_coloring

local_nqueens: build/csp.o
	$(COMPILE) -o bin/local_nqueens examples/local_nqueens.cpp build/csp.o
	time ./bin/local_nqueens $(N)

test: sudoku nqueens

build/csp.o: csp.cpp csp.h
	$(COMPILE) -o build/csp.o csp.cpp -c

clean:
	rm -r bin
	rm -r build

$(shell mkdir bin build)
