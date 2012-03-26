// FB Alpha Gals Pinball driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym3812.h"
#include "msm6295.h"
#include "bitswap.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM0;
static UINT8 *Drv68KRAM1;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvColRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;

static UINT32 *DrvPalette;
static UINT8   DrvRecalc;

static UINT8 *soundlatch;

static UINT16 DrvInputs[3];
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[2];
static UINT8 DrvReset;

static INT32 game_select;

static struct BurnInputInfo GalspnblInputList[] = {
	{"Coin 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"Coin 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"	},
	{"Left Flippers",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"Right Flippers",	BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"Launch Ball / Tilt",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Galspnbl)

static struct BurnDIPInfo GalspnblDIPList[]=
{
	{0x07, 0xff, 0xff, 0xff, NULL			},
	{0x08, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x07, 0x01, 0x03, 0x02, "Easy"			},
	{0x07, 0x01, 0x03, 0x03, "Normal"		},
	{0x07, 0x01, 0x03, 0x01, "Hard"			},
	{0x07, 0x01, 0x03, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    4, "Extra Ball"		},
	{0x07, 0x01, 0x0c, 0x04, "100K 500K"		},
	{0x07, 0x01, 0x0c, 0x0c, "200K 800K"		},
	{0x07, 0x01, 0x0c, 0x08, "200K only"		},
	{0x07, 0x01, 0x0c, 0x00, "None"			},

	{0   , 0xfe, 0   ,    2, "Hit Difficulty"	},
	{0x07, 0x01, 0x10, 0x10, "Normal"		},
	{0x07, 0x01, 0x10, 0x00, "Hard"			},

	{0   , 0xfe, 0   ,    2, "Slide Show"		},
	{0x07, 0x01, 0x20, 0x20, "Off"			},
	{0x07, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x07, 0x01, 0x40, 0x00, "Off"			},
	{0x07, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    0, "Coin A"		},
	{0x08, 0x01, 0x07, 0x02, "4 Coins 1 Credits"	},
	{0x08, 0x01, 0x07, 0x03, "3 Coins 1 Credits"	},
	{0x08, 0x01, 0x07, 0x04, "2 Coins 1 Credits"	},
	{0x08, 0x01, 0x07, 0x01, "2 Coins/1 Credit 3/2"	},
	{0x08, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x08, 0x01, 0x07, 0x06, "1 Coin/1 Credit 2/3"	},
	{0x08, 0x01, 0x07, 0x00, "1 Coin/1 Credit 5/6"	},
	{0x08, 0x01, 0x07, 0x05, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x08, 0x01, 0x38, 0x10, "4 Coins 1 Credits"	},
	{0x08, 0x01, 0x38, 0x18, "3 Coins 1 Credits"	},
	{0x08, 0x01, 0x38, 0x20, "2 Coins 1 Credits"	},
	{0x08, 0x01, 0x38, 0x08, "2 Coins/1 Credit 3/2"	},
	{0x08, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x08, 0x01, 0x38, 0x30, "1 Coin/1 Credit 2/3"	},
	{0x08, 0x01, 0x38, 0x00, "1 Coin/1 Credit 5/6"	},
	{0x08, 0x01, 0x38, 0x28, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    8, "Balls"		},
	{0x08, 0x01, 0xc0, 0x00, "2"			},
	{0x08, 0x01, 0xc0, 0xc0, "3"			},
	{0x08, 0x01, 0xc0, 0x80, "4"			},
	{0x08, 0x01, 0xc0, 0x40, "5"			},
};

STDDIPINFO(Galspnbl)

void __fastcall galspnbl_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0xa80011:
			*soundlatch = data;
			ZetNmi();
		return;

		case 0xa80051: // scroll (unused)
		return;
	}
}

UINT8 __fastcall galspnbl_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xa80001:
			return DrvInputs[0];

		case 0xa80011:
			return DrvInputs[1];

		case 0xa80021:
			return DrvInputs[2] ^ 0x03;

		case 0xa80031:
			return DrvDips[0];

		case 0xa80041:
			return DrvDips[1];
	}

	return 0;
}

static inline void palette_update(UINT16 i)
{
	UINT16 p = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvPalRAM + i)));

	INT32 b = (p >> 8) & 0x0f;
	INT32 g = (p >> 4) & 0x0f;
	INT32 r = (p >> 0) & 0x0f;

	r |= r << 4;
	g |= g << 4;
	b |= b << 4;

	DrvPalette[0x8000+(i/2)] = BurnHighCol(r, g, b, 0);
}

