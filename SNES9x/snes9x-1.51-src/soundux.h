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




#ifndef _SOUND_H_
#define _SOUND_H_

enum
{
	SOUND_SAMPLE = 0,
	SOUND_NOISE
};

enum
{
	SOUND_SILENT,
	SOUND_ATTACK,
	SOUND_DECAY,
	SOUND_SUSTAIN,
	SOUND_RELEASE,
	SOUND_GAIN,
	SOUND_INCREASE_LINEAR,
	SOUND_INCREASE_BENT_LINE,
	SOUND_DECREASE_LINEAR,
	SOUND_DECREASE_EXPONENTIAL
};

enum
{
	MODE_NONE = SOUND_SILENT,
	MODE_ADSR,
	MODE_RELEASE = SOUND_RELEASE,
	MODE_GAIN,
	MODE_INCREASE_LINEAR,
	MODE_INCREASE_BENT_LINE,
	MODE_DECREASE_LINEAR,
	MODE_DECREASE_EXPONENTIAL
};

#define NUM_CHANNELS			8
#define SOUND_DECODE_LENGTH		16
#define SOUND_BUFFER_SIZE		(1024 * 16)
#define MAX_BUFFER_SIZE			SOUND_BUFFER_SIZE
#define SOUND_BUFFER_SIZE_MASK	(SOUND_BUFFER_SIZE - 1)

#define ENV_RANGE	0x800
#define ENV_MAX		0x7FF
#define ENV_SHIFT	4

#ifdef __sgi
#include <audio.h>
#endif /* __sgi */

typedef struct {
	int				sound_fd;				// ** port specific
	int				sound_switch;			// channel on/off
	int				playback_rate;			// 32000Hz is recommended
	int				buffer_size;			// ** port specific
	int				noise_gen;				// ** unused
	bool8			mute_sound;				// mute
	int				stereo;					// stereo or mono
	bool8			sixteen_bit;			// 16bit or 8bit sample
	bool8			encoded;				// ** port specific
#ifdef __sun
	int				last_eof;				// ** port specific
#endif
#ifdef __sgi
	ALport			al_port;				// ** port specific
#endif /* __sgi */
	int32			samples_mixed_so_far;	// ** port specific
	int32			play_position;			// ** port specific
	uint32			err_counter;			// ** port specific
	uint32			err_rate;				// ** port specific

	uint16			stereo_switch;			// stereo channel on/off
	double			pitch_mul;				// used with Settings.FixFrequency
} SoundStatus;

EXTERN_C volatile SoundStatus so;

typedef struct {
    int32			state;					// ADSR/GAIN/RELEASE/SILENT
    int32			type;					// sample or noise
    short			volume_left;			// VOL(L)
    short			volume_right;			// VOL(R)
    uint32			hertz;					// ((P(H) << 8) + P(L)) * 8
    uint32			frequency;				// normalized pitch
    uint32			count;					// ** unused
    bool8			loop;					// loop flag in BRR header
    int32			envx;					// ** unused
    short			left_vol_level;			// ** unused
    short			right_vol_level;		// ** unused
    short			envx_target;			// ** unused
    uint32			env_error;				// ** unused
    uint32			erate;					// ** unused
    int32 			direction;				// ** unused
    uint32			attack_rate;			// ** unused
    uint32			decay_rate;				// ** unused
    uint32			sustain_rate;			// ** unused
    uint32			release_rate;			// ** unused
    uint32			sustain_level;			// ** unused
    signed short	sample;					// signed 16 bit sample
    signed short	decoded[16];			// decoded 16 samples
    signed short	previous16[2];
    signed short	*block;
    uint16			sample_number;			// SRCN
    bool8			last_block;				// end flag in BRR header
    bool8			needs_decode;			// true when BRR block will be decoded
    uint32			block_pointer;			// currect block
    uint32			sample_pointer;			// pointer in a block
    int32			*echo_buf_ptr;			// EchoBuffer[] or DummyEchoBuffer[]
    int32			mode;					// ADSR/GAIN/RELEASE/SILENT
    int32			envxx;					// ** unused
    signed short	next_sample;			// ** unused
    int32			interpolate;			// ** unused
    int32			previous[2];			// last two nybbles for BRR decode
    uint32			dummy[8];				// Just incase they are needed in the future,
    										// for snapshot compatibility.

	int32			nb_index;				// index of cached samples
	int16			nb_sample[4];			// cached samples for interpolation
	int16			out_sample;				// OUTX << 4
	int32			xenvx;					// ENVX << 4
	int32			xenvx_target;			// ENVX target << 4
	int32			xenv_count;				// counter for envelope timing
	int32			xenv_rate;				// envelope timing from env_counter_table
	int32			xsmp_count;				// counter for pitch
	int32			xattack_rate;			// envelope timing from env_counter_table
	int32			xdecay_rate;			// envelope timing from env_counter_table
	int32			xsustain_rate;			// envelope timing from env_counter_table
	int32			xsustain_level;			// (128 / 8 * (SR + 1)) << 4
} Channel;

