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
                                  (unsigned char *)buffer,bufsize,5000);
  if(ret < 0)
  {
    fprintf(stderr,"Could not send buffer to USB device (input request): %s\n",
            usberror_to_string(ret));
  }
  return ret;
}

static inline int send_buffer(USBKeyboard *kb, KURequest req,
                              uint16_t val, uint16_t idx,
                              void *buffer, uint16_t bufsize)
{
  int ret=libusb_control_transfer(kb->handle,REQ_OUT,(uint8_t)req,val,idx,
                                  (unsigned char *)buffer,bufsize,5000);
  if(ret < 0)
  {
    fprintf(stderr,"Could not send buffer to USB device (output request): %s\n",
            usberror_to_string(ret));
  }
  return ret;
}

static int get_keymap(USBKeyboard *kb, uint8_t mapindex,
                      uint8_t *buffer, uint8_t mapsize)
{
  int ret=receive_buffer(kb,KURQ_GET_KEYMAP,mapindex,0,buffer,mapsize);

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

static int set_keymap(USBKeyboard *kb, uint8_t mapindex,
                      uint8_t *buffer, uint8_t mapsize, int just_delete)
{
  uint16_t value=mapindex;

  if(just_delete)
  {
    value|=0x0100;
    mapsize=0;
  }

  int ret=send_buffer(kb,KURQ_SET_KEYMAP,value,0,buffer,mapsize);

  if(ret < 0)
  {
    fprintf(stderr,"Error while writing key map (index %d).\n",mapindex);
    return -1;
  }

  if(ret != mapsize)
  {
    fprintf(stderr,"Written unexpected number of bytes (%d instead of %d)\n",
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

static int check_mapindex(const KBHwinfo *info, uint8_t min, int mapindex)
{
  if(mapindex < min || mapindex > info->max_mapindex)
  {
    fprintf(stderr,"Invalid index, must be in range %hhu...%hhu\n",
            min,info->max_mapindex);
    return -1;
  }
  return 0;
}

static int write_blob_to_file(const char *outfilename,
                              const uint8_t *data, size_t size)
{
  FILE *out=fopen(outfilename,"w");

  if(out == NULL)
  {
    perror("fopen()");
    return -1;
  }

  int ret=0;
  if(fwrite(data,size,1,out) != 1)
  {
    perror("fwrite()");
    ret=-1;
  }
  fclose(out);
  return ret;
}

static int download_keymap(USBKeyboard *kb, const KBHwinfo *info,
                           const char *outfilename, int mapindex)
{
  if(check_mapindex(info,0,mapindex) != 0) return -1;

  uint8_t keymap[KEYMAP_NAME_LENGTH+info->num_of_keys];
  if(get_keymap(kb,mapindex,keymap,sizeof(keymap)) != 0) return -1;

  if(keymap[0] == '\0' || keymap[0] == 0xff)
    fprintf(stderr,"Warning: key map data belongs to deleted entry.\n");

  return write_blob_to_file(outfilename,keymap,sizeof(keymap));
}

static int download_layout(USBKeyboard *kb, const KBHwinfo *info,
                           const char *outfilename)
{
  uint8_t vector[info->matrix_bvlen];
  int ret=receive_buffer(kb,KURQ_GET_LAYOUT,0,0,vector,info->matrix_bvlen);

  if(ret < 0)
  {
    fprintf(stderr,"Error while reading keyboard layout.\n");
    return -1;
  }

  if(ret != info->matrix_bvlen)
  {
    fprintf(stderr,"Received unexpected number of bytes (%d instead of %d)\n",
            ret,info->matrix_bvlen);
    return -1;
  }

  return write_blob_to_file(outfilename,vector,sizeof(vector));
}

static int read_keymap_from_file(const char *infilename,
                                 uint8_t *buffer, int bufsize)
{
  FILE *in=fopen(infilename,"r");
  int ret=-1;
  long size=0;

  if(in != NULL)
  {
    if(fseek(in,0L,SEEK_END) == 0 && (size=ftell(in)) != -1)
    {
      if(bufsize == size)
      {
        rewind(in);

        if(fread(buffer,bufsize,1,in) == 1) ret=0;
        else perror("fread()");
      }
      else
      {
        fprintf(stderr,"Invalid file size, expecting file of size %d\n",
                bufsize);
      }
    }
    else
    {
      if(size == 0) perror("fseek()");
      else          perror("ftell()");
    }
    fclose(in);
  }
  else perror("fopen()");

  return ret;
}

static int upload_keymap(USBKeyboard *kb, const KBHwinfo *info,
                         const char *infilename, int mapindex)
{
  if(check_mapindex(info,1,mapindex) != 0) return -1;

  uint8_t keymap[KEYMAP_NAME_LENGTH+info->num_of_keys];
  if(read_keymap_from_file(infilename,
                           keymap,KEYMAP_NAME_LENGTH+info->num_of_keys) == -1)
    return -1;

  if(keymap[0] == '\0' || keymap[0] == 0xff)
  {
    fprintf(stderr,"Key map data invalid.\n");
    return -1;
  }

  return set_keymap(kb,mapindex,keymap,sizeof(keymap),0);
}

static int delete_keymap(USBKeyboard *kb, const KBHwinfo *info, int mapindex)
{
  if(check_mapindex(info,1,mapindex) != 0) return -1;
  return set_keymap(kb,mapindex,NULL,0,1);
}

static void usage(const char *progname)
{
  fprintf(stderr,
          "Usage: %s [-n num] -l filename\n"
          "       %s [-n num] -g filename index\n"
          "       %s [-n num] -k filename index\n"
          "       %s [-n num] -d index\n"
          "       %s [-n num] -r\n"
          "\nOptions:\n"
          "-l  Get keyboard layout, write to file.\n"
          "-g  Get key map stored at given index, write to file.\n"
          "-k  Write key map at given index.\n"
          "-d  Delete key map at given index.\n"
          "-r  Reset keyboard controller.\n"
          "-n  Select keyboard if there is more than one.\n"
          "No option: print basic hardware information and all key maps.\n",
          progname,progname,progname,progname);
}

int main(int argc, char *argv[])
{
  USBKeyboard kb;
  int ret;

  if((ret=kb_get_device(&kb,-1)) != 0)
  {
    if(ret == 1) fprintf(stderr,"\nUse option -n to select a keyboard.\n");
    return EXIT_FAILURE;
  }

  ret=EXIT_FAILURE;
  int try_close_device=1;

  KBHwinfo info;
  if(kb_claim_device(&kb) == 0 &&
     receive_buffer(&kb,KURQ_GET_HWINFO,0,0,
                    &info,sizeof(KBHwinfo)) == sizeof(KBHwinfo))
  {
    /* lame command line handling, should use getopt() */
    int temp=-1;
    if(argc == 1)
    {
      temp=show_status(&kb,&info);
    }
    else
    {
      if(argc == 3 && strcmp(argv[1],"-l") == 0)
        temp=download_layout(&kb,&info,argv[2]);
      else if(argc == 4 && strcmp(argv[1],"-g") == 0)
        temp=download_keymap(&kb,&info,argv[2],atoi(argv[3]));
      else if(argc == 4 && strcmp(argv[1],"-k") == 0)
        temp=upload_keymap(&kb,&info,argv[2],atoi(argv[3]));
      else if(argc == 3 && strcmp(argv[1],"-d") == 0)
        temp=delete_keymap(&kb,&info,atoi(argv[2]));
      else if(argc == 2 && strcmp(argv[1],"-r") == 0)
      {
        /* sending will fail, so we'll fake success */
        libusb_control_transfer(kb.handle,REQ_OUT,KURQ_RESET,0,0,NULL,0,5000);
        temp=0;
        try_close_device=0;
      }
      else usage(argv[0]);
    }

    if(temp == 0) ret=EXIT_SUCCESS;
  }

  if(try_close_device) kb_close_device(&kb);

  return ret;
}
