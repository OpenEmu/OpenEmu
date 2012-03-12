#include "toaplan.h"
// Toaplan -- palette functions

UINT8* ToaPalSrc;			// Pointer to input palette
UINT8* ToaPalSrc2;
UINT32* ToaPalette;
UINT32* ToaPalette2;
INT32 nToaPalLen;

UINT8 ToaRecalcPalette;		// Set to 1 to force recalculation of the entire palette (not needed now)

INT32 ToaPalInit()
{
	return 0;
}

INT32 ToaPalExit()
{
	return 0;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (BURN_ENDIAN_SWAP_INT16(nColour) & 0x001F) << 3;	// Red
	r |= r >> 5;
	g = (BURN_ENDIAN_SWAP_INT16(nColour) & 0x03E0) >> 2;  	// Green
	g |= g >> 5;
	b = (BURN_ENDIAN_SWAP_INT16(nColour) & 0x7C00) >> 7;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

INT32 ToaPalUpdate()
{
	UINT16* ps = (UINT16*)ToaPalSrc;
	UINT32* pd = ToaPalette;
	
	pBurnDrvPalette = ToaPalette;

	for (INT32 i = 0; i < nToaPalLen; i++) {
		pd[i] = CalcCol(ps[i]);
	}
	return 0;
}

INT32 ToaPal2Update()
{
	UINT16* ps = (UINT16*)ToaPalSrc2;
	UINT32* pd = ToaPalette2;
	
//	pBurnDrvPalette = ToaPalette2;

	for (INT32 i = 0; i < nToaPalLen; i++) {
		pd[i] = CalcCol(ps[i]);
	}
	return 0;
}
