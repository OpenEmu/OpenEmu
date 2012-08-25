// FB Alpha Dark Seal driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "h6280_intf.h"
#include "bitswap.h"
#include "deco16ic.h"
#include "burn_ym2203.h"
#include "burn_ym2151.h"
#include "msm6295.h"
#include "timer.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvHucROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *Drv68KRAM;
static UINT8 *DrvHucRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvPalRAM;
static UINT8 *DrvPf3RAM;
static UINT8 *DrvPf2RAM;
static UINT8 *DrvPf1RAM;
static UINT8 *DrvPf12RowRAM;
static UINT8 *DrvPf34RowRAM;
static UINT8 *DrvPfCtrlRAM0;
static UINT8 *DrvPfCtrlRAM1;
static UINT32 *DrvPalette;

static INT16 *SoundBuffer;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDip[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static UINT8 DrvRecalc = 0;
static INT32 vblank = 0;

static struct BurnInputInfo DarksealInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
};

STDINPUTINFO(Darkseal)

static struct BurnDIPInfo DarksealDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x07, 0x00, "3 Coins 1 Credit"	},
	{0x11, 0x01, 0x07, 0x01, "2 Coins 1 Credit"	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credit"	},
	{0x11, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x11, 0x01, 0x38, 0x00, "3 Coins 1 Credit"	},
	{0x11, 0x01, 0x38, 0x08, "2 Coins 1 Credit"	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin  1 Credit"	},
	{0x11, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x11, 0x01, 0x40, 0x40, "Off"			},
	{0x11, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Lives"		},
	{0x12, 0x01, 0x03, 0x00, "1"			},
	{0x12, 0x01, 0x03, 0x01, "2"			},
	{0x12, 0x01, 0x03, 0x03, "3"			},
	{0x12, 0x01, 0x03, 0x02, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x0c, 0x08, "Easy"			},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x12, 0x01, 0x0c, 0x04, "Hard"			},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Energy"		},
	{0x12, 0x01, 0x30, 0x00, "2"			},
	{0x12, 0x01, 0x30, 0x10, "2.5"			},
	{0x12, 0x01, 0x30, 0x30, "3"			},
	{0x12, 0x01, 0x30, 0x20, "4"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"	},
	{0x12, 0x01, 0x40, 0x00, "No"			},
	{0x12, 0x01, 0x40, 0x40, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Darkseal)

static inline void palette_write(INT32 offset)
{
	UINT16 *data = (UINT16*)(DrvPalRAM + offset);
	UINT8 r,g,b;

	r = (BURN_ENDIAN_SWAP_INT16(data[0x0000/2]) >> 0) & 0xff;
	g = (BURN_ENDIAN_SWAP_INT16(data[0x0000/2]) >> 8) & 0xff;
	b = (BURN_ENDIAN_SWAP_INT16(data[0x1000/2]) >> 0) & 0xff;

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

void __fastcall darkseal_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfffff0) == 0x180000) {
		switch (address & 0x0e)
		{
			case 0x06:
				memcpy (DrvSprBuf, DrvSprRAM, 0x800);
			return;

			case 0x08:
				deco16_soundlatch = data;
				h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
			return;
		}

		return;
	}
}

void __fastcall darkseal_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff0) == 0x180000) {
		switch (address & 0x0f)
		{
			case 0x06:
				memcpy (DrvSprBuf, DrvSprRAM, 0x800);
			return;

			case 0x08:
				deco16_soundlatch = data & 0xff;
				h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
			return;
		}

		return;
	}

	if ((address & 0xfffff0) == 0x240000) {
		*((UINT16*)(DrvPfCtrlRAM0 + (address & 0x0e))) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}

	if ((address & 0xfffff0) == 0x2a0000) {
		*((UINT16*)(DrvPfCtrlRAM1 + (address & 0x0e))) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
}

UINT8 __fastcall darkseal_read_byte(UINT32 address)
{
	if ((address & 0xfffff0) == 0x180000) {
		switch (address & 0xf)
		{
			case 0:
				return DrvDip[1];

			case 1:
				return DrvDip[0];

			case 2:
				return DrvInputs[0] >> 8;

			case 3:
				return DrvInputs[0] & 0xff;

			case 4:
				return 0xff;

			case 5:
				return (DrvInputs[1] ^ vblank) & 0xff;
		}

		return 0xff;
	}

	return 0;
}

