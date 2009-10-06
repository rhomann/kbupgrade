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

#include "pindefs.h"

static void clock_data(void)
{
  SHIFT_PORT|=SHIFT_CLOCK;
  _delay_us(5);
  SHIFT_PORT&=~SHIFT_CLOCK;
}

/*
 * activates on Escape key
 *
 */
static inline void  bootLoaderInit(void)
{
  DDRC=0;
  PORTC=0xff;
  LED_DDR=LED_ALL_PINS;
  LED_PORT=LED_ALL_PINS|_BV(PIND7);
  DDRB=_BV(DDB2)|_BV(DDB3)|_BV(DDB4);
  PORTB=~(SHIFT_CLOCK|_BV(PB2));

  /* shift a 0 to the row with the Escape key */
  for(int i=0; i < 16; ++i)
  {
    if(i == 8) PORTB&=~SHIFT_DATA;
    _delay_us(5);
    clock_data();
    if(i == 8) PORTB|=SHIFT_DATA;
  }

  _delay_us(20);
}

#define bootLoaderCondition()   ((PINC&_BV(PINC7)) == 0)
