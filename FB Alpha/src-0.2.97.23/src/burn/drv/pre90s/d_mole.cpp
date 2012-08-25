// Mole Attack FB Driver Module
// Based on MAME driver by Jason Nelson and Phil Stroffolino

#include "burnint.h"
#include "m6502_intf.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}


//--------------------------------------------------------------------------------------
// Variables


static UINT8 *Mem, *Rom, *Gfx, *BankRam;
static UINT8 DrvJoy1[12], DrvJoy2[12], DrvReset, DrvDips;
static INT32 *Palette;
static INT16 *pAY8910Buffer[3], *pFMBuffer = NULL;

static INT32 tile_bank, flipscreen;


//--------------------------------------------------------------------------------------
// Inputs

//  buttons are laid out as follows:
//  7   8   9
//  4   5   6
//  1   2   3


static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"     ,   BIT_DIGITAL,   DrvJoy1 + 9,	  "p1 coin"  },
	{"Start 1"    ,   BIT_DIGITAL,   DrvJoy1 + 10,	"p1 start" },
	{"Start 2"    ,   BIT_DIGITAL,   DrvJoy2 + 10,	"p2 start" },
	{"P1 button 1",	  BIT_DIGITAL,   DrvJoy1 + 0,   "p1 fire 1", },
	{"P1 button 2",	  BIT_DIGITAL,   DrvJoy1 + 1,   "p1 fire 2", },
	{"P1 button 3",	  BIT_DIGITAL,   DrvJoy1 + 2,   "p1 fire 3", },
	{"P1 button 4",	  BIT_DIGITAL,   DrvJoy1 + 3,   "p1 fire 4", },
	{"P1 button 5",	  BIT_DIGITAL,   DrvJoy1 + 4,   "p1 fire 5", },
	{"P1 button 6",	  BIT_DIGITAL,   DrvJoy1 + 5,   "p1 fire 6", },
	{"P1 button 7",	  BIT_DIGITAL,   DrvJoy1 + 6,   "p1 fire 7", },
	{"P1 button 8",	  BIT_DIGITAL,   DrvJoy1 + 7,   "p1 fire 8", },
	{"P1 button 9",	  BIT_DIGITAL,   DrvJoy1 + 8,   "p1 fire 9", },

	{"P2 button 1",	  BIT_DIGITAL,   DrvJoy2 + 0,   "p2 fire 1", },
	{"P2 button 2",	  BIT_DIGITAL,   DrvJoy2 + 1,   "p2 fire 2", },
	{"P2 button 3",	  BIT_DIGITAL,   DrvJoy2 + 2,   "p2 fire 3", },
	{"P2 button 4",	  BIT_DIGITAL,   DrvJoy2 + 3,   "p2 fire 4", },
	{"P2 button 5",	  BIT_DIGITAL,   DrvJoy2 + 4,   "p2 fire 5", },
	{"P2 button 6",	  BIT_DIGITAL,   DrvJoy2 + 5,   "p2 fire 6", },
	{"P2 button 7",	  BIT_DIGITAL,   DrvJoy2 + 6,   "p2 fire 7", },
	{"P2 button 8",	  BIT_DIGITAL,   DrvJoy2 + 7,   "p2 fire 8", },
	{"P2 button 9",	  BIT_DIGITAL,   DrvJoy2 + 8,   "p2 fire 9", },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, &DrvDips,	"dip"	   },
};

STDINPUTINFO(Drv)


static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x16, 0xff, 0xff, 0x00, NULL },

	{0   , 0xfe, 0   , 2   , "Passing Points" },
	{0x16, 0x01, 0x01, 0x00, "300" },
	{0x16, 0x01, 0x01, 0x01, "400" },

	{0   , 0xfe, 0   , 2   , "Coinage" },
	{0x16, 0x01, 0x02, 0x00, "1 Coin 1 Play"  },
	{0x16, 0x01, 0x02, 0x02, "1 Coin 2 Plays" },

	{0   , 0xfe, 0   , 2   , "Cabinet" },
	{0x16, 0x01, 0x10, 0x00, "Upright"  },
	{0x16, 0x01, 0x10, 0x10, "Cocktail" },
};

STDDIPINFO(Drv)


//--------------------------------------------------------------------------------------
// Memory handling


static UINT8 mole_protection_r(UINT8 offset)
{
	switch (offset)
	{
		case 0x08:			// random mole placement
			return 0xb0;

		case 0x26:
			if (M6502GetPC() == 0x53d7)
			{
				return 0x06;	// bonus round
			}
			else			// pc == 0x515b, 0x5162
			{ 
				return 0xc6;	// game start
			}

		case 0x86:			// game over
			return 0x91;

		case 0xae:			// coinage
			return 0x32;
	}

	return 0x00;
}

