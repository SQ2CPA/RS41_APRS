#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define CONFIG_APRS_CALLSIGN "SP0LND" // APRS CALLSIGN WITHOUT SSID (max 6 chars!)
#define CONFIG_APRS_SSID 1            // APRS SSID (0-15)

#define CONFIG_APRS_INTERVAL 45 // in seconds (AFSK and LoRa at once)

#define CONFIG_APRS_FLIGHT_ID 1 // flight number, used as N in APRS comment

#define CONFIG_STARTUP_TONE_ENABLE // enable startup tone at 144.025 MHz

#define CONFIG_NOHUB // add NOHUB path to be invisible on Sondehub

#define CONFIG_RADIO_OFFSET_AFSK -0.005000F
#define CONFIG_RADIO_OFFSET_LORA -0.015900F

#endif