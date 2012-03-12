// Graphics format: 8 × 8 pixel tiles, 4/8 bits/pixel.

// Create a unique name for each of the functions
#define FN(a,b,c,d,e,f,g,h) RenderTile ## a ## _ ## b ## _ROT ## c ## d ## e ## f ## g ## h
#define FUNCTIONNAME(a,b,c,d,e,f,g,h) FN(a,b,c,d,e,f,g,h)

#if ROT == 0

 #if XFLIP == 0
  #define ADVANCECOLUMN pPixel += (BPP >> 3)
 #else
  #error illegal XFLIP value
 #endif

 #if YFLIP == 0
  #define ADVANCEROW pTileRow += ((BPP >> 3) * XSIZE)
 #else
  #error illegal YFLIP value
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

#if EIGHTBIT == 0
 #define DEPTH _16
#elif EIGHTBIT == 1
 #define DEPTH _256
#else
 #error illegal eightbit value
#endif

#define TESTCOLOUR(x) x

#if BPP == 16
 #define PLOTPIXEL(a) if (TESTCOLOUR(a)) {							\
   	*((UINT16*)pPixel) = (UINT16)pTilePalette[a];	\
 }
#elif BPP == 24
 #define PLOTPIXEL(a) if (TESTCOLOUR(a)) {							\
	UINT32 nRGB = pTilePalette[a];							\
	pPixel[0] = (UINT8)nRGB;								\
	pPixel[1] = (UINT8)(nRGB >> 8);							\
	pPixel[2] = (UINT8)(nRGB >> 16);						\
 }
#elif BPP == 32
 #define PLOTPIXEL(a) if (TESTCOLOUR(a)) {							\
	 *((UINT32*)pPixel) = (UINT32)pTilePalette[a];		\
 }
#else
 #error unsupported bitdepth specified.
#endif

// For decorating the function name
#if XFLIP == 1
 #if YFLIP == 1
  #define FLIP _FLIPXY
 #else
  #define FLIP _FLIPX
 #endif
#elif YFLIP == 1
 #define FLIP _FLIPY
#else
 #define FLIP _NOFLIP
#endif

#if ROWSELECT == 0
 #define SELECT _NOROWSELECT
#elif ROWSELECT == 1
 #define SELECT _ROWSELECT
#else
 #error unsupported rowselect mode specified.
#endif

#if ROWSCROLL == 0
 #define SCROLL _NOROWSCROLL
#elif ROWSCROLL == 1
 #define SCROLL _ROWSCROLL
#else
 #error unsupported rowscroll mode specified.
#endif

