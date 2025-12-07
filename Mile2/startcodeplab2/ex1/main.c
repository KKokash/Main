#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include <sys/types.h>



#include "sensor_db.h"


int main()
{
    FILE *f = open_db("sensor_db.csv", true);
    sensor_data_t v ;
    v.id = 1;
    v.value = 0.001;
    v.ts = time(NULL);

    insert_sensor(f, v);

   v.id = 2;
   v.value = 0.002;
    v.ts = time(NULL);

    insert_sensor(f, v);
    close_db(f);
    return 0;
}

