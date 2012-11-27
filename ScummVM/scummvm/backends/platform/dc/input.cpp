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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#define RONIN_TIMER_ACCESS

#include <common/scummsys.h>
#include <common/events.h>
#include "dc.h"

int handleInput(struct mapledev *pad, int &mouse_x, int &mouse_y,
		byte &shiftFlags, Interactive *inter)
{
  static const char numpadmap[] = "0000039601740285";
  int lmb=0, rmb=0, newkey=0;
  static int lastkey = 0;
  static byte lastlmb = 0, lastrmb = 0;
  static int8 mouse_wheel = 0, lastwheel = 0;
  shiftFlags = 0;
  for (int i=0; i<4; i++, pad++)
    if (pad->func & MAPLE_FUNC_CONTROLLER) {
      int buttons = pad->cond.controller.buttons;

      if (!(buttons & 0x060e)) exit(0);

      if (!(buttons & 4)) lmb++;
      if (!(buttons & 2)) rmb++;

      if (!(buttons & 8)) newkey = Common::KEYCODE_F5;
      else if (!(buttons & 512)) newkey = ' ';
      else if (!(buttons & 1024)) newkey = numpadmap[(buttons>>4)&15];

      if (!(buttons & 128)) if (inter) newkey = 1001; else mouse_x++;
      if (!(buttons & 64)) if (inter) newkey = 1002; else mouse_x--;
      if (!(buttons & 32)) if (inter) newkey = 1003; else mouse_y++;
      if (!(buttons & 16)) if (inter) newkey = 1004; else mouse_y--;

      mouse_x += ((int)pad->cond.controller.joyx-128)>>4;
      mouse_y += ((int)pad->cond.controller.joyy-128)>>4;

      if (pad->cond.controller.ltrigger > 200) newkey = 1005;
      else if (pad->cond.controller.rtrigger > 200) newkey = 1006;

    } else if (pad->func & MAPLE_FUNC_MOUSE) {
      int buttons = pad->cond.mouse.buttons;

      if (!(buttons & 4)) lmb++;
      if (!(buttons & 2)) rmb++;

      if (!(buttons & 8)) newkey = Common::KEYCODE_F5;

      mouse_x += pad->cond.mouse.axis1;
      mouse_y += pad->cond.mouse.axis2;
      mouse_wheel += pad->cond.mouse.axis3;

      if (inter)
	inter->mouse(mouse_x, mouse_y);

      pad->cond.mouse.axis1 = 0;
      pad->cond.mouse.axis2 = 0;
      pad->cond.mouse.axis3 = 0;
    } else if (pad->func & MAPLE_FUNC_KEYBOARD) {
      for (int p=0; p<6; p++) {
	int shift = pad->cond.kbd.shift;
	int key = pad->cond.kbd.key[p];
	if (shift & 0x08) lmb++;
	if (shift & 0x80) rmb++;
	if (shift & 0x11) shiftFlags |= Common::KBD_CTRL;
	if (shift & 0x44) shiftFlags |= Common::KBD_ALT;
	if (shift & 0x22) shiftFlags |= Common::KBD_SHIFT;
	if (key >= 4 && key <= 0x1d)
	  newkey = key+('a'-4);
	else if (key >= 0x1e && key <= 0x26)
	  newkey = key+((shift & 0x22)? ('!'-0x1e) : ('1'-0x1e));
	else if (key >= 0x59 && key <= 0x61)
	  newkey = key+('1'-0x59);
	else if (key >= 0x2d && key <= 0x38 && key != 0x31)
	  newkey = ((shift & 0x22)?
		    "=?`{ }+*?<>?" :
		    "-^@[ ];:?,./")[key - 0x2d];
	else if (key >= 0x3a && key <= 0x43)
	  newkey = key+(Common::KEYCODE_F1-0x3a);
	else if (key >= 0x54 && key <= 0x57)
	  newkey = "/*-+"[key-0x54];
	else switch(key) {
	case 0x27: case 0x62:
	  newkey = ((shift & 0x22)? '~' : '0'); break;
	case 0x28: case 0x58:
	  newkey = Common::KEYCODE_RETURN; break;
	case 0x29:
	  newkey = Common::KEYCODE_ESCAPE; break;
	case 0x2a:
	  newkey = Common::KEYCODE_BACKSPACE; break;
	case 0x2b:
	  newkey = Common::KEYCODE_TAB; break;
	case 0x2c:
	  newkey = Common::KEYCODE_SPACE; break;
	case 0x4c:
	  if ((shift & 0x11) && (shift & 0x44))
	    exit(0);
	  break;
	case 0x4f:
	  if (inter) newkey = 1001; else mouse_x++; break;
	case 0x50:
	  if (inter) newkey = 1002; else mouse_x--; break;
	case 0x51:
	  if (inter) newkey = 1003; else mouse_y++; break;
	case 0x52:
	  if (inter) newkey = 1004; else mouse_y--; break;
	case 0x63:
	  newkey = '.'; break;
	case 0x64: case 0x87:
	  newkey = ((shift & 0x22)? '_' : '\\'); break;
	case 0x89:
	  newkey = ((shift & 0x22)? '|' : '?'); break;
	}
      }
    }

  if (lmb && inter && !lastlmb) {
    newkey = 1000;
    lmb = 0;
  }

  if (lmb && !lastlmb) {
    lastlmb = 1;
    return -Common::EVENT_LBUTTONDOWN;
  } else if (lastlmb && !lmb) {
    lastlmb = 0;
    return -Common::EVENT_LBUTTONUP;
  }
  if (rmb && !lastrmb) {
    lastrmb = 1;
    return -Common::EVENT_RBUTTONDOWN;
  } else if (lastrmb && !rmb) {
    lastrmb = 0;
    return -Common::EVENT_RBUTTONUP;
  }

  if (mouse_wheel != lastwheel)
    if (((int8)(mouse_wheel - lastwheel)) > 0) {
      lastwheel++;
      return -Common::EVENT_WHEELDOWN;
    } else {
      --lastwheel;
      return -Common::EVENT_WHEELUP;
    }

  if (newkey && inter && newkey != lastkey) {
    int transkey = inter->key(newkey, shiftFlags);
    if (transkey) {
      newkey = transkey;
      inter = NULL;
    }
  }

  if (!newkey || (lastkey && newkey != lastkey)) {
    int upkey = lastkey;
    lastkey = 0;
    if (upkey)
      return upkey | (1<<30);
  } else if (!lastkey) {
    lastkey = newkey;
    if (newkey >= 1000 || !inter)
      return newkey;
  }

  return 0;
}

