// FB Alpha Backfire! driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "arm_intf.h"
#include "ymz280b.h"
#include "eeprom.h"
#include "deco16ic.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvArmROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
//static UINT8 *DrvGfxROM4;
static UINT8 *DrvSndROM;
static UINT8 *DrvArmRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM0;
static UINT8 *DrvSprRAM1;

static UINT16 *DrvTmpBitmap0;
static UINT16 *DrvTmpBitmap_p;
static UINT16 *DrvTmpBitmap1;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvDips[1];
static UINT16 DrvInputs[3];
static UINT8 DrvReset;

static UINT32 *priority;
static INT32 nPreviousDip;

static struct BurnInputInfo BackfireInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy3 + 8,	"p1 fire 4"	},
	{"P1 Button 5",		BIT_DIGITAL,	DrvJoy3 + 9,	"p1 fire 5"	},
	{"P1 Button 6",		BIT_DIGITAL,	DrvJoy3 + 10,	"p1 fire 6"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy3 + 12,	"p2 fire 4"	},
	{"P2 Button 5",		BIT_DIGITAL,	DrvJoy3 + 13,	"p2 fire 5"	},
	{"P2 Button 6",		BIT_DIGITAL,	DrvJoy3 + 14,	"p2 fire 6"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Backfire)

