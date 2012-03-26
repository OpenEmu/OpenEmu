// FB Alpha Speed Spin driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "zet.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvVidRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvAttrRAM;

static UINT32  *DrvPalette;
static UINT8  DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *display_enable;
static UINT8 *banks;

static UINT8  DrvInputs[3];
static UINT8  DrvJoy1[8];
static UINT8  DrvJoy2[8];
static UINT8  DrvJoy3[8];
static UINT8  DrvDips[2];
static UINT8  DrvReset;

static INT32 black_color;

static struct BurnInputInfo SpeedspnInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 7,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Speedspn)

static struct BurnDIPInfo SpeedspnDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x13, 0x01, 0x0f, 0x01, "5 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x04, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x00, "5 Coins/2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x06, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x03, "4 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0f, 0x02, "4 Coins 5 Credits"	},
	{0x13, 0x01, 0x0f, 0x05, "3 Coins/5 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x08, "2 Coins 5 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,   16, "Coin B"		},
	{0x13, 0x01, 0xf0, 0x10, "5 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x40, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x00, "5 Coins/2 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x60, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0x30, "4 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x20, "4 Coins 5 Credits"	},
	{0x13, 0x01, 0xf0, 0x50, "3 Coins/5 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "2 Coins 5 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    2, "World Cup"		},
	{0x14, 0x01, 0x01, 0x00, "Off"			},
	{0x14, 0x01, 0x01, 0x01, "On"			},

	{0   , 0xfe, 0   ,    2, "Backhand"		},
	{0x14, 0x01, 0x02, 0x02, "Automatic"		},
	{0x14, 0x01, 0x02, 0x00, "Manual"		},

	{0   , 0xfe, 0   ,    4, "Points to Win"	},
	{0x14, 0x01, 0x0c, 0x0c, "11 Points and 1 Adv."	},
	{0x14, 0x01, 0x0c, 0x08, "11 Points and 2 Adv."	},
	{0x14, 0x01, 0x0c, 0x04, "21 Points and 1 Adv."	},
	{0x14, 0x01, 0x0c, 0x00, "21 Points and 2 Adv."	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x30, 0x20, "Easy"			},
	{0x14, 0x01, 0x30, 0x30, "Normal"		},
	{0x14, 0x01, 0x30, 0x10, "Hard"			},
	{0x14, 0x01, 0x30, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x80, 0x80, "Off"			},
	{0x14, 0x01, 0x80, 0x00, "On"			},

};

STDDIPINFO(Speedspn)

static void bankswitch(INT32 data)
{
	const INT32 offsets[10] = {
		0x30000, 0x1c000, 0x24000, 0x5c000, 0x50000, 0x44000, 0x20000, 0x54000, 0x58000, 0x08000
	};

	banks[0] = (data > 8) ? 9 : data;

	ZetMapArea(0xc000, 0xffff, 0, DrvZ80ROM0 + offsets[banks[0]]);
	ZetMapArea(0xc000, 0xffff, 2, DrvZ80ROM0 + offsets[banks[0]]);
}

static void vidram_bankswitch(INT32 data)
{
	banks[1] = data & 1;

	ZetMapArea(0x9000, 0x9fff, 0, DrvVidRAM + banks[1] * 0x1000);
	ZetMapArea(0x9000, 0x9fff, 1, DrvVidRAM + banks[1] * 0x1000);
	ZetMapArea(0x9000, 0x9fff, 2, DrvVidRAM + banks[1] * 0x1000);
}

static inline void speedspn_palette_write(INT32 offset)
{
	INT32 r = DrvPalRAM[offset + 1] & 0x0f;
	INT32 g = DrvPalRAM[offset + 0] >> 4;
	INT32 b = DrvPalRAM[offset + 0] & 0x0f;

	offset /= 2;

	DrvPalette[offset] = BurnHighCol((r << 4) | r, (g << 4) | g, (b << 4) | b, 0);
	if (DrvPalette[offset] == 0) black_color = offset;
}

void __fastcall speedspn_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf800) == 0x8000) {
		address &= 0x7ff;

		if (data != DrvPalRAM[address]) {
			DrvPalRAM[address] = data;
		
			speedspn_palette_write(address & ~1);
		}
	}
}

void __fastcall speedspn_main_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x01:
		case 0x10: // ?
		return;

		case 0x07:
			*display_enable = ~data & 1;
		return;

		case 0x12:
			bankswitch(data);
		return;

		case 0x13:
			*soundlatch = data;
			ZetClose();
			ZetOpen(1);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			ZetOpen(0);
		return;

		case 0x17:
			vidram_bankswitch(data);
		return;
	}
}

