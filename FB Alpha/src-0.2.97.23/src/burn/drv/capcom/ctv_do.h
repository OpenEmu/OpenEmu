// CPS Tiles (header)

// Draw a nxn tile
// pCtvLine, pTile, nTileAdd are defined
// CU_FLIPX is 1 to flip the tile horizontally
// CU_CARE  is 1 to clip output based on nCtvRollX/Y
// CU_ROWS  is 1 to shift output based on CpstRowShift
// CU_SIZE  is 8, 16 or 32
// CU_BPP is 1 2 3 4 bytes per pixel
// CU_MASK CPS1 BgHi CPS2 Sprite Masking

#ifndef CU_FLIPX
 #error "CU_FLIPX wasn\'t defined"
#endif

#ifndef CU_CARE
 #error "CU_CARE wasn\'t defined"
#endif

#ifndef CU_ROWS
 #error "CU_ROWS wasn\'t defined"
#endif

#ifndef CU_SIZE
 #error "CU_SIZE wasn\'t defined"
#endif

#ifndef CU_BPP
 #error "CU_BPP wasn\'t defined"
#endif

#ifndef CU_MASK
 #error "CU_MASK wasn\'t defined"
#endif

{
  INT32 y;
  UINT32 *ctp;
  UINT32 nBlank = 0;

  UINT32 b;      			// Eight bit-packed pixels (msb) AAAABBBB CCCCDDDD EEEEFFFF GGGGHHHH (lsb)
  UINT32 c;				// 32-bit colour value
  UINT8 *pPix;			// Pointer to output bitmap

#if CU_ROWS == 1
  INT16 *Rows = CpstRowShift;
#endif

  ctp = CpstPal;

for (y = 0; y < CU_SIZE; y++, pCtvLine += nBurnPitch, pCtvTile += nCtvTileAdd

#if CU_ROWS==1
     ,Rows++
#endif

#if CU_MASK==1
	 ,pZVal += 384
#endif

)
{
#if CU_CARE==1
  UINT32 rx = nCtvRollX;	// Copy of nCtvRollX
#endif

#if CU_MASK==1
  UINT16 *pPixZ;
#endif

#if CU_CARE==1
  if (nCtvRollY & 0x20004000) { nCtvRollY += 0x7fff; continue; } else nCtvRollY += 0x7fff;	// okay to plot line
#endif

  // Point to the line to draw
  pPix = pCtvLine;
#if CU_MASK==1
  pPixZ = pZVal;
#endif

#if CU_ROWS==1
 #if CU_MASK==1
  pPixZ += Rows[0];
 #endif
  pPix += Rows[0] * nBurnBpp;
 #if CU_CARE==1
  rx += Rows[0] * 0x7fff;
 #endif
#endif

// Make macros for plotting c and advancing pPix by one pixel
#if CU_BPP==2
 #if   CU_MASK==1
  #define PLOT { if(*pPixZ < ZValue) { *((UINT16 *)pPix)=(UINT16)c; *pPixZ=ZValue; } }
  #define ADV { pPix+=2; pPixZ++; }
 #else
  #define PLOT { *((UINT16 *)pPix)=(UINT16)c; }
  #define ADV pPix+=2
 #endif
#elif CU_BPP==3
 #if   CU_MASK==1
  #define PLOT { if(*pPixZ < ZValue) { pPix[0]=(UINT8)c; pPix[1]=(UINT8)(c>>8); pPix[2]=(UINT8)(c>>16); } }
  #define ADV { pPix+=3; pPixZ++; }
 #else
  #define PLOT { pPix[0]=(UINT8)c; pPix[1]=(UINT8)(c>>8); pPix[2]=(UINT8)(c>>16); }
  #define ADV pPix+=3
 #endif
#elif CU_BPP==4
 #if   CU_MASK==1
  #define PLOT { if(*pPixZ < ZValue) { *((UINT32 *)pPix)=c; *pPixZ=ZValue; } }
  #define ADV { pPix+=4; pPixZ++; }
 #else
  #define PLOT { *((UINT32 *)pPix)=c; }
  #define ADV pPix+=4
 #endif
#else
 #error Unsupported CU_BPP
#endif

// Make macros for plotting the next pixel from 'b' (= 8 packed pixels)
// or skipping the pixel.
#if CU_FLIPX==0
 #define NEXTPIXEL ADV; b <<= 4;
 #if CU_MASK==2
  #define DRAWPIXEL { c = (b >> 28); if (c && CpstPmsk & (1 << (c ^ 15))) { c = ctp[c]; PLOT } }
 #else
  #define DRAWPIXEL { if (b & 0xf0000000) { c = ctp[b >> 28]; PLOT } }
 #endif
#else
 #define NEXTPIXEL ADV; b >>= 4;
 #if CU_MASK==2
  #define DRAWPIXEL { c = (b & 15); if (c && CpstPmsk & (1 << (c ^ 15))) { c = ctp[c]; PLOT } }
 #else
  #define DRAWPIXEL { if (b & 0x0000000f) { c = ctp[b & 15]; PLOT } }
 #endif
#endif

#define EIGHT(x) x x x x x x x x

#if CU_CARE==1
 // If we need to clip left or right, check nCtvRollX before plotting
 #define DO_PIX if ((rx & 0x20004000) == 0) DRAWPIXEL NEXTPIXEL rx += 0x7fff;
#else
 // Always plot
 #define DO_PIX DRAWPIXEL NEXTPIXEL
#endif

#define DRAW_8 nBlank |= b; EIGHT(DO_PIX)

#if   CU_SIZE==8
  // 8x8 tiles
  b=*((UINT32 *)(pCtvTile+0)); DRAW_8
#elif CU_SIZE==16
 // 16x16 tiles
 #if CU_FLIPX==0
  b=*((UINT32 *)(pCtvTile+0)); DRAW_8
  b=*((UINT32 *)(pCtvTile+4)); DRAW_8
 #else
  b=*((UINT32 *)(pCtvTile+4)); DRAW_8
  b=*((UINT32 *)(pCtvTile+0)); DRAW_8
 #endif

#elif CU_SIZE==32
 // 32x32 tiles
 #if CU_FLIPX==0
  b=*((UINT32 *)(pCtvTile+ 0)); DRAW_8
  b=*((UINT32 *)(pCtvTile+ 4)); DRAW_8
  b=*((UINT32 *)(pCtvTile+ 8)); DRAW_8
  b=*((UINT32 *)(pCtvTile+12)); DRAW_8
 #else
  b=*((UINT32 *)(pCtvTile+12)); DRAW_8
  b=*((UINT32 *)(pCtvTile+ 8)); DRAW_8
  b=*((UINT32 *)(pCtvTile+ 4)); DRAW_8
  b=*((UINT32 *)(pCtvTile+ 0)); DRAW_8
 #endif

#else
 #error Unsupported CU_SIZE
#endif

#undef DRAW_8
#undef DO_PIX
#undef EIGHT
#undef DRAWPIXEL
#undef NEXTPIXEL

#undef ADV
#undef PLOT
}

  return nBlank == 0;
}
