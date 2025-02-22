#include "odometer.h"
#include "backlog.h"
#include "radio.h"

extern HardwareSerial Serial1;

extern bool hasFix;

extern double latitude;
extern double longitude;

extern TinyGPSPlus gps;

extern int satellites;

extern int beaconNum;

double lastLatitude = 0.0, lastLongitude = 0.0;

uint32_t distanceMetersAccumulator = 0;
uint32_t lastDistanceLoggedKm = 0;

extern uint32_t totalDistanceKm;

int lastSaveMinute = -1;

namespace ODOMETER
{
    void setup()
    {
        uint32_t firstRunFlag;
        EEPROM.get(EEPROM_FIRST_RUN_FLAG_ADDRESS, firstRunFlag);

        EEPROM.put(0, 0xFFFFFFFF);

        if (firstRunFlag != EEPROM_FIRST_RUN_FLAG_VALUE)
        {
            GpsData gpsData = {53.340649, 17.647825};

            totalDistanceKm = 0;
            lastDistanceLoggedKm = 0;

            EEPROM.put(EEPROM_GPS_DATA_ADDRESS, gpsData);

            EEPROM.put(EEPROM_TOTAL_DISTANCE_ADDRESS, totalDistanceKm);
            EEPROM.put(EEPROM_LAST_DISTANCE_LOGGED_ADDRESS, lastDistanceLoggedKm);
            EEPROM.put(EEPROM_FIRST_RUN_FLAG_ADDRESS, EEPROM_FIRST_RUN_FLAG_VALUE);

            lastLatitude = gpsData.lastLat;
            lastLongitude = gpsData.lastLng;
        }
        else
        {
            GpsData gpsData;

            EEPROM.get(EEPROM_GPS_DATA_ADDRESS, gpsData);

            EEPROM.get(EEPROM_TOTAL_DISTANCE_ADDRESS, totalDistanceKm);
            EEPROM.get(EEPROM_LAST_DISTANCE_LOGGED_ADDRESS, lastDistanceLoggedKm);

            lastLatitude = gpsData.lastLat;
            lastLongitude = gpsData.lastLng;
        }
    }

    void update()
    {
        if (hasFix)
        {
            float distanceMeters = gps.distanceBetween(lastLatitude, lastLongitude, latitude, longitude);

            double diff = fabs(longitude - lastLongitude);
            if (diff > 180.0)
                distanceMeters = 0;

            if (distanceMeters <= MAX_JUMP_DISTANCE_METERS)
                distanceMeters = 0;

            distanceMetersAccumulator += (unsigned long)distanceMeters;
            if (distanceMeters < 0)
                distanceMeters = 0;

            while (distanceMetersAccumulator >= 1000)
            {
                totalDistanceKm++;
                distanceMetersAccumulator -= 1000;
            }

            lastLatitude = latitude;
            lastLongitude = longitude;

            if ((totalDistanceKm - lastDistanceLoggedKm) >= 250)
            {
                BACKLOG::saveHistoricalData();

                lastDistanceLoggedKm = totalDistanceKm;

                EEPROM.put(EEPROM_LAST_DISTANCE_LOGGED_ADDRESS, 0xFFFFFFFF);

                EEPROM.put(EEPROM_LAST_DISTANCE_LOGGED_ADDRESS, lastDistanceLoggedKm);
            }

            if (beaconNum % 20 == 0 && satellites > 3)
            {
                GpsData gpsData = {lastLatitude, lastLongitude};

                EEPROM.put(EEPROM_TOTAL_DISTANCE_ADDRESS, 0xFFFFFFFF);
                EEPROM.put(EEPROM_GPS_DATA_ADDRESS, 0xFFFFFFFF);
                EEPROM.put(EEPROM_GPS_DATA_ADDRESS + 4, 0xFFFFFFFF);

                EEPROM.put(EEPROM_TOTAL_DISTANCE_ADDRESS, totalDistanceKm);
                EEPROM.put(EEPROM_GPS_DATA_ADDRESS, gpsData);
            }

            int currentMinute = gps.time.minute();
            if (currentMinute % SAVE_TO_EEPROM_INTERVAL_MINUTES == 0 && currentMinute != lastSaveMinute)
            {
                GpsData gpsData = {lastLatitude, lastLongitude};

                EEPROM.put(EEPROM_TOTAL_DISTANCE_ADDRESS, 0xFFFFFFFF);
                EEPROM.put(EEPROM_GPS_DATA_ADDRESS, 0xFFFFFFFF);
                EEPROM.put(EEPROM_GPS_DATA_ADDRESS + 4, 0xFFFFFFFF);

                EEPROM.put(EEPROM_TOTAL_DISTANCE_ADDRESS, totalDistanceKm);
                EEPROM.put(EEPROM_GPS_DATA_ADDRESS, gpsData);

                lastSaveMinute = currentMinute;
            }
        }
    }
}