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

#ifndef USBREQUESTS_H
#define USBREQUESTS_H

/* Vender-specific control requests. */
typedef enum
{
  KURQ_GET_HWINFO=0x10, /* Get hardware information. We start at 0x10,
                           outside the HID ID range. */
  KURQ_GET_LAYOUT,  /* Get keyboard layout. */
  KURQ_GET_KEYMAP,  /* Get selected key map (0=default, other from EEPROM). */
  KURQ_SET_KEYMAP,  /* Set key map (write to EEPROM). */

  /* internal requests */
  KURQ_GET_DATA_FROM_PGM,
  KURQ_GET_DATA_FROM_EEPROM,
  KURQ_WRITE_DATA_TO_EEPROM
} KURequest;

/* The structure returned by KURQ_GET_HWINFO, not larger than 8 bytes. */
typedef struct
{
  uint8_t max_mapindex;     /* Maximum value for the key map index. */
  uint8_t current_mapindex; /* Active key map. */
  uint8_t num_of_keys;      /* Number of keys on the keyboard. */
  uint8_t num_of_rows;      /* Number of rows on the keyboard matrix. */
  uint8_t num_of_cols;      /* Number of columns on the keyboard matrix. */
  uint8_t matrix_bvlen;     /* Number of bytes used to encode the matrix. */
} KBHwinfo;

#endif /* !USBREQUESTS_H */
