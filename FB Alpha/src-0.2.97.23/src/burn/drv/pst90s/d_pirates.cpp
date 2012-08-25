// FB Alpha Pirates driver
// Based on MAME driver by David Haywood and Nicola Salmoria

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "bitswap.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvMiscRAM;
static UINT32 *DrvPalette;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips;
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static UINT8  *DrvOkiBank;
static UINT16 *DrvScrollX;

static INT32 is_genix = 0;

static struct BurnInputInfo PiratesInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	&DrvDips,	"dip"		},
};

STDINPUTINFO(Pirates)

static struct BurnDIPInfo PiratesDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0x0c, NULL		},

	{0   , 0xfe, 0   ,    2, "Service Mode"	},
	{0x13, 0x01, 0x0c, 0x0c, "Off"		},
	{0x13, 0x01, 0x0c, 0x00, "On"		},
};

STDDIPINFO(Pirates)

inline static void genix_hack()
{
	if (is_genix) {
		Drv68KRAM[0x9e98] = 0x04;
		Drv68KRAM[0x9e99] = 0x00;
		Drv68KRAM[0x9e9a] = 0x00;
		Drv68KRAM[0x9e9b] = 0x00;
	}
}

static void palette_write(INT32 offset)
{
	UINT16 data = *((UINT16*)(DrvPalRAM + offset));

	UINT8 r,g,b;

	r = (data >> 10) & 0x1f;
	g = (data >>  5) & 0x1f;
	b = (data >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[offset >> 1] = BurnHighCol(r, g, b, 0);
}

static void set_oki_bank(INT32 data)
{
	*DrvOkiBank = data;
	memcpy (MSM6295ROM, DrvSndROM + (data << 12), 0x40000);

bprintf (PRINT_NORMAL, _T("%2.2x\n"), data);
}

void __fastcall pirates_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfffc00) == 0x109c00) {
		Drv68KRAM[(address & 0xffff) ^ 1] = data;
		genix_hack();

		return;
	}

	if ((address & 0xffc000) == 0x800000) {
		DrvPalRAM[address & 0x3fff] = data;
		palette_write(address & 0x3ffe);
		return;
	}

//bprintf (PRINT_NORMAL, _T("%5.5x, %2.2x\n"), address, data);

	switch (address)
	{
		case 0x600000:
		case 0x600001:
			if (*DrvOkiBank != (data & 0x40))
				set_oki_bank(data & 0x40);
		return;

		case 0xa00000:
		case 0xa00001:
			MSM6295Command(0, data);
		return;
	}
}

void __fastcall pirates_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffc00) == 0x109c00) {
		*((UINT16 *)(Drv68KRAM + (address & 0xfffe))) = data;
		genix_hack();

		return;
	}

	if ((address & 0xffc000) == 0x800000) {
		*((UINT16 *)(DrvPalRAM + (address & 0x3ffe))) = data;
		palette_write(address & 0x3ffe);
		return;
	}

//bprintf (PRINT_NORMAL, _T("%5.5x, %4.4x\n"), address, data);

	switch (address)
	{
		case 0x600000:
			if (*DrvOkiBank != (data & 0x40))
				set_oki_bank(data & 0x40);
		return;

		case 0x700000:
			*DrvScrollX = data & 0x1ff;
		return;

		case 0xa00000:
			MSM6295Command(0, data & 0xff);
		return;
	}
}

UINT8 __fastcall pirates_read_byte(UINT32 address)
{
	genix_hack();
//bprintf (PRINT_NORMAL, _T("%5.5x, b\n"), address);


	switch (address)
	{
		case 0x300000:
		case 0x300001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0x400000:
		case 0x400001:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0xa00001:
			return MSM6295ReadStatus(0);
	}

	return 0;
}

