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

#ifndef DEBOUNCE_ITERATIONS
#define DEBOUNCE_ITERATIONS  7
#endif /* DEBOUNCE_ITERATIONS */

#if DEBOUNCE_ITERATIONS < 1 || DEBOUNCE_ITERATIONS > 255
#error DEBOUNCE_ITERATIONS must be between 2 and 255 (1 to disable).
#endif

/* If the number of iterations is 1, then no software debouncing will be
 * performed. Useful if the keys are debounced by hardware already. */
#if DEBOUNCE_ITERATIONS == 1
#undef DEBOUNCE_ITERATIONS
#endif /* DEBOUNCE_ITERATIONS */

static char update_column_states(void);
static Mode process_columns(void);

static char wait_no_keys(void)
{
  for(uint8_t row=0; row < NUM_OF_ROWS; ++row)
  {
    if(column_states[row] != COLUMNSTATE_EMPTY) return 0;
  }
  return 1;
}

static uint8_t get_command_key(void)
{
  for(uint8_t row=0; row < NUM_OF_ROWS; ++row)
  {
    Columnstate state=column_states[row];
    if(state == COLUMNSTATE_EMPTY) continue;

    for(uint8_t col=0; col < NUM_OF_COLUMNS; ++col, state>>=1)
    {
      uint8_t key;
      if((state&1) || (key=current_keymap.mat[row][col]) == 0) continue;
      return key;
    }
  }
  return KEY__;
}

#include "commandmode.c"

/*
 * Standard scankey() function. It calls the function for updating the column
 * state, handles debouncing, and calls the function for processing the scanned
 * keys (i.e., preparing the USB report).
 */
static uint8_t scankeys(void)
{
#if DEBOUNCE_ITERATIONS
  static uint8_t debounce=2;
#endif /* DEBOUNCE_ITERATIONS */

  if(update_column_states())
  {
    /* keyboard state changed, debouncing required */
#if DEBOUNCE_ITERATIONS
    debounce=(DEBOUNCE_ITERATIONS)-1;
    return 0;
#endif /* DEBOUNCE_ITERATIONS */
  }
#ifndef DEBOUNCE_ITERATIONS
  else
  {
    /* nothing changed */
    return 0;
  }
#endif /* !DEBOUNCE_ITERATIONS */

#if DEBOUNCE_ITERATIONS
  if(debounce == 0 || --debounce)
  {
    /* nothing changed or still debouncing */
    return 0;
  }
#endif /* DEBOUNCE_ITERATIONS */

  static Mode mode;
#ifdef LED_ONOFF
  static char scrlck_led_state;
#endif /* LED_ONOFF */

  switch(mode)
  {
   case MODE_NORMAL:
    if((mode=process_columns()) == MODE_NORMAL)
    {
      /* so we need to construct a USB report... */
      return 1;
    }
    break;
   case MODE_ENTER_COMMAND:
   case MODE_LEAVE_COMMAND:
    /* wait until no key is pressed for a smooth transition between normal and
     * command mode */
    if(wait_no_keys())
    {
      if(mode == MODE_ENTER_COMMAND)
      {
        mode=MODE_COMMAND;
#ifdef LED_ONOFF
        scrlck_led_state=LED_PORT&LED_SCROLL;
        LED_ONOFF(SCROLL,~scrlck_led_state);
#endif /* LED_ONOFF */
      }
      else
      {
        mode=MODE_NORMAL;
#ifdef LED_ONOFF
        LED_ONOFF(SCROLL,scrlck_led_state);
#endif /* LED_ONOFF */

        /* inject a report with no keys pressed (the values in the buffer
         * should be all 0) */
        usb_report_buffer.keys[0]=KEY__;
        return 1;
      }
    }
    break;
   case MODE_COMMAND:
    mode=MODE_LEAVE_COMMAND;
    if(process_command(get_command_key()))
    {
      /* inject a report containing the Scroll Lock keycode (the rest of the
       * buffer should be all 0) */
      usb_report_buffer.keys[0]=CMDMODE_ENTER_KEY;
      return 1;
    }
    break;
  }

  return 0;
}
