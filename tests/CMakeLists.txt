function(create_test testname FILES)

    add_executable(test_${testname}
        main.cpp
        test_${testname}.cpp
        ${FILES}
    )

    target_link_libraries(test_${testname}
        PUBLIC
            AshDBLib
            ${CONAN_LIBS_BOOST}
            coverage_config
    )

    add_test(NAME test_${testname}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_${testname}
    )

endfunction(create_test)

set(SOURCE_FILES
    ../src/ashdb.cpp
    ../src/status.cpp
    ../include/ashdb/ashdb.h
)

create_test("basic" "${SOURCE_FILES}")
create_test("class1" "${SOURCE_FILES}")
create_test("batch" "${SOURCE_FILES}")

