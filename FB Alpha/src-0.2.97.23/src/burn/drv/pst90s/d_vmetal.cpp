// FB Alpha Varia Metal driver module
// Based on MAME driver by 

// There is a bug in the background scrolling of some levels
// Also a bug in sprite drawing, flipped case? (see special shot)

#include "tiles_generic.h"
#include "msm6295.h"
#include "es8712.h"

static UINT8 *AllMem		= NULL;
static UINT8 *MemEnd		= NULL;
static UINT8 *AllRam		= NULL;
static UINT8 *RamEnd		= NULL;
static UINT8 *Drv68KROM		= NULL;
static UINT8 *DrvGfxROM		= NULL;
static UINT8 *DrvSndROM0	= NULL;
static UINT8 *DrvSndROM1	= NULL;
static UINT8 *Drv68KRAM		= NULL;
static UINT8 *DrvPalRAM		= NULL;
static UINT8 *DrvSprRAM		= NULL;
static UINT8 *DrvTXTRAM		= NULL;
static UINT8 *DrvMD1RAM		= NULL;
static UINT8 *DrvMD2RAM		= NULL;
static UINT8 *DrvTLUTRAM	= NULL;
static UINT8 *DrvVidRegs	= NULL;

static UINT16 *DrvPriBmp	= NULL;

static UINT32 *DrvPalette	= NULL;
static UINT8  DrvRecalc;

static INT32 blackpen = 0;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static struct BurnInputInfo vmetalInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 3,	"service"	},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 4,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy2 + 2,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(vmetal)

static struct BurnDIPInfo vmetalDIPList[]=
{
	{0x16, 0xff, 0xff, 0xff, NULL			},
	{0x17, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x16, 0x01, 0x07, 0x05, "3 Coins 1 Credits"	},
	{0x16, 0x01, 0x07, 0x06, "2 Coins 1 Credits"	},
	{0x16, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x16, 0x01, 0x07, 0x04, "1 Coin  2 Credits"	},
	{0x16, 0x01, 0x07, 0x03, "1 Coin  3 Credits"	},
	{0x16, 0x01, 0x07, 0x02, "1 Coin  4 Credits"	},
	{0x16, 0x01, 0x07, 0x01, "1 Coin  5 Credits"	},
	{0x16, 0x01, 0x07, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x16, 0x01, 0x10, 0x10, "Off"			},
	{0x16, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x16, 0x01, 0x20, 0x00, "Off"			},
	{0x16, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x17, 0x01, 0x03, 0x02, "Easy"			},
	{0x17, 0x01, 0x03, 0x03, "Normal"		},
	{0x17, 0x01, 0x03, 0x01, "Hard"			},
	{0x17, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x17, 0x01, 0x0c, 0x08, "1"			},
	{0x17, 0x01, 0x0c, 0x04, "2"			},
	{0x17, 0x01, 0x0c, 0x0c, "3"			},
	{0x17, 0x01, 0x0c, 0x00, "4"			},

	{0   , 0xfe, 0   ,    2, "Bonus Life"		},
	{0x17, 0x01, 0x10, 0x10, "Every 30000"		},
	{0x17, 0x01, 0x10, 0x00, "Every 60000"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x17, 0x01, 0x80, 0x80, "Off"			},
	{0x17, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(vmetal)

void __fastcall vmetal_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x400001:
		case 0x400003:
			MSM6295Command(0, data);
		return;
	}

	if ((address & 0xfffffff0) == 0x500000) {
		es8712Write(0, (address/2) & 0x07, data);
		return;
	}
}

void __fastcall vmetal_write_word(UINT32 address, UINT16 data)
{
	if (address == 0x200000) {
		if ((data & 0x40) == 0)
			es8712Reset(0);
		else
			es8712Play(0);

		if (data & 0x10)
			es8712SetBankBase(0, 0x100000);
		else
			es8712SetBankBase(0, 0x000000);
	}
}

#define read_dip(dip) (((DrvDips[0] & (1 << (dip-1))) ? 0x40 : 0) | ((DrvDips[1] & (1 << (dip-1))) ? 0x80 : 0))

UINT8 __fastcall vmetal_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x400001:
			return MSM6295ReadStatus(0);
	}

	return 0;
}

