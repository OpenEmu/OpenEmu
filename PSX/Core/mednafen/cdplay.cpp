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

//#include <mednafen/mednafen.h>
#include "mednafen.h"
#include "cdrom/cdromif.h"
#include "netplay.h"
#include <blip/Blip_Buffer.h>
#include <trio/trio.h>
#include <vector>
#include <math.h>

using namespace CDUtility;

#include <mednafen/resampler/resampler.h>

namespace MDFN_IEN_CDPLAY
{

static std::vector<double> sqrt_lut; //[65536];
static std::vector<double> sin_lut; //[65536];
static SpeexResamplerState *resampler = NULL;

static uint8 *controller_ptr;
static uint8 last_controller;

enum
{
 PLAYMODE_PAUSE = -1,
 PLAYMODE_STOP = 0,
 PLAYMODE_PLAY = 1,
 PLAYMODE_SCAN_FORWARD = 2,
 PLAYMODE_SCAN_REVERSE = 3,
};

static int PlayMode;
static int32 PlaySector;
static int16 CDDABuffer[588 * 2];

static int16 ResampBuffer[588 * 2][2];	// Resampler input buffer, * 2 for resampler leftovers
static uint32 ResampBufferPos;
static uint32 PrevRate;

static std::vector<CDIF *> *cdifs;

static int32 CurrentATLI;

struct AudioTrackInfo
{
 inline AudioTrackInfo(unsigned disc_, int32 track_, int32 lba_, int32 final_lba_)
 {
  disc = disc_;
  track = track_;
  lba = lba_;
  final_lba = final_lba_;
 }

