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

#include <util/delay.h>

#define LED_ALL_PINS  (_BV(PIND4)|_BV(PIND5)|_BV(PIND6))
#define LED_PORT      PORTD
#define LED_DDR       DDRD

/*
 * activates on Right Control key
 */
static inline void  bootLoaderInit(void)
{
  PORTB=~_BV(PB0);
  DDRB=_BV(DDB0);
  PORTC=0xff;
  DDRC=0;
  LED_PORT=LED_ALL_PINS;
  LED_DDR=LED_ALL_PINS;
  _delay_us(20);
}

#define bootLoaderCondition()   ((PINC&_BV(PINC5)) == 0)
