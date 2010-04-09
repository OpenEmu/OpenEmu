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

#include "../mednafen.h"
#include "../state.h"
#include "gb.h"
#include "sound.h"
#include "sound/Gb_Apu.h"
#include <blip/Stereo_Buffer.h>

static Gb_Apu gb_apu;
static Stereo_Buffer gb_buf;

void MDFNGBSOUND_Reset(void)
{
	gb_apu.reset();
}

uint32 MDFNGBSOUND_Read(int ts, uint32 addr)
{
	uint32 ret;

	ret = gb_apu.read_register(ts, addr);

	//printf("Read: %04x %02x\n", addr, ret);

	return(ret);
}

void MDFNGBSOUND_Write(int ts, uint32 addr, uint8 val)
{
	//if(addr == 0xFF26)
 	// printf("%04x %02x\n", addr, val);
	gb_apu.write_register(ts, addr, val);
}

void MDFNGB_SetSoundMultiplier(double multiplier)
{
	gb_buf.clock_rate((long)(4194304 * multiplier));
}

void MDFNGB_SetSoundVolume(uint32 volume)
{
	gb_apu.volume((double)volume / 2 / 100);
}

void MDFNGB_Sound(int rate)
{
	gb_buf.set_sample_rate(rate?rate:44100, 40);
}

int MDFNGBSOUND_StateAction(StateMem *sm, int load, int data_only)
{
 Gb_ApuState gb_state;
 int ret = 1;

 memset(&gb_state, 0, sizeof(Gb_ApuState));

 if(!load)
 {
  gb_apu.save_state(&gb_state);
 }

 SFORMAT StateRegs[] =
 {
  SFARRAYN(gb_state.regs, sizeof(gb_state.regs), "regs"),
  SFARRAY32N(gb_state.sq_phase, 2, "sq_phase"), 
  SFARRAY32N(gb_state.sq_sweep_delay, 2, "sq_sweep_delay"),
  SFARRAY32N(gb_state.sq_sweep_freq, 2, "sq_sweep_freq"),
  SFVARN(gb_state.noise_bits, "noise_bits"),
  SFVARN(gb_state.wave_pos, "wave_pos"),
  SFARRAYN(gb_state.wave, sizeof(gb_state.wave), "wave"),
  SFARRAY32N(gb_state.env_delay, 3, "env_delay"),
  SFARRAY32N(gb_state.length, 4, "length"),
  SFARRAY32N(gb_state.volume, 4, "volume"),
  SFARRAY32N(gb_state.enabled, 4, "enabled"),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "APU"))
  ret = 0;
 else if(load)
 {
  gb_apu.load_state(&gb_state);
 }
 return(ret);
}

static bool forcemono;

void MDFNGBSOUND_Init(bool WantMono)
{
	gb_buf.set_sample_rate(FSettings.SndRate?FSettings.SndRate:44100, 40);
	gb_buf.clock_rate(4194304);

        forcemono = WantMono;

        if(forcemono)
 	 gb_apu.output(gb_buf.left(), NULL, NULL);
	else
         gb_apu.output(gb_buf.center(), gb_buf.left(), gb_buf.right());
}

int16 *MDFNGBSOUND_Flush(int ts, int32 *length)
{
	static int16 buffer[8000];

	gb_apu.end_frame(ts);

	if(forcemono)
        {
         gb_buf.left()->end_frame(ts);
         *length = gb_buf.left()->read_samples(buffer, 8000);
        }
	else
	{
	 gb_buf.end_frame(ts);
	 *length = gb_buf.read_samples(buffer, 8000);
	 *length /= 2;
	}

	if(!FSettings.SndRate)
	{
	 *length = 0;
	 return(NULL);
	}

	return(buffer);
}
