// FB Alpha Boogie Wing / Great Ragtime Show driver module
// Based on MAME driver by Bryan McPhail and David Haywood

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "h6280_intf.h"
#include "deco16ic.h"
#include "msm6295.h"
#include "burn_ym2151.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KCode;
static UINT8 *DrvHucROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvGfxROM4;
static UINT8 *DrvGfxROM5;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *Drv68KRAM;
static UINT8 *DrvHucRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprRAM1;
static UINT8 *DrvPalBuf;
static UINT8 *DrvSprBuf;
static UINT8 *DrvSprBuf1;
static UINT8 *DrvProtRAM;
static UINT8 *DrvUnkRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *flipscreen;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static UINT16 *tempdraw[2];

static INT32 DrvOkiBank;

static struct BurnInputInfo BoogwingInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 14,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Boogwing)

static struct BurnDIPInfo BoogwingDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x14, 0x01, 0x07, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x07, 0x01, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x14, 0x01, 0x38, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x38, 0x08, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Continue Coin"	},
	{0x14, 0x01, 0x80, 0x80, "Normal Coin Credit"	},
	{0x14, 0x01, 0x80, 0x00, "2 Start/1 Continue"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x01, "1"			},
	{0x15, 0x01, 0x03, 0x03, "2"			},
	{0x15, 0x01, 0x03, 0x02, "3"			},
	{0x15, 0x01, 0x03, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x0c, 0x08, "Easy"			},
	{0x15, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x15, 0x01, 0x0c, 0x04, "Hard"			},
	{0x15, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Coin Slots"		},
	{0x15, 0x01, 0x10, 0x10, "Common"		},
	{0x15, 0x01, 0x10, 0x00, "Individual"		},

	{0   , 0xfe, 0   ,    2, "Stage Reset"		},
	{0x15, 0x01, 0x20, 0x20, "Point of Termination"	},
	{0x15, 0x01, 0x20, 0x00, "Beginning of Stage"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Boogwing)

void __fastcall boogwing_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x220000:
		case 0x220001:
			deco16_priority = data;
		return;

		case 0x240000:
		case 0x240001:
			memcpy (DrvSprBuf , DrvSprRAM , 0x800);
		return;

		case 0x244000:
		case 0x244001:
			memcpy (DrvSprBuf1, DrvSprRAM1, 0x800);
		return;

		case 0x24e151:
			deco16_soundlatch = data;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		break;

		case 0x282008:
		case 0x282009:
			memcpy (DrvPalBuf, DrvPalRAM, 0x2000);
		return;
	}

	if ((address & 0xffff800) == 0x24e000) {
		DrvProtRAM[(address & 0x7ff) ^ 1] = data;
		return;
	}
}

void __fastcall boogwing_main_write_word(UINT32 address, UINT16 data)
{
	deco16_write_control_word(0, address, 0x260000, data)
	deco16_write_control_word(1, address, 0x270000, data)

	switch (address)
	{
		case 0x220000:
			deco16_priority = data;
		return;

		case 0x240000:
			memcpy (DrvSprBuf , DrvSprRAM , 0x800);
		return;

		case 0x244000:
			memcpy (DrvSprBuf1, DrvSprRAM1, 0x800);
		return;

		case 0x24e150:
			deco16_soundlatch = data & 0xff;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		break;

		case 0x282008:
			memcpy (DrvPalBuf, DrvPalRAM, 0x2000);
		return;
	}

	if ((address & 0xffff800) == 0x24e000) {
		*((UINT16*)(DrvProtRAM + (address & 0x7fe))) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
}

UINT8 __fastcall boogwing_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x24e6c0:
			return DrvDips[1];

		case 0x24e6c1:
			return DrvDips[0];

		case 0x24e138:
		case 0x24e139:
			return (DrvInputs[0] & 0x07) | (deco16_vblank & 0x08);

		case 0x24e344:
			return DrvInputs[1] >> 8;

		case 0x24e345:
			return DrvInputs[1] >> 0;
	}

	return 0;
}

