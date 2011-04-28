/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
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

#include "mapinc.h"

static void GenMMC1Power(CartInfo *info);
static int GenMMC1Init(CartInfo *info, int prg, int chr, int wram, int battery);

static uint8 DRegs[4];
static uint8 Buffer,BufferShift;

static int mmc1opts;

static void (*MMC1CHRHook4)(uint32 A, uint8 V);
static void (*MMC1PRGHook16)(uint32 A, uint8 V);

static int WRAM_Size;
static uint8 *WRAM=NULL;
static uint8 *CHRRAM=NULL;
static int is155;

static DECLFW(MBWRAM)
{
 if(!(DRegs[3]&0x10) || is155)
  Page[A>>11][A]=V;     // WRAM is enabled.
}

static DECLFR(MAWRAM)
{
 if((DRegs[3]&0x10) && !is155)
  return X.DB;          // WRAM is disabled
 return(Page[A>>11][A]);
}

static void MMC1CHR(void)
{
    if(mmc1opts&4)
    {
     if(DRegs[0]&0x10)
      setprg8r(0x10,0x6000,(DRegs[1]>>4)&1);
     else
      setprg8r(0x10,0x6000,(DRegs[1]>>3)&1);
    }

    if(MMC1CHRHook4)
    {
     if(DRegs[0]&0x10)
     {
      MMC1CHRHook4(0x0000,DRegs[1]);
      MMC1CHRHook4(0x1000,DRegs[2]);
     }   
     else
     {
      MMC1CHRHook4(0x0000,(DRegs[1]&0xFE));
      MMC1CHRHook4(0x1000,DRegs[1]|1);
     }
    }
    else
    {
     if(DRegs[0]&0x10)
     {
      setchr4(0x0000,DRegs[1]);
      setchr4(0x1000,DRegs[2]);
     }
     else
      setchr8(DRegs[1]>>1);
    }
}

static void MMC1PRG(void)
{
        uint8 offs;

        offs=DRegs[1]&0x10;
	if(MMC1PRGHook16)
	{
         switch(DRegs[0]&0xC)
         {
          case 0xC: MMC1PRGHook16(0x8000,(DRegs[3]+offs));
                    MMC1PRGHook16(0xC000,0xF+offs);
                    break;
          case 0x8: MMC1PRGHook16(0xC000,(DRegs[3]+offs));
                    MMC1PRGHook16(0x8000,offs);
                    break;
          case 0x0:
          case 0x4:
                    MMC1PRGHook16(0x8000,((DRegs[3]&~1)+offs));
                    MMC1PRGHook16(0xc000,((DRegs[3]&~1)+offs+1));
                    break;
         }
	}
	else
         switch(DRegs[0]&0xC)
         {
          case 0xC: setprg16(0x8000,(DRegs[3]+offs));
                    setprg16(0xC000,0xF+offs);
                    break;
          case 0x8: setprg16(0xC000,(DRegs[3]+offs));
                    setprg16(0x8000,offs);
                    break;
          case 0x0:
          case 0x4:
                    setprg16(0x8000,((DRegs[3]&~1)+offs));
                    setprg16(0xc000,((DRegs[3]&~1)+offs+1));
                    break;
         }
}

static void MMC1MIRROR(void)
{
                switch(DRegs[0]&3)
                {
                 case 2: setmirror(MI_V);break;
                 case 3: setmirror(MI_H);break;
                 case 0: setmirror(MI_0);break;
                 case 1: setmirror(MI_1);break;
                }
}

#ifdef WANT_DEBUGGER
enum
{
 MMC1_GSREG_CONTROL = 0,
 MMC1_GSREG_CHRBANK0,
 MMC1_GSREG_CHRBANK1,
 MMC1_GSREG_PRGBANK,
 MMC1_GSREG_BUFFER,
 MMC1_GSREG_BUFFERSHIFT
};

