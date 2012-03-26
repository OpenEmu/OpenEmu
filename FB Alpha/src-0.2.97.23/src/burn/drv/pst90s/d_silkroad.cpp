// FB Alpha Legend of Silkroad driver
// Based on MAME driver by David Haywood, R.Belmont, and Stephh

#include "tiles_generic.h"
#include "sek.h"
#include "burn_ym2151.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvGfxROM;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRAM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvTransTab;
static UINT16 *DrvSysRegs;
static UINT32 *DrvPalette;

static UINT8 DrvRecalc = 0;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1",	BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"Coin 2",	BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},

	{"P1 Start",	BIT_DIGITAL,	DrvJoy2 + 2,	"p1 start"	},
	{"P1 Up",	BIT_DIGITAL,	DrvJoy1 + 15,	"p1 up"		},
	{"P1 Down",	BIT_DIGITAL,	DrvJoy1 + 14,	"p1 down"	},
	{"P1 Left",	BIT_DIGITAL,	DrvJoy1 + 12,	"p1 left"	},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 13,	"p1 right"	},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 11,	"p1 fire 1"	},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 10,	"p1 fire 2"	},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 9,	"p1 fire 3"	},

	{"P2 Start",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 start"	},
	{"P2 Up",	BIT_DIGITAL,	DrvJoy1 + 7,	"p2 up"		},
	{"P2 Down",	BIT_DIGITAL,	DrvJoy1 + 6,	"p2 down"	},
	{"P2 Left",	BIT_DIGITAL,	DrvJoy1 + 4,	"p2 left"	},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy1 + 5,	"p2 right"	},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy1 + 3,	"p2 fire 1"	},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy1 + 2,	"p2 fire 2"	},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy1 + 1,	"p2 fire 3"	},

	{"Test",	BIT_DIGITAL,	DrvJoy2 + 5,	"diag"		},
	{"Service",	BIT_DIGITAL,	DrvJoy2 + 4,	"service"	},

	{"Reset",	BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip 1",	BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip 2",	BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	{0x15, 0xff, 0xff, 0x1c, NULL},
	{0x16, 0xff, 0xff, 0xf7, NULL},

	{0,    0xfe, 0,    2,    "Lives"		},
	{0x15, 0x01, 0x01, 0x01, "1"			},
	{0x15, 0x01, 0x01, 0x00, "2"			},

	{0,    0xfe, 0,    2,    "Blood Effect"		},
	{0x15, 0x01, 0x02, 0x02, "Off"			},
	{0x15, 0x01, 0x02, 0x00, "On"			},
 
	{0,    0xfe, 0,    8,    "Difficulty"		},
	{0x15, 0x01, 0xe0, 0x60, "Easiest"		},
	{0x15, 0x01, 0xe0, 0x40, "Easier"		},
	{0x15, 0x01, 0xe0, 0x20, "Easy"			},
	{0x15, 0x01, 0xe0, 0x00, "Normal"		},
	{0x15, 0x01, 0xe0, 0xe0, "Medium"		},
	{0x15, 0x01, 0xe0, 0xc0, "Hard"			},
	{0x15, 0x01, 0xe0, 0xa0, "Harder"		},
	{0x15, 0x01, 0xe0, 0x80, "Hardest"		},

	{0,    0xfe, 0,    2,    "Free Play"		},
	{0x16, 0x01, 0x02, 0x02, "Off"			},
	{0x16, 0x01, 0x02, 0x00, "On"			},

	{0,    0xfe, 0,    2,    "Demo Sounds"		},
	{0x16, 0x01, 0x08, 0x08, "Off"			},
	{0x16, 0x01, 0x08, 0x00, "On"			},

	{0,    0xfe, 0,    2,    "Chute Type"		},
	{0x16, 0x01, 0x10, 0x10, "Single"		},
	{0x16, 0x01, 0x10, 0x00, "Multi"		},

	{0,    0xfe, 0,    8,    "Coinage"		},
	{0x16, 0x01, 0xe0, 0x00, "5 Coins 1 Credit"	},
	{0x16, 0x01, 0xe0, 0x20, "4 Coins 1 Credit"	},
	{0x16, 0x01, 0xe0, 0x40, "3 Coins 1 Credit"	},
	{0x16, 0x01, 0xe0, 0x60, "2 Coins 1 Credit"	},
	{0x16, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"	},
	{0x16, 0x01, 0xe0, 0xc0, "1 Coin  2 Credits"	},
	{0x16, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"	},
	{0x16, 0x01, 0xe0, 0x80, "1 Coin  4 Credits"	},
};

STDDIPINFO(Drv)

static void palette_write(INT32 offset, UINT16 pal)
{
	UINT8 r = (pal >> 10) & 0x1f;
	UINT8 g = (pal >>  5) & 0x1f;
	UINT8 b = (pal >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[offset] = BurnHighCol(r, g, b, 0);
}

UINT8 __fastcall silkroad_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xc00000:
			return DrvInputs[0] >> 8;

		case 0xc00001:
			return DrvInputs[0] & 0xff;

		case 0xc00002:
			return 0xff;

		case 0xc00003:
			return DrvInputs[1] & 0xff;

		case 0xc00004:
			return DrvDips[1];

		case 0xc00005:
			return DrvDips[0];

		case 0xc00025:
			return MSM6295ReadStatus(0);

		case 0xc0002d:
			return BurnYM2151ReadStatus();

		case 0xc00031:
			return MSM6295ReadStatus(1);
	}

	return 0;
}

void __fastcall silkroad_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0xc00025:
			MSM6295Command(0, data);
		return;

		case 0xc00029:
			BurnYM2151SelectRegister(data);
		return;

		case 0xc0002d:
			BurnYM2151WriteRegister(data);
		return;

		case 0xc00031:
			MSM6295Command(1, data);
		return;
	}
}

