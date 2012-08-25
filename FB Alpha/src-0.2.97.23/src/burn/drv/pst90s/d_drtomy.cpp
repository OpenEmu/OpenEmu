#include "tiles_generic.h"
#include "m68000_intf.h"
#include "msm6295.h"

static UINT8 *Mem, *MemEnd, *RamStart, *RamEnd;
static UINT8 *Rom, *Gfx0, *Gfx1, *Snd;
static UINT8 *VidRam, *PalRam, *SprRam, *WorkRam;
static UINT32 *Palette, *DrvPalette;
static UINT8 DrvRecalc;
static UINT8 DrvJoy1[8], DrvJoy2[8], DrvDips[2], DrvInputs[2], DrvReset;

static INT32 m6295bank;

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy1 + 7,	"p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy2 + 6,	"p1 start" },
	{"P1 Up"        ,	BIT_DIGITAL  , DrvJoy1 + 0, "p1 up"    },
	{"P1 Down"      ,	BIT_DIGITAL  , DrvJoy1 + 1, "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 3, "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 2, "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 5,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 2"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 7,	"p2 start" },
	{"P2 Up"        ,	BIT_DIGITAL  , DrvJoy2 + 0, "p2 up"    },
	{"P2 Down"      ,	BIT_DIGITAL  , DrvJoy2 + 1, "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 3, "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 2, "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 2"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL },
	{0x12, 0xff, 0xff, 0xaf, NULL },

	{0   , 0xfe, 0   , 11  , "Coin A" },
	{0x11, 0x01, 0x0f, 0x0a, "2 Coins 1 Credit"	 },
	{0x11, 0x01, 0x0f, 0x07, "3 Coins 2 Credits" },
	{0x11, 0x01, 0x0f, 0x00, "5 Coins 4 Credits" },
	{0x11, 0x01, 0x0f, 0x0f, "1 Coin 1 Credit"	 },
	{0x11, 0x01, 0x0f, 0x06, "3 Coins 4 Credits" },
	{0x11, 0x01, 0x0f, 0x09, "2 Coins 3 Credits" },
	{0x11, 0x01, 0x0f, 0x0e, "1 Coin 2 Credits"	 },
	{0x11, 0x01, 0x0f, 0x08, "2 Coins 5 Credits" },
	{0x11, 0x01, 0x0f, 0x0d, "1 Coin 3 Credits"	 },
	{0x11, 0x01, 0x0f, 0x0c, "1 Coin 4 Credits"	 },
	{0x11, 0x01, 0x0f, 0x0b, "1 Coin 5 Credits"	 },

	{0   , 0xfe, 0   , 11  , "Coin B" },
	{0x11, 0x01, 0xf0, 0xa0, "2 Coins 1 Credit"  },
	{0x11, 0x01, 0xf0, 0x70, "3 Coins 2 Credits" },
	{0x11, 0x01, 0xf0, 0x00, "5 Coins 4 Credits" },
	{0x11, 0x01, 0xf0, 0xf0, "1 Coin 1 Credit"	 },
	{0x11, 0x01, 0xf0, 0x60, "3 Coins 4 Credits" },
	{0x11, 0x01, 0xf0, 0x90, "2 Coins 3 Credits" },
	{0x11, 0x01, 0xf0, 0xe0, "1 Coin 2 Credits"	 },
	{0x11, 0x01, 0xf0, 0x80, "2 Coins 5 Credits" },
	{0x11, 0x01, 0xf0, 0xd0, "1 Coin 3 Credits"	 },
	{0x11, 0x01, 0xf0, 0xc0, "1 Coin 4 Credits"	 },
	{0x11, 0x01, 0xf0, 0xb0, "1 Coin 5 Credits"	 },

	{0   , 0xfe, 0   , 2   , "Time"	},
	{0x12, 0x01, 0x01, 0x00, "Less"	},
	{0x12, 0x01, 0x01, 0x01, "More"	},

	{0   , 0xfe, 0   , 2   , "Number of Virus" },
	{0x12, 0x01, 0x02, 0x02, "Less" },	
	{0x12, 0x01, 0x02, 0x00, "More"	},

	{0   , 0xfe, 0   , 2   , "Test Mode" },
	{0x12, 0x01, 0x08, 0x08, "Off" },
	{0x12, 0x01, 0x08, 0x00, "On"	 },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x12, 0x01, 0x10, 0x10, "Off" },
	{0x12, 0x01, 0x10, 0x00, "On"	 },

	{0   , 0xfe, 0   , 2   , "Language"	},
	{0x12, 0x01, 0x20, 0x20, "English" },
	{0x12, 0x01, 0x20, 0x00, "Italian" },

	{0   , 0xfe, 0   , 2   , "Allow Continue"	},
	{0x12, 0x01, 0x40, 0x40, "No"	 },
	{0x12, 0x01, 0x40, 0x00, "Yes" },
};

