// FB Alpha Pac-Man-based S2650 game hardware driver module
// Based on MAME driver by Nicola Salmoria and many others

#include "tiles_generic.h"
#include "s2650_intf.h"
#include "sn76496.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvPrgROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvPrgRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvScrRAM;
static UINT8 *DrvColRAM;
static UINT8 *DrvSprRAM0;
static UINT8 *DrvSprRAM1;
static UINT8 *DrvSprRAM2;
static UINT8 *flipscreen;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8  DrvJoy1[8];
static UINT8  DrvJoy2[8];
static UINT8  DrvDips[1];
static UINT8  DrvInputs[2];
static UINT8  DrvReset;

static INT32 s2650_bank;
static INT32 watchdog;
static INT32 vblank;

static struct BurnInputInfo DrivfrcpInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Drivfrcp)

static struct BurnInputInfo _8bpmInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(_8bpm)

static struct BurnInputInfo PorkyInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 start"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Porky)

static struct BurnDIPInfo DrivfrcpDIPList[]=
{
	{0x07, 0xff, 0xff, 0x00, NULL		},
};

STDDIPINFO(Drivfrcp)

static struct BurnDIPInfo _8bpmDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x00, NULL		},

	{0   , 0xfe, 0   ,    2, "Cabinet"	},
	{0x0f, 0x01, 0x01, 0x00, "Upright"	},
	{0x0f, 0x01, 0x01, 0x01, "Cocktail"	},
};

STDDIPINFO(_8bpm)

static struct BurnDIPInfo PorkyDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x00, NULL		},

	{0   , 0xfe, 0   ,    2, "Cabinet"	},
	{0x0e, 0x01, 0x01, 0x00, "Upright"	},
	{0x0e, 0x01, 0x01, 0x01, "Cocktail"	},
};

STDDIPINFO(Porky)

static inline void bankswitch(INT32 data)
{
	INT32 bank = (data & 1) ? 0x4000 : 0;

	if (s2650_bank != (data & 1)) {

		for (INT32 i = 0; i <= 0x8000; i+= 0x8000) {
			s2650MapMemory(DrvPrgROM + 0x00000 + bank, 0x0000 | i, 0x0fff | i, S2650_ROM);
			s2650MapMemory(DrvPrgROM + 0x01000 + bank, 0x2000 | i, 0x2fff | i, S2650_ROM);
			s2650MapMemory(DrvPrgROM + 0x02000 + bank, 0x4000 | i, 0x4fff | i, S2650_ROM);
			s2650MapMemory(DrvPrgROM + 0x03000 + bank, 0x6000 | i, 0x6fff | i, S2650_ROM);
		}

		s2650_bank = data & 1;
	}
}

static void s2650games_write(UINT16 address, UINT8 data)
{
	switch (address & 0x1fff)
	{
		case 0x1503:
			*flipscreen = data & 1;
		return;

		case 0x1507: // coin counter
		return;

		case 0x15c0:
			watchdog = 0;
		return;

		case 0x15c7:
			bankswitch(data);
		return;
	}

	if ((address & 0x1c00) == 0x1000) {
		DrvColRAM[address & 0x1f] = data;
		return;
	}

	if ((address & 0x1ff0) == 0x1560) {
		DrvSprRAM2[address & 0x0f] = data;
		return;
	}
}

static UINT8 s2650games_read(UINT16 address)
{
	switch (address & 0x1fff)
	{
		case 0x1500:
			return DrvInputs[0];

		case 0x1540:
			return DrvInputs[1];

		case 0x1580:
			return DrvDips[0];
	}

	return 0;
}

static void s2650games_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0x1ff)
	{
		case S2650_DATA_PORT:
			SN76496Write(0, data);
		return;
	}
}