 unsigned disc;
 int32 track;
 int32 lba;
 int32 final_lba;	// Inclusive.
};

static std::vector<AudioTrackInfo> AudioTrackList;

static void InitLUT(void);

static int LoadCD(std::vector<CDIF *> *CDInterfaces)
{
 cdifs = CDInterfaces;

 AudioTrackList.clear();

 for(unsigned disc = 0; disc < cdifs->size(); disc++)
 {
  TOC toc;

  (*cdifs)[disc]->ReadTOC(&toc);

  for(int32 track = toc.first_track; track <= toc.last_track; track++)
  {
   if(!(toc.tracks[track].control & 0x4))
   {
    AudioTrackList.push_back(AudioTrackInfo(disc, track, toc.tracks[track].lba, toc.tracks[track + 1].lba - 1));
   }
  }
 }

 if(!AudioTrackList.size())
 {
  puts("Audio track doesn't exist");
  return(0);
 }

 CurrentATLI = 0;
 PlaySector = AudioTrackList[CurrentATLI].lba;
 PlayMode = PLAYMODE_PLAY;   //STOP;

 {
  int err;
  resampler = speex_resampler_init(2, 44100, (int)48000, 5, &err);
  PrevRate = 48000;
 }
 //resampler.buffer_size(588 * 2 + 100);
 //resampler.time_ratio((double)44100 / 48000, 0.9965);
 ResampBufferPos = 0;

 InitLUT();

 return(1);
}

static bool TestMagicCD(std::vector<CDIF *> *CDInterfaces)
{
 CDUtility::TOC magic_toc;

 for(unsigned i = 0; i < CDInterfaces->size(); i++)
 {
  (*CDInterfaces)[i]->ReadTOC(&magic_toc);

  // If any audio track is found, return true.
  for(int32 track = magic_toc.first_track; track <= magic_toc.last_track; track++)
   if(!(magic_toc.tracks[track].control & 0x4))
    return(true);
 }

 return(false);
}

static void CloseGame(void)
{
 if(resampler)
 {
  speex_resampler_destroy(resampler);
  resampler = NULL;
 }
 sin_lut.resize(0);
 sqrt_lut.resize(0);
}

static uint8 SubQBuf[3][0xC];

static void GenSubQFromSubPW(uint8 *SubPWBuf)
{
 uint8 sq[0xC];

 memset(sq, 0, 0xC);

 for(int i = 0; i < 96; i++)
  sq[i >> 3] |= ((SubPWBuf[i] & 0x40) >> 6) << (7 - (i & 7));

 if(!subq_check_checksum(sq))
  puts("SubQ checksum error!");
 else
 {
  uint8 adr = sq[0] & 0xF;

  if(adr <= 0x3)
   memcpy(SubQBuf[adr], sq, 0xC);
 }
}
static const int lobes = 2;
static const int oversample_shift = 7;	//1; //7;
static const int oversample = 1 << oversample_shift;
static const int oversample_mo = oversample - 1;

static void InitLUT(void)
{
 sqrt_lut.resize(65536);
 sin_lut.resize(65536);

 for(int i = 0; i < 65536; i++)
  sqrt_lut[i] = sqrt((double)i / 65536);

 for(int i = 0; i < 65536; i++)
  sin_lut[i] = sin((double)i * M_PI * 2 / 65536);
}

static void Emulate(EmulateSpecStruct *espec)
{
 uint8 sector_buffer[2352 + 96];
 uint8 new_controller = *controller_ptr;

 espec->MasterCycles = 588;

 //printf("%d %d\n", toc.tracks[100].lba, AudioTrackList[AudioTrackList.size() - 1] + 1);

 if(PlaySector < AudioTrackList[CurrentATLI].lba)	// Reverse-scanning handling.
 {
  if(CurrentATLI > 0)
  {
   CurrentATLI--;
   PlaySector = AudioTrackList[CurrentATLI].final_lba;
  }
  else
  {
   CurrentATLI = 0;
   PlayMode = PLAYMODE_STOP;
   PlaySector = AudioTrackList[CurrentATLI].lba;
  }
 }
 else if(PlaySector > AudioTrackList[CurrentATLI].final_lba)
 {
  if((CurrentATLI + 1) < AudioTrackList.size())
   CurrentATLI++;
  else
  {
   CurrentATLI = 0;
   PlayMode = PLAYMODE_STOP;
  }

  PlaySector = AudioTrackList[CurrentATLI].lba;
 }

 if(PlayMode == PLAYMODE_STOP || PlayMode == PLAYMODE_PAUSE)
 {
  //memset(CDDABuffer, 0, sizeof(CDDABuffer));
  for(int i = 0; i < 588; i++)
  {
   ResampBuffer[ResampBufferPos][0] = 0;
   ResampBuffer[ResampBufferPos][1] = 0;
   ResampBufferPos++;
  }
 }
 else
 {
  (*cdifs)[AudioTrackList[CurrentATLI].disc]->ReadRawSector(sector_buffer, PlaySector);
  GenSubQFromSubPW(sector_buffer + 2352);

  for(int i = 0; i < 588 * 2; i++)
  {
   CDDABuffer[i] = MDFN_de16lsb(&sector_buffer[i * sizeof(int16)]);

   ResampBuffer[ResampBufferPos + (i >> 1)][i & 1] = CDDABuffer[i] / 2;
  }
  ResampBufferPos += 588;
 }

 if(espec->SoundBuf)
 {
  if((int)espec->SoundRate == 44100)
  {
   memcpy(espec->SoundBuf, ResampBuffer, ResampBufferPos * 2 * sizeof(int16));
   espec->SoundBufSize = ResampBufferPos;
   ResampBufferPos = 0;
  }
  else
  {
   spx_uint32_t in_len;
   spx_uint32_t out_len;

   if(PrevRate != (uint32)espec->SoundRate)
   {
    speex_resampler_set_rate(resampler, 44100, (uint32)espec->SoundRate);
    PrevRate = (uint32)espec->SoundRate;
   }

   in_len = ResampBufferPos;
   out_len = 524288;	// FIXME, real size.

   speex_resampler_process_interleaved_int(resampler, (const spx_int16_t *)ResampBuffer, &in_len, (spx_int16_t *)espec->SoundBuf, &out_len);

   assert(in_len <= ResampBufferPos);

   if((ResampBufferPos - in_len) > 0)
    memmove(ResampBuffer, ResampBuffer + in_len, (ResampBufferPos - in_len) * sizeof(int16) * 2);

   ResampBufferPos -= in_len;

   //printf("%d\n", ResampBufferPos);
   assert((ResampBufferPos + 588) <= (sizeof(ResampBuffer) / sizeof(int16) / 2));

   espec->SoundBufSize = out_len;
  }
 }
// for(int i = 0; i < espec->SoundBufSize * 2; i++)
//  espec->SoundBuf[i] = (rand() & 0x7FFF) - 0x4000;	//(rand() * 192) >> 8

 if(!espec->skip)
 {
  char tmpbuf[256];
  const MDFN_PixelFormat &format = espec->surface->format;
  uint32 *pixels = espec->surface->pixels;
  uint32 text_color = format.MakeColor(0xE0, 0xE0, 0xE0);
  uint32 text_shadow_color = format.MakeColor(0x20, 0x20, 0x20);
  uint32 cur_sector = PlaySector;

  espec->DisplayRect.x = 0;
  espec->DisplayRect.y = 0;

  espec->DisplayRect.w = 320;
  espec->DisplayRect.h = 240;

  espec->surface->Fill(0, 0, 0, 0);

  {
   uint32 color_table[256];

   for(int i = 0; i < 170; i++)
   {
    int m = 255 * i / 170;

    color_table[i] = format.MakeColor(m, 0, m);

    //printf("%d %d %08x\n", m, i, color_table[i]);
   }

  for(int i = 0; i < 588; i++)
  {
   int32 unip_samp;
   int32 next_unip_samp; 

   unip_samp = ((CDDABuffer[i * 2 + 0] + CDDABuffer[i * 2 + 1]) >> 1) + 32768;
   next_unip_samp = ((CDDABuffer[(i * 2 + 2) % 1176] + CDDABuffer[(i * 2 + 3) % 1176]) >> 1) + 32768;

   for(int osi = 0; osi < oversample; osi++)
   {
    double sample;
    int x;
    int y;
    double x_raw, y_raw;
    double x_raw2, y_raw2;
    double x_raw_prime, y_raw_prime;

    sample = (double)(unip_samp * (oversample - osi) + next_unip_samp * osi) / (oversample * 65536);

    int32 theta_i = (int64)65536 * (i * oversample + osi) / (oversample * 588);
    int32 theta_i_alt = (int64)65536 * (i * oversample + osi) / (oversample * 588);

    double radius = sin_lut[(lobes * theta_i) & 0xFFFF];	//	 * sin_lut[(16384 + (theta_i)) & 0xFFFF];
    double radius2 = sin_lut[(lobes * (theta_i + 1)) & 0xFFFF];	// * sin_lut[(16384 + ((theta_i + 1))) & 0xFFFF];

    x_raw = radius * sin_lut[(16384 + theta_i_alt) & 0xFFFF];
    y_raw = radius * sin_lut[theta_i_alt & 0xFFFF];

    x_raw2 = radius2 * sin_lut[(16384 + theta_i_alt + 1) & 0xFFFF];
    y_raw2 = radius2 * sin_lut[(theta_i_alt + 1) & 0xFFFF];

    // Approximation, of course.
    x_raw_prime = (x_raw2 - x_raw) / (1 * M_PI * 2 / 65536);
    y_raw_prime = (y_raw2 - y_raw) / (1 * M_PI * 2 / 65536);

//    printf("%f %f\n", y_raw_prime, sin_lut[(16384 + lobes * theta_i_alt) & 0xFFFF] + sin_lut[(16384 + theta_i_alt) & 0xFFFF]);

    if(x_raw_prime || y_raw_prime)
    {
     x_raw_prime = x_raw_prime / sqrt(x_raw_prime * x_raw_prime + y_raw_prime * y_raw_prime);
     y_raw_prime = y_raw_prime / sqrt(x_raw_prime * x_raw_prime + y_raw_prime * y_raw_prime);
    }

    x_raw += (sample - 0.5) * y_raw_prime / 2;
    y_raw += (sample - 0.5) * -x_raw_prime / 2;

    x = 160 + 100 * x_raw;
    y = 120 + 100 * y_raw;

    if((x >= 0 && x < 320) && (y >= 0 && y < 240))
     pixels[x + y * espec->surface->pitch32] = format.MakeColor(255, 255, 255);	//color_table[(int)(sample * 150)];	//x * x + y * y; //format.MakeColor(sample * 255, 0, sample * 255);
   }
  }
  }

  {
   TOC toc;

   (*cdifs)[AudioTrackList[CurrentATLI].disc]->ReadTOC(&toc);

   trio_snprintf(tmpbuf, 256, "Disc: %d/%d", AudioTrackList[CurrentATLI].disc + 1, cdifs->size());
   DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)tmpbuf, text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
   pixels += 22 * espec->surface->pitch32;

   trio_snprintf(tmpbuf, 256, "Track: %d/%d", AudioTrackList[CurrentATLI].track, toc.last_track);
   DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)tmpbuf, text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
   pixels += 22 * espec->surface->pitch32;

   trio_snprintf(tmpbuf, 256, "Sector: %d/%d", cur_sector, toc.tracks[100].lba - 1);
   DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)tmpbuf, text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
   pixels += 22 * espec->surface->pitch32;
  }

  pixels += 22 * espec->surface->pitch32;


  DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)"SubQ", text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
  pixels += 22 * espec->surface->pitch32;

  //trio_snprintf(tmpbuf, 256, "Q-Mode: %01x", SubQBuf[1][0] & 0xF);
  //DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)tmpbuf, text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
  //pixels += 22 * espec->surface->pitch32;

  trio_snprintf(tmpbuf, 256, "Track: %d", BCD_to_U8(SubQBuf[1][1]));
  DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)tmpbuf, text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
  pixels += 22 * espec->surface->pitch32;

  trio_snprintf(tmpbuf, 256, "Index: %d", BCD_to_U8(SubQBuf[1][2]));
  DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)tmpbuf, text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
  pixels += 22 * espec->surface->pitch32;


  trio_snprintf(tmpbuf, 256, "Relative: %02d:%02d:%02d", BCD_to_U8(SubQBuf[1][3]), BCD_to_U8(SubQBuf[1][4]), BCD_to_U8(SubQBuf[1][5]));
  DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)tmpbuf, text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
  pixels += 22 * espec->surface->pitch32;

  trio_snprintf(tmpbuf, 256, "Absolute: %02d:%02d:%02d", BCD_to_U8(SubQBuf[1][7]), BCD_to_U8(SubQBuf[1][8]), BCD_to_U8(SubQBuf[1][9]));
  DrawTextTransShadow(pixels, espec->surface->pitch32 * 4, 320, (UTF8 *)tmpbuf, text_color, text_shadow_color, 0, MDFN_FONT_9x18_18x18);
  pixels += 22 * espec->surface->pitch32;
 }

 if(PlayMode != PLAYMODE_STOP && PlayMode != PLAYMODE_PAUSE)
 {
  const int scan_amount = 4; //16;

  if(new_controller & 0x40)
   PlaySector += scan_amount;
  else if(new_controller & 0x80)
   PlaySector -= scan_amount;
  else
   PlaySector++;
 }

 if(!(last_controller & 0x1) && (new_controller & 1))
 {
  PlayMode = (PlayMode == PLAYMODE_PLAY) ? PLAYMODE_PAUSE : PLAYMODE_PLAY;
 }

 if(!(last_controller & 0x2) && (new_controller & 2)) // Stop
 {
  PlayMode = PLAYMODE_STOP;
  PlaySector = AudioTrackList[CurrentATLI].lba;
 }

 if(!(last_controller & 0x4) && (new_controller & 4))
 {
  if(CurrentATLI < (AudioTrackList.size() - 1))
   CurrentATLI++;

  PlaySector = AudioTrackList[CurrentATLI].lba;
 }

 if(!(last_controller & 0x8) && (new_controller & 8))
 {
  if(CurrentATLI)
   CurrentATLI--;

  PlaySector = AudioTrackList[CurrentATLI].lba;
 }

 if(!(last_controller & 0x10) && (new_controller & 0x10))
 {
  CurrentATLI = std::min<int>(CurrentATLI + 10, AudioTrackList.size() - 1);
  PlaySector = AudioTrackList[CurrentATLI].lba;
 }

 if(!(last_controller & 0x20) && (new_controller & 0x20))
 {
  CurrentATLI = std::max<int>(CurrentATLI - 10, 0);
  PlaySector = AudioTrackList[CurrentATLI].lba;
 }


 last_controller = new_controller;
}

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { NULL, NULL }
};

