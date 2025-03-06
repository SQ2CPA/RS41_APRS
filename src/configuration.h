#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define CONFIG_APRS_CALLSIGN "N0CALL" // APRS CALLSIGN WITHOUT SSID (max 6 chars!)
#define CONFIG_APRS_SSID 11           // APRS SSID (0-15)

#define CONFIG_APRS_INTERVAL 45        // beacon interval in seconds (AFSK and LoRa at once, first AFSK then LoRa), default to 45
#define CONFIG_APRS_INTERVAL_NOFIX 120 // beacon interval without GPS fix in seconds (AFSK and LoRa at once, first AFSK then LoRa), defaults to 120

#define CONFIG_APRS_FLIGHT_ID 1 // flight number, used as N attribute in APRS comment

#define CONFIG_STARTUP_TONE_ENABLE // enable startup tone at 144.025 MHz

#define CONFIG_NOHUB // add NOHUB path to be ignored by sondehub APRSIS gateway (if you want to use aprs2sondehub)

#define CONFIG_VDD_CALIBRATION 1.0F // Internal voltage calibration [only for STM32L]
#define CONFIG_TEMPERATURE_OFFSET 0 // Temperature offset [only for STM32L]

#define CONFIG_NO_TXCO // Uncomment if you don't use TXCO with SX1278

// #define CONFIG_SET_BOR [only for STM32L]
// #define CONFIG_ERASE_EEPROM [only for STM32L]

#endif