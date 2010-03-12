/*
 * Keyboard Upgrade -- Firmware for homebrew computer keyboard controllers.
 * Copyright (C) 2009, 2010  Robert Homann
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
static uint8_t row_column_to_key(uint8_t, uint8_t, char);
static Mode process_columns(void);

static char wait_no_keys(uint8_t fnkey)
{
  for(uint8_t row=0; row < NUM_OF_ROWS; ++row)
  {
    if(column_states[row] != COLUMNSTATE_EMPTY)
    {
      /* if necessary, ignore the FN keys if one was depressed at the time when
       * the command key was hit */
      if(fnkey == 0) return 0;

      if(row == eeprom_read_byte(&CONFIG_POINTER->fnkey1_row))
      {
        if(column_states[row] !=
           (Columnstate)~((Columnstate)1 << eeprom_read_byte(&CONFIG_POINTER->fnkey1_column)))
          return 0;
      }
      else if(row == eeprom_read_byte(&CONFIG_POINTER->fnkey2_row))
      {
        if(column_states[row] !=
           (Columnstate)~((Columnstate)1 << eeprom_read_byte(&CONFIG_POINTER->fnkey2_column)))
          return 0;
      }
      else
        return 0;
    }
  }
  return 1;
}

static uint8_t get_command_key(uint8_t fnkey)
{
  for(uint8_t row=0; row < NUM_OF_ROWS; ++row)
  {
    Columnstate state=column_states[row];
    if(state == COLUMNSTATE_EMPTY) continue;

    /* if necessary, ignore the FN keys if one was depressed at the time when
     * the command key was hit */
    if(fnkey)
    {
      if(row == eeprom_read_byte(&CONFIG_POINTER->fnkey1_row))
        state|=(Columnstate)1 << eeprom_read_byte(&CONFIG_POINTER->fnkey1_column);
      else if(row == eeprom_read_byte(&CONFIG_POINTER->fnkey2_row))
        state|=(Columnstate)1 << eeprom_read_byte(&CONFIG_POINTER->fnkey2_column);
    }

    for(uint8_t col=0; col < NUM_OF_COLUMNS; ++col, state>>=1)
    {
      uint8_t key;
      if((state&1) || (key=row_column_to_key(row,col,1)) == 0) continue;
      return key;
    }
  }
  return KEY__;
}

static char get_first_valid_fn_row_column(uint8_t *row, uint8_t *col)
{
  for(*row=0; *row < NUM_OF_ROWS; ++*row)
  {
    Columnstate state=column_states[*row];
    if(state == COLUMNSTATE_EMPTY) continue;

    for(*col=0; *col < NUM_OF_COLUMNS; ++*col, state>>=1)
    {
      if((state&1) == 0)
      {
        uint8_t key=row_column_to_key(*row,*col,0);
        if(key != KEY_function &&
           key != CMDMODE_ENTER_KEY && key != CMDMODE_ABORT_KEY) return 0;
      }
    }
  }
  return -1;
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
    {
    uint8_t prev_fnkeys=current_fnkey_combination;

    if((mode=process_columns()) == MODE_NORMAL)
    {
      /* so we need to construct a USB report... */
      if(current_fnkey_combination != prev_fnkeys)
      {
        /* function key state has toggled */
        uint8_t temp=get_current_keymap_index(current_fnkey_combination);

        if(current_fnkey_combination)
        {
          if(temp != get_current_keymap_index(0))
            set_current_keymap(temp,0);
        }
        else
        {
          if(temp != get_current_keymap_index(prev_fnkeys))
            set_current_keymap(temp,0);
        }
      }
      return 1;
    }
    break;
    }
   case MODE_ENTER_COMMAND:
   case MODE_LEAVE_COMMAND:
    /* wait until no key is pressed for a smooth transition between normal and
     * command mode */
    if(wait_no_keys(current_fnkey_combination))
    {
      if(mode == MODE_ENTER_COMMAND)
      {
        MODE_TRANSITION(mode,MODE_COMMAND);
#ifdef LED_ONOFF
        scrlck_led_state=LED_PORT&LED_SCROLL;
        LED_ONOFF(SCROLL,~scrlck_led_state);
#endif /* LED_ONOFF */
      }
      else
      {
        MODE_TRANSITION(mode,MODE_NORMAL);
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
   case MODE_GET_FNKEY1_ENTER:
   case MODE_GET_FNKEY2_ENTER:
    /* mode only reachable if FN is not depressed */
    if(wait_no_keys(0))
    {
      mode=(mode == MODE_GET_FNKEY1_ENTER)?MODE_GET_FNKEY1:MODE_GET_FNKEY2;
    }
    break;
   case MODE_COMMAND:
    MODE_TRANSITION(mode,MODE_LEAVE_COMMAND);

    uint8_t temp=get_command_key(current_fnkey_combination);

    temp=(current_fnkey_combination == 0)
      ?process_command(temp)
      :process_fnkey_command(temp);

    if(temp == 1)
    {
      /* inject a report containing the Scroll Lock keycode (the rest of the
       * buffer should be all 0) */
      usb_report_buffer.keys[0]=CMDMODE_ENTER_KEY;
      return 1;
    }
    else if(temp > 1)
    {
      /* command needs an argument; next mode is stored in temp */
      MODE_TRANSITION(mode,temp);
    }
    break;
   case MODE_GET_FNKEY1:
   case MODE_GET_FNKEY2:
    {
      uint8_t row, col;
      if(get_first_valid_fn_row_column(&row,&col) == 0)
      {
        if(mode == MODE_GET_FNKEY1)
        {
          eeprom_write_byte(&CONFIG_POINTER->fnkey1_row,row);
          eeprom_write_byte(&CONFIG_POINTER->fnkey1_column,col);
        }
        else
        {
          eeprom_write_byte(&CONFIG_POINTER->fnkey2_row,row);
          eeprom_write_byte(&CONFIG_POINTER->fnkey2_column,col);
        }
      }
      mode=MODE_LEAVE_COMMAND;
      break;
    }
  }

  return 0;
}