UINT16 __fastcall boogwing_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x24e6c0:
			return (DrvDips[1] << 8) | (DrvDips[0] << 0);

		case 0x24e138:
			return (DrvInputs[0] & 0x07) | (deco16_vblank & 0x08);

		case 0x24e344:
			return DrvInputs[1];
	}

	return 0;
}

static INT32 boogwing_bank_callback( const INT32 bank )
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

static INT32 boogwing_bank_callback2( const INT32 bank )
{
	INT32 offset = ((bank >> 4) & 0x7) * 0x1000;
	if ((bank & 0xf) == 0xa) offset += 0x800;

	return offset;
}

static void DrvYM2151WritePort(UINT32, UINT32 data)
{
	if ((data & 0x02) != (UINT32)(DrvOkiBank & 0x02))
		memcpy (DrvSndROM1, DrvSndROM1 + 0x40000 + ((data & 0x02) >> 1) * 0x40000, 0x40000);

	if ((data & 0x01) != (UINT32)(DrvOkiBank & 0x01))
		memcpy (DrvSndROM0, DrvSndROM0 + 0x40000 + ((data & 0x01) >> 0) * 0x40000, 0x40000);

	DrvOkiBank = data;	
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	deco16SoundReset();
	DrvYM2151WritePort(0, 1);

	deco16Reset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x100000;
	Drv68KCode	= Next; Next += 0x100000;
	DrvHucROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x040000;
	DrvGfxROM1	= Next; Next += 0x400000;
	DrvGfxROM5	= Next; Next += 0x100000;
	DrvGfxROM2	= Next; Next += 0x400000;
	DrvGfxROM3	= Next; Next += 0x800000;
	DrvGfxROM4	= Next; Next += 0x800000;

	MSM6295ROM	= Next;
	DrvSndROM0	= Next; Next += 0x100000;
	DrvSndROM1	= Next; Next += 0x0c0000;

	DrvPalette	= (UINT32*)Next; Next += 0x0800 * sizeof(UINT32);

	tempdraw[0]	= (UINT16*)Next; Next += 320 * 240 * sizeof(UINT16);
	tempdraw[1]	= (UINT16*)Next; Next += 320 * 240 * sizeof(UINT16);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvHucRAM	= Next; Next += 0x002000;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvSprRAM1	= Next; Next += 0x000800;
	DrvSprBuf	= Next; Next += 0x000800;
	DrvSprBuf1	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x002000;
	DrvPalBuf	= Next; Next += 0x002000;

	DrvProtRAM	= Next; Next += 0x000800;
	DrvUnkRAM	= Next; Next += 0x000400;

	flipscreen	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}


