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

#include <zlib.h>
#include "pce.h"
#include "vdc.h"
#include "psg.h"
#include "input.h"
#include "huc.h"
#include "cdrom.h"
#include "hes.h"
#include "debug.h"
#include "tsushin.h"
#include "../netplay.h"
#include "../mempatcher.h"

#ifdef WANT_DEBUGGER
static RegType SGXRegsVDC[] =
{
        { "MAWR-B", "Memory Write Address, VDC-B", 2 },
        { "MARR-B", "Memory Read Address, VDC-B", 2 },
        { "CR-B", "Control, VDC-B", 2 },
        { "RCR-B", "Raster Counter, VDC-B", 2 },
        { "BXR-B", "X Scroll, VDC-B", 2 },
        { "BYR-B", "Y Scroll, VDC-B", 2 },
        { "MWR-B", "Memory Width, VDC-B", 2 },

        { "HSR-B", "HSR, VDC-B", 2 },
        { "HDR-B", "HDR, VDC-B", 2 },
        { "VSR-B", "VSR, VDC-B", 2 },
        { "VDR-B", "VDR, VDC-B", 2 },

        { "VCR-B", "VCR, VDC-B", 2 },
        { "DCR-B", "DMA Control, VDC-B", 2 },
        { "SOUR-B", "VRAM DMA Source Address, VDC-B", 2 },
        { "DESR-B", "VRAM DMA Dest Address, VDC-B", 2 },
        { "LENR-B", "VRAM DMA Length, VDC-B", 2 },
        { "SATB-B", "SATB Update Address, VDC-B", 2 },
        { "", "", 0 },
};

static RegGroupType SGXRegsVDCGroup =
{
        SGXRegsVDC,
        PCEDBG_GetRegister,
        PCEDBG_SetRegister
};

static RegType PCERegs[] =
{
        { "PC", "Program Counter", 2 },
        { "A", "Accumulator", 1 },
        { "X", "X Index Register", 1 },
        { "Y", "Y Index Register", 1 },
        { "SP", "Stack Pointer", 1 },
        { "P", "Status", 1 },
        { "MPR0", "MPR0", 1},
        { "MPR1", "MPR1", 1},
        { "MPR2", "MPR2", 1},
        { "MPR3", "MPR3", 1},
        { "MPR4", "MPR4", 1},
        { "MPR5", "MPR5", 1},
        { "MPR6", "MPR6", 1},
        { "MPR7", "MPR7", 1},
        { "SPD", "CPU Speed", 1 },
        { "IRQM", "IRQ Mask", 1},
        { "TIMS", "Timer Status", 1},
        { "TIMV", "Timer Value", 1},
        { "TIML", "Timer Load", 1},
        { "TIMD", "Timer Div Counter", 2},
        { "", "", 0 },
};

static RegGroupType PCERegsGroup =
{
        PCERegs,
        PCEDBG_GetRegister,
        PCEDBG_SetRegister
};


static RegType PCERegsVDC[] =
{
        { "MAWR", "Memory Write Address", 2 },
        { "MARR", "Memory Read Address", 2 },
        { "CR", "Control", 2 },
        { "RCR", "Raster Counter", 2 },
        { "BXR", "X Scroll", 2 },
        { "BYR", "Y Scroll", 2 },
        { "MWR", "Memory Width", 2 },

        { "HSR", "HSR", 2 },
        { "HDR", "HDR", 2 },
        { "VSR", "VSR", 2 },
        { "VDR", "VDR", 2 },

        { "VCR", "VCR", 2 },
        { "DCR", "DMA Control", 2 },
        { "SOUR", "VRAM DMA Source Address", 2 },
        { "DESR", "VRAM DMA Dest Address", 2 },
        { "LENR", "VRAM DMA Length", 2 },
        { "SATB", "SATB Update Address", 2 },

        { "Frm Cntr", "Frame Counter", 2 },
        { "VCECR", "VCE CR", 2 },
        { "", "", 0 },
};

static RegGroupType PCERegsVDCGroup =
{
        PCERegsVDC,
        PCEDBG_GetRegister,
        PCEDBG_SetRegister
};

