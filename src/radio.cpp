#include "pinout.h"
#include "configuration.h"
#include <Arduino.h>
#include "SX1278.h"
#include "ax25.h"

extern uint8_t outputPower;

extern double latitude;
extern double longitude;

extern String comment;

extern char timestamp[7];

namespace RADIO
{
    static uint64_t currentFrequency = 433000000;

    void setupAFSK(int frequency)
    {
        currentFrequency = frequency;

        SX1278_begin();

        delay(100);
    }

    void setupLoRa(int frequency, bool isFast)
    {
        currentFrequency = frequency;

        SX1278_begin();

        if (isFast)
        {
            SX1278_init_LoRa(9, 125, 7, frequency);
        }
        else
        {
            SX1278_init_LoRa(12, 125, 5, frequency);
        }

        SX1278_set_LoRa_power(outputPower);

        delay(100);
    }

    void startupTone()
    {
        setupAFSK(144025000);

        for (int i = 0; i < 50; i++)
        {
            uint64_t frequency = currentFrequency + 100 * i;
            SX1278_enable_TX_direct(&frequency, 15, 3000);

            unsigned long startedAt = millis();
            while (millis() - startedAt < 10)
            {
                SX1278_mod_direct_out(500);
            }
        }

        SX1278_sleep();

        delay(1000);
    }

    char *ax25_base91enc(char *s, uint8_t n, uint32_t v)
    {
        for (s += n, *s = '\0'; n; n--)
        {
            *(--s) = v % 91 + 33;
            v /= 91;
        }
        return (s);
    }

    void processLatitudeAPRS(double lat, char *result)
    {
        char north_south = (lat < 0) ? 'S' : 'N';
        lat = fabs(lat);

        int degrees = (int)lat;
        double minutes = (lat - degrees) * 60.0;

        sprintf(result, "%02d%02d.%02d%c", degrees, (int)minutes, (int)((minutes - (int)minutes) * 100), north_south);
    }

    void processLongitudeAPRS(double lon, char *result)
    {
        char east_west = (lon < 0) ? 'W' : 'E';
        lon = fabs(lon);

        int degrees = (int)lon;
        double minutes = (lon - degrees) * 60.0;

        sprintf(result, "%03d%02d.%02d%c", degrees, (int)minutes, (int)((minutes - (int)minutes) * 100), east_west);
    }

    void sendAFSKStatus(char *status)
    {
        ax25_aprs_send_status_packet(&currentFrequency, outputPower, SX1278_DEVIATION, CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID, status);
    }

    void sendAFSK()
    {
        char lat[12];
        char lng[13];

        processLatitudeAPRS(latitude, lat);
        processLongitudeAPRS(longitude, lng);

        ax25_aprs_send_position_packet(&currentFrequency, outputPower, SX1278_DEVIATION, CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID, lat, lng, timestamp, (char *)comment.c_str());
    }

    void sendLoRaStatus(char *status)
    {
        char packet[256];
        snprintf(packet, sizeof(packet), "\x3c\xff\x01%s-%d>APLAIR,NOHUB:>%s", CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID, status);

        SX1278_send_LoRa_packet((const uint8_t *)packet, strlen(packet));
    }

    void sendLoRa()
    {
        char lat[12];
        char lng[13];

        processLatitudeAPRS(latitude, lat);
        processLongitudeAPRS(longitude, lng);

        char packet[256];

#ifdef CONFIG_NOHUB
        snprintf(packet, sizeof(packet), "\x3c\xff\x01%s-%d>APLAIR,NOHUB:/%sh%s/%sO%s", CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID, timestamp, lat, lng, comment.c_str());
#else
        snprintf(packet, sizeof(packet), "\x3c\xff\x01%s-%d>APLAIR:/%sh%s/%sO%s", CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID, timestamp, lat, lng, comment.c_str());
#endif

        SX1278_send_LoRa_packet((const uint8_t *)packet, strlen(packet));
    }

    void reset()
    {
        SX1278_reset();
    }
}