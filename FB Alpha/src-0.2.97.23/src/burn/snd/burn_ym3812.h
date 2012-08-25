#include "driver.h"
extern "C" {
 #include "fmopl.h"
}
#include "timer.h"

INT32 BurnTimerUpdateYM3812(INT32 nCycles);
void BurnTimerEndFrameYM3812(INT32 nCycles);
void BurnTimerUpdateEndYM3812();
INT32 BurnTimerAttachSekYM3812(INT32 nClockspeed);
INT32 BurnTimerAttachZetYM3812(INT32 nClockspeed);
INT32 BurnTimerAttachM6809YM3812(INT32 nClockspeed);
INT32 BurnTimerAttachHD6309YM3812(INT32 nClockspeed);
INT32 BurnTimerAttachM6800YM3812(INT32 nClockspeed);
INT32 BurnTimerAttachHD63701YM3812(INT32 nClockspeed);
INT32 BurnTimerAttachM6803YM3812(INT32 nClockspeed);
INT32 BurnTimerAttachM6502YM3812(INT32 nClockspeed);
INT32 BurnTimerAttachH6280YM3812(INT32 nClockspeed);

extern "C" void BurnYM3812UpdateRequest();

INT32 BurnYM3812Init(INT32 nClockFrequency, OPL_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), INT32 bAddSignal);
void BurnYM3812SetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir);
void BurnYM3812Reset();
void BurnYM3812Exit();
extern void (*BurnYM3812Update)(INT16* pSoundBuf, INT32 nSegmentEnd);
void BurnYM3812Scan(INT32 nAction, INT32* pnMin);

#define BURN_SND_YM3812_ROUTE			0

#define BurnYM3812Read(a) YM3812Read(0, a)

#if defined FBA_DEBUG
	#define BurnYM3812Write(a, n) if (!DebugSnd_YM3812Initted) bprintf(PRINT_ERROR, _T("BurnYM3812Write called without init\n")); YM3812Write(0, a, n)
#else
	#define BurnYM3812Write(a, n) YM3812Write(0, a, n)
#endif
