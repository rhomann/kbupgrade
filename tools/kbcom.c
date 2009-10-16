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
#include <string.h>

#include "kbcom.h"

static int init_libusb(void)
{
  usb_init();

  if(usb_find_busses() <= 0)
  {
    fprintf(stderr,"No USB busses found.\n");
    return -1;
  }

  if(usb_find_devices() <= 0)
  {
    fprintf(stderr,"No USB devices found.\n");
    return -1;
  }

  return 0;
}

static struct usb_device *find_keyboard_by_ids(void)
{
  struct usb_device *found_dev=NULL;

  for(struct usb_bus *bus=usb_busses; bus != NULL; bus=bus->next)
  {
    for(struct usb_device *dev=bus->devices; dev != NULL; dev=dev->next)
    {
      if(dev->descriptor.idVendor != KBUPGRADE_VENDOR_ID ||
         dev->descriptor.idProduct != KBUPGRADE_DEVICE_ID)
        continue;

      if(found_dev == NULL) found_dev=dev;
      else
      {
        fprintf(stderr,"Found more than one matching device, cannot continue.\n");
        return NULL;
      }
    }
  }

  if(found_dev == NULL) fprintf(stderr,"Found no matching device.\n");
  return found_dev;
}

static int read_compare_dev_string(usb_dev_handle *dev, int idx,
                                   const char *expected, const char *what)
{
  int len;
  unsigned char buffer[256];
  const unsigned char *bufptr=buffer+2;

  if((len=usb_control_msg(dev,USB_ENDPOINT_IN,USB_REQ_GET_DESCRIPTOR,
                          (USB_DT_STRING << 8)+idx,0x0409,
                          (char *)buffer,sizeof(buffer),1000)) < 0)
  {
    fprintf(stderr,"Could not read %s from USB device.\n",what);
    return -1;
  }

  if(buffer[1] != USB_DT_STRING)
  {
    fprintf(stderr,"Got invalid value for %s from USB device.\n",what);
    return -1;
  }

  if(buffer[0] < len) len=buffer[0];
  len=len/2-1;

  int i;
  for(i=0; i < len && expected[i] == *bufptr; ++i, bufptr+=2)
    ;

  if(i == len) return 0;

  fprintf(stderr,"Got unexpected %s from USB device.\n",what);
  return -1;
}

static usb_dev_handle *open_keyboard_dev(struct usb_device *kb)
{
  if(kb == NULL) return NULL;

  usb_dev_handle *dev=usb_open(kb);

  if(dev == NULL)
  {
    fprintf(stderr,"Could not open USB device: %s\n",usb_strerror());
    return NULL;
  }

  if(read_compare_dev_string(dev,kb->descriptor.iManufacturer,
                             KBUPGRADE_VENDOR_NAME,
                             "manufacturer name") == -1 ||
     read_compare_dev_string(dev,kb->descriptor.iProduct,
                             KBUPGRADE_DEVICE_NAME,
                             "product name") == -1)
  {
    usb_close(dev);
    dev=NULL;
  }

  return dev;
}

int kb_get_device(USBKeyboard *kb)
{
  if(init_libusb() == -1) return -1;

  kb->dev=find_keyboard_by_ids();
  kb->handle=open_keyboard_dev(kb->dev);
  kb->iface=-1;

  return (kb->handle != NULL)?0:-1;
}

int kb_claim_device(USBKeyboard *kb)
{
  if(usb_set_configuration(kb->handle,kb->dev->config->bConfigurationValue) != 0)
  {
    fprintf(stderr,"Could not set USB configuration: %s\n",usb_strerror());
  }

  kb->iface=kb->dev->config->interface->altsetting->bInterfaceNumber;

#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
  if(usb_detach_kernel_driver_np(kb->handle,kb->iface) != 0)
  {
    fprintf(stderr,"Could not detach kernel driver from interface: %s\n",usb_strerror());
    return -1;
  }
#endif /* LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP */

  if(usb_claim_interface(kb->handle,
                         kb->dev->config->interface->altsetting->bInterfaceNumber) != 0)
  {
    fprintf(stderr,"Could not claim USB interface: %s.\n",usb_strerror());
  }

  return 0;
}

void kb_close_device(USBKeyboard *kb)
{
  if(kb->iface >= 0 && usb_release_interface(kb->handle,kb->iface) != 0)
  {
    fprintf(stderr,"Could not release USB interface: %s.\n",usb_strerror());
  }

  if(kb->handle != NULL) usb_close(kb->handle);
  kb->dev=NULL;
  kb->handle=NULL;
  kb->iface=-1;
}
