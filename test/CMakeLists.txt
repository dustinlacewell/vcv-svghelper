cmake_minimum_required(VERSION 3.10)

set(THIS ${PROJECT_NAME}-test)

set(SOURCE_FILES
    ../my-library/source/foo.cpp
    foo-test.cpp
)

add_subdirectory(googletest)

include_directories(../my-library/include)

add_executable(${THIS} ${SOURCE_FILES})

# Set a fake version to test
target_compile_definitions(${THIS} PRIVATE
    FOO_MAJOR_VERSION=1
    FOO_MINOR_VERSION=2
)

target_link_libraries(${THIS} gtest_main)

add_test(NAME ${THIS} COMMAND ${THIS})

