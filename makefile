COMPLILE = g++ -std=c++11 -O3

all: nqueens

nqueens: csp.o
	$(COMPLILE) -o bin/nqueens examples/nqueens.cpp build/csp.o 
	time ./bin/nqueens

sudoku: csp.o
	$(COMPLILE) -o bin/sudoku examples/sudoku.cpp build/csp.o
	time ./bin/sudoku

test: sudoku nqueens

csp.o: csp.cpp csp.h
	$(COMPLILE) -o build/csp.o csp.cpp -c

clean:
	rm -r bin
	rm -r build

$(shell   mkdir bin build)
