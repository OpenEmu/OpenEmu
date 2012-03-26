// FB Alpha Meijinsen Driver Module 
// Based on MAME driver by Tomasz Slanina

#include "burnint.h"
#include "sek.h"
#include "zet.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT32 *Palette;
static UINT8 *Mem,  *M68KRom, *Z80Rom, *Prom;
static INT16 *pFMBuffer, *pAY8910Buffer[3];
static UINT8 DrvJoy1[9], DrvJoy2[8], DrvReset, DrvDips;
static UINT8 soundlatch, deposits1, deposits2, credits;

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 coin"   },
	{"Coin 2",		BIT_DIGITAL,  DrvJoy2 + 6,  "p2 coin"   },
	{"Start 1",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"  },
	{"Start 2",		BIT_DIGITAL,	DrvJoy2 + 7,  "p2 start"  },

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0, 	"p1 up"     },
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1, 	"p1 down"   },
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 2, 	"p1 right"  },
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3, 	"p1 left"   },
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1" },
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2" },

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0, 	"p2 up"     },
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1, 	"p2 down"   },
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 2, 	"p2 right"  },
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3, 	"p2 left"   },
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1" },
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2" },

	{"Clear Credit",	BIT_DIGITAL, 	DrvJoy1 + 8,  "clear credit" },
	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"     },
	{"Dip Switches",	BIT_DIPSWITCH,	&DrvDips,	"dip"  },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[] =
{
	// Defaults
	{0x12, 0xFF, 0xFF, 0x08, NULL},

	{0,    0xFE, 0,	   8,	 "Game time (actual game)"},
	{0x12, 0x01, 0x07, 0x07, "01:00"},
	{0x12, 0x01, 0x07, 0x06, "02:00"},
	{0x12, 0x01, 0x07, 0x05, "03:00"},
	{0x12, 0x01, 0x07, 0x04, "04:00"},
	{0x12, 0x01, 0x07, 0x03, "05:00"},
	{0x12, 0x01, 0x07, 0x02, "10:00"},
	{0x12, 0x01, 0x07, 0x01, "20:00"},
	{0x12, 0x01, 0x07, 0x00, "00:30"},

	{0,    0xFE, 0,	   2,	 "Coinage"},
	{0x12, 0x01, 0x08, 0x08, "A 1C/1C B 1C/5C"},
	{0x12, 0x01, 0x08, 0x00, "A 1C/2C B 2C/1C"},

	{0,    0xFE, 0,	   2,	 "2 Player"},
	{0x12, 0x01, 0x10, 0x00, "1C"},
	{0x12, 0x01, 0x10, 0x10, "2C"},

	{0,    0xFE, 0,	   2,	 "Game time (tsumeshougi)"},
	{0x12, 0x01, 0x20, 0x20, "01:00"},
	{0x12, 0x01, 0x20, 0x00, "02:00"},
};

STDDIPINFO(Drv)


UINT16 inputs(INT32 inp)
{
	UINT8 ret = 0;

	switch (inp)
	{
		case 0x00: { // start + service
			ret |= DrvJoy1[7] << 0;	// 
			ret |= DrvJoy2[7] << 1;
			ret |= DrvJoy1[8] << 7; // clear credit
		}
		break;

		case 0x01: { // player 1 controls
			for (INT32 i = 0; i < 6; i++) ret |= DrvJoy1[i] << i;
		}
		break;

		case 0x02: { // player 2 controls
			for (INT32 i = 0; i < 6; i++) ret |= DrvJoy2[i] << i;
		}
		break;

		case 0x03: { // coins
			ret = (DrvJoy1[6] | (DrvJoy2[6] << 1)) ^ 3;
		}
		break;

		break;
	}

	return ret;
}

UINT8 soundlatch_r(UINT32)
{
	return soundlatch;
}

UINT8 __fastcall alpha_mcu_r(UINT8 offset)
{
	static UINT8 coinvalue = 0;
	static const UINT8 coinage1[2][2] = { {1, 1}, {1, 2} };
	static const UINT8 coinage2[2][2] = { {1, 5}, {2, 1} };

	static INT32 latch;

	switch (offset)
	{
		case 0x01: // Dipswitch 2
			M68KRom[0x180e44] = DrvDips & 0xff;
		return 0;

		case 0x45: // Coin value
			M68KRom[0x180e44] = credits & 0xff; 
		return 0;

		case 0x53: // Query microcontroller for coin insert
		{
			credits = 0;

			if ((inputs(3) & 0x03) == 0x03) latch = 0; 

			M68KRom[0x180e52] = 0x22;

			if ((inputs(3) & 0x03) != 0x03 && latch == 0)
			{
				M68KRom[0x180e44] = 0x00;

				latch = 1;

				coinvalue = (~DrvDips>>3) & 1;

				if (~inputs(3) & 0x01)
				{
					deposits1++;
					if (deposits1 == coinage1[coinvalue][0])
					{
						credits = coinage1[coinvalue][1];
						deposits1 = 0;
					}
					else
						credits = 0;
				}
				else if (~inputs(3) & 0x02)
				{
					deposits2++;
					if (deposits2 == coinage2[coinvalue][0])
					{
						credits = coinage2[coinvalue][1];
						deposits2 = 0;
					}
					else
						credits = 0;
				}
			}
		}

		return 0;
	}

	return 0;
}