void mole_write_byte(UINT16 address, UINT8 data)
{
	// Tile RAM
	if (address >= 0x8000 && address <= 0x83ff)
	{
		BankRam[address & 0x3ff] = tile_bank;
		Rom[address] = data;
		return;
	}

	switch (address)
	{
		case 0x0800: // ?
		case 0x0820:
		case 0x8c40:
		case 0x8c80:
		case 0x8c81:
		break;

		case 0x8400:
			tile_bank = data;
		break;

		case 0x8c00: // ay8910_write_port
		case 0x8c01: // ay8910_control_port
			AY8910Write(0, ~address & 1, data);
		break;

		case 0x8d00: // watchdog
		break;

		case 0x8dc0: // flipscreen
			flipscreen = data & 1;
		break;
	}
	
	if (address <= 0x3ff) {
		Rom[address] = data;
	}
}

UINT8 mole_read_byte(UINT16 address)
{
	UINT8 ret = 0;

	switch (address)
	{
		case 0x8d00: // input port 0
			return DrvDips & 0x03;

		case 0x8d40: // input port 1
		{
			for (INT32 i = 0; i < 8; i++)
				ret |= DrvJoy1[i] << i;

			return ret;
		}

		case 0x8d80: // input port 2
		{
			ret |= DrvJoy1[8];
			ret |= DrvJoy2[0] << 1;
			ret |= DrvJoy2[1] << 2;
			ret |= DrvJoy2[2] << 3;
			ret |= DrvDips & 0x10;
			ret |= DrvJoy2[10] << 5;
			ret |= DrvJoy1[10] << 6;
			ret |= DrvJoy1[ 9] << 7;

			return ret;
		}

		case 0x8dc0: // input port 3
		{
			ret |= DrvJoy2[7];
			ret |= DrvJoy2[6] << 1;
			ret |= DrvJoy2[3] << 2;
			ret |= DrvJoy2[8] << 3;
			ret |= DrvJoy2[5] << 4;
			ret |= DrvJoy2[4] << 5;

			return ret;
		}
	}

	// Protection read
	if (address >= 0x800 && address <= 0x8ff)
	{
		return mole_protection_r(address & 0xff);
	}
	
	if (address <= 0x3ff) {
		return Rom[address];
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// Initilizing functions


static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (Rom + 0x0000, 0, 0x0400);
	memset (Rom + 0x8000, 0, 0x0400);
	memset (BankRam,      0, 0x0400);

	tile_bank = 0;
	flipscreen = 0;

	M6502Open(0);
	M6502Reset();
	M6502Close();

	AY8910Reset(0);

	return 0;
}

static void mole_palette_init()
{
	for (INT32 i = 0; i < 8; i++) {
		Palette[i] |= (i & 1) ? 0xff0000 : 0;
		Palette[i] |= (i & 4) ? 0x00ff00 : 0;
		Palette[i] |= (i & 2) ? 0x0000ff : 0;
	}
	
}

static INT32 mole_gfx_convert()
{
	UINT8 a, b, c;
	UINT8 *tmp = (UINT8*)BurnMalloc(0x6000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, Gfx, 0x6000);

	for (INT32 i = 0; i < 0x8000; i++)
	{
		a = (tmp[0x0000 + (i >> 3)] >> (i & 7)) & 1;
		b = (tmp[0x1000 + (i >> 3)] >> (i & 7)) & 1;
		c = (tmp[0x2000 + (i >> 3)] >> (i & 7)) & 1;

		Gfx[0x0007 ^ i] = (a << 2) | (b << 1) | c;

		a = (tmp[0x3000 + (i >> 3)] >> (i & 7)) & 1;
		b = (tmp[0x4000 + (i >> 3)] >> (i & 7)) & 1;
		c = (tmp[0x5000 + (i >> 3)] >> (i & 7)) & 1;

		Gfx[0x8007 ^ i] = (a << 2) | (b << 1) | c;
	}

	BurnFree (tmp);
	
	return 0;
}

static INT32 DrvInit()
{
	Mem = (UINT8*)BurnMalloc(0x10000 + 0x10000 + 0x400 + (0x20 * sizeof(INT32)));
	if (Mem == NULL) {
		return 1;
	}

	pFMBuffer = (INT16 *)BurnMalloc(nBurnSoundLen * 3 * sizeof(INT16));
	if (pFMBuffer == NULL) {
		return 1;
	}

	memset (Mem, 0, 0x20420);

	Rom = Mem + 0x00000;
	Gfx = Mem + 0x10000;
	BankRam = Mem + 0x20000;
	Palette = (INT32 *)(Mem + 0x20400);

	{
		BurnLoadRom(Rom + 0x5000, 0, 1);
		BurnLoadRom(Rom + 0x6000, 1, 1);
		BurnLoadRom(Rom + 0x7000, 2, 1);

		BurnLoadRom(Gfx + 0x0000, 3, 1);
		BurnLoadRom(Gfx + 0x1000, 4, 1);
		BurnLoadRom(Gfx + 0x2000, 5, 1);
		BurnLoadRom(Gfx + 0x3000, 6, 1);
		BurnLoadRom(Gfx + 0x4000, 7, 1);
		BurnLoadRom(Gfx + 0x5000, 8, 1);
	}

	mole_gfx_convert();
	mole_palette_init();

	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(Rom + 0x0000, 0x0000, 0x03ff, M6502_RAM); // Rom
	M6502MapMemory(Rom + 0x5000, 0x5000, 0x7fff, M6502_ROM); // Rom
	M6502MapMemory(Rom + 0x5000, 0xd000, 0xffff, M6502_ROM); // Rom Mirror
	M6502SetReadHandler(mole_read_byte);
	M6502SetWriteHandler(mole_write_byte);
	M6502SetReadMemIndexHandler(mole_read_byte);
	M6502SetWriteMemIndexHandler(mole_write_byte);
	M6502SetReadOpHandler(mole_read_byte);
	M6502SetReadOpArgHandler(mole_read_byte);
	M6502Close();

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;

	AY8910Init(0, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910SetAllRoutes(0, 1.00, BURN_SND_ROUTE_BOTH);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	M6502Exit();
	AY8910Exit(0);

	BurnFree (Mem);
	BurnFree (pFMBuffer);

	return 0;
}


//--------------------------------------------------------------------------------------
// Drawing functions


static INT32 DrvDraw()
{
	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sy = ((offs / 40) % 25) << 3;
		INT32 sx = (offs % 40) << 3;

		INT32 code = ((BankRam[offs] & 3) << 14) | ((Rom[0x8000 + offs]) << 6);

		UINT8 *gfxsrc = Gfx + code;

		for (INT32 y = sy; y < sy + 8; y++)
		{
			for (INT32 x = sx; x < sx + 8; x++, gfxsrc++)
			{
				INT32 pxl = Palette[*gfxsrc];

				INT32 pos;

				if (flipscreen)
					pos = (199 - y) * 320 + (319 - x);
				else
					pos = y * 320 + x;

				PutPix(pBurnDraw + pos * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
			}
		}
	}

	return 0;
}


static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	M6502Open(0);
	M6502Run(4000000 / 60);
	M6502SetIRQLine(M6502_IRQ_LINE, M6502_IRQSTATUS_AUTO);
	M6502Close();

	if (pBurnSoundOut) {
		AY8910Render(&pAY8910Buffer[0], pBurnSoundOut, nBurnSoundLen, 0);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// Savestates


static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = Rom + 0x0000;
		ba.nLen	  = 0x0400;
		ba.szName = "Work Ram";
		BurnAcb(&ba);

		ba.Data	  = Rom + 0x8000;
		ba.nLen	  = 0x0400;
		ba.szName = "Video Ram";
		BurnAcb(&ba);

		ba.Data	  = BankRam;
		ba.nLen	  = 0x0400;
		ba.szName = "Bank Ram";
		BurnAcb(&ba);

		M6502Scan(nAction);			// Scan m6502
		AY8910Scan(nAction, pnMin);		// Scan AY8910

		// Scan critical driver variables
		SCAN_VAR(tile_bank);
		SCAN_VAR(flipscreen);
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// Game drivers


// Mole Attack

static struct BurnRomInfo moleRomDesc[] = {
	{ "m3a.5h",	0x1000, 0x5fbbdfef, 1 | BRF_ESS | BRF_PRG }, //  0 M6502 Code
	{ "m2a.7h",	0x1000, 0xf2a90642, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "m1a.8h",	0x1000, 0xcff0119a, 1 | BRF_ESS | BRF_PRG }, //  2

	{ "mea.4a",	0x1000, 0x49d89116, 2 | BRF_GRA },	     //  3 Graphics tiles
	{ "mca.6a",	0x1000, 0x04e90300, 2 | BRF_GRA },	     //  4
	{ "maa.9a",	0x1000, 0x6ce9442b, 2 | BRF_GRA },	     //  5
	{ "mfa.3a",	0x1000, 0x0d0c7d13, 2 | BRF_GRA },	     //  6
	{ "mda.5a",	0x1000, 0x41ae1842, 2 | BRF_GRA },	     //  7
	{ "mba.8a",	0x1000, 0x50c43fc9, 2 | BRF_GRA },	     //  8
};

STD_ROM_PICK(mole)
STD_ROM_FN(mole)

struct BurnDriver BurnDrvMole = {
	"mole", NULL, NULL, NULL, "1982",
	"Mole Attack\0", NULL, "Yachiyo Electronics, Ltd.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, moleRomInfo, moleRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x08,
	320, 200, 4, 3
};