static void SetInput(int port, const char *type, void *ptr)
{
 controller_ptr = (uint8 *)ptr;
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFNNPCMD_POWER:
  case MDFNNPCMD_RESET: break;
 }
}

static MDFNSetting CDPlaySettings[] =
{
 { NULL }
};

static const InputDeviceInputInfoStruct IDII[] =
{
 { "play_pause", "Play/Pause", 0, IDIT_BUTTON, NULL },
 { "stop", "Stop", 1, IDIT_BUTTON, NULL },
 { "next_track", "Next Track", 2, IDIT_BUTTON, NULL },
 { "previous_track", "Previous Track", 3, IDIT_BUTTON, NULL },

 { "next_track_10", "Next Track 10", 4, IDIT_BUTTON, NULL },
 { "previous_track_10", "Previous Track 10", 5, IDIT_BUTTON, NULL },

 { "scan_forward", "Scan Forward", 6, IDIT_BUTTON, NULL },
 { "scan_reverse", "Scan Reverse", 7, IDIT_BUTTON, NULL },

 //{ "reverse_seek", "Reverse Seek", 1, IDIT_BUTTON, NULL },
 //{ "forward_seek", "Forward Seek", 2, IDIT_BUTTON, NULL },
 //{ "fast_reverse_seek", "Fast Reverse Seek", 3, IDIT_BUTTON, NULL },
 //{ "fast_forward_seek", "Fast Forward Seek", 4, IDIT_BUTTON, NULL },
};

