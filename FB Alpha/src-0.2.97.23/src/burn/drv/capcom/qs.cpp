#include "cps.h"
// QSound

static INT32 nQsndCyclesExtra;

static INT32 qsndTimerOver(INT32, INT32)
{
//	bprintf(PRINT_NORMAL, _T("  - IRQ -> 1.\n"));
	ZetSetIRQLine(0xFF, ZET_IRQSTATUS_AUTO);

	return 0;
}

INT32 QsndInit()
{
	INT32 nVolumeShift;
	INT32 nRate;

	// Init QSound z80
	if (QsndZInit()) {
		return 1;
	}
	BurnTimerInit(qsndTimerOver, NULL);

	if (Cps1Qs == 1) {
		nCpsZ80Cycles = 6000000 * 100 / nBurnFPS;
		BurnTimerAttachZet(6000000);
	} else {
		nCpsZ80Cycles = 8000000 * 100 / nBurnFPS;
		BurnTimerAttachZet(8000000);
	}

	if (nBurnSoundRate >= 0) {
		nRate = nBurnSoundRate;
	} else {
		nRate = 11025;
	}

	nVolumeShift = 0;

	// These games are too soft at normal volumes
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "csclub",   6) == 0) {
		nVolumeShift = -1;
	}

	// These games are loud at normal volumes (no clipping)
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "1944",	   4) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "dimaho",   6) == 0 ||
		strcmp( BurnDrvGetTextA(DRV_NAME), "gmahou"     ) == 0)
	{
		nVolumeShift = 1;
	}

	// These games are too loud at normal volumes (no clipping)
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "sgemf",    5) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "progear",  7) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "pzloop2",  7) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "ringdest", 8) == 0 ||
		strcmp( BurnDrvGetTextA(DRV_NAME), "ringdstd"   ) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "smbomb",   6) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "pfght",    5) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "mpang",    5) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "sfa2",     4) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "sfz2",     4) == 0)
	{
		nVolumeShift = 1;
	}
	
	// These games are too loud at normal volumes (no clipping)
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "gigawing", 8) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "spf2",     4) == 0)
	{
		nVolumeShift = 2;
	}
	
	// These games are too loud at normal volumes (clipping)
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "19xx",   4) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "ddtod",  5) == 0)
	{
		nVolumeShift = 2;
	}

	QscInit(nRate, nVolumeShift);		// Init QSound chip

	return 0;
}

void QsndReset()
{
	ZetOpen(0);
	BurnTimerReset();
	BurnTimerSetRetrig(0, 1.0 / 252.0);
	ZetClose();

	nQsndCyclesExtra = 0;
}

void QsndExit()
{
	QscExit();							// Exit QSound chip
	QsndZExit();
}

INT32 QsndScan(INT32 nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		QsndZScan(nAction);				// Scan Z80
		QscScan(nAction);				// Scan QSound Chip
	}

	return 0;
}

void QsndNewFrame()
{
	ZetNewFrame();

	ZetOpen(0);
	ZetIdle(nQsndCyclesExtra);

	QscNewFrame();
}

void QsndEndFrame()
{
	BurnTimerEndFrame(nCpsZ80Cycles);
	if (pBurnSoundOut) QscUpdate(nBurnSoundLen);

	nQsndCyclesExtra = ZetTotalCycles() - nCpsZ80Cycles;
	ZetClose();
}

void QsndSyncZ80()
{
	int nCycles = (INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles;

	if (nCycles <= ZetTotalCycles()) {
		return;
	}

	BurnTimerUpdate(nCycles);
}
