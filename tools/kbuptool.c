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
#include <ctype.h>

#include "kbcom.h"
#include "usbrequests.h"

#define REQ_IN   (LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_ENDPOINT_IN)
#define REQ_OUT  (LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_ENDPOINT_OUT)

#define KEYMAP_NAME_LENGTH 20

typedef union
{
  uint8_t bytes[8];
  uint16_t words[4];
} RWbuffer;

static inline int receive_buffer(USBKeyboard *kb, KURequest req,
                                 uint16_t val, uint16_t idx,
                                 void *buffer, uint16_t bufsize)
{
  int ret=libusb_control_transfer(kb->handle,REQ_IN,(uint8_t)req,val,idx,
                                  (unsigned char *)buffer,bufsize,1000);
  if(ret < 0)
  {
    fprintf(stderr,"Could not send buffer to USB device: %s\n",
            usberror_to_string(ret));
  }
  return ret;
}

static int get_keymap(USBKeyboard *kb, uint8_t mapindex,
                      uint8_t *buffer, uint8_t mapsize)
{
  int ret=receive_buffer(kb,KURQ_GET_KEYMAP,libusb_cpu_to_le16(mapindex),0,
                         buffer,mapsize);

  if(ret < 0)
  {
    fprintf(stderr,"Error while reading key map (index %d).\n",mapindex);
    return -1;
  }

  if(ret != mapsize)
  {
    fprintf(stderr,"Received unexpected number of bytes (%d instead of %d)\n",
            ret,mapsize);
    return -1;
  }

  return 0;
}

static int show_status(USBKeyboard *kb, const KBHwinfo *info)
{
  printf("Keyboard has %hhu keys on %hhu rows and %hhu columns, "
         "supports up to %hhu key maps.\n"
         "Matrix is encoded in %hhu bytes.\n",
         info->num_of_keys,info->num_of_rows,info->num_of_cols,
         info->max_mapindex+1,info->matrix_bvlen);

  if(info->max_mapindex > 15)
  {
    /* unreasonable */
    fprintf(stderr,"Maximum key map index seems invalid: %d.\n",
            info->max_mapindex);
    return -1;
  }

  for(int i=0; i <= info->max_mapindex; ++i)
  {
    uint8_t keymap[KEYMAP_NAME_LENGTH+info->num_of_keys];

    if(get_keymap(kb,i,keymap,sizeof(keymap)) != 0) return -1;

    printf("%s Key map %d: ",(i != info->current_mapindex)?"  ":"->",i);
    if(keymap[0] != '\0' && keymap[0] != 0xff)
    {
      putchar('"');
      for(int j=0; j < KEYMAP_NAME_LENGTH && keymap[j] != '\0'; ++j)
      {
        if(isprint(keymap[j])) putchar(keymap[j]);
        else                   putchar('.');
      }
      printf("\"\n");
    }
    else printf("(empty)\n");
  }

  return 0;
}

static int download_keymap(USBKeyboard *kb, const KBHwinfo *info,
                           const char *outfilename, int mapindex)
{
  if(mapindex < 0 || mapindex > info->max_mapindex)
  {
    fprintf(stderr,"Invalid index, must be in range 0...%hhu\n",
            info->max_mapindex);
    return -1;
  }

  uint8_t keymap[KEYMAP_NAME_LENGTH+info->num_of_keys];
  if(get_keymap(kb,mapindex,keymap,sizeof(keymap)) != 0) return -1;

  FILE *out=fopen(outfilename,"w");
  if(out == NULL)
  {
    perror("fopen()");
    return -1;
  }

  int ret=0;
  if(fwrite(keymap,sizeof(keymap),1,out) != 1)
  {
    perror("fwrite()");
    ret=-1;
  }
  fclose(out);
  return ret;
}

static void usage(const char *progname)
{
  fprintf(stderr,
          "Usage: %s [-g filename index]\n"
          "-g  Get key map stored at given index.\n",
          progname);
}

int main(int argc, char *argv[])
{
  USBKeyboard kb;
  int ret=EXIT_FAILURE;

  if(kb_get_device(&kb) == -1) return EXIT_FAILURE;

  KBHwinfo info;
  if(kb_claim_device(&kb) == 0 &&
     receive_buffer(&kb,KURQ_GET_HWINFO,0,0,
                    &info,sizeof(KBHwinfo)) == sizeof(KBHwinfo))
  {
    /* lame command line handling */
    int temp=-1;
    if(argc == 1)
    {
      temp=show_status(&kb,&info);
    }
    else if(argc == 4)
    {
      if(strcmp(argv[1],"-g") == 0)
        temp=download_keymap(&kb,&info,argv[2],atoi(argv[3]));
    }
    else usage(argv[0]);

    if(temp == 0) ret=EXIT_SUCCESS;
  }

  kb_close_device(&kb);

  return ret;
}
