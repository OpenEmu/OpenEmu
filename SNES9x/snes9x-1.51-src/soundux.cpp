/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/




#ifdef __DJGPP__
#include <allegro.h>
#undef TRUE
#endif

#include <stdio.h>
#include "snes9x.h"
#include "apu.h"
#include "memmap.h"
#include "soundux.h"

// gaussian table by libopenspc and SNEeSe
static const int32 gauss[512] =
{
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
	0x001, 0x001, 0x001, 0x002, 0x002, 0x002, 0x002, 0x002,
	0x002, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004,
	0x004, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x005,
	0x006, 0x006, 0x006, 0x006, 0x007, 0x007, 0x007, 0x008,
	0x008, 0x008, 0x009, 0x009, 0x009, 0x00A, 0x00A, 0x00A,
	0x00B, 0x00B, 0x00B, 0x00C, 0x00C, 0x00D, 0x00D, 0x00E,
	0x00E, 0x00F, 0x00F, 0x00F, 0x010, 0x010, 0x011, 0x011,
	0x012, 0x013, 0x013, 0x014, 0x014, 0x015, 0x015, 0x016,
	0x017, 0x017, 0x018, 0x018, 0x019, 0x01A, 0x01B, 0x01B,
	0x01C, 0x01D, 0x01D, 0x01E, 0x01F, 0x020, 0x020, 0x021,
	0x022, 0x023, 0x024, 0x024, 0x025, 0x026, 0x027, 0x028,
	0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E, 0x02F, 0x030,
	0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038,
	0x03A, 0x03B, 0x03C, 0x03D, 0x03E, 0x040, 0x041, 0x042,
	0x043, 0x045, 0x046, 0x047, 0x049, 0x04A, 0x04C, 0x04D,
	0x04E, 0x050, 0x051, 0x053, 0x054, 0x056, 0x057, 0x059,
	0x05A, 0x05C, 0x05E, 0x05F, 0x061, 0x063, 0x064, 0x066,
	0x068, 0x06A, 0x06B, 0x06D, 0x06F, 0x071, 0x073, 0x075,
	0x076, 0x078, 0x07A, 0x07C, 0x07E, 0x080, 0x082, 0x084,
	0x086, 0x089, 0x08B, 0x08D, 0x08F, 0x091, 0x093, 0x096,
	0x098, 0x09A, 0x09C, 0x09F, 0x0A1, 0x0A3, 0x0A6, 0x0A8,
	0x0AB, 0x0AD, 0x0AF, 0x0B2, 0x0B4, 0x0B7, 0x0BA, 0x0BC,
	0x0BF, 0x0C1, 0x0C4, 0x0C7, 0x0C9, 0x0CC, 0x0CF, 0x0D2,
	0x0D4, 0x0D7, 0x0DA, 0x0DD, 0x0E0, 0x0E3, 0x0E6, 0x0E9,
	0x0EC, 0x0EF, 0x0F2, 0x0F5, 0x0F8, 0x0FB, 0x0FE, 0x101,
	0x104, 0x107, 0x10B, 0x10E, 0x111, 0x114, 0x118, 0x11B,
	0x11E, 0x122, 0x125, 0x129, 0x12C, 0x130, 0x133, 0x137,
	0x13A, 0x13E, 0x141, 0x145, 0x148, 0x14C, 0x150, 0x153,
	0x157, 0x15B, 0x15F, 0x162, 0x166, 0x16A, 0x16E, 0x172,
	0x176, 0x17A, 0x17D, 0x181, 0x185, 0x189, 0x18D, 0x191,
	0x195, 0x19A, 0x19E, 0x1A2, 0x1A6, 0x1AA, 0x1AE, 0x1B2,
	0x1B7, 0x1BB, 0x1BF, 0x1C3, 0x1C8, 0x1CC, 0x1D0, 0x1D5,
	0x1D9, 0x1DD, 0x1E2, 0x1E6, 0x1EB, 0x1EF, 0x1F3, 0x1F8,
	0x1FC, 0x201, 0x205, 0x20A, 0x20F, 0x213, 0x218, 0x21C,
	0x221, 0x226, 0x22A, 0x22F, 0x233, 0x238, 0x23D, 0x241,
	0x246, 0x24B, 0x250, 0x254, 0x259, 0x25E, 0x263, 0x267,
	0x26C, 0x271, 0x276, 0x27B, 0x280, 0x284, 0x289, 0x28E,
	0x293, 0x298, 0x29D, 0x2A2, 0x2A6, 0x2AB, 0x2B0, 0x2B5,
	0x2BA, 0x2BF, 0x2C4, 0x2C9, 0x2CE, 0x2D3, 0x2D8, 0x2DC,
	0x2E1, 0x2E6, 0x2EB, 0x2F0, 0x2F5, 0x2FA, 0x2FF, 0x304,
	0x309, 0x30E, 0x313, 0x318, 0x31D, 0x322, 0x326, 0x32B,
	0x330, 0x335, 0x33A, 0x33F, 0x344, 0x349, 0x34E, 0x353,
	0x357, 0x35C, 0x361, 0x366, 0x36B, 0x370, 0x374, 0x379,
	0x37E, 0x383, 0x388, 0x38C, 0x391, 0x396, 0x39B, 0x39F,
	0x3A4, 0x3A9, 0x3AD, 0x3B2, 0x3B7, 0x3BB, 0x3C0, 0x3C5,
	0x3C9, 0x3CE, 0x3D2, 0x3D7, 0x3DC, 0x3E0, 0x3E5, 0x3E9,
	0x3ED, 0x3F2, 0x3F6, 0x3FB, 0x3FF, 0x403, 0x408, 0x40C,
	0x410, 0x415, 0x419, 0x41D, 0x421, 0x425, 0x42A, 0x42E,
	0x432, 0x436, 0x43A, 0x43E, 0x442, 0x446, 0x44A, 0x44E,
	0x452, 0x455, 0x459, 0x45D, 0x461, 0x465, 0x468, 0x46C,
	0x470, 0x473, 0x477, 0x47A, 0x47E, 0x481, 0x485, 0x488,
	0x48C, 0x48F, 0x492, 0x496, 0x499, 0x49C, 0x49F, 0x4A2,
	0x4A6, 0x4A9, 0x4AC, 0x4AF, 0x4B2, 0x4B5, 0x4B7, 0x4BA,
	0x4BD, 0x4C0, 0x4C3, 0x4C5, 0x4C8, 0x4CB, 0x4CD, 0x4D0,
	0x4D2, 0x4D5, 0x4D7, 0x4D9, 0x4DC, 0x4DE, 0x4E0, 0x4E3,
	0x4E5, 0x4E7, 0x4E9, 0x4EB, 0x4ED, 0x4EF, 0x4F1, 0x4F3,
	0x4F5, 0x4F6, 0x4F8, 0x4FA, 0x4FB, 0x4FD, 0x4FF, 0x500,
	0x502, 0x503, 0x504, 0x506, 0x507, 0x508, 0x50A, 0x50B,
	0x50C, 0x50D, 0x50E, 0x50F, 0x510, 0x511, 0x511, 0x512,
	0x513, 0x514, 0x514, 0x515, 0x516, 0x516, 0x517, 0x517,
	0x517, 0x518, 0x518, 0x518, 0x518, 0x518, 0x519, 0x519
};

//static const int32	*G1 = &gauss[256], *G2 = &gauss[512],
//						*G3 = &gauss[255], *G4 = &gauss[-1];

#define	G1(n)	gauss[256 + (n)]
#define	G2(n)	gauss[512 + (n)]
#define	G3(n)	gauss[255 + (n)]
#define	G4(n)	gauss[ -1 + (n)]

// envelope/noise table by libopenspc and SNEeSe
int32 env_counter_table[32] =
{
	0x0000, 0x000F, 0x0014, 0x0018, 0x001E, 0x0028, 0x0030, 0x003C,
	0x0050, 0x0060, 0x0078, 0x00A0, 0x00C0, 0x00F0, 0x0140, 0x0180,
	0x01E0, 0x0280, 0x0300, 0x03C0, 0x0500, 0x0600, 0x0780, 0x0A00,
	0x0C00, 0x0F00, 0x1400, 0x1800, 0x1E00, 0x2800, 0x3C00, 0x7800
};

static int32		env_counter_max;
static const int32	env_counter_max_master = 0x7800;

static int	rand_seed = 1;

extern int32	Loop[16];
extern int32	Echo[24000];
extern int32	FilterTaps[8];
extern int32	MixBuffer[SOUND_BUFFER_SIZE];
extern int32	EchoBuffer[SOUND_BUFFER_SIZE];
extern int32	DummyEchoBuffer[SOUND_BUFFER_SIZE];
extern uint32	FIRIndex;

// For backward compatibility ------------------------------
static uint32 OldAttackRate[16] =
{
	 4100,  2600,  1500,  1000,   640,   380,   260,   160,
	   96,    64,    40,    24,    16,    10,     6,     1
};

static uint32 OldDecayRate[8] =
{
	 1200,   740,   440,   290,   180,   110,    74,    37
};

static uint32 OldSustainRate[32] =
{
	   ~0, 38000, 28000, 24000, 19000, 14000, 12000,  9400,
	 7100,  5900,  4700,  3500,  2900,  2400,  1800,  1500,
	 1200,   880,   740,   590,   440,   370,   290,   220,
	  180,   150,   110,    92,    74,    55,    37,    18
};

