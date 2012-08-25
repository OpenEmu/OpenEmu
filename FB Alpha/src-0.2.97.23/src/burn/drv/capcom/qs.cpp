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

	QscInit(nRate);		// Init QSound chip

	return 0;
}

void QsndSetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir)
{
	QscSetRoute(nIndex, nVolume, nRouteDir);
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
