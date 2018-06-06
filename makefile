all: sudoku

nqueens: csp.o
	g++ -std=c++11 -o examples/nqueens.o examples/nqueens.cpp csp.o -O3
	time ./examples/nqueens.o

sudoku: csp.o
	g++ -std=c++11 -o examples/sudoku.o examples/sudoku.cpp csp.o -O3
	time ./examples/sudoku.o

test: sudoku nqueens

csp.o: csp.cpp csp.h
	g++ -std=c++11 -o csp.o csp.cpp -O3 -c

clean:
	rm *.o
	rm examples/*.o