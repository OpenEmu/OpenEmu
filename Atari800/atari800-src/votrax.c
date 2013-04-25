/* License note by perry_m:
Permission has been granted by the authors Mike Coates and Tom Haukap
to distribute this file under the terms of the GNU GPL license of Atari800.
The original version written by Mike Coates is from MAME.
This extensively modified version with new samples and mixing written
by Tom Haukap is from PinMAME.
I have also made modifications to this file for use in Atari800 and
allow any modifications to be distributed under any of the licenses
of Atari800, MAME and PinMAME. */

/**************************************************************************

	Votrax SC-01 Emulator

 	Mike@Dissfulfils.co.uk
	Tom.Haukap@t-online.de
	modified for Atari800 by perry_m@fastmail.fm

**************************************************************************

Votrax_Start         - Start emulation, load samples from Votrax subdirectory
Votrax_Stop          - End emulation, free memory used for samples
Votrax_PutByte       - Write data to votrax port
Votrax_GetStatus     - Return busy status (1 = busy)

**************************************************************************/

#ifdef PBI_DEBUG
#define VERBOSE 1
#endif

#if VERBOSE
#define LOG(x) printf x
#else
#define LOG(x)
#endif

#include "votrax.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

static struct {
	int busy;

	int actPhoneme;
	int actIntonation;

	struct Votrax_interface *intf;

	SWORD* pActPos;
	int	iRemainingSamples;

	SWORD *lpBuffer;
	SWORD* pBufferPos;

	int   iSamplesInBuffer;
	int	  iDelay;  /* a count of samples to output '0' in a Delay state */

} votraxsc01_locals;

#define INT16 SWORD
#define UINT16 UWORD
#include "vtxsmpls.inc"

#if VERBOSE
static const char *PhonemeNames[65] =
{
 "EH3","EH2","EH1","PA0","DT" ,"A2" ,"A1" ,"ZH",
 "AH2","I3" ,"I2" ,"I1" ,"M"  ,"N"  ,"B"  ,"V",
 "CH" ,"SH" ,"Z"  ,"AW1","NG" ,"AH1","OO1","OO",
 "L"  ,"K"  ,"J"  ,"H"  ,"G"  ,"F"  ,"D"  ,"S",
 "A"  ,"AY" ,"Y1" ,"UH3","AH" ,"P"  ,"O"  ,"I",
 "U"  ,"Y"  ,"T"  ,"R"  ,"E"  ,"W"  ,"AE" ,"AE1",
 "AW2","UH2","UH1","UH" ,"O2" ,"O1" ,"IU" ,"U1",
 "THV","TH" ,"ER" ,"EH" ,"E1" ,"AW" ,"PA1","STOP",
 0
};
#endif

/* phoneme types*/
#define PT_NS 0
#define PT_V  1
#define PT_VF 2
#define PT_F  3
#define PT_N  4
#define PT_VS 5
#define PT_FS 6


static int sample_rate[4] = {22050, 22050, 22050, 22050};

/* converts milliseconds to a count of samples */
static int time_to_samples(int ms)
{
	return sample_rate[votraxsc01_locals.actIntonation]*ms/1000;
}

