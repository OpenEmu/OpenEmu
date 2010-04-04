/* Cygne
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Dox dox@space.pl
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

#include "wswan.h"
#include "../netplay.h"
#include "../md5.h"
#include "../mempatcher.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <zlib.h>

#include "gfx.h"
#include "memory.h"
#include "start.h"
#include "sound.h"
#include "v30mz.h"
#include "rtc.h"
#include "eeprom.h"
#include "debug.h"

int 		wsc = 1;			/*color/mono*/
uint32		rom_size;

uint16 WSButtonStatus;

static void Reset(void)
{
	int		u0;

	v30mz_reset();				/* Reset CPU */
	WSwan_MemoryReset();
        WSwan_GfxReset();
        WSwan_SoundReset();
	WSwan_InterruptReset();
        WSwan_RTCReset();
	WSwan_EEPROMReset();

	for(u0=0;u0<0xc9;u0++)
	 WSwan_writeport(u0,startio[u0]);

	v30mz_set_reg(NEC_SS,0);
	v30mz_set_reg(NEC_SP,0x2000);
}

static uint8 *chee;
static void Emulate(EmulateSpecStruct *espec)
{
 MDFNGameInfo->fb = espec->pixels;

 WSButtonStatus = chee[0] | (chee[1] << 8);

 MDFNMP_ApplyPeriodicCheats();

 while(!wsExecuteLine(espec->pixels, espec->skip))
 {

 }


 *(espec->SoundBuf) = WSwan_SoundFlush(espec->SoundBufSize);
}

typedef struct
{
 const uint8 id;
 const char *name;
} DLEntry;

static DLEntry Developers[] =
{
 { 0x01, "Bandai" },
 { 0x02, "Taito" },
 { 0x03, "Tomy" },
 { 0x04, "Koei" },
 { 0x05, "Data East" },
 { 0x06, "Asmik" }, // Asmik Ace?
 { 0x07, "Media Entertainment" },
 { 0x08, "Nichibutsu" },
 { 0x0A, "Coconuts Japan" },
 { 0x0B, "Sammy" },
 { 0x0C, "Sunsoft" },
 { 0x0D, "Mebius" },
 { 0x0E, "Banpresto" },
 { 0x10, "Jaleco" },
 { 0x11, "Imagineer" },
 { 0x12, "Konami" },
 { 0x16, "Kobunsha" },
 { 0x17, "Bottom Up" },
 { 0x18, "Naxat" },	// Mechanic Arms?  Media Entertainment? Argh!
 { 0x19, "Sunrise" },
 { 0x1A, "Cyberfront" },
 { 0x1B, "Megahouse" },
 { 0x1D, "Interbec" },
 { 0x1E, "NAC" },
 { 0x1F, "Emotion" }, // Bandai Visual??
 { 0x20, "Athena" },
 { 0x21, "KID" },
 { 0x24, "Omega Micott" },
 { 0x25, "Upstar" },
 { 0x26, "Kadokawa/Megas" },
 { 0x27, "Cocktail Soft" },
 { 0x28, "Squaresoft" },
 { 0x2B, "TomCreate" },
 { 0x2D, "Namco" },
 { 0x2F, "Gust" },
 { 0x36, "Capcom" },
};

static bool TestMagic(const char *name, MDFNFILE *fp)
{
// if(strcasecmp(fp->ext, "ws") && strcasecmp(fp->ext, "wsc"))
//  return(FALSE);

 if(fp->size < 65536)
  return(FALSE);

 return(TRUE);
}

