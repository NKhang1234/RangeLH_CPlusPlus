cmake_minimum_required(VERSION 3.14)
project(RangeLH_Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Locate GoogleTest
find_package(GTest REQUIRED)
include(GoogleTest)

# 2. Include directory for headers
include_directories(Implement)
include_directories(BPlusTree)

enable_testing()

# ---------------------------------------------------
# 3. BloomRF Component
# ---------------------------------------------------
add_library(BloomRF_lib Implement/bloomRF.cpp)

add_executable(bloomRF_test UnitTest/bloomRF_test.cpp)
target_link_libraries(bloomRF_test 
    BloomRF_lib 
    GTest::gtest_main 
    pthread
)
gtest_discover_tests(bloomRF_test)

# ---------------------------------------------------
# 4. Linear Hashing Component
# ---------------------------------------------------
# Create the library for Linear Hashing logic
add_library(LinearHashing_lib Implement/linearHashing.cpp)

# Create the test executable
add_executable(linearHashing_test UnitTest/linearHashing_test.cpp)

# Link the test to its library and GTest
target_link_libraries(linearHashing_test 
    LinearHashing_lib 
    GTest::gtest_main 
    pthread
)
gtest_discover_tests(linearHashing_test)

# ---------------------------------------------------
# 5. RangeLH Component
# ---------------------------------------------------
# Create the library for RangeLH logic
add_library(RangeLH_lib 
    Implement/rangeLH.cpp 
    Implement/linearHashing.cpp 
    Implement/bloomRF.cpp
)

# Create the test executable
add_executable(rangeLH_test UnitTest/rangeLH_test.cpp)

# Link the test to its library and GTest
target_link_libraries(rangeLH_test 
    RangeLH_lib 
    GTest::gtest_main 
    pthread
)
gtest_discover_tests(rangeLH_test)

# ---------------------------------------------------
# 6. B+ Tree Indexing
# ---------------------------------------------------
# Define an INTERFACE library for header-only templates
add_library(bPlusTree_lib 
    BPlusTree/bPlusTree.cpp
)

# Create the test executable
add_executable(bPlusTree_test UnitTest/bPlusTree_test.cpp)

# Link the test to its library and GTest
target_link_libraries(bPlusTree_test 
    bPlusTree_lib 
    GTest::gtest_main 
    pthread
)
gtest_discover_tests(bPlusTree_test)

# ---------------------------------------------------
# 7. Benchmark Section
# ---------------------------------------------------

# Find Google Benchmark
find_package(benchmark REQUIRED)

# Benchmark executable
add_executable(index_benchmark
    Benchmark/benchmark.cpp
)

# Link all indexing libraries
target_link_libraries(index_benchmark
    RangeLH_lib
    bPlusTree_lib
    benchmark::benchmark
    pthread
)

# Ensure optimized build for benchmarking
target_compile_options(index_benchmark PRIVATE -O3 -march=native)




# Configure: cmake ..
# Build: make

# Run all tests
# ctest --output-on-failure

# Run individual test executables
# ./bloomRF_test
# ./linearHashing_test
# ./rangeLH_test
# ./bPlusTree_test
# ./index_benchmark