void __fastcall galspnbl_palette_write_word(UINT32 address, UINT16 data)
{
	*((UINT16*)(DrvPalRAM + (address & 0x7fe))) = BURN_ENDIAN_SWAP_INT16(data);
	palette_update(address & 0x7fe);
}

void __fastcall galspnbl_palette_write_byte(UINT32 address, UINT8 data)
{
	DrvPalRAM[(address & 0x7ff) ^ 1] = data;
	palette_update(address & 0x7fe);
}

void __fastcall galspnbl_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xf800:
			MSM6295Command(0, data);
		return;

		case 0xf810:
		case 0xf811:
			BurnYM3812Write(address & 1, data);
		return;
	}
}

UINT8 __fastcall galspnbl_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xf800:
			return MSM6295ReadStatus(0);

		case 0xf810:
		case 0xf811:
			return BurnYM3812Read(address & 1);

		case 0xfc20:
			return *soundlatch;
	}

	return 0;
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static void DrvYM3812IrqHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	BurnYM3812Reset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x400000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x100000;
	DrvGfxROM1		= Next; Next += 0x080000;

	MSM6295ROM		= Next;
	DrvSndROM		= Next; Next += 0x040000;

	DrvPalette		= (UINT32*)Next; Next += 0x8400 * sizeof(UINT32);

	AllRam			= Next;

	Drv68KRAM0		= Next; Next += 0x004000;
	Drv68KRAM1		= Next; Next += 0x008000;
	DrvVidRAM0		= Next; Next += 0x004000;
	DrvVidRAM1		= Next; Next += 0x040000;
	DrvColRAM		= Next; Next += 0x001000;
	DrvSprRAM		= Next; Next += 0x001000;
	DrvPalRAM		= Next; Next += 0x000800;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;
	
	return 0;
}

static void DrvStaticPaletteInit()
{
	for (INT32 i = 0; i < 0x8000; i++)
	{
		INT32 r = (BURN_ENDIAN_SWAP_INT16(i) >>  5) & 0x1f;
		INT32 g = (BURN_ENDIAN_SWAP_INT16(i) >> 10) & 0x1f;
		INT32 b = (BURN_ENDIAN_SWAP_INT16(i) >>  0) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void DrvGfxExpand(UINT8 *s, INT32 len)
{
	for (INT32 i = (len - 1) * 2; i >= 0; i -= 2)
	{
		s[i+1] = s[i/2] & 0x0f;
		s[i+0] = s[i/2] >> 4;
	}
}

static INT32 DrvInit(INT32 select)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x100001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x100000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x200001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x200000,  5, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x300001,  6, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x300000,  7, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  9, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001, 10, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 11, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x000001, 12, 2)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 13, 1)) return 1;

		DrvGfxExpand(DrvGfxROM0, 0x80000);
		DrvGfxExpand(DrvGfxROM1, 0x40000);
		DrvStaticPaletteInit();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x3fffff, SM_ROM);
	SekMapMemory(Drv68KRAM0,	0x700000, 0x703fff, SM_RAM);
	SekMapMemory(Drv68KRAM1,	0x708000, 0x70ffff, SM_RAM);
	SekMapMemory(Drv68KRAM0,	0x800000, 0x803fff, SM_RAM);
	SekMapMemory(Drv68KRAM1,	0x808000, 0x80ffff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x880000, 0x880fff, SM_RAM);
	SekMapMemory(DrvColRAM,		0x900000, 0x900fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,	0x904000, 0x904fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x980000, 0x9bffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0xa01000, 0xa017ff, SM_ROM);
	SekMapHandler(1,		0xa01000, 0xa017ff, SM_WRITE);
//	SekSetWriteWordHandler(0,	galspnbl_main_write_word);
	SekSetWriteByteHandler(0,	galspnbl_main_write_byte);
