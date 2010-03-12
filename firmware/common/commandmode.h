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

#ifndef COMMANDMODE_H
#define COMMANDMODE_H
#define MODE_NORMAL                 ((Mode)0)
#define MODE_ENTER_COMMAND          ((Mode)1)
#define MODE_COMMAND                ((Mode)2)
#define MODE_LEAVE_COMMAND          ((Mode)3)
#define MODE_GET_FNKEY1_ENTER       ((Mode)4)
#define MODE_GET_FNKEY2_ENTER       ((Mode)5)
#define MODE_GET_FNKEY1             ((Mode)6)
#define MODE_GET_FNKEY2             ((Mode)7)

#define MODE_TRANSITION(VAR,MODE)  (VAR)=(MODE)

#define CMDMODE_ENTER_KEY   KEY_scrlck
#define CMDMODE_ABORT_KEY   KEY_esc

typedef uint8_t Mode;
#endif /* !COMMANDMODE_H */
