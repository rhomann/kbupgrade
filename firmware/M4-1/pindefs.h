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

#define ROWS_PORT   PORTB
#define ROWS_DDR    DDRB
#define ROWS_ALL    (_BV(PB0)|_BV(PB1)|_BV(PB2))

#define SHIFT_PORT  PORTB
#define SHIFT_CLOCK _BV(PB3)
#define SHIFT_DATA  _BV(PB4)

#define COLS_PORT1  PORTC
#define COLS_DDR1   DDRC
#define COLS_PIN1   PINC

#define COLS_PORT2  PORTD
#define COLS_DDR2   DDRD
#define COLS_PIN2   PIND

#define LED_PORT        PORTD
#define LED_DDR         DDRD
#define LED_SCROLL_PIN  PIND4
#define LED_CAPS_PIN    PIND5
#define LED_NUM_PIN     PIND6
