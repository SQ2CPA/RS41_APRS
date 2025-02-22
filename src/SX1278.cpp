#include "SX1278.h"
#include "SX1278_registers.h"
#include <SPI.h>
#include "pinout.h"
#include "configuration.h"

#if SX1278_MOD_OPTION == MOD_F_HOP
uint8_t freq_lsb_lo;
uint8_t freq_lsb_hi;
#endif

static void SX1278_write_reg(uint8_t address, uint8_t reg_value)
{
    digitalWrite(SX1278_NSS, LOW);
    SPI.transfer(address | 0x80);
    SPI.transfer(reg_value);
    digitalWrite(SX1278_NSS, HIGH);
}

static uint8_t SX1278_read_reg(uint8_t address)
{
    digitalWrite(SX1278_NSS, LOW);
    SPI.transfer(address & 0x7F);
    uint8_t reg_value = SPI.transfer(0);
    digitalWrite(SX1278_NSS, HIGH);

    return reg_value;
}

void SX1278_begin(void)
{
    SPI.begin();

    pinMode(SX1278_NSS, OUTPUT);
    digitalWrite(SX1278_NSS, HIGH);

    pinMode(SX1278_RESET, OUTPUT);
    SX1278_enable();

    delay(10);

    SX1278_write_reg(REG_TCXO, 0x10);

    delay(10);

    pinMode(SX1278_DIO2, OUTPUT);
}

void SX1278_enable(void)
{
    digitalWrite(SX1278_RESET, HIGH);
}

void SX1278_disable(void)
{
    digitalWrite(SX1278_RESET, LOW);
}

void SX1278_sleep(void)
{
    // Set SX1278 operating mode to sleep mode
    // 7: 0->FSK/OOK Mode
    // 6-5: 00->FSK
    // 4: 0 (reserved)
    // 3: 1->Low Frequency Mode
    // 2-0: 000->Sleep Mode
    SX1278_write_reg(REG_OP_MODE, 0x08);
}

void SX1278_reset(void)
{
    digitalWrite(SX1278_RESET, LOW);
    delay(5);
    digitalWrite(SX1278_RESET, HIGH);
    delay(5);
}

// AFSK Mode

void SX1278_enable_TX_direct(uint64_t *freq, uint8_t pwr, uint16_t deviation)
{
    SX1278_set_TX_frequency(freq);

    // Set packet mode config
    // 7: unused
    // 6: Data Mode 0->Continuos mode
    // 5: Io Home On 0->off
    // 4: Io Home Pwr Frame 0->off
    // 3: Beacon On 0->off
    // 2-0: Payload length MSB
    SX1278_write_reg(REG_PACKET_CONFIG_2, 0x00);

    // Set SX1278 operating mode
    // 7: 0->FSK/OOK Mode
    // 6-5: 00->FSK
    // 4: 0 (reserved)
    // 3: 1->Low Frequency Mode
    // 2-0: 011->Transmitter Mode (TX)
    SX1278_write_reg(REG_OP_MODE, 0x0B);

#if SX1278_MOD_OPTION == MOD_F_HOP
    SX1278_write_reg(REG_PLL_HOP, 0xAD);
#endif

    if (pwr <= 15)
        SX1278_set_TX_power(pwr, false);
    else
        SX1278_set_TX_power(pwr, true);

    SX1278_write_reg(REG_TCXO, 0x10);

    SX1278_set_TX_deviation(freq, deviation);
}

void SX1278_mod_direct_out(uint32_t delay)
{
#if SX1278_MOD_OPTION == MOD_DIO2
    digitalWrite(SX1278_DIO2, HIGH);
    delayMicroseconds(delay);
    digitalWrite(SX1278_DIO2, LOW);
    delayMicroseconds(delay);
#elif SX1278_MOD_OPTION == MOD_F_HOP
    SX1278_write_reg(REG_FR_LSB, freq_lsb_lo);
    delayMicroseconds(delay);
    SX1278_write_reg(REG_FR_LSB, freq_lsb_hi);
    delayMicroseconds(delay);
#endif
}

void SX1278_set_direct__out(bool value)
{
    digitalWrite(SX1278_DIO2, value);
}

void SX1278_set_TX_deviation(uint64_t *freq, uint16_t deviation)
{
#if SX1278_MOD_OPTION == MOD_DIO2
    SX1278_write_reg(REG_FDEV_LSB, deviation / (SX1278_CRYSTAL_FREQ >> 19));
#elif SX1278_MOD_OPTION == MOD_F_HOP
    SX1278_write_reg(REG_FDEV_LSB, 0 / (SX1278_CRYSTAL_FREQ >> 19));

    // Frequency value is calculate by:
    // Freg = (Frf * 2^19) / Fxo
    // Resolution is 61.035 Hz if Fxo = 32 MHz
    // Frf_LSB for high an low FSK frequency needed for fast frequency hopping
    freq_lsb_lo = (uint32_t)((*freq + CONFIG_RADIO_OFFSET_AFSK - deviation / 2) << 19) / SX1278_CRYSTAL_FREQ; // Frf_LSB with carrier frequency + deviation
    freq_lsb_hi = (uint32_t)((*freq + CONFIG_RADIO_OFFSET_AFSK + deviation / 2) << 19) / SX1278_CRYSTAL_FREQ; // Frf_LSB with carrier frequency + deviation
#endif
}

