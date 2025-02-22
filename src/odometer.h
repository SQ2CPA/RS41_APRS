#ifndef ODOMETER_H_
#define ODOMETER_H_

#include "Arduino.h"
#include <TinyGPS++.h>
#include <EEPROM.h>

#define MAX_JUMP_DISTANCE_METERS 10000000
#define SAVE_TO_EEPROM_INTERVAL_MINUTES 10

#define EEPROM_FIRST_RUN_FLAG_ADDRESS 4
#define EEPROM_TOTAL_DISTANCE_ADDRESS 8
#define EEPROM_LAST_DISTANCE_LOGGED_ADDRESS 12
#define EEPROM_GPS_DATA_ADDRESS 16

#define EEPROM_FIRST_RUN_FLAG_VALUE 0xAABBCCDD

struct GpsData
{
    double lastLat;
    double lastLng;
};

namespace ODOMETER
{
    void setup();

    void update();
}

#endif