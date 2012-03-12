// FB Alpha Ashita No Joe / Success Joe driver module
// Based on MAME driver by driver by David Haywood and Pierpaolo Prazzoli

#include "tiles_generic.h"
#include "burn_ym2203.h"
#include "msm5205.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvGfxROM4;
static UINT8 *DrvZ80Banks;
static UINT8 *DrvPfRAM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvPalRAM;

static UINT32 *DrvPalette;

static UINT8 *soundlatch;
static UINT8 *soundstatus;
static UINT8 *tilemap_reg;

static UINT16 *scrollx;
static UINT16 *scrolly;

static INT32 flipscreen;

static UINT8 adpcm_byte = 0;
static UINT8 MSM5205_vclk_toggle = 0;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static struct BurnInputInfo AshnojoeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 11,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 13,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 8,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Ashnojoe)

static struct BurnDIPInfo AshnojoeDIPList[]=
{
	{0x12, 0xff, 0xff, 0x00, NULL			},
	{0x13, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x01, 0x00, "Upright"		},
	{0x12, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x02, 0x00, "Off"			},
	{0x12, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x04, 0x00, "Off"			},
	{0x12, 0x01, 0x04, 0x04, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x08, 0x08, "Off"			},
	{0x12, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x12, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x30, 0x00, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x30, 0x30, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x30, 0x10, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x03, 0x01, "Easy"			},
	{0x13, 0x01, 0x03, 0x00, "Normal"		},
	{0x13, 0x01, 0x03, 0x02, "Medium"		},
	{0x13, 0x01, 0x03, 0x03, "Hard"			},

	{0   , 0xfe, 0   ,    2, "Number of controller"	},
	{0x13, 0x01, 0x04, 0x00, "2"			},
	{0x13, 0x01, 0x04, 0x04, "1"			},
};

STDDIPINFO(Ashnojoe)

void __fastcall ashnojoe_write_byte(UINT32 a, UINT8 d)
{
	bprintf (0, _T("%5.5x, %2.2x wb\n"), a, d);
}

void __fastcall ashnojoe_write_word(UINT32 a, UINT16 d)
{
	switch (a)
	{
		case 0x4a006:
			*tilemap_reg = d & 0xff;
		return;

		case 0x4a008:
			*soundlatch = d & 0xff;
			*soundstatus = 1;
		return;

		case 0x4a010:
		case 0x4a012:
		case 0x4a014:
		case 0x4a016:
		case 0x4a018:
			scrollx[(a & 0x0f) / 2] = d;
		return;

		case 0x4a020:
		case 0x4a022:
		case 0x4a024:
		case 0x4a026:
		case 0x4a028:
			scrolly[(a & 0x0f) / 2] = d;
		return;
	}
}

UINT8 __fastcall ashnojoe_read_byte(UINT32 a)
{
	bprintf (0, _T("%5.5x, rb\n"), a);

	return 0;
}

UINT16 __fastcall ashnojoe_read_word(UINT32 a)
{
	switch (a)
	{
		case 0x4a000:
			return DrvInputs[0];

		case 0x4a002:
			return DrvInputs[1];

		case 0x4a004:
			return ((DrvDips[1] << 8) | (DrvDips[0]));

		case 0x4a00a:
			return 0; // debug? 1 kills sound
	}

	return 0;
}

void __fastcall ashnojoe_sound_write_port(UINT16 p, UINT8 d)
{
	switch (p & 0xff)
	{
		case 0x00:
			BurnYM2203Write(0, 0, d);
		return;

		case 0x01:
			BurnYM2203Write(0, 1, d);
		return;

		case 0x02:
			adpcm_byte = d;
		return;
	}
}

UINT8 __fastcall ashnojoe_sound_read_port(UINT16 p)
{
	switch (p & 0xff)
	{
		case 0x00:
			return BurnYM2203Read(0, 0);

		case 0x01:
			return BurnYM2203Read(0, 1);

		case 0x04:
			*soundstatus = 0;
			return *soundlatch;

		case 0x06:
			return *soundstatus;
	}

	return 0;
}

void DrvYM2203WritePortA(UINT32, UINT32 d)
{
	MSM5205ResetWrite(0, !(d & 0x01));
}

void DrvYM2203WritePortB(UINT32, UINT32 d)
{
	INT32 bank = (d & 0x0f) * 0x8000;

	ZetMapArea(0x8000, 0xffff, 0, DrvZ80Banks + bank);
	ZetMapArea(0x8000, 0xffff, 2, DrvZ80Banks + bank);
}

inline static void DrvIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 4000000.0;
}

