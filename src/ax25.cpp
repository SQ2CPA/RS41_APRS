#include <Arduino.h>
#include <SPI.h>

#include "ax25.h"
#include "SX1278.h"
#include "configuration.h"

uint16_t continuous_crc = 0xFFFF;

bool rectangle_wave_out_state = true;
uint8_t consecutive_true_bit_counter = 0;

static void ax25_set_rectangle_wave_out(bool rectangle_wave_out_state)
{
    if (rectangle_wave_out_state)
    {
        SX1278_mod_direct_out(APRS_1200_MARK_DELAY);
    }
    else
    {
        SX1278_mod_direct_out(APRS_2400_SPACE_DELAY);
        SX1278_mod_direct_out(APRS_2400_SPACE_DELAY);
    }
}

static void ax25_calc_crc(uint16_t *crc, bool bit)
{
    const uint16_t POLY = 0x8408;
    bool crc_lsb_old = *crc & 0x0001;

    *crc = *crc >> 1;

    if (crc_lsb_old != bit)
        *crc ^= POLY;
}

void ax25_TX_flag(uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
        ax25_TX_byte(0x7E, true);
}

void ax25_TX_c_string(const char *c_string, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        ax25_TX_byte(c_string[i], false);
}

void ax25_TX_byte(byte tx_byte, bool is_flag)
{
    bool working_bit;

    for (uint8_t i = 0; i < 8; i++)
    {
        working_bit = tx_byte & 0x01 << i;

        ax25_calc_crc(&continuous_crc, working_bit);

        if (working_bit)
        {
            ax25_set_rectangle_wave_out(rectangle_wave_out_state);
            consecutive_true_bit_counter++;

            if (consecutive_true_bit_counter == 5 && !is_flag)
            {
                rectangle_wave_out_state = !rectangle_wave_out_state;
                ax25_set_rectangle_wave_out(rectangle_wave_out_state);

                consecutive_true_bit_counter = 0;
            }
        }
        else
        {
            rectangle_wave_out_state = !rectangle_wave_out_state;
            ax25_set_rectangle_wave_out(rectangle_wave_out_state);

            consecutive_true_bit_counter = 0;
        }
    }
}

void ax25_aprs_send_position_packet(uint64_t *freq, uint8_t pwr, uint32_t deviation, const char *src_call, uint8_t src_SSID, char *latitude, char *longitude, char *timestamp, char *comment)
{
    SX1278_enable_TX_direct(freq, pwr, deviation);

    delay(100);

    ax25_TX_flag(APRS_FLAGS_AT_BEGINNING);

    continuous_crc = 0xFFFF;

    char buf[8];
    sprintf(buf, "%-6s%c", "APLAIR", 0);
    for (uint8_t i = 0; i < 6; i++)
        ax25_TX_byte(buf[i] << 1, false);
    ax25_TX_byte((buf[6] << 1), false);

    sprintf(buf, "%-6s%c", src_call, src_SSID);
    for (uint8_t i = 0; i < 6; i++)
        ax25_TX_byte(buf[i] << 1, false);
    ax25_TX_byte((buf[6] << 1), false);

#ifdef CONFIG_NOHUB
    sprintf(buf, "%-6s%c", "NOHUB", 0);
    for (uint8_t i = 0; i < 6; i++)
        ax25_TX_byte(buf[i] << 1, false);
    ax25_TX_byte((buf[6] << 1) | 1, false);
#else
    ax25_TX_byte((buf[6] << 1) | 1, true);
#endif

    ax25_TX_byte(0x03, false);
    ax25_TX_byte(0xf0, false);

    ax25_TX_byte('/', false);
    ax25_TX_c_string(timestamp, strlen(timestamp));
    ax25_TX_byte('h', false);

    ax25_TX_c_string(latitude, 8);

    ax25_TX_byte('/', false);

    ax25_TX_c_string(longitude, 9);

    ax25_TX_byte('O', false);

    ax25_TX_c_string(comment, strlen(comment));

    byte crc_lsb = continuous_crc;
    byte crc_msb = continuous_crc >> 8;
    ax25_TX_byte(crc_lsb ^ 0xFF, false);
    ax25_TX_byte(crc_msb ^ 0xFF, false);

    ax25_TX_flag(3);

    SX1278_sleep();
}

void ax25_aprs_send_status_packet(uint64_t *freq, uint8_t pwr, uint32_t deviation, const char *src_call, uint8_t src_SSID, char *status_message)
{
    SX1278_enable_TX_direct(freq, pwr, deviation);

    ax25_TX_flag(APRS_FLAGS_AT_BEGINNING);

    continuous_crc = 0xFFFF;

    char buf[8];
    sprintf(buf, "%-6s%c", "APLAIR", 0);
    for (uint8_t i = 0; i < 6; i++)
        ax25_TX_byte(buf[i] << 1, false);
    ax25_TX_byte((buf[6] << 1), false);

    sprintf(buf, "%-6s%c", src_call, src_SSID);
    for (uint8_t i = 0; i < 6; i++)
        ax25_TX_byte(buf[i] << 1, false);
    ax25_TX_byte((buf[6] << 1), false);

#ifdef CONFIG_NOHUB
    sprintf(buf, "%-6s%c", "NOHUB", 0);
    for (uint8_t i = 0; i < 6; i++)
        ax25_TX_byte(buf[i] << 1, false);
    ax25_TX_byte((buf[6] << 1) | 1, false);
#else
    ax25_TX_byte((buf[6] << 1) | 1, true);
#endif

    ax25_TX_byte(0x03, false);
    ax25_TX_byte(0xf0, false);

    ax25_TX_byte('>', false);
    ax25_TX_c_string(status_message, strlen(status_message));

    byte crc_lsb = continuous_crc;
    byte crc_msb = continuous_crc >> 8;
    ax25_TX_byte(crc_lsb ^ 0xFF, false);
    ax25_TX_byte(crc_msb ^ 0xFF, false);

    ax25_TX_flag(3);

    SX1278_sleep();
}