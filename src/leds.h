/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (c) 2025 Vincent Jardin
 *
 * LED management for Debug Probe
 */

#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>

/**
 * Initialize all LEDs (GPIO and PWM).
 *
 * @return 0 on success, negative error code on failure
 */
int leds_init(void);

/**
 * Toggle GPIO LEDs (D1, D2, D3).
 * Called periodically from main loop.
 */
void leds_gpio_toggle(void);

/**
 * Update PWM LEDs (D4, D5) brightness.
 * Called periodically from main loop for breathing effect.
 */
void leds_pwm_update(void);

/**
 * Set PWM LED brightness directly.
 *
 * @param led LED index (0 = D4 green, 1 = D5 yellow)
 * @param brightness Brightness level 0-100 (percent)
 * @return 0 on success, negative error code on failure
 */
int leds_pwm_set_brightness(int led, uint32_t brightness);

/**
 * Get current PWM LED brightness.
 *
 * @param led LED index (0 = D4 green, 1 = D5 yellow)
 * @return Brightness level 0-100, or negative error code
 */
int leds_pwm_get_brightness(int led);

/**
 * Enable or disable automatic breathing effect.
 *
 * @param enable true to enable breathing, false to disable
 */
void leds_pwm_set_breathing(bool enable);

/**
 * Check if breathing effect is enabled.
 *
 * @return true if breathing is enabled
 */
bool leds_pwm_get_breathing(void);

#endif /* LEDS_H */
