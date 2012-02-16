/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2011 CaH4e3
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
 * Super Game (Sugar Softec) protected mapper
 * Pocahontas 2 (Unl) [U][!], etc.
 * TODO: 9in1 LION KING HANGS!
 */

#include "mapinc.h"
#include "mmc3.h"

static uint8 cmdin;

static uint8 regperm[8][8] = 
  {
    {0, 1, 2, 3, 4, 5, 6, 7},
    {0, 2, 6, 1, 7, 3, 4, 5},
    {0, 5, 4, 1, 7, 2, 6, 3}, /* unused*/
    {0, 6, 3, 7, 5, 2, 4, 1},
    {0, 2, 5, 3, 6, 1, 7, 4},
    {0, 1, 2, 3, 4, 5, 6, 7}, /* empty*/
    {0, 1, 2, 3, 4, 5, 6, 7}, /* empty*/
    {0, 1, 2, 3, 4, 5, 6, 7}, /* empty*/
  };

static uint8 adrperm[8][8] = 
  {
    {0, 1, 2, 3, 4, 5, 6, 7},
    {3, 2, 0, 4, 1, 5, 6, 7},
    {0, 1, 2, 3, 4, 5, 6, 7}, /* unused*/
    {5, 0, 1, 2, 3, 7, 6, 4},
    {3, 1, 0, 5, 2, 4, 6, 7},
    {0, 1, 2, 3, 4, 5, 6, 7}, /* empty*/
    {0, 1, 2, 3, 4, 5, 6, 7}, /* empty*/
    {0, 1, 2, 3, 4, 5, 6, 7}, /* empty*/
  };

static void UNL8237CW(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x40)
    setchr1(A,((EXPREGS[1]&0xc)<<6)|(V&0x7F)|((EXPREGS[1]&0x20)<<2));
  else
    setchr1(A,((EXPREGS[1]&0xc)<<6)|V);
}

static void UNL8237PW(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x40)
  {
    uint8 sbank = (EXPREGS[1]&0x10);
    if(EXPREGS[0]&0x80)
    {
      uint8 bank = ((EXPREGS[1]&3)<<4)|(EXPREGS[0]&0x7)|(sbank>>1);
      if(EXPREGS[0]&0x20)
        setprg32(0x8000,bank>>1);
      else
      {
        setprg16(0x8000,bank);
        setprg16(0xC000,bank);
      }
    }
    else
      setprg8(A,((EXPREGS[1]&3)<<5)|(V&0x0F)|sbank);
  }
  else
  {
    if(EXPREGS[0]&0x80)
    {
      uint8 bank = ((EXPREGS[1]&3)<<4)|(EXPREGS[0]&0xF);
      if(EXPREGS[0]&0x20)
        setprg32(0x8000,bank>>1);
      else
      {
        setprg16(0x8000,bank);
        setprg16(0xC000,bank);
      }
    }
    else
      setprg8(A,((EXPREGS[1]&3)<<5)|(V&0x1F));
  }
}

static void UNL8237ACW(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x40)
    setchr1(A,((EXPREGS[1]&0xE)<<7)|(V&0x7F)|((EXPREGS[1]&0x20)<<2));
  else
    setchr1(A,((EXPREGS[1]&0xE)<<7)|V);
}

static void UNL8237APW(uint32 A, uint8 V)
{
	if(EXPREGS[0]&0x40)
	{
		uint8 sbank = (EXPREGS[1]&0x10);
		if(EXPREGS[0]&0x80)
		{
			uint8 bank = ((EXPREGS[1]&3)<<4)|((EXPREGS[1]&8)<<3)|(EXPREGS[0]&0x7)|(sbank>>1);
			if(EXPREGS[0]&0x20) {
				/*        FCEU_printf("8000:%02X\n",bank>>1);*/
				setprg32(0x8000,bank>>1);
			}
			else
			{
				/*        FCEU_printf("8000-C000:%02X\n",bank);*/
				setprg16(0x8000,bank);
				setprg16(0xC000,bank);
			}
		}
		else {
			/*      FCEU_printf("%04x:%02X\n",A,((EXPREGS[1]&3)<<5)|((EXPREGS[1]&8)<<4)|(V&0x0F)|sbank);*/
			setprg8(A,((EXPREGS[1]&3)<<5)|((EXPREGS[1]&8)<<4)|(V&0x0F)|sbank);
		}
	}
	else
	{
		if(EXPREGS[0]&0x80)
		{
			uint8 bank = ((EXPREGS[1]&3)<<4)|((EXPREGS[1]&8)<<3)|(EXPREGS[0]&0xF);
			if(EXPREGS[0]&0x20) {
				/*        FCEU_printf("8000:%02X\n",(bank>>1)&0x07);*/
				setprg32(0x8000,bank>>1);
			}
			else
			{
				/*        FCEU_printf("8000-C000:%02X\n",bank&0x0F);*/
				setprg16(0x8000,bank);
				setprg16(0xC000,bank);
			}
		}
		else {
			/*      FCEU_printf("%04X:%02X\n",A,(((EXPREGS[1]&3)<<5)|((EXPREGS[1]&8)<<4)|(V&0x1F))&0x1F);*/
			setprg8(A,((EXPREGS[1]&3)<<5)|((EXPREGS[1]&8)<<4)|(V&0x1F));
		}
	}
}

static DECLFW(UNL8237Write)
{
	uint8 dat = V;
	uint8 adr = adrperm[EXPREGS[2]][((A>>12)&6)|(A&1)];
	uint16 addr = (adr & 1)|((adr & 6)<<12)|0x8000;
	if(adr < 4)
	{
		if(!adr)
			dat = (dat & 0xC0)|(regperm[EXPREGS[2]][dat & 7]);
		MMC3_CMDWrite(addr,dat);
	}
	else 
		MMC3_IRQWrite(addr,dat);
}

static DECLFW(UNL8237ExWrite)
{
  switch(A)
  {
    case 0x5000: EXPREGS[0]=V; FixMMC3PRG(MMC3_cmd); break;
    case 0x5001: EXPREGS[1]=V; FixMMC3PRG(MMC3_cmd); FixMMC3CHR(MMC3_cmd); break;
    case 0x5007: EXPREGS[2]=V; break;
  }
}

static void UNL8237Power(void)
{
  EXPREGS[0]=EXPREGS[2]=0;
  EXPREGS[1]=3;
  GenMMC3Power();
  SetWriteHandler(0x8000,0xFFFF,UNL8237Write);
  SetWriteHandler(0x5000,0x7FFF,UNL8237ExWrite);
}

void UNL8237_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 256, 0, 0);
  cwrap=UNL8237CW;
  pwrap=UNL8237PW;
  info->Power=UNL8237Power;
  AddExState(EXPREGS, 3, 0, "EXPR");
  AddExState(&cmdin, 1, 0, "CMDIN");
}

void UNL8237A_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 256, 0, 0);
  cwrap=UNL8237ACW;
  pwrap=UNL8237APW;
  info->Power=UNL8237Power;
  AddExState(EXPREGS, 3, 0, "EXPR");
  AddExState(&cmdin, 1, 0, "CMDIN");
}
