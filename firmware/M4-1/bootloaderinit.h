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

#define LED_ALL_PINS  (LED_SCROLL_PIN|LED_CAPS_PIN|LED_NUM_PIN)

static void clock_data(void)
{
  SHIFT_PORT|=SHIFT_CLOCK;
  _delay_us(5);
  SHIFT_PORT&=~SHIFT_CLOCK;
}

/*
 * activates on "U" key
 */
static inline void  bootLoaderInit(void)
{
  DDRC=0;
  PORTC=0xff;
  LED_DDR=LED_ALL_PINS;
  LED_PORT=LED_ALL_PINS|_BV(PIND7);
  DDRB=_BV(DDB2)|_BV(DDB3)|_BV(DDB4);
  PORTB=~(SHIFT_CLOCK|_BV(PB2));

  _delay_us(15);
  for(int i=0; i < 16; ++i)
  {
    _delay_us(5);
    clock_data();
  }

  _delay_us(20);
}

#define bootLoaderCondition()   ((PINC&_BV(PINC5)) == 0)
