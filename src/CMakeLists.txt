set(SOURCE_FILES
    ashdb.cpp
)

set(HEADER_FILES
    ../include/ashdb/ashdb.h
    ../include/ashdb/primitives.h
)

add_library(AshDBLib STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(AshDBLib
    ${CONAN_LIBS}
)