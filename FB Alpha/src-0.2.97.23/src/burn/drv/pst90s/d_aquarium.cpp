// FB Alpha Aquarium driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "burn_ym2151.h"
#include "msm6295.h"
#include "bitswap.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvZ80Bank;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvSndROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvPalRAM;
static UINT8 *DrvMidRAM;
static UINT8 *DrvBakRAM;
static UINT8 *DrvTxtRAM;
static UINT8 *DrvSprRAM;
static UINT16 *DrvScroll;
static UINT32 *DrvPalette;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static UINT8 *soundlatch;
static UINT8 *soundack;
static UINT8 *flipscreen;

static struct BurnInputInfo AquariumInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 3,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Aquarium)

static struct BurnDIPInfo AquariumDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL					},
	{0x13, 0xff, 0xff, 0xff, NULL					},
	{0x14, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x12, 0x01, 0x03, 0x02, "Easy"					},
	{0x12, 0x01, 0x03, 0x03, "Normal"				},
	{0x12, 0x01, 0x03, 0x01, "Hard"					},
	{0x12, 0x01, 0x03, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    3, "Winning Rounds (Player VS CPU)"	},
	{0x12, 0x01, 0x0c, 0x0c, "1/1"					},
	{0x12, 0x01, 0x0c, 0x08, "2/3"					},
	{0x12, 0x01, 0x0c, 0x04, "3/5"					},

	{0   , 0xfe, 0   ,    3, "Winning Rounds (Player VS Player)"	},
	{0x12, 0x01, 0x30, 0x30, "1/1"					},
	{0x12, 0x01, 0x30, 0x20, "2/3"					},
	{0x12, 0x01, 0x30, 0x10, "3/5"					},

	{0   , 0xfe, 0   ,    8, "Coinage"				},	
	{0x13, 0x01, 0x07, 0x00, "5 Coins 1 Credits "			},
	{0x13, 0x01, 0x07, 0x01, "4 Coins 1 Credits "			},
	{0x13, 0x01, 0x07, 0x02, "3 Coins 1 Credits "			},
	{0x13, 0x01, 0x07, 0x03, "2 Coins 1 Credits "			},
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Credits "			},
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Credits "			},
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Credits "			},
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Credits "			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x13, 0x01, 0x10, 0x10, "Off"					},
	{0x13, 0x01, 0x10, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x13, 0x01, 0x20, 0x00, "Off"					},
	{0x13, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Language"				},
	{0x14, 0x01, 0xff, 0x00, "Japanese"				},
	{0x14, 0x01, 0xff, 0xff, "English"				},
};

STDDIPINFO(Aquarium)

UINT8 __fastcall aquarium_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xd80080:
			return DrvDips[1];

		case 0xd80081:
			return DrvDips[0];

		case 0xd80086:
		{
			INT32 data = DrvInputs[1] | *soundack;
			*soundack = 0;
			return data;
		}
	}

	return 0;
}

UINT16 __fastcall aquarium_read_word(UINT32 address)
{
	switch (address)
	{
		case 0xd80084:
			return DrvInputs[0];
	}

	return 0;
}

void __fastcall aquarium_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0xd8008b:
			*soundlatch = data;
			ZetNmi();
		return;
	}
}

void __fastcall aquarium_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0xd80014:
		case 0xd80016:
		case 0xd80018:
		case 0xd8001a:
		case 0xd8001c:
		case 0xd8001e:
			DrvScroll[(address - 0xd80014)/2] = data;
		return;

		case 0xd80056:
			*flipscreen = ~data & 1; // seems right?
		return;
	}
}

static void sound_bankswitch(INT32 nBank)
{
	*DrvZ80Bank = nBank & 7;

	nBank = ((nBank & 7) + 1) * 0x8000;

	ZetMapArea(0x8000, 0xffff, 0, DrvZ80ROM + nBank);
	ZetMapArea(0x8000, 0xffff, 2, DrvZ80ROM + nBank);
}

void __fastcall aquarium_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			BurnYM2151SelectRegister(data);
		return;

		case 0x01:
			BurnYM2151WriteRegister(data);
		return;

		case 0x02:
			MSM6295Command(0, BITSWAP08(data, 0, 1, 2, 3, 4, 5, 6, 7));
		return;

		case 0x06:
			*soundack = 0x80;
		return;

		case 0x08:
			sound_bankswitch(data);
		return;
	}
}

UINT8 __fastcall aquarium_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x01:
			return BurnYM2151ReadStatus();

		case 0x02:
			return BITSWAP08(MSM6295ReadStatus(0), 0, 1, 2, 3, 4, 5, 6, 7);

		case 0x04:
			return *soundlatch;
	}

	return 0;
}

