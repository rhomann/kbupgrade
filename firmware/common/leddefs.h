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

#ifndef LEDDEFS_H
#define LEDDEFS_H

#ifdef LED_NUM_PIN
#define LED_NUM     0x01
#endif /* LED_NUM_PIN */

#ifdef LED_CAPS_PIN
#define LED_CAPS    0x02
#endif /* LED_CAPS_PIN */

#ifdef LED_SCROLL_PIN
#define LED_SCROLL  0x04
#endif /* LED_SCROLL_PIN */

#ifdef LED_COMPOSE_PIN
#define LED_COMPOSE 0x08
#endif /* LED_COMPOSE_PIN */

#ifdef LED_KANA_PIN
#define LED_KANA    0x10
#endif /* LED_KANA_PIN */

#define LED_ONOFF(WHICH,STATE)\
  if((STATE)&(LED_##WHICH)) (LED_PORT)&=~_BV(LED_##WHICH##_PIN);\
  else                      (LED_PORT)|=_BV(LED_##WHICH##_PIN)

static void set_led_state(uint8_t state)
{
#ifdef LED_NUM_PIN
  LED_ONOFF(NUM,state);
#endif /* LED_NUM_PIN */

#ifdef LED_CAPS_PIN
  LED_ONOFF(CAPS,state);
#endif /* LED_CAPS_PIN */

#ifdef LED_SCROLL_PIN
  LED_ONOFF(SCROLL,state);
#endif /* LED_SCROLL_PIN */

#ifdef LED_COMPOSE_PIN
  LED_ONOFF(COMPOSE,state);
#endif /* LED_COMPOSE_PIN */

#ifdef LED_KANA_PIN
  LED_ONOFF(KANA,state);
#endif /* LED_KANA_PIN */
}

#endif /* !LEDDEFS_H */
