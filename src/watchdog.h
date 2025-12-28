/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (c) 2025 Vincent Jardin
 *
 * Watchdog management for Debug Probe
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

/**
 * Initialize the watchdog with a 5 second timeout.
 * The watchdog will reset the system if not fed within the timeout.
 *
 * @return 0 on success, negative error code on failure
 */
int watchdog_init(void);

/**
 * Feed the watchdog to prevent system reset.
 * Should be called periodically from the main loop.
 */
void watchdog_feed(void);

#endif /* WATCHDOG_H */
