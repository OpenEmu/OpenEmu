#include "gal.h"

GalRenderBackground GalRenderBackgroundFunction;
GalCalcPalette GalCalcPaletteFunction;
GalDrawBullet GalDrawBulletsFunction;
GalExtendTileInfo GalExtendTileInfoFunction;
GalExtendSpriteInfo GalExtendSpriteInfoFunction;
GalRenderFrame GalRenderFrameFunction;

UINT8 GalFlipScreenX;
UINT8 GalFlipScreenY;
UINT8 *GalGfxBank;
UINT8 GalPaletteBank;
UINT8 GalSpriteClipStart;
UINT8 GalSpriteClipEnd;
UINT8 FroggerAdjust;
UINT8 GalBackgroundRed;
UINT8 GalBackgroundGreen;
UINT8 GalBackgroundBlue;
UINT8 GalBackgroundEnable;
UINT8 SfxTilemap;
UINT8 GalOrientationFlipX;
UINT8 GalColourDepth;
UINT8 DarkplntBulletColour;
UINT8 DambustrBgColour1;
UINT8 DambustrBgColour2;
UINT8 DambustrBgPriority;
UINT8 DambustrBgSplitLine;
UINT8 *RockclimTiles;
UINT16 RockclimScrollX;
UINT16 RockclimScrollY;

// Graphics decode helpers
INT32 CharPlaneOffsets[2]   = { 0, 0x4000 };
INT32 CharXOffsets[8]       = { 0, 1, 2, 3, 4, 5, 6, 7 };
INT32 CharYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
INT32 SpritePlaneOffsets[2] = { 0, 0x4000 };
INT32 SpriteXOffsets[16]    = { 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71 };
INT32 SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 128, 136, 144, 152, 160, 168, 176, 184 };

// Tile extend helpers
void UpperExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	*Code += 0x100;
}

void PiscesExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	*Code |= GalGfxBank[0] << 8;
}

void Batman2ExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	if (*Code & 0x80) *Code |= GalGfxBank[0] << 8;
}

void GmgalaxExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	*Code |= GalGfxBank[0] << 9;
}

void MooncrstExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	if (GalGfxBank[2] && (*Code & 0xc0) == 0x80) *Code = (*Code & 0x3f) | (GalGfxBank[0] << 6) | (GalGfxBank[1] << 7) | 0x0100;
}

void MoonqsrExtendTileInfo(UINT16 *Code, INT32*, INT32 Attr, INT32)
{
	*Code |= (Attr & 0x20) << 3;
}

void SkybaseExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	*Code |= GalGfxBank[2] << 8;
}

void JumpbugExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	if ((*Code & 0xc0) == 0x80 && (GalGfxBank[2] & 0x01)) *Code += 128 + ((GalGfxBank[0] & 0x01) << 6) + ((GalGfxBank[1] & 0x01) << 7) + ((~GalGfxBank[4] & 0x01) << 8);
}

void FroggerExtendTileInfo(UINT16*, INT32 *Colour, INT32, INT32)
{
	*Colour = ((*Colour >> 1) & 0x03) | ((*Colour << 2) & 0x04);
}

void MshuttleExtendTileInfo(UINT16 *Code, INT32*, INT32 Attr, INT32)
{
	*Code |= (Attr & 0x30) << 4;
}

void Fourin1ExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	*Code |= Fourin1Bank << 8;
}

void MarinerExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32 x)
{
	UINT8 *Prom = GalProm + 0x120;
	
	*Code |= (Prom[x] & 0x01) << 8;
}

void MimonkeyExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32)
{
	*Code |= (GalGfxBank[0] << 8) | (GalGfxBank[1] << 9);
}

void DambustrExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32 x)
{
	if (GalGfxBank[0] == 0) {
		*Code |= 0x300;
	} else {
		if (x == 28) {
			*Code |= 0x300;
		} else {
			*Code &= 0xff;
		}
	}
}

void Ad2083ExtendTileInfo(UINT16 *Code, INT32 *Colour, INT32 Attr, INT32)
{
	INT32 Bank = Attr & 0x30;
	*Code |= (Bank << 4);
	*Colour |= ((Attr & 0x40) >> 3);
}

void RacknrolExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32 x)
{
	UINT8 Bank = GalGfxBank[x] & 7;	
	*Code |= Bank << 8;
}

// Sprite extend helpers
void UpperExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	*Code += 0x40;
}

void PiscesExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	*Code |= GalGfxBank[0] << 6;
}

void GmgalaxExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	*Code |= (GalGfxBank[0] << 7) | 0x40;
}

void MooncrstExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	if (GalGfxBank[2] && (*Code & 0x30) == 0x20) *Code = (*Code & 0x0f) | (GalGfxBank[0] << 4) | (GalGfxBank[1] << 5) | 0x40;
}

void MoonqsrExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	*Code |= (Base[2] & 0x20) << 1;
}

void SkybaseExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	*Code |= GalGfxBank[2] << 6;
}

void RockclimExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	if (GalGfxBank[2]) *Code |= 0x40;
}

void JumpbugExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	if ((*Code & 0x30) == 0x20 && (GalGfxBank[2] & 0x01) != 0) *Code += 32 + ((GalGfxBank[0] & 0x01) << 4) + ((GalGfxBank[1] & 0x01) << 5) + ((~GalGfxBank[4] & 0x01) << 6);
}

void FroggerExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16*, UINT8 *Colour)
{
	*Colour = ((*Colour >> 1) & 0x03) | ((*Colour << 2) & 0x04);
}

void CalipsoExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8 *xFlip, UINT8 *yFlip, UINT16 *Code, UINT8*)
{
	*Code = Base[1];
	*xFlip = 0;
	*yFlip = 0;
}

void MshuttleExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	*Code |= (Base[2] & 0x30) << 2;
}

void Fourin1ExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	*Code |= Fourin1Bank << 6;
}

void DkongjrmExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8 *xFlip, UINT8*, UINT16 *Code, UINT8*)
{
	*Code = (Base[1] & 0x7f) | 0x80;
	*xFlip = 0;
}

void MimonkeyExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*)
{
	*Code |= (GalGfxBank[0] << 6) | (GalGfxBank[1] << 7);
}

void Ad2083ExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8 *xFlip, UINT8*, UINT16 *Code, UINT8*)
{
	*Code = (Base[1] & 0x7f) | ((Base[2] & 0x30) << 2);
	*xFlip = 0;
}

// Hardcode a Galaxian PROM for any games that are missing a PROM dump
void HardCodeGalaxianPROM()
{
	GalProm[0x00]= 0x00;
	GalProm[0x01]= 0x00;
	GalProm[0x02]= 0x00;
	GalProm[0x03]= 0xf6;
	GalProm[0x04]= 0x00;
	GalProm[0x05]= 0x16;
	GalProm[0x06]= 0xc0;
	GalProm[0x07]= 0x3f;
	GalProm[0x08]= 0x00;
	GalProm[0x09]= 0xd8;
	GalProm[0x0a]= 0x07;
	GalProm[0x0b]= 0x3f;
	GalProm[0x0c]= 0x00;
	GalProm[0x0d]= 0xc0;
	GalProm[0x0e]= 0xc4;
	GalProm[0x0f]= 0x07;
	GalProm[0x10]= 0x00;
	GalProm[0x11]= 0xc0;
	GalProm[0x12]= 0xa0;
	GalProm[0x13]= 0x07;
	GalProm[0x14]= 0x00;
	GalProm[0x15]= 0x00;
	GalProm[0x16]= 0x00;
	GalProm[0x17]= 0x07;
	GalProm[0x18]= 0x00;
	GalProm[0x19]= 0xf6;
	GalProm[0x1a]= 0x07;
	GalProm[0x1b]= 0xf0;
	GalProm[0x1c]= 0x00;
	GalProm[0x1d]= 0x76;
	GalProm[0x1e]= 0x07;
	GalProm[0x1f]= 0xc6;
}

void HardCodeMooncrstPROM()
{
	GalProm[0x00]= 0x00;
	GalProm[0x01]= 0x7a;
	GalProm[0x02]= 0x36;
	GalProm[0x03]= 0x07;
	GalProm[0x04]= 0x00;
	GalProm[0x05]= 0xf0;
	GalProm[0x06]= 0x38;
	GalProm[0x07]= 0x1f;
	GalProm[0x08]= 0x00;
	GalProm[0x09]= 0xc7;
	GalProm[0x0a]= 0xf0;
	GalProm[0x0b]= 0x3f;
	GalProm[0x0c]= 0x00;
	GalProm[0x0d]= 0xdb;
	GalProm[0x0e]= 0xc6;
	GalProm[0x0f]= 0x38;
	GalProm[0x10]= 0x00;
	GalProm[0x11]= 0x36;
	GalProm[0x12]= 0x07;
	GalProm[0x13]= 0xf0;
	GalProm[0x14]= 0x00;
	GalProm[0x15]= 0x33;
	GalProm[0x16]= 0x3f;
	GalProm[0x17]= 0xdb;
	GalProm[0x18]= 0x00;
	GalProm[0x19]= 0x3f;
	GalProm[0x1a]= 0x57;
	GalProm[0x1b]= 0xc6;
	GalProm[0x1c]= 0x00;
	GalProm[0x1d]= 0xc6;
	GalProm[0x1e]= 0x3f;
	GalProm[0x1f]= 0xff;
}

// Pallet generation
#define RGB_MAXIMUM			224
#define MAX_NETS			3
#define MAX_RES_PER_NET			18
#define Combine2Weights(tab,w0,w1)	((INT32)(((tab)[0]*(w0) + (tab)[1]*(w1)) + 0.5))
#define Combine3Weights(tab,w0,w1,w2)	((INT32)(((tab)[0]*(w0) + (tab)[1]*(w1) + (tab)[2]*(w2)) + 0.5))

