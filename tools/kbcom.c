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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kbcom.h"

#define KBDEVCONFIG 1
#define KBDEVIFACE  0

const char *usberror_to_string(enum libusb_error err)
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

static int get_string(libusb_device_handle *handle, int idx,
                      unsigned char *buffer, size_t bufsize, char **copy)
{
  int ret=0;

  if(idx > 0)
  {
    /* index 0 means "not present", so we are only trying positive values of
     * idx */
    if((ret=libusb_get_string_descriptor_ascii(handle,idx,buffer,bufsize)) < 0)
      return ret;
  }
  else buffer[0]='\0';

  if(copy == NULL) return 0;

  if(ret == 0)
  {
    ret=1;
    buffer[0]='?';
  }

  if((*copy=malloc(ret+1)) == NULL) return LIBUSB_ERROR_NO_MEM;

  memcpy(*copy,buffer,ret);
  (*copy)[ret]='\0';
  return 0;
}

static int read_compare_string(libusb_device_handle *handle, int idx,
                               const char *expected, size_t min_match_len,
                               char **string, const char *what)
{
  unsigned char buffer[256];
  int ret;

  if((ret=get_string(handle,idx,buffer,sizeof(buffer),string)) != 0)
  {
    fprintf(stderr,"Could not read %s from USB device: %s\n",
            what,usberror_to_string(ret));
    return -1;
  }

  if(min_match_len == 0) min_match_len=strlen(expected);
  return (strncmp((const char *)buffer,expected,min_match_len) == 0)?0:-1;
}

static int detach_dev(libusb_device_handle *handle, int *conf, int *detached)
{
  *detached=0;

  int ret;
  if((ret=libusb_get_configuration(handle,conf)) != 0)
  {
    fprintf(stderr,"Could not get USB device configuration: %s\n",
            usberror_to_string(ret));
    return -1;
  }

  if((ret=libusb_kernel_driver_active(handle,KBDEVIFACE)) == 1)
  {
    if((ret=libusb_detach_kernel_driver(handle,KBDEVIFACE)) == 0)
    {
      *detached=1;
      return 0;
    }
    fprintf(stderr,"Could not detach driver from USB interface: %s\n",
            usberror_to_string(ret));
  }
  else if(ret != 0)
  {
    fprintf(stderr,"Could not check if kernel driver is active: %s\n",
            usberror_to_string(ret));
  }

  return -1;
}

static void reattach_dev(libusb_device_handle *handle, int was_attached,
                         int iface)
{
  int ret;

  if(was_attached &&
     (ret=libusb_attach_kernel_driver(handle,iface)) != 0)
  {
    fprintf(stderr,"Could not re-attach to kernel driver: %s\n",
            usberror_to_string(ret));
  }
}

static void free_string_list(char **list)
{
  if(list == NULL) return;
  for(size_t i=0; list[i] != NULL; ++i) free(list[i]);
  free(list);
}

static char *make_name(char *devname, char *kbname)
{
  size_t len1=strlen(kbname);
  size_t len2=strlen(devname);

  if((kbname=realloc(kbname,len1+len2+4)) != NULL)
  {
    kbname[len1++]=' ';
    kbname[len1++]='(';
    memcpy(kbname+len1,devname,len2);
    len1+=len2;
    kbname[len1++]=')';
    kbname[len1]='\0';
  }

  free(devname);
  return kbname;
}

static ssize_t filter_device_list(libusb_device **list, ssize_t len,
                                  libusb_device ***filtered_list,
                                  char ***fullname_list)
{
  *filtered_list=NULL;
  *fullname_list=NULL;

  libusb_device **new_list=malloc((len+1)*sizeof(libusb_device *));
  char **names=malloc((len+1)*sizeof(char *));

  if(new_list == NULL || names == NULL)
  {
    fprintf(stderr,"Out of memory.\n");
    libusb_free_device_list(list,1);
    if(names != NULL) free(names);
    return LIBUSB_ERROR_NO_MEM;
  }

  new_list[0]=NULL;
  names[0]=NULL;

  struct libusb_device_descriptor descr;
  ssize_t count=0;
  libusb_device_handle *handle;
  int ret, was_attached;

  for(ssize_t i=0; i < len; ++i)
  {
    if((ret=libusb_get_device_descriptor(list[i],&descr)) != 0)
    {
      fprintf(stderr,"Could not get device descriptor: %s\n",
              usberror_to_string(ret));
      goto exit_early_error;
    }

    if(descr.idVendor == KBUPGRADE_VENDOR_ID &&
       descr.idProduct == KBUPGRADE_DEVICE_ID)
    {
      /* seems to be right, but we'd better check the vendor and product IDs,
       * too */
      ret=libusb_open(list[i],&handle);

      if(ret != 0)
      {
        fprintf(stderr,"Could not open device: %s\n",usberror_to_string(ret));
        goto exit_early_error;
      }

      int conf;
      if((ret=detach_dev(handle,&conf,&was_attached)) != 0)
        goto exit_error;

      if((ret=libusb_claim_interface(handle,KBDEVIFACE)) != 0)
      {
        fprintf(stderr,"Could not claim USB interface: %s\n",
                usberror_to_string(ret));
        reattach_dev(handle,was_attached,KBDEVIFACE);
        goto exit_error;
      }

      char *devname;
      if(read_compare_string(handle,descr.iManufacturer,
                             KBUPGRADE_VENDOR_NAME,0,
                             NULL,"manufacturer name") == 0 &&
         read_compare_string(handle,descr.iProduct,
                             KBUPGRADE_DEVICE_NAME,strlen(PACKAGE_NAME)+5,
                             &devname,"product name") == 0)
      {
        unsigned char buffer[256];
        char *kbname;
        if((ret=get_string(handle,descr.iSerialNumber,buffer,sizeof(buffer),
                           &kbname)) != 0)
        {
          fprintf(stderr,"Could not read keyboard name from USB device: %s\n",
                  usberror_to_string(ret));
          goto exit_deep_error;
        }

        /* vendor name is matching and the device name is close enough (the
         * micro version is not taken into account); reference the device and
         * add to filtered list */
        new_list[count]=libusb_ref_device(list[i]);
        new_list[count+1]=NULL;
        names[count]=make_name(devname,kbname);
        names[count+1]=NULL;
        if(names[count] == NULL)
        {
          fprintf(stderr,"Out of memory.\n");
          goto exit_deep_error;
        }
        ++count;
      }

      libusb_release_interface(handle,KBDEVIFACE);
      reattach_dev(handle,was_attached,KBDEVIFACE);
      libusb_close(handle);
    }
  }

  libusb_free_device_list(list,1);
  *filtered_list=new_list;
  *fullname_list=names;
  return count;

exit_deep_error:
  libusb_release_interface(handle,KBDEVIFACE);
  reattach_dev(handle,was_attached,KBDEVIFACE);

exit_error:
  libusb_close(handle);

exit_early_error:
  libusb_free_device_list(new_list,1);
  libusb_free_device_list(list,1);
  free_string_list(names);
  return ret;
}

