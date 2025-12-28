# Raspberry Pi Debug Probe - Hardware Summary

Source: [Official Schematic (PDF)](https://datasheets.raspberrypi.com/debug/raspberry-pi-debug-probe-schematics.pdf)

## Overview

- **MCU**: RP2040 (U1)
- **Flash**: W25Q16JVUXIQ - 16Mbit/2MB QSPI Flash (U3)
- **Crystal**: 12MHz (X1)
- **Regulator**: AP2112K-3.3TRG1 - 3.3V LDO (U2)
- **USB**: Micro-B connector (J1 - 690-005-298-486)

## Connectors

### J2 - UART Connector (SM03B-SRSS-TB - 3-pin JST)

| Pin | Signal | GPIO | Notes |
|-----|--------|------|-------|
| 1   | TX     | GPIO4 | UART1 TX, 100R series resistor (R4) |
| 2   | GND    | -    | Ground |
| 3   | RX     | GPIO5 | UART1 RX via buffer U4 (74AUP1T17GW), 100R (R6) |

Note: GPIO6 also connected to RX line (directly through R5 100R)

### J3 - DEBUG/SWD Connector (SM03B-SRSS-TB - 3-pin JST)

| Pin | Signal | GPIO | Notes |
|-----|--------|------|-------|
| 1   | SWCLK  | GPIO12 | Clock, 100R series resistor (R11) |
| 2   | GND    | -     | Ground |
| 3   | SWDIO  | GPIO13 | Data via buffer U5, 100R (R12). GPIO14 direct |

### J4 - Auxiliary GPIO Header (THS-03-R - unpopulated)

| Pin | Signal |
|-----|--------|
| 1   | GPIO0  |
| 2   | GPIO1  |
| 3   | GND    |

## LEDs

| LED | Color  | GPIO   | Location |
|-----|--------|--------|----------|
| D1  | Red    | GPIO2  | Power/Status |
| D2  | Green  | GPIO7  | UART side |
| D3  | Yellow | GPIO8  | UART side |
| D4  | Green  | GPIO15 | DEBUG side |
| D5  | Yellow | GPIO16 | DEBUG side |

All LEDs have 470R current limiting resistors.

## BOOTSEL Button (SW1)

- Connected between QSPI_SS and GND
- Directly bridges QSPI chip select to force USB boot mode
- Part: TP-1221U-K9K5325

## Key Differences from Standard Pico

| Feature | Standard Pico | Debug Probe |
|---------|---------------|-------------|
| UART Console | UART0 (GPIO0/1) | UART1 (GPIO4/5) |
| USB | Micro-B | Micro-B |
| Flash | 2MB | 2MB |
| User GPIO | 26 pins | Limited (6 exposed) |

## Zephyr Board Target

For Zephyr OS, use: `rpi_pico` with overlay for UART1, or check for `rpi_debug_probe` board support.

## References

- [Official Documentation](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html)
- [Schematic PDF](https://datasheets.raspberrypi.com/debug/raspberry-pi-debug-probe-schematics.pdf)
- [Debugprobe Firmware](https://github.com/raspberrypi/debugprobe)
- [Zephyr Board Docs](https://docs.zephyrproject.org/latest/boards/raspberrypi/rpi_debug_probe/doc/index.html)