void __fastcall silkroad_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xffffc000) == 0x600000) {
		*((UINT16 *)(DrvPalRAM + (address & 0x3ffe))) = BURN_ENDIAN_SWAP_INT16(data);

		palette_write((address >> 2) & 0x0fff, data);

		return;
	}

	if (address >= 0xc0010c && address <= 0xc00123) {
		DrvSysRegs[(address - 0xc0010c) >> 1] = data;

		return;
	}
}

void __fastcall silkroad_write_long(UINT32 address, UINT32 data)
{
	if ((address & 0xffffc000) == 0x600000) {
		*((UINT32 *)(DrvPalRAM + (address & 0x3ffc))) = BURN_ENDIAN_SWAP_INT32(data);

		palette_write((address >> 2) & 0x0fff, data >> 16);

		return;
	}
}

static INT32 DrvGfxROMDecode()
{
	static INT32 Planes[6] = { 0x0000008, 0x0000000, 0x1000008, 0x1000000, 0x2000008, 0x2000000 };
	static INT32 XOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
				 0x010, 0x011, 0x012, 0x013, 0x014, 0x015, 0x016, 0x017 };
	static INT32 YOffs[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
				 0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x600000);
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < 4; i++ ){
		if (BurnLoadRom(tmp + 0x000000, 2 + i * 3, 1)) return 1;
		if (BurnLoadRom(tmp + 0x200000, 3 + i * 3, 1)) return 1;
		if (BurnLoadRom(tmp + 0x400000, 4 + i * 3, 1)) return 1;

		if (i == 0) { // fix rom04
			for (INT32 j = 0x1b3fff; j >= 0; j--) {
				tmp[0x44c000 + j] = tmp[0x44c000 + (j - 1)];
			}
		}

		for (INT32 j = 0; j < 0x600000; j++) {
			tmp[j] ^= 0xff;
		}

		GfxDecode(0x08000, 6, 16, 16, Planes, XOffs, YOffs, 0x200, tmp, DrvGfxROM + 0x0800000 * i);
	}

	BurnFree (tmp);

	DrvTransTab = (UINT8*)BurnMalloc(0x20000);
	if (DrvTransTab == NULL) {
		return 1;
	}	

	{
		memset (DrvTransTab, 0, 0x20000);
		for (INT32 i = 0, c = 0; i < 0x2000000; i++) {
			if (DrvGfxROM[i]) {
				DrvTransTab[i>>8] |= 1;
				c++;
			}

			if ((i & 0xff) == 0xff) {
				if (c == 0x100) DrvTransTab[i>>8] |= 2;
				c = 0;
			}
		} 
	}

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	BurnYM2151Reset();

	MSM6295Reset(0);
	MSM6295Reset(1);

	for (INT32 nChannel = 0; nChannel < 4; nChannel++) {
		MSM6295SampleInfo[0][nChannel] = DrvSndROM0 + (nChannel << 8);
		MSM6295SampleData[0][nChannel] = DrvSndROM0 + (nChannel << 16);

		MSM6295SampleInfo[1][nChannel] = DrvSndROM1 + (nChannel << 8);
		MSM6295SampleData[1][nChannel] = DrvSndROM1 + (nChannel << 16);
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x0200000;

	MSM6295ROM	= Next;
	DrvSndROM0	= Next; Next += 0x0080000;
	DrvSndROM1	= Next; Next += 0x0040000;

	DrvPalette	= (UINT32  *)Next; Next += 0x0001001 * sizeof (UINT32);
	
	AllRam		= Next;

	DrvSprRAM	= Next; Next += 0x0001000;
	DrvPalRAM	= Next; Next += 0x0004000;
	DrvVidRAM	= Next; Next += 0x000c000;
	Drv68KRAM	= Next; Next += 0x0020000;

	DrvSysRegs	= (UINT16*)Next; Next += 0x0000020 * sizeof (UINT16);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

	DrvGfxROM = (UINT8*)BurnMalloc(0x2000000);
	if (DrvGfxROM == NULL) {
		return 1;
	}

	if (DrvGfxROMDecode()) return 1;

	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM + 0, 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 1, 1, 2)) return 1;

		for (INT32 i = 0; i < 0x200000; i+= 4) {
			INT32 t = Drv68KROM[i + 1];
			Drv68KROM[i + 1] = Drv68KROM[i + 2];
			Drv68KROM[i + 2] = t;
		}

		if (BurnLoadRom(DrvSndROM0, 14, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1, 15, 1)) return 1;
	}

	SekInit(0, 0x68EC020);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x40c000, 0x40cfff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x600000, 0x603fff, SM_ROM);
	SekMapMemory(DrvVidRAM,		0x800000, 0x80bfff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0xfe0000, 0xffffff, SM_RAM);
	SekSetWriteByteHandler(0,	silkroad_write_byte);
	SekSetWriteWordHandler(0,	silkroad_write_word);
	SekSetWriteLongHandler(0,	silkroad_write_long);
	SekSetReadByteHandler(0,	silkroad_read_byte);
	SekClose();

	BurnYM2151Init(3579545, 100.0);
	MSM6295Init(0, 1056000 / 132, 45.0, 1);
	MSM6295Init(1, 2112000 / 132, 45.0, 1);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();

	BurnYM2151Exit();
	MSM6295Exit(0);
	MSM6295Exit(1);

	GenericTilesExit();

	BurnFree (AllMem);
	BurnFree (DrvGfxROM);
	BurnFree (DrvTransTab);

	DrvRecalc = 0;

	return 0;
}

