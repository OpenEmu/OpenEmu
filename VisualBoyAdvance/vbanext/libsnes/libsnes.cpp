#include <stdio.h>
#include <string.h>

#ifndef _MSC_VER
#define TRUE 1
#define FALSE 0
#endif

#define LIBSNES_CORE 1

#if defined(_MSC_VER) && defined(LIBSNES_CORE)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#include "libsnes.hpp"

#include "../src/Util.h"
#include "../src/gba/GBA.h"
#include "../src/gba/Sound.h"
#include "../src/gba/RTC.h"
#include "../src/gba/Globals.h"

static snes_video_refresh_t video_cb;
static snes_audio_sample_t audio_cb;
static snes_input_poll_t poll_cb;
static snes_input_state_t input_cb;
extern uint64_t joy;

// Workaround for broken-by-design GBA save semantics.
uint16_t systemGbPalette[24];
uint8_t libsnes_save_buf[0x20000 + 0x2000];
static unsigned libsnes_save_size = sizeof(libsnes_save_buf);

EXPORT uint8_t *snes_get_memory_data(unsigned id)
{
   if (id != SNES_MEMORY_CARTRIDGE_RAM)
      return 0;

   return libsnes_save_buf;
}

EXPORT unsigned snes_get_memory_size(unsigned id)
{
   if (id != SNES_MEMORY_CARTRIDGE_RAM)
      return 0;

   return libsnes_save_size;
}

static bool scan_area(const uint8_t *data, unsigned size)
{
   for (unsigned i = 0; i < size; i++)
      if (data[i] != 0xff)
         return true;

   return false;
}

static void adjust_save_ram()
{
   if (scan_area(libsnes_save_buf, 512) &&
         !scan_area(libsnes_save_buf + 512, sizeof(libsnes_save_buf) - 512))
   {
      libsnes_save_size = 512;
      fprintf(stderr, "Detecting EEprom 8kbit\n");
   }
   else if (scan_area(libsnes_save_buf, 0x2000) && 
         !scan_area(libsnes_save_buf + 0x2000, sizeof(libsnes_save_buf) - 0x2000))
   {
      libsnes_save_size = 0x2000;
      fprintf(stderr, "Detecting EEprom 64kbit\n");
   }

   else if (scan_area(libsnes_save_buf, 0x10000) && 
         !scan_area(libsnes_save_buf + 0x10000, sizeof(libsnes_save_buf) - 0x10000))
   {
      libsnes_save_size = 0x10000;
      fprintf(stderr, "Detecting Flash 512kbit\n");
   }
   else if (scan_area(libsnes_save_buf, 0x20000) && 
         !scan_area(libsnes_save_buf + 0x20000, sizeof(libsnes_save_buf) - 0x20000))
   {
      libsnes_save_size = 0x20000;
      fprintf(stderr, "Detecting Flash 1Mbit\n");
   }
   else
      fprintf(stderr, "Did not detect any particular SRAM type.\n");

   if (libsnes_save_size == 512 || libsnes_save_size == 0x2000)
      eepromData = libsnes_save_buf;
   else if (libsnes_save_size == 0x10000 || libsnes_save_size == 0x20000)
      flashSaveMemory = libsnes_save_buf;
}


EXPORT unsigned snes_library_revision_major(void)
{
   return 1;
}

EXPORT unsigned snes_library_revision_minor(void)
{
   return 3;
}

EXPORT const char *snes_library_id(void)
{
   return "VBANext";
}

EXPORT void snes_set_video_refresh(snes_video_refresh_t cb)
{
   video_cb = cb;
}

EXPORT void snes_set_audio_sample(snes_audio_sample_t cb)
{
   audio_cb = cb;
}

EXPORT void snes_set_input_poll(snes_input_poll_t cb)
{
   poll_cb = cb;
}

EXPORT void snes_set_input_state(snes_input_state_t cb)
{
   input_cb = cb;
}

EXPORT void snes_set_controller_port_device(bool, unsigned)
{
}

EXPORT void snes_set_cartridge_basename(const char*)
{
}

