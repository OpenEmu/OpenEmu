// FB Alpha Lemmings driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "sek.h"
#include "burn_ym2151.h"
#include "m6809_intf.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvM6809ROM;
static UINT8 *DrvM6809RAM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvPxlRAM0;
static UINT8 *DrvPxlRAM1;
static UINT8 *DrvSprRAM0;
static UINT8 *DrvSprRAM1;
static UINT8 *DrvSprBuf0;
static UINT8 *DrvSprBuf1;
static UINT8 *DrvSprTBuf0;
static UINT8 *DrvSprTBuf1;
static UINT8 *DrvCtrlRAM;

static UINT8 *soundlatch;

static UINT16 *pTempDraw;

static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static UINT8 FakeAnInp[8];
static UINT8 FakeTrackBallX[2];
static UINT8 FakeTrackBallY[2];

static struct BurnInputInfo LemmingsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	FakeAnInp + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	FakeAnInp + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	FakeAnInp + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	FakeAnInp + 3,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	FakeAnInp + 4,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	FakeAnInp + 5,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	FakeAnInp + 6,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	FakeAnInp + 7,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Lemmings)

static struct BurnDIPInfo LemmingsDIPList[]=
{
	{0x11, 0xff, 0xff, 0x04, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x04, 0x04, "Off"			},
	{0x11, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Credits for 1 Player"	},
	{0x12, 0x01, 0x03, 0x03, "1"			},
	{0x12, 0x01, 0x03, 0x02, "2"			},
	{0x12, 0x01, 0x03, 0x01, "3"			},
	{0x12, 0x01, 0x03, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Credits for 2 Player"	},
	{0x12, 0x01, 0x0c, 0x0c, "1"			},
	{0x12, 0x01, 0x0c, 0x08, "2"			},
	{0x12, 0x01, 0x0c, 0x04, "3"			},
	{0x12, 0x01, 0x0c, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Credits for Continue"	},
	{0x12, 0x01, 0x30, 0x30, "1"			},
	{0x12, 0x01, 0x30, 0x20, "2"			},
	{0x12, 0x01, 0x30, 0x10, "3"			},
	{0x12, 0x01, 0x30, 0x00, "4"			},
	
	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0x07, 0x01, "1 Coin  7 Credits"	},
	{0x13, 0x01, 0x07, 0x00, "1 Coin  8 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0x38, 0x08, "1 Coin  7 Credits"	},
	{0x13, 0x01, 0x38, 0x00, "1 Coin  8 Credits"	},
};

STDDIPINFO(Lemmings)

static inline void palette_update_one(INT32 offset)
{
	UINT16 *p = (UINT16*)DrvPalRAM;

	offset = (offset & 0xffc) / 2;

	DrvPalette[offset/2] = BurnHighCol(BURN_ENDIAN_SWAP_INT16(p[offset+1]), BURN_ENDIAN_SWAP_INT16(p[offset+1]) >> 8, BURN_ENDIAN_SWAP_INT16(p[offset]), 0);
}

static inline void pixel_layer_update_one(INT32 offset)
{
	UINT16 src = *((UINT16*)(DrvPxlRAM0 + offset));

	pTempDraw[offset + 0] = ((BURN_ENDIAN_SWAP_INT16(src) >> 8) & 0x0f) + 0x100;
	pTempDraw[offset + 1] = ((BURN_ENDIAN_SWAP_INT16(src) >> 0) & 0x0f) + 0x100;
}

static inline void pixel_layer_update_two(INT32 offset)
{
	UINT16 src = *((UINT16*)(DrvPxlRAM1 + offset));

	INT32 sy = offset / 0x200;
	INT32 sx = offset & 0x1fe;

	INT32 off = ((((sx / 8) * 32) + (sy / 8)) * 64) + ((sy & 7) * 8) + (sx & 6);

	DrvGfxROM2[off + 0] = ((BURN_ENDIAN_SWAP_INT16(src) >> 8) & 0x0f);
	DrvGfxROM2[off + 1] = ((BURN_ENDIAN_SWAP_INT16(src) >> 0) & 0x0f);
}

static void __fastcall lemmings_main_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfff000) == 0x160000) {
		*((UINT16*)(DrvPalRAM + (address & 0x000ffe))) = BURN_ENDIAN_SWAP_INT16(data);
		palette_update_one(address);
		return;
	}