static void ashnojoe_vclk_cb()
{
	if (MSM5205_vclk_toggle == 0)
	{
		MSM5205DataWrite(0, adpcm_byte >> 4);
	}
	else
	{
		MSM5205DataWrite(0, adpcm_byte & 0xf);
		ZetNmi();
	}

	MSM5205_vclk_toggle ^= 1;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();

	BurnYM2203Reset();
	MSM5205Reset();

	ZetClose();

	adpcm_byte = 0;
	MSM5205_vclk_toggle = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x100000;
	DrvZ80ROM		= Next; Next += 0x010000;
	DrvZ80Banks		= Next; Next += 0x080000;

	DrvGfxROM0		= Next; Next += 0x040000;
	DrvGfxROM1		= Next; Next += 0x040000;
	DrvGfxROM2		= Next; Next += 0x040000;
	DrvGfxROM3		= Next; Next += 0x200000;
	DrvGfxROM4		= Next; Next += 0x600000;

	DrvPalette		= (UINT32*)Next; Next += 0x0800 * sizeof(UINT32);

	AllRam			= Next;

	DrvPfRAM		= Next; Next += 0x009000;
	Drv68KRAM		= Next; Next += 0x004000;
	DrvPalRAM		= Next; Next += 0x001000;

	DrvZ80RAM		= Next; Next += 0x002000;

	soundlatch		= Next; Next += 0x000001;
	soundstatus		= Next; Next += 0x000001;

	tilemap_reg		= Next; Next += 0x000001;

	scrollx			= (UINT16*)Next; Next += 0x000008 * sizeof(UINT16);
	scrolly			= (UINT16*)Next; Next += 0x000008 * sizeof(UINT16);

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static void DrvGfxExpand(UINT8 *gfx, INT32 len, INT32 bs) //0,1,2 -8x8, 3,4 16x16
{
	if (bs) BurnByteswap(gfx, len);

	for (INT32 i = len-1; i >= 0; i--) {
		gfx[i * 2 + 1] = gfx[i] & 0x0f;
		gfx[i * 2 + 0] = gfx[i] >> 4;
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
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080000,  2, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x010000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x010000,  7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x010000,  9, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x080000, 11, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM4 + 0x000000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x080000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x100000, 14, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x180000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x200000, 16, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x280000, 17, 1)) return 1;

		if (BurnLoadRom(DrvZ80Banks + 0x000000,  18, 1)) return 1;

		DrvGfxExpand(DrvGfxROM0, 0x020000, 0);
		DrvGfxExpand(DrvGfxROM1, 0x020000, 0);
		DrvGfxExpand(DrvGfxROM2, 0x020000, 0);
		DrvGfxExpand(DrvGfxROM3, 0x100000, 1);
		DrvGfxExpand(DrvGfxROM4, 0x300000, 1);
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(DrvPfRAM,			0x040000, 0x048fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x049000, 0x049fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0x04c000, 0x04ffff, SM_RAM);
	SekMapMemory(Drv68KROM + 0x080000,	0x080000, 0x0bffff, SM_ROM);
	SekSetWriteWordHandler(0,		ashnojoe_write_word);
	SekSetWriteByteHandler(0,		ashnojoe_write_byte);
	SekSetReadWordHandler(0,		ashnojoe_read_word);
	SekSetReadByteHandler(0,		ashnojoe_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM);
	ZetMapArea(0x6000, 0x7fff, 0, DrvZ80RAM);
	ZetMapArea(0x6000, 0x7fff, 1, DrvZ80RAM);
	ZetMapArea(0x6000, 0x7fff, 2, DrvZ80RAM);
	ZetSetOutHandler(ashnojoe_sound_write_port);
	ZetSetInHandler(ashnojoe_sound_read_port);
	ZetMemEnd();

	MSM5205Init(0, DrvSynchroniseStream, 384000, ashnojoe_vclk_cb, MSM5205_S48_4B, 100, 1);

	BurnYM2203Init(1, 4000000, &DrvIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetPorts(0, NULL, NULL, &DrvYM2203WritePortA, &DrvYM2203WritePortB);
	BurnYM2203SetVolumeShift(4);
	BurnTimerAttachZet(4000000);

	ZetClose();

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();
	ZetExit();

	BurnYM2203Exit();
	MSM5205Exit();

	BurnFree(AllMem);

	return 0;
}

static inline void DrvRecalcPalette()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;

	for (INT32 i = 0; i < 0x1000/2; i++) {
		INT32 d = p[i];

		r = (d >> 10) & 0x1f;
		g = (d >>  5) & 0x1f;
		b = (d >>  0) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void draw_16x16_layer(UINT8 *ram, UINT8 *gfx, INT32 color_off, INT32 scroll_off, INT32 mask, INT32 transparent)
{
	INT32 xscroll = (scrollx[scroll_off] + 112) & 0x1ff;
	INT32 yscroll = (scrolly[scroll_off] +  24) & 0x1ff;

	UINT16 *vram = (UINT16*)ram;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx -= xscroll;
		if (sx < -15) sx += 512;
		sy -= yscroll;
		if (sy < -15) sy += 512;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 code  =   vram[offs * 2 + 0] & mask;
		INT32 color = ((vram[offs * 2 + 1] >> 8) & 0x1f) + color_off; 

		if (flipscreen) {
			if (transparent) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, 272 - sx, 192 - sy, color, 4, 15, 0, gfx);
			} else {
				Render16x16Tile_FlipXY_Clip(pTransDraw, code, 272 - sx, 192 - sy, color, 4, 0, gfx);
			}
		} else {
			if (transparent) {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, gfx);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, gfx);
			}
		}
	}
}

