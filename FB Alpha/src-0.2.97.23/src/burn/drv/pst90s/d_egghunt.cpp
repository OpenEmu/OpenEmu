// FB Alpha Egg Hunt driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "zet.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM0;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvVidRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvColRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM1;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;

static UINT8 ram_bank;
static UINT8 gfx_bank;
static UINT8 oki_bank;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo EgghuntInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 7,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Egghunt)

static struct BurnDIPInfo EgghuntDIPList[]=
{
	{0x10, 0xff, 0xff, 0x61, NULL			},
	{0x11, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    2, "Debug Mode"		},
	{0x10, 0x01, 0x01, 0x01, "Off"			},
	{0x10, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    3, "Credits per Player"	},
	{0x10, 0x01, 0x60, 0x60, "1"			},
	{0x10, 0x01, 0x60, 0x40, "2"			},
	{0x10, 0x01, 0x60, 0x00, "3"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x10, 0x01, 0x80, 0x80, "Off"			},
	{0x10, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Censor Pictures"	},
	{0x11, 0x01, 0x80, 0x00, "No"			},
	{0x11, 0x01, 0x80, 0x80, "Yes"			},
};

STDDIPINFO(Egghunt)

static inline void palette_write(INT32 offset)
{
	INT32 d = (DrvPalRAM[offset + 1] << 8) | (DrvPalRAM[offset + 0]);

	INT32 r = (d >> 10) & 0x1f;
	INT32 g = (d >>  5) & 0x1f;
	INT32 b = (d >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

void __fastcall egghunt_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf800) == 0xc000) {
		DrvPalRAM[address & 0x7ff] = data;
		palette_write(address & 0x7fe);
		return;
	}
}

static void set_ram_bank(INT32 data)
{
	ram_bank = data & 1;

	if (ram_bank) {
		ZetMapArea(0xd000, 0xdfff, 0, DrvSprRAM);
		ZetMapArea(0xd000, 0xdfff, 1, DrvSprRAM);
		ZetMapArea(0xd000, 0xdfff, 2, DrvSprRAM);
	} else {
		ZetMapArea(0xd000, 0xdfff, 0, DrvVidRAM);
		ZetMapArea(0xd000, 0xdfff, 1, DrvVidRAM);
		ZetMapArea(0xd000, 0xdfff, 2, DrvVidRAM);
	}
}

void __fastcall egghunt_main_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			set_ram_bank(data);
		return;

		case 0x01:
			gfx_bank = data & 0x33;
		return;

		case 0x03:
			ZetClose();
			ZetOpen(1);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			ZetOpen(0);
			*soundlatch = data;
		return;
	}
}

UINT8 __fastcall egghunt_main_read_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return DrvDips[0];

		case 0x01:
			return DrvInputs[0];

		case 0x02:
			return DrvInputs[1];

		case 0x03:
			return DrvInputs[2];

		case 0x04:
			return DrvDips[1];

		case 0x06:
			return 0xff;
	}

	return 0;
}

static void set_oki_bank(INT32 data)
{
	MSM6295ROM = DrvSndROM0 + ((data & 0x10) >> 4) * 0x40000;

	for (INT32 nChannel = 0; nChannel < 4; nChannel++) {
		MSM6295SampleInfo[0][nChannel] = MSM6295ROM + (nChannel << 8);
		MSM6295SampleData[0][nChannel] = MSM6295ROM + (nChannel << 16);
	}
}

void __fastcall egghunt_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe001:
			oki_bank = data;
			set_oki_bank(data);
		return;

		case 0xe004:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall egghunt_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xe000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;

		case 0xe001:
			return oki_bank;

		case 0xe004:
			return MSM6295ReadStatus(0);
	}

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	set_oki_bank(0);
	MSM6295Reset(0);

	oki_bank = 0;
	ram_bank = 0;
	gfx_bank = 0;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4]  = { 0x400000, 0x600000, 0x000000, 0x200000 };
	INT32 XOffs0[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
	INT32 XOffs1[16]= { 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x84, 0x85, 0x86, 0x87, 0x80, 0x81, 0x82, 0x83 };
	INT32 YOffs[16] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < 0x100000; i++) tmp[i] = DrvGfxROM0[i] ^ 0xff;

	GfxDecode(0x08000, 4,  8,  8, Plane, XOffs0, YOffs, 0x040, tmp, DrvGfxROM0);

	for (INT32 i = 0; i < 0x100000; i++) tmp[i] = DrvGfxROM1[i] ^ 0xff;

	GfxDecode(0x01000, 4, 16, 16, Plane, XOffs1, YOffs, 0x100, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x020000;
	DrvZ80ROM1	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x200000;
	DrvGfxROM1	= Next; Next += 0x100000;

	DrvSndROM0	= Next; Next += 0x080000;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x002000;
	DrvVidRAM	= Next; Next += 0x001000;
	DrvSprRAM	= Next; Next += 0x001000;
	DrvColRAM	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x000800;

	DrvZ80RAM1	= Next; Next += 0x001000;

	soundlatch	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvInit()
{
	BurnSetRefreshRate(58.0);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvZ80ROM0 + 0x000000,  0, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x000000,  1, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x040000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x080000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x0c0000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0000,  9, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 10, 1)) return 1;

		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvPalRAM);