static UINT8 s2650games_read_port(UINT16 port)
{
	switch (port & 0x1ff)
	{
		case S2650_SENSE_PORT:
			return vblank ? 0 : 0x80;

		case 0x01:
		{
			switch (s2650GetPc())
			{
				case 0x0030: // drivfrcp & _8bpm
				case 0x0034: // porky
				case 0x0291: // drivfrcp
				case 0x0466: // _8bpm
					return 1;
			}

			return 0;
		}
	}

	return 0;
}

static INT32 DrvDoReset(INT32 clear_ram)
{
	if (clear_ram) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	s2650Open(0);
	s2650_bank = -1;
	bankswitch(0);
	s2650Reset();
	s2650Close();

	watchdog = 0;

	return 0;
}

static void DrvPaletteInit()
{
	UINT32 tmp[32];

	for (INT32 i = 0; i < 32; i++)
	{
		INT32 bit0, bit1, bit2;

		bit0 = (DrvColPROM[i] >> 0) & 0x01;
		bit1 = (DrvColPROM[i] >> 1) & 0x01;
		bit2 = (DrvColPROM[i] >> 2) & 0x01;
		INT32 r = (bit0 * 33) + (bit1 * 71) + (bit2 * 151);

		bit0 = (DrvColPROM[i] >> 3) & 0x01;
		bit1 = (DrvColPROM[i] >> 4) & 0x01;
		bit2 = (DrvColPROM[i] >> 5) & 0x01;
		INT32 g = (bit0 * 33) + (bit1 * 71) + (bit2 * 151);

		bit0 = (DrvColPROM[i] >> 6) & 0x01;
		bit1 = (DrvColPROM[i] >> 7) & 0x01;
		INT32 b = (bit0 * 81) + (bit1 * 174);

		tmp[i] = BurnHighCol(r, g, b, 0);
	}

	for (INT32 i = 0; i < 128; i++)
	{
		INT32 ctabentry = DrvColPROM[i + 0x20] &= 0x0f;
		DrvPalette[i] = tmp[ctabentry];
	}
}