UINT16 __fastcall darkseal_read_word(UINT32 address)
{
	if ((address & 0xfffff0) == 0x180000) {
		switch (address & 0xe)
		{
			case 0:
				return (DrvDip[0] | (DrvDip[1]<<8));

			case 2:
				return DrvInputs[0];

			case 4:
				return DrvInputs[1] ^ vblank;
		}

		return 0xffff;
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
	
	deco16SoundReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;

	DrvHucROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x040000;
	DrvGfxROM1	= Next; Next += 0x100000;
	DrvGfxROM2	= Next; Next += 0x100000;
	DrvGfxROM3	= Next; Next += 0x200000;

	MSM6295ROM	= Next; Next += 0x140000;

	DrvPalette	= (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvHucRAM	= Next; Next += 0x002000;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvSprBuf	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x002000;
	DrvPf3RAM	= Next; Next += 0x002000;
	DrvPf12RowRAM	= Next; Next += 0x001000;
	DrvPf34RowRAM	= Next; Next += 0x001000;
	DrvPf2RAM	= Next; Next += 0x002000;
	DrvPf1RAM	= Next; Next += 0x002000;
	DrvPfCtrlRAM0	= Next; Next += 0x000010;
	DrvPfCtrlRAM1	= Next; Next += 0x000010;

	RamEnd		= Next;
	
	SoundBuffer = (INT16*)Next; Next += nBurnSoundLen * 2 * sizeof(INT16);

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4]  = { 0x00000*8, 0x10000*8, 0x8000*8, 0x18000*8 };
	INT32 Plane1[4]  = { 8, 0, 0x40000*8+8, 0x40000*8 };
	INT32 Plane2[4]  = { 8, 0, 0x80000*8+8, 0x80000*8 };
	INT32 XOffs0[8]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs0[8]  = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 };
	INT32 XOffs1[16] = { 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7, 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs1[16] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x020000);

	GfxDecode(0x1000, 4,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x080000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x080000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x100000);

	GfxDecode(0x2000, 4, 16, 16, Plane2, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM3);

	BurnFree (tmp);

	return 0;
}

static void DrvPrgDecode()
{
	for (INT32 i = 0; i < 0x80000; i++)
		Drv68KROM[i] = BITSWAP08(Drv68KROM[i], 7, 1, 5, 4, 3, 2, 6, 0);
}

