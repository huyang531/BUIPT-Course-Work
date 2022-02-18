cmake_minimum_required(VERSION 3.19)
project(2d)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(2d
        n_body.cpp
        vector2d.h
        vector2d.cpp)