#endif

bool PCE_ACEnabled;
bool PCE_InDebug = 0;
uint64 PCE_TimestampBase;

extern MDFNGI EmulatedPCE;
static bool IsSGX;
static bool IsHES;
int pce_overclocked;

// Accessed in debug.cpp
static uint8 BaseRAM[32768]; // 8KB for PCE, 32KB for Super Grafx

uint8 PCEIODataBuffer;
readfunc PCERead[0x100], NonCheatPCERead[0x100];
writefunc PCEWrite[0x100];

static DECLFR(PCEBusRead)
{
 //printf("BUS Read: %02x:%04x(%06x)\n", A >> 13, A & 0x1FFF, A);
 return(0xFF);
}

static DECLFW(PCENullWrite)
{
 //printf("Null Write: %02x:%04x(%06x) %02x\n", A >> 13, A & 0x1FFF, A, V);
}

static DECLFR(BaseRAMReadSGX)
{
 return(BaseRAM[A & 0x7FFF]);
}

static DECLFW(BaseRAMWriteSGX)
{
 BaseRAM[A & 0x7FFF] = V;
}

static DECLFR(BaseRAMRead)
{
 return(BaseRAM[A & 0x1FFF]);
}

static DECLFW(BaseRAMWrite)
{
 BaseRAM[A & 0x1FFF] = V;
}

static DECLFR(IORead)
{
 A &= 0x1FFF;

 switch(A & 0x1c00)
 {
  case 0x0000: if(!PCE_InDebug) HuC6280_StealCycles(1); 
	       return(VDC_Read(A));
  case 0x0400: if(!PCE_InDebug) HuC6280_StealCycles(1); 
	       return(VCE_Read(A));
  case 0x0800: if(HuCPU.in_block_move)
		return(0);
	       return(PCEIODataBuffer); 
  case 0x0c00: if(HuCPU.in_block_move)
                return(0);
	       {
		uint8 ret = HuC6280_TimerRead(A);
                if(!PCE_InDebug) PCEIODataBuffer = ret;
                return(ret);
               }
  case 0x1000: if(HuCPU.in_block_move)
                return(0);
	       {
	        uint8 ret = INPUT_Read(A);
                if(!PCE_InDebug) PCEIODataBuffer = ret;
                return(ret);
               }
  case 0x1400: if(HuCPU.in_block_move)
                return(0);
	       {
	        uint8 ret = HuC6280_IRQStatusRead(A);
		if(!PCE_InDebug) PCEIODataBuffer = ret;
	        return(ret);
	       }
  case 0x1800: if(IsTsushin)
		return(PCE_TsushinRead(A));

	       if(!PCE_IsCD) break;
	       if((A & 0x1E00) == 0x1A00)
	       {
		if(PCE_ACEnabled)
		 return(PCE_ACRead(A));
		else
		 return(0);
	       }
	       else
		return(PCECD_Read(A));
  case 0x1C00: if(IsHES) return(ReadIBP(A)); 
	       break; // Expansion
  //default: printf("Eeep\n");break;
 }
 return(0xFF);
}

static DECLFW(IOWrite)
{
 A &= 0x1FFF;
  
 switch(A & 0x1c00)
 {
  case 0x0000: if(!PCE_InDebug) HuC6280_StealCycles(1);
	       VDC_Write(A, V); break;
  case 0x0400: if(!PCE_InDebug) HuC6280_StealCycles(1); 
	       VCE_Write(A, V); break;
  case 0x0800: PCEIODataBuffer = V; PSG_Write(A, V); break;
  case 0x0c00: PCEIODataBuffer = V; HuC6280_TimerWrite(A, V); break;
  case 0x1000: PCEIODataBuffer = V; INPUT_Write(A, V); break;
  case 0x1400: PCEIODataBuffer = V; HuC6280_IRQStatusWrite(A, V); break;
  case 0x1800: if(IsTsushin)
                PCE_TsushinWrite(A, V);

	       if(!PCE_IsCD) break;
	       if((A & 0x1E00) == 0x1A00)
	       {
		if(PCE_ACEnabled)
		 PCE_ACWrite(A, V);
	       }
	       else
	        PCECD_Write(A, V); 
	       break;
  //case 0x1C00: break; // Expansion
  //default: printf("Eep: %04x\n", A); break;
 }

}