UINT16 __fastcall pirates_read_word(UINT32 address)
{
	genix_hack();
//bprintf (PRINT_NORMAL, _T("%5.5x, w\n"), address);

	switch (address)
	{
		case 0x300000:
			return DrvInputs[0];

		case 0x400000:
			return DrvInputs[1];
	}

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4]   = { 0x400000 * 3,  0x400000 * 2,  0x400000 * 1,  0x400000 * 0 };
	INT32 XOffs[16]  = { 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8 };
	INT32 YOffs0[8]  = { 8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7 };
	INT32 YOffs1[16] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	 		   8*16, 9*16,10*16,11*16,12*16,13*16,14*16,15*16 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x200000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x200000);

	GfxDecode(0x10000, 4,  8,  8, Plane, XOffs, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x200000);

	GfxDecode(0x04000, 4, 16, 16, Plane, XOffs, YOffs1, 0x100, tmp, DrvGfxROM1);

	BurnFree(tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x100000;

	DrvGfxROM0	= Next; Next += 0x400000;
	DrvGfxROM1	= Next; Next += 0x400000;

	MSM6295ROM	= Next; Next += 0x040000;
	DrvSndROM	= Next; Next += 0x080000;

	DrvPalette	= (UINT32*)Next; Next += 0x2000 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x004000;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvMiscRAM	= Next; Next += 0x005000;

	DrvScrollX	= (UINT16*)Next; Next += 0x000002;

	DrvOkiBank	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	MSM6295Reset(0);

	set_oki_bank(0);

	return 0;
}

static void pirates_decrypt_68k()
{
	UINT16 *buf = (UINT16 *)BurnMalloc(0x100000);
 	UINT16 *rom = (UINT16 *)Drv68KROM;

	memcpy (buf, rom, 0x100000);

	for (INT32 i=0; i< 0x100000/2; i++)
 	{
		INT32 adrl, adrr;
		UINT8 vl, vr;

 		adrl = BITSWAP24(i,23,22,21,20,19,18,4,8,3,14,2,15,17,0,9,13,10,5,16,7,12,6,1,11);
		vl = BITSWAP08(buf[adrl],    4,2,7,1,6,5,0,3);

		adrr = BITSWAP24(i,23,22,21,20,19,18,4,10,1,11,12,5,9,17,14,0,13,6,15,8,3,16,7,2);
		vr = BITSWAP08(buf[adrr]>>8, 1,4,7,0,3,5,6,2);

		rom[i] = (vr<<8) | vl;
	}

    BurnFree (buf);
}

static void pirates_decrypt_p()
{
	UINT8 *buf = (UINT8*)BurnMalloc(0x200000);
	UINT8 *rom = DrvGfxROM0;
	memcpy (buf, rom, 0x200000);

	for (INT32 i=0; i<0x200000/4; i++)
	{
		INT32 adr = BITSWAP24(i,23,22,21,20,19,18,10,2,5,9,7,13,16,14,11,4,1,6,12,17,3,0,15,8);
		rom[adr+0*(0x200000/4)] = BITSWAP08(buf[i+0*(0x200000/4)], 2,3,4,0,7,5,1,6);
		rom[adr+1*(0x200000/4)] = BITSWAP08(buf[i+1*(0x200000/4)], 4,2,7,1,6,5,0,3);
		rom[adr+2*(0x200000/4)] = BITSWAP08(buf[i+2*(0x200000/4)], 1,4,7,0,3,5,6,2);
		rom[adr+3*(0x200000/4)] = BITSWAP08(buf[i+3*(0x200000/4)], 2,3,4,0,7,5,1,6);
	}

	BurnFree (buf);
}

static void pirates_decrypt_s()
{
	UINT8 *buf = (UINT8*)BurnMalloc(0x200000);
	UINT8 *rom = DrvGfxROM1;
	memcpy (buf, rom, 0x200000);

	for (INT32 i=0; i<0x200000/4; i++)
	{
		INT32 adr = BITSWAP24(i,23,22,21,20,19,18,17,5,12,14,8,3,0,7,9,16,4,2,6,11,13,1,10,15);
		rom[adr+0*(0x200000/4)] = BITSWAP08(buf[i+0*(0x200000/4)], 4,2,7,1,6,5,0,3);
		rom[adr+1*(0x200000/4)] = BITSWAP08(buf[i+1*(0x200000/4)], 1,4,7,0,3,5,6,2);
		rom[adr+2*(0x200000/4)] = BITSWAP08(buf[i+2*(0x200000/4)], 2,3,4,0,7,5,1,6);
		rom[adr+3*(0x200000/4)] = BITSWAP08(buf[i+3*(0x200000/4)], 4,2,7,1,6,5,0,3);
	}

	BurnFree (buf);
}

