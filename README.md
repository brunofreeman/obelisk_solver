# obelisk_solver

`obelisk_solver` is a C++ program that I quickly programmed to brute-force solve [Project Genius's Obelisk Puzzle](https://www.projectgeniusinc.com/true-genius-collection).
The solver outputs tiles in ascending order (although by the nature of the puzzle, flipping a solution does not invalidate it).
Each tile in the solution is output to the console in the following format:
```txt
---------------------
|||||   |   |   |||||
---------------------
|   |           |   |
---------------------
|   |           |   |
---------------------
|   |           |   |
---------------------
|||||   |   |   |||||
---------------------
```
Puzzle tiles may have peg holes in any of twelve locations, represented by the twelve spaces in the above diagram (three along each side of the ASCII tile).
If a given space is left blank (as above), no hole is present there for the associated tile.
Otherwise, the space will be filled with a `1`, `2`, or `3`.
These numbers indicate that the given holes should be filled with a peg of the corresponding length.
The puzzle has one peg of length `1`, nine of length `2`, and four of length `3` to fill the thirty-one holes distributed among the nine tiles.
