#include "gal.h"

// This module is not accurate to the arcade hardware - it is ported from my previous Galaxian driver for FBA,
// which was based on an old version of MAME. It is considered "good enough" for the purpose of giving the impression
// of the star layers without being totally accurate

struct Star
{
	INT32 x, y, Colour;
};

static struct Star Stars[252];

INT32 GalStarsEnable     = 0;
INT32 GalStarsScrollPos  = 0;
INT32 GalStarsBlinkState = 0;
INT32 GalBlinkTimerStartFrame = 0;
static double GalBlinkEveryFrames = (0.693 * (100000 + 2.0 + 10000) * 0.00001) * (16000.0 / 132 / 2);

void GalInitStars()
{
	INT32 nStars, Generator, x, y;

	GalStarsEnable     = 0;
	GalStarsScrollPos  = -1;
	GalStarsBlinkState = 0;

	nStars = 0;
	Generator = 0;

	for (y = 255; y >= 0; y--) {
		for (x = 511; x >= 0; x--) {
			INT32 Bit0;

			Bit0 = ((~Generator >> 16) & 0x01) ^ ((Generator >> 4) & 0x01);

			Generator = (Generator << 1) | Bit0;

			if (((~Generator >> 16) & 0x01) && (Generator & 0xff) == 0xff) {
				INT32 Colour;

				Colour = (~(Generator >> 8)) & 0x3f;

				if (Colour) {
					Stars[nStars].x = x;
					Stars[nStars].y = y;
					Stars[nStars].Colour = Colour;

					nStars++;
				}
			}
		}
	}
}

static INT32 GalCheckStarsBlinkState()
{
	INT32 CurrentFrame = GetCurrentFrame();

	if ((CurrentFrame - GalBlinkTimerStartFrame) >= (INT32)GalBlinkEveryFrames) {
		GalBlinkTimerStartFrame = CurrentFrame;
		return 1;
	}

	return 0;
}

static inline void GalPlotStar(INT32 x, INT32 y, INT32 Colour)
{
	if (y >= 0 && y < nScreenHeight && x >= 0 && x < nScreenWidth) {
		pTransDraw[(y * nScreenWidth) + x] = Colour + GAL_PALETTE_STARS_OFFSET;
	}
}

void GalaxianRenderStarLayer()
{
	GalStarsScrollPos++;
	
	for (INT32 Offs = 0; Offs < 252; Offs++) {
		INT32 x, y;
		
		x = ((Stars[Offs].x + GalStarsScrollPos) & 0x01ff) >>1;
		y = (Stars[Offs].y + ((GalStarsScrollPos + Stars[Offs].x) >> 9)) & 0xff;
		
		if ((y & 0x01) ^ ((x >> 3) & 0x01)) {
			if (GalFlipScreenX) x = 255 - x;
			if (GalFlipScreenY) y = 255 - y;
			y -= 16;
			GalPlotStar(x, y, Stars[Offs].Colour);
		}
	}
}

void JumpbugRenderStarLayer()
{
	if (GalCheckStarsBlinkState()) GalStarsBlinkState++;
	
	for (INT32 Offs = 0; Offs < 252; Offs++) {
		INT32 x, y;
		
		x = Stars[Offs].x >> 1;
		y = Stars[Offs].y >> 1;
		
		if ((y & 0x01) ^ ((x >> 3) & 0x01)) {
			switch (GalStarsBlinkState & 0x03) {
				case 0: {
					if (!(Stars[Offs].Colour & 0x01)) continue;
					break;
				}
				
				case 1: {
					if (!(Stars[Offs].Colour & 0x04)) continue;
					break;
				}
				
				case 2: {
					if (!(Stars[Offs].y & 0x02)) continue;
					break;
				}
				
				case 3: {
					break;
				}
			}
			
			x = Stars[Offs].x >> 1;
			y = Stars[Offs].y & 0xff;
			
			if (x >= 240) continue;
			
			if (GalFlipScreenX) x = 255 - x;
			if (GalFlipScreenY) y = 255 - y;
			y -= 16;
			GalPlotStar(x, y, Stars[Offs].Colour);
		}
	}
}

void ScrambleRenderStarLayer()
{
	if (GalCheckStarsBlinkState()) GalStarsBlinkState++;
	
	for (INT32 Offs = 0; Offs < 252; Offs++) {
		INT32 x, y;
		
		x = Stars[Offs].x >> 1;
		y = Stars[Offs].y;
		
		if ((y & 0x01) ^ ((x >> 3) & 0x01)) {
			switch (GalStarsBlinkState & 0x03) {
				case 0: {
					if (!(Stars[Offs].Colour & 0x01)) continue;
					break;
				}
				
				case 1: {
					if (!(Stars[Offs].Colour & 0x04)) continue;
					break;
				}
				
				case 2: {
					if (!(Stars[Offs].y & 0x02)) continue;
					break;
				}
				
				case 3: {
					break;
				}
			}
			
			if (GalFlipScreenX) x = 255 - x;
			if (GalFlipScreenY) y = 255 - y;
			y -= 16;
			GalPlotStar(x, y, Stars[Offs].Colour);
		}
	}
}

void MarinerRenderStarLayer()
{
	UINT8 *Prom = GalProm + 0x120;
	
	GalStarsScrollPos++;
	
	for (INT32 Offs = 0; Offs < 252; Offs++) {
		INT32 x, y;
		
		x = ((Stars[Offs].x + GalStarsScrollPos) & 0x01ff) >>1;
		y = (Stars[Offs].y + ((GalStarsScrollPos + Stars[Offs].x) >> 9)) & 0xff;
		
		if ((y & 0x01) ^ ((x >> 3) & 0x01)) {
			if (GalFlipScreenX) x = 255 - x;
			if (GalFlipScreenY) y = 255 - y;
			y -= 16;
			
			if (Prom[((x / 8) + 1) & 0x1f] & 0x04) {
				GalPlotStar(x, y, Stars[Offs].Colour);
			}
		}
	}
}

void RescueRenderStarLayer()
{
	if (GalCheckStarsBlinkState()) GalStarsBlinkState++;
	
	for (INT32 Offs = 0; Offs < 252; Offs++) {
		INT32 x, y;
		
		x = Stars[Offs].x >> 1;
		y = Stars[Offs].y;
		
		if (x < 128 && ((y & 0x01) ^ ((x >> 3) & 0x01))) {
			switch (GalStarsBlinkState & 0x03) {
				case 0: {
					if (!(Stars[Offs].Colour & 0x01)) continue;
					break;
				}
				
				case 1: {
					if (!(Stars[Offs].Colour & 0x04)) continue;
					break;
				}
				
				case 2: {
					if (!(Stars[Offs].y & 0x02)) continue;
					break;
				}
				
				case 3: {
					break;
				}
			}
			
			if (GalFlipScreenX) x = 255 - x;
			if (GalFlipScreenY) y = 255 - y;
			y -= 16;
			GalPlotStar(x, y, Stars[Offs].Colour);
		}
	}
}