static struct BurnDIPInfo BackfireDIPList[]=
{
	{0x1a, 0xff, 0xff, 0x08, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x1a, 0x01, 0x08, 0x08, "Off"			},
	{0x1a, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Single Screen (Hack)"	},
	{0x1a, 0x01, 0x80, 0x00, "Off"			},
	{0x1a, 0x01, 0x80, 0x80, "On"			},
};

STDDIPINFO(Backfire)

void backfire_write_byte(UINT32 address, UINT8 data)
{
	Write16Byte(((UINT8*)deco16_pf_control[0]),	0x100000, 0x10001f) // 16-bit
	Write16Byte(deco16_pf_ram[0],		0x110000, 0x111fff) // 16-bit
	Write16Byte(deco16_pf_ram[1],		0x114000, 0x115fff) // 16-bit
	Write16Byte(deco16_pf_rowscroll[0],	0x120000, 0x120fff) // 16-bit
	Write16Byte(deco16_pf_rowscroll[1],	0x124000, 0x124fff) // 16-bit
	Write16Byte(((UINT8*)deco16_pf_control[1]),	0x130000, 0x13001f) // 16-bit
	Write16Byte(deco16_pf_ram[2],		0x140000, 0x141fff) // 16-bit
	Write16Byte(deco16_pf_ram[3],		0x144000, 0x145fff) // 16-bit
	Write16Byte(deco16_pf_rowscroll[2],	0x150000, 0x150fff) // 16-bit
	Write16Byte(deco16_pf_rowscroll[3],	0x154000, 0x154fff) // 16-bit

	switch (address)
	{
		case 0x1c0000:
			YMZ280BWrite(0, data);
		return;

		case 0x1c0004:
			YMZ280BWrite(1, data);
		return;
	}
}

void backfire_write_long(UINT32 address, UINT32 data)
{
	Write16Long(((UINT8*)deco16_pf_control[0]),	0x100000, 0x10001f) // 16-bit
	Write16Long(deco16_pf_ram[0],		0x110000, 0x111fff) // 16-bit
	Write16Long(deco16_pf_ram[1],		0x114000, 0x115fff) // 16-bit
	Write16Long(deco16_pf_rowscroll[0],	0x120000, 0x120fff) // 16-bit
	Write16Long(deco16_pf_rowscroll[1],	0x124000, 0x124fff) // 16-bit
	Write16Long(((UINT8*)deco16_pf_control[1]),	0x130000, 0x13001f) // 16-bit
	Write16Long(deco16_pf_ram[2],		0x140000, 0x141fff) // 16-bit
	Write16Long(deco16_pf_ram[3],		0x144000, 0x145fff) // 16-bit
	Write16Long(deco16_pf_rowscroll[2],	0x150000, 0x150fff) // 16-bit
	Write16Long(deco16_pf_rowscroll[3],	0x154000, 0x154fff) // 16-bit

	switch (address)
	{
		case 0x1a4000:
			EEPROMWrite(data & 0x02, data & 0x04, data & 0x01);
		return;

		case 0x1a8000:
			priority[0] = data; // left
		return;

		case 0x1ac000:
			priority[1] = data; // right
		return;

		case 0x1c0000:
			YMZ280BWrite(0, data);
		return;

		case 0x1c0004:
			YMZ280BWrite(1, data);
		return;
	}
}

UINT8 backfire_read_byte(UINT32 address)
{
	Read16Byte(((UINT8*)deco16_pf_control[0]),	0x100000, 0x10001f) // 16-bit
	Read16Byte(deco16_pf_ram[0],		0x110000, 0x111fff) // 16-bit
	Read16Byte(deco16_pf_ram[1],		0x114000, 0x115fff) // 16-bit
	Read16Byte(deco16_pf_rowscroll[0],	0x120000, 0x120fff) // 16-bit
	Read16Byte(deco16_pf_rowscroll[1],	0x124000, 0x124fff) // 16-bit
	Read16Byte(((UINT8*)deco16_pf_control[1]),	0x130000, 0x13001f) // 16-bit
	Read16Byte(deco16_pf_ram[2],		0x140000, 0x141fff) // 16-bit
	Read16Byte(deco16_pf_ram[3],		0x144000, 0x145fff) // 16-bit
	Read16Byte(deco16_pf_rowscroll[2],	0x150000, 0x150fff) // 16-bit
	Read16Byte(deco16_pf_rowscroll[3],	0x154000, 0x154fff) // 16-bit

	switch (address)
	{
		case 0x190000: return DrvInputs[0];
		case 0x190002: return DrvInputs[2];
		case 0x194002: return DrvInputs[1];

		case 0x1c0000: return YMZ280BRead(0);
		case 0x1c0004: return YMZ280BRead(1);
	}

	return 0;
}

UINT32 backfire_read_long(UINT32 address)
{
	Read16Long(((UINT8*)deco16_pf_control[0]),	0x100000, 0x10001f) // 16-bit
	Read16Long(deco16_pf_ram[0],		0x110000, 0x111fff) // 16-bit
	Read16Long(deco16_pf_ram[1],		0x114000, 0x115fff) // 16-bit
	Read16Long(deco16_pf_rowscroll[0],	0x120000, 0x120fff) // 16-bit
	Read16Long(deco16_pf_rowscroll[1],	0x124000, 0x124fff) // 16-bit
	Read16Long(((UINT8*)deco16_pf_control[1]),	0x130000, 0x13001f) // 16-bit
	Read16Long(deco16_pf_ram[2],		0x140000, 0x141fff) // 16-bit
	Read16Long(deco16_pf_ram[3],		0x144000, 0x145fff) // 16-bit
	Read16Long(deco16_pf_rowscroll[2],	0x150000, 0x150fff) // 16-bit
	Read16Long(deco16_pf_rowscroll[3],	0x154000, 0x154fff) // 16-bit

	switch (address)
	{
		case 0x190000: {
			UINT32 vblnk=0;
			vblnk ^= 1 << 16;

			UINT32 ret = 0;
			ret |= EEPROMRead() << 24;
			ret |= (DrvInputs[2] & 0xbf) << 16;
			ret |= deco16_vblank;
			ret |= DrvInputs[0];
			ret |= vblnk;
			return ret;
		}

		case 0x194000: {
			UINT32 ret = 0;
			ret |= EEPROMRead() << 24;
			ret |= DrvInputs[1] << 16;
			ret |= DrvInputs[1] <<  0;
			return ret;
		}

		case 0x1c0000:
			return YMZ280BRead(0);

		case 0x1c0004:
			return YMZ280BRead(1);
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvArmROM		= Next; Next += 0x0100000;

	DrvGfxROM0		= Next; Next += 0x0800000;
	DrvGfxROM1		= Next; Next += 0x0800000;
	DrvGfxROM2		= Next; Next += 0x0200000;
	DrvGfxROM3		= Next; Next += 0x0800000;
//	DrvGfxROM4		= Next; Next += 0x0800000;

	YMZ280BROM		= Next; 
	DrvSndROM		= Next; Next += 0x0400000;

	DrvPalette		= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);

	DrvTmpBitmap_p		= (UINT16*)Next;
	DrvTmpBitmap0		= (UINT16*)Next; Next += 320 * 240 * sizeof(UINT16);
	DrvTmpBitmap1		= (UINT16*)Next; Next += 320 * 240 * sizeof(UINT16);

	AllRam			= Next;

	DrvArmRAM		= Next; Next += 0x0008000;
	DrvPalRAM		= Next; Next += 0x0002000;
	DrvSprRAM0		= Next; Next += 0x0002000;
	DrvSprRAM1		= Next; Next += 0x0002000;

	priority		= (UINT32*)Next; Next += 2 * sizeof(UINT32);

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ArmOpen(0);
	ArmReset();
	ArmClose();

	YMZ280BReset();

	EEPROMReset();

	deco16Reset();

	return 0;
}

static INT32 backfire_bank_callback( INT32 bank )
{
	bank = bank >> 4;
	bank = (bank & 1) | ((bank & 4) >> 1) | ((bank & 2) << 1);

	return bank * 0x1000;
}

static void sprite_decode(UINT8 *gfx, INT32 len)
{
	INT32 Plane[4] = { 16, 0, 24, 8 };
	INT32 XOffs[16] = { 512,513,514,515,516,517,518,519, 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs[16] = { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,  8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32};

	UINT8 *tmp = (UINT8*)BurnMalloc(len);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, gfx, len);

	GfxDecode((len * 2) / 0x100, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, gfx);

	BurnFree (tmp);
}

static void decode_samples()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x200000);

	for (INT32 i = 0; i < 0x200000; i++) {
		tmp[((i & 1) << 20) | (i >> 1) ] = DrvSndROM[i];
	}

	memcpy (DrvSndROM, tmp, 0x200000);

	BurnFree (tmp);
}

