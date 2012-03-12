extern UINT8 *pPsikyoshTiles;
extern UINT32  *pPsikyoshSpriteBuffer;
extern UINT32  *pPsikyoshBgRAM;
extern UINT32  *pPsikyoshVidRegs;
extern UINT32  *pPsikyoshPalRAM;
extern UINT32  *pPsikyoshZoomRAM;

void PsikyoshVideoInit(INT32 gfx_max, INT32 gfx_min);
void PsikyoshVideoExit();
INT32  PsikyoshDraw();


//--------------------------------------------------------------------------------
// Macro hell. 

// Standard tile drawing with full clipping.
//-----------------------------------------------------------------------
#define PUTPIXEL_CLIP(forloop, splitpixel, putpixel)				\
	UINT32 *dest;							\
	for (INT32 y = 0; y < 16; y++, sy++, src+=inc) {				\
		if (sy < 0 || sy >= nScreenHeight) continue;			\
										\
		dest = DrvTmpDraw + sy * nScreenWidth;				\
										\
forloop										\
		{								\
			if (sx < 0 || sx >= nScreenWidth) continue;		\
										\
splitpixel									\
										\
			if (c) {						\
putpixel									\
			}							\
		}								\
										\
		sx -= 16;							\
	}

// Tile drawing with clipping and priority
//-----------------------------------------------------------------------
#define PUTPIXEL_PRIO_CLIP(forloop, splitpixel, putpixel)			\
	UINT32 *dest;							\
	UINT16 *pri;							\
	for (INT32 y = 0; y < 16; y++, sy++, src+=inc) {				\
		if (sy < 0 || sy >= nScreenHeight) continue;			\
										\
		dest = DrvTmpDraw + sy * nScreenWidth;				\
		pri  = DrvPriBmp + sy * nScreenWidth;				\
										\
		forloop {							\
			if (sx < 0 || sx >= nScreenWidth) continue;		\
										\
			if (z >= pri[sx]) {					\
splitpixel									\
										\
				if (c) {					\
putpixel									\
					pri[sx] = z;				\
				}						\
			}							\
		}								\
										\
		sx -= 16;							\
	}

// Standard tile drawing with no clipping.
//-----------------------------------------------------------------------
#define PUTPIXEL(forloop, splitpixel, putpixel)					\
	UINT32 *dest = DrvTmpDraw + sy * nScreenWidth;			\
	for (INT32 y = 0; y < 16; y++, sy++, src+=inc) {				\
forloop										\
		{								\
										\
splitpixel									\
										\
			if (c) {						\
putpixel									\
			}							\
		}								\
										\
		dest += nScreenWidth;						\
		sx -= 16;							\
	}

// Tile drawing with no clipping and with priorities.
//-----------------------------------------------------------------------
#define PUTPIXEL_PRIO(forloop, splitpixel, putpixel)				\
	UINT32 *dest = DrvTmpDraw + sy * nScreenWidth;			\
	UINT16 *pri = DrvPriBmp + sy * nScreenWidth;			\
	for (INT32 y = 0; y < 16; y++, sy++, src+=inc) {				\
		forloop {							\
			if (z >= pri[sx]) {					\
splitpixel									\
										\
				if (c) {					\
putpixel									\
					pri[sx] = z;				\
				}						\
			}							\
		}								\
		dest += nScreenWidth;						\
		pri += nScreenWidth;						\
		sx -= 16;							\
	}

// Zoom drawing with priorities.
//-----------------------------------------------------------------------
#define ZOOMPIXEL_PRIO(putpixel)						\
	for (INT32 y = sy; y < ey; y++)						\
	{									\
		UINT8 *source = DrvZoomBmp + (y_index >> 10) * 256;	\
		UINT32  *dest = DrvTmpDraw + y * nScreenWidth;		\
		UINT16 *pri = DrvPriBmp + y * nScreenWidth;		\
										\
		INT32 x_index = x_index_base;					\
		for (INT32 x = sx; x < ex; x++)					\
		{								\
			if (z >= pri[x])					\
			{							\
				INT32 c = source[x_index>>10];			\
				if (c)						\
				{						\
putpixel									\
					pri[x] = z;				\
				}						\
			}							\
			x_index += dx;						\
		}								\
		y_index += dy;							\
	}

// Standard Zoom drawing
//-----------------------------------------------------------------------
#define ZOOMPIXEL_NORMAL(putpixel)	\
	for (INT32  y = sy; y < ey; y++)						\
	{									\
		UINT8 *source = DrvZoomBmp + (y_index >> 10) * 256;	\
		UINT32  *dest = DrvTmpDraw + y * nScreenWidth;		\
		INT32 x_index = x_index_base;					\
										\
		for (INT32 x = sx; x < ex; x++)					\
		{								\
			INT32 c = source[x_index>>10];				\
			if(c) {							\
putpixel									\
			}							\
			x_index += dx;						\
		}								\
		y_index += dy;							\
	}

