#include <RadioLib.h>
#include "pinout.h"
#include "configuration.h"

SX1278 radio = new Module(SX1278_NSS, SX1278_DIO0, SX1278_RESET, SX1278_DIO1);

AFSKClient audio(&radio, SX1278_DIO2);

AX25Client ax25(&audio);

APRSClient aprsAFSK(&ax25);

extern int outputPower;

extern double latitude;
extern double longitude;

extern String comment;

namespace RADIO
{
    int setupAFSK(int frequency)
    {
        int state = radio.beginFSK(float(frequency + CONFIG_RADIO_OFFSET_AFSK) / 1000000.0F);
        if (state != RADIOLIB_ERR_NONE)
        {
            return state;
        }

        state = audio.begin();

        state = ax25.begin(CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID);

#ifdef CONFIG_RADIO_AFSK_MARK
        ax25.setCorrection(CONFIG_RADIO_AFSK_MARK, CONFIG_RADIO_AFSK_SPACE, CONFIG_RADIO_AFSK_LENGTH);
#endif

        state = aprsAFSK.begin('O');

        radio.setOutputPower(outputPower);

        return state;
    }

    int setupLoRa(int frequency, bool isFast)
    {
        int state = radio.begin(float(frequency + CONFIG_RADIO_OFFSET_LORA) / 1000000.0F);
        if (state != RADIOLIB_ERR_NONE)
        {
            return state;
        }

        if (isFast)
        {
            radio.setSpreadingFactor(9);
            radio.setCodingRate(7);
            radio.setBandwidth(125);
        }
        else
        {

            radio.setSpreadingFactor(12);
            radio.setCodingRate(5);
            radio.setBandwidth(125);
        }

        radio.setOutputPower(outputPower);

        return state;
    }

    bool setup()
    {
        int state = setupAFSK(433000000);

        return (state == RADIOLIB_ERR_NONE);
    }

    void startupTone()
    {
        setupAFSK(144025000);

#ifdef CONFIG_TEST_AFSK_MARK
        audio.tone(1200);

        delay(300000);
#endif

#ifdef CONFIG_TEST_AFSK_SPACE
        audio.tone(2200);

        delay(300000);
#endif

        for (int i = 0; i < 50; i++)
        {
            audio.tone(100 * i);
            delay(10);
        }

        delay(50);

        audio.noTone();

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

    String double2string(double n, int ndec)
    {
        String r = "";
        if (n > -1 && n < 0)
        {
            r = "-";
        }
        int v = n;
        r += v;
        r += '.';
        for (int i = 0; i < ndec; i++)
        {
            n -= v;
            n = 10 * abs(n);
            v = n;
            r += v;
        }
        return r;
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

    void sendAFSK()
    {
#ifdef CONFIG_NOHUB
        char *repeaterCallsigns[] = {"NOHUB"};
        uint8_t repeaterSSIDs[] = {0};
        aprsAFSK.useRepeaters(repeaterCallsigns, repeaterSSIDs, 1);
#endif

        char lat[12];
        char lng[13];

        processLatitudeAPRS(latitude, lat);
        processLongitudeAPRS(longitude, lng);

        aprsAFSK.sendPosition("APLAIR", 0, lat, lng, (char *)comment.c_str());
    }

    void sendLoRa()
    {
        char lat[12];
        char lng[13];

        processLatitudeAPRS(latitude, lat);
        processLongitudeAPRS(longitude, lng);

        char packet[256];
#ifdef CONFIG_NOHUB
        snprintf(packet, sizeof(packet), "%s-%d>APLAIR,NOHUB:!%s/%sO%s", CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID, lat, lng, comment.c_str());
#else
        snprintf(packet, sizeof(packet), "%s-%d>APLAIR:!%s/%sO%s", CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID, lat, lng, comment.c_str());
#endif

        char fullPacket[260];
        snprintf(fullPacket, sizeof(fullPacket), "\x3c\xff\x01%s", packet);
        radio.transmit(fullPacket);
    }

    void reset()
    {
        radio.standby();
        radio.reset();
    }
}