	if ((address & 0xfffff0) == 0x170000) {
		*((UINT16*)(DrvCtrlRAM + (address & 0x0000e))) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}

	if ((address & 0xf80000) == 0x300000) {
		*((UINT16*)(DrvPxlRAM0 + (address & 0x7fffe))) = BURN_ENDIAN_SWAP_INT16(data);
		pixel_layer_update_one(address & 0x7fffe);
		return;
	}

	if ((address & 0xfe0000) == 0x380000) {
		*((UINT16*)(DrvPxlRAM1 + (address & 0x1fffe))) = BURN_ENDIAN_SWAP_INT16(data);
		pixel_layer_update_two(address & 0x1fffe);
		return;
	}

	switch (address)
	{
		case 0x1a0064:
			*soundlatch = data & 0xff;
			M6809SetIRQ(1, M6809_IRQSTATUS_ACK);
		return;

		case 0x1c0000:
			memcpy (DrvSprBuf0, DrvSprRAM0,	0x800);
		return;

		case 0x1e0000:
			memcpy (DrvSprBuf1, DrvSprRAM1,	0x800);
		return;
	}
}

static void __fastcall lemmings_main_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xf80000) == 0x300000) {
		DrvPxlRAM0[(address & 0x7ffff)^1] = data;
		pixel_layer_update_one(address & 0x7fffe);
		return;
	}

	if ((address & 0xfe0000) == 0x380000) {
		DrvPxlRAM1[(address & 0x1ffff)^1] = data;
		pixel_layer_update_two(address & 0x1fffe);
		return;
	}
}

static UINT16 __fastcall lemmings_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x190000:
			return FakeTrackBallX[0];

		case 0x190002:
			return FakeTrackBallY[0];

		case 0x190008:
			return FakeTrackBallX[1];

		case 0x19000a:
			return FakeTrackBallY[1];

		case 0x1a0320:
			return (DrvInputs[1] & 0xfffb) | (DrvDips[0] & 0x04);

		case 0x1a041a:
			return DrvInputs[0];
	}
	
	return 0;
}

static UINT8 __fastcall lemmings_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x1a0321: // Flipper seems to work better than proper vblank status
			static INT32 vblank;
			vblank ^= 8;
			return (DrvInputs[1] & 0xf3) | (DrvDips[0] & 0x04) | vblank;

		case 0x1a04e6:
			return DrvDips[2];

		case 0x1a04e7:
			return DrvDips[1];
	}
	
	return 0;
}

static void lemmings_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x0800:
			BurnYM2151SelectRegister(data);
		return;

		case 0x0801:
			BurnYM2151WriteRegister(data);
		return;

		case 0x1000:
			MSM6295Command(0, data);
		return;

		case 0x1800:
			M6809SetIRQ(1, M6809_IRQSTATUS_NONE);
		return;
	}
}

static UINT8 lemmings_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x0800:
		case 0x0801:
			return BurnYM2151ReadStatus();

		case 0x1000:
			return MSM6295ReadStatus(0);

		case 0x1800:
			return *soundlatch;
	}

	return 0;
}

