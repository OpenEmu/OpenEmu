#include "driver.h"
extern "C" {
 #include "fmopl.h"
}
#include "timer.h"

INT32 BurnTimerUpdateYM3526(INT32 nCycles);
void BurnTimerEndFrameYM3526(INT32 nCycles);
INT32 BurnTimerAttachSekYM3526(INT32 nClockspeed);
INT32 BurnTimerAttachZetYM3526(INT32 nClockspeed);
INT32 BurnTimerAttachM6809YM3526(INT32 nClockspeed);
INT32 BurnTimerAttachHD6309YM3526(INT32 nClockspeed);
INT32 BurnTimerAttachM6800YM3526(INT32 nClockspeed);
INT32 BurnTimerAttachHD63701YM3526(INT32 nClockspeed);
INT32 BurnTimerAttachM6803YM3526(INT32 nClockspeed);
INT32 BurnTimerAttachM6502YM3526(INT32 nClockspeed);

extern "C" void BurnYM3526UpdateRequest();

INT32 BurnYM3526Init(INT32 nClockFrequency, OPL_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), INT32 bAddSignal);
void BurnYM3526SetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir);
void BurnYM3526Reset();
void BurnYM3526Exit();
extern void (*BurnYM3526Update)(INT16* pSoundBuf, INT32 nSegmentEnd);
void BurnYM3526Scan(INT32 nAction, INT32* pnMin);

#define BURN_SND_YM3526_ROUTE			0

#define BurnYM3526Read(a) YM3526Read(0, a)

#if defined FBA_DEBUG
	#define BurnYM3526Write(a, n) if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("BurnYM3526Write called without init\n")); YM3526Write(0, a, n)
#else
	#define BurnYM3526Write(a, n) YM3526Write(0, a, n)
#endif
