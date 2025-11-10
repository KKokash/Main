cmake_minimum_required(VERSION 4.0)
project(lab4 C)

set(CMAKE_C_STANDARD 11)

include_directories(startcodeplab1)
include_directories(startcodeplab1/lib)

add_executable(lab4
        startcodeplab1/lib/dplist.c
        startcodeplab1/lib/dplist.h
        startcodeplab1/config.h
        startcodeplab1/datamgr.c
        startcodeplab1/datamgr.h
        startcodeplab1/file_creator.c
        startcodeplab1/main.c)