static double ComputeResistorWeights(INT32 MinVal, INT32 MaxVal, double Scaler, INT32 Count1, const INT32 *Resistances1, double *Weights1, INT32 PullDown1, INT32 PullUp1,	INT32 Count2, const INT32 *Resistances2, double *Weights2, INT32 PullDown2, INT32 PullUp2, INT32 Count3, const INT32 *Resistances3, double *Weights3, INT32 PullDown3, INT32 PullUp3)
{
	INT32 NetworksNum;

	INT32 ResCount[MAX_NETS];
	double r[MAX_NETS][MAX_RES_PER_NET];
	double w[MAX_NETS][MAX_RES_PER_NET];
	double ws[MAX_NETS][MAX_RES_PER_NET];
	INT32 r_pd[MAX_NETS];
	INT32 r_pu[MAX_NETS];

	double MaxOut[MAX_NETS];
	double *Out[MAX_NETS];

	INT32 i, j, n;
	double Scale;
	double Max;

	NetworksNum = 0;
	for (n = 0; n < MAX_NETS; n++) {
		INT32 Count, pd, pu;
		const INT32 *Resistances;
		double *Weights;

		switch (n) {
			case 0: {
				Count = Count1;
				Resistances = Resistances1;
				Weights = Weights1;
				pd = PullDown1;
				pu = PullUp1;
				break;
			}
			
			case 1: {
				Count = Count2;
				Resistances = Resistances2;
				Weights = Weights2;
				pd = PullDown2;
				pu = PullUp2;
				break;
			}
		
			case 2:
			default: {
				Count = Count3;
				Resistances = Resistances3;
				Weights = Weights3;
				pd = PullDown3;
				pu = PullUp3;
				break;
			}
		}

		if (Count > 0) {
			ResCount[NetworksNum] = Count;
			for (i = 0; i < Count; i++) {
				r[NetworksNum][i] = 1.0 * Resistances[i];
			}
			Out[NetworksNum] = Weights;
			r_pd[NetworksNum] = pd;
			r_pu[NetworksNum] = pu;
			NetworksNum++;
		}
	}

	for (i = 0; i < NetworksNum; i++) {
		double R0, R1, Vout, Dst;

		for (n = 0; n < ResCount[i]; n++) {
			R0 = (r_pd[i] == 0) ? 1.0 / 1e12 : 1.0 / r_pd[i];
			R1 = (r_pu[i] == 0) ? 1.0 / 1e12 : 1.0 / r_pu[i];

			for (j = 0; j < ResCount[i]; j++) {
				if (j == n) {
					if (r[i][j] != 0.0) R1 += 1.0 / r[i][j];
				} else {
					if (r[i][j] != 0.0) R0 += 1.0 / r[i][j];
				}
			}

			R0 = 1.0/R0;
			R1 = 1.0/R1;
			Vout = (MaxVal - MinVal) * R0 / (R1 + R0) + MinVal;

			Dst = (Vout < MinVal) ? MinVal : (Vout > MaxVal) ? MaxVal : Vout;

			w[i][n] = Dst;
		}
	}

	j = 0;
	Max = 0.0;
	for (i = 0; i < NetworksNum; i++) {
		double Sum = 0.0;

		for (n = 0; n < ResCount[i]; n++) Sum += w[i][n];

		MaxOut[i] = Sum;
		if (Max < Sum) {
			Max = Sum;
			j = i;
		}
	}

	if (Scaler < 0.0) {
		Scale = ((double)MaxVal) / MaxOut[j];
	} else {
		Scale = Scaler;
	}

	for (i = 0; i < NetworksNum; i++) {
		for (n = 0; n < ResCount[i]; n++) {
			ws[i][n] = w[i][n] * Scale;
			(Out[i])[n] = ws[i][n];
		}
	}

	return Scale;

}

void GalaxianCalcPalette()
{
	static const INT32 RGBResistances[3] = {1000, 470, 220};
	double rWeights[3], gWeights[3], bWeights[2];
	
	ComputeResistorWeights(0, RGB_MAXIMUM, -1.0, 3, &RGBResistances[0], rWeights, 470, 0, 3, &RGBResistances[0], gWeights, 470, 0, 2, &RGBResistances[1], bWeights, 470, 0);
			
	// Colour PROM
	for (INT32 i = 0; i < 32; i++) {
		UINT8 Bit0, Bit1, Bit2, r, g, b;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],0);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],1);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],2);
		r = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],3);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],4);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],5);
		g = Combine3Weights(gWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],6);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],7);
		b = Combine2Weights(bWeights, Bit0, Bit1);

		GalPalette[i] = BurnHighCol(r, g, b, 0);
	}
	
	// Stars
	for (INT32 i = 0; i < GAL_PALETTE_NUM_COLOURS_STARS; i++) {
		INT32 Bits, r, g, b;
		INT32 Map[4] = {0x00, 0x88, 0xcc, 0xff};

		Bits = (i >> 0) & 0x03;
		r = Map[Bits];
		Bits = (i >> 2) & 0x03;
		g = Map[Bits];
		Bits = (i >> 4) & 0x03;
		b = Map[Bits];

		GalPalette[i + GAL_PALETTE_STARS_OFFSET] = BurnHighCol(r, g, b, 0);
	}

	// Bullets
	for (INT32 i = 0; i < GAL_PALETTE_NUM_COLOURS_BULLETS - 1; i++) {
		GalPalette[i + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0xff, 0);
	}
	GalPalette[GAL_PALETTE_NUM_COLOURS_BULLETS - 1 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0x00, 0);
}

