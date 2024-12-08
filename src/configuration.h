#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define CONFIG_APRS_CALLSIGN "SQ2CPA" // APRS CALLSIGN WITHOUT SSID (max 6 chars!)
#define CONFIG_APRS_SSID 11           // APRS SSID (0-15)

#define CONFIG_APRS_INTERVAL 10 // in seconds (AFSK and LoRa at once)

#define CONFIG_APRS_FLIGHT_ID 1 // flight number, used as N in APRS comment

#define CONFIG_STARTUP_TONE_ENABLE // enable startup tone at 144.025 MHz

#endif