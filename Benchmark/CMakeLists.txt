cmake_minimum_required(VERSION 3.14)
project(index_bench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -march=native -DNDEBUG")

find_package(benchmark REQUIRED)

add_executable(index_benchmark index_benchmark.cpp)
target_link_libraries(index_benchmark benchmark::benchmark pthread)
