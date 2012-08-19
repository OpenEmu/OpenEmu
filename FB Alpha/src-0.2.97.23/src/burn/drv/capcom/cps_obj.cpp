#include "cps.h"
// CPS Objs (sprites)

INT32 nCpsObjectBank;

UINT8 *CpsBootlegSpriteRam = NULL;

INT32 Cps1LockSpriteList910000 = 0;
INT32 Cps1DetectEndSpriteList8000 = 0;

Cps1ObjGetCallback Cps1ObjGetCallbackFunction = NULL;
Cps1ObjDrawCallback Cps1ObjDrawCallbackFunction = NULL;

// Our copy of the sprite table
static UINT8 *ObjMem = NULL;

static INT32 nMax = 0;
static INT32 nGetNext = 0;

static INT32 nMaxZValue;
static INT32 nMaxZMask;

static INT32 nZOffset;

// Object frames, so you can lag the Objs by nFrameCount-1 frames
struct ObjFrame {
	INT32 nShiftX, nShiftY;
	UINT8* Obj;
	INT32 nCount;
};

static INT32 nFrameCount = 0;
static struct ObjFrame of[3];

INT32 CpsObjInit()
{
	nMax = 0x100;				// CPS1 has 256 sprites

	if (Cps == 2) {				// CPS2 has 1024 sprites
		nMax = 0x400;
	}

	nFrameCount = 2;			// CPS2 sprites lagged by 1 frame and double buffered
								// CPS1 sprites lagged by 1 frame

	ObjMem = (UINT8*)BurnMalloc((nMax << 3) * nFrameCount);
	if (ObjMem == NULL) {
		return 1;
	}

	// Set up the frame buffers
	for (INT32 i = 0; i < nFrameCount; i++) {
		of[i].Obj = ObjMem + (nMax << 3) * i;
		of[i].nCount = 0;
	}

	nGetNext=0;

	if (Cps == 2) {
		memset(ZBuf, 0, 384 * 224 * 2);
		nMaxZMask = nZOffset = 0;
		nMaxZValue = 1;
	}

	return 0;
}

INT32 CpsObjExit()
{
	for (INT32 i = 0; i < nFrameCount; i++) {
		of[i].Obj = NULL;
		of[i].nCount = 0;
	}

	BurnFree(ObjMem);

	nFrameCount = 0;
	nMax = 0;
	
	Cps1DetectEndSpriteList8000 = 0;
	
	Cps1ObjGetCallbackFunction = NULL;
	Cps1ObjDrawCallbackFunction = NULL;

	return 0;
}

// Get CPS sprites into Obj
INT32 CpsObjGet()
{
	INT32 i;
	UINT8 *pg, *po;
	struct ObjFrame* pof;
	UINT8* Get = NULL;
	
	if (Cps1ObjGetCallbackFunction) {
		return Cps1ObjGetCallbackFunction();
	}

	pof = of + nGetNext;

	pof->nCount = 0;

	po = pof->Obj;
	pof->nShiftX = -0x40;
	pof->nShiftY = -0x10;

	if (Cps == 2) {
		Get = CpsRam708 + ((nCpsObjectBank ^ 1) << 15);		// Select CPS2 sprite buffer
//		Get = CpsRam708 + ((GetCurrentFrame() & 1) << 15);	// Select CPS2 sprite buffer

		pof->nShiftX = -CpsSaveFrg[0][0x9];
		pof->nShiftY = -CpsSaveFrg[0][0xB];
	} else {
		INT32 nOff = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(CpsReg + 0x00))) << 8;
		nOff &= 0xfff800;
		Get = CpsFindGfxRam(nOff, 0x800);		
		
		if (Cps1LockSpriteList910000) {
			Get = CpsFindGfxRam(0x910000, 0x800);
		}
	}
	
	if (Get==NULL) return 1;

	// Make a copy of all active sprites in the list
	for (pg = Get, i = 0; i < nMax; pg += 8, i++) {
		UINT16* ps = (UINT16*)pg;

		if (Cps == 2) {
			if (BURN_ENDIAN_SWAP_INT16(ps[1]) & 0x8000)	{														// end of sprite list
				break;
			}
			if (BURN_ENDIAN_SWAP_INT16(ps[3]) >= 0xff00) {														// end of sprite list (ringdest)
				break;
			}
		} else {
			if (BURN_ENDIAN_SWAP_INT16(ps[3]) >= 0xff00) {													// end of sprite list
				break;
			}
			if (Cps1DetectEndSpriteList8000) {
				if (BURN_ENDIAN_SWAP_INT16(ps[1]) & 0x8000) {
					break;
				}
			}
		}
		
		if ((BURN_ENDIAN_SWAP_INT16(ps[0]) | BURN_ENDIAN_SWAP_INT16(ps[3])) == 0) {													// sprite blank
			continue;
		}

		// Okay - this sprite is active:
		memcpy(po, pg, 8); // copy it over

		pof->nCount++;
		po += 8;
	}

	nGetNext++;
	if (nGetNext >= nFrameCount) {
		nGetNext = 0;
	}

	return 0;
}

