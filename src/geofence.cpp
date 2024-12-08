#include "geofence.h"

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

    float getAFSKFrequency(int16_t gps_latitude, int16_t gps_longitude)
    {
        if (gps_latitude == 0 && gps_longitude == 0)
            return APRS_AFSK_FREQUENCY_DEFAULT;

        if (checkIfPointInRegion(sizeof(region1_vertices_lat) / sizeof(region1_vertices_lat[0]), region1_vertices_lat, region1_vertices_long, gps_latitude, gps_longitude))
            return APRS_AFSK_FREQUENCY_REGION1;

        if (checkIfPointInRegion(sizeof(region2_vertices_lat) / sizeof(region2_vertices_lat[0]), region2_vertices_lat, region2_vertices_long, gps_latitude, gps_longitude))
            return APRS_AFSK_FREQUENCY_REGION2;

        if (checkIfPointInRegion(sizeof(australia_vertices_lat) / sizeof(australia_vertices_lat[0]), australia_vertices_lat, australia_vertices_long, gps_latitude, gps_longitude))
            return APRS_AFSK_FREQUENCY_AUSTRALIA;

        if (checkIfPointInRegion(sizeof(newzealand_vertices_lat) / sizeof(newzealand_vertices_lat[0]), newzealand_vertices_lat, newzealand_vertices_long, gps_latitude, gps_longitude))
            return APRS_AFSK_FREQUENCY_NEWZEALAND;

        if (checkIfPointInRegion(sizeof(thailand_vertices_lat) / sizeof(thailand_vertices_lat[0]), thailand_vertices_lat, thailand_vertices_long, gps_latitude, gps_longitude))
            return APRS_AFSK_FREQUENCY_THAILAND;

        if (checkIfPointInRegion(sizeof(japan_vertices_lat) / sizeof(japan_vertices_lat[0]), japan_vertices_lat, japan_vertices_long, gps_latitude, gps_longitude))
            return APRS_AFSK_FREQUENCY_JAPAN;

        if (checkIfPointInRegion(sizeof(china_vertices_lat) / sizeof(china_vertices_lat[0]), china_vertices_lat, china_vertices_long, gps_latitude, gps_longitude))
            return APRS_AFSK_FREQUENCY_CHINA;

        if (checkIfPointInRegion(sizeof(uk_vertices_lat) / sizeof(uk_vertices_lat[0]), uk_vertices_lat, uk_vertices_long, gps_latitude, gps_longitude))
            return APRS_AFSK_FREQUENCY_UK;

        return APRS_AFSK_FREQUENCY_DEFAULT;
    }

}