//	SekSetReadWordHandler(0,	galspnbl_main_read_word);
	SekSetReadByteHandler(0,	galspnbl_main_read_byte);
	SekSetWriteByteHandler(1, 	galspnbl_palette_write_byte);
	SekSetWriteWordHandler(1, 	galspnbl_palette_write_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80RAM);
	ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80RAM);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(galspnbl_sound_write);
	ZetSetReadHandler(galspnbl_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM3812Init(3579545, &DrvYM3812IrqHandler, &DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);

	MSM6295Init(0, 1056000 / 132, 50.0, 1);

	game_select = select;

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	MSM6295Exit(0);
	BurnYM3812Exit();

	SekExit();
	ZetExit();

	GenericTilesExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_pixel_layer()
{
	UINT16 *ram = ((UINT16*)DrvVidRAM1) + 16 * 512;
	UINT16 *dst = pTransDraw;

	for (INT32 y = 0; y < 224; y++)
	{
		for (INT32 x = 0; x < nScreenWidth; x++)
		{
			dst[x] = ram[x] >> 1;
		}

		dst += nScreenWidth;
		ram += 512;
	}
}

static void draw_fg_layer()
{
	UINT16 *vid = (UINT16*)DrvVidRAM0;
	UINT16 *col = (UINT16*)DrvColRAM;

	for (INT32 offs = 0; offs < 0x1000 / 2; offs++)
	{
		INT32 sx    = (offs & 0x3f) * 16;
		INT32 sy    = (offs / 0x40) * 8 - 16;

		if (sx >= nScreenWidth || sy < 0) continue;
		if (sy >= nScreenHeight) break;

		INT32 code  = BURN_ENDIAN_SWAP_INT16(vid[offs]) & 0x1fff;
		INT32 attr  = BURN_ENDIAN_SWAP_INT16(col[offs]);
		INT32 color = (attr & 0x00f0) >> 4;

		if (attr & 0x0008) continue;

		Render8x8Tile_Mask_Clip(pTransDraw, code*2+0, sx + 0, sy, color, 4, 0, 0x8200, DrvGfxROM0);
		Render8x8Tile_Mask_Clip(pTransDraw, code*2+1, sx + 8, sy, color, 4, 0, 0x8200, DrvGfxROM0);
	}
}

static void draw_sprites(int priority)
{
	UINT16 *spriteram = (UINT16*)DrvSprRAM;

	for (INT32 offs = (0x1000 - 16) / 2; offs >= 0; offs -= 8)
	{
		INT32 attr = BURN_ENDIAN_SWAP_INT16(spriteram[offs]);

		if ((attr & 0x0004) && ((attr & 0x0040) == 0 || (nCurrentFrame & 1)) && (attr & 0x0020) == priority)
		{
			INT32 code  = BURN_ENDIAN_SWAP_INT16(spriteram[offs + 1]) & 0x3fff;
			INT32 color = BURN_ENDIAN_SWAP_INT16(spriteram[offs + 2]);
			INT32 sy    = BURN_ENDIAN_SWAP_INT16(spriteram[offs + 3]) - 16;
			INT32 sx    = BURN_ENDIAN_SWAP_INT16(spriteram[offs + 4]);
			INT32 size  = 1 << (color & 0x0003);
			      color = (color & 0x00f0) >> 4;
			INT32 flipx = attr & 0x0001;
			INT32 flipy = attr & 0x0002;

			for (INT32 row = 0; row < size; row++)
			{
				for (INT32 col = 0; col < size; col++)
				{
					INT32 x = sx + 8 * (flipx ? (size - 1 - col) : col);
					INT32 y = sy + 8 * (flipy ? (size - 1 - row) : row);

					INT32 tile = code + ((row & 1) << 1) + ((row & 2) << 2) + ((row & 4) << 3) + (col & 1) + ((col & 2) << 1) + ((col & 4) << 2);

					if (flipy) {
						if (flipx) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, tile, x, y, color, 4, 0, 0x8000, DrvGfxROM1);
						} else {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, tile, x, y, color, 4, 0, 0x8000, DrvGfxROM1);
						}
					} else {
						if (flipx) {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, tile, x, y, color, 4, 0, 0x8000, DrvGfxROM1);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, tile, x, y, color, 4, 0, 0x8000, DrvGfxROM1);
						}
					}
				}
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvStaticPaletteInit();

		for (INT32 i = 0; i < 0x800; i+=2) {
			palette_update(i);
		}

		DrvRecalc = 0;
	}

	if(~nBurnLayer & 1) BurnTransferClear();
	if (nBurnLayer & 1) draw_pixel_layer();
	if (nBurnLayer & 2) draw_sprites(0x00);
	if (nBurnLayer & 4) draw_fg_layer();
	if (nBurnLayer & 8) draw_sprites(0x20);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 3 * sizeof(UINT16));

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= DrvJoy1[i] << i;
			DrvInputs[1] ^= DrvJoy2[i] << i;
			DrvInputs[2] ^= DrvJoy3[i] << i;
		}

		if (game_select) { // Right flipper button is swapped
			DrvInputs[1] = ((DrvInputs[1] & 0x20) >> 1) | (DrvInputs[1] & 0xcf) | 0x20;
		}
	}

	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { 10000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nSegment = nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += SekRun(nSegment);
		if (i == (nInterleave - 1)) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);

		nSegment = nCyclesTotal[1] / nInterleave;
		BurnTimerUpdateYM3812((1 + i) * nSegment);
	}

	BurnTimerEndFrameYM3812(nCyclesTotal[1]);

	if (pBurnSoundOut) {
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


// Gals Pinball

static struct BurnRomInfo galspnblRomDesc[] = {
	{ "7.rom",	0x80000, 0xce0189bf, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "3.rom",	0x80000, 0x9b0a8744, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "8.rom",	0x80000, 0xeee2f087, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "4.rom",	0x80000, 0x56298489, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "9.rom",	0x80000, 0xd9e4964c, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "5.rom",	0x80000, 0xa5e71ee4, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "10.rom",	0x80000, 0x3a20e1e5, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "6.rom",	0x80000, 0x94927d20, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "2.rom",	0x10000, 0xfae688a7, 2 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "17.rom",	0x40000, 0x7d435701, 3 | BRF_GRA },           //  9 Foreground Tiles
	{ "18.rom",	0x40000, 0x136adaac, 3 | BRF_GRA },           // 10

	{ "15.rom",	0x20000, 0x4beb840d, 4 | BRF_GRA },           // 11 Sprite Tiles
	{ "16.rom",	0x20000, 0x93d3c610, 4 | BRF_GRA },           // 12

	{ "1.rom",	0x40000, 0x93c06d3d, 5 | BRF_SND },           // 13 MSM6295 Samples
};

STD_ROM_PICK(galspnbl)
STD_ROM_FN(galspnbl)

static INT32 galspnblInit()
{
	return DrvInit(0);
}

struct BurnDriver BurnDrvGalspnbl = {
	"galspnbl", NULL, NULL, NULL, "1996",
	"Gals Pinball\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 1, HARDWARE_MISC_POST90S, GBF_PINBALL, 0,
	NULL, galspnblRomInfo, galspnblRomName, NULL, NULL, GalspnblInputInfo, GalspnblDIPInfo,
	galspnblInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x8400,
	224, 512, 3, 4
};


// Hot Pinball

static struct BurnRomInfo hotpinblRomDesc[] = {
	{ "hp_07.bin",	0x80000, 0x978cc13e, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "hp_03.bin",	0x80000, 0x68388726, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "hp_08.bin",	0x80000, 0xbd16be12, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "hp_04.bin",	0x80000, 0x655b0cf0, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "hp_09.bin",	0x80000, 0xa6368624, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "hp_05.bin",	0x80000, 0x48efd028, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "hp_10.bin",	0x80000, 0xa5c63e34, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "hp_06.bin",	0x80000, 0x513eda91, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "hp_02.bin",	0x10000, 0x82698269, 2 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "hp_13.bin",	0x40000, 0xd53b64b9, 3 | BRF_GRA },           //  9 Foreground Tiles
	{ "hp_14.bin",	0x40000, 0x2fe3fcee, 3 | BRF_GRA },           // 10

	{ "hp_11.bin",	0x20000, 0xdeecd7f1, 4 | BRF_GRA },           // 11 Sprite Tiles
	{ "hp_12.bin",	0x20000, 0x5fd603c2, 4 | BRF_GRA },           // 12

	{ "hp_01.bin",	0x40000, 0x93c06d3d, 5 | BRF_SND },           // 13 MSM6295 Samples
};

STD_ROM_PICK(hotpinbl)
STD_ROM_FN(hotpinbl)

static INT32 hotpinblInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvHotpinbl = {
	"hotpinbl", NULL, NULL, NULL, "1995",
	"Hot Pinball\0", NULL, "Comad & New Japan System", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 1, HARDWARE_MISC_POST90S, GBF_PINBALL, 0,
	NULL, hotpinblRomInfo, hotpinblRomName, NULL, NULL, GalspnblInputInfo, GalspnblDIPInfo,
	hotpinblInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x8400,
	224, 512, 3, 4
};
