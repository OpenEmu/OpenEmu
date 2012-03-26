#include "burnint.h"
#include "burn_sound.h"

#define DAC_NUM		(8)	// Maximum DAC chips

struct dac_info
{
	INT16	Output;
	INT32 	nVolShift;
	INT32 	nCurrentPosition;
	INT32	Initialized;
	INT32	(*pSyncCallback)();
};

static struct dac_info dac_table[DAC_NUM];

static INT16 UnsignedVolTable[256];
static INT16 SignedVolTable[256];

static INT16 *buffer = NULL;

static INT32 NumChips;

static INT32 bAddSignal;

static void UpdateStream(INT32 chip, INT32 length)
{
	struct dac_info *ptr;

	if (buffer == NULL) {	// delay buffer allocation for cases when fps is not 60
		buffer = (INT16*)BurnMalloc(nBurnSoundLen * sizeof(INT16));
		memset (buffer, 0, nBurnSoundLen * sizeof(INT16));
	}

        ptr = &dac_table[chip];
        if (ptr->Initialized == 0) return;

        if (length > nBurnSoundLen) length = nBurnSoundLen;
        length -= ptr->nCurrentPosition;
        if (length <= 0) return;

        INT16 *buf = buffer + ptr->nCurrentPosition;

        INT16 Out = ptr->Output;

        ptr->nCurrentPosition += length;

        if (Out) {              
                while (length--) {
                        *buf++ = *buf + Out;
                }
        }
}

void DACUpdate(INT16* Buffer, INT32 Length)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACUpdate called without init\n"));
#endif

	struct dac_info *ptr;

	for (INT32 i = 0; i < NumChips; i++) {
		UpdateStream(i, nBurnSoundLen);
	}

	INT16 *buf = buffer;

	if (bAddSignal) {
		while (Length--) {
			Buffer[0] = BURN_SND_CLIP((INT32)(buf[0] + Buffer[0]));
			Buffer[1] = BURN_SND_CLIP((INT32)(buf[0] + Buffer[1]));
			Buffer += 2;
			buf[0] = 0; // clear buffer
			buf++;
		}
	} else {
		while (Length--) {
			Buffer[1] = Buffer[0] = buf[0];
			Buffer += 2;
			buf[0] = 0; // clear buffer
			buf++;
		}
	}

	for (INT32 i = 0; i < NumChips; i++) {
		ptr = &dac_table[i];
		ptr->nCurrentPosition = 0;
	}
}

void DACWrite(INT32 Chip, UINT8 Data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACWrite called without init\n"));
	if (Chip > NumChips) bprintf(PRINT_ERROR, _T("DACWrite called with invalid chip number %x\n"), Chip);
#endif

	struct dac_info *ptr;

	ptr = &dac_table[Chip];

	UpdateStream(Chip, ptr->pSyncCallback());

	ptr->Output = UnsignedVolTable[Data] >> ptr->nVolShift;
}

void DACSignedWrite(INT32 Chip, UINT8 Data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACSignedWrite called without init\n"));
	if (Chip > NumChips) bprintf(PRINT_ERROR, _T("DACSignedWrite called with invalid chip number %x\n"), Chip);
#endif

	struct dac_info *ptr;

	ptr = &dac_table[Chip];

	UpdateStream(Chip, ptr->pSyncCallback());

	ptr->Output = SignedVolTable[Data] >> ptr->nVolShift;
}

static void DACBuildVolTables()
{
	for (INT32 i = 0;i < 256;i++) {
		UnsignedVolTable[i] = i * 0x101 / 2;
		SignedVolTable[i] = i * 0x101 - 0x8000;
	}
}

void DACInit(INT32 Num, UINT32 /*Clock*/, INT32 bAdd, INT32 (*pSyncCB)())
{
#if defined FBA_DEBUG
	if (Num >= DAC_NUM) bprintf (PRINT_ERROR, _T("DACInit called for too many chips (%d)! Change DAC_NUM (%d)!\n"), Num, DAC_NUM);
	if (pSyncCB == NULL) bprintf (PRINT_ERROR, _T("DACInit called with NULL callback!\n"));
#endif

	struct dac_info *ptr;

	DebugSnd_DACInitted = 1;
	
	NumChips = Num + 1;

	ptr = &dac_table[Num];

	memset (ptr, 0, sizeof(dac_info));

	ptr->Initialized = 1;
	ptr->nVolShift = 0;
	ptr->pSyncCallback = pSyncCB;

	DACBuildVolTables(); // necessary to build for every chip?
	
	bAddSignal = bAdd;
}

void DACSetVolShift(INT32 Chip, INT32 nShift)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACSetVolShift called without init\n"));
	if (Chip > NumChips) bprintf(PRINT_ERROR, _T("DACSetVolShift called with invalid chip number %x\n"), Chip);
#endif

	struct dac_info *ptr;

	ptr = &dac_table[Chip];
	ptr->nVolShift = nShift;
}

void DACReset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACReset called without init\n"));
#endif

	struct dac_info *ptr;

	for (INT32 i = 0; i < NumChips; i++) {
		ptr = &dac_table[i];

		ptr->nCurrentPosition = 0;
		ptr->Output = 0;
	}
}

void DACExit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACExit called without init\n"));
#endif

	struct dac_info *ptr;

	for (INT32 i = 0; i < DAC_NUM; i++) {
		ptr = &dac_table[i];

		ptr->Initialized = 0;
		ptr->pSyncCallback = NULL;
	}

	NumChips = 0;
	
	DebugSnd_DACInitted = 0;

	BurnFree (buffer);
}

INT32 DACScan(INT32 nAction,INT32 *pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_DACInitted) bprintf(PRINT_ERROR, _T("DACScan called without init\n"));
#endif
	
	if (pnMin != NULL) {
		*pnMin = 0x029719;
	}
	
	struct dac_info *ptr;

	if (nAction & ACB_DRIVER_DATA) {
		for (INT32 i = 0; i < NumChips; i++) {
			ptr = &dac_table[i];

			SCAN_VAR(ptr->Output);
		}
	}

	return 0;
}