static RegType DBGMMC1Regs[] =
{
 { MMC1_GSREG_CONTROL, "Control", "Control(MMC1 register 0)", 1 },
 { MMC1_GSREG_CHRBANK0, "CHRBank0", "CHR Bank Register 0(MMC1 register 1)", 1 },
 { MMC1_GSREG_CHRBANK1, "CHRBank1", "CHR Bank Register 1(MMC1 register 2)", 1 },
 { MMC1_GSREG_PRGBANK, "PRGBank", "PRG Bank Bank Register(MMC1 register 3)", 1 },
 { MMC1_GSREG_BUFFER, "Buffer", "Shifty buffer that takes your moneys!", 1 },
 { MMC1_GSREG_BUFFERSHIFT, "BufferShift", "Place to stick bit in shifty bit buffer", 1 },
 { 0, "", "", 0 },
};

static uint32 MMC1DBG_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 value = 0xDEADBEEF;

 switch(id)
 {
  case MMC1_GSREG_CONTROL:
	value = DRegs[0];

	if(special)
	{
	 static const char *PMNames[4] = { "One-screen(0)", "One-screen(1)", "Vertical", "Horizontal" };
	 char tmpstr[256];
	 tmpstr[0] = 0;

	 sprintf(tmpstr + strlen(tmpstr), "PRG Bank Size: %dKiB", (DRegs[0] & 0x8) ? 16 : 32);
	 if(DRegs[0] & 0x8)
	 {
	  sprintf(tmpstr + strlen(tmpstr), " @ 0x%04x, ", (DRegs[0] & 0x4) ? 0x8000 : 0xC000);
	 }
	 else
	  strcat(tmpstr, ", ");

	 sprintf(tmpstr + strlen(tmpstr), "CHR Bank Size: %dKiB, ", (DRegs[0]&0x10) ? 4 : 8);

	 if(!CartHasHardMirroring())
	  sprintf(tmpstr + strlen(tmpstr), "Mirroring: %s", PMNames[DRegs[0] & 0x3]);

	 strncpy(special, tmpstr, special_len);
	}
	break;

  case MMC1_GSREG_CHRBANK0:
	value = DRegs[1];
	break;

  case MMC1_GSREG_CHRBANK1:
	value = DRegs[2];
	break;

  case MMC1_GSREG_PRGBANK:
	value = DRegs[3];
	break;

  case MMC1_GSREG_BUFFER:
	value = Buffer;
	break;

  case MMC1_GSREG_BUFFERSHIFT:
	value = BufferShift;
	break;
 }

 return(value);
}

static void MMC1DBG_SetRegister(const unsigned int id, uint32 value)
{
 switch(id)
 {
  case MMC1_GSREG_CONTROL:
	DRegs[0] = value;
	break;

  case MMC1_GSREG_CHRBANK0:
	DRegs[1] = value;
	break;

  case MMC1_GSREG_CHRBANK1:
	DRegs[2] = value;
	break;

  case MMC1_GSREG_PRGBANK:
	DRegs[3] = value;
	break;

  case MMC1_GSREG_BUFFER:
	Buffer = value;
	break;

  case MMC1_GSREG_BUFFERSHIFT:
	BufferShift = value % 5;
	break;
 }

 MMC1MIRROR();
 MMC1CHR();
 MMC1PRG();
}

static RegGroupType DBGMMC1RegsGroup =
{
 "MMC1",
 DBGMMC1Regs,
 MMC1DBG_GetRegister,
 MMC1DBG_SetRegister,
};

#endif


static uint64 lreset;
static DECLFW(MMC1_write)
{
        int n=(A>>13)-4;
	//MDFN_DispMessage("%016x",timestampbase+timestamp);
	//printf("$%04x:$%02x, $%04x\n",A,V,X.PC);
	//DumpMem("out",0xe000,0xffff);

	/* The MMC1 is busy so ignore the write. */
	/* As of version Mednafen 0.81, the timestamp is only
	   increased before each instruction is executed(in other words
	   precision isn't that great), but this should still work to
	   deal with 2 writes in a row from a single RMW instruction.
	*/
	if( (timestampbase+timestamp)<(lreset+2))
	 return;
        if (V&0x80)
        {
	 DRegs[0]|=0xC;
         BufferShift=Buffer=0;
	 MMC1PRG();
	 lreset=timestampbase+timestamp;
         return;
        }

        Buffer|=(V&1)<<(BufferShift++);

  if (BufferShift==5) {
        DRegs[n] = Buffer;
        BufferShift = Buffer = 0;

        switch(n){
        case 0:
		MMC1MIRROR();
                MMC1CHR();
                MMC1PRG();
                break;
        case 1:
                MMC1CHR();
                MMC1PRG();
                break;
        case 2:
                MMC1CHR();
                break;
        case 3:
                MMC1PRG();
                break;
        }
  }
}

