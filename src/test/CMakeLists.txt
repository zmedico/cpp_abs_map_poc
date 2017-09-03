cmake_minimum_required(VERSION 3.0)
project(cpp_abs_map_poc_test CXX)

SET(CMAKE_CXX_FLAGS
  "-std=c++11")

include_directories(
  "."
  "..")

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR /usr/include/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# Make test executable
set(TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests Catch)
