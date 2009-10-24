/*
 * Keyboard Upgrade -- Firmware for homebrew computer keyboard controllers.
 * Copyright (C) 2009  Robert Homann
 *
 * Based on RUMP (http://mg8.org/rump/), Copyright (C) 2008  Chris Lee
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

#include <avr/eeprom.h>

#include "usbdrv.h"
#include "usbrequests.h"

char PROGMEM usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH]=
{
  0x05, 0x01,   // USAGE_PAGE (Generic Desktop)
  0x09, 0x06,   // USAGE (Keyboard)
  0xa1, 0x01,   // COLLECTION (Application)
  0x05, 0x07,   //   USAGE_PAGE (Keyboard)
  0x19, 0xe0,   //   USAGE_MINIMUM (Keyboard LeftControl)
  0x29, 0xe7,   //   USAGE_MAXIMUM (Keyboard Right GUI)
  0x15, 0x00,   //   LOGICAL_MINIMUM (0)
  0x25, 0x01,   //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,   //   REPORT_SIZE (1)
  0x95, 0x08,   //   REPORT_COUNT (8)
  0x81, 0x02,   //   INPUT (Data,Var,Abs)
  0x95, 0x01,   //   REPORT_COUNT (1)
  0x75, 0x08,   //   REPORT_SIZE (8)
  0x81, 0x03,   //   INPUT (Cnst,Var,Abs)
  0x95, 0x05,   //   REPORT_COUNT (5)
  0x75, 0x01,   //   REPORT_SIZE (1)
  0x05, 0x08,   //   USAGE_PAGE (LEDs)
  0x19, 0x01,   //   USAGE_MINIMUM (Num Lock)
  0x29, 0x05,   //   USAGE_MAXIMUM (Kana)
  0x91, 0x02,   //   OUTPUT (Data,Var,Abs)
  0x95, 0x01,   //   REPORT_COUNT (1)
  0x75, 0x03,   //   REPORT_SIZE (3)
  0x91, 0x03,   //   OUTPUT (Cnst,Var,Abs)
  0x95, 0x06,   //   REPORT_COUNT (6)
  0x75, 0x08,   //   REPORT_SIZE (8)
  0x15, 0x00,   //   LOGICAL_MINIMUM (0)
  0x25, 0x65,   //   LOGICAL_MAXIMUM (101)
  0x05, 0x07,   //   USAGE_PAGE (Keyboard)
  0x19, 0x00,   //   USAGE_MINIMUM (Reserved (no event indicated))
  0x29, 0x65,   //   USAGE_MAXIMUM (Keyboard Application)
  0x81, 0x00,   //   INPUT (Data,Ary,Abs)
  0xc0          // END_COLLECTION
};

typedef struct
{
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} Report;

static Report usb_report_buffer;
static uchar usb_protocol_ver_buffer;
static uchar usb_idle_rate;
static uchar usb_expect_report;

static const uint8_t *copy_data_ptr;
static uint8_t        copy_data_len;

static usbMsgLen_t handle_request(const usbRequest_t *rq)
{
  static uint8_t buffer[8];

  usbMsgPtr=(void *)buffer;
  switch(rq->bRequest)
  {
   case KURQ_GET_HWINFO:
    ((KBHwinfo *)buffer)->max_mapindex=MAXIMUM_KEYMAP_INDEX;
    ((KBHwinfo *)buffer)->current_mapindex=0;
    ((KBHwinfo *)buffer)->num_of_keys=NUM_OF_KEYS;
    ((KBHwinfo *)buffer)->num_of_rows=NUM_OF_ROWS;
    ((KBHwinfo *)buffer)->num_of_cols=NUM_OF_COLUMNS;
    ((KBHwinfo *)buffer)->matrix_bvlen=MATRIX_BITVECTOR_LEN;
    return sizeof(KBHwinfo);
   case KURQ_GET_KEYMAP:
    if(rq->wValue.bytes[0] == 0)
    {
      usb_expect_report=KURQ_GET_DATA_FROM_PGM;
      copy_data_ptr=(const uint8_t *)&standard_stored_keymap;
    }
    else if(rq->wValue.bytes[0] <= MAXIMUM_KEYMAP_INDEX)
    {
      usb_expect_report=KURQ_GET_DATA_FROM_EEPROM;
      copy_data_ptr=(const uint8_t *)0+(rq->wValue.bytes[0]-1)*sizeof(Storedmap);
    }
    else return 0;
    copy_data_len=sizeof(Storedmap);
    return USB_NO_MSG;
   default:
    return 0;
  }
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  usbRequest_t *rq=(void *)data;
  usbMsgPtr=(void *)&usb_report_buffer;

  if((rq->bmRequestType&USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
  {
    if((rq->bmRequestType&USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR)
      return handle_request(rq);
    return 0;
  }

  switch(rq->bRequest)
  {
   case USBRQ_HID_GET_REPORT:
    return sizeof(usb_report_buffer);
   case USBRQ_HID_GET_IDLE:
    usbMsgPtr=&usb_idle_rate;
    return 1;
   case USBRQ_HID_GET_PROTOCOL:
    if(rq->wValue.bytes[1] < 1) usb_protocol_ver_buffer=rq->wValue.bytes[1];
    return 0;
   case USBRQ_HID_SET_REPORT:
    if(rq->wLength.word == 1)
    {
      usb_expect_report=USBRQ_HID_SET_REPORT;
      return USB_NO_MSG;
    }
    return 0;
   case USBRQ_HID_SET_IDLE:
    usb_idle_rate=rq->wValue.bytes[1];
    return 0;
   case USBRQ_HID_SET_PROTOCOL:
    usbMsgPtr=&usb_protocol_ver_buffer;
    return 1;
   default:
    return 0;
  }
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
  uchar ret=0;

#ifdef USB_SET_LED_STATE
  if(usb_expect_report == USBRQ_HID_SET_REPORT)
  {
    USB_SET_LED_STATE(data[0]);
    ret=1;
  }
#endif /* USB_SET_LED_STATE */
  usb_expect_report=0;
  return ret;
}

uchar usbFunctionRead(uchar *data, uchar len)
{
  uchar ret=0;

  switch(usb_expect_report)
  {
   case KURQ_GET_DATA_FROM_PGM:
   case KURQ_GET_DATA_FROM_EEPROM:
    if(copy_data_len < len) ret=copy_data_len;
    else                    ret=len;
    if(usb_expect_report == KURQ_GET_DATA_FROM_PGM)
      memcpy_P(data,copy_data_ptr,ret);
    else
      eeprom_read_block(data,copy_data_ptr,ret);
    copy_data_ptr+=ret;
    copy_data_len-=ret;
    if(copy_data_len > 0) return ret;
    break;
   default:
    break;
  }

  usb_expect_report=0;
  return ret;
}

#ifdef USB_SET_LED_STATE
#undef USB_SET_LED_STATE
#endif /* USB_SET_LED_STATE */