static int OldNoiseFreq[32] =
{
	    0,    16,    21,    25,    31,    42,    50,    63,
	   84,   100,   125,   167,   200,   250,   333,   400,
	  500,   667,   800,  1000,  1300,  1600,  2000,  2700,
	 3200,  4000,  5300,  6400,  8000, 10700, 16000, 32000
};
// ---------------------------------------------------------

#define	FIXED_POINT 		0x10000UL
#define	FIXED_POINT_SHIFT	16

#undef	ABS
#define	ABS(a)	((a) < 0 ? -(a) : (a))

#define CLIP16(v) \
	if ((v) < -32768) \
		(v) = -32768; \
	else \
	if ((v) > 32767) \
		(v) = 32767

#define CLIP8(v) \
	if ((v) < -128) \
		(v) = -128; \
	else \
	if ((v) > 127) \
		(v) = 127

void S9xAPUSetEndOfSample (int i, Channel *);
void S9xAPUSetEndX (int);
void S9xSetEnvRate (Channel *, int32, int32);
void MixStereo (int);
void MixMono (int);

static void S9xSetSoundFrequency (int, int);
static void S9xConvertSoundOldValues ();
static void DecodeBlock (Channel *);
static void AltDecodeBlock (Channel *);
static void AltDecodeBlock2 (Channel *);
STATIC inline uint8 *S9xGetSampleAddress (int);

EXTERN_C void DecodeBlockAsm (int8 *, int16 *, int32 *, int32 *);
EXTERN_C void DecodeBlockAsm2 (int8 *, int16 *, int32 *, int32 *);

static bool8 DoFakeMute = FALSE;

STATIC inline uint8 *S9xGetSampleAddress (int sample_number)
{
	uint32 addr = (((APU.DSP[APU_DIR] << 8) + (sample_number << 2)) & 0xFFFF);
	return (IAPU.RAM + addr);
}

void S9xAPUSetEndOfSample (int i, Channel *ch)
{
	ch->state = SOUND_SILENT;
	ch->mode = MODE_NONE;
	ch->out_sample = 0;
	ch->xenvx = 0;
  if(!DoFakeMute) {
	APU.DSP[APU_ENDX] |=   1 << i;
	APU.DSP[APU_KON]  &= ~(1 << i);
	APU.DSP[APU_KOFF] &= ~(1 << i);
	APU.KeyedChannels &= ~(1 << i);
  }
}

#ifdef __DJGPP
END_OF_FUNCTION (S9xAPUSetEndOfSample)
#endif

void S9xAPUSetEndX (int i)
{
  if(!DoFakeMute) {
	APU.DSP[APU_ENDX] |= 1 << i;
  }
}

#ifdef __DJGPP
END_OF_FUNCTION (S9xAPUSetEndX)
#endif

void S9xSetEnvRate (Channel *ch, int32 rate_count, int32 xtarget)
{
	ch->xenvx_target = xtarget;
	ch->xenv_rate = rate_count;
}

#ifdef __DJGPP
END_OF_FUNCTION(S9xSetEnvRate);
#endif

void S9xSetEnvelopeRate (int channel, int32 rate_count, int32 xtarget)
{
	S9xSetEnvRate (&SoundData.channels[channel], rate_count, xtarget);
}

#ifdef __DJGPP
END_OF_FUNCTION(S9xSetEnvelopeRate);
#endif

void S9xSetSoundVolume (int channel, short volume_left, short volume_right)
{
	Channel *ch = &SoundData.channels[channel];

	if (so.stereo_switch + 1)
	{
		volume_left  = ((so.stereo_switch & (  1 << channel)) ? volume_left  : 0);
		volume_right = ((so.stereo_switch & (256 << channel)) ? volume_right : 0);
	}

	if (!so.stereo)
		volume_left = (ABS(volume_right) + ABS(volume_left)) >> 1;

	ch->volume_left  = volume_left;
	ch->volume_right = volume_right;
}

void S9xSetMasterVolume (short volume_left, short volume_right)
{
	if (Settings.DisableMasterVolume)
	{
		SoundData.master_volume_left  = 127;
		SoundData.master_volume_right = 127;
		SoundData.master_volume[0] = SoundData.master_volume[1] = 127;
	}
	else
	{
		if (!so.stereo)
			volume_left = (ABS(volume_right) + ABS(volume_left)) >> 1;

		SoundData.master_volume_left  = volume_left;
		SoundData.master_volume_right = volume_right;
		SoundData.master_volume[Settings.ReverseStereo]     = volume_left;
		SoundData.master_volume[1 ^ Settings.ReverseStereo] = volume_right;
	}
}

void S9xSetEchoVolume (short volume_left, short volume_right)
{
	if (!so.stereo)
		volume_left = (ABS(volume_right) + ABS(volume_left)) >> 1;

	SoundData.echo_volume_left  = volume_left;
	SoundData.echo_volume_right = volume_right;
	SoundData.echo_volume[Settings.ReverseStereo]     = volume_left;
	SoundData.echo_volume[1 ^ Settings.ReverseStereo] = volume_right;
}

void S9xSetEchoEnable (uint8 byte)
{
#if 0
	SoundData.echo_channel_enable = byte;
	if (!SoundData.echo_write_enabled || Settings.DisableSoundEcho)
		byte = 0;
	if (byte && !SoundData.echo_enable)
	{
		memset (Loop, 0, sizeof (Loop));
		memset (Echo, 0, sizeof (Echo));
	}
#endif
	SoundData.echo_enable = byte;
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		if (byte & (1 << i))
			SoundData.channels[i].echo_buf_ptr = EchoBuffer;
		else
			SoundData.channels[i].echo_buf_ptr = DummyEchoBuffer;
	}
}

void S9xSetEchoFeedback (int feedback)
{
	SoundData.echo_feedback = feedback;
}

void S9xSetEchoDelay (unsigned int delay)
{
	SoundData.echo_buffer_size = (delay << 10) * so.playback_rate / 32000;
	if (!so.stereo)
		SoundData.echo_buffer_size >>= 1;
	if (SoundData.echo_buffer_size)
		SoundData.echo_ptr %= SoundData.echo_buffer_size;
	else
		SoundData.echo_ptr = 0;
}

void S9xSetEchoWriteEnable (uint8 byte)
{
	SoundData.echo_write_enabled = byte;
	//printf("Echo write enable: %d\n", byte);
}

void S9xSetFrequencyModulationEnable (uint8 byte)
{
	SoundData.pitch_mod = byte & ~1;
}

void S9xSetSoundKeyOff (int channel)
{
	Channel *ch = &SoundData.channels[channel];

	if (ch->state != SOUND_SILENT)
	{
		ch->state = SOUND_RELEASE;
		ch->mode = MODE_RELEASE;
		S9xSetEnvRate (ch, env_counter_max, 0);
	}
}

void S9xPrepareSoundForSnapshotSave (bool8 restore)
{
	static uint32 temp_hertz[NUM_CHANNELS];

	int i, j;

	if (!restore)
	{
		for (i = 0; i < NUM_CHANNELS; i++)
		{
			Channel *ch = &SoundData.channels[i];

			ch->count = 0;
			ch->envx = ch->xenvx >> 4;
			ch->envx_target = ch->xenvx_target >> 4;
			ch->direction = 0;
			ch-> left_vol_level = (ch->xenvx * ch->volume_left ) >> 11;
			ch->right_vol_level = (ch->xenvx * ch->volume_right) >> 11;
			ch->release_rate = 8;
			ch->sustain_level = ch->xsustain_level >> 8;

			if (env_counter_max < ch->xenv_count)
				ch->env_error = 0;
			else
				ch->env_error = (uint32)
					((double) FIXED_POINT / env_counter_max * (env_counter_max - ch->xenv_count));

			if (ch->xenv_rate < 0)
				ch->erate = 0;
			else
				ch->erate = (uint32)
					((double) FIXED_POINT / env_counter_max * ch->xenv_rate);

			for (j = 0; j < 32; j++)
				if (env_counter_table[j] == ch->xattack_rate)
					break;
			ch->attack_rate  = OldAttackRate[(unsigned) (((j - 1) >> 1) & 0xF)];

			for (j = 0; j < 32; j++)
				if (env_counter_table[j] == ch->xdecay_rate)
					break;
			ch->decay_rate   = OldDecayRate[(unsigned) (((j - 0x10) >> 1) & 0x7)];

			for (j = 0; j < 32; j++)
				if (env_counter_table[j] == ch->xsustain_rate)
					break;
			ch->sustain_rate = OldSustainRate[(unsigned) (j & 0x1F)];
		}

		for (j = 0; j < 32; j++)
			if (env_counter_table[j] == SoundData.noise_rate)
				break;

		for (i = 0; i < NUM_CHANNELS; i++)
		{
			Channel *ch = &SoundData.channels[i];

			temp_hertz[i] = ch->hertz;
			if (ch->type == SOUND_NOISE)
				ch->hertz = OldNoiseFreq[(unsigned) (j & 0x1F)];
		}
	}
	else
	{
		for (i = 0; i < NUM_CHANNELS; i++)
		{
			Channel *ch = &SoundData.channels[i];

			ch->hertz = temp_hertz[i];
		}
	}
}

