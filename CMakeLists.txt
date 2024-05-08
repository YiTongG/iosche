cmake_minimum_required(VERSION 3.26)
project(iosche)

set(CMAKE_CXX_STANDARD 14)

add_executable(iosche
        main.cpp
        debug.h
        IORequest.h
        scheduler.h
        simulator.h)