static void silkroad_draw_tile(INT32 code, INT32 sx, INT32 sy, INT32 color, INT32 flipx)
{
	if (!DrvTransTab[code]) return;

	if (flipx) {
		if (sx >= 0 && sx <= 352 && sy >= 0 && sy <= 208) {
			if (DrvTransTab[code] & 2) {
				Render16x16Tile_FlipX(pTransDraw, code, sx, sy, color, 6, 0, DrvGfxROM);
			} else {
				Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 6, 0, 0, DrvGfxROM);
			}
		} else {
			if (DrvTransTab[code] & 2) {
				Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 6, 0, DrvGfxROM);
			} else {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 6, 0, 0, DrvGfxROM);
			}
		}
	} else {
		if (sx >= 0 && sx <= 352 && sy >= 0 && sy <= 208) {
			if (DrvTransTab[code] & 2) {
				Render16x16Tile(pTransDraw, code, sx, sy, color, 6, 0, DrvGfxROM);
			} else {
				Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 6, 0, 0, DrvGfxROM);
			}
		} else {
			if (DrvTransTab[code] & 2) {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 6, 0, DrvGfxROM);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 6, 0, 0, DrvGfxROM);
			}
		}
	}
}

static void draw_sprites(INT32 pri)
{
	UINT32 *source = (UINT32*)DrvSprRAM;
	UINT32 *finish = source + 0x1000/4;
	UINT32 *maxspr = source;

	while (maxspr < finish)
	{
		if ((BURN_ENDIAN_SWAP_INT32(maxspr[1]) & 0xff000000) == 0xff000000) break;
		maxspr += 2;
	}

	finish = maxspr - 2;

	while (finish >= source)
	{
		INT32 xpos   = (BURN_ENDIAN_SWAP_INT32(finish[0]) & 0x01ff) - 50;
		INT32 ypos   = (BURN_ENDIAN_SWAP_INT32(finish[0]) >> 16)    - 16;
		INT32 attr   =  BURN_ENDIAN_SWAP_INT32(finish[1]) >> 16;
		INT32 tileno = (BURN_ENDIAN_SWAP_INT32(finish[1]) & 0xffff) | ((attr & 0x8000) << 1);
		finish -= 2;

		INT32 priority = attr & 0x1000;
		if (pri != priority || ypos == -16 || ypos > 223) continue;

		INT32 color = attr & 0x003f;
		INT32 flipx = attr & 0x0080;
		INT32 width = ((attr >> 8) & 0x0f) + 1;

		if (flipx) {
			for (INT32 wcount = width; wcount > 0; wcount--) {
				INT32 sx = xpos + (((wcount << 4) - 0x10) | 8);

				if (sx < -15 || sx > 379) continue;

				silkroad_draw_tile(tileno+(width-wcount), sx, ypos, color, 1);
			}
		} else {
			for (INT32 wcount = 0; wcount < width; wcount++) {
				INT32 sx = xpos + ((wcount << 4) | 8);

				if (sx < -15 || sx > 379) continue;

				silkroad_draw_tile(tileno + wcount, sx, ypos, color, 0);
			}
		}
	}
}

