// Graphics format: 16 × 16 pixel tiles, 4 bits/pixel.

// Create a unique name for each of the functions
#define FN(a,b,c,d,e,f,g,h) RenderTile ## a ## b ## c ## _ROT ## d ## e ## f ## g ## h
#define FUNCTIONNAME(a,b,c,d,e,f,g,h) FN(a,b,c,d,e,f,g,h)

#if FLIP == 0
 #define DOFLIP _NOFLIP
 #define FLIPX 0
 #define FLIPY 0
#elif FLIP == 1
 #define DOFLIP _FLIPX
 #define FLIPX 1
 #define FLIPY 0
#elif FLIP == 2
 #define DOFLIP _FLIPY
 #define FLIPX 0
 #define FLIPY 1
#elif FLIP == 3
 #define DOFLIP _FLIPXY
 #define FLIPX 1
 #define FLIPY 1
#else
 #error unsupported flip mode specified.
#endif

#if ZOOM == 0
 #define ZOOMMODE _NOZOOM
 #define CHECKX(x)
 #if FLIPX == 0
  #define OFFSET(x) (x)
 #else
  #define OFFSET(x) (15 - (x))
 #endif
#elif ZOOM == 1
 #define ZOOMMODE _ZOOM
 #define CHECKX(x) if (x >= nTileXSize) { continue; }
 #if FLIPX == 0
  #define OFFSET(x) (pXZoomInfo[x])
 #else
  #define OFFSET(x) (15 - pXZoomInfo[x])
 #endif
#else
 #error unsupported zoom mode specified.
#endif

#if ROT == 0
 #if FLIPY == 0
  #define ADVANCEROW pTileRow += ((BPP >> 3) * 320)
 #else
  #define ADVANCEROW pTileRow -= ((BPP >> 3) * 320)
 #endif
#else
 #error unsupported rotation angle specified
#endif

#if DOCLIP == 0
 #define CLIP _NOCLIP
#elif DOCLIP == 1
 #define CLIP _CLIP
#else
 #error illegal doclip value.
#endif

#if TRANS == 15 || TRANS == 0
 #define TESTCOLOUR(x) ((x) != TRANS)
 #if TRANS == 0
  #define TRANSMODE _TRANS0
 #else
  #define TRANSMODE _TRANS15
 #endif
#elif TRANS == -1
 #define TESTCOLOUR(x) 1
 #define TRANSMODE _SOLID
#else
 #error unsupported transparency mode specified
#endif

#if ZBUFFER == 0
 #define ZBUF _NOZBUFFER
 #define ADVANCEZROW
 #define TESTZBUF(a) 1
 #define WRITEZBUF(a)
#elif ZBUFFER == 1
 #define ZBUF _RZBUFFER
 #define TESTZBUF(a) (pZTileRow[a] <= nZPos)
 #define WRITEZBUF(a)
#elif ZBUFFER == 2
 #define ZBUF _WZBUFFER
 #define TESTZBUF(a) 1
 #define WRITEZBUF(a) pZTileRow[a] = nZPos
#elif ZBUFFER == 3
 #define ZBUF _RWZBUFFER
 #define TESTZBUF(a) (pZTileRow[a] <= nZPos)
 #define WRITEZBUF(a) pZTileRow[a] = nZPos
#else
 #error unsupported zbuffer mode specified.
#endif

#if ZBUFFER != 0
 #if FLIPY == 0
  #define ADVANCEZROW pZTileRow += 320
 #else
  #define ADVANCEZROW pZTileRow -= 320
 #endif
#endif

#if BPP == 16
 #define PLOTPIXEL(a, b) if (TESTCOLOUR(b) && TESTZBUF(a)) {			\
   	WRITEZBUF(a);														\
   	((UINT16*)pTileRow)[a] = (UINT16)pTilePalette[b];	\
 }
