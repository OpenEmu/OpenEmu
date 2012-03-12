// epxb, epxc filter, added by regret
// source from Snes9X rerecording (http://code.google.com/p/snes9x151-rerecording/)

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef unsigned int uint32;
# ifdef __GNUC__  /* long long is not part of ISO C++ */
__extension__
# endif
typedef long long int64;
typedef unsigned long long uint64;

#define Mask_2	0x07E0	// 00000 111111 00000
#define Mask13	0xF81F	// 11111 000000 11111
#define Mask_1	0x001F	// 00000 000000 11111
#define Mask_3	0xF800	// 11111 000000 00000
#define CONVERT_16_TO_32(pixel) \
        (((((pixel) >> 11)        ) << /*RedShift+3*/  19) | \
         ((((pixel) >> 6)   & 0x1f) << /*GreenShift+3*/11) | \
          (((pixel)         & 0x1f) << /*BlueShift+3*/ 3))

#define Interp01(c1, c2) \
	((((c1) == (c2)) ? (c1) : \
	(((((((c1) & Mask_2) *  3) + ((c2) & Mask_2)) >> 2) & Mask_2) + \
	 ((((((c1) & Mask13) *  3) + ((c2) & Mask13)) >> 2) & Mask13))))

#ifdef LSB_FIRST
	#define TWO_PIX(left,right) ((left) | ((right) << 16))
	#define THREE_PIX(left,middle,right) uint48((left) | ((middle) << 16), (right))
	#define TWO_PIX_32(left,right) (CONVERT_16_TO_32(left) | ((uint64)CONVERT_16_TO_32(right) << 32))
	#define THREE_PIX_32(left,middle,right) uint96(CONVERT_16_TO_32(left) | ((uint64)CONVERT_16_TO_32(middle) << 32), CONVERT_16_TO_32(right))
#else
	#define TWO_PIX(left,right) ((right) | ((left) << 16))
	#define THREE_PIX(left,middle,right) uint48((middle) | ((left) << 16), (right)) // is this right?
//	#define THREE_PIX(left,middle,right) uint48((right) | ((middle) << 16), (left)) // or this?
#endif

// shared EPX-type loop macros
// All of these parameters will be constant values,
// so I hope the compiler is smart enough to optimize away "if(0) ..."

#define DrawRows(scale,diags)            \
	{                                    \
		h = srcHeight - 1;               \
		DoRow(0,1,scale,diags);          \
		for (h = srcHeight - 2; h; h--)  \
			DoRow(1,1,scale,diags);      \
		DoRow(1,0,scale,diags);          \
	}

#define DoRow(topValid,botValid,scale,diags)              \
	{                                                     \
		w = srcWidth - 1;                                 \
		InitLine(topValid,botValid,scale,diags);          \
		DrawPix(0,topValid,0,botValid);                   \
		for (w = srcWidth - 2; w; w--)                    \
		{                                                 \
			NextPixel(topValid,botValid,1,diags);         \
			DrawPix(topValid,topValid,botValid,botValid); \
		}                                                 \
		NextPixel(topValid,botValid,0,diags);             \
		DrawPix(topValid,0,botValid,0);                   \
		srcPtr += srcPitch;                               \
		dstPtr += dstPitch * scale;                       \
	}

#define InitLine(topValid, botValid, scale, diags)                \
	{                                                             \
		if(topValid) uP  = (uint16 *) (srcPtr - srcPitch);        \
		if(botValid) lP  = (uint16 *) (srcPtr + srcPitch);        \
					 sP  = (uint16 *) srcPtr;                     \
					 dP1 = (destType *) dstPtr;                   \
					 dP2 = (destType *) (dstPtr + dstPitch);      \
		if(scale>=3) dP3 = (destType *) (dstPtr + (dstPitch<<1)); \
		if(topValid) colorD = *uP++;                              \
		if(botValid) colorB = *lP++;                              \
					colorX = *sP++;                               \
					colorC = *sP;                                 \
		if(diags) if(topValid) colorH = *uP;                      \
		if(diags) if(botValid) colorG = *lP;                      \
	}

