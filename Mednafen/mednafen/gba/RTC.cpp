// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005 Forgotten and the VBA development team

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

#include "../mednafen.h"
#include "GBA.h"
#include "Globals.h"
#include "Port.h"

#include <time.h>
#include <memory.h>

namespace MDFN_IEN_GBA
{

RTC::RTC()
{
 InitTime();
 Reset(); 
}

RTC::~RTC()
{

}

void RTC::InitTime(void)
{
 time_t long_time;

 time( &long_time );                /* Get time as long integer. */

 curtime = (int64)long_time * 16777216;
}

uint16 RTC::Read(uint32 address)
{
    if(address == 0x80000c8)
      return byte2;
    else if(address == 0x80000c6)
      return byte1;
    else if(address == 0x80000c4)
      return byte0;

 abort();
}

static uint8 toBCD(uint8 value)
{
  value = value % 100;
  int l = value % 10;
  int h = value / 10;
  return h * 16 + l;
}

void RTC::Write(uint32 address, uint16 value)
{
  if(address == 0x80000c8) {
    byte2 = (uint8)value; // enable ?
  } else if(address == 0x80000c6) {
    byte1 = (uint8)value; // read/write
  } else if(address == 0x80000c4) {
    if(byte2 & 1) {
      if(state == IDLE && byte0 == 1 && value == 5) {
          state = COMMAND;
          bits = 0;
          command = 0;
      } else if(!(byte0 & 1) && (value & 1)) { // bit transfer
        byte0 = (uint8)value;        
        switch(state) {
        case COMMAND:
          command |= ((value & 2) >> 1) << (7-bits);
          bits++;
          if(bits == 8) {
            bits = 0;
            switch(command) {
            case 0x60:
              // not sure what this command does but it doesn't take parameters
              // maybe it is a reset or stop
              state = IDLE;
              bits = 0;
              break;
            case 0x62:
              // this sets the control state but not sure what those values are
              state = READDATA;
              dataLen = 1;
              break;
            case 0x63:
              dataLen = 1;
              data[0] = 0x40;
              state = DATA;
              break;
           case 0x64:
              break;
            case 0x65:
              {
                struct tm *newtime;
                time_t long_time;

                long_time = curtime / 16777216;
                newtime = localtime( &long_time ); /* Convert to local time. */
                
                dataLen = 7;
                data[0] = toBCD(newtime->tm_year);
                data[1] = toBCD(newtime->tm_mon+1);
                data[2] = toBCD(newtime->tm_mday);
                data[3] = toBCD(newtime->tm_wday);
                data[4] = toBCD(newtime->tm_hour);
                data[5] = toBCD(newtime->tm_min);
                data[6] = toBCD(newtime->tm_sec);
                state = DATA;
              }
              break;              
            case 0x67:
              {
                struct tm *newtime;
                time_t long_time;

		long_time = curtime / 16777216;
                newtime = localtime( &long_time ); /* Convert to local time. */
                
                dataLen = 3;
                data[0] = toBCD(newtime->tm_hour);
                data[1] = toBCD(newtime->tm_min);
                data[2] = toBCD(newtime->tm_sec);
                state = DATA;
              }
              break;
            default:
              //systemMessage(0, N_("Unknown RTC command %02x"), command);
              state = IDLE;
              break;
            }
          }
          break;
        case DATA:
          if(byte1 & 2) {
          } else {
            byte0 = (byte0 & ~2) |
              ((data[bits >> 3] >>
                (bits & 7)) & 1)*2;
            bits++;
            if(bits == 8*dataLen) {
              bits = 0;
              state = IDLE;
            }
          }
          break;
        case READDATA:
          if(!(byte1 & 2)) {
          } else {
            data[bits >> 3] =
              (data[bits >> 3] >> 1) |
              ((value << 6) & 128);
            bits++;
            if(bits == 8*dataLen) {
              bits = 0;
              state = IDLE;
            }
          }
          break;
		default:
          break;
        }
      } else
        byte0 = (uint8)value;
    }
  }
}

void RTC::Reset(void)
{
 byte0 = 0;
 byte1 = 0;
 byte2 = 0;
 command = 0;
 dataLen = 0;
 bits = 0;
 state = IDLE;

 memset(data, 0, sizeof(data));
}

int RTC::StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFVAR(byte0),
  SFVAR(byte1),
  SFVAR(byte2),
  SFVAR(command),
  SFVAR(dataLen),
  SFVAR(bits),
  SFVAR(state),
  SFARRAY(data, 12),
  SFEND
 };


 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "RTC");

 if(load)
 {

 }

 return(ret);
}

}