#elif BPP == 24
 #define PLOTPIXEL(a, b) if (TESTCOLOUR(b) && TESTZBUF(a)) {			\
	UINT32 nRGB = pTilePalette[b];								\
   	WRITEZBUF(a);														\
	pTileRow[3 * a + 0] = (UINT8)nRGB;							\
	pTileRow[3 * a + 1] = (UINT8)(nRGB >> 8);					\
	pTileRow[3 * a + 2] = (UINT8)(nRGB >> 16);					\
 }
#elif BPP == 32
 #define PLOTPIXEL(a, b) if (TESTCOLOUR(b) && TESTZBUF(a)) {			\
   	WRITEZBUF(a);														\
	((UINT32*)pTileRow)[a] = (UINT32)pTilePalette[b];		\
 }
#else
 #error unsupported bitdepth specified.
#endif

// For decorating the function name
#if ROWSCROLL == 0
 #define SCROLL _NOROWSCROLL
#elif ROWSCROLL == 1
 #define SCROLL _ROWSCROLL
#else
 #error unsupported rowscroll mode specified.
#endif

static void FUNCTIONNAME(BPP,TRANSMODE,DOFLIP,ROT,SCROLL,ZOOMMODE,ZBUF,CLIP)()
{
// Create an empty function if unsupported features are requested
#if ROT == 0

	UINT8* pTileRow;
 #if ZBUFFER != 0
	UINT16* pZTileRow;
 #endif

 #if ROWSCROLL == 1
	INT32 nRowOffset;
 #endif
	INT32 y;

 #if ZOOM == 1
  #if ZBUFFER == 0
   #if FLIPY == 0
	for (y = 0, pTileRow = pTile; y < nTileYSize; ADVANCEROW, pTileData += pYZoomInfo[y], y++) {
   #else
  	for (y = nTileYSize - 1, pTileRow = pTile + ((BPP >> 3) * 320 * (nTileYSize - 1)); y >= 0; ADVANCEROW, pTileData += pYZoomInfo[nTileYSize - 1 - y], y--) {
   #endif
  #else
   #if FLIPY == 0
  	for (y = 0, pTileRow = pTile, pZTileRow = pZTile; y < nTileYSize; ADVANCEROW, ADVANCEZROW, pTileData += pYZoomInfo[y], y++) {
   #else
  	for (y = nTileYSize - 1, pTileRow = pTile + ((BPP >> 3) * 320 * (nTileYSize - 1)), pZTileRow = pZTile + (320 * (nTileYSize - 1)); y >= 0; ADVANCEROW, ADVANCEZROW, pTileData += pYZoomInfo[nTileYSize - 1 - y],  y--) {
   #endif
  #endif
 #else
  #if ZBUFFER == 0
   #if FLIPY == 0
	for (y = 0, pTileRow = pTile; y < 16; y++, ADVANCEROW, pTileData += 16) {
   #else
  	for (y = 15, pTileRow = pTile + ((BPP >> 3) * 320 * 15); y >= 0; y--, ADVANCEROW, pTileData += 16) {
   #endif
  #else
   #if FLIPY == 0
  	for (y = 0, pTileRow = pTile, pZTileRow = pZTile; y < 16; y++, ADVANCEROW, ADVANCEZROW, pTileData += 16) {
   #else
  	for (y = 15, pTileRow = pTile + ((BPP >> 3) * 320 * 15), pZTileRow = pZTile + (320 * 15); y >= 0; y--, ADVANCEROW, ADVANCEZROW, pTileData += 16) {
   #endif
  #endif
 #endif

 #if DOCLIP == 1
		if (nTileYPos + y < 0) {
  #if FLIPY == 0
			continue;
  #else
			return;
  #endif
		}

		if (nTileYPos + y >= 224) {
  #if FLIPY == 0
			return;
  #else
			continue;
  #endif
		}
 #endif

  #if ROWSCROLL == 1
		nRowOffset = (nTileXPos - BURN_ENDIAN_SWAP_INT16(pTileRowInfo[(nTileYPos + y) & 0xFF])) & nTilemapWith;
		if (nRowOffset >= 320) {
			nRowOffset -= nTilemapWith + 1;
		}
		if (nRowOffset <= -16) {
			continue;
		}
  #endif

 #if ROWSCROLL == 1
  #define XPOS nRowOffset
  #define ROWOFFSET nRowOffset
 #else
  #define XPOS nTileXPos
  #define ROWOFFSET 0
 #endif

 #if DOCLIP == 1 || ROWSCROLL == 1
  #define CLIPPIXEL(a,b) if (XPOS >= 0 - (a) && XPOS < 320 - (a)) { PLOTPIXEL(ROWOFFSET + a,b); }
		CLIPPIXEL( 0, pTileData[OFFSET( 0)]);
		CLIPPIXEL( 1, pTileData[OFFSET( 1)]);
		CLIPPIXEL( 2, pTileData[OFFSET( 2)]);
		CLIPPIXEL( 3, pTileData[OFFSET( 3)]);
		CLIPPIXEL( 4, pTileData[OFFSET( 4)]);
		CLIPPIXEL( 5, pTileData[OFFSET( 5)]);
		CLIPPIXEL( 6, pTileData[OFFSET( 6)]);
		CLIPPIXEL( 7, pTileData[OFFSET( 7)]); CHECKX( 8);
		CLIPPIXEL( 8, pTileData[OFFSET( 8)]); CHECKX( 9);
		CLIPPIXEL( 9, pTileData[OFFSET( 9)]); CHECKX(10);
		CLIPPIXEL(10, pTileData[OFFSET(10)]); CHECKX(11);
		CLIPPIXEL(11, pTileData[OFFSET(11)]); CHECKX(12);
		CLIPPIXEL(12, pTileData[OFFSET(12)]); CHECKX(13);
		CLIPPIXEL(13, pTileData[OFFSET(13)]); CHECKX(14);
		CLIPPIXEL(14, pTileData[OFFSET(14)]); CHECKX(15);
		CLIPPIXEL(15, pTileData[OFFSET(15)]);
  #undef CLIPPIXEL
 #else
		PLOTPIXEL( 0, pTileData[OFFSET( 0)]);
		PLOTPIXEL( 1, pTileData[OFFSET( 1)]);
		PLOTPIXEL( 2, pTileData[OFFSET( 2)]);
		PLOTPIXEL( 3, pTileData[OFFSET( 3)]);
		PLOTPIXEL( 4, pTileData[OFFSET( 4)]);
		PLOTPIXEL( 5, pTileData[OFFSET( 5)]);
		PLOTPIXEL( 6, pTileData[OFFSET( 6)]);
		PLOTPIXEL( 7, pTileData[OFFSET( 7)]); CHECKX( 8);
		PLOTPIXEL( 8, pTileData[OFFSET( 8)]); CHECKX( 9);
		PLOTPIXEL( 9, pTileData[OFFSET( 9)]); CHECKX(10);
		PLOTPIXEL(10, pTileData[OFFSET(10)]); CHECKX(11);
		PLOTPIXEL(11, pTileData[OFFSET(11)]); CHECKX(12);
		PLOTPIXEL(12, pTileData[OFFSET(12)]); CHECKX(13);
		PLOTPIXEL(13, pTileData[OFFSET(13)]); CHECKX(14);
		PLOTPIXEL(14, pTileData[OFFSET(14)]); CHECKX(15);
		PLOTPIXEL(15, pTileData[OFFSET(15)]);
 #endif

 #undef ROWOFFSET
 #undef XPOS
	}
#endif
}

#undef DOFLIP
#undef FLIPX
#undef FLIPY
#undef OFFSET
#undef CHECKX
#undef ZOOMMODE
#undef ZBUF
#undef TRANSMODE
#undef CLIP
#undef WRITEZBUF
#undef PLOTPIXEL
#undef TESTCOLOUR
#undef TESTZBUF
#undef ADVANCEROW
#undef ADVANCEZROW
#undef SCROLL
#undef SELECT
#undef DEPTH
#undef FUNCTIONNAME
#undef FN

