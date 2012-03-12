#include "cave.h"

UINT8* CavePalSrc;
UINT8 CaveRecalcPalette;	// Set to 1 to force recalculation of the entire palette

UINT32* CavePalette = NULL;
static UINT16* CavePalCopy = NULL;

INT32 CavePalInit(INT32 nPalSize)
{
	CavePalette = (UINT32*)BurnMalloc(nPalSize * sizeof(UINT32));
	memset(CavePalette, 0, nPalSize * sizeof(UINT32));

	CavePalCopy = (UINT16*)BurnMalloc(nPalSize * sizeof(UINT16));
	memset(CavePalCopy, 0, nPalSize * sizeof(UINT16));
	
	pBurnDrvPalette = CavePalette;

	return 0;
}

INT32 CavePalExit()
{
	BurnFree(CavePalette);
	BurnFree(CavePalCopy);

	return 0;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour & 0x03E0) >> 2;	// Red
	r |= r >> 5;
	g = (nColour & 0x7C00) >> 7;  	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

INT32 CavePalUpdate4Bit(INT32 nOffset, INT32 nNumPalettes)
{
	INT32 i, j;

	UINT16* ps = (UINT16*)CavePalSrc + nOffset;
	UINT16* pc;
	UINT32* pd;

	UINT16 c;

	if (CaveRecalcPalette) {

		for (i = 0; i < 0 + nNumPalettes; i++) {

			pc = CavePalCopy + (i << 8);
			pd = CavePalette + (i << 8);

			for (j = 0; j < 16; j++, ps++, pc++, pd++) {

				c = *ps;
				*pc = c;
				*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(c));

			}
		}

		CaveRecalcPalette = 0;
		return 0;
	}


	for (i = 0; i < 0 + nNumPalettes; i++) {

		pc = CavePalCopy + (i << 8);
		pd = CavePalette + (i << 8);

		for (j = 0; j < 16; j++, ps++, pc++, pd++) {

			c = *ps;
			if (*pc != c) {
				*pc = c;
				*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(c));
			}

		}
	}

	return 0;
}

INT32 CavePalUpdate8Bit(INT32 nOffset, INT32 nNumPalettes)
{
	if (CaveRecalcPalette) {
		INT32 i, j;

		UINT16* ps = (UINT16*)CavePalSrc + nOffset;
		UINT16* pc;
		UINT32* pd;

		UINT16 c;

		for (i = 0; i < nNumPalettes; i++) {

			pc = CavePalCopy + nOffset + (i << 8);
			pd = CavePalette + nOffset + (i << 8);

			for (j = 0; j < 256; j++, ps++, pc++, pd++) {

				c = *ps;
				*pc = c;
				*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(c));

			}
		}

		CaveRecalcPalette = 0;
	}

	return 0;
}

// Update the PC copy of the palette on writes to the palette memory
void CavePalWriteByte(UINT32 nAddress, UINT8 byteValue)
{
	nAddress ^= 1;
	CavePalSrc[nAddress] = byteValue;							// write byte

	if (*((UINT8*)(CavePalCopy + nAddress)) != byteValue) {
		*((UINT8*)(CavePalCopy + nAddress)) = byteValue;
		CavePalette[nAddress >> 1] = CalcCol(*(UINT16*)(CavePalSrc + (nAddress & ~0x01)));
	}
}

void CavePalWriteWord(UINT32 nAddress, UINT16 wordValue)
{
	nAddress >>= 1;

	((UINT16*)CavePalSrc)[nAddress] = BURN_ENDIAN_SWAP_INT16(wordValue);		// write word

	if (CavePalCopy[nAddress] != wordValue) {
		CavePalCopy[nAddress] = wordValue;
		CavePalette[nAddress] = CalcCol(wordValue);
	}
}

