#define FN(a,b,c,d) RenderTile ## a ## _ROT ## b ## c ## d
#define FUNCTIONNAME(a,b,c,d) FN(a,b,c,d)

#define ISOPAQUE 0

#if ROT == 0
 #define ADVANCECOLUMN pPixel += (BPP >> 3)
 #define ADVANCEROW pTileRow += ((BPP >> 3) * 320)
#elif ROT == 270
 #define ADVANCECOLUMN pPixel -= ((BPP >> 3) * 240)
 #define ADVANCEROW pTileRow += (BPP >> 3)
#else
 #error unsupported rotation angle specified.
#endif

#if DOCLIP == 0
 #define CLIP _NOCLIP
 #define TESTCLIP(x) 1
#elif DOCLIP == 1
 #define CLIP _CLIP
 #define TESTCLIP(x) (nTileXPos + x) >= 0 && (nTileXPos + x) < 320
#else
 #error illegal doclip value.
#endif

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
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTCLIP(a)) {			\
   	*((UINT16 *)pPixel) = (UINT16)pTilePalette[b];	\
 }
#elif BPP == 24
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTCLIP(a)) {			\
	UINT32 nRGB = pTilePalette[b];							\
	pPixel[0] = (UINT8)nRGB;								\
	pPixel[1] = (UINT8)(nRGB >> 8);							\
	pPixel[2] = (UINT8)(nRGB >> 16);						\
 }
#elif BPP == 32
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTCLIP(a)) {			\
	 *((UINT32 *)pPixel) = (UINT32)pTilePalette[b];		\
 }
#else
 #error unsupported bitdepth specified.
#endif

#if ROWMODE == 0
 #define MODE _NORMAL
#elif ROWMODE == 1
 #define MODE _ROWSEL
#else
 #error unsupported rowmode specified.
#endif

static void FUNCTIONNAME(BPP,ROT,CLIP,MODE)()
{
#if ROWMODE == 0
	UINT8 *pTileRow, *pPixel;
	INT32 y, nColour;

	for (y = 0, pTileRow = pTile; y < 8; y++, ADVANCEROW) {
		pPixel = pTileRow;
#else
	UINT8 *pPixel = pTile;
	INT32 nColour;
#endif

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

#if ROWMODE == 0
		nColour = *pTileData++;
#else
		nColour = *pTileData;
#endif
		PLOTPIXEL(6,nColour >> 4);
		ADVANCECOLUMN;
		PLOTPIXEL(7,nColour & 0x0F);
#if ROWMODE == 0
	}
#endif
}

#undef MODE
#undef PLOTPIXEL
#undef TESTCLIP
#undef TESTCOLOUR
#undef ADVANCEROW
#undef ADVANCECOLUMN
#undef CLIP
#undef FUNCTIONNAME
#undef FN
