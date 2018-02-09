cmake_minimum_required(VERSION 3.9)
project(boggle)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(../external/googletest external/googletest)
enable_testing()
include(GoogleTest)

add_library(boggle INTERFACE)
target_include_directories(boggle INTERFACE include)

add_executable(boggle-test test/test.cpp)
target_link_libraries(boggle-test gtest gtest_main)
target_link_libraries(boggle-test boggle)
gtest_add_tests(TARGET boggle-test)
