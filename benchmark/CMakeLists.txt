project(benchmark)

include_directories(test ../include)
include_directories(test ../src)

add_executable(benchmark_agg
        benchmark_agg.cpp
        ../src/smart_agg.cpp
        ../src/simple_agg.cpp)

