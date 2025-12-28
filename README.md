# Zephyr Hello World for Raspberry Pi Debug Probe

A Zephyr OS application that runs on the Raspberry Pi Debug Probe hardware.
It provides a Zephyr shell over USB CDC and can output "Bonjour" messages on
the UART J2 connector. All five LEDs blink together once per second.

## Features

- Zephyr shell over USB CDC (/dev/ttyACM0)
- Bonjour message output on UART1 (J2 connector), controllable via shell
- BOOTSEL button detection with firmware update reminder
- All five LEDs blinking

## Hardware

This application targets the Raspberry Pi Debug Probe, which is based on the
RP2040 microcontroller. The Debug Probe has different GPIO assignments than
a standard Raspberry Pi Pico.

### USB Interface

The Debug Probe connects via USB and appears as a CDC ACM serial device:

    /dev/ttyACM0    (Linux)

This is used for the Zephyr shell. No additional hardware is required.

### USB Devices (lsusb)

With the Debug Probe and an FTDI adapter connected:

    $ lsusb | grep -E "2e8a|0403"
    Bus 001 Device 091: ID 2e8a:000a RPi-vjardin Debug Probe Shell Demo
    Bus 001 Device 084: ID 0403:6001 Future Technology Devices Intl FT232 Serial

The Debug Probe (VID 2e8a) provides the shell, while the FTDI adapter (VID 0403)
receives Bonjour messages from the J2 UART connector.

### UART Connector (J2)

The J2 connector is a 3-pin JST (SM03B-SRSS-TB) connector used for Bonjour
message output:

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

Looking at the J2/UART connector from the board edge, the wiring is:

    J2 Pin 1 (TX)  ----> Orange wire (adapter RX)
    J2 Pin 2 (GND) ----> White wire  (adapter GND)
    J2 Pin 3 (RX)  ----> Yellow wire (adapter TX) - optional

Only the orange and white wires are needed to receive the "Bonjour" messages.
On Linux, the FTDI adapter typically appears as /dev/ttyUSB0.

### LEDs

The Debug Probe has five LEDs, all controlled by the firmware. No external
wiring is required. All LEDs blink together once per second.

    LED   Color    GPIO   Location
    ---   ------   ----   --------
    D1    Red       2     Status
    D2    Green     7     UART side
    D3    Yellow    8     UART side
    D4    Green    15     DEBUG side
    D5    Yellow   16     DEBUG side

### BOOTSEL Button

The BOOTSEL button can be detected by the firmware while running. When pressed,
the application displays on the shell:

    BOOTSEL pressed, unplug/plug USB to flash a new firmware

This reminds the user how to enter bootloader mode for firmware updates.

Note: Reading the BOOTSEL button requires special handling because it is wired
to the QSPI flash chip select (QSPI_SS). The firmware temporarily disables
flash access from a RAM-resident function to safely read the button state.

## Shell Commands

Connect to the shell via USB:

    picocom -b 115200 /dev/ttyACM0

Available commands:

    bonjour on          Enable Bonjour message on UART1 (J2)
    bonjour off         Disable Bonjour message
    kernel version      Show Zephyr version
    kernel threads      List all threads
    kernel thread list  Detailed thread information
    device list         List all devices
    reboot              Reboot the device
    help                Show all available commands

## Architecture

The application uses Zephyr's multi-threaded RTOS architecture:

    Thread          Priority   Purpose
    ------          --------   -------
    usbd            -8         USB device stack
    usbd@50110000   -8         USB controller driver
    sysworkq        -1         System work queue
    main             0         Main loop (LED toggle, BOOTSEL check)
    shell_uart      14         Shell command processing
    idle            15         Idle thread

The shell and USB communication run in dedicated threads, allowing the main
loop to focus on LED blinking and button detection. USB events are handled
via interrupts and processed through the system work queue.

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
   see all five LEDs blinking.

### Method 2: Using picotool

If you have picotool installed:

    picotool load build/zephyr/zephyr.uf2
    picotool reboot

## Testing

1. Connect to the shell via USB:

       picocom -b 115200 /dev/ttyACM0

2. All five LEDs on the Debug Probe should blink every second.

3. Enable Bonjour messages:

       debug-probe:~$ bonjour on
       Bonjour message enabled

4. Connect a second terminal to the J2 UART (via FTDI adapter):

       picocom -b 115200 /dev/ttyUSB0

   You should see "Bonjour" printed every second.

5. Press and hold the BOOTSEL button - you should see the firmware update
   message on the shell instead of Bonjour on UART.

6. Disable Bonjour:

       debug-probe:~$ bonjour off
       Bonjour message disabled

## Project Structure

    zephyr-picoprobe-hello/
    |- CMakeLists.txt           Build configuration
    |- prj.conf                 Zephyr kernel configuration
    |- boards/
    |  |- rpi_pico.overlay      Device tree overlay for Debug Probe
    |- src/
    |  |- main.c                Main application (LEDs, BOOTSEL, Bonjour)
    |  |- shell_cmds.c          Shell command implementations
    |- docs/
    |  |- hardware-summary.md   Hardware pinout reference
    |  |- raspberry-pi-debug-probe-schematics.pdf

## Device Tree Overlay

The boards/rpi_pico.overlay file configures:

- USB CDC ACM as the console and shell interface
- UART1 (GPIO4=TX, GPIO5=RX) for Bonjour output on J2 connector
- All five LEDs with descriptive aliases (led_red, led_green_uart, etc.)

This is necessary because the Debug Probe uses different pins than the standard
Raspberry Pi Pico board definition in Zephyr.

## License

SPDX-License-Identifier: AGPL-3.0-or-later
Copyright (c) 2025 Vincent Jardin