void CpsObjDrawInit()
{
	nZOffset = nMaxZMask;

	if (nZOffset >= 0xFC00) {
		// The Z buffer might moverflow the next fram, so initialise it
		memset(ZBuf, 0, 384 * 224 * 2);
		nZOffset = 0;
	}

	nMaxZValue = nZOffset + 1;
	nMaxZMask = nZOffset;

	return;
}

INT32 Cps1ObjDraw(INT32 nLevelFrom,INT32 nLevelTo)
{
	INT32 i; UINT16 *ps; INT32 nPsAdd;
	struct ObjFrame *pof;
	(void)nLevelFrom; (void)nLevelTo;
	
	if (Cps1ObjDrawCallbackFunction) {
		return Cps1ObjDrawCallbackFunction(nLevelFrom, nLevelTo);
	}

	// Draw the earliest frame we have in history
	pof=of+nGetNext;

	// Point to Obj list
	ps=(UINT16 *)pof->Obj;

	if (!CpsDrawSpritesInReverse) {
		ps+=(pof->nCount-1)<<2; nPsAdd=-4; // CPS1 is reversed
	} else {
		nPsAdd=4;
	}

	// Go through all the Objs
	for (i=0; i<pof->nCount; i++,ps+=nPsAdd) {
		INT32 x,y,n,a,bx,by,dx,dy; INT32 nFlip;

		x = BURN_ENDIAN_SWAP_INT16(ps[0]); y = BURN_ENDIAN_SWAP_INT16(ps[1]); n = BURN_ENDIAN_SWAP_INT16(ps[2]); a = BURN_ENDIAN_SWAP_INT16(ps[3]);
			
		// Find out sprite size
		bx=((a>> 8)&15)+1;
		by=((a>>12)&15)+1;
		
		n = GfxRomBankMapper(GFXTYPE_SPRITES, n);
		if (n == -1) continue;
		
		n |= (y & 0x6000) << 3; // high bits of address
		
		// CPS1 coords are 9 bit signed?
		x&=0x01ff; if (x>=0x1c0) x-=0x200;
		y&=0x01ff; y^=0x100; y-=0x100;

		x+=pof->nShiftX;
		y+=pof->nShiftY;

		// Find the palette for the tiles on this sprite
		CpstPal = CpsPal + ((a & 0x1F) << 4);

		nFlip=(a>>5)&3;		

		// Take care with tiles if the sprite goes off the screen
		if (x<0 || y<0 || x+(bx<<4)>384 || y+(by<<4)>224) {
			nCpstType=CTT_16X16 | CTT_CARE;
		} else {
			nCpstType=CTT_16X16;
		}

		nCpstFlip=nFlip;
		for (dy=0;dy<by;dy++) {
			for (dx=0;dx<bx;dx++) {
				INT32 ex,ey;
				if (nFlip&1) ex=(bx-dx-1);
				else ex=dx;
				if (nFlip&2) ey=(by-dy-1);
				else ey=dy;

				nCpstX=x+(ex<<4);
				nCpstY=y+(ey<<4);
				nCpstTile = (n & ~0x0F) + (dy << 4) + ((n + dx) & 0x0F);
				nCpstTile <<= 7;
				CpstOneObjDoX[0]();
			}
		}

	}
	return 0;
}