UINT16 __fastcall vmetal_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x200000:
			return DrvInputs[0];

		case 0x200002:
			return DrvInputs[1];

		case 0x31fefe:
			return read_dip(8);

		case 0x31ff7e:
			return read_dip(7);

		case 0x31ffbe:
			return read_dip(6);

		case 0x31ffde:
			return read_dip(5);

		case 0x31ffee:
			return read_dip(4);

		case 0x31fff6:
			return read_dip(3);

		case 0x31fffa:
			return read_dip(2);

		case 0x31fffc:
			return read_dip(1);
	}

	if ((address & 0xffff0000) == 0x160000) {
		UINT16 *vreg = (UINT16*)DrvVidRegs;

		INT32 offset = ((address & 0xfffe) | ((BURN_ENDIAN_SWAP_INT16(vreg[0x0ab/2]) & 0x7f) << 16)) * 2;

		return (DrvGfxROM[offset + 0] << 12) | (DrvGfxROM[offset + 1] << 8) | (DrvGfxROM[offset + 2] << 4) | (DrvGfxROM[offset + 3] << 0);
	}

	return 0;
}

static inline void palette_write(INT32 offset)
{
	if (offset & 0x2000) {
		INT32 rgb = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvPalRAM + offset)));

		if (rgb == 0) blackpen = offset/2;

		UINT8 g = (rgb >> 11);
		UINT8 r = (rgb >> 6) & 0x1f;
		UINT8 b = (rgb >> 1) & 0x1f;

		DrvPalette[(offset/2)&0xfff] = BurnHighCol((r << 3) | (r >> 5), (g << 3) | (g >> 5), (b << 3) | (b >> 5), 0);
	}
}

void __fastcall vmetal_palette_write_byte(UINT32 address, UINT8 data)
{
	DrvPalRAM[(address & 0x3fff)^1] = data;

	palette_write(address & 0x3ffe);
}

void __fastcall vmetal_palette_write_word(UINT32 address, UINT16 data)
{
	*((UINT16*)(DrvPalRAM + (address & 0x3ffe))) = BURN_ENDIAN_SWAP_INT16(data);

	palette_write(address & 0x3ffe);
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	es8712Reset(0);
	MSM6295Reset(0);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x100000;

	DrvGfxROM	= Next; Next += 0x800000 * 2;

	MSM6295ROM	= Next;
	DrvSndROM0	= Next; Next += 0x100000;
	DrvSndROM1	= Next; Next += 0x200000;

	DrvPalette	= (UINT32*)Next; Next += 0x1000 * sizeof(UINT32);

	DrvPriBmp	= (UINT16*)Next; Next += 320 * 224 * sizeof(UINT16);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;

	DrvPalRAM	= Next; Next += 0x004000;
	DrvSprRAM	= Next; Next += 0x004000;

	DrvTXTRAM	= Next; Next += 0x020000;
	DrvMD1RAM	= Next; Next += 0x020000;
	DrvMD2RAM	= Next; Next += 0x020000;

	DrvTLUTRAM	= Next; Next += 0x000800;
	DrvVidRegs	= Next; Next += 0x001000;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void expand_gfx()
{
	for (INT32 i = 0; i < 0x800000; i+=8) { // properly interleave graphics
		for (INT32 j = 0; j < 8; j++) {
			DrvGfxROM[0x800000+j] = DrvGfxROM[i + ((j & 1) << 2) + ((j & 6) >> 1)];
		}

		memcpy (DrvGfxROM + i, DrvGfxROM + 0x800000, 8);
	}

	for (INT32 i = 0x1000000-1; i >= 0; i--) { // unpack 4bpp pixel data
		DrvGfxROM[i] = (DrvGfxROM[i/2] >> ((i & 1) << 2)) & 0x0f;
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
		if (BurnLoadRom(Drv68KROM + 0x000000,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x000001,  1, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM + 0x000002,  2, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM + 0x000000,  3, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM + 0x000003,  4, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM + 0x000001,  5, 4)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 6, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x000000, 7, 1)) return 1;

		expand_gfx();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvTXTRAM,		0x100000, 0x11ffff, SM_RAM);
	SekMapMemory(DrvMD1RAM,		0x120000, 0x13ffff, SM_RAM);
	SekMapMemory(DrvMD2RAM,		0x140000, 0x15ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x170000, 0x173fff, SM_RAM); 
	SekMapMemory(DrvSprRAM,		0x174000, 0x177fff, SM_RAM);
	SekMapMemory(DrvTLUTRAM,	0x178000, 0x1787ff, SM_RAM);
	SekMapMemory(DrvVidRegs,	0x178800, 0x1797ff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0xff0000, 0xffffff, SM_RAM);
	SekSetWriteByteHandler(0,	vmetal_write_byte);
	SekSetWriteWordHandler(0,	vmetal_write_word);
	SekSetReadByteHandler(0,	vmetal_read_byte);
	SekSetReadWordHandler(0,	vmetal_read_word);

	SekMapHandler(1,		0x170000, 0x173fff, SM_WRITE);
	SekSetWriteByteHandler(1,	vmetal_palette_write_byte);
	SekSetWriteWordHandler(1,	vmetal_palette_write_word);
	SekClose();

	es8712Init(0, DrvSndROM1, 12000 / 60 /* Rate / FPS */, 50.0, 0);

	MSM6295Init(0, 1320000 / 132, 75.0, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	MSM6295Exit(0);
	es8712Exit(0);
	SekExit();

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	return 0;
}

static void draw_layer_8x8()
{
	UINT16 *vram = (UINT16*)DrvTXTRAM;
	UINT16 *vlut = (UINT16*)DrvTLUTRAM;

	for (INT32 y = 0; y < 224 / 8; y++) {
		for (INT32 x = 0; x < 320 / 8; x++) {
			INT32 offs = (y << 8) | x;

			INT32 sx = x << 3;
			INT32 sy = y << 3;
			if (sy >= nScreenHeight || sx >= nScreenWidth) continue;

			INT32 data  = BURN_ENDIAN_SWAP_INT16(vram[offs]);
			if (data & 0x8000) continue;
	
			INT32 index = (data & 0x7ff0) >> 3;
			UINT32 lut = (BURN_ENDIAN_SWAP_INT16(vlut[index]) << 16) | (BURN_ENDIAN_SWAP_INT16(vlut[index + 1]));

			INT32 code  = (data & 0x0f) | (lut & 0x3fff0);
			INT32 color = (lut >> 20) & 0x1f;
	
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0xe00, DrvGfxROM);
		}
	}
}

