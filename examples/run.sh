# if [[ $# -le 1 ]]; then
#     echo "Illegal number of arguments. These are the possible options:"
#     echo "    nqueens <N> -- Run nqueens with N queens"
#     echo "    sudoku      -- Run sudoku"
#     exit 2
# fi

mkdir -p bin
mkdir -p build

COMPILE="g++ -std=c++1z -O2"
MAIN=$1
LIBS="build/csp.o"

$COMPILE -o $LIBS ../csp.cpp -c
$COMPILE -o bin/$MAIN $MAIN.cpp $LIBS
time ./bin/$MAIN "${@:2}"
