#include "Arduino.h"
#include "configuration.h"

extern uint32_t lastBeaconTx;

extern int beaconNum;
extern int beaconLoRaFrequency;

extern int beaconInterval;

namespace BEACON
{
    bool checkBeaconInterval()
    {
        uint32_t lastTx = millis() - lastBeaconTx;

        if (lastBeaconTx == 0 || lastTx >= beaconInterval * 1000)
        {
            lastBeaconTx = millis();
            beaconNum++;

            beaconLoRaFrequency++;

            if (beaconLoRaFrequency >= 3) // Include UK 439 Mhz LoRa QRG
                // if (beaconLoRaFrequency >= 2) // Exclude UK 439 Mhz LoRa QRG
                beaconLoRaFrequency = 0;
            return true;
        }

        return false;
    }
}