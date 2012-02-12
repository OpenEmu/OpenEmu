/// \file
/// \brief Sound filtering code

#include <math.h>
#include <stdio.h>
#include "types.h"

#include "sound.h"
#include "x6502.h"
#include "fceu.h"
#include "filter.h"

#include "fcoeffs.h"

static int32 sq2coeffs[SQ2NCOEFFS];
static int32 coeffs[NCOEFFS];

static uint32 mrindex;
static uint32 mrratio;

void SexyFilter(int32 *in, int32 *out, int32 count)
{
	static int64 acc1=0,acc2=0;
	int32 mul1,mul2,vmul;

	mul1=(94<<16)/FSettings.SndRate;
	mul2=(24<<16)/FSettings.SndRate;
	vmul=(FSettings.SoundVolume<<16)*3/4/100;

#if SOUND_QUALITY == 1
	vmul >>= 2;
#else
	vmul <<= 1;			/* TODO:  Increase volume in low quality sound rendering code itself */
#endif

	do
	{    
		int64 ino=(int64)*in*vmul;
		acc1+=((ino-acc1)*mul1)>>16;
		acc2+=((ino-acc1-acc2)*mul2)>>16;
		//printf("%d ",*in);
		*in=0;  
		{   
			int32 t=(acc1-ino+acc2)>>16;
			//if(t>32767 || t<-32768) printf("Flow: %d\n",t);
			if(t>32767)
				t=32767;
			if(t<-32768)
				t=-32768;   
			*out=t;
		}  
		in++;  
		out++;  
		count--; 
	}while(count);
}

/* Returns number of samples written to out. */
/* leftover is set to the number of samples that need to be copied 
   from the end of in to the beginning of in.
*/

//static uint32 mva=1000;

/* This filtering code assumes that almost all input values stay below 32767.
   Do not adjust the volume in the wlookup tables and the expansion sound
   code to be higher, or you *might* overflow the FIR code.
*/

#if SOUND_QUALITY == 1
int32 NeoFilterSound(int32 *in, int32 *out, uint32 inlen, int32 *leftover)
{
	uint32 x;
	uint32 max;
	int32 *outsave=out;
	int32 count=0;

	max=(inlen-1)<<16;

	for(x=mrindex;x<max;x+=mrratio)
	{
		int32 acc=0,acc2=0;
		unsigned int c;
		int32 *S,*D;

		for(c=SQ2NCOEFFS,S=&in[(x>>16)-SQ2NCOEFFS],D=sq2coeffs;c;c--,D++)
		{
			acc+=(S[c]**D)>>6;
			acc2+=(S[1+c]**D)>>6;
		}

		acc=((int64)acc*(65536-(x&65535))+(int64)acc2*(x&65535))>>(16+11);
		*out=acc;
		out++;   
		count++; 
	}

	mrindex=x-max;

	mrindex+=SQ2NCOEFFS*65536;
	*leftover=SQ2NCOEFFS+1;

	if(GameExpSound.NeoFill)
		GameExpSound.NeoFill(outsave,count);

	SexyFilter(outsave,outsave,count);
	return(count);
}
#endif

void MakeFilters(int32 rate)
{
	const int32 *tabs[6]={C44100NTSC,C44100PAL,C48000NTSC,C48000PAL,C96000NTSC,
		C96000PAL};
	const int32 *sq2tabs[6]={SQ2C44100NTSC,SQ2C44100PAL,SQ2C48000NTSC,SQ2C48000PAL,
		SQ2C96000NTSC,SQ2C96000PAL};

	const int32 *tmp;
	int32 x;
	uint32 nco;

#if SOUND_QUALITY == 1
	nco=SQ2NCOEFFS;
#else
	nco=NCOEFFS;
#endif

	mrindex=(nco+1)<<16;
	mrratio=(PAL?(int64)(PAL_CPU*65536):(int64)(NTSC_CPU*65536))/rate;

#if SOUND_QUALITY == 1
	tmp=sq2tabs[(PAL?1:0)|(rate==48000?2:0)|(rate==96000?4:0)];
#else
	tmp=tabs[(PAL?1:0)|(rate==48000?2:0)|(rate==96000?4:0)];
#endif

#if SOUND_QAULITY == 1
	for(x=0;x<SQ2NCOEFFS>>1;x++)
		sq2coeffs[x]=sq2coeffs[SQ2NCOEFFS-1-x]=tmp[x];
#else
	for(x=0;x<NCOEFFS>>1;x++)
		coeffs[x]=coeffs[NCOEFFS-1-x]=tmp[x];
#endif
}
