// burn_ym2203.h
#include "driver.h"
extern "C" {
 #include "ay8910.h"
 #include "fm.h"
}
#include "timer.h"

extern "C" void BurnYM2203UpdateRequest();

INT32 BurnYM2203Init(INT32 num, INT32 nClockFrequency, FM_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), double (*GetTimeCallback)(), INT32 bAddSignal);
void BurnYM2203SetVolumeShift(INT32 Shift);
void BurnYM2203Reset();
void BurnYM2203Exit();
extern void (*BurnYM2203Update)(INT16* pSoundBuf, INT32 nSegmentEnd);
void BurnYM2203Scan(INT32 nAction, INT32* pnMin);

#define BurnYM2203Read(i, a) YM2203Read(i, a)

#if defined FBA_DEBUG
	#define BurnYM2203Write(i, a, n) if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203Write called without init\n")); YM2203Write(i, a, n)
	#define BurnYM2203SetPorts(c, read0, read1, write0, write1)	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203SetPorts called without init\n")); AY8910SetPorts(c, read0, read1, write0, write1)
#else
	#define BurnYM2203Write(i, a, n) YM2203Write(i, a, n)
	#define BurnYM2203SetPorts(c, read0, read1, write0, write1)	AY8910SetPorts(c, read0, read1, write0, write1)
#endif
