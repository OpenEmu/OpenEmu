// FB Alpha 1945K-III driver module
// Port to Finalburn Alpha by OopsWare. 2007

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "msm6295.h"

static UINT8 *Mem	= NULL;
static UINT8 *MemEnd	= NULL;
static UINT8 *RamStart;
static UINT8 *RamEnd;
static UINT8 *Rom68K;
static UINT8 *RomBg;
static UINT8 *RomSpr;
static UINT8 *Ram68K;
static UINT16 *RamPal;
static UINT16 *RamSpr0;
static UINT16 *RamSpr1;
static UINT16 *RamBg;

static UINT32 *RamCurPal;
static UINT8 bRecalcPalette;

static UINT16 *scrollx;
static UINT16 *scrolly;
static UINT8 *m6295bank;

static UINT8 DrvReset;
static UINT8 DrvButton[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static UINT8 DrvJoy1[8]   = { 0, 0, 0, 0, 0, 0, 0, 0 };
static UINT8 DrvJoy2[8]   = { 0, 0, 0, 0, 0, 0, 0, 0 };
static UINT8 DrvInput[6]  = { 0, 0, 0, 0, 0, 0 };

static struct BurnInputInfo _1945kiiiInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvButton + 2,	"p1 start"	},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 4"	},

	{"P2 Start",		BIT_DIGITAL,	DrvButton + 3,	"p2 start"	},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},	
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 4"	},		
	
	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"		},
};

STDINPUTINFO(_1945kiii)

static struct BurnDIPInfo _1945kiiiDIPList[] = {
	{0x14, 0xFF, 0xFF, 0xef, NULL			},
	{0x15, 0xFF, 0xFF, 0xff, NULL			},

	{0,    0xFE, 0,    8,	 "Coin 1"		},
	{0x14, 0x01, 0x07, 0x07, "1 coin  1 credit"	},
	{0x14, 0x01, 0x07, 0x06, "2 coins 1 credit"	},
	{0x14, 0x01, 0x07, 0x05, "3 coins 1 credit"	},
	{0x14, 0x01, 0x07, 0x04, "1 coin 2 credits"	},
	{0x14, 0x01, 0x07, 0x03, "Free Play"		},
	{0x14, 0x01, 0x07, 0x02, "5 coins 1 credit"	},
	{0x14, 0x01, 0x07, 0x01, "4 coins 1 credit"	},
	{0x14, 0x01, 0x07, 0x00, "1 coin 3 credits"	},
	{0,    0xFE, 0,    4,	 "Difficulty"		},
	{0x14, 0x01, 0x18, 0x18, "Hardest"		},
	{0x14, 0x01, 0x18, 0x10, "Hard"			},
	{0x14, 0x01, 0x18, 0x08, "Normal"		},
	{0x14, 0x01, 0x18, 0x00, "Easy"			},
	{0,    0xFE, 0,    4,	 "Lives"		},
	{0x14, 0x01, 0x60, 0x60, "3"			},
	{0x14, 0x01, 0x60, 0x40, "2"			},
	{0x14, 0x01, 0x60, 0x20, "4"			},
	{0x14, 0x01, 0x60, 0x00, "5"			},
	{0,    0xFE, 0,    2,	 "Service"		},
	{0x14, 0x01, 0x80, 0x80, "Off"			}, 
	{0x14, 0x01, 0x80, 0x00, "On"			},

	{0,    0xFE, 0,	   2,	 "Demo sound"		},
	{0x15, 0x01, 0x01, 0x01, "Off"			},
	{0x15, 0x01, 0x01, 0x00, "On"			},
	{0,    0xFE, 0,    2,	 "Allow Continue"	},
	{0x15, 0x01, 0x02, 0x02, "Yes"			},
	{0x15, 0x01, 0x02, 0x00, "No"			},
};

STDDIPINFO(_1945kiii)

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour & 0x001F) << 3;
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;
	g |= g >> 5;
	b = (nColour & 0x7C00) >> 7;
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

static void sndSetBank(UINT8 bank0, UINT8 bank1)
{
	if (bank0 != m6295bank[0]) {
		m6295bank[0] = bank0;
		for (INT32 nChannel = 0; nChannel < 4; nChannel++) {
			MSM6295SampleInfo[0][nChannel] = MSM6295ROM + 0x000000 + 0x040000 * bank0 + (nChannel << 8);
			MSM6295SampleData[0][nChannel] = MSM6295ROM + 0x000000 + 0x040000 * bank0 + (nChannel << 16);
		}
	}

	if (bank1 != m6295bank[1]) {
		m6295bank[1] = bank1;
		for (INT32 nChannel = 0; nChannel < 4; nChannel++) {
			MSM6295SampleInfo[1][nChannel] = MSM6295ROM + 0x080000 + 0x040000 * bank1 + (nChannel << 8);
			MSM6295SampleData[1][nChannel] = MSM6295ROM + 0x080000 + 0x040000 * bank1 + (nChannel << 16);
		}
	}
}

