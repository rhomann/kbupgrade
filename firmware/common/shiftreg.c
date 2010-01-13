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

#ifndef SHIFT_NUM_OF_PINS
#error Please define the total number of pins on your shift register(s) as\
 SHIFT_NUM_OF_PINS.
#endif /* !SHIFT_NUM_OF_PINS */

static void shift_clock_data(void)
{
  SHIFT_PORT|=SHIFT_CLOCK;
  _delay_us(5);
  SHIFT_PORT&=~SHIFT_CLOCK;
}

#ifndef SHIFT_NO_CLEAR_FUNCTION
/* set all outputs of shift register to 1 */
static void shift_clear_all(void)
{
  SHIFT_PORT|=SHIFT_DATA;
  for(int i=0; i < SHIFT_NUM_OF_PINS; ++i)
  {
    _delay_us(5);
    shift_clock_data();
  }
}
#endif /* !SHIFT_NO_CLEAR_FUNCTION */