static InputDeviceInfoStruct InputDeviceInfo[] =
{
 {
  "controller",
  "Controller",
  NULL,
  NULL,
  sizeof(IDII) / sizeof(InputDeviceInputInfoStruct),
  IDII,
 }
};

static const InputPortInfoStruct PortInfo[] =
{
 { "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};
}

using namespace MDFN_IEN_CDPLAY;

MDFNGI EmulatedCDPlay =
{
 "cdplay",
 "Mednafen Test CD-DA Player",
 KnownExtensions,
 MODPRIO_INTERNAL_EXTRA_LOW,
 NULL,          // Debug info
 &InputInfo,    //
 NULL,
 NULL,
 LoadCD,
 TestMagicCD,
 CloseGame,
 NULL,
 NULL,            // Layer names, null-delimited
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 false,
 NULL, //StateAction,
 Emulate,
 SetInput,
 DoSimpleCommand,
 CDPlaySettings,
 MDFN_MASTERCLOCK_FIXED(44100),
 75 * 65536 * 256,
 false, // Multires possible?

 320,   // lcm_width
 240,   // lcm_height           
 NULL,  // Dummy


 320,   // Nominal width
 240,    // Nominal height
 512, 			// Framebuffer width
 256,                  	// Framebuffer height

 2,     // Number of output sound channels
};

