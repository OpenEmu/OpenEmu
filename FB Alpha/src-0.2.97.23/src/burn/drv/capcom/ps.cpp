// PSound (CPS1 sound)
#include "cps.h"
#include "driver.h"
extern "C" {
 #include "ym2151.h"
}

UINT8 PsndCode, PsndFade;						// Sound code/fade sent to the z80 program

static INT32 nSyncPeriod;
static INT32 nSyncNext;

static INT32 nCyclesDone;

static void drvYM2151IRQHandler(INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
		ZetRun(0x0800);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

INT32 PsndInit()
{
	nCpsZ80Cycles = 4000000 * 100 / nBurnFPS;
	nSyncPeriod = nCpsZ80Cycles / 32;

	// Init PSound z80
	if (PsndZInit()!= 0) {
		return 1;
	}

	// Init PSound mixing (not critical if it fails)
	PsmInit();

	YM2151SetIrqHandler(0, &drvYM2151IRQHandler);

	PsndCode = 0; PsndFade = 0;

	nCyclesDone = 0;

	return 0;
}

INT32 PsndExit()
{
	PsmExit();
	PsndZExit();

	return 0;
}

INT32 PsndScan(INT32 nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nCyclesDone); SCAN_VAR(nSyncNext);
		PsndZScan(nAction);							// Scan Z80
		SCAN_VAR(PsndCode); SCAN_VAR(PsndFade);		// Scan sound info
	}
	return 0;
}

void PsndNewFrame()
{
	ZetNewFrame();
	PsmNewFrame();
	nSyncNext = nSyncPeriod;

	ZetIdle(nCyclesDone % nCpsZ80Cycles);
	nCyclesDone = 0;
}

INT32 PsndSyncZ80(INT32 nCycles)
{
	while (nSyncNext < nCycles) {
		PsmUpdate(nSyncNext * nBurnSoundLen / nCpsZ80Cycles);
		ZetRun(nSyncNext - ZetTotalCycles());
		nSyncNext += nSyncPeriod;
	}

	nCyclesDone = ZetRun(nCycles - ZetTotalCycles());

	return 0;
}
