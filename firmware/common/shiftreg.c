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

#ifdef SHIFT_PISO_REGISTER
#if SHIFT_NUM_OF_PINS <= 8
typedef uint8_t  Shiftregbits;
#elif SHIFT_NUM_OF_PINS <= 16
typedef uint16_t Shiftregbits;
#else
#error Number of pins on shift register is greater than 16; not implemented.
#endif
#endif /* SHIFT_PISO_REGISTER */

static void shift_clock_data(void)
{
  SHIFT_PORT|=SHIFT_CLOCK;
  _delay_us(5);
  SHIFT_PORT&=~SHIFT_CLOCK;
}

#if !(defined SHIFT_NO_CLEAR_FUNCTION || defined SHIFT_PISO_REGISTER)
/* set all outputs of shift register to 1 */
static void shift_clear_all(void)
{
  SHIFT_PORT|=SHIFT_DATA;
  for(uint8_t i=0; i < SHIFT_NUM_OF_PINS; ++i)
  {
    _delay_us(5);
    shift_clock_data();
  }
}
#endif /* !(SHIFT_NO_CLEAR_FUNCTION || SHIFT_PISO_REGISTER) */

#ifdef SHIFT_PISO_REGISTER
static void shift_load_reg(void)
{
  SHIFT_PORT&=~SHIFT_NLOAD;
  _delay_us(5);
  SHIFT_PORT|=SHIFT_NLOAD;
}

static Shiftregbits shift_read_out(void)
{
  Shiftregbits data=0;

  for(uint8_t i=0; i < SHIFT_NUM_OF_PINS-1; ++i)
  {
    if(SHIFT_PIN&SHIFT_DATA) data|=1;
    data<<=1;
    _delay_us(5);
    shift_clock_data();
  }

  if(SHIFT_PIN&SHIFT_DATA) data|=1;

  return data;
}
#endif /* SHIFT_PISO_REGISTER */
