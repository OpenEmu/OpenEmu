/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2004 Ki
 *  Copyright (C) 2007 Mednafen Team
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

#include "pce.h"
#include "psg.h"
#include "cdrom.h"
#include "adpcm.h"

#define ADPCM_MAXVOLUME			1024

typedef Blip_Synth<blip_good_quality, 16384> ADSynth;
static ADSynth ADPCMSynth;

static uint8	*RAM = NULL; //0x10000;
static uint16	Addr;
static uint32   ReadAddr;
static uint16	WriteAddr;
static uint32	LengthCount, CacheLengthCount;
static uint32   HalfCount;

static uint8 LastCmd;
static uint32 SampleFreq;

static bool ADPCMLP;

static uint8 ReadBuffer;
static int32 ReadPending;
static int32 WritePending;
static uint8 WritePendingValue;

/* volume, fadein/fadeout */
static int32	CurrentVolume = ADPCM_MAXVOLUME;
static int32	InitialVolume = ADPCM_MAXVOLUME;
static int32	VolumeStep;
static bool	bFadeIn  = FALSE;
static bool	bFadeOut = FALSE;
static int32	ClockCount;
static int32	FadeCycle;

static int32 bigdivacc;
static int32 bigdiv;
static int32 smalldiv;
static int32 last_pcm;
extern Blip_Buffer sbuf[2];

static void (*_pfnNotification)(uint32 adpcmState);

/*
	The original decoder is provided by Dave Shadoff,
	and further optimized by Ki.
*/
static uint32 ad_sample;
static int32		ad_ref_index;