static int Load(const char *name, MDFNFILE *fp)
{
 if(!TestMagic(name, fp))
  return(-1);

 rom_size = uppow2(fp->size);

 wsCartROM = (uint8 *)calloc(1, rom_size);

 memcpy(wsCartROM, fp->data, fp->size);

 MDFN_printf(_("ROM:       %dKiB\n"), rom_size / 1024);
 md5_context md5;
 md5.starts();
 md5.update(wsCartROM, rom_size);
 md5.finish(MDFNGameInfo->MD5);
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());

 uint8 header[10];
 memcpy(header, wsCartROM + rom_size - 10, 10);

 {
  const char *developer_name = "???";
  for(unsigned int x = 0; x < sizeof(Developers) / sizeof(DLEntry); x++)
  {
   if(Developers[x].id == header[0])
   {
    developer_name = Developers[x].name;
    break;
   }
  }
  MDFN_printf(_("Developer: %s (0x%02x)\n"), developer_name, header[0]);
 }

 uint32 SRAMSize = 0;
 eeprom_size = 0;

 switch(header[5])
 {
  case 0x01: SRAMSize = 8*1024; break;
  case 0x02: SRAMSize = 32*1024; break;
  case 0x03: SRAMSize = 16 * 65536; break;
  case 0x04: SRAMSize = 32 * 65536; break; // Dicing Knight!

  case 0x10: eeprom_size = 128; break;
  case 0x20: eeprom_size = 2*1024; break;
  case 0x50: eeprom_size = 1024; break;
 }

 //printf("%02x\n", header[5]);

 if(eeprom_size)
  MDFN_printf(_("EEPROM:  %d bytes\n"), eeprom_size);

 if(SRAMSize)
  MDFN_printf(_("Battery-backed RAM:  %d bytes\n"), SRAMSize);

 MDFN_printf(_("Recorded Checksum:  0x%04x\n"), header[8] | (header[9] << 8));
 {
  uint16 real_crc = 0;
  for(unsigned int i = 0; i < rom_size - 2; i++)
   real_crc += wsCartROM[i];
  MDFN_printf(_("Real Checksum:      0x%04x\n"), real_crc);
 }

 if((header[8] | (header[9] << 8)) == 0x8de1 && (header[0]==0x01)&&(header[2]==0x27)) /* Detective Conan */
 {
  //puts("HAX");
  /* WS cpu is using cache/pipeline or there's protected ROM bank where pointing CS */
  wsCartROM[0xfffe8]=0xea;
  wsCartROM[0xfffe9]=0x00;
  wsCartROM[0xfffea]=0x00;
  wsCartROM[0xfffeb]=0x00;
  wsCartROM[0xfffec]=0x20;
 }

 if(header[6] & 0x1)
  MDFNGameInfo->rotated = MDFN_ROTATE90;

 MDFNMP_Init(16384, (1 << 20) / 1024);

 v30mz_init(WSwan_readmem20, WSwan_writemem20, WSwan_readport, WSwan_writeport);
 WSwan_MemoryInit(wsc, SRAMSize); // EEPROM and SRAM are loaded in this func.
 WSwan_GfxInit();
 MDFN_LoadGameCheats(NULL);
 MDFNGameInfo->fps = (uint32)((uint64)3072000 * 65536 * 256 / (159*256));
 MDFNGameInfo->GameSetMD5Valid = FALSE;

 if(MDFN_GetSettingB("wswan.forcemono"))
 {
  MDFNGameInfo->soundchan = 1;
  WSwan_SoundInit(1);
 }
 else
 {
  MDFNGameInfo->soundchan = 2;
  WSwan_SoundInit(0);
 }
 wsMakeTiles();

 Reset();

 return(1);
}

static void CloseGame(void)
{
 WSwan_MemoryKill(); // saves sram/eeprom

 MDFN_FlushGameCheats(0);

 if(wsCartROM)
 {
  free(wsCartROM);
  wsCartROM = NULL;
 }
}

static void SetInput(int port, const char *type, void *ptr)
{
 if(!port) chee = (uint8 *)ptr;
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 if(!v30mz_StateAction(sm, load, data_only))
  return(0);
 
 // Call MemoryStateAction before others StateActions...
 if(!WSwan_MemoryStateAction(sm, load, data_only))
  return(0);

 if(!WSwan_GfxStateAction(sm, load, data_only))
  return(0);

 if(!WSwan_RTCStateAction(sm, load, data_only))
  return(0);

 if(!WSwan_InterruptStateAction(sm, load, data_only))
  return(0);

 if(!WSwan_SoundStateAction(sm, load, data_only))
  return(0);

 if(!WSwan_EEPROMStateAction(sm, load, data_only))
 {
  puts("Oops");
  return(0);
 }

 return(1);
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFNNPCMD_POWER:
  case MDFNNPCMD_RESET: Reset();
                        break;
 }
}

