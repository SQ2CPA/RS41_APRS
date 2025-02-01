#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define CONFIG_APRS_CALLSIGN "N0CALL" // APRS CALLSIGN WITHOUT SSID (max 6 chars!)
#define CONFIG_APRS_SSID 0            // APRS SSID (0-15)

#define CONFIG_APRS_INTERVAL 45        // beacon interval in seconds (AFSK and LoRa at once, first AFSK then LoRa), default to 45
#define CONFIG_APRS_INTERVAL_NOFIX 120 // beacon interval without GPS fix in seconds (AFSK and LoRa at once, first AFSK then LoRa), defaults to 120

#define CONFIG_APRS_FLIGHT_ID 1 // flight number, used as N attribute in APRS comment

#define CONFIG_STARTUP_TONE_ENABLE // enable startup tone at 144.025 MHz

#define CONFIG_NOHUB // add NOHUB path to be ignored by sondehub APRSIS gateway (if you want to use aprs2sondehub)

#define CONFIG_RADIO_OFFSET_AFSK 0 // not required if you have TXCO instead of crystal for SX1278
#define CONFIG_RADIO_OFFSET_LORA 0 // not required if you have TXCO instead of crystal for SX1278

#define CONFIG_RADIO_AFSK_MARK 4          // Positive or negative correction offset for mark (1200 Hz) audio frequency in Hz
#define CONFIG_RADIO_AFSK_SPACE 8         // Positive or negative correction offset for space (2200 Hz) audio frequency in Hz
#define CONFIG_RADIO_AFSK_LENGTH 1.00545F // Audio tone length modifier, defaults to 1.0F

// #define CONFIG_TEST_AFSK_MARK
// #define CONFIG_TEST_AFSK_SPACE

// #define CONFIG_SET_BOR

#endif