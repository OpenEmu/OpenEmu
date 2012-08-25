#include "burnint.h"
#include "burn_sound.h"
#include "sn76496.h"

#define MAX_SN76496_CHIPS 5

#define MAX_OUTPUT 0x7fff

#define STEP 0x10000

struct SN76496
{
	UINT32 UpdateStep;
	INT32 VolTable[16];	/* volume table         */
	INT32 Register[8];	/* registers */
	INT32 LastRegister;	/* last register written */
	INT32 Volume[4];	/* volume of voice 0-2 and noise */
	INT32 RNG;		/* noise generator      */
	INT32 NoiseMode;	/* active noise mode */
	INT32 FeedbackMask;     /* mask for feedback */
	INT32 WhitenoiseTaps;   /* mask for white noise taps */
	INT32 WhitenoiseInvert; /* white noise invert flag */
	INT32 Period[4];
	INT32 Count[4];
	INT32 Output[4];
	INT32 bSignalAdd;
	double nVolume;
	INT32 nOutputDir;
};

static INT32 NumChips;
static struct SN76496 *Chip0 = NULL;
static struct SN76496 *Chip1 = NULL;
static struct SN76496 *Chip2 = NULL;
static struct SN76496 *Chip3 = NULL;
static struct SN76496 *Chip4 = NULL;

void SN76496Update(INT32 Num, INT16* pSoundBuf, INT32 Length)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SN76496Initted) bprintf(PRINT_ERROR, _T("SN76496Update called without init\n"));
	if (Num > NumChips) bprintf(PRINT_ERROR, _T("SN76496Update called with invalid chip %x\n"), Num);