static const uint32	_LUT[49*16] =
{
	0x0002ffff, 0x0006ffff, 0x000affff, 0x000effff, 0x00120002, 0x00160004, 0x001a0006, 0x001e0008,
	0xfffeffff, 0xfffaffff, 0xfff6ffff, 0xfff2ffff, 0xffee0002, 0xffea0004, 0xffe60006, 0xffe20008,
	0x0002ffff, 0x0006ffff, 0x000affff, 0x000effff, 0x00130002, 0x00170004, 0x001b0006, 0x001f0008,
	0xfffeffff, 0xfffaffff, 0xfff6ffff, 0xfff2ffff, 0xffed0002, 0xffe90004, 0xffe50006, 0xffe10008,
	0x0002ffff, 0x0006ffff, 0x000bffff, 0x000fffff, 0x00150002, 0x00190004, 0x001e0006, 0x00220008,
	0xfffeffff, 0xfffaffff, 0xfff5ffff, 0xfff1ffff, 0xffeb0002, 0xffe70004, 0xffe20006, 0xffde0008,
	0x0002ffff, 0x0007ffff, 0x000cffff, 0x0011ffff, 0x00170002, 0x001c0004, 0x00210006, 0x00260008,
	0xfffeffff, 0xfff9ffff, 0xfff4ffff, 0xffefffff, 0xffe90002, 0xffe40004, 0xffdf0006, 0xffda0008,
	0x0002ffff, 0x0007ffff, 0x000dffff, 0x0012ffff, 0x00190002, 0x001e0004, 0x00240006, 0x00290008,
	0xfffeffff, 0xfff9ffff, 0xfff3ffff, 0xffeeffff, 0xffe70002, 0xffe20004, 0xffdc0006, 0xffd70008,
	0x0003ffff, 0x0009ffff, 0x000fffff, 0x0015ffff, 0x001c0002, 0x00220004, 0x00280006, 0x002e0008,
	0xfffdffff, 0xfff7ffff, 0xfff1ffff, 0xffebffff, 0xffe40002, 0xffde0004, 0xffd80006, 0xffd20008,
	0x0003ffff, 0x000affff, 0x0011ffff, 0x0018ffff, 0x001f0002, 0x00260004, 0x002d0006, 0x00340008,
	0xfffdffff, 0xfff6ffff, 0xffefffff, 0xffe8ffff, 0xffe10002, 0xffda0004, 0xffd30006, 0xffcc0008,
	0x0003ffff, 0x000affff, 0x0012ffff, 0x0019ffff, 0x00220002, 0x00290004, 0x00310006, 0x00380008,
	0xfffdffff, 0xfff6ffff, 0xffeeffff, 0xffe7ffff, 0xffde0002, 0xffd70004, 0xffcf0006, 0xffc80008,
	0x0004ffff, 0x000cffff, 0x0015ffff, 0x001dffff, 0x00260002, 0x002e0004, 0x00370006, 0x003f0008,
	0xfffcffff, 0xfff4ffff, 0xffebffff, 0xffe3ffff, 0xffda0002, 0xffd20004, 0xffc90006, 0xffc10008,
	0x0004ffff, 0x000dffff, 0x0016ffff, 0x001fffff, 0x00290002, 0x00320004, 0x003b0006, 0x00440008,
	0xfffcffff, 0xfff3ffff, 0xffeaffff, 0xffe1ffff, 0xffd70002, 0xffce0004, 0xffc50006, 0xffbc0008,
	0x0005ffff, 0x000fffff, 0x0019ffff, 0x0023ffff, 0x002e0002, 0x00380004, 0x00420006, 0x004c0008,
	0xfffbffff, 0xfff1ffff, 0xffe7ffff, 0xffddffff, 0xffd20002, 0xffc80004, 0xffbe0006, 0xffb40008,
	0x0005ffff, 0x0010ffff, 0x001bffff, 0x0026ffff, 0x00320002, 0x003d0004, 0x00480006, 0x00530008,
	0xfffbffff, 0xfff0ffff, 0xffe5ffff, 0xffdaffff, 0xffce0002, 0xffc30004, 0xffb80006, 0xffad0008,
	0x0006ffff, 0x0012ffff, 0x001fffff, 0x002bffff, 0x00380002, 0x00440004, 0x00510006, 0x005d0008,
	0xfffaffff, 0xffeeffff, 0xffe1ffff, 0xffd5ffff, 0xffc80002, 0xffbc0004, 0xffaf0006, 0xffa30008,
	0x0006ffff, 0x0013ffff, 0x0021ffff, 0x002effff, 0x003d0002, 0x004a0004, 0x00580006, 0x00650008,
	0xfffaffff, 0xffedffff, 0xffdfffff, 0xffd2ffff, 0xffc30002, 0xffb60004, 0xffa80006, 0xff9b0008,
	0x0007ffff, 0x0016ffff, 0x0025ffff, 0x0034ffff, 0x00430002, 0x00520004, 0x00610006, 0x00700008,
	0xfff9ffff, 0xffeaffff, 0xffdbffff, 0xffccffff, 0xffbd0002, 0xffae0004, 0xff9f0006, 0xff900008,
	0x0008ffff, 0x0018ffff, 0x0029ffff, 0x0039ffff, 0x004a0002, 0x005a0004, 0x006b0006, 0x007b0008,
	0xfff8ffff, 0xffe8ffff, 0xffd7ffff, 0xffc7ffff, 0xffb60002, 0xffa60004, 0xff950006, 0xff850008,
	0x0009ffff, 0x001bffff, 0x002dffff, 0x003fffff, 0x00520002, 0x00640004, 0x00760006, 0x00880008,
	0xfff7ffff, 0xffe5ffff, 0xffd3ffff, 0xffc1ffff, 0xffae0002, 0xff9c0004, 0xff8a0006, 0xff780008,
	0x000affff, 0x001effff, 0x0032ffff, 0x0046ffff, 0x005a0002, 0x006e0004, 0x00820006, 0x00960008,
	0xfff6ffff, 0xffe2ffff, 0xffceffff, 0xffbaffff, 0xffa60002, 0xff920004, 0xff7e0006, 0xff6a0008,
	0x000bffff, 0x0021ffff, 0x0037ffff, 0x004dffff, 0x00630002, 0x00790004, 0x008f0006, 0x00a50008,
	0xfff5ffff, 0xffdfffff, 0xffc9ffff, 0xffb3ffff, 0xff9d0002, 0xff870004, 0xff710006, 0xff5b0008,
	0x000cffff, 0x0024ffff, 0x003cffff, 0x0054ffff, 0x006d0002, 0x00850004, 0x009d0006, 0x00b50008,
	0xfff4ffff, 0xffdcffff, 0xffc4ffff, 0xffacffff, 0xff930002, 0xff7b0004, 0xff630006, 0xff4b0008,
	0x000dffff, 0x0027ffff, 0x0042ffff, 0x005cffff, 0x00780002, 0x00920004, 0x00ad0006, 0x00c70008,
	0xfff3ffff, 0xffd9ffff, 0xffbeffff, 0xffa4ffff, 0xff880002, 0xff6e0004, 0xff530006, 0xff390008,
	0x000effff, 0x002bffff, 0x0049ffff, 0x0066ffff, 0x00840002, 0x00a10004, 0x00bf0006, 0x00dc0008,
	0xfff2ffff, 0xffd5ffff, 0xffb7ffff, 0xff9affff, 0xff7c0002, 0xff5f0004, 0xff410006, 0xff240008,
	0x0010ffff, 0x0030ffff, 0x0051ffff, 0x0071ffff, 0x00920002, 0x00b20004, 0x00d30006, 0x00f30008,
	0xfff0ffff, 0xffd0ffff, 0xffafffff, 0xff8fffff, 0xff6e0002, 0xff4e0004, 0xff2d0006, 0xff0d0008,
	0x0011ffff, 0x0034ffff, 0x0058ffff, 0x007bffff, 0x00a00002, 0x00c30004, 0x00e70006, 0x010a0008,
	0xffefffff, 0xffccffff, 0xffa8ffff, 0xff85ffff, 0xff600002, 0xff3d0004, 0xff190006, 0xfef60008,
	0x0013ffff, 0x003affff, 0x0061ffff, 0x0088ffff, 0x00b00002, 0x00d70004, 0x00fe0006, 0x01250008,
	0xffedffff, 0xffc6ffff, 0xff9fffff, 0xff78ffff, 0xff500002, 0xff290004, 0xff020006, 0xfedb0008,
	0x0015ffff, 0x0040ffff, 0x006bffff, 0x0096ffff, 0x00c20002, 0x00ed0004, 0x01180006, 0x01430008,
	0xffebffff, 0xffc0ffff, 0xff95ffff, 0xff6affff, 0xff3e0002, 0xff130004, 0xfee80006, 0xfebd0008,
	0x0017ffff, 0x0046ffff, 0x0076ffff, 0x00a5ffff, 0x00d50002, 0x01040004, 0x01340006, 0x01630008,
	0xffe9ffff, 0xffbaffff, 0xff8affff, 0xff5bffff, 0xff2b0002, 0xfefc0004, 0xfecc0006, 0xfe9d0008,
	0x001affff, 0x004effff, 0x0082ffff, 0x00b6ffff, 0x00eb0002, 0x011f0004, 0x01530006, 0x01870008,
	0xffe6ffff, 0xffb2ffff, 0xff7effff, 0xff4affff, 0xff150002, 0xfee10004, 0xfead0006, 0xfe790008,
	0x001cffff, 0x0055ffff, 0x008fffff, 0x00c8ffff, 0x01020002, 0x013b0004, 0x01750006, 0x01ae0008,
	0xffe4ffff, 0xffabffff, 0xff71ffff, 0xff38ffff, 0xfefe0002, 0xfec50004, 0xfe8b0006, 0xfe520008,
	0x001fffff, 0x005effff, 0x009dffff, 0x00dcffff, 0x011c0002, 0x015b0004, 0x019a0006, 0x01d90008,
	0xffe1ffff, 0xffa2ffff, 0xff63ffff, 0xff24ffff, 0xfee40002, 0xfea50004, 0xfe660006, 0xfe270008,
	0x0022ffff, 0x0067ffff, 0x00adffff, 0x00f2ffff, 0x01390002, 0x017e0004, 0x01c40006, 0x02090008,
	0xffdeffff, 0xff99ffff, 0xff53ffff, 0xff0effff, 0xfec70002, 0xfe820004, 0xfe3c0006, 0xfdf70008,
	0x0026ffff, 0x0072ffff, 0x00bfffff, 0x010bffff, 0x01590002, 0x01a50004, 0x01f20006, 0x023e0008,
	0xffdaffff, 0xff8effff, 0xff41ffff, 0xfef5ffff, 0xfea70002, 0xfe5b0004, 0xfe0e0006, 0xfdc20008,
	0x002affff, 0x007effff, 0x00d2ffff, 0x0126ffff, 0x017b0002, 0x01cf0004, 0x02230006, 0x02770008,
	0xffd6ffff, 0xff82ffff, 0xff2effff, 0xfedaffff, 0xfe850002, 0xfe310004, 0xfddd0006, 0xfd890008,
	0x002effff, 0x008affff, 0x00e7ffff, 0x0143ffff, 0x01a10002, 0x01fd0004, 0x025a0006, 0x02b60008,
	0xffd2ffff, 0xff76ffff, 0xff19ffff, 0xfebdffff, 0xfe5f0002, 0xfe030004, 0xfda60006, 0xfd4a0008,
	0x0033ffff, 0x0099ffff, 0x00ffffff, 0x0165ffff, 0x01cb0002, 0x02310004, 0x02970006, 0x02fd0008,
	0xffcdffff, 0xff67ffff, 0xff01ffff, 0xfe9bffff, 0xfe350002, 0xfdcf0004, 0xfd690006, 0xfd030008,
	0x0038ffff, 0x00a8ffff, 0x0118ffff, 0x0188ffff, 0x01f90002, 0x02690004, 0x02d90006, 0x03490008,
	0xffc8ffff, 0xff58ffff, 0xfee8ffff, 0xfe78ffff, 0xfe070002, 0xfd970004, 0xfd270006, 0xfcb70008,
	0x003dffff, 0x00b8ffff, 0x0134ffff, 0x01afffff, 0x022b0002, 0x02a60004, 0x03220006, 0x039d0008,
	0xffc3ffff, 0xff48ffff, 0xfeccffff, 0xfe51ffff, 0xfdd50002, 0xfd5a0004, 0xfcde0006, 0xfc630008,
	0x0044ffff, 0x00ccffff, 0x0154ffff, 0x01dcffff, 0x02640002, 0x02ec0004, 0x03740006, 0x03fc0008,
	0xffbcffff, 0xff34ffff, 0xfeacffff, 0xfe24ffff, 0xfd9c0002, 0xfd140004, 0xfc8c0006, 0xfc040008,
	0x004affff, 0x00dfffff, 0x0175ffff, 0x020affff, 0x02a00002, 0x03350004, 0x03cb0006, 0x04600008,
	0xffb6ffff, 0xff21ffff, 0xfe8bffff, 0xfdf6ffff, 0xfd600002, 0xfccb0004, 0xfc350006, 0xfba00008,
	0x0052ffff, 0x00f6ffff, 0x019bffff, 0x023fffff, 0x02e40002, 0x03880004, 0x042d0006, 0x04d10008,
	0xffaeffff, 0xff0affff, 0xfe65ffff, 0xfdc1ffff, 0xfd1c0002, 0xfc780004, 0xfbd30006, 0xfb2f0008,
	0x005affff, 0x010fffff, 0x01c4ffff, 0x0279ffff, 0x032e0002, 0x03e30004, 0x04980006, 0x054d0008,
	0xffa6ffff, 0xfef1ffff, 0xfe3cffff, 0xfd87ffff, 0xfcd20002, 0xfc1d0004, 0xfb680006, 0xfab30008,
	0x0063ffff, 0x012affff, 0x01f1ffff, 0x02b8ffff, 0x037f0002, 0x04460004, 0x050d0006, 0x05d40008,
	0xff9dffff, 0xfed6ffff, 0xfe0fffff, 0xfd48ffff, 0xfc810002, 0xfbba0004, 0xfaf30006, 0xfa2c0008,
	0x006dffff, 0x0148ffff, 0x0223ffff, 0x02feffff, 0x03d90002, 0x04b40004, 0x058f0006, 0x066a0008,
	0xff93ffff, 0xfeb8ffff, 0xfdddffff, 0xfd02ffff, 0xfc270002, 0xfb4c0004, 0xfa710006, 0xf9960008,
	0x0078ffff, 0x0168ffff, 0x0259ffff, 0x0349ffff, 0x043b0002, 0x052b0004, 0x061c0006, 0x070c0008,
	0xff88ffff, 0xfe98ffff, 0xfda7ffff, 0xfcb7ffff, 0xfbc50002, 0xfad50004, 0xf9e40006, 0xf8f40008,
	0x0084ffff, 0x018dffff, 0x0296ffff, 0x039fffff, 0x04a80002, 0x05b10004, 0x06ba0006, 0x07c30008,
	0xff7cffff, 0xfe73ffff, 0xfd6affff, 0xfc61ffff, 0xfb580002, 0xfa4f0004, 0xf9460006, 0xf83d0008,
	0x0091ffff, 0x01b4ffff, 0x02d8ffff, 0x03fbffff, 0x051f0002, 0x06420004, 0x07660006, 0x08890008,
	0xff6fffff, 0xfe4cffff, 0xfd28ffff, 0xfc05ffff, 0xfae10002, 0xf9be0004, 0xf89a0006, 0xf7770008,
	0x00a0ffff, 0x01e0ffff, 0x0321ffff, 0x0461ffff, 0x05a20002, 0x06e20004, 0x08230006, 0x09630008,
	0xff60ffff, 0xfe20ffff, 0xfcdfffff, 0xfb9fffff, 0xfa5e0002, 0xf91e0004, 0xf7dd0006, 0xf69d0008,
	0x00b0ffff, 0x0210ffff, 0x0371ffff, 0x04d1ffff, 0x06330002, 0x07930004, 0x08f40006, 0x0a540008,
	0xff50ffff, 0xfdf0ffff, 0xfc8fffff, 0xfb2fffff, 0xf9cd0002, 0xf86d0004, 0xf70c0006, 0xf5ac0008,
	0x00c2ffff, 0x0246ffff, 0x03caffff, 0x054effff, 0x06d20002, 0x08560004, 0x09da0006, 0x0b5e0008,
	0xff3effff, 0xfdbaffff, 0xfc36ffff, 0xfab2ffff, 0xf92e0002, 0xf7aa0004, 0xf6260006, 0xf4a20008
};


