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
void BurnYMF278BReset();
void BurnYMF278BExit();
void BurnYMF278BUpdate(INT32 nSegmentEnd);
void BurnYMF278BScan(INT32 nAction, INT32* pnMin);


