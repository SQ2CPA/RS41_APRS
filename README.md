# Software for APRS Tracker based on components from RS41 radiosonde

# WORK IN PROGRESS! NOT TESTED YET!

## Check my parts list [here](https://github.com/SQ2CPA/parts)

The goal of this project is to show that you can use some components from RS41 radiosonde in your own trackers.
You can also check WSPR version [here](https://github.com/SQ2CPA/RS41_WSPR).

## Used components from RS41

1. STM32F100C8
2. 24 MHz crystal

## Available modulactions with QRG

-   AFSK APRS ([frequency list here](https://github.com/SQ2CPA/RS41_APRS))
-   LoRa APRS ([frequency list here](https://github.com/SQ2CPA/RS41_APRS))

Software is doing TX of both modulations one by one without delay!

## Unique features

-   Disable GPS when doing APRS TX
-   Geofancing for AFSK APRS, LoRa QRG rotation every TX
-   Historical location (not available yet)
-   Day of the flight (not available yet)

## Power usage

Unknown for now

## Pinouts

Pins definitions availabe at `./src/pinout.h`

1. GPS TX at PA10, RX at PA9, ON/OFF at PB7 (GPS TX could be also used as serial debug)
2. RFM98 at SPI: RESET at PA3, NSS at PA4, SCK at PA5, MISO at PA6, MOSI at PA7, DIO2 at PA13, DIO1 at PA14, DIO0 at PA15
3. SWDIO at PA13, CLK at PA14

PCB is currently unavailable to public!

More information should available soon.

## Build

build via platformio

## Flash

via openocd

unlock first time by `./unlock.bat`
flash `./flash.bat`

or you can flash `./.pio/build/stm32f100c8t6/firmware.elf` via your favorite flasher

# 73, Damian SQ2CPA, Poland
