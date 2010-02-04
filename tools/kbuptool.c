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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <ctype.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

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
    fprintf(stderr,"Warning: downloaded key map data of deleted entry.\n");

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
  fprintf(stderr,"kbuptool is part of " PACKAGE_STRING ".\n\
Copyright (C) 2009, 2010  Robert Homann.\n\
\n\
This program is distributed in the hope that it will be useful, but WITHOUT\n\
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n\
FOR A PARTICULAR PURPOSE. See kbuptool -V for details and license information.\n\
\n\
Usage: %s [-n num] -l -f filename\n\
       %s [-n num] -g -f filename -i index\n\
       %s [-n num] -k -f filename -i index\n\
       %s [-n num] -d -i index\n\
       %s [-n num] [-r]\n\
       %s -hV\n\
\n\
Options:\n\
  -l  Get keyboard layout, write to file.\n\
  -g  Get key map stored at given index, write to file.\n\
  -k  Write key map at given index to the keyboard.\n\
  -d  Delete key map at given index in the keyboard.\n\
  -f  Specify the name of the file to be read or written.\n\
  -i  Key map index, where index 1 is the first user-defined key map.\n\
  -r  Reset keyboard controller.\n\
  -n  Select keyboard number if there is more than one.\n\
  -V  Show version information.\n\
  -h  This help screen.\n\
\n\
No options: print basic hardware information and all key maps.\n",
    progname,progname,progname,progname,progname,progname);
}

static void version_info(void)
{
  printf("kbuptool is part of " PACKAGE_STRING ".\n\
Copyright (C) 2009, 2010  Robert Homann.\n\
Please send feedback or bug reports to " PACKAGE_BUGREPORT ".\n\
\n\
This program is free software; you can redistribute it and/or modify it under\n\
the terms of the GNU General Public License as published by the Free Software\n\
Foundation; either version 2, or (at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful, but WITHOUT\n\
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n\
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n\
You should have received a copy of the GNU General Public License along with\n\
this program (see the file COPYING); if not, write to the Free Software\n\
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n");
}

typedef enum
{
  MODE_SHOW_STATUS=0,
  MODE_DLOAD_LAYOUT,
  MODE_DLOAD_KEYMAP,
  MODE_UPLOAD_KEYMAP,
  MODE_DELETE_KEYMAP,
  MODE_RESET
} Progmode;

typedef struct
{
  Progmode mode;
  char *filename;
  ssize_t keyboard_index;
  int keymap_index;
} Options;

#define SET_MODE_ONCE(MODE)\
{\
  if(opts->mode != MODE_SHOW_STATUS)\
  {\
    fprintf(stderr,"Invalid combination of options. Try -h.\n");\
    return -1;\
  }\
  opts->mode=(MODE);\
}

static int commandline(int argc, char *argv[], Options *opts)
{
  opts->mode=MODE_SHOW_STATUS;
  opts->filename=NULL;
  opts->keyboard_index=-1;
  opts->keymap_index=-1;

  for(int ret=0; ret != -1; /* nothing */)
  {
    ret=getopt(argc,argv,"df:ghi:kln:rV");
    switch(ret)
    {
     case -1:
      break;
     case '?':
     case ':':
      fprintf(stderr,"Use -h for help.\n");
      return -1;
     case 'd':
      SET_MODE_ONCE(MODE_DELETE_KEYMAP);
      break;
     case 'f':
      opts->filename=optarg;
      break;
     case 'g':
      SET_MODE_ONCE(MODE_DLOAD_KEYMAP);
      break;
     case 'h':
      return 1;
     case 'i':
      opts->keymap_index=atoi(optarg);
      if(opts->keymap_index < 0)
      {
        fprintf(stderr,"Error: key map index cannot be negative.\n");
        return -1;
      }
      break;
     case 'k':
      SET_MODE_ONCE(MODE_UPLOAD_KEYMAP);
      break;
     case 'l':
      SET_MODE_ONCE(MODE_DLOAD_LAYOUT);
      break;
     case 'n':
      opts->keyboard_index=atoi(optarg);
      if(opts->keyboard_index <= 0)
      {
        fprintf(stderr,"Error: keyboard number must be greater than 0.\n");
        return -1;
      }
      break;
     case 'r':
      SET_MODE_ONCE(MODE_RESET);
      break;
     case 'V':
      return 2;
     default:
      fprintf(stderr,"Error parsing command line. Use -h for help.\n");
      return -1;
    }
  }

  if(optind < argc)
  {
    int multiple=(argc-optind > 1);
    fprintf(stderr,"Invalid option%s:",multiple?"s":"");
    if(multiple)
    {
      fputc('\n',stderr);
      while(optind < argc) fprintf(stderr,"  %s\n",argv[optind++]);
    }
    else fprintf(stderr," %s\n",argv[optind]);
    return -1;
  }

  switch(opts->mode)
  {
   case MODE_DLOAD_LAYOUT: case MODE_DLOAD_KEYMAP: case MODE_UPLOAD_KEYMAP:
    if(opts->filename == NULL)
    {
      fprintf(stderr,"No file name specified. Try -f.\n");
      return -1;
    }
    break;
   default:
    break;
  }

  switch(opts->mode)
  {
   case MODE_DLOAD_KEYMAP: case MODE_UPLOAD_KEYMAP: case MODE_DELETE_KEYMAP:
    if(opts->keymap_index < 0)
    {
      fprintf(stderr,"No key map specified. Try -i.\n");
      return -1;
    }
    break;
   default:
    break;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  Options opts;

  switch(commandline(argc,argv,&opts))
  {
   case 1:
    usage(argv[0]);
    return EXIT_SUCCESS;
   case 2:
    version_info();
    return EXIT_SUCCESS;
   case -1:
    return EXIT_FAILURE;
   default:
    break;
  }

  USBKeyboard kb;
  int ret;

  if(opts.keyboard_index > 0) --opts.keyboard_index;
  if((ret=kb_get_device(&kb,opts.keyboard_index)) != 0)
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
    int temp=-1;

    switch(opts.mode)
    {
     case MODE_SHOW_STATUS:
      temp=show_status(&kb,&info);
      break;
     case MODE_DLOAD_LAYOUT:
      temp=download_layout(&kb,&info,opts.filename);
      break;
     case MODE_DLOAD_KEYMAP:
      temp=download_keymap(&kb,&info,opts.filename,opts.keymap_index);
      break;
     case MODE_UPLOAD_KEYMAP:
      temp=upload_keymap(&kb,&info,opts.filename,opts.keymap_index);
      break;
     case MODE_DELETE_KEYMAP:
      temp=delete_keymap(&kb,&info,opts.keymap_index);
      break;
     case MODE_RESET:
      /* sending will fail, so we'll fake success */
      libusb_control_transfer(kb.handle,REQ_OUT,KURQ_RESET,0,0,NULL,0,5000);
      temp=0;
      try_close_device=0;
      break;
     default:
      usage(argv[0]);
      break;
    }

    if(temp == 0) ret=EXIT_SUCCESS;
  }

  if(try_close_device) kb_close_device(&kb);

  return ret;
}