#define NextPixel(topValid, botValid, rightValid, diags)      \
	{                                                         \
		colorA = colorX;                                      \
		colorX = colorC;                                      \
		if(rightValid) colorC = *++sP;                        \
		if(diags) {                                           \
			if(botValid){                                     \
				colorF = colorB;                              \
				colorB = colorG;                              \
				if(rightValid) colorG = *++lP;                \
			}                                                 \
			if(topValid){                                     \
				colorE = colorD;                              \
				colorD = colorH;                              \
				if(rightValid) colorH = *++uP;                \
			}                                                 \
		} else {                                              \
			if(botValid) colorB = *lP++;                      \
			if(topValid) colorD = *uP++;                      \
		}                                                     \
	}

#define DrawInit(scale,uintDest)                                                            \
	uint8 *srcPtr = src, *dstPtr = dst;                                     \
	const uint32 srcPitch = srcpitch, dstPitch = dstpitch;                                \
	const uint32 srcHeight = nHeight;                              \
	const uint32 srcWidth = nWidth;                               \
	uint16	colorX, colorA, colorB, colorC, colorD, colorE=0, colorF=0, colorG=0, colorH=0; \
	uint16	*sP, *uP, *lP;                                                                  \
	typedef uintDest destType;                                                              \
	destType *dP1, *dP2, *dP3=0;                                                            \
	int		w, h;

// code for improved 2X EPX, which tends to do better with diagonal edges than regular EPX
void RenderEPXB(unsigned char *src, unsigned int srcpitch, unsigned char *dst, unsigned int dstpitch, int nWidth, int nHeight, int vidDepth)
{
	// E D H
	// A X C
	// F B G
	#define DrawPix(on00,on01,on10,on11) /* on00 on01 */                                                                                                       \
	{                                    /* on10 on11 */                                                                                                       \
		if ((((on00||on10)?colorA:colorX) != ((on01||on11)?colorC:colorX))                                                                                     \
		&& (((on10||on11)?colorB:colorX) != ((on00||on01)?colorD:colorX))                                                                                      \
		&& ((!on00||!on01||!on10|!on11) ||                                                                                                                     \
			((colorX == colorA) || (colorX == colorB) || (colorX == colorC) || (colorX == colorD) || /* diagonal    */                                         \
			 (((colorE != colorG) || (colorX == colorF) || (colorX == colorH)) &&                    /*  edge       */                                         \
			  ((colorF != colorH) || (colorX == colorE) || (colorX == colorG))))))                   /*   smoothing */                                         \
		{                                                                                                                                                      \
			*dP1++ = _TWO_PIX((on00 && colorD == colorA && (colorX != colorE || colorX != colorG || colorD != colorH || colorA != colorF)) ? colorD : colorX,  \
							  (on01 && colorC == colorD && (colorX != colorH || colorX != colorF || colorC != colorG || colorD != colorE)) ? colorC : colorX); \
			*dP2++ = _TWO_PIX((on10 && colorA == colorB && (colorX != colorF || colorX != colorH || colorA != colorE || colorB != colorG)) ? colorA : colorX,  \
							  (on11 && colorB == colorC && (colorX != colorG || colorX != colorE || colorB != colorF || colorC != colorH)) ? colorB : colorX); \
		} else {                                                                                                                                               \
			*dP1++ = _TWO_PIX(colorX, colorX);                                                                                                                 \
			*dP2++ = _TWO_PIX(colorX, colorX);                                                                                                                 \
		}                                                                                                                                                      \
	}

	// again, this supports 32-bit or 16-bit rendering
	if(vidDepth == 32)
	{
#define _TWO_PIX TWO_PIX_32
		DrawInit(2,uint64);
		DrawRows(2,1); // 2x scale, and diags is on since we do use all 8 surrounding pixels
#undef _TWO_PIX
	}
	else
	{
#define _TWO_PIX TWO_PIX
		DrawInit(2,uint32);
		DrawRows(2,1); // 2x scale, and diags is on since we do use all 8 surrounding pixels
#undef _TWO_PIX
	}

	#undef DrawPix
}