static void pCommonSpeedhackCallback()
{
	ArmIdleCycles(1120);
	//bprintf (0, _T("idle skip triggered!\n"));
}

static INT32 DrvInit(UINT32 speedhack)
{
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvArmROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(DrvArmROM  + 0x000000,  1, 2)) return 1;

		for (INT32 i = 0; i < 0x100000; i+=4) {
			BurnByteswap(DrvArmROM + i + 1, 2);
		}

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  2, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x200000,  3, 1)) return 1;

		for (INT32 i = 0; i < 0x400000; i++) {
			INT32 j = (i & 0x17ffff) | ((i & 0x80000) << 2) | ((i & 0x200000) >> 2);
			DrvGfxROM0[j] = DrvGfxROM1[i];
		}
		memset (DrvGfxROM1, 0, 0x400000);

		if (BurnLoadRom(DrvGfxROM2 + 0x000000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000001,  5, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000000,  6, 2)) return 1;

	//	if (BurnLoadRom(DrvGfxROM4 + 0x000001,  7, 2)) return 1;
	//	if (BurnLoadRom(DrvGfxROM4 + 0x000000,  8, 2)) return 1;

		memset (DrvSndROM, 0xff, 0x400000);
		if (BurnLoadRom(DrvSndROM  + 0x000000,  9, 1)) return 1;
		if (BurnLoadRom(DrvSndROM  + 0x200000, 10, 1)) return 1;

		deco156_decrypt(DrvArmROM, 0x100000);

		deco56_decrypt_gfx(DrvGfxROM0, 0x400000);
		deco56_decrypt_gfx(DrvGfxROM2, 0x100000);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM1, 0x400000, 0);
		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x400000, 1);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x100000, 0);

		sprite_decode(DrvGfxROM3, 0x400000);
	//	sprite_decode(DrvGfxROM4, 0x400000);

		decode_samples();
	}

	ArmInit(1);
	ArmOpen(0);
	ArmMapMemory(DrvArmROM,		0x000000, 0x0fffff, ARM_ROM);
	ArmMapMemory(DrvPalRAM,		0x160000, 0x161fff, ARM_RAM);
	ArmMapMemory(DrvArmRAM,		0x170000, 0x177fff, ARM_RAM);
	ArmMapMemory(DrvSprRAM0,	0x184000, 0x185fff, ARM_RAM);
	ArmMapMemory(DrvSprRAM1,	0x18c000, 0x18dfff, ARM_RAM);
	ArmSetWriteByteHandler(backfire_write_byte);
	ArmSetWriteLongHandler(backfire_write_long);
	ArmSetReadByteHandler(backfire_read_byte);
	ArmSetReadLongHandler(backfire_read_long);
	ArmClose();

	ArmSetSpeedHack(speedhack ? speedhack : ~0, pCommonSpeedhackCallback);

	EEPROMInit(&eeprom_interface_93C46);

	YMZ280BInit(14000000, NULL);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);

	deco16Init(0, 0, 1);
	deco16_set_bank_callback(0, backfire_bank_callback);
	deco16_set_bank_callback(1, backfire_bank_callback);
	deco16_set_bank_callback(2, backfire_bank_callback);
	deco16_set_bank_callback(3, backfire_bank_callback);
	deco16_set_color_base(1, 0x400);
	deco16_set_color_base(2, 0x100);
	deco16_set_color_base(3, 0x500);
	deco16_set_graphics(DrvGfxROM0, 0x800000, DrvGfxROM1, 0x800000, DrvGfxROM2, 0x200000);
	deco16_set_global_offsets(0, 8);

	GenericTilesInit();

	nPreviousDip = DrvDips[0] & 0x80;

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	EEPROMExit();

	ArmExit();

	YMZ280BExit();
	YMZ280BROM = NULL;

	GenericTilesExit();

	deco16Exit();

	BurnFree (AllMem);

	return 0;
}

