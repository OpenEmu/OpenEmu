// FB Alpha Andamiro "Midas" hardware driver module
// Based on MAME driver by Luca Elia

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "eeprom.h"
#include "ymz280b.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvSprROM;
static UINT8 *DrvTxtROM;
static UINT8 *Drv68KRAM0;
static UINT8 *Drv68KRAM1;
static UINT8 *DrvPalRAM;
static UINT8 *DrvGfxRAM;
static UINT32 *DrvPalette;
static UINT8 *DrvSprTransTab;
static UINT8 *DrvTxtTransTab;
static UINT16 *DrvGfxRegs;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvJoy4[16];
static UINT8 DrvJoy5[16];
static UINT8 DrvJoy6[16];
static UINT8 DrvDips[1];
static UINT8 DrvReset;
static UINT16 DrvInputs[6];

static INT32 DrvRecalc0 = 0;

static short zoom_table[16][16];

static struct BurnInputInfo LivequizInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 8,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy6 + 10,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy6 + 11,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy6 + 8,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy6 + 9,	"p1 fire 4"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 10,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 fire 4"	},

	{"P3 Start",		BIT_DIGITAL,	DrvJoy5 + 0,	"p3 start"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy4 + 0,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy4 + 1,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy4 + 2,	"p3 fire 3"	},
	{"P3 Button 4",		BIT_DIGITAL,	DrvJoy4 + 3,	"p3 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Livequiz)

static struct BurnDIPInfo LivequizDIPList[] =
{
	{0x11, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    2, "Freeze"	},
	{0x11, 0x01, 0x80, 0x80, "Off"		},
	{0x11, 0x01, 0x80, 0x00, "On"		},
};

STDDIPINFO(Livequiz)

inline static void palette_write(INT32 offs)
{
	DrvPalette[offs/4] = BurnHighCol(DrvPalRAM[offs + 0], DrvPalRAM[offs + 3], DrvPalRAM[offs + 2], 0);
}

void __fastcall midas_write_byte(UINT32 address, UINT8 data)
{
	if (address >= 0xa00000 && address <= 0xa3ffff) {
		DrvPalRAM[(address & 0x3ffff) ^ 1] = data;
		palette_write(address & 0x3fffc);
		return;
	}

	switch (address)
	{
		case 0x9a0001:
			EEPROMWrite(data & 0x02, data & 0x01, data & 0x04);
		return;

		case 0xb80009:
			YMZ280BSelectRegister(data);
		return;

		case 0xb8000b:
			YMZ280BWriteRegister(data);
		return;
	}
}

void __fastcall midas_write_word(UINT32 address, UINT16 data)
{
	if (address >= 0xa00000 && address <= 0xa3ffff) {
		*((UINT16*)(DrvPalRAM + (address & 0x3fffe))) = BURN_ENDIAN_SWAP_INT16(data);
		palette_write(address & 0x3fffc);
		return;
	}

	switch (address)
	{
		case 0x9c0000:
			DrvGfxRegs[0] = data;
		return;

		case 0x9c0002:
		{
			DrvGfxRegs[1] = data;
			*((UINT16*)(DrvGfxRAM + (DrvGfxRegs[0] << 1))) = BURN_ENDIAN_SWAP_INT16(data);
			DrvGfxRegs[0] += DrvGfxRegs[2];
		}
		return;

		case 0x9c0004:
			DrvGfxRegs[2] = data;
		return;
	}
}

UINT8 __fastcall midas_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x900001:
			return DrvInputs[5];

		case 0x920001:
			return DrvInputs[2] | (EEPROMRead() ? 8 : 0);

		case 0x940000:
			return (DrvInputs[0] >> 8);

		case 0x980000:
			return (DrvInputs[1] >> 8);

		case 0xb8000b:
			return YMZ280BReadStatus();

		case 0xba0001:
			return DrvInputs[4];

		case 0xbc0001:
			return DrvInputs[3];
	}

	return 0;
}

