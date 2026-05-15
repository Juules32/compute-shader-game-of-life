# compute-shader-game-of-life
This is a project written in C++ with the purpose of demonstrating and measuring the power of parallelization in the form of compute shaders. Simulating [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) is a fitting use case, since each cell in the grid can update independently based on its current state and the state of its eight neighboring cells.

The program was developed as part of a graphics programming course at the IT University of Copenhagen in 2026.

## How to build and run on Windows
 >[!NOTE]
 > You don't have to build the application yourself! Check out [releases](https://github.com/Juules32/compute-shader-game-of-life/releases).

Requirements:
- A C++ compiler (tested with [GCC](https://gcc.gnu.org/) and [clang](https://clang.llvm.org/))
- A generator (tested with `"Ninja"`, `"Visual Studio 17 2022"` and `"MinGW Makefiles"`)
- [CMake](https://cmake.org/)

From the repository root folder:
```
cmake -B build -DCMAKE_BUILD_TYPE=Release     # Configure
cmake --build build                           # Build
./build/compute-shader-game-of-life.exe       # Run
```