static int LoadCommon(void);
static void LoadCommonPre(void);

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 if(memcmp(fp->data, "HESM", 4) && strcasecmp(fp->ext, "pce") && strcasecmp(fp->ext, "sgx"))
  return(FALSE);

 return(TRUE);
}

static int Load(const char *name, MDFNFILE *fp)
{
 uint32 headerlen = 0;
 uint32 r_size;

 IsHES = 0;

 if(!memcmp(fp->data, "HESM", 4))
  IsHES = 1;

 if(strcasecmp(fp->ext, "pce") && strcasecmp(fp->ext, "sgx") && !IsHES)
  return(-1);

 LoadCommonPre();

 if(!IsHES)
 {
  if(fp->size & 0x200) // 512 byte header!
   headerlen = 512;
 }

 r_size = fp->size - headerlen;
 if(r_size > 4096 * 1024) r_size = 4096 * 1024;

 for(int x = 0; x < 0x100; x++)
 {
  PCERead[x] = PCEBusRead;
  PCEWrite[x] = PCENullWrite;
 }

 uint32 crc = crc32(0, fp->data + headerlen, fp->size - headerlen);


 if(IsHES)
 {
  if(!PCE_HESLoad(fp->data, fp->size))
   return(0);
 }
 else
  HuCLoad(fp->data + headerlen, fp->size - headerlen, crc);

 if(!strcasecmp(fp->ext, "sgx"))
  IsSGX = TRUE;

 if(fp->size >= 8192 && !memcmp(fp->data + headerlen, "DARIUS Version 1.11b", strlen("DARIUS VERSION 1.11b")))
 {
  MDFN_printf("SuperGfx:  Darius Plus\n");
  IsSGX = 1;
 }

 if(crc == 0x4c2126b0)
 {
  MDFN_printf("SuperGfx:  Aldynes\n");
  IsSGX = 1;
 }

 if(crc == 0x8c4588e2)
 {
  MDFN_printf("SuperGfx:  1941 - Counter Attack\n");
  IsSGX = 1;
 }
 if(crc == 0x1f041166)
 {
  MDFN_printf("SuperGfx:  Madouou Granzort\n");
  IsSGX = 1;
 }
 if(crc == 0xb486a8ed)
 {
  MDFN_printf("SuperGfx:  Daimakaimura\n");
  IsSGX = 1;
 }
 if(crc == 0x3b13af61)
 {
  MDFN_printf("SuperGfx:  Battle Ace\n");
  IsSGX = 1;
 }

 return(LoadCommon());
}

static void LoadCommonPre(void)
{
 // FIXME:  Make these globals less global!
 pce_overclocked = MDFN_GetSettingUI("pce.ocmultiplier");
 PCE_ACEnabled = MDFN_GetSettingB("pce.arcadecard");

 if(pce_overclocked > 1)
  MDFN_printf(_("CPU overclock: %dx\n"), pce_overclocked);

 if(MDFN_GetSettingUI("pce.cdspeed") > 1)
  MDFN_printf(_("CD-ROM speed:  %ux\n"), (unsigned int)MDFN_GetSettingUI("pce.cdspeed"));

 memset(HuCPUFastMap, 0, sizeof(HuCPUFastMap));
 for(int x = 0; x < 0x100; x++)
 {
  PCERead[x] = PCEBusRead;
  PCEWrite[x] = PCENullWrite;
 }

 MDFNMP_Init(1024, (1 << 21) / 1024);

 #ifdef WANT_DEBUGGER
 MDFNDBG_AddRegGroup(&PCERegsGroup);
 MDFNDBG_AddRegGroup(&PCERegsVDCGroup);
 #endif
}

