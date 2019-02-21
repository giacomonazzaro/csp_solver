# A simple CSP solver
The code implements a simple [constraint satisfaction problem solver](https://en.wikipedia.org/wiki/Constraint_satisfaction_problem).

The library is minimal, it only includes `<stdio.h>` for printing, `<cassert>` for debugging and `<initializer_list>` as array utiliy. The only data structures used are arrays, implemented in `utils/array.h`. Memory managment is implemented in `utils/stack_allocator.h`.

## Features and heuristics
- Backtrack search.
- Minimum remaining values + max degree heuristics.
- Generalized Arc Consistency.
- Forward propagation.

## Examples

### N-queens (N = 20)
Solve in less than 0.01 seconds.
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

### Sudoku (hard)
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
