cmake_minimum_required(VERSION 3.28)
project(project_solution_finder_ultimate_backend)

set(CMAKE_CXX_STANDARD 26)

add_executable(project_solution_finder_ultimate_backend main.cpp
        restapp.cpp
        restapp.h)

find_package(cpprestsdk REQUIRED)
find_package(SQLite3 REQUIRED)

target_link_libraries(project_solution_finder_ultimate_backend PUBLIC
        cpprestsdk::cpprest
        SQLite::SQLite3
)