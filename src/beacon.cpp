#include "Arduino.h"
#include "configuration.h"

extern uint32_t lastBeaconTx;

extern int beaconNum;
extern int beaconLoRaFrequency;

namespace BEACON
{
    bool checkBeaconInterval()
    {
        uint32_t lastTx = millis() - lastBeaconTx;

        if (lastBeaconTx == 0 || lastTx >= int(CONFIG_APRS_INTERVAL) * 1000)
        {
            lastBeaconTx = millis();
            beaconNum++;

            beaconLoRaFrequency++;

            if (beaconLoRaFrequency >= 3)
                beaconLoRaFrequency = 0;

            return true;
        }

        return false;
    }
}