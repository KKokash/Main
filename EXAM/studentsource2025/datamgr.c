//
// Created by karam on 12/14/25.
//

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "datamgr.h"
#include <pthread.h>
#include "lib/dplist.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 20
#endif

#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 10
#endif


static dplist_t *sensor_list = NULL;

static void *element_copy(void *element)
{
    if (element == NULL) return NULL;
    sensor_element_t *src = (sensor_element_t *)element;
    sensor_element_t *copy = malloc(sizeof(sensor_element_t));
    if (copy == NULL) {
        fprintf(stderr, "malloc failed in element_copy\n");
        exit(EXIT_FAILURE);
    }
    memcpy(copy, src, sizeof(sensor_element_t));
    return (void *)copy;
}

static void element_free(void **element)
{
    if (element == NULL || *element == NULL) return;
    free(*element);
    *element = NULL;
}

static int element_compare(void *x, void *y)
{
    if (x == NULL || y == NULL) {
        fprintf(stderr, "NULL passed to element_compare\n");
        exit(EXIT_FAILURE);
    }
    uint16_t a = ((sensor_element_t *)x)->sensor_id;
    uint16_t b = ((sensor_element_t *)y)->sensor_id;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static void ensure_list_created()
{
    if (sensor_list == NULL) {
        sensor_list = dpl_create(element_copy, element_free, element_compare);
        if (sensor_list == NULL) {
            fprintf(stderr, "Could not create sensor_list\n");
            exit(EXIT_FAILURE);
        }
    }
}

static sensor_element_t *find_sensor_by_id(uint16_t sensor_id)
{
    if (sensor_list == NULL) return NULL;
    sensor_element_t key;
    key.sensor_id = sensor_id;
    int idx = dpl_get_index_of_element(sensor_list, &key);
    if (idx < 0) return NULL;
    return (sensor_element_t *) dpl_get_element_at_index(sensor_list, idx);
}

void datamgr_free()
{
    if (sensor_list != NULL) {
        dpl_free(&sensor_list, true); /* free elements and list */
        sensor_list = NULL;
    }
}

static void log_message(datamgr_args_t *args, char *msg) {
    if (args == NULL || args->log_mtx == NULL) return;

    pthread_mutex_lock(args->log_mtx);
    write(args->log_fd, msg, strlen(msg));
    write(args->log_fd, "\n", 1); // Log process expects newline
    pthread_mutex_unlock(args->log_mtx);
}


void* datamgr_thread(void* arg) {
    datamgr_args_t* args = (datamgr_args_t*)arg;

    // 1. Parse the Room Map
    FILE *fp_sensor_map = fopen(args->room_sensor_map_path, "r");
    if (fp_sensor_map == NULL) {
        fprintf(stderr, "Cannot open room_sensor.map file\n");
        exit(EXIT_FAILURE);
    }

    ensure_list_created();
    uint16_t room_id, sensor_id;
    int room_count_index = 0;

    while (fscanf(fp_sensor_map, "%hu %hu", &room_id, &sensor_id) == 2) {
        sensor_element_t *dummy = malloc(sizeof(sensor_element_t));
        if (dummy == NULL) { fprintf(stderr, "Malloc failed\n"); exit(EXIT_FAILURE); }

        dummy->room_id = room_id;
        dummy->sensor_id = sensor_id;
        dummy->count = 0;
        dummy->index = 0;
        dummy->running_avg = 0.0;
        dummy->last_modified = 0;
        for (int i = 0; i < RUN_AVG_LENGTH; i++) { dummy->readings[i] = 0; }

        dpl_insert_at_index(sensor_list, dummy, room_count_index, true);
        free(dummy);
        room_count_index++;
    }
    fclose(fp_sensor_map);

    // 2. Main Loop: Read from sbuffer
    sensor_data_t data;
    while (1) {
        // Blocks until data is available or buffer is closed
        int result = sbuffer_read(args->buffer, &data,READER_DATAMGR);

        if (result == SBUFFER_FAILURE) break; // error :(

        // check for the EOS
        if (data.id == 0) break;

        sensor_element_t *el = find_sensor_by_id(data.id);

        // Log invalid ID
        if (el == NULL) {
            char log_buf[100];
            snprintf(log_buf, sizeof(log_buf), "Received sensor data with invalid sensor node ID %d", data.id);
            log_message(args, log_buf);
            continue;
        }

        // 3. Update sensor data
        el->readings[el->index] = data.value;
        el->index = (el->index + 1) % RUN_AVG_LENGTH;
        if (el->count < RUN_AVG_LENGTH) { el->count++; }

        double sum = 0.0;
        for (int i = 0; i < el->count; i++) { sum += el->readings[i]; }

        if (el->count == RUN_AVG_LENGTH) {
            el->running_avg = sum / RUN_AVG_LENGTH;
        } else {
            el->running_avg = 0.0;
        }
        el->last_modified = data.ts;

        // 4. Check Thresholds and Log
        if (el->count == RUN_AVG_LENGTH) {
            char log_buf[100];
            if (el->running_avg > SET_MAX_TEMP) {
                snprintf(log_buf, sizeof(log_buf),
                         "Sensor node %d reports it's too hot (avg temp = %.2f)",
                         el->sensor_id, el->running_avg);
                log_message(args, log_buf);
            } else if (el->running_avg < SET_MIN_TEMP) {
                snprintf(log_buf, sizeof(log_buf),
                         "Sensor node %d reports it's too cold (avg temp = %.2f)",
                         el->sensor_id, el->running_avg);
                log_message(args, log_buf);
            }
        }
    }

    // Cleanup
    datamgr_free();
    return NULL;
}