static void S9xConvertSoundOldValues ()
{
	int i, j;
	int old_noise_freq = 0;

	for (i = 0; i < NUM_CHANNELS; i++)
	{
		Channel *ch = &SoundData.channels[i];

		ch->xenvx = ch->envx << 4;
		ch->xenvx_target = ch->envx_target << 4;
		ch->out_sample = ((ch->sample * ch->xenvx) >> 11) & ~1;
		ch->xsustain_level = ch->sustain_level << 8;

		ch->xenv_rate  = (int32) ((double) ch->erate     * env_counter_max / FIXED_POINT);
		ch->xenv_count = env_counter_max -
						 (int32) ((double) ch->env_error * env_counter_max / FIXED_POINT);

		for (j = 0; j < 16; j++)
			if (OldAttackRate[j]  == ch->attack_rate)
				break;
		ch->xattack_rate  = env_counter_table[(unsigned) (((j << 1) + 1) & 0x1F)];

		for (j = 0; j <  8; j++)
			if (OldDecayRate[j]   == ch->decay_rate)
				break;
		ch->xdecay_rate   = env_counter_table[(unsigned) (((j << 1) + 0x10) & 0x1F)];

		for (j = 0; j < 32; j++)
			if (OldSustainRate[j] == ch->sustain_rate)
				break;
		ch->xsustain_rate = env_counter_table[(unsigned) (j & 0x1F)];

		if (ch->type == SOUND_NOISE)
		{
			old_noise_freq = ch->hertz;
			ch->hertz = 32000;
		}
	}

	if (old_noise_freq)
	{
		for (j = 0; j < 32; j++)
			if (OldNoiseFreq[j] == old_noise_freq)
				break;
		SoundData.noise_rate = env_counter_table[(unsigned) (j & 0x1F)];
	}
	else
		SoundData.noise_rate = 0;
}

void S9xFixSoundAfterSnapshotLoad (int version)
{
	S9xSetEchoEnable (APU.DSP[APU_EON]);
	S9xSetEchoWriteEnable (!(APU.DSP[APU_FLG] & APU_ECHO_DISABLED));
	S9xSetEchoDelay (APU.DSP[APU_EDL] & 0xF);
	S9xSetEchoFeedback ((signed char) APU.DSP[APU_EFB]);

	S9xSetFilterCoefficient (0, (signed char) APU.DSP[APU_C0]);
	S9xSetFilterCoefficient (1, (signed char) APU.DSP[APU_C1]);
	S9xSetFilterCoefficient (2, (signed char) APU.DSP[APU_C2]);
	S9xSetFilterCoefficient (3, (signed char) APU.DSP[APU_C3]);
	S9xSetFilterCoefficient (4, (signed char) APU.DSP[APU_C4]);
	S9xSetFilterCoefficient (5, (signed char) APU.DSP[APU_C5]);
	S9xSetFilterCoefficient (6, (signed char) APU.DSP[APU_C6]);
	S9xSetFilterCoefficient (7, (signed char) APU.DSP[APU_C7]);

	if (version < 2)
		S9xConvertSoundOldValues ();

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		S9xSetSoundFrequency (i, SoundData.channels[i].hertz);
		SoundData.channels[i].needs_decode = TRUE;
		SoundData.channels[i].nb_index = 0;
		SoundData.channels[i].nb_sample[0] = 0;
		SoundData.channels[i].nb_sample[1] = 0;
		SoundData.channels[i].nb_sample[2] = 0;
		SoundData.channels[i].nb_sample[3] = 0;
		SoundData.channels[i].xsmp_count = 0;
		SoundData.channels[i].previous[0] = (int32) SoundData.channels[i].previous16[0];
		SoundData.channels[i].previous[1] = (int32) SoundData.channels[i].previous16[1];
	}

	SoundData.noise_count = 0;

	SoundData.master_volume[Settings.ReverseStereo]     = SoundData.master_volume_left;
	SoundData.master_volume[1 ^ Settings.ReverseStereo] = SoundData.master_volume_right;
	SoundData.echo_volume[Settings.ReverseStereo]     = SoundData.echo_volume_left;
	SoundData.echo_volume[1 ^ Settings.ReverseStereo] = SoundData.echo_volume_right;
}

void S9xSetFilterCoefficient (int tap, int value)
{
	FilterTaps[tap & 7] = value;
	SoundData.no_filter =
		FilterTaps[0] == 127 &&
		FilterTaps[1] == 0   &&
		FilterTaps[2] == 0   &&
		FilterTaps[3] == 0   &&
		FilterTaps[4] == 0   &&
		FilterTaps[5] == 0   &&
		FilterTaps[6] == 0   &&
		FilterTaps[7] == 0;
}

void S9xSetSoundADSR (int channel, int ar, int dr, int sr, int sl)
{
	Channel *ch = &SoundData.channels[channel];

	ch->xattack_rate   = env_counter_table[(ar << 1) + 1];
	ch->xdecay_rate    = env_counter_table[(dr << 1) + 0x10];
	ch->xsustain_rate  = env_counter_table[sr];
	ch->xsustain_level = (ENV_RANGE >> 3) * (sl + 1);

	switch (ch->state)
	{
		case SOUND_ATTACK:
			S9xSetEnvRate (ch, ch->xattack_rate, ENV_MAX);
			break;

		case SOUND_DECAY:
			S9xSetEnvRate (ch, ch->xdecay_rate, ch->xsustain_level);
			break;

		case SOUND_SUSTAIN:
			S9xSetEnvRate (ch, ch->xsustain_rate, 0);
			break;
	}
}

void S9xSetEnvelopeHeight (int channel, int32 xlevel)
{
	Channel *ch = &SoundData.channels[channel];

	ch->xenvx = ch->xenvx_target = xlevel;
	ch->xenv_rate = 0;
	if (xlevel == 0 && ch->state != SOUND_SILENT && ch->state != SOUND_GAIN)
		S9xAPUSetEndOfSample (channel, ch);
}

uint8 S9xGetEnvelopeHeight (int channel)
{
	if (Settings.SoundEnvelopeHeightReading)
		return ((SoundData.channels[channel].xenvx >> ENV_SHIFT) & 0x7F);
	else
		return (0);
}

static void S9xSetSoundFrequency (int channel, int hertz)
{
	if (so.playback_rate)
		SoundData.channels[channel].frequency = (uint32)
			((int64) (hertz << (FIXED_POINT_SHIFT - 15)) * 32000 / so.playback_rate);
	if (Settings.FixFrequency)
		SoundData.channels[channel].frequency = (uint32)
			(SoundData.channels[channel].frequency * so.pitch_mul);
}

void S9xSetSoundHertz (int channel, int hertz)
{
	SoundData.channels[channel].hertz = hertz;
	S9xSetSoundFrequency (channel, hertz);
}

void S9xSetSoundType (int channel, int type_of_sound)
{
	SoundData.channels[channel].type = type_of_sound;
}

bool8 S9xSetSoundMute (bool8 mute)
{
	bool8 old = so.mute_sound;
	so.mute_sound = mute;
	return (old);
}

static void AltDecodeBlock (Channel *ch)
{
	if (ch->block_pointer > 0x10000 - 9)
	{
		ch->last_block = TRUE;
		ch->loop = FALSE;
		ch->block = ch->decoded;
		memset ((void *) ch->decoded, 0, sizeof (int16) * 16);
		return;
	}

	signed char *compressed = (signed char *) &IAPU.RAM[ch->block_pointer];

	unsigned char filter = *compressed;
	ch->last_block = filter & 1;
	ch->loop = (filter & 2) != 0;

	signed short *raw = ch->block = ch->decoded;

#if (defined (USE_X86_ASM) && (defined (__i386__) || defined (__i486__) || \
	defined (__i586__) || defined (__WIN32__) || defined (__DJGPP)))
	if (Settings.AltSampleDecode == 1)
		DecodeBlockAsm (compressed, raw, &ch->previous[0], &ch->previous[1]);
	else
		DecodeBlockAsm2 (compressed, raw, &ch->previous[0], &ch->previous[1]);
#else
	int32 out;
	unsigned char shift;
	signed char sample1, sample2;
	uint32 i;

	compressed++;

	int32 prev0 = ch->previous[0];
	int32 prev1 = ch->previous[1];
	shift = filter >> 4;

	switch ((filter >> 2) & 3)
	{
		case 0:
			for (i = 8; i != 0; i--)
			{
				sample1 = *compressed++;
				sample2 = sample1 << 4;
				sample2 >>= 4;
				sample1 >>= 4;
				*raw++ = ((int32) sample1 << shift);
				*raw++ = ((int32) sample2 << shift);
			}
			prev1 = *(raw - 2);
			prev0 = *(raw - 1);
			break;

		case 1:
			for (i = 8; i != 0; i--)
			{
				sample1 = *compressed++;
				sample2 = sample1 << 4;
				sample2 >>= 4;
				sample1 >>= 4;
				prev0 = (int16) prev0;
				*raw++ = prev1 = ((int32) sample1 << shift) + prev0 - (prev0 >> 4);
				prev1 = (int16) prev1;
				*raw++ = prev0 = ((int32) sample2 << shift) + prev1 - (prev1 >> 4);
			}
			break;

		case 2:
			for (i = 8; i != 0; i--)
			{
				sample1 = *compressed++;
				sample2 = sample1 << 4;
				sample2 >>= 4;
				sample1 >>= 4;

				out = (sample1 << shift) - prev1 + (prev1 >> 4);
				prev1 = (int16) prev0;
				prev0 &= ~3;
				*raw++ = prev0 = out + (prev0 << 1) - (prev0 >> 5) -
					(prev0 >> 4);

				out = (sample2 << shift) - prev1 + (prev1 >> 4);
				prev1 = (int16) prev0;
				prev0 &= ~3;
				*raw++ = prev0 = out + (prev0 << 1) - (prev0 >> 5) -
					(prev0 >> 4);
			}
			break;

		case 3:
			for (i = 8; i != 0; i--)
			{
				sample1 = *compressed++;
				sample2 = sample1 << 4;
				sample2 >>= 4;
				sample1 >>= 4;
				out = (sample1 << shift);

				out = out - prev1 + (prev1 >> 3) + (prev1 >> 4);
				prev1 = (int16) prev0;
				prev0 &= ~3;
				*raw++ = prev0 = out + (prev0 << 1) - (prev0 >> 3) -
					(prev0 >> 4) - (prev1 >> 6);

				out = (sample2 << shift);
				out = out - prev1 + (prev1 >> 3) + (prev1 >> 4);
				prev1 = (int16) prev0;
				prev0 &= ~3;
				*raw++ = prev0 = out + (prev0 << 1) - (prev0 >> 3) -
					(prev0 >> 4) - (prev1 >> 6);
			}
			break;
	}

	ch->previous[0] = prev0;
	ch->previous[1] = prev1;
