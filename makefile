COMPLILE = g++ -std=c++11 -O3

all: sudoku

nqueens: csp.o
	$(COMPLILE) -o examples/nqueens.o examples/nqueens.cpp csp.o 
	time ./examples/nqueens.o

sudoku: csp.o
	$(COMPLILE) -o examples/sudoku.o examples/sudoku.cpp csp.o
	time ./examples/sudoku.o

test: sudoku nqueens

csp.o: csp.cpp csp.h
	$(COMPLILE) -o csp.o csp.cpp -c

clean:
	rm *.o
	rm examples/*.o