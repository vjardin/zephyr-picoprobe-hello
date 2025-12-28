/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (c) 2025 Vincent Jardin
 *
 * LED management for Debug Probe
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>

#include "leds.h"

/* GPIO-controlled LEDs (accent LEDs: D1, D2, D3) */
#define NUM_GPIO_LEDS 3

static const struct gpio_dt_spec gpio_leds[NUM_GPIO_LEDS] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led_green_uart), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led_yellow_uart), gpios),
};

/* PWM-controlled LEDs (debug LEDs: D4, D5) with brightness control */
#define NUM_PWM_LEDS 2

static const struct pwm_dt_spec pwm_leds[NUM_PWM_LEDS] = {
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0)),
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1)),
};

/* PWM brightness state */
static uint32_t pwm_brightness[NUM_PWM_LEDS] = {0, 0};
static uint32_t breathing_brightness = 0;
static bool breathing_increasing = true;
static bool breathing_enabled = true;

#define PWM_STEP 5
#define PWM_MAX 100

int leds_init(void)
{
	int ret;

	/* Initialize GPIO LEDs (D1, D2, D3) */
	for (int i = 0; i < NUM_GPIO_LEDS; i++) {
		if (!gpio_is_ready_dt(&gpio_leds[i])) {
			printk("GPIO LED%d not ready\n", i);
			return -ENODEV;
		}

		ret = gpio_pin_configure_dt(&gpio_leds[i], GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			printk("Failed to configure GPIO LED%d: %d\n", i, ret);
			return ret;
		}
	}

	/* Initialize PWM LEDs (D4, D5) */
	for (int i = 0; i < NUM_PWM_LEDS; i++) {
		if (!pwm_is_ready_dt(&pwm_leds[i])) {
			printk("PWM LED%d not ready\n", i);
			return -ENODEV;
		}
	}

	printk("LEDs initialized (3 GPIO + 2 PWM)\n");
	return 0;
}

void leds_gpio_toggle(void)
{
	for (int i = 0; i < NUM_GPIO_LEDS; i++) {
		gpio_pin_toggle_dt(&gpio_leds[i]);
	}
}

void leds_pwm_update(void)
{
	if (breathing_enabled) {
		/* Update breathing brightness */
		if (breathing_increasing) {
			breathing_brightness += PWM_STEP;
			if (breathing_brightness >= PWM_MAX) {
				breathing_brightness = PWM_MAX;
				breathing_increasing = false;
			}
		} else {
			breathing_brightness -= PWM_STEP;
			if (breathing_brightness == 0) {
				breathing_increasing = true;
			}
		}

		/* Apply breathing to both PWM LEDs */
		for (int i = 0; i < NUM_PWM_LEDS; i++) {
			pwm_brightness[i] = breathing_brightness;
			uint32_t pulse = (pwm_leds[i].period * breathing_brightness) / PWM_MAX;
			pwm_set_pulse_dt(&pwm_leds[i], pulse);
		}
	}
}

int leds_pwm_set_brightness(int led, uint32_t brightness)
{
	if (led < 0 || led >= NUM_PWM_LEDS) {
		return -EINVAL;
	}

	if (brightness > PWM_MAX) {
		brightness = PWM_MAX;
	}

	pwm_brightness[led] = brightness;
	uint32_t pulse = (pwm_leds[led].period * brightness) / PWM_MAX;
	return pwm_set_pulse_dt(&pwm_leds[led], pulse);
}

int leds_pwm_get_brightness(int led)
{
	if (led < 0 || led >= NUM_PWM_LEDS) {
		return -EINVAL;
	}

	return pwm_brightness[led];
}

void leds_pwm_set_breathing(bool enable)
{
	breathing_enabled = enable;
}

bool leds_pwm_get_breathing(void)
{
	return breathing_enabled;
}

/* Shell commands */

static int cmd_led_status(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "GPIO LEDs: D1 (red), D2 (green), D3 (yellow) - toggling");
	shell_print(sh, "PWM LEDs:");
	shell_print(sh, "  D4 (green debug): %d%%", pwm_brightness[0]);
	shell_print(sh, "  D5 (yellow debug): %d%%", pwm_brightness[1]);
	shell_print(sh, "Breathing: %s", breathing_enabled ? "enabled" : "disabled");

	return 0;
}

static int cmd_led_brightness(const struct shell *sh, size_t argc, char **argv)
{
	if (argc < 3) {
		shell_error(sh, "Usage: led brightness <led> <0-100>");
		shell_print(sh, "  led: 0 = D4 (green), 1 = D5 (yellow)");
		return -EINVAL;
	}

	int led = atoi(argv[1]);
	int brightness = atoi(argv[2]);

	if (led < 0 || led > 1) {
		shell_error(sh, "Invalid LED: %d (use 0 or 1)", led);
		return -EINVAL;
	}

	if (brightness < 0 || brightness > 100) {
		shell_error(sh, "Invalid brightness: %d (use 0-100)", brightness);
		return -EINVAL;
	}

	/* Disable breathing when manually setting brightness */
	breathing_enabled = false;

	int ret = leds_pwm_set_brightness(led, brightness);
	if (ret < 0) {
		shell_error(sh, "Failed to set brightness: %d", ret);
		return ret;
	}

	shell_print(sh, "D%d brightness set to %d%% (breathing disabled)",
		    led == 0 ? 4 : 5, brightness);

	return 0;
}

static int cmd_led_breathing(const struct shell *sh, size_t argc, char **argv)
{
	if (argc < 2) {
		shell_print(sh, "Breathing: %s", breathing_enabled ? "enabled" : "disabled");
		return 0;
	}

	if (strcmp(argv[1], "on") == 0) {
		breathing_enabled = true;
		shell_print(sh, "Breathing enabled");
	} else if (strcmp(argv[1], "off") == 0) {
		breathing_enabled = false;
		shell_print(sh, "Breathing disabled");
	} else {
		shell_error(sh, "Usage: led breathing [on|off]");
		return -EINVAL;
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_led,
	SHELL_CMD(status, NULL, "Show LED status", cmd_led_status),
	SHELL_CMD(brightness, NULL, "Set PWM LED brightness: led brightness <0|1> <0-100>",
		  cmd_led_brightness),
	SHELL_CMD(breathing, NULL, "Control breathing effect: led breathing [on|off]",
		  cmd_led_breathing),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(led, &sub_led, "LED control commands", NULL);
