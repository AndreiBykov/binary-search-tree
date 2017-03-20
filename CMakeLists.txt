cmake_minimum_required(VERSION 3.6)
project(BinaryTree)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(SOURCE_FILES main.cpp)
add_executable(BinaryTree ${SOURCE_FILES})