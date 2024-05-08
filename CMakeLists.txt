cmake_minimum_required(VERSION 2.8)
project(iosche)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
add_executable(iosche
        main.cpp
        debug.h
        IORequest.h
        scheduler.h
        simulator.h)
