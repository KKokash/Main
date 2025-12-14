//
// Created by karam on 12/14/25.
//

// connmgr.c pseudo-skeleton
#include "connmgr.h"
#include "config.h"
#include "lib/tcpsock.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>


typedef struct {
    tcpsock_t* client_socket;
    sbuffer_t* buffer;
    int timeout;
    int log_fd;
    pthread_mutex_t* log_mtx;
} client_thread_args_t;

void* client_handler(void* arg);

void* connmgr_thread(void* arg) {
    connmgr_args_t* args = (connmgr_args_t*)arg;

    tcpsock_t* server_socket = NULL;
    // 1. Open the server socket using args->port [cite: 19]
    if (tcp_passive_open(&server_socket, args->port) != TCP_NO_ERROR) {
        perror("Passive open failed");
        return NULL;
    }

    printf("Server started on port %d\n", args->port);

    int clients_served = 0;

    // 2. Accept loop
    while (clients_served < args->max_clients) { // Req: Stop after MAX_CONN [cite: 89]
        tcpsock_t* client_socket = NULL;

        // Wait for connection
        if (tcp_wait_for_connection(server_socket, &client_socket) != TCP_NO_ERROR) {
            perror("Wait for connection failed");
            continue;
        }

        // 3. Prepare args for the worker thread

        client_thread_args_t* client_args = malloc(sizeof(client_thread_args_t));
        client_args->client_socket = client_socket;
        client_args->buffer = args->buffer;
        client_args->timeout = args->timeout_sec;
        client_args->log_fd = args->log_fd;
        client_args->log_mtx = args->log_mtx;

        // 4. Create a thread for this client [cite: 91]
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, client_args) != 0) {
            perror("Failed to create client thread");
            tcp_close(&client_socket);
            free(client_args);
        } else {
            // Detach so we don't have to join it manually
            pthread_detach(tid);
        }

        clients_served++;
    }

    // Clean up server socket
    tcp_close(&server_socket);
    return NULL;
}
void* client_handler(void* arg) {
    client_thread_args_t* args = (client_thread_args_t*)arg;
    tcpsock_t* client = args->client_socket;

    // 1. Set Timeout
    struct timeval tv;
    tv.tv_sec = args->timeout;
    tv.tv_usec = 0;

    int sd;
    tcp_get_sd(client, &sd); // Get raw socket descriptor
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    sensor_data_t data;
    int bytes, result;
    int first_packet = 1;

    do {
        // Read ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void*)&data.id, &bytes);

        // Check for timeout or error
        if (result != TCP_NO_ERROR) break;

        // Read Temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void*)&data.value, &bytes);
        if (result != TCP_NO_ERROR) break;

        // Read Timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void*)&data.ts, &bytes);
        if (result != TCP_NO_ERROR) break;

        // 3. Log ID if this is the first packet
        if (first_packet) {
             char log_msg[100];
             snprintf(log_msg, 100, "Sensor node %d has opened a new connection", data.id);

             pthread_mutex_lock(args->log_mtx);
             write(args->log_fd, log_msg, strlen(log_msg)); // Write to Pipe
             write(args->log_fd, "\n", 1);
             pthread_mutex_unlock(args->log_mtx);

             first_packet = 0;
        }

        // 4. Insert into sbuffer
        sbuffer_insert(args->buffer, &data);

    } while (result == TCP_NO_ERROR);

    // 5. Log Close Connection
    if (!first_packet) {
        char log_msg[100];
        snprintf(log_msg, 100, "Sensor node %d has closed the connection", data.id);
        pthread_mutex_lock(args->log_mtx);
        write(args->log_fd, log_msg, strlen(log_msg));
        write(args->log_fd, "\n", 1);
        pthread_mutex_unlock(args->log_mtx);
    }

    // Cleanup
    tcp_close(&client);
    free(args);
    return NULL;
}