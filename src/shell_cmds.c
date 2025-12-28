/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (c) 2025 Vincent Jardin
 */

#include <zephyr/shell/shell.h>

/* Bonjour message state - can be toggled via shell */
bool bonjour_enabled;

static int cmd_bonjour_on(const struct shell *sh, size_t argc, char **argv)
{
	bonjour_enabled = true;
	shell_print(sh, "Bonjour message enabled");
	return 0;
}

static int cmd_bonjour_off(const struct shell *sh, size_t argc, char **argv)
{
	bonjour_enabled = false;
	shell_print(sh, "Bonjour message disabled");
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_bonjour,
	SHELL_CMD(on, NULL, "Enable Bonjour message", cmd_bonjour_on),
	SHELL_CMD(off, NULL, "Disable Bonjour message", cmd_bonjour_off),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(bonjour, &sub_bonjour, "Bonjour message control", NULL);