static INT32 DrvGfxDecode5bpp()
{
	// actually 5 bits / pixel, but fba doesn't seem to like that...
	INT32 Plane[6] = { 0x1800000, 0x1000000, 0x800008, 0x800000, 0x000008, 0x000000 };
	INT32 XOffs[16] = { 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7, 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs[16] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x400000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp + 0x000000, DrvGfxROM1, 0x200000);
	memset (tmp + 0x200000, 0,          0x200000);
	memcpy (tmp + 0x200000, DrvGfxROM5, 0x100000);

	GfxDecode(0x4000, 6, 16, 16, Plane, XOffs, YOffs, 0x200, tmp, DrvGfxROM1);

	for (INT32 i = 0; i < 0x400000; i++) DrvGfxROM1[i] &= 0x1f;

	BurnFree (tmp);

	return 0;
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
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080000,  3, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x000000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  5, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,  6, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x100000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM5 + 0x000000,  9, 2)) return 1; // skip a byte!

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x100000, 11, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 12, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 13, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM4 + 0x000001, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000000, 15, 2)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x040000, 16, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x040000, 17, 1)) return 1;

		deco56_decrypt_gfx(DrvGfxROM0, 0x020000);
		deco56_decrypt_gfx(DrvGfxROM1, 0x300000);
		deco56_decrypt_gfx(DrvGfxROM2, 0x200000);
		deco56_remap_gfx(DrvGfxROM5, 0x100000);
		deco102_decrypt_cpu(Drv68KROM, Drv68KCode, 0x100000, 0x42ba, 0x00, 0x18);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x020000, 1);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x200000, 0); 
		deco16_sprite_decode(DrvGfxROM3, 0x400000);
		deco16_sprite_decode(DrvGfxROM4, 0x400000);

		DrvGfxDecode5bpp();
	}

	deco16Init(0, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x200000 * 2, DrvGfxROM2, 0x200000 * 2);
	deco16_set_color_base(1, 0x100);
	deco16_set_color_base(2, 0x300);
	deco16_set_color_base(3, 0x400);
	deco16_set_global_offsets(0, 8);
	deco16_set_transparency_mask(1, 0x1f);
	deco16_set_color_mask(2, 0x1f);
	deco16_set_color_mask(3, 0x1f);
	deco16_set_bank_callback(1, boogwing_bank_callback);
	deco16_set_bank_callback(2, boogwing_bank_callback2);
	deco16_set_bank_callback(3, boogwing_bank_callback2);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0fffff, SM_READ);
	SekMapMemory(Drv68KCode,		0x000000, 0x0fffff, SM_FETCH);
	SekMapMemory(Drv68KRAM,			0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x242000, 0x2427ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0x246000, 0x2467ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[0],		0x264000, 0x265fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x266000, 0x267fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x268000, 0x268fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x26a000, 0x26afff, SM_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x274000, 0x275fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x276000, 0x277fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x278000, 0x278fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x27a000, 0x27afff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x284000, 0x285fff, SM_RAM);
	SekSetWriteWordHandler(0,		boogwing_main_write_word);
	SekSetWriteByteHandler(0,		boogwing_main_write_byte);
	SekSetReadWordHandler(0,		boogwing_main_read_word);
	SekSetReadByteHandler(0,		boogwing_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 8055000, 0, DrvYM2151WritePort, 0.80, 1006875, 1.40, 2013750, 0.30);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.80, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.80, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	deco16Exit();

	deco16SoundExit();

	SekExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites(UINT8 *ram, UINT8 *gfx, INT32 coloff, INT32 gfx_region, INT32 bpp)
{
	if (bpp != (nBurnBpp & 4)) return;

	UINT16 *spriteram_base = (UINT16*)ram;

	INT32 colmask = (gfx_region == 4) ? 0x0f : 0x1f;

	INT32 sflipscreen = !*flipscreen;
	INT32 priority = deco16_priority;

	for (INT32 offs = 0x400 - 4; offs >= 0; offs -= 4)
	{
		INT32 inc, mult, pri = 0, spri = 0, alpha = 0xff;

		INT32 sprite = BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 1]);

		if (!sprite) continue;

		INT32 y = BURN_ENDIAN_SWAP_INT16(spriteram_base[offs]);

		if ((y & 0x1000) && (nCurrentFrame & 1))
			continue;

		INT32 x = BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]);
		INT32 colour = (x >> 9) & colmask;

		INT32 fx = y & 0x2000;
		INT32 fy = y & 0x4000;
		INT32 multi = (1 << ((y & 0x0600) >> 9)) - 1;

		if (gfx_region == 4)
		{
			if ((BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0xc000) == 0xc000)
				spri = 4;
			else if ((BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0xc000))
				spri = 16;
			else
				spri = 64;

			if (BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0x2000)	alpha = 0x80;

			if (priority == 0x2)
			{
				if (BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0x8000) alpha = 0x80;

				if ((BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0xc000) == 0xc000)
					pri = 4;
				else if ((BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0xc000) == 0x8000)
					pri = 16;
				else
					pri = 64;
			}
			else
			{
				if ((BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0x8000) == 0x8000)
					pri = 16;
				else
					pri = 64;
			}
		}
		else
		{
			if (BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0x8000)
				spri = 8;
			else
				spri = 32;

			if (priority == 0x1)
			{
				if ((BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0xc000))
					pri = 16;
				else
					pri = 64;
			}
			else
			{
				if ((BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0xc000) == 0xc000)
					pri = 4;
				else if ((BURN_ENDIAN_SWAP_INT16(spriteram_base[offs + 2]) & 0xc000) == 0x8000)
					pri = 16;
				else
					pri = 64;
			}
		}

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (sflipscreen)
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

		while (multi >= 0)
		{
			if (!bpp) {
				deco16_draw_prio_sprite(pTransDraw, gfx, sprite - multi * inc, (colour << 4) + coloff, x, y + mult * multi, fx, fy, pri, spri);
			} else {
				deco16_draw_alphaprio_sprite(DrvPalette, gfx, sprite - multi * inc, (colour << 4) + coloff, x, y + mult * multi, fx, fy, pri, spri, alpha);	
			}

			multi--;
		}
	}
}

