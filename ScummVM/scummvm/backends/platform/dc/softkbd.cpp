/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <common/scummsys.h>
#include "common/events.h"
#include "engines/engine.h"

#include <ronin/ronin.h>
#include <string.h>
#include <assert.h>

#include "dc.h"


extern void draw_trans_quad(float x1, float y1, float x2, float y2,
			    int c0, int c1, int c2, int c3);


static const char key_names[] =
  "Esc\0F1\0F2\0F3\0F4\0F5\0F6\0F7\0F8\0F9\0F10\0"
  "1\0!\0""2\0\"\0""3\0#\0""4\0$\0""5\0%\0"
  "6\0&\0""7\0'\0""8\0(\0""9\0)\0""0\0~\0-\0=\0"
  "q\0Q\0w\0W\0e\0E\0r\0R\0t\0T\0y\0Y\0u\0U\0i\0I\0o\0O\0p\0P\0@\0`\0"
  "a\0A\0s\0S\0d\0D\0f\0F\0g\0G\0h\0H\0j\0J\0k\0K\0l\0L\0;\0+\0:\0*\0"
  "z\0Z\0x\0X\0c\0C\0v\0V\0b\0B\0n\0N\0m\0M\0,\0<\0.\0>\0/\0?\0\\\0_\0"
  "Shf\0Ctl\0Alt\0Space\0BS\0Ret";

#define K(a,b) ((a)|((b)<<8))

static const short key_codes[] =
  {
    Common::KEYCODE_ESCAPE, Common::KEYCODE_F1, Common::KEYCODE_F2, Common::KEYCODE_F3, Common::KEYCODE_F4, Common::KEYCODE_F5, Common::KEYCODE_F6, Common::KEYCODE_F7, Common::KEYCODE_F8, Common::KEYCODE_F9, Common::KEYCODE_F10,
    K('1','!'), K('2','"'), K('3','#'), K('4','$'), K('5','%'),
    K('6','&'), K('7','\''), K('8','('), K('9',')'), K('0','~'), K('-','='),
    K('q','Q'), K('w','W'), K('e','E'), K('r','R'), K('t','T'),
    K('y','Y'), K('u','U'), K('i','I'), K('o','O'), K('p','P'), K('@','`'),
    K('a','A'), K('s','S'), K('d','D'), K('f','F'), K('g','G'),
    K('h','H'), K('j','J'), K('k','K'), K('l','L'), K(';','+'), K(':','*'),
    K('z','Z'), K('x','X'), K('c','C'), K('v','V'), K('b','B'),
    K('n','N'), K('m','M'), K(',','<'), K('.','>'), K('/','?'), K('\\','_'),
    ~Common::KBD_SHIFT, ~Common::KBD_CTRL, ~Common::KBD_ALT, ' ', Common::KEYCODE_BACKSPACE, Common::KEYCODE_RETURN
  };

SoftKeyboard::SoftKeyboard(const OSystem_Dreamcast *_os)
  : os(_os), shiftState(0), keySel(0)
{
  assert((sizeof(key_codes)/sizeof(key_codes[0])) == SK_NUM_KEYS);

  const char *np = key_names;
  for (int i=0; i<SK_NUM_KEYS; i++) {
    labels[0][i].create_texture(np);
    np += strlen(np)+1;
    if (key_codes[i]>8192) {
      labels[1][i].create_texture(np);
      np += strlen(np)+1;
    }
  }
}

void SoftKeyboard::draw(float x, float y, int transp)
{
  float x0;
  int c = 0;
  unsigned int txt_alpha_mask = (255-2*transp)<<24;
  unsigned int bg_alpha_mask = (128-transp)<<24;

  draw_trans_quad(x, y, x+312.0, y+172.0,
		  bg_alpha_mask|0x8080ff, bg_alpha_mask|0x8080ff,
		  bg_alpha_mask|0x8080ff, bg_alpha_mask|0x8080ff);
  x0 = x += 4.0;
  y += 4.0;
  for (int i=0; i<SK_NUM_KEYS; i++) {
    float w = (i == 58? 164.0 : 24.0);
    unsigned int bg = (i == keySel? bg_alpha_mask|0xffff00 :
		       bg_alpha_mask|0xc0c0ff);
    draw_trans_quad(x, y, x+w, y+24.0, bg, bg, bg, bg);
    if (key_codes[i]<0 && (shiftState & ~key_codes[i]))
      labels[0][i].draw(x+2, y+5, txt_alpha_mask|0xffffff, 0.5);
    else if (key_codes[i]>8192 && (shiftState & Common::KBD_SHIFT))
      labels[1][i].draw(x+2, y+5, txt_alpha_mask|0x000000, 0.5);
    else
      labels[0][i].draw(x+2, y+5, txt_alpha_mask|0x000000, 0.5);
    x += w+4.0;
    if (++c == 11) {
      c = 0;
      x = x0;
      y += 28.0;
    }
  }
}

int SoftKeyboard::key(int k, byte &shiftFlags)
{
  switch(k) {
  case 1001:
    if (++keySel == SK_NUM_KEYS)
      keySel = 0;
    break;
  case 1002:
    if (--keySel < 0)
      keySel = SK_NUM_KEYS - 1;
    break;
  case 1003:
    if (keySel >= 55) {
      if (keySel > 58)
	keySel += 5;
      keySel -= 55;
    } else if (keySel > 47) {
      if ((keySel += 6) < 59)
	keySel = 59;
    } else
      keySel += 11;
    break;
  case 1004:
    if (keySel > 58)
      keySel -= 6;
    else if ((keySel -= 11) < 0)
      if ((keySel += 66) > 58)
	if ((keySel -= 5) < 59)
	  keySel = 59;
    break;
  case 1000:
  case 13:
  case 32:
  case 319:
    if (key_codes[keySel]<0)
      shiftState ^= ~key_codes[keySel];
    else {
      shiftFlags = shiftState;
      if (key_codes[keySel] > 8192)
	return ((shiftState & Common::KBD_SHIFT)? (key_codes[keySel]>>8):
		key_codes[keySel]) & 0xff;
      else
	return key_codes[keySel];
    }
    break;
  }
  return 0;
}

void SoftKeyboard::mouse(int x, int y)
{
  os->mouseToSoftKbd(x, y, x, y);
  if (x >= 0 && x < 11*28 && y >= 0 && y < 6*28 &&
     x%28 >= 4 && y%28 >= 4)
    if ((keySel = 11*(y/28)+(x/28)) > 58)
      if ((keySel -= 5) < 59)
	keySel = 58;
}
