//
// Created by karam on 12/14/25.
//

#ifndef STUDENTSOURCE2025_DATAMGR_H
#define STUDENTSOURCE2025_DATAMGR_H
#include <bits/pthreadtypes.h>

#include "sbuffer.h"

typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    time_t last_modified;
    double readings[RUN_AVG_LENGTH];
    int index;
    int count;
} sensor_element_t;

typedef struct {
    sbuffer_t* buffer;
    int log_fd;
    pthread_mutex_t* log_mtx;
    const char* room_sensor_map_path; // e.g., "room_sensor.map"
} datamgr_args_t;
#define ERROR_HANDLER(condition, ...)    do {                       \
if (condition) {                              \
printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
exit(EXIT_FAILURE);                         \
}                                             \
} while(0)

void* datamgr_thread(void* arg);
#endif //STUDENTSOURCE2025_DATAMGR_H