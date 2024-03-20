# Project solution ultimate backend

**This is the backend part for the [Project solution ultimate](https://github.com/lucley64/project-solution-finder-ultimate/tree/main)**

## C++

The c++ part is a rest api coded with [cpprestsdk](https://github.com/microsoft/cpprestsdk) and [sqlite3](https://www.sqlite.org/cintro.html)

### Compilation

**_Cmake 3.28 and ninja are required_**

To compile, simply execute this command:\
`cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -S . -B cmake-build-debug`

### Running

To run the program, run the following command: 
`./cmake-build-debug/project_solution_finder_ultimate_backend <sqlite-database-path> [--export]`

The `--export` option is used to export all the solutions in a solutions.json file

## Python

TBA