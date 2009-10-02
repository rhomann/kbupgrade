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

static void encode(const Map *const map, Storedmap *stored)
{
  memset(stored,0,sizeof(Storedmap));

  int i=0;

  for(const uint8_t *ptr=(void *)map->mat, *max=ptr+sizeof(map->mat);
      ptr < max;
      ++ptr)
  {
    if(*ptr != 0)
    {
      if(*ptr != KEY_trash) stored->codes[i]=*ptr;
      else                  stored->codes[i]=0;
      ++i;
    }
  }
}
