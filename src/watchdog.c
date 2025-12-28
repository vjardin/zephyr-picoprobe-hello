/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (c) 2025 Vincent Jardin
 *
 * Watchdog management for Debug Probe
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/watchdog.h>

#include "watchdog.h"

/* Watchdog timeout in milliseconds */
#define WDT_TIMEOUT_MS 5000

/* Watchdog device */
static const struct device *const wdt_dev = DEVICE_DT_GET(DT_NODELABEL(wdt0));
static int wdt_channel_id = -1;

/**
 * Initialize the watchdog with a 5 second timeout.
 * The watchdog will reset the system if not fed within the timeout.
 *
 * @return 0 on success, negative error code on failure
 */
int watchdog_init(void)
{
	int ret;

	if (!device_is_ready(wdt_dev)) {
		printk("Watchdog device not ready\n");
		return -ENODEV;
	}

	struct wdt_timeout_cfg wdt_cfg = {
		.window.min = 0,
		.window.max = WDT_TIMEOUT_MS,
		.callback = NULL,
		.flags = WDT_FLAG_RESET_SOC,
	};

	wdt_channel_id = wdt_install_timeout(wdt_dev, &wdt_cfg);
	if (wdt_channel_id < 0) {
		printk("Watchdog install failed: %d\n", wdt_channel_id);
		return wdt_channel_id;
	}

	ret = wdt_setup(wdt_dev, WDT_OPT_PAUSE_HALTED_BY_DBG);
	if (ret != 0) {
		printk("Watchdog setup failed: %d\n", ret);
		wdt_channel_id = -1;
		return ret;
	}

	printk("Watchdog started (%d ms timeout)\n", WDT_TIMEOUT_MS);
	return 0;
}

/**
 * Feed the watchdog to prevent system reset.
 * Should be called periodically from the main loop.
 */
void watchdog_feed(void)
{
	if (wdt_channel_id >= 0) {
		wdt_feed(wdt_dev, wdt_channel_id);
	}
}
