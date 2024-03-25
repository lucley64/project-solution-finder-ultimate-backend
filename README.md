# Project solution ultimate backend

**This is the backend part for
the [Project solution ultimate](https://github.com/lucley64/project-solution-finder-ultimate/tree/main)**

## C++

The c++ part is a rest api coded with [cpprestsdk](https://github.com/microsoft/cpprestsdk)
and [sqlite3](https://www.sqlite.org/cintro.html)

### Compilation

**_Cmake 3.28 and ninja are required_**

#### Required: install cpprestsdk and sqlite3

1. cpprestsdk:

   If you have apt, run: \
   `sudo apt update` \
   `sudo apt install libcpprest-dev`

2. sqlite3: 

    Same for sqlite3: \
   `sudo apt update` \
   `sudo apt install sqlite3 libsqlite3-dev`

To compile, simply execute this command:\
`cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -S . -B cmake-build-debug`

### Running

To run the program, run the following command:
`./cmake-build-debug/project_solution_finder_ultimate_backend <sqlite-database-path> [--export]`

The `--export` option is used to export all the solutions in a solutions.json file

## Python

Create a python virtual environment using for example virtualenv 

To check if virtualenv is already installed : 
`pip list | grep virtualenv`

If virtualenv does not appear, run : 
`pip install virtualenv`

Move to the directory containing project-solution-finder-ultimate-backend : 
`cd path/to/project-solution-finder-ultimate-backend`

To create a virtual environment, execute the following command : 
`python<version> -m venv <virtual-environment-name>`

Activate the virtual environment : 
`source virtual-environment-name>/bin/activate`

Download all required librairies : 
`pip -r model/requirements.txt`