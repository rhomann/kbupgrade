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

void main(void) __attribute__((noreturn));
void main(void)
{
  wdt_enable(WDTO_2S);
  setup();

  while(1)
  {
    wdt_reset();
    usbPoll();

    update_needed|=scankeys();

    if(update_needed && usbInterruptIsReady())
    {
      usbSetInterrupt((void *)&usb_report_buffer,sizeof(usb_report_buffer));

      /* reset timer in case it runs out very soon */
      TCNT0=0;
      update_needed=0;
    }
  }
}