//--------------------------------------------------------------------------------

// split up a 4bpp pixel
#define SPLITPIXEL							\
	INT32 c = src[(x >> 1)];						\
	if (x & 1) {							\
		c &= 0xf;						\
	} else {							\
		c >>= 4;						\
	}

// use 8bpp pixel
#define NORMALPIXEL	INT32 c = src[x];	

#define SETNORMALPIXEL	dest[sx] = pal[c];

#define SETBLENDPIXEL	dest[sx] = alpha_blend(dest[sx], pal[c], alpha);

#define SETVARIABLEPIXEL							\
	if (alphatable[c] == 0xff) {						\
		dest[sx] = pal[c];						\
	} else {								\
		dest[sx] = alpha_blend(dest[sx], pal[c], alphatable[c]);	\
	}

#define FORLOOP_NORMAL	for (INT32 x = 0; x < 16; x++, sx++)

#define FORLOOP_FLIPX	for (INT32 x = 15; x >= 0; x--, sx++)

// set pixels for zoom routines
#define ZSETNORMALPIXEL	dest[x] = pal[c];

#define ZSETBLENDPIXEL	dest[x] = alpha_blend(dest[x], pal[c], alpha);

#define ZSETVARIABLEPIXEL								\
	if (alphatable[c] == 0xff) {							\
		dest[x] = pal[c];						\
	} else {									\
		dest[x] = alpha_blend(dest[x], pal[c], alphatable[c]);			\
	}

//--------------------------------------------------------------------------------

// these aren't really necessary, they just help me keep track of what things do...

#define PUTPIXEL_4BPP_NORMAL_CLIP()		PUTPIXEL_CLIP(FORLOOP_NORMAL, SPLITPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_4BPP_ALPHA_CLIP()		PUTPIXEL_CLIP(FORLOOP_NORMAL, SPLITPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_4BPP_ALPHATAB_CLIP()		PUTPIXEL_CLIP(FORLOOP_NORMAL, SPLITPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_4BPP_NORMAL_PRIO_CLIP()	PUTPIXEL_PRIO_CLIP(FORLOOP_NORMAL, SPLITPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_4BPP_ALPHA_PRIO_CLIP()		PUTPIXEL_PRIO_CLIP(FORLOOP_NORMAL, SPLITPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_4BPP_ALPHATAB_PRIO_CLIP()	PUTPIXEL_PRIO_CLIP(FORLOOP_NORMAL, SPLITPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_8BPP_NORMAL_CLIP()		PUTPIXEL_CLIP(FORLOOP_NORMAL, NORMALPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_8BPP_ALPHA_CLIP()		PUTPIXEL_CLIP(FORLOOP_NORMAL, NORMALPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_8BPP_ALPHATAB_CLIP()		PUTPIXEL_CLIP(FORLOOP_NORMAL, NORMALPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_8BPP_NORMAL_PRIO_CLIP()	PUTPIXEL_PRIO_CLIP(FORLOOP_NORMAL, NORMALPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_8BPP_ALPHA_PRIO_CLIP()		PUTPIXEL_PRIO_CLIP(FORLOOP_NORMAL, NORMALPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_8BPP_ALPHATAB_PRIO_CLIP()	PUTPIXEL_PRIO_CLIP(FORLOOP_NORMAL, NORMALPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_4BPP_NORMAL_FLIPX_CLIP()	PUTPIXEL_CLIP(FORLOOP_FLIPX, SPLITPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_4BPP_ALPHA_FLIPX_CLIP()	PUTPIXEL_CLIP(FORLOOP_FLIPX, SPLITPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_4BPP_ALPHATAB_FLIPX_CLIP()	PUTPIXEL_CLIP(FORLOOP_FLIPX, SPLITPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_4BPP_NORMAL_PRIO_FLIPX_CLIP()	PUTPIXEL_PRIO_CLIP(FORLOOP_FLIPX, SPLITPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_4BPP_ALPHA_PRIO_FLIPX_CLIP()	PUTPIXEL_PRIO_CLIP(FORLOOP_FLIPX, SPLITPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_4BPP_ALPHATAB_PRIO_FLIPX_CLIP()PUTPIXEL_PRIO_CLIP(FORLOOP_FLIPX, SPLITPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_8BPP_NORMAL_FLIPX_CLIP()	PUTPIXEL_CLIP(FORLOOP_FLIPX, NORMALPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_8BPP_ALPHA_FLIPX_CLIP()	PUTPIXEL_CLIP(FORLOOP_FLIPX, NORMALPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_8BPP_ALPHATAB_FLIPX_CLIP()	PUTPIXEL_CLIP(FORLOOP_FLIPX, NORMALPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_8BPP_NORMAL_PRIO_FLIPX_CLIP()	PUTPIXEL_PRIO_CLIP(FORLOOP_FLIPX, NORMALPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_8BPP_ALPHA_PRIO_FLIPX_CLIP()	PUTPIXEL_PRIO_CLIP(FORLOOP_FLIPX, NORMALPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_8BPP_ALPHATAB_PRIO_FLIPX_CLIP()PUTPIXEL_PRIO_CLIP(FORLOOP_FLIPX, NORMALPIXEL, SETVARIABLEPIXEL)