UINT8 __fastcall speedspn_main_read_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x10:
			return DrvInputs[0];

		case 0x11:
			return DrvInputs[1];

		case 0x12:
			return DrvInputs[2];

		case 0x13:
			return DrvDips[0];

		case 0x14:
			return DrvDips[1];

		case 0x16:
			return 0; // irq ack?
	}

	return 0;
}

static void oki_bankswitch(INT32 data)
{
	banks[2] = data & 3;

	MSM6295ROM = DrvSndROM + (banks[2] * 0x40000);

	for (INT32 nChannel = 0; nChannel < 4; nChannel++) {
		MSM6295SampleInfo[0][nChannel] = MSM6295ROM + (nChannel << 8);
		MSM6295SampleData[0][nChannel] = MSM6295ROM + (nChannel << 16);
	}
}

void __fastcall speedspn_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x9000:
			oki_bankswitch(data);
		return;

		case 0x9800:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall speedspn_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x9800:
			return MSM6295ReadStatus(0); 

		case 0xa000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	bankswitch(0);
	vidram_bankswitch(0);
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	oki_bankswitch(1);
	MSM6295Reset(0);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x090000;
	DrvZ80ROM1	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x100000;
	DrvGfxROM1	= Next; Next += 0x080000;

	DrvSndROM	= Next; Next += 0x100000;

	DrvPalette	= (UINT32*)Next; Next += 0x400 * sizeof(UINT32);

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x002000;
	DrvZ80RAM1	= Next; Next += 0x000800;

	DrvVidRAM	= Next; Next += 0x002000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvAttrRAM	= Next; Next += 0x000800;

	soundlatch	= Next; Next += 0x000001;
	display_enable	= Next; Next += 0x000001;
	banks		= Next; Next += 0x000003;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4]  = { 0x200000, 0x300000, 0x000000, 0x100000 };
	INT32 XOffs0[8]  = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007 };
	INT32 YOffs0[8]  = { 0x038, 0x030, 0x028, 0x020, 0x018, 0x010, 0x008, 0x000 };

	INT32 Plane1[4]  = { 0x000004, 0x000000, 0x100004, 0x100000 };
	INT32 XOffs1[16] = { 0x10b, 0x10a, 0x109, 0x108, 0x103, 0x102, 0x101, 0x100,
			   0x00b, 0x00a, 0x009, 0x008, 0x003, 0x002, 0x001, 0x000 };
	INT32 YOffs1[16] = { 0x0f0, 0x0e0, 0x0d0, 0x0c0, 0x0b0, 0x0a0, 0x090, 0x080,
			   0x070, 0x060, 0x050, 0x040, 0x030, 0x020, 0x010, 0x000 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < 0x80000; i++) tmp[i] = DrvGfxROM0[i] ^ 0xff;

	GfxDecode(0x4000, 4,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	for (INT32 i = 0; i < 0x40000; i++) tmp[i] = DrvGfxROM1[i] ^ 0xff;

	GfxDecode(0x0800, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM1);

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
		if (BurnLoadRom(DrvZ80ROM0 + 0x000000,  0, 1)) return 1;
		memmove (DrvZ80ROM0 + 0x10000, DrvZ80ROM0 + 0x08000, 0x78000);

		if (BurnLoadRom(DrvZ80ROM1 + 0x000000,  1, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 1)) return 1;
		memcpy (DrvSndROM + 0x000000, DrvGfxROM0 + 0x000000, 0x020000);
		memcpy (DrvSndROM + 0x040000, DrvGfxROM0 + 0x000000, 0x020000);
		memcpy (DrvSndROM + 0x080000, DrvGfxROM0 + 0x000000, 0x020000);
		memcpy (DrvSndROM + 0x0c0000, DrvGfxROM0 + 0x000000, 0x020000);
		memcpy (DrvSndROM + 0x020000, DrvGfxROM0 + 0x000000, 0x020000);
		memcpy (DrvSndROM + 0x060000, DrvGfxROM0 + 0x020000, 0x020000);
		memcpy (DrvSndROM + 0x0a0000, DrvGfxROM0 + 0x040000, 0x020000);
		memcpy (DrvSndROM + 0x0e0000, DrvGfxROM0 + 0x060000, 0x020000);

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x020000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x040000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x060000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x020000,  8, 1)) return 1;

		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM0);
	ZetMapArea(0x8000, 0x87ff, 0, DrvPalRAM);
//	ZetMapArea(0x8000, 0x87ff, 1, DrvPalRAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvPalRAM);
	ZetMapArea(0x8800, 0x8fff, 0, DrvAttrRAM);
	ZetMapArea(0x8800, 0x8fff, 1, DrvAttrRAM);
	ZetMapArea(0x8800, 0x8fff, 2, DrvAttrRAM);
	ZetMapArea(0xa000, 0xbfff, 0, DrvZ80RAM0);
	ZetMapArea(0xa000, 0xbfff, 1, DrvZ80RAM0);
	ZetMapArea(0xa000, 0xbfff, 2, DrvZ80RAM0);
	ZetSetWriteHandler(speedspn_main_write);
	ZetSetOutHandler(speedspn_main_write_port);
	ZetSetInHandler(speedspn_main_read_port);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(speedspn_sound_write);
	ZetSetReadHandler(speedspn_sound_read);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1122000 / 132, 100.0, 0);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();

	MSM6295Exit(0);

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	return 0;
}

