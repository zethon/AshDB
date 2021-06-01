include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCE_FILES
    ../src/ashdb.cpp
    ../src/status.cpp
    ../include/ashdb/ashdb.h
)

add_executable(bench_basic
    bench_basic.cpp
    ${FILES}
)

target_link_libraries(bench_basic
    PUBLIC
        AshDBLib
        ${CONAN_LIBS_BOOST}
        ${CONAN_LIBS_BENCHMARK}
)