static ALWAYS_INLINE int32 saturated_addition(int32 val, int32 add, int32 min, int32 max)
{
	val += add;

	if (val < min)	val = min;
	if (val > max)	val = max;

	return val;
}

static ALWAYS_INLINE int32 decode(uint8 code)
{
	uint32 data = _LUT[ad_ref_index * 16 + code];

	ad_sample += ((int32)data >> 16);
	ad_sample &= 4095;
	ad_ref_index = saturated_addition(ad_ref_index, (int16)(data & 0xffff), 0, 48);
	return ad_sample - 2048;
}

#ifdef WANT_DEBUGGER
uint32 ADPCM_GetRegister(const std::string &name, std::string *special)
{
 uint32 value = 0xDEADBEEF;

 if(name == "ADFREQ")
  value = SampleFreq;
 else if(name == "ADCUR")
  value = ad_sample;
 else if(name == "ADWrAddr")
  value = WriteAddr;
 else if(name == "ADWrNibble")
  value = 0; // fixme
 else if(name == "ADRdAddr")
  value = ReadAddr >> 1;
 else if(name == "ADRdNibble")
  value = ReadAddr & 1;

 return(value);
}

void ADPCM_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 while(Length--)
 {
  Address &= 0xFFFF;
  *Buffer = RAM[Address];
  Address++;
  Buffer++;
 }
}

