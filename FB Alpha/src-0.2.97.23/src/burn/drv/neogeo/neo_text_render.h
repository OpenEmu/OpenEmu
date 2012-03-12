#define FN(a) RenderTile ## a
#define FUNCTIONNAME(a) FN(a)

#define ISOPAQUE 0

#define ADVANCECOLUMN pPixel += (BPP >> 3)

#define ADVANCEROW pTileRow += ((BPP >> 3) * nNeoScreenWidth)

#if ISOPAQUE == 0
 #define OPACITY _TRANS
 #define TESTCOLOUR(x) x
#elif ISOPAQUE == 1
 #define OPACITY _OPAQUE
 #define TESTCOLOUR(x) 1
#else
 #error illegal isopaque value
#endif

#if BPP == 16
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b)) {						\
   	*((UINT16*)pPixel) = (UINT16)pTilePalette[b];	\
 }
#elif BPP == 24
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b)) {						\
	UINT32 nRGB = pTilePalette[b];							\
	pPixel[0] = (UINT8)nRGB;								\
	pPixel[1] = (UINT8)(nRGB >> 8);							\
	pPixel[2] = (UINT8)(nRGB >> 16);						\
 }
#elif BPP == 32
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b)) {						\
	*((UINT32*)pPixel) = (UINT32)pTilePalette[b];		\
 }
#else
 #error unsupported bitdepth specified.
#endif

static void FUNCTIONNAME(BPP)()
{
	UINT8 *pTileRow, *pPixel;
	INT32 y, nColour;

	for (y = 0, pTileRow = pTile; y < 8; y++, ADVANCEROW) {
		pPixel = pTileRow;

		nColour = *pTileData++;
		PLOTPIXEL(0,nColour >> 4);
		ADVANCECOLUMN;
		PLOTPIXEL(1,nColour & 0x0F);
		ADVANCECOLUMN;

		nColour = *pTileData++;
		PLOTPIXEL(2,nColour >> 4);
		ADVANCECOLUMN;
		PLOTPIXEL(3,nColour & 0x0F);
		ADVANCECOLUMN;

		nColour = *pTileData++;
		PLOTPIXEL(4,nColour >> 4);
		ADVANCECOLUMN;
		PLOTPIXEL(5,nColour & 0x0F);
		ADVANCECOLUMN;

		nColour = *pTileData++;
		PLOTPIXEL(6,nColour >> 4);
		ADVANCECOLUMN;
		PLOTPIXEL(7,nColour & 0x0F);
	}
}

#undef PLOTPIXEL
#undef TESTCLIP
#undef TESTCOLOUR
#undef ADVANCEROW
#undef ADVANCECOLUMN
#undef CLIP
#undef FUNCTIONNAME
#undef FN
