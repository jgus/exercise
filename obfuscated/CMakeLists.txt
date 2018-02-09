cmake_minimum_required(VERSION 3.9)
project(obfuscated_string)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(../external/googletest external/googletest)
enable_testing()
include(GoogleTest)

add_library(obfuscated INTERFACE)
target_include_directories(obfuscated INTERFACE include)

add_executable(obfuscated-test test/test.cpp)
target_link_libraries(obfuscated-test gtest gtest_main)
target_link_libraries(obfuscated-test obfuscated)
gtest_add_tests(TARGET obfuscated-test)
