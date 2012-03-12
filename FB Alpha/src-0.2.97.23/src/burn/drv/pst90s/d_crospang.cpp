// FB Alpha Cross Pang driver module
// Based on MAME driver by Pierpaolo Prazzoli and David Haywood

#include "tiles_generic.h"
#include "msm6295.h"
#include "burn_ym3812.h"

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[3];

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvSndROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvFgRAM;
static UINT8 *DrvBgRAM;
static UINT8 *DrvSprRAM;
static UINT32 *DrvPalette;

static UINT8 *soundlatch;
static UINT8 *tile_bank;
static UINT16 *fg_scroll_x;
static UINT16 *fg_scroll_y;
static UINT16 *bg_scroll_x;
static UINT16 *bg_scroll_y;

static UINT8 DrvRecalc;

static INT32 bestri = 0;

static struct BurnInputInfo CrospangInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 8,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Crospang)

static struct BurnDIPInfo CrospangDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x7e, NULL			},
	{0x12, 0xff, 0xff, 0xf7, NULL			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x11, 0x01, 0x03, 0x00, "3 Coins 1 Credit"	},
	{0x11, 0x01, 0x03, 0x01, "2 Coins 1 Credit"	},
	{0x11, 0x01, 0x03, 0x02, "1 Coin 1 Credit"	},
	{0x11, 0x01, 0x03, 0x03, "1 Coin 2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Number of Powers"	},
	{0x11, 0x01, 0x20, 0x00, "1"			},
	{0x11, 0x01, 0x20, 0x20, "2"			},

	{0   , 0xfe, 0   ,    4, "Extra Balls"		},
	{0x11, 0x01, 0xc0, 0xc0, "1"			},
	{0x11, 0x01, 0xc0, 0x80, "2"			},
	{0x11, 0x01, 0xc0, 0x40, "3"			},
	{0x11, 0x01, 0xc0, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Minimum Balls per Row"},
	{0x12, 0x01, 0x18, 0x18, "3"			},
	{0x12, 0x01, 0x18, 0x10, "4"			},
	{0x12, 0x01, 0x18, 0x08, "5"			},
	{0x12, 0x01, 0x18, 0x00, "6"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x40, 0x40, "Off"			},
	{0x12, 0x01, 0x40, 0x00, "On"			},
};

STDDIPINFO(Crospang)

static struct BurnDIPInfo HeuksunDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfe, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x11, 0x01, 0x03, 0x00, "3 Coins 1 Credit"	},
	{0x11, 0x01, 0x03, 0x01, "2 Coins 1 Credit"	},
	{0x11, 0x01, 0x03, 0x02, "1 Coin 1 Credit"	},
	{0x11, 0x01, 0x03, 0x03, "1 Coin 2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Heuksun)

static struct BurnDIPInfo BestriDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfb, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x01, 0x01, "Off"			},
	{0x11, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x11, 0x01, 0x06, 0x00, "3 Coins 1 Credit"	},
	{0x11, 0x01, 0x06, 0x04, "2 Coins 1 Credit"	},
	{0x11, 0x01, 0x06, 0x02, "1 Coin 1 Credit"	},
	{0x11, 0x01, 0x06, 0x06, "1 Coin 2 Credits"	},
};

STDDIPINFO(Bestri)

void __fastcall crospang_write_byte(UINT32 address, UINT8 data)
{
	if (bestri == 0) return;

	switch (address)
	{
		case 0x10000e:
			*tile_bank = (data >> 2) & 0x0f;
		return;
	}
}