void RockclimCalcPalette()
{
	static const INT32 RGBResistances[3] = {1000, 470, 220};
	double rWeights[3], gWeights[3], bWeights[2];
	
	ComputeResistorWeights(0, RGB_MAXIMUM, -1.0, 3, &RGBResistances[0], rWeights, 470, 0, 3, &RGBResistances[0], gWeights, 470, 0, 2, &RGBResistances[1], bWeights, 470, 0);
			
	// Colour PROM
	for (INT32 i = 0; i < 64; i++) {
		UINT8 Bit0, Bit1, Bit2, r, g, b;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],0);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],1);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],2);
		r = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],3);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],4);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],5);
		g = Combine3Weights(gWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],6);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],7);
		b = Combine2Weights(bWeights, Bit0, Bit1);

		GalPalette[i] = BurnHighCol(r, g, b, 0);
	}
	
	// Stars
	for (INT32 i = 0; i < GAL_PALETTE_NUM_COLOURS_STARS; i++) {
		INT32 Bits, r, g, b;
		INT32 Map[4] = {0x00, 0x88, 0xcc, 0xff};

		Bits = (i >> 0) & 0x03;
		r = Map[Bits];
		Bits = (i >> 2) & 0x03;
		g = Map[Bits];
		Bits = (i >> 4) & 0x03;
		b = Map[Bits];

		GalPalette[i + GAL_PALETTE_STARS_OFFSET] = BurnHighCol(r, g, b, 0);
	}

	// Bullets
	for (INT32 i = 0; i < GAL_PALETTE_NUM_COLOURS_BULLETS - 1; i++) {
		GalPalette[i + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0xff, 0);
	}
	GalPalette[GAL_PALETTE_NUM_COLOURS_BULLETS - 1 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0x00, 0);
}

void MarinerCalcPalette()
{
	GalaxianCalcPalette();
	
	for (INT32 i = 0; i < 16; i++) {
		INT32 b = 0x0e * BIT(i, 0) + 0x1f * BIT(i, 1) + 0x43 * BIT(i, 2) + 0x8f * BIT(i, 3);
		GalPalette[i + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, b, 0);
	}
}

void StratgyxCalcPalette()
{
	GalaxianCalcPalette();
	
	for (INT32 i = 0; i < 8; i++) {
		INT32 r = BIT(i, 0) * 0x7c;
		INT32 g = BIT(i, 1) * 0x3c;
		INT32 b = BIT(i, 2) * 0x47;
		GalPalette[i + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(r, g, b, 0);
	}
}

void RescueCalcPalette()
{
	GalaxianCalcPalette();
	
	for (INT32 i = 0; i < 128; i++) {
		INT32 b = i * 2;
		GalPalette[i + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, b, 0);
	}
}

void MinefldCalcPalette()
{
	RescueCalcPalette();
	
	for (INT32 i = 0; i < 128; i++) {
		INT32 r = (INT32)(i * 1.5);
		INT32 g = (INT32)(i * 0.75);
		INT32 b = i / 2;
		GalPalette[i + 128 + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(r, g, b, 0);
	}
}

void DarkplntCalcPalette()
{
	static const INT32 RGBResistances[3] = {1000, 470, 220};
	double rWeights[3], gWeights[3], bWeights[2];
	
	ComputeResistorWeights(0, RGB_MAXIMUM, -1.0, 3, &RGBResistances[0], rWeights, 470, 0, 3, &RGBResistances[0], gWeights, 470, 0, 2, &RGBResistances[1], bWeights, 470, 0);
			
	// Colour PROM
	for (INT32 i = 0; i < 32; i++) {
		UINT8 Bit0, Bit1, Bit2, r, g, b;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],0);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],1);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],2);
		r = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		g = 0;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],3);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],4);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],5);
		b = Combine2Weights(bWeights, Bit0, Bit1);

		GalPalette[i] = BurnHighCol(r, g, b, 0);
	}
	
	// Stars
	for (INT32 i = 0; i < GAL_PALETTE_NUM_COLOURS_STARS; i++) {
		INT32 Bits, r, g, b;
		INT32 Map[4] = {0x00, 0x88, 0xcc, 0xff};

		Bits = (i >> 0) & 0x03;
		r = Map[Bits];
		Bits = (i >> 2) & 0x03;
		g = Map[Bits];
		Bits = (i >> 4) & 0x03;
		b = Map[Bits];

		GalPalette[i + GAL_PALETTE_STARS_OFFSET] = BurnHighCol(r, g, b, 0);
	}

	// Bullets
	GalPalette[0 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xef, 0x00, 0x00, 0);
	GalPalette[1 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0x00, 0x00, 0xef, 0);
}