#endif

	INT32 i;
	struct SN76496 *R = Chip0;
	
	if (Num >= MAX_SN76496_CHIPS) return;
	
	if (Num == 1) R = Chip1;
	if (Num == 2) R = Chip2;
	if (Num == 3) R = Chip3;
	if (Num == 4) R = Chip4;
	
	/* If the volume is 0, increase the counter */
	for (i = 0;i < 4;i++)
	{
		if (R->Volume[i] == 0)
		{
			/* note that I do count += length, NOT count = length + 1. You might think */
			/* it's the same since the volume is 0, but doing the latter could cause */
			/* interferencies when the program is rapidly modulating the volume. */
			if (R->Count[i] <= Length*STEP) R->Count[i] += Length*STEP;
		}
	}

	while (Length > 0)
	{
		INT32 Vol[4];
		UINT32 Out;
		INT32 Left;


		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		Vol[0] = Vol[1] = Vol[2] = Vol[3] = 0;

		for (i = 0;i < 3;i++)
		{
			if (R->Output[i]) Vol[i] += R->Count[i];
			R->Count[i] -= STEP;
			/* Period[i] is the half period of the square wave. Here, in each */
			/* loop I add Period[i] twice, so that at the end of the loop the */
			/* square wave is in the same status (0 or 1) it was at the start. */
			/* vol[i] is also incremented by Period[i], since the wave has been 1 */
			/* exactly half of the time, regardless of the initial position. */
			/* If we exit the loop in the middle, Output[i] has to be inverted */
			/* and vol[i] incremented only if the exit status of the square */
			/* wave is 1. */
			while (R->Count[i] <= 0)
			{
				R->Count[i] += R->Period[i];
				if (R->Count[i] > 0)
				{
					R->Output[i] ^= 1;
					if (R->Output[i]) Vol[i] += R->Period[i];
					break;
				}
				R->Count[i] += R->Period[i];
				Vol[i] += R->Period[i];
			}
			if (R->Output[i]) Vol[i] -= R->Count[i];
		}

		Left = STEP;
		do
		{
			INT32 NextEvent;


			if (R->Count[3] < Left) NextEvent = R->Count[3];
			else NextEvent = Left;

			if (R->Output[3]) Vol[3] += R->Count[3];
			R->Count[3] -= NextEvent;
			if (R->Count[3] <= 0)
			{
		        if (R->NoiseMode == 1) /* White Noise Mode */
		        {
			        if (((R->RNG & R->WhitenoiseTaps) != R->WhitenoiseTaps) && ((R->RNG & R->WhitenoiseTaps) != 0)) /* crappy xor! */
					{
				        R->RNG >>= 1;
				        R->RNG |= R->FeedbackMask;
					}
					else
					{
				        R->RNG >>= 1;
					}
					R->Output[3] = R->WhitenoiseInvert ? !(R->RNG & 1) : R->RNG & 1;
				}
				else /* Periodic noise mode */
				{
			        if (R->RNG & 1)
					{
				        R->RNG >>= 1;
				        R->RNG |= R->FeedbackMask;
					}
					else
					{
				        R->RNG >>= 1;
					}
					R->Output[3] = R->RNG & 1;
				}
				R->Count[3] += R->Period[3];
				if (R->Output[3]) Vol[3] += R->Period[3];
			}
			if (R->Output[3]) Vol[3] -= R->Count[3];

			Left -= NextEvent;
		} while (Left > 0);

		Out = Vol[0] * R->Volume[0] + Vol[1] * R->Volume[1] +
				Vol[2] * R->Volume[2] + Vol[3] * R->Volume[3];

		if (Out > MAX_OUTPUT * STEP) Out = MAX_OUTPUT * STEP;

		Out /= STEP;
		
		INT32 nLeftSample = 0, nRightSample = 0;
		if ((R->nOutputDir & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(Out * R->nVolume);
		}
		if ((R->nOutputDir & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(Out * R->nVolume);
		}
		
		if (R->bSignalAdd) {
			pSoundBuf[0] = BURN_SND_CLIP(pSoundBuf[0] + nLeftSample);
			pSoundBuf[1] = BURN_SND_CLIP(pSoundBuf[1] + nRightSample);
		} else {
			pSoundBuf[0] = BURN_SND_CLIP(nLeftSample);
			pSoundBuf[1] = BURN_SND_CLIP(nRightSample);
		}
		
		pSoundBuf += 2;
		Length--;
	}
}

void SN76496Write(INT32 Num, INT32 Data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SN76496Initted) bprintf(PRINT_ERROR, _T("SN76496Write called without init\n"));
	if (Num > NumChips) bprintf(PRINT_ERROR, _T("SN76496Write called with invalid chip %x\n"), Num);
#endif

	struct SN76496 *R = Chip0;
	INT32 n, r, c;
	
	if (Num >= MAX_SN76496_CHIPS) return;
	
	if (Num == 1) R = Chip1;
	if (Num == 2) R = Chip2;
	if (Num == 3) R = Chip3;
	if (Num == 4) R = Chip4;
	
	if (Data & 0x80) {
		r = (Data & 0x70) >> 4;
		R->LastRegister = r;
		R->Register[r] = (R->Register[r] & 0x3f0) | (Data & 0x0f);
	} else {
		r = R->LastRegister;
	}
	
	c = r / 2;

	switch (r)
	{
		case 0:	/* tone 0 : frequency */
		case 2:	/* tone 1 : frequency */
		case 4:	/* tone 2 : frequency */
		    if ((Data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x0f) | ((Data & 0x3f) << 4);
			R->Period[c] = R->UpdateStep * R->Register[r];
			if (R->Period[c] == 0) R->Period[c] = R->UpdateStep;
			if (r == 4)
			{
				/* update noise shift frequency */
				if ((R->Register[6] & 0x03) == 0x03)
					R->Period[3] = 2 * R->Period[2];
			}
			break;
		case 1:	/* tone 0 : volume */
		case 3:	/* tone 1 : volume */
		case 5:	/* tone 2 : volume */
		case 7:	/* noise  : volume */
			R->Volume[c] = R->VolTable[Data & 0x0f];
			if ((Data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x3f0) | (Data & 0x0f);
			break;
		case 6:	/* noise  : frequency, mode */
			{
			        if ((Data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x3f0) | (Data & 0x0f);
				n = R->Register[6];
				R->NoiseMode = (n & 4) ? 1 : 0;
				/* N/512,N/1024,N/2048,Tone #3 output */
				R->Period[3] = ((n&3) == 3) ? 2 * R->Period[2] : (R->UpdateStep << (5+(n&3)));
			        /* Reset noise shifter */
				R->RNG = R->FeedbackMask; /* this is correct according to the smspower document */
				//R->RNG = 0xF35; /* this is not, but sounds better in do run run */
				R->Output[3] = R->RNG & 1;
			}
			break;
	}
}

static void SN76496SetGain(struct SN76496 *R,INT32 Gain)
{
	INT32 i;
	double Out;

	Gain &= 0xff;

	/* increase max output basing on gain (0.2 dB per step) */
	Out = MAX_OUTPUT / 3;
	while (Gain-- > 0)
		Out *= 1.023292992;	/* = (10 ^ (0.2/20)) */

	/* build volume table (2dB per step) */
	for (i = 0;i < 15;i++)
	{
		/* limit volume to avoid clipping */
		if (Out > MAX_OUTPUT / 3) R->VolTable[i] = MAX_OUTPUT / 3;
		else R->VolTable[i] = (INT32)Out;

		Out /= 1.258925412;	/* = 10 ^ (2/20) = 2dB */
	}
	R->VolTable[15] = 0;
}

static void SN76496Init(struct SN76496 *R, INT32 Clock)
{
	INT32 i;
	
	R->UpdateStep = (UINT32)(((double)STEP * nBurnSoundRate * 16) / Clock);
		
	for (i = 0; i < 4; i++) R->Volume[i] = 0;
	
	R->LastRegister = 0;
	for (i = 0; i < 8; i += 2) {
		R->Register[i + 0] = 0x00;
		R->Register[i + 1] = 0x0f;
	}
	
	for (i = 0; i < 4; i++) {
		R->Output[i] = 0;
		R->Period[i] = R->Count[i] = R->UpdateStep;
	}
	
	R->FeedbackMask = 0x4000;
	R->WhitenoiseTaps = 0x03;
	R->WhitenoiseInvert = 1;
	
	R->RNG = R->FeedbackMask;
	R->Output[3] = R->RNG & 1;
}

static void GenericStart(INT32 Num, INT32 Clock, INT32 FeedbackMask, INT32 NoiseTaps, INT32 NoiseInvert, INT32 SignalAdd)
{
	DebugSnd_SN76496Initted = 1;
	
	if (Num >= MAX_SN76496_CHIPS) return;
	
	NumChips = Num;
	
	if (Num == 0) {
		Chip0 = (struct SN76496*)BurnMalloc(sizeof(*Chip0));
		memset(Chip0, 0, sizeof(*Chip0));
	
		SN76496Init(Chip0, Clock);
		SN76496SetGain(Chip0, 0);
		
		Chip0->FeedbackMask = FeedbackMask;
		Chip0->WhitenoiseTaps = NoiseTaps;
		Chip0->WhitenoiseInvert = NoiseInvert;
		Chip0->bSignalAdd = SignalAdd;
		Chip0->nVolume = 1.00;
		Chip0->nOutputDir = BURN_SND_ROUTE_BOTH;
	}
	
	if (Num == 1) {
		Chip1 = (struct SN76496*)BurnMalloc(sizeof(*Chip1));
		memset(Chip1, 0, sizeof(*Chip1));
	
		SN76496Init(Chip1, Clock);
		SN76496SetGain(Chip1, 0);
		
		Chip1->FeedbackMask = FeedbackMask;
		Chip1->WhitenoiseTaps = NoiseTaps;
		Chip1->WhitenoiseInvert = NoiseInvert;
		Chip1->bSignalAdd = SignalAdd;
		Chip1->nVolume = 1.00;
		Chip1->nOutputDir = BURN_SND_ROUTE_BOTH;
	}
	
	if (Num == 2) {
		Chip2 = (struct SN76496*)BurnMalloc(sizeof(*Chip2));
		memset(Chip2, 0, sizeof(*Chip2));
	
		SN76496Init(Chip2, Clock);
		SN76496SetGain(Chip2, 0);
		
		Chip2->FeedbackMask = FeedbackMask;
		Chip2->WhitenoiseTaps = NoiseTaps;
		Chip2->WhitenoiseInvert = NoiseInvert;
		Chip2->bSignalAdd = SignalAdd;
		Chip2->nVolume = 1.00;
		Chip2->nOutputDir = BURN_SND_ROUTE_BOTH;
	}
	
	if (Num == 3) {
		Chip3 = (struct SN76496*)BurnMalloc(sizeof(*Chip3));
		memset(Chip3, 0, sizeof(*Chip3));
	
		SN76496Init(Chip3, Clock);
		SN76496SetGain(Chip3, 0);
		
		Chip3->FeedbackMask = FeedbackMask;
		Chip3->WhitenoiseTaps = NoiseTaps;
		Chip3->WhitenoiseInvert = NoiseInvert;
		Chip3->bSignalAdd = SignalAdd;
		Chip3->nVolume = 1.00;
		Chip3->nOutputDir = BURN_SND_ROUTE_BOTH;
	}
	
	if (Num == 4) {
		Chip4 = (struct SN76496*)BurnMalloc(sizeof(*Chip4));
		memset(Chip4, 0, sizeof(*Chip4));
	
		SN76496Init(Chip4, Clock);
		SN76496SetGain(Chip4, 0);
		
		Chip4->FeedbackMask = FeedbackMask;
		Chip4->WhitenoiseTaps = NoiseTaps;
		Chip4->WhitenoiseInvert = NoiseInvert;
		Chip4->bSignalAdd = SignalAdd;
		Chip4->nVolume = 1.00;
		Chip4->nOutputDir = BURN_SND_ROUTE_BOTH;
	}
}

void SN76489Init(INT32 Num, INT32 Clock, INT32 SignalAdd)
{
	return GenericStart(Num, Clock, 0x4000, 0x03, 1, SignalAdd);
}

void SN76489AInit(INT32 Num, INT32 Clock, INT32 SignalAdd)
{
	return GenericStart(Num, Clock, 0x8000, 0x06, 0, SignalAdd);
}

void SN76494Init(INT32 Num, INT32 Clock, INT32 SignalAdd)
{
	return GenericStart(Num, Clock, 0x8000, 0x06, 0, SignalAdd);
}

void SN76496Init(INT32 Num, INT32 Clock, INT32 SignalAdd)
{
	return GenericStart(Num, Clock, 0x8000, 0x06, 0, SignalAdd);
}

void SN76496SetRoute(INT32 Num, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SN76496Initted) bprintf(PRINT_ERROR, _T("SN76496SetRoute called without init\n"));
	if (Num > NumChips) bprintf(PRINT_ERROR, _T("SN76496SetRoute called with invalid chip %i\n"), Num);
#endif

	struct SN76496 *R = Chip0;
	if (Num >= MAX_SN76496_CHIPS) return;
	if (Num == 1) R = Chip1;
	if (Num == 2) R = Chip2;
	if (Num == 3) R = Chip3;
	if (Num == 4) R = Chip4;
	
	R->nVolume = nVolume;
	R->nOutputDir = nRouteDir;
}

void SN76496Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_SN76496Initted) bprintf(PRINT_ERROR, _T("SN76496Exit called without init\n"));
#endif

	NumChips = 0;
	
	BurnFree(Chip0);
	BurnFree(Chip1);
	BurnFree(Chip2);
	BurnFree(Chip3);
	BurnFree(Chip4);
	
	DebugSnd_SN76496Initted = 0;
}