static void MMC1CMReset(void)
{
        int i;

        for(i=0;i<4;i++)
         DRegs[i]=0;
        Buffer = BufferShift = 0;
        DRegs[0]=0x1F;

        DRegs[1]=0;
        DRegs[2]=0;                  // Should this be something other than 0?
        DRegs[3]=0;

        MMC1MIRROR();
        MMC1CHR();
        MMC1PRG();
}

static int DetectMMC1WRAMSize(uint32 crc32)
{
        switch(crc32)
        {
         default:return(8);
	 case 0xc6182024:	/* Romance of the 3 Kingdoms */
         case 0x2225c20f:       /* Genghis Khan */
         case 0x4642dda6:       /* Nobunaga's Ambition */
	 case 0x29449ba9:	/* ""	"" (J) */
	 case 0x2b11e0b0:	/* ""	"" (J) */
		MDFN_printf(_(">8KB external WRAM present.  Use UNIF if you hack the ROM image.\n"));
		return(16);
        }
}

static uint32 NWCIRQCount;
static uint8 NWCRec;
#define NWCDIP 0xE
 
static void NWCIRQHook(int a)
{ 
        if(!(NWCRec&0x10))
        {
         NWCIRQCount+=a;
         if((NWCIRQCount|(NWCDIP<<25))>=0x3e000000)
          {
           NWCIRQCount=0;
           X6502_IRQBegin(MDFN_IQEXT);
          }
        }
} 

static void NWCCHRHook(uint32 A, uint8 V)
{
 if((V&0x10)) // && !(NWCRec&0x10))
 {
  NWCIRQCount=0;
  X6502_IRQEnd(MDFN_IQEXT);
 }

 NWCRec=V;
 if(V&0x08)
  MMC1PRG();
 else
  setprg32(0x8000,(V>>1)&3);
}

static void NWCPRGHook(uint32 A, uint8 V)
{
 if(NWCRec&0x8)
  setprg16(A,8|(V&0x7));
 else
  setprg32(0x8000,(NWCRec>>1)&3);
}

static void NWCPower(CartInfo *info)
{
 GenMMC1Power(info);
 setchr8r(0,0);
}

static int NWC_StateAction(StateMem *sm, int load, int data_only);
int Mapper105_Init(CartInfo *info)
{
 if(!(GenMMC1Init(info, 256, 256, 8, 0)))
  return(0);

 MMC1CHRHook4=NWCCHRHook;
 MMC1PRGHook16=NWCPRGHook;
 MapIRQHook=NWCIRQHook;
 info->Power=NWCPower;
 info->StateAction = NWC_StateAction;
 return(1);
}

static void GenMMC1Power(CartInfo *info)
{
 lreset=0;
 if(mmc1opts&1)
 {
  if(mmc1opts&4)
   memset(WRAM,0,8192);
  else if(!(mmc1opts&2))
   memset(WRAM,0,8192);
 }

 if(mmc1opts&1)
 {
  setprg8r(0x10,0x6000,0);
 }

 MMC1CMReset();
}

static void GenMMC1Close(void)
{
 if(CHRRAM)
  MDFN_free(CHRRAM);
 if(WRAM)
  MDFN_free(WRAM);
 CHRRAM=WRAM=NULL;
} 

