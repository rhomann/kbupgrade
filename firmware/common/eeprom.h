/*
 * Keyboard Upgrade -- Firmware for homebrew computer keyboard controllers.
 * Copyright (C) 2009, 2010  Robert Homann
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

#ifndef EEPROM_H
#define EEPROM_H
#include <avr/eeprom.h>

typedef struct
{
  uint8_t current_keymap_index;
  uint8_t fnkey1_row;
  uint8_t fnkey1_column;
  uint8_t fnkey2_row;
  uint8_t fnkey2_column;
  uint8_t fnkey_keymap_index[3];
} PersistentConfig;

#define KEYMAP_POINTER_FROM_INDEX(I)  ((uint8_t *)sizeof(PersistentConfig)+\
                                       (((I)-1)*sizeof(Storedmap)))
#define KEYMAP_POINTER_NULL           ((void *)(E2END+1))
#define CONFIG_POINTER                ((PersistentConfig *)0)

#ifndef MAXIMUM_KEYMAP_INDEX
#define MAXIMUM_KEYMAP_INDEX   ((E2END+1-sizeof(PersistentConfig))/\
                                sizeof(Storedmap))
#endif /*! MAXIMUM_KEYMAP_INDEX */
#endif /* !EEPROM_H */