static void draw_layer(INT32 offset, INT32 scrollx, INT32 scrolly)
{
	UINT32 *vidram = (UINT32*)(DrvVidRAM + offset);

	for (INT32 offs = 0; offs < 0x4000 / 4; offs++)
	{
		INT32 sx = (offs << 4) & 0x3f0;
		INT32 sy = (offs >> 2) & 0x3f0;
		sx -= scrollx + 50;
		sy -= scrolly + 16;
		if (sy < -15) sy += 0x400;
		if (sx < -15) sx += 0x400;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 code  =  BURN_ENDIAN_SWAP_INT32(vidram[offs]) & 0xffff;
		INT32 color = (BURN_ENDIAN_SWAP_INT32(vidram[offs]) >> 16) & 0x1f;
		INT32 flipx = (BURN_ENDIAN_SWAP_INT32(vidram[offs]) >> 16) & 0x80;

		silkroad_draw_tile(code + 0x18000, sx, sy, color, flipx);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc != nBurnBpp) {
		UINT16 *pal = (UINT16*)DrvPalRAM;
		for (INT32 i = 0; i < 0x2000; i+=2) {
			palette_write(i/2, pal[i]);
		}

		DrvPalette[0x1000] = BurnHighCol(0xff, 0x00, 0xff, 0);
		DrvRecalc = nBurnBpp;
	}

	if (nBurnLayer & 1) {
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			pTransDraw[i] = 0x7c0;
		}
	} else {
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			pTransDraw[i] = 0x1000;
		}
	}

	if (nBurnLayer    & 2) draw_layer(0x0000, DrvSysRegs[ 0] & 0x3ff, DrvSysRegs[ 1] & 0x3ff);
	if (nSpriteEnable & 1) draw_sprites(0x0000);
	if (nBurnLayer    & 4) draw_layer(0x4000, DrvSysRegs[ 5] & 0x3ff, DrvSysRegs[10] & 0x3ff);
	if (nSpriteEnable & 2) draw_sprites(0x1000);
	if (nBurnLayer    & 8) draw_layer(0x8000, DrvSysRegs[ 4] & 0x3ff, DrvSysRegs[ 2] & 0x3ff);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = DrvInputs[1] = 0xffff;
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= DrvJoy1[i] << i;
			DrvInputs[1] ^= DrvJoy2[i] << i;
		}

		DrvInputs[1] ^= DrvJoy2[4] << 6;

		if (!(DrvInputs[0] & 0x00c0)) DrvInputs[0] |= 0x00c0;
		if (!(DrvInputs[0] & 0x0030)) DrvInputs[0] |= 0x0030;
		if (!(DrvInputs[0] & 0xc000)) DrvInputs[0] |= 0xc000;
		if (!(DrvInputs[0] & 0x3000)) DrvInputs[0] |= 0x3000;
	}

	INT32 nTotalCycles = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));

	SekOpen(0);
	SekRun(nTotalCycles);
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnSoundOut) {
		BurnYM2151Render(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029682;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);

		BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);
		MSM6295Scan(1, nAction);

		SCAN_VAR(DrvInputs[0]);
		SCAN_VAR(DrvInputs[1]);
	}

	return 0;
}