static void lemmingsYM2151IrqHandler(INT32 irq)
{
	M6809SetIRQ(0, irq ? M6809_IRQSTATUS_ACK : M6809_IRQSTATUS_NONE);
	M6809Run(1000); // fix music tempo
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	M6809Open(0);
	M6809Reset();
	M6809Close();

	MSM6295Reset(0);
	BurnYM2151Reset();
	
	FakeTrackBallX[0] = FakeTrackBallX[1] = 0xff;
	FakeTrackBallY[0] = FakeTrackBallY[1] = 0x00;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x100000;
	DrvM6809ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x080000;
	DrvGfxROM1	= Next; Next += 0x080000;

	MSM6295ROM	= Next; Next += 0x040000;

	DrvPalette	= (UINT32*)Next; Next += 0x0401 * sizeof(INT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x001000;
	DrvPxlRAM0	= Next; Next += 0x080000;
	DrvPxlRAM1	= Next; Next += 0x020000;
	DrvVidRAM	= Next; Next += 0x003000;
	DrvSprRAM0	= Next; Next += 0x000800;
	DrvSprRAM1	= Next; Next += 0x000800;
	DrvSprBuf0	= Next; Next += 0x000800;
	DrvSprBuf1	= Next; Next += 0x000800;
	DrvSprTBuf0	= Next; Next += 0x000800;
	DrvSprTBuf1	= Next; Next += 0x000800;

	DrvM6809RAM	= Next; Next += 0x000800;

	soundlatch	= Next; Next += 0x000001;

	DrvCtrlRAM	= Next; Next += 0x000010;

	DrvGfxROM2	= Next; Next += 0x020000; // expanded char tiles

	pTempDraw	= (UINT16*)Next; Next += 0x800 * 0x200 * sizeof(INT16);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[ 3] = { 0x20000*8, 0x10000*8, 0x00000 };
	INT32 XOffs[16] = { 7, 6, 5, 4, 3, 2, 1, 0, 16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 };
	INT32 YOffs[16] = { 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8, 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x30000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x30000);

	GfxDecode(0x0800, 3, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x30000);

	GfxDecode(0x0800, 3, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM1);

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
		if (BurnLoadRom(Drv68KROM + 0x000001,	 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x000000,	 1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x040001,	 2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x040000,	 3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x080001,	 4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x080000,	 5, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x0c0001,	 6, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x0c0000,	 7, 2)) return 1;

		if (BurnLoadRom(DrvM6809ROM,		 8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,	 9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x10000,	10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x20000,	11, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,	12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x10000,	13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x20000,	14, 1)) return 1;

		if (BurnLoadRom(MSM6295ROM,		15, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0x120000, 0x1207ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0x140000, 0x1407ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x160000, 0x160fff, SM_ROM);
	SekMapMemory(DrvVidRAM,			0x200000, 0x202fff, SM_RAM);
	SekMapMemory(DrvPxlRAM0,		0x300000, 0x37ffff, SM_ROM);
	SekMapMemory(DrvPxlRAM1,		0x380000, 0x39ffff, SM_ROM);
	SekSetWriteWordHandler(0,		lemmings_main_write_word);
	SekSetWriteByteHandler(0,		lemmings_main_write_byte);
	SekSetReadWordHandler(0,		lemmings_main_read_word);
	SekSetReadByteHandler(0,		lemmings_main_read_byte);
	SekClose();

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809RAM,		0x0000, 0x07ff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM + 0x8000,	0x8000, 0xffff, M6809_ROM);
	M6809SetWriteByteHandler(lemmings_sound_write);
	M6809SetReadByteHandler(lemmings_sound_read);
	M6809Close();

	BurnYM2151Init(3580000, 45.0);
	BurnYM2151SetIrqHandler(&lemmingsYM2151IrqHandler);

	MSM6295Init(0, 1023924 / 132, 50.0, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	BurnYM2151Exit();
	MSM6295Exit(0);
	MSM6295ROM = NULL;

	SekExit();
	M6809Exit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites(UINT8 *ram, UINT8 *rom, INT32 color_offset, INT32 pri)
{
	UINT16 *sprdata = (UINT16*)ram;

	for (INT32 offs = 0; offs < 0x400; offs += 4)
	{
		INT32 inc;

		if ((BURN_ENDIAN_SWAP_INT16(sprdata[offs + 2]) & 0x2000) != pri) continue;

		INT32 sy    = BURN_ENDIAN_SWAP_INT16(sprdata[offs + 0]);

		if ((sy & 0x1000) && (nCurrentFrame & 1)) continue;

		INT32 code  = BURN_ENDIAN_SWAP_INT16(sprdata[offs + 1]) & 0x3fff;
		INT32 sx    = BURN_ENDIAN_SWAP_INT16(sprdata[offs + 2]);
		INT32 color = (sx >>9) & 0xf;

		INT32 flipx = sy & 0x2000;
		INT32 flipy = sy & 0x4000;
		INT32 multi = (1 << ((sy & 0x0600) >> 9)) - 1;

		sx = sx & 0x01ff;
		sy = (sy & 0x01ff) - 16;
		if (sx >= 320) sx -= 512;
		if (sy >= 256) sy -= 512;

		if (sx > 320 || sx < -16) continue;

		code &= ~multi;

		if (flipy) {
			inc = 1;
		} else {
			code += multi;
			inc = -1;
		}

		while (multi >= 0)
		{
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, (code - multi * inc) & 0x7ff, sx, sy + -16 * multi, color, 4, 0, color_offset, rom);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, (code - multi * inc) & 0x7ff, sx, sy + -16 * multi, color, 4, 0, color_offset, rom);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, (code - multi * inc) & 0x7ff, sx, sy + -16 * multi, color, 4, 0, color_offset, rom);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, (code - multi * inc) & 0x7ff, sx, sy + -16 * multi, color, 4, 0, color_offset, rom);
				}
			}

			multi--;
		}
	}
}

