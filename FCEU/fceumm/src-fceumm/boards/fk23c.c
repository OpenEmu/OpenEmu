/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2006 CaH4e3
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

#include "mapinc.h"
#include "mmc3.h"

static uint8 unromchr;
static uint32 dipswitch = 0;
static uint8 *CHRRAM=NULL;
static uint32 CHRRAMSize;

static void BMCFK23CCW(uint32 A, uint8 V)
{
	if(EXPREGS[0]&0x40)
		setchr8(EXPREGS[2]|unromchr);
	else if(EXPREGS[0] & 0x20)
		setchr1r(0x10, A, V);
	else
	{
		uint16 base=(EXPREGS[2]&0x7F)<<3;
		if(EXPREGS[3]&2)
		{
			int cbase=(MMC3_cmd&0x80)<<5;
			setchr1(A,V|base);
			setchr1(0x0000^cbase,DRegBuf[0]|base);
			setchr1(0x0400^cbase,EXPREGS[6]|base);
			setchr1(0x0800^cbase,DRegBuf[1]|base);
			setchr1(0x0c00^cbase,EXPREGS[7]|base);
		}
		else
			setchr1(A,V|base);
	}
}

static void BMCFK23CPW(uint32 A, uint8 V)
{
  uint32 bank = (EXPREGS[1] & 0x1F);
  uint32 hiblock = ((EXPREGS[0] & 8) << 4)|((EXPREGS[0] & 0x80) << 1)|(UNIFchrrama?((EXPREGS[2] & 0x40)<<3):0);
  uint32 block = (EXPREGS[1] & 0x60) | hiblock;
  uint32 extra = (EXPREGS[3] & 2);
  switch(EXPREGS[0]&7)
  {
   case 0: setprg8(A, (block << 1) | (V & 0x3F));
           if(extra)
           {
            setprg8(0xC000,EXPREGS[4]);
            setprg8(0xE000,EXPREGS[5]);
           }
           break;
   case 1: setprg8(A, ((hiblock | (EXPREGS[1] & 0x70)) << 1) | (V & 0x1F));
           if(extra)
           {
            setprg8(0xC000,EXPREGS[4]);
            setprg8(0xE000,EXPREGS[5]);
           }
           break;
   case 2: setprg8(A, ((hiblock | (EXPREGS[1] & 0x78)) << 1) | (V & 0x0F));
           if(extra)
           {
            setprg8(0xC000,EXPREGS[4]);
            setprg8(0xE000,EXPREGS[5]);
           }
           break;
   case 3: setprg16(0x8000,(bank | block));
           setprg16(0xC000,(bank | block));
           break;
   case 4: setprg32(0x8000,(bank | block) >> 1);
           break;
  }
  setprg8r(0x10,0x6000,A001B&3);
}

static DECLFW(BMCFK23CHiWrite)
{
	if(EXPREGS[0]&0x40)
	{
		if(EXPREGS[0]&0x30)
			unromchr=0;
		else
		{
			unromchr=V&3;
			FixMMC3CHR(MMC3_cmd);
		}
	}
	else
	{
		if((A==0x8001)&&(EXPREGS[3]&2)&&(MMC3_cmd&8))
		{
			EXPREGS[4|(MMC3_cmd&3)]=V;
			FixMMC3PRG(MMC3_cmd);
			FixMMC3CHR(MMC3_cmd);
		}
		else
			if(A<0xC000) {
				if(UNIFchrrama) { /* hacky... strange behaviour, must be bit scramble due to pcb layot restrictions*/
					/* check if it not interfer with other dumps*/
					if((A==0x8000)&&(V==0x46))
						V=0x47;
					else if((A==0x8000)&&(V==0x47))
						V=0x46;
				}
				MMC3_CMDWrite(A,V);
				FixMMC3PRG(MMC3_cmd);
			}
			else
				MMC3_IRQWrite(A,V);
	}
}

static DECLFW(BMCFK23CWrite)
{
	/*  FCEU_printf("lo %04x:%02x\n",A,V);*/
	if(dipswitch) /* нулевой дип берет любые записи по дефолту, дальше идет выбор*/
	{
		if(A&(1<<(dipswitch+3))) {
			EXPREGS[A&3]=V;
			/*      FCEU_printf(" reg %d set!\n",A&3);*/
			FixMMC3PRG(MMC3_cmd);
			FixMMC3CHR(MMC3_cmd);
		}
	}
	else
	{
		EXPREGS[A&3]=V;
		/*    FCEU_printf(" reg %d set!\n",A&3);*/
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
}

static void BMCFK23CReset(void)
{
  if(dipswitch<=8)
    dipswitch++;
  else
    dipswitch=0;
  EXPREGS[0]=EXPREGS[1]=EXPREGS[2]=EXPREGS[3]=0;
  EXPREGS[4]=EXPREGS[5]=EXPREGS[6]=EXPREGS[7]=0xFF;
  MMC3RegReset();
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void BMCFK23CPower(void)
{
  GenMMC3Power();
  EXPREGS[0]=4;
  EXPREGS[1]=0xFF;
  EXPREGS[2]=EXPREGS[3]=0;
  dipswitch = 0;
  EXPREGS[4]=EXPREGS[5]=EXPREGS[6]=EXPREGS[7]=0xFF;
  SetWriteHandler(0x5000,0x5fff,BMCFK23CWrite);
  SetWriteHandler(0x8000,0xFFFF,BMCFK23CHiWrite);
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void BMCFK23CAPower(void)
{
  GenMMC3Power();
  dipswitch = 0;
  EXPREGS[0]=EXPREGS[1]=EXPREGS[2]=EXPREGS[3]=0;
  EXPREGS[4]=EXPREGS[5]=EXPREGS[6]=EXPREGS[7]=0xFF;
  SetWriteHandler(0x5000,0x5fff,BMCFK23CWrite);
  SetWriteHandler(0x8000,0xFFFF,BMCFK23CHiWrite);
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void BMCFK23CAClose(void)
{
	if(CHRRAM)
		free(CHRRAM);
	CHRRAM = NULL;
}

void BMCFK23C_Init(CartInfo *info)
{
  GenMMC3_Init(info, 512, 256, 128, 0);
  cwrap=BMCFK23CCW;
  pwrap=BMCFK23CPW;
  info->Power=BMCFK23CPower;
  info->Reset=BMCFK23CReset;
  AddExState(EXPREGS, 8, 0, "EXPR");
  AddExState(&unromchr, 1, 0, "UNCHR");
  AddExState(&dipswitch, 1, 0, "DIPSW");
}

void BMCFK23CA_Init(CartInfo *info)
{
  GenMMC3_Init(info, 512, 256, 128, 0);
  cwrap=BMCFK23CCW;
  pwrap=BMCFK23CPW;
  info->Power=BMCFK23CAPower;
  info->Reset=BMCFK23CReset;
  info->Close=BMCFK23CAClose;

  CHRRAMSize=8192;
  CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSize);
  SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
  AddExState(CHRRAM, CHRRAMSize, 0, "CHRRAM");
  AddExState(EXPREGS, 8, 0, "EXPR");
  AddExState(&unromchr, 1, 0, "UNCHR");
  AddExState(&dipswitch, 1, 0, "DIPSW");
}
