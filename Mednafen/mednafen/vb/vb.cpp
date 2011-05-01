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

#include "vb.h"
#include "timer.h"
#include "vsu.h"
#include "vip.h"
#ifdef WANT_DEBUGGER
#include "debug.h"
#endif
#include "input.h"
#include "../general.h"
#include "../md5.h"
#include "../mempatcher.h"
#include <iconv.h>

//#define VB_SUPPORT_BIN_EXT		// Even with this enabled, any *.bin file loaded must be in the internal database to be recognized as a VB game.

namespace MDFN_IEN_VB
{

enum
{
 ANAGLYPH_PRESET_DISABLED = 0,
 ANAGLYPH_PRESET_RED_BLUE,
 ANAGLYPH_PRESET_RED_CYAN,
 ANAGLYPH_PRESET_RED_ELECTRICCYAN,
 ANAGLYPH_PRESET_RED_GREEN,
 ANAGLYPH_PRESET_GREEN_MAGENTA,
 ANAGLYPH_PRESET_YELLOW_BLUE,
};

static const uint32 AnaglyphPreset_Colors[][2] =
{
 { 0, 0 },
 { 0xFF0000, 0x0000FF },
 { 0xFF0000, 0x00B7EB },
 { 0xFF0000, 0x00FFFF },
 { 0xFF0000, 0x00FF00 },
 { 0x00FF00, 0xFF00FF },
 { 0xFFFF00, 0x0000FF },
};


int32 VB_InDebugPeek;

static uint32 VB3DMode;

static Blip_Buffer sbuf[2];

static uint8 *WRAM = NULL;

static uint8 *GPRAM = NULL;
static uint32 GPRAM_Mask;

static uint8 *GPROM = NULL;
static uint32 GPROM_Mask;

V810 *VB_V810 = NULL;

VSU *VB_VSU = NULL;
static uint32 VSU_CycleFix;

static uint8 WCR;

static int32 next_vip_ts, next_timer_ts, next_input_ts;


static uint32 IRQ_Asserted;

static INLINE void RecalcIntLevel(void)
{
 int ilevel = -1;

 for(int i = 4; i >= 0; i--)
 {
  if(IRQ_Asserted & (1 << i))
  {
   ilevel = i;
   break;
  }
 }

 VB_V810->SetInt(ilevel);
}

void VBIRQ_Assert(int source, bool assert)
{
 assert(source >= 0 && source <= 4);

 IRQ_Asserted &= ~(1 << source);

 if(assert)
  IRQ_Asserted |= 1 << source;
 
 RecalcIntLevel();
}



static uint8 HWCTRL_Read(v810_timestamp_t &timestamp, uint32 A)
{
 uint8 ret = 0;

 if(A & 0x3)
 { 
  puts("HWCtrl Bogus Read?");
  return(ret);
 }

 switch(A & 0xFF)
 {
  default: printf("Unknown HWCTRL Read: %08x\n", A);
	   break;

  case 0x18:
  case 0x1C:
  case 0x20: ret = TIMER_Read(timestamp, A);
	     break;

  case 0x24: ret = WCR | 0xFC;
	     break;

  case 0x10:
  case 0x14:
  case 0x28: ret = VBINPUT_Read(timestamp, A);
             break;

 }

 return(ret);
}

static void HWCTRL_Write(v810_timestamp_t &timestamp, uint32 A, uint8 V)
{
 if(A & 0x3)
 {
  puts("HWCtrl Bogus Write?");
  return;
 }

 switch(A & 0xFF)
 {
  default: printf("Unknown HWCTRL Write: %08x %02x\n", A, V);
           break;

  case 0x18:
  case 0x1C:
  case 0x20: TIMER_Write(timestamp, A, V);
	     break;

  case 0x24: WCR = V & 0x3;
	     break;

  case 0x10:
  case 0x14:
  case 0x28: VBINPUT_Write(timestamp, A, V);
	     break;
 }
}

uint8 MDFN_FASTCALL MemRead8(v810_timestamp_t &timestamp, uint32 A)
{
 uint8 ret = 0;
 A &= (1 << 27) - 1;

 //if((A >> 24) <= 2)
 // printf("Read8: %d %08x\n", timestamp, A);

 switch(A >> 24)
 {
  case 0: ret = VIP_Read8(timestamp, A);
	  break;

  case 1: break;

  case 2: ret = HWCTRL_Read(timestamp, A);
	  break;

  case 3: break;
  case 4: break;

  case 5: ret = WRAM[A & 0xFFFF];
	  break;

  case 6: if(GPRAM)
	   ret = GPRAM[A & GPRAM_Mask];
	  else
	   printf("GPRAM(Unmapped) Read: %08x\n", A);
	  break;

  case 7: ret = GPROM[A & GPROM_Mask];
	  break;
 }
 return(ret);
}

uint16 MDFN_FASTCALL MemRead16(v810_timestamp_t &timestamp, uint32 A)
{
 uint16 ret = 0;

 A &= (1 << 27) - 1;

 //if((A >> 24) <= 2)
 // printf("Read16: %d %08x\n", timestamp, A);


 switch(A >> 24)
 {
  case 0: ret = VIP_Read16(timestamp, A);
	  break;

  case 1: break;

  case 2: ret = HWCTRL_Read(timestamp, A);
	  break;

  case 3: break;

  case 4: break;

  case 5: ret = le16toh(*(uint16 *)&WRAM[A & 0xFFFF]);
	  break;

  case 6: if(GPRAM)
           ret = le16toh(*(uint16 *)&GPRAM[A & GPRAM_Mask]);
	  else printf("GPRAM(Unmapped) Read: %08x\n", A);
	  break;

  case 7: ret = le16toh(*(uint16 *)&GPROM[A & GPROM_Mask]);
	  break;
 }
 return(ret);
}

void MDFN_FASTCALL MemWrite8(v810_timestamp_t &timestamp, uint32 A, uint8 V)
{
 A &= (1 << 27) - 1;

 //if((A >> 24) <= 2)
 // printf("Write8: %d %08x %02x\n", timestamp, A, V);

 switch(A >> 24)
 {
  case 0: VIP_Write8(timestamp, A, V);
          break;

  case 1: VB_VSU->Write((timestamp + VSU_CycleFix) >> 2, A, V);
          break;

  case 2: HWCTRL_Write(timestamp, A, V);
          break;

  case 3: break;

  case 4: break;

  case 5: WRAM[A & 0xFFFF] = V;
          break;

  case 6: if(GPRAM)
           GPRAM[A & GPRAM_Mask] = V;
          break;

  case 7: // ROM, no writing allowed!
          break;
 }
}

void MDFN_FASTCALL MemWrite16(v810_timestamp_t &timestamp, uint32 A, uint16 V)
{
 A &= (1 << 27) - 1;

 //if((A >> 24) <= 2)
 // printf("Write16: %d %08x %04x\n", timestamp, A, V);

 switch(A >> 24)
 {
  case 0: VIP_Write16(timestamp, A, V);
          break;

  case 1: VB_VSU->Write((timestamp + VSU_CycleFix) >> 2, A, V);
          break;

  case 2: HWCTRL_Write(timestamp, A, V);
          break;

  case 3: break;

  case 4: break;

  case 5: *(uint16 *)&WRAM[A & 0xFFFF] = htole16(V);
          break;

  case 6: if(GPRAM)
           *(uint16 *)&GPRAM[A & GPRAM_Mask] = htole16(V);
          break;

  case 7: // ROM, no writing allowed!
          break;
 }
}

static void FixNonEvents(void)
{
 if(next_vip_ts & 0x40000000)
  next_vip_ts = VB_EVENT_NONONO;

 if(next_timer_ts & 0x40000000)
  next_timer_ts = VB_EVENT_NONONO;

 if(next_input_ts & 0x40000000)
  next_input_ts = VB_EVENT_NONONO;
}

static void EventReset(void)
{
 next_vip_ts = VB_EVENT_NONONO;
 next_timer_ts = VB_EVENT_NONONO;
 next_input_ts = VB_EVENT_NONONO;
}

static INLINE int32 CalcNextTS(void)
{
 int32 next_timestamp = next_vip_ts;

 if(next_timestamp > next_timer_ts)
  next_timestamp  = next_timer_ts;

 if(next_timestamp > next_input_ts)
  next_timestamp  = next_input_ts;

 return(next_timestamp);
}

static void RebaseTS(const v810_timestamp_t timestamp)
{
 //printf("Rebase: %08x %08x %08x\n", timestamp, next_vip_ts, next_timer_ts);

 assert(next_vip_ts > timestamp);
 assert(next_timer_ts > timestamp);
 assert(next_input_ts > timestamp);

 next_vip_ts -= timestamp;
 next_timer_ts -= timestamp;
 next_input_ts -= timestamp;
}

void VB_SetEvent(const int type, const v810_timestamp_t next_timestamp)
{
 //assert(next_timestamp > VB_V810->v810_timestamp);

 if(type == VB_EVENT_VIP)
  next_vip_ts = next_timestamp;
 else if(type == VB_EVENT_TIMER)
  next_timer_ts = next_timestamp;
 else if(type == VB_EVENT_INPUT)
  next_input_ts = next_timestamp;

 if(next_timestamp < VB_V810->GetEventNT())
  VB_V810->SetEventNT(next_timestamp);
}


static int32 MDFN_FASTCALL EventHandler(const v810_timestamp_t timestamp)
{
 if(timestamp >= next_vip_ts)
  next_vip_ts = VIP_Update(timestamp);

 if(timestamp >= next_timer_ts)
  next_timer_ts = TIMER_Update(timestamp);

 if(timestamp >= next_input_ts)
  next_input_ts = VBINPUT_Update(timestamp);

 return(CalcNextTS());
}

static void ForceEventUpdates(const v810_timestamp_t timestamp)
{
 next_vip_ts = VIP_Update(timestamp);
 next_timer_ts = TIMER_Update(timestamp);
 next_input_ts = VBINPUT_Update(timestamp);

 VB_V810->SetEventNT(CalcNextTS());
 //printf("FEU: %d %d %d\n", next_vip_ts, next_timer_ts, next_input_ts);
}

static void VB_Power(void)
{
 memset(WRAM, 0, 65536);

 VIP_Power();
 VB_VSU->Power();
 TIMER_Power();
 VBINPUT_Power();

 EventReset();
 IRQ_Asserted = 0;
 RecalcIntLevel();
 VB_V810->Reset();

 VSU_CycleFix = 0;
 WCR = 0;


 ForceEventUpdates(0);	//VB_V810->v810_timestamp);
}

static void SettingChanged(const char *name)
{
 if(!strcasecmp(name, "vb.3dmode"))
 {
  // FIXME, TODO (complicated)
  //VB3DMode = MDFN_GetSettingUI("vb.3dmode");
  //VIP_Set3DMode(VB3DMode);
 }
 else if(!strcasecmp(name, "vb.disable_parallax"))
 {
  VIP_SetParallaxDisable(MDFN_GetSettingB("vb.disable_parallax"));
 }
 else if(!strcasecmp(name, "vb.anaglyph.lcolor") || !strcasecmp(name, "vb.anaglyph.rcolor") ||
	!strcasecmp(name, "vb.anaglyph.preset") || !strcasecmp(name, "vb.default_color"))

 {
  uint32 lcolor = MDFN_GetSettingUI("vb.anaglyph.lcolor"), rcolor = MDFN_GetSettingUI("vb.anaglyph.rcolor");
  int preset = MDFN_GetSettingI("vb.anaglyph.preset");

  if(preset != ANAGLYPH_PRESET_DISABLED)
  {
   lcolor = AnaglyphPreset_Colors[preset][0];
   rcolor = AnaglyphPreset_Colors[preset][1];
  }
  VIP_SetAnaglyphColors(lcolor, rcolor);
  VIP_SetDefaultColor(MDFN_GetSettingUI("vb.default_color"));
 }
 else if(!strcasecmp(name, "vb.input.instant_read_hack"))
 {
  VBINPUT_SetInstantReadHack(MDFN_GetSettingB("vb.input.instant_read_hack"));
 }
 else if(!strcasecmp(name, "vb.instant_display_hack"))
  VIP_SetInstantDisplayHack(MDFN_GetSettingB("vb.instant_display_hack"));
 else if(!strcasecmp(name, "vb.allow_draw_skip"))
  VIP_SetAllowDrawSkip(MDFN_GetSettingB("vb.allow_draw_skip"));
 else
  abort();


}

struct VB_HeaderInfo
{
 char game_title[256];
 uint32 game_code;
 uint16 manf_code;
 uint8 version;
};

static void ReadHeader(MDFNFILE *fp, VB_HeaderInfo *hi)
{
 iconv_t sjis_ict = iconv_open("UTF-8", "shift_jis");

 if(sjis_ict != (iconv_t)-1)
 {
  char *in_ptr, *out_ptr;
  size_t ibl, obl;

  ibl = 20;
  obl = sizeof(hi->game_title) - 1;

  in_ptr = (char*)fp->data + (0xFFFFFDE0 & (fp->size - 1));
  out_ptr = hi->game_title;

  iconv(sjis_ict, (ICONV_CONST char **)&in_ptr, &ibl, &out_ptr, &obl);
  iconv_close(sjis_ict);

  *out_ptr = 0;

  MDFN_trim(hi->game_title);
 }
 else
  hi->game_title[0] = 0;

 hi->game_code = MDFN_de32lsb(fp->data + (0xFFFFFDFB & (fp->size - 1)));
 hi->manf_code = MDFN_de16lsb(fp->data + (0xFFFFFDF9 & (fp->size - 1)));
 hi->version = fp->data[0xFFFFFDFF & (fp->size - 1)];
}

#ifdef VB_SUPPORT_BIN_EXT
static bool FindGame(MDFNFILE *fp)
{
 static const char *GameNames[] =
 {
  "POLYGOBLOCK",	// 3D Tetris
  "BOUND HIGH",
  "GALACTIC PINBALL",
  "GOLF",
  "ｲﾝｽﾏｳｽ ﾉ ﾔｶﾀ",
  "JACK BROS",
  "MARIO CLASH",
  "Mario's Tennis",
  "NESTER'S FUNKY BOWLI",
  "Panic Bomber",
  "REDALARM",
  "VB SDｶﾞﾝﾀﾞﾑ",
  "SPACE INVADERS V.C.",
  "SPACE SQUASH",
  "TELEROBOXER",
  "T&EVIRTUALGOLF",
  "ﾄﾋﾞﾀﾞｾ!ﾊﾟﾆﾎﾞﾝ",
  "VERTICAL FORCE",
  "VIRTUAL BOWLING",
  "VB ﾜﾘｵﾗﾝﾄﾞ",
  "VIRTUAL FISHING JPN",
  "ﾊﾞ-ﾁｬﾙLAB",
  "VIRTUAL LEAGUE BB",
  "ﾊﾞｰﾁｬﾙﾌﾟﾛﾔｷｭｳ'95",
  "V･TETRIS",
  "WATERWORLD",
  NULL
 };
 VB_HeaderInfo hinfo;

 ReadHeader(fp, &hinfo);

 const char **gni = GameNames;
 while(*gni)
 {
  if(!strcmp(hinfo.game_title, *gni))
   return(true);
  gni++;
 }
 return(false);
}
#endif

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 if(!strcasecmp(fp->ext, "vb") || !strcasecmp(fp->ext, "vboy"))
  return(true);

