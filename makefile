all: csp.o
	g++ -std=c++11 -o example.o example.cpp csp.o -O3
	time ./example.o

csp.o: csp.cpp csp.h
	g++ -std=c++11 -o csp.o csp.cpp -O3 -c

clean:
	rm *.o