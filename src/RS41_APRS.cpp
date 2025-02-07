#include "Arduino.h"
#include <RadioLib.h>
#include "pinout.h"
#include "configuration.h"
#include "radio.h"
#include "beacon.h"
#include <TinyGPS++.h>
#include "geofence.h"

HardwareSerial Serial1(GPS_RX, GPS_TX);

TinyGPSPlus gps;

int beaconLoRaFrequency = 0; // 0-2
int beaconNum = 0;
uint32_t lastBeaconTx = 0;

bool hasFix = false;
uint32_t timeToFixFrom = millis();
int timeToFix = -1;

void turnOnGPS()
{
    digitalWrite(GPS_EN, HIGH);
    Serial1.begin(9600);

    hasFix = false;
    timeToFixFrom = millis();
    timeToFix = -1;
}

void turnOffGPS()
{
    Serial1.end();
    digitalWrite(GPS_EN, LOW);
}
void setup()
{
#ifdef CONFIG_SET_BOR
    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();

    FLASH_OBProgramInitTypeDef OBInit;
    HAL_FLASHEx_OBGetConfig(&OBInit);

    OBInit.OptionType = OPTIONBYTE_BOR;
    OBInit.BORLevel = OB_BOR_LEVEL2; // 1=1.7V, 2=1.8V, 3=2.1V

    if (HAL_FLASHEx_OBProgram(&OBInit) == HAL_OK)
    {
        HAL_FLASH_OB_Launch();
    }

    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
#endif

    pinMode(GPS_EN, OUTPUT);

    digitalWrite(GPS_EN, LOW);

    RADIO::setup();

#ifdef CONFIG_STARTUP_TONE_ENABLE
    RADIO::startupTone();
#endif

    delay(10000);
}

int outputPower = 15;

double latitude = 0, longitude = 0;
String comment = "";

int beaconInterval = 0;

void loop()
{
    while (Serial1.available() > 0)
    {
        byte data = Serial1.read();

        gps.encode(data);
    }

    comment = "P" + String(beaconNum);
    comment += "S" + String(gps.satellites.value());
    comment += "O" + String(outputPower);
    comment += "F0";

    comment += "N" + String(CONFIG_APRS_FLIGHT_ID);

    comment += " ";

    beaconInterval = int(CONFIG_APRS_INTERVAL);

    if (!gps.altitude.isValid())
    {
        if (gps.location.isValid())
        {
            comment += "2D ";
        }
        else if (gps.time.isValid())
        {
            if (gps.time.second() == 0 && gps.time.minute() == 0 && gps.time.hour() == 0)
            {
                comment += "T- ";
            }
            else
            {
                comment += "T+ ";
            }
        }

        beaconInterval = int(CONFIG_APRS_INTERVAL_NOFIX);
    }
    else if (!hasFix)
    {
        if (gps.satellites.value() == 0)
        {
            beaconInterval = int(CONFIG_APRS_INTERVAL_NOFIX);
        }
        else
        {
            hasFix = true;

            timeToFix = (millis() - timeToFixFrom) / 1000;
        }
    }

    if (timeToFix > -1)
        comment += "FT" + String(timeToFix);

    int knots = 0,
        course = 0;

    int altitudeInFeet = 0, altitudeInMeters = 0;

    if (gps.location.isValid())
    {
        latitude = gps.location.lat();
        longitude = gps.location.lng();

        knots = gps.speed.knots();
        course = gps.course.deg();

        altitudeInFeet = gps.altitude.feet();
        altitudeInMeters = gps.altitude.meters();

        if (altitudeInFeet < 0)
            altitudeInFeet = 0;

        if (altitudeInMeters < 0)
            altitudeInMeters = 0;
    }
    else
    {
        latitude = 0;
        longitude = 0;

        knots = 0;
        course = 0;
        altitudeInFeet = 0;
        altitudeInMeters = 0;
    }

    if (BEACON::checkBeaconInterval())
    {
        turnOffGPS();

        outputPower = 15 + beaconNum / 3;

        if (outputPower > 20)
            outputPower = 20;

        String tspeed = String(knots);
        String tdirection = String(course);
        String taltitude = String(altitudeInFeet);

        while (tspeed.length() < 3)
            tspeed = "0" + tspeed;
        while (tdirection.length() < 3)
            tdirection = "0" + tdirection;
        while (taltitude.length() < 6)
            taltitude = "0" + taltitude;

        comment = tdirection + "/" + tspeed + "/A=" + taltitude + "/" + comment;

        String ocomment = comment;

        int frequencyAFSK = GEOFENCE::getAFSKFrequency(latitude, longitude);

        RADIO::setupAFSK(frequencyAFSK);
        RADIO::sendAFSK();

        comment = ocomment;

        switch (beaconLoRaFrequency)
        {
        case 0:
            RADIO::setupLoRa(APRS_LORA_FREQUENCY_FAST, true);
            comment.replace("F0", "F2");
            break;
        case 1:
            RADIO::setupLoRa(APRS_LORA_FREQUENCY_SLOW, false);
            comment.replace("F0", "F1");
            break;
        case 2:
            RADIO::setupLoRa(APRS_LORA_FREQUENCY_UK, false);
            comment.replace("F0", "F3");
            break;
        }

        RADIO::sendLoRa();

        RADIO::reset();

        turnOnGPS();
    }
}