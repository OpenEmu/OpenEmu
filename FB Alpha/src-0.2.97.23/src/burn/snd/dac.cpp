#include "burnint.h"
#include "burn_sound.h"

#define DEFAULT_SAMPLE_RATE (48000 * 4)

struct dac_info
{
	INT16			Output;
	INT16			UnsignedVolTable[256];
	INT16			SignedVolTable[256];
	UINT32 			Delta;
	INT32 			nVolShift;
};

static INT32 NumChips;
static struct dac_info *Chip0 = NULL;
static struct dac_info *Chip1 = NULL;
static struct dac_info *Chip2 = NULL;
static struct dac_info *Chip3 = NULL;
static struct dac_info *Chip4 = NULL;
static struct dac_info *Chip5 = NULL;
static struct dac_info *Chip6 = NULL;
static struct dac_info *Chip7 = NULL;

static INT32 bAddSignal;

void DACUpdate(INT16* Buffer, INT32 Length)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACUpdate called without init\n"));
#endif

	INT32 Out = Chip0->Output;
	
	if (NumChips >= 2) Out += Chip1->Output;
	if (NumChips >= 3) Out += Chip2->Output;
	if (NumChips >= 4) Out += Chip3->Output;
	if (NumChips >= 5) Out += Chip4->Output;
	if (NumChips >= 6) Out += Chip5->Output;
	if (NumChips >= 7) Out += Chip6->Output;
	if (NumChips >= 8) Out += Chip7->Output;
	
	Out = BURN_SND_CLIP(Out);
	
	while (Length--) {
		if (bAddSignal) {
			Buffer[0] += Out;
			Buffer[1] += Out;
		} else {
			Buffer[0] = Out;
			Buffer[1] = Out;
		}
		Buffer += 2;
	}
}

void DACWrite(INT32 Chip, UINT8 Data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACWrite called without init\n"));
	if (Chip > NumChips) bprintf(PRINT_ERROR, _T("DACWrite called with invalid chip number %x\n"), Chip);
#endif

	if (Chip == 0) Chip0->Output = Chip0->UnsignedVolTable[Data] >> Chip0->nVolShift;
	if (Chip == 1) Chip1->Output = Chip1->UnsignedVolTable[Data] >> Chip1->nVolShift;
	if (Chip == 2) Chip2->Output = Chip2->UnsignedVolTable[Data] >> Chip2->nVolShift;
	if (Chip == 3) Chip3->Output = Chip3->UnsignedVolTable[Data] >> Chip3->nVolShift;
	if (Chip == 4) Chip4->Output = Chip4->UnsignedVolTable[Data] >> Chip4->nVolShift;
	if (Chip == 5) Chip5->Output = Chip5->UnsignedVolTable[Data] >> Chip5->nVolShift;
	if (Chip == 6) Chip6->Output = Chip6->UnsignedVolTable[Data] >> Chip6->nVolShift;
	if (Chip == 7) Chip7->Output = Chip7->UnsignedVolTable[Data] >> Chip7->nVolShift;
}

void DACSignedWrite(INT32 Chip, UINT8 Data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACSignedWrite called without init\n"));
	if (Chip > NumChips) bprintf(PRINT_ERROR, _T("DACSignedWrite called with invalid chip number %x\n"), Chip);
#endif

	if (Chip == 0) Chip0->Output = Chip0->SignedVolTable[Data] >> Chip0->nVolShift;
	if (Chip == 1) Chip1->Output = Chip1->SignedVolTable[Data] >> Chip1->nVolShift;
	if (Chip == 2) Chip2->Output = Chip2->SignedVolTable[Data] >> Chip2->nVolShift;
	if (Chip == 3) Chip3->Output = Chip3->SignedVolTable[Data] >> Chip3->nVolShift;
	if (Chip == 4) Chip4->Output = Chip4->SignedVolTable[Data] >> Chip4->nVolShift;
	if (Chip == 5) Chip5->Output = Chip5->SignedVolTable[Data] >> Chip5->nVolShift;
	if (Chip == 6) Chip6->Output = Chip6->SignedVolTable[Data] >> Chip6->nVolShift;
	if (Chip == 7) Chip7->Output = Chip7->SignedVolTable[Data] >> Chip7->nVolShift;
}

