cmake_minimum_required(VERSION 4.0)
project(lab5 C)

set(CMAKE_C_STANDARD 11)

include_directories(startcodeplab3)
include_directories(startcodeplab3/lib)

add_executable(lab5
        startcodeplab3/lib/tcpsock.c
        startcodeplab3/lib/tcpsock.h
        startcodeplab3/config.h
        startcodeplab3/sensor_node.c
        startcodeplab3/test_server.c)