static void draw_combined_playfield(INT32 color, INT32 priority) // opaque
{
	UINT16 *src0 = tempdraw[0];
	UINT16 *src1 = tempdraw[1];
	UINT16 *dest = pTransDraw;
	UINT8 *prio = deco16_prio_map;

	for (INT32 y = 0; y < nScreenHeight; y++) {
		for (INT32 x = 0; x < nScreenWidth; x++) {
			dest[x] = color | (src0[x] & 0x0f) | ((src1[x] & 0x0f) << 4);
			prio[x] = priority;
		}
		src0 += nScreenWidth;
		src1 += nScreenWidth;
		dest += nScreenWidth;
		prio += 512;
	}
}

static INT32 DrvDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalBuf);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x400;
	}

	if ((deco16_priority & 0x07) == 0x05) {
		UINT8 *tptr = deco16_pf_rowscroll[3];
		deco16_pf_rowscroll[3] = deco16_pf_rowscroll[2];

		deco16_draw_layer(2, tempdraw[0], 0x10000);
		deco16_draw_layer(3, tempdraw[1], 0x10000);

		deco16_pf_rowscroll[3] = tptr;
	}

	deco16_clear_prio_map();

	INT32 bpp0 = (deco16_get_tilemap_size(0) == 1) ? DECO16_LAYER_5BITSPERPIXEL : DECO16_LAYER_4BITSPERPIXEL;
	INT32 bpp1 = (deco16_get_tilemap_size(1) == 1) ? DECO16_LAYER_5BITSPERPIXEL : DECO16_LAYER_4BITSPERPIXEL;

	switch (deco16_priority & 0x07)
	{
		case 0x01:
		case 0x02:
			if (nSpriteEnable & 4) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_PRIORITY(0x00) | DECO16_LAYER_OPAQUE);
			if (nSpriteEnable & 1) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x08) | bpp1);
			if (nSpriteEnable & 2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x20));
		break;

		case 0x03:
			if (nSpriteEnable & 4) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_PRIORITY(0x00) | DECO16_LAYER_OPAQUE);
			if (nSpriteEnable & 1) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x08) | bpp1);
			// should use pf2 to add shadows...
		break;

		case 0x05:
			if (nSpriteEnable & 1) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x00) | DECO16_LAYER_OPAQUE);
			if (nSpriteEnable &16) draw_combined_playfield(0x300, DECO16_LAYER_PRIORITY(0x20));
		break;

		case 0x00:
		case 0x04:
		case 0x06:
		case 0x07:
			if (nSpriteEnable & 4) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_PRIORITY(0x00) | DECO16_LAYER_OPAQUE);
			if (nSpriteEnable & 2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x08));
			if (nSpriteEnable & 1) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x20) | bpp1);
		break;
	}

	draw_sprites(DrvSprBuf , DrvGfxROM3, 0x500, 3, 0);
	draw_sprites(DrvSprBuf1, DrvGfxROM4, 0x700, 4, 0);

	if (nSpriteEnable & 8) deco16_draw_layer(0, pTransDraw, DECO16_LAYER_PRIORITY(0xff) | bpp0);

	BurnTransferCopy(DrvPalette);

	draw_sprites(DrvSprBuf , DrvGfxROM3, 0x500, 3, 4);
	draw_sprites(DrvSprBuf1, DrvGfxROM4, 0x700, 4, 4);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 2 * sizeof(INT16)); 
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nInterleave = 256;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 14000000 / 58, 8055000 / 58 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	h6280Open(0);

	deco16_vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		nCyclesDone[1] += h6280Run(nCyclesTotal[1] / nInterleave);

		if (i == 248) deco16_vblank = 0x08;
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
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
		
		SCAN_VAR(DrvOkiBank);

		INT32 bank = DrvOkiBank;
		DrvOkiBank = -1;
		DrvYM2151WritePort(0, bank);
	}

	return 0;
}


