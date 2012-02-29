#include <stdio.h>
#include <stdint.h>
#ifndef _MSC_VER
#include <stdbool.h>
#else
#define TRUE 1
#define FALSE 0
typedef unsigned char bool;
#endif
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#define LIBSNES_CORE 1

#if defined(_MSC_VER) && defined(LIBSNES_CORE)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#include "libsnes.h"

/* emulator-specific includes */

#include "../src-fceumm/fceu.h"
#include "../src-fceumm/myendian.h"
#include "../src-fceumm/input.h"
#include "../src-fceumm/state.h"
#include "../src-fceumm/ppu.h"
#include "../src-fceumm/cart.h"
#include "../src-fceumm/x6502.h"
#include "../src-fceumm/git.h"
#include "../src-fceumm/palette.h"
#include "../src-fceumm/sound.h"
#include "../src-fceumm/file.h"
#include "../src-fceumm/cheat.h"
#include "../src-fceumm/ines.h"
#include "../src-fceumm/unif.h"
#include "../src-fceumm/fds.h"

#include <string.h>
#include "memstream.h"

static snes_video_refresh_t video_cb = NULL;
static snes_audio_sample_t audio_cb = NULL;
static snes_input_poll_t poll_cb = NULL;
static snes_input_state_t input_cb = NULL;

/* emulator-specific variables */

static uint16_t palette[256];

static int32 *sound = 0;
static int32 ssize = 0;
static uint8 *gfx = 0;
static uint32 JSReturn[2];
static uint32 current_palette = 0;

/* extern forward decls.*/
extern FCEUGI *FCEUGameInfo;
extern uint8 *XBuf;
extern CartInfo iNESCart;
extern CartInfo UNIFCart;

/* emulator-specific callback functions */

const char * GetKeyboard(void) { return ""; }
void FCEUD_SetPalette(unsigned char index, unsigned char r, unsigned char g, unsigned char b)
{
   r >>= 3;
   g >>= 3;
   b >>= 3;
   palette[index] = (r << 10) | (g << 5) | (b << 0);
}

bool FCEUD_ShouldDrawInputAids() { return 1; }
void FCEUD_PrintError(const char *c) { }
void FCEUD_Message(const char *text) { }
void FCEUD_SoundToggle() { FCEUI_SetSoundVolume(256); }
void FCEUD_VideoChanged() {}

#define MAX_PAH 1024

/*palette for FCEU*/
#define MAXPAL 13

struct st_palettes {
	char name[32];
	char desc[32];
	unsigned int data[64];
};

