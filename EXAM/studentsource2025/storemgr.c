//
// Created by karam on 12/14/25.
//

#define _GNU_SOURCE
#include <stdbool.h>
#include "storemgr.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include <pthread.h>

static void log_message(storagemgr_args_t *args, char *msg) {
    if (args == NULL || args->log_mtx == NULL) return;

    pthread_mutex_lock(args->log_mtx);
    write(args->log_fd, msg, strlen(msg));
    write(args->log_fd, "\n", 1); // Log process expects newline
    pthread_mutex_unlock(args->log_mtx);
}

FILE* open_db(char *filename, bool append) {
    FILE *f = fopen(filename, append ? "a" : "w");
    if (f == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
    }
    return f;
}

int insert_sensor(FILE *f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    if (f == NULL) return -1;
    // Write in CSV format
    fprintf(f, "%d, %f, %ld\n", id, value, ts);
    fflush(f);
    return 0;
}

int close_db(FILE *f) {
    if (f == NULL) return -1;
    fclose(f);
    return 0;
}


void *storage_mgr_thread(void *arg) {
    storagemgr_args_t *args = (storagemgr_args_t *)arg;

    // 1. Create/Open the CSV file
    FILE *csv_file = open_db("data.csv", false);
    if (csv_file == NULL) return NULL;

    // Log creation
    log_message(args, "A new data.csv file has been created.");

    sensor_data_t data;
    int result;

    // 2. Loop to read from buffer
    while (1) {

        result = sbuffer_read(args->buffer, &data,READER_STOREMGR);

        if (result != SBUFFER_SUCCESS) break;


        if (data.id == 0) break;

        //write to csv
        insert_sensor(csv_file, data.id, data.value, data.ts);

        // 4 Log Insertion Success
        char log_buf[100];
        snprintf(log_buf, sizeof(log_buf), "Data insertion from sensor %d succeeded.", data.id);
        log_message(args, log_buf);
    }

    close_db(csv_file);

    log_message(args, "The data.csv file has been closed.");

    return NULL;
}