/**
 * \author {AUTHOR}
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include <pthread.h>

/**
 * Implements a sequential test server (only one connection at the same time)
 */
int MAX_CONN;
int PORT;
pthread_mutex_t mutex;       // protects served_count
int served_count = 0;


void* connection(void* arg)
{
    tcpsock_t* client = (tcpsock_t*)arg;      // <-- get the client socket
    sensor_data_t data;
    int bytes, result;

    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void*)&data.id, &bytes);
        if (result != TCP_NO_ERROR) break;

        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void*)&data.value, &bytes);
        if (result != TCP_NO_ERROR) break;

        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void*)&data.ts, &bytes);

        if ((result == TCP_NO_ERROR) && bytes) {
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",
                   data.id, data.value, (long int)data.ts);
        }
    } while (result == TCP_NO_ERROR);

    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else if (result != TCP_NO_ERROR)
        printf("Error occurred on connection to peer\n");

    // Close client
    tcp_close(&client);

    // Update served_count in a thread-safe way
    pthread_mutex_lock(&mutex);
    served_count++;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Please provide the right arguments: first the port, then the max nb of clients");
        return -1;
    }

    MAX_CONN = atoi(argv[2]); // e.g., 4
    PORT     = atoi(argv[1]); // e.g., 5678

    pthread_mutex_init(&mutex, NULL);

    // Create ONE listening socket
    tcpsock_t* server = NULL;
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);  // socket+bind+listen

    printf("Test server is started\n");

    // Accept loop: stop after serving MAX_CONN clients
    while (1) {
        pthread_mutex_lock(&mutex);
        if (served_count >= MAX_CONN) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);


        tcpsock_t* client = NULL;
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE); // accept
        printf("Incoming client connection\n");


        pthread_t tid;
        if (pthread_create(&tid, NULL, connection, (void*)client) != 0) {
            perror("Failed to create thread");
            tcp_close(&client); // avoid leak if thread creation fails
            // keep accepting others
        } else {

            pthread_detach(tid);
        }
    }

    // Close the listener and clean up
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    pthread_mutex_destroy(&mutex);

    printf("Test server is shutting down\n");
    return 0;
}