/*
 * Keyboard Upgrade -- Firmware for homebrew computer keyboard controllers.
 * Copyright (C) 2010  Robert Homann
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

#include <util/delay.h>

/*
 * activates on Escape key
 */
static inline void  bootLoaderInit(void)
{
  PORTA=~_BV(PA4);
  DDRA=_BV(DDA4);
  PORTB=0xff;
  DDRB=0;
  PORTC=0xff;
  DDRC=0;
  PORTD=0xf0;
  DDRD=0;
  _delay_us(20);
}

#define bootLoaderCondition()   ((PINB&_BV(PINB5)) == 0)