// Delay sprite drawing by one frame
INT32 Cps2ObjDraw(INT32 nLevelFrom, INT32 nLevelTo)
{
	const INT32 nPsAdd = 4;

	UINT16 *ps;
	struct ObjFrame *pof;
	CpstOneDoFn pCpstOne;
	INT32 nCount;

	bool bMask = 0;

	// Draw the earliest frame we have in history
	pof = of + nGetNext;

	// Point to Obj list
	ps = (UINT16*)pof->Obj + nPsAdd * (nMaxZValue - nZOffset - 1);
	nCount = nZOffset + pof->nCount;

	// Go through all the Objs
	for (ZValue = (UINT16)nMaxZValue; ZValue <= nCount; ZValue++, ps += nPsAdd) {
		INT32 x, y, n, a, bx, by, dx, dy;
		INT32 nFlip;
		INT32 v = BURN_ENDIAN_SWAP_INT16(ps[0]) >> 13;

		if ((nSpriteEnable & (1 << v)) == 0) {
			continue;
		}

		// Check if sprite is between these levels
		if (v > nLevelTo) {
			bMask = 1;
			continue;
		}
		if (v < nLevelFrom) {
			continue;
		}

		if (bMask) {
			nMaxZMask = ZValue;
		} else {
			nMaxZValue = ZValue;
		}

		// Select CpstOne function;
		if (bMask || nMaxZMask > nMaxZValue) {
			pCpstOne = CpstOneObjDoX[1];
		} else {
			pCpstOne = CpstOneObjDoX[0];
		}

		x = BURN_ENDIAN_SWAP_INT16(ps[0]);
		y = BURN_ENDIAN_SWAP_INT16(ps[1]);
		n = BURN_ENDIAN_SWAP_INT16(ps[2]);
		a = BURN_ENDIAN_SWAP_INT16(ps[3]);

		if (a & 0x80) {														// marvel vs capcom ending sprite off-set
			x += CpsSaveFrg[0][0x9];
		}
		
		// CPS2 coords are 10 bit signed (-512 to 511)
		x &= 0x03FF; x ^= 0x200; x -= 0x200;
		y &= 0x03FF; y ^= 0x200; y -= 0x200;

#if 0
		// This *MAY* be needed to get correct sprite positions when raster interrups are used.
		if (nRasterline[1]) {
			for (INT32 i = 1; i < MAX_RASTER; i++) {
				if ((y < nRasterline[i]) || (nRasterline[i] == 0)) {
					x -= CpsSaveFrg[i - 1][0x09];
					y -= CpsSaveFrg[i - 1][0x0B];
					break;
				}
			}
		} else {
			x -= CpsSaveFrg[0][0x9];
			y -= CpsSaveFrg[0][0xB];
		}
#else
		// Ignore sprite offsets when raster interrupts are used (seems to work for all games).
		x += pof->nShiftX;
		y += pof->nShiftY;

//		x -= CpsSaveFrg[0][0x9];
//		y -= CpsSaveFrg[0][0xB];

#endif
		n |= (BURN_ENDIAN_SWAP_INT16(ps[1]) & 0x6000) << 3;	// high bits of address
		
		// Find the palette for the tiles on this sprite
		CpstPal = CpsPal + ((a & 0x1F) << 4);

		nFlip = (a >> 5) & 3;
		// Find out sprite size
		bx = ((a >> 8) & 15) + 1;
		by = ((a >> 12) & 15) + 1;

		// Take care with tiles if the sprite goes off the screen
		if (x < 0 || y < 0 || x + (bx << 4) > 383 || y + (by << 4) > 223) {
			nCpstType = CTT_16X16 | CTT_CARE;
		} else {
			nCpstType = CTT_16X16;
		}

//		if (v == 0) {
//			bprintf(PRINT_IMPORTANT, _T("  - %4i: 0x%04X 0x%04X 0x%04X 0x%04X\n"), ZValue - (UINT16)nMaxZValue, ps[0], ps[1], ps[2], ps[3]);
//		}

		nCpstFlip = nFlip;
		for (dy = 0; dy < by; dy++) {
			for (dx = 0; dx < bx; dx++) {
				INT32 ex, ey;

				if (nFlip & 1) {
					ex = (bx - dx - 1);
				} else {
					ex = dx;
				}

				if (nFlip & 2) {
					ey = (by - dy - 1);
				} else {
					ey = dy;
				}

				nCpstX = x + (ex << 4);
				nCpstY = y + (ey << 4);

//				nCpstTile = n + (dy << 4) + dx;								// normal version
				nCpstTile = (n & ~0x0F) + (dy << 4) + ((n + dx) & 0x0F);	// pgear fix
				nCpstTile <<= 7;						// Find real tile address					

				pCpstOne();
			}
		}
	}

	return 0;
}