static void FUNCTIONNAME(BPP,XSIZE,ROT,FLIP,SCROLL,SELECT,CLIP,DEPTH)()
{
// Create an empty function if unsupported features are requested
#if ROT == 0 && XFLIP == 0 && YFLIP == 0 && !(ROWSCROLL == 1 && ROWSELECT == 1) && EIGHTBIT == 1

	UINT8 *pTileRow, *pPixel;
	INT32 nColour;

 #if ROWSELECT == 0
	INT32 y;
 #endif

 #if ROWSELECT == 0
  	for (y = 0, pTileRow = pTile; y < 8; y++, ADVANCEROW) {

  #if DOCLIP == 1
		if (nTileYPos + y < 0) {

   #if EIGHTBIT == 0
			pTileData++;
   #else
			pTileData += 2;
   #endif

			continue;
		}

		if (nTileYPos + y >= nCaveYSize) {
			return;
		}
  #endif

 #else
		pTileRow = pTile;
 #endif

		pPixel = pTileRow;

 #if ROWSCROLL == 1
		nRowOffset = (nTileXPos - pTileRowInfo[y]) & 0x01FF;
		if (nRowOffset >= 0x01F8) {
			nRowOffset -= 0x0200;
		}
		if (nRowOffset >= XSIZE) {
  #if EIGHTBIT == 0
			pTileData++;
  #else
			pTileData += 2;
  #endif
			continue;
		}
		pPixel += (BPP >> 3) * nRowOffset;
 #endif

 #if ROWSCROLL == 1
  #define XPOS nRowOffset
 #else
  #define XPOS nTileXPos
 #endif

 #if EIGHTBIT == 0
  #if DOCLIP == 1 || ROWSCROLL == 1
		nColour = BURN_ENDIAN_SWAP_INT32(*pTileData++);

		if (XPOS <= (XSIZE - 8)) {
			if (XPOS < 0) {
				nColour >>= -XPOS * 4;
				pPixel += -XPOS * (BPP >> 3);
			}

			switch (XPOS < 0 ? -XPOS : 0) {
				case 0:
					PLOTPIXEL(nColour & 0x0F);
					ADVANCECOLUMN;
					nColour >>= 4;
				case 1:
					PLOTPIXEL(nColour & 0x0F);
					ADVANCECOLUMN;
					nColour >>= 4;
				case 2:
					PLOTPIXEL(nColour & 0x0F);
					ADVANCECOLUMN;
					nColour >>= 4;
				case 3:
					PLOTPIXEL(nColour & 0x0F);
					ADVANCECOLUMN;
					nColour >>= 4;
				case 4:
					PLOTPIXEL(nColour & 0x0F);
					ADVANCECOLUMN;
					nColour >>= 4;
				case 5:
					PLOTPIXEL(nColour & 0x0F);
					ADVANCECOLUMN;
					nColour >>= 4;
				case 6:
					PLOTPIXEL(nColour & 0x0F);
					ADVANCECOLUMN;
					nColour >>= 4;
				case 7:
					PLOTPIXEL(nColour & 0x0F);
			}
		} else {

   #define CLIPPIXEL(a,b) if (XPOS < XSIZE - (a)) { b; }
			CLIPPIXEL(0, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(1, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(2, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(3, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(4, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(5, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(6, PLOTPIXEL(nColour & 0xFF));
   #undef CLIPPIXEL

		}
  #else
		nColour = BURN_ENDIAN_SWAP_INT32(*pTileData++);
		PLOTPIXEL(nColour & 0x0F);
		ADVANCECOLUMN;
		nColour >>= 4;
		PLOTPIXEL(nColour & 0x0F);
		ADVANCECOLUMN;

		nColour >>= 4;
		PLOTPIXEL(nColour & 0x0F);
		ADVANCECOLUMN;
		nColour >>= 4;
		PLOTPIXEL(nColour & 0x0F);
		ADVANCECOLUMN;

		nColour >>= 4;
		PLOTPIXEL(nColour & 0x0F);
		ADVANCECOLUMN;
		nColour >>= 4;
		PLOTPIXEL(nColour & 0x0F);
		ADVANCECOLUMN;

		nColour >>= 4;
		PLOTPIXEL(nColour & 0x0F);
		ADVANCECOLUMN;
		nColour >>= 4;
		PLOTPIXEL(nColour & 0x0F);
  #endif
 #else
  #if DOCLIP == 1 || ROWSCROLL == 1
		nColour = BURN_ENDIAN_SWAP_INT32(pTileData[0]);

		if (XPOS <= (XSIZE - 8)) {
			if (XPOS < 0) {
				if (XPOS < -3) {
					nColour = BURN_ENDIAN_SWAP_INT32(pTileData[1]);
				}
				nColour >>= (-XPOS & 3) * 8;
				pPixel += -XPOS * (BPP >> 3);
			}

			switch (XPOS < 0 ? -XPOS : 0) {
				case 0:
					PLOTPIXEL(nColour & 0xFF);
					ADVANCECOLUMN;
					nColour >>= 8;
				case 1:
					PLOTPIXEL(nColour & 0xFF);
					ADVANCECOLUMN;
					nColour >>= 8;
				case 2:
					PLOTPIXEL(nColour & 0xFF);
					ADVANCECOLUMN;
					nColour >>= 8;
				case 3:
					PLOTPIXEL(nColour & 0xFF);
					ADVANCECOLUMN;
					nColour = BURN_ENDIAN_SWAP_INT32(pTileData[1]);
				case 4:
					PLOTPIXEL(nColour & 0xFF);
					ADVANCECOLUMN;
					nColour >>= 8;
				case 5:
					PLOTPIXEL(nColour & 0xFF);
					ADVANCECOLUMN;
					nColour >>= 8;
				case 6:
					PLOTPIXEL(nColour & 0xFF);
					ADVANCECOLUMN;
					nColour >>= 8;
				case 7:
					PLOTPIXEL(nColour & 0xFF);
			}
		} else {

   #define CLIPPIXEL(a,b) if (XPOS < XSIZE - (a)) { b; }
			CLIPPIXEL(0, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(1, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(2, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(3, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour = BURN_ENDIAN_SWAP_INT32(pTileData[1]);
			CLIPPIXEL(4, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(5, PLOTPIXEL(nColour & 0xFF));
			ADVANCECOLUMN;
			nColour >>= 8;
			CLIPPIXEL(6, PLOTPIXEL(nColour & 0xFF));
   #undef CLIPPIXEL

		}

		pTileData += 2;

  #else
		nColour = BURN_ENDIAN_SWAP_INT32(*pTileData++);
		PLOTPIXEL(nColour & 0xFF);
		ADVANCECOLUMN;
		nColour >>= 8;
		PLOTPIXEL(nColour & 0xFF);
		ADVANCECOLUMN;

		nColour >>= 8;
		PLOTPIXEL(nColour & 0xFF);
		ADVANCECOLUMN;
		nColour >>= 8;
		PLOTPIXEL(nColour & 0xFF);
		ADVANCECOLUMN;

		nColour = BURN_ENDIAN_SWAP_INT32(*pTileData++);
		PLOTPIXEL(nColour & 0xFF);
		ADVANCECOLUMN;
		nColour >>= 8;
		PLOTPIXEL(nColour & 0xFF);
		ADVANCECOLUMN;

		nColour >>= 8;
		PLOTPIXEL(nColour & 0xFF);
		ADVANCECOLUMN;
		nColour >>= 8;
		PLOTPIXEL(nColour & 0xFF);
  #endif
 #endif

 #undef XPOS

 #if ROWSELECT == 0
	}
 #endif

#endif
}

#undef FLIP
#undef CLIP
#undef PLOTPIXEL
#undef TESTCLIP
#undef TESTCOLOUR
#undef ADVANCEROW
#undef ADVANCECOLUMN
#undef SCROLL
#undef SELECT
#undef DEPTH
#undef FUNCTIONNAME
#undef FN

