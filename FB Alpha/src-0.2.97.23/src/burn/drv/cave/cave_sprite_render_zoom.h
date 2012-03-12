// Create a unique name for each of the functions
#define FN(a,b,c,d,e,f,g) RenderSprite ## a ## _ ## b ## _ROT ## c ## d ## e ## _NOCLIP ## f ## g
#define FUNCTIONNAME(a,b,c,d,e,f,g) FN(a,b,c,d,e,f,g)

#if ROT == 0
 #define ADVANCECOLUMN pPixel += (BPP >> 3)
 #define ADVANCEROW pRow += ((BPP >> 3) * XSIZE)
#else
 #error unsupported rotation angle specified
#endif

#if EIGHTBIT == 0
 #define DEPTH _16
#elif EIGHTBIT == 1
 #define DEPTH _256
#else
 #error illegal eightbit value
#endif

#define TESTCOLOUR(x) x

#if ZBUFFER == 0
 #define ZBUF _NOZBUFFER
 #define ADVANCEZCOLUMN
 #define ADVANCEZROW
 #define TESTZBUF(a) 1
 #define WRITEZBUF(a)
#elif ZBUFFER == 1
 #define ZBUF _RZBUFFER
 #define ADVANCEZCOLUMN pZPixel += 1
 #define ADVANCEZROW pZRow += XSIZE
 #define TESTZBUF(a) (pZPixel[a] <= nZPos)
 #define WRITEZBUF(a)
#elif ZBUFFER == 2
 #define ZBUF _WZBUFFER
 #define ADVANCEZCOLUMN pZPixel += 1
 #define ADVANCEZROW pZRow += XSIZE
 #define TESTZBUF(a) 1
 #define WRITEZBUF(a) pZPixel[a] = nZPos
#elif ZBUFFER == 3
 #define ZBUF _RWZBUFFER
 #define ADVANCEZCOLUMN pZPixel += 1
 #define ADVANCEZROW pZRow += XSIZE
 #define TESTZBUF(a) (pZPixel[a] <= nZPos)
 #define WRITEZBUF(a) pZPixel[a] = nZPos
#else
 #error unsupported zbuffer mode specified.
#endif

#if BPP == 16
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTZBUF(a)) {						\
   	WRITEZBUF(a);																\
	*((UINT16*)(pPixel + a * 2)) = (UINT16)pSpritePalette[b];	\
 }
#elif BPP == 24
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTZBUF(a)) {						\
	WRITEZBUF(a);																\
	UINT32 nRGB = pSpritePalette[b];										\
	pPixel[a * 3 + 0] = (UINT8)nRGB;									\
	pPixel[a * 3 + 1] = (UINT8)(nRGB >> 8);								\
	pPixel[a * 3 + 2] = (UINT8)(nRGB >> 16);							\
 }
#elif BPP == 32
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTZBUF(a)) {						\
	WRITEZBUF(a);																\
	*((UINT32*)(pPixel + a * 4)) = (UINT32)pSpritePalette[b];		\
 }
#else
 #error unsupported bitdepth specified.
#endif

#if XFLIP == 0
 #define FLIP _NOFLIP
#elif XFLIP == 1
 #define FLIP _FLIPX
#else
 #error illegal XFLIP value
#endif

#if ZOOM == 0
 #define ZOOMMODE _NOZOOM
#elif ZOOM == 1
 #define ZOOMMODE _ZOOMOUT
#elif ZOOM == 2
 #define ZOOMMODE _ZOOMIN
#else
 #error unsupported zoom mode specified.
#endif

static void FUNCTIONNAME(BPP,XSIZE,ROT,FLIP,ZOOMMODE,ZBUF,DEPTH)()
{
// Create an empty function if unsupported features are requested
#if ROT == 0 && XFLIP == 0 && EIGHTBIT == 1

	INT32 nSpriteColumn;

	INT32 nSpriteXOffset2;

 #if ZOOM == 2
	INT32 nPrevSpriteXOffset;
	INT32 nPrevSpriteXOffsetStart = nSpriteXOffset & 0xFFFF0000;
	INT32 nPrevSpriteYOffset = nSpriteYOffset & 0xFFFF0000;

	if (nPrevSpriteXOffsetStart == 0) {
		nPrevSpriteXOffsetStart = 0xFEDC1234;
	}

	if (nPrevSpriteYOffset == 0) {
		nPrevSpriteYOffset = 0xFEDC1234;
	}
 #endif

	UINT8* pSpriteRowData = (UINT8*)pSpriteData;

	for (nSpriteRow = nYSize; nSpriteRow > 0; nSpriteRow -= 0x00010000, nSpriteYOffset += nSpriteYZoomSize) {

 #if ZOOM == 2
		if ((nSpriteYOffset & 0xFFFF0000) == (nPrevSpriteYOffset & 0xFFFF0000)) {

			ADVANCEROW;
  #if ZBUFFER != 0
			ADVANCEZROW;
  #endif

			continue;
		}
		nPrevSpriteYOffset = nSpriteYOffset;
		nPrevSpriteXOffset = nPrevSpriteXOffsetStart;
 #endif
		pSpriteRowData = ((UINT8*)pSpriteData) + (nSpriteYOffset >> 16) * nSpriteRowSize;

		nSpriteXOffset2 = nSpriteXOffset;

 #if ZBUFFER != 0
		pZPixel = pZRow;
 #endif

		for (nSpriteColumn = nXSize, pPixel = pRow; nSpriteColumn > 0; nSpriteColumn -= 0x00010000, nSpriteXOffset2 += nSpriteXZoomSize) {

 #if ZOOM == 2
			if ((nSpriteXOffset2 & 0xFFFF0000) != (nPrevSpriteXOffset & 0xFFFF0000)) {

				nPrevSpriteXOffset = nSpriteXOffset2;
 #endif
				PLOTPIXEL(0, pSpriteRowData[nSpriteXOffset2 >> 16]);

 #if ZOOM == 2
			}
 #endif

			ADVANCECOLUMN;
 #if ZBUFFER != 0
			ADVANCEZCOLUMN;
 #endif

		}

		ADVANCEROW;
 #if ZBUFFER != 0
		ADVANCEZROW;
 #endif
	}

#endif
}

#undef OFFSET
#undef FLIP
#undef PLOTPIXEL
#undef TESTCOLOUR
#undef ADVANCEZROW
#undef ADVANCEZCOLUMN
#undef ADVANCEROW
#undef ADVANCECOLUMN
#undef TESTZBUF
#undef WRITEZBUF
#undef ZBUF
#undef ZOOMMODE
#undef DEPTH
#undef FUNCTIONNAME
#undef FN