 #ifdef VB_SUPPORT_BIN_EXT
 if(!strcasecmp(fp->ext, "bin") && FindGame(fp))
  return(true);
 #endif

 return(false);
}

static int Load(const char *name, MDFNFILE *fp)
{
 V810_Emu_Mode cpu_mode;
 md5_context md5;


 VB_InDebugPeek = 0;

 cpu_mode = (V810_Emu_Mode)MDFN_GetSettingI("vb.cpu_emulation");

 if(fp->size != round_up_pow2(fp->size))
 {
  puts("VB ROM image size is not a power of 2???");
  return(0);
 }

 if(fp->size < 256)
 {
  puts("VB ROM image size is too small??");
  return(0);
 }

 if(fp->size > (1 << 24))
 {
  puts("VB ROM image size is too large??");
  return(0);
 }

 md5.starts();
 md5.update(fp->data, fp->size);
 md5.finish(MDFNGameInfo->MD5);

 VB_HeaderInfo hinfo;

 ReadHeader(fp, &hinfo);

 MDFN_printf(_("Title:     %s\n"), hinfo.game_title);
 MDFN_printf(_("Game ID Code: %u\n"), hinfo.game_code);
 MDFN_printf(_("Manufacturer Code: %d\n"), hinfo.manf_code);
 MDFN_printf(_("Version:   %u\n"), hinfo.version);

 MDFN_printf(_("ROM:       %dKiB\n"), (int)(fp->size / 1024));
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());
 
 MDFN_printf("\n");

 MDFN_printf(_("V810 Emulation Mode: %s\n"), (cpu_mode == V810_EMU_MODE_ACCURATE) ? _("Accurate") : _("Fast"));

 VB_V810 = new V810();
 VB_V810->Init(cpu_mode, true);

 VB_V810->SetMemReadHandlers(MemRead8, MemRead16, NULL);
 VB_V810->SetMemWriteHandlers(MemWrite8, MemWrite16, NULL);

 VB_V810->SetIOReadHandlers(MemRead8, MemRead16, NULL);
 VB_V810->SetIOWriteHandlers(MemWrite8, MemWrite16, NULL);

 for(int i = 0; i < 256; i++)
 {
  VB_V810->SetMemReadBus32(i, false);
  VB_V810->SetMemWriteBus32(i, false);
 }

 std::vector<uint32> Map_Addresses;

 for(uint64 A = 0; A < 1ULL << 32; A += (1 << 27))
 {
  for(uint64 sub_A = 5 << 24; sub_A < (6 << 24); sub_A += 65536)
  {
   Map_Addresses.push_back(A + sub_A);
  }
 }

 WRAM = VB_V810->SetFastMap(&Map_Addresses[0], 65536, Map_Addresses.size(), "WRAM");
 Map_Addresses.clear();


 // Round up the ROM size to 65536(we mirror it a little later)
 GPROM_Mask = (fp->size < 65536) ? (65536 - 1) : (fp->size - 1);

 for(uint64 A = 0; A < 1ULL << 32; A += (1 << 27))
 {
  for(uint64 sub_A = 7 << 24; sub_A < (8 << 24); sub_A += GPROM_Mask + 1)
  {
   Map_Addresses.push_back(A + sub_A);
   //printf("%08x\n", (uint32)(A + sub_A));
  }
 }


 GPROM = VB_V810->SetFastMap(&Map_Addresses[0], GPROM_Mask + 1, Map_Addresses.size(), "Cart ROM");
 Map_Addresses.clear();

 // Mirror ROM images < 64KiB to 64KiB
 for(uint64 i = 0; i < 65536; i += fp->size)
 {
  memcpy(GPROM + i, fp->data, fp->size);
 }

 GPRAM_Mask = 0xFFFF;

 for(uint64 A = 0; A < 1ULL << 32; A += (1 << 27))
 {
  for(uint64 sub_A = 6 << 24; sub_A < (7 << 24); sub_A += GPRAM_Mask + 1)
  {
   //printf("GPRAM: %08x\n", A + sub_A);
   Map_Addresses.push_back(A + sub_A);
  }
 }


 GPRAM = VB_V810->SetFastMap(&Map_Addresses[0], GPRAM_Mask + 1, Map_Addresses.size(), "Cart RAM");
 Map_Addresses.clear();

 memset(GPRAM, 0, GPRAM_Mask + 1);

 {
  gzFile gp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb");

  if(gp)
  {
   if(gzread(gp, GPRAM, 65536) != 65536)
    puts("Error reading GPRAM");
   gzclose(gp);
  }
 }

 VIP_Init();
 VB_VSU = new VSU(&sbuf[0], &sbuf[1]);
 VBINPUT_Init();

 VB3DMode = MDFN_GetSettingUI("vb.3dmode");
 uint32 prescale = MDFN_GetSettingUI("vb.liprescale");
 uint32 sbs_separation = MDFN_GetSettingUI("vb.sidebyside.separation");

 VIP_Set3DMode(VB3DMode, MDFN_GetSettingUI("vb.3dreverse"), prescale, sbs_separation);


 //SettingChanged("vb.3dmode");
 SettingChanged("vb.disable_parallax");
 SettingChanged("vb.anaglyph.lcolor");
 SettingChanged("vb.anaglyph.rcolor");
 SettingChanged("vb.anaglyph.preset");
 SettingChanged("vb.default_color");

 SettingChanged("vb.instant_display_hack");
 SettingChanged("vb.allow_draw_skip");

 SettingChanged("vb.input.instant_read_hack");

 MDFNGameInfo->fps = (int64)20000000 * 65536 * 256 / (259 * 384 * 4);


 VB_Power();


 #ifdef WANT_DEBUGGER
 VBDBG_Init();
 #endif


 MDFNGameInfo->nominal_width = 384;
 MDFNGameInfo->nominal_height = 224;
 MDFNGameInfo->fb_width = 384;
 MDFNGameInfo->fb_height = 224;

 switch(VB3DMode)
 {
  default: break;

  case VB3DMODE_VLI:
        MDFNGameInfo->nominal_width = 768 * prescale;
        MDFNGameInfo->nominal_height = 224;
        MDFNGameInfo->fb_width = 768 * prescale;
        MDFNGameInfo->fb_height = 224;
        break;

  case VB3DMODE_HLI:
        MDFNGameInfo->nominal_width = 384;
        MDFNGameInfo->nominal_height = 448 * prescale;
        MDFNGameInfo->fb_width = 384;
        MDFNGameInfo->fb_height = 448 * prescale;
        break;

  case VB3DMODE_CSCOPE:
	MDFNGameInfo->nominal_width = 512;
	MDFNGameInfo->nominal_height = 384;
	MDFNGameInfo->fb_width = 512;
	MDFNGameInfo->fb_height = 384;
	break;

  case VB3DMODE_SIDEBYSIDE:
	MDFNGameInfo->nominal_width = 384 * 2 + sbs_separation;
  	MDFNGameInfo->nominal_height = 224;
  	MDFNGameInfo->fb_width = 384 * 2 + sbs_separation;
 	MDFNGameInfo->fb_height = 224;
	break;
 }
 MDFNGameInfo->lcm_width = MDFNGameInfo->fb_width;
 MDFNGameInfo->lcm_height = MDFNGameInfo->fb_height;


 MDFNMP_Init(32768, ((uint64)1 << 27) / 32768);
 MDFNMP_AddRAM(65536, 5 << 24, WRAM);
 if((GPRAM_Mask + 1) >= 32768)
  MDFNMP_AddRAM(GPRAM_Mask + 1, 6 << 24, GPRAM);
 return(1);
}

