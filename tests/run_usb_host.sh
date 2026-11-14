#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# USB HOST (firmware/src/usb_host.c) on this computer: the driver against a model of the USB controller, a keyboard,
# hubs and a stick (tests/usb_host_test.c). Twice: without and with the updater's SysEx ring (FELUCCA_OTA).
set -e
cd "$(dirname "$0")/.."
mkdir -p build/host
CC="${CC:-cc}"
$CC -std=gnu11 -O1 -g -fsanitize=address,undefined -Wall -Wno-unused-function -o build/host/usb_host_test tests/usb_host_test.c
build/host/usb_host_test
$CC -std=gnu11 -O1 -g -DFELUCCA_OTA=1 -fsanitize=address,undefined -Wall -Wno-unused-function -o build/host/usb_host_ota_test tests/usb_host_test.c
build/host/usb_host_ota_test
