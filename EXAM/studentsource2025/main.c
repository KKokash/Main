//
// Created by karam on 12/14/25.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "storemgr.h"
#include "config.h"
#include "sbuffer.h"
#include "connmgr.h"
#include "datamgr.h"


void run_log_process(int pipe_read_fd) {
    FILE *log_file = fopen("gateway.log", "w");
    if (!log_file) {
        perror("Failed to open gateway.log");
        exit(EXIT_FAILURE);
    }

    FILE *pipe_fp = fdopen(pipe_read_fd, "r");
    if (!pipe_fp) {
        perror("Failed to convert pipe fd");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    int seq_num = 0;

    // Read line by line from pipe
    while (getline(&line, &len, pipe_fp) != -1) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[24] = '\0';

        fprintf(log_file, "%d %s %s", seq_num++, time_str, line);
        fflush(log_file);
    }

    free(line);
    fclose(pipe_fp);
    fclose(log_file);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <port> <max_clients>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[1]);
    int max_clients = atoi(argv[2]);

    // Create Pipe
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("Pipe failed");
        return -1;
    }

    // Fork Log Process
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    if (pid == 0) {
        // Child: Log Process
        close(pipe_fds[1]);
        run_log_process(pipe_fds[0]);
        return 0;
    }

    // Parent: Main Process
    close(pipe_fds[0]);
    int log_write_fd = pipe_fds[1];

    // Init Resources
    sbuffer_t *buffer;
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        perror("SBuffer init failed");
        return -1;
    }

    pthread_mutex_t log_mtx;
    pthread_mutex_init(&log_mtx, NULL);

    //  Create Threads
    pthread_t conn_tid, data_tid, store_tid;

    // Setup Arguments
    connmgr_args_t conn_args = {buffer, port, max_clients, 5 /*timeout*/, log_write_fd, &log_mtx}; // 5s timeout default
    datamgr_args_t data_args = {buffer, log_write_fd, &log_mtx, "room_sensor.map"};
    storagemgr_args_t store_args = {buffer, log_write_fd, &log_mtx};

    if (pthread_create(&conn_tid, NULL, connmgr_thread, &conn_args) != 0) perror("Connmgr create failed");
    if (pthread_create(&data_tid, NULL, datamgr_thread, &data_args) != 0) perror("Datamgr create failed");
    if (pthread_create(&store_tid, NULL, storagemgr_thread, &store_args) != 0) perror("Storemgr create failed");

    // Wait for Connmgr to finish
    pthread_join(conn_tid, NULL);

    //  Signal End of Stream
    sensor_data_t stop_data = {0, 0, 0};
    sbuffer_insert(buffer, &stop_data);


    pthread_join(data_tid, NULL);
    pthread_join(store_tid, NULL);

    // Cleanup
    sbuffer_free(&buffer);
    pthread_mutex_destroy(&log_mtx);
    close(log_write_fd);

    // Wait for log process to exit
    waitpid(pid, NULL, 0);

    return 0;
}