struct st_palettes palettes[] = {
	{ "asqrealc", "AspiringSquire's Real palette",
		{ 0x6c6c6c, 0x00268e, 0x0000a8, 0x400094,
			0x700070, 0x780040, 0x700000, 0x621600,
			0x442400, 0x343400, 0x005000, 0x004444,
			0x004060, 0x000000, 0x101010, 0x101010,
			0xbababa, 0x205cdc, 0x3838ff, 0x8020f0,
			0xc000c0, 0xd01474, 0xd02020, 0xac4014,
			0x7c5400, 0x586400, 0x008800, 0x007468,
			0x00749c, 0x202020, 0x101010, 0x101010,
			0xffffff, 0x4ca0ff, 0x8888ff, 0xc06cff,
			0xff50ff, 0xff64b8, 0xff7878, 0xff9638,
			0xdbab00, 0xa2ca20, 0x4adc4a, 0x2ccca4,
			0x1cc2ea, 0x585858, 0x101010, 0x101010,
			0xffffff, 0xb0d4ff, 0xc4c4ff, 0xe8b8ff,
			0xffb0ff, 0xffb8e8, 0xffc4c4, 0xffd4a8,
			0xffe890, 0xf0f4a4, 0xc0ffc0, 0xacf4f0,
			0xa0e8ff, 0xc2c2c2, 0x202020, 0x101010 }
	},
	{ "loopy", "Loopy's palette",
		{ 0x757575, 0x271b8f, 0x0000ab, 0x47009f,
			0x8f0077, 0xab0013, 0xa70000, 0x7f0b00,
			0x432f00, 0x004700, 0x005100, 0x003f17,
			0x1b3f5f, 0x000000, 0x000000, 0x000000,
			0xbcbcbc, 0x0073ef, 0x233bef, 0x8300f3,
			0xbf00bf, 0xe7005b, 0xdb2b00, 0xcb4f0f,
			0x8b7300, 0x009700, 0x00ab00, 0x00933b,
			0x00838b, 0x000000, 0x000000, 0x000000,
			0xffffff, 0x3fbfff, 0x5f97ff, 0xa78bfd,
			0xf77bff, 0xff77b7, 0xff7763, 0xff9b3b,
			0xf3bf3f, 0x83d313, 0x4fdf4b, 0x58f898,
			0x00ebdb, 0x000000, 0x000000, 0x000000,
			0xffffff, 0xabe7ff, 0xc7d7ff, 0xd7cbff,
			0xffc7ff, 0xffc7db, 0xffbfb3, 0xffdbab,
			0xffe7a3, 0xe3ffa3, 0xabf3bf, 0xb3ffcf,
			0x9ffff3, 0x000000, 0x000000, 0x000000 }
	},
	{ "quor", "Quor's palette",
		{ 0x3f3f3f, 0x001f3f, 0x00003f, 0x1f003f,
			0x3f003f, 0x3f0020, 0x3f0000, 0x3f2000,
			0x3f3f00, 0x203f00, 0x003f00, 0x003f20,
			0x003f3f, 0x000000, 0x000000, 0x000000,
			0x7f7f7f, 0x405f7f, 0x40407f, 0x5f407f,
			0x7f407f, 0x7f4060, 0x7f4040, 0x7f6040,
			0x7f7f40, 0x607f40, 0x407f40, 0x407f60,
			0x407f7f, 0x000000, 0x000000, 0x000000,
			0xbfbfbf, 0x809fbf, 0x8080bf, 0x9f80bf,
			0xbf80bf, 0xbf80a0, 0xbf8080, 0xbfa080,
			0xbfbf80, 0xa0bf80, 0x80bf80, 0x80bfa0,
			0x80bfbf, 0x000000, 0x000000, 0x000000,
			0xffffff, 0xc0dfff, 0xc0c0ff, 0xdfc0ff,
			0xffc0ff, 0xffc0e0, 0xffc0c0, 0xffe0c0,
			0xffffc0, 0xe0ffc0, 0xc0ffc0, 0xc0ffe0,
			0xc0ffff, 0x000000, 0x000000, 0x000000 }
	},
	{ "chris", "Chris Covell's palette",
		{ 0x808080, 0x003DA6, 0x0012B0, 0x440096,
			0xA1005E, 0xC70028, 0xBA0600, 0x8C1700,
			0x5C2F00, 0x104500, 0x054A00, 0x00472E,
			0x004166, 0x000000, 0x050505, 0x050505,
			0xC7C7C7, 0x0077FF, 0x2155FF, 0x8237FA,
			0xEB2FB5, 0xFF2950, 0xFF2200, 0xD63200,
			0xC46200, 0x358000, 0x058F00, 0x008A55,
			0x0099CC, 0x212121, 0x090909, 0x090909,
			0xFFFFFF, 0x0FD7FF, 0x69A2FF, 0xD480FF,
			0xFF45F3, 0xFF618B, 0xFF8833, 0xFF9C12,
			0xFABC20, 0x9FE30E, 0x2BF035, 0x0CF0A4,
			0x05FBFF, 0x5E5E5E, 0x0D0D0D, 0x0D0D0D,
			0xFFFFFF, 0xA6FCFF, 0xB3ECFF, 0xDAABEB,
			0xFFA8F9, 0xFFABB3, 0xFFD2B0, 0xFFEFA6,
			0xFFF79C, 0xD7E895, 0xA6EDAF, 0xA2F2DA,
			0x99FFFC, 0xDDDDDD, 0x111111, 0x111111 }
	},
	{ "matt", "Matthew Conte's palette",
		{ 0x808080, 0x0000bb, 0x3700bf, 0x8400a6,
			0xbb006a, 0xb7001e, 0xb30000, 0x912600,
			0x7b2b00, 0x003e00, 0x00480d, 0x003c22,
			0x002f66, 0x000000, 0x050505, 0x050505,
			0xc8c8c8, 0x0059ff, 0x443cff, 0xb733cc,
			0xff33aa, 0xff375e, 0xff371a, 0xd54b00,
			0xc46200, 0x3c7b00, 0x1e8415, 0x009566,
			0x0084c4, 0x111111, 0x090909, 0x090909,
			0xffffff, 0x0095ff, 0x6f84ff, 0xd56fff,
			0xff77cc, 0xff6f99, 0xff7b59, 0xff915f,
			0xffa233, 0xa6bf00, 0x51d96a, 0x4dd5ae,
			0x00d9ff, 0x666666, 0x0d0d0d, 0x0d0d0d,
			0xffffff, 0x84bfff, 0xbbbbff, 0xd0bbff,
			0xffbfea, 0xffbfcc, 0xffc4b7, 0xffccae,
			0xffd9a2, 0xcce199, 0xaeeeb7, 0xaaf7ee,
			0xb3eeff, 0xdddddd, 0x111111, 0x111111 }
	},
	{ "pasofami", "PasoFami/99 palette",
		{ 0x7f7f7f, 0x0000ff, 0x0000bf, 0x472bbf,
			0x970087, 0xab0023, 0xab1300, 0x8b1700,
			0x533000, 0x007800, 0x006b00, 0x005b00,
			0x004358, 0x000000, 0x000000, 0x000000,
			0xbfbfbf, 0x0078f8, 0x0058f8, 0x6b47ff,
			0xdb00cd, 0xe7005b, 0xf83800, 0xe75f13,
			0xaf7f00, 0x00b800, 0x00ab00, 0x00ab47,
			0x008b8b, 0x000000, 0x000000, 0x000000,
			0xf8f8f8, 0x3fbfff, 0x6b88ff, 0x9878f8,
			0xf878f8, 0xf85898, 0xf87858, 0xffa347,
			0xf8b800, 0xb8f818, 0x5bdb57, 0x58f898,
			0x00ebdb, 0x787878, 0x000000, 0x000000,
			0xffffff, 0xa7e7ff, 0xb8b8f8, 0xd8b8f8,
			0xf8b8f8, 0xfba7c3, 0xf0d0b0, 0xffe3ab,
			0xfbdb7b, 0xd8f878, 0xb8f8b8, 0xb8f8d8,
			0x00ffff, 0xf8d8f8, 0x000000, 0x000000 }
	},
	{ "crashman", "CrashMan's palette",
		{ 0x585858, 0x001173, 0x000062, 0x472bbf,
			0x970087, 0x910009, 0x6f1100, 0x4c1008,
			0x371e00, 0x002f00, 0x005500, 0x004d15,
			0x002840, 0x000000, 0x000000, 0x000000,
			0xa0a0a0, 0x004499, 0x2c2cc8, 0x590daa,
			0xae006a, 0xb00040, 0xb83418, 0x983010,
			0x704000, 0x308000, 0x207808, 0x007b33,
			0x1c6888, 0x000000, 0x000000, 0x000000,
			0xf8f8f8, 0x267be1, 0x5870f0, 0x9878f8,
			0xff73c8, 0xf060a8, 0xd07b37, 0xe09040,
			0xf8b300, 0x8cbc00, 0x40a858, 0x58f898,
			0x00b7bf, 0x787878, 0x000000, 0x000000,
			0xffffff, 0xa7e7ff, 0xb8b8f8, 0xd8b8f8,
			0xe6a6ff, 0xf29dc4, 0xf0c0b0, 0xfce4b0,
			0xe0e01e, 0xd8f878, 0xc0e890, 0x95f7c8,
			0x98e0e8, 0xf8d8f8, 0x000000, 0x000000 }
	},
	{ "mess", "MESS palette",
		{ 0x747474, 0x24188c, 0x0000a8, 0x44009c,
			0x8c0074, 0xa80010, 0xa40000, 0x7c0800,
			0x402c00, 0x004400, 0x005000, 0x003c14,
			0x183c5c, 0x000000, 0x000000, 0x000000,
			0xbcbcbc, 0x0070ec, 0x2038ec, 0x8000f0,
			0xbc00bc, 0xe40058, 0xd82800, 0xc84c0c,
			0x887000, 0x009400, 0x00a800, 0x009038,
			0x008088, 0x000000, 0x000000, 0x000000,
			0xfcfcfc, 0x3cbcfc, 0x5c94fc, 0x4088fc,
			0xf478fc, 0xfc74b4, 0xfc7460, 0xfc9838,
			0xf0bc3c, 0x80d010, 0x4cdc48, 0x58f898,
			0x00e8d8, 0x000000, 0x000000, 0x000000,
			0xfcfcfc, 0xa8e4fc, 0xc4d4fc, 0xd4c8fc,
			0xfcc4fc, 0xfcc4d8, 0xfcbcb0, 0xfcd8a8,
			0xfce4a0, 0xe0fca0, 0xa8f0bc, 0xb0fccc,
			0x9cfcf0, 0x000000, 0x000000, 0x000000 }
	},
	{ "zaphod-cv", "Zaphod's VS Castlevania palette",
		{ 0x7f7f7f, 0xffa347, 0x0000bf, 0x472bbf,
			0x970087, 0xf85898, 0xab1300, 0xf8b8f8,
			0xbf0000, 0x007800, 0x006b00, 0x005b00,
			0xffffff, 0x9878f8, 0x000000, 0x000000,
			0xbfbfbf, 0x0078f8, 0xab1300, 0x6b47ff,
			0x00ae00, 0xe7005b, 0xf83800, 0x7777ff,
			0xaf7f00, 0x00b800, 0x00ab00, 0x00ab47,
			0x008b8b, 0x000000, 0x000000, 0x472bbf,
			0xf8f8f8, 0xffe3ab, 0xf87858, 0x9878f8,
			0x0078f8, 0xf85898, 0xbfbfbf, 0xffa347,
			0xc800c8, 0xb8f818, 0x7f7f7f, 0x007800,
			0x00ebdb, 0x000000, 0x000000, 0xffffff,
			0xffffff, 0xa7e7ff, 0x5bdb57, 0xe75f13,
			0x004358, 0x0000ff, 0xe7005b, 0x00b800,
			0xfbdb7b, 0xd8f878, 0x8b1700, 0xffe3ab,
			0x00ffff, 0xab0023, 0x000000, 0x000000 }
	},
	{ "zaphod-smb", "Zaphod's VS SMB palette",
		{ 0x626a00, 0x0000ff, 0x006a77, 0x472bbf,
			0x970087, 0xab0023, 0xab1300, 0xb74800,
			0xa2a2a2, 0x007800, 0x006b00, 0x005b00,
			0xffd599, 0xffff00, 0x009900, 0x000000,
			0xff66ff, 0x0078f8, 0x0058f8, 0x6b47ff,
			0x000000, 0xe7005b, 0xf83800, 0xe75f13,
			0xaf7f00, 0x00b800, 0x5173ff, 0x00ab47,
			0x008b8b, 0x000000, 0x91ff88, 0x000088,
			0xf8f8f8, 0x3fbfff, 0x6b0000, 0x4855f8,
			0xf878f8, 0xf85898, 0x595958, 0xff009d,
			0x002f2f, 0xb8f818, 0x5bdb57, 0x58f898,
			0x00ebdb, 0x787878, 0x000000, 0x000000,
			0xffffff, 0xa7e7ff, 0x590400, 0xbb0000,
			0xf8b8f8, 0xfba7c3, 0xffffff, 0x00e3e1,
			0xfbdb7b, 0xffae00, 0xb8f8b8, 0xb8f8d8,
			0x00ff00, 0xf8d8f8, 0xffaaaa, 0x004000 }
	},
	{ "vs-drmar", "VS Dr. Mario palette",
		{ 0x5f97ff, 0x000000, 0x000000, 0x47009f,
			0x00ab00, 0xffffff, 0xabe7ff, 0x000000,
			0x000000, 0x000000, 0x000000, 0x000000,
			0xe7005b, 0x000000, 0x000000, 0x000000,
			0x5f97ff, 0x000000, 0x000000, 0x000000,
			0x000000, 0x8b7300, 0xcb4f0f, 0x000000,
			0xbcbcbc, 0x000000, 0x000000, 0x000000,
			0x000000, 0x000000, 0x000000, 0x000000,
			0x00ebdb, 0x000000, 0x000000, 0x000000,
			0x000000, 0xff9b3b, 0x000000, 0x000000,
			0x83d313, 0x000000, 0x3fbfff, 0x000000,
			0x0073ef, 0x000000, 0x000000, 0x000000,
			0x00ebdb, 0x000000, 0x000000, 0x000000,
			0x000000, 0x000000, 0xf3bf3f, 0x000000,
			0x005100, 0x000000, 0xc7d7ff, 0xffdbab,
			0x000000, 0x000000, 0x000000, 0x000000 }
	},
	{ "vs-cv", "VS Castlevania palette",
		{ 0xaf7f00, 0xffa347, 0x008b8b, 0x472bbf,
			0x970087, 0xf85898, 0xab1300, 0xf8b8f8,
			0xf83800, 0x007800, 0x006b00, 0x005b00,
			0xffffff, 0x9878f8, 0x00ab00, 0x000000,
			0xbfbfbf, 0x0078f8, 0xab1300, 0x6b47ff,
			0x000000, 0xe7005b, 0xf83800, 0x6b88ff,
			0xaf7f00, 0x00b800, 0x6b88ff, 0x00ab47,
			0x008b8b, 0x000000, 0x000000, 0x472bbf,
			0xf8f8f8, 0xffe3ab, 0xf87858, 0x9878f8,
			0x0078f8, 0xf85898, 0xbfbfbf, 0xffa347,
			0x004358, 0xb8f818, 0x7f7f7f, 0x007800,
			0x00ebdb, 0x000000, 0x000000, 0xffffff,
			0xffffff, 0xa7e7ff, 0x5bdb57, 0x6b88ff,
			0x004358, 0x0000ff, 0xe7005b, 0x00b800,
			0xfbdb7b, 0xffa347, 0x8b1700, 0xffe3ab,
			0xb8f818, 0xab0023, 0x000000, 0x007800 }
	},
	{ "vs-smb", "VS SMB/VS Ice Climber palette",
		{ 0xaf7f00, 0x0000ff, 0x008b8b, 0x472bbf,
			0x970087, 0xab0023, 0x0000ff, 0xe75f13,
			0xbfbfbf, 0x007800, 0x5bdb57, 0x005b00,
			0xf0d0b0, 0xffe3ab, 0x00ab00, 0x000000,
			0xbfbfbf, 0x0078f8, 0x0058f8, 0x6b47ff,
			0x000000, 0xe7005b, 0xf83800, 0xf87858,
			0xaf7f00, 0x00b800, 0x6b88ff, 0x00ab47,
			0x008b8b, 0x000000, 0x000000, 0x3fbfff,
			0xf8f8f8, 0x006b00, 0x8b1700, 0x9878f8,
			0x6b47ff, 0xf85898, 0x7f7f7f, 0xe7005b,
			0x004358, 0xb8f818, 0x0078f8, 0x58f898,
			0x00ebdb, 0xfbdb7b, 0x000000, 0x000000,
			0xffffff, 0xa7e7ff, 0xb8b8f8, 0xf83800,
			0xf8b8f8, 0xfba7c3, 0xffffff, 0x00ffff,
			0xfbdb7b, 0xffa347, 0xb8f8b8, 0xb8f8d8,
			0xb8f818, 0xf8d8f8, 0x000000, 0x007800 }
	}
};

