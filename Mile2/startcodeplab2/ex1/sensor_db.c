//
// Created by karam on 11/13/25.
//
#include "stdlib.h"
#include "stdio.h"
#include "sensor_db.h"
#include <time.h>

FILE * open_db( char *filename, bool append)
{
    FILE *f = NULL;

    if (append)
        f = fopen(filename, "a");  // append mode
    else
        f = fopen(filename, "w");  // overwrite mode

    if (f == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    return f;
}

int insert_sensor(FILE *f, sensor_data_t sensor_data)
{
    if (f == NULL) {
        fprintf(stderr, "Error: file not open\n");
        return 0;
    }

    // Write one reading: id,value,timestamp
    fprintf(f, "%hu, %f, %ld\n", sensor_data.id, sensor_data.value, sensor_data.ts);

    // Make sure it’s saved to disk right away
    fflush(f);
    return 1;
}

int close_db(FILE * f)
{
    if (f != NULL)
    {
        fclose(f);
        return 1;
    }
    return 0;
}
