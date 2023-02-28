cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
project(vcv-svghelper)

set(PLUGIN_NAME ${PROJECT_NAME})
set(ADDITIONAL_PLUGIN_DISTRIBUTABLES README.md)
include(RackSDK.cmake)

target_include_directories(${RACK_PLUGIN_LIB} PRIVATE include)
target_sources(${RACK_PLUGIN_LIB} PRIVATE src/SvgHelper.cpp)
