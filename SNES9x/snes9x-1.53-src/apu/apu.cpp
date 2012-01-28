/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


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
                             Andreas Naive (andreasnaive@gmail.com),
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
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

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
 ***********************************************************************************/


#include <math.h>
#include "snes9x.h"
#include "apu.h"
#include "snapshot.h"
#include "display.h"
#include "linear_resampler.h"
#include "hermite_resampler.h"

#define APU_DEFAULT_INPUT_RATE		32000
#define APU_MINIMUM_SAMPLE_COUNT	512
#define APU_MINIMUM_SAMPLE_BLOCK	128
#define APU_NUMERATOR_NTSC			15664
#define APU_DENOMINATOR_NTSC		328125
#define APU_NUMERATOR_PAL			34176
#define APU_DENOMINATOR_PAL			709379
#define APU_DEFAULT_RESAMPLER		HermiteResampler

SNES_SPC	*spc_core = NULL;

static uint8 APUROM[64] =
{
	0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0,
	0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
	0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4,
	0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
	0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB,
	0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
	0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD,
	0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
};

namespace spc
{
	static apu_callback	sa_callback     = NULL;
	static void			*extra_data     = NULL;

	static bool8		sound_in_sync   = TRUE;
	static bool8		sound_enabled   = FALSE;

	static int			buffer_size;
	static int			lag_master      = 0;
	static int			lag             = 0;

	static uint8		*landing_buffer = NULL;
	static uint8		*shrink_buffer  = NULL;

	static Resampler	*resampler      = NULL;

	static int32		reference_time;
	static uint32		remainder;

	static const int	timing_hack_numerator   = SNES_SPC::tempo_unit;
	static int			timing_hack_denominator = SNES_SPC::tempo_unit;
	/* Set these to NTSC for now. Will change to PAL in S9xAPUTimingSetSpeedup
	   if necessary on game load. */
	static uint32		ratio_numerator = APU_NUMERATOR_NTSC;
	static uint32		ratio_denominator = APU_DENOMINATOR_NTSC;
}

static void EightBitize (uint8 *, int);
static void DeStereo (uint8 *, int);
static void ReverseStereo (uint8 *, int);
static void UpdatePlaybackRate (void);
static void from_apu_to_state (uint8 **, void *, size_t);
static void to_apu_from_state (uint8 **, void *, size_t);
static void SPCSnapshotCallback (void);
static inline int S9xAPUGetClock (int32);
static inline int S9xAPUGetClockRemainder (int32);


static void EightBitize (uint8 *buffer, int sample_count)
{
	uint8	*buf8  = (uint8 *) buffer;
	int16	*buf16 = (int16 *) buffer;

	for (int i = 0; i < sample_count; i++)
		buf8[i] = (uint8) ((buf16[i] / 256) + 128);
}

static void DeStereo (uint8 *buffer, int sample_count)
{
	int16	*buf = (int16 *) buffer;
	int32	s1, s2;

	for (int i = 0; i < sample_count >> 1; i++)
	{
		s1 = (int32) buf[2 * i];
		s2 = (int32) buf[2 * i + 1];
		buf[i] = (int16) ((s1 + s2) >> 1);
	}
}

static void ReverseStereo (uint8 *src_buffer, int sample_count)
{
	int16	*buffer = (int16 *) src_buffer;

	for (int i = 0; i < sample_count; i += 2)
	{
		buffer[i + 1] ^= buffer[i];
		buffer[i] ^= buffer[i + 1];
		buffer[i + 1] ^= buffer[i];
	}
}

bool8 S9xMixSamples (uint8 *buffer, int sample_count)
{
	static int	shrink_buffer_size = -1;
	uint8		*dest;

	if (!Settings.SixteenBitSound || !Settings.Stereo)
	{
		/* We still need both stereo samples for generating the mono sample */
		if (!Settings.Stereo)
			sample_count <<= 1;

		/* We still have to generate 16-bit samples for bit-dropping, too */
		if (shrink_buffer_size < (sample_count << 1))
		{
			delete[] spc::shrink_buffer;
			spc::shrink_buffer = new uint8[sample_count << 1];
			shrink_buffer_size = sample_count << 1;
		}

		dest = spc::shrink_buffer;
	}
	else
		dest = buffer;

	if (Settings.Mute)
	{
		memset(dest, 0, sample_count << 1);
		spc::resampler->clear();

		return (FALSE);
	}
	else
	{
		if (spc::resampler->avail() >= (sample_count + spc::lag))
		{
			spc::resampler->read((short *) dest, sample_count);
			if (spc::lag == spc::lag_master)
				spc::lag = 0;
		}
		else
		{
			memset(buffer, (Settings.SixteenBitSound ? 0 : 128), (sample_count << (Settings.SixteenBitSound ? 1 : 0)) >> (Settings.Stereo ? 0 : 1));
			if (spc::lag == 0)
				spc::lag = spc::lag_master;

			return (FALSE);
		}
	}

	if (Settings.ReverseStereo && Settings.Stereo)
		ReverseStereo(dest, sample_count);

	if (!Settings.Stereo || !Settings.SixteenBitSound)
	{
		if (!Settings.Stereo)
		{
			DeStereo(dest, sample_count);
			sample_count >>= 1;
		}

		if (!Settings.SixteenBitSound)
			EightBitize(dest, sample_count);

		memcpy(buffer, dest, (sample_count << (Settings.SixteenBitSound ? 1 : 0)));
	}

	return (TRUE);
}