static int NWC_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(WRAM, WRAM_Size),
  SFARRAY(CHRRAM, 8192),
  SFARRAY(DRegs, 4),
  SFVAR(lreset),
  SFVAR(Buffer),
  SFVAR(BufferShift),
  SFVAR(NWCIRQCount),
  SFVAR(NWCRec),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  MMC1MIRROR();
  MMC1CHR();
  MMC1PRG();
  lreset=0;      /* timestamp(base) is not stored in save states. */
 }
 return(ret);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 //uint32 NWCIRQCount;
 //uint8 NWCRec;

 SFORMAT StateRegs[] =
 {
  SFARRAY(WRAM, WRAM_Size),
  SFARRAY(CHRRAM, 8192),
  SFARRAY(DRegs, 4),
  SFVAR(lreset),
  SFVAR(Buffer),
  SFVAR(BufferShift),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  MMC1MIRROR();
  MMC1CHR();
  MMC1PRG();
  lreset=0;      /* timestamp(base) is not stored in save states. */
 }
 return(ret);
}

static int GenMMC1Init(CartInfo *info, int prg, int chr, int wram, int battery)
{
 is155=0;

 info->StateAction = StateAction;
 info->Close=GenMMC1Close;
 MMC1PRGHook16=MMC1CHRHook4=0;
 mmc1opts=0;
 PRGmask16[0]&=(prg>>14)-1;
 CHRmask4[0]&=(chr>>12)-1;
 CHRmask8[0]&=(chr>>13)-1;

 WRAM_Size = wram * 1024;
 if(wram) 
 { 
  if(!(WRAM=(uint8*)malloc(wram*1024)))
  {
   GenMMC1Close();
   return(0);
  }
  mmc1opts|=1;
  if(wram>8) mmc1opts|=4;
  SetupCartPRGMapping(0x10,WRAM,wram*1024,1);

  if(battery)
  {
   mmc1opts|=2;

   info->SaveGame[0]=WRAM+((mmc1opts&4)?8192:0);
   info->SaveGameLen[0]=8192;
  }
 }
 if(!chr)
 {
  if(!(CHRRAM=(uint8*)malloc(8192)))
  {
   GenMMC1Close();
   return(0);
  }
  SetupCartCHRMapping(0, CHRRAM, 8192, 1);
 }
 info->Power=GenMMC1Power;


 #ifdef WANT_DEBUGGER
 MDFNDBG_AddRegGroup(&DBGMMC1RegsGroup);
 #endif

 SetWriteHandler(0x8000,0xFFFF,MMC1_write);
 SetReadHandler(0x8000,0xFFFF,CartBR);

 if(mmc1opts&1)
 {
  MDFNMP_AddRAM(8192,0x6000,WRAM);
  SetReadHandler(0x6000,0x7FFF,MAWRAM);
  SetWriteHandler(0x6000,0x7FFF,MBWRAM);
 }

 return(1);
}

int Mapper1_Init(CartInfo *info)
{
 int ws=DetectMMC1WRAMSize(info->CRC32);
 return(GenMMC1Init(info, 512, 256, ws, info->battery));
}

/* Same as mapper 1, without respect for WRAM enable bit. */
int Mapper155_Init(CartInfo *info)
{
 int ret = GenMMC1Init(info,512,256,8,info->battery);

 is155=1;

 return(ret);
}

//static void GenMMC1Init(int prg, int chr, int wram, int battery)
int SAROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 128, 64, 8, info->battery));
}

int SBROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 128, 64, 0, 0));
}

int SCROM_Init(CartInfo *info)	
{
 return(GenMMC1Init(info, 128, 128, 0, 0));
}

int SEROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 32, 64, 0, 0));
}

int SGROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 256, 0, 0, 0));
}

int SKROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 256, 64, 8, info->battery));
}

int SLROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 256, 128, 0, 0));
}

int SL1ROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 128, 128, 0, 0));
}

/* Begin unknown - may be wrong - perhaps they use different MMC1s from the
   similarly functioning boards?
*/

int SL2ROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 256, 256, 0, 0));
}

int SFROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 256, 256, 0, 0));
}

int SHROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 256, 256, 0, 0));
}

/* End unknown  */
/*              */
/*              */

int SNROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 256, 0, 8, info->battery));
}

int SOROM_Init(CartInfo *info)
{
 return(GenMMC1Init(info, 256, 0, 16, info->battery));
}