static void simpl156_palette_recalc()
{
	UINT32 *p = (UINT32*)DrvPalRAM;

	for (INT32 i = 0; i < 0x2000 / 4; i++)
	{
		INT32 r = (BURN_ENDIAN_SWAP_INT16(p[i]) >>  0) & 0x1f;
		INT32 g = (BURN_ENDIAN_SWAP_INT16(p[i]) >>  5) & 0x1f;
		INT32 b = (BURN_ENDIAN_SWAP_INT16(p[i]) >> 10) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void draw_sprites(UINT16 *dest, UINT8 *ram, UINT8 *gfx, INT32 coloff)
{
	UINT32 *spriteram = (UINT32*)ram;

	for (INT32 offs = (0x1400 / 4) - 4; offs >= 0; offs -= 4)
	{
		INT32 x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri;

		sprite = BURN_ENDIAN_SWAP_INT32(spriteram[offs + 1]) & 0xffff;

		y = BURN_ENDIAN_SWAP_INT32(spriteram[offs]) & 0xffff;
		flash = y & 0x1000;
		if (flash && (nCurrentFrame & 1))
			continue;

		x = BURN_ENDIAN_SWAP_INT32(spriteram[offs + 2]) & 0xffff;
		colour = (x >> 9) & 0x1f;

		pri = x & 0xc000;

		switch (pri & 0xc000)
		{
			case 0x0000: pri = 0;   break;
			case 0x4000: pri = 0xf0;break;
			case 0x8000: pri = 0;   break;
			case 0xc000: pri = 0xf0;break;
		}

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		if (x > 320) continue;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (1)
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			deco16_draw_prio_sprite(dest, gfx, sprite - multi * inc, (colour<<4)+coloff, x, y + mult * multi, fx, fy, pri);

			multi--;
		}
	}
}

static INT32 DrvDraw()
{
	if ((DrvDips[0] & 0x80) && nPreviousDip == 0) { // single screen
		DrvTmpBitmap0 = pTransDraw;
		BurnDrvSetVisibleSize(320, 240);
		BurnDrvSetAspect(4, 3);
		Reinitialise();
	} else if (!(DrvDips[0] & 0x80) && nPreviousDip == 0x80) { // two screens
		DrvTmpBitmap0 = DrvTmpBitmap_p;
		BurnDrvSetVisibleSize(640, 240);
		BurnDrvSetAspect(8, 3);
		Reinitialise();
	}
	nPreviousDip = DrvDips[0] & 0x80;

	simpl156_palette_recalc();

	deco16_pf12_update();
	deco16_pf34_update();

	if (nPreviousDip == 0) nScreenWidth = 320;

	// left
	{
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			DrvTmpBitmap0[i] = 0x100;
		}

		deco16_clear_prio_map();

		if (priority[0] == 0) {
			deco16_draw_layer(2, DrvTmpBitmap0, 1);
			deco16_draw_layer(0, DrvTmpBitmap0, 2);
		} else if (priority[0] == 2) {
			deco16_draw_layer(0, DrvTmpBitmap0, 2);
			deco16_draw_layer(2, DrvTmpBitmap0, 4);
		}

		draw_sprites(DrvTmpBitmap0, DrvSprRAM0, DrvGfxROM3, 0x200);
	}

	// right
	if (nPreviousDip == 0) {
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			DrvTmpBitmap1[i] = 0x500;
		}

		deco16_clear_prio_map();

		if (priority[1] == 0) {
			deco16_draw_layer(3, DrvTmpBitmap1, 1);
			deco16_draw_layer(1, DrvTmpBitmap1, 2);
		} else if (priority[1] == 2) {
			deco16_draw_layer(1, DrvTmpBitmap1, 2);
			deco16_draw_layer(3, DrvTmpBitmap1, 4);
		}

		draw_sprites(DrvTmpBitmap1, DrvSprRAM1, DrvGfxROM3 /*DrvGfxROM4*/, 0x600);

	// combine

		UINT16 *dst0 = pTransDraw;
		UINT16 *dst1 = pTransDraw + 320;
		UINT16 *src0 = DrvTmpBitmap0;
		UINT16 *src1 = DrvTmpBitmap1;

		for (INT32 y = 0; y < nScreenHeight; y++) {
			memcpy (dst0, src0, 320 * sizeof(UINT16));
			memcpy (dst1, src1, 320 * sizeof(UINT16));

			dst0 += 640;
			dst1 += 640;
			src0 += 320;
			src1 += 320;
		}

		nScreenWidth = 320 * 2;
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
		DrvInputs[0] = 0x00ff;
		DrvInputs[1] = 0x00ff;
		DrvInputs[2] = 0xffe7;

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		DrvInputs[2] |= DrvDips[0] & 0x0008;

		if (DrvDips[0] & 0x80) DrvInputs[1] |= 0x80;
	}

	INT32 nTotalCycles = 28000000 / 60;

	ArmOpen(0);
	deco16_vblank = 0x10;
	ArmRun(nTotalCycles - 2240);
	ArmSetIRQLine(ARM_IRQ_LINE, ARM_HOLD_LINE);
	deco16_vblank = 0x00;
	ArmRun(2240);
	ArmClose();

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
	
	if (pnMin != NULL) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ArmScan(nAction, pnMin);

		YMZ280BScan();

		deco16Scan();
	}

	return 0;
}


