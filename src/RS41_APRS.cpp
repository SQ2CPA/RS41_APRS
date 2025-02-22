#include <Arduino.h>
#include "pinout.h"
#include "configuration.h"
#include "radio.h"
#include "beacon.h"
#include <TinyGPS++.h>
#include "geofence.h"
#include "backlog.h"
#include "odometer.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_ll_adc.h"

HardwareSerial Serial1(GPS_RX, GPS_TX);

TinyGPSPlus gps;

int beaconLoRaFrequency = 0; // 0-2
int beaconNum = 0;
uint32_t lastBeaconTx = 0;

bool hasFix = false;
uint32_t timeToFixFrom = millis();
int timeToFix = -1;

char timestamp[7] = "000000";

uint32_t totalDistanceKm = 0;

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

int32_t readVref()
{
#ifdef __LL_ADC_CALC_VREFANALOG_VOLTAGE
    return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION_12B));
#else
    return (VREFINT * ADC_RANGE / analogRead(AVREF));
#endif
}

int32_t readVdd()
{
#ifdef __LL_ADC_CALC_VREFANALOG_VOLTAGE
    return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION_12B));
#else
    return (VREFINT * ADC_RANGE / analogRead(AVREF)); // ADC sample to mV
#endif
}

int32_t readTemperature(int32_t VRef)
{
#ifdef __LL_ADC_CALC_TEMPERATURE
    return (__LL_ADC_CALC_TEMPERATURE(VRef, analogRead(ATEMP), LL_ADC_RESOLUTION_12B));
#elif defined(__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS)
    return (__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(AVG_SLOPE, V25, CALX_TEMP, VRef, analogRead(ATEMP), LL_ADC_RESOLUTION)); //   like this
#else
    return 0;
#endif
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

#ifdef CONFIG_STARTUP_TONE_ENABLE
    RADIO::startupTone();
#endif

#ifdef CONFIG_ERASE_EEPROM
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for (uint32_t i = 0; i < 2048; i += 4)
    {
        HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, DATA_EEPROM_BASE + i, 0xFFFFFFFF);
    }
    HAL_FLASHEx_DATAEEPROM_Lock();

    RADIO::startupTone();

    while (1)
        ;
#endif

    delay(10000);
    ODOMETER::setup();

    pinMode(ATEMP, INPUT_ANALOG);
    pinMode(AVREF, INPUT_ANALOG);

    analogReadResolution(12);
}

uint8_t outputPower = 15;

double latitude = 0, longitude = 0;
String comment = "";

int beaconInterval = 0;

int satellites = 0;

void loop()
{
    while (Serial1.available() > 0)
    {
        byte data = Serial1.read();

        gps.encode(data);
    }

    satellites = gps.satellites.value();

    uint32_t Vref = readVref();

    uint32_t temperature = readTemperature(Vref) + CONFIG_TEMPERATURE_OFFSET;

    comment = "P" + String(beaconNum);
    comment += "S" + String(satellites);

    if (temperature > -100 && temperature < 50)
        comment += "T" + String(temperature);

    char voltage[3];
    sprintf(voltage, "%03d", (int(Vref * CONFIG_VDD_CALIBRATION)) / 10);

    comment += "V" + String(voltage);

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
        if (satellites == 0)
        {
            beaconInterval = int(CONFIG_APRS_INTERVAL_NOFIX);
        }
        else
        {
            hasFix = true;

            timeToFix = (millis() - timeToFixFrom) / 1000;
        }
    }

    if (gps.time.isValid())
        snprintf(timestamp, sizeof(timestamp), "%02d%02d%02d", gps.time.hour(), gps.time.minute(), gps.time.second());

    if (timeToFix > -1)
        comment += "FT" + String(timeToFix);

    if (totalDistanceKm < 1000)
    {
        comment += " ODO=" + String(totalDistanceKm) + "km";
    }
    else
    {
        comment += " ODO=" + String(int((totalDistanceKm + 500) / 1000)) + "kkm";
    }

    // comment += " THIS IS TEST!!!";

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

    if (altitudeInMeters > 9000 && altitudeInMeters < 16000)
        ODOMETER::update();

    if (BEACON::checkBeaconInterval())
    {
        turnOffGPS();

        char *backlogStatus = BACKLOG::checkBeaconInterval();

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

        if (backlogStatus != nullptr)
        {
            RADIO::sendAFSKStatus(backlogStatus);
        }
        else
        {
            RADIO::sendAFSK();
        }

        RADIO::reset();

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

        if (backlogStatus != nullptr)
        {
            RADIO::sendLoRaStatus(backlogStatus);
        }
        else
        {
            RADIO::sendLoRa();
        }

        RADIO::reset();

        turnOnGPS();
    }
}