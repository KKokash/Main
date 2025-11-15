//
// Created by karam on 11/15/25.
//
#include "stdlib.h"
#include "stdio.h"
#include "sensor_db.h"
#include <time.h>
#include  "logger.h"

FILE * open_db( char *filename, bool append)
{
    create_log_process();
    FILE *f = NULL;

    if (append)
        f = fopen(filename, "a");  // append mode
    else
        f = fopen(filename, "w");  // overwrite mode

    if (f == NULL) {
        write_to_log_process("Failed to open file.\n");
        exit(EXIT_FAILURE);
    }
    write_to_log_process("Data file opened.\n");
    return f;
}

int insert_sensor(FILE *f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts)
{
    if (f == NULL) {
        write_to_log_process("Error: file not open.\n");
        return 0;
    }

    // Write one reading: id,value,timestamp
    fprintf(f, "%hu, %f, %ld\n", id, value, ts);
    write_to_log_process("Data inserted.");
    // Make sure saved to disk right away
    fflush(f);
    return 1;
}

int close_db(FILE * f)
{
    if (f != NULL)
    {
        fclose(f);
        write_to_log_process("Data file closed.");
        return 1;
    }
    return 0;
}