// Backfire! (set 1)

static struct BurnRomInfo backfireRomDesc[] = {
	{ "ra00-0.2j",		0x080000, 0x790da069, 1 | BRF_PRG | BRF_ESS }, //  0 Arm code (Encrypted)
	{ "ra01-0.3j",		0x080000, 0x447cb57b, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "mbz-00.9a",		0x200000, 0x1098d504, 2 | BRF_GRA },           //  2 Characters & tiles
	{ "mbz-01.10a",		0x200000, 0x19b81e5c, 2 | BRF_GRA },           //  3

	{ "mbz-02.12a",		0x100000, 0x2bd2b0a1, 3 | BRF_GRA },           //  4 Tiles

	{ "mbz-03.15a",		0x200000, 0x2e818569, 4 | BRF_GRA },           //  5 Left screen sprites
	{ "mbz-04.16a",		0x200000, 0x67bdafb1, 4 | BRF_GRA },           //  6

	{ "mbz-03.18a",		0x200000, 0x2e818569, 5 | BRF_GRA },           //  7 Right screen sprites
	{ "mbz-04.19a",		0x200000, 0x67bdafb1, 5 | BRF_GRA },           //  8

	{ "mbz-05.17l",		0x200000, 0x947c1da6, 6 | BRF_SND },           //  9 YMZ280b Samples
	{ "mbz-06.19l",		0x080000, 0x4a38c635, 6 | BRF_SND },           // 10

