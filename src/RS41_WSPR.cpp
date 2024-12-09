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

void setup()
{
    // Serial1.begin(9600); // DEBUG ONLY

    pinMode(GPS_EN, OUTPUT);

    digitalWrite(GPS_EN, LOW);

    RADIO::setup();

#ifdef CONFIG_STARTUP_TONE_ENABLE
    RADIO::startupTone();
#endif
}

int outputPower = 15;

double latitude = 0, longitude = 0;
String comment = "";

int outputPowerFrames = 0;

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

void loop()
{
    while (Serial1.available() > 0)
    {
        byte data = Serial1.read();

        gps.encode(data);
    }

    comment = "P" + String(beaconNum);
    comment += "S" + String(gps.satellites.value());

    if (outputPower < 20)
        comment += "O" + String(outputPower);

    comment += "N" + String(CONFIG_APRS_FLIGHT_ID);

    comment += " ";

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
    }
    else if (!hasFix)
    {
        hasFix = true;
        timeToFix = (millis() - timeToFixFrom) / 1000;
    }

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

        outputPowerFrames++;

        if (outputPowerFrames >= 6 && outputPower < 20)
        {
            outputPowerFrames = 0;
            outputPower++;
        }

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

        float frequencyAFSK = GEOFENCE::getAFSKFrequency(latitude, longitude);

        RADIO::setupAFSK(frequencyAFSK);
        RADIO::sendAFSK();

        switch (beaconLoRaFrequency)
        {
        case 1:
            RADIO::setupLoRa(APRS_LORA_FREQUENCY_FAST, "1200");
            break;
        case 0:
            RADIO::setupLoRa(APRS_LORA_FREQUENCY_SLOW, "300");
            break;
        case 2:
            RADIO::setupLoRa(APRS_LORA_FREQUENCY_UK, "300");
            break;
        }

        RADIO::sendLoRa();

        RADIO::reset();

        turnOnGPS();
    }
}