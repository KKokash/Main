#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "datamgr.h"
#include "lib/dplist.h"

typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    time_t last_modified;
    double readings[RUN_AVG_LENGTH];
    int index;
    int count;
} sensor_element_t;

/* forward declarations */
static void *element_copy(void *element);
static void element_free(void **element);
static int element_compare(void *x, void *y);

/* list holding all sensors */
static dplist_t *sensor_list = NULL;
static void *element_copy(void *element)
{
    if (element == NULL) return NULL;

    sensor_element_t *src = (sensor_element_t *)element;
    sensor_element_t *copy = malloc(sizeof(sensor_element_t));
    ERROR_HANDLER(copy == NULL, "malloc failed in element_copy");

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
        ERROR_HANDLER(true, "NULL passed to element_compare");
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
        ERROR_HANDLER(sensor_list == NULL, "Could not create sensor_list");
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

void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data)
{
    ERROR_HANDLER(fp_sensor_map == NULL, "Cannot open room_sensor.map file");
    ERROR_HANDLER(fp_sensor_data == NULL, "Cannot open sensor_data file");
    ensure_list_created();
    uint16_t room_id, sensor_id;
    int room_count_index = 0;
    rewind(fp_sensor_map);
    // This reads two numbers from one line:
    while (fscanf(fp_sensor_map, "%hu %hu", &room_id, &sensor_id) == 2) {
        sensor_element_t *dummy = malloc(sizeof(sensor_element_t));
        ERROR_HANDLER(dummy == NULL, "malloc failed in datamgr_parse_sensor_files");

        dummy->room_id = room_id;
        dummy->sensor_id = sensor_id;
        dummy->count =0;
        dummy->index = 0;
        dummy->running_avg = 0.0;
        dummy->last_modified = 0;
        for (int i = 0; i<RUN_AVG_LENGTH; i++) {dummy->readings[i] = 0;}
        dpl_insert_at_index(sensor_list, dummy, room_count_index, true);
        free(dummy);
        room_count_index++;
    }
    rewind(fp_sensor_data);
    while (1)
    {
        uint16_t sid;
        double temperature;
        time_t timestamp;
        size_t r = fread(&sid, sizeof(uint16_t), 1, fp_sensor_data);
        if (r != 1) break; // to see if its succesful
        r = fread(&temperature, sizeof(double), 1, fp_sensor_data);
        if (r != 1) break;
        r = fread(&timestamp, sizeof(time_t), 1, fp_sensor_data);
        if (r != 1) break;

        sensor_element_t *el = find_sensor_by_id(sid);
        if (el == NULL) {
            fprintf(stderr, "Unknown sensor id %hu at time %ld (ignored)\n", sid, (long)timestamp);
            continue;
        }

        el->readings[el->index] = temperature;
        el->index = (el->index + 1) % RUN_AVG_LENGTH;

        if (el->count <RUN_AVG_LENGTH) {el->count =  el->count + 1; }
        double sum = 0.0;
        for (int i = 0; i<el->count; i++) {sum += el->readings[i];}
        if (el->count == RUN_AVG_LENGTH)
        {
            el->running_avg = sum / RUN_AVG_LENGTH;
        }
        else
        {
            el->running_avg = 0.0;
        }
        el->last_modified = timestamp;
        if (el->count == RUN_AVG_LENGTH)
        {
            if (el->running_avg >SET_MAX_TEMP)
            {
                fprintf(stderr,"Temperature of room %hu is %.2f exceeds the maximum temperature of %.2f\n",el->room_id,temperature,(double)SET_MAX_TEMP);
            }
            else if (el->running_avg < SET_MIN_TEMP)
            {
                fprintf(stderr,"Temperature of room %hu is %.2f exceeds the min temperature of %.2f\n",el->room_id,temperature,(double)SET_MIN_TEMP);
            }
        }
    }
    fclose(fp_sensor_map);
    fclose(fp_sensor_data);
}
    void datamgr_free()
    {
        if (sensor_list != NULL) {
            dpl_free(&sensor_list, true); /* free elements and list */
            sensor_list = NULL;
        }
    }

    /* Example implementations for getters (use ERROR_HANDLER if sensor not found) */

    uint16_t datamgr_get_room_id(sensor_id_t sensor_id)
    {
        sensor_element_t *el = find_sensor_by_id(sensor_id);
        ERROR_HANDLER(el == NULL, "datamgr_get_room_id: invalid sensor id ");
        return el->room_id;
    }

    sensor_value_t datamgr_get_avg(sensor_id_t sensor_id)
    {
        sensor_element_t *el = find_sensor_by_id(sensor_id);
        ERROR_HANDLER(el == NULL, "datamgr_get_avg: invalid sensor id");
        return (sensor_value_t) el->running_avg;
    }

    time_t datamgr_get_last_modified(sensor_id_t sensor_id)
    {
        sensor_element_t *el = find_sensor_by_id(sensor_id);
        ERROR_HANDLER(el == NULL, "datamgr_get_last_modified: invalid sensor id");
        return el->last_modified;
    }

    int datamgr_get_total_sensors()
    {
        if (sensor_list == NULL) return 0;
        return dpl_size(sensor_list);
    }