UINT8 __fastcall meijinsn_read_byte(UINT32 a)
{
	if ((a & ~0xff) == 0x080e00) {
		return alpha_mcu_r(a & 0xff);
	}

	switch (a)
	{
		case 0x1a0000:
			return inputs(0);

		case 0x1a0001:
			return inputs(1);

		case 0x1c0000:
			return inputs(2);
	}

	return 0;
}

void __fastcall meijinsn_write_byte(UINT32 a, UINT8 d)
{
	if (a == 0x1a0001) {
		soundlatch = d & 0xff;
		return;
	}
}


UINT8 __fastcall meijinsn_in_port(UINT16 a)
{
	if ((a & 0xff) == 0x01) { // AY8910 read port
		return AY8910Read(0);
	}

	return 0;
}

void __fastcall meijinsn_out_port(UINT16 a, UINT8 data)
{
	switch (a & 0xff)
	{
		case 0x00: // AY8910 control port
			AY8910Write(0, 0, data);
		break;

		case 0x01: // AY8910 write port
			AY8910Write(0, 1, data);
		break;

		case 0x02: // Soundlatch clear
			soundlatch = 0;
		break;
	}
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (M68KRom + 0x100000, 0, 0x082000); // clear 68k RAM
	memset (Z80Rom  + 0x008000, 0, 0x000800); // clear z80 RAM

	deposits1 = 0;
	deposits2 = 0;
	credits   = 0;

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	AY8910Reset(0);

	return 0;
}

static void Pallete_Init()
{
#define combine_3_weights(tab,w0,w1,w2)		((int)(((tab)[0]*(w0) + (tab)[1]*(w1) + (tab)[2]*(w2)) + 0.5))

	float weights_r[3] = { 41.697944,  73.045335, 140.256721};
	float weights_g[3] = { 41.697944,  73.045335, 140.256721};
	float weights_b[3] = { 83.228546, 159.809836,   0.000000};

	for (INT32 i = 0; i < 0x10; i++)
	{
		INT32 bit0,bit1,bit2,r,g,b;

		// red component
		bit0 = (Prom[i] >> 0) & 0x01;
		bit1 = (Prom[i] >> 1) & 0x01;
		bit2 = (Prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);

		// green component
		bit0 = (Prom[i] >> 3) & 0x01;
		bit1 = (Prom[i] >> 4) & 0x01;
		bit2 = (Prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);

		// blue component
		bit0 = (Prom[i] >> 6) & 0x01;
		bit1 = (Prom[i] >> 7) & 0x01;
		b = combine_3_weights(weights_b, bit0, bit1, 0);

		Palette[i] = (r << 16) | (g << 8) | b;
	}
}

