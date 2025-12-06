//
// Created by karam on 12/6/25.
//
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sbuffer.h"
#include  <pthread.h>

sbuffer_t *buffer;
int readCount =-1;
pthread_mutex_t mutex;

void* writerFunc(void* arg)
{
    FILE* fp = arg;
    while (1) {
        uint16_t id;
        double   value;
        time_t   ts;

        size_t r = fread(&id, sizeof(uint16_t), 1, fp);
        if (r != 1) break;
        r = fread(&value, sizeof(double), 1, fp);
        if (r != 1) break;
        r = fread(&ts, sizeof(time_t), 1, fp);
        if (r != 1) break;

        sensor_data_t s = { .id = id, .value = value, .ts = ts };
        sbuffer_insert(buffer, &s);
        usleep(10000);
    }

    sensor_data_t eos = { .id = 0, .value = 0.0, .ts = 0 };
    sbuffer_insert(buffer, &eos);

    return NULL;
}
void *readerFunc(void* arg)
{
    while (1)
    {
        if (sbuffer_size(buffer) == 0){break;}
        pthread_mutex_lock(&mutex);
        readCount++;
        pthread_mutex_unlock(&mutex);
        sbuffer_node_t* sbuffer_node = sbuffer_get_reference_at_index(buffer, readCount);
        sensor_data_t sensor_data = (sbuffer_node)->data;
        fprintf(arg, "%hu, %f, %ld\n", sensor_data.id, sensor_data.value, sensor_data.ts);
        fflush(arg);
        usleep(25000);
    }
}


int main()
{
    FILE *fp_data = fopen("sensor_data", "rb");
    FILE *fp_csv = fopen("sensor_csv", "a");
    if (fp_data == NULL) {
        perror("Error opening sensor_data");
        return EXIT_FAILURE;
    }
    if (fp_csv == NULL)
    {
        perror("Error opening sensor_csv");
        return EXIT_FAILURE;
    }
    if (sbuffer_init(&buffer)!=0)
    {
        perror("Error creating buffer");
        return EXIT_FAILURE;
    }
    pthread_mutex_init(&mutex, NULL);
    pthread_t pidwriter, pidread1,pidread2;
    if (pthread_create(&pidwriter, NULL, writerFunc, fp_data))
    {
        perror("Failed to create thread1");
        return EXIT_FAILURE;
    }
    if (pthread_create(&pidread1, NULL, readerFunc, fp_csv))
    {
        perror("Failed to create thread2");
        return EXIT_FAILURE;
    }
    if (pthread_create(&pidread2, NULL, readerFunc, fp_csv))
    {
        perror("Failed to create thread3");
        return EXIT_FAILURE;
    }

    sbuffer_free(&buffer);
    pthread_mutex_destroy(&mutex);

}