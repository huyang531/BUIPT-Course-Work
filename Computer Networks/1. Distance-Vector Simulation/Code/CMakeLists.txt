cmake_minimum_required(VERSION 3.21)
project(dv_algorithm_simulation)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(dv_algorithm_simulation
        dv_algorithm.cpp
        dv_algorithm.h
        dv_msg.cpp
        dv_msg.h
        main.cpp
        networkrouting.cpp
        networkrouting.h
        transport.cpp
        transport.h
        user_cmd.cpp
        user_cmd.h)