void __fastcall crospang_write_word(UINT32 address, UINT16 data)
{
	if (bestri == 0) {
		switch (address)
		{
			case 0x100002:
				*fg_scroll_y = (data + 8) & 0x1ff;
			return;
	
			case 0x100004:
				*bg_scroll_x = (data + 4) & 0x1ff;
			return;
	
			case 0x100006:
				*bg_scroll_y = (data + 8) & 0x1ff;
			return;
	
			case 0x100008:
				*fg_scroll_x = (data + 0) & 0x1ff;
			return;
			
			case 0x270000:
				*soundlatch = data & 0xff;
			return;
		}
	} else {
		switch (address)
		{
			case 0x100004:
				*fg_scroll_x = ((data ^ 0x0000) + 32) & 0x1ff;
			return;
	
			case 0x100006:
				*fg_scroll_y = ((data ^ 0xff54) +  7) & 0x1ff;
			return;
	
			case 0x10000a:
				*bg_scroll_y = ((data ^ 0xfeaa) +  7) & 0x1ff;
			return;
	
			case 0x10000c:
				*bg_scroll_x = ((data ^ 0x0000) - 60) & 0x1ff;
			return;
	
			case 0x270000:
				*soundlatch = data & 0xff;
			return;
		}
	}
}

UINT8 __fastcall crospang_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x280002:
			return DrvInputs[1] >> 8;
	}

	return 0;
}

UINT16 __fastcall crospang_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x280000:
		case 0x280002:
		case 0x280004:
			return DrvInputs[(address >> 1) & 3];
	}

	return 0;
}

void __fastcall crospang_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			BurnYM3812Write(0, data);
		return;

		case 0x01:
			BurnYM3812Write(1, data);
		return;

		case 0x02:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall crospang_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return BurnYM3812Read(0);

		case 0x02:
			return MSM6295ReadStatus(0);

		case 0x06:
			return *soundlatch;
	}

	return 0;
}

inline static INT32 crospangSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(ZetTotalCycles() * nSoundRate / 3579545);
}

void crospangYM3812IrqHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM3812Reset();
	MSM6295Reset(0);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x100000;
	DrvZ80ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x400000;
	DrvGfxROM1	= Next; Next += 0x400000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x040000;

	DrvPalette	= (UINT32*)Next; Next += 0x0300 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvFgRAM	= Next; Next += 0x000800;
	DrvBgRAM	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x000800;

	DrvZ80RAM	= Next; Next += 0x000800;

	soundlatch	= Next; Next += 0x000001;

	tile_bank	= Next; Next += 0x000001;

	fg_scroll_x	= (UINT16*)Next; Next += 0x0001 * sizeof (UINT16);
	bg_scroll_x	= (UINT16*)Next; Next += 0x0001 * sizeof (UINT16);
	fg_scroll_y	= (UINT16*)Next; Next += 0x0001 * sizeof (UINT16);
	bg_scroll_y	= (UINT16*)Next; Next += 0x0001 * sizeof (UINT16);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode(INT32 gfx0len, INT32 gfx1len, INT32 type)
{
	INT32 Plane0[4] = { ((gfx0len / 2) * 8) + 8,  ((gfx0len / 2) * 8) + 0,  8,  0 };
	INT32 Plane1[4] = { ((gfx1len / 2) * 8) + 8,  ((gfx1len / 2) * 8) + 0,  8,  0 };
	INT32 XOffs[16] = { 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			       0,      1,      2,      3,      4,      5,      6,      7 };
	INT32 YOffs[16] = { 0*16, 1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
			  8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 };

	UINT8 *tmp = (UINT8*)BurnMalloc(((gfx0len - 1) | (gfx1len - 1)) + 1);
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < gfx0len; i++) { // rearrange tiles a bit
		INT32 k = (type) ? (((i & 0x40000) << 2) | ((i & 0x180000) >> 1) | (i & 0x3ffff)) : i;
		tmp[k ^ (gfx0len/2) ^ 0x20] = DrvGfxROM0[i];
	}

	GfxDecode((gfx0len * 2) / (16 * 16), 4, 16, 16, Plane0, XOffs, YOffs, 0x200, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, gfx1len);

	GfxDecode((gfx1len * 2) / (16 * 16), 4, 16, 16, Plane1, XOffs, YOffs, 0x200, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 crospangLoadRoms()
{
	if (BurnLoadRom(DrvGfxROM1 + 0x000000,	6, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x000001,	7, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000,	8, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100001,	9, 2)) return 1;

	DrvGfxDecode(0x80000, 0x200000, 0);

	return 0;
}

static INT32 heuksunLoadRoms()
{
	if (BurnLoadRom(DrvGfxROM1 + 0x000000,	6, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x000001,	7, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x080000,	8, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x080001,	9, 2)) return 1;

	DrvGfxDecode(0x200000, 0x100000, 1);

	return 0;
}

static INT32 bestriLoadRoms()
{
	if (BurnLoadRom(DrvGfxROM0 + 0x100000,	6, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x100001,	7, 2)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,	8, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x000001,	9, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000, 10, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100001, 11, 2)) return 1;

	DrvGfxDecode(0x200000, 0x200000, 1);

	return 0;
}