unsigned snes_library_revision_major(void)
{
   return 1;
}

unsigned snes_library_revision_minor(void)
{
   return 3;
}

const char *snes_library_id(void)
{
   return "FCEUNext";
}

void snes_set_video_refresh(snes_video_refresh_t cb)
{
   video_cb = cb;
}

void snes_set_audio_sample(snes_audio_sample_t cb)
{
   audio_cb = cb;
}

void snes_set_input_poll(snes_input_poll_t cb)
{
   poll_cb = cb;
}

void snes_set_input_state(snes_input_state_t cb)
{
   input_cb = cb;
}

void snes_set_controller_port_device(bool a, unsigned b)
{}

static char g_basename[1024];

void snes_set_cartridge_basename(const char * path_)
{
	char path[1024], *split;
	strncpy(path, path_, sizeof(path));

	split = strrchr(path_, '/');
	if (!split) split = strrchr(path_, '\\');
	if (split)
	{
		int len_filename, len_split;

		strncpy(g_basename, split + 1, sizeof(g_basename));
		len_filename = strlen(path_);
		len_split = strlen(split);
	}
	else
	{
		strncpy(g_basename, path_, sizeof(path_));
	}

	fprintf(stderr, "BASENAME: %s\n", g_basename);
}

/* SSNES extension.*/
static snes_environment_t environ_cb;
void snes_set_environment(snes_environment_t cb) { environ_cb = cb; }

