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

#include "../shared.h"
#include "cd.h"
#include "pcm.h"

static bool IC_On;	// Set by D7 of writes to 0x0007.
static uint8 WB;	// Wave bank select.  Set by the lower 4 bits of writes to 0x0007 when D6 is clear.
static uint8 CB;	// Channel select.  Set by the lower 3 bits of wries to 0x0007 when D6 is set.
static uint8 ChannelOn;	// Register 0x0008, on/off register for each of the 8 channels
static uint8 WaveData[0x10000];	// 64KiB of waveform data
typedef struct
{
	uint32 Counter;	// Address counter.  The lower 11 bits are the fractional component.
	uint8 ENV;	// 0x0000
        uint8 PAN;      // 0x0001.  Upper 4 bits affect R output, lower 4 bits affect L output.
	uint16 FD;	// FDL(0x0002) and FDH(0x0003)  Controls the incrementing of the address counter.
        uint16 LS;      // LSL(0x0004) and LSH(0x0005)  When 0xFF(stop data) is read from waveform memory,
			// the address counter will be reloaded with this value.
	uint8 ST;	// 0x0006
		        //  Upper 8-bits of the channel's read address are set to when a channel is enabled(the lower 8 bits are set to 0).
	
} pcm_channel_t;

static pcm_channel_t Channels[8];


// Last timestamp
static int32 last_ts;

// Divide by (4 * 384) counter
static int32 div_count;

static void PCM_Clock(void);
static void MDCD_PCM_Update(void);

void MDCD_PCM_Write(uint32 A, uint8 V)
{
 A &= 0x1FFF;

 MDCD_PCM_Update();

 if(A >= 0x1000)
 {
  WaveData[(WB << 12) | (A & 0x0FFF)] = V;
 }
 else switch(A)
 {
  case 0x00: Channels[CB].ENV = V;
	     break;

  case 0x01: Channels[CB].PAN = V;
	     break;

  case 0x02: Channels[CB].FD &= 0xFF00;
	     Channels[CB].FD |= V << 0;
	     break;

  case 0x03: Channels[CB].FD &= 0x00FF;
	     Channels[CB].FD |= V << 8;
	     break;

  case 0x04: Channels[CB].LS &= 0xFF00;
	     Channels[CB].LS |= V << 0;
	     break;

  case 0x05: Channels[CB].LS &= 0x00FF;
	     Channels[CB].LS |= V << 8;
	     break;

  case 0x06: Channels[CB].ST = V;
	     break;

  case 0x07: IC_On = V & 0x80;
	     if(V & 0x40)
	      CB = V & 0x7;
	     else
	      WB = V & 0xF;
	     break;

  case 0x08: ChannelOn = V;
	     break;
 }
}

static void PCM_Clock(void)
{
 int32 left = 0;
 int32 right = 0;

 for(int ch = 0; ch < 8; ch++)
 {
  pcm_channel_t *CurChan = &Channels[ch];

  if(!IC_On)
  {
   CurChan->Counter = CurChan->ST << (8 + 11);
  }
  else
  {
   uint8 tmp_sample;

   CurChan->Counter += CurChan->FD;
   tmp_sample = WaveData[(CurChan->Counter >> 11) & 0xFFFF];

   if(tmp_sample == 0xFF)
   {
    CurChan->Counter = CurChan->LS << 11;
   }
  }
  int32 sample = WaveData[(CurChan->Counter >> 11) & 0xFFFF];

  if(sample & 0x80)
   sample = -(sample & 0x7F);

  sample *= CurChan->ENV;

  if(ChannelOn & (1 << ch))
  {
   left += (sample * (CurChan->PAN & 0x0F)) >> 4;
   right += (sample * ((CurChan->PAN >> 4) & 0x0F)) >> 4;
  }
 }

 if(left < -0x8000)
  left = -0x8000;
 if(left > 0x7FFF)
  left = 0x7FFF;

 if(right < -0x8000)
  right = -0x8000;
 if(right > 0x7FFF)
  right = 0x7FFF;
}

void MDCD_PCM_Run(int32 cycles)
{
 div_count -= cycles;
 while(div_count <= 0)
 {
  PCM_Clock();
  div_count += 384;
 } 
}

void MDCD_PCM_Update(void)
{
 #if 0
 int32 cycles = md_cd_timestamp - last_ts;


 last_ts = md_cd_timestamp;
 #endif
}

void MDCD_PCM_Reset(void)
{
 IC_On = 0;
 WB = 0;
 CB = 0;
 ChannelOn = 0;


 memset(WaveData, 0, sizeof(WaveData));
 memset(Channels, 0, sizeof(Channels));
}