UINT16 __fastcall k1945iiiReadWord(UINT32 sekAddress)
{
	switch (sekAddress)
	{
		case 0x400000:
			return DrvInput[0] | (DrvInput[1] << 8);

		case 0x440000:
			return DrvInput[2] | (DrvInput[3] << 8);

		case 0x480000:
			return DrvInput[4] | (DrvInput[5] << 8);

		case 0x4C0000:
			return MSM6295ReadStatus(0);

		case 0x500000:
			return MSM6295ReadStatus(1);
	}

	return 0;
}

void __fastcall k1945iiiWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress)
	{
		case 0x4C0000:
			MSM6295Command(0, byteValue);
		return;

		case 0x500000:
			MSM6295Command(1, byteValue);
		return;
	}
}

void __fastcall k1945iiiWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress)
	{
		case 0x340000:
			scrollx[0] = wordValue;
		return;

		case 0x380000:
			scrolly[0] = wordValue;
		return;

		case 0x3C0000:
			sndSetBank((wordValue & 2) >> 1, (wordValue & 4) >> 2);
		return;			
	}
}

void __fastcall k1945iiiWriteWordPalette(UINT32 sekAddress, UINT16 wordValue)
{
	sekAddress = (sekAddress & 0xffe) / 2;
	RamPal[sekAddress] = BURN_ENDIAN_SWAP_INT16(wordValue);
	if (sekAddress < 0x200) RamCurPal[sekAddress] = CalcCol(wordValue);
}

static INT32 MemIndex()
{
	UINT8 *Next;

	Next		= Mem;

	Rom68K 		= Next; Next += 0x0100000;
	RomBg		= Next; Next += 0x0200000;
	RomSpr		= Next; Next += 0x0400000;
	MSM6295ROM	= Next; Next += 0x0100000;
	
	RamCurPal	= (UINT32 *) Next; Next += 0x00200 * sizeof(UINT32);

	RamStart	= Next;
	
	Ram68K		= Next; Next += 0x020000;
	RamPal		= (UINT16 *) Next; Next += 0x000800 * sizeof(UINT16);
	RamSpr0		= (UINT16 *) Next; Next += 0x000800 * sizeof(UINT16);
	RamSpr1		= (UINT16 *) Next; Next += 0x000800 * sizeof(UINT16);
	RamBg		= (UINT16 *) Next; Next += 0x000800 * sizeof(UINT16);

	m6295bank	= Next; Next += 0x000002;
	scrollx		= (UINT16 *) Next; Next += 0x000001 * sizeof(UINT16);
	scrolly		= (UINT16 *) Next; Next += 0x000001 * sizeof(UINT16);

	RamEnd		= Next;
	
	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	MSM6295Reset(0);
	MSM6295Reset(1);

	m6295bank[0] = 1;
	m6295bank[1] = 1;
	sndSetBank(0, 0);

	return 0;
}

static void decode_sprites() // Pixel order - 0,2,1,3 -> 0,1,2,3
{
	for (INT32 i = 0; i < 0x400000; i+=4) {
		UINT8 c = RomSpr[i+2];
		RomSpr[i+2] = RomSpr[i+1];
		RomSpr[i+1] = c;
	}
}