void snes_init(void)
{
   if (environ_cb)
   {
      struct snes_geometry geom = { 256, 240, 256, 240 };
      environ_cb(SNES_ENVIRONMENT_SET_GEOMETRY, &geom);
   }
}

static void emulator_set_input(void)
{
    FCEUI_SetInput(0, SI_GAMEPAD, &JSReturn[0], 0);
    FCEUI_SetInput(1, SI_GAMEPAD, &JSReturn[1], 0);
}

static void emulator_set_custom_palette (void)
{
	if (current_palette == 0 )
	{
		FCEU_ResetPalette();	/* Do palette reset*/
	}
	else
	{
		/* Now setup this palette*/
		uint8 i,r,g,b;

		for ( i = 0; i < 64; i++ )
		{
			r = palettes[current_palette-1].data[i] >> 16;
			g = ( palettes[current_palette-1].data[i] & 0xff00 ) >> 8;
			b = ( palettes[current_palette-1].data[i] & 0xff );
			FCEUD_SetPalette( i, r, g, b);
			FCEUD_SetPalette( i+64, r, g, b);
			FCEUD_SetPalette( i+128, r, g, b);
			FCEUD_SetPalette( i+192, r, g, b);
		}
	}
}

static void fceu_init(void)
{
	emulator_set_input();
	emulator_set_custom_palette();

	FCEUD_SoundToggle();
}