static INT32 DrvInit(INT32 (*pRomLoadCallback)())
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM + 0x000000,	0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x000001,	1, 2)) return 1;
	
		if (BurnLoadRom(DrvZ80ROM,		2, 1)) return 1;
	
		if (BurnLoadRom(DrvSndROM,		3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,	4, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,	5, 2)) return 1;

		if (pRomLoadCallback) {
			if (pRomLoadCallback()) return 1;
		}
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvFgRAM,		0x120000, 0x1207ff, SM_RAM);
	SekMapMemory(DrvBgRAM,		0x122000, 0x1227ff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x200000, 0x2005ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x210000, 0x2107ff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0x320000, 0x32ffff, SM_RAM); // crospang, heuksun
	SekMapMemory(Drv68KRAM,		0x3a0000, 0x3affff, SM_RAM); // bestri
	SekSetWriteByteHandler(0,	crospang_write_byte);
	SekSetWriteWordHandler(0,	crospang_write_word);
	SekSetReadByteHandler(0,	crospang_read_byte);
	SekSetReadWordHandler(0,	crospang_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM);
	ZetSetOutHandler(crospang_sound_out);
	ZetSetInHandler(crospang_sound_in);
	ZetMemEnd();
	ZetClose();

	BurnYM3812Init(3579545, &crospangYM3812IrqHandler, crospangSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(3579545);

	MSM6295Init(0, 1056000 / 132, 80.0, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	BurnYM3812Exit();
	MSM6295Exit(0);
	SekExit();
	ZetExit();

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	bestri = 0;

	return 0;
}

static void draw_layer(UINT8 *src, INT32 coloffs, INT32 scrollx, INT32 scrolly, INT32 transp)
{
	UINT16 *vram = (UINT16*)src;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx -= scrollx;
		if (sx < -15) sx += 0x200;
		sy -= scrolly;
		if (sy < -15) sy += 0x200;

		if (sy >= nScreenHeight || sx >= nScreenWidth) continue;

		INT32 data  = BURN_ENDIAN_SWAP_INT16(vram[offs]);
		INT32 code  = (data & 0xfff) + (*tile_bank << 12);
		INT32 color = (data >> 12) + coloffs;

		if (transp) {
			if (!(code&0xfff)) continue;

			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
		} else {
			Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
		}
	}
}

