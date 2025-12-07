/**
 * \author {AUTHOR}
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"

#include <pthread.h>

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */


int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&(*buffer)->mtx, NULL);
    pthread_cond_init(&(*buffer)->cond, NULL);
    return SBUFFER_SUCCESS;
}

int sbuffer_size(sbuffer_t *list) {
    if (list == NULL) return -1;
    pthread_mutex_lock(&list->mtx);
    int size = 0;
    sbuffer_node_t *node = list->head;
    while (node) { size++; node = node->next; }
    pthread_mutex_unlock(&list->mtx);
    return size;
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
    (*buffer)->tail = NULL;
    pthread_mutex_unlock(&(*buffer)->mtx);
    pthread_mutex_destroy(&(*buffer)->mtx);
    pthread_cond_destroy(&(*buffer)->cond);
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&buffer->mtx);
    while (buffer->head == NULL) {
        pthread_cond_wait(&buffer->cond, &buffer->mtx);
    }
    sbuffer_node_t *node = buffer->head;
    *data = node->data;
    if (buffer->head == buffer->tail) {
        buffer->head = buffer->tail = NULL;
    } else {
        buffer->head = buffer->head->next;
    }
    free(node);
    pthread_mutex_unlock(&buffer->mtx);
    return SBUFFER_SUCCESS;
}

sbuffer_node_t *sbuffer_get_reference_at_index(sbuffer_t *list, int index) {
    if (list == NULL || list->head == NULL) return NULL;
    if (index <= 0) return list->head;
    if (index > sbuffer_size(list) - 1) index = sbuffer_size(list) - 1;
    sbuffer_node_t *node = list->head;
    for (int i = 0; i < index; i++) node = node->next;
    return node;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;
    sbuffer_node_t *node = malloc(sizeof(sbuffer_node_t));
    if (node == NULL) return SBUFFER_FAILURE;
    node->data = *data;
    node->next = NULL;
    pthread_mutex_lock(&buffer->mtx);
    if (buffer->tail == NULL) {
        buffer->head = buffer->tail = node;
    } else {
        buffer->tail->next = node;
        buffer->tail = node;
    }
    pthread_cond_signal(&buffer->cond);
    pthread_mutex_unlock(&buffer->mtx);
    return SBUFFER_SUCCESS;
}