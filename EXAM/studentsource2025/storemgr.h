//
// Created by karam on 12/14/25.
//

#ifndef STUDENTSOURCE2025_STOREMGR_H
#define STUDENTSOURCE2025_STOREMGR_H
#include <bits/pthreadtypes.h>

#include "sbuffer.h"

typedef struct {
    sbuffer_t* buffer;
    int log_fd;
    pthread_mutex_t* log_mtx;
    const char* csv_path; // e.g., "data.csv"
} storagemgr_args_t;

void* storagemgr_thread(void* arg);

#endif //STUDENTSOURCE2025_STOREMGR_H