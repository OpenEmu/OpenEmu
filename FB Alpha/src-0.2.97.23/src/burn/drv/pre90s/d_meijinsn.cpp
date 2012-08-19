// FB Alpha Meijinsen Driver Module 
// Based on MAME driver by Tomasz Slanina

#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvColPROM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;

static INT16 *pAY8910Buffer[3];

static UINT16 DrvInputs[3];
static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvReset;
static UINT8 DrvDips;

static INT32 credits;
static INT32 mcu_latch;
static INT32 deposits1;
static INT32 deposits2;

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"Coin 2",		BIT_DIGITAL,    DrvJoy3 + 1,    "p2 coin"	},

	{"Start 1",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0, 	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1, 	"p1 down"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2, 	"p1 right"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3, 	"p1 left"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"Start 2",		BIT_DIGITAL,	DrvJoy1 + 9,    "p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0, 	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1, 	"p2 down"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 2, 	"p2 right"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3, 	"p2 left"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Service Mode",	BIT_DIGITAL, 	DrvJoy1 + 15,   "Service"	},
	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip Switches",	BIT_DIPSWITCH,	&DrvDips,	"dip"		},
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[] =
{
	{0x12, 0xFF, 0xFF, 0x08, NULL				},

	{0,    0xFE, 0,	   8,	 "Game time (actual game)"	},
	{0x12, 0x01, 0x07, 0x07, "01:00"			},
	{0x12, 0x01, 0x07, 0x06, "02:00"			},
	{0x12, 0x01, 0x07, 0x05, "03:00"			},
	{0x12, 0x01, 0x07, 0x04, "04:00"			},
	{0x12, 0x01, 0x07, 0x03, "05:00"			},
	{0x12, 0x01, 0x07, 0x02, "10:00"			},
	{0x12, 0x01, 0x07, 0x01, "20:00"			},
	{0x12, 0x01, 0x07, 0x00, "00:30"			},

	{0,    0xFE, 0,	   2,	 "Coinage"			},
	{0x12, 0x01, 0x08, 0x08, "A 1C/1C B 1C/5C"		},
	{0x12, 0x01, 0x08, 0x00, "A 1C/2C B 2C/1C"		},

	{0,    0xFE, 0,	   2,	 "2 Player"			},
	{0x12, 0x01, 0x10, 0x00, "1C"				},
	{0x12, 0x01, 0x10, 0x10, "2C"				},

	{0,    0xFE, 0,	   2,	 "Game time (tsumeshougi)"	},
	{0x12, 0x01, 0x20, 0x20, "01:00"			},
	{0x12, 0x01, 0x20, 0x00, "02:00"			},
};

STDDIPINFO(Drv)

static UINT8 alpha_mcu_r(UINT8 offset)
{
	static const UINT8 coinage1[2][2] = {{1,1}, {1,2}};
	static const UINT8 coinage2[2][2] = {{1,5}, {2,1}};

	UINT16 source = SekReadWord(0x180e00 + offset);

	switch (offset)
	{
		case 0x00:
			SekWriteWord(0x180e00, (source & 0xff00) | DrvDips);
			return 0;

		case 0x44:
			SekWriteWord(0x180e44, (source & 0xff00) | credits);
			return 0;

		case 0x52:
			credits = 0;
			if ((DrvInputs[2] & 0x3) == 0)
				mcu_latch = 0;

			SekWriteWord(0x180e52, (source & 0xff00) | 0x22);

			if ((DrvInputs[2] & 0x1) == 0x1 && !mcu_latch)
			{
				SekWriteWord(0x180e44, (source & 0xff00) | 0x00);
				mcu_latch = 1;

				INT32 coinvalue = (~DrvDips >> 3) & 1;

				deposits1++;
				if (deposits1 == coinage1[coinvalue][0])
				{
					credits = coinage1[coinvalue][1];
					deposits1 = 0;
				}
				else
					credits = 0;
			}
			else if ((DrvInputs[2] & 0x2) == 0x2 && !mcu_latch)
			{
				SekWriteWord(0x180e44, (source & 0xff00) | 0x00);
				mcu_latch = 1;

				INT32 coinvalue = (~DrvDips >> 3) & 1;

				deposits2++;
				if (deposits2 == coinage2[coinvalue][0])
				{
					credits = coinage2[coinvalue][1];
					deposits2 = 0;
				}
				else
					credits = 0;
			}

			return 0;
	}

	return 0;
}

UINT8 __fastcall meijinsn_read_byte(UINT32 address)
{
	if ((address & ~0xff) == 0x080e00) {
		return alpha_mcu_r(address & 0xfe);
	}

	switch (address)
	{
		case 0x1a0000:
			return DrvInputs[0] >> 8;

		case 0x1a0001:
			return DrvInputs[0] >> 0;

		case 0x1c0000:
			return DrvInputs[1] >> 8;
	}

	return 0;
}

void __fastcall meijinsn_write_byte(UINT32 address, UINT8 data)
{
	if (address == 0x1a0001) {
		*soundlatch = data;
		return;
	}
}

UINT8 __fastcall meijinsn_in_port(UINT16 port)
{
	if ((port & 0xff) == 0x01) {
		return AY8910Read(0);
	}

	return 0;
}

void __fastcall meijinsn_out_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			AY8910Write(0, port & 1, data);
		break;

		case 0x02:
			*soundlatch = 0;
		break;
	}
}

