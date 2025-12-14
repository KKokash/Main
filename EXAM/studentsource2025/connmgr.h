//
// Created by karam on 12/14/25.
//

#ifndef STUDENTSOURCE2025_CONNMGR_H
#define STUDENTSOURCE2025_CONNMGR_H
#include <bits/pthreadtypes.h>

#include "sbuffer.h"

typedef struct {
    sbuffer_t* buffer;
    int port;
    int max_clients;
    int timeout_sec;
    int log_fd;                 // write-end of the pipe
    pthread_mutex_t* log_mtx;   // mutex to protect write() to pipe
} connmgr_args_t;

void* connmgr_thread(void* arg);

#endif //STUDENTSOURCE2025_CONNMGR_H