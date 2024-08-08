cmake_minimum_required(VERSION 3.26)
project(vmm)

set(CMAKE_CXX_STANDARD 17)

add_executable(vmm main.cpp
        pager.h
        global.h
        log.h)