void aquariumYM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
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

	MSM6295Reset(0);
	BurnYM2151Reset();

	Drv68KROM[0x000a5c] = DrvDips[2]; // set region (hack)
	Drv68KROM[0x000a5d] = DrvDips[2];

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[5]  = { 0x400000, 0, 1, 2, 3 };
	INT32 Plane2[4]  = { 48, 16, 32, 0 };
	INT32 XOffs0[16] = { 8, 12, 0, 4, 24, 28, 16, 20, 40, 44, 32, 36, 56, 60, 48, 52 };
	INT32 XOffs2[16] = { 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs0[16] = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
	INT32 YOffs1[8]  = { 0, 32, 64, 96, 128, 160, 192, 224 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x100000);

	GfxDecode(0x1000, 5, 16, 16, Plane0    , XOffs0, YOffs0, 0x400, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x020000);

	GfxDecode(0x1000, 4,  8,  8, Plane0 + 1, XOffs0, YOffs1, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x100000);

	GfxDecode(0x2000, 4, 16, 16, Plane2    , XOffs2, YOffs0, 0x400, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x100000);

	GfxDecode(0x1000, 5, 16, 16, Plane0    , XOffs0, YOffs0, 0x400, tmp, DrvGfxROM3);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;
	DrvZ80ROM	= Next; Next += 0x040000;

	DrvGfxROM0	= Next; Next += 0x100000;
	DrvGfxROM1	= Next; Next += 0x040000;
	DrvGfxROM2	= Next; Next += 0x200000;
	DrvGfxROM3	= Next; Next += 0x100000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x080000;

	DrvPalette	= (UINT32*)Next; Next += 0x0800 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x001000;
	DrvMidRAM	= Next; Next += 0x001000;
	DrvBakRAM	= Next; Next += 0x001000;
	DrvTxtRAM	= Next; Next += 0x002000;
	DrvSprRAM	= Next; Next += 0x002000;

	DrvZ80RAM	= Next; Next += 0x000800;

	soundack	= Next; Next += 0x000001;
	soundlatch 	= Next; Next += 0x000001;
	flipscreen	= Next; Next += 0x000001;
	DrvZ80Bank	= Next; Next += 0x000001;

	DrvScroll	= (UINT16*)Next; Next += 0x000008 * sizeof(UINT16);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvLoad5bpp(UINT8 *gfx, INT32 num)
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x20000);
	if (tmp == NULL) {
		return 1;
	}

	if (BurnLoadRom(tmp, num, 1)) return 1;

	for (INT32 len = 0; len < 0x020000; len++)
	{
		gfx[len*4+0] =  ((tmp[len] & 0x20) << 2) | ((tmp[len] & 0x10) >> 1);
		gfx[len*4+1] =  ((tmp[len] & 0x80) << 0) | ((tmp[len] & 0x40) >> 3);
		gfx[len*4+2] =  ((tmp[len] & 0x02) << 6) | ((tmp[len] & 0x01) << 3);
		gfx[len*4+3] =  ((tmp[len] & 0x08) << 4) | ((tmp[len] & 0x04) << 1);
	}

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

		if (BurnLoadRom(DrvZ80ROM,		1, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0,		2, 1)) return 1;
		if (DrvLoad5bpp(DrvGfxROM0 + 0x80000,	3   )) return 1;

		if (BurnLoadRom(DrvGfxROM1,		4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2,		5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3,		6, 1)) return 1;
		if (DrvLoad5bpp(DrvGfxROM3 + 0x80000,	7   )) return 1;

		if (BurnLoadRom(DrvSndROM,		8, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvMidRAM,		0xc00000, 0xc00fff, SM_RAM);
	SekMapMemory(DrvBakRAM,		0xc01000, 0xc01fff, SM_RAM);
	SekMapMemory(DrvTxtRAM,		0xc02000, 0xc03fff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0xc80000, 0xc81fff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0xd00000, 0xd00fff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0xff0000, 0xffffff, SM_RAM);
	SekSetWriteByteHandler(0,	aquarium_write_byte);
	SekSetWriteWordHandler(0,	aquarium_write_word);
	SekSetReadByteHandler(0,	aquarium_read_byte);
	SekSetReadWordHandler(0,	aquarium_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM);
	ZetMapArea(0x7800, 0x7fff, 0, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 1, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 2, DrvZ80RAM);
	ZetSetOutHandler(aquarium_sound_out);
	ZetSetInHandler(aquarium_sound_in);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1122000 / 132, 1);
	MSM6295SetRoute(0, 0.47, BURN_SND_ROUTE_BOTH);

	BurnYM2151Init(3579545);
	BurnYM2151SetIrqHandler(&aquariumYM2151IrqHandler);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.45, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.45, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	BurnYM2151Exit();
	MSM6295Exit(0);
	SekExit();
	ZetExit();

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	return 0;
}

