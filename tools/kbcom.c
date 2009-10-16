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

static const char *usberror_to_string(enum libusb_error err)
{
  switch(err)
  {
   case LIBUSB_SUCCESS:             return "Success (no error).";
   case LIBUSB_ERROR_IO:            return "Input/output error.";
   case LIBUSB_ERROR_INVALID_PARAM: return "Invalid parameter.";
   case LIBUSB_ERROR_ACCESS:        return "Access denied (insufficient permissions).";
   case LIBUSB_ERROR_NO_DEVICE:     return "No such device (it may have been disconnected).";
   case LIBUSB_ERROR_NOT_FOUND:     return "Entity not found.";
   case LIBUSB_ERROR_BUSY:          return "Resource busy.";
   case LIBUSB_ERROR_TIMEOUT:       return "Operation timed out.";
   case LIBUSB_ERROR_OVERFLOW:      return "Overflow.";
   case LIBUSB_ERROR_PIPE:          return "Pipe error.";
   case LIBUSB_ERROR_INTERRUPTED:   return "System call interrupted (perhaps due to signal).";
   case LIBUSB_ERROR_NO_MEM:        return "Insufficient memory.";
   case LIBUSB_ERROR_NOT_SUPPORTED: return "Operation not supported or unimplemented on this platform.";
   case LIBUSB_ERROR_OTHER:         return "Other error.";
   default:                         return "Unknown error.";
  }
}

/*
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
    fprintf(stderr,"Could not read %s from USB device: %s\n",what,usb_strerror());
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
    fprintf(stderr,usberrorstring,usb_strerror());
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
*/

int kb_get_device(USBKeyboard *kb)
{
  if(libusb_init(&kb->ctx) != 0)
  {
    fprintf(stderr,"Could not initialize USB context.\n");
    return -1;
  }

  kb->iface=-1;

  if((kb->handle=libusb_open_device_with_vid_pid(kb->ctx,KBUPGRADE_VENDOR_ID,
                                                 KBUPGRADE_DEVICE_ID)) == NULL)
  {
    fprintf(stderr,"No matching device found.\n");
    libusb_exit(kb->ctx);
    kb->ctx=NULL;
    return -1;
  }

  return 0;
}

int kb_claim_device(USBKeyboard *kb)
{
  int ret, conf;

  if((ret=libusb_get_configuration(kb->handle,&conf)) != 0)
  {
    fprintf(stderr,"Could not get USB device configuration: %s\n",
            usberror_to_string(ret));
    return -1;
  }

  int iface=0;

  if((ret=libusb_kernel_driver_active(kb->handle,iface)) == 1)
  {
    fprintf(stderr,"Kernel driver is active, detaching.\n");
    if((ret=libusb_detach_kernel_driver(kb->handle,iface)) != 0)
    {
      fprintf(stderr,"Could not detach driver from USB interface: %s\n",
              usberror_to_string(ret));
      return -1;
    }
  }
  else if(ret != 0)
  {
    fprintf(stderr,"Could not check if kernel driver is active: %s\n",
            usberror_to_string(ret));
    return -1;
  }

  if((ret=libusb_set_configuration(kb->handle,conf)) != 0)
  {
    fprintf(stderr,"Could not set USB device configuration: %s\n",
            usberror_to_string(ret));
    return -1;
  }

  if((ret=libusb_claim_interface(kb->handle,iface)) != 0)
  {
    fprintf(stderr,"Could not claim USB interface: %s\n",
            usberror_to_string(ret));
    return -1;
  }

  kb->iface=iface;
  return 0;
}

void kb_close_device(USBKeyboard *kb)
{
  if(kb->handle != NULL)
  {
    int ret;
    if(kb->iface != -1)
    {
      if((ret=libusb_release_interface(kb->handle,kb->iface)) != 0)
      {
        fprintf(stderr,"Could not release USB interface: %s\n",
                usberror_to_string(ret));
      }
      if((ret=libusb_attach_kernel_driver(kb->handle,kb->iface)) != 0)
      {
        fprintf(stderr,"Could not attach to kernel driver: %s\n",
                usberror_to_string(ret));
      }
    }
    libusb_close(kb->handle);
  }

  if(kb->ctx != NULL) libusb_exit(kb->ctx);

  kb->ctx=NULL;
  kb->handle=NULL;
  kb->iface=-1;
}