void DambustrCalcPalette()
{
	static const INT32 RGBResistances[3] = {1000, 470, 220};
	double rWeights[3], gWeights[3], bWeights[2];
	
	ComputeResistorWeights(0, RGB_MAXIMUM, -1.0, 3, &RGBResistances[0], rWeights, 470, 0, 3, &RGBResistances[0], gWeights, 470, 0, 2, &RGBResistances[1], bWeights, 470, 0);
			
	// Colour PROM
	for (INT32 i = 0; i < 32; i++) {
		UINT8 Bit0, Bit1, Bit2, r, g, b;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],0);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],1);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],2);
		b = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],3);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],4);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],5);
		r = Combine3Weights(gWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],6);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],7);
		g = Combine2Weights(bWeights, Bit0, Bit1);

		GalPalette[i] = BurnHighCol(r, g, b, 0);
	}
	
	// Stars
	for (INT32 i = 0; i < GAL_PALETTE_NUM_COLOURS_STARS; i++) {
		INT32 Bits, r, g, b;
		INT32 Map[4] = {0x00, 0x88, 0xcc, 0xff};

		Bits = (i >> 0) & 0x03;
		r = Map[Bits];
		Bits = (i >> 2) & 0x03;
		g = Map[Bits];
		Bits = (i >> 4) & 0x03;
		b = Map[Bits];

		GalPalette[i + GAL_PALETTE_STARS_OFFSET] = BurnHighCol(r, g, b, 0);
	}

	// Bullets
	for (INT32 i = 0; i < GAL_PALETTE_NUM_COLOURS_BULLETS - 1; i++) {
		GalPalette[i + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0xff, 0);
	}
	GalPalette[GAL_PALETTE_NUM_COLOURS_BULLETS - 1 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0x00, 0);
	
	for (INT32 i = 0; i < 8; i++) {
		INT32 r = BIT(i, 0) * 0x47;
		INT32 g = BIT(i, 1) * 0x47;
		INT32 b = BIT(i, 2) * 0x4f;
		GalPalette[i + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(r, g, b, 0);
	}
}

#undef RGB_MAXIMUM
#undef MAX_NETS
#undef MAX_RES_PER_NET
#undef Combine_2Weights
#undef Combine_3Weights

// Background and Stars rendering
void GalaxianDrawBackground()
{
	if (GalStarsEnable) GalaxianRenderStarLayer();
}

void RockclimDrawBackground()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = GalVideoRam2[TileIndex];
			Colour = 0;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= RockclimScrollX & 0x1ff;
			y -= RockclimScrollY & 0xff;
			
			if (x < -8) x += 512;
			if (y < -8) y += 256;
			
			y -= 16;

			if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
				Render8x8Tile(pTransDraw, Code, x, y, Colour, 4, 32, RockclimTiles);
			} else {
				Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 32, RockclimTiles);
			}

			TileIndex++;
		}
	}
}

void JumpbugDrawBackground()
{
	if (GalStarsEnable) JumpbugRenderStarLayer();
}

