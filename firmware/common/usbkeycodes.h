/*
 * Keyboard Upgrade -- Firmware for homebrew computer keyboard controllers.
 * Copyright (C) 2009  Robert Homann
 *
 * Based on RUMP (http://mg8.org/rump/), Copyright (C) 2008  Chris Lee
 *
 * Based on c64key (http://symlink.dk/projects/c64key/),
 * Copyright (C) 2006-2007  Mikkel Holm Olsen
 *
 * Based on HID-Test by Christian Starkjohann, Objective Development
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

#ifndef USBKEYCODES_H
#define USBKEYCODES_H

/*
 * The USB keycodes are enumerated here - the first part is simply
 * an enumeration of the allowed scan-codes used for USB HID devices.
 */
enum keycodes {
  KEY__=0,
  KEY_errorRollOver,
  KEY_POSTfail,
  KEY_errorUndefined,
  KEY_A,        /* 4 */
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G,
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,        /* 0x10 */
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q,
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z,
  KEY_1,
  KEY_2,
  KEY_3,        /* 0x20 */
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
  KEY_0,        /* 0x27 */
  KEY_enter,
  KEY_esc,
  KEY_bckspc,   /* backspace */
  KEY_tab,
  KEY_spc,      /* space */
  KEY_minus,    /* - (and _) */
  KEY_equal,    /* = (and +) */
  KEY_lbr,      /* [ */
  KEY_rbr,      /* ]  -- 0x30 */
  KEY_bckslsh,  /* \ (and |) */
  KEY_hash,     /* Non-US # and ~ */
  KEY_smcol,    /* ; (and :) */
  KEY_ping,     /* ' and " */
  KEY_grave,    /* Grave accent and tilde */
  KEY_comma,    /* , (and <) */
  KEY_dot,      /* . (and >) */
  KEY_slash,    /* / (and ?) */
  KEY_cpslck,   /* capslock */
  KEY_F1,
  KEY_F2,
  KEY_F3,
  KEY_F4,
  KEY_F5,
  KEY_F6,
  KEY_F7,       /* 0x40 */
  KEY_F8,
  KEY_F9,
  KEY_F10,
  KEY_F11,
  KEY_F12,
  KEY_PrtScr,
  KEY_scrlck,
  KEY_break,
  KEY_ins,
  KEY_home,
  KEY_pgup,
  KEY_del,
  KEY_end,
  KEY_pgdn,
  KEY_rarr,
  KEY_larr,     /* 0x50 */
  KEY_darr,
  KEY_uarr,
  KEY_numlock,
  KEY_KPslash,
  KEY_KPast,
  KEY_KPminus,
  KEY_KPplus,
  KEY_KPenter,
  KEY_KP1,
  KEY_KP2,
  KEY_KP3,
  KEY_KP4,
  KEY_KP5,
  KEY_KP6,
  KEY_KP7,
  KEY_KP8,      /* 0x60 */
  KEY_KP9,
  KEY_KP0,
  KEY_KPdot,
  KEY_Euro,     /* Non-US \ and | */
  KEY_Application,
  KEY_Power,
  KEY_KPequal,
  KEY_F13,
  KEY_F14,
  KEY_F15,
  KEY_F16,
  KEY_F17,
  KEY_F18,
  KEY_F19,
  KEY_F20,
  KEY_F21,      /* 0x70 */
  KEY_F22,
  KEY_F23,
  KEY_F24,
  KEY_Execute,
  KEY_Help,
  KEY_Menu,
  KEY_Select,
  KEY_Stop,
  KEY_Again,
  KEY_Undo,
  KEY_Cut,
  KEY_Copy,
  KEY_Paste,
  KEY_Find,
  KEY_Mute,
  KEY_VolUp,    /* 0x80 */
  KEY_VolDn,
  KEY_lcpslck,  /* locking Caps Lock */
  KEY_lnumlock, /* locking Num Lock */
  KEY_lscrlck,  /* locking Scroll Lock */
  KEY_KPcomma,
  KEY_KPeq400,  /* equal sign on AS/400 */
  KEY_Int1,
  KEY_Int2,
  KEY_Int3,
  KEY_Int4,
  KEY_Int5,
  KEY_Int6,
  KEY_Int7,
  KEY_Int8,
  KEY_Int9,
  KEY_Lang1,    /* 0x90 */
  KEY_Lang2,
  KEY_Lang3,
  KEY_Lang4,
  KEY_Lang5,
  KEY_Lang6,
  KEY_Lang7,
  KEY_Lang8,
  KEY_Lang9,
  KEY_AltEr,
  KEY_SysReq,
  KEY_Cancel,
  KEY_Clear,
  KEY_Priot,
  KEY_Return,
  KEY_Sep,
  KEY_Out,      /* 0xa0 */
  KEY_Oper,
  KEY_ClrAgn,
  KEY_CrSel,
  KEY_ExSel,

  KEY_KP00=0xb0,
  KEY_KP000,
  KEY_ThouSep,
  KEY_DecSep,
  CurrUnit,
  CurrSub,
  KPlparen,
  KPrparen,
  KPlcurl,
  KPrcurl,
  KPtab,
  KPbckspc,
  KEY_KPA,
  KEY_KPB,
  KEY_KPC,
  KEY_KPD,
  KEY_KPE,      /* 0xc0 */
  KEY_KPF,
  KEY_KPxor,
  KEY_KPhat,
  KEY_KPperc,
  KEY_KPless,
  KEY_KPgt,
  KEY_KPand,
  KEY_KPlazyand,
  KEY_KPor,
  KEY_KPlazyor,
  KEY_KPcolon,
  KEY_KPhash,
  KEY_KPspc,
  KEY_KPat,
  KEY_KPexcl,
  KEY_KPmemsto, /* 0xd0 */
  KEY_KPmemrcl,
  KEY_KPmemclr,
  KEY_KPmemadd,
  KEY_KPmemsub,
  KEY_KPmemmul,
  KEY_KPmemdiv,
  KEY_KPplmi,
  KEY_KPclr,
  KEY_KPclrentry,
  KEY_KPbin,
  KEY_KPoct,
  KEY_KPdec,
  KEY_KPhex,

  /*
   * These are NOT standard USB HID - handled specially in decoding,
   * so they will be mapped to the modifier byte in the USB report.
   */
  NOKEY_Modifiers=0xe0,
  MOD_LCTRL=0xe0,             /* 0x01 */
  MOD_LSHIFT,                 /* 0x02 */
  MOD_LALT,                   /* 0x04 */
  MOD_LGUI,                   /* 0x08 */
  MOD_RCTRL,                  /* 0x10 */
  MOD_RSHIFT,                 /* 0x20 */
  MOD_RALT,                   /* 0x40 */
  MOD_RGUI,                   /* 0x80 */

  /* This pseudo key is used to deactivate real keys in mappings. */
  KEY_trash
};

#endif /* USBKEYCODES_H */
