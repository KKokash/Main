#include <stdio.h>
#include <stdlib.h>
#include "datamgr.h"   // Include your data manager header

int main(void)
{
    // 1. Open the two files
    FILE *fp_map = fopen("room_sensor.map", "r");   // text file
    FILE *fp_data = fopen("sensor_data", "rb");     // binary file

    // 2. Check if files were opened correctly
    if (fp_map == NULL) {
        perror("Error opening room_sensor.map");
        return EXIT_FAILURE;
    }
    if (fp_data == NULL) {
        perror("Error opening sensor_data");
        fclose(fp_map);  // close the map file before returning
        return EXIT_FAILURE;
    }

    // 3. Call  parser function to load everything
    datamgr_parse_sensor_files(fp_map, fp_data);

    // 4. Now test  getters
    printf("\n=== Sensor info summary ===\n");
    int total = datamgr_get_total_sensors();
    printf("Total sensors: %d\n", total);

    // Loop through sensors and print info
    for (int i = 0; i < total; ++i) {
        // Get the element directly from the list
        // (we don’t have a direct iterator in your API, so this part is pseudo unless you expose it)
        // For demonstration, assume you know some IDs:
        uint16_t test_id = 1; // Replace with actual sensor IDs from your map file
        printf("Sensor %u -> Room %u, Avg: %.2f, Last: %ld\n",
               test_id,
               datamgr_get_room_id(test_id),
               datamgr_get_avg(test_id),
               (long)datamgr_get_last_modified(test_id));
    }

    // 6. Free all allocated memory
    datamgr_free();

    printf("Program finished successfully.\n");
    return EXIT_SUCCESS;
}