cmake_minimum_required(VERSION 3.14)

project(simple_logger)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(simple_logger INTERFACE)
target_include_directories(simple_logger INTERFACE include)