void FroggerDrawBackground()
{
	GalPalette[GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, 0x47, 0);
	
	if (GalFlipScreenX) {
		for (INT32 y = 0; y < nScreenHeight; y++) {
			for (INT32 x = nScreenWidth - 1; x > 128 - 8; x--) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	} else {
		for (INT32 y = 0; y < nScreenHeight; y++) {
			for (INT32 x = 0; x < 128 + 8; x++) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	}
}

void TurtlesDrawBackground()
{
	GalPalette[GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(GalBackgroundRed * 0x55, GalBackgroundGreen * 0x47, GalBackgroundBlue * 0x55, 0);
	
	for (INT32 y = 0; y < nScreenHeight; y++) {
		for (INT32 x = 0; x < nScreenWidth; x++) {
			pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
		}
	}
}

void ScrambleDrawBackground()
{
	GalPalette[GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, 0x56, 0);
	
	if (GalBackgroundEnable) {
		for (INT32 y = 0; y < nScreenHeight; y++) {
			for (INT32 x = 0; x < nScreenWidth; x++) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	}
	
	if (GalStarsEnable) ScrambleRenderStarLayer();
}

void AnteaterDrawBackground()
{
	GalPalette[GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, 0x56, 0);
	
	if (GalBackgroundEnable) {
		if (GalFlipScreenX) {
			for (INT32 y = 0; y < nScreenHeight; y++) {
				for (INT32 x = nScreenWidth - 1; x > 256 - 56; x--) {
					pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
				}
			}
		} else {
			for (INT32 y = 0; y < nScreenHeight; y++) {
				for (INT32 x = 0; x < 56; x++) {
					pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
				}
			}
		}
	}
}

void MarinerDrawBackground()
{
	UINT8 *BgColourProm = GalProm + 0x20;
	INT32 x;

	if (GalFlipScreenX) {
		for (x = 0; x < 32; x++) {
			INT32 Colour;
		
			if (x == 0) {
				Colour = 0;
			} else {
				Colour = BgColourProm[0x20 + x - 1];
			}
		
			INT32 xStart = 8 * (31 - x);
			for (INT32 sy = 0; sy < nScreenHeight; sy++) {
				for (INT32 sx = xStart; sx < xStart + 8; sx++) {
					pTransDraw[(sy * nScreenWidth) + sx] = GAL_PALETTE_BACKGROUND_OFFSET + Colour;
				}
			}
		}
	} else {
		for (x = 0; x < 32; x++) {
			INT32 Colour;
		
			if (x == 31) {
				Colour = 0;
			} else {
				Colour = BgColourProm[x + 1];
			}
		
			INT32 xStart = x * 8;
			for (INT32 sy = 0; sy < nScreenHeight; sy++) {
				for (INT32 sx = xStart; sx < xStart + 8; sx++) {
					pTransDraw[(sy * nScreenWidth) + sx] = GAL_PALETTE_BACKGROUND_OFFSET + Colour;
				}
			}
		}
	}
	
	if (GalStarsEnable) MarinerRenderStarLayer();
}

void StratgyxDrawBackground()
{
	UINT8 *BgColourProm = GalProm + 0x20;
	
	for (INT32 x = 0; x < 32; x++) {
		INT32 xStart, Colour = 0;

		if ((~BgColourProm[x] & 0x02) && GalBackgroundRed)   Colour |= 0x01;
		if ((~BgColourProm[x] & 0x02) && GalBackgroundGreen) Colour |= 0x02;
		if ((~BgColourProm[x] & 0x01) && GalBackgroundBlue)  Colour |= 0x04;

		if (GalFlipScreenX) {
			xStart = 8 * (31 - x);
		} else {
			xStart = 8 * x;
		}
		
		for (INT32 sy = 0; sy < nScreenHeight; sy++) {
			for (INT32 sx = xStart; sx < xStart + 8; sx++) {
				pTransDraw[(sy * nScreenWidth) + sx] = GAL_PALETTE_BACKGROUND_OFFSET + Colour;
			}
		}
	}
}

void RescueDrawBackground()
{
	if (GalBackgroundEnable) {
		INT32 x;

		for (x = 0; x < 128; x++) {
			for (INT32 y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET + x;
			}
		}
		
		for (x = 0; x < 120; x++) {
			for (INT32 y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + (x + 128)] = GAL_PALETTE_BACKGROUND_OFFSET + x + 8;
			}
		}
		
		for (x = 0; x < 8; x++) {
			for (INT32 y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + (x + 248)] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	}

	if (GalStarsEnable) RescueRenderStarLayer();
}

void MinefldDrawBackground()
{
	if (GalBackgroundEnable) {
		INT32 x;

		for (x = 0; x < 128; x++) {
			for (INT32 y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET + x;
			}
		}
		
		for (x = 0; x < 120; x++) {
			for (INT32 y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + (x + 128)] = GAL_PALETTE_BACKGROUND_OFFSET + x + 128;
			}
		}
		
		for (x = 0; x < 8; x++) {
			for (INT32 y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + (x + 248)] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	}

	if (GalStarsEnable) RescueRenderStarLayer();
}

void DambustrDrawBackground()
{
	INT32 xClipStart = GalFlipScreenX ? 254 - DambustrBgSplitLine : 0;
	INT32 xClipEnd = GalFlipScreenX ? 0 : 254 - DambustrBgSplitLine;
	
	for (INT32 x = 0; x < 256 - DambustrBgSplitLine; x++) {
		if (DambustrBgPriority && (x < xClipStart || x > xClipEnd)) continue;
		for (INT32 y = 0; y < nScreenHeight; y++) {
			pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET + ((GalFlipScreenX) ? DambustrBgColour2 : DambustrBgColour1);
		}
	}
	
	for (INT32 x = 255; x > 256 - DambustrBgSplitLine; x--) {
		if (DambustrBgPriority && (x < xClipStart || x > xClipEnd)) continue;
		for (INT32 y = 0; y < nScreenHeight; y++) {
			pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET + ((GalFlipScreenX) ? DambustrBgColour1 : DambustrBgColour2);
		}
	}

	if (GalStarsEnable && !DambustrBgPriority) GalaxianRenderStarLayer();
}

// Char Layer rendering
static void GalRenderBgLayer(UINT8 *pVideoRam)
{
	INT32 mx, my, Attr, Colour, x, y, TileIndex = 0, RamPos;
	UINT16 Code;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			RamPos = TileIndex & 0x1f;
			Code = pVideoRam[TileIndex];
			Attr = GalSpriteRam[(RamPos * 2) + 1];
			Colour = Attr  & ((GalColourDepth == 3) ? 0x03 : 0x07);
			
			if (GalExtendTileInfoFunction) GalExtendTileInfoFunction(&Code, &Colour, Attr, RamPos);
			
			if (SfxTilemap) {
				x = 8 * my;
				y = 8 * mx;
			} else {
				x = 8 * mx;
				y = 8 * my;
			}
		
			y -= 16;
		
			if (GalFlipScreenX) x = nScreenWidth - 8 - x;
			if (GalFlipScreenY) y = nScreenHeight - 8 - y;
		
			INT32 px, py;
		
			UINT32 nPalette = Colour << GalColourDepth;
		
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					UINT8 c = GalChars[(Code * 64) + (py * 8) + px];
					if (GalFlipScreenX) c = GalChars[(Code * 64) + (py * 8) + (7 - px)];
					if (GalFlipScreenY) c = GalChars[(Code * 64) + ((7 - py) * 8) + px];
					if (GalFlipScreenX && GalFlipScreenY) c = GalChars[(Code * 64) + ((7 - py) * 8) + (7 - px)];
				
					if (c) {
						INT32 xPos = x + px;
						INT32 yPos = y + py;
					
						if (SfxTilemap) {
							if (GalFlipScreenX) {
								xPos += GalScrollVals[mx];
							} else {
								xPos -= GalScrollVals[mx];
							}
							
							if (xPos < 0) xPos += 256;
							if (xPos > 255) xPos -= 256;
						} else {
							if (GalFlipScreenY) {
								yPos += GalScrollVals[mx];
							} else {
								yPos -= GalScrollVals[mx];
							}
							
							if (yPos < 0) yPos += 256;
							if (yPos > 255) yPos -= 256;
						}
						
						if (GalOrientationFlipX) {
							xPos = nScreenWidth - 1 - xPos;
						}
					
						if (yPos >= 0 && yPos < nScreenHeight) {					
							UINT16* pPixel = pTransDraw + (yPos * nScreenWidth);
						
							if (xPos >= 0 && xPos < nScreenWidth) {
								pPixel[xPos] = c | nPalette;
							}
						}
					}
				}
			}
		
			TileIndex++;
		}
	}
}