static void DACBuildVolTables()
{
	INT32 i;
	
	for (i = 0;i < 256;i++) {
		Chip0->UnsignedVolTable[i] = i * 0x101 / 2;
		Chip0->SignedVolTable[i] = i * 0x101 - 0x8000;
		
		if (NumChips >= 2) {
			Chip1->UnsignedVolTable[i] = i * 0x101 / 2;
			Chip1->SignedVolTable[i] = i * 0x101 - 0x8000;
		}
		
		if (NumChips >= 3) {
			Chip2->UnsignedVolTable[i] = i * 0x101 / 2;
			Chip2->SignedVolTable[i] = i * 0x101 - 0x8000;
		}
		
		if (NumChips >= 4) {
			Chip3->UnsignedVolTable[i] = i * 0x101 / 2;
			Chip3->SignedVolTable[i] = i * 0x101 - 0x8000;
		}
		
		if (NumChips >= 5) {
			Chip4->UnsignedVolTable[i] = i * 0x101 / 2;
			Chip4->SignedVolTable[i] = i * 0x101 - 0x8000;
		}
		
		if (NumChips >= 6) {
			Chip5->UnsignedVolTable[i] = i * 0x101 / 2;
			Chip5->SignedVolTable[i] = i * 0x101 - 0x8000;
		}
		
		if (NumChips >= 7) {
			Chip6->UnsignedVolTable[i] = i * 0x101 / 2;
			Chip6->SignedVolTable[i] = i * 0x101 - 0x8000;
		}
		
		if (NumChips >= 8) {
			Chip7->UnsignedVolTable[i] = i * 0x101 / 2;
			Chip7->SignedVolTable[i] = i * 0x101 - 0x8000;
		}
	}
}

void DACInit(INT32 Num, UINT32 Clock, INT32 bAdd)
{
	DebugSnd_DACInitted = 1;
	
	NumChips = Num;
	
	if (Num == 0) {
		Chip0 = (struct dac_info*)malloc(sizeof(struct dac_info));
		memset(Chip0, 0, sizeof(Chip0));	
		Chip0->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
		Chip0->Output = 0;
		Chip0->nVolShift = 0;
	}
	
	if (Num == 1) {
		Chip1 = (struct dac_info*)malloc(sizeof(struct dac_info));
		memset(Chip1, 0, sizeof(Chip1));	
		Chip1->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
		Chip1->Output = 0;
		Chip1->nVolShift = 0;
	}
	
	if (Num == 2) {
		Chip2 = (struct dac_info*)malloc(sizeof(struct dac_info));
		memset(Chip2, 0, sizeof(Chip2));	
		Chip2->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
		Chip2->Output = 0;
		Chip2->nVolShift = 0;
	}
	
	if (Num == 3) {
		Chip3 = (struct dac_info*)malloc(sizeof(struct dac_info));
		memset(Chip3, 0, sizeof(Chip3));	
		Chip3->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
		Chip3->Output = 0;
		Chip3->nVolShift = 0;
	}
	
	if (Num == 4) {
		Chip4 = (struct dac_info*)malloc(sizeof(struct dac_info));
		memset(Chip4, 0, sizeof(Chip4));	
		Chip4->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
		Chip4->Output = 0;
		Chip4->nVolShift = 0;
	}
	
	if (Num == 5) {
		Chip5 = (struct dac_info*)malloc(sizeof(struct dac_info));
		memset(Chip5, 0, sizeof(Chip5));	
		Chip5->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
		Chip5->Output = 0;
		Chip5->nVolShift = 0;
	}
	
	if (Num == 6) {
		Chip6 = (struct dac_info*)malloc(sizeof(struct dac_info));
		memset(Chip6, 0, sizeof(Chip6));	
		Chip6->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
		Chip6->Output = 0;
		Chip6->nVolShift = 0;
	}
	
	if (Num == 7) {
		Chip7 = (struct dac_info*)malloc(sizeof(struct dac_info));
		memset(Chip7, 0, sizeof(Chip7));	
		Chip7->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
		Chip7->Output = 0;
		Chip7->nVolShift = 0;
	}
	
	DACBuildVolTables();
	
	bAddSignal = bAdd;	
}