void ADPCM_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 while(Length--)
 {
  Address &= 0xFFFF;
  RAM[Address] = *Buffer;
  Address++;
  Buffer++;
 }
}
#endif

static void RedoLPF(int f)
{
         if(ADPCMLP)
         {
          if(f >= 14)
          {
           int rolloff = (int)((((double)32087.5 / (16 - f)) / 2) * 0.70);
           ADPCMSynth.treble_eq( blip_eq_t::blip_eq_t(-1000, rolloff, FSettings.SndRate));
          }
          else
          {
           int rolloff = (int)((((double)32087.5 / (16 - f)) / 2) * 0.80);
           ADPCMSynth.treble_eq( blip_eq_t::blip_eq_t(-1000, rolloff, FSettings.SndRate));
          }
         }
}


bool ADPCM_Init()
{
	uint32 ADPCMVolumeSetting;

	if(!(RAM = (uint8 *)MDFN_malloc(0x10000, _("PCE ADPCM RAM"))))
	{
	 return(0);
	}

	ADPCMVolumeSetting = MDFN_GetSettingUI("pce.adpcmvolume");
	if(ADPCMVolumeSetting != 100)
	{
	 MDFN_printf("ADPCM Volume: %d%%\n", ADPCMVolumeSetting);
	}

	ADPCMSynth.volume(0.42735f * ADPCMVolumeSetting / 100);
	ADPCMLP = MDFN_GetSettingB("pce.adpcmlp");
	bigdivacc = (int32)((double)1789772.727272 * 4 * 65536 / 32087.5 * pce_overclocked);

	#ifdef WANT_DEBUGGER
	MDFNDBG_AddASpace(ADPCM_GetAddressSpaceBytes, ADPCM_PutAddressSpaceBytes, "adpcm", "ADPCM RAM", 16);
	#endif

	return TRUE;
}


