#ifndef PINOUT_H_
#define PINOUT_H_

#ifdef STM32F1

#define GPS_RX PA10
#define GPS_TX PA9 // Used also as DEBUG
#define GPS_EN PB7

#define SX1278_RESET PA3
#define SX1278_NSS PA4
#define SX1278_SCK PA5
#define SX1278_MISO PA6
#define SX1278_MOSI PA7

#define SX1278_DIO2 PB13
#define SX1278_DIO1 PB14
#define SX1278_DIO0 PB15

#endif

#ifdef STM32L0

#define GPS_RX PA10
#define GPS_TX PA9 // Used also as DEBUG
#define GPS_EN PB7

#define SX1278_RESET PA3
#define SX1278_NSS PA4
#define SX1278_SCK PA5
#define SX1278_MISO PA6
#define SX1278_MOSI PA7

#define SX1278_DIO2 PB0
#define SX1278_DIO1 PB1
#define SX1278_DIO0 PB2

#endif

#endif