static INT32 DrvInit()
{
	Mem = (UINT8 *)BurnMalloc(0x210020 + (0x10 * sizeof(UINT32)));
	if (Mem == NULL) {
		return 1;
	}

	M68KRom = Mem + 0x000000;
	Z80Rom  = Mem + 0x200000;
	Prom    = Mem + 0x210000;
	Palette = (UINT32 *)(Mem + 0x210020);

	pFMBuffer = (INT16 *)malloc (nBurnSoundLen * 3 * sizeof(INT16));
	if (pFMBuffer == NULL) {
		return 1;
	}

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;

	// Load roms
	{
		INT32 i;
		for (i = 0; i < 8; i+=2) {
			BurnLoadRom(M68KRom + 0x100001, i + 0, 2);
			BurnLoadRom(M68KRom + 0x100000, i + 1, 2);

			memcpy (M68KRom + 0x00000 + 0x08000 * (i >> 1), M68KRom + 0x100000, 0x08000);
			memcpy (M68KRom + 0x20000 + 0x08000 * (i >> 1), M68KRom + 0x108000, 0x08000);
		}

		BurnLoadRom(Z80Rom + 0x00000, 8, 1);
		BurnLoadRom(Z80Rom + 0x04000, 9, 1);
		BurnLoadRom(Prom,	     10, 1);
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(M68KRom,		 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(M68KRom + 0x100000, 0x100000, 0x181fff, SM_RAM);
	SekSetReadByteHandler(0, meijinsn_read_byte);
	SekSetWriteByteHandler(0, meijinsn_write_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xffff, 0, Z80Rom);
	ZetMapArea(0x0000, 0x7fff, 2, Z80Rom);
	ZetMapArea(0x8000, 0x87ff, 0, Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0x87ff, 1, Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0x87ff, 2, Z80Rom + 0x8000);
	ZetSetInHandler(meijinsn_in_port);
	ZetSetOutHandler(meijinsn_out_port);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 2000000, nBurnSoundRate, &soundlatch_r, NULL, NULL, NULL);

	Pallete_Init();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	AY8910Exit(0);
	SekExit();
	ZetExit();

	BurnFree (Mem);
	Palette = NULL;
	BurnFree (pFMBuffer);
	Mem = M68KRom = Z80Rom = Prom = NULL;
	pFMBuffer = NULL;
	pAY8910Buffer[0] = pAY8910Buffer[1] = pAY8910Buffer[2] = NULL;

	return 0;
}


static INT32 DrvDraw()
{
	for (INT32 i = 0; i < 0x4000; i++)
	{
		INT32 sx, sy, x, data1, data2, color, data;

		sx = (i >> 6) & 0xfc;
		sy = i & 0xff;

		if (sy < 16 || sy > 239 || sx < 12 || sx > 240) continue;
		sx -= 12; sy = (sy - 16) * 232;

		data1 = M68KRom[0x100001 + (i << 1)];
		data2 = M68KRom[0x100000 + (i << 1)];

		for (x = 0; x < 4; x++)
		{
			color = ((data1 >> x) & 1) | ((data1 >> (3 + x)) & 2);
			data  = ((data2 >> x) & 1) | ((data2 >> (3 + x)) & 2);

			UINT32 c = Palette[(color << 2) | data];

			PutPix(pBurnDraw + (sy + sx + (x ^ 3)) * nBurnBpp, BurnHighCol(c >> 16, c >> 8, c, 0));
		}
	}

	return 0;
}


static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 160;

	INT32 nCyclesSegment;
	INT32 nCyclesDone[2], nCyclesTotal[2];

	nCyclesTotal[0] = 9000000 / 60;
	nCyclesTotal[1] = 4000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext;

		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[0];
		nCyclesDone[0] += SekRun(nCyclesSegment);
		if (i ==  79) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		if (i == 159) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

		nNext = (i + 1) * nCyclesTotal[1] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[1];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[1] += nCyclesSegment;
		ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);

		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSample;
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample /= 4;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
			}
			nSoundBufferPos += nSegmentLength;
		}
	}

	ZetClose();
	SekClose();

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSample;
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample /= 4;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
			}
		}
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}


static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {					// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {			// Scan volatile ram		
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = M68KRom + 0x100000;
		ba.nLen	  = 0x082000;
		ba.szName = "Main 68K Ram";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  = Z80Rom + 0x8000;
		ba.nLen	  = 0x00800;
		ba.szName = "Main Z80 Ram";
		BurnAcb(&ba);

		SekScan(nAction);			// Scan 68K
		ZetScan(nAction);			// Scan Z80
		AY8910Scan(nAction, pnMin);		// Scan AY8910

		// Scan critical driver variables
		SCAN_VAR(soundlatch);
		SCAN_VAR(deposits1);
		SCAN_VAR(deposits2);
		SCAN_VAR(credits);
	}

	return 0;
}


// Meijinsen

static struct BurnRomInfo meijinsnRomDesc[] = {
	{ "p1",  0x08000, 0x8c9697a3, BRF_PRG | BRF_ESS }, //  0 M68000 Code
	{ "p2",  0x08000, 0xf7da3535, BRF_PRG | BRF_ESS }, //  1
	{ "p3",  0x08000, 0x0af0b266, BRF_PRG | BRF_ESS }, //  2
	{ "p4",  0x08000, 0xaab159c5, BRF_PRG | BRF_ESS }, //  3
	{ "p5",  0x08000, 0x0ed10a47, BRF_PRG | BRF_ESS }, //  4
	{ "p6",  0x08000, 0x60b58755, BRF_PRG | BRF_ESS }, //  5
	{ "p7",  0x08000, 0x604c76f1, BRF_PRG | BRF_ESS }, //  6
	{ "p8",  0x08000, 0xe3eaef19, BRF_PRG | BRF_ESS }, //  7

	{ "p9",  0x04000, 0xaedfefdf, BRF_PRG | BRF_ESS }, //  8 Z80 Code
	{ "p10", 0x04000, 0x93b4d764, BRF_PRG | BRF_ESS }, //  9

	{ "clr", 0x00020, 0x7b95b5a7, BRF_GRA },	   // 10 Color Prom
};

STD_ROM_PICK(meijinsn)
STD_ROM_FN(meijinsn)

struct BurnDriver BurnDrvMeijinsn = {
	"meijinsn", NULL, NULL, NULL, "1986",
	"Meijinsen\0", NULL, "SNK Electronics corp.", "Miscellaneous",
	L"\u540D\u4EBA\u6226\0Meijinsen\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PUZZLE, 0,
	NULL, meijinsnRomInfo, meijinsnRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x10,
	232, 224, 4, 3
};