static void pirates_decrypt_oki()
{
	UINT8 *buf = (UINT8*)BurnMalloc(0x80000);
	UINT8 *rom = DrvSndROM;
	memcpy (buf, rom, 0x80000);

	for (INT32 i=0; i<0x80000; i++)
	{
		INT32 adr = BITSWAP24(i,23,22,21,20,19,10,16,13,8,4,7,11,14,17,12,6,2,0,5,18,15,3,1,9);
		rom[adr] = BITSWAP08(buf[i], 2,3,4,0,7,5,1,6);
	}

	BurnFree (buf);
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
		if (BurnLoadRom(Drv68KROM + 1,	0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0,	1, 2)) return 1;

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvGfxROM0 + i * 0x80000, 2 + i, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + i * 0x80000, 6 + i, 1)) return 1;
		}

		if (BurnLoadRom(DrvSndROM,	10, 1)) return 1;

		pirates_decrypt_68k();
		pirates_decrypt_p();
		pirates_decrypt_s();
		pirates_decrypt_oki();

		DrvGfxDecode();
	}

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "pirates")) {
		*((UINT16*)(Drv68KROM + 0x62c0)) = 0x6006; // bypass protection
	} else {
		is_genix = 1;
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		 0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM + 0x0000, 0x100000, 0x109bff, SM_RAM);
	SekMapMemory(Drv68KRAM + 0x9c00, 0x109c00, 0x109fff, (is_genix) ? SM_ROM : SM_RAM);
	SekMapMemory(Drv68KRAM + 0xa000, 0x10a000, 0x10ffff, SM_RAM);
//	SekMapMemory(Drv68KRAM,		 0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(DrvSprRAM,		 0x500000, 0x5007ff, SM_RAM);
	SekMapMemory(DrvPalRAM,		 0x800000, 0x803fff, SM_ROM);
	SekMapMemory(DrvMiscRAM,	 0x900000, 0x904fff, SM_RAM);
	SekSetWriteByteHandler(0,	 pirates_write_byte);
	SekSetWriteWordHandler(0,	 pirates_write_word);
	SekSetReadByteHandler(0,	 pirates_read_byte);
	SekSetReadWordHandler(0,	 pirates_read_word);
	SekClose();

	MSM6295Init(0, 1333333 / (132 + 39), 0); // otherwise too fast!
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();

	MSM6295Exit(0);

	BurnFree (AllMem);

	MSM6295ROM = NULL;
	is_genix = 0;

	return 0;
}

static void draw_layer(INT32 offset, INT32 coloffs, INT32 transp)
{
	UINT16 *vram = (UINT16*)(DrvMiscRAM + offset);

	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 sx = (offs >> 5) << 3;
		INT32 sy = (offs & 0x1f) << 3;

		sx -= *DrvScrollX;
		if (sx < -15) sx += 0x200;

		if (sy < 16 || sy > 239 || sx > 287 || sx < -15) continue;

		INT32 code = vram[offs*2];
		INT32 color = vram[offs*2+1];

		if (transp) {
			if (!code && !color) continue;

			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy - 16, color + coloffs, 4, 0, 0, DrvGfxROM0);	
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy - 16, color + coloffs, 4, 0, DrvGfxROM0);
		}
	}
}

static void draw_text_layer()
{
	UINT16 *vram = (UINT16*)(DrvMiscRAM + 0x0180);

	for (INT32 offs = 0; offs < 36 * 32; offs++)
	{
		INT32 sx = (offs >> 5) << 3;
		INT32 sy = (offs & 0x1f) << 3;

		if (sy < 16 || sy > 239 || sx >= 288) continue;

		INT32 code = vram[offs*2];
		INT32 color = vram[offs*2+1];

		if (!code && !color) continue;

		Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0, 0, DrvGfxROM0);	
	}
}