int S9xGetSampleCount (void)
{
	return (spc::resampler->avail() >> (Settings.Stereo ? 0 : 1));
}

void S9xFinalizeSamples (void)
{
	if (!Settings.Mute)
	{
		if (!spc::resampler->push((short *) spc::landing_buffer, spc_core->sample_count()))
		{
			/* We weren't able to process the entire buffer. Potential overrun. */
			spc::sound_in_sync = FALSE;

			if (Settings.SoundSync && !Settings.TurboMode)
				return;
		}
	}

	if (!Settings.SoundSync || Settings.TurboMode || Settings.Mute)
		spc::sound_in_sync = TRUE;
	else
	if (spc::resampler->space_empty() >= spc::resampler->space_filled())
		spc::sound_in_sync = TRUE;
	else
		spc::sound_in_sync = FALSE;

	spc_core->set_output((SNES_SPC::sample_t *) spc::landing_buffer, spc::buffer_size >> 1);
}

void S9xLandSamples (void)
{
	if (spc::sa_callback != NULL)
		spc::sa_callback(spc::extra_data);
	else
		S9xFinalizeSamples();
}

void S9xClearSamples (void)
{
	spc::resampler->clear();
	spc::lag = spc::lag_master;
}

bool8 S9xSyncSound (void)
{
	if (!Settings.SoundSync || spc::sound_in_sync)
		return (TRUE);

	S9xLandSamples();

	return (spc::sound_in_sync);
}

void S9xSetSamplesAvailableCallback (apu_callback callback, void *data)
{
	spc::sa_callback = callback;
	spc::extra_data  = data;
}

static void UpdatePlaybackRate (void)
{
	if (Settings.SoundInputRate == 0)
		Settings.SoundInputRate = APU_DEFAULT_INPUT_RATE;

	double time_ratio = (double) Settings.SoundInputRate * spc::timing_hack_numerator / (Settings.SoundPlaybackRate * spc::timing_hack_denominator);
	spc::resampler->time_ratio(time_ratio);
}

bool8 S9xInitSound (int buffer_ms, int lag_ms)
{
	// buffer_ms : buffer size given in millisecond
	// lag_ms    : allowable time-lag given in millisecond

	int	sample_count     = buffer_ms * 32000 / 1000;
	int	lag_sample_count = lag_ms    * 32000 / 1000;

	spc::lag_master = lag_sample_count;
	if (Settings.Stereo)
		spc::lag_master <<= 1;
	spc::lag = spc::lag_master;

	if (sample_count < APU_MINIMUM_SAMPLE_COUNT)
		sample_count = APU_MINIMUM_SAMPLE_COUNT;

	spc::buffer_size = sample_count;
	if (Settings.Stereo)
		spc::buffer_size <<= 1;
	if (Settings.SixteenBitSound)
		spc::buffer_size <<= 1;

	printf("Sound buffer size: %d (%d samples)\n", spc::buffer_size, sample_count);

	if (spc::landing_buffer)
		delete[] spc::landing_buffer;
	spc::landing_buffer = new uint8[spc::buffer_size * 2];
	if (!spc::landing_buffer)
		return (FALSE);

	/* The resampler and spc unit use samples (16-bit short) as
	   arguments. Use 2x in the resampler for buffer leveling with SoundSync */
	if (!spc::resampler)
	{
		spc::resampler = new APU_DEFAULT_RESAMPLER(spc::buffer_size >> (Settings.SoundSync ? 0 : 1));
		if (!spc::resampler)
		{
			delete[] spc::landing_buffer;
			return (FALSE);
		}
	}
	else
		spc::resampler->resize(spc::buffer_size >> (Settings.SoundSync ? 0 : 1));

	spc_core->set_output((SNES_SPC::sample_t *) spc::landing_buffer, spc::buffer_size >> 1);

	UpdatePlaybackRate();

	spc::sound_enabled = S9xOpenSoundDevice();

	return (spc::sound_enabled);
}

void S9xSetSoundControl (uint8 voice_switch)
{
	spc_core->dsp_set_stereo_switch(voice_switch << 8 | voice_switch);
}

void S9xSetSoundMute (bool8 mute)
{
	Settings.Mute = mute;
	if (!spc::sound_enabled)
		Settings.Mute = TRUE;
}

void S9xDumpSPCSnapshot (void)
{
	spc_core->dsp_dump_spc_snapshot();
}