static UINT8 ay8910_port_a_r(UINT32)
{
	return *soundlatch;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	AY8910Reset(0);

	credits = 0;
	mcu_latch = 0;
	deposits1 = 0;
	deposits2 = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x040000;
	DrvZ80ROM		= Next; Next += 0x008000;

	DrvColPROM		= Next; Next += 0x000020;

	DrvPalette		= (UINT32*)Next; Next += 0x0010 * sizeof(UINT32);

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x002000;
	DrvZ80RAM		= Next; Next += 0x000800;
	DrvVidRAM		= Next; Next += 0x008000;

	soundlatch		= Next; Next += 0x000001;

	RamEnd			= Next;

	pAY8910Buffer[0]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[1]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[2]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);

	MemEnd			= Next;

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x10; i++)
	{
		INT32 bit0 = (DrvColPROM[i] >> 0) & 0x01;
		INT32 bit1 = (DrvColPROM[i] >> 1) & 0x01;
		INT32 bit2 = (DrvColPROM[i] >> 2) & 0x01;

		INT32 r = ((bit0 * 4169) + (bit1 * 7304) + (bit2 * 14025)) / 100;

		bit0 = (DrvColPROM[i] >> 3) & 0x01;
		bit1 = (DrvColPROM[i] >> 4) & 0x01;
		bit2 = (DrvColPROM[i] >> 5) & 0x01;

		INT32 g = ((bit0 * 4169) + (bit1 * 7304) + (bit2 * 14025)) / 100;

		bit0 = (DrvColPROM[i] >> 6) & 0x01;
		bit1 = (DrvColPROM[i] >> 7) & 0x01;

		INT32 b = ((bit0 * 8322) + (bit1 * 15980)) / 100;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		UINT8 *tmp = BurnMalloc(0x10000);

		if (BurnLoadRom(tmp + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(tmp + 0x000000,  1, 2)) return 1;

		memcpy (Drv68KROM + 0x000000, tmp + 0x000000, 0x008000);
		memcpy (Drv68KROM + 0x020000, tmp + 0x008000, 0x008000);

		if (BurnLoadRom(tmp + 0x000001,  2, 2)) return 1;
		if (BurnLoadRom(tmp + 0x000000,  3, 2)) return 1;

		memcpy (Drv68KROM + 0x008000, tmp + 0x000000, 0x008000);
		memcpy (Drv68KROM + 0x028000, tmp + 0x008000, 0x008000);

		if (BurnLoadRom(tmp + 0x000001,  4, 2)) return 1;
		if (BurnLoadRom(tmp + 0x000000,  5, 2)) return 1;

		memcpy (Drv68KROM + 0x010000, tmp + 0x000000, 0x008000);
		memcpy (Drv68KROM + 0x030000, tmp + 0x008000, 0x008000);

		if (BurnLoadRom(tmp + 0x000001,  6, 2)) return 1;
		if (BurnLoadRom(tmp + 0x000000,  7, 2)) return 1;

		memcpy (Drv68KROM + 0x018000, tmp + 0x000000, 0x008000);
		memcpy (Drv68KROM + 0x038000, tmp + 0x008000, 0x008000);

		BurnFree (tmp);

		if (BurnLoadRom(DrvZ80ROM + 0x00000, 8, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x04000, 9, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x000, 10, 1)) return 1;

		DrvPaletteInit();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(DrvVidRAM,		0x100000, 0x107fff, SM_RAM);
	SekMapMemory(Drv68KRAM, 	0x180000, 0x181fff, SM_RAM);
	SekSetWriteByteHandler(0,	meijinsn_write_byte);
	SekSetReadByteHandler(0,	meijinsn_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetOutHandler(meijinsn_out_port);
	ZetSetInHandler(meijinsn_in_port);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 2000000, nBurnSoundRate, &ay8910_port_a_r, NULL, NULL, NULL);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();
	ZetExit();
	AY8910Exit(0);

	BurnFree(AllMem);

	return 0;
}

static void draw_layer()
{
	UINT16 *vram = (UINT16*)DrvVidRAM;

	for (INT32 i = 0; i < 0x4000; i++)
	{
		INT32 sx = (i >> 6) & 0xfc;
		INT32 sy = i & 0xff;

		if (sy < 16 || sy > 239 || sx < 12 || sx > 240) continue;

		sx -= 12;
		sy -= 16;

		INT32 data = vram[i];

		for (INT32 x = 0; x < 4; x++, data >>= 1)
		{
			pTransDraw[sy * 232 + sx + (x ^ 3)] = (data & 1) | ((data >> 3) & 2) | ((data >> 6) & 4) | ((data >> 9) & 8);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	draw_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0, 3 * sizeof(INT16));

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nInterleave = 160;
	INT32 nCyclesSegment;
	INT32 nCyclesTotal[2] = { 9000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext;

		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[0];
		nCyclesDone[0] += SekRun(nCyclesSegment);
		if (i ==   0) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		if (i == 159) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

		nNext = (i + 1) * nCyclesTotal[1] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[1];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[1] += nCyclesSegment;
		ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
	}

	ZetClose();
	SekClose();

	if (pBurnSoundOut) {
		INT32 nSample;
		AY8910Update(0, &pAY8910Buffer[0], nBurnSoundLen);
		for (INT32 n = 0; n < nBurnSoundLen; n++) {
			nSample  = pAY8910Buffer[0][n];
			nSample += pAY8910Buffer[1][n];
			nSample += pAY8910Buffer[2][n];

			nSample /= 4;

			nSample = BURN_SND_CLIP(nSample);

			pBurnSoundOut[(n << 1) + 0] = nSample;
			pBurnSoundOut[(n << 1) + 1] = nSample;
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

	if (pnMin) {
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);

		SCAN_VAR(mcu_latch);
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
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x10,
	232, 224, 4, 3
};
