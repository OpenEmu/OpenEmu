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
#include "gb_apu/Gb_Apu.h"
#include <blip/Stereo_Buffer.h>

static Gb_Apu gb_apu;
static Stereo_Buffer *gb_buf = NULL;

void MDFNGBSOUND_Reset(void)
{
	// TODO: set hardware mode to mode_dmg, mode_cgb, or mode_agb
	// (latter if you're running classic GB game on Game Boy Advance)
	gb_apu.reset();
}

uint32 MDFNGBSOUND_Read(int ts, uint32 addr)
{
	uint32 ret;

	ret = gb_apu.read_register(ts*GB_APU_OVERCLOCK, addr);

	return(ret);
}

void MDFNGBSOUND_Write(int ts, uint32 addr, uint8 val)
{
	//if(addr == 0xFF26)
 	// printf("%04x %02x\n", addr, val);
	gb_apu.write_register(ts * GB_APU_OVERCLOCK, addr, val);
}

static bool RedoBuffer(uint32 rate)
{
	if(gb_buf)
	{
	 delete gb_buf;
	 gb_buf = NULL;
	}

	gb_apu.set_output(NULL, NULL, NULL);

	if(rate)
	{
	 gb_buf = new Stereo_Buffer();
	 
         gb_buf->set_sample_rate(rate, 40);
         gb_buf->clock_rate((long)(4194304 * GB_APU_OVERCLOCK * 1));

         gb_apu.set_output(gb_buf->center(), gb_buf->left(), gb_buf->right());
        }

	return(TRUE);
}

bool MDFNGB_SetSoundRate(uint32 rate)
{
	RedoBuffer(rate);

	return(TRUE);
}

int MDFNGBSOUND_StateAction(StateMem *sm, int load, int data_only)
{
 gb_apu_state_t gb_state;
 int ret = 1;

 //if(!load) // always save state, in case there is none to load
 {
  gb_apu.save_state(&gb_state);
 }

 SFORMAT StateRegs[] =
 {
  SFVARN(gb_state, "apu_state"),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "APU"))
  ret = 0;
 else if(load)
 {
  // TODO: set hardware mode to mode_dmg, mode_cgb, or mode_agb
  // (latter if you're running classic GB game on Game Boy Advance)
  gb_apu.reset();
  gb_apu.load_state(gb_state);
 }
 return(ret);
}

void MDFNGBSOUND_Init(void)
{
        gb_apu.volume(0.5);

	RedoBuffer(0);
}

int32 MDFNGBSOUND_Flush(int ts, int16 *SoundBuf, const int32 MaxSoundFrames)
{
	int32 SoundFrames = 0;

	gb_apu.end_frame(ts * GB_APU_OVERCLOCK);

	if(SoundBuf && gb_buf)
	{
	 gb_buf->end_frame(ts * GB_APU_OVERCLOCK);
	 SoundFrames = gb_buf->read_samples(SoundBuf, MaxSoundFrames * 2) / 2;
	}
	else if(gb_buf)
	 exit(1);

	return(SoundFrames);
}