//	ZetMapArea(0xc000, 0xc7ff, 1, DrvPalRAM);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvPalRAM);
	ZetMapArea(0xc800, 0xcfff, 0, DrvColRAM);
	ZetMapArea(0xc800, 0xcfff, 1, DrvColRAM);
	ZetMapArea(0xc800, 0xcfff, 2, DrvColRAM);
	ZetMapArea(0xd000, 0xdfff, 0, DrvVidRAM);
	ZetMapArea(0xd000, 0xdfff, 1, DrvVidRAM);
	ZetMapArea(0xd000, 0xdfff, 2, DrvVidRAM);
	ZetMapArea(0xe000, 0xffff, 0, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xffff, 1, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xffff, 2, DrvZ80RAM0);
	ZetSetWriteHandler(egghunt_main_write);
	ZetSetOutHandler(egghunt_main_write_port);
	ZetSetInHandler(egghunt_main_read_port);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0xf000, 0xffff, 0, DrvZ80RAM1);
	ZetMapArea(0xf000, 0xffff, 1, DrvZ80RAM1);
	ZetMapArea(0xf000, 0xffff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(egghunt_sound_write);
	ZetSetReadHandler(egghunt_sound_read);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1056000 / 132, 100.0, 0);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	MSM6295Exit(0);
	MSM6295ROM = NULL;

	ZetExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites()
{
	for (INT32 offs = 0x1000 - 0x40; offs >= 0; offs -= 0x20)
	{
		INT32 attr  = DrvSprRAM[offs + 1];
		INT32 code  = DrvSprRAM[offs + 0] | ((attr & 0xe0) << 3);
		INT32 sx    = DrvSprRAM[offs + 3] | ((attr & 0x10) << 4);
		INT32 sy    = ((DrvSprRAM[offs + 2] + 8) & 0xff) - 8;

		if (attr & 0xe0) {
			if (gfx_bank & 0x20) {
				code += 0x400 << (gfx_bank & 1);
			}
		}

		Render16x16Tile_Mask_Clip(pTransDraw, code, sx - 64, sy - 8, attr & 0x0f, 4, 15, 0, DrvGfxROM1);
	}
}

static void draw_background_layer()
{
	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 sx = ((offs & 0x3f) << 3) - 64;
		INT32 sy = ((offs >> 6)   << 3) - 8;

		if (sy < 0 || sy >= nScreenHeight || sx < 0 || sx >= nScreenWidth) continue;

		INT32 code  = ((DrvVidRAM[offs * 2 + 1] & 0x3f) << 8) | (DrvVidRAM[offs * 2 + 0]);
		INT32 color = DrvColRAM[offs] & 0x3f;

		if (code & 0x2000) {
			if (gfx_bank & 0x02) {
				code += 0x2000 << (gfx_bank & 1);
			}
		}

		Render8x8Tile(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x800; i+=2) {
			palette_write(i);
		}
		DrvRecalc = 0;
	}

	draw_background_layer();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { 6000000 / 60, 6000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = nCyclesTotal[0] / nInterleave;

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == (nInterleave - 1)) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();

		ZetOpen(1);
		nCyclesDone[1] += ZetRun(nSegment);
		ZetClose();
	}

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);

		MSM6295Scan(0, nAction);

		SCAN_VAR(oki_bank);
		SCAN_VAR(gfx_bank);
		SCAN_VAR(ram_bank);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		set_ram_bank(ram_bank);
		ZetClose();

		set_oki_bank(oki_bank);
	}

	return 0;
}


// Egg Hunt

static struct BurnRomInfo egghuntRomDesc[] = {
	{ "prg.bin",	0x20000, 0xeb647145, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code

	{ "rom2.bin",	0x10000, 0x88a71bc3, 2 | BRF_ESS | BRF_PRG }, //  1 Z80 #1 Code

	{ "rom3.bin",	0x40000, 0x9d51ac49, 3 | BRF_GRA },           //  2 Tiles
	{ "rom4.bin",	0x40000, 0x41c63041, 3 | BRF_GRA },           //  3
	{ "rom5.bin",	0x40000, 0x6f96cb97, 3 | BRF_GRA },           //  4
	{ "rom6.bin",	0x40000, 0xb5a41d4b, 3 | BRF_GRA },           //  5

	{ "rom7.bin",	0x20000, 0x1b43fb57, 4 | BRF_GRA },           //  6 Sprites
	{ "rom8.bin",	0x20000, 0xf8122d0d, 4 | BRF_GRA },           //  7
	{ "rom9.bin",	0x20000, 0xdbfa0ffe, 4 | BRF_GRA },           //  8
	{ "rom10.bin",	0x20000, 0x14f5fc74, 4 | BRF_GRA },           //  9

	{ "rom1.bin",	0x80000, 0xf03589bc, 5 | BRF_SND },           // 10 Samples
};

STD_ROM_PICK(egghunt)
STD_ROM_FN(egghunt)

struct BurnDriver BurnDrvEgghunt = {
	"egghunt", NULL, NULL, NULL, "1995",
	"Egg Hunt\0", NULL, "Invi Image", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, egghuntRomInfo, egghuntRomName, NULL, NULL, EgghuntInputInfo, EgghuntDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	384, 240, 4, 3
};
