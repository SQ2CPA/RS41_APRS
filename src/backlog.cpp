#include "backlog.h"
#include "radio.h"

extern double latitude;
extern double longitude;

extern int beaconNum;

int beaconsCount = 10;

extern TinyGPSPlus gps;

char locator_qth[7];

namespace BACKLOG
{
    char final_buffer[256] = {0};

    char *ax25_base91enc(char *s, uint8_t n, uint64_t v)
    {
        if (v >= (uint64_t)pow(91, n))
        {
            memset(s, '\0', n + 1);
            return s;
        }

        memset(s, '\0', n + 1);
        for (s += n, *s = '\0'; n; n--)
        {
            *(--s) = (v % 91) + 33;
            v /= 91;
        }
        return s;
    }

    void saveToEEPROM(const char *encoded_data)
    {
        uint8_t current_index;
        EEPROM.get(EEPROM_BASE91_INDEX_ADDRESS, current_index);

        int retries = 0;

        while (true)
        {
            if (current_index >= EEPROM_BASE91_SLOT_COUNT)
            {
                current_index = 0;
            }

            if (++retries > 10)
                break;

            uint16_t write_address = EEPROM_BASE91_DATA_START + current_index * EEPROM_BASE91_SLOT_SIZE;

            EEPROM.put(0, 0xFFFFFFFF);

            EEPROM.put(write_address, 0xFFFFFFFF);
            EEPROM.put(write_address + 4, 0xFFFFFFFF);

            const uint32_t *ptr = reinterpret_cast<const uint32_t *>(encoded_data);
            uint32_t part1 = ptr[0];
            uint32_t part2 = ptr[1];

            EEPROM.put(write_address, part1);
            EEPROM.put(write_address + 4, part2);

            bool isOk = true;

            for (int i = 0; i < EEPROM_BASE91_SLOT_SIZE; i++)
            {
                byte d;
                EEPROM.get(write_address + i, d);

                if (d == 0xFF)
                {
                    isOk = false;
                    break;
                }
            }

            current_index = (current_index + 1) % EEPROM_BASE91_SLOT_COUNT;

            if (isOk)
                break;

            EEPROM.put(write_address, 0xFFFFFFFF);
            EEPROM.put(write_address + 4, 0xFFFFFFFF);
        }

        EEPROM.put(EEPROM_BASE91_INDEX_ADDRESS, 0xFF);
        EEPROM.put(EEPROM_BASE91_INDEX_ADDRESS, current_index);
    }

    void saveHistoricalData()
    {
        double lat = latitude;
        double lng = longitude;
        uint8_t day = gps.date.day();
        uint8_t month = gps.date.month();
        uint8_t hour = gps.time.hour();
        long altitudem = gps.altitude.meters();

        if (lat < -90.0 || lat > 90.0 || lng < -180.0 || lng > 180.0)
        {
            strcpy(locator_qth, "ERROR");
            return;
        }

        lat += 90.0;
        lng += 180.0;

        locator_qth[0] = 'A' + (int)(lng / 20);
        locator_qth[1] = 'A' + (int)(lat / 10);
        locator_qth[2] = '0' + ((int)lng % 20) / 2;
        locator_qth[3] = '0' + ((int)lat % 10);
        locator_qth[4] = 'a' + round((lng - floor(lng)) * 12);
        locator_qth[5] = 'a' + round((lat - floor(lat)) * 24);
        locator_qth[6] = '\0';

        uint8_t altitude_encoded;
        if (altitudem < 9000)
        {
            altitude_encoded = 0;
        }
        else if (altitudem > 16000)
        {
            altitude_encoded = 29;
        }
        else
        {
            altitude_encoded = (altitudem - 9000) / 250;
        }

        uint64_t dataBits = 0;
        dataBits |= ((uint64_t)(locator_qth[0] - 'A')) << 47;
        dataBits |= ((uint64_t)(locator_qth[1] - 'A')) << 42;
        dataBits |= ((uint64_t)(locator_qth[2] - '0')) << 38;
        dataBits |= ((uint64_t)(locator_qth[3] - '0')) << 34;
        dataBits |= ((uint64_t)(locator_qth[4] - 'a')) << 29;
        dataBits |= ((uint64_t)(locator_qth[5] - 'a')) << 24;
        dataBits |= ((uint64_t)(month & 0x0F)) << 20;
        dataBits |= ((uint64_t)(day & 0x1F)) << 15;
        dataBits |= ((uint64_t)(hour & 0x1F)) << 10;
        dataBits |= ((uint64_t)(altitude_encoded & 0x1F));

        char encoded_base91[9];
        ax25_base91enc(encoded_base91, 8, dataBits);
        saveToEEPROM(encoded_base91);
    }

    char *checkBeaconInterval()
    {
        if (beaconNum < 5)
            return nullptr;

        beaconsCount++;

        if (beaconsCount < 10)
            return nullptr;

        beaconsCount = 0;

        uint8_t current_index;

        EEPROM.get(EEPROM_BASE91_INDEX_ADDRESS, current_index);

        uint8_t sent_slots = 0;

        final_buffer[0] = '\0';

        uint8_t oldest_index = (current_index + 1) % EEPROM_BASE91_SLOT_COUNT;
        uint8_t middle_index = (current_index + EEPROM_BASE91_SLOT_COUNT / 2) % EEPROM_BASE91_SLOT_COUNT;
        uint8_t newest_index = current_index;

        auto read_slot = [&](uint8_t index)
        {
            uint16_t read_address = EEPROM_BASE91_DATA_START + index * EEPROM_BASE91_SLOT_SIZE;

            char backlog_buffer[EEPROM_BASE91_SLOT_SIZE + 1] = {0};

            uint32_t part1;
            uint32_t part2;

            EEPROM.get(read_address, part1);
            EEPROM.get(read_address + 4, part2);

            memcpy(backlog_buffer, &part1, sizeof(uint32_t));
            memcpy(backlog_buffer + sizeof(uint32_t), &part2, sizeof(uint32_t));

            backlog_buffer[EEPROM_BASE91_SLOT_SIZE] = '\0';

            bool isOk = true;

            for (int j = 0; j < EEPROM_BASE91_SLOT_SIZE; j++)
            {
                if (backlog_buffer[j] == 0xFF)
                {
                    isOk = false;
                    break;
                }
            }

            if (isOk && backlog_buffer[0] >= 33 && backlog_buffer[0] <= 126)
            {
                strcat(final_buffer, backlog_buffer);
                sent_slots++;
            }
        };

        uint8_t i = 0;
        while (sent_slots < 15 && i < EEPROM_BASE91_SLOT_COUNT)
        {
            if (sent_slots < 15)
                read_slot((oldest_index + i) % EEPROM_BASE91_SLOT_COUNT);
            if (sent_slots < 15)
                read_slot((middle_index + i) % EEPROM_BASE91_SLOT_COUNT);
            if (sent_slots < 15)
                read_slot((newest_index + EEPROM_BASE91_SLOT_COUNT - i) % EEPROM_BASE91_SLOT_COUNT);
            i++;
        }

        if (sent_slots == 0)
            sprintf(final_buffer, "BACKLOG_EMPTY");

        current_index = (current_index + 15) % EEPROM_BASE91_SLOT_COUNT;

        EEPROM.put(0, 0xFFFFFFFF);

        EEPROM.put(EEPROM_BASE91_INDEX_ADDRESS, 0xFF);
        EEPROM.put(EEPROM_BASE91_INDEX_ADDRESS, current_index);

        return final_buffer;
    }
}