static void CloseGame(void)
{
 // Only save cart RAM if it has been modified.
 for(unsigned int i = 0; i < GPRAM_Mask + 1; i++)
 {
  if(GPRAM[i])
  {
   if(!MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, GPRAM, 65536))
   {

   }
   break;
  }
 }
 //VIP_Kill();
 
 if(VB_VSU)
 {
  delete VB_VSU;
  VB_VSU = NULL;
 }

 /*
 if(GPRAM)
 {
  MDFN_free(GPRAM);
  GPRAM = NULL;
 }

 if(GPROM)
 {
  MDFN_free(GPROM);
  GPROM = NULL;
 }
 */

 if(VB_V810)
 {
  VB_V810->Kill();
  delete VB_V810;
  VB_V810 = NULL;
 }
}

void VB_ExitLoop(void)
{
 VB_V810->Exit();
}

static void Emulate(EmulateSpecStruct *espec)
{
 v810_timestamp_t v810_timestamp;

 MDFNMP_ApplyPeriodicCheats();

 VBINPUT_Frame();

 if(espec->SoundFormatChanged)
 {
  for(int y = 0; y < 2; y++)
  {
   sbuf[y].set_sample_rate(espec->SoundRate ? espec->SoundRate : 44100, 50);
   sbuf[y].clock_rate((long)(VB_MASTER_CLOCK / 4));
   sbuf[y].bass_freq(20);
  }
 }

 VIP_StartFrame(espec);

 v810_timestamp = VB_V810->Run(EventHandler);

 FixNonEvents();
 ForceEventUpdates(v810_timestamp);

 VB_VSU->EndFrame((v810_timestamp + VSU_CycleFix) >> 2);

 if(espec->SoundBuf)
 {
  for(int y = 0; y < 2; y++)
  {
   sbuf[y].end_frame((v810_timestamp + VSU_CycleFix) >> 2);
   espec->SoundBufSize = sbuf[y].read_samples(espec->SoundBuf + y, espec->SoundBufMaxSize, 1);
  }
 }

 VSU_CycleFix = (v810_timestamp + VSU_CycleFix) & 3;

 espec->MasterCycles = v810_timestamp;

 TIMER_ResetTS();
 VBINPUT_ResetTS();
 VIP_ResetTS();

 RebaseTS(v810_timestamp);

 VB_V810->ResetTS();


#if 0
 if(espec->SoundRate)
 {
  unsigned long long crf = (unsigned long long)sbuf[0].clock_rate_factor(sbuf[0].clock_rate());
  double real_rate = (double)crf * sbuf[0].clock_rate() / (1ULL << BLIP_BUFFER_ACCURACY);

  printf("%f\n", real_rate);
 }
#endif
}

}

