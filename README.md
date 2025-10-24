# A simple CSP solver
Implementation of a simple solver for [constraint satisfaction problems](https://en.wikipedia.org/wiki/Constraint_satisfaction_problem).

The library is minimal, it only includes `<stdio.h>` for printing, `<cassert>` for debugging and `<initializer_list>` as array utiliy.  
The only data structures used are arrays, implemented in `utils/array.h`. Memory managment is implemented in `utils/stack_allocator.h`.

## Features and heuristics
- Backtrack search
- Minimum remaining values + max degree heuristics
- Generalized arc consistency
- Forward propagation

## Examples
Demo applications can be compiled with `make`.

### N-queens
The goal is to find an arrangement of N queens on a N by N chessboard so that no two queens threaten each other. This is a [classic toy problem](https://en.wikipedia.org/wiki/Eight_queens_puzzle) in artificial intelligence.  
Example solution for `N = 20`. Solve in less than 0.01 seconds.
```
Q • • • • • • • • • • • • • • • • • • •  
• • Q • • • • • • • • • • • • • • • • •  
• • • • Q • • • • • • • • • • • • • • •  
• • • • • • • • • • • • • • Q • • • • •  
• • • Q • • • • • • • • • • • • • • • •  
• • • • • • • • Q • • • • • • • • • • •  
• • • • • • • • • • • • Q • • • • • • •  
• • • • • • • • • • • • • • • • • Q • •  
• • • • • • • • • • • • • • • Q • • • •  
• • • • • • • • • • • • • Q • • • • • •  
• Q • • • • • • • • • • • • • • • • • •  
• • • • • • • • • • • • • • • • • • • Q  
• • • • • • • Q • • • • • • • • • • • •  
• • • • • • • • • • • • • • • • • • Q •  
• • • • • • Q • • • • • • • • • • • • •  
• • • • • • • • • • • Q • • • • • • • •  
• • • • • Q • • • • • • • • • • • • • •  
• • • • • • • • • • Q • • • • • • • • •  
• • • • • • • • • • • • • • • • Q • • •  
• • • • • • • • • Q • • • • • • • • • •  
```

### Sudoku
Hard sudoku problem. Solve in less than 0.01 seconds.
 ```
 8 • • • • • • • •     8 1 2 7 5 3 6 4 9  
 • • 3 6 • • • • •     9 4 3 6 8 2 1 7 5  
 • 7 • • 9 • 2 • •     6 7 5 4 9 1 2 8 3  
 • 5 • • • 7 • • •     1 5 4 2 3 7 8 9 6  
 • • • • 4 5 7 • •     3 6 9 8 4 5 7 2 1  
 • • • 1 • • • 3 •     2 8 7 1 6 9 5 3 4  
 • • 1 • • • • 6 8     5 2 1 9 7 4 3 6 8  
 • • 8 5 • • • 1 •     4 3 8 5 2 6 9 1 7  
 • 9 • • • • 4 • •     7 9 6 3 1 8 4 5 2  
 ```

### Tiles
Generate tile arragement given initial state and adjacency constraints.  
Input                      |  Output
:-------------------------:|:-------------------------:
![](https://github.com/user-attachments/assets/d231bee0-7ff6-4055-8061-efa1e0ba4064)  |  ![](https://github.com/user-attachments/assets/1ea03554-a972-4e30-8b97-b0c929fca584)

