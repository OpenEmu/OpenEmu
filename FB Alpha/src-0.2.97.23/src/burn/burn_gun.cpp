#include "burnint.h"
#include "burn_gun.h"

// Generic Light Gun support for FBA
// written by Barry Harris (Treble Winner) based on the code in Kev's opwolf driver

INT32 nBurnGunNumPlayers = 0;
static bool bBurnGunDrawTargets = true;

static INT32 nBurnGunMaxX = 0;
static INT32 nBurnGunMaxY = 0;

INT32 BurnGunX[MAX_GUNS];
INT32 BurnGunY[MAX_GUNS];

#define P1Colour	0xfc, 0x12, 0xee
#define P2Colour	0x1c, 0xfc, 0x1c
#define P3Colour	0x15, 0x93, 0xfd
#define P4Colour	0xf7, 0xfa, 0x0e

#define a 0,
#define b 1,

UINT8 BurnGunTargetData[18][18] = {
	{ a a a a  a a a a  b a a a  a a a a  a a },
	{ a a a a  a a b b  b b b a  a a a a  a a },
	{ a a a a  b b a a  b a a b  b a a a  a a },
	{ a a a b  a a a a  b a a a  a b a a  a a },
	{ a a b a  a a a a  b a a a  a a b a  a a },
	{ a a b a  a a a b  b b a a  a a b a  a a },
	{ a b a a  a a b b  b b b a  a a a b  a a },
	{ a b a a  a b b a  a a a b  a a a b  a a },
	{ b b b b  b b b a  a a b b  b b b b  b a },
	{ a b a a  a b b a  a a a b  b a a b  a a },
	{ a b a a  a a b a  b a b b  a a a b  a a },
	{ a a b a  a a a b  b b b a  a a b a  a a },
	{ a a b a  a a a a  b b a a  a a b a  a a },
	{ a a a b  a a a a  b a a a  a b a a  a a },
	{ a a a a  b b a a  b a a b  b a a a  a a },
	{ a a a a  a a b b  b b b a  a a a a  a a },
	{ a a a a  a a a a  b a a a  a a a a  a a },
	{ a a a a  a a a a  a a a a  a a a a  a a },
};
#undef b
#undef a

UINT8 BurnGunReturnX(INT32 num)
{
#if defined FBA_DEBUG
	if (!Debug_BurnGunInitted) bprintf(PRINT_ERROR, _T("BurnGunReturnX called without init\n"));
	if (num >= nBurnGunNumPlayers) bprintf(PRINT_ERROR, _T("BurnGunReturnX called with invalid player %x\n"), num);
#endif

	if (num > MAX_GUNS - 1) return 0xff;

	float temp = (float)((BurnGunX[num] >> 8) + 8) / nBurnGunMaxX * 0xff;
	return (UINT8)temp;
}

UINT8 BurnGunReturnY(INT32 num)
{
#if defined FBA_DEBUG
	if (!Debug_BurnGunInitted) bprintf(PRINT_ERROR, _T("BurnGunReturnY called without init\n"));
	if (num >= nBurnGunNumPlayers) bprintf(PRINT_ERROR, _T("BurnGunReturnY called with invalid player %x\n"), num);
#endif

	if (num > MAX_GUNS - 1) return 0xff;
	
	float temp = (float)((BurnGunY[num] >> 8) + 8) / nBurnGunMaxY * 0xff;
	return (UINT8)temp;
}

void BurnGunMakeInputs(INT32 num, INT16 x, INT16 y)
{
#if defined FBA_DEBUG
	if (!Debug_BurnGunInitted) bprintf(PRINT_ERROR, _T("BurnGunMakeInputs called without init\n"));
	if (num >= nBurnGunNumPlayers) bprintf(PRINT_ERROR, _T("BurnGunMakeInputs called with invalid player %x\n"), num);
#endif

	if (num > MAX_GUNS - 1) return;
	
	const INT32 MinX = -8 * 0x100;
	const INT32 MinY = -8 * 0x100;
	
	BurnGunX[num] += x;
	BurnGunY[num] += y;
	
	if (BurnGunX[num] < MinX) BurnGunX[num] = MinX;
	if (BurnGunX[num] > MinX + nBurnGunMaxX * 0x100) BurnGunX[num] = MinX + nBurnGunMaxX * 0x100;
	if (BurnGunY[num] < MinY) BurnGunY[num] = MinY;
	if (BurnGunY[num] > MinY + nBurnGunMaxY * 0x100) BurnGunY[num] = MinY + nBurnGunMaxY * 0x100;
}
	
