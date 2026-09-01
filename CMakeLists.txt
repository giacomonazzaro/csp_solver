cmake_minimum_required(VERSION 3.10)

# Project name
project(CSPSolver)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add the source files
set(SOURCES
    csp.cpp
    utils/allocator.cpp
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
add_executable(map examples/map.cpp ${SOURCES} ${HEADERS})
add_executable(knight_tour examples/knight_tour.cpp ${SOURCES} ${HEADERS})