#endif
    ch->block_pointer += 9;
}

static void AltDecodeBlock2 (Channel *ch)
{
	int32 out;
	unsigned char filter;
	unsigned char shift;
	signed char sample1, sample2;
	uint32 i;

	if (ch->block_pointer > 0x10000 - 9)
	{
		ch->last_block = TRUE;
		ch->loop = FALSE;
		ch->block = ch->decoded;
		memset ((void *) ch->decoded, 0, sizeof (int16) * 16);
		return;
	}

	signed char *compressed = (signed char *) &IAPU.RAM[ch->block_pointer];

	filter = *compressed;
	ch->last_block = filter & 1;
	ch->loop = (filter & 2) != 0;

	compressed++;
	signed short *raw = ch->block = ch->decoded;

	shift = filter >> 4;
	int32 prev0 = ch->previous[0];
	int32 prev1 = ch->previous[1];

	if(shift > 12)
		shift -= 4;

	switch ((filter >> 2) & 3)
	{
		case 0:
			for (i = 8; i != 0; i--)
			{
				sample1 = *compressed++;
				sample2 = sample1 << 4;
				sample2 >>= 4;
				sample1 >>= 4;

				out = (int32) (sample1 << shift);

				prev1 = prev0;
				prev0 = out;
				CLIP16(out);
				*raw++ = (int16) out;

				out = (int32) (sample2 << shift);

				prev1 = prev0;
				prev0 = out;
				CLIP16(out);
				*raw++ = (int16) out;
			}
			break;

		case 1:
			for (i = 8; i != 0; i--)
			{
				sample1 = *compressed++;
				sample2 = sample1 << 4;
				sample2 >>= 4;
				sample1 >>= 4;
				out = (int32) (sample1 << shift);
				out += (int32) ((double) prev0 * 15/16);

				prev1 = prev0;
				prev0 = out;
				CLIP16(out);
				*raw++ = (int16) out;

				out = (int32) (sample2 << shift);
				out += (int32) ((double) prev0 * 15/16);

				prev1 = prev0;
				prev0 = out;
				CLIP16(out);
				*raw++ = (int16) out;
			}
			break;

		case 2:
			for (i = 8; i != 0; i--)
			{
				sample1 = *compressed++;
				sample2 = sample1 << 4;
				sample2 >>= 4;
				sample1 >>= 4;

				out = ((sample1 << shift) * 256 + (prev0 & ~0x2) * 488 - prev1 * 240) >> 8;

				prev1 = prev0;
				prev0 = (int16) out;
				*raw++ = (int16) out;

				out = ((sample2 << shift) * 256 + (prev0 & ~0x2) * 488 - prev1 * 240) >> 8;

				prev1 = prev0;
				prev0 = (int16) out;
				*raw++ = (int16) out;
			}
			break;

		case 3:
			for (i = 8; i != 0; i--)
			{
				sample1 = *compressed++;
				sample2 = sample1 << 4;
				sample2 >>= 4;
				sample1 >>= 4;
				out = (int32) (sample1 << shift);
				out += (int32) ((double) prev0 * 115/64 - (double) prev1 * 13/16);

				prev1 = prev0;
				prev0 = out;

				CLIP16(out);
				*raw++ = (int16) out;

				out = (int32) (sample2 << shift);
				out += (int32) ((double) prev0 * 115/64 - (double) prev1 * 13/16);

				prev1 = prev0;
				prev0 = out;

				CLIP16(out);
				*raw++ = (int16) out;
			}
			break;
	}

	ch->previous[0] = prev0;
	ch->previous[1] = prev1;

	ch->block_pointer += 9;
}

static void DecodeBlock (Channel *ch)
{
	int32 out;
	unsigned char filter;
	unsigned char shift;
	signed char sample1, sample2;
	bool invalid_header;

	if (Settings.AltSampleDecode)
	{
		if (Settings.AltSampleDecode < 3)
			AltDecodeBlock (ch);
		else
			AltDecodeBlock2 (ch);
		return;
	}

	if (ch->block_pointer > 0x10000 - 9)
	{
		ch->last_block = TRUE;
		ch->loop = FALSE;
		ch->block = ch->decoded;
		return;
	}

	signed char *compressed = (signed char *) &IAPU.RAM[ch->block_pointer];

	filter = *compressed;
	ch->last_block = filter & 1;
	ch->loop = (filter & 2) != 0;

	compressed++;
	signed short *raw = ch->block = ch->decoded;

	// Seperate out the header parts used for decoding

	shift = filter >> 4;

	// Header validity check: if range(shift) is over 12, ignore
	// all bits of the data for that block except for the sign bit of each
	invalid_header = !(shift < 0xD);

	filter &= 0x0C;

	int32 prev0 = ch->previous[0];
	int32 prev1 = ch->previous[1];

	for (uint32 i = 8; i != 0; i--)
	{
		sample1 = *compressed++;
		sample2 = sample1 << 4;
		//Sample 2 = Bottom Nibble, Sign Extended.
		sample2 >>= 4;
		//Sample 1 = Top Nibble, shifted down and Sign Extended.
		sample1 >>= 4;

		for (int nybblesmp = 0; nybblesmp < 2; nybblesmp++)
		{
			out = (nybblesmp ? sample2 : sample1);
			if (!invalid_header)
				out = (out << shift) >> 1;
			else
				out &= ~0x7FF;

			switch (filter)
			{
				case 0x00:
					// Method0 -[Smp]
					break;

				case 0x04:
					// Method1 -[Delta]+[Smp-1](15/16)
					out += prev0 >> 1;
					out += (-prev0) >> 5;
					break;

				case 0x08:
					// Method2 -[Delta]+[Smp-1](61/32)-[Smp-2](15/16)
					out += prev0;
					out += (-(prev0 + (prev0 >> 1))) >> 5;
					out -= prev1 >> 1;
					out += prev1 >> 5;
					break;

				case 0x0C:
					// Method3 -[Delta]+[Smp-1](115/64)-[Smp-2](13/16)
					out += prev0;
					out += (-(prev0 + (prev0 << 2) + (prev0 << 3))) >> 7;
					out -= prev1 >> 1;
					out += (prev1 + (prev1 >> 1)) >> 4;
					break;
			}

			CLIP16(out);

			prev1 = (signed short) prev0;
			prev0 = *raw++ = (signed short) (out << 1);
		}
	}

	ch->previous[0] = prev0;
	ch->previous[1] = prev1;

	ch->block_pointer += 9;
}