// Boogie Wings (Euro v1.5, 92.12.07)

static struct BurnRomInfo boogwingRomDesc[] = {
	{ "kn_00-2.2b",		0x040000, 0xe38892b9,  1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "kn_02-2.2e",		0x040000, 0x8426efef,  1 | BRF_PRG | BRF_ESS }, //  1
	{ "kn_01-2.4b",		0x040000, 0x3ad4b54c,  1 | BRF_PRG | BRF_ESS }, //  2
	{ "kn_03-2.4e",		0x040000, 0x10b61f4a,  1 | BRF_PRG | BRF_ESS }, //  3

	{ "km06.18p",		0x010000, 0x3e8bc4e1,  2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "km05.9e",		0x010000, 0xd10aef95,  3 | BRF_GRA },           //  5 Characters
	{ "km04.8e",		0x010000, 0x329323a8,  3 | BRF_GRA },           //  6

	{ "mbd-01.9b",		0x100000, 0xd7de4f4b,  4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mbd-00.8b",		0x100000, 0xadb20ba9,  4 | BRF_GRA },           //  8
	{ "mbd-02.10e",		0x080000, 0xb25aa721,  8 | BRF_GRA },           //  9

	{ "mbd-03.13b",		0x100000, 0xcf798f2c,  5 | BRF_GRA },           // 10 Background Tiles
	{ "mbd-04.14b",		0x100000, 0xd9764d0b,  5 | BRF_GRA },           // 11

	{ "mbd-05.16b",		0x200000, 0x1768c66a,  6 | BRF_GRA },           // 12 Sprite Bank A
	{ "mbd-06.17b",		0x200000, 0x7750847a,  6 | BRF_GRA },           // 13

	{ "mbd-07.18b",		0x200000, 0x241faac1,  7 | BRF_GRA },           // 14 Sprite Bank B
	{ "mbd-08.19b",		0x200000, 0xf13b1e56,  7 | BRF_GRA },           // 15

	{ "mbd-10.17p",		0x080000, 0xf159f76a,  9 | BRF_SND },           // 16 OKI M6295 Samples 0

	{ "mbd-09.16p",		0x080000, 0xf44f2f87, 10 | BRF_SND },           // 17 OKI M6295 Samples 1

	{ "kj-00.15n",		0x000400, 0xadd4d50b, 11 | BRF_OPT },           // 18 Unknown PROMs
};

STD_ROM_PICK(boogwing)
STD_ROM_FN(boogwing)

struct BurnDriver BurnDrvBoogwing = {
	"boogwing", NULL, NULL, NULL, "1992",
	"Boogie Wings (Euro v1.5, 92.12.07)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, boogwingRomInfo, boogwingRomName, NULL, NULL, BoogwingInputInfo, BoogwingDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Boogie Wings (Asia v1.5, 92.12.07)

static struct BurnRomInfo boogwingaRomDesc[] = {
	{ "km_00-2.2b",		0x040000, 0x71ab71c6,  1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "km_02-2.2e",		0x040000, 0xe90f07f9,  1 | BRF_PRG | BRF_ESS }, //  1
	{ "km_01-2.4b",		0x040000, 0x7fdce2d3,  1 | BRF_PRG | BRF_ESS }, //  2
	{ "km_03-2.4e",		0x040000, 0x0b582de3,  1 | BRF_PRG | BRF_ESS }, //  3