static void PrepareVoiceData(int nextPhoneme, int nextIntonation)
{
	int iNextRemainingSamples;
	SWORD *pNextPos, *lpHelp;

	int iFadeOutSamples;
	int iFadeOutPos;
	
	int iFadeInSamples;
	int iFadeInPos;

	int doMix;
	/* used only for SecondStart phonemes */
	int AdditionalSamples;
	/* dwCount is the length of samples to produce in ms from iLengthms */
	int dwCount, i;

	SWORD data;

	AdditionalSamples = 0;
	/* some phonenemes have a SecondStart */
	if ( PhonemeData[votraxsc01_locals.actPhoneme].iType>=PT_VS && votraxsc01_locals.actPhoneme!=nextPhoneme ) {
		AdditionalSamples = PhonemeData[votraxsc01_locals.actPhoneme].iSecondStart;
	}

	if ( PhonemeData[nextPhoneme].iType>=PT_VS ) {
		/* 'stop phonemes' will stop playing until the next phoneme is sent*/
		votraxsc01_locals.iRemainingSamples = 0;
		return;
	}

	/* length of samples to produce*/
	dwCount = time_to_samples(PhonemeData[nextPhoneme].iLengthms);

	votraxsc01_locals.iSamplesInBuffer = dwCount+AdditionalSamples;

	if ( AdditionalSamples )
		memcpy(votraxsc01_locals.lpBuffer, PhonemeData[votraxsc01_locals.actPhoneme].lpStart[votraxsc01_locals.actIntonation], AdditionalSamples*sizeof(SWORD));

	lpHelp = votraxsc01_locals.lpBuffer + AdditionalSamples;

	iNextRemainingSamples = 0;
	pNextPos = NULL;

	iFadeOutSamples = 0;
	iFadeOutPos     = 0;
	
	iFadeInSamples   = 0;
	iFadeInPos       = 0;

	doMix = 0;

	/* set up processing*/
	if ( PhonemeData[votraxsc01_locals.actPhoneme].sameAs!=PhonemeData[nextPhoneme].sameAs  ) {
		/* do something, if they are the same all FadeIn/Out values are 0, */
		/* the buffer is simply filled with the samples of the new phoneme */

		switch ( PhonemeData[votraxsc01_locals.actPhoneme].iType ) {
			case PT_NS:
				/* "fade" out NS:*/
				iFadeOutSamples = time_to_samples(30);
				iFadeOutPos = 0;

				/* fade in new phoneme*/
				iFadeInPos = -time_to_samples(30);
				iFadeInSamples = time_to_samples(30);
				break;

			case PT_V:
			case PT_VF:
				switch ( PhonemeData[nextPhoneme].iType ){
					case PT_F:
					case PT_VF:
						/* V-->F, V-->VF: fade out 30 ms fade in from 30 ms to 60 ms without mixing*/
						iFadeOutPos = 0;
						iFadeOutSamples = time_to_samples(30);

						iFadeInPos = -time_to_samples(30);
						iFadeInSamples = time_to_samples(30);
						break;

					case PT_N:
						/* V-->N: fade out 40 ms fade from 0 ms to 40 ms without mixing*/
						iFadeOutPos = 0;
						iFadeOutSamples = time_to_samples(40);

						iFadeInPos = -time_to_samples(10);
						iFadeInSamples = time_to_samples(10);
						break;

					default:
						/* fade out 20 ms, no fade in from 10 ms to 30 ms*/
						iFadeOutPos = 0;
						iFadeOutSamples = time_to_samples(20);

						iFadeInPos = -time_to_samples(0);
						iFadeInSamples = time_to_samples(20);
						break;
				}
				break;

			case PT_N:
				switch ( PhonemeData[nextPhoneme].iType ){
					case PT_V:
					case PT_VF:
						/* N-->V, N-->VF: fade out 30 ms fade in from 10 ms to 50 ms without mixing*/
						iFadeOutPos = 0;
						iFadeOutSamples = time_to_samples(30);

						iFadeInPos = -time_to_samples(10);
						iFadeInSamples = time_to_samples(40);
						break;

					default:
						break;
				}

			case PT_VS:
			case PT_FS:
				iFadeOutPos = 0;
				iFadeOutSamples = PhonemeData[votraxsc01_locals.actPhoneme].iLength[votraxsc01_locals.actIntonation] - PhonemeData[votraxsc01_locals.actPhoneme].iSecondStart;
				votraxsc01_locals.pActPos = PhonemeData[votraxsc01_locals.actPhoneme].lpStart[votraxsc01_locals.actIntonation] + PhonemeData[votraxsc01_locals.actPhoneme].iSecondStart;
				votraxsc01_locals.iRemainingSamples = iFadeOutSamples;
				doMix = 1;

				iFadeInPos = -time_to_samples(0);
				iFadeInSamples = time_to_samples(0);

				break;

			default:
				/* fade out 30 ms, no fade in*/
				iFadeOutPos = 0;
				iFadeOutSamples = time_to_samples(20);

				iFadeInPos = -time_to_samples(20);
				break;
		}

		if ( !votraxsc01_locals.iDelay ) {
			/* this is true if after a stop and a phoneme was sent a second phoneme is sent*/
			/* during the delay time of the chip. Ignore the first phoneme data*/
			iFadeOutPos = 0;
			iFadeOutSamples = 0;
		}

	}
	else {
		/* the next one is of the same type as the previous one; continue to use the samples of the last phoneme*/
		iNextRemainingSamples = votraxsc01_locals.iRemainingSamples;
		pNextPos = votraxsc01_locals.pActPos;
	}

	for (i=0; i<dwCount; i++)
	{
		data = 0x00;

		/* fade out*/
		if ( iFadeOutPos<iFadeOutSamples ) 
		{
			double dFadeOut = 1.0;

			if ( !doMix )
				dFadeOut = 1.0-sin((1.0*iFadeOutPos/iFadeOutSamples)*3.1415/2);

			if ( !votraxsc01_locals.iRemainingSamples ) {
				votraxsc01_locals.iRemainingSamples = PhonemeData[votraxsc01_locals.actPhoneme].iLength[votraxsc01_locals.actIntonation];
				votraxsc01_locals.pActPos = PhonemeData[votraxsc01_locals.actPhoneme].lpStart[votraxsc01_locals.actIntonation];
			}

			data = (SWORD) (*votraxsc01_locals.pActPos++ * dFadeOut);

			votraxsc01_locals.iRemainingSamples--;
			iFadeOutPos++;
		}

		/* fade in or copy*/
		if ( iFadeInPos>=0 )
		{
			double dFadeIn = 1.0;
			
			if ( iFadeInPos<iFadeInSamples ) {
				dFadeIn = sin((1.0*iFadeInPos/iFadeInSamples)*3.1415/2);
				iFadeInPos++;
			}

			if ( !iNextRemainingSamples ) {
				iNextRemainingSamples = PhonemeData[nextPhoneme].iLength[nextIntonation];
				pNextPos = PhonemeData[nextPhoneme].lpStart[nextIntonation];
			}

			data += (SWORD) (*pNextPos++ * dFadeIn);
			
			iNextRemainingSamples--;
		}
		iFadeInPos++;

		*lpHelp++ = data;
	}

	votraxsc01_locals.pBufferPos = votraxsc01_locals.lpBuffer;

	votraxsc01_locals.pActPos = pNextPos;
	votraxsc01_locals.iRemainingSamples = iNextRemainingSamples;
}