static int LoadCommon(void)
{ 
 IsSGX |= MDFN_GetSettingB("pce.forcesgx") ? 1 : 0;

 if(IsHES)
  IsSGX = 1;
 // Don't modify IsSGX past this point.
 

 #ifdef WANT_DEBUGGER
 MDFNDBG_AddASpace(PCEDBG_GetAddressSpaceBytes, PCEDBG_PutAddressSpaceBytes, "cpu", "CPU Logical", 16);
 MDFNDBG_AddASpace(PCEDBG_GetAddressSpaceBytes, PCEDBG_PutAddressSpaceBytes, "physical", "CPU Physical", 21);
 MDFNDBG_AddASpace(PCEDBG_GetAddressSpaceBytes, PCEDBG_PutAddressSpaceBytes, "ram", "RAM", IsSGX ? 15 : 13);
 #endif

 VDC_Init(IsSGX);

 if(IsSGX)
 {
  MDFN_printf("SuperGrafx Emulation Enabled.\n");
  PCERead[0xF8] = PCERead[0xF9] = PCERead[0xFA] = PCERead[0xFB] = BaseRAMReadSGX;
  PCEWrite[0xF8] = PCEWrite[0xF9] = PCEWrite[0xFA] = PCEWrite[0xFB] = BaseRAMWriteSGX;

  for(int x = 0xf8; x < 0xfb; x++)
   HuCPUFastMap[x] = BaseRAM - 0xf8 * 8192;
 }
 else
 {
  PCERead[0xF8] = PCERead[0xF9] = PCERead[0xFA] = PCERead[0xFB] = BaseRAMRead;
  PCEWrite[0xF8] = PCEWrite[0xF9] = PCEWrite[0xFA] = PCEWrite[0xFB] = BaseRAMWrite;

  for(int x = 0xf8; x < 0xfb; x++)
   HuCPUFastMap[x] = BaseRAM - x * 8192;
 }

 MDFNMP_AddRAM(IsSGX ? 32768 : 8192, 0xf8 * 8192, BaseRAM);

 PCERead[0xFF] = IORead;
 PCEWrite[0xFF] = IOWrite;

 HuC6280_Init();

 #ifdef WANT_DEBUGGER
 if(IsSGX)
  MDFNDBG_AddRegGroup(&SGXRegsVDCGroup);
 #endif

 MDFNGameInfo->pitch = 1024 * sizeof(uint32);

 if(MDFN_GetSettingB("pce.forcemono"))
 {
  MDFNGameInfo->soundchan = 1;
  PSG_Init(1);
 }
 else
 {
  MDFNGameInfo->soundchan = 2;
  PSG_Init(0);
 }

 PCEINPUT_Init();

 PCE_Power();

 MDFNGameInfo->LayerNames = IsSGX ? "BG0\0SPR0\0BG1\0SPR1\0" : "Background\0Sprites\0";
 MDFNGameInfo->fps = (uint32)((double)7159090.90909090 / 455 / 263 * 65536 * 256);


 memcpy(NonCheatPCERead, PCERead, sizeof(PCERead));

 MDFN_LoadGameCheats(0);
 MDFNMP_InstallReadPatches();

 MDFNGameInfo->DisplayRect.y = MDFN_GetSettingUI("pce.slstart");
 MDFNGameInfo->DisplayRect.h = MDFN_GetSettingUI("pce.slend") - MDFNGameInfo->DisplayRect.y + 1;

 return(1);
}

static int LoadCD(void)
{
 IsHES = 0;
 IsSGX = 0;

 LoadCommonPre();

 if(!HuCLoadCD(/*MDFN_GetSettingS("pce.cdbios").c_str())*/"/syscard3.pce"))
  return(0);

 return(LoadCommon());
}


static void CloseGame(void)
{
 if(IsHES)
  HES_Close();
 else
 {
  MDFN_FlushGameCheats(0);
  HuCClose();
 }
 VDC_Close();
 PSG_Kill();
}

