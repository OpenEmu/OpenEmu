/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
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
 *
 * A98402 board, A9711, A9746 similar
 * King of Fighters 96, The (Unl), Street Fighter Zero 2 (Unl)
 * 
 */

#include "mapinc.h"
#include "mmc3.h"

static void M187CW(uint32 A, uint8 V)
{
	if((A&0x1000)==((MMC3_cmd&0x80)<<5))
		setchr1(A,V|0x100);
	else
		setchr1(A,V);
}

static void M187PW(uint32 A, uint8 V)
{
	if(EXPREGS[0]&0x80)
	{
		uint8 bank=EXPREGS[0]&0x1F;
		if(EXPREGS[0]&0x20)
		{
			if(EXPREGS[0]&0x40)
				setprg32(0x8000,bank>>2);
			else
				setprg32(0x8000,bank>>1); /* hacky hacky! two mappers in one! need real hw carts to test*/
		}
		else
		{
			setprg16(0x8000,bank);
			setprg16(0xC000,bank);
		}
	}
	else
		setprg8(A,V&0x3F);
}

static DECLFW(M187Write8000)
{
  EXPREGS[1]=1;
  MMC3_CMDWrite(A,V);
}

static DECLFW(M187Write8001)
{
  if(EXPREGS[1])
    MMC3_CMDWrite(A,V);
}

static DECLFW(M187WriteLo)
{
  if((A==0x5000)||(A==0x6000))
  {
    EXPREGS[0]=V;
    FixMMC3PRG(MMC3_cmd);
  }
}

static uint8 prot_data[4] = { 0x83, 0x83, 0x42, 0x00 };
static DECLFR(M187Read)
{
  return prot_data[EXPREGS[1]&3];
}

static void M187Power(void)
{
  EXPREGS[0]=EXPREGS[1]=0;
  GenMMC3Power();
  SetReadHandler(0x5000,0x5FFF,M187Read);
  SetWriteHandler(0x5000,0x6FFF,M187WriteLo);
  SetWriteHandler(0x8000,0x8000,M187Write8000);
  SetWriteHandler(0x8001,0x8001,M187Write8001);
}

void Mapper187_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 256, 0, 0);
  pwrap=M187PW;
  cwrap=M187CW;
  info->Power=M187Power;
  AddExState(EXPREGS, 3, 0, "EXPR");
}
