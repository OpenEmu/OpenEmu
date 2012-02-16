/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#include <stdlib.h>
#include "share.h"

typedef struct {
  int32 mzx, mzy, mzxold, mzyold;
  uint32 readbit;
  uint32 data;
} MOUSE;

static MOUSE Mouse;

static void StrobeMOUSE(int w)
{
	Mouse.readbit=0;
	if((Mouse.mzxold-Mouse.mzx)>0)
		Mouse.data|=0x0C;
	else if((Mouse.mzxold-Mouse.mzx)<0)
		Mouse.data|=0x04;
	if((Mouse.mzyold-Mouse.mzy)>0)
		Mouse.data|=0x30;
	else if((Mouse.mzyold-Mouse.mzy)<0)
		Mouse.data|=0x10;
}

static uint8 ReadMOUSE(int w)
{
	uint8 ret=0;
	if(Mouse.readbit>=8)
		ret|=1;
	else
	{
		ret|=(Mouse.data>>Mouse.readbit)&1;
		Mouse.readbit++;
	}
	return(ret);
}

static void UpdateMOUSE(int w, void *data, int arg)
{
  uint32 *ptr=(uint32*)data;
  Mouse.data=0;
  Mouse.mzxold=Mouse.mzx;
  Mouse.mzyold=Mouse.mzy;
  Mouse.mzx=ptr[0];
  Mouse.mzy=ptr[1];
  Mouse.data|=ptr[2];
  if((Mouse.mzxold-Mouse.mzx)>0)
    Mouse.data|=0x0C;
  else if((Mouse.mzxold-Mouse.mzx)<0)
    Mouse.data|=0x04;
  if((Mouse.mzyold-Mouse.mzy)>0)
    Mouse.data|=0x30;
  else if((Mouse.mzyold-Mouse.mzy)<0)
    Mouse.data|=0x10;
}

static INPUTC MOUSEC={ReadMOUSE,0,StrobeMOUSE,UpdateMOUSE,0,0};

INPUTC *FCEU_InitMouse(int w)
{
  Mouse.mzx=0;
  Mouse.mzy=0;
  Mouse.mzxold=0;
  Mouse.mzyold=0;
  Mouse.data=0;
  return(&MOUSEC);
}