// Final crash sprites

INT32 FcrashObjGet()
{
	INT32 i;
	UINT8 *pg, *po;
	struct ObjFrame* pof;
	UINT8* Get = NULL;
	
	pof = of + nGetNext;

	pof->nCount = 0;

	po = pof->Obj;
	pof->nShiftX = -0x40;
	pof->nShiftY = -0x10;

	Get = CpsRam90 + 0x50c8;
	
	if (Get==NULL) return 1;

	// Make a copy of all active sprites in the list
	for (pg = Get, i = 0; i < nMax; pg += 8, i++) {
		UINT16* ps = (UINT16*)pg;
		INT32 n, y, x, a;
		
		y = BURN_ENDIAN_SWAP_INT16(ps[-1]);
		
		if (y == 0x8000) { // end of sprite list
			break;
		}
		
		n = BURN_ENDIAN_SWAP_INT16(ps[0]);
		a = BURN_ENDIAN_SWAP_INT16(ps[1]);
		x = BURN_ENDIAN_SWAP_INT16(ps[2]);

		po[0] = n & 0xff;
		po[1] = n >> 8;
		po[2] = a & 0xff;
		po[3] = a >> 8;
		po[4] = x & 0xff;
		po[5] = x >> 8;
		po[6] = y & 0xff;
		po[7] = y >> 8;	

		pof->nCount++;
		po += 8;
	}

	nGetNext++;
	if (nGetNext >= nFrameCount) {
		nGetNext = 0;
	}

	return 0;
}

INT32 KodbObjGet()
{
	INT32 i;
	UINT8 *pg, *po;
	struct ObjFrame* pof;
	UINT8* Get = NULL;
	
	pof = of + nGetNext;

	pof->nCount = 0;

	po = pof->Obj;
	pof->nShiftX = -0x40;
	pof->nShiftY = -0x10;

	Get = CpsRam90 + 0x50c8;
	
	if (Get==NULL) return 1;

	// Make a copy of all active sprites in the list
	for (pg = Get, i = 0; i < nMax; pg += 8, i++) {
		UINT16* ps = (UINT16*)pg;
		INT32 n, y, x, a;
		
		y = BURN_ENDIAN_SWAP_INT16(ps[-1]);
		
		if (y == 0xffff) { // end of sprite list
			break;
		}
		
		n = BURN_ENDIAN_SWAP_INT16(ps[0]);
		a = BURN_ENDIAN_SWAP_INT16(ps[1]);
		x = BURN_ENDIAN_SWAP_INT16(ps[2]);

		po[0] = n & 0xff;
		po[1] = n >> 8;
		po[2] = a & 0xff;
		po[3] = a >> 8;
		po[4] = x & 0xff;
		po[5] = x >> 8;
		po[6] = y & 0xff;
		po[7] = y >> 8;

		pof->nCount++;
		po += 8;
	}

	nGetNext++;
	if (nGetNext >= nFrameCount) {
		nGetNext = 0;
	}

	return 0;
}