UINT16 __fastcall midas_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x900000:
			return DrvInputs[5];

		case 0x980000:
			return DrvInputs[1];

		case 0xb00000:
		case 0xb20000:
		case 0xb40000:
		case 0xb60000:
			return 0xffff;

		case 0xbc0000:
			return DrvInputs[3];
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

	EEPROMReset();

	YMZ280BReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x200000;

	DrvTxtROM	= Next; Next += 0x040000;
	DrvSprROM	= Next; Next += 0x800000;

	DrvTxtTransTab	= Next; Next += 0x040000 / 0x040;
	DrvSprTransTab	= Next; Next += 0x800000 / 0x100;

	YMZ280BROM	= Next; Next += 0x200000;

	DrvPalette	= (UINT32*)Next; Next += 0x10000 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM0	= Next; Next += 0x020000;
	Drv68KRAM1	= Next; Next += 0x040000;
	DrvPalRAM	= Next; Next += 0x080000;
	DrvGfxRAM	= Next; Next += 0x020000;

	DrvGfxRegs	= (UINT16*)Next; Next += 3 * sizeof(UINT16);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void DrvInitZoomTable()
{
	zoom_table[0][0] = -1;

	for (INT32 x = 1; x < 16; x++) {
		for (INT32 y = 0; y < 16; y++) {
			float t = ((16.0000-1.0000) / x) * y;
			zoom_table[x][y] = (t >= 16) ? -1 : (INT32)t;
		}
	}
}

static void DrvFillTransTabs(UINT8 *tab, UINT8 *gfx, INT32 gfxlen, INT32 tilesize)
{
	for (INT32 i = 0; i < gfxlen; i+=tilesize)
	{
		tab[i/tilesize] = 0;

		INT32 count = 0;

		for (INT32 j = 0; j < tilesize; j++) {
			if (gfx[i + j]) {
				tab[i/tilesize] = 1;
				count++;
			}
		}

		if (count >= (tilesize-1)) {
			tab[i/tilesize] |= 2;
		}
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[8]  = { 0x3000008, 0x3000000, 0x1000008, 0x1000000, 0x2000008, 0x2000000, 0x0000008, 0x0000000 };
	INT32 XOffs0[16] = { 0x107, 0x106, 0x105, 0x104, 0x103, 0x102, 0x101, 0x100, 0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000 };
	INT32 YOffs0[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070, 0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0 };

	INT32 Plane1[8]  = { 0x008, 0x009, 0x00a, 0x00b, 0x000, 0x001, 0x002, 0x003 };
	INT32 XOffs1[8]  = { 0x104, 0x100, 0x184, 0x180, 0x004, 0x000, 0x084, 0x080 };
	INT32 YOffs1[8]  = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x800000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvSprROM, 0x800000);

	GfxDecode(0x8000, 8, 16, 16, Plane0, XOffs0, YOffs0, 0x200, tmp, DrvSprROM);

	memcpy (tmp, DrvTxtROM, 0x040000);

	GfxDecode(0x1000, 8,  8,  8, Plane1, XOffs1, YOffs1, 0x200, tmp, DrvTxtROM);

	DrvFillTransTabs(DrvSprTransTab, DrvSprROM, 0x800000, 0x100);
	DrvFillTransTabs(DrvTxtTransTab, DrvTxtROM, 0x040000, 0x040);

	BurnFree (tmp);

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
		if (BurnLoadRom(Drv68KROM,		0, 1)) return 1;

		if (BurnLoadRom(DrvTxtROM,      	7, 1)) return 1;

		if (BurnLoadRom(DrvSprROM + 0x000000,	3, 1)) return 1;
		if (BurnLoadRom(DrvSprROM + 0x200000,	4, 1)) return 1;
		if (BurnLoadRom(DrvSprROM + 0x400000,	5, 1)) return 1;
		if (BurnLoadRom(DrvSprROM + 0x600000,	6, 1)) return 1;

		if (BurnLoadRom(YMZ280BROM,	        8, 1)) return 1;

		DrvGfxDecode();
	}

	*((UINT16*)(Drv68KROM + 0x13345a)) = BURN_ENDIAN_SWAP_INT16(0x4e75); // patch out protection

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0xa00000, 0xa3ffff, SM_ROM);
	SekMapMemory(DrvPalRAM + 0x3ffff,	0xa40000, 0xa7ffff, SM_RAM);
	SekMapMemory(Drv68KRAM0,		0xd00000, 0xd1ffff, SM_RAM);
	SekMapMemory(Drv68KRAM1,		0xe00000, 0xe3ffff, SM_RAM);
	SekMapMemory(DrvGfxRAM,			0xf90000, 0xfaffff, SM_RAM);
	SekSetWriteByteHandler(0,		midas_write_byte);
	SekSetWriteWordHandler(0,		midas_write_word);
	SekSetReadByteHandler(0,		midas_read_byte);
	SekSetReadWordHandler(0,		midas_read_word);
	SekClose();

	YMZ280BInit(16934400, NULL);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_1, 0.80, BURN_SND_ROUTE_LEFT);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_2, 0.80, BURN_SND_ROUTE_RIGHT);

	EEPROMInit(&eeprom_interface_93C46);

	DrvInitZoomTable();

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	YMZ280BExit();
	YMZ280BROM = NULL;

	EEPROMExit();

	SekExit();

	BurnFree (AllMem);

	DrvRecalc0 = 0;

	return 0;
}

