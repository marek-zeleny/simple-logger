cmake_minimum_required(VERSION 3.14)

project(tiny_logger)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tiny_logger INTERFACE
        tiny_logger.h
)