static INT32 DrvInit()
{
	BurnSetRefreshRate(58.00);
	
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM + 0x00001,	0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x00000,	1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x40001,	2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x40000,	3, 2)) return 1;

		if (BurnLoadRom(DrvHucROM,		4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,	5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x10000,	6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,	7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x00000,	8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x00000,	9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x80000,  10, 1)) return 1;

		if (BurnLoadRom(MSM6295ROM + 0x000000,   11, 1)) return 1;
		if (BurnLoadRom(MSM6295ROM + 0x100000,   12, 1)) return 1;

		DrvPrgDecode();
		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x100000, 0x103fff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x120000, 0x1207ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x140000, 0x141fff, SM_RAM); // split ram
	SekMapMemory(DrvPf3RAM + 0x001000,	0x200000, 0x200fff, SM_RAM); // mirror
	SekMapMemory(DrvPf3RAM,			0x202000, 0x203fff, SM_RAM);
	SekMapMemory(DrvPf12RowRAM,		0x220000, 0x220fff, SM_RAM);
	SekMapMemory(DrvPf34RowRAM,		0x222000, 0x222fff, SM_RAM);
	SekMapMemory(DrvPf2RAM,			0x260000, 0x261fff, SM_RAM);
	SekMapMemory(DrvPf1RAM,			0x262000, 0x263fff, SM_RAM);
	SekSetWriteWordHandler(0,		darkseal_write_word);
	SekSetWriteByteHandler(0,		darkseal_write_byte);
	SekSetReadWordHandler(0,		darkseal_read_word);
	SekSetReadByteHandler(0,		darkseal_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 8055000, 1, NULL, 0.55, 1006875, 1.00, 2013750, 0.60);
	BurnYM2203SetAllRoutes(0, 0.45, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();
	
	deco16SoundExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites()
{
	UINT16 *SprRAM = (UINT16*)DrvSprBuf;

	for (INT32 offs = 0; offs < 0x400; offs += 4)
	{
		INT32 sprite = BURN_ENDIAN_SWAP_INT16(SprRAM[offs+1]) & 0x1fff;
		if (!sprite) continue;

		INT32 y = BURN_ENDIAN_SWAP_INT16(SprRAM[offs]);
		INT32 x = BURN_ENDIAN_SWAP_INT16(SprRAM[offs+2]);

		INT32 flash = ((y >> 12) & 1) & GetCurrentFrame();
		if (flash) continue;

		INT32 color = ((x >> 9) & 0x1f) + 0x10;

		INT32 fx = y & 0x2000;
		INT32 fy = y & 0x4000;
		INT32 multi = (1 << ((y & 0x0600) >> 9)) - 1;

		x &= 0x01ff;
		y &= 0x01ff;
		if (x > 255) x -= 512;
		if (y > 255) y -= 512;
		x =  240 - x;
		y = (240 - y) - 8;

		if (x > 256) continue;

		sprite &= ~multi;

		INT32 inc = -1;

		if (!fy) {
			sprite += multi;
			inc = 1;
		}

		while (multi >= 0)
		{
			if (fy) {
				if (fx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sprite - multi * inc, x, y+-16*multi, color, 4, 0, 0, DrvGfxROM3);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sprite - multi * inc, x, y+-16*multi, color, 4, 0, 0, DrvGfxROM3);
				}
			} else {
				if (fx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sprite - multi * inc, x, y+-16*multi, color, 4, 0, 0, DrvGfxROM3);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, sprite - multi * inc, x, y+-16*multi, color, 4, 0, 0, DrvGfxROM3);
				}
			}

			multi--;
		}
	}
}

static void draw_pf1_layer(INT32 scroll_x, INT32 scroll_y)
{
	UINT16 *vram = (UINT16*)DrvPf1RAM;

	for (INT32 offs = 0; offs < 64 * 64; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		sx -= scroll_x;
		if (sx < -7) sx += 0x200;
		sy -= scroll_y + 8;
		if (sy < -7) sy += 0x200;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs]);
		INT32 color = code >> 12;

		code &= 0xfff;
		if (!code) continue;

		if (sy >= 0 && sx >= 0 && sy <= nScreenHeight-8 && sx <= nScreenWidth-8) {
			Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
		}
	}
}

static void draw_pf23_layer_no_rowscroll(UINT8 *ram, UINT8 *gfx_base, INT32 coloffs, INT32 transp, INT32 scroll_x, INT32 scroll_y)
{
	UINT16 *vram = (UINT16*)ram;

	for (INT32 offs = 0; offs < 64 * 64; offs++)
	{
		INT32 sx = (offs & 0x3f);
		INT32 sy = (offs >> 6);

		INT32 ofst = (sx & 0x1f) + ((sy & 0x1f) << 5) + ((sx & 0x20) << 5) + ((sy & 0x20) << 6);

		sx <<= 4, sy <<= 4;

		sx -= scroll_x;
		if (sx < -15) sx += 0x400;
		sy -= scroll_y + 8;
		if (sy < -15) sy += 0x400;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[ofst]);
		INT32 color = code >> 12;

		code &= 0xfff;
		if (!code && transp) continue;

		if (transp) {
			if (sy >= 0 && sx >= 0 && sy <= nScreenHeight-16 && sx <= nScreenWidth-16) {
				Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, coloffs, gfx_base);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, coloffs, gfx_base);
			}
		} else {
			if (sy >= 0 && sx >= 0 && sy <= nScreenHeight-16 && sx <= nScreenWidth-16) {
				Render16x16Tile(pTransDraw, code, sx, sy, color, 4, coloffs, gfx_base);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, coloffs, gfx_base);
			}
		}
	}
}

