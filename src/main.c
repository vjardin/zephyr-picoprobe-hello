/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (c) 2025 Vincent Jardin
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/irq.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/bos.h>
#include <zephyr/usb/msos_desc.h>
#include <hardware/structs/ioqspi.h>
#include <hardware/structs/sio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#include <sample_usbd.h>
#include <cmsis_dap.h>

#include "msosv2.h"
#include "webusb.h"
#include "leds.h"
#include "watchdog.h"

/* Bonjour state from shell_cmds.c */
extern bool bonjour_enabled;

/* UART1 device for Bonjour output on J2 connector */
static const struct device *const uart1_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

/* Send string to UART1 (J2 connector) */
static void uart1_print(const char *str)
{
	while (*str) {
		uart_poll_out(uart1_dev, *str++);
	}
}

/*
 * Read BOOTSEL button state.
 *
 * The BOOTSEL button is wired to QSPI_SS (flash chip select). To read it,
 * we must temporarily disable flash access. This is dangerous because the
 * CPU normally fetches instructions from flash (XIP = eXecute In Place).
 *
 * The __ramfunc attribute places this function in RAM instead of flash.
 * At startup, the code is copied from flash to RAM. When called, the CPU
 * executes from RAM, allowing us to safely disable flash during the read.
 *
 * The sequence is:
 *   1. Disable interrupts (ISRs in flash would crash if flash is disabled)
 *   2. Override QSPI_SS pin to act as GPIO input (this disables flash!)
 *   3. Short delay for the signal to settle
 *   4. Read button state from the QSPI GPIO bank
 *   5. Restore QSPI_SS to normal operation (re-enables flash)
 *   6. Re-enable interrupts
 *
 * Without __ramfunc, the CPU would crash immediately when trying to fetch
 * the next instruction from the now-disabled flash.
 */
static bool __ramfunc get_bootsel_button(void)
{
	const uint32_t CS_PIN_INDEX = 1;
	unsigned int key = irq_lock();

	/*
	 * Set QSPI_SS output override to LOW (value 2).
	 * This drives the chip select low, disabling flash, so we can
	 * read the button state on that pin. The OEOVER field is bits 13:12.
	 */
	uint32_t oeover_low = 2u << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB;
	hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
			oeover_low,
			IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

	/* Short delay for the signal to settle */
	for (volatile int i = 0; i < 1000; ++i) {
		;
	}

	/* Read the button state (pressed = low = 0) */
	bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

	/*
	 * Restore QSPI_SS output override to NORMAL (value 0).
	 * This re-enables normal flash operation.
	 */
	hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
			0u,
			IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

	irq_unlock(key);

	return button_state;
}

int main(void)
{
	int ret;
	const struct device *const console_dev =
		DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const swd_dev =
		DEVICE_DT_GET_ONE(zephyr_swdp_gpio);
	struct usbd_context *sample_usbd;
	uint32_t dtr = 0;
	bool bootsel_msg_shown = false;

	/* Initialize CMSIS-DAP with the SWD device */
	ret = dap_setup(swd_dev);
	if (ret) {
		printk("Failed to initialize DAP: %d\n", ret);
		return ret;
	}

	/* Setup USB device with all registered classes (CDC ACM + DAP) */
	sample_usbd = sample_usbd_setup_device(NULL);
	if (sample_usbd == NULL) {
		printk("Failed to setup USB device\n");
		return -ENODEV;
	}

	/* Add MSOS v2 descriptor for automatic WinUSB driver on Windows */
	ret = usbd_add_descriptor(sample_usbd, &bos_vreq_msosv2);
	if (ret) {
		printk("Failed to add MSOS v2 descriptor: %d\n", ret);
		return ret;
	}

	/* Add WebUSB descriptor for browser-based debugging tools */
	ret = usbd_add_descriptor(sample_usbd, &bos_vreq_webusb);
	if (ret) {
		printk("Failed to add WebUSB descriptor: %d\n", ret);
		return ret;
	}

	ret = usbd_init(sample_usbd);
	if (ret) {
		printk("Failed to initialize USB: %d\n", ret);
		return ret;
	}

	ret = usbd_enable(sample_usbd);
	if (ret) {
		printk("Failed to enable USB: %d\n", ret);
		return ret;
	}

	/*
	 * Wait for DTR (terminal connected) before starting.
	 * Timeout after 3 seconds to allow simulation without DTR support.
	 */
	if (device_is_ready(console_dev)) {
		int dtr_timeout = 30; /* 30 * 100ms = 3 seconds */

		while (!dtr && dtr_timeout > 0) {
			uart_line_ctrl_get(console_dev, UART_LINE_CTRL_DTR,
					   &dtr);
			k_sleep(K_MSEC(100));
			dtr_timeout--;
		}
	}

	/* Check UART1 for Bonjour output */
	if (!device_is_ready(uart1_dev)) {
		printk("UART1 device not ready\n");
	}

	printk("Starting Debug Probe LED demo...\n");
	printk("Note: I2C shell disabled - GPIO4/5 used by UART1 (J2 connector)\n");

	/* Initialize LEDs */
	ret = leds_init();
	if (ret < 0) {
		return ret;
	}

	/* Initialize watchdog */
	watchdog_init();

	uint32_t loop_count = 0;

	while (1) {
		/* Check BOOTSEL button and Bonjour ~once per second (64ms * 16 = 1024ms) */
		if ((loop_count & 0x0F) == 0) {
			if (get_bootsel_button()) {
				if (!bootsel_msg_shown) {
					printk("BOOTSEL pressed, "
					       "unplug/plug USB to flash a new firmware\n");
					bootsel_msg_shown = true;
				}
			} else {
				bootsel_msg_shown = false;
				if (bonjour_enabled) {
					uart1_print("Bonjour\r\n");
				}
			}

			/* Toggle GPIO LEDs once per second */
			leds_gpio_toggle();
		}

		/* Update PWM LEDs with breathing effect */
		leds_pwm_update();

		/* Feed the watchdog to prevent reset */
		watchdog_feed();

		loop_count++;
		k_sleep(K_MSEC(64));
	}

	return 0;
}
