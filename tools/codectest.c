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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usbkeycodes.h"
#include "keyboard.h"
#include "stdmap.h"
#include "stdstoredmap.h"
#include "keymapdecoder.h"

#define ZERO_CODES_ARE_TRASH_CODES 1

static Columnstate column_valid_mask[NUM_OF_ROWS];

#include "keymapencode.c"
#include "keymapdecode.c"

static void print_square(const char *what, const Map *const map)
{
  printf("%s:\n",what);
  for(size_t y=0; y < NUM_OF_ROWS; ++y)
  {
    for(size_t x=0; x < NUM_OF_COLUMNS; ++x)
    {
      printf(" %02x",map->mat[y][x]);
    }
  }
  putchar('\n');
}

static void print_stored(const char *what, const Storedmap *const stored)
{
  printf("%s:\n",what);
  for(size_t i=0; i < NUM_OF_KEYS; ++i)
  {
    printf(" %02x",stored->codes[i]);
  }
  putchar('\n');
}

static void print_masks(const char *what, const Columnstate colmask[NUM_OF_ROWS])
{
  printf("%s:\n",what);
  for(size_t i=0; i < NUM_OF_ROWS; ++i)
  {
    printf(" %02hhx",colmask[i]);
  }
  putchar('\n');
}

static void make_column_masks(Columnstate colmask[NUM_OF_ROWS],
                              const Map *const map)
{
  for(uint8_t row=0; row < NUM_OF_ROWS; ++row)
  {
    colmask[row]=COLUMNSTATE_EMPTY;
    for(uint8_t col=0; col < NUM_OF_COLUMNS; ++col)
    {
      if((map->mat[row][col]) != 0) colmask[row]&=~(1 << col);
    }
  }
}

int main(void)
{
  Map       map_dest;
  Storedmap stored_dest;

  print_square("Converting square map",&keymap);
  encode(&keymap,&stored_dest);
  print_stored("Got stored map",&stored_dest);
  if(memcmp(stored_map.codes,stored_dest.codes,NUM_OF_KEYS) != 0)
  {
    print_stored("Should have been",&stored_map);
    fprintf(stdout,"Error: Encoder doesn't work.\n");
    return EXIT_FAILURE;
  }

  print_stored("Converting stored map",&stored_map);
  decode(&map_dest,stored_map.codes);
  print_square("Got square map",&map_dest);
  if(memcmp(keymap.mat,map_dest.mat,NUM_OF_ROWS*NUM_OF_COLUMNS) != 0)
  {
    print_square("Should have been",&keymap);
    fprintf(stdout,"Error: Decoder doesn't work.\n");
    return EXIT_FAILURE;
  }


  Columnstate correct_masks[NUM_OF_ROWS];

  make_column_masks(correct_masks,&keymap);
  print_masks("Expected column masks",correct_masks);
  if(memcmp(correct_masks,column_valid_mask,sizeof(correct_masks)) != 0)
  {
    print_masks("Got masks",column_valid_mask);
    fprintf(stdout,"Error: Mask construction doesn't work.\n");
    return EXIT_FAILURE;
  }

  printf("OK.\n");
  return EXIT_SUCCESS;
}