typedef struct
{
    short			master_volume_left;		// MVOL(L)
    short			master_volume_right;	// MVOL(R)
    short			echo_volume_left;		// EVOL(L)
    short			echo_volume_right;		// EVOL(R)
    int32			echo_enable;			// EON
    int32			echo_feedback;			// EFB
    int32			echo_ptr;				// index of Echo[]
    int32			echo_buffer_size;		// num of echo samples
    int32			echo_write_enabled;		// ECEN
    int32			echo_channel_enable;	// ** unused
    int32			pitch_mod;				// PMOD
    uint32			dummy[3];				// Just incase they are needed in the future,
											// for snapshot compatibility.
    Channel			channels[NUM_CHANNELS];
    bool8			no_filter;				// true when simple echo
    int32			master_volume[2];
    int32			echo_volume[2];
    int32			noise_hertz;			// ** unused

	int32			noise_count;			// counter for noise frequency
	int32			noise_rate;				// noise frequency from env_counter_table
} SSoundData;

EXTERN_C SSoundData SoundData;


void S9xSetEnvelopeRate (int channel, int32 rate_count, int32 xtarget);
void S9xSetSoundVolume (int channel, short volume_left, short volume_right);
void S9xSetMasterVolume (short volume_left, short volume_right);
void S9xSetEchoVolume (short volume_left, short volume_right);
void S9xSetEchoEnable (uint8 byte);
void S9xSetEchoFeedback (int echo_feedback);
void S9xSetEchoDelay (unsigned int byte);
void S9xSetEchoWriteEnable (uint8 byte);
void S9xSetFrequencyModulationEnable (uint8 byte);
void S9xSetSoundKeyOff (int channel);
void S9xPrepareSoundForSnapshotSave (bool8 restore);
void S9xFixSoundAfterSnapshotLoad (int version);
void S9xSetFilterCoefficient (int tap, int value);
void S9xSetSoundADSR (int channel, int ar, int dr, int sr, int sl);
void S9xSetEnvelopeHeight (int channel, int32 xlevel);
uint8 S9xGetEnvelopeHeight (int channel);
void S9xSetSoundHertz (int channel, int hertz);
void S9xSetSoundType (int channel, int type_of_sound);
bool8 S9xSetSoundMute (bool8 mute);
void S9xResetSound (bool8 full);
void S9xSetPlaybackRate (uint32 playback_rate);
bool8 S9xSetSoundMode (int channel, int mode);
void S9xSetSoundControl (int sound_switch);
void S9xPlaySample (int channel);

void S9xFixEnvelope (int channel, uint8 gain, uint8 adsr1, uint8 adsr2);
bool8 S9xOpenSoundDevice (int mode, bool8 stereo, int buffer_size);

EXTERN_C void S9xMixSamples (uint8 *buffer, int sample_count);
EXTERN_C void S9xMixSamplesO (uint8 *buffer, int sample_count, int byte_offset);

#endif
