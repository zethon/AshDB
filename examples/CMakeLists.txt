project(AshDBExamples)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(example1
    example1.cpp
    ${HEADER_FILES}
)

target_link_libraries(example1 
    AshDBLib
    ${CONAN_LIBS}
)

add_executable(example2
    example2.cpp
    ${HEADER_FILES}
)

target_link_libraries(example2
    AshDBLib
    ${CONAN_LIBS}
)