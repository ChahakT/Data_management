cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_COMPILER /usr/bin/mpic++)

project(main)


add_compile_options(
-std=c++17
-Wno-unused-function
-Wno-unused-parameter
-Wno-reorder
-Wall -Wextra -pedantic -g
-O3
)

add_library(reduce STATIC
        src/reduce_tree.cpp
		)
include_directories(main include)

add_executable(main
        src/main.cpp
		src/simple_agg.cpp
		src/smart_agg.cpp
)

target_link_libraries(main PRIVATE reduce)

