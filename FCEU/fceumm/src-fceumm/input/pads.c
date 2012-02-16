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
#include "share.h"
#include "../vsuni.h"

static uint8 joy_readbit[2];
static uint8 joy[4]={0,0,0,0};

static int FSDisable=0;  /* Set to 1 if NES-style four-player adapter is disabled. */
void FCEUI_DisableFourScore(int s)
{
 FSDisable=s;
}

extern uint8 LastStrobe;

SFORMAT FCEUCTRL_STATEINFO[]={
  { joy_readbit, 2, "JYRB"},
  { joy, 4, "JOYS"},
  { &LastStrobe, 1, "LSTS"},
  { 0 }
 };

/* This function is a quick hack to get the NSF player to use emulated gamepad
   input.
*/
uint8 FCEU_GetJoyJoy(void)
{
  return(joy[0]|joy[1]|joy[2]|joy[3]);
}

static void StrobeGP(int w)
{
	joy_readbit[w]=0;
}

static uint8 ReadGPVS(int w)
{
  uint8 ret=0;
  if(joy_readbit[w]>=8)
    ret=1;
  else
  {
    ret = ((joy[w]>>(joy_readbit[w]))&1);
      joy_readbit[w]++;
  }
  return ret;
}

static uint8 ReadGP(int w)
{
	uint8 ret;
	if(joy_readbit[w]>=8)
		ret = ((joy[2+w]>>(joy_readbit[w]&7))&1);
	else
		ret = ((joy[w]>>(joy_readbit[w]))&1);
	if(joy_readbit[w]>=16) ret=0;
	if(FSDisable)
	{
		if(joy_readbit[w]>=8)
			ret|=1;
	}
	else
	{
		if(joy_readbit[w]==19-w)
			ret|=1;
	}
	joy_readbit[w]++;
	return ret;
}

static void UpdateGP(int w, void *data, int arg)
{
  uint32 *ptr=(uint32*)data;
  if(!w)
  {
    joy[0]=*(uint32*)ptr;
    joy[2]=*(uint32*)ptr>>16;
  }
  else
  {
    joy[1]=*(uint32*)ptr>>8;
    joy[3]=*(uint32*)ptr>>24;
  }
  if(FCEUGameInfo->type==GIT_VSUNI)
    FCEU_VSUniSwap(&joy[0],&joy[1]);
}

static INPUTC GPC={ReadGP,0,StrobeGP,UpdateGP,0,0};
static INPUTC GPCVS={ReadGPVS,0,StrobeGP,UpdateGP,0,0};

INPUTC *FCEU_InitJoyPad(int w)
{
  joy_readbit[w]=0;
  joy[w]=0;
  joy[w+2]=0;
  if(FCEUGameInfo->type==GIT_VSUNI)
     return(&GPCVS);
  else
     return(&GPC);
}

static uint8 F4ReadBit[2];
static void StrobeFami4(void)
{
  F4ReadBit[0]=F4ReadBit[1]=0;
}

static uint8 ReadFami4(int w, uint8 ret)
{
	ret&=1;
	ret|=((joy[2+w]>>(F4ReadBit[w]))&1)<<1;
	if(F4ReadBit[w]>=8) ret|=2;
	else F4ReadBit[w]++;
	return(ret);
}

static INPUTCFC FAMI4C={ReadFami4,0,StrobeFami4,0,0,0};
INPUTCFC *FCEU_InitFami4(int w)
{
  memset(&F4ReadBit,0,sizeof(F4ReadBit));
  return(&FAMI4C);
}