static inline void palette_write(INT32 offset)
{
	UINT8 r,g,b;
	UINT16 data = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvPalRAM + offset)));

	r = ((data >> 11) & 0x1e) | ((data >> 3) & 0x01);
	g = ((data >>  7) & 0x1e) | ((data >> 2) & 0x01);
	b = ((data >>  3) & 0x1e) | ((data >> 1) & 0x01);

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

static void draw_bg_layer(UINT8 *src, UINT8 *gfx_base, INT32 scrollx, INT32 scrolly, INT32 transp, INT32 priority)
{
	UINT16 *vram = (UINT16*)src;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 color = BURN_ENDIAN_SWAP_INT16(vram[(offs * 2) + 1]);
		INT32 prior = (color & 0x0020) >> 5;
		if (prior != priority) continue;

		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx -= scrollx + 16;
		if (sx < -15) sx += 0x200;
		sy -= scrolly + 16;
		if (sy < -15) sy += 0x200;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 flipx = color & 0x0100;
		INT32 flipy = color & 0x0200;
		INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2]) & 0x0fff;

		if (transp) {
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color & 0x001f, 5, 0, 0x400, gfx_base);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color & 0x001f, 5, 0, 0x400, gfx_base);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color & 0x001f, 5, 0, 0x400, gfx_base);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color & 0x001f, 5, 0, 0x400, gfx_base);
				}
			}
		} else {
			if (flipy) {
				if (flipx) {
					Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color & 0x001f, 5, 0x400, gfx_base);
				} else {
					Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, sy, color & 0x001f, 5, 0x400, gfx_base);
				}
			} else {
				if (flipx) {
					Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, color & 0x001f, 5, 0x400, gfx_base);
				} else {
					Render16x16Tile_Clip(pTransDraw, code, sx, sy, color & 0x001f, 5, 0x400, gfx_base);
				}
			}
		}
	}
}

static void draw_tx_layer(INT32 scrollx, INT32 scrolly)
{
	UINT16 *vram = (UINT16*)DrvTxtRAM;

	for (INT32 offs = 0; offs < 64 * 64; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		sx -= scrollx + 16;
		if (sx < -7) sx += 0x200;
		sy -= scrolly + 16;
		if (sy < -7) sy += 0x200;

		INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs]) & 0xfff;
		INT32 color = BURN_ENDIAN_SWAP_INT16(vram[offs]) >> 12;

		if (sx >= nScreenWidth || sy >= nScreenHeight || code == 0) continue;

		Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x200, DrvGfxROM1);
	}
}