INT32 DinopicObjGet()
{
	INT32 i;
	UINT8 *pg, *po;
	struct ObjFrame* pof;
	UINT8* Get = NULL;
	
	pof = of + nGetNext;

	pof->nCount = 0;

	po = pof->Obj;
	pof->nShiftX = -0x40;
	pof->nShiftY = -0x10;

	Get = CpsBootlegSpriteRam + 0x1000;
	
	if (Get==NULL) return 1;

	// Make a copy of all active sprites in the list
	for (pg = Get, i = 0; i < nMax; pg += 8, i++) {
		UINT16* ps = (UINT16*)pg;
		INT32 n, y, x, a;
		
		y = BURN_ENDIAN_SWAP_INT16(ps[-1]);
		
		if (y == 0x8000) { // end of sprite list
			break;
		}
		
		n = BURN_ENDIAN_SWAP_INT16(ps[0]);
		a = BURN_ENDIAN_SWAP_INT16(ps[1]);
		x = BURN_ENDIAN_SWAP_INT16(ps[2]);
		
		po[0] = n & 0xff;
		po[1] = n >> 8;
		po[2] = a & 0xff;
		po[3] = a >> 8;
		po[4] = x & 0xff;
		po[5] = x >> 8;
		po[6] = y & 0xff;
		po[7] = y >> 8;

		pof->nCount++;
		po += 8;
	}

	nGetNext++;
	if (nGetNext >= nFrameCount) {
		nGetNext = 0;
	}

	return 0;
}

INT32 DaimakaibObjGet()
{
	INT32 i;
	UINT8 *pg, *po;
	struct ObjFrame* pof;
	UINT8* Get = NULL;
	
	pof = of + nGetNext;

	pof->nCount = 0;

	po = pof->Obj;
	pof->nShiftX = -0x40;
	pof->nShiftY = -0x10;

	// writes a blank sprite, followed by end of sprite list marker, start at 0x10 to ignore these
	Get = CpsBootlegSpriteRam + 0x1010;
	
	if (Get==NULL) return 1;

	// Make a copy of all active sprites in the list
	for (pg = Get, i = 0; i < nMax; pg += 8, i++) {
		UINT16* ps = (UINT16*)pg;
		INT32 n, y, x, a;
		
		y = BURN_ENDIAN_SWAP_INT16(ps[-1]);
		
		if (y == 0x8000) { // end of sprite list
			break;
		}
		
		n = BURN_ENDIAN_SWAP_INT16(ps[0]);
		a = BURN_ENDIAN_SWAP_INT16(ps[1]);
		x = BURN_ENDIAN_SWAP_INT16(ps[2]);
		
		n = GfxRomBankMapper(GFXTYPE_SPRITES, n);
		if (n == -1) continue;
		
		n |= (y & 0x6000) << 3; // high bits of address

		po[0] = n & 0xff;
		po[1] = n >> 8;
		po[2] = a & 0xff;
		po[3] = a >> 8;
		po[4] = x & 0xff;
		po[5] = x >> 8;
		po[6] = y & 0xff;
		po[7] = y >> 8;

		pof->nCount++;
		po += 8;
	}

	nGetNext++;
	if (nGetNext >= nFrameCount) {
		nGetNext = 0;
	}

	return 0;
}