static void draw_8x8_layer(UINT8 *ram, UINT8 *gfx, INT32 color_off, INT32 scroll_off, INT32 high)
{
	INT32 xscroll = (scrollx[scroll_off] + 112) & 0x1ff;
	INT32 yscroll = (scrolly[scroll_off] +  24) & (high ? 0x1ff : 0x0ff);

	UINT16 *vram = (UINT16*)ram;

	for (INT32 offs = 0; offs < 64 * (32 << high); offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		sx -= xscroll;
		if (sx < -7) sx += 512;
		sy -= yscroll;
		if (sy < -7) sy += 256 << high;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 code  =  vram[offs] & 0x0fff;
		INT32 color = (vram[offs] >> 12) + color_off; 

		if (flipscreen) {
			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, 280 - sx, 200 - sy, color, 4, 15, 0, gfx);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, gfx);
		}
	}
}

static INT32 DrvDraw()
{
	DrvRecalcPalette();

	flipscreen = *tilemap_reg & 0x01;

	INT32 backlayer = (*tilemap_reg & 0x02) ? 0x7000 : 0x6000;

	if (nSpriteEnable & 0x01) draw_16x16_layer(DrvPfRAM + backlayer, DrvGfxROM3, 0x70, 4, 0x1fff, 0);

	if (nSpriteEnable & 0x02) draw_8x8_layer(DrvPfRAM + 0x2000, DrvGfxROM1, 0x60, 3, 1);

	if (nSpriteEnable & 0x04) draw_16x16_layer(DrvPfRAM + 0x5000, DrvGfxROM4, 0x40, 2, 0x7fff, 1); 

	if (nSpriteEnable & 0x08) draw_16x16_layer(DrvPfRAM + 0x4000, DrvGfxROM4, 0x20, 1, 0x7fff, 1);

	if (nSpriteEnable & 0x10) draw_8x8_layer(DrvPfRAM + 0x0000, DrvGfxROM0, 0x10, 0, 1);

	if (nSpriteEnable & 0x20) draw_8x8_layer(DrvPfRAM + 0x8000, DrvGfxROM2, 0x00, 5, 1);

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
		memset (DrvInputs, 0, 2 * sizeof(INT16));

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

	}

	INT32 nInterleave = MSM5205CalcInterleave(0, 4000000);
	INT32 nCyclesTotal[2] = { 8000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };
	INT32 nNext[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		nNext[0] += nCyclesTotal[0] / nInterleave;

		nCyclesDone[0] += SekRun(nNext[0] - nCyclesDone[0]);
		if (i == (nInterleave - 1)) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

		nNext[1] += nCyclesTotal[1] / nInterleave;
		BurnTimerUpdate(nNext[1]);
		MSM5205Update();
		nCyclesDone[1] += nNext[1];
	}

	BurnTimerEndFrame(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
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

		BurnYM2203Scan(nAction, pnMin);
		MSM5205Scan(nAction, pnMin);
	}

	return 0;
}


// Success Joe (World)

static struct BurnRomInfo scessjoeRomDesc[] = {
	{ "5.4q",		0x10000, 0xc805f9e7, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "6.4s",		0x10000, 0xeda7a537, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sj201-nw.6m",	0x40000, 0x5a64ca42, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "9.8q",		0x08000, 0x8767e212, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 Code

	{ "8.5e",		0x10000, 0x9bcb160e, 3 | BRF_GRA },           //  4 Character Tiles
	{ "7.5c",		0x10000, 0xb250c69d, 3 | BRF_GRA },           //  5

	{ "4.4e",		0x10000, 0xaa6336d3, 4 | BRF_GRA },           //  6 Character Tiles
	{ "3.4c",		0x10000, 0x7e2d86b5, 4 | BRF_GRA },           //  7

