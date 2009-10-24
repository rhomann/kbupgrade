/*
 * Keyboard Upgrade -- Firmware for homebrew computer keyboard controllers.
 * Copyright (C) 2009  Robert Homann
 *
 * This file is part of the Keyboard Upgrade package.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Keyboard Upgrade package; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

#ifndef KBCOM_H
#define KBCOM_H
#include <libusb-1.0/libusb.h>

typedef struct
{
  libusb_context *ctx;
  libusb_device_handle *handle;
  int iface, conf;
  int was_attached;
} USBKeyboard;

const char *usberror_to_string(enum libusb_error err);

int kb_get_device(USBKeyboard *kb);
int kb_claim_device(USBKeyboard *kb);
void kb_close_device(USBKeyboard *kb);
#endif /* !KBCOM_H */
