#include "burnint.h"
#include "burn_sound.h"
#include "segapcm.h"

#define MAX_CHIPS		2

struct segapcm
{
	UINT8 ram[0x800];
	UINT8 low[16];
	const UINT8 *rom;
	INT32 bankshift;
	INT32 bankmask;
	INT32 UpdateStep;
	double Volume[2];
	INT32 OutputDir[2];
};

static struct segapcm *Chip[MAX_CHIPS] = { NULL, };
static INT32 *Left[MAX_CHIPS] = { NULL, };
static INT32 *Right[MAX_CHIPS] = { NULL, };

static INT32 nNumChips = 0;

static void SegaPCMUpdateOne(INT32 nChip, INT32 nLength)
{
	INT32 Channel;
	
	memset(Left[nChip], 0, nLength * sizeof(INT32));
	memset(Right[nChip], 0, nLength * sizeof(INT32));

	for (Channel = 0; Channel < 16; Channel++) {
		UINT8 *Regs = Chip[nChip]->ram + 8 * Channel;
		if (!(Regs[0x86] & 1)) {
			const UINT8 *Rom = Chip[nChip]->rom + ((Regs[0x86] & Chip[nChip]->bankmask) << Chip[nChip]->bankshift);
			UINT32 Addr = (Regs[0x85] << 16) | (Regs[0x84] << 8) | Chip[nChip]->low[Channel];
			UINT32 Loop = (Regs[0x05] << 16) | (Regs[0x04] << 8);
			UINT8 End = Regs[6] + 1;
			INT32 i;
			
			for (i = 0; i < nLength; i++) {
				INT8 v = 0;

				if ((Addr >> 16) == End) {
					if (Regs[0x86] & 2) {
						Regs[0x86] |= 1;
						break;
					} else {
						Addr = Loop;
					}
				}

				v = Rom[Addr >> 8] - 0x80;

				Left[nChip][i] += v * Regs[2];
				Right[nChip][i] += v * Regs[3];
				Addr = (Addr + ((Regs[7] * Chip[nChip]->UpdateStep) >> 16)) & 0xffffff;
			}

			Regs[0x84] = Addr >> 8;
			Regs[0x85] = Addr >> 16;
			Chip[nChip]->low[Channel] = Regs[0x86] & 1 ? 0 : Addr;
		}
	}
}

