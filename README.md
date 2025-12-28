# Zephyr Hello World for Raspberry Pi Debug Probe

A simple Zephyr OS application that runs on the Raspberry Pi Debug Probe hardware.
It prints "Bonjour" every second on the UART and blinks the red LED.

## Hardware

This application targets the Raspberry Pi Debug Probe, which is based on the RP2040
microcontroller. The Debug Probe has different GPIO assignments than a standard
Raspberry Pi Pico.

### UART Connector (J2)

The J2 connector is a 3-pin JST (SM03B-SRSS-TB) connector:

    +-------+--------+-------+
    | Pin 1 | Pin 2  | Pin 3 |
    |  TX   |  GND   |  RX   |
    +-------+--------+-------+

Connect a USB-to-serial adapter (3.3V logic level) as follows:

    Debug Probe J2          USB-Serial Adapter
    --------------          ------------------
    Pin 1 (TX)    --------> RX (see "Bonjour")
    Pin 2 (GND)   --------> GND
    Pin 3 (RX)    --------> TX (optional, not used by this app)

Serial settings: 115200 baud, 8N1.

### FTDI USB-to-Serial Cable Wiring

Tested with an FTDI FT232 adapter. Make sure the voltage jumper is set to 3.3V.
A compatible adapter can be purchased at:
https://fr.aliexpress.com/item/1005008296799409.html (select "USB to TTL (D)").

    lsusb output:
    ID 0403:6001 Future Technology Devices International, Ltd
    FT232 Serial (UART) IC

Looking at the J2/UART connector from the board edge, the wiring is (left to right):

    J2 Pin 1 (TX)  ----> Orange wire (adapter RX)
    J2 Pin 2 (GND) ----> White wire  (adapter GND)
    J2 Pin 3 (RX)  ----> Yellow wire (adapter TX) - optional

Only the orange and white wires are needed to receive the "Bonjour" messages.
The yellow wire is only required if you need to send data to the Debug Probe.

On Linux, the FTDI adapter typically appears as /dev/ttyUSB0.

### Red LED (D1)

The red LED is directly controlled by the firmware on GPIO2. No external wiring
is required. It blinks once per second.

## Prerequisites

You need a working Zephyr development environment:

- Zephyr SDK (tested with 0.17.4)
- West build tool
- Python virtual environment with Zephyr dependencies

## Building

1. Activate the Zephyr environment:

       cd /path/to/zephyr-workspace
       source .venv/bin/activate
       export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
       export ZEPHYR_SDK_INSTALL_DIR=~/toolchains/zephyr-sdk
       export ZEPHYR_BASE=/path/to/zephyr

2. Navigate to this project directory:

       cd /path/to/zephyr-picoprobe-hello

3. Build the firmware:

       west build -b rpi_pico

   For a clean rebuild:

       west build -b rpi_pico --pristine

4. The output files are in build/zephyr/:

   - zephyr.uf2  : UF2 file for drag-and-drop flashing
   - zephyr.bin  : Raw binary
   - zephyr.elf  : ELF with debug symbols

## Flashing

### Method 1: UF2 Drag-and-Drop

1. Hold the BOOTSEL button on the Debug Probe
2. Connect the Debug Probe to your computer via USB
3. Release BOOTSEL - a drive named "RPI-RP2" should appear
4. Copy the UF2 file:

       cp build/zephyr/zephyr.uf2 /media/$USER/RPI-RP2/

   The device will automatically reboot and start running. You'll
   see the red LED blinking.

### Method 2: Using picotool

If you have picotool installed:

    picotool load build/zephyr/zephyr.uf2
    picotool reboot

## Testing

1. Connect a serial terminal to the USB-serial adapter:

       picocom -b 115200 /dev/ttyUSB0

   or:

       minicom -b 115200 -D /dev/ttyUSB0

2. You should see "Bonjour" printed every second.

3. The red LED (D1) on the Debug Probe should blink every second.

## Project Structure

    zephyr-picoprobe-hello/
    |- CMakeLists.txt           Build configuration
    |- prj.conf                 Zephyr kernel configuration
    |- boards/
    |  |- rpi_pico.overlay      Device tree overlay for Debug Probe
    |- src/
    |  |- main.c                Application source code
    |- docs/
    |  |- hardware-summary.md   Hardware pinout reference
    |  |- raspberry-pi-debug-probe-schematics.pdf

## Device Tree Overlay

The boards/rpi_pico.overlay file configures:

- UART1 as the console (GPIO4=TX, GPIO5=RX) matching the J2/UART connector
- Red LED on GPIO2 as an alias "led0"

This is necessary because the Debug Probe uses different pins than the standard
Raspberry Pi Pico board definition in Zephyr.

## License

SPDX-License-Identifier: AGPL-3.0-or-later
Copyright (c) 2025 Vincent Jardin