	{ "km06.18p",		0x010000, 0x3e8bc4e1,  2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "km05.9e",		0x010000, 0xd10aef95,  3 | BRF_GRA },           //  5 Characters
	{ "km04.8e",		0x010000, 0x329323a8,  3 | BRF_GRA },           //  6

	{ "mbd-01.9b",		0x100000, 0xd7de4f4b,  4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mbd-00.8b",		0x100000, 0xadb20ba9,  4 | BRF_GRA },           //  8
	{ "mbd-02.10e",		0x080000, 0xb25aa721,  4 | BRF_GRA },           //  9

	{ "mbd-03.13b",		0x100000, 0xcf798f2c,  5 | BRF_GRA },           // 10 Background Tiles
	{ "mbd-04.14b",		0x100000, 0xd9764d0b,  5 | BRF_GRA },           // 11

	{ "mbd-05.16b",		0x200000, 0x1768c66a,  6 | BRF_GRA },           // 12 Sprite Bank A
	{ "mbd-06.17b",		0x200000, 0x7750847a,  6 | BRF_GRA },           // 13

	{ "mbd-07.18b",		0x200000, 0x241faac1,  7 | BRF_GRA },           // 14 Sprite Bank B
	{ "mbd-08.19b",		0x200000, 0xf13b1e56,  7 | BRF_GRA },           // 15

	{ "mbd-10.17p",		0x080000, 0xf159f76a,  8 | BRF_SND },           // 16 OKI M6295 Samples 0

	{ "mbd-09.16p",		0x080000, 0xf44f2f87,  9 | BRF_SND },           // 17 OKI M6295 Samples 1

	{ "kj-00.15n",		0x000400, 0xadd4d50b, 10 | BRF_OPT },           // 18 Unknown PROMs
};

STD_ROM_PICK(boogwinga)
STD_ROM_FN(boogwinga)

struct BurnDriver BurnDrvBoogwinga = {
	"boogwinga", "boogwing", NULL, NULL, "1992",
	"Boogie Wings (Asia v1.5, 92.12.07)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, boogwingaRomInfo, boogwingaRomName, NULL, NULL, BoogwingInputInfo, BoogwingDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// The Great Ragtime Show (Japan v1.5, 92.12.07)

static struct BurnRomInfo ragtimeRomDesc[] = {
	{ "kh_00-2.2b",		0x040000, 0x553e179f,  1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "kh_02-2.2e",		0x040000, 0x6c759ec0,  1 | BRF_PRG | BRF_ESS }, //  1
	{ "kh_01-2.4b",		0x040000, 0x12dfee70,  1 | BRF_PRG | BRF_ESS }, //  2
	{ "kh_03-2.4e",		0x040000, 0x076fea18,  1 | BRF_PRG | BRF_ESS }, //  3

	{ "km06.18p",		0x010000, 0x3e8bc4e1,  2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "km05.9e",		0x010000, 0xd10aef95,  3 | BRF_GRA },           //  5 Characters
	{ "km04.8e",		0x010000, 0x329323a8,  3 | BRF_GRA },           //  6

