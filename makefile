all: csp.o
	g++ -std=c++11 -o main.o main.cpp csp.o -O3
	time ./main.o

csp.o: csp.cpp
	g++ -std=c++11 -o csp.o csp.cpp -O3 -c

clean:
	rm *.o