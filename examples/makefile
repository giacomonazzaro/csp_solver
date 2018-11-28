COMPILE = g++ -std=c++11 -O3

all: nqueens

nqueens: build/csp.o
	$(COMPILE) -o bin/nqueens nqueens.cpp build/csp.o 
	time ./bin/nqueens

sudoku: build/csp.o
	$(COMPILE) -o bin/sudoku sudoku.cpp build/csp.o
	time ./bin/sudoku

test: sudoku nqueens

build/csp.o: ../csp.cpp ../csp.h
	$(COMPILE) -o build/csp.o ../csp.cpp -c

clean:
	rm -r bin
	rm -r build

$(shell mkdir bin build)
