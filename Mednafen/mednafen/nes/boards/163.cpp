/* Mednafen - Multi-system Emulator
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

/* Mapper 163(Nanjing) emulation Based on code from VirtuaNES */

#include "mapinc.h"

static uint8 *WRAM = NULL;
static uint8 security, trigger, strobe;
static uint8 regs[2];
static uint8 chrbank[2];

static void Sync(void)
{
 setprg32(0x8000, (regs[0] & 0xF) | ((regs[1] & 0xF) << 4));
 setchr4(0x0000, chrbank[0]);
 setchr4(0x1000, chrbank[1]);
}

static DECLFW(Write)
{
 //printf("Write: %04x %02x\n", A, V);
 switch(A & 0xF300)
 {
  case 0x5000: regs[0] = V;
	       if(!(regs[0] & 0x80) && scanline < 128)
	       {
		chrbank[0] = 0;
	        chrbank[1] = 1;
	       }
	       Sync();
	       break;

  case 0x5100: if(A & 1)
	       {
	        if(strobe && !V)
		 trigger ^= 1;
		strobe = V;
	       }
	       else
	       {
		// Needed by NJ023...
		if(V == 6)
		{
		 regs[0] &= ~0x0F;
		 regs[0] |= 0x03;
		 regs[1] &= ~0x0F;	// Is this correct?
		 Sync();
		}

		//printf("Oops: %04x %02x\n", A, V);
	       }
	       break;

  case 0x5200: regs[1] = V;
	       Sync();
	       break;

  case 0x5300: security = V;
	       break;

 }
}

static DECLFR(ReadLow)
{
 //printf("Readlow: %04x\n", A);
 switch(A & 0xF700)
 {
  case 0x5100: return(security);
  case 0x5500: return(trigger ? security : 0);
  default: return(4);
 }
 return(0x00);
}

static void Mapper163_HB(void)
{
 if(regs[0] & 0x80)
 {
  switch(scanline)
  {
   case 127: chrbank[0] = 1;
	     chrbank[1] = 1;
	     break;

   case 239: chrbank[0] = 0;
	     chrbank[1] = 0;
	     break;
  }
 }
 Sync();
}

static DECLFW(BWRAM)
{
 //printf("WriteRA: %04x %02x\n", A, V);
 WRAM[A-0x6000]=V;
}

static DECLFR(AWRAM)
{
 //printf("ReadRA: %04x\n", A);
 return(WRAM[A-0x6000]);
}

static void Power(CartInfo *info)
{

 regs[0] = 0x8F;
 regs[1] = 0x00;
 strobe = 1;
 security = 0x00;
 trigger = 0;

 chrbank[0] = 0;
 chrbank[1] = 1;
 Sync();

 if(!info->battery)
  memset(WRAM, 0xFF, 8192);	// 0xFF or 0x00?
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(WRAM, 8192),
  SFARRAY(regs, 2),
  SFARRAY(chrbank, 2),
  SFVAR(strobe),
  SFVAR(security),
  SFVAR(trigger),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  Sync();
 }
 return(ret);
}

static void Close(void)
{
 if(WRAM)
 {
  free(WRAM);
  WRAM = NULL;
 }
}

int Mapper163_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 info->Close = Close;

 GameHBIRQHook2 = Mapper163_HB;


 if(!(WRAM = (uint8 *)malloc(8192)))
 {
  return(0);
 }

 memset(WRAM, 0x00, 8192);	// 0x00 needed over 0xFF for NJ079.  Probably programmed that way in the factory?
 MDFNMP_AddRAM(8192, 0x6000, WRAM);
 if(info->battery)
 {
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }

 SetReadHandler(0x5000, 0x5FFF, ReadLow);
 SetWriteHandler(0x5000, 0x5FFF, Write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 SetReadHandler(0x6000, 0x7FFF, AWRAM);
 SetWriteHandler(0x6000, 0x7FFF, BWRAM);

 return(1);
}