void SX1278_set_TX_power(uint8_t pwr, bool pa_boost_mode_20dbm)
{
    uint8_t TX_out;

    if (pa_boost_mode_20dbm)
    {
        SX1278_write_reg(REG_PA_DAC, 0x87);
        TX_out = pwr - 5;
    }
    else
    {
        SX1278_write_reg(REG_PA_DAC, 0x84);
        TX_out = pwr - 2;
    }

    // PA selection and output power control
    // 7-5: PA select 0 -> RFO pin (14dbm) | 1 -> PA_BOOST pin (20dbm)
    // 6-4: Max Pwr (Pmaxreg = (Pmax - 10.8) / 0.6) -> set to max
    // 3-0: Out Pwr
    SX1278_write_reg(REG_PA_CONFIG, (0xF0 | TX_out));
}

void SX1278_set_TX_frequency(uint64_t *freq)
{
    uint64_t freq_tmp = *freq;

    freq_tmp = (((freq_tmp + 0) << 19) / SX1278_CRYSTAL_FREQ);

    SX1278_write_reg(REG_FR_MSB, freq_tmp >> 16);
    SX1278_write_reg(REG_FR_MID, freq_tmp >> 8);
    SX1278_write_reg(REG_FR_LSB, freq_tmp);
}

// LoRa Mode

static void set_LoRa_mode();
static void clear_LoRa_irq();

void SX1278_set_LoRa_parameters(uint8_t sf, uint8_t bw, uint8_t cr)
{
    uint8_t bw_bits = 0;
    switch (bw)
    {
    case 7:
        bw_bits = 0x00;
        break; // 7.8 kHz
    case 10:
        bw_bits = 0x01;
        break; // 10.4 kHz
    case 15:
        bw_bits = 0x02;
        break; // 15.6 kHz
    case 20:
        bw_bits = 0x03;
        break; // 20.8 kHz
    case 31:
        bw_bits = 0x04;
        break; // 31.25 kHz
    case 41:
        bw_bits = 0x05;
        break; // 41.7 kHz
    case 62:
        bw_bits = 0x06;
        break; // 62.5 kHz
    case 125:
        bw_bits = 0x07;
        break; // 125 kHz
    default:
        bw_bits = 0x07;
        break;
    }

    uint8_t cr_bits = (cr - 4);

    uint8_t config1 = (bw_bits << 4) | (cr_bits << 1);
    SX1278_write_reg(REG_MODEM_CONFIG1, config1);

    uint8_t config2 = (sf << 4) | 0x04;
    SX1278_write_reg(REG_MODEM_CONFIG2, config2);

    uint8_t config3 = 0x04;

    if (bw == 125 && sf >= 11)
    {
        config3 |= 0x08;
    }
    SX1278_write_reg(REG_MODEM_CONFIG3, config3);

    uint8_t preamble = 8;

    SX1278_write_reg(REG_PREAMBLE_MSB, preamble >> 8);
    SX1278_write_reg(REG_PREAMBLE_LSB, preamble & 0xFF);

    SX1278_write_reg(REG_SYNC_WORD, 0x12);
}

void SX1278_init_LoRa(uint8_t sf, uint8_t bw, uint8_t cr, uint64_t frequency)
{
    SX1278_write_reg(REG_OP_MODE, 0x00);
    delay(10);

    set_LoRa_mode();

    SX1278_set_LoRa_parameters(sf, bw, cr);

    SX1278_write_reg(REG_TCXO, 0x10);

    SX1278_write_reg(REG_DIO_MAPPING1, 0x40); // DIO0 = 01 (TxDone)

    SX1278_write_reg(REG_LNA, 0x23); // Max gain, boost enabled

    SX1278_set_TX_frequency(&frequency);
}

void SX1278_set_LoRa_power(uint8_t txPower)
{
    if (txPower > 20)
        txPower = 20;
    if (txPower < 2)
        txPower = 2;

    bool paBoost = txPower > 15;
    SX1278_set_TX_power(txPower, paBoost);
}

void SX1278_send_LoRa_packet(const uint8_t *data, size_t length)
{
    SX1278_write_reg(REG_OP_MODE, 0x81);
    delay(10);

    SX1278_write_reg(REG_FIFO_ADDR_PTR, 0x00);
    SX1278_write_reg(REG_FIFO_TX_BASE_ADDR, 0x00);

    for (size_t i = 0; i < length; i++)
    {
        SX1278_write_reg(REG_FIFO, data[i]);
    }

    SX1278_write_reg(REG_PAYLOAD_LENGTH, length);

    clear_LoRa_irq();

    SX1278_write_reg(REG_OP_MODE, 0x83);

    while (!SX1278_LoRa_transmission_done())
        delay(10);
}

bool SX1278_LoRa_transmission_done()
{
    uint8_t irqFlags = SX1278_read_reg(REG_IRQ_FLAGS);
    if (irqFlags & 0x08)
    {
        clear_LoRa_irq();
        return true;
    }
    return false;
}

static void set_LoRa_mode()
{
    SX1278_write_reg(REG_OP_MODE, 0x80);
    delay(10);

    SX1278_write_reg(REG_OP_MODE, 0x81);
    delay(10);
}

static void clear_LoRa_irq()
{
    SX1278_write_reg(REG_IRQ_FLAGS, 0xFF);
}