	{ "mbd-01.9b",		0x100000, 0xd7de4f4b,  4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mbd-00.8b",		0x100000, 0xadb20ba9,  4 | BRF_GRA },           //  8
	{ "mbd-02.10e",		0x080000, 0xb25aa721,  8 | BRF_GRA },           //  9

	{ "mbd-03.13b",		0x100000, 0xcf798f2c,  5 | BRF_GRA },           // 10 Background Tiles
	{ "mbd-04.14b",		0x100000, 0xd9764d0b,  5 | BRF_GRA },           // 11

	{ "mbd-05.16b",		0x200000, 0x1768c66a,  6 | BRF_GRA },           // 12 Sprite Bank A
	{ "mbd-06.17b",		0x200000, 0x7750847a,  6 | BRF_GRA },           // 13

	{ "mbd-07.18b",		0x200000, 0x241faac1,  7 | BRF_GRA },           // 14 Sprite Bank B
	{ "mbd-08.19b",		0x200000, 0xf13b1e56,  7 | BRF_GRA },           // 15

	{ "mbd-10.17p",		0x080000, 0xf159f76a,  9 | BRF_SND },           // 16 OKI M6295 Samples 0

	{ "mbd-09.16p",		0x080000, 0xf44f2f87, 10 | BRF_SND },           // 17 OKI M6295 Samples 1

	{ "kj-00.15n",		0x000400, 0xadd4d50b, 11 | BRF_OPT },           // 18 Unknown PROMs
};

STD_ROM_PICK(ragtime)
STD_ROM_FN(ragtime)

struct BurnDriver BurnDrvRagtime = {
	"ragtime", "boogwing", NULL, NULL, "1992",
	"The Great Ragtime Show (Japan v1.5, 92.12.07)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, ragtimeRomInfo, ragtimeRomName, NULL, NULL, BoogwingInputInfo, BoogwingDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// The Great Ragtime Show (Japan v1.3, 92.11.26)

static struct BurnRomInfo ragtimeaRomDesc[] = {
	{ "kh_00-1.2b",		0x040000, 0x88f0155a,  1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "kh_02-1.2e",		0x040000, 0x8811b41b,  1 | BRF_PRG | BRF_ESS }, //  1
	{ "kh_01-1.4b",		0x040000, 0x4dab63ad,  1 | BRF_PRG | BRF_ESS }, //  2
	{ "kh_03-1.4e",		0x040000, 0x8a4cbb18,  1 | BRF_PRG | BRF_ESS }, //  3

	{ "km06.18p",		0x010000, 0x3e8bc4e1,  2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "km05.9e",		0x010000, 0xd10aef95,  3 | BRF_GRA },           //  5 Characters
	{ "km04.8e",		0x010000, 0x329323a8,  3 | BRF_GRA },           //  6

	{ "mbd-01.9b",		0x100000, 0xd7de4f4b,  4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mbd-00.8b",		0x100000, 0xadb20ba9,  4 | BRF_GRA },           //  8
	{ "mbd-02.10e",		0x080000, 0xb25aa721,  8 | BRF_GRA },           //  9

	{ "mbd-03.13b",		0x100000, 0xcf798f2c,  5 | BRF_GRA },           // 10 Background Tiles
	{ "mbd-04.14b",		0x100000, 0xd9764d0b,  5 | BRF_GRA },           // 11

	{ "mbd-05.16b",		0x200000, 0x1768c66a,  6 | BRF_GRA },           // 12 Sprite Bank A
	{ "mbd-06.17b",		0x200000, 0x7750847a,  6 | BRF_GRA },           // 13

	{ "mbd-07.18b",		0x200000, 0x241faac1,  7 | BRF_GRA },           // 14 Sprite Bank B
	{ "mbd-08.19b",		0x200000, 0xf13b1e56,  7 | BRF_GRA },           // 15

	{ "mbd-10.17p",		0x080000, 0xf159f76a,  9 | BRF_SND },           // 16 OKI M6295 Samples 0

	{ "mbd-09.16p",		0x080000, 0xf44f2f87, 10 | BRF_SND },           // 17 OKI M6295 Samples 1

	{ "kj-00.15n",		0x000400, 0xadd4d50b, 11 | BRF_OPT },           // 18 Unknown PROMs
};

STD_ROM_PICK(ragtimea)
STD_ROM_FN(ragtimea)

struct BurnDriver BurnDrvRagtimea = {
	"ragtimea", "boogwing", NULL, NULL, "1992",
	"The Great Ragtime Show (Japan v1.3, 92.11.26)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, ragtimeaRomInfo, ragtimeaRomName, NULL, NULL, BoogwingInputInfo, BoogwingDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};