static void draw_pf23_layer_rowscroll(INT32 scroll_x, INT32 scroll_y)
{
	UINT16 *vram = (UINT16*)DrvPf3RAM;
	UINT16 *rows = (UINT16*)DrvPf34RowRAM;
	UINT16 *dest;

	for (INT32 y = 8; y < 248; y++)
	{
		INT32 row = (scroll_y + y) >> 4;
		INT32 xscr = scroll_x + (BURN_ENDIAN_SWAP_INT16(rows[0x40+y]) & 0x3ff);
		dest = pTransDraw + ((y-8) * nScreenWidth);

		for (INT32 x = 0; x < 256+16; x+=16)
		{
			INT32 col = ((xscr + x) & 0x3ff) >> 4;
			INT32 sx = x - (xscr & 0x0f);
			if (sx < -15) sx += 0x400;
			if (sx < 0) sx += 16;

			INT32 ofst = (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);

			INT32 code  =   BURN_ENDIAN_SWAP_INT16(vram[ofst]) & 0xfff;
			INT32 color = ((BURN_ENDIAN_SWAP_INT16(vram[ofst]) >> 12) << 4) | 0x400;
			
			UINT8 *src = DrvGfxROM2 + (code << 8) + (((scroll_y + y) & 0x0f) << 4);
			for (INT32 xx = sx; xx < (((sx+16) < nScreenWidth) ? (sx+16) : nScreenWidth); xx++) {
				if (xx >= nScreenWidth) break;
				dest[xx] = src[xx-sx] | color;
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x1000; i+=2) {
			palette_write(i);
		}
	}

	UINT16 *ctrl0 = (UINT16*)DrvPfCtrlRAM0;
	UINT16 *ctrl1 = (UINT16*)DrvPfCtrlRAM1;

	INT32 flipscreen = ~BURN_ENDIAN_SWAP_INT16(ctrl0[0]) & 0x80;

	if (ctrl0[6] & 0x4000) {
		draw_pf23_layer_rowscroll(BURN_ENDIAN_SWAP_INT16(ctrl0[3]) & 0x3ff, BURN_ENDIAN_SWAP_INT16(ctrl0[4]) & 0x3ff);
	} else {
		draw_pf23_layer_no_rowscroll(DrvPf3RAM, DrvGfxROM2, 0x400, 0, BURN_ENDIAN_SWAP_INT16(ctrl0[3]) & 0x3ff, BURN_ENDIAN_SWAP_INT16(ctrl0[4]) & 0x3ff);
	}

	draw_pf23_layer_no_rowscroll(DrvPf2RAM, DrvGfxROM1, 0x300, 1, BURN_ENDIAN_SWAP_INT16(ctrl1[1]) & 0x3ff, BURN_ENDIAN_SWAP_INT16(ctrl1[2]) & 0x3ff);

	draw_sprites();

	draw_pf1_layer(BURN_ENDIAN_SWAP_INT16(ctrl1[3]) & 0x1ff, BURN_ENDIAN_SWAP_INT16(ctrl1[4]) & 0x1ff);

	if (flipscreen) {
		INT32 full = nScreenWidth * nScreenHeight;
		for (INT32 i = 0; i < full / 2; i++) {
			INT32 t = pTransDraw[i + 0];
			pTransDraw[i + 0] = pTransDraw[(full-1)-i];
			pTransDraw[(full-1)-i] = t;
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
		memset (DrvInputs, 0xff, 2 * sizeof(UINT16));

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nInterleave = 256;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 12000000 / 58, 8055000 / 58 };
	INT32 nCyclesDone[2] = { 0, 0 };

	h6280NewFrame();
	
	SekOpen(0);
	h6280Open(0);

	vblank = 8;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		nCyclesDone[1] += h6280Run(nCyclesTotal[1] / nInterleave);

		if (i ==   7) vblank = 0;
		if (i == 247) vblank = 8;
		
		INT32 nSegmentLength = nBurnSoundLen / nInterleave;
		INT16* pSoundBuf = SoundBuffer + (nSoundBufferPos << 1);
		deco16SoundUpdate(pSoundBuf, nSegmentLength);
		nSoundBufferPos += nSegmentLength;
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	BurnTimerEndFrame(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = SoundBuffer + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
		}
		
		for (INT32 i = 0; i < nBurnSoundLen; i++) {
			pBurnSoundOut[(i << 1) + 0] += SoundBuffer[(i << 1) + 0];
			pBurnSoundOut[(i << 1) + 1] += SoundBuffer[(i << 1) + 1];
		}
	}

	h6280Close();	
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029722;
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
		
		deco16SoundScan(nAction, pnMin);
		
		deco16Scan();
		
		SCAN_VAR(vblank);
	}

	return 0;
}