using namespace MDFN_IEN_VB;

#ifdef WANT_DEBUGGER
static DebuggerInfoStruct DBGInfo =
{
 "shift_jis",
 4,
 2,             // Instruction alignment(bytes)
 32,
 32,
 0x00000000,
 ~0,

 VBDBG_MemPeek,
 VBDBG_Disassemble,
 NULL,
 NULL,	//ForceIRQ,
 NULL,
 VBDBG_FlushBreakPoints,
 VBDBG_AddBreakPoint,
 VBDBG_SetCPUCallback,
 VBDBG_SetBPCallback,
 VBDBG_GetBranchTrace,
 NULL, 	//KING_SetGraphicsDecode,
 VBDBG_SetLogFunc,
};
#endif


static int StateAction(StateMem *sm, int load, int data_only)
{
 int ret = 1;
 SFORMAT StateRegs[] =
 {
  SFARRAY(WRAM, 65536),
  SFARRAY(GPRAM, GPRAM_Mask ? (GPRAM_Mask + 1) : 0),
  SFVAR(WCR),
  SFVAR(IRQ_Asserted),
  SFVAR(VSU_CycleFix),


  // TODO: Remove these(and recalc on state load)
  SFVAR(next_vip_ts), 
  SFVAR(next_timer_ts),
  SFVAR(next_input_ts),

  SFEND
 };

 ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");

 ret &= VB_V810->StateAction(sm, load, data_only);

 ret &= VB_VSU->StateAction(sm, load, data_only);
 ret &= TIMER_StateAction(sm, load, data_only);
 ret &= VBINPUT_StateAction(sm, load, data_only);
 ret &= VIP_StateAction(sm, load, data_only);

 if(load)
 {

 }
 return(ret);
}