static INT32 DrvInit()
{
	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();	

	{
		if (BurnLoadRom(Rom68K     + 0x00000, 0, 2)) return 1;
		if (BurnLoadRom(Rom68K     + 0x00001, 1, 2)) return 1;

		if (BurnLoadRom(RomSpr     + 0x00000, 2, 2)) return 1;
		if (BurnLoadRom(RomSpr     + 0x00001, 3, 2)) return 1;

		if (BurnLoadRom(RomBg      + 0x00000, 4, 1)) return 1;

		if (BurnLoadRom(MSM6295ROM + 0x00000, 5, 1)) return 1;
		if (BurnLoadRom(MSM6295ROM + 0x80000, 6, 1)) return 1;

		decode_sprites();
	}

	{
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(Rom68K,			0x000000, 0x0FFFFF, SM_ROM);
		SekMapMemory(Ram68K,			0x100000, 0x10FFFF, SM_RAM);
		SekMapMemory((UINT8 *)RamPal,	0x200000, 0x200FFF, SM_ROM);
		SekMapHandler(1,			0x200000, 0x200FFF, SM_WRITE);	// palette write
		SekMapMemory((UINT8 *)RamSpr0,	0x240000, 0x240FFF, SM_RAM);
		SekMapMemory((UINT8 *)RamSpr1,	0x280000, 0x280FFF, SM_RAM);
		SekMapMemory((UINT8 *)RamBg,	0x2C0000, 0x2C0FFF, SM_RAM);
		SekMapMemory(Ram68K + 0x10000,		0x8C0000, 0x8CFFFF, SM_RAM);

		SekSetReadWordHandler(0, k1945iiiReadWord);
//		SekSetReadByteHandler(0, k1945iiiReadByte);
		SekSetWriteWordHandler(0, k1945iiiWriteWord);
		SekSetWriteByteHandler(0, k1945iiiWriteByte);

//		SekSetWriteByteHandler(1, k1945iiiWriteBytePalette);
		SekSetWriteWordHandler(1, k1945iiiWriteWordPalette);
		SekClose();
	}
	
	MSM6295Init(0, 7500, 1);
	MSM6295Init(1, 7500, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	MSM6295SetRoute(1, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();
	
	MSM6295Exit(0);
	MSM6295Exit(1);
	
	BurnFree(Mem);

	return 0;
}

static void DrawBackground()
{
	for (INT32 offs = 0; offs < 64*32; offs++)
	{
		INT32 sx = ((offs & 0x1f) * 16) - scrollx[0];
		if (sx <= -192) sx += 512;
		
		INT32 sy = ((offs / 0x20) * 16) - scrolly[0];
//		if (sy <= -288) sy += 512;
		
		if (sx <= -16 || sx >= 320 || sy <= -16 || sy >= 224)
			continue;

		INT32 code = BURN_ENDIAN_SWAP_INT16(RamBg[offs]) & 0x1fff;
		
		if (sx >= 0 && sx <= 304 && sy >= 0 && sy <= 208) {
			Render16x16Tile(pTransDraw, code, sx, sy, 0, 8, 0, RomBg);
		} else {
			Render16x16Tile_Clip(pTransDraw, code, sx, sy, 0, 8, 0, RomBg);
		}
	}
}

static void DrawSprites()
{
	for (INT32 i = 0; i < 0x1000/2; i++)
	{
		INT32 sx		 =  BURN_ENDIAN_SWAP_INT16(RamSpr0[i]) >> 8;
		INT32 sy		 =  BURN_ENDIAN_SWAP_INT16(RamSpr0[i]) & 0xff;
		INT32 code	 = (BURN_ENDIAN_SWAP_INT16(RamSpr1[i]) & 0x7ffe) >> 1;
		sx 		|= (BURN_ENDIAN_SWAP_INT16(RamSpr1[i]) & 0x0001) << 8;

		if (sx >= 336) sx -= 512;
		if (sy >= 240) sy -= 256;
			
		if (sx >= 0 && sx <= 304 && sy > 0 && sy <= 208) {
			Render16x16Tile_Mask(pTransDraw, code, sx, sy, 0, 8, 0, 0x100, RomSpr);
		} else if (sx >= -16 && sx < 320 && sy >= -16 && sy < 224) {
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, 0, 8, 0, 0x100, RomSpr);
		}
	}
}

static INT32 DrvDraw()
{
	if (bRecalcPalette) {
		for (INT32 i = 0; i < 0x200; i++) {
			RamCurPal[i] = CalcCol(RamPal[i]);
		}

		bRecalcPalette = 0;	
	}

	DrawBackground();
	DrawSprites();

	BurnTransferCopy(RamCurPal);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInput, 0xff, 4);
		for (INT32 i = 0; i < 8; i++) {
			DrvInput[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInput[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInput[2] ^= (DrvButton[i] & 1) << i;
		}
	}

	INT32 nTotalCycles = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));

	SekOpen(0);
	SekRun(nTotalCycles);
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnSoundOut) {
		memset(pBurnSoundOut, 0, nBurnSoundLen * 2 * sizeof(INT16));
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
	}
	
	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	if (pnMin) *pnMin =  0x029671;

	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {	
		memset(&ba, 0, sizeof(ba));
    		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd - RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {

		SekScan(nAction);

		MSM6295Scan(0, nAction);
		MSM6295Scan(1, nAction);
		
		if (nAction & ACB_WRITE) {
			INT32 bank0 = m6295bank[0];
			INT32 bank1 = m6295bank[1];
			m6295bank[0] = ~0;
			m6295bank[1] = ~0;

			sndSetBank(bank0, bank1);

			bRecalcPalette = 1;
		}
	}
	
	return 0;
}


// 1945k III

static struct BurnRomInfo _1945kiiiRomDesc[] = {
	{ "prg-1.u51",	0x080000, 0x6b345f27, 1 | BRF_ESS | BRF_PRG },	// 0 68000 code 
	{ "prg-2.u52", 	0x080000, 0xce09b98c, 1 | BRF_ESS | BRF_PRG }, 	// 1
	
	{ "m16m-1.u62",	0x200000, 0x0b9a6474, BRF_GRA }, 		// 2 Sprites
	{ "m16m-2.u63",	0x200000, 0x368a8c2e, BRF_GRA },		// 3 
	
	{ "m16m-3.u61",	0x200000, 0x32fc80dd, BRF_GRA }, 		// 4 Background Layer
	
	{ "snd-1.su7",	0x080000, 0xbbb7f0ff, BRF_SND }, 		// 5 MSM #0 Samples

	{ "snd-2.su4",	0x080000, 0x47e3952e, BRF_SND }, 		// 6 MSM #1 Samples
};

STD_ROM_PICK(_1945kiii)
STD_ROM_FN(_1945kiii)

struct BurnDriver BurnDrv1945kiii = {
	"1945kiii", NULL, NULL, NULL, "2000",
	"1945k III\0", NULL, "Oriental", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, _1945kiiiRomInfo, _1945kiiiRomName, NULL, NULL, _1945kiiiInputInfo, _1945kiiiDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &bRecalcPalette, 0x200,
	224, 320, 3, 4
};
