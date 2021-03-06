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

static uint8_t process_command(uint8_t key)
{
  uint8_t temp;

  switch(key)
  {
   case CMDMODE_ABORT_KEY:
    /* the official way to exit command mode */
    break;
   case CMDMODE_ENTER_KEY:
    /* go back to normal mode, emit the keystroke for the command mode key */
    return 1;
   case KEY_F1: case KEY_F2: case KEY_F3: case KEY_F4: case KEY_F5:
   case KEY_F6: case KEY_F7: case KEY_F8: case KEY_F9: case KEY_F10:
    key=key-KEY_F1+KEY_1;
    /* fall through */
   case KEY_1: case KEY_2: case KEY_3: case KEY_4: case KEY_5:
   case KEY_6: case KEY_7: case KEY_8: case KEY_9: case KEY_0:
    /* set key map 0..9 */
    if(key == KEY_0) temp=0;
    else             temp=key-KEY_1+1;
    if(temp != get_current_keymap_index(0)) set_current_keymap(temp,1);
    break;
   case KEY_F:
    /* set first FN key */
    return MODE_GET_FNKEY1_ENTER;
   case KEY_G:
    /* set second FN key */
    return MODE_GET_FNKEY2_ENTER;
   default:
    break;
  }

  return 0;
}

static uint8_t process_fnkey_command(uint8_t key)
{
  uint8_t temp;

  switch(key)
  {
   case CMDMODE_ABORT_KEY:
    /* delete FN key */
    if(current_fnkey_combination&0x01)
    {
      if(eeprom_read_byte(&CONFIG_POINTER->fnkey1_row) != 0xff)
      {
        eeprom_write_byte(&CONFIG_POINTER->fnkey1_row,0xff);
        eeprom_write_byte(&CONFIG_POINTER->fnkey1_column,0xff);
      }
    }
    if(current_fnkey_combination&0x02)
    {
      if(eeprom_read_byte(&CONFIG_POINTER->fnkey2_row) != 0xff)
      {
        eeprom_write_byte(&CONFIG_POINTER->fnkey2_row,0xff);
        eeprom_write_byte(&CONFIG_POINTER->fnkey2_column,0xff);
      }
    }
    break;
   case CMDMODE_ENTER_KEY:
    /* go back to normal mode, emit the keystroke for the command mode key */
    return 1;
   case KEY_F1: case KEY_F2: case KEY_F3: case KEY_F4: case KEY_F5:
   case KEY_F6: case KEY_F7: case KEY_F8: case KEY_F9: case KEY_F10:
    key=key-KEY_F1+KEY_1;
    /* fall through */
   case KEY_1: case KEY_2: case KEY_3: case KEY_4: case KEY_5:
   case KEY_6: case KEY_7: case KEY_8: case KEY_9: case KEY_0:
    /* set key map 0..9 */
    if(key == KEY_0) temp=0;
    else             temp=key-KEY_1+1;
    if(temp != get_current_keymap_index(current_fnkey_combination))
      write_current_keymap_index(current_fnkey_combination,temp);
    break;
   default:
    break;
  }

  return 0;
}
