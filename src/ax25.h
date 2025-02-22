#ifndef __AX25__H__
#define __AX25__H__

#include <Arduino.h>

#define APRS_1200_MARK_DELAY 400
#define APRS_2400_SPACE_DELAY 200
#define APRS_FLAGS_AT_BEGINNING 30

void ax25_TX_flag(uint8_t len);
void ax25_TX_c_string(const char *c_string, uint16_t len);
void ax25_TX_byte(byte tx_byte, bool is_flag);

void ax25_aprs_send_position_packet(uint64_t *freq, uint8_t pwr, uint32_t deviation, const char *src_call, uint8_t src_SSID, char *latitude, char *longitude, char *timestamp, char *comment);
void ax25_aprs_send_status_packet(uint64_t *freq, uint8_t pwr, uint32_t deviation, const char *src_call, uint8_t src_SSID, char *status_message);

#endif