// The Legend of Silkroad

static struct BurnRomInfo silkroadRomDesc[] = {
	{ "rom02.bin",	0x100000, 0x4e5200fc, 1 | BRF_PRG }, //  0 Motorola '020 Code
	{ "rom03.bin",	0x100000, 0x73ccc78c, 1 | BRF_PRG }, //  1

	{ "rom12.bin",	0x200000, 0x96393d04, 2 | BRF_GRA }, //  2 Tiles
	{ "rom08.bin",	0x200000, 0x23f1d462, 2 | BRF_GRA }, //  3
	{ "rom04.bin",	0x200000, 0x2cf6ed30, 2 | BRF_GRA }, //  4

	{ "rom13.bin",	0x200000, 0x4ca1698e, 2 | BRF_GRA }, //  5
	{ "rom09.bin",	0x200000, 0xef0b5bf4, 2 | BRF_GRA }, //  6
	{ "rom05.bin",	0x200000, 0x512d6e25, 2 | BRF_GRA }, //  7

	{ "rom14.bin",	0x200000, 0xd00b19c4, 2 | BRF_GRA }, //  8
	{ "rom10.bin",	0x200000, 0x7d324280, 2 | BRF_GRA }, //  9
	{ "rom06.bin",	0x200000, 0x3ac26060, 2 | BRF_GRA }, // 10

	{ "rom07.bin",	0x200000, 0x9fc6ff9d, 2 | BRF_GRA }, // 11
	{ "rom11.bin",	0x200000, 0x11abaf1c, 2 | BRF_GRA }, // 12
	{ "rom15.bin",	0x200000, 0x26a3b168, 2 | BRF_GRA }, // 13

	{ "rom00.bin",  0x080000, 0xb10ba7ab, 3 | BRF_SND }, // 14 OKI Samples
	{ "rom01.bin",  0x040000, 0xdb8cb455, 4 | BRF_SND }, // 15
};

STD_ROM_PICK(silkroad)
STD_ROM_FN(silkroad)

struct BurnDriver BurnDrvSilkroad = {
	"silkroad", NULL, NULL, NULL, "1999",
	"The Legend of Silkroad\0", NULL, "Unico", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SCRFIGHT, 0,
	NULL, silkroadRomInfo, silkroadRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1001,
	380, 224, 4, 3
};
