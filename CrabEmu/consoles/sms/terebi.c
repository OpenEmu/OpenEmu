/*
    This file is part of CrabEmu.

    Copyright (C) 2008 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "terebi.h"
#include "smsmem.h"

static uint8 terebi_x;
static uint8 terebi_y;
static int terebi_flags;

extern uint8 *sms_read_map[256];
extern uint8 *sms_write_map[256];

/* This function is... well, not pretty. Apparently the Terebi Oekaki isn't all
   that accurate in actuality. Many thanks to Omar Cornut of MEKA fame for
   figuring out how to actually calculate what should be sent to the game.
   (The odd math in here is adapted from MEKA's TVOekaki_Update function) */
void terebi_update(int x, int y, int pressed)   {
    x -= 5;
    y -= 4;

    if(x < 0)
        x = 0;
    else if(x > 255)
        x = 255;

    if(y < 0)
        y = 0;
    else if(y > 191)
        y = 191;

    y += 32;
    
    terebi_x = (uint8)x;
    terebi_y = (uint8)y;

    if(pressed) {
        terebi_flags |= TEREBI_OEKAKI_PRESSED;
    }
    else    {
        terebi_flags &= ~TEREBI_OEKAKI_PRESSED;
    }
}

uint8 terebi_mread(uint16 addr) {
    if(addr == 0x8000)  {
        return (terebi_flags & TEREBI_OEKAKI_PRESSED) ? 0 : 1;
    }
    else if(addr == 0xA000)     {
        if((terebi_flags & TEREBI_OEKAKI_PRESSED)) {
            return (terebi_flags & TEREBI_OEKAKI_AXIS_Y) ? terebi_x : terebi_y;
        }
        else    {
            return 0;
        }
    }

    return sms_read_map[addr >> 8][addr & 0xFF];
}

void terebi_mwrite(uint16 addr, uint8 data) {
    if(addr == 0x6000)  {
        if(data & 0x01) {
            terebi_flags |= TEREBI_OEKAKI_AXIS_Y;
        }
        else    {
            terebi_flags &= ~TEREBI_OEKAKI_AXIS_Y;
        }
    }

    sms_write_map[addr >> 8][addr & 0xFF] = data;
}

uint16 terebi_mread16(uint16 addr)  {
    uint16 rv;
    int top, bot;

    if(addr == 0x8000)  {
        return (uint16)(terebi_flags & TEREBI_OEKAKI_PRESSED);
    }
    else if(addr == 0xA000)     {
        if((terebi_flags & TEREBI_OEKAKI_PRESSED)) {
            return (terebi_flags & TEREBI_OEKAKI_AXIS_Y) ? terebi_y : terebi_x;
        }
        else    {
            return 0;
        }
    }

    top = addr >> 8;
    bot = addr & 0xFF;

    rv = sms_read_map[top][bot++];

    if(bot <= 0xFF) {
        return rv | (sms_read_map[top][bot] << 8);
    }
    else    {
        return rv | (sms_read_map[(uint8)(top + 1)][0] << 8);
    }
}

void terebi_mwrite16(uint16 addr, uint16 data)  {
    int top, bot;

    if(addr == 0x6000)  {
        if(data & 0x01) {
            terebi_flags |= TEREBI_OEKAKI_AXIS_Y;
        }
        else    {
            terebi_flags &= ~TEREBI_OEKAKI_AXIS_Y;
        }
    }

    top = addr >> 8;
    bot = addr & 0xFF;

    sms_write_map[top][bot++] = (uint8)data;
    
    if(bot <= 0xFF)
        sms_write_map[top][bot] = (uint8)(data >> 8);
    else
        sms_write_map[(uint8)(top + 1)][0] = (uint8)(data >> 8);
}