INT32 SN76496Scan(INT32 nAction,INT32 *pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SN76496Initted) bprintf(PRINT_ERROR, _T("SN76496Scan called without init\n"));
#endif

	char szName[16];
	
	if (pnMin != NULL) {
		*pnMin = 0x029719;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		for (INT32 i = 0; i < NumChips; i++) {
			SN76496 *Chip = Chip0;
			if (i == 1) Chip = Chip1;
			if (i == 2) Chip = Chip2;
			if (i == 3) Chip = Chip3;
			if (i == 4) Chip = Chip4;
			
			memset(szName, 0, 16);
			sprintf(szName, "Chip%iVolTable", i);
			ScanVar(Chip->VolTable, 16 * sizeof(INT32), szName);
			memset(szName, 0, 16);
			sprintf(szName, "Chip%iRegisters", i);
			ScanVar(Chip->Register, 8 * sizeof(INT32), szName);
			memset(szName, 0, 16);
			sprintf(szName, "Chip%iVolume", i);
			ScanVar(Chip->Volume, 4 * sizeof(INT32), szName);
			memset(szName, 0, 16);
			sprintf(szName, "Chip%iPeriod", i);
			ScanVar(Chip->Period, 4 * sizeof(INT32), szName);
			memset(szName, 0, 16);
			sprintf(szName, "Chip%iCount", i);
			ScanVar(Chip->Count, 4 * sizeof(INT32), szName);
			memset(szName, 0, 16);
			sprintf(szName, "Chip%iOutput", i);
			ScanVar(Chip->Output, 4 * sizeof(INT32), szName);
			
			SCAN_VAR(Chip->LastRegister);
			SCAN_VAR(Chip->RNG);
			SCAN_VAR(Chip->NoiseMode);
			SCAN_VAR(Chip->FeedbackMask);
			SCAN_VAR(Chip->WhitenoiseTaps);
			SCAN_VAR(Chip->WhitenoiseInvert);
		}
	}
	
	return 0;
}

#undef MAX_SN76496_CHIPS
#undef MAX_OUTPUT
#undef STEP
