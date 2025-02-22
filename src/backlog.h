#ifndef BACKLOG_H_
#define BACKLOG_H_

#include <Arduino.h>
#include <EEPROM.h>
#include <TinyGPS++.h>

#define EEPROM_BASE91_SLOT_COUNT 140
#define EEPROM_BASE91_SLOT_SIZE 8
#define EEPROM_BASE91_DATA_START 32
#define EEPROM_BASE91_INDEX_ADDRESS (EEPROM_BASE91_DATA_START + EEPROM_BASE91_SLOT_COUNT * EEPROM_BASE91_SLOT_SIZE)

#define EEPROM_LOCAL_OFFSET_ADDRESS 24

namespace BACKLOG
{
    void saveHistoricalData();

    char *checkBeaconInterval();
}

#endif