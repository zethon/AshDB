project(AshDBExamples)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCE_FILES
    example1.cpp
)

set(HEADER_FILES
)

add_executable(example1
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(example1 
    AshDBLib
    ${CONAN_LIBS}
)