void ADPCM_Close()
{
	if(RAM)
	{
	 MDFN_free(RAM);
	 RAM = NULL;
	}
}


void ADPCM_Reset()
{
	_pfnNotification(ADPCM_STATE_NORMAL);

	Addr			= 0;
	ReadAddr		= 0;
	WriteAddr		= 0;
	LengthCount	= 0;
	LastCmd		= 0;

	// for adpcm play
	CurrentVolume = InitialVolume;
	VolumeStep = InitialVolume / 10;
	bFadeIn  = FALSE;
	bFadeOut = FALSE;
	ClockCount = 0;
	FadeCycle = 0;

	ad_sample = 2048;
	ad_ref_index = 0;
	bigdiv = 0;
	smalldiv = 0;

	RedoLPF(SampleFreq);
}

void ADPCM_Power(void)
{
 memset(RAM, 0x00, 65536);

 ReadPending = WritePending = 0;
 ReadBuffer = 0;

 CurrentVolume = InitialVolume;
 VolumeStep = InitialVolume / 10;
 bFadeIn  = FALSE;
 bFadeOut = FALSE;
 ClockCount = 0;
 FadeCycle = 0;
 LastCmd = 0;
 SampleFreq = 0;

 ad_sample = 2048;
 ad_ref_index = 0;

 ADPCM_Reset();
}

