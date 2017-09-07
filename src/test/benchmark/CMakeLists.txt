cmake_minimum_required(VERSION 3.0)
project(cpp_abs_map_poc_benchmark CXX)

SET(CMAKE_CXX_FLAGS
  "-std=c++11 -pg")

include_directories(
  ".."
  "../..")


add_executable(benchmark_generic_iter benchmark_generic_iter.cpp)
add_executable(benchmark_normal_iter benchmark_normal_iter.cpp)
