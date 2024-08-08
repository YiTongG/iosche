cmake_minimum_required(VERSION 3.26)
project(scheduler)

set(CMAKE_CXX_STANDARD 17)

add_executable(scheduler main.cpp
        global.h
        log.h)