	{ "gal16v8b.6b",	0x000117, 0x00000000, 7 | BRF_NODUMP },        // 11 PLDs
	{ "gal16v8b.6d",	0x000117, 0x00000000, 7 | BRF_NODUMP },        // 12
	{ "gal16v8b.12n",	0x000117, 0x00000000, 7 | BRF_NODUMP },        // 13
};

STD_ROM_PICK(backfire)
STD_ROM_FN(backfire)

static INT32 backfireInit()
{
	return DrvInit(0xce44);
}

struct BurnDriver BurnDrvBackfire = {
	"backfire", NULL, NULL, NULL, "1995",
	"Backfire! (set 1)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_RACING, 0,
	NULL, backfireRomInfo, backfireRomName, NULL, NULL, BackfireInputInfo, BackfireDIPInfo,
	backfireInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	640, 240, 8, 3
};


// Backfire! (set 2)

static struct BurnRomInfo backfireaRomDesc[] = {
	{ "rb-00h.h2",		0x080000, 0x60973046, 1 | BRF_PRG | BRF_ESS }, //  0 Arm code (Encrypted)
	{ "rb-01l.h3",		0x080000, 0x27472f60, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "mbz-00.9a",		0x200000, 0x1098d504, 2 | BRF_GRA },           //  2 Characters & tiles
	{ "mbz-01.10a",		0x200000, 0x19b81e5c, 2 | BRF_GRA },           //  3

	{ "mbz-02.12a",		0x100000, 0x2bd2b0a1, 3 | BRF_GRA },           //  4 Tiles

	{ "mbz-03.15a",		0x200000, 0x2e818569, 4 | BRF_GRA },           //  5 Left screen sprites
	{ "mbz-04.16a",		0x200000, 0x67bdafb1, 4 | BRF_GRA },           //  6

	{ "mbz-03.18a",		0x200000, 0x2e818569, 5 | BRF_GRA },           //  7 Right screen sprites
	{ "mbz-04.19a",		0x200000, 0x67bdafb1, 5 | BRF_GRA },           //  8

	{ "mbz-05.17l",		0x200000, 0x947c1da6, 6 | BRF_SND },           //  9 YMZ280b Samples
	{ "mbz-06.19l",		0x080000, 0x4a38c635, 6 | BRF_SND },           // 10
};

STD_ROM_PICK(backfirea)
STD_ROM_FN(backfirea)

static INT32 backfireaInit()
{
	return DrvInit(0xcee4);
}

struct BurnDriverD BurnDrvBackfirea = {
	"backfirea", "backfire", NULL, NULL, "1995",
	"Backfire! (set 2)\0", "Set inputs to \"Joystick\" in test mode", "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_RACING, 0,
	NULL, backfireaRomInfo, backfireaRomName, NULL, NULL, BackfireInputInfo, BackfireDIPInfo,
	backfireaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	640, 240, 8, 3
};
