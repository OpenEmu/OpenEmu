// -*- C++ -*-
// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef VBA_RTC_H
#define VBA_RTC_H

namespace MDFN_IEN_GBA
{

class RTC
{
 public:

 RTC();
 ~RTC();

 void InitTime(void);

 uint16 Read(uint32 address);
 void Write(uint32 address, uint16 value);
 void Reset(void);

 int StateAction(StateMem *sm, int load, int data_only);

 private:

 enum RTCSTATE { IDLE, COMMAND, DATA, READDATA };

 uint8 byte0;
 uint8 byte1;
 uint8 byte2;
 uint8 command;
 int dataLen;
 int bits;
 RTCSTATE state;
 uint8 data[12];

 int64 curtime;        // GBA CPU cycles since the Epoch, in local time.

};

}

using namespace MDFN_IEN_GBA;
#endif