static void draw_sprites()
{
	UINT16 *ram = (UINT16*)DrvSprRAM;

	for (INT32 offs = 0;offs < 0x400;offs += 4)
	{
		INT32 sprite = BURN_ENDIAN_SWAP_INT16(ram[offs + 1]) & 0x7fff;
		if (!sprite) continue;

		INT32 y = BURN_ENDIAN_SWAP_INT16(ram[offs]);
		if ((y & 0x1000) && (GetCurrentFrame() & 1)) continue; // flash

		INT32 x = BURN_ENDIAN_SWAP_INT16(ram[offs + 2]);
		INT32 color = (x >>9) & 0xf;

		INT32 fx = y & 0x2000;
		INT32 fy = y & 0x4000;
		INT32 multi = (1 << ((y & 0x0600) >> 9)) - 1;

		x &= 0x01ff;
		y &= 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = (240 - y) - 7;
		x = (304 - x) - 5;

		INT32 inc = -1;

		if (!fy)
		{
			sprite += multi;
			inc = 1;
		}

		while (multi >= 0)
		{
			if (fy) {
				if (fx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sprite - multi * inc, x, y - 16 * multi, color, 4, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sprite - multi * inc, x, y - 16 * multi, color, 4, 0, 0, DrvGfxROM1);
				}
			} else {
				if (fx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sprite - multi * inc, x, y - 16 * multi, color, 4, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, sprite - multi * inc, x, y - 16 * multi, color, 4, 0, 0, DrvGfxROM1);
				}
			}

			multi--;
		}
	}
}
	
static INT32 DrvDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		UINT16 *pal = (UINT16*)DrvPalRAM;
		for (INT32 i = 0; i < 0x300; i++) {
			r = (BURN_ENDIAN_SWAP_INT16(pal[i]) >> 10) & 0x1f;
			g = (BURN_ENDIAN_SWAP_INT16(pal[i]) >>  5) & 0x1f;
			b = (BURN_ENDIAN_SWAP_INT16(pal[i]) >>  0) & 0x1f;

			r = (r << 3) | (r >> 2);
			g = (g << 3) | (g >> 2);
			b = (b << 3) | (b >> 2);

			DrvPalette[i] = BurnHighCol(r, g, b, 0);
		}
	}

	draw_layer(DrvBgRAM, 0x20, *bg_scroll_x, *bg_scroll_y, 0);
	draw_layer(DrvFgRAM, 0x10, *fg_scroll_x, *fg_scroll_y, 1);

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
		DrvInputs[0] = DrvInputs[1] = 0xffff;

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		DrvInputs[2] = (DrvDips[1] << 8) | DrvDips[0];
	}

	INT32 nTotalCycles[2] = { 7159090 / 60, 3579545 / 60 };

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);

	SekRun(nTotalCycles[0]);
	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		BurnTimerEndFrameYM3812(nTotalCycles[1]);
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029698;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		BurnYM3812Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);
	}

	return 0;
}


// Cross Pang

static struct BurnRomInfo crospangRomDesc[] = {
	{ "p1.bin",	0x20000, 0x0bcbbaad, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "p2.bin",	0x20000, 0x0947d204, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "s1.bin",	0x10000, 0xd61a224c, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "s2.bin",	0x20000, 0x9f9ecd22, 3 | BRF_SND },           //  3 Oki Samples

	{ "rom1.bin",	0x40000, 0x905042bb, 4 | BRF_GRA },           //  4 Background Tiles
	{ "rom2.bin",	0x40000, 0xbc4381e9, 4 | BRF_GRA },           //  5

	{ "rom3.bin",	0x80000, 0xcc6e1fce, 5 | BRF_GRA },           //  6 Sprites
	{ "rom4.bin",	0x80000, 0x9a91d494, 5 | BRF_GRA },           //  7
	{ "rom5.bin",	0x80000, 0x53a34dc5, 5 | BRF_GRA },           //  8
	{ "rom6.bin",	0x80000, 0x9c633082, 5 | BRF_GRA },           //  9
};

STD_ROM_PICK(crospang)
STD_ROM_FN(crospang)

static INT32 crospangInit()
{
	return DrvInit(crospangLoadRoms);
}

struct BurnDriver BurnDrvCrospang = {
	"crospang", NULL, NULL, NULL, "1998",
	"Cross Pang\0", NULL, "F2 System", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, crospangRomInfo, crospangRomName, NULL, NULL, CrospangInputInfo, CrospangDIPInfo,
	crospangInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	320, 240, 4, 3
};