static void draw_layer_16x16(UINT8 *ram, INT32 scrolloff)
{
	UINT16 *vram = (UINT16*)ram;
	UINT16 *vlut = (UINT16*)DrvTLUTRAM;
	UINT16 *scrl = (UINT16*)(DrvVidRegs + 0x6a + scrolloff);

	for (INT32 offs = 0; offs < 256 * 256; offs++)
	{
		INT32 sx = (offs & 0xff) << 4;
		INT32 sy = (offs >> 8) << 4;

		sx -= BURN_ENDIAN_SWAP_INT16(scrl[0]) & 0xfff;
		if (sx < -15) sx += 0x1000;

		if (sy >= nScreenHeight || sx >= nScreenWidth) continue;

		INT32 data  = BURN_ENDIAN_SWAP_INT16(vram[offs]);
		if (data & 0x8000) continue;

		INT32 index = (data & 0x7ff0) >> 3;

		UINT32 lu = (BURN_ENDIAN_SWAP_INT16(vlut[index]) << 16) | (BURN_ENDIAN_SWAP_INT16(vlut[index + 1]) << 0);

		INT32 code  = (data & 0x0f) | ((lu >> 2) & 0xfff0);
		INT32 color = (lu >> 20) & 0xff;

		Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM);
	}
}

static void RenderZoomedPriorityTile(UINT16 *dest, UINT8 *gfx, INT32 code, INT32 color, INT32 t, INT32 sx, INT32 sy, INT32 fx, INT32 fy, INT32 width, INT32 height, INT32 zoomx, INT32 zoomy, INT32 prio)
{
	INT32 h = ((zoomy << 4) + 0x8000) >> 16;
	INT32 w = ((zoomx << 4) + 0x8000) >> 16;

	if (!h || !w || sx + w < 0 || sy + h < 0 || sx >= nScreenWidth || sy >= nScreenHeight) return;

	if (fy) fy  = (height-1)*width;
	if (fx) fy |= (width-1);

	INT32 hz = (height << 12) / h;
	INT32 wz = (width << 12) / w;

	INT32 starty = 0, startx = 0, endy = h, endx = w;
	if (sy < 0) starty = 0 - sy;
	if (sx < 0) startx = 0 - sx;
	if (sy + h >= nScreenHeight) endy -= (h + sy) - nScreenHeight;
	if (sx + w >= nScreenWidth ) endx -= (w + sx) - nScreenWidth;

	UINT8  *src = gfx + (code * width * height);
	UINT16 *dst = dest + (sy + starty) * nScreenWidth + sx;
	UINT16 *pri = DrvPriBmp + (sy + starty) * nScreenWidth + sx;

	for (INT32 y = starty; y < endy; y++)
	{
		INT32 zy = ((y * hz) >> 12) * width;

		for (INT32 x = startx; x < endx; x++)
		{
			INT32 pxl = src[(zy + ((x * wz) >> 12)) ^ fy];

			if (pxl != t) {
				if ((pri[x] & prio) == 0) {
					dst[x] = pxl | color;
					pri[x] = prio;
				}
			}
		}

		dst += nScreenWidth;
		pri += nScreenWidth;
	}
}

