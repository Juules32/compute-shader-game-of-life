# How to build and run on Windows
From the repository root folder:
```
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build
./compute-shader-game-of-life.exe
```
If `"MinGW Makefiles"` is not installed as a generator,
the project has also been tested with `"Visual Studio 17 2022"` and `"Ninja"`.