// Sprite Rendering
static void GalRenderSprites(const UINT8 *SpriteBase)
{
	INT32 SprNum;
	INT32 ClipOfs = GalFlipScreenX ? 16 : 0;
	INT32 xMin = GalSpriteClipStart - ClipOfs;
	INT32 xMax = GalSpriteClipEnd - ClipOfs + 1;
	
	for (SprNum = 7; SprNum >= 0; SprNum--) {
		const UINT8 *Base = &SpriteBase[SprNum * 4];
		UINT8 Base0 = FroggerAdjust ? ((Base[0] >> 4) | (Base[0] << 4)) : Base[0];
		INT32 sy = 240 - (Base0 - (SprNum < 3));
		UINT16 Code = Base[1] & 0x3f;
		UINT8 xFlip = Base[1] & 0x40;
		UINT8 yFlip = Base[1] & 0x80;
		UINT8 Colour = Base[2] & ((GalColourDepth == 3) ? 0x03 : 0x07);
		INT32 sx = Base[3];

		if (GalExtendSpriteInfoFunction) GalExtendSpriteInfoFunction(Base, &sx, &sy, &xFlip, &yFlip, &Code, &Colour);
		
		if (GalFlipScreenX) {
			sx = 242 - sx;
			xFlip = !xFlip;
		}
		
		if (sx < xMin || sx > xMax) continue;
		
		if (GalFlipScreenY) {
			sy = 240 - sy;
			yFlip = !yFlip;
		}
		
		sy -= 16;
		
		if (GalOrientationFlipX) {
			sx = 242 - 1 - sx;
			xFlip = !xFlip;
		}
		
		if (sx > 16 && sx < (nScreenWidth - 16) && sy > 16 && sy < (nScreenHeight - 16)) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				}
			}
		}
	}
}

// Bullet rendering
static inline void GalDrawPixel(INT32 x, INT32 y, INT32 Colour)
{
	if (y >= 0 && y < nScreenHeight && x >= 0 && x < nScreenWidth) {
		pTransDraw[(y * nScreenWidth) + x] = Colour;
	}
}

void GalaxianDrawBullets(INT32 Offs, INT32 x, INT32 y)
{
	x -= 4;
	
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
}

void TheendDrawBullets(INT32 Offs, INT32 x, INT32 y)
{
	x -= 4;
	
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 7] = BurnHighCol(0xff, 0x00, 0xff, 0);
	
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
}

void ScrambleDrawBullets(INT32, INT32 x, INT32 y)
{
	x -= 6;
	
	GalDrawPixel(x, y, GAL_PALETTE_BULLETS_OFFSET + 7);
}

void MoonwarDrawBullets(INT32, INT32 x, INT32 y)
{
	x -= 6;
	
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 7] = BurnHighCol(0xef, 0xef, 0x97, 0);
	
	GalDrawPixel(x, y, GAL_PALETTE_BULLETS_OFFSET + 7);
}

void MshuttleDrawBullets(INT32, INT32 x, INT32 y)
{
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 0] = BurnHighCol(0xff, 0xff, 0xff, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 1] = BurnHighCol(0xff, 0xff, 0x00, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 2] = BurnHighCol(0x00, 0xff, 0xff, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 3] = BurnHighCol(0x00, 0xff, 0x00, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 4] = BurnHighCol(0xff, 0x00, 0xff, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 5] = BurnHighCol(0xff, 0x00, 0x00, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 6] = BurnHighCol(0x00, 0x00, 0xff, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 7] = BurnHighCol(0x00, 0x00, 0x00, 0);
	
	--x;
	GalDrawPixel(x, y, ((x & 0x40) == 0) ? GAL_PALETTE_BULLETS_OFFSET +  + ((x >> 2) & 7) : GAL_PALETTE_BULLETS_OFFSET +  + 4);
	--x;
	GalDrawPixel(x, y, ((x & 0x40) == 0) ? GAL_PALETTE_BULLETS_OFFSET +  + ((x >> 2) & 7) : GAL_PALETTE_BULLETS_OFFSET +  + 4);
	--x;
	GalDrawPixel(x, y, ((x & 0x40) == 0) ? GAL_PALETTE_BULLETS_OFFSET +  + ((x >> 2) & 7) : GAL_PALETTE_BULLETS_OFFSET +  + 4);
	--x;
	GalDrawPixel(x, y, ((x & 0x40) == 0) ? GAL_PALETTE_BULLETS_OFFSET +  + ((x >> 2) & 7) : GAL_PALETTE_BULLETS_OFFSET +  + 4);
}

