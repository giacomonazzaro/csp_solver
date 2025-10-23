cmake_minimum_required(VERSION 3.10)

# Project name
project(CSPSolver)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
set(SOURCES
    csp.cpp
)

# Add the header files
set(HEADERS
    csp.h
)

# Add executable for nqueens
add_executable(nqueens examples/nqueens.cpp ${SOURCES} ${HEADERS})

# Add executable for sudoku
add_executable(sudoku examples/sudoku.cpp ${SOURCES} ${HEADERS})

add_executable(tiles examples/tiles.cpp ${SOURCES} ${HEADERS})