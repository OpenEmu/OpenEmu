#include "driver.h"
extern "C" {
 #include "fmopl.h"
}
#include "timer.h"

INT32 BurnTimerUpdateY8950(INT32 nCycles);
void BurnTimerEndFrameY8950(INT32 nCycles);
void BurnTimerUpdateEndY8950();
INT32 BurnTimerAttachSekY8950(INT32 nClockspeed);
INT32 BurnTimerAttachZetY8950(INT32 nClockspeed);
INT32 BurnTimerAttachM6809Y8950(INT32 nClockspeed);
INT32 BurnTimerAttachHD6309Y8950(INT32 nClockspeed);
INT32 BurnTimerAttachM6800Y8950(INT32 nClockspeed);
INT32 BurnTimerAttachHD63701Y8950(INT32 nClockspeed);
INT32 BurnTimerAttachM6803Y8950(INT32 nClockspeed);
INT32 BurnTimerAttachM6502Y8950(INT32 nClockspeed);

extern "C" void BurnY8950UpdateRequest();

INT32 BurnY8950Init(INT32 nClockFrequency, UINT8* Y8950ADPCMROM, INT32 nY8950ADPCMSize, OPL_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), INT32 bAddSignal);
void BurnY8950Reset();
void BurnY8950Exit();
extern void (*BurnY8950Update)(INT16* pSoundBuf, INT32 nSegmentEnd);
void BurnY8950Scan(INT32 nAction, INT32* pnMin);

#define BurnY8950Read(a) Y8950Read(0, a)

#if defined FBA_DEBUG
	#define BurnY8950Write(a, n) if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("BurnY8950Write called without init\n")); Y8950Write(0, a, n)
#else
	#define BurnY8950Write(a, n) Y8950Write(0, a, n)
#endif