/*-----------------------------------------------------------------------------
	[SetNotificationFunction]
		
-----------------------------------------------------------------------------*/
void
ADPCM_SetNotificationFunction(void	(*pfnNotification)(uint32))
{
	_pfnNotification = pfnNotification;
}


/*-----------------------------------------------------------------------------
	[SetAddrLo]
		
-----------------------------------------------------------------------------*/
void ADPCM_SetAddrLo(uint8		addrLo)
{
	Addr &= 0xff00;
	Addr |= addrLo;
}


/*-----------------------------------------------------------------------------
	[SetAddrHi]
		
-----------------------------------------------------------------------------*/
void ADPCM_SetAddrHi(uint8 addrHi)
{
	Addr &= 0xff;
	Addr |= addrHi << 8;
}


uint8 ADPCM_ReadBuffer()
{
	if(!PCE_InDebug)
	 ReadPending = 24;

        //if((LastCmd & 0x20) && LengthCount) puts("Ooppsssiieee");

        return ReadBuffer;
}

void ADPCM_WriteBuffer(uint8            data)
{
        WritePending = (int32)(76 + -3.428571428571428354f * (SampleFreq + 1)); // Icky kludge for Sherlock Holmes and Record of Lodoss War
        WritePendingValue = data;
}

uint8 ADPCM_Read180D(void)
{
	return(LastCmd);
}

