# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2025 Vincent Jardin

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_bonjour)

target_sources(app PRIVATE
    src/main.c
    src/shell_cmds.c
)