void snes_term(void) {}

void snes_power(void)
{
   PowerNES();
}

void snes_reset(void)
{
   ResetNES();
}


typedef struct
{
   unsigned snes;
   unsigned nes;
} keymap;

static const keymap bindmap[] = {
   { SNES_DEVICE_ID_JOYPAD_A, JOY_A },
   { SNES_DEVICE_ID_JOYPAD_B, JOY_B },
   { SNES_DEVICE_ID_JOYPAD_SELECT, JOY_SELECT },
   { SNES_DEVICE_ID_JOYPAD_START, JOY_START },
   { SNES_DEVICE_ID_JOYPAD_UP, JOY_UP },
   { SNES_DEVICE_ID_JOYPAD_DOWN, JOY_DOWN },
   { SNES_DEVICE_ID_JOYPAD_LEFT, JOY_LEFT },
   { SNES_DEVICE_ID_JOYPAD_RIGHT, JOY_RIGHT },
};

static void update_input(void)
{
	unsigned i;
	unsigned char pad[2];

	pad[0] = 0;
	pad[1] = 0;

	poll_cb();

	for ( i = 0; i < 8; i++)
		pad[0] |= input_cb(SNES_PORT_1, SNES_DEVICE_JOYPAD, 0, bindmap[i].snes) ? bindmap[i].nes : 0;
	for ( i = 0; i < 8; i++)
		pad[1] |= input_cb(SNES_PORT_2, SNES_DEVICE_JOYPAD, 0, bindmap[i].snes) ? bindmap[i].nes : 0;
    JSReturn[0] = pad[0];
        JSReturn[1] = pad[1];
}

