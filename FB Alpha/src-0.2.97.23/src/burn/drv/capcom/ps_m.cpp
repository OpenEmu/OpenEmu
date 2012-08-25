#include "cps.h"
#include "burn_ym2151.h"

// CPS1 sound Mixing

INT32 bPsmOkay = 0;										// 1 if the module is okay
static INT16* WaveBuf = NULL;

static INT32 nPos;

INT32 PsmInit()
{
	INT32 nMemLen, nRate, nRet;
	bPsmOkay = 0;										// not OK yet

	if (nBurnSoundRate > 0) {
		nRate = nBurnSoundRate;
	} else {
		nRate = 11025;
	}

	if (BurnYM2151Init(3579540)) {				// Init FM sound chip
		return 1;
	}
	BurnYM2151SetAllRoutes(0.35, BURN_SND_ROUTE_BOTH);

	// Allocate a buffer for the intermediate sound (between YM2151 and pBurnSoundOut)
	nMemLen = nBurnSoundLen * 2 * sizeof(INT16);
	WaveBuf = (INT16*)BurnMalloc(nMemLen);
	if (WaveBuf == NULL) {
		PsmExit();
		return 1;
	}
	memset(WaveBuf, 0, nMemLen);						// Init to silence

	// Init ADPCM
	MSM6295ROM = CpsAd;
	if (Forgottn) {
		nRet = MSM6295Init(0, 6061, 1);
	} else {
		nRet = MSM6295Init(0, 7576, 1);
	}
	MSM6295SetRoute(0, 0.30, BURN_SND_ROUTE_BOTH);

	if (nRet!=0) {
		PsmExit(); return 1;
	}

	bPsmOkay = 1;										// OK

	return 0;
}

INT32 PsmExit()
{
	bPsmOkay = 0;

	MSM6295Exit(0);

	BurnFree(WaveBuf);

	BurnYM2151Exit();									// Exit FM sound chip
	return 0;
}

void PsmNewFrame()
{
	nPos = 0;
}

INT32 PsmUpdate(INT32 nEnd)
{
	if (bPsmOkay == 0 || pBurnSoundOut == NULL) {
		return 1;
	}

	if (nEnd <= nPos) {
		return 0;
	}
	if (nEnd > nBurnSoundLen) {
		nEnd = nBurnSoundLen;
	}

	// Render FM
	BurnYM2151Render(pBurnSoundOut + (nPos << 1), nEnd - nPos);

	// Render ADPCM
	MSM6295Render(0, pBurnSoundOut + (nPos << 1), nEnd - nPos);

	nPos = nEnd;

	return 0;
}
