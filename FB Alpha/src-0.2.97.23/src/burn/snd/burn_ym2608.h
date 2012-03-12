// burn_ym2608.h
#include "driver.h"
extern "C" {
 #include "ay8910.h"
 #include "fm.h"
}
#include "timer.h"

extern "C" void BurnYM2608UpdateRequest();

INT32 BurnYM2608Init(INT32 nClockFrequency, UINT8* YM2608ADPCMROM, INT32* nYM2608ADPCMSize, FM_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), double (*GetTimeCallback)(), INT32 bAddSignal);
void BurnYM2608Reset();
void BurnYM2608Exit();
extern void (*BurnYM2608Update)(INT16* pSoundBuf, INT32 nSegmentEnd);
void BurnYM2608Scan(INT32 nAction, INT32* pnMin);

#define BurnYM2608Read(a) YM2608Read(0, a)

#if defined FBA_DEBUG
	#define BurnYM2608Write(a, n) if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("BurnYM2608Write called without init\n")); YM2608Write(0, a, n)
#else
	#define BurnYM2608Write(a, n) YM2608Write(0, a, n)
#endif
