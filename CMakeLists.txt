# compile src/*.cpp including include/*.hpp

cmake_minimum_required(VERSION 3.10)

project(vcv-svghelper)

set(CMAKE_CXX_STANDARD 17)

set(RACK_SDK_DIR "B:/code/Rack/Rack-SDK" CACHE PATH "Path to Rack SDK")

## include Rack SDK headers
include_directories(include)
include_directories(${RACK_SDK_DIR}/include)
include_directories(${RACK_SDK_DIR}/dep/include)

## compile src
file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_library(vcv-svghelper STATIC ${SRC_FILES})


## link against Rack SDK static library

if (WIN32)
	target_link_libraries(vcv-svghelper INTERFACE ${RACK_SDK_DIR}/libRack.dll.a)
else()
	target_link_libraries(vcv-svghelper INTERFACE ${RACK_SDK_DIR}/librack.a)
endif()

add_executable(dummy main.cpp)
target_link_libraries(dummy vcv-svghelper)
