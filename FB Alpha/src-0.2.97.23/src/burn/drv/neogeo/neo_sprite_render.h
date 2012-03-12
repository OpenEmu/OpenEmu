// Create a unique name for each of the functions
#define FN(a,b,c,d) RenderBank ## a ## _ZOOM ## b ## c ## d
#define FUNCTIONNAME(a,b,c,d) FN(a,b,c,d)

#if DOCLIP == 0
 #define CLIP _NOCLIP
 #define TESTCLIP(x) 1
#elif DOCLIP == 1
 #define CLIP _CLIP
 #define TESTCLIP(x) (nBankXPos + x) >= 0 && (nBankXPos + x) < nNeoScreenWidth
#else
 #error illegal doclip value.
#endif

#if ISOPAQUE == 0
 #define OPACITY _TRANS
 #define TESTCOLOUR(x) x
#else
 #error illegal isopaque value
#endif

#if BPP == 16
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTCLIP(a)) {			\
   	*((UINT16*)pPixel) = (UINT16)pTilePalette[b];	\
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
	 *((UINT32*)pPixel) = (UINT32)pTilePalette[b];		\
 }
#else
 #error unsupported bitdepth specified.
#endif

#if XZOOM == 0
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);
#elif XZOOM == 1
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);
#elif XZOOM == 2
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);
#elif XZOOM == 3
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);
#elif XZOOM == 4
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);
#elif XZOOM == 5
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);
#elif XZOOM == 6
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);
#elif XZOOM == 7
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);
#elif XZOOM == 8
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);
#elif XZOOM == 9
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);
#elif XZOOM == 10
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);
#elif XZOOM == 11
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);
#elif XZOOM == 12
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(12),nColour & 0x0F);
#elif XZOOM == 13
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(12),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(13),nColour & 0x0F);
#elif XZOOM == 14
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET(5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(12),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(13),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(14),nColour & 0x0F);
#elif XZOOM == 15
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine +	1];			\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(12),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(13),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(14),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(15),nColour & 0x0F);
#else
 #error unsupported zoom factor specified.
#endif

#define NORMALOFFSET(x) (x)
#define MIRROROFFSET(x) (XZOOM - x)

// #undef USE_SPEEDHACKS