void MixStereo (int sample_count)
{
	DoFakeMute=Settings.FakeMuteFix;

	static int32 noise_cache[256];
	static int32 wave[SOUND_BUFFER_SIZE];

	int pitch_mod = SoundData.pitch_mod & ~APU.DSP[APU_NON];

	int32 noise_index = 0;
	int32 noise_count = 0;

	if (APU.DSP[APU_NON])
	{
		noise_count = SoundData.noise_count;

		for (uint32 I = 0; I < (uint32) sample_count; I += 2)
		{
			noise_count -= SoundData.noise_rate;
			while (noise_count <= 0)
			{
				rand_seed = rand_seed * 48828125 + 1;
				noise_cache[noise_index] = rand_seed;
				noise_index = (noise_index + 1) & 0xFF;
				noise_count += env_counter_max;
			}
		}
	}

	for (uint32 J = 0; J < NUM_CHANNELS; J++)
	{
		Channel *ch = &SoundData.channels[J];
		uint32 freq = ch->frequency;

		bool8 last_block = FALSE;

		if (ch->type == SOUND_NOISE)
		{
			noise_index = 0;
			noise_count = SoundData.noise_count;
		}

		if (ch->state == SOUND_SILENT || last_block || !(so.sound_switch & (1 << J)))
			continue;

		bool8 mod1 = pitch_mod & (1 << J);
		bool8 mod2 = pitch_mod & (1 << (J + 1));

		if (ch->needs_decode)
		{
			DecodeBlock(ch);
			ch->needs_decode = FALSE;
			ch->sample = ch->block[0];
			ch->sample_pointer = 0;
		}

		for (uint32 I = 0; I < (uint32) sample_count; I += 2)
		{
			switch (ch->state)
			{
				case SOUND_ATTACK:
					if (ch->xenv_rate == env_counter_max_master)
						ch->xenvx += (ENV_RANGE >> 1); // FIXME
					else
					{
						ch->xenv_count -= ch->xenv_rate;
						while (ch->xenv_count <= 0)
						{
							ch->xenvx += (ENV_RANGE >> 6); // 1/64
							ch->xenv_count += env_counter_max;
						}
					}

					if (ch->xenvx > ENV_MAX)
					{
						ch->xenvx = ENV_MAX;

						if (ch->xsustain_level != ENV_RANGE)
						{
							ch->state = SOUND_DECAY;
							S9xSetEnvRate (ch, ch->xdecay_rate, ch->xsustain_level);
						}
						else
						{
							ch->state = SOUND_SUSTAIN;
							S9xSetEnvRate (ch, ch->xsustain_rate, 0);
						}
					}

					break;

				case SOUND_DECAY:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= ((ch->xenvx - 1) >> 8) + 1; // 1 - 1/256
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= ch->xenvx_target)
					{
						if (ch->xenvx <= 0)
						{
							S9xAPUSetEndOfSample (J, ch);
							goto stereo_exit;
						}
						else
						{
							ch->state = SOUND_SUSTAIN;
							S9xSetEnvRate (ch, ch->xsustain_rate, 0);
						}
					}

					break;

				case SOUND_SUSTAIN:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= ((ch->xenvx - 1) >> 8) + 1;  // 1 - 1/256
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= 0)
					{
						S9xAPUSetEndOfSample (J, ch);
						goto stereo_exit;
					}

					break;

				case SOUND_RELEASE:
					ch->xenv_count -= env_counter_max;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= (ENV_RANGE >> 8); // 1/256
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= 0)
					{
						S9xAPUSetEndOfSample (J, ch);
						goto stereo_exit;
					}

					break;

				case SOUND_INCREASE_LINEAR:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx += (ENV_RANGE >> 6); // 1/64
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx > ENV_MAX)
					{
						ch->xenvx = ENV_MAX;
						ch->state = SOUND_GAIN;
						ch->mode  = MODE_GAIN;
						S9xSetEnvRate (ch, 0, 0);
					}

					break;

				case SOUND_INCREASE_BENT_LINE:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						if (ch->xenvx >= ((ENV_RANGE * 3) >> 2)) // 0x600
							ch->xenvx += (ENV_RANGE >> 8); // 1/256
						else
							ch->xenvx += (ENV_RANGE >> 6); // 1/64

						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx > ENV_MAX)
					{
						ch->xenvx = ENV_MAX;
						ch->state = SOUND_GAIN;
						ch->mode  = MODE_GAIN;
						S9xSetEnvRate (ch, 0, 0);
					}

					break;

				case SOUND_DECREASE_LINEAR:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= (ENV_RANGE >> 6); // 1/64
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= 0)
					{
						S9xAPUSetEndOfSample (J, ch);
						goto stereo_exit;
					}

					break;

				case SOUND_DECREASE_EXPONENTIAL:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= ((ch->xenvx - 1) >> 8) + 1; // 1 - 1/256
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= 0)
					{
						S9xAPUSetEndOfSample (J, ch);
						goto stereo_exit;
					}

					break;

				case SOUND_GAIN:
					S9xSetEnvRate (ch, 0, 0);

					break;
			}

			ch->xsmp_count += mod1 ? (((int64) freq * (32768 + wave[I >> 1])) >> 15) : freq;

			while (ch->xsmp_count >= 0)
			{
				ch->xsmp_count -= FIXED_POINT;
				ch->nb_sample[ch->nb_index] = ch->sample;
				ch->nb_index = (ch->nb_index + 1) & 3;

				ch->sample_pointer++;
				if (ch->sample_pointer == SOUND_DECODE_LENGTH)
				{
					ch->sample_pointer = 0;

					if (ch->last_block)
					{
						S9xAPUSetEndX (J);
						if (!ch->loop)
						{
							ch->xenvx = 0;
							last_block = TRUE;
							//S9xAPUSetEndOfSample (J, ch);
							while (ch->xsmp_count >= 0)
							{
								ch->xsmp_count -= FIXED_POINT;
								ch->nb_sample[ch->nb_index] = 0;
								ch->nb_index = (ch->nb_index + 1) & 3;
							}

							break;
						}
						else
						{
							ch->last_block = FALSE;
							uint8 *dir = S9xGetSampleAddress (ch->sample_number);
							ch->block_pointer = READ_WORD(dir + 2); // loop pointer
						}
					}

					DecodeBlock (ch);
				}

				ch->sample = ch->block[ch->sample_pointer];
			}

			int32 outx, d;

			if (ch->type == SOUND_SAMPLE)
			{
				if (Settings.InterpolatedSound)
				{
					// 4-point gaussian interpolation
					d = ch->xsmp_count >> (FIXED_POINT_SHIFT - 8);
					outx  = ((G4(-d) * ch->nb_sample[ ch->nb_index         ]) >> 11) & ~1;
					outx += ((G3(-d) * ch->nb_sample[(ch->nb_index + 1) & 3]) >> 11) & ~1;
					outx += ((G2( d) * ch->nb_sample[(ch->nb_index + 2) & 3]) >> 11) & ~1;
					outx = ((outx & 0xFFFF) ^ 0x8000) - 0x8000;
					outx += ((G1( d) * ch->nb_sample[(ch->nb_index + 3) & 3]) >> 11) & ~1;
					CLIP16(outx);
				}
				else
					outx = ch->sample;
			}
			else // SAMPLE_NOISE
			{
				noise_count -= SoundData.noise_rate;
				while (noise_count <= 0)
				{
					noise_count += env_counter_max;
					noise_index = (noise_index + 1) & 0xFF;
				}

				outx = noise_cache[noise_index] >> 16;
			}

			outx = ((outx * ch->xenvx) >> 11) & ~1;
			ch->out_sample = outx;

			if (mod2)
				wave[I >> 1] = outx;

			int32 VL, VR;

			VL = (outx * ch->volume_left ) >> 7;
			VR = (outx * ch->volume_right) >> 7;

			MixBuffer[I      ^ Settings.ReverseStereo ] += VL;
			MixBuffer[I + (1 ^ Settings.ReverseStereo)] += VR;
			ch->echo_buf_ptr[I      ^ Settings.ReverseStereo ] += VL;
			ch->echo_buf_ptr[I + (1 ^ Settings.ReverseStereo)] += VR;
		}

	stereo_exit: ;
	}
	DoFakeMute=FALSE;

	if (APU.DSP[APU_NON])
		SoundData.noise_count = noise_count;
}

#ifdef __DJGPP
END_OF_FUNCTION(MixStereo);
#endif