static void DrvGfxDecode()
{
	INT32 Planes[2]  = { 0, 4 };
	INT32 XOffs0[8]  = { 8*8, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 };
	INT32 XOffs1[16] = { 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 };
	INT32 YOffs[16]  = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc( 0x4000 );
	if (tmp)
	{
		memcpy (tmp, DrvGfxROM0, 0x4000);

		GfxDecode(0x400, 2,  8,  8, Planes, XOffs0, YOffs, 0x080, tmp, DrvGfxROM0);
		GfxDecode(0x100, 2, 16, 16, Planes, XOffs1, YOffs, 0x200, tmp, DrvGfxROM1);

		BurnFree (tmp);
	}
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvPrgROM		= Next; Next += 0x008000;

	DrvGfxROM0		= Next; Next += 0x010000;
	DrvGfxROM1		= Next; Next += 0x010000;

	DrvColPROM		= Next; Next += 0x000120;

	DrvPalette		= (UINT32*)Next; Next += 0x080 * sizeof(UINT32);

	AllRam			= Next;

	DrvPrgRAM		= Next; Next += 0x000400;
	DrvVidRAM		= Next; Next += 0x000400;
	DrvScrRAM		= Next; Next += 0x000100;
	DrvColRAM		= Next; Next += 0x000020;
	DrvSprRAM2		= Next; Next += 0x000010;
	DrvSprRAM1		= DrvScrRAM + 0x090;
	DrvSprRAM0		= DrvPrgRAM + 0x3f0;

	flipscreen		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvInit(INT32 game, INT32 swap)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvPrgROM, 0, 1)) return 1;
		memcpy (DrvPrgROM + 0x4000, DrvPrgROM, 0x4000);

		if (game) {
			if (BurnLoadRom(DrvPrgROM + 0x4000, 1, 1)) return 1;
		}

		for (INT32 i = 0; i < 0x8000; i++) {	// porky & 8bpm have data lines scrambled
			DrvPrgROM[i] = (DrvPrgROM[i] & ~(1 | (1 << swap))) | ((DrvPrgROM[i] >> swap) & 1) | ((DrvPrgROM[i] & 1) << swap);
		}

		if (BurnLoadRom(DrvGfxROM1, 1 + game, 1)) return 1;

		for (INT32 i = 0; i < 0x4000; i++) {
			DrvGfxROM0[((i & 0x2000) >> 1) | ((i & 0x1000) << 1) | (i & 0xfff)] = DrvGfxROM1[i];
		}

		if (BurnLoadRom(DrvColPROM + 0x000, 2 + game, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x020, 3 + game, 1)) return 1;

		DrvPaletteInit();
		DrvGfxDecode();
	}

	s2650Init(1);
	s2650Open(0);
	for (INT32 i = 0; i <= 0xe000; i+= 0x2000) {
		s2650MapMemory(DrvScrRAM,		0x1400 | i, 0x14ff | i, S2650_RAM);
		s2650MapMemory(DrvVidRAM,		0x1800 | i, 0x1bff | i, S2650_RAM);
		s2650MapMemory(DrvPrgRAM,		0x1c00 | i, 0x1fff | i, S2650_RAM);
	}
	s2650SetWriteHandler(s2650games_write);
	s2650SetReadHandler(s2650games_read);
	s2650SetOutHandler(s2650games_write_port);
	s2650SetInHandler(s2650games_read_port);
	s2650Close();

	SN76496Init(0, 307200, 0);
	SN76496SetRoute(0, 0.75, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 drivfrcpInit() { return DrvInit(0,0); }
static INT32 _8bpmInit()    { return DrvInit(0,6); }
static INT32 porkyInit()    { return DrvInit(1,4); }

static INT32 DrvExit()
{
	GenericTilesExit();

	s2650Exit();
	SN76496Exit();

	BurnFree (AllMem);

	return 0;
}

static void draw_layer()
{
	UINT8 *ram = DrvScrRAM + 0x00a0;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sy = (offs >> 5) << 3;
		INT32 sx = (offs & 0x1f) << 3;

		sy -= DrvScrRAM[offs & 0x1f] + 16;
		if (sy < -7) sy += 256;

		if (sy >= nScreenHeight) continue;

		INT32 code  = DrvVidRAM[offs] | ((ram[offs & 0x1f] & 0x03) << 8);
		INT32 color = DrvColRAM[offs & 0x1f] & 0x1f;

		if (*flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, 248 - sx, 216 - sy, color, 2, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 2, 0, DrvGfxROM0);
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0x0e; offs >= 0; offs -= 2)
	{
		INT32 attr  = DrvSprRAM0[offs];
		INT32 sx    = DrvSprRAM2[offs + 1] ^ 0xff;
		INT32 sy    = DrvSprRAM2[offs] - 15;
		INT32 color = DrvSprRAM0[offs + 1] & 0x1f;
		INT32 code  = (attr >> 2) | ((DrvSprRAM1[offs] & 3) << 6);
		INT32 flipx = attr & 0x01;
		INT32 flipy = attr & 0x02;

		if (offs <= 4) sy += 1; // hack

		RenderTileTranstab(pTransDraw, DrvGfxROM1, code, color << 2, 0, sx, sy - 16, flipx, flipy, 16, 16, DrvColPROM + 0x020);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	draw_layer();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset(1);
	}

	if (watchdog >= 180) {
		DrvDoReset(0);
	}
	watchdog++;

	{
		memset (DrvInputs, 0xff, 2);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	s2650Open(0);

	vblank = 0;

	for (INT32 i = 0; i < 32; i++) {
		if (i == 31) {
			vblank = 1;
			s2650_set_irq_line(0x03, 1);
		}
		INT32 nSegment = (1536000 / 60) / 32;

		s2650Run(nSegment);

		if (i == 31) {
			s2650_set_irq_line(0x03, 0);
		}
	}

	s2650Close();

	if (pBurnSoundOut) {
		SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029709;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		s2650Scan(nAction, pnMin);

		SCAN_VAR(watchdog);
		SCAN_VAR(s2650_bank);
	}

	if (nAction & ACB_WRITE) {
		s2650Open(0);
		INT32 bank = s2650_bank;
		s2650_bank = -1;
		bankswitch(bank);
		s2650Close();
	}

	return 0;
}


// Driving Force (Pac-Man conversion)

static struct BurnRomInfo drivfrcpRomDesc[] = {
	{ "drivforc.1",		0x4000, 0x10b59d27, 1 | BRF_PRG | BRF_ESS }, //  0 s2650 Code

	{ "drivforc.2",		0x4000, 0x56331cb5, 2 | BRF_GRA },           //  1 Tiles and Sprites

	{ "drivforc.pr1",	0x0020, 0x045aa47f, 3 | BRF_GRA },           //  2 Color PROMs
	{ "drivforc.pr2",	0x0100, 0x9e6d2f1d, 3 | BRF_GRA },           //  3
};

STD_ROM_PICK(drivfrcp)
STD_ROM_FN(drivfrcp)

struct BurnDriver BurnDrvDrivfrcp = {
	"drivfrcp", NULL, NULL, NULL, "1984",
	"Driving Force (Pac-Man conversion)\0", NULL, "Shinkai Inc. (Magic Eletronics Inc. license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 1, HARDWARE_PACMAN, GBF_RACING, 0,
	NULL, drivfrcpRomInfo, drivfrcpRomName, NULL, NULL, DrivfrcpInputInfo, DrivfrcpDIPInfo,
	drivfrcpInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x80,
	224, 256, 3, 4
};


// Eight Ball Action (Pac-Man conversion)

static struct BurnRomInfo _8bpmRomDesc[] = {
	{ "8bpmp.bin",		0x4000, 0xb4f7eba7, 1 | BRF_PRG | BRF_ESS }, //  0 s2650 Code

	{ "8bpmc.bin",		0x4000, 0x1c894a6d, 2 | BRF_GRA },           //  1 Tiles and Sprites

	{ "8bpm.7f",		0x0020, 0x4cf54241, 3 | BRF_GRA },           //  2 Color PROMs
	{ "8bpm.4a",		0x0100, 0x618505a0, 3 | BRF_GRA },           //  3
};

STD_ROM_PICK(_8bpm)
STD_ROM_FN(_8bpm)

struct BurnDriver BurnDrv_8bpm = {
	"8bpm", "8ballact", NULL, NULL, "1985",
	"Eight Ball Action (Pac-Man conversion)\0", NULL, "Seatongrove Ltd (Magic Eletronics USA license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_SPORTSMISC, 0,
	NULL, _8bpmRomInfo, _8bpmRomName, NULL, NULL, _8bpmInputInfo, _8bpmDIPInfo,
	_8bpmInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x80,
	224, 256, 3, 4
};


// Porky

static struct BurnRomInfo porkyRomDesc[] = {
	{ "pp",			0x4000, 0x00592624, 1 | BRF_PRG | BRF_ESS }, //  0 s2650 Code
	{ "ps",			0x4000, 0x2efb9861, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "pc",			0x4000, 0xa20e3d39, 2 | BRF_GRA },           //  2 Tiles and Sprites

	{ "7f",			0x0020, 0x98bce7cc, 3 | BRF_GRA },           //  3 Color PROMs
	{ "4a",			0x0100, 0x30fe0266, 3 | BRF_GRA },           //  4
};

STD_ROM_PICK(porky)
STD_ROM_FN(porky)

struct BurnDriver BurnDrvPorky = {
	"porky", NULL, NULL, NULL, "1985",
	"Porky\0", NULL, "Shinkai Inc. (Magic Eletronics Inc. license)", "Pac-man",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PACMAN, GBF_MISC, 0,
	NULL, porkyRomInfo, porkyRomName, NULL, NULL, PorkyInputInfo, PorkyDIPInfo,
	porkyInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x80,
	224, 256, 3, 4
};