static void draw_layer()
{
	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 sx = (((offs / 32) << 3) + 192) & 0x1ff;
		INT32 sy = (((offs & 0x1f) << 3) - 8) & 0x0ff;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 code  = DrvVidRAM[offs * 2 + 1] | ((DrvVidRAM[offs * 2 + 0] & 0x3f) << 8);
		INT32 color = DrvAttrRAM[offs ^ 0x400];

		if (color & 0x80) {
			Render8x8Tile_FlipX(pTransDraw, code, sx, sy, color & 0x3f, 4, 0, DrvGfxROM0);
		} else {
			Render8x8Tile(pTransDraw, code, sx, sy, color & 0x3f, 4, 0, DrvGfxROM0);
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0x1000; offs < 0x2000; offs += 4)
	{
		INT32 sy   = DrvVidRAM[offs + 3];
		INT32 attr = DrvVidRAM[offs + 2];
		INT32 code = DrvVidRAM[offs + 1] + ((attr & 0xe0) << 3);
		INT32 sx   = DrvVidRAM[offs + 0] + ((attr & 0x10) << 4);

		if (attr == 0 && sx != 0)	// fix junk on char. select screen
			break;

		if (attr == 0 && code == 0)	// speedup
			continue;

		Render16x16Tile_Mask_Clip(pTransDraw, code, (0x1f8 - sx) - 64, sy - 8, attr & 0x0f, 4, 0x0f, 0, DrvGfxROM1);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x800; i+=2) {
			speedspn_palette_write(i);
		}
		DrvRecalc = 0;
	}

	if (*display_enable)
	{
		draw_layer();
		draw_sprites();
	}
	else
	{
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			pTransDraw[i] = black_color;
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
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= DrvJoy1[i] << i;
			DrvInputs[1] ^= DrvJoy2[i] << i;
			DrvInputs[2] ^= DrvJoy3[i] << i;
		}
	}

	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { 6000000 / 60, 6000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nSegment;

		ZetOpen(0);
		nSegment = nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == (nInterleave - 1)) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();

		ZetOpen(1);
		nCyclesDone[1] += ZetRun(nSegment);
		ZetClose();
	}

	if (pBurnSoundOut) {
		MSM6295Render(0,pBurnSoundOut, nBurnSoundLen);
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
		ZetScan(nAction);

		MSM6295Scan(0, nAction);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		bankswitch(banks[0]);
		vidram_bankswitch(banks[1]);
		ZetClose();

		oki_bankswitch(banks[2]);

		DrvRecalc = 1;
	}

	return 0;
}


// Speed Spin

static struct BurnRomInfo speedspnRomDesc[] = {
	{ "tch-ss1.u78",	0x80000, 0x41b6b45b, 1 | BRF_PRG | BRF_ESS }, //  0 Main Z80 Code

	{ "tch-ss2.u96",	0x10000, 0x4611fd0c, 2 | BRF_PRG | BRF_ESS }, //  1 Sound Z80 Code

	{ "tch-ss3.u95",	0x80000, 0x1c9deb5e, 3 | BRF_SND           }, //  2 Samples

	{ "tch-ss4.u70",	0x20000, 0x41517859, 4 | BRF_GRA           }, //  3 Background Tiles
	{ "tch-ss5.u69",	0x20000, 0x832b2f34, 4 | BRF_GRA           }, //  4
	{ "tch-ss6.u60",	0x20000, 0xf1fd7289, 4 | BRF_GRA           }, //  5
	{ "tch-ss7.u59",	0x20000, 0xc4958543, 4 | BRF_GRA           }, //  6

	{ "tch-ss8.u39",	0x20000, 0x2f27b16d, 5 | BRF_GRA           }, //  7 Sprites
	{ "tch-ss9.u34",	0x20000, 0xc372f8ec, 5 | BRF_GRA           }, //  8
};

STD_ROM_PICK(speedspn)
STD_ROM_FN(speedspn)

struct BurnDriver BurnDrvSpeedspn = {
	"speedspn", NULL, NULL, NULL, "1994",
	"Speed Spin\0", NULL, "TCH", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_SPORTSMISC, 0,
	NULL, speedspnRomInfo, speedspnRomName, NULL, NULL, SpeedspnInputInfo, SpeedspnDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	384, 240, 4, 3
};
