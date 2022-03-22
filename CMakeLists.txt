cmake_minimum_required(VERSION 3.5)

project(main)

if (APPLE)
    set(CMAKE_CXX_COMPILER /usr/local/bin/mpic++)
    set(CMAKE_C_COMPILER /usr/local/bin/mpicc)
else ()
    set(CMAKE_CXX_COMPILER /usr/bin/mpic++)
endif ()

add_compile_options(
        -std=c++17
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-reorder
        -Wno-sign-compare
        -Wno-vla
        -Wall -Wextra -pedantic -g
        -O3
)

include_directories(main include)

add_executable(main
        src/main.cpp
        src/simple_agg.cpp
        src/smart_agg.cpp
        include/agg.h
        src/simple_intersect.cpp
        include/timing.cpp
        include/intersect.h
        src/smart_intersection.cpp)

add_subdirectory(tests)