static void draw_16x16_zoom(INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 xz, INT32 yz, INT32 fx, INT32 fy)
{
	if (yz == 1 || xz == 1) return;

	fx = (fx * 0x0f);
	fy = (fy >> 1) * 0x0f;

	UINT8 *src = DrvSprROM + (code << 8);

	INT16 *xm = zoom_table[xz-1];
	INT16 *ym = zoom_table[yz-1];

	for (INT32 y = 0; y < 16; y++)
	{
		INT32 yy = sy + y;

		if (ym[y ^ fy] == -1 || yy < 0 || yy >= nScreenHeight) continue;

		INT32 yyz = (ym[y ^ fy] << 4);

		for (INT32 x = 0; x < 16; x++)
		{
			INT16 xxz = xm[x ^ fx];
			if (xxz == -1) continue;

			INT32 xx = sx + x;

			INT32 pxl = src[yyz|xxz];

			if (!pxl || xx < 0 || xx >= nScreenWidth || yy < 0 || yy >= nScreenHeight) continue;

			pTransDraw[(yy * nScreenWidth) + xx] = pxl | color;
		}
	}
}

static void draw_sprites()
{
	UINT16 *codes =	(UINT16*)DrvGfxRAM;
	UINT16 *s     =	codes + 0x8000;

	INT32 sx_old = 0, sy_old = 0, ynum_old = 0, xzoom_old = 0;
	INT32 xdim, ydim, xscale, yscale, ynum;

	for (INT32 i = 0; i < 0x180; i++, s++, codes+=0x40)
	{
		INT32 zoom	= BURN_ENDIAN_SWAP_INT16(s[0x000]);
		INT32 sy		= BURN_ENDIAN_SWAP_INT16(s[0x200]);
		INT32 sx		= BURN_ENDIAN_SWAP_INT16(s[0x400]);

		INT32 xzoom	= ((zoom >> 8) & 0x0f) + 1;
		INT32 yzoom	= (zoom & 0x7f) + 1;

		if (sy & 0x40) {
			ynum	= ynum_old;
			sx	= sx_old + xzoom_old;
			sy	= sy_old;
		} else {
			ynum	= sy & 0x3f;
			sx	= (sx >> 7);
			sy	= 0x200 - (sy >> 7);

			if (ynum > 0x20) ynum = 0x20;
		}

		if (sx >= 0x1f0) sx -= 0x200;

		ynum_old  = ynum;
		sx_old    = sx;
		sy_old    = sy;
		xzoom_old = xzoom;

		sy	<<= 16;

		xdim	= xzoom << 16;
		ydim	= yzoom << 13;

		xscale	= xzoom;
		yscale	= yzoom >> 3;

		for (INT32 y = 0; y < ynum; y++)
		{
			INT32 code = BURN_ENDIAN_SWAP_INT16(codes[y * 2]);
			if (code & 0x8000) continue; // only 8mb of sprites...

			INT32 attr = BURN_ENDIAN_SWAP_INT16(codes[y * 2 + 1]);

			INT32 yy = (((sy + y * ydim) >> 16) & 0x1ff) - 16;

			if (sx >= nScreenWidth || yy >= nScreenHeight || !DrvSprTransTab[code]) continue;

			if (yscale != 16 || xscale != 16) {
				draw_16x16_zoom(code, attr & 0xff00, sx, yy, xscale, yscale, attr & 1, attr & 2);
				continue;
			}

			if (DrvSprTransTab[code] & 2) {
				if (sx < 0 || sx > (nScreenWidth-16) || yy < 0 || yy > (nScreenHeight-16)) {
					if (attr & 2) {
						if (attr & 1) {
							Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						} else {
							Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						}
					} else {
						if (attr & 1) {
							Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						} else {
							Render16x16Tile_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						}
					}
				} else {
					if (attr & 2) {
						if (attr & 1) {
							Render16x16Tile_FlipXY(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						} else {
							Render16x16Tile_FlipY(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						}
					} else {
						if (attr & 1) {
							Render16x16Tile_FlipX(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						} else {
							Render16x16Tile(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						}
					}
				}
			} else {
				if (sx < 0 || sx > (nScreenWidth-16) || yy < 0 || yy > (nScreenHeight-16)) {
					if (attr & 2) {
						if (attr & 1) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						}
					} else {
						if (attr & 1) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						}
					}
				} else {
					if (attr & 2) {
						if (attr & 1) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						} else {
							Render16x16Tile_Mask_FlipY(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						}
					} else {
						if (attr & 1) {
							Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						} else {
							Render16x16Tile_Mask(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						}
					}
				}
			}
		}
	}
}

static void draw_layer()
{
	UINT16 *vram = (UINT16*)(DrvGfxRAM + 0xe000);

	for (INT32 offs = 0; offs < 0x28 * 0x20; offs++)
	{
		INT32 sx = (offs >> 5) << 3;
		INT32 sy = ((offs & 0x1f) << 3) - 16;

		INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs]) & 0xfff;
		INT32 color = BURN_ENDIAN_SWAP_INT16(vram[offs]) >> 12;

		if (sy < 0 || sy >= nScreenHeight || !DrvTxtTransTab[code]) continue;

		if (DrvTxtTransTab[code] & 2) {
			Render8x8Tile(pTransDraw, code, sx, sy, color, 8, 0, DrvTxtROM); 
		} else {
			Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 8, 0, 0, DrvTxtROM);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc0 != nBurnBpp) {
		for (INT32 i = 0; i < 0x40000; i+=4) {
			palette_write(i);
		}

		DrvRecalc0 = nBurnBpp;
	}

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++)
		pTransDraw[i] = 0xfff;

	draw_sprites();

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
		memset (DrvInputs, 0xff, 6 * sizeof(UINT16));

		for (INT32 i = 0; i < 16; i++)
		{
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
			DrvInputs[5] ^= (DrvJoy6[i] & 1) << i;
		}

		DrvInputs[2] &= ~0x0008;
		DrvInputs[5]  = (DrvInputs[5] & 0xff00) | DrvDips[0];
	}

	SekOpen(0);
	SekRun(12000000 / 60);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnSoundOut) {
		YMZ280BRender(pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029698;
	}

	EEPROMScan(nAction, pnMin);

	if (nAction & ACB_VOLATILE) {

		memset(&ba, 0, sizeof(ba));
    		ba.Data		= AllRam;
		ba.nLen		= RamEnd - AllRam;
		ba.szName	= "All RAM";
		BurnAcb(&ba);

		SekScan(nAction);

		YMZ280BScan();
	}

	return 0;
}