static void Emulate(EmulateSpecStruct *espec)
{
 MDFNGameInfo->fb = espec->pixels;
	INPUT_Frame();

 MDFNMP_ApplyPeriodicCheats();

 VDC_RunFrame(espec->pixels, espec->LineWidths, IsHES ? 1 : espec->skip);

 *(espec->SoundBuf) = PSG_Flush(espec->SoundBufSize);

 if(IsHES && !espec->skip)
  HES_Draw(espec->pixels, *(espec->SoundBuf), *(espec->SoundBufSize));
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(BaseRAM, IsSGX? 32768 : 8192),
  SFVAR(PCEIODataBuffer),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");

 ret &= HuC6280_StateAction(sm, load, data_only);
 ret &= VDC_StateAction(sm, load, data_only);
 ret &= PSG_StateAction(sm, load, data_only);
 ret &= INPUT_StateAction(sm, load, data_only);
 ret &= HuC_StateAction(sm, load, data_only);

 if(load)
 {

 }

 return(ret);
}

void PCE_Power(void)
{
 memset(BaseRAM, 0x00, sizeof(BaseRAM));
 PCEIODataBuffer = 0xFF;

 HuC6280_Power();
 VDC_Power();
 PSG_Power();
 HuC_Power();
 PCEINPUT_Power();

 if(IsHES)
  HES_Reset();

 if(PCE_IsCD)
  PCECD_Power();
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFNNPCMD_RESET: PCE_Power(); break;
  case MDFNNPCMD_POWER: PCE_Power(); break;
 }
}

static MDFNSetting PCESettings[] = 
{
  { "pce.input.port1", gettext_noop("Select input device for input port 1."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce.input.port2", gettext_noop("Select input device for input port 2."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce.input.port3", gettext_noop("Select input device for input port 3."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce.input.port4", gettext_noop("Select input device for input port 4."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce.input.port5", gettext_noop("Select input device for input port 5."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce.slstart", gettext_noop("First rendered scanline."), MDFNST_UINT, "4" },
  { "pce.slend", gettext_noop("Last rendered scanline."), MDFNST_UINT, "235" },
  { "pce.mouse_sensitivity", gettext_noop("Set mouse sensitivity."), MDFNST_FLOAT, "0.50", NULL, NULL, NULL, PCEINPUT_SettingChanged },
  { "pce.disable_softreset", gettext_noop("If set, when RUN+SEL are pressed simultaneously, disable both buttons temporarily."), MDFNST_BOOL, "0", NULL, NULL, NULL, PCEINPUT_SettingChanged },
  { "pce.forcemono", gettext_noop("Force monophonic sound output."), MDFNST_BOOL, "0" },
  { "pce.forcesgx", gettext_noop("Force SuperGrafx emulation."), MDFNST_BOOL, "0" },
  { "pce.arcadecard", gettext_noop("Enable Arcade Card emulation."), MDFNST_BOOL, "1" },
  { "pce.ocmultiplier", gettext_noop("CPU overclock multiplier."), MDFNST_UINT, "1", "1", "50"},
  { "pce.cdspeed", gettext_noop("CD-ROM data transfer speed multiplier."), MDFNST_UINT, "1", "1", "100" },
  { "pce.nospritelimit", gettext_noop("No 16-sprites-per-scanline limit option."), MDFNST_BOOL, "0" },
  { "pce.colormap", gettext_noop("Load custom color map from specified file."), MDFNST_STRING, "" },

  { "pce.cdbios", gettext_noop("Path to the CD BIOS"), MDFNST_STRING, "pce.cdbios PATH NOT SET" },
  { "pce.adpcmlp", gettext_noop("Enable lowpass filter dependent on playback-frequency."), MDFNST_BOOL, "0" },
  { "pce.cdpsgvolume", gettext_noop("PSG volume when playing a CD game."), MDFNST_UINT, "100", "0", "200" },
  { "pce.cddavolume", gettext_noop("CD-DA volume."), MDFNST_UINT, "100", "0", "200" },
  { "pce.adpcmvolume", gettext_noop("ADPCM volume."), MDFNST_UINT, "100", "0", "200" },
  { NULL }
};

static DECLFR(CheatReadFunc)
{
  std::vector<SUBCHEAT>::iterator chit;
  uint8 retval = NonCheatPCERead[(A / 8192) & 0xFF](A);

  for(chit = SubCheats[A & 0x7].begin(); chit != SubCheats[A & 0x7].end(); chit++)
  {
   if(A == chit->addr)
   {
    if(chit->compare == -1 || chit->compare == retval)
    {
     retval = chit->value;
     break;
    }
   }
  }
 return(retval);
}

static uint8 MemRead(uint32 addr)
{
 return(NonCheatPCERead[(addr / 8192) & 0xFF](addr));
}

static void InstallReadPatch(uint32 address)
{
 HuCPUFastMap[address / 8192] = NULL;
 PCERead[address / 8192] = CheatReadFunc;
}

static void RemoveReadPatches(void)
{
 for(int x = 0; x < 0x100; x++)
  PCERead[x] = NonCheatPCERead[x];
}

#ifdef WANT_DEBUGGER
void PCEDBG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 PCE_InDebug = 1;

 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFF;

   uint8 wmpr = HuCPU.MPR[Address >> 13];
   *Buffer = PCERead[wmpr]((wmpr << 13) | (Address & 0x1FFF));

   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "physical"))
 {
  while(Length--)
  {
   Address &= 0x1FFFFF;

   uint8 wmpr = Address >> 13;
   *Buffer = PCERead[wmpr]((wmpr << 13) | (Address & 0x1FFF));

   Address++;
   Buffer++;
  }

 }
 else if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= (IsSGX ? 32768 : 8192) - 1;
   *Buffer = BaseRAM[Address];
   Address++;
   Buffer++;
  }
 }
 PCE_InDebug = 0;
}