void Votrax_PutByte(UBYTE data)
{
	int Phoneme, Intonation;

	Phoneme = data & 0x3F;
	Intonation = (data >> 6)&0x03;

#ifdef VERBOSE
	if (!votraxsc01_locals.intf) {
		LOG(("Error: votraxsc01_locals.intf not set"));
		return;
	}
#endif /* VERBOSE */
	LOG(("Votrax SC-01: %s at intonation %d\n", PhonemeNames[Phoneme], Intonation));
	PrepareVoiceData(Phoneme, Intonation);

	if ( votraxsc01_locals.actPhoneme==0x3f )
		votraxsc01_locals.iDelay = time_to_samples(20);
		
	if ( !votraxsc01_locals.busy ) 
	{
		votraxsc01_locals.busy = 1;
		if ( votraxsc01_locals.intf->BusyCallback )
			(*votraxsc01_locals.intf->BusyCallback)(votraxsc01_locals.busy);
	}

	votraxsc01_locals.actPhoneme = Phoneme;
	votraxsc01_locals.actIntonation = Intonation;
}

UBYTE Votrax_GetStatus(void)
{
	return votraxsc01_locals.busy;
}

void Votrax_Update(int num, SWORD *buffer, int length)
{
	int samplesToCopy;

#if 0
	/* if it is a different intonation */
	if ( num!=votraxsc01_locals.actIntonation ) {
		/* clear buffer */
		memset(buffer, 0x00, length*sizeof(SWORD));
		return;
	}
#endif

	while ( length ) {
		/* Case 1: if in a delay state, output 0's*/
		if ( votraxsc01_locals.iDelay ) {
			samplesToCopy = (length<=votraxsc01_locals.iDelay)?length:votraxsc01_locals.iDelay;

			memset(buffer, 0x00, samplesToCopy*sizeof(SWORD));
			buffer += samplesToCopy;

			votraxsc01_locals.iDelay -= samplesToCopy;
			length -= samplesToCopy; /* missing in the original */
		}
		/* Case 2: there are no samples left in the buffer */
		else if ( votraxsc01_locals.iSamplesInBuffer==0 ) {
			if ( votraxsc01_locals.busy ) {
				/* busy -> idle */
				votraxsc01_locals.busy = 0;
				if ( votraxsc01_locals.intf->BusyCallback )
					(*votraxsc01_locals.intf->BusyCallback)(votraxsc01_locals.busy);
			}

			if ( votraxsc01_locals.iRemainingSamples==0 ) {
				if ( PhonemeData[votraxsc01_locals.actPhoneme].iType>=PT_VS ) {
					votraxsc01_locals.pActPos = PhonemeData[0x3f].lpStart[0];
					votraxsc01_locals.iRemainingSamples = PhonemeData[0x3f].iLength[0];
				}
				else {
					votraxsc01_locals.pActPos = PhonemeData[votraxsc01_locals.actPhoneme].lpStart[votraxsc01_locals.actIntonation];
					votraxsc01_locals.iRemainingSamples = PhonemeData[votraxsc01_locals.actPhoneme].iLength[votraxsc01_locals.actIntonation];
				}

			}

			/* if there aren't enough remaining, reduce the amount */
			samplesToCopy = (length<=votraxsc01_locals.iRemainingSamples)?length:votraxsc01_locals.iRemainingSamples;

			memcpy(buffer, votraxsc01_locals.pActPos, samplesToCopy*sizeof(SWORD));
			buffer += samplesToCopy;

			votraxsc01_locals.pActPos += samplesToCopy;
			votraxsc01_locals.iRemainingSamples -= samplesToCopy;

			length -= samplesToCopy;
		}
		/* Case 3: output the samples in the buffer */
		else {
			samplesToCopy = (length<=votraxsc01_locals.iSamplesInBuffer)?length:votraxsc01_locals.iSamplesInBuffer;

			memcpy(buffer, votraxsc01_locals.pBufferPos, samplesToCopy*sizeof(SWORD));
			buffer += samplesToCopy;

			votraxsc01_locals.pBufferPos += samplesToCopy;
			votraxsc01_locals.iSamplesInBuffer -= samplesToCopy;

			length -= samplesToCopy;
		}
	}
}

