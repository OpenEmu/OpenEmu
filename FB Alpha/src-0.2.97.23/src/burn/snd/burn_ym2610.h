// burn_ym2610.h
#include "driver.h"
extern "C" {
 #include "ay8910.h"
 #include "fm.h"
}
#include "timer.h"

extern "C" void BurnYM2610UpdateRequest();

void BurnYM2610MapADPCMROM(UINT8* YM2610ADPCMAROM, INT32 nYM2610ADPCMASize, UINT8* YM2610ADPCMBROM, INT32 nYM2610ADPCMBSize);
INT32 BurnYM2610Init(INT32 nClockFrequency, UINT8* YM2610ADPCMAROM, INT32* nYM2610ADPCMASize, UINT8* YM2610ADPCMBROM, INT32* nYM2610ADPCMBSize, FM_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), double (*GetTimeCallback)(), INT32 bAddSignal);
void BurnYM2610SetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir);
void BurnYM2610SetLeftVolume(INT32 nIndex, double nLeftVolume);
void BurnYM2610SetRightVolume(INT32 nIndex, double nRightVolume);
void BurnYM2610Reset();
void BurnYM2610Exit();
extern void (*BurnYM2610Update)(INT16* pSoundBuf, INT32 nSegmentEnd);
void BurnYM2610Scan(INT32 nAction, INT32* pnMin);

extern INT32 bYM2610UseSeperateVolumes;

#define BURN_SND_YM2610_YM2610_ROUTE_1		0
#define BURN_SND_YM2610_YM2610_ROUTE_2		1
#define BURN_SND_YM2610_AY8910_ROUTE		2

#define BurnYM2610SetAllRoutes(v, d)							\
	BurnYM2610SetRoute(BURN_SND_YM2610_YM2610_ROUTE_1, v, d);	\
	BurnYM2610SetRoute(BURN_SND_YM2610_YM2610_ROUTE_2, v, d);	\
	BurnYM2610SetRoute(BURN_SND_YM2610_AY8910_ROUTE  , v, d);
	
#define BurnYM2610Read(a) YM2610Read(0, a)

#if defined FBA_DEBUG
	#define BurnYM2610Write(a, n) if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("BurnYM2610Write called without init\n")); YM2610Write(0, a, n)
#else
	#define BurnYM2610Write(a, n) YM2610Write(0, a, n)
#endif
