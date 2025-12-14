//
// Created by karam on 12/14/25.
//
#include <stdlib.h>
#include "sbuffer.h"
#include <pthread.h>

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    (*buffer)->head_read_by_data = 0;
    (*buffer)->head_read_by_store = 0;
    pthread_mutex_init(&(*buffer)->mtx, NULL);
    pthread_cond_init(&(*buffer)->cond, NULL);
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    if (buffer == NULL || *buffer == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&(*buffer)->mtx);
    sbuffer_node_t *node;
    while ((*buffer)->head) {
        node = (*buffer)->head;
        (*buffer)->head = node->next;
        free(node);
    }
    pthread_mutex_unlock(&(*buffer)->mtx);
    pthread_mutex_destroy(&(*buffer)->mtx);
    pthread_cond_destroy(&(*buffer)->cond);
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;
    sbuffer_node_t *node = malloc(sizeof(sbuffer_node_t));
    if (node == NULL) return SBUFFER_FAILURE;

    node->data = *data;
    node->next = NULL;
    node->readers_left = 2; // Both DataMgr and StoreMgr need it

    pthread_mutex_lock(&buffer->mtx);
    if (buffer->tail == NULL) {
        buffer->head = buffer->tail = node;
    } else {
        buffer->tail->next = node;
        buffer->tail = node;
    }
    pthread_cond_broadcast(&buffer->cond); // Wake up both readers
    pthread_mutex_unlock(&buffer->mtx);
    return SBUFFER_SUCCESS;
}

int sbuffer_read(sbuffer_t *buffer, sensor_data_t *data, int reader_id) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&buffer->mtx);

    while (1) {
        // If buffer is empty, wait
        if (buffer->head == NULL) {
            pthread_cond_wait(&buffer->cond, &buffer->mtx);
            continue;
        }

        // Check if this reader has already read the head
        int *my_flag = (reader_id == READER_DATAMGR) ? &buffer->head_read_by_data : &buffer->head_read_by_store;

        if (*my_flag == 1) {
            // We already read the head, but it hasn't been removed yet (other thread is slow).
            // We must wait for the other thread to finish reading it so it gets removed.
            pthread_cond_wait(&buffer->cond, &buffer->mtx);
            continue;
        }

        // If we are here, there is a head and we haven't read it yet.
        *data = buffer->head->data;
        *my_flag = 1; // Mark as read by me
        buffer->head->readers_left--;

        // If no readers left, remove the node
        if (buffer->head->readers_left == 0) {
            sbuffer_node_t *node = buffer->head;
            buffer->head = buffer->head->next;
            if (buffer->head == NULL) {
                buffer->tail = NULL;
            }
            // Reset flags for the NEW head
            buffer->head_read_by_data = 0;
            buffer->head_read_by_store = 0;

            free(node);
            // Signal that state changed (head moved), so waiting threads can proceed
            pthread_cond_broadcast(&buffer->cond);
        }
        break; // We got our data
    }

    pthread_mutex_unlock(&buffer->mtx);
    return SBUFFER_SUCCESS;
}