static void FUNCTIONNAME(BPP,XZOOM,CLIP,OPACITY)()
{
	UINT8 *pTileRow, *pPixel;
	INT32 nColour = 0, nTransparent = 0;
	INT32 nTileNumber, nTileAttrib = 0;
	INT32 nTile, nLine;
	INT32 nPrevTile;
	INT32 nYPos;

	UINT8* pZoomValue = NeoZoomROM + (nBankYZoom << 8);

	INT32 nLinesTotal = (nBankSize >= 0x20) ? 0x01FF : ((nBankSize << 4) - 1);
	INT32 nLinesDone  = 0;

	while (nLinesDone <= nLinesTotal) {
		nLine = (nBankYPos + nLinesDone) & 0x01FF;
		nYPos = nLine;

//		bprintf(PRINT_NORMAL, _T("  - s:%i l:%i y:%i %i z:%i\n"), nLinesTotal, nLinesDone, nYPos, nBankYPos, nBankYZoom);

		// Skip everything above the part of the display we need to render
		if (nYPos < nSliceStart) {
			nLinesDone += nSliceStart - nYPos;
			continue;
		}
		// Skip everything below the part of the display we need to render
		if (nYPos >= nSliceEnd) {
			nLinesDone += nSliceStart + 512 - nYPos;
			continue;
		}

		// This part of the sprite strip is in the part of the display we need to render
		{
			INT32 nStartTile = (nLinesDone >= 0x0100) ? 0x10 : 0;
			INT32 nStartLine = nLinesDone & 0xFF;
			INT32 nEndLine   = (nLinesDone < 0x0100 && nLinesTotal >= 0x0100) ? 0xFF : nLinesTotal & 0xFF;

			INT32 nThisLine;

			// Handle wraparound for full-size sprite strips
			if (nBankSize > 0x10 && nBankYZoom != 0xFF) {

				if (nBankSize <= 0x20) {

					// normal wrap

					if (nLinesDone >= 0x0100) {
						if (nLinesDone < (0x01FF - nBankYZoom)) {
							nLinesDone = (0x01FF - nBankYZoom);

							continue;
						}

						nStartLine -= 0xFF - nBankYZoom;
						nEndLine -= 0xFF - nBankYZoom;
					}

				} else {

					// Full strip, full wrap

					if (nLinesDone >= 0x0100) {
						nStartLine -= 0xFF - nBankYZoom;
						if (nStartLine < 0) {

#if 1 && defined USE_SPEEDHACKS
							nStartLine += nBankYZoom + 1;
							if (nStartLine < 0) {
								nLinesDone = 0x200;
								continue;
							}
#else
							nStartLine = nBankYZoom - (-nStartLine - 1) % (nBankYZoom + 1);
#endif

							nStartTile = 0;
						}
					} else {
						if (nStartLine > nBankYZoom) {

#if 1 && defined USE_SPEEDHACKS
							nStartLine -= nBankYZoom + 1;
							if (nStartLine > nBankYZoom) {
								nLinesDone = 0x0100;
								continue;
							}
#else
							nStartLine %= nBankYZoom + 1;
#endif

							nStartTile = 0x10;
						}
					}

					nEndLine = nBankYZoom;
				}
			}

			nLinesDone += nEndLine - nStartLine + 1;

#if 1 && defined USE_SPEEDHACKS
			if (nBankSize <= 0x20 && nEndLine > nBankYZoom) {
				nEndLine = nBankYZoom;
//				nLinesDone |= 0x00FF;
			}
#endif

			// Clip to the part of the screen we need to render
			if (nEndLine - nStartLine > nSliceEnd - nYPos - 1) {
				nEndLine = nStartLine + nSliceEnd - nYPos - 1;
			}

			pTileRow = pBurnDraw + (nYPos - 0x10) * (BPP >> 3) * nNeoScreenWidth + nBankXPos * (BPP >> 3);
			nThisLine = nStartLine;

			nPrevTile = ~0;

			while (nThisLine <= nEndLine) {

				nTile = nStartTile + (pZoomValue[nThisLine] >> 4);

				if (nTile != nPrevTile) {
					nPrevTile = nTile;

					nTileNumber = pBank[nTile << 1];
					nTileAttrib = pBank[(nTile << 1) + 1];

					nTileNumber += (nTileAttrib & 0xF0) << 12;
					nTileNumber &= nNeoTileMaskActive;

					if (nTileAttrib & 8) {
						nTileNumber &= ~7;
						nTileNumber |= nNeoSpriteFrame08;
					} else {
						if (nTileAttrib & 4) {
							nTileNumber &= ~3;
							nTileNumber |= nNeoSpriteFrame04;
						}
					}

					nTransparent = NeoTileAttribActive[nTileNumber];

					if (nTransparent == 0) {
						pTileData = (UINT32*)(NeoSpriteROMActive + (nTileNumber << 7));
						pTilePalette = &NeoPalette[(nTileAttrib & 0xFF00) >> 4];
					}
				}

				if (nTransparent == 0) {
					nLine = (pZoomValue[nThisLine] & 0x0F) << 1;
					if (nTileAttrib & 2) {							// Flip Y
						nLine ^= 0x1E;
					}

					if (nTileAttrib & 1) {							// Flip X
						pPixel = pTileRow + XZOOM * (BPP >> 3);
						PLOTLINE(MIRROROFFSET,pPixel -= (BPP >> 3));
					} else {
						pPixel = pTileRow;
						PLOTLINE(NORMALOFFSET,pPixel += (BPP >> 3));
					}
				}

				pTileRow += ((BPP >> 3) * nNeoScreenWidth);

				nThisLine++;
			}
		}
	}
}

#undef PLOTLINE
#undef OPACITY
#undef PLOTPIXEL
#undef TESTCOLOUR
#undef TESTCLIP
#undef CLIP