void ADPCM_Write180D(uint8 data)
{
	//printf("Wr180D: %02x\n", data);
	if ((LastCmd & 0x80) && !(data & 0x80))
        {        
	 ADPCM_Reset();
	}                        

        if(data & 0x20)
        {
         _pfnNotification(ADPCM_STATE_NORMAL);
         if(data & 0x40)
         {
          HalfCount = CacheLengthCount >> 1;
         }
         else
         {
          HalfCount = CacheLengthCount;
         }
        }
        else
        {
         _pfnNotification(ADPCM_STATE_NORMAL);
        }

        if((data & 0x40) && !(LastCmd & 0x40))
        {
         ad_sample = 2048;
         ad_ref_index = 0;
        }

	if(data & 0x10)
	{
	 CacheLengthCount = LengthCount = Addr << 1;
	}

        // D2 and D3 control read address
        if(!(LastCmd & 0x8) && (data & 0x08))
        {
         if(data & 0x4)
          ReadAddr = Addr << 1;
         else
          ReadAddr = ((Addr - 1) << 1) & 0x1FFFF;
        }

        // D0 and D1 control write address
        if(!(LastCmd & 0x2) && (data & 0x2))
        {
         WriteAddr = Addr;
         if(!(data & 0x1))
          WriteAddr = (WriteAddr - 1) & 0xFFFF;
        }
	LastCmd = data;
}


bool ADPCM_IsWritePending(void)
{
 return(WritePending > 0);
}

bool ADPCM_IsBusyReading(void)
{
 return(ReadPending > 0);
}

static ALWAYS_INLINE void ADPCM_PB_Run(int32 basetime, int32 run_time)
{
 bigdiv -= run_time * 65536;

 while(bigdiv <= 0)
 {
  bigdiv += bigdivacc;

  if((LastCmd & 0x20) && LengthCount)
  {
   smalldiv --;
   while(smalldiv <= 0)
   {
    int32 pcm;

    smalldiv += (16 - SampleFreq);
    ReadAddr = (ReadAddr + 1) & 0x1FFFF;
    pcm = decode((RAM[ReadAddr >> 1] >> (((ReadAddr ^ 1) & 1) * 4)) & 0x0F);

    LengthCount--;
    if(LengthCount < HalfCount)
    {
     _pfnNotification(ADPCM_STATE_HALF_PLAYED);
    }

    if(!LengthCount)
    {
     LastCmd &= ~0x60;
     _pfnNotification(ADPCM_STATE_FULL_PLAYED);
     break;
    }
    pcm = (pcm * CurrentVolume) >> 8;
    uint32 synthtime = ((basetime + (bigdiv >> 16)));
    if(pce_overclocked > 1) 
     synthtime /= pce_overclocked;

    if(FSettings.SndRate)
    {
     ADPCMSynth.offset(synthtime, pcm - last_pcm, &sbuf[0]);
     ADPCMSynth.offset(synthtime, pcm - last_pcm, &sbuf[1]);
    }
    last_pcm = pcm;
   }
  }
 }
}

static ALWAYS_INLINE void ADPCM_AdvanceClock(int32  cycles)
{
        if (bFadeOut || bFadeIn)
        {
                ClockCount += cycles;

                while (ClockCount >= FadeCycle * pce_overclocked)
                {
                        ClockCount -= FadeCycle * pce_overclocked;

                        if (bFadeOut)
                        {
                                if (CurrentVolume > 0)
                                {
                                        CurrentVolume -= VolumeStep;
                                        if (CurrentVolume < 0)
                                        {
                                                CurrentVolume = 0;
                                                bFadeOut = FALSE;
                                                break;
                                        }
                                }
                        }
                        else if (bFadeIn)
                        {
                                if (CurrentVolume < InitialVolume)
                                {
                                        CurrentVolume += VolumeStep;
                                        if (CurrentVolume > InitialVolume)
                                        {
                                                CurrentVolume = InitialVolume;
                                                bFadeIn = FALSE;
                                                break;
                                        }
                                }
                        }
                }
        }
}


