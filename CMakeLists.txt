cmake_minimum_required(VERSION 3.20)
project(AshDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CMake/conan.cmake)
conan_cmake_run(REQUIRES
    boost/1.74.0
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
    OPTIONS
        boost:without_contract=False
        boost:without_test=False
        boost:without_filesystem=False
)
conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)

option(BUILD_UNIT_TESTS "Build unit tests (default OFF)" OFF)
option(BUILD_EXAMPLES "Build examples (default OFF)" OFF)

add_subdirectory(src)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
