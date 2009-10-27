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

#include <inttypes.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "pindefs.h"
#include "leddefs.h"

#include "usbkeycodes.h"
#include "keyboard.h"
#include "stdmap.h"
#include "keymapdecoder.h"

static Map current_keymap;
static Columnstate column_valid_mask[NUM_OF_ROWS];
static Columnstate column_states[NUM_OF_ROWS];

static uint8_t get_keymap_config(void)
{
  uint8_t idx=~PIND;
  idx=(((idx&0x08) >> 2)|((idx&0x02) >> 1))&0x03;
  return idx;
}

#include "keymapdecode.c"
#define USB_SET_LED_STATE  set_led_state
#include "usbfuns.c"
#include "processcolumns.c"
#include "scankeys.c"

static inline void clock_data(void)
{
  SHIFT_PORT|=SHIFT_CLOCK;
  _delay_us(5);
  SHIFT_PORT&=~SHIFT_CLOCK;
}

/*
 * Update global array column_states with values read from the I/O pins.
 */
static char update_column_states(void)
{
  char state_changed=0;

  /* shift a zero through the shift register */
  SHIFT_PORT&=~SHIFT_DATA;

  /* enable pull-ups on the three remaining rows */
  ROWS_DDR&=~ROWS_ALL;
  ROWS_PORT|=ROWS_ALL;

  for(uint8_t row=0; row < NUM_OF_ROWS; ++row)
  {
    /* move the 0 to the next pin of shift register */
    clock_data();
    if(row == 0)
    {
      SHIFT_PORT|=SHIFT_DATA;
    }
    else if(row >= 16)
    {
      ROWS_DDR=(ROWS_DDR&~ROWS_ALL)|_BV(row&0x07);
      ROWS_PORT=(ROWS_PORT|ROWS_ALL)&~_BV(row&0x07);
    }

    _delay_us(30);

    /* read 9 column bits */
    Columnstate cols=COLS_PIN1;
    if(COLS_PIN2&_BV(7)) cols|=0x100;

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
  /* the ADCs on port A will be used for the trackpoint in the future, so we
   * don't touch that port for now */
  PORTA=0x00;
  DDRA=0x00;

  /* port B has inputs with pull-ups at pins 0, 1, 2, 5, 6, and 7; and outputs
   * at 3 and 4; only shift clock is set to 0 */
  PORTB=~SHIFT_CLOCK;
  DDRB=_BV(PB3)|_BV(PB4);

  /* port C are all inputs with pull-ups */
  PORTC=0xff;
  DDRC=0x00;

  /* port D has 2 inputs for jumper headers (pins 1, 3), one input for column 9
   * (pin 7), 3 outputs for LED grounds (pins 4, 5, 6), and two ports for USB
   * data (pins 0 and 2) */
  PORTD=0xfa;   /* 1111 1010 */
  DDRD=0x70;    /* 0111 0000 */

  /* set all outputs of shift registers to 1 */
  SHIFT_PORT|=SHIFT_DATA;
  for(int i=0; i < 16; ++i)
  {
    _delay_us(5);
    clock_data();
  }

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

  set_current_keymap();

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
