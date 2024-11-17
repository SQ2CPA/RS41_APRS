#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_APRS_CALLSIGN "SP0LND" // APRS CALLSIGN without SSID
#define CONFIG_APRS_SSID 11 // We prefer SSID -11 for APRS balloons

#define CONFIG_APRS_FLIGHT_ID 1 // Flight number, used as N in comment

#define CONFIG_STARTUP_TONE_ENABLE // enable startup tone at 144.025 MHz

#define CONFIG_STM32_CLOCK_6MHZ // SET 6 MHz clock speed instead of 24 MHz (lower power usage by STM32)

#endif