static void draw_sprites()
{
	UINT16 *sram = (UINT16*)DrvSprRAM;

	for (INT32 offs = 0; offs < 0x0800; offs += 8)
	{
		INT32 code = (BURN_ENDIAN_SWAP_INT16(sram[offs+5]) & 0xff) | ((BURN_ENDIAN_SWAP_INT16(sram[offs+6]) & 0x3f) << 8);

		if (~BURN_ENDIAN_SWAP_INT16(sram[offs+4]) & 0x80)
		{
			INT32 x = (((BURN_ENDIAN_SWAP_INT16(sram[offs+0]) & 0xff) | ((BURN_ENDIAN_SWAP_INT16(sram[offs+1]) & 0x7f) << 8)) - ((BURN_ENDIAN_SWAP_INT16(sram[offs+1]) & 0x80) << 9)) - 16;
			INT32 y = (((BURN_ENDIAN_SWAP_INT16(sram[offs+2]) & 0xff) | ((BURN_ENDIAN_SWAP_INT16(sram[offs+3]) & 0x7f) << 8)) - ((BURN_ENDIAN_SWAP_INT16(sram[offs+3]) & 0x80) << 9)) - 16;

			INT32 col  =  BURN_ENDIAN_SWAP_INT16(sram[offs+7]) & 0x0f;
			INT32 chain = BURN_ENDIAN_SWAP_INT16(sram[offs+4]) & 0x07;
			INT32 flipy = BURN_ENDIAN_SWAP_INT16(sram[offs+4]) & 0x10;
			INT32 flipx = BURN_ENDIAN_SWAP_INT16(sram[offs+4]) & 0x20;

			if (~BURN_ENDIAN_SWAP_INT16(sram[offs+4]) & 0x08) {
				if (flipy) y += chain << 4;
				if (flipx) x += chain << 4;
			}

			for (INT32 pos = chain; pos >= 0; pos--)
			{
				if (flipy) {
					if (flipx) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, x, y,       col, 4, 0, 0x300, DrvGfxROM2);
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, x, y + 256, col, 4, 0, 0x300, DrvGfxROM2);
					} else {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, x, y,       col, 4, 0, 0x300, DrvGfxROM2);
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, x, y + 256, col, 4, 0, 0x300, DrvGfxROM2);
					}
				} else {
					if (flipx) {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, x, y,       col, 4, 0, 0x300, DrvGfxROM2);
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, x, y + 256, col, 4, 0, 0x300, DrvGfxROM2);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, code, x, y,       col, 4, 0, 0x300, DrvGfxROM2);
						Render16x16Tile_Mask_Clip(pTransDraw, code, x, y + 256, col, 4, 0, 0x300, DrvGfxROM2);
					}
				}

				code++;

				if (BURN_ENDIAN_SWAP_INT16(sram[offs+4]) & 0x08)
				{
					if (flipy) y -= 16;
					else y += 16;
				}
				else
				{
					if (flipx) x -= 16;
					else x += 16;
				}
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

	draw_bg_layer(DrvBakRAM, DrvGfxROM3, DrvScroll[2] & 0x1ff, DrvScroll[3] & 0x1ff, 0, 0);
	draw_bg_layer(DrvMidRAM, DrvGfxROM0, DrvScroll[0] & 0x1ff, DrvScroll[1] & 0x1ff, 1, 0);

	draw_sprites();

	draw_bg_layer(DrvBakRAM, DrvGfxROM3, DrvScroll[2] & 0x1ff, DrvScroll[3] & 0x1ff, 1, 1);
	draw_bg_layer(DrvMidRAM, DrvGfxROM0, DrvScroll[0] & 0x1ff, DrvScroll[1] & 0x1ff, 1, 1);

	draw_tx_layer(DrvScroll[4] & 0x1ff, DrvScroll[5] & 0x1ff);

	if (*flipscreen) {
		INT32 len = nScreenWidth * nScreenHeight;
		for (INT32 i = 0; i < len / 2; i++) {
			INT32 t = pTransDraw[i + 0];
			pTransDraw[i + 0] = pTransDraw[len-(1+i)];
			pTransDraw[len-(1+i)] = t;
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
		memset (DrvInputs, 0xff, 4);
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
		DrvInputs[1] &= 0x7f;
	}

	INT32 nSegment;
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 16000000 / 60, 6000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekNewFrame();
	ZetNewFrame();
	
	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nSegment = (nCyclesTotal[0] - nCyclesDone[0]) / (nInterleave - i);
		nCyclesDone[0] += SekRun(nSegment);

		nSegment = (nCyclesTotal[1] - nCyclesDone[1]) / (nInterleave - i);
		nCyclesDone[1] += ZetRun(nSegment);

		if (pBurnSoundOut) {
			nSegment = nBurnSoundLen / nInterleave;

			BurnYM2151Render(pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
			MSM6295Render(0, pBurnSoundOut + (nSoundBufferPos << 1), nSegment);

			nSoundBufferPos += nSegment;
		}
	}

	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		nSegment = nBurnSoundLen - nSoundBufferPos;
		if (nSegment > 0) {
			BurnYM2151Render(pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
			MSM6295Render(0, pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
		}
	}

	ZetClose();
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
		*pnMin = 0x029698;
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
		ZetScan(nAction);

		BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		sound_bankswitch(*DrvZ80Bank);
		ZetClose();
	}

	return 0;
}


// Aquarium (Japan)

static struct BurnRomInfo aquariumRomDesc[] = {
	{ "aquar3",	0x080000, 0x344509a1, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code

	{ "aquar5",	0x040000, 0xfa555be1, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "aquar1",	0x080000, 0x575df6ac, 3 | BRF_GRA },           //  2 Midground Tiles
	{ "aquar6",	0x020000, 0x9065b146, 3 | BRF_GRA },           //  3

	{ "aquar2",	0x020000, 0xaa071b05, 4 | BRF_GRA },           //  6 Foreground Tiles

	{ "aquarf1",	0x100000, 0x14758b3c, 5 | BRF_GRA },           //  7 Sprites

	{ "aquar8",	0x080000, 0x915520c4, 6 | BRF_GRA },           //  4 Background Tiles
	{ "aquar7",	0x020000, 0xb96b2b82, 6 | BRF_GRA },           //  5

	{ "aquar4",	0x080000, 0x9a4af531, 7 | BRF_SND },           //  8 MSM6295 Samples
};

STD_ROM_PICK(aquarium)
STD_ROM_FN(aquarium)

struct BurnDriver BurnDrvAquarium = {
	"aquarium", NULL, NULL, NULL, "1996",
	"Aquarium (Japan)\0", NULL, "Excellent System", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, aquariumRomInfo, aquariumRomName, NULL, NULL, AquariumInputInfo, AquariumDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 256, 4, 3
};