void DACSetVolShift(INT32 Chip, INT32 nShift)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACSetVolShift called without init\n"));
	if (Chip > NumChips) bprintf(PRINT_ERROR, _T("DACSetVolShift called with invalid chip number %x\n"), Chip);
#endif

	if (Chip == 0) Chip0->nVolShift = nShift;
	if (Chip == 1) Chip1->nVolShift = nShift;
	if (Chip == 2) Chip2->nVolShift = nShift;
	if (Chip == 3) Chip3->nVolShift = nShift;
	if (Chip == 4) Chip4->nVolShift = nShift;
	if (Chip == 5) Chip5->nVolShift = nShift;
	if (Chip == 6) Chip6->nVolShift = nShift;
	if (Chip == 7) Chip7->nVolShift = nShift;
}

void DACReset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACReset called without init\n"));
#endif

	Chip0->Output = 0;
	if (NumChips >= 2) Chip1->Output = 0;
	if (NumChips >= 3) Chip2->Output = 0;
	if (NumChips >= 4) Chip3->Output = 0;
	if (NumChips >= 5) Chip4->Output = 0;
	if (NumChips >= 6) Chip5->Output = 0;
	if (NumChips >= 7) Chip6->Output = 0;
	if (NumChips >= 8) Chip7->Output = 0;
}

void DACExit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACExit called without init\n"));
#endif

	if (Chip0) {
		free(Chip0);
		Chip0 = NULL;
	}
	
	if (Chip1) {
		free(Chip1);
		Chip1 = NULL;
	}
	
	if (Chip2) {
		free(Chip2);
		Chip2 = NULL;
	}
	
	if (Chip3) {
		free(Chip3);
		Chip3 = NULL;
	}
	
	if (Chip4) {
		free(Chip4);
		Chip4 = NULL;
	}
	
	if (Chip5) {
		free(Chip5);
		Chip5 = NULL;
	}
	
	if (Chip6) {
		free(Chip6);
		Chip6 = NULL;
	}
	
	if (Chip7) {
		free(Chip7);
		Chip7 = NULL;
	}
	
	NumChips = 0;
	
	DebugSnd_DACInitted = 0;
}

INT32 DACScan(INT32 nAction,INT32 *pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACScan called without init\n"));
#endif
	
	if (pnMin != NULL) {
		*pnMin = 0x029719;
	}
	
	if (nAction & ACB_DRIVER_DATA) {	
		ScanVar(&Chip0->Output, sizeof(INT16), "DAC0Output");
		if (NumChips >= 2) ScanVar(&Chip1->Output, sizeof(INT16), "DAC1Output");
		if (NumChips >= 3) ScanVar(&Chip2->Output, sizeof(INT16), "DAC2Output");
		if (NumChips >= 4) ScanVar(&Chip3->Output, sizeof(INT16), "DAC3Output");
		if (NumChips >= 5) ScanVar(&Chip4->Output, sizeof(INT16), "DAC4Output");
		if (NumChips >= 6) ScanVar(&Chip5->Output, sizeof(INT16), "DAC5Output");
		if (NumChips >= 7) ScanVar(&Chip6->Output, sizeof(INT16), "DAC6Output");
		if (NumChips >= 8) ScanVar(&Chip7->Output, sizeof(INT16), "DAC7Output");		
	}
	
	return 0;
}

#undef DEFAULT_SAMPLE_RATE
