cmake_minimum_required(VERSION 3.9)
project(dispatcher)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(../external/googletest external/googletest)
enable_testing()
include(GoogleTest)

add_library(dispatcher src/dispatcher.cpp include/dispatcher/dispatcher.hpp)
target_include_directories(dispatcher PUBLIC include)

add_executable(dispatcher-test test/test.cpp)
target_link_libraries(dispatcher-test gtest gtest_main)
target_link_libraries(dispatcher-test dispatcher)
gtest_add_tests(TARGET dispatcher-test)
