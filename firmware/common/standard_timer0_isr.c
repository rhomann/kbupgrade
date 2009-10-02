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

ISR(TIMER0_COMP_vect,ISR_BLOCK)
{
  static uint8_t counter=0;

  TCNT0=0;

  /* return if we don't need periodic reports */
  if(usb_idle_rate == 0) return;

  /* we are called every 16 ms and USB idle times are given in 4 ms steps */
  if(counter >= 4)
  {
    /* we just idled 16 ms, but that was not long enough */
    counter-=4;
  }
  else
  {
    /* tell the main loop that the host wants an update, reset idle counter */
    update_needed=1;
    counter=usb_idle_rate;
  }
}
