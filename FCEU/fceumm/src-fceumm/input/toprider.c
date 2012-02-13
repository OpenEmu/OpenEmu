/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 Xodnizel
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

static uint32 bs,bss;
static uint32 boop;

static uint8 Read(int w, uint8 ret)
{
  if(w)
  {
    ret|=(bs&1)<<3;
    ret|=(boop&1)<<4;
    bs>>=1;
    boop>>=1;
  }
  return(ret);
}

static void Write(uint8 V)
{
	bs=bss;
}

static void Update(void *data, int arg)
{
  bss=*(uint8*)data;
  bss|=bss<<8;
  bss|=bss<<8;
}

static INPUTCFC TopRider={Read,Write,0,Update,0,0};

INPUTCFC *FCEU_InitTopRider(void)
{
  return(&TopRider);
}

