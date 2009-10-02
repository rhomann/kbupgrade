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
#define DEBOUNCE_ITERATIONS  10
#endif /* DEBOUNCE_ITERATIONS */

#if DEBOUNCE_ITERATIONS <= 1 || DEBOUNCE_ITERATIONS > 255
#error DEBOUNCE_ITERATIONS must be between 2 and 255.
#endif

static char update_column_states(void);
static void process_columns(void);

/*
 * Standard scankey() function. It calls the function for updating the column
 * state, handles debouncing, and calls the function for processing the scanned
 * keys (i.e., preparing the USB report).
 */
static uint8_t scankeys(void)
{
  static uint8_t debounce=4;

  if(update_column_states())
  {
    /* keyboard state changed, debouncing required */
    debounce=(DEBOUNCE_ITERATIONS)-1;
    return 0;
  }

  if(debounce == 0 || --debounce)
  {
    /* nothing changed or still debouncing */
    return 0;
  }

  /* so we need to construct a USB report... */
  process_columns();
  return 1;
}
