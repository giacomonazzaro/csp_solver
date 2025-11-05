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
Demo applications can be compiled with `cmake`.

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
This can achive the same results of the ["wave function collapse" algorithm](https://github.com/mxgmn/WaveFunctionCollapse) used for procedural generation.

<table>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/bc8a1fd1-0ff9-41ad-bb70-1373f1745611" style="width:300px; image-rendering: pixelated;"></td>
    <td><img src="https://github.com/user-attachments/assets/a13446c9-feef-4342-a5bc-a14298b1d4e8" style="width:300px; image-rendering: pixelated;"</td>
  </tr>
</table>
