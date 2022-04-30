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

add_library(simple_agg STATIC src/simple_agg.cpp)
add_library(smart_agg STATIC src/smart_agg.cpp)
add_library(smart_agg_v2 STATIC src/smart_agg_v2.cpp)
add_library(simple_intersect STATIC src/simple_intersect.cpp)
add_library(smart_intersect STATIC src/smart_intersect.cpp)
add_library(simple_join STATIC src/simple_join.cpp)
add_library(smart_join STATIC src/smart_join.cpp)

add_executable(main src/main.cpp)
add_executable(benchmark_agg benchmark/benchmark_agg.cpp)
add_executable(benchmark_intersect benchmark/benchmark_intersect.cpp)

target_link_libraries(main PRIVATE simple_agg smart_agg simple_intersect smart_intersect simple_join smart_join smart_agg_v2)
target_link_libraries(benchmark_agg PRIVATE simple_agg smart_agg smart_agg_v2)
target_link_libraries(benchmark_intersect PRIVATE simple_intersect smart_intersect)
#add_subdirectory(tests)
