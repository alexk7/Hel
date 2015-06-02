cmake_minimum_required(VERSION 3.2)
project(hel)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(include)
add_executable(doh doh/main.cpp)
add_executable(2049 2049/main.cpp)
add_executable(hel_usage_test test/main.cpp)