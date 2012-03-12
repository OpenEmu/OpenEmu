// Create a unique name for each of the functions
#define FN(a,b,c,d,e,f,g) RenderSprite ## a ## _ ## b ## _ROT ## c ## d ## e ## _CLIPX ## f ## g
#define FUNCTIONNAME(a,b,c,d,e,f,g) FN(a,b,c,d,e,f,g)

#if ROT == 0
 #define ADVANCEWORD pPixel += ((BPP >> 3) * 16)
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
 #define ADVANCEZWORD
 #define ADVANCEZROW
 #define TESTZBUF(a) 1
 #define WRITEZBUF(a)
#elif ZBUFFER == 1
 #define ZBUF _RZBUFFER
 #define ADVANCEZWORD pZPixel += 16
 #define ADVANCEZROW pZRow += XSIZE
 #define TESTZBUF(a) (pZPixel[a] <= nZPos)
 #define WRITEZBUF(a)
#elif ZBUFFER == 2
 #define ZBUF _WZBUFFER
 #define ADVANCEZWORD pZPixel += 16
 #define ADVANCEZROW pZRow += XSIZE
 #define TESTZBUF(a) 1
 #define WRITEZBUF(a) pZPixel[a] = nZPos
#elif ZBUFFER == 3
 #define ZBUF _RWZBUFFER
 #define ADVANCEZWORD pZPixel += 16
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
 #define ZOOMMODE _ZOOMXY
#else
 #error unsupported rowscroll mode specified.
#endif

#if XFLIP == 0
 #define OFFSET(a) a
 #define CLIP(a,b) if (nColumn >= (XSIZE - a)) { continue; }			\
	if (nXPos >= (0 - a)) { b }
#else
 #define OFFSET(a) (15 - a)
 #define CLIP(a,b) if (nColumn >= (0 - a) && nColumn < (XSIZE - a)) { b }
#endif

#if EIGHTBIT == 0
 #define PLOT8_CLIP(x)		  				 	 	     				\
	CLIP(OFFSET((x + 0)), PLOTPIXEL(OFFSET((x + 0)), nColour & 0x0F));	\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 1)), PLOTPIXEL(OFFSET((x + 1)), nColour & 0x0F));	\
																		\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 2)), PLOTPIXEL(OFFSET((x + 2)), nColour & 0x0F));	\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 3)), PLOTPIXEL(OFFSET((x + 3)), nColour & 0x0F));	\
																		\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 4)), PLOTPIXEL(OFFSET((x + 4)), nColour & 0x0F));	\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 5)), PLOTPIXEL(OFFSET((x + 5)), nColour & 0x0F));	\
																		\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 6)), PLOTPIXEL(OFFSET((x + 6)), nColour & 0x0F));	\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 7)), PLOTPIXEL(OFFSET((x + 7)), nColour & 0x0F));

 #define PLOT8_NOCLIP(x)		   										\
	PLOTPIXEL(OFFSET((x + 0)), nColour & 0x0F);							\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 1)), nColour & 0x0F);							\
																		\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 2)), nColour & 0x0F);							\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 3)), nColour & 0x0F);							\
																		\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 4)), nColour & 0x0F);							\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 5)), nColour & 0x0F);							\
																		\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 6)), nColour & 0x0F);							\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 7)), nColour & 0x0F);
#else
 #define PLOT4_CLIP(x) 				 	 	    		 				\
	CLIP(OFFSET((x + 0)), PLOTPIXEL(OFFSET((x + 0)), nColour & 0xFF));	\
	nColour >>= 8;	  													\
	CLIP(OFFSET((x + 1)), PLOTPIXEL(OFFSET((x + 1)), nColour & 0xFF));	\
	nColour >>= 8;	  													\
																	  	\
	CLIP(OFFSET((x + 2)), PLOTPIXEL(OFFSET((x + 2)), nColour & 0xFF));	\
	nColour >>= 8;														\
	CLIP(OFFSET((x + 3)), PLOTPIXEL(OFFSET((x + 3)), nColour & 0xFF));

 #define PLOT4_NOCLIP(x)	   											\
	PLOTPIXEL(OFFSET((x + 0)), nColour & 0xFF);							\
	nColour >>= 8;														\
	PLOTPIXEL(OFFSET((x + 1)), nColour & 0xFF);							\
	nColour >>= 8;														\
																		\
	PLOTPIXEL(OFFSET((x + 2)), nColour & 0xFF);							\
	nColour >>= 8;														\
	PLOTPIXEL(OFFSET((x + 3)), nColour & 0xFF);
