/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (c) 2025 Vincent Jardin
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		printk("LED GPIO not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Failed to configure LED GPIO\n");
		return -1;
	}

	while (1) {
		printk("Bonjour\n");
		gpio_pin_toggle_dt(&led);
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