static void draw_sprites() // metro_draw_sprites
{
	UINT16 *videoregs = (UINT16*)(DrvVidRegs + 0xf00);
	UINT16 *spriteram = (UINT16*)DrvSprRAM;
	UINT8 *gfx_max  = DrvGfxROM + 0x1000000;

	INT32 sprites     = BURN_ENDIAN_SWAP_INT16(videoregs[0x00]) & 0x1ff;
	if (sprites == 0) return;

	INT32 color_start = (BURN_ENDIAN_SWAP_INT16(videoregs[0x04]) & 0x0f) << 4;

	static const INT32 primask[4] = { 0x0000, 0xff00, 0xfff0, 0xfffc };

	static const INT32 zoomtable[0x40] = {
		0xAAC,0x800,0x668,0x554,0x494,0x400,0x390,0x334,
		0x2E8,0x2AC,0x278,0x248,0x224,0x200,0x1E0,0x1C8,
		0x1B0,0x198,0x188,0x174,0x164,0x154,0x148,0x13C,
		0x130,0x124,0x11C,0x110,0x108,0x100,0x0F8,0x0F0,
		0x0EC,0x0E4,0x0DC,0x0D8,0x0D4,0x0CC,0x0C8,0x0C4,
		0x0C0,0x0BC,0x0B8,0x0B4,0x0B0,0x0AC,0x0A8,0x0A4,
		0x0A0,0x09C,0x098,0x094,0x090,0x08C,0x088,0x080,
		0x078,0x070,0x068,0x060,0x058,0x050,0x048,0x040
	};

	UINT16 *src;
	INT32 inc;

	for (INT32 i = 0; i < 0x20; i++)
	{
		if (BURN_ENDIAN_SWAP_INT16(videoregs[0x02/2]) & 0x8000)
		{
			src = spriteram;
			inc = (8 / 2);
		} else {
			src = spriteram + (sprites - 1) * (8 / 2);
			inc = -(8 / 2);
		}

		for (INT32 j = 0; j < sprites; j++)
		{
			INT32 x = BURN_ENDIAN_SWAP_INT16(src[0]);
			INT32 curr_pri = (x & 0xf800) >> 11;

			if ((curr_pri == 0x1f) || (curr_pri != i))
			{
				src += inc;
				continue;
			}

			INT32 pri = (BURN_ENDIAN_SWAP_INT16(videoregs[0x01]) & 0x0300) >> 8;

			if (!(BURN_ENDIAN_SWAP_INT16(videoregs[0x01]) & 0x8000))
			{
				if (curr_pri > (BURN_ENDIAN_SWAP_INT16(videoregs[0x01]) & 0x001f))
					pri = (BURN_ENDIAN_SWAP_INT16(videoregs[0x01]) & 0x0c00) >> 10;
			}

			INT32 y      = BURN_ENDIAN_SWAP_INT16(src[1]);
			INT32 attr   = BURN_ENDIAN_SWAP_INT16(src[2]);
			INT32 code   = BURN_ENDIAN_SWAP_INT16(src[3]);

			INT32 flipx  =   attr & 0x8000;
			INT32 flipy  =   attr & 0x4000;
			INT32 color  =  (attr & 0x00f0) >> 4;

			INT32 width  = (((attr >> 11) & 0x7) + 1) * 8;
			INT32 height = (((attr >>  8) & 0x7) + 1) * 8;

			INT32 zoom = zoomtable[(y & 0xfc00) >> 10] << 8;

			x = (x & 0x07ff)-64;
			y = (y & 0x03ff)-64;

			UINT8 *gfxdata = DrvGfxROM + (32 * (((attr & 0x000f) << 16) + code))*2;

			if ((gfxdata + (width / 2 * height - 1)*2) >= gfx_max)
				continue;

			RenderZoomedPriorityTile(pTransDraw, gfxdata, 0, (color + color_start)<<4, 15, x, y, flipx, flipy, width, height, (zoom*width) >> 4, (zoom*height)>>4, primask[pri]);

			src += inc;
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0x2000; i < 0x4000; i+=2) {
			palette_write(i);
		}
		DrvRecalc = 0;
	}

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = blackpen;
		DrvPriBmp[i] = 0;
	}

	draw_layer_16x16(DrvMD1RAM, 0x10);
	draw_layer_16x16(DrvMD2RAM, 0x00); 
	draw_sprites();
	draw_layer_8x8();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 2 * sizeof(UINT16));
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nTotalCycles = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));

	SekOpen(0);
	SekRun(nTotalCycles);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnSoundOut) {
		es8712Update(0, pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029708;
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

		es8712Scan(0, nAction);
		MSM6295Scan(0, nAction);

		SCAN_VAR(blackpen);

		DrvRecalc = 1;
	}

	return 0;
}