#define Interp44(c1, c2, c3, c4) \
	((((((c1) & Mask_2) * 5 + ((c2) & Mask_2) + ((c3) & Mask_2) + ((c4) & Mask_2)) >> 3) & Mask_2) + \
	 (((((c1) & Mask13) * 5 + ((c2) & Mask13) + ((c3) & Mask13) + ((c4) & Mask13)) >> 3) & Mask13))

// EPX3 scaled down to 2X
void RenderEPXC(unsigned char *src, unsigned int srcpitch, unsigned char *dst, unsigned int dstpitch, int nWidth, int nHeight, int vidDepth)
{
	// E D H
	// A X C
	// F B G
	#define DrawPix(on00,on01,on10,on11) /* on00 on01 */ \
	{                                    /* on10 on11 */ \
		if ((((on00||on10)?colorA:colorX) != ((on01||on11)?colorC:colorX)) \
		 && (((on10||on11)?colorB:colorX) != ((on00||on01)?colorD:colorX))) \
		{ \
			const bool XnE = colorX != colorE; \
			const bool XnF = colorX != colorF; \
			const bool XnG = colorX != colorG; \
			const bool XnH = colorX != colorH; \
			const bool DA = on00 && colorD == colorA && (XnE || XnG || colorD != colorH || colorA != colorF); \
			const bool AB = on10 && colorA == colorB && (XnF || XnH || colorA != colorE || colorB != colorG); \
			const bool BC = on11 && colorB == colorC && (XnG || XnE || colorB != colorF || colorC != colorH); \
			const bool CD = on01 && colorC == colorD && (XnH || XnF || colorC != colorG || colorD != colorE); \
			if (!on00||!on01||!on10||!on11 || ((colorA != colorC) && (colorB != colorD) && \
				((colorX == colorA) || (colorX==colorB) || (colorX==colorC) || (colorX==colorD) || (colorX==colorE) || (colorX==colorF) || (colorX==colorG) || (colorX==colorH)))) \
			{ \
				const uint16 colorAA = on00&&on10 && ((DA && XnF) || (AB && XnE)) ? colorA : colorX; \
				const uint16 colorBB = on10&&on11 && ((AB && XnG) || (BC && XnF)) ? colorB : colorX; \
				const uint16 colorCC = on01&&on11 && ((BC && XnH) || (CD && XnG)) ? colorC : colorX; \
				const uint16 colorDD = on00&&on01 && ((CD && XnE) || (DA && XnH)) ? colorD : colorX; \
				*dP1++ = _TWO_PIX(Interp44(on00 && DA ? colorA : colorX, colorDD, colorAA, colorX), Interp44(on01 && CD ? colorC : colorX, colorBB, colorCC, colorX)); \
				*dP2++ = _TWO_PIX(Interp44(on10 && AB ? colorA : colorX, colorAA, colorBB, colorX), Interp44(on11 && BC ? colorC : colorX, colorCC, colorDD, colorX)); \
			} else { \
				*dP1++ = _TWO_PIX(Interp01(colorX, on00 && DA && (colorX!=colorB&&colorX!=colorC) ? colorA : colorX), Interp01(colorX, on01 && CD && (colorX!=colorA&&colorX!=colorB) ? colorC : colorX)); \
				*dP2++ = _TWO_PIX(Interp01(colorX, on10 && AB && (colorX!=colorC&&colorX!=colorD) ? colorA : colorX), Interp01(colorX, on11 && BC && (colorX!=colorD&&colorX!=colorA) ? colorC : colorX)); \
			} \
		} else { \
			*dP1++ = _TWO_PIX(colorX, colorX); \
			*dP2++ = _TWO_PIX(colorX, colorX); \
		} \
	}

	if(vidDepth == 32)
	{
#define _TWO_PIX TWO_PIX_32
		DrawInit(2,uint64);
		DrawRows(2,1); // 2x scale, and diags is on since we do use all 8 surrounding pixels
#undef _TWO_PIX
	}
	else
	{
#define _TWO_PIX TWO_PIX
		DrawInit(2,uint32);
		DrawRows(2,1); // 2x scale, and diags is on since we do use all 8 surrounding pixels
#undef _TWO_PIX
	}

	#undef DrawPix
}
