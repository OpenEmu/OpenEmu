// FB Alpha Dottori Kun driver module
// Based on MAME driver by Takahiro Nogi

#include "tiles_generic.h"
#include "zet.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvZ80RAM;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvReset;
static UINT8 DrvInputs[1];

static UINT8 *nColor;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Up",	BIT_DIGITAL,	DrvJoy1 + 0,    "p1 up"    },
	{"P1 Down",	BIT_DIGITAL,	DrvJoy1 + 1,    "p1 down", },
	{"P1 Left",	BIT_DIGITAL,	DrvJoy1 + 2, 	"p1 left"  },
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3, 	"p1 right" },
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 start",    BIT_DIGITAL,	DrvJoy1 + 6,	"p1 start" },
	{"P1 Coin",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 coin"  },

	{"Reset",	BIT_DIGITAL,	&DrvReset,	"reset"    },
};

STDINPUTINFO(Drv)

UINT8 __fastcall dotrikun_in_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return DrvInputs[0];
	}

	return 0;
}

void __fastcall dotrikun_out_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			*nColor = data;
		return;
	}
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM	= Next; Next += 0x010000;

	DrvPalette	= (UINT32*)Next; Next += 0x000002 * sizeof (UINT32);

	AllRam		= Next;

	DrvZ80RAM	= Next; Next += 0x000800;

	nColor		= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
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
		if (BurnLoadRom(DrvZ80ROM, 0, 1)) return 1;
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea (0x0000, 0x3fff, 0, DrvZ80ROM);
	ZetMapArea (0x0000, 0x3fff, 2, DrvZ80ROM);
	ZetMapArea (0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea (0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea (0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetOutHandler(dotrikun_out_port);
	ZetSetInHandler(dotrikun_in_port);
	ZetMemEnd();
	ZetClose();

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();

	BurnFree (AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPalette[0] = BurnHighCol((*nColor & 0x08) ? 0xff : 0, (*nColor & 0x10) ? 0xff : 0, (*nColor & 0x20) ? 0xff : 0, 0);
		DrvPalette[1] = BurnHighCol((*nColor & 0x01) ? 0xff : 0, (*nColor & 0x02) ? 0xff : 0, (*nColor & 0x04) ? 0xff : 0, 0);
	}

	for (INT32 offs = 0; offs < 0x0600; offs++)
	{
		INT32 sx = (offs & 0x0f) << 4;
		INT32 sy = (offs >> 4) << 1;
		INT32 px = DrvZ80RAM[offs];

		for (INT32 i = 0; i < 8; i++, sx+=2)
		{
			INT32 pen = (px >> (7 - i)) & 1;

			if (sx > nScreenWidth || sy >= nScreenHeight) continue;

			pTransDraw[((sy + 0) << 8) | (sx + 0)] = pen;
			pTransDraw[((sy + 0) << 8) | (sx + 1)] = pen;
			pTransDraw[((sy + 1) << 8) | (sx + 0)] = pen;
			pTransDraw[((sy + 1) << 8) | (sx + 1)] = pen;
		}
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = 0xff;
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		}
	}

	ZetOpen(0);
	ZetRun(4000000 / 60);
	ZetRaiseIrq(0);
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
	}

	return 0;
}


// Dottori Kun (new version)

static struct BurnRomInfo dotrikunRomDesc[] = {
	{ "14479a.mpr",	0x4000, 0xb77a50db, BRF_ESS | BRF_PRG }, //  Z80 code
};

STD_ROM_PICK(dotrikun)
STD_ROM_FN(dotrikun)

struct BurnDriver BurnDrvdotrikun = {
	"dotrikun", NULL, NULL, NULL, "1990",
	"Dottori Kun (new version)\0", NULL, "Sega", "Test Hardware",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_SEGA_MISC, GBF_MAZE, 0,
	NULL, dotrikunRomInfo, dotrikunRomName, NULL, NULL, DrvInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x02,
	256, 192, 4, 3
};


// Dottori Kun (old version)

static struct BurnRomInfo dotriku2RomDesc[] = {
	{ "14479.mpr",	0x4000, 0xa6aa7fa5, BRF_ESS | BRF_PRG }, //  Z80 code
};

STD_ROM_PICK(dotriku2)
STD_ROM_FN(dotriku2)

struct BurnDriver BurnDrvdotriku2 = {
	"dotrikun2", "dotrikun", NULL, NULL, "1990",
	"Dottori Kun (old version)\0", NULL, "Sega", "Test Hardware",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 1, HARDWARE_SEGA_MISC, GBF_MAZE, 0,
	NULL, dotriku2RomInfo, dotriku2RomName, NULL, NULL, DrvInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x02,
	256, 192, 4, 3
};