// Varia Metal

static struct BurnRomInfo vmetalRomDesc[] = {
	{ "5b.u19",	0x080000, 0x4933ac6c, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "6b.u18",	0x080000, 0x4eb939d5, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "1.u29",	0x200000, 0xb470c168, 2 | BRF_GRA },           //  2 8x8, 16x16, and Sprite tiles
	{ "2.u31",	0x200000, 0xb36f8d60, 2 | BRF_GRA },           //  3
	{ "3.u28",	0x200000, 0x00fca765, 2 | BRF_GRA },           //  4
	{ "4.u30",	0x200000, 0x5a25a49c, 2 | BRF_GRA },           //  5

	{ "8.u9",	0x080000, 0xc14c001c, 3 | BRF_SND },           //  6 OKI MSM6295 Samples

	{ "7.u12",	0x200000, 0xa88c52f1, 4 | BRF_SND },           //  7 ES8712 Samples
};

STD_ROM_PICK(vmetal)
STD_ROM_FN(vmetal)

struct BurnDriver BurnDrvVmetal = {
	"vmetal", NULL, NULL, NULL, "1995",
	"Varia Metal\0", "Imperfect graphics", "Excellent System", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, vmetalRomInfo, vmetalRomName, NULL, NULL, vmetalInputInfo, vmetalDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	224, 320, 3, 4
};


// Varia Metal (New Ways Trading Co.)

static struct BurnRomInfo vmetalnRomDesc[] = {
	{ "vm5.bin",	0x080000, 0x43ef844e, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "vm6.bin",	0x080000, 0xcb292ab1, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "1.u29",	0x200000, 0xb470c168, 2 | BRF_GRA },           //  2 8x8, 16x16, and Sprite tiles
	{ "2.u31",	0x200000, 0xb36f8d60, 2 | BRF_GRA },           //  3
	{ "3.u28",	0x200000, 0x00fca765, 2 | BRF_GRA },           //  4
	{ "4.u30",	0x200000, 0x5a25a49c, 2 | BRF_GRA },           //  5

	{ "8.u9",	0x080000, 0xc14c001c, 3 | BRF_SND },           //  6 OKI MSM6295 Samples

	{ "7.u12",	0x200000, 0xa88c52f1, 4 | BRF_SND },           //  7 ES8712 Samples
};

STD_ROM_PICK(vmetaln)
STD_ROM_FN(vmetaln)

struct BurnDriver BurnDrvVmetaln = {
	"vmetaln", "vmetal", NULL, NULL, "1995",
	"Varia Metal (New Ways Trading Co.)\0", "Imperfect graphics", "Excellent System (New Ways Trading Co. license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, vmetalnRomInfo, vmetalnRomName, NULL, NULL, vmetalInputInfo, vmetalDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	224, 320, 3, 4
};
