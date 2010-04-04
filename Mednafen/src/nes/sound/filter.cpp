/*  Festalon - NSF Player
 *  Copyright (C) 2004 Xodnizel
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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../nes.h"

#include "filter.h"

#include "fcoeffs.h"

#include "cputest.h"

/* Returns number of samples written to out. */
/* leftover is set to the number of samples that need to be copied 
   from the end of in to the beginning of in.
*/

/* This filtering code assumes that almost all input values stay below 32767.
   Do not adjust the volume in the wlookup tables and the expansion sound
   code to be higher, or you *might* overflow the FIR code.
*/
int32 FESTAFILT_Do(FESTAFILT *ff, int16 *in, int16 *out, uint32 maxoutlen, uint32 inlen, int32 *leftover, int sinput)
{
	int32 x;
	int32 max;
	uint32 count=0;
	int32 *flout = ff->boobuf;

	max = inlen & ~0x1F;
	max -= NCOEFFS;
	if(max < 0) max = 0;

	if(0)
	{


	}
	#ifdef ARCH_X86
	else if(ff->cpuext & MM_MMX)
	{
 	 uint32 bigcount = max / ff->mrratio;
	 if(sinput)
	 {
	  #define FILTMMX_SKIP_ADD_STR "skippy"
	  #define FILTMMX_SKIP_ADD
	  #ifdef __x86_64__
	  #include "filter-amd64-mmx.h"
	  #else
	  #include "filter-mmx.h"
	  #endif
	  #undef FILTMMX_SKIP_ADD
	  #undef FILTMMX_SKIP_ADD_STR
	 }
	 else
	 {
	  #define FILTMMX_SKIP_ADD_STR ""
	  #ifdef __x86_64__
	  #include "filter-amd64-mmx.h"
	  #else
	  #include "filter-mmx.h"
	  #endif
	  #undef FILTMMX_SKIP_ADD_STR
	 }
	}
	#endif
        else
         for(x=0;x<max;x+=ff->mrratio)
        {
         int32 acc = 0;
         unsigned int c;
         int16 *wave;
         int16 *coeffs;

         for(c=0,wave=&in[x],coeffs=ff->coeffs_i16;c<NCOEFFS;c+=2)
         {
          acc += ((int32)wave[c] * coeffs[c]) >> 16;
          acc += ((int32)(wave+1)[c] * (coeffs+1)[c]) >> 16;
         }
	 *flout = acc;
         if(!sinput)
          *flout += 32767;
	 flout++;
	 count++;
	}

        *leftover=inlen - max;
	count = max / ff->mrratio;
	static int16 last_sample;
	for(uint32 x = 0; x < count; x++)
	{
         ff->NESSynth->offset(x, ff->boobuf[x] - last_sample, ff->NESBuf);
	 last_sample = ff->boobuf[x]; 
	}
	uint32 newlen;

	ff->NESBuf->end_frame(count);
	newlen = ff->NESBuf->read_samples(out, maxoutlen, 0);
	
	return(newlen);
}

void FESTAFILT_Kill(FESTAFILT *ff)
{
 delete ff->NESSynth;
 delete ff->NESBuf;
 free(ff->realmem);
}


void FESTAFILT_SetMultiplier(FESTAFILT *ff, double multiplier)
{
 ff->multiplier = multiplier;
 ff->NESBuf->clock_rate((long)(ff->imrate * multiplier));
}

void FESTAFILT_SetVolume(FESTAFILT *ff, unsigned int newvolume)
{
 ff->SoundVolume = newvolume;
}

FESTAFILT * FESTAFILT_Init(int32 rate, int PAL, double multiplier, uint32 volume)
{
 double *tabs[2]={COEF_NTSC,COEF_PAL};
 double *tmp;
 int32 x;
 uint32 nco;
 uint32 div;
 FESTAFILT *ff;
 void *realmem;
 double cpuclock;


 if(PAL)
  cpuclock = 1662607.125;
 else
  cpuclock = 1789772.72;

 if(!(realmem=malloc(16 + sizeof(FESTAFILT)))) return(0);

 ff = (FESTAFILT *)realmem;

 ff = (FESTAFILT *)(unsigned long)(((unsigned long)ff+0xFLL)&~0xFLL);
 memset(ff,0,sizeof(FESTAFILT));
 ff->realmem = realmem;

 nco=NCOEFFS;

 tmp=tabs[(PAL?1:0)];
 div = 32;

 ff->multiplier = multiplier;
 ff->mrratio=div;

 for(x=0;x<NCOEFFS>>1;x++)
  ff->coeffs_i16[x]=ff->coeffs_i16[NCOEFFS-1-x]=(int16)(tmp[x] * 65536);

 ff->rate=rate;
 
 ff->imrate = cpuclock / div;
 ff->lrhfactor = rate / ff->imrate;
 ff->SoundVolume = volume;

 ff->NESBuf = new Blip_Buffer();
 ff->NESSynth = new Blip_Synth<blip_good_quality, 65535>();
 ff->NESBuf->set_sample_rate(rate ? rate : 44100, 50);
 ff->NESSynth->volume((float)volume*3/2/100);
 ff->NESBuf->clock_rate((long)(ff->imrate * multiplier));
 ff->NESBuf->bass_freq(80);
 ff->cpuext = ac_mmflag();
 return(ff);
}