#endif

static void FUNCTIONNAME(BPP,XSIZE,ROT,FLIP,ZOOMMODE,ZBUF,DEPTH)()
{
// Create an empty function if unsupported features are requested
#if ROT == 0

	INT32 x, nColumn;
	INT32 nColour;

 #if ZBUFFER == 0
	for (nSpriteRow = 0; nSpriteRow < nYSize; ADVANCEROW, nSpriteRow++, pSpriteData += nSpriteRowSize) {
 #else
		for (nSpriteRow = 0; nSpriteRow < nYSize; ADVANCEROW, ADVANCEZROW, nSpriteRow++, pSpriteData += nSpriteRowSize) {
 #endif
		nColumn = nXPos;

 #if ZBUFFER == 0
  #if XFLIP == 0
		for (x = (0 << EIGHTBIT), pPixel = pRow; x < nXSize; x += (2 << EIGHTBIT), nColumn += 16, ADVANCEWORD) {
  #else
		for (x = nXSize - (2 << EIGHTBIT), pPixel = pRow; x >= 0; x -= (2 << EIGHTBIT), nColumn += 16, ADVANCEWORD) {
  #endif
 #else
  #if XFLIP == 0
		for (x = 0, pPixel = pRow, pZPixel = pZRow; x < nXSize; x += (2 << EIGHTBIT), nColumn += 16, ADVANCEWORD, ADVANCEZWORD) {
  #else
		for (x = nXSize - (2 << EIGHTBIT), pPixel = pRow, pZPixel = pZRow; x >= 0; x -= (2 << EIGHTBIT), nColumn += 16, ADVANCEWORD, ADVANCEZWORD) {
  #endif
 #endif

 #if EIGHTBIT == 0
			if (nColumn >= 0 && nColumn < (XSIZE - 16)) {
  #if XFLIP == 0
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x]);
				PLOT8_NOCLIP(0);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 1]);
				PLOT8_NOCLIP(8);
  #else
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 1]);
				PLOT8_NOCLIP(8);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x]);
				PLOT8_NOCLIP(0);
  #endif
			} else {
  #if XFLIP == 0
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x]);
				PLOT8_CLIP(0);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 1]);
				PLOT8_CLIP(8);
  #else
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 1]);
				PLOT8_CLIP(8);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x]);
				PLOT8_CLIP(0);
  #endif
 #else
			if (nColumn >= 0 && nColumn < (XSIZE - 16)) {
  #if XFLIP == 0
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x]);
				PLOT4_NOCLIP(0);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 1]);
				PLOT4_NOCLIP(4);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 2]);
				PLOT4_NOCLIP(8);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 3]);
				PLOT4_NOCLIP(12);
  #else
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 3]);
				PLOT4_NOCLIP(12);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 2]);
				PLOT4_NOCLIP(8);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 1]);
				PLOT4_NOCLIP(4);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x]);
				PLOT4_NOCLIP(0);
  #endif
			} else {
  #if XFLIP == 0
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x]);
				PLOT4_CLIP(0);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 1]);
				PLOT4_CLIP(4);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 2]);
				PLOT4_CLIP(8);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 3]);
				PLOT4_CLIP(12);
  #else
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 3]);
				PLOT4_CLIP(12);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 2]);
				PLOT4_CLIP(8);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x + 1]);
				PLOT4_CLIP(4);
				nColour = BURN_ENDIAN_SWAP_INT32(pSpriteData[x]);
				PLOT4_CLIP(0);
  #endif
 #endif
			}
		}
	}
#endif
}

#undef PLOT4_CLIP
#undef PLOT4_NOCLIP
#undef PLOT8_CLIP
#undef PLOT8_NOCLIP
#undef OFFSET
#undef FLIP
#undef PLOTPIXEL
#undef CLIP
#undef TESTCOLOUR
#undef ADVANCEZROW
#undef ADVANCEZWORD
#undef ADVANCEROW
#undef ADVANCEWORD
#undef TESTZBUF
#undef WRITEZBUF
#undef ZBUF
#undef ZOOMMODE
#undef DEPTH
#undef FUNCTIONNAME
#undef FN

