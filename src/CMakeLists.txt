set(SOURCE_FILES
    ashdb.cpp
    status.cpp
)

set(HEADER_FILES
    ../include/ashdb/ashdb.h
    ../include/ashdb/options.h
    ../include/ashdb/primitives.h
)

add_library(AshDBLib STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(AshDBLib
    ${CONAN_LIBS}
    coverage_config
)