// Live Quiz Show

static struct BurnRomInfo livequizRomDesc[] = {
	{ "flash.u1",		0x200000, 0x8ec44493, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code

	{ "main_pic12c508a.u27",0x000400, 0xa84f0a7e, 2 | BRF_PRG | BRF_OPT }, //  1 Mcu Code (not used)
	{ "sub_pic12c508a.u4",	0x000400, 0xe52ebdc4, 2 | BRF_PRG | BRF_OPT }, //  2

	{ "flash.u15",		0x200000, 0xd6eb56f1, 3 | BRF_GRA },           //  3 Sprites
	{ "flash.u16",		0x200000, 0x4c9fd873, 3 | BRF_GRA },           //  4
	{ "flash.u19",		0x200000, 0xdaa81532, 3 | BRF_GRA },           //  5
	{ "flash.u20",		0x200000, 0xb540a8c7, 3 | BRF_GRA },           //  6

	{ "27c4096.u23",	0x080000, 0x25121de8, 4 | BRF_GRA },           //  7 Characters

	{ "flash.u5",		0x200000, 0xdc062792, 5 | BRF_SND },           //  8 YMZ280b Samples
};

STD_ROM_PICK(livequiz)
STD_ROM_FN(livequiz)

struct BurnDriver BurnDrvLivequiz = {
	"livequiz", NULL, NULL, NULL, "1999",
	"Live Quiz Show\0", NULL, "Andamiro Entertainment Co. Ltd.", "Andamiro Midas",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, livequizRomInfo, livequizRomName, NULL, NULL, LivequizInputInfo, LivequizDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x10000,
	320, 224, 4, 3
};