void PCEDBG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 PCE_InDebug = 1;

 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFF;

   uint8 wmpr = HuCPU.MPR[Address >> 13];
   if(hl)
   {
    if(HuCPUFastMap[wmpr])
     HuCPUFastMap[wmpr][((wmpr << 13) | (Address & 0x1FFF))] = *Buffer;
   }
   else
    PCEWrite[wmpr](Address, *Buffer);

   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "physical"))
 {
  while(Length--)
  {
   Address &= 0x1FFFFF;

   uint8 wmpr = Address >> 13;

   if(hl)
   {
    if(HuCPUFastMap[wmpr])
     HuCPUFastMap[wmpr][((wmpr << 13) | (Address & 0x1FFF))] = *Buffer;
   }
   else
    PCEWrite[wmpr](Address, *Buffer);
   Address++;
   Buffer++;
  }

 }
 else if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= (IsSGX ? 32768 : 8192) - 1;
   BaseRAM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }

 PCE_InDebug = 0;
}


static DebuggerInfoStruct DBGInfo =
{
 7,
 16,
 21,
 0x2000,
 0x2000, // ZP

 PCEDBG_MemPeek,
 PCEDBG_Disassemble,
 NULL,
 PCEDBG_IRQ,
 NULL, //NESDBG_GetVector,
 PCEDBG_FlushBreakPoints,
 PCEDBG_AddBreakPoint,
 PCEDBG_SetCPUCallback,
 PCEDBG_SetBPCallback,
 PCEDBG_GetBranchTrace,
 VDC_SetGraphicsDecode,
 VDC_GetGraphicsDecodeBuffer,
 PCEDBG_SetLogFunc,
};
#endif

MDFNGI EmulatedPCE =
{
 "pce",
 #ifdef WANT_DEBUGGER
 &DBGInfo,
 #endif
 &PCEInputInfo,
 Load,
 LoadCD,
 CloseGame,
 VDC_ToggleLayer,
 NULL,
 InstallReadPatch,
 RemoveReadPatches,
 MemRead,
 StateAction,
 Emulate,
 VDC_SetPixelFormat,
 PCEINPUT_SetInput,
 NULL,
 NULL,
 NULL,
 PSG_SetSoundMultiplier,
 PSG_SetSoundVolume,
 PSG_Sound,
 DoSimpleCommand,
 PCESettings,
 0,
 NULL,
 320,
 232,
 256, // Save state preview width
 1024 * sizeof(uint32),
 { 0, 4, 320, 232 },
};

