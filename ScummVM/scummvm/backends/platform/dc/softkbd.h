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

#ifndef DC_SOFTKBD_H
#define DC_SOFTKBD_H

#include "label.h"

#define SK_NUM_KEYS 61

class OSystem_Dreamcast;

class SoftKeyboard : public Interactive
{
 private:

  const OSystem_Dreamcast *os;
  Label labels[2][SK_NUM_KEYS];
  byte shiftState;
  int8 keySel;

 public:
  SoftKeyboard(const OSystem_Dreamcast *os);

  void draw(float x, float y, int transp = 0);
  int key(int k, byte &shiftFlags);
  void mouse(int x, int y);
};

#endif /* DC_SOFTKBD_H */