static void SPCSnapshotCallback (void)
{
	S9xSPCDump(S9xGetFilenameInc((".spc"), SPC_DIR));
	printf("Dumped key-on triggered spc snapshot.\n");
}

bool8 S9xInitAPU (void)
{
	spc_core = new SNES_SPC;
	if (!spc_core)
		return (FALSE);

	spc_core->init();
	spc_core->init_rom(APUROM);

	spc_core->dsp_set_spc_snapshot_callback(SPCSnapshotCallback);

	spc::landing_buffer = NULL;
	spc::shrink_buffer  = NULL;
	spc::resampler      = NULL;

	return (TRUE);
}

void S9xDeinitAPU (void)
{
	if (spc_core)
	{
		delete spc_core;
		spc_core = NULL;
	}

	if (spc::resampler)
	{
		delete spc::resampler;
		spc::resampler = NULL;
	}

	if (spc::landing_buffer)
	{
		delete[] spc::landing_buffer;
		spc::landing_buffer = NULL;
	}

	if (spc::shrink_buffer)
	{
		delete[] spc::shrink_buffer;
		spc::shrink_buffer = NULL;
	}
}

static inline int S9xAPUGetClock (int32 cpucycles)
{
	return (spc::ratio_numerator * (cpucycles - spc::reference_time) + spc::remainder) /
			spc::ratio_denominator;
}

static inline int S9xAPUGetClockRemainder (int32 cpucycles)
{
	return (spc::ratio_numerator * (cpucycles - spc::reference_time) + spc::remainder) %
			spc::ratio_denominator;
}

uint8 S9xAPUReadPort (int port)
{
	return ((uint8) spc_core->read_port(S9xAPUGetClock(CPU.Cycles), port));
}

void S9xAPUWritePort (int port, uint8 byte)
{
	spc_core->write_port(S9xAPUGetClock(CPU.Cycles), port, byte);
}

void S9xAPUSetReferenceTime (int32 cpucycles)
{
	spc::reference_time = cpucycles;
}

void S9xAPUExecute (void)
{
	/* Accumulate partial APU cycles */
	spc_core->end_frame(S9xAPUGetClock(CPU.Cycles));

	spc::remainder = S9xAPUGetClockRemainder(CPU.Cycles);

	S9xAPUSetReferenceTime(CPU.Cycles);
}

void S9xAPUEndScanline (void)
{
	S9xAPUExecute();

	if (spc_core->sample_count() >= APU_MINIMUM_SAMPLE_BLOCK || !spc::sound_in_sync)
		S9xLandSamples();
}

void S9xAPUTimingSetSpeedup (int ticks)
{
	if (ticks != 0)
		printf("APU speedup hack: %d\n", ticks);

	spc::timing_hack_denominator = SNES_SPC::tempo_unit - ticks;
	spc_core->set_tempo(spc::timing_hack_denominator);

	spc::ratio_numerator = Settings.PAL ? APU_NUMERATOR_PAL : APU_NUMERATOR_NTSC;
	spc::ratio_denominator = Settings.PAL ? APU_DENOMINATOR_PAL : APU_DENOMINATOR_NTSC;
	spc::ratio_denominator = spc::ratio_denominator * spc::timing_hack_denominator / spc::timing_hack_numerator;

	UpdatePlaybackRate();
}

void S9xAPUAllowTimeOverflow (bool allow)
{
	if (allow)
		printf("APU time overflow allowed\n");

	spc_core->spc_allow_time_overflow(allow);
}

void S9xResetAPU (void)
{
	spc::reference_time = 0;
	spc::remainder = 0;
	spc_core->reset();
	spc_core->set_output((SNES_SPC::sample_t *) spc::landing_buffer, spc::buffer_size >> 1);

	spc::resampler->clear();
}

void S9xSoftResetAPU (void)
{
	spc::reference_time = 0;
	spc::remainder = 0;
	spc_core->soft_reset();
	spc_core->set_output((SNES_SPC::sample_t *) spc::landing_buffer, spc::buffer_size >> 1);

	spc::resampler->clear();
}

static void from_apu_to_state (uint8 **buf, void *var, size_t size)
{
	memcpy(*buf, var, size);
	*buf += size;
}

static void to_apu_from_state (uint8 **buf, void *var, size_t size)
{
	memcpy(var, *buf, size);
	*buf += size;
}

void S9xAPUSaveState (uint8 *block)
{
	uint8	*ptr = block;

	spc_core->copy_state(&ptr, from_apu_to_state);

	SET_LE32(ptr, spc::reference_time);
	ptr += sizeof(int32);
	SET_LE32(ptr, spc::remainder);
}

void S9xAPULoadState (uint8 *block)
{
	uint8	*ptr = block;

	S9xResetAPU();

	spc_core->copy_state(&ptr, to_apu_from_state);

	spc::reference_time = GET_LE32(ptr);
	ptr += sizeof(int32);
	spc::remainder = GET_LE32(ptr);
}