bool OSystem_Dreamcast::pollEvent(Common::Event &event)
{
  unsigned int t = Timer();

  if (_timerManager != NULL)
    ((DefaultTimerManager *)_timerManager)->handler();

  if (((int)(t-_devpoll))<0)
    return false;
  _devpoll += USEC_TO_TIMER(17000);
  if (((int)(t-_devpoll))>=0)
    _devpoll = t + USEC_TO_TIMER(17000);

  maybeRefreshScreen();

  int mask = getimask();
  setimask(15);
  checkSound();
  int e = handleInput(locked_get_pads(), _ms_cur_x, _ms_cur_y,
		      event.kbd.flags, (_softkbd_on? &_softkbd : NULL));
  setimask(mask);
  if (_ms_cur_x<0) _ms_cur_x=0;
  if (_ms_cur_x>=_screen_w) _ms_cur_x=_screen_w-1;
  if (_ms_cur_y<0) _ms_cur_y=0;
  if (_ms_cur_y>=_screen_h) _ms_cur_y=_screen_h-1;
  event.mouse.x = _ms_cur_x;
  event.mouse.y = _ms_cur_y;
  if (_overlay_visible) {
    event.mouse.x -= _overlay_x;
    event.mouse.y -= _overlay_y;
  }
  event.kbd.ascii = 0;
  event.kbd.keycode = Common::KEYCODE_INVALID;
  if (e<0) {
    event.type = (Common::EventType)-e;
    return true;
  } else if (e>0) {
    bool processed = false, down = !(e&(1<<30));
    e &= ~(1<<30);
    if (e < 1000) {
      event.type = (down? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP);
      event.kbd.keycode = (Common::KeyCode)e;
      event.kbd.ascii = (e>='a' && e<='z' && (event.kbd.flags & Common::KBD_SHIFT)?
			  e &~ 0x20 : e);
      processed = true;
    } else if (down) {
      if (e == 1005)
	setFeatureState(kFeatureVirtualKeyboard,
			!getFeatureState(kFeatureVirtualKeyboard));
    }
    return processed;
  } else if (_ms_cur_x != _ms_old_x || _ms_cur_y != _ms_old_y) {
    event.type = Common::EVENT_MOUSEMOVE;
    _ms_old_x = _ms_cur_x;
    _ms_old_y = _ms_cur_y;
    return true;
  } else {
    event.type = (Common::EventType)0;
    return false;
  }
}