int Votrax_Start(void *sound_interface)
{
	int i, buffer_size;
	/* clear local variables */
	memset(&votraxsc01_locals, 0x00, sizeof votraxsc01_locals);

	/* copy interface */
	votraxsc01_locals.intf = (struct Votrax_interface *)sound_interface;

	votraxsc01_locals.actPhoneme = 0x3f;

	/* find the largest possible size of iSamplesInBuffer */
	buffer_size = 0;
	for (i = 0; i <= 0x3f; i++) {
		int dwCount;
		int size;
		int AdditionalSamples;
		AdditionalSamples = PhonemeData[i].iSecondStart;
		dwCount = time_to_samples(PhonemeData[i].iLengthms);
		size = dwCount + AdditionalSamples;
		if (size > buffer_size)  buffer_size = size;
	}
	votraxsc01_locals.lpBuffer = (SWORD*) Util_malloc(buffer_size*sizeof(SWORD));
	PrepareVoiceData(votraxsc01_locals.actPhoneme, votraxsc01_locals.actIntonation);
	return 0;
}

void Votrax_Stop(void)
{
	if ( votraxsc01_locals.lpBuffer ) {
		free(votraxsc01_locals.lpBuffer);
		votraxsc01_locals.lpBuffer = NULL;
	}
}

int Votrax_Samples(int currentP, int nextP, int cursamples)
{
	int AdditionalSamples = 0;
	int dwCount;
	int delay = 0;
	/* some phonemes have a SecondStart */
	if ( PhonemeData[currentP].iType>=PT_VS && currentP!=nextP) {
		AdditionalSamples = PhonemeData[currentP].iSecondStart;
	}

	if ( PhonemeData[nextP].iType>=PT_VS ) {
		/* 'stop phonemes' will stop playing until the next phoneme is sent*/
		/* votraxsc01_locals.iRemainingSamples = 0; */
		return cursamples;
	}
	if (currentP == 0x3f) delay = time_to_samples(20);

	/* length of samples to produce*/
	dwCount = time_to_samples(PhonemeData[nextP].iLengthms);
	return dwCount + AdditionalSamples + delay ;
}

/*
vim:ts=4:sw=4:
*/
