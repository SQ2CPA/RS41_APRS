#include "geofence.h"

extern String comment;

namespace GEOFENCE
{
    bool checkIfPointInRegion(uint8_t number_of_vertices, const int16_t *vertices_latitude_degrees_list, const int16_t *vertices_longitude_list, int16_t test_point_latitude, int16_t test_point_longitude)
    {
        bool c = false;
        for (uint8_t i = 0, j = number_of_vertices - 1; i < number_of_vertices; j = i++)
        {
            if (((vertices_longitude_list[i] > test_point_longitude) != (vertices_longitude_list[j] > test_point_longitude)) &&
                (test_point_latitude < (vertices_latitude_degrees_list[j] - vertices_latitude_degrees_list[i]) *
                                               (test_point_longitude - vertices_longitude_list[i]) /
                                               (vertices_longitude_list[j] - vertices_longitude_list[i]) +
                                           vertices_latitude_degrees_list[i]))
            {
                c = !c;
            }
        }
        return c;
    }

    int getAFSKFrequency(int16_t gps_latitude, int16_t gps_longitude)
    {
        if (gps_latitude != 0 && gps_longitude != 0)
        {
            gps_latitude = (int16_t)(gps_latitude * 100);
            gps_longitude = (int16_t)(gps_longitude * 100);
        }

        int frequency = APRS_AFSK_FREQUENCY_DEFAULT;
        int F = 4;

        if (gps_latitude == 0 && gps_longitude == 0)
        {
            F = 4;
            frequency = APRS_AFSK_FREQUENCY_DEFAULT;
        }
        else if (checkIfPointInRegion(sizeof(region1_vertices_lat) / sizeof(region1_vertices_lat[0]), region1_vertices_lat, region1_vertices_long, gps_latitude, gps_longitude))
        {
            F = 4;
            frequency = APRS_AFSK_FREQUENCY_REGION1;
        }
        else if (checkIfPointInRegion(sizeof(region2_vertices_lat) / sizeof(region2_vertices_lat[0]), region2_vertices_lat, region2_vertices_long, gps_latitude, gps_longitude))
        {
            F = 5;
            frequency = APRS_AFSK_FREQUENCY_REGION2;
        }
        else if (checkIfPointInRegion(sizeof(australia_vertices_lat) / sizeof(australia_vertices_lat[0]), australia_vertices_lat, australia_vertices_long, gps_latitude, gps_longitude))
        {
            F = 11;
            frequency = APRS_AFSK_FREQUENCY_AUSTRALIA;
        }
        else if (checkIfPointInRegion(sizeof(newzealand_vertices_lat) / sizeof(newzealand_vertices_lat[0]), newzealand_vertices_lat, newzealand_vertices_long, gps_latitude, gps_longitude))
        {
            F = 10;
            frequency = APRS_AFSK_FREQUENCY_NEWZEALAND;
        }
        else if (checkIfPointInRegion(sizeof(thailand_vertices_lat) / sizeof(thailand_vertices_lat[0]), thailand_vertices_lat, thailand_vertices_long, gps_latitude, gps_longitude))
        {
            F = 9;
            frequency = APRS_AFSK_FREQUENCY_THAILAND;
        }
        else if (checkIfPointInRegion(sizeof(japan_vertices_lat) / sizeof(japan_vertices_lat[0]), japan_vertices_lat, japan_vertices_long, gps_latitude, gps_longitude))
        {
            F = 8;
            frequency = APRS_AFSK_FREQUENCY_JAPAN;
        }
        else if (checkIfPointInRegion(sizeof(china_vertices_lat) / sizeof(china_vertices_lat[0]), china_vertices_lat, china_vertices_long, gps_latitude, gps_longitude))
        {
            F = 7;
            frequency = APRS_AFSK_FREQUENCY_CHINA;
        }
        else if (checkIfPointInRegion(sizeof(uk_vertices_lat) / sizeof(uk_vertices_lat[0]), uk_vertices_lat, uk_vertices_long, gps_latitude, gps_longitude))
        {
            F = 4;
            frequency = APRS_AFSK_FREQUENCY_REGION1;

            // F = 12;
            // frequency = APRS_AFSK_FREQUENCY_UK;
        }

        comment.replace("F0", "F" + String(F));
        return frequency;
    }

}