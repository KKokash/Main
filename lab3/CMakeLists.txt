cmake_minimum_required(VERSION 4.0)
project(lab3 C)

set(CMAKE_C_STANDARD 11)

include_directories(startcodeclab3/ex1)
include_directories(startcodeclab3/ex2)
include_directories(startcodeclab3/milestone1)

add_executable(lab3
        startcodeclab3/ex1/dplist.c
        startcodeclab3/ex1/dplist.h
        startcodeclab3/ex1/program.c
        startcodeclab3/ex2/dplist.h
        startcodeclab3/milestone1/dplist.c
        startcodeclab3/milestone1/dplist.h
        startcodeclab3/milestone1/dplist_test.c
        startcodeclab3/milestone1/ourtest.c
        startcodeclab3/milestone1/randomtest.c)