EXPORT void snes_run(void)
{
	unsigned i, y, x, ssize;
	uint8_t *gfx;
	static uint16_t video_out[1024 * 240];

	ssize = 0;
	FCEUI_Emulate(&gfx, &sound, &ssize);

	gfx = XBuf;
	for ( y = 0; y < 240; y++)
		for ( x = 0; x < 256; x++, gfx++)
			video_out[y * 1024 + x] = palette[*gfx];

	video_cb(video_out, 256, 240);
	update_input();

	for ( i = 0; i < ssize; i++)
		audio_cb(sound[i] & 0xffff, sound[i] & 0xffff);
}


static unsigned serialize_size = 0;

unsigned snes_serialize_size(void)
{
   if (serialize_size == 0)
   {
      /* Something arbitrarily big.*/
      uint8_t *buffer = (uint8_t*)malloc(1000000);
      memstream_set_buffer(buffer, 1000000);

      FCEUSS_Save();
      serialize_size = memstream_get_last_size();
      free(buffer);
   }

   return serialize_size;
}

bool snes_serialize(uint8_t *data, unsigned size)
{
   if (size != snes_serialize_size())
      return FALSE;

   memstream_set_buffer(data, size);
   FCEUSS_Save();
   return TRUE;
}

bool snes_unserialize(const uint8_t *data, unsigned size)
{
   if (size != snes_serialize_size())
      return FALSE;

   memstream_set_buffer((uint8_t*)data, size);
   FCEUSS_Load();
   return TRUE;
}

