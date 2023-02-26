cmake_minimum_required(VERSION 3.10)

option(BUILD_MYLIBRARY "Build my library" ON)
option(BUILD_TESTS "Build the tests." ON)

if(BUILD_MYLIBRARY)
    add_subdirectory(my-library)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
