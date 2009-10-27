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

#include "commandmode.h"

#ifndef NO_GHOSTKEY_PREVENTION
/*
 * This function may look rather expensive, but it is not as expensive as you
 * might think.
 *
 * Let R and C be the number of rows and columns in the keyboard matrix,
 * respectively. The double loop looks like (R-1)^2 worst case running time at
 * first, but this is not the case. The other loop does at most R-1 iterations.
 * The inner loop is entered only if at least 2 columns got activated for row r.
 * Either it finds an entry with equal column assignments, in which case the
 * function returns; or it does not, in which case the inner loop does at most
 * R-1 iterations. The loop can, however, be entered only at most floor(C/2)
 * times since only that many ghost key-free column states with at least 2
 * activated columns can occur. If there are more states with 2 or more
 * activated columns, then they will be detected in the floor(C/2)-th
 * invokation of the inner loop, at latest.
 *
 * Therefore, the worst case running time of this function is less than
 * (R-1)+(R-1)*floor(C/2), which is roughly like (R-1)*(C/2).
 *
 * The expected running time is R-1 iterations since we can expect that ghost
 * keys occur very infrequently.
 */
static uint8_t ghost_keys_present(void)
{
  for(uint8_t r=0; r < NUM_OF_ROWS-1; ++r)
  {
    /* no ghost keys present for this row if no more than a single column got
     * activated */
    Columnstate temp=~column_states[r];
    if(!(temp&(Columnstate)(temp-1))) continue;

    /* search for equal column states, report ghost key condition if found */
    for(uint8_t s=r+1; s < NUM_OF_ROWS; ++s)
    {
      if(column_states[r] == column_states[s]) return 1;
    }
  }

  return 0;
}
#endif /* !NO_GHOSTKEY_PREVENTION */

/*
 * Update the usb_report_buffer using the column states stored in
 * column_states. This function also avoids ghost key constellations.
 */
static Mode process_columns(void)
{
#ifndef NO_GHOSTKEY_PREVENTION
  uint8_t ghosts=ghost_keys_present();

  if(!ghosts)
#endif /* !NO_GHOSTKEY_PREVENTION */
    memset(&usb_report_buffer,0,sizeof(usb_report_buffer));
#ifndef NO_GHOSTKEY_PREVENTION
  else usb_report_buffer.modifiers=0;
#endif /* !NO_GHOSTKEY_PREVENTION */

  uint8_t num_of_keys=0;
  for(uint8_t row=0; row < NUM_OF_ROWS; ++row)
  {
    Columnstate state=column_states[row];
    if(state == COLUMNSTATE_EMPTY) continue;

    for(uint8_t col=0; col < NUM_OF_COLUMNS; ++col, state>>=1)
    {
      uint8_t key;
      if((state&1) || (key=current_keymap.mat[row][col]) == 0) continue;

      /* column got activated, and the key should not be ignored */
      if(key < NOKEY_Modifiers)
      {
#ifndef NO_GHOSTKEY_PREVENTION
        if(ghosts) continue;
#endif /* !NO_GHOSTKEY_PREVENTION */
        if(key != KEY_scrlck)
        {
          if(num_of_keys < 6) usb_report_buffer.keys[num_of_keys++]=key;
          else if(num_of_keys == 6)
          {
            memset(usb_report_buffer.keys,KEY_errorRollOver,6);
            ++num_of_keys;
          }
        }
        else
        {
          /* command key pressed, enter command mode */
          memset(&usb_report_buffer,0,sizeof(usb_report_buffer));
          return MODE_ENTER_COMMAND;
        }
      }
      else
      {
        usb_report_buffer.modifiers|=_BV(key&0x0f);
      }
    }
  }

  return MODE_NORMAL;
}