void MixMono (int sample_count)
{
	DoFakeMute=Settings.FakeMuteFix;

	static int32 noise_cache[256];
	static int32 wave[SOUND_BUFFER_SIZE];

	int pitch_mod = SoundData.pitch_mod & ~APU.DSP[APU_NON];

	int32 noise_index = 0;
	int32 noise_count = 0;

	if (APU.DSP[APU_NON])
	{
		noise_count = SoundData.noise_count;

		for (uint32 I = 0; I < (uint32) sample_count; I++)
		{
			noise_count -= SoundData.noise_rate;
			while (noise_count <= 0)
			{
				rand_seed = rand_seed * 48828125 + 1;
				noise_cache[noise_index] = rand_seed;
				noise_index = (noise_index + 1) & 0xFF;
				noise_count += env_counter_max;
			}
		}
	}

	for (uint32 J = 0; J < NUM_CHANNELS; J++)
	{
		Channel *ch = &SoundData.channels[J];
		uint32 freq = ch->frequency;

		bool8 last_block = FALSE;

		if (ch->type == SOUND_NOISE)
		{
			noise_index = 0;
			noise_count = SoundData.noise_count;
		}

		if (ch->state == SOUND_SILENT || last_block || !(so.sound_switch & (1 << J)))
			continue;

		bool8 mod1 = pitch_mod & (1 << J);
		bool8 mod2 = pitch_mod & (1 << (J + 1));

		if (ch->needs_decode)
		{
			DecodeBlock(ch);
			ch->needs_decode = FALSE;
			ch->sample = ch->block[0];
			ch->sample_pointer = 0;
		}

		for (uint32 I = 0; I < (uint32) sample_count; I++)
		{
			switch (ch->state)
			{
				case SOUND_ATTACK:
					if (ch->xenv_rate == env_counter_max_master)
						ch->xenvx += (ENV_RANGE >> 1); // FIXME
					else
					{
						ch->xenv_count -= ch->xenv_rate;
						while (ch->xenv_count <= 0)
						{
							ch->xenvx += (ENV_RANGE >> 6); // 1/64
							ch->xenv_count += env_counter_max;
						}
					}

					if (ch->xenvx > ENV_MAX)
					{
						ch->xenvx = ENV_MAX;

						if (ch->xsustain_level != ENV_RANGE)
						{
							ch->state = SOUND_DECAY;
							S9xSetEnvRate (ch, ch->xdecay_rate, ch->xsustain_level);
						}
						else
						{
							ch->state = SOUND_SUSTAIN;
							S9xSetEnvRate (ch, ch->xsustain_rate, 0);
						}
					}

					break;

				case SOUND_DECAY:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= ((ch->xenvx - 1) >> 8) + 1; // 1 - 1/256
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= ch->xenvx_target)
					{
						if (ch->xenvx <= 0)
						{
							S9xAPUSetEndOfSample (J, ch);
							goto mono_exit;
						}
						else
						{
							ch->state = SOUND_SUSTAIN;
							S9xSetEnvRate (ch, ch->xsustain_rate, 0);
						}
					}

					break;

				case SOUND_SUSTAIN:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= ((ch->xenvx - 1) >> 8) + 1;  // 1 - 1/256
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= 0)
					{
						S9xAPUSetEndOfSample (J, ch);
						goto mono_exit;
					}

					break;

				case SOUND_RELEASE:
					ch->xenv_count -= env_counter_max;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= (ENV_RANGE >> 8); // 1/256
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= 0)
					{
						S9xAPUSetEndOfSample (J, ch);
						goto mono_exit;
					}

					break;

				case SOUND_INCREASE_LINEAR:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx += (ENV_RANGE >> 6); // 1/64
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx > ENV_MAX)
					{
						ch->xenvx = ENV_MAX;
						ch->state = SOUND_GAIN;
						ch->mode  = MODE_GAIN;
						S9xSetEnvRate (ch, 0, 0);
					}

					break;

				case SOUND_INCREASE_BENT_LINE:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						if (ch->xenvx >= ((ENV_RANGE * 3) >> 2)) // 0x600
							ch->xenvx += (ENV_RANGE >> 8); // 1/256
						else
							ch->xenvx += (ENV_RANGE >> 6); // 1/64

						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx > ENV_MAX)
					{
						ch->xenvx = ENV_MAX;
						ch->state = SOUND_GAIN;
						ch->mode  = MODE_GAIN;
						S9xSetEnvRate (ch, 0, 0);
					}

					break;

				case SOUND_DECREASE_LINEAR:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= (ENV_RANGE >> 6); // 1/64
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= 0)
					{
						S9xAPUSetEndOfSample (J, ch);
						goto mono_exit;
					}

					break;

				case SOUND_DECREASE_EXPONENTIAL:
					ch->xenv_count -= ch->xenv_rate;
					while (ch->xenv_count <= 0)
					{
						ch->xenvx -= ((ch->xenvx - 1) >> 8) + 1; // 1 - 1/256
						ch->xenv_count += env_counter_max;
					}

					if (ch->xenvx <= 0)
					{
						S9xAPUSetEndOfSample (J, ch);
						goto mono_exit;
					}

					break;

				case SOUND_GAIN:
					S9xSetEnvRate (ch, 0, 0);

					break;
			}

			ch->xsmp_count += mod1 ? (((int64) freq * (32768 + wave[I])) >> 15) : freq;

			while (ch->xsmp_count >= 0)
			{
				ch->xsmp_count -= FIXED_POINT;
				ch->nb_sample[ch->nb_index] = ch->sample;
				ch->nb_index = (ch->nb_index + 1) & 3;

				ch->sample_pointer++;
				if (ch->sample_pointer == SOUND_DECODE_LENGTH)
				{
					ch->sample_pointer = 0;

					if (ch->last_block)
					{
						S9xAPUSetEndX (J);
						if (!ch->loop)
						{
							ch->xenvx = 0;
							last_block = TRUE;
							//S9xAPUSetEndOfSample (J, ch);
							while (ch->xsmp_count >= 0)
							{
								ch->xsmp_count -= FIXED_POINT;
								ch->nb_sample[ch->nb_index] = 0;
								ch->nb_index = (ch->nb_index + 1) & 3;
							}

							break;
						}
						else
						{
							ch->last_block = FALSE;
							uint8 *dir = S9xGetSampleAddress (ch->sample_number);
							ch->block_pointer = READ_WORD(dir + 2); // loop pointer
						}
					}

					DecodeBlock (ch);
				}

				ch->sample = ch->block[ch->sample_pointer];
			}

			int32 outx, d;

			if (ch->type == SOUND_SAMPLE)
			{
				if (Settings.InterpolatedSound)
				{
					// 4-point gaussian interpolation
					d = ch->xsmp_count >> (FIXED_POINT_SHIFT - 8);
					outx  = ((G4(-d) * ch->nb_sample[ ch->nb_index         ]) >> 11) & ~1;
					outx += ((G3(-d) * ch->nb_sample[(ch->nb_index + 1) & 3]) >> 11) & ~1;
					outx += ((G2( d) * ch->nb_sample[(ch->nb_index + 2) & 3]) >> 11) & ~1;
					outx = ((outx & 0xFFFF) ^ 0x8000) - 0x8000;
					outx += ((G1( d) * ch->nb_sample[(ch->nb_index + 3) & 3]) >> 11) & ~1;
					CLIP16(outx);
				}
				else
					outx = ch->sample;
			}
			else // SAMPLE_NOISE
			{
				noise_count -= SoundData.noise_rate;
				while (noise_count <= 0)
				{
					noise_count += env_counter_max;
					noise_index = (noise_index + 1) & 0xFF;
				}

				outx = noise_cache[noise_index] >> 16;
			}

			outx = ((outx * ch->xenvx) >> 11) & ~1;
			ch->out_sample = outx;

			if (mod2)
				wave[I] = outx;

			int32 V;

			V = (outx * ch->volume_left ) >> 7;

			MixBuffer[I] += V;
			ch->echo_buf_ptr[I] += V;
		}

	mono_exit: ;
	}
	DoFakeMute=FALSE;

	if (APU.DSP[APU_NON])
		SoundData.noise_count = noise_count;
}

#ifdef __DJGPP
END_OF_FUNCTION(MixMono);
#endif

#ifdef __sun
extern uint8 int2ulaw (int);
#endif

// For backwards compatibility with older port specific code
void S9xMixSamplesO (uint8 *buffer, int sample_count, int byte_offset)
{
	S9xMixSamples (buffer+byte_offset, sample_count);
}

#ifdef __DJGPP
END_OF_FUNCTION(S9xMixSamplesO);
#endif

