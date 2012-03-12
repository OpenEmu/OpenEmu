// burn_ym2413.h
#include "driver.h"
extern "C" {
 #include "ym2413.h"
}

INT32 BurnYM2413Init(INT32 nClockFrequency, float nVolume);
void BurnYM2413Reset();
void BurnYM2413Exit();
extern void (*BurnYM2413Render)(INT16* pSoundBuf, INT32 nSegmentLength);
void BurnYM2413Scan(INT32 nAction);
void BurnYM2413IncreaseVolume(INT32 nFactor);
void BurnYM2413DecreaseVolume(INT32 nFactor);

#define BurnYM2413Read(a) YM2413Read(0, a)

static inline void BurnYM2413Write(INT32 Address, const UINT8 nValue)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2413Initted) bprintf(PRINT_ERROR, _T("BurnYM2413Write called without init\n"));
#endif

	YM2413Write(0, Address, nValue);
}
