cmake_minimum_required(VERSION 4.0)
project(Mile3 C)

set(CMAKE_C_STANDARD 11)

include_directories(startcodeplab4)

add_executable(Mile3
        startcodeplab4/config.h
        startcodeplab4/sbuffer.c
        startcodeplab4/sbuffer.h
        startcodeplab4/main.c
        startcodeplab4/main.c
        startcodeplab4/main.c)