static void dump_devices(char *const *names, ssize_t len)
{
  fprintf(stderr,"Multiple keyboards found:\n");
  for(ssize_t i=0; i < len; ++i) fprintf(stderr,"  #%zd: %s\n",i+1,names[i]);
}

static int open_device_or_dump_list(USBKeyboard *kb,
                                    libusb_device **filtered_list, char **names,
                                    ssize_t num_of_devs, ssize_t keyboard_index)
{
  int ret=-1;

  if(keyboard_index == -1)
  {
    if(num_of_devs > 1)
    {
      dump_devices(names,num_of_devs);
      ret=1;
    }
    else keyboard_index=0;
  }

  if(keyboard_index >= 0)
  {
    if(keyboard_index < num_of_devs)
    {
      /* valid index */
      ret=libusb_open(filtered_list[keyboard_index],&kb->handle);
    }
    else
    {
      fprintf(stderr,
              "Found matching %zd devices, your choice is out of range.\n",
              num_of_devs);
    }
  }

  libusb_free_device_list(filtered_list,1);
  free_string_list(names);

  return ret;
}

int kb_get_device(USBKeyboard *kb, ssize_t keyboard_index)
{
  if(libusb_init(&kb->ctx) != 0)
  {
    fprintf(stderr,"Could not initialize USB context.\n");
    return -1;
  }

  kb->iface=-1;
  kb->was_attached=0;

  int ret=-1;
  libusb_device **list;
  ssize_t num_of_devs=libusb_get_device_list(kb->ctx,&list);

  if(num_of_devs > 0)
  {
    libusb_device **filtered_list;
    char **names;

    if((num_of_devs=filter_device_list(list,num_of_devs,&filtered_list,
                                       &names)) > 0 &&
       (ret=open_device_or_dump_list(kb,filtered_list,names,num_of_devs,
                                     keyboard_index)) == 0)
    {
      return 0;
    }
  }
  else if(num_of_devs == 0)
  {
    fprintf(stderr,"No matching USB devices found.\n");
  }
  else
  {
    fprintf(stderr,"Could not get USB device list: %s\n",
            usberror_to_string(num_of_devs));
  }

  libusb_exit(kb->ctx);
  kb->ctx=NULL;
  return ret;
}

int kb_claim_device(USBKeyboard *kb)
{
  int ret=detach_dev(kb->handle,&kb->conf,&kb->was_attached);

  if(ret != 0) return ret;

  if((ret=libusb_set_configuration(kb->handle,KBDEVCONFIG)) != 0)
  {
    fprintf(stderr,"Could not set USB device configuration: %s\n",
            usberror_to_string(ret));
    return -1;
  }

  if((ret=libusb_claim_interface(kb->handle,KBDEVIFACE)) != 0)
  {
    fprintf(stderr,"Could not claim USB interface: %s\n",
            usberror_to_string(ret));
    return -1;
  }

  kb->iface=KBDEVIFACE;
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
      if((ret=libusb_set_configuration(kb->handle,kb->conf)) != 0)
      {
        fprintf(stderr,"Could not set USB device configuration: %s\n",
                usberror_to_string(ret));
      }
      reattach_dev(kb->handle,kb->was_attached,kb->iface);
    }
    libusb_close(kb->handle);
  }

  if(kb->ctx != NULL) libusb_exit(kb->ctx);

  kb->ctx=NULL;
  kb->handle=NULL;
  kb->iface=-1;
}