void S9xMixSamples (uint8 *buffer, int sample_count)
{
	int I, J;

	if (!so.mute_sound)
	{
		memset (MixBuffer, 0, sample_count * sizeof (MixBuffer[0]));
		if (!Settings.DisableSoundEcho)
			memset (EchoBuffer, 0, sample_count * sizeof (EchoBuffer[0]));

		if (so.stereo)
			MixStereo (sample_count);
		else
			MixMono (sample_count);
	}

	/* Mix and convert waveforms */
	if (so.sixteen_bit)
	{
		// 16-bit sound
		if (so.mute_sound)
			memset (buffer, 0, sample_count << 1);
		else
		{
			if (!Settings.DisableSoundEcho)
			{
				if (so.stereo)
				{
					// 16-bit stereo sound with echo enabled ...
					if (SoundData.no_filter)
					{
						// ... but no filter defined.
						for (J = 0; J < sample_count; J++)
						{
							int E = Echo[SoundData.echo_ptr];

							Loop[FIRIndex & 15] = E;
							E = (E * 127) >> 7;
							FIRIndex++;

							if (SoundData.echo_write_enabled)
							{
								I = EchoBuffer[J] + ((E * SoundData.echo_feedback) >> 7);
								CLIP16(I);
								Echo[SoundData.echo_ptr] = I;
							}
							else // FIXME: Snes9x's echo buffer is not in APU_RAM
								Echo[SoundData.echo_ptr] = 0;

							if (++SoundData.echo_ptr >= SoundData.echo_buffer_size)
								SoundData.echo_ptr = 0;

							I = (MixBuffer[J] * SoundData.master_volume[J & 1] +
								E * SoundData.echo_volume[J & 1]) >> 7;
							CLIP16(I);
							((int16 *) buffer) [J] = I;
						}
					}
					else
					{
						// ... with filter defined.
						for (J = 0; J < sample_count; J++)
						{
							int E = Echo[SoundData.echo_ptr];

							Loop[FIRIndex & 15] = E;
							E  = E                          * FilterTaps[0];
							E += Loop[(FIRIndex -  2) & 15] * FilterTaps[1];
							E += Loop[(FIRIndex -  4) & 15] * FilterTaps[2];
							E += Loop[(FIRIndex -  6) & 15] * FilterTaps[3];
							E += Loop[(FIRIndex -  8) & 15] * FilterTaps[4];
							E += Loop[(FIRIndex - 10) & 15] * FilterTaps[5];
							E += Loop[(FIRIndex - 12) & 15] * FilterTaps[6];
							E += Loop[(FIRIndex - 14) & 15] * FilterTaps[7];
							E >>= 7;
							FIRIndex++;

							if (SoundData.echo_write_enabled)
							{
								I = EchoBuffer[J] + ((E * SoundData.echo_feedback) >> 7);
								CLIP16(I);
								Echo[SoundData.echo_ptr] = I;
							}
							else // FIXME: Snes9x's echo buffer is not in APU_RAM
								Echo[SoundData.echo_ptr] = 0;

							if (++SoundData.echo_ptr >= SoundData.echo_buffer_size)
								SoundData.echo_ptr = 0;

							I = (MixBuffer[J] * SoundData.master_volume[J & 1] +
								E * SoundData.echo_volume[J & 1]) >> 7;
							CLIP16(I);
							((int16 *) buffer) [J] = I;
						}
					}
				}
				else
				{
					// 16-bit mono sound with echo enabled...
					if (SoundData.no_filter)
					{
						// ... no filter defined
						for (J = 0; J < sample_count; J++)
						{
							int E = Echo[SoundData.echo_ptr];

							Loop[FIRIndex & 7] = E;
							E = (E * 127) >> 7;
							FIRIndex++;

							if (SoundData.echo_write_enabled)
							{
								I = EchoBuffer[J] + ((E * SoundData.echo_feedback) >> 7);
								CLIP16(I);
								Echo[SoundData.echo_ptr] = I;
							}
							else // FIXME: Snes9x's echo buffer is not in APU_RAM
								Echo[SoundData.echo_ptr] = 0;

							if (++SoundData.echo_ptr >= SoundData.echo_buffer_size)
								SoundData.echo_ptr = 0;

							I = (MixBuffer[J] * SoundData.master_volume[0] +
								E * SoundData.echo_volume[0]) >> 7;
							CLIP16(I);
							((int16 *) buffer) [J] = I;
						}
					}
					else
					{
						// ... with filter defined
						for (J = 0; J < sample_count; J++)
						{
							int E = Echo[SoundData.echo_ptr];

							Loop[FIRIndex & 7] = E;
							E  = E                        * FilterTaps[0];
							E += Loop[(FIRIndex - 1) & 7] * FilterTaps[1];
							E += Loop[(FIRIndex - 2) & 7] * FilterTaps[2];
							E += Loop[(FIRIndex - 3) & 7] * FilterTaps[3];
							E += Loop[(FIRIndex - 4) & 7] * FilterTaps[4];
							E += Loop[(FIRIndex - 5) & 7] * FilterTaps[5];
							E += Loop[(FIRIndex - 6) & 7] * FilterTaps[6];
							E += Loop[(FIRIndex - 7) & 7] * FilterTaps[7];
							E >>= 7;
							FIRIndex++;

							if (SoundData.echo_write_enabled)
							{
								I = EchoBuffer[J] + ((E * SoundData.echo_feedback) >> 7);
								CLIP16(I);
								Echo[SoundData.echo_ptr] = I;
							}
							else // FIXME: Snes9x's echo buffer is not in APU_RAM
								Echo[SoundData.echo_ptr] = 0;

							if (++SoundData.echo_ptr >= SoundData.echo_buffer_size)
								SoundData.echo_ptr = 0;

							I = (MixBuffer[J] * SoundData.master_volume[0] +
								E * SoundData.echo_volume[0]) >> 7;
							CLIP16(I);
							((int16 *) buffer) [J] = I;
						}
					}
				}
			}
			else
			{
				// 16-bit mono or stereo sound, no echo
				for (J = 0; J < sample_count; J++)
				{
					I = (MixBuffer[J] * SoundData.master_volume[J & 1]) >> 7;
					CLIP16(I);
					((int16 *) buffer) [J] = I;
				}
			}
		}
	}
    else
    {
		// 8-bit sound
		if (so.mute_sound)
			memset (buffer, 128, sample_count);
		else
	#ifdef __sun
		if (so.encoded)
		{
			for (J = 0; J < sample_count; J++)
			{
				I = (MixBuffer[J] * SoundData.master_volume[0]) >> 7;
				CLIP16(I);
				buffer[J] = int2ulaw (I);
			}
		}
		else
	#endif
		{
			if (!Settings.DisableSoundEcho)
			{
				if (so.stereo)
				{
					// 8-bit stereo sound with echo enabled...
					if (SoundData.no_filter)
					{
						// ... but no filter
						for (J = 0; J < sample_count; J++)
						{
							int E = Echo[SoundData.echo_ptr];

							Loop[FIRIndex & 15] = E;
							E = (E * 127) >> 7;
							FIRIndex++;

							if (SoundData.echo_write_enabled)
							{
								I = EchoBuffer[J] + ((E * SoundData.echo_feedback) >> 7);
								CLIP16(I);
								Echo[SoundData.echo_ptr] = I;
							}
							else // FIXME: Snes9x's echo buffer is not in APU_RAM
								Echo[SoundData.echo_ptr] = 0;

							if (++SoundData.echo_ptr >= SoundData.echo_buffer_size)
								SoundData.echo_ptr = 0;

							I = (MixBuffer[J] * SoundData.master_volume[J & 1] +
								E * SoundData.echo_volume[J & 1]) >> 15;
							CLIP8(I);
							buffer[J] = I + 128;
						}
					}
					else
					{
						// ... with filter
						for (J = 0; J < sample_count; J++)
						{
							int E = Echo[SoundData.echo_ptr];

							Loop[FIRIndex & 15] = E;
							E  = E                          * FilterTaps[0];
							E += Loop[(FIRIndex -  2) & 15] * FilterTaps[1];
							E += Loop[(FIRIndex -  4) & 15] * FilterTaps[2];
							E += Loop[(FIRIndex -  6) & 15] * FilterTaps[3];
							E += Loop[(FIRIndex -  8) & 15] * FilterTaps[4];
							E += Loop[(FIRIndex - 10) & 15] * FilterTaps[5];
							E += Loop[(FIRIndex - 12) & 15] * FilterTaps[6];
							E += Loop[(FIRIndex - 14) & 15] * FilterTaps[7];
							E >>= 7;
							FIRIndex++;

							if (SoundData.echo_write_enabled)
							{
								I = EchoBuffer[J] + ((E * SoundData.echo_feedback) >> 7);
								CLIP16(I);
								Echo[SoundData.echo_ptr] = I;
							}
							else // FIXME: Snes9x's echo buffer is not in APU_RAM
								Echo[SoundData.echo_ptr] = 0;

							if (++SoundData.echo_ptr >= SoundData.echo_buffer_size)
								SoundData.echo_ptr = 0;

							I = (MixBuffer[J] * SoundData.master_volume[J & 1] +
								E * SoundData.echo_volume[J & 1]) >> 15;
							CLIP8(I);
							buffer[J] = I + 128;
						}
					}
				}
				else
				{
					// 8-bit mono sound with echo enabled...
					if (SoundData.no_filter)
					{
						// ... but no filter.
						for (J = 0; J < sample_count; J++)
						{
							int E = Echo[SoundData.echo_ptr];

							Loop[FIRIndex & 7] = E;
							E = (E * 127) >> 7;
							FIRIndex++;

							if (SoundData.echo_write_enabled)
							{
								I = EchoBuffer[J] + ((E * SoundData.echo_feedback) >> 7);
								CLIP16(I);
								Echo[SoundData.echo_ptr] = I;
							}
							else // FIXME: Snes9x's echo buffer is not in APU_RAM
								Echo[SoundData.echo_ptr] = 0;

							if (++SoundData.echo_ptr >= SoundData.echo_buffer_size)
								SoundData.echo_ptr = 0;

							I = (MixBuffer[J] * SoundData.master_volume[0] +
								E * SoundData.echo_volume[0]) >> 15;
							CLIP8(I);
							buffer[J] = I + 128;
						}
					}
					else
					{
						// ... with filter.
						for (J = 0; J < sample_count; J++)
						{
							int E = Echo[SoundData.echo_ptr];

							Loop[FIRIndex & 7] = E;
							E  = E                        * FilterTaps[0];
							E += Loop[(FIRIndex - 1) & 7] * FilterTaps[1];
							E += Loop[(FIRIndex - 2) & 7] * FilterTaps[2];
							E += Loop[(FIRIndex - 3) & 7] * FilterTaps[3];
							E += Loop[(FIRIndex - 4) & 7] * FilterTaps[4];
							E += Loop[(FIRIndex - 5) & 7] * FilterTaps[5];
							E += Loop[(FIRIndex - 6) & 7] * FilterTaps[6];
							E += Loop[(FIRIndex - 7) & 7] * FilterTaps[7];
							E >>= 7;
							FIRIndex++;

							if (SoundData.echo_write_enabled)
							{
								I = EchoBuffer[J] + ((E * SoundData.echo_feedback) >> 7);
								CLIP16(I);
								Echo[SoundData.echo_ptr] = I;
							}
							else // FIXME: Snes9x's echo buffer is not in APU_RAM
								Echo[SoundData.echo_ptr] = 0;

							if (++SoundData.echo_ptr >= SoundData.echo_buffer_size)
								SoundData.echo_ptr = 0;

							I = (MixBuffer[J] * SoundData.master_volume[0] +
								E * SoundData.echo_volume[0]) >> 15;
							CLIP8(I);
							buffer[J] = I + 128;
						}
					}
				}
			}
			else
			{
				// 8-bit mono or stereo sound, no echo
				for (J = 0; J < sample_count; J++)
				{
					I = (MixBuffer[J] * SoundData.master_volume[J & 1]) >> 15;
					CLIP8(I);
					buffer[J] = I + 128;
				}
			}
		}
	}
}

#ifdef __DJGPP
END_OF_FUNCTION(S9xMixSamples);
#endif