static void draw_sprites()
{
	UINT16 *source = ((UINT16*)DrvSprRAM) + 4;
	UINT16 *finish = source + 0x800/2-4;

	while (source < finish)
	{
		INT32 sx, sy, flipx, flipy, code, color;

		sx = source[1] - 32;
		sy = source[-1];

		if (sy & 0x8000) break;

		code  = source[2] >> 2;
		color = source[0] & 0xff;
		flipx = source[2] & 2;
		flipy = source[2] & 1;

		sy = (0xf2 - sy) - 16;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x1800, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x1800, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x1800, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x1800, DrvGfxROM1);
			}
		}

		source+=4;
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x4000; i+=2) {
			palette_write(i);
		}
	}

	if (nBurnLayer & 1)
		draw_layer(0x2a80, 0x100, 0);
	else
		memset (pTransDraw, 0, nScreenWidth * nScreenHeight * 2);

	if (nBurnLayer & 2) draw_layer(0x1380, 0x080, 1);

	if (nBurnLayer & 8) draw_sprites();

	if (nBurnLayer & 4) draw_text_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = 0xffff;
		DrvInputs[1] = 0x0003 | DrvDips;
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 10;

	SekOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		SekRun((16000000 / 60) / nInterleave);

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			MSM6295Render(0, pSoundBuf, nSegmentLength);

			nSoundBufferPos += nSegmentLength;
		}
	}

	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		MSM6295Render(0, pSoundBuf, nSegmentLength);
	}

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

	if (nAction & ACB_MEMORY_RAM) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);

		MSM6295Scan(0, nAction);
	}

	set_oki_bank(*DrvOkiBank);

	return 0;
}


// Pirates

static struct BurnRomInfo piratesRomDesc[] = {
	{ "r_449b.bin",		0x80000, 0x224aeeda, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "l_5c1e.bin",		0x80000, 0x46740204, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "p4_4d48.bin",	0x80000, 0x89fda216, 2 | BRF_GRA },           //  2 Tiles
	{ "p2_5d74.bin",	0x80000, 0x40e069b4, 2 | BRF_GRA },           //  3
	{ "p1_7b30.bin",	0x80000, 0x26d78518, 2 | BRF_GRA },           //  4
	{ "p8_9f4f.bin",	0x80000, 0xf31696ea, 2 | BRF_GRA },           //  5

	{ "s1_6e89.bin",	0x80000, 0xc78a276f, 3 | BRF_GRA },           //  6 Sprites
	{ "s2_6df3.bin",	0x80000, 0x9f0bad96, 3 | BRF_GRA },           //  7
	{ "s4_fdcc.bin",	0x80000, 0x8916ddb5, 3 | BRF_GRA },           //  8
	{ "s8_4b7c.bin",	0x80000, 0x1c41bd2c, 3 | BRF_GRA },           //  9

	{ "s89_49d4.bin",	0x80000, 0x63a739ec, 4 | BRF_SND },           // 10 Oki Samples
};

STD_ROM_PICK(pirates)
STD_ROM_FN(pirates)

struct BurnDriver BurnDrvPirates = {
	"pirates", NULL, NULL, NULL, "1994",
	"Pirates\0", NULL, "NIX", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MISC, 0,
	NULL, piratesRomInfo, piratesRomName, NULL, NULL, PiratesInputInfo, PiratesDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	288, 224, 4, 3
};


// Genix Family

static struct BurnRomInfo genixRomDesc[] = {
	{ "1.15",	0x80000, 0xd26abfb0, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "2.16",	0x80000, 0xa14a25b4, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "7.34",	0x40000, 0x58da8aac, 2 | BRF_GRA },           //  2 Tiles
	{ "9.35",	0x40000, 0x96bad9a8, 2 | BRF_GRA },           //  3
	{ "8.48",	0x40000, 0x0ddc58b6, 2 | BRF_GRA },           //  4
	{ "10.49",	0x40000, 0x2be308c5, 2 | BRF_GRA },           //  5

	{ "6.69",	0x40000, 0xb8422af7, 3 | BRF_GRA },           //  6 Sprites
	{ "5.70",	0x40000, 0xe46125c5, 3 | BRF_GRA },           //  7
	{ "4.71",	0x40000, 0x7a8ed21b, 3 | BRF_GRA },           //  8
	{ "3.72",	0x40000, 0xf78bd6ca, 3 | BRF_GRA },           //  9

	{ "0.31",	0x80000, 0x80d087bc, 4 | BRF_SND },           // 10 Oki Samples
};

STD_ROM_PICK(genix)
STD_ROM_FN(genix)

struct BurnDriver BurnDrvGenix = {
	"genix", NULL, NULL, NULL, "1994",
	"Genix Family\0", NULL, "NIX", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MISC, 0,
	NULL, genixRomInfo, genixRomName, NULL, NULL, PiratesInputInfo, PiratesDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	288, 224, 4, 3
};
