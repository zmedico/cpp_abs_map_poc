cmake_minimum_required(VERSION 3.8)
project(cpp_abs_map_poc_test CXX)

SET(CMAKE_CXX_FLAGS
  "-std=c++11")
set(SRCS
  "main.cpp")

add_executable(tests ${SRCS})

include_directories( "../")
