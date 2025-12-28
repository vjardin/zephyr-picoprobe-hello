# Zephyr Hello World for Raspberry Pi Debug Probe

[![Build](https://github.com/vjardin/zephyr-picoprobe-hello/actions/workflows/build.yml/badge.svg)](https://github.com/vjardin/zephyr-picoprobe-hello/actions/workflows/build.yml)

A Zephyr OS application that runs on the Raspberry Pi Debug Probe hardware.
It provides a Zephyr shell over USB CDC and can output "Bonjour" messages on
the UART J2 connector. Three LEDs blink on/off while two debug LEDs display
a smooth breathing effect with adjustable brightness.

## Features

- Zephyr shell over USB CDC (/dev/ttyACM0)
- Comprehensive shell commands for hardware debugging
- Bonjour message output on UART1 (J2 connector), controllable via shell
- BOOTSEL button detection with firmware update reminder
- GPIO LEDs blinking (D1, D2, D3) and PWM LEDs with breathing effect (D4, D5)
- Shell commands for LED brightness control
- Watchdog timer with 5 second timeout
- Internal temperature sensor (ADC channel 4)
- Runtime log level control

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

The Debug Probe has five LEDs controlled by the firmware. No external wiring
is required. Three LEDs use GPIO (on/off toggle), two use PWM (brightness
control with breathing effect).

    LED   Color    GPIO   Control    Location
    ---   ------   ----   -------    --------
    D1    Red       2     GPIO       Status
    D2    Green     7     GPIO       UART side
    D3    Yellow    8     GPIO       UART side
    D4    Green    15     PWM        DEBUG side
    D5    Yellow   16     PWM        DEBUG side

GPIO LEDs (D1, D2, D3) toggle on/off once per second. PWM LEDs (D4, D5)
display a smooth breathing effect with adjustable brightness via shell
commands.

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

### Application Commands

    bonjour on          Enable Bonjour message on UART1 (J2)
    bonjour off         Disable Bonjour message

### LED Commands

    led status                    Show LED status and brightness levels
    led brightness <led> <0-100>  Set PWM LED brightness (0=D4, 1=D5)
    led breathing [on|off]        Enable/disable breathing effect

Setting brightness manually disables the breathing effect. Examples:

    debug-probe:~$ led status
    GPIO LEDs: D1 (red), D2 (green), D3 (yellow) - toggling
    PWM LEDs:
      D4 (green debug): 50%
      D5 (yellow debug): 50%
    Breathing: enabled

    debug-probe:~$ led brightness 0 75
    D4 brightness set to 75% (breathing disabled)

    debug-probe:~$ led breathing on
    Breathing enabled

### Kernel Commands

    kernel version      Show Zephyr version
    kernel threads      List all threads
    kernel thread list  Detailed thread information
    kernel uptime       Show system uptime
    kernel reboot       Reboot options (warm/cold)
    kernel stacks       Show thread stack usage

### Device Commands

    device list         List all devices and their status

### GPIO Commands

    gpio conf <device> <pin> <mode>   Configure pin (in, out, od)
    gpio get <device> <pin>           Read pin state
    gpio set <device> <pin> <value>   Set pin high (1) or low (0)
    gpio toggle <device> <pin>        Toggle pin state
    gpio blink <device> <pin>         Blink pin
    gpio info <device> <pin>          Show pin configuration

### Hardware Info Commands

    hwinfo devid        Show unique device ID
    hwinfo reset_cause  Show last reset reason

### Flash Commands

    flash read <device> <offset> <len>   Read flash contents
    flash page_info <device> <offset>    Show flash page info
    flash erase <device> <offset> <len>  Erase flash region
    flash write <device> <offset> <data> Write to flash

### USB Device Commands

    usbd defaults       Initialize USB with defaults
    usbd enable         Enable USB device
    usbd disable        Disable USB device
    usbd device         Show device info
    usbd config         Show configuration
    usbd wakeup         Trigger remote wakeup

### Log Commands

    log status          Show all log module levels
    log enable <module> <level>    Set module log level
    log disable <module>           Disable module logging
    log go              Resume logging
    log halt            Pause logging

Log levels: 0=off, 1=err, 2=wrn, 3=inf, 4=dbg

### CRC Commands

    crc8 <data>         Calculate CRC-8
    crc16 <data>        Calculate CRC-16
    crc32 <data>        Calculate CRC-32

### PWM Commands

    pwm cycles <device> <channel> <period> <pulse>   Set PWM in cycles
    pwm usec <device> <channel> <period> <pulse>     Set PWM in microseconds
    pwm nsec <device> <channel> <period> <pulse>     Set PWM in nanoseconds

### ADC Commands (Internal Temperature)

    adc <device> acq_time <ch> default   Set acquisition time
    adc <device> resolution <bits>       Set resolution (12-bit max)
    adc <device> read <channel>          Read ADC value

To read the internal temperature sensor (channel 4):

    debug-probe:~$ adc adc@4004c000 acq_time 0 default
    debug-probe:~$ adc adc@4004c000 resolution 12
    debug-probe:~$ adc adc@4004c000 read 4
    read: 747

Convert raw ADC value to temperature:

    Voltage = raw x 3.3 / 4096
    Temperature (°C) = 27 - (Voltage - 0.706) / 0.001721

Example: raw=747 → V=0.601V → T ~ 88°C (note: varies by chip calibration, TODO)

### I2C Commands

    i2c scan <device>                 Scan for devices on bus
    i2c read <device> <addr> <reg>    Read from device
    i2c read_byte <device> <addr>     Read single byte
    i2c write <device> <addr> <data>  Write to device
    i2c recover <device>              Recover bus from stuck state

Note: The Debug Probe hardware does not expose I2C pins on external connectors.
The I2C shell is included for completeness but cannot be used without hardware
modification. The default I2C0 pins (GPIO4/GPIO5) are used by UART1 (J2 connector).

### Watchdog Commands

    wdt setup <device> <options>      Configure watchdog
    wdt feed <device> <channel>       Feed watchdog

Note: The application automatically feeds the watchdog every second.

### POSIX Commands

    posix uname -a      Show system information (sysname, nodename, etc.)

### Memory Commands

    devmem dump <addr> <len>    Dump memory contents
    devmem load <addr> <data>   Write to memory address

### Other Commands

    date                Show/set system date
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

2. LEDs D1, D2, D3 should blink on/off every second. LEDs D4 and D5
   should display a smooth breathing effect (fade in/out).

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
    |  |- main.c                Main application (BOOTSEL, Bonjour)
    |  |- leds.c                LED management (GPIO and PWM)
    |  |- leds.h                LED API declarations
    |  |- shell_cmds.c          Shell command implementations
    |  |- watchdog.c            Watchdog management
    |  |- watchdog.h            Watchdog API declarations
    |- docs/
    |  |- hardware-summary.md   Hardware pinout reference
    |  |- raspberry-pi-debug-probe-schematics.pdf

## Device Tree Overlay

The boards/rpi_pico.overlay file configures:

- USB CDC ACM as the console and shell interface
- UART1 (GPIO4=TX, GPIO5=RX) for Bonjour output on J2 connector
- GPIO LEDs (D1, D2, D3) with gpio-leds compatible
- PWM LEDs (D4, D5) with pwm-leds compatible for brightness control
- PWM pinctrl routing slice 7B to GPIO15 and slice 0A to GPIO16
- ADC for internal temperature sensor (channel 4)
- Watchdog timer with debug halt pause

This is necessary because the Debug Probe uses different pins than the standard
Raspberry Pi Pico board definition in Zephyr.

## License

SPDX-License-Identifier: AGPL-3.0-or-later
Copyright (c) 2025 Vincent Jardin
