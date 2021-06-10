cmake_minimum_required(VERSION 3.20)
project(AshDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_UNIT_TESTS "Build unit tests (default OFF)" OFF)
option(BUILD_EXAMPLES "Build examples (default OFF)" OFF)
option(CODE_COVERAGE "Enable coverage reporting (default OFF)" OFF)
option(BUILD_BENCHMARKS "Build benchmark tests (default OFF)" OFF)

include(CMake/conan.cmake)
conan_cmake_run(REQUIRES
    boost/1.74.0
    benchmark/1.5.3
    BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
    BUILD missing
    OPTIONS
        boost:without_test=False
)
conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)

add_library(coverage_config INTERFACE)

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Add required flags (GCC & LLVM/Clang)
    target_compile_options(coverage_config INTERFACE
        -O0        # no optimization
        -g         # generate debug info
        --coverage # sets all required flags
    )

    target_link_options(coverage_config INTERFACE --coverage)
endif(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

add_subdirectory(src)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if (BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
