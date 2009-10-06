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

#ifdef NO_GHOSTKEY_PREVENTION
#ifndef NO_ENHANCED_GKP
#define NO_ENHANCED_GKP 1
#endif /* !NO_ENHANCED_GKP */
#endif /* NO_GHOSTKEY_PREVENTION */

static void decode(Map *map, const uint8_t src[NUM_OF_KEYS])
{
  memset(map,0,sizeof(Map));

#ifndef NO_ENHANCED_GKP
  memset(column_valid_mask,0xff,sizeof(Columnstate)*NUM_OF_ROWS);
#endif /* !NO_ENHANCED_GKP */

  int keys_left=NUM_OF_KEYS;
#ifdef __AVR_ARCH__
  const prog_uint8_t *ptr=matrix_bits;
#else /* !__AVR_ARCH__ */
  const uint8_t *ptr=matrix_bits;
#define pgm_read_byte(PTR) (*(PTR))
#endif /* __AVR_ARCH__ */

  uint8_t mask=0x80;
  uint8_t bits=pgm_read_byte(ptr);

#ifndef NO_ENHANCED_GKP
  Columnstate cmask=(Columnstate)(COLUMNSTATE_EMPTY << 1);
  uint8_t columns=NUM_OF_COLUMNS, row=0;
#endif /* !NO_ENHANCED_GKP */

  for(uint8_t *dest=(void *)map->mat; keys_left > 0; ++dest)
  {
    if(bits&mask)
    {
      *dest=*src++;
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
static void decode_from_pgm(Map *map, PGM_VOID_P stored)
{
  uint8_t temp[NUM_OF_KEYS];

  memcpy_P(temp,((const uint8_t *)stored)+sizeof(Storedmap)-NUM_OF_KEYS,
           NUM_OF_KEYS);
  decode(map,temp);
}
#endif /* __AVR_ARCH__ */
