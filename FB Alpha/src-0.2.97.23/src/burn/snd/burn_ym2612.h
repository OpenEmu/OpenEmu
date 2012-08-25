// burn_ym2612.h
#include "driver.h"
extern "C" {
 #include "fm.h"
}
#include "timer.h"

extern "C" void BurnYM2612UpdateRequest();

INT32 BurnYM2612Init(INT32 num, INT32 nClockFrequency, FM_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), double (*GetTimeCallback)(), INT32 bAddSignal);
void BurnYM2612SetRoute(INT32 nChip, INT32 nIndex, double nVolume, INT32 nRouteDir);
void BurnYM2612Reset();
void BurnYM2612Exit();
extern void (*BurnYM2612Update)(INT16* pSoundBuf, INT32 nSegmentEnd);
void BurnYM2612Scan(INT32 nAction, INT32* pnMin);

#define BURN_SND_YM2612_YM2612_ROUTE_1		0
#define BURN_SND_YM2612_YM2612_ROUTE_2		1

#define BurnYM2612SetAllRoutes(i, v, d)								\
	BurnYM2612SetRoute(i, BURN_SND_YM2612_YM2612_ROUTE_1, v, d);	\
	BurnYM2612SetRoute(i, BURN_SND_YM2612_YM2612_ROUTE_2, v, d);	
	
#define BurnYM2612Read(i, a) YM2612Read(i, a)

#if defined FBA_DEBUG
	#define BurnYM2612Write(i, a, n) if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("BurnYM2612Write called without init\n")); YM2612Write(i, a, n)
#else
	#define BurnYM2612Write(i, a, n) YM2612Write(i, a, n)
#endif

#define BURN_SND_YM3438_YM3438_ROUTE_1		BURN_SND_YM2612_YM2612_ROUTE_1
#define BURN_SND_YM3438_YM3438_ROUTE_2		BURN_SND_YM2612_YM2612_ROUTE_2

#define BurnYM3438SetAllRoutes(i, v, d)								\
	BurnYM3438SetRoute(i, BURN_SND_YM3438_YM3438_ROUTE_1, v, d);	\
	BurnYM3438SetRoute(i, BURN_SND_YM3438_YM3438_ROUTE_2, v, d);
	
#define BurnYM3438Init(i, n, a, b, c, d) BurnYM2612Init(i, n, a, b, c, d)
#define BurnYM3438SetRoute(i, n, v, d) BurnYM2612SetRoute(i, n, v, d)
#define BurnYM3438Reset() BurnYM2612Reset()
#define BurnYM3438Exit() BurnYM2612Exit()
#define BurnYM3438Update(p, i) BurnYM2612Update(p, i)
#define BurnYM3438Scan(n, i) BurnYM2612Scan(n, i)

#define BurnYM3438Read(i, a) YM2612Read(i, a)

#if defined FBA_DEBUG
	#define BurnYM3438Write(i, a, n) if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("BurnYM3438Write called without init\n")); YM2612Write(i, a, n)
#else
	#define BurnYM3438Write(i, a, n) YM2612Write(i, a, n)
#endif
