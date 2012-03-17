#include "gens.hpp"
#include "g_main.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_32x.h"

#include "macros/math_m.h"

int Debug;
int Frame_Skip;
int Frame_Number;
int DAC_Improv;


void Init_Tab(void)
{
	int x, y, dep;
	
	for (x = 0; x < 1024; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0x3FC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 9)] = (unsigned short) dep;
		}
	}
	
	for (x = 0; x < 512; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0x1FC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 8) + 0x8000] =
				(unsigned short) (dep + 0x8000);
		}
	}
	
	for (x = 0; x < 256; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0xFC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xC000] =
				(unsigned short) (dep + 0xC000);
		}
	}
	
	for (x = 0; x < 256; x++)
	{
		for (y = 0; y < 32; y++)
		{
			dep = (x & 3) + (((x & 0xFC) >> 2) << 7);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xE000] =
				(unsigned short) (dep + 0xE000);
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xF000] =
				(unsigned short) (dep + 0xF000);
		}
	}
	
	for (x = 0; x < 512; x++)
		Z80_M68K_Cycle_Tab[x] = (int) ((double) x * 7.0 / 15.0);
}


void Check_Country_Order(void)
{
	if ((Country_Order[0] == Country_Order[1])
	    || (Country_Order[0] == Country_Order[2])
	    || (Country_Order[1] == Country_Order[2])
	    || (Country_Order[0] == Country_Order[2]) || (Country_Order[0] > 2)
	    || (Country_Order[0] < 0) || (Country_Order[1] > 2)
	    || (Country_Order[1] < 0) || (Country_Order[2] > 2)
	    || (Country_Order[2] < 0))
	{
		Country_Order[0] = 0;
		Country_Order[1] = 1;
		Country_Order[2] = 2;
	}
}


/**
 * Set_CPU_Freq(): Sets the CPU frequencies.
 * @param system System: 0 == MD only, 1 == MCD, 2 == 32X
 */
void Set_Clock_Freq(const int system)
{
	if (CPU_Mode)
	{
		// PAL
		CPL_Z80 = roundDouble((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = roundDouble((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = roundDouble(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = roundDouble(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		
		if (system == 1) // SegaCD
		{
			CD_Access_Timer = 2080;
			Timer_Step = 136752;
		}
		
		if (system == 2) // 32X
			_32X_VDP.Mode &= ~0x8000;
		
		YM2612_Init(CLOCK_PAL / 7, audio->soundRate(), YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, audio->soundRate());
	}
	else
	{
		// NTSC
		CPL_Z80 = roundDouble((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = roundDouble((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 = roundDouble(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = roundDouble(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;
		
		if (system == 1) // SegaCD
		{
			CD_Access_Timer = 2096;
			Timer_Step = 135708;
		}
		
		if (system == 2) // 32X
			_32X_VDP.Mode |= 0x8000;
		
		YM2612_Init(CLOCK_NTSC / 7, audio->soundRate(), YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, audio->soundRate());
	}
	
	if (system == 2) // 32X
		_32X_VDP.State |= 0x2000;
}
