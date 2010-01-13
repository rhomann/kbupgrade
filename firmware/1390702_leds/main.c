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

#include <inttypes.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define CFG_KEYMAP0_PORT  PIND
#define CFG_KEYMAP0_PIN   PIND7

#include "pindefs.h"
#include "leddefs.h"

#include "usbkeycodes.h"
#include "keyboard.h"
#include "stdmap.h"
#include "keymapdecoder.h"

static Map current_keymap;
static Columnstate column_valid_mask[NUM_OF_ROWS];
static Columnstate column_states[NUM_OF_ROWS];

#include "keymapdecode.c"
#define USB_SET_LED_STATE  set_led_state
#include "usbfuns.c"
#include "processcolumns.c"
#include "scankeys.c"
#include "shiftreg.c"

/*
 * Update global array column_states with values read from the I/O pins.
 */
static char update_column_states(void)
{
  char state_changed=0;

  /* shift a zero through the shift register */
  SHIFT_PORT&=~SHIFT_DATA;

  /* enable pull-ups on the twelve remaining rows */
  ROWS_DDR1=0x00;
  ROWS_PORT1=0xff;
  ROWS_DDR2&=~ROWS_ALL2;
  ROWS_PORT2|=ROWS_ALL2;

  for(uint8_t row=0; row < NUM_OF_ROWS; ++row)
  {
    /* move the 0 to the next pin of shift register */
    shift_clock_data();
    if(row == 0)
    {
      SHIFT_PORT|=SHIFT_DATA;
    }
    else if(row >= SHIFT_NUM_OF_PINS)
    {
      if(row == 8+SHIFT_NUM_OF_PINS)
      {
        ROWS_DDR1=0x00;
        ROWS_PORT1=0xff;
      }

      if(row < 8+SHIFT_NUM_OF_PINS)
      {
        ROWS_DDR1=_BV(row&0x07);
        ROWS_PORT1=~_BV(row&0x07);
      }
      else
      {
        /* remaining four rows on second row port */
        ROWS_DDR2=(ROWS_DDR2&~ROWS_ALL2)|_BV(row&0x07);
        ROWS_PORT2=(ROWS_PORT2|ROWS_ALL2)&~_BV(row&0x07);
      }
    }

    _delay_us(30);

    /* read 8 column bits */
    Columnstate cols=COLS_PIN;
    cols|=column_valid_mask[row];
    if(column_states[row] != cols)
    {
      column_states[row]=cols;
      state_changed=1;
    }
  }

  return state_changed;
}

static void setup(void)
{
  /* port A has 4 inputs for some rows (pins 0, 1, 2, 3), two outputs for
   * driving the shift register (pins 4 and 5), and two unused pins (pins 6 and
   * 7, tri-stated) */
  PORTA=ROWS_ALL2;
  DDRA=SHIFT_CLOCK|SHIFT_DATA;

  /* ports B and C are all inputs, enable pull-ups */
  PORTB=0xff;
  DDRB=0x00;
  PORTC=0xff;
  DDRC=0x00;

  /* port D has 3 inputs for jumper headers (pins 1, 3, 7), 3 outputs for LED
   * grounds (pins 4, 5, 6), and two ports for USB data (pins 0 and 2) */
  PORTD=0xfa;   /* 1111 1010 */
  DDRD=0x70;    /* 0111 0000 */

  shift_clear_all();

  /* initialize USB stuff, force enumeration */
  usbInit();
  usbDeviceDisconnect();
  wdt_reset();

  /* flash the LEDs a bit */
  set_led_state(LED_NUM);
  _delay_ms(50);
  set_led_state(LED_CAPS);
  _delay_ms(50);
  set_led_state(LED_SCROLL);
  _delay_ms(50);
  set_led_state(0);

  set_current_keymap(get_current_keymap_index());

  _delay_ms(100);
  set_led_state(LED_SCROLL);
  _delay_ms(50);
  set_led_state(LED_CAPS);
  _delay_ms(50);
  set_led_state(LED_NUM);
  _delay_ms(50);
  set_led_state(0);

  /* that was a delay of at least 400 ms, now connect again */
  wdt_reset();
  usbDeviceConnect();

  /* set timer 0 to CTC mode, prescaling to 1024, and a value for the Output
   * Compare Register so that the timer triggers an interrupt every 16 ms */
#if F_CPU == 16*1000*1000
  OCR0=249;
#elif F_CPU == 12*1000*1000
  OCR0=187;
#else
#error Unsupported value of F_CPU.
#endif
  TCNT0=OCR0-1;
  TIMSK|=_BV(OCIE0);
  TCCR0=_BV(WGM01)|_BV(CS02)|_BV(CS00);

  sei();
}

volatile static uint8_t update_needed;

#include "standard_timer0_isr.c"
#include "standard_main.c"