STDDIPINFO(Drv)

static inline void drtomy_okibank_w(UINT8 data)
{
	m6295bank = data & 3;
	memcpy (Snd + 0x20000, Snd + 0x40000 + m6295bank * 0x20000, 0x20000);
}

static inline void set_palette(UINT16 pal, INT32 offset)
{
	INT32 r = (pal >> 10) & 0x1f;
	INT32 g = (pal >>  5) & 0x1f;
	INT32 b = (pal >>  0) & 0x1f;

	    r = (r << 3) | (r >> 2);
	    g = (g << 3) | (g >> 2);
	    b = (b << 3) | (b >> 2);

	Palette[offset] = (r << 16) | (g << 8) | b;
	DrvPalette[offset] = BurnHighCol(r, g, b, 0);
}

UINT8 __fastcall drtomy_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x700001:
			return DrvDips[0]; 

		case 0x700003:
			return DrvDips[1]; 

		case 0x700005:
			return DrvInputs[0]; 

		case 0x700007:
			return DrvInputs[1];

		case 0x70000f:
			return MSM6295ReadStatus(0);
	}

	return 0;
}

UINT16 __fastcall drtomy_read_word(UINT32)
{
	return 0;
}

void __fastcall drtomy_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x70000d:
			if (m6295bank != (data & 3)) {
				drtomy_okibank_w(data);
			}
		return;

		case 0x70000f:
			MSM6295Command(0, data);
		return;
	}
}

void __fastcall drtomy_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfff800) == 0x200000) {
		UINT16 *pal = (UINT16*)(PalRam + (address & 0x7fe));

		*pal = data;

		set_palette(*pal, (address >> 1) & 0x3ff);

		return;
	}
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (VidRam,  0, 0x2000);
	memset (PalRam,  0, 0x0800);
	memset (SprRam,  0, 0x1000);
	memset (WorkRam, 0, 0x4000);
	memset (Palette, 0, 0x1000);

	DrvRecalc = 1;

	SekOpen(0);
	SekReset();
	SekClose();

	m6295bank = 0;
	MSM6295Reset(0);

	return 0;
}

static INT32 DrvGfxDecode()
{
	static INT32 Planes[4] = { 0x000000, 0x200000, 0x400000, 0x600000 };
	static INT32 XOffs[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
				 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 };
	static INT32 YOffs[16] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
				 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, Gfx0, 0x100000);

	GfxDecode(0x8000, 4,  8,  8, Planes, XOffs, YOffs, 0x040, tmp, Gfx0);
	GfxDecode(0x2000, 4, 16, 16, Planes, XOffs, YOffs, 0x100, tmp, Gfx1);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Rom		= Next; Next += 0x040000;
	Gfx0		= Next; Next += 0x200000;
	Gfx1		= Next; Next += 0x200000;
	Snd		= Next; Next += 0x0c0000;

	DrvPalette	= (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	RamStart	= Next;

	VidRam		= Next; Next += 0x002000;
	PalRam		= Next; Next += 0x000800;
	SprRam		= Next; Next += 0x001000;
	WorkRam		= Next; Next += 0x004000;

	Palette		= (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Rom + 0x000000, 0, 2)) return 1;
		if (BurnLoadRom(Rom + 0x000001, 1, 2)) return 1;

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(Gfx0 + i * 0x40000, i + 2, 1)) return 1;
		}

		if (BurnLoadRom(Snd + 0x000000, 6, 1)) return 1;
		memcpy (Snd + 0x40000, Snd + 0x00000, 0x80000);
		memcpy (Snd + 0x20000, Snd + 0x00000, 0x20000);

		MSM6295ROM = Snd;

		if (DrvGfxDecode()) return 1;
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Rom,	0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(VidRam,	0x100000, 0x101fff, SM_RAM);
	SekMapMemory(PalRam,	0x200000, 0x1007ff, SM_ROM);
	SekMapMemory(SprRam,	0x440000, 0x440fff, SM_RAM);
	SekMapMemory(WorkRam,	0xffc000, 0xffffff, SM_RAM);
	SekSetWriteByteHandler(0, drtomy_write_byte);
	SekSetWriteWordHandler(0, drtomy_write_word);
	SekSetReadByteHandler(0, drtomy_read_byte);
	SekSetReadWordHandler(0, drtomy_read_word);
	SekClose();

	MSM6295Init(0, 1625000 / 132 /*?*/, 0);
	MSM6295SetRoute(0, 0.80, BURN_SND_ROUTE_BOTH);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();
	GenericTilesExit();
	MSM6295Exit(0);

	BurnFree (Mem);
	
	MSM6295ROM = NULL;

	return 0;
}
	