static MDFNSetting WSwanSettings[] =
{
 //{ "ngp.language", "If =1, tell games to display in English, if =0, in Japanese.", MDFNST_UINT, "1", "0", "1" },
 { "wswan.rotateinput",  gettext_noop("Virtually rotate D-pads along with screen."), MDFNST_BOOL, "0" },
 { "wswan.forcemono", gettext_noop("Force monophonic sound output."), MDFNST_BOOL, "0" },
 { "wswan.name", gettext_noop("Name"), MDFNST_STRING, "Mednafen" },
 { "wswan.byear", gettext_noop("Birth Year"), MDFNST_UINT, "1989", "0", "9999" },
 { "wswan.bmonth", gettext_noop("Birth Month"), MDFNST_UINT, "6", "1", "12" },
 { "wswan.bday", gettext_noop("Birth Day"), MDFNST_UINT, "23", "1", "31" },
 { "wswan.sex", gettext_noop("Sex"), MDFNST_STRING, "F" },
 { "wswan.blood", gettext_noop("Blood Type"), MDFNST_STRING, "O" },
 { NULL }
};

static const InputDeviceInputInfoStruct IDII[] =
{
 { "up-x", "UP ↑, X Cursors", 0, IDIT_BUTTON, "down-x" },
 { "right-x", "RIGHT →, X Cursors", 3, IDIT_BUTTON, "left-x" },
 { "down-x", "DOWN ↓, X Cursors", 1, IDIT_BUTTON, "up-x" },
 { "left-x", "LEFT ←, X Cursors", 2, IDIT_BUTTON, "right-x" },

 { "up-y", "UP ↑, Y Cur: MUST NOT = X CURSORS", 4, IDIT_BUTTON, "down-y" },
 { "right-y", "RIGHT →, Y Cur: MUST NOT = X CURSORS", 7, IDIT_BUTTON, "left-y" },
 { "down-y", "DOWN ↓, Y Cur: MUST NOT = X CURSORS", 5, IDIT_BUTTON, "up-y" },
 { "left-y", "LEFT ←, Y Cur: MUST NOT = X CURSORS", 6, IDIT_BUTTON, "right-y" },

 { "start", "Start", 8, IDIT_BUTTON, NULL },
 { "a", "A", 10, IDIT_BUTTON_CAN_RAPID,  NULL },
 { "b", "B", 9, IDIT_BUTTON_CAN_RAPID, NULL },
};

static InputDeviceInfoStruct InputDeviceInfo[] =
{
 {
  "gamepad",
  "Gamepad",
  NULL,
  sizeof(IDII) / sizeof(InputDeviceInputInfoStruct),
  IDII,
 }
};

static const InputPortInfoStruct PortInfo[] =
{
 { 0, "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};


#ifdef WANT_DEBUGGER
static DebuggerInfoStruct DBGInfo =
{
 7 + 1 + 8, // Fixme, probably not right...  maximum number of prefixes + 1 for opcode + 4 for operand(go with 8 to be safe)

 16,
 20,
 0x0000,
 ~0,

 WSwanDBG_MemPeek,
 WSwanDBG_Disassemble,
 WSwanDBG_ToggleSyntax,
 WSwanDBG_IRQ,
 NULL, //NESDBG_GetVector,
 WSwanDBG_FlushBreakPoints,
 WSwanDBG_AddBreakPoint,
 WSwanDBG_SetCPUCallback,
 WSwanDBG_SetBPCallback,
 WSwanDBG_GetBranchTrace,
 WSwan_GfxSetGraphicsDecode,
 WSwan_GfxGetGraphicsDecodeBuffer
};
#endif

MDFNGI EmulatedWSwan =
{
 "wswan",
 #ifdef WANT_DEBUGGER
 &DBGInfo,
 #endif
 &InputInfo,
 Load,
 NULL,
 CloseGame,
 WSwan_GfxToggleLayer,
 "Background\0Foreground\0Sprites\0",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 WSwan_SetPixelFormat,
 SetInput,
 NULL,
 NULL,
 NULL,
 WSwan_SetSoundMultiplier,
 WSwan_SetSoundVolume,
 WSwan_Sound,
 DoSimpleCommand,
 WSwanSettings,
 0,
 NULL,
 224,
 144,
 224, // Save state preview width
 256 * sizeof(uint32),
 {0, 0, 224, 144},
};

