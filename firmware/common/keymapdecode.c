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

#ifdef __AVR_ARCH__
#include "eeprom.h"
#endif /* __AVR_ARCH__ */

#ifdef NO_GHOSTKEY_PREVENTION
#ifndef NO_ENHANCED_GKP
#define NO_ENHANCED_GKP 1
#endif /* !NO_ENHANCED_GKP */
#endif /* NO_GHOSTKEY_PREVENTION */

static void decode(Map *map, const uint8_t *src, uint8_t from_eeprom)
{
#ifndef __AVR_ARCH__
#define pgm_read_byte(PTR)     (*(PTR))
#define eeprom_read_byte(PTR)  (*(PTR))
#endif /* !__AVR_ARCH__ */

  memset(map,0,sizeof(Map));

#ifndef NO_ENHANCED_GKP
  memset(column_valid_mask,0xff,sizeof(Columnstate)*NUM_OF_ROWS);
#endif /* !NO_ENHANCED_GKP */

  int keys_left=NUM_OF_KEYS;
  uint8_t mask=0x80;
  const uint8_t *ptr=matrix_bits;
  uint8_t bits=pgm_read_byte(ptr);

#ifndef NO_ENHANCED_GKP
  Columnstate cmask=(Columnstate)(COLUMNSTATE_EMPTY << 1);
  uint8_t columns=NUM_OF_COLUMNS, row=0;
#endif /* !NO_ENHANCED_GKP */

  for(uint8_t *dest=(void *)map->mat; keys_left > 0; ++dest)
  {
    if(bits&mask)
    {
      if(from_eeprom) *dest=eeprom_read_byte(src);
      else            *dest=pgm_read_byte(src);
      ++src;
#ifdef ZERO_CODES_ARE_TRASH_CODES
      if(*dest == 0) *dest=KEY_trash;
#endif /* ZERO_CODES_ARE_TRASH_CODES */
#ifndef NO_ENHANCED_GKP
      column_valid_mask[row]&=cmask;
#endif /* !NO_ENHANCED_GKP */
      --keys_left;
    }
    if((uint8_t)(mask>>=1) == 0)
    {
      ++ptr;
      bits=pgm_read_byte(ptr);
      mask=0x80;
    }
#ifndef NO_ENHANCED_GKP
    if(--columns)
    {
      cmask=(cmask << 1)|1;
    }
    else
    {
      cmask=(Columnstate)(COLUMNSTATE_EMPTY << 1);
      columns=NUM_OF_COLUMNS;
      ++row;
    }
#endif /* !NO_ENHANCED_GKP */
  }
}

#ifdef __AVR_ARCH__
static const void *get_eeprom_keymap_pointer(uint8_t idx)
{
  if(idx == 0 || idx > MAXIMUM_KEYMAP_INDEX) return KEYMAP_POINTER_NULL;

  const uint8_t *ptr=KEYMAP_POINTER_FROM_INDEX(idx);
  uint8_t temp=eeprom_read_byte(ptr);
  if(temp == 0x00 || temp == 0xff) return KEYMAP_POINTER_NULL;
  return ptr+KEYMAP_NAME_LENGTH;
}

static uint8_t current_keymap_index;

static void set_current_keymap(void)
{
  const uint8_t *mapptr=NULL;

  current_keymap_index=get_keymap_config();

  if(current_keymap_index > 0)
    mapptr=get_eeprom_keymap_pointer(current_keymap_index);

  if(mapptr == KEYMAP_POINTER_NULL)
  {
    current_keymap_index=0;
    mapptr=standard_stored_keymap.codes;
  }

  decode(&current_keymap,mapptr,current_keymap_index);
}
#endif /* __AVR_ARCH__ */
