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
    int setupAFSK(float frequency)
    {
        int state = radio.beginFSK(frequency + float(CONFIG_RADIO_OFFSET_AFSK));
        if (state != RADIOLIB_ERR_NONE)
        {
            return state;
        }

        state = audio.begin();
        if (state != RADIOLIB_ERR_NONE)
        {
            return state;
        }

        state = ax25.begin(CONFIG_APRS_CALLSIGN, CONFIG_APRS_SSID);
        if (state != RADIOLIB_ERR_NONE)
        {
            return state;
        }

        state = aprsAFSK.begin('O');
        if (state != RADIOLIB_ERR_NONE)
        {
            return state;
        }

        radio.setOutputPower(outputPower);

        return state;
    }

    int setupLoRa(float frequency, String speed)
    {
        int state = radio.begin(frequency + float(CONFIG_RADIO_OFFSET_LORA));

        if (speed == "300")
        {
            radio.setSpreadingFactor(12);
            radio.setCodingRate(5);
            radio.setBandwidth(125);
        }
        else
        {
            radio.setSpreadingFactor(9);
            radio.setCodingRate(7);
            radio.setBandwidth(125);
        }

        radio.setOutputPower(outputPower);

        return state;
    }

    bool setup()
    {
        int state = setupAFSK(433.0);

        return (state == RADIOLIB_ERR_NONE);
    }

    void startupTone()
    {
        setupAFSK(144.025);

        for (int i = 0; i < 50; i++)
        {
            audio.tone(100 * i);
            delay(20);
        }

        delay(250);

        audio.noTone();
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

    String processLatitudeAPRS(double lat)
    {
        String degrees = double2string(lat, 6);
        String north_south, latitude, convDeg3;
        float convDeg, convDeg2;

        if (abs(degrees.toFloat()) < 10)
        {
            latitude += "0";
        }
        if (degrees.indexOf("-") == 0)
        {
            north_south = "S";
            latitude += degrees.substring(1, degrees.indexOf("."));
        }
        else
        {
            north_south = "N";
            latitude += degrees.substring(0, degrees.indexOf("."));
        }
        convDeg = abs(degrees.toFloat()) - abs(int(degrees.toFloat()));
        convDeg2 = (convDeg * 60) / 100;
        convDeg3 = String(convDeg2, 6);
        latitude += convDeg3.substring(convDeg3.indexOf(".") + 1, convDeg3.indexOf(".") + 3) + "." + convDeg3.substring(convDeg3.indexOf(".") + 3, convDeg3.indexOf(".") + 5);
        latitude += north_south;
        return latitude;
    }

    String processLongitudeAPRS(double lon)
    {
        String degrees = double2string(lon, 6);
        String east_west, longitude, convDeg3;
        float convDeg, convDeg2;

        if (abs(degrees.toFloat()) < 100)
        {
            longitude += "0";
        }
        if (abs(degrees.toFloat()) < 10)
        {
            longitude += "0";
        }
        if (degrees.indexOf("-") == 0)
        {
            east_west = "W";
            longitude += degrees.substring(1, degrees.indexOf("."));
        }
        else
        {
            east_west = "E";
            longitude += degrees.substring(0, degrees.indexOf("."));
        }
        convDeg = abs(degrees.toFloat()) - abs(int(degrees.toFloat()));
        convDeg2 = (convDeg * 60) / 100;
        convDeg3 = String(convDeg2, 6);
        longitude += convDeg3.substring(convDeg3.indexOf(".") + 1, convDeg3.indexOf(".") + 3) + "." + convDeg3.substring(convDeg3.indexOf(".") + 3, convDeg3.indexOf(".") + 5);
        longitude += east_west;
        return longitude;
    }

    void sendAFSK()
    {
#ifdef CONFIG_NOHUB
        char *repeaterCallsigns[] = {"NOHUB"};
        uint8_t repeaterSSIDs[] = {0};
        aprsAFSK.useRepeaters(repeaterCallsigns, repeaterSSIDs, 1);
#endif

        String lat = processLatitudeAPRS(latitude);
        String lng = processLongitudeAPRS(longitude);

        aprsAFSK.sendPosition("APLAIR", 0, (char *)lat.c_str(), (char *)lng.c_str(), (char *)comment.c_str());
    }

    void sendLoRa()
    {
        String lat = processLatitudeAPRS(latitude);
        String lng = processLongitudeAPRS(longitude);

        String packet = "";

#ifdef CONFIG_NOHUB
        packet = String(CONFIG_APRS_CALLSIGN) + "-" + String(CONFIG_APRS_SSID) + ">APLAIR,NOHUB:!" + lat + "/" + lng + "O" + comment;
#else
        packet = String(CONFIG_APRS_CALLSIGN) + "-" + String(CONFIG_APRS_SSID) + ">APLAIR:!" + lat + "/" + lng + "O" + comment;
#endif

        radio.transmit("\x3c\xff\x01" + packet);
    }

    void reset()
    {
        radio.standby();
        radio.reset();
    }
}