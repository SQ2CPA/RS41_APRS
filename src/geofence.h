#ifndef GEOFENCE_H_
#define GEOFENCE_H_

#include "Arduino.h"

const int16_t region1_vertices_lat[] = {3855, 2158, 711, 2476, 3460, 4255, 4955, 4255, 5455, 4855, 7855, 7855};
const int16_t region1_vertices_long[] = {-3355, -2717, 8823, 8812, 7106, 7055, 8655, 10355, 12255, 14655, 17855, -2755};

const int16_t region2_vertices_lat[] = {732, -1086, -2066, -1565, 3485, 6004, 7083, 8171, 8408, 8375, 6125, 3456};
const int16_t region2_vertices_long[] = {-7910, -6539, -7102, -11285, -14942, -16664, -16875, -15364, -8754, -5977, -6047, -4535};

const int16_t australia_vertices_lat[] = {-764, -1513, -3352, -4086, -4256, -4073, -3590, -2414, -1249, -283};
const int16_t australia_vertices_long[] = {12441, 10990, 11008, 11439, 13864, 14919, 15385, 16035, 15754, 14321};

const int16_t newzealand_vertices_lat[] = {-5163, -4893, -4460, -3979, -3330, -3016, -4729};
const int16_t newzealand_vertices_long[] = {16698, 17366, 17859, 17999, 17771, 16435, 15591};

const int16_t thailand_vertices_lat[] = {772, 667, 781, 1715, 2138, 2429, 2847, 2738, 2413};
const int16_t thailand_vertices_long[] = {9743, 10507, 11360, 11175, 10859, 10200, 9760, 9233, 8890};

const int16_t japan_vertices_lat[] = {4174, 3885, 3663, 3557, 3236, 3041, 3229, 3844, 4555, 5330, 5449};
const int16_t japan_vertices_long[] = {13782, 13677, 13395, 13026, 12859, 13457, 14740, 15452, 15162, 14749, 14213};

const int16_t china_vertices_lat[] = {1730, 1747, 2307, 3488, 4508, 5128, 5095, 5073, 5073, 4420, 2468};
const int16_t china_vertices_long[] = {11093, 9792, 8157, 7313, 7665, 8966, 10144, 11690, 12446, 13185, 12306};

const int16_t uk_vertices_lat[] = {5888, 5444, 4999, 5055, 5222, 5666, 5777, 5888};
const int16_t uk_vertices_long[] = {-777, -888, -555, 144, 222, -222, -111, -333};

// APRS AFSK frequencies

#define APRS_AFSK_FREQUENCY_REGION1 144800000
#define APRS_AFSK_FREQUENCY_REGION2 144390000
#define APRS_AFSK_FREQUENCY_BRAZIL 145570000
#define APRS_AFSK_FREQUENCY_CHINA 144640000
#define APRS_AFSK_FREQUENCY_JAPAN 144660000
#define APRS_AFSK_FREQUENCY_THAILAND 145525000
#define APRS_AFSK_FREQUENCY_NEWZEALAND 144575000
#define APRS_AFSK_FREQUENCY_AUSTRALIA 145.175000
#define APRS_AFSK_FREQUENCY_UK 434500000

#define APRS_AFSK_FREQUENCY_DEFAULT APRS_AFSK_FREQUENCY_REGION1

// APRS LoRa frequencies

#define APRS_LORA_FREQUENCY_FAST 434855000
#define APRS_LORA_FREQUENCY_SLOW 433775000
#define APRS_LORA_FREQUENCY_UK 439912500

namespace GEOFENCE
{
    bool checkIfPointInRegion(uint8_t number_of_vertices, const int16_t *vertices_latitude_degrees_list, const int16_t *vertices_longitude_list, int16_t test_point_latitude, int16_t test_point_longitude);
    int getAFSKFrequency(int16_t gps_latitude, int16_t gps_longitude);
}

#endif