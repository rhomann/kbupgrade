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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

#include "kbcom.h"

static int do_stuff(USBKeyboard *kb)
{
  /* do useful things here */
  return 0;
}

int main(int argc, char *argv[])
{
  USBKeyboard kb;
  int ret=EXIT_FAILURE;

  if(kb_get_device(&kb) == -1) return EXIT_FAILURE;
  if(kb_claim_device(&kb) == 0)
  {
    if(do_stuff(&kb) == 0) ret=EXIT_SUCCESS;
  }

  kb_close_device(&kb);

  return ret;
}