// SSNES extension.
static snes_environment_t environ_cb;
EXPORT void snes_set_environment(snes_environment_t cb) { environ_cb = cb; }
static const char *full_path;

EXPORT void snes_init(void)
{
   memset(libsnes_save_buf, 0xff, sizeof(libsnes_save_buf));

   if (environ_cb)
   {
      snes_geometry geom = { 240, 160, 240, 160 };
      environ_cb(SNES_ENVIRONMENT_SET_GEOMETRY, &geom);
      environ_cb(SNES_ENVIRONMENT_GET_FULLPATH, &full_path);
      unsigned pitch = 512;
      environ_cb(SNES_ENVIRONMENT_SET_PITCH, &pitch);

      snes_system_timing timing;
      timing.fps =  16777216.0 / 280896.0;
      timing.sample_rate = 32000.0;

      environ_cb(SNES_ENVIRONMENT_SET_TIMING, &timing);
   }
}

static unsigned serialize_size = 0;

typedef struct  {
	char romtitle[256];
	char romid[5];
	int flashSize;
	int saveType;
	int rtcEnabled;
	int mirroringEnabled;
	int useBios;
} ini_t;

static const ini_t gbaover[256] = {
			//romtitle,							    	romid	flash	save	rtc	mirror	bios
			{"2 Games in 1 - Dragon Ball Z - The Legacy of Goku I & II (USA)",	"BLFE",	0,	1,	0,	0,	0},
			{"2 Games in 1 - Dragon Ball Z - Buu's Fury + Dragon Ball GT - Transformation (USA)", "BUFE", 0, 1, 0, 0, 0},
			{"Boktai - The Sun Is in Your Hand (Europe)(En,Fr,De,Es,It)",		"U3IP",	0,	0,	1,	0,	0},
			{"Boktai - The Sun Is in Your Hand (USA)",				"U3IE",	0,	0,	1,	0,	0},
			{"Boktai 2 - Solar Boy Django (USA)",					"U32E",	0,	0,	1,	0,	0},
			{"Boktai 2 - Solar Boy Django (Europe)(En,Fr,De,Es,It)",		"U32P",	0,	0,	1,	0,	0},
			{"Bokura no Taiyou - Taiyou Action RPG (Japan)",			"U3IJ",	0,	0,	1,	0,	0},
			{"Card e-Reader+ (Japan)",						"PSAJ",	131072,	0,	0,	0,	0},
			{"Classic NES Series - Bomberman (USA, Europe)",			"FBME",	0,	1,	0,	1,	0},
			{"Classic NES Series - Castlevania (USA, Europe)",			"FADE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Donkey Kong (USA, Europe)",			"FDKE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Dr. Mario (USA, Europe)",			"FDME",	0,	1,	0,	1,	0},
			{"Classic NES Series - Excitebike (USA, Europe)",			"FEBE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Legend of Zelda (USA, Europe)",			"FZLE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Ice Climber (USA, Europe)",			"FICE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Metroid (USA, Europe)",				"FMRE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Pac-Man (USA, Europe)",				"FP7E",	0,	1,	0,	1,	0},
			{"Classic NES Series - Super Mario Bros. (USA, Europe)",		"FSME",	0,	1,	0,	1,	0},
			{"Classic NES Series - Xevious (USA, Europe)",				"FXVE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Zelda II - The Adventure of Link (USA, Europe)",	"FLBE",	0,	1,	0,	1,	0},
			{"Digi Communication 2 - Datou! Black Gemagema Dan (Japan)",		"BDKJ",	0,	1,	0,	0,	0},
			{"e-Reader (USA)",							"PSAE",	131072,	0,	0,	0,	0},
			{"Dragon Ball GT - Transformation (USA)",				"BT4E",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - Buu's Fury (USA)",					"BG3E",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - Taiketsu (Europe)(En,Fr,De,Es,It)",			"BDBP",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - Taiketsu (USA)",					"BDBE",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy of Goku II International (Japan)",		"ALFJ",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy of Goku II (Europe)(En,Fr,De,Es,It)",	"ALFP", 0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy of Goku II (USA)",				"ALFE",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy Of Goku (Europe)(En,Fr,De,Es,It)",		"ALGP",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy of Goku (USA)",				"ALGE",	131072,	1,	0,	0,	0},
			{"F-Zero - Climax (Japan)",						"BFTJ",	131072,	0,	0,	0,	0},
			{"Famicom Mini Vol. 01 - Super Mario Bros. (Japan)",			"FMBJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 12 - Clu Clu Land (Japan)",				"FCLJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 13 - Balloon Fight (Japan)",			"FBFJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 14 - Wrecking Crew (Japan)",			"FWCJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 15 - Dr. Mario (Japan)",				"FDMJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 16 - Dig Dug (Japan)",				"FTBJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 17 - Takahashi Meijin no Boukenjima (Japan)",	"FTBJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 18 - Makaimura (Japan)",				"FMKJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 19 - Twin Bee (Japan)",				"FTWJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 20 - Ganbare Goemon! Karakuri Douchuu (Japan)",	"FGGJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 21 - Super Mario Bros. 2 (Japan)",			"FM2J",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 22 - Nazo no Murasame Jou (Japan)",			"FNMJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 23 - Metroid (Japan)",				"FMRJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 24 - Hikari Shinwa - Palthena no Kagami (Japan)",	"FPTJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 25 - The Legend of Zelda 2 - Link no Bouken (Japan)","FLBJ",0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 26 - Famicom Mukashi Banashi - Shin Onigashima - Zen Kou Hen (Japan)","FFMJ",0,1,0,	1,	0},
			{"Famicom Mini Vol. 27 - Famicom Tantei Club - Kieta Koukeisha - Zen Kou Hen (Japan)","FTKJ",0,1,0,	1,	0},
			{"Famicom Mini Vol. 28 - Famicom Tantei Club Part II - Ushiro ni Tatsu Shoujo - Zen Kou Hen (Japan)","FTUJ",0,1,0,1,0},
			{"Famicom Mini Vol. 29 - Akumajou Dracula (Japan)",			"FADJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 30 - SD Gundam World - Gachapon Senshi Scramble Wars (Japan)","FSDJ",0,1,	0,	1,	0},
			{"Game Boy Wars Advance 1+2 (Japan)",					"BGWJ",	131072,	0,	0,	0,	0},
			{"Golden Sun - The Lost Age (USA)",					"AGFE",	65536,	0,	0,	1,	0},
			{"Golden Sun (USA)",							"AGSE",	65536,	0,	0,	1,	0},
			{"Koro Koro Puzzle - Happy Panechu! (Japan)",				"KHPJ",	0,	4,	0,	0,	0},
			{"Mario vs. Donkey Kong (Europe)",					"BM5P",	0,	3,	0,	0,	0},
			{"Pocket Monsters - Emerald (Japan)",					"BPEJ",	131072,	0,	1,	0,	0},
			{"Pocket Monsters - Fire Red (Japan)",					"BPRJ",	131072,	0,	0,	0,	0},
			{"Pocket Monsters - Leaf Green (Japan)",				"BPGJ",	131072,	0,	0,	0,	0},
			{"Pocket Monsters - Ruby (Japan)",					"AXVJ",	131072,	0,	1,	0,	0},
			{"Pocket Monsters - Sapphire (Japan)",					"AXPJ",	131072,	0,	1,	0,	0},
			{"Pokemon Mystery Dungeon - Red Rescue Team (USA, Australia)",		"B24E",	131072,	0,	0,	0,	0},
			{"Pokemon - Blattgruene Edition (Germany)",				"BPGD",	131072,	0,	0,	0,	0},
			{"Pokemon - Edicion Rubi (Spain)",					"AXVS",	131072,	0,	1,	0,	0},
			{"Pokemon - Edicion Esmeralda (Spain)",					"BPES",	131072,	0,	1,	0,	0},
			{"Pokemon - Edicion Rojo Fuego (Spain)",				"BPRS",	131072,	1,	0,	0,	0},
			{"Pokemon - Edicion Verde Hoja (Spain)",				"BPGS",	131072,	1,	0,	0,	0},
			{"Pokemon - Eidicion Zafiro (Spain)",					"AXPS",	131072,	0,	1,	0,	0},
			{"Pokemon - Emerald Version (USA, Europe)",				"BPEE",	131072,	0,	1,	0,	0},
			{"Pokemon - Feuerrote Edition (Germany)",				"BPRD",	131072,	0,	0,	0,	0},
			{"Pokemon - Fire Red Version (USA, Europe)",				"BPRE",	131072,	0,	0,	0,	0},
			{"Pokemon - Leaf Green Version (USA, Europe)",				"BPGE",	131072,	0,	0,	0,	0},
			{"Pokemon - Rubin Edition (Germany)",					"AXVD",	131072,	0,	1,	0,	0},
			{"Pokemon - Ruby Version (USA, Europe)",				"AXVE",	131072,	0,	1,	0,	0},
			{"Pokemon - Sapphire Version (USA, Europe)",				"AXPE",	131072,	0,	1,	0,	0},
			{"Pokemon - Saphir Edition (Germany)",					"AXPD",	131072,	0,	1,	0,	0},
			{"Pokemon - Smaragd Edition (Germany)",					"BPED",	131072,	0,	1,	0,	0},
			{"Pokemon - Version Emeraude (France)",					"BPEF",	131072,	0,	1,	0,	0},
			{"Pokemon - Version Rouge Feu (France)",				"BPRF",	131072,	0,	0,	0,	0},
			{"Pokemon - Version Rubis (France)",					"AXVF",	131072,	0,	1,	0,	0},
			{"Pokemon - Version Saphir (France)",					"AXPF",	131072,	0,	1,	0,	0},
			{"Pokemon - Version Vert Feuille (France)",				"BPGF",	131072,	0,	0,	0,	0},
			{"Pokemon - Versione Rubino (Italy)",					"AXVI",	131072,	0,	1,	0,	0},
			{"Pokemon - Versione Rosso Fuoco (Italy)",				"BPRI",	131072,	0,	0,	0,	0},
			{"Pokemon - Versione Smeraldo (Italy)",					"BPEI",	131072,	0,	1,	0,	0},
			{"Pokemon - Versione Verde Foglia (Italy)",				"BPGI",	131072,	0,	0,	0,	0},
			{"Pokemon - Versione Zaffiro (Italy)",					"AXPI",	131072,	0,	1,	0,	0},
			{"Rockman EXE 4.5 - Real Operation (Japan)",				"BR4J",	0,	0,	1,	0,	0},
			{"Rocky (Europe)(En,Fr,De,Es,It)",					"AROP",	0,	1,	0,	0,	0},
			{"Sennen Kazoku (Japan)",						"BKAJ",	131072,	0,	1,	0,	0},
			{"Shin Bokura no Taiyou - Gyakushuu no Sabata (Japan)",			"U33J",	0,	1,	1,	0,	0},
			{"Super Mario Advance 4 (Japan)",					"AX4J",	131072,	0,	0,	0,	0},
			{"Super Mario Advance 4 - Super Mario Bros. 3 (Europe)(En,Fr,De,Es,It)","AX4P",	131072,	0,	0,	0,	0},
			{"Super Mario Advance 4 - Super Mario Bros 3 - Super Mario Advance 4 v1.1 (USA)","AX4E",131072,0,0,0,0},
			{"Top Gun - Combat Zones (USA)(En,Fr,De,Es,It)",			"A2YE",	0,	5,	0,	0,	0},
			{"Yoshi no Banyuuinryoku (Japan)",					"KYGJ",	0,	4,	0,	0,	0},
			{"Yoshi - Topsy-Turvy (USA)",						"KYGE",	0,	1,	0,	0,	0},
			{"Yu-Gi-Oh! GX - Duel Academy (USA)",					"BYGE",	0,	2,	0,	0,	1},
			{"Yu-Gi-Oh! - Ultimate Masters - 2006 (Europe)(En,Jp,Fr,De,Es,It)",	"BY6P",	0,	2,	0,	0,	0},
			{"Zoku Bokura no Taiyou - Taiyou Shounen Django (Japan)",		"U32J",	0,	0,	1,	0,	0}
};

void LoadImagePreferences()
{
	char buffer[5];
	buffer[0] = rom[0xac];
	buffer[1] = rom[0xad];
	buffer[2] = rom[0xae];
	buffer[3] = rom[0xaf];
	buffer[4] = 0;
	fprintf(stderr, "GameID in ROM is: %s\n", buffer);

	bool found = false;
	int found_no = 0;

	for(int i = 0; i < 256; i++)
	{
		if(!strcmp(gbaover[i].romid, buffer))
		{
			found = true;
			found_no = i;
         break;
		}
	}

	if(found)
	{
		fprintf(stderr, "found ROM in vba-over list\n");

		enableRtc = gbaover[found_no].rtcEnabled;

		if(gbaover[found_no].flashSize != 0)
			flashSize = gbaover[found_no].flashSize;
		else
			flashSize = 65536;

		cpuSaveType = gbaover[found_no].saveType;

		mirroringEnable = gbaover[found_no].mirroringEnabled;
	}

	fprintf(stderr, "RTC = %d\n", enableRtc);
	fprintf(stderr, "flashSize = %d\n", flashSize);
	fprintf(stderr, "cpuSaveType = %d\n", cpuSaveType);
	fprintf(stderr, "mirroringEnable = %d\n", mirroringEnable);
}

static void gba_init(void)
{
	//default values
	cpuSaveType = 0;
	flashSize = 0x10000;
	enableRtc = false;
	mirroringEnable = false;

	LoadImagePreferences();

	utilUpdateSystemColorMaps();

	if(flashSize == 0x10000 || flashSize == 0x20000)
		flashSetSize(flashSize);

	if(rtcEnable)
		rtcEnable(enableRtc);
	doMirroring(mirroringEnable);

	soundInit();
	soundSetSampleRate(32000);

	CPUInit(0, false);
	CPUReset();

	soundReset();
	soundResume();

	uint8_t *state_buf = new uint8_t[2000000];
	serialize_size = CPUWriteState_libgba(state_buf, 2000000);
	delete[] state_buf;
}

EXPORT void snes_term(void) {}

EXPORT void snes_power(void)
{
   CPUReset();
}

EXPORT void snes_reset(void)
{
   CPUReset();
}

void systemReadJoypadGB(int) {}

static void systemReadJoypadGBA(void)
{
   poll_cb();

   u32 J = 0;

   static const unsigned binds[] = {
      SNES_DEVICE_ID_JOYPAD_A,
      SNES_DEVICE_ID_JOYPAD_B,
      SNES_DEVICE_ID_JOYPAD_SELECT,
      SNES_DEVICE_ID_JOYPAD_START,
      SNES_DEVICE_ID_JOYPAD_RIGHT,
      SNES_DEVICE_ID_JOYPAD_LEFT,
      SNES_DEVICE_ID_JOYPAD_UP,
      SNES_DEVICE_ID_JOYPAD_DOWN,
      SNES_DEVICE_ID_JOYPAD_R,
      SNES_DEVICE_ID_JOYPAD_L
   };

   for (unsigned i = 0; i < 10; i++)
      J |= input_cb(SNES_PORT_1, SNES_DEVICE_JOYPAD, 0, binds[i]) << i;

   joy = J;
}

static bool can_dupe;
static bool screen_drawn;

EXPORT void snes_run(void)
{
   static bool first = true;
   if (first)
   {
      adjust_save_ram();
      if (environ_cb)
         environ_cb(SNES_ENVIRONMENT_GET_CAN_DUPE, &can_dupe);
      first = false;
   }

   screen_drawn = false;
   while (!screen_drawn)
   {
      CPULoop();
      systemReadJoypadGBA();
   }
}


EXPORT unsigned snes_serialize_size(void)
{
   return serialize_size;
}

EXPORT bool snes_serialize(uint8_t *data, unsigned size)
{
   return CPUWriteState_libgba(data, size);
}

EXPORT bool snes_unserialize(const uint8_t *data, unsigned size)
{
   return CPUReadState_libgba(data, size);
}

EXPORT void snes_cheat_reset(void)
{}

EXPORT void snes_cheat_set(unsigned, bool, const char*)
{}

EXPORT bool snes_load_cartridge_normal(const char*, const uint8_t *rom_data, unsigned rom_size)
{
   const char *tmppath = "VBA-tmp.gba";
   unsigned ret;

   if (full_path)
      ret = CPULoadRom(full_path);
   else
   {
      FILE *file = fopen(tmppath, "wb");
      if (!file)
         return false;

      fwrite(rom_data, 1, rom_size, file);
      fclose(file);
      ret = CPULoadRom(tmppath);
      remove(tmppath);
   }

   gba_init();

   return ret;
}

EXPORT bool snes_load_cartridge_bsx_slotted(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

EXPORT bool snes_load_cartridge_bsx(
  const char*, const uint8_t *, unsigned,
  const char*, const uint8_t *, unsigned
)
{ return false; }

EXPORT bool snes_load_cartridge_sufami_turbo(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

EXPORT bool snes_load_cartridge_super_game_boy(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

EXPORT void snes_unload_cartridge(void)
{}

EXPORT bool snes_get_region(void)
{
   return SNES_REGION_NTSC;
}

void systemOnSoundShutdown()
{}

void systemSoundNonblock(bool)
{}

void systemSoundSetThrottle(u16)
{}

bool systemSoundInitDriver(long)
{
   return true;
}

void systemSoundPause()
{}

void systemSoundReset()
{}

void systemSoundResume()
{}

#if 0
void systemOnWriteDataToSoundBuffer(int16_t *finalWave, int length)
{
   for (int i = 0; i < length; i += 2)
      audio_cb(finalWave[i + 0], finalWave[i + 1]);
}
#endif

static uint16_t pix_buf[160 * 256];

#if __SSE2__
#include <emmintrin.h>
void systemDrawScreen()
{
   screen_drawn = true;

   for (unsigned y = 0; y < 160; y++)
   {
      uint16_t *dst = pix_buf + y * 256;
      const uint32_t *src = (const uint32_t*)pix + 241 * (y + 1);

      for (unsigned x = 0; x < 240; x += 8)
      {
         __m128i input[2] = {
            _mm_loadu_si128((const __m128i*)(src + 0)),
            _mm_loadu_si128((const __m128i*)(src + 4)),
         };
         src += 8;

         __m128i output = _mm_packs_epi32(input[0], input[1]);

         _mm_store_si128((__m128i*)dst, output);
         dst += 8;
      }
   }

   video_cb(pix_buf, 240, 160);
}
#else
void systemDrawScreen()
{
   screen_drawn = true;
   for (unsigned y = 0; y < 160; y++)
   {
      uint16_t *dst = pix_buf + y * 256;
      const uint32_t *src = (const uint32_t*)pix + 241 * (y + 1); // Don't ask why ... :(
      for (unsigned x = 0; x < 240; x++)
         dst[x] = (uint16_t)(src[x] & 0x7fff);
   }

   video_cb(pix_buf, 240, 160);
}
#endif

// Stubs
u16 systemColorMap16[0x10000];
u32 systemColorMap32[0x10000];
int systemColorDepth = 32;
int systemDebug = 0;
int systemVerbose = 0;
int systemFrameSkip = 0;
int systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
int systemSpeed = 0;
int systemRedShift = 10;
int systemGreenShift = 5;
int systemBlueShift = 0;

void systemMessage(int, const char*, ...)
{}


bool systemSoundInit()
{
   return true;
}

bool systemCanChangeSoundQuality()
{
   return true;
}

void systemFrame()
{}

void systemGbPrint(unsigned char*, int, int, int, int)
{}

void systemGbBorderOn()
{}