void DarkplntDrawBullets(INT32, INT32 x, INT32 y)
{
	if (GalFlipScreenX) x++;
	
	x -= 6;
	
	GalDrawPixel(x, y, GAL_PALETTE_BULLETS_OFFSET + DarkplntBulletColour);
}

void DambustrDrawBullets(INT32 Offs, INT32 x, INT32 y)
{
	INT32 Colour;
	
	if (GalFlipScreenX) x++;
	
	x -= 6;
	
	for (INT32 i = 0; i < 2; i++) {
		if (Offs < 16) {
			Colour = GAL_PALETTE_BULLETS_OFFSET + 7;
			y--;
		} else {
			Colour = GAL_PALETTE_BULLETS_OFFSET;
			x--;
		}
	}
	
	GalDrawPixel(x, y, Colour);
}

static void GalDrawBullets(const UINT8 *Base)
{
	for (INT32 y = 0; y < nScreenHeight; y++) {
		UINT8 Shell = 0xff;
		UINT8 Missile = 0xff;
		UINT8 yEff;
		INT32 Which;
		
		yEff = (GalFlipScreenY) ? (y + 16 - 1) ^ 255 : y + 16 - 1;
		
		for (Which = 0; Which < 3; Which++) {
			if ((UINT8)(Base[Which * 4 + 1] + yEff) == 0xff) Shell = Which;
		}
		
		yEff = (GalFlipScreenY) ? (y + 16) ^ 255 : y + 16;
		
		for (Which = 3; Which < 8; Which++) {
			if ((UINT8)(Base[Which * 4 + 1] + yEff) == 0xff) {
				if (Which != 7) {
					Shell = Which;
				} else {
					Missile = Which;
				}
			}
		}
		
		if (Shell != 0xff) GalDrawBulletsFunction(Shell, (GalOrientationFlipX) ? Base[Shell * 4 + 3] : 255 - Base[Shell * 4 + 3], y);
		if (Missile != 0xff) GalDrawBulletsFunction(Missile, (GalOrientationFlipX) ? Base[Missile * 4 + 3] : 255 - Base[Missile * 4 + 3], y);
	}
}

// Render a frame
void GalDraw()
{
	if (GalRenderFrameFunction) {
		GalRenderFrameFunction();
	} else {
		BurnTransferClear();
		GalCalcPaletteFunction();
		if (GalRenderBackgroundFunction) GalRenderBackgroundFunction();
		GalRenderBgLayer(GalVideoRam);
		GalRenderSprites(&GalSpriteRam[0x40]);
		if (GalDrawBulletsFunction) GalDrawBullets(&GalSpriteRam[0x60]);
		BurnTransferCopy(GalPalette);
	}
}

void DkongjrmRenderFrame()
{
	BurnTransferClear();
	GalCalcPaletteFunction();
	if (GalRenderBackgroundFunction) GalRenderBackgroundFunction();
	GalRenderBgLayer(GalVideoRam);
	GalRenderSprites(&GalSpriteRam[0x40]);
	GalRenderSprites(&GalSpriteRam[0x60]);
	GalRenderSprites(&GalSpriteRam[0xc0]);
	GalRenderSprites(&GalSpriteRam[0xe0]);
	if (GalDrawBulletsFunction) GalDrawBullets(&GalSpriteRam[0x60]);
	BurnTransferCopy(GalPalette);
}

void DambustrRenderFrame()
{
	BurnTransferClear();
	GalCalcPaletteFunction();
	if (GalRenderBackgroundFunction) GalRenderBackgroundFunction();
	GalRenderBgLayer(GalVideoRam);
	GalRenderSprites(&GalSpriteRam[0x40]);
	if (GalDrawBulletsFunction) GalDrawBullets(&GalSpriteRam[0x60]);
	if (DambustrBgPriority) {
		if (GalRenderBackgroundFunction) GalRenderBackgroundFunction();
		memset(GalVideoRam2, 0x20, 0x400);
		for (INT32 i = 0; i < 32; i++) {
			INT32 Colour = GalSpriteRam[(i << 1) | 1] & 7;
			if (Colour > 3) {
				for (INT32 j = 0; j < 32; j++) GalVideoRam2[(32 * j) + i] = GalVideoRam[(32 * j) + i];
			}
		}
		GalRenderBgLayer(GalVideoRam2);
	}	
	BurnTransferCopy(GalPalette);
}
