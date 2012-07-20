#include "stella.h"
#include "../md5.h"
#include "../mempatcher.h"
#include "../player.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <zlib.h>

#include "Settings.hxx"
#include "TIA.hxx"
#include "Props.hxx"
#include "PropsSet.hxx"
#include "Cart.hxx"
#include "Console.hxx"
#include "Serializer.hxx"
#include "Event.hxx"
#include "Switches.hxx"
#include "MD5.hxx"
#include "SoundSDL.hxx"

namespace MDFN_IEN_WSWAN
{


int 		wsc = 1;			/*color/mono*/
uint32		rom_size;

uint16 WSButtonStatus;


static bool IsWSR;
static uint8 WSRCurrentSong;

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
	{
	 if(u0 != 0xC4 && u0 != 0xC5 && u0 != 0xBA && u0 != 0xBB)
	  WSwan_writeport(u0,startio[u0]);
	}

	v30mz_set_reg(NEC_SS,0);
	v30mz_set_reg(NEC_SP,0x2000);

	if(IsWSR)
	{
	 v30mz_set_reg(NEC_AW, WSRCurrentSong);
	}
}

static uint8 *chee;
static void Emulate(EmulateSpecStruct *espec)
{
 espec->DisplayRect.x = 0;
 espec->DisplayRect.y = 0;
 espec->DisplayRect.w = 224;
 espec->DisplayRect.h = 144;

 if(espec->VideoFormatChanged)
  WSwan_SetPixelFormat(espec->surface->format);

 if(espec->SoundFormatChanged)
  WSwan_SetSoundRate(espec->SoundRate);

 uint16 butt_data = chee[0] | (chee[1] << 8);

 WSButtonStatus = butt_data;
 

 MDFNMP_ApplyPeriodicCheats();

 while(!wsExecuteLine(espec->surface, espec->skip))
 {

 }


 espec->SoundBufSize = WSwan_SoundFlush(espec->SoundBuf, espec->SoundBufMaxSize);

 espec->MasterCycles = v30mz_timestamp;
 v30mz_timestamp = 0;

 if(IsWSR)
 {
  bool needreload = FALSE;
  static uint16 last;

  Player_Draw(espec->surface, &espec->DisplayRect, WSRCurrentSong, espec->SoundBuf, espec->SoundBufSize);

  if((WSButtonStatus & 0x02) && !(last & 0x02))
  {
   WSRCurrentSong++;
   needreload = 1;
  }

  if((WSButtonStatus & 0x08) && !(last & 0x08))
  {
   WSRCurrentSong--;
   needreload = 1;
  }

  if((WSButtonStatus & 0x100) && !(last & 0x100))
   needreload = 1;

  if((WSButtonStatus & 0x01) && !(last & 0x01))
  {
   WSRCurrentSong += 10;
   needreload = 1;
  }

  if((WSButtonStatus & 0x04) && !(last & 0x04))
  {
   WSRCurrentSong -= 10;
   needreload = 1;
  }


  last = WSButtonStatus;

  if(needreload)
   Reset();
 }
}

typedef struct
{
 const uint8 id;
 const char *name;
} DLEntry;

static const DLEntry Developers[] =
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
 if(strcasecmp(fp->ext, "ws") && strcasecmp(fp->ext, "wsc") && strcasecmp(fp->ext, "wsr"))
  return(FALSE);

 if(fp->size < 65536)
  return(FALSE);

 return(TRUE);
}

static int Load(const char *name, MDFNFILE *fp)
{
 uint32 real_rom_size;

 if(fp->size < 65536)
 {
  MDFN_PrintError(_("%s ROM image is too small."), MDFNGameInfo->fullname);
  return(0);
 }

 if(!memcmp(fp->data + fp->size - 0x20, "WSRF", 4))
 {
  const uint8 *wsr_footer = fp->data + fp->size - 0x20;

  IsWSR = TRUE;
  WSRCurrentSong = wsr_footer[0x5];

  Player_Init(256, "", "", "");
 }
 else
  IsWSR = false;

 real_rom_size = (fp->size + 0xFFFF) & ~0xFFFF;
 rom_size = round_up_pow2(real_rom_size); //fp->size);

 wsCartROM = (uint8 *)calloc(1, rom_size);


 // This real_rom_size vs rom_size funny business is intended primarily for handling
 // WSR files.
 if(real_rom_size < rom_size)
  memset(wsCartROM, 0xFF, rom_size - real_rom_size);

 memcpy(wsCartROM + (rom_size - real_rom_size), fp->data, fp->size);

 MDFN_printf(_("ROM:       %dKiB\n"), real_rom_size / 1024);
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

 if(!IsWSR)
 {
  if(header[6] & 0x1)
   MDFNGameInfo->rotated = MDFN_ROTATE90;
 }

 MDFNMP_Init(16384, (1 << 20) / 1024);

 #ifdef WANT_DEBUGGER
 WSwanDBG_Init();
 #endif

 v30mz_init(WSwan_readmem20, WSwan_writemem20, WSwan_readport, WSwan_writeport);
 WSwan_MemoryInit(MDFN_GetSettingB("wswan.language"), wsc, SRAMSize, IsWSR); // EEPROM and SRAM are loaded in this func.
 WSwan_GfxInit();
 MDFNGameInfo->fps = (uint32)((uint64)3072000 * 65536 * 256 / (159*256));
 MDFNGameInfo->GameSetMD5Valid = FALSE;

 WSwan_SoundInit();

 wsMakeTiles();

 Reset();

 return(1);
}

