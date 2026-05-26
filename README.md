# Indexing Structures Comparison

This project introduces a new indexing structure named **RangeLH** and evaluates its performance against the traditional **B+ Tree**. It provides a comparative benchmark for key-value storage and retrieval, with a focus on range query efficiency.

## Implemented Structures

The following indexing and storage structures are included:

- **B+ Tree**: A balanced tree structure for on-disk storage that maintains sorted data and allows for efficient insertion, deletion, and search operations.
- **Linear Hashing**: A dynamic hashing scheme that expands the hash table gracefully as the number of records grows.
- **Bloom Filter (BloomRF)**: A space-efficient probabilistic data structure used to test whether an element is a member of a set. False positive matches are possible, but false negatives are not.
- **Range-based Linear Hashing (RangeLH)**: An extension of Linear Hashing that incorporates Bloom filters to optimize range queries.
- **Storage Layer**: A component that simulates a storage system, which the indexing structures use to manage data.

## Dependencies

To build and run this project, you will need:

- **CMake** (version 3.14 or higher)
- A **C++17 compatible compiler** (e.g., GCC, Clang)
- **Google Test**: For running unit tests. The project will try to find it using `find_package(GTest REQUIRED)`.
- **Google Benchmark**: For running the performance benchmarks. The project will try to find it using `find_package(benchmark REQUIRED)`.
- **pthreads**: The POSIX threads library.

## Building the Project

Follow these steps to build the project from source:

1.  **Clone the repository:**

    ```bash
    git clone <repository-url>
    cd <repository-directory>
    ```

2.  **Create a build directory:** It's good practice to build the project in a separate directory.

    ```bash
    mkdir build
    cd build
    ```

3.  **Configure with CMake:** Run CMake from the `build` directory to generate the build files.

    ```bash
    cmake ..
    ```

4.  **Compile the code:** Use `make` to compile all the libraries, tests, and benchmark executables.
    ```bash
    make
    ```

## Running Tests

The project includes a suite of unit tests for each component. To run all tests, execute the following command from the `build` directory:

```bash
ctest --output-on-failure
```

You can also run test executables for individual components:

```bash
./bloomRF_test
./linearHashing_test
./rangeLH_test
./bPlusTree_test
./storage_test
```

## Running the Benchmarks

The `index_benchmark` executable runs performance tests on the `RangeLH` and `bPlusTree` indexing structures.

To run the benchmarks, execute the following command from the `build` directory:

```bash
./index_benchmark
```

The benchmark executable is compiled with `-O3` and `-march=native` flags for optimized performance. The output will show the results of the different benchmark cases, allowing for a comparison of the indexing structures.
