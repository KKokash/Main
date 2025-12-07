cmake_minimum_required(VERSION 4.0)
project(Mile2 C)

set(CMAKE_C_STANDARD 11)

include_directories(startcodeplab2/ex1)
include_directories(startcodeplab2/milestone2)

add_executable(Mile2
        startcodeplab2/ex1/config.h
        startcodeplab2/ex1/main.c
        startcodeplab2/ex1/sensor_db.h
        startcodeplab2/milestone2/config.h
        startcodeplab2/milestone2/logger.h
        startcodeplab2/milestone2/main.c
        startcodeplab2/milestone2/sensor_db.h
        startcodeplab2/ex1/sensor_db.c
        startcodeplab2/ex2/main.c
        startcodeplab2/milestone2/sensor_db.c
        startcodeplab2/milestone2/logger.c
        startcodeplab2/milestone2/sensor_db.c)