void BurnGunInit(INT32 nNumPlayers, bool bDrawTargets)
{
	Debug_BurnGunInitted = 1;
	
	if (nNumPlayers > MAX_GUNS) nNumPlayers = MAX_GUNS;
	nBurnGunNumPlayers = nNumPlayers;
	bBurnGunDrawTargets = bDrawTargets;
	
	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&nBurnGunMaxY, &nBurnGunMaxX);
	} else {
		BurnDrvGetVisibleSize(&nBurnGunMaxX, &nBurnGunMaxY);
	}
	
	for (INT32 i = 0; i < MAX_GUNS; i++) {
		BurnGunX[i] = ((nBurnGunMaxX / 2) - 7) << 8;
		BurnGunY[i] = ((nBurnGunMaxY / 2) - 8) << 8;
	}
}

void BurnGunExit()
{
#if defined FBA_DEBUG
	if (!Debug_BurnGunInitted) bprintf(PRINT_ERROR, _T("BurnGunExit called without init\n"));
#endif

	nBurnGunNumPlayers = 0;
	bBurnGunDrawTargets = true;
	
	nBurnGunMaxX = 0;
	nBurnGunMaxY = 0;
	
	for (INT32 i = 0; i < MAX_GUNS; i++) {
		BurnGunX[i] = 0;
		BurnGunY[i] = 0;
	}
	
	Debug_BurnGunInitted = 0;
}

void BurnGunScan()
{
#if defined FBA_DEBUG
	if (!Debug_BurnGunInitted) bprintf(PRINT_ERROR, _T("BurnGunScan called without init\n"));
#endif

	SCAN_VAR(BurnGunX);
	SCAN_VAR(BurnGunY);
}

void BurnGunDrawTarget(INT32 num, INT32 x, INT32 y)
{
#if defined FBA_DEBUG
	if (!Debug_BurnGunInitted) bprintf(PRINT_ERROR, _T("BurnGunDrawTarget called without init\n"));
	if (num >= nBurnGunNumPlayers) bprintf(PRINT_ERROR, _T("BurnGunDrawTarget called with invalid player %x\n"), num);
#endif

	if (bBurnGunDrawTargets == false) return;
	
	if (num > MAX_GUNS - 1) return;
	
	UINT8* pTile = pBurnDraw + nBurnGunMaxX * nBurnBpp * (y - 1) + nBurnBpp * x;
	
	UINT32 nTargetCol = 0;
	if (num == 0) nTargetCol = BurnHighCol(P1Colour, 0);
	if (num == 1) nTargetCol = BurnHighCol(P2Colour, 0);
	if (num == 2) nTargetCol = BurnHighCol(P3Colour, 0);
	if (num == 3) nTargetCol = BurnHighCol(P4Colour, 0);

	for (INT32 y2 = 0; y2 < 17; y2++) {

		pTile += nBurnGunMaxX * nBurnBpp;

		if ((y + y2) < 0 || (y + y2) > nBurnGunMaxY - 1) {
			continue;
		}

		for (INT32 x2 = 0; x2 < 17; x2++) {

			if ((x + x2) < 0 || (x + x2) > nBurnGunMaxX - 1) {
				continue;
			}

			if (BurnGunTargetData[y2][x2]) {
				if (nBurnBpp == 2) {
					((UINT16*)pTile)[x2] = (UINT16)nTargetCol;
				} else {
					((UINT32*)pTile)[x2] = nTargetCol;
				}
			}
		}
	}
}

#undef P1Colour
#undef P2Colour
#undef P3Colour
#undef P4Colour
