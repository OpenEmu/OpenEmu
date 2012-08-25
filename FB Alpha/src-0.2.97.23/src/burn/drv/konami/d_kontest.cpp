// FB Alpha Konami Test Board (GX800) driver module
// Based on MAME driver by Angelo Salese

#include "tiles_generic.h"
#include "z80_intf.h"
#include "sn76496.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvColPROM;
static UINT8 *DrvColRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvTileRAM;
static UINT8 *DrvTileRAMExp;

static UINT8 *irq_enable;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvInputs[4];
static UINT8 DrvDips[1];
static UINT8 DrvReset;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 fire 3"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy4 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy4 + 0,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy4 + 2,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service 1",		BIT_DIGITAL,	DrvJoy3 + 6,	"service"	},
	{"Service 2",		BIT_DIGITAL,	DrvJoy3 + 7,    "service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	{0x14, 0xff, 0xff, 0x80, NULL		},

	{0   , 0xfe, 0   ,    2, "Orientation"	},
	{0x14, 0x01, 0x80, 0x80, "Horizontal"	},
	{0x14, 0x01, 0x80, 0x00, "Vertical"	},
};

STDDIPINFO(Drv)

static inline void DrvExpandTile(INT32 offset, UINT8 data)
{
	DrvTileRAMExp[offset * 4 + 0] = ((data >> 3) & 0x01) | ((data >> 6) & 0x02);
	DrvTileRAMExp[offset * 4 + 1] = ((data >> 2) & 0x01) | ((data >> 5) & 0x02);
	DrvTileRAMExp[offset * 4 + 2] = ((data >> 1) & 0x01) | ((data >> 4) & 0x02);
	DrvTileRAMExp[offset * 4 + 3] = ((data >> 0) & 0x01) | ((data >> 3) & 0x02);
}

static void __fastcall kontest_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf000) == 0xf000) {
		DrvTileRAM[address & 0x0fff] = data;
		DrvExpandTile(address & 0x0fff, data);
		return;
	}
}

static void __fastcall kontest_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			SN76496Write(0, data);
		return;

		case 0x04:
			SN76496Write(1, data);
		return;

		case 0x08:
			*irq_enable = data & 0x08;
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		return;
	}
}

static UINT8 __fastcall kontest_read_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			return DrvInputs[port & 0x03];
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM		= Next; Next += 0x008000;

	DrvColPROM		= Next; Next += 0x000020;

	DrvPalette		= (UINT32*)Next; Next += 0x0020 * sizeof(UINT32);

	DrvTileRAMExp		= Next; Next += 0x004000;

	AllRam			= Next;

	DrvColRAM		= Next; Next += 0x000800;
	DrvVidRAM		= Next; Next += 0x000800;
	DrvTileRAM		= Next; Next += 0x001000;

	irq_enable		= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x20; i++)
	{
		INT32 bit0 = 0;
		INT32 bit1 = (DrvColPROM[i] >> 6) & 0x01;
		INT32 bit2 = (DrvColPROM[i] >> 7) & 0x01;
		INT32 b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (DrvColPROM[i] >> 3) & 0x01;
		bit1 = (DrvColPROM[i] >> 4) & 0x01;
		bit2 = (DrvColPROM[i] >> 5) & 0x01;
		INT32 g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (DrvColPROM[i] >> 0) & 0x01;
		bit1 = (DrvColPROM[i] >> 1) & 0x01;
		bit2 = (DrvColPROM[i] >> 2) & 0x01;
		INT32 r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

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
		if (BurnLoadRom(DrvZ80ROM , 0, 1)) return 1;

		if (BurnLoadRom(DrvColPROM, 1, 1)) return 1;

		DrvPaletteInit();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvColRAM);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvColRAM);
	ZetMapArea(0xe000, 0xe7ff, 2, DrvColRAM);
	ZetMapArea(0xe800, 0xefff, 0, DrvVidRAM);
	ZetMapArea(0xe800, 0xefff, 1, DrvVidRAM);
	ZetMapArea(0xe800, 0xefff, 2, DrvVidRAM);
	ZetMapArea(0xf000, 0xffff, 0, DrvTileRAM);
//	ZetMapArea(0xf000, 0xffff, 1, DrvTileRAM);
	ZetMapArea(0xf000, 0xffff, 2, DrvTileRAM);
	ZetSetWriteHandler(kontest_write);
	ZetSetOutHandler(kontest_write_port);
	ZetSetInHandler(kontest_read_port);
	ZetMemEnd();
	ZetClose();

	SN76489AInit(0, 1500000, 0);
	SN76489AInit(1, 1500000, 1);

	SN76496SetRoute(0, 0.60, BURN_SND_ROUTE_RIGHT);
	SN76496SetRoute(1, 0.50, BURN_SND_ROUTE_LEFT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	SN76496Exit();

	BurnFree(AllMem);

	return 0;
}

static void DrawLayer()
{
	for (INT32 offs = 0; offs < 32 * 64; offs++)
	{
		INT32 sx    = (offs & 0x3f) * 8;
		INT32 sy    = (offs / 0x40) * 8;

		INT32 code  = DrvVidRAM[offs];
		INT32 color = DrvColRAM[(offs & 0x3f) + ((offs & 0x780) / 2)] & 0x07;

		if (sx < 256 || sy < 16 || sy >= 240) continue;

		Render8x8Tile(pTransDraw, code, sx - 256, sy - 16, color, 2, 0, DrvTileRAMExp);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	DrawLayer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 4 * sizeof(UINT8));

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
		}

		DrvInputs[2] = (DrvInputs[2] & 0x7f) | (DrvDips[0] & 0x80);
	}

	ZetOpen(0);
	ZetRun(3000000 / 60);

	if (*irq_enable)
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);

	ZetClose();

	if (pBurnSoundOut) {
		SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
		SN76496Update(1, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
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

		ZetScan(nAction);
		SN76496Scan(nAction, pnMin);
	}

	if (nAction & ACB_WRITE) {
		for (INT32 i = 0; i < 0x1000; i++) {
			DrvExpandTile(i, DrvTileRAM[i]);
		}
	}

	return 0;
}


// Konami Test Board (GX800, Japan)

static struct BurnRomInfo kontestRomDesc[] = {
	{ "800b01.10d",	0x8000, 0x520f83dc, 1 | BRF_PRG | BRF_ESS },	//  0 Z80 Code

	{ "800a02.4f",	0x0020, 0x6d604171, 2 | BRF_GRA },		//  1 Color PROM
};

STD_ROM_PICK(kontest)
STD_ROM_FN(kontest)

struct BurnDriver BurnDrvKontest = {
	"kontest", NULL, NULL, NULL, "1987?",
	"Konami Test Board (GX800, Japan)\0", NULL, "Konami", "GX800",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_MISC, 0,
	NULL, kontestRomInfo, kontestRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	256, 224, 4, 3
};
