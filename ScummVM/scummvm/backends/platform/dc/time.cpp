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

#define RONIN_TIMER_ACCESS

#include <common/scummsys.h>
#include "dc.h"


uint32 OSystem_Dreamcast::getMillis()
{
  static uint32 msecs=0;
  static unsigned int t0=0;

  unsigned int t = Timer();
  unsigned int dm, dt = t - t0;

  t0 = t;
  dm = (dt << 6)/3125U;
  dt -= (dm * 3125U)>>6;
  t0 -= dt;

  return msecs += dm;
}

void OSystem_Dreamcast::delayMillis(uint msecs)
{
  getMillis();
  unsigned int t, start = Timer();
  int time = (((unsigned int)msecs)*3125U)>>6;
  while (((int)((t = Timer())-start))<time) {
    if (_timerManager != NULL)
      ((DefaultTimerManager *)_timerManager)->handler();
    checkSound();
  }
  getMillis();
}