void ADPCM_Run(int32 clocks)
{
 ADPCM_AdvanceClock(clocks);
 ADPCM_PB_Run(HuCPU.timestamp, clocks);

 if(WritePending)
 {
  WritePending -= clocks;
  if(WritePending <= 0)
  {
   LengthCount += 2;

   if(LengthCount >= HalfCount)
   {
    _pfnNotification(ADPCM_STATE_NORMAL);
   }
   RAM[WriteAddr++] = WritePendingValue;
   WritePending = 0;
  }
 }

 if(!WritePending)
  WritePending = PCECD_DoADFun(&WritePendingValue) ? 10 : 0;

 if(ReadPending)
 {
  ReadPending -= clocks;
  if(ReadPending <= 0)
  {
   uint8 nib1, nib2;
   nib1 = (RAM[(ReadAddr >> 1)] >> (((ReadAddr ^ 1) & 1) * 4)) & 0x0F;
   ReadAddr = (ReadAddr + 1) & 0x1FFFF;
   nib2 = (RAM[(ReadAddr >> 1)] >> (((ReadAddr ^ 1) & 1) * 4)) & 0x0F;

   ReadAddr = (ReadAddr + 1) & 0x1FFFF;
   ReadBuffer = (nib1 << 4) | nib2;
   ReadPending = 0;
  }
 }
}

void ADPCM_SetFreq(uint32 freq)
{
	if(freq != SampleFreq)
	 RedoLPF(freq);

	SampleFreq = freq;
}

bool ADPCM_IsPlaying()
{
	return(LastCmd & 0x20);
}

void ADPCM_FadeOut(int32 ms)
{
	if (ms == 0)
	{
		CurrentVolume = 0;
		bFadeOut = FALSE;
		bFadeIn  = FALSE;
		FadeCycle = 0;
	}
	else if (CurrentVolume > 0)
	{
		FadeCycle = (int32)(((7159090.0 / ((double)CurrentVolume / (double)VolumeStep)) * (double)ms) / 1000.0);
		bFadeOut	= TRUE;
		bFadeIn	= FALSE;
	}
	else
	{
		bFadeOut = FALSE;
		bFadeIn  = FALSE;
		FadeCycle = 0;
	}
}

void ADPCM_FadeIn(int32	ms)
{
	if (ms == 0)
	{
		CurrentVolume = InitialVolume;
		bFadeOut = FALSE;
		bFadeIn  = FALSE;
		FadeCycle = 0;
	}
	else if (InitialVolume - CurrentVolume > 0)
	{
		FadeCycle = (int32)(((7159090.0 / (((double)InitialVolume - (double)CurrentVolume) / (double)VolumeStep)) * (double)ms) / 1000.0);
		bFadeOut = FALSE;
		bFadeIn  = TRUE;
	}
	else
	{
		bFadeOut = FALSE;
		bFadeIn  = FALSE;
		FadeCycle = 0;
	}
}

int ADPCM_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
	SFARRAY(RAM, 0x10000),

	SFVAR(bigdiv),
	SFVAR(smalldiv),
	SFVAR(Addr),
	SFVAR(ReadAddr),
	SFVAR(WriteAddr),
	SFVAR(LengthCount),
	SFVAR(CacheLengthCount),
	SFVAR(HalfCount),
	SFVAR(LastCmd),
	SFVAR(SampleFreq),

	SFVAR(ReadPending),
	SFVAR(ReadBuffer),

	SFVAR(WritePending),
	SFVAR(WritePendingValue),

	SFVAR(CurrentVolume),
	SFVAR(VolumeStep),
	SFVAR(bFadeIn),
	SFVAR(bFadeOut),
	SFVAR(ClockCount),
	SFVAR(FadeCycle),

	SFVAR(ad_sample),
	SFVAR(ad_ref_index),
	SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "APCM");
 if(load)
 {
  RedoLPF(SampleFreq);
 }
 return(ret);
}