#define PUTPIXEL_4BPP_NORMAL()			PUTPIXEL(FORLOOP_NORMAL, SPLITPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_4BPP_ALPHA()			PUTPIXEL(FORLOOP_NORMAL, SPLITPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_4BPP_ALPHATAB()		PUTPIXEL(FORLOOP_NORMAL, SPLITPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_4BPP_NORMAL_PRIO()		PUTPIXEL_PRIO(FORLOOP_NORMAL, SPLITPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_4BPP_ALPHA_PRIO()		PUTPIXEL_PRIO(FORLOOP_NORMAL, SPLITPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_4BPP_ALPHATAB_PRIO()		PUTPIXEL_PRIO(FORLOOP_NORMAL, SPLITPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_8BPP_NORMAL()			PUTPIXEL(FORLOOP_NORMAL, NORMALPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_8BPP_ALPHA()			PUTPIXEL(FORLOOP_NORMAL, NORMALPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_8BPP_ALPHATAB()		PUTPIXEL(FORLOOP_NORMAL, NORMALPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_8BPP_NORMAL_PRIO()		PUTPIXEL_PRIO(FORLOOP_NORMAL, NORMALPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_8BPP_ALPHA_PRIO()		PUTPIXEL_PRIO(FORLOOP_NORMAL, NORMALPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_8BPP_ALPHATAB_PRIO()		PUTPIXEL_PRIO(FORLOOP_NORMAL, NORMALPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_4BPP_NORMAL_FLIPX()		PUTPIXEL(FORLOOP_FLIPX, SPLITPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_4BPP_ALPHA_FLIPX()		PUTPIXEL(FORLOOP_FLIPX, SPLITPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_4BPP_ALPHATAB_FLIPX()		PUTPIXEL(FORLOOP_FLIPX, SPLITPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_4BPP_NORMAL_PRIO_FLIPX()	PUTPIXEL_PRIO(FORLOOP_FLIPX, SPLITPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_4BPP_ALPHA_PRIO_FLIPX()	PUTPIXEL_PRIO(FORLOOP_FLIPX, SPLITPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_4BPP_ALPHATAB_PRIO_FLIPX()	PUTPIXEL_PRIO(FORLOOP_FLIPX, SPLITPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_8BPP_NORMAL_FLIPX()		PUTPIXEL(FORLOOP_FLIPX, NORMALPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_8BPP_ALPHA_FLIPX()		PUTPIXEL(FORLOOP_FLIPX, NORMALPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_8BPP_ALPHATAB_FLIPX()		PUTPIXEL(FORLOOP_FLIPX, NORMALPIXEL, SETVARIABLEPIXEL)

#define PUTPIXEL_8BPP_NORMAL_PRIO_FLIPX()	PUTPIXEL_PRIO(FORLOOP_FLIPX, NORMALPIXEL, SETNORMALPIXEL)
#define PUTPIXEL_8BPP_ALPHA_PRIO_FLIPX()	PUTPIXEL_PRIO(FORLOOP_FLIPX, NORMALPIXEL, SETBLENDPIXEL)
#define PUTPIXEL_8BPP_ALPHATAB_PRIO_FLIPX()PUTPIXEL_PRIO(FORLOOP_FLIPX, NORMALPIXEL, SETVARIABLEPIXEL)


#define PUTPIXEL_ZOOM_NORMAL_PRIO()		ZOOMPIXEL_PRIO(ZSETNORMALPIXEL)
#define PUTPIXEL_ZOOM_NORMAL()			ZOOMPIXEL_NORMAL(ZSETNORMALPIXEL)

#define PUTPIXEL_ZOOM_ALPHA_PRIO()		ZOOMPIXEL_PRIO(ZSETBLENDPIXEL)
#define PUTPIXEL_ZOOM_ALPHA()			ZOOMPIXEL_NORMAL(ZSETBLENDPIXEL)

#define PUTPIXEL_ZOOM_ALPHATAB_PRIO()		ZOOMPIXEL_PRIO(ZSETVARIABLEPIXEL)
#define PUTPIXEL_ZOOM_ALPHATAB()		ZOOMPIXEL_NORMAL(ZSETVARIABLEPIXEL)