void SegaPCMUpdate(INT16* pSoundBuf, INT32 nLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SegaPCMInitted) bprintf(PRINT_ERROR, _T("SegaPCMUpdate called without init\n"));
#endif

	for (INT32 i = 0; i < nNumChips + 1; i++) {
		SegaPCMUpdateOne(i, nLength);
	}
	
	for (INT32 i = 0; i < nLength; i++) {
		INT32 nLeftSample = 0;
		INT32 nRightSample = 0;
		
		if ((Chip[0]->OutputDir[BURN_SND_SEGAPCM_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(Left[0][i] * Chip[0]->Volume[BURN_SND_SEGAPCM_ROUTE_1]);
		}
		if ((Chip[0]->OutputDir[BURN_SND_SEGAPCM_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(Left[0][i] * Chip[0]->Volume[BURN_SND_SEGAPCM_ROUTE_1]);
		}
		
		if ((Chip[0]->OutputDir[BURN_SND_SEGAPCM_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(Right[0][i] * Chip[0]->Volume[BURN_SND_SEGAPCM_ROUTE_2]);
		}
		if ((Chip[0]->OutputDir[BURN_SND_SEGAPCM_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(Right[0][i] * Chip[0]->Volume[BURN_SND_SEGAPCM_ROUTE_2]);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
		
		if (nNumChips >= 1) {
			if ((Chip[1]->OutputDir[BURN_SND_SEGAPCM_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(Left[1][i] * Chip[1]->Volume[BURN_SND_SEGAPCM_ROUTE_1]);
			}
			if ((Chip[1]->OutputDir[BURN_SND_SEGAPCM_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(Left[1][i] * Chip[1]->Volume[BURN_SND_SEGAPCM_ROUTE_1]);
			}
		
			if ((Chip[1]->OutputDir[BURN_SND_SEGAPCM_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(Right[1][i] * Chip[1]->Volume[BURN_SND_SEGAPCM_ROUTE_2]);
			}
			if ((Chip[1]->OutputDir[BURN_SND_SEGAPCM_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(Right[1][i] * Chip[1]->Volume[BURN_SND_SEGAPCM_ROUTE_2]);
			}
		
			nLeftSample = BURN_SND_CLIP(nLeftSample);
			nRightSample = BURN_SND_CLIP(nRightSample);
		}
		
		pSoundBuf[0] += nLeftSample;
		pSoundBuf[1] += nRightSample;
		pSoundBuf += 2;
	}
}

void SegaPCMInit(INT32 nChip, INT32 clock, INT32 bank, UINT8 *pPCMData, INT32 PCMDataSize)
{
	INT32 Mask, RomMask;
	
	Chip[nChip] = (struct segapcm*)BurnMalloc(sizeof(*Chip[nChip]));
	memset(Chip[nChip], 0, sizeof(*Chip[nChip]));

	Chip[nChip]->rom = pPCMData;
	
	memset(Chip[nChip]->ram, 0xff, 0x800);
	
	Left[nChip] = (INT32*)BurnMalloc(nBurnSoundLen * sizeof(INT32));
	Right[nChip] = (INT32*)BurnMalloc(nBurnSoundLen * sizeof(INT32));

	Chip[nChip]->bankshift = bank;
	Mask = bank >> 16;
	if(!Mask)
		Mask = BANK_MASK7 >> 16;

	for (RomMask = 1; RomMask < PCMDataSize; RomMask *= 2) {}
	RomMask--;

	Chip[nChip]->bankmask = Mask & (RomMask >> Chip[nChip]->bankshift);
	
	double Rate = (double)clock / 128 / nBurnSoundRate;
	Chip[nChip]->UpdateStep = (INT32)(Rate * 0x10000);
	
	Chip[nChip]->Volume[BURN_SND_SEGAPCM_ROUTE_1] = 1.00;
	Chip[nChip]->Volume[BURN_SND_SEGAPCM_ROUTE_2] = 1.00;
	Chip[nChip]->OutputDir[BURN_SND_SEGAPCM_ROUTE_1] = BURN_SND_ROUTE_LEFT;
	Chip[nChip]->OutputDir[BURN_SND_SEGAPCM_ROUTE_2] = BURN_SND_ROUTE_RIGHT;
	
	nNumChips = nChip;
	
	DebugSnd_SegaPCMInitted = 1;
}

void SegaPCMSetRoute(INT32 nChip, INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SegaPCMInitted) bprintf(PRINT_ERROR, _T("SegaPCMSetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("SegaPCMSetRoute called with invalid index %i\n"), nIndex);
	if (nChip > nNumChips) bprintf(PRINT_ERROR, _T("SegaPCMSetRoute called with invalid chip %i\n"), nChip);
#endif

	Chip[nChip]->Volume[nIndex] = nVolume;
	Chip[nChip]->OutputDir[nIndex] = nRouteDir;
}

void SegaPCMExit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_SegaPCMInitted) bprintf(PRINT_ERROR, _T("SegaPCMExit called without init\n"));
#endif

	for (INT32 i = 0; i < nNumChips + 1; i++) {
		BurnFree(Chip[i]);
		BurnFree(Left[i]);
		BurnFree(Right[i]);
	}
	
	nNumChips = 0;
	
	DebugSnd_SegaPCMInitted = 0;
}

INT32 SegaPCMScan(INT32 nAction,INT32 *pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SegaPCMInitted) bprintf(PRINT_ERROR, _T("SegaPCMScan called without init\n"));
#endif

	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029719;
	}
	
	for (INT32 i = 0; i < nNumChips + 1; i++) {
		if (nAction & ACB_DRIVER_DATA) {
			ScanVar(Chip[i]->low, 16 * sizeof(UINT8), "SegaPCMlow");
		
			memset(&ba, 0, sizeof(ba));
			ba.Data	  = Chip[i]->ram;
			ba.nLen	  = 0x800;
			ba.szName = "SegaPCMRAM";	
			BurnAcb(&ba);
		}
	}
	
	return 0;
}

UINT8 SegaPCMRead(INT32 nChip, UINT32 Offset)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SegaPCMInitted) bprintf(PRINT_ERROR, _T("SegaPCMRead called without init\n"));
	if (nChip > nNumChips) bprintf(PRINT_ERROR, _T("SegaPCMRead called with invalid chip %i\n"), nChip);
#endif

	return Chip[nChip]->ram[Offset & 0x07ff];
}

void SegaPCMWrite(INT32 nChip, UINT32 Offset, UINT8 Data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SegaPCMInitted) bprintf(PRINT_ERROR, _T("SegaPCMWrite called without init\n"));
	if (nChip > nNumChips) bprintf(PRINT_ERROR, _T("SegaPCMWrite called with invalid chip %i\n"), nChip);
#endif

	Chip[nChip]->ram[Offset & 0x07ff] = Data;
}