INT32 WofhObjGet()
{
	INT32 i;
	UINT8 *pg, *po;
	struct ObjFrame* pof;
	UINT8* Get = NULL;
	
	pof = of + nGetNext;

	pof->nCount = 0;

	po = pof->Obj;
	pof->nShiftX = -0x40;
	pof->nShiftY = -0x10;

	Get = CpsRam90 + 0x1000;
	
	if (Get==NULL) return 1;

	// Make a copy of all active sprites in the list
	for (pg = Get, i = 0; i < nMax; pg += 8, i++) {
		UINT16* ps = (UINT16*)pg;
		INT32 n, y, x, a;
		
		y = BURN_ENDIAN_SWAP_INT16(ps[-1]);
		
		if (y == 0x8000) { // end of sprite list
			break;
		}
		
		n = BURN_ENDIAN_SWAP_INT16(ps[0]);
		a = BURN_ENDIAN_SWAP_INT16(ps[1]);
		x = BURN_ENDIAN_SWAP_INT16(ps[2]);

		po[0] = n & 0xff;
		po[1] = n >> 8;
		po[2] = a & 0xff;
		po[3] = a >> 8;
		po[4] = x & 0xff;
		po[5] = x >> 8;
		po[6] = y & 0xff;
		po[7] = y >> 8;

		pof->nCount++;
		po += 8;
	}

	nGetNext++;
	if (nGetNext >= nFrameCount) {
		nGetNext = 0;
	}

	return 0;
}

INT32 Sf2mdtObjGet()
{
	INT32 i;
	UINT8 *pg, *po;
	struct ObjFrame* pof;
	UINT8* Get = NULL;
	
	pof = of + nGetNext;

	pof->nCount = 0;

	po = pof->Obj;
	pof->nShiftX = -0x40;
	pof->nShiftY = -0x10;

	Get = CpsBootlegSpriteRam + 0x1000;
	
	if (Get==NULL) return 1;

	// Make a copy of all active sprites in the list
	for (pg = Get, i = 0; i < nMax; pg += 8, i++) {
		UINT16* ps = (UINT16*)pg;
		INT32 n, y, x, a;
		
		y = BURN_ENDIAN_SWAP_INT16(ps[-1]);
		
		if (y == 0x8000) { // end of sprite list
			break;
		}
		
		n = BURN_ENDIAN_SWAP_INT16(ps[0]);
		a = BURN_ENDIAN_SWAP_INT16(ps[1]);
		x = BURN_ENDIAN_SWAP_INT16(ps[2] + 0x03);
		
		po[0] = n & 0xff;
		po[1] = n >> 8;
		po[2] = a & 0xff;
		po[3] = a >> 8;
		po[4] = x & 0xff;
		po[5] = x >> 8;
		po[6] = y & 0xff;
		po[7] = y >> 8;

		pof->nCount++;
		po += 8;
	}

	nGetNext++;
	if (nGetNext >= nFrameCount) {
		nGetNext = 0;
	}

	return 0;
}

INT32 FcrashObjDraw(INT32 nLevelFrom,INT32 nLevelTo)
{
	INT32 i; UINT16 *ps; INT32 nPsAdd;
	struct ObjFrame *pof;
	(void)nLevelFrom; (void)nLevelTo;

	// Draw the earliest frame we have in history
	pof=of+nGetNext;

	// Point to Obj list
	ps=(UINT16 *)pof->Obj;

	nPsAdd=4;

	// Go through all the Objs
	for (i=0; i<pof->nCount; i++,ps+=nPsAdd) {
		INT32 x,y,n,a; INT32 nFlip;

		n = BURN_ENDIAN_SWAP_INT16(ps[0]);
		a = BURN_ENDIAN_SWAP_INT16(ps[1]);
		x = BURN_ENDIAN_SWAP_INT16(ps[2]);
		y = BURN_ENDIAN_SWAP_INT16(ps[3]);
		
		x &= 0x1ff;
		y &= 0x1ff;
		
		x -= 16;
		y = 224 - y;

		// Find the palette for the tiles on this sprite
		CpstPal = CpsPal + ((a & 0x1F) << 4);

		nFlip=(a>>5)&3;		

		// Take care with tiles if the sprite goes off the screen
		if (x<0 || y<0 || x+(1<<4)>384 || y+(1<<4)>224) {
			nCpstType=CTT_16X16 | CTT_CARE;
		} else {
			nCpstType=CTT_16X16;
		}

		nCpstFlip=nFlip;
		nCpstX=x;
		nCpstY=y;
		nCpstTile = n;
		nCpstTile <<= 7;
		CpstOneObjDoX[0]();
	}
	
	return 0;
}