static void draw_sprites()
{
	INT32 i, x, y, ex, ey;

	static const INT32 x_offset[2] = {0x0,0x2};
	static const INT32 y_offset[2] = {0x0,0x1};

	UINT16 *spriteram = (UINT16*)SprRam;

	for (i = 3; i < 0x1000/2; i+=4){
		INT32 sx = BURN_ENDIAN_SWAP_INT16(spriteram[i+2]) & 0x01ff;
		INT32 sy = (240 - (BURN_ENDIAN_SWAP_INT16(spriteram[i]) & 0x00ff)) & 0x00ff;
		INT32 number = BURN_ENDIAN_SWAP_INT16(spriteram[i+3]);
		INT32 color = (BURN_ENDIAN_SWAP_INT16(spriteram[i+2]) & 0x1e00) >> 9;
		INT32 attr = (BURN_ENDIAN_SWAP_INT16(spriteram[i]) & 0xfe00) >> 9;

		INT32 xflip = attr & 0x20;
		INT32 yflip = attr & 0x40;
		INT32 spr_size;

		if (attr & 0x04){
			spr_size = 1;
		} else {
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++){
			for (x = 0; x < spr_size; x++){

				ex = xflip ? (spr_size-1-x) : x;
				ey = yflip ? (spr_size-1-y) : y;

				INT32 sxx = sx-0x09+x*8;
				INT32 syy = sy+y*8;

				syy -= 0x10;

				INT32 code = number + x_offset[ex] + y_offset[ey];

				if (yflip) {
					if (xflip) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sxx, syy, color, 4, 0, 0x100, Gfx0);
					} else {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sxx, syy, color, 4, 0, 0x100, Gfx0);
					}
				} else {
					if (xflip) {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sxx, syy, color, 4, 0, 0x100, Gfx0);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, code, sxx, syy, color, 4, 0, 0x100, Gfx0);
					}
				}
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x300; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	{
		UINT16 *bg_ram = (UINT16*)(VidRam + 0x1000);
		UINT16 *fg_ram = (UINT16*)(VidRam + 0x0000);

		for (INT32 offs = 0; offs < 0x400 >> 1; offs++) {
			INT32 code, color, sx, sy;

			sx = (offs << 4) & 0x1f0;
			sy = (offs >> 1) & 0x1f0;

			if (sx > 0x140 || sy < 0x10) continue;
			sy -= 0x10;

			// background
			code  = BURN_ENDIAN_SWAP_INT16(bg_ram[offs]) & 0xfff;
			color = BURN_ENDIAN_SWAP_INT16(bg_ram[offs]) >> 12;

			Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, Gfx1);

			// foreground
			code  = BURN_ENDIAN_SWAP_INT16(fg_ram[offs]) & 0xfff;
			color = BURN_ENDIAN_SWAP_INT16(fg_ram[offs]) >> 12;

			if (!code) continue;

			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x200, Gfx1);
		}
	}

	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static inline void makeInputs()
{
	DrvInputs[0] = DrvInputs[1] = 0xff;

	for (INT32 i = 0; i < 8; i++) {
		DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
	}
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	makeInputs();

	SekOpen(0);
	SekRun(12000000 / 60);
	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
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

	if (nAction & ACB_MEMORY_RAM) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd - RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		if (nAction & ACB_WRITE)
			DrvRecalc = 1;
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		MSM6295Scan(0, nAction);

		SCAN_VAR(m6295bank);

		if (nAction & ACB_WRITE)
			drtomy_okibank_w(m6295bank);
	}

	return 0;
}

// Dr. Tomy

static struct BurnRomInfo drtomyRomDesc[] = {
	{ "15.u21", 0x20000, 0x0b8d763b, 1 | BRF_PRG | BRF_ESS },
	{ "16.u22", 0x20000, 0x206f4d65, 1 | BRF_PRG | BRF_ESS },

	{ "20.u80", 0x40000, 0x4d4d86ff, 2 | BRF_GRA },
	{ "19.u81", 0x40000, 0x49ecbfe2, 2 | BRF_GRA },
	{ "18.u82", 0x40000, 0x8ee5c921, 2 | BRF_GRA },
	{ "17.u83", 0x40000, 0x42044b1c, 2 | BRF_GRA },

	{ "14.u23", 0x80000, 0x479614ec, 3 | BRF_SND },
};

STD_ROM_PICK(drtomy)
STD_ROM_FN(drtomy)

struct BurnDriver BurnDrvDrtomy = {
	"drtomy", NULL, NULL, NULL, "1993",
	"Dr. Tomy\0", NULL, "Playmark", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, drtomyRomInfo, drtomyRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 240, 4, 3
};