	{ "2.3m",		0x10000, 0xc3254938, 5 | BRF_GRA },           //  8 Character Tiles
	{ "1.1m",		0x10000, 0x5d16a6fa, 5 | BRF_GRA },           //  9

	{ "sj402-nw.8e",	0x80000, 0xb6d33d06, 6 | BRF_GRA },           // 10 Background Tiles
	{ "sj403-nw.7e",	0x80000, 0x07143f56, 6 | BRF_GRA },           // 11

	{ "sj404-nw.7a",	0x80000, 0x8f134128, 7 | BRF_GRA },           // 12 Foreground Tiles
	{ "sj405-nw.7c",	0x80000, 0x6fd81699, 7 | BRF_GRA },           // 13
	{ "sj406-nw.7d",	0x80000, 0x634e33e6, 7 | BRF_GRA },           // 14
	{ "sj407-nw.7f",	0x80000, 0x5c66ff06, 7 | BRF_GRA },           // 15
	{ "sj408-nw.7g",	0x80000, 0x6a3b1ea1, 7 | BRF_GRA },           // 16
	{ "sj409-nw.7j",	0x80000, 0xd8764213, 7 | BRF_GRA },           // 17

	{ "sj401-nw.10r",	0x80000, 0x25dfab59, 8 | BRF_PRG | BRF_ESS }, // 18 Z80 Banks
};

STD_ROM_PICK(scessjoe)
STD_ROM_FN(scessjoe)

struct BurnDriver BurnDrvScessjoe = {
	"scessjoe", NULL, NULL, NULL, "1990",
	"Success Joe (World)\0", "Incomplete sound", "WAVE / Taito Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_MISC, 0,
	NULL, scessjoeRomInfo, scessjoeRomName, NULL, NULL, AshnojoeInputInfo, AshnojoeDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x800,
	288, 208, 4, 3
};


// Ashita no Joe (Japan)

static struct BurnRomInfo ashnojoeRomDesc[] = {
	{ "5.bin",		0x10000, 0xc61e1569, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "6.bin",		0x10000, 0xc0a16338, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sj201-nw.6m",	0x40000, 0x5a64ca42, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "9.8q",		0x08000, 0x8767e212, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 Code

	{ "8.5e",		0x10000, 0x9bcb160e, 3 | BRF_GRA },           //  4 Character Tiles
	{ "7.bin",		0x10000, 0x7e1efc42, 3 | BRF_GRA },           //  5

	{ "4.4e",		0x10000, 0xaa6336d3, 4 | BRF_GRA },           //  6 Character Tiles
	{ "3.4c",		0x10000, 0x7e2d86b5, 4 | BRF_GRA },           //  7

	{ "2.3m",		0x10000, 0xc3254938, 5 | BRF_GRA },           //  8 Character Tiles
	{ "1.bin",		0x10000, 0x1bf585f0, 5 | BRF_GRA },           //  9

	{ "sj402-nw.8e",	0x80000, 0xb6d33d06, 6 | BRF_GRA },           // 10 Background Tiles
	{ "sj403-nw.7e",	0x80000, 0x07143f56, 6 | BRF_GRA },           // 11

	{ "sj404-nw.7a",	0x80000, 0x8f134128, 7 | BRF_GRA },           // 12 Foreground Tiles
	{ "sj405-nw.7c",	0x80000, 0x6fd81699, 7 | BRF_GRA },           // 13
	{ "sj406-nw.7d",	0x80000, 0x634e33e6, 7 | BRF_GRA },           // 14
	{ "sj407-nw.7f",	0x80000, 0x5c66ff06, 7 | BRF_GRA },           // 15
	{ "sj408-nw.7g",	0x80000, 0x6a3b1ea1, 7 | BRF_GRA },           // 16
	{ "sj409-nw.7j",	0x80000, 0xd8764213, 7 | BRF_GRA },           // 17

	{ "sj401-nw.10r",	0x80000, 0x25dfab59, 8 | BRF_PRG | BRF_ESS }, // 18 Z80 Banks
};

STD_ROM_PICK(ashnojoe)
STD_ROM_FN(ashnojoe)

struct BurnDriver BurnDrvAshnojoe = {
	"ashnojoe", "scessjoe", NULL, NULL, "1990",
	"Ashita no Joe (Japan)\0", "Incomplete sound", "WAVE / Taito Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_MISC, 0,
	NULL, ashnojoeRomInfo, ashnojoeRomName, NULL, NULL, AshnojoeInputInfo, AshnojoeDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x800,
	288, 208, 4, 3
};