static void draw_layer()
{
	UINT16 *ram = (UINT16*)DrvVidRAM;

	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 sy = ((offs & 0x1f) * 8) - 16;
		INT32 sx = (offs / 0x20) * 8;

		INT32 attr  = BURN_ENDIAN_SWAP_INT16(ram[offs]);
		INT32 code  = attr & 0x7ff;
		INT32 color = attr >> 12;

		if (sy >= nScreenHeight || sx >= nScreenWidth) continue;

		Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM2);
	}
}

static void copy_pixel_layer()
{
	UINT16 *ctrl = (UINT16*)DrvCtrlRAM;
	INT32 x0 = -BURN_ENDIAN_SWAP_INT16(ctrl[2]);
	INT32 x1 = -BURN_ENDIAN_SWAP_INT16(ctrl[0]);

	UINT16 *src = pTempDraw + (16 * 0x800);
	UINT16 *dst = pTransDraw;

	if (BURN_ENDIAN_SWAP_INT16(ctrl[6]) & 0x02) // window mode
	{
		for (INT32 y = 0; y < nScreenHeight; y++, src += 0x800, dst += nScreenWidth) {
			for (INT32 x = 0; x < 160; x++) {
				INT32 pxl = src[(x - x0) & 0x7ff];
				if (pxl!=0x100) {
					dst[x] = pxl;
				}

				pxl = src[((x - x1) + 160) & 0x7ff];
				if (pxl!=0x100) {
					dst[x + 160] = pxl;
				}
			}
		}
	}
	else
	{
		for (INT32 y = 0; y < nScreenHeight; y++, src += 0x800, dst += nScreenWidth) {
			for (INT32 x = 0; x < nScreenWidth; x++) {
				INT32 pxl = src[(x - x1) & 0x7ff];
				if (pxl!=0x100) {
					dst[x] = pxl;
				}
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x1000; i+=4) {
			palette_update_one(i);
		}

		DrvPalette[0x400] = BurnHighCol(0, 0, 0, 0); // black

		DrvRecalc = 0;
	}

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x400;
	}

	if (nSpriteEnable & 1) draw_sprites(DrvSprTBuf1, DrvGfxROM1, 0x300, 0x0000);

	if (nBurnLayer & 1) copy_pixel_layer();

	if (nSpriteEnable & 2) draw_sprites(DrvSprTBuf0, DrvGfxROM0, 0x200, 0x0000);
	if (nSpriteEnable & 4) draw_sprites(DrvSprTBuf1, DrvGfxROM1, 0x300, 0x2000);
	
	if (nBurnLayer & 2) draw_layer();

	if (nSpriteEnable & 8) draw_sprites(DrvSprTBuf0, DrvGfxROM0, 0x200, 0x2000);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	M6809NewFrame();

	{
		memset (DrvInputs, 0xff, 2 * sizeof(INT16));
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
		
		if (FakeAnInp[0]) FakeTrackBallY[0] -= 0x04;
		if (FakeAnInp[1]) FakeTrackBallY[0] += 0x04;
		if (FakeAnInp[2]) FakeTrackBallX[0] += 0x04;
		if (FakeAnInp[3]) FakeTrackBallX[0] -= 0x04;
		if (FakeAnInp[4]) FakeTrackBallY[1] -= 0x04;
		if (FakeAnInp[5]) FakeTrackBallY[1] += 0x04;
		if (FakeAnInp[6]) FakeTrackBallX[1] += 0x04;
		if (FakeAnInp[7]) FakeTrackBallX[1] -= 0x04;
	}

	INT32 nSegment;
	INT32 nInterleave = 256;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 14000000 / 60, 4027500 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	M6809Open(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nSegment = nCyclesTotal[0] - nCyclesDone[0];
		nCyclesDone[0] += SekRun(nSegment);

		nSegment = nCyclesTotal[1] - nCyclesDone[1];
		nCyclesDone[1] += M6809Run(nSegment);

		if (pBurnSoundOut) {
			nSegment = nBurnSoundLen / nInterleave;

			BurnYM2151Render(pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
			MSM6295Render(0, pBurnSoundOut + (nSoundBufferPos << 1), nSegment);

			nSoundBufferPos += nSegment;
		}
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		nSegment = nBurnSoundLen - nSoundBufferPos;
		if (nSegment > 0) {
			BurnYM2151Render(pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
			MSM6295Render(0, pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
		}
	}

	M6809Close();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	memcpy (DrvSprTBuf0, DrvSprBuf0, 0x800);
	memcpy (DrvSprTBuf1, DrvSprBuf1, 0x800);

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

		BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);
		
		SCAN_VAR(FakeTrackBallX);
		SCAN_VAR(FakeTrackBallY);
	}

	return 0;
}


// Lemmings (US prototype)

static struct BurnRomInfo lemmingsRomDesc[] = {
	{ "lemmings.5",		0x20000, 0xe9a2b439, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "lemmings.1",		0x20000, 0xbf52293b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "lemmings.6",		0x20000, 0x0e3dc0ea, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "lemmings.2",		0x20000, 0x0cf3d7ce, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "lemmings.7",		0x20000, 0xd020219c, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "lemmings.3",		0x20000, 0xc635494a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "lemmings.8",		0x20000, 0x9166ce09, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "lemmings.4",		0x20000, 0xaa845488, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "lemmings.15",	0x10000, 0xf0b24a35, 2 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "lemmings.9",		0x10000, 0xe06442f5, 3 | BRF_GRA },           //  9 Sprite Bank 0
	{ "lemmings.10",	0x10000, 0x36398848, 3 | BRF_GRA },           // 10
	{ "lemmings.11",	0x10000, 0xb46a54e5, 3 | BRF_GRA },           // 11

	{ "lemmings.12",	0x10000, 0xdc9047ff, 4 | BRF_GRA },           // 12 Sprite Bank 1
	{ "lemmings.13",	0x10000, 0x7cc15491, 4 | BRF_GRA },           // 13
	{ "lemmings.14",	0x10000, 0xc162788f, 4 | BRF_GRA },           // 14

	{ "lemmings.16",	0x20000, 0xf747847c, 5 | BRF_SND },           // 15 OKI MSM6295 Samples
};

STD_ROM_PICK(lemmings)
STD_ROM_FN(lemmings)

struct BurnDriver BurnDrvLemmings = {
	"lemmings", NULL, NULL, NULL, "1991",
	"Lemmings (US prototype)\0", NULL, "Data East USA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_MISC, 0,
	NULL, lemmingsRomInfo, lemmingsRomName, NULL, NULL, LemmingsInputInfo, LemmingsDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};