void snes_cheat_reset(void) {}

void snes_cheat_set(unsigned a, bool b, const char* c) { }

bool snes_load_cartridge_normal(const char* a, const uint8_t *rom_data, unsigned rom_size)
{
	const char *full_path;
	struct snes_system_timing timing;

   if (!environ_cb)
   {
      fprintf(stderr, "Environment callback not set. Cannot continue ...\n");
      return FALSE;
   }

   if (!environ_cb(SNES_ENVIRONMENT_GET_FULLPATH, &full_path) || !full_path)
   {
      fprintf(stderr, "GET_FULLPATH extension not supported. Cannot continue ...\n");
      return FALSE;
   }

   FCEUI_Initialize();

   FCEUI_SetSoundVolume(256);
   //FCEUI_Sound(32050);
   FCEUI_Sound(48000);

   FCEUGameInfo = FCEUI_LoadGame(full_path);

   fceu_init();

   //timing.sample_rate = 32050.0;
   timing.sample_rate = 48000;
   if (FSettings.PAL)
      timing.fps = 838977920.0/16777215.0;
   else
      timing.fps = 1008307711.0/16777215.0;

   environ_cb(SNES_ENVIRONMENT_SET_TIMING, &timing);

   return TRUE;
}

bool snes_load_cartridge_bsx_slotted(
  const char* a, const uint8_t* b, unsigned c,
  const char* d, const uint8_t* e, unsigned f
)
{ return FALSE; }

bool snes_load_cartridge_bsx(
  const char* a, const uint8_t * b, unsigned c,
  const char* d, const uint8_t * e, unsigned f
)
{ return FALSE; }

bool snes_load_cartridge_sufami_turbo(
  const char* a, const uint8_t* b, unsigned c,
  const char* d, const uint8_t* e, unsigned f,
  const char* g, const uint8_t* h, unsigned i
)
{ return FALSE; }

bool snes_load_cartridge_super_game_boy(
  const char* a, const uint8_t* b, unsigned c,
  const char* d, const uint8_t* e, unsigned f
)
{ return FALSE; }

void snes_unload_cartridge(void)
{
	FCEUI_CloseGame();
}

bool snes_get_region(void)
{
   return FSettings.PAL ? SNES_REGION_PAL : SNES_REGION_NTSC;
}

uint8_t *snes_get_memory_data(unsigned id)
{
   if (id != SNES_MEMORY_CARTRIDGE_RAM)
      return NULL;

   if (iNESCart.battery)
	   return iNESCart.SaveGame[0];
   if (UNIFCart.battery)
      return UNIFCart.SaveGame[0];

   return 0;
}

unsigned snes_get_memory_size(unsigned id)
{
   if (id != SNES_MEMORY_CARTRIDGE_RAM)
      return 0;

   if (iNESCart.battery)
      return iNESCart.SaveGameLen[0];
   if (UNIFCart.battery)
      return UNIFCart.SaveGameLen[0];

   return 0;
}
