all: main.cpp
	g++ -std=c++11 -o main main.cpp -O3
	time ./main