// Dark Seal (World revision 3)

static struct BurnRomInfo darksealRomDesc[] = {
	{ "ga_04-3.j12",0x20000, 0xbafad556, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ga_01-3.h14",0x20000, 0xf409050e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ga_00.h12",	0x20000, 0xfbf3ac63, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ga_05.j14",	0x20000, 0xd5e3ae3f, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz_06-1.j15",0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz_02.j1",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz_03.j2",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.h3",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.e20",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.b1",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.b3",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz_08.l17",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz_07.k14",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(darkseal)
STD_ROM_FN(darkseal)

struct BurnDriver BurnDrvDarkseal = {
	"darkseal", NULL, NULL, NULL, "1990",
	"Dark Seal (World revision 3)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, darksealRomInfo, darksealRomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Dark Seal (World revision 1)

static struct BurnRomInfo darksea1RomDesc[] = {
	{ "ga_04.j12",	0x20000, 0xa1a985a9, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ga_01.h14",	0x20000, 0x98bd2940, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ga_00.h12",	0x20000, 0xfbf3ac63, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ga_05.j14",	0x20000, 0xd5e3ae3f, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz_06-1.j15",0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz_02.j1",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz_03.j2",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.h3",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.e20",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.b1",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.b3",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz_08.l17",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz_07.k14",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(darksea1)
STD_ROM_FN(darksea1)

struct BurnDriver BurnDrvDarksea1 = {
	"darkseal1", "darkseal", NULL, NULL, "1990",
	"Dark Seal (World revision 1)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, darksea1RomInfo, darksea1RomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Dark Seal (Japan)

static struct BurnRomInfo darkseajRomDesc[] = {
	{ "fz_04-4.j12",0x20000, 0x817faa2c, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fz_01-4.h14",0x20000, 0x373caeee, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "fz_00-2.h12",0x20000, 0x1ab99aa7, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "fz_05-2.j14",0x20000, 0x3374ef8c, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz_06-1.j15",0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz_02.j1",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz_03.j2",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.h3",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.e20",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.b1",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.b3",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz_08.l17",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz_07.k14",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(darkseaj)
STD_ROM_FN(darkseaj)

struct BurnDriver BurnDrvDarkseaj = {
	"darksealj", "darkseal", NULL, NULL, "1990",
	"Dark Seal (Japan)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, darkseajRomInfo, darkseajRomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Gate of Doom (US revision 4)

static struct BurnRomInfo gatedoomRomDesc[] = {
	{ "gb_04-4.j12",0x20000, 0x8e3a0bfd, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gb_01-4.h14",0x20000, 0x8d0fd383, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gb_00.h12",	0x20000, 0xa88c16a1, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gb_05.j14",	0x20000, 0x252d7e14, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz_06-1.j15",0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz_02.j1",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz_03.j2",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.h3",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.e20",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.b1",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.b3",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz_08.l17",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz_07.k14",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(gatedoom)
STD_ROM_FN(gatedoom)

struct BurnDriver BurnDrvGatedoom = {
	"gatedoom", "darkseal", NULL, NULL, "1990",
	"Gate of Doom (US revision 4)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, gatedoomRomInfo, gatedoomRomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Gate of Doom (US revision 1)

static struct BurnRomInfo gatedom1RomDesc[] = {
	{ "gb_04.j12",	0x20000, 0x4c3bbd2b, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gb_01.h14",	0x20000, 0x59e367f4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gb_00.h12",	0x20000, 0xa88c16a1, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gb_05.j14",	0x20000, 0x252d7e14, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz_06-1.j15",0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz_02.j1",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz_03.j2",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.h3",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.e20",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.b1",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.b3",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz_08.l17",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz_07.k14",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(gatedom1)
STD_ROM_FN(gatedom1)

struct BurnDriver BurnDrvGatedom1 = {
	"gatedoom1", "darkseal", NULL, NULL, "1990",
	"Gate of Doom (US revision 1)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, gatedom1RomInfo, gatedom1RomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};
