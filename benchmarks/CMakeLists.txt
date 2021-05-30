function(create_benchmark bm_name FILES)

    add_executable(bench_${bm_name}
        bench_${bm_name}.cpp
        ${FILES}
    )

    target_link_libraries(bench_${bm_name}
        PUBLIC
            AshDBLib
            ${CONAN_LIBS_BOOST}
            ${CONAN_LIBS_BENCHMARK}
    )

endfunction()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCE_FILES
    ../src/ashdb.cpp
    ../src/status.cpp
    ../include/ashdb/ashdb.h
)

create_benchmark("basic" "${SOURCE_FILES}")
create_benchmark("batch" "${SOURCE_FILES}")