// Heuk Sun Baek Sa (Korea)

static struct BurnRomInfo heuksunRomDesc[] = {
	{ "ua02.j3",	0x80000, 0xdb2b9c8e, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ua03.j5",	0x80000, 0xde9f01e8, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "us02.r4",	0x10000, 0xc7cc05fa, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "us08.u7",	0x40000, 0xae177589, 3 | BRF_SND },           //  3 Oki Samples

	{ "uc07.t11",	0x80000, 0x4d1ed885, 4 | BRF_GRA },           //  4 Background Tiles
	{ "uc08.r11",	0x80000, 0x242cee69, 4 | BRF_GRA },           //  5

	{ "ud14.p11",	0x40000, 0x4fc2b574, 5 | BRF_GRA },           //  6 Sprites
	{ "ud15.m11",	0x40000, 0x1d6187a6, 5 | BRF_GRA },           //  7
	{ "ud16.l11",	0x40000, 0xeabec43e, 5 | BRF_GRA },           //  8
	{ "ud17.j11",	0x40000, 0xc6b04171, 5 | BRF_GRA },           //  9
};

STD_ROM_PICK(heuksun)
STD_ROM_FN(heuksun)

static INT32 heuksunInit()
{
	return DrvInit(heuksunLoadRoms);
}

struct BurnDriver BurnDrvHeuksun = {
	"heuksun", NULL, NULL, NULL, "199?",
	"Heuk Sun Baek Sa (Korea)\0", NULL, "Oksan / F2 System", "Miscellaneous",
	L"\uD751\uC120\uBC31\uC0AC (Korea)\0Heuk Sun Baek Sa\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MISC, 0,
	NULL, heuksunRomInfo, heuksunRomName, NULL, NULL, CrospangInputInfo, HeuksunDIPInfo,
	heuksunInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	320, 240, 4, 3
};


// Bestri (Korea)

static struct BurnRomInfo bestriRomDesc[] = {
	{ "ua02.i3",	0x80000, 0x9e94023d, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ua03.i5",	0x80000, 0x08cfa8d8, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "us02.p3",	0x10000, 0xc7cc05fa, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "us08.q7",	0x40000, 0x85d8f3de, 3 | BRF_SND },           //  3 Oki Samples

	{ "uc07 p12",	0x80000, 0x3d299954, 4 | BRF_GRA },           //  4 Background Tiles
	{ "uc08.m12",	0x80000, 0x2fc0c30e, 4 | BRF_GRA },           //  5
	{ "uc29.k12",	0x80000, 0x0260c321, 4 | BRF_GRA },           //  6
	{ "uc28.n12",	0x80000, 0x9938be27, 4 | BRF_GRA },           //  7

	{ "ud14.j12",	0x80000, 0x141c696e, 5 | BRF_GRA },           //  8 Sprites
	{ "ud15.h12",	0x80000, 0x7c04adc0, 5 | BRF_GRA },           //  9
	{ "ud16.g12",	0x80000, 0x3282ea76, 5 | BRF_GRA },           // 10
	{ "ud17.e12",	0x80000, 0x3a3a3f1a, 5 | BRF_GRA },           // 11
};

STD_ROM_PICK(bestri)
STD_ROM_FN(bestri)

static INT32 bestriInit()
{
	bestri = 1;

	return DrvInit(bestriLoadRoms);
}

struct BurnDriver BurnDrvBestri = {
	"bestri", NULL, NULL, NULL, "1998",
	"Bestri (Korea)\0", NULL, "F2 System", "Miscellaneous",
	L"Bestri\0\uBCA0\uC2A4\uD2B8\uB77C\uC774 (Korea)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, bestriRomInfo, bestriRomName, NULL, NULL, CrospangInputInfo, BestriDIPInfo,
	bestriInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	320, 240, 4, 3
};