static bool ToggleLayer(int which)
{
 return(1);
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFN_MSC_POWER:
  case MDFN_MSC_RESET: VB_Power(); break;
 }
}

static const MDFNSetting_EnumList V810Mode_List[] =
{
 { "fast", (int)V810_EMU_MODE_FAST, gettext_noop("Fast Mode"), gettext_noop("Fast mode trades timing accuracy, cache emulation, and executing from hardware registers and RAM not intended for code use for performance.")},
 { "accurate", (int)V810_EMU_MODE_ACCURATE, gettext_noop("Accurate Mode"), gettext_noop("Increased timing accuracy, though not perfect, along with cache emulation, at the cost of decreased performance.  Additionally, even the pipeline isn't correctly and fully emulated in this mode.") },
 { NULL, 0 },
};

static const MDFNSetting_EnumList VB3DMode_List[] =
{
 { "anaglyph", VB3DMODE_ANAGLYPH, gettext_noop("Anaglyph"), gettext_noop("Used in conjuction with classic dual-lens-color glasses.") },
 { "cscope",  VB3DMODE_CSCOPE, gettext_noop("CyberScope"), gettext_noop("Intended for use with the CyberScope 3D device.") },
 { "sidebyside", VB3DMODE_SIDEBYSIDE, gettext_noop("Side-by-Side"), gettext_noop("The left-eye image is displayed on the left, and the right-eye image is displayed on the right.") },
// { "overunder", VB3DMODE_OVERUNDER },
 { "vli", VB3DMODE_VLI, gettext_noop("Vertical Line Interlaced"), gettext_noop("Vertical lines alternate between left view and right view.") },
 { "hli", VB3DMODE_HLI, gettext_noop("Horizontal Line Interlaced"), gettext_noop("Horizontal lines alternate between left view and right view.") },
 { NULL, 0 },
};