static void CloseGame(void)
{
 WSwan_MemoryKill(); // saves sram/eeprom

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
  case MDFN_MSC_POWER:
  case MDFN_MSC_RESET: Reset();
                        break;
 }
}

static const MDFNSetting_EnumList SexList[] =
{
 { "m", WSWAN_SEX_MALE },
 { "male", WSWAN_SEX_MALE, gettext_noop("Male") },

 { "f", WSWAN_SEX_FEMALE },
 { "female", WSWAN_SEX_FEMALE, gettext_noop("Female") },

 { "3", 3 },

 { NULL, 0 },
};

static const MDFNSetting_EnumList BloodList[] =
{
 { "a", WSWAN_BLOOD_A, "A" },
 { "b", WSWAN_BLOOD_B, "B" },
 { "o", WSWAN_BLOOD_O, "O" },
 { "ab", WSWAN_BLOOD_AB, "AB" },

 { "5", 5 },

 { NULL, 0 },
};

static const MDFNSetting_EnumList LanguageList[] =
{
 { "japanese", 0, gettext_noop("Japanese") },
 { "0", 0 },

 { "english", 1, gettext_noop("English") },
 { "1", 1 },

 { NULL, 0 },
};

static const MDFNSetting WSwanSettings[] =
{
 { "wswan.rotateinput", MDFNSF_NOFLAGS, gettext_noop("Virtually rotate D-pads along with screen."), NULL, MDFNST_BOOL, "0" },
 { "wswan.language", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Language games should display text in."), gettext_noop("The only game this setting is known to affect is \"Digimon Tamers - Battle Spirit\"."), MDFNST_ENUM, "english", NULL, NULL, NULL, NULL, LanguageList },
 { "wswan.name", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Name"), NULL, MDFNST_STRING, "Mednafen" },
 { "wswan.byear", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Birth Year"), NULL, MDFNST_UINT, "1989", "0", "9999" },
 { "wswan.bmonth", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Birth Month"), NULL, MDFNST_UINT, "6", "1", "12" },
 { "wswan.bday", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Birth Day"), NULL, MDFNST_UINT, "23", "1", "31" },
 { "wswan.sex", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Sex"), NULL, MDFNST_ENUM, "F", NULL, NULL, NULL, NULL, SexList },
 { "wswan.blood", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Blood Type"), NULL, MDFNST_ENUM, "O", NULL, NULL, NULL, NULL, BloodList },
 { NULL }
};

static const InputDeviceInputInfoStruct IDII[] =
{
 { "up-x", "UP ↑, X Cursors", 0, IDIT_BUTTON, "down-x",				{ "right-x", "down-x", "left-x" } },
 { "right-x", "RIGHT →, X Cursors", 3, IDIT_BUTTON, "left-x",			{ "down-x", "left-x", "up-x" } },
 { "down-x", "DOWN ↓, X Cursors", 1, IDIT_BUTTON, "up-x", 			{ "left-x", "up-x", "right-x" } },
 { "left-x", "LEFT ←, X Cursors", 2, IDIT_BUTTON, "right-x",			{ "up-x", "right-x", "down-x" } },

 { "up-y", "UP ↑, Y Cur: MUST NOT = X CURSORS", 4, IDIT_BUTTON, "down-y",	{ "right-y", "down-y", "left-y" } },
 { "right-y", "RIGHT →, Y Cur: MUST NOT = X CURSORS", 7, IDIT_BUTTON, "left-y",	{ "down-y", "left-y", "up-y" } },
 { "down-y", "DOWN ↓, Y Cur: MUST NOT = X CURSORS", 5, IDIT_BUTTON, "up-y",	{ "left-y", "up-y", "right-y" } },
 { "left-y", "LEFT ←, Y Cur: MUST NOT = X CURSORS", 6, IDIT_BUTTON, "right-y",	{ "up-y", "right-y", "down-y" } },

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
 { 0, "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};


#ifdef WANT_DEBUGGER
static DebuggerInfoStruct DBGInfo =
{
 "shift_jis",
 7 + 1 + 8,	// Fixme, probably not right...  maximum number of prefixes + 1 for opcode + 4 for operand(go with 8 to be safe)
 1,             // Instruction alignment(bytes)
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
};
#endif

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".ws", gettext_noop("WonderSwan ROM Image") },
 { ".wsc", gettext_noop("WonderSwan Color ROM Image") },
 { ".wsr", gettext_noop("WonderSwan Music Rip") },
 { NULL, NULL }
};

}

using namespace MDFN_IEN_WSWAN;

MDFNGI EmulatedWSwan =
{
 "wswan",
 "WonderSwan",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 #ifdef WANT_DEBUGGER
 &DBGInfo,
 #else
 NULL,
 #endif
 &InputInfo,
 Load,
 TestMagic,
 NULL,
 NULL,
 CloseGame,
 WSwan_SetLayerEnableMask,
 "Background\0Foreground\0Sprites\0",
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetInput,
 DoSimpleCommand,
 WSwanSettings,
 MDFN_MASTERCLOCK_FIXED(3072000),
 0,
 FALSE, // Multires possible?

 224,   // lcm_width
 144,   // lcm_height
 NULL,  // Dummy

 224,	// Nominal width
 144,	// Nominal height

 224,	// Framebuffer width
 144,	// Framebuffer height

 2,     // Number of output sound channels
};

