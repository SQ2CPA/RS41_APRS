#ifndef __SX1278__H__
#define __SX1278__H__

#include <Arduino.h>

#define SX1278_DEVIATION 3000 // FSK deviation in Hz

#define SX1278_CRYSTAL_FREQ 32000000

#define MOD_DIO2 0
#define MOD_F_HOP 1

#define SX1278_MOD_OPTION MOD_DIO2

void SX1278_begin(void);

void SX1278_enable(void);
void SX1278_disable(void);
void SX1278_sleep(void);
void SX1278_reset(void);

void SX1278_enable_TX_direct(uint64_t *freq, uint8_t pwr, uint16_t deviation);

void SX1278_mod_direct_out(uint32_t delay);
void SX1278_set_direct__out(bool value);

void SX1278_set_TX_deviation(uint64_t *freq, uint16_t deviation);
void SX1278_set_TX_power(uint8_t pwr, bool pa_boost_mode_20dbm);
void SX1278_set_TX_frequency(uint64_t *freq);

void SX1278_init_LoRa(uint8_t sf, uint8_t bw, uint8_t cr, uint64_t frequency);

void SX1278_set_LoRa_power(uint8_t txPower);

void SX1278_send_LoRa_packet(const uint8_t *data, size_t length);

bool SX1278_LoRa_transmission_done();

#endif