static const MDFNSetting_EnumList AnaglyphPreset_List[] =
{
 { "disabled", ANAGLYPH_PRESET_DISABLED, gettext_noop("Disabled"), gettext_noop("Forces usage of custom anaglyph colors.") },
 { "0", ANAGLYPH_PRESET_DISABLED },

 { "red_blue", ANAGLYPH_PRESET_RED_BLUE, gettext_noop("Red/Blue"), gettext_noop("Classic red/blue anaglyph.") },
 { "red_cyan", ANAGLYPH_PRESET_RED_CYAN, gettext_noop("Red/Cyan"), gettext_noop("Improved quality red/cyan anaglyph.") },
 { "red_electriccyan", ANAGLYPH_PRESET_RED_ELECTRICCYAN, gettext_noop("Red/Electric Cyan"), gettext_noop("Alternate version of red/cyan") },
 { "red_green", ANAGLYPH_PRESET_RED_GREEN, gettext_noop("Red/Green") },
 { "green_magenta", ANAGLYPH_PRESET_GREEN_MAGENTA, gettext_noop("Green/Magenta") },
 { "yellow_blue", ANAGLYPH_PRESET_YELLOW_BLUE, gettext_noop("Yellow/Blue") },

 { NULL, 0 },
};

static MDFNSetting VBSettings[] =
{
 { "vb.cpu_emulation", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("CPU emulation mode."), NULL, MDFNST_ENUM, "fast", NULL, NULL, NULL, NULL, V810Mode_List },
 { "vb.input.instant_read_hack", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input latency reduction hack."), gettext_noop("Reduces latency in some games by 20ms by returning the current pad state, rather than latched state, on serial port data reads.  This hack may cause some homebrew software to malfunction, but it should be relatively safe for commercial official games."), MDFNST_BOOL, "1", NULL, NULL, NULL, SettingChanged },
 
 { "vb.instant_display_hack", MDFNSF_NOFLAGS, gettext_noop("Display latency reduction hack."), gettext_noop("Reduces latency in games by displaying the framebuffer 20ms earlier.  This hack has some potential of causing graphical glitches, so it is disabled by default."), MDFNST_BOOL, "0", NULL, NULL, NULL, SettingChanged },
 { "vb.allow_draw_skip", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Allow draw skipping."), gettext_noop("If vb.instant_display_hack is set to \"1\", and this setting is set to \"1\", then frame-skipping the drawing to the emulated framebuffer will be allowed.  THIS WILL CAUSE GRAPHICAL GLITCHES, AND THEORETICALLY(but unlikely) GAME CRASHES, ESPECIALLY WITH DIRECT FRAMEBUFFER DRAWING GAMES."), MDFNST_BOOL, "0", NULL, NULL, NULL, SettingChanged },

 // FIXME: We're going to have to set up some kind of video mode change notification for changing vb.3dmode while the game is running to work properly.
 { "vb.3dmode", MDFNSF_NOFLAGS, gettext_noop("3D mode."), NULL, MDFNST_ENUM, "anaglyph", NULL, NULL, NULL, /*SettingChanged*/NULL, VB3DMode_List },
 { "vb.liprescale", MDFNSF_NOFLAGS, gettext_noop("Line Interlaced prescale."), NULL, MDFNST_UINT, "2", "1", "10", NULL, NULL },

 { "vb.disable_parallax", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Disable parallax for BG and OBJ rendering."), NULL, MDFNST_BOOL, "0", NULL, NULL, NULL, SettingChanged },
 { "vb.default_color", MDFNSF_NOFLAGS, gettext_noop("Default maximum-brightness color to use in non-anaglyph 3D modes."), NULL, MDFNST_UINT, "0xF0F0F0", "0x000000", "0xFFFFFF", NULL, SettingChanged },

 { "vb.anaglyph.preset", MDFNSF_NOFLAGS, gettext_noop("Anaglyph preset colors."), NULL, MDFNST_ENUM, "red_blue", NULL, NULL, NULL, SettingChanged, AnaglyphPreset_List },
 { "vb.anaglyph.lcolor", MDFNSF_NOFLAGS, gettext_noop("Anaglyph maximum-brightness color for left view."), NULL, MDFNST_UINT, "0xffba00", "0x000000", "0xFFFFFF", NULL, SettingChanged },
 { "vb.anaglyph.rcolor", MDFNSF_NOFLAGS, gettext_noop("Anaglyph maximum-brightness color for right view."), NULL, MDFNST_UINT, "0x00baff", "0x000000", "0xFFFFFF", NULL, SettingChanged },

 { "vb.sidebyside.separation", MDFNSF_NOFLAGS, gettext_noop("Number of pixels to separate L/R views by."), gettext_noop("This setting refers to pixels before vb.xscale(fs) scaling is taken into consideration.  For example, a value of \"100\" here will result in a separation of 300 screen pixels if vb.xscale(fs) is set to \"3\"."), MDFNST_UINT, /*"96"*/"0", "0", "1024", NULL, NULL },

 { "vb.3dreverse", MDFNSF_NOFLAGS, gettext_noop("Reverse left/right 3D views."), NULL, MDFNST_BOOL, "0", NULL, NULL, NULL, SettingChanged },
 { NULL }
};


static const InputDeviceInputInfoStruct IDII[] =
{
 { "a", "A", 7, IDIT_BUTTON_CAN_RAPID,  NULL },
 { "b", "B", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "rt", "Right-Back", 13, IDIT_BUTTON, NULL },
 { "lt", "Left-Back", 12, IDIT_BUTTON, NULL },

 { "up-r", "UP ↑ (Right D-Pad)", 8, IDIT_BUTTON, "down-r" },
 { "right-r", "RIGHT → (Right D-Pad)", 11, IDIT_BUTTON, "left-r" },

 { "right-l", "RIGHT → (Left D-Pad)", 3, IDIT_BUTTON, "left-l" },
 { "left-l", "LEFT ← (Left D-Pad)", 2, IDIT_BUTTON, "right-l" },
 { "down-l", "DOWN ↓ (Left D-Pad)", 1, IDIT_BUTTON, "up-l" },
 { "up-l", "UP ↑ (Left D-Pad)", 0, IDIT_BUTTON, "down-l" },

 { "start", "Start", 5, IDIT_BUTTON, NULL },
 { "select", "Select", 4, IDIT_BUTTON, NULL },

 { "left-r", "LEFT ← (Right D-Pad)", 10, IDIT_BUTTON, "right-r" },
 { "down-r", "DOWN ↓ (Right D-Pad)", 9, IDIT_BUTTON, "up-r" },
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


static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".vb", gettext_noop("Nintendo Virtual Boy") },
 { ".vboy", gettext_noop("Nintendo Virtual Boy") },
 #ifdef VB_SUPPORT_BIN_EXT
 { ".bin", gettext_noop("Nintendo Virtual Boy (Deprecated)") },
 #endif
 { NULL, NULL }
};

MDFNGI EmulatedVB =
{
 "vb",
 "Virtual Boy",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 #ifdef WANT_DEBUGGER
 &DBGInfo,
 #else
 NULL,		// Debug info
 #endif
 &InputInfo,	//
 Load,
 TestMagic,
 NULL,
 NULL,
 CloseGame,
 ToggleLayer,
 NULL,		// Layer names, null-delimited
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 VBINPUT_SetInput,
 DoSimpleCommand,
 VBSettings,
 MDFN_MASTERCLOCK_FIXED(VB_MASTER_CLOCK),
 0,
 false, // Multires possible?

 0,   // lcm_width
 0,   // lcm_height
 NULL,  // Dummy

 384,	// Nominal width
 224,	// Nominal height

 384,	// Framebuffer width
 256,	// Framebuffer height

 2,     // Number of output sound channels
};
