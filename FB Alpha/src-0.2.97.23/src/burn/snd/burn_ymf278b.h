#include "driver.h"
extern "C" {
 #include "ymf278b.h"
}

#include "timer.h"

void BurnYMF278BSelectRegister(INT32 nRegister, UINT8 nValue);
void BurnYMF278BWriteRegister(INT32 nRegister, UINT8 nValue);
UINT8 BurnYMF278BReadStatus();
UINT8 BurnYMF278BReadData();

INT32 BurnYMF278BInit(INT32 nClockFrequency, UINT8* YMF278BROM, void (*IRQCallback)(INT32, INT32), INT32 (*StreamCallback)(INT32));
void BurnYMF278BSetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir);
void BurnYMF278BReset();
void BurnYMF278BExit();
void BurnYMF278BUpdate(INT32 nSegmentEnd);
void BurnYMF278BScan(INT32 nAction, INT32* pnMin);

#define BURN_SND_YMF278B_YMF278B_ROUTE_1		0
#define BURN_SND_YMF278B_YMF278B_ROUTE_2		1

#define BurnYMF278BSetAllRoutes(v, d)								\
	BurnYMF278BSetRoute(BURN_SND_YMF278B_YMF278B_ROUTE_1, v, d);	\
	BurnYMF278BSetRoute(BURN_SND_YMF278B_YMF278B_ROUTE_2, v, d);