void S9xResetSound (bool8 full)
{
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		SoundData.channels[i].state = SOUND_SILENT;
		SoundData.channels[i].mode = MODE_NONE;
		SoundData.channels[i].type = SOUND_SAMPLE;
		SoundData.channels[i].volume_left = 0;
		SoundData.channels[i].volume_right = 0;
		SoundData.channels[i].hertz = 0;
		SoundData.channels[i].loop = FALSE;
		SoundData.channels[i].xsmp_count = 0;
		SoundData.channels[i].xenvx = 0;
		SoundData.channels[i].xenvx_target = 0;
		SoundData.channels[i].xenv_count = 0;
		SoundData.channels[i].xenv_rate = 0;
		SoundData.channels[i].xattack_rate = 0;
		SoundData.channels[i].xdecay_rate = 0;
		SoundData.channels[i].xsustain_rate = 0;
		SoundData.channels[i].xsustain_level = 0;
		if(full)
		{
			SoundData.channels[i].out_sample = 0;
			SoundData.channels[i].block_pointer = 0;
			SoundData.channels[i].sample_pointer = 0;
			SoundData.channels[i].sample = 0;
			SoundData.channels[i].sample_number = 0;
			SoundData.channels[i].last_block = 0;
			for(int j = 0 ; j < 2 ; j++) SoundData.channels[i].previous[j] = 0;
			for(int j = 0 ; j < 2 ; j++) SoundData.channels[i].previous16[j] = 0;
			for(int j = 0 ; j < 16 ; j++) SoundData.channels[i].decoded[j] = 0;
		}
	}

	FilterTaps [0] = 127;
	FilterTaps [1] = 0;
	FilterTaps [2] = 0;
	FilterTaps [3] = 0;
	FilterTaps [4] = 0;
	FilterTaps [5] = 0;
	FilterTaps [6] = 0;
	FilterTaps [7] = 0;

	rand_seed = 1;

	so.mute_sound = TRUE;
	so.noise_gen = 1;
	so.sound_switch = 255;
	so.stereo_switch = ~0;
	so.samples_mixed_so_far = 0;
	so.play_position = 0;
	so.err_counter = 0;

	if (full)
	{
		SoundData.echo_volume_left = 0;
		SoundData.echo_volume_right = 0;
		SoundData.echo_enable = 0;
		SoundData.echo_write_enabled = 0;
		SoundData.pitch_mod = 0;
		SoundData.dummy[0] = 0;
		SoundData.dummy[1] = 0;
		SoundData.dummy[2] = 0;
		SoundData.echo_volume[0] = 0;
		SoundData.echo_volume[1] = 0;
		SoundData.noise_count = 0;
		SoundData.noise_rate = 0;
		memset (Loop, 0, sizeof (Loop));
		memset (Echo, 0, sizeof (Echo));
	}

	// At least Super Bomberman 2 requires the defaule master volume is not zero.
#if 1
	SoundData.master_volume_left  = 127;
	SoundData.master_volume_right = 127;
	SoundData.master_volume [0] = SoundData.master_volume [1] = 127;
#else
	SoundData.master_volume_left  = 0;
	SoundData.master_volume_right = 0;
	SoundData.master_volume [0] = SoundData.master_volume [1] = 0;
#endif
	SoundData.no_filter = TRUE;
	SoundData.echo_ptr = 0;
	SoundData.echo_feedback = 0;
	SoundData.echo_buffer_size = 1;

	if (so.playback_rate)
		so.err_rate = (uint32) (FIXED_POINT * SNES_SCANLINE_TIME * so.playback_rate);
	else
		so.err_rate = 0;
}

void S9xSetPlaybackRate (uint32 playback_rate)
{
	if (playback_rate > 48000)
		playback_rate = 48000;

	so.playback_rate = playback_rate;
	so.err_rate = (uint32) (FIXED_POINT * SNES_SCANLINE_TIME * so.playback_rate);

	memset (Loop, 0, sizeof (Loop));
	memset (Echo, 0, sizeof (Echo));
	S9xSetEchoDelay (APU.DSP[APU_EDL] & 0xF);

	for (int i = 0; i < NUM_CHANNELS; i++)
		S9xSetSoundFrequency (i, SoundData.channels[i].hertz);

	env_counter_max = env_counter_max_master * playback_rate / 32000;
	SoundData.noise_count = env_counter_max;
}

bool8 S9xInitSound (int mode, bool8 stereo, int buffer_size)
{
	so.sound_fd = -1;
	so.sound_switch = 255;
	so.stereo_switch = ~0;

	so.playback_rate = 0;
	so.buffer_size = 0;
	so.stereo = stereo;
	so.sixteen_bit = Settings.SixteenBitSound;
	so.encoded = FALSE;
	so.pitch_mul = 0.985; // XXX: necessary for most cards in linux...?

	S9xResetSound (TRUE);

	if (!(mode & 7))
		return (1);

	S9xSetSoundMute (TRUE);
	if (!S9xOpenSoundDevice (mode, stereo, buffer_size))
	{
	#ifdef NOSOUND
		S9xMessage (S9X_WARNING, S9X_SOUND_NOT_BUILT,
			"No sound support compiled in");
	#else
		S9xMessage (S9X_ERROR, S9X_SOUND_DEVICE_OPEN_FAILED,
			"Sound device open failed");
	#endif
		return (0);
	}

	return (1);
}

bool8 S9xSetSoundMode (int channel, int mode)
{
	Channel *ch = &SoundData.channels[channel];

	switch (mode)
	{
		case MODE_RELEASE:
			if (ch->mode != MODE_NONE)
			{
				ch->mode = MODE_RELEASE;
				return (TRUE);
			}
			break;

		case MODE_DECREASE_LINEAR:
		case MODE_DECREASE_EXPONENTIAL:
		case MODE_GAIN:
			if (ch->mode != MODE_RELEASE)
			{
				ch->mode = mode;
				if (ch->state != SOUND_SILENT)
					ch->state = mode;

				return (TRUE);
			}
			break;

		case MODE_INCREASE_LINEAR:
		case MODE_INCREASE_BENT_LINE:
			if (ch->mode != MODE_RELEASE)
			{
				ch->mode = mode;
				if (ch->state != SOUND_SILENT)
					ch->state = mode;

				return (TRUE);
			}
			break;

		case MODE_ADSR:
			if (ch->mode == MODE_NONE || ch->mode == MODE_ADSR)
			{
				ch->mode = mode;
				return (TRUE);
			}
	}

	return (FALSE);
}

void S9xSetSoundControl (int sound_switch)
{
	so.sound_switch = sound_switch;
}

void S9xPlaySample (int channel)
{
	Channel *ch = &SoundData.channels[channel];

	ch->state = SOUND_SILENT;
	ch->mode = MODE_NONE;
	ch->xenvx = 0;

	S9xFixEnvelope (channel,
		APU.DSP[APU_GAIN  + (channel << 4)],
		APU.DSP[APU_ADSR1 + (channel << 4)],
		APU.DSP[APU_ADSR2 + (channel << 4)]);

	ch->sample_number = APU.DSP[APU_SRCN + channel * 0x10];
	if (APU.DSP[APU_NON] & (1 << channel))
		ch->type = SOUND_NOISE;
	else
		ch->type = SOUND_SAMPLE;

	S9xSetSoundFrequency (channel, ch->hertz);
	ch->loop = FALSE;
	ch->needs_decode = TRUE;
	ch->last_block = FALSE;
	ch->previous[0] = ch->previous[1] = 0;
	uint8 *dir = S9xGetSampleAddress (ch->sample_number);
	ch->block_pointer = READ_WORD (dir);
	ch->sample_pointer = 0;
	ch->xenv_count = env_counter_max;
	ch->xsmp_count = 3 * FIXED_POINT; // since gaussian interpolation uses 4 points
	ch->nb_sample[0] = 0;
	ch->nb_sample[1] = 0;
	ch->nb_sample[2] = 0;
	ch->nb_sample[3] = 0;
	ch->nb_index = 0;

	switch (ch->mode)
	{
		case MODE_ADSR: // FIXME: rapid attack
		#if 0
			ch->state = SOUND_ATTACK;
			ch->xenvx = 0;
			S9xSetEnvRate (ch, ch->xattack_rate, ENV_MAX);
			break;
		#else
			if (ch->xattack_rate == env_counter_max_master)
			{
				ch->xenvx = ENV_MAX;
				if (ch->xsustain_level == ENV_RANGE)
				{
					ch->state = SOUND_SUSTAIN;
					S9xSetEnvRate (ch, ch->xsustain_rate, 0);
				}
				else
				{
					ch->state = SOUND_DECAY;
					S9xSetEnvRate (ch, ch->xdecay_rate, ch->xsustain_level);
				}
			}
			else
			{
				ch->state = SOUND_ATTACK;
				ch->xenvx = 0;
				S9xSetEnvRate (ch, ch->xattack_rate, ENV_MAX);
			}

			break;
		#endif

		case MODE_GAIN:
			ch->state = SOUND_GAIN;
			break;

		case MODE_INCREASE_LINEAR:
			ch->state = SOUND_INCREASE_LINEAR;
			break;

		case MODE_INCREASE_BENT_LINE:
			ch->state = SOUND_INCREASE_BENT_LINE;
			break;

		case MODE_DECREASE_LINEAR:
			ch->state = SOUND_DECREASE_LINEAR;
			break;

		case MODE_DECREASE_EXPONENTIAL:
			ch->state = SOUND_DECREASE_EXPONENTIAL;
			break;

		default:
			break;
	}

	S9xFixEnvelope (channel,
		APU.DSP[APU_GAIN  + (channel << 4)],
		APU.DSP[APU_ADSR1 + (channel << 4)],
		APU.DSP[APU_ADSR2 + (channel << 4)]);
}
