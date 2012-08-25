#include "tiles_generic.h"
#include "z80_intf.h"

#include "driver.h"
extern "C" {
#include "ay8910.h"
}

UINT8 DrvJoy1[7] = {0, 0, 0, 0, 0, 0, 0};
UINT8 DrvJoy2[7] = {0, 0, 0, 0, 0, 0, 0};
UINT8 BjDip[2] = {0, 0};
static UINT8 DrvReset = 0;
static INT32 bombjackIRQ = 0;
static INT32 latch;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static UINT8 *Mem = NULL;
static UINT8 *MemEnd = NULL;
static UINT8 *RamStart = NULL;
static UINT8 *RamEnd = NULL;
static UINT8 *BjGfx = NULL;
static UINT8 *BjMap = NULL;
static UINT8 *BjRom = NULL;
static UINT8 *BjRam = NULL;
static UINT8 *BjColRam = NULL;
static UINT8 *BjVidRam = NULL;
static UINT8 *BjSprRam = NULL;

// sound cpu
static UINT8 *SndRom = NULL;
static UINT8 *SndRam = NULL;

// graphics tiles
static UINT8 *text = NULL;
static UINT8 *sprites = NULL;
static UINT8 *tiles = NULL;

// pallete
static UINT8 *BjPalSrc = NULL;
static UINT32 *BjPalReal = NULL;

static INT16* pFMBuffer;
static INT16* pAY8910Buffer[9];

// Dip Switch and Input Definitions
static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 0,	  "p1 coin"  },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 1,	  "p1 start" },

	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 2, 	"p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 3, 	"p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 4, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 5, 	"p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 6,		"p1 fire 1"},

	{"P2 Coin"      , BIT_DIGITAL  , DrvJoy2 + 0,	  "p2 coin"  },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 1,	  "p2 start" },

	{"P2 Up"        , BIT_DIGITAL  , DrvJoy2 + 2, 	"p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy2 + 3, 	"p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 4, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 5, 	"p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 6,		"p2 fire 1"},

	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,		"reset"    },
	{"Dip Sw(1)"    , BIT_DIPSWITCH, BjDip + 0  ,	  "dip"      },
	{"Dip Sw(2)"    , BIT_DIPSWITCH, BjDip + 1  ,	  "dip"      },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo BjDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0xc0, NULL},
	{0x10, 0xff, 0xff, 0x00, NULL},

	// Dip Sw(1)
	{0,		0xfe, 0,	4,	  "Coin A"},
	{0x0f, 0x01, 0x03, 0x00, "1 coin 1 credit"},
	{0x0f, 0x01, 0x03, 0x01, "1 coin 2 credits"},
	{0x0f, 0x01, 0x03, 0x02, "1 coin 3 credits"},
	{0x0f, 0x01, 0x03, 0x03, "1 coin 6 credits"},

	{0,		0xfe, 0,	4,	  "Coin B"},
	{0x0f, 0x01, 0x0c, 0x04, "2 coins 1 credit"},
	{0x0f, 0x01, 0x0c, 0x00, "1 coin 1 credit"},
	{0x0f, 0x01, 0x0c, 0x08, "1 coin 2 credits"},
	{0x0f, 0x01, 0x0c, 0x0c, "1 coin 3 credits"},

	{0,		0xfe, 0,	4,	  "Lives"},
	{0x0f, 0x01, 0x30, 0x30, "2"},
	{0x0f, 0x01, 0x30, 0x00, "3"},
	{0x0f, 0x01, 0x30, 0x10, "4"},
	{0x0f, 0x01, 0x30, 0x20, "5"},

	{0,		0xfe, 0,	2,	  "Cabinet"},
	{0x0f, 0x01, 0x40, 0x40, "Upright"},
	{0x0f, 0x01, 0x40, 0x00, "Cocktail"},

	{0,		0xfe, 0,	2,	  "Demo sounds"},
	{0x0f, 0x01, 0x80, 0x00, "Off"},
	{0x0f, 0x01, 0x80, 0x80, "On"},

	// Dip Sw(2)
	{0,		0xfe, 0,	4,	  "Initial high score"},
	{0x10, 0x01, 0x07, 0x00, "10000"},
	{0x10, 0x01, 0x07, 0x01, "100000"},
	{0x10, 0x01, 0x07, 0x02, "30000"},
	{0x10, 0x01, 0x07, 0x03, "50000"},
	{0x10, 0x01, 0x07, 0x04, "100000"},
	{0x10, 0x01, 0x07, 0x05, "50000"},
	{0x10, 0x01, 0x07, 0x06, "100000"},
	{0x10, 0x01, 0x07, 0x07, "50000"},

	{0,		0xfe, 0,	4,	  "Bird speed"},
	{0x10, 0x01, 0x18, 0x00, "Easy"},
	{0x10, 0x01, 0x18, 0x08, "Medium"},
	{0x10, 0x01, 0x18, 0x10, "Hard"},
	{0x10, 0x01, 0x18, 0x18, "Hardest"},

	{0,		0xfe, 0,	4,	  "Enemies number & speed"},
	{0x10, 0x01, 0x60, 0x20, "Easy"},
	{0x10, 0x01, 0x60, 0x00, "Medium"},
	{0x10, 0x01, 0x60, 0x40, "Hard"},
	{0x10, 0x01, 0x60, 0x60, "Hardest"},

	{0,		0xfe, 0,	2,	  "Special coin"},
	{0x10, 0x01, 0x80, 0x00, "Easy"},
	{0x10, 0x01, 0x80, 0x80, "Hard"},
};

STDDIPINFO(Bj)

// Bomb Jack (set 1)
static struct BurnRomInfo BombjackRomDesc[] = {
	{ "09_j01b.bin",    0x2000, 0xc668dc30, BRF_ESS | BRF_PRG },		//  0 Z80 code
	{ "10_l01b.bin",    0x2000, 0x52a1e5fb, BRF_ESS | BRF_PRG },		//  1
	{ "11_m01b.bin",    0x2000, 0xb68a062a, BRF_ESS | BRF_PRG },		//  2
	{ "12_n01b.bin",    0x2000, 0x1d3ecee5, BRF_ESS | BRF_PRG },		//  3
	{ "13.1r",          0x2000, 0x70e0244d, BRF_ESS | BRF_PRG },		//  4

	// graphics 3 bit planes:
	{ "03_e08t.bin",    0x1000, 0x9f0470d5, BRF_GRA },			 // chars
	{ "04_h08t.bin",    0x1000, 0x81ec12e6, BRF_GRA },
	{ "05_k08t.bin",    0x1000, 0xe87ec8b1, BRF_GRA },

	{ "14_j07b.bin",    0x2000, 0x101c858d, BRF_GRA },			 // sprites
	{ "15_l07b.bin",    0x2000, 0x013f58f2, BRF_GRA },
	{ "16_m07b.bin",    0x2000, 0x94694097, BRF_GRA },

	{ "06_l08t.bin",    0x2000, 0x51eebd89, BRF_GRA },			 // background tiles
	{ "07_n08t.bin",    0x2000, 0x9dd98e9d, BRF_GRA },
	{ "08_r08t.bin",    0x2000, 0x3155ee7d, BRF_GRA },

	{ "02_p04t.bin",    0x1000, 0x398d4a02, BRF_GRA },			 // background tilemaps

	{ "01_h03t.bin",    0x2000, 0x8407917d, BRF_ESS | BRF_SND },		// sound CPU
};

STD_ROM_PICK(Bombjack)
STD_ROM_FN(Bombjack)


// Bomb Jack (set 2)
static struct BurnRomInfo Bombjac2RomDesc[] = {
	{ "09_j01b.bin",    0x2000, 0xc668dc30, BRF_ESS | BRF_PRG },		//  0 Z80 code
	{ "10_l01b.bin",    0x2000, 0x52a1e5fb, BRF_ESS | BRF_PRG },		//  1
	{ "11_m01b.bin",    0x2000, 0xb68a062a, BRF_ESS | BRF_PRG },		//  2
	{ "12_n01b.bin",    0x2000, 0x1d3ecee5, BRF_ESS | BRF_PRG },		//  3
	{ "13_r01b.bin",    0x2000, 0xbcafdd29, BRF_ESS | BRF_PRG },		//  4

	// graphics 3 bit planes:
	{ "03_e08t.bin",    0x1000, 0x9f0470d5, BRF_GRA },			 // chars
	{ "04_h08t.bin",    0x1000, 0x81ec12e6, BRF_GRA },
	{ "05_k08t.bin",    0x1000, 0xe87ec8b1, BRF_GRA },

	{ "14_j07b.bin",    0x2000, 0x101c858d, BRF_GRA },			 // sprites
	{ "15_l07b.bin",    0x2000, 0x013f58f2, BRF_GRA },
	{ "16_m07b.bin",    0x2000, 0x94694097, BRF_GRA },

	{ "06_l08t.bin",    0x2000, 0x51eebd89, BRF_GRA },			 // background tiles
	{ "07_n08t.bin",    0x2000, 0x9dd98e9d, BRF_GRA },
	{ "08_r08t.bin",    0x2000, 0x3155ee7d, BRF_GRA },

	{ "02_p04t.bin",    0x1000, 0x398d4a02, BRF_GRA },			 // background tilemaps

	{ "01_h03t.bin",    0x2000, 0x8407917d, BRF_ESS | BRF_SND },		// sound CPU
};

STD_ROM_PICK(Bombjac2)
STD_ROM_FN(Bombjac2)


static INT32 DrvDoReset()
{
	bombjackIRQ = 0;
	latch = 0;
	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}

	for (INT32 i = 0; i < 3; i++) {
		AY8910Reset(i);
	}
	return 0;
}


UINT8 __fastcall BjMemRead(UINT16 addr)
{
	UINT8 inputs=0;
	
	if (addr >= 0x9820 && addr <= 0x987f) return BjSprRam[addr - 0x9820];

	if (addr==0xb000) {
		if (DrvJoy1[5])
			inputs|=0x01;
		if (DrvJoy1[4])
			inputs|=0x02;
		if (DrvJoy1[2])
			inputs|=0x04;
		if (DrvJoy1[3])
			inputs|=0x08;
		if (DrvJoy1[6])
			inputs|=0x10;
		return inputs;
	}
	if (addr==0xb001) {
		if (DrvJoy2[5])
			inputs|=0x01;
		if (DrvJoy2[4])
			inputs|=0x02;
		if (DrvJoy2[2])
			inputs|=0x04;
		if (DrvJoy2[3])
			inputs|=0x08;
		if (DrvJoy2[6])
			inputs|=0x10;
		return inputs;
	}
	if (addr==0xb002) {
		if (DrvJoy1[0])
			inputs|=0x01;
		if (DrvJoy1[1])
			inputs|=0x04;
		if (DrvJoy2[0])
			inputs|=0x02;
		if (DrvJoy2[1])
			inputs|=0x08;
		return inputs;
	}
	if (addr==0xb004) {
		return BjDip[0]; // Dip Sw(1)
	}
	if (addr==0xb005) {
		return BjDip[1]; // Dip Sw(2)
	}
	return 0;
}

void __fastcall BjMemWrite(UINT16 addr,UINT8 val)
{
	if (addr >= 0x9820 && addr <= 0x987f) { BjSprRam[addr - 0x9820] = val; return; }
	
	if (addr==0xb000)
	{
		bombjackIRQ = val;
	}
	if(addr==0xb800)
	{
		latch=val;
		return;
	}
	BjRam[addr]=val;
}

UINT8 __fastcall SndMemRead(UINT16 a)
{
	if (a==0xFF00)
	{
		return 0x7f;
	}
	if(a==0x6000)
	{
		INT32 res;
		res = latch;
		latch = 0;
		return res;
	}
	return 0;
}



void __fastcall SndPortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	switch (a) {
		case 0x00: {
			AY8910Write(0, 0, d);
			return;
				   }
		case 0x01: {
			AY8910Write(0, 1, d);
			return;
				   }
		case 0x10: {
			AY8910Write(1, 0, d);
			return;
				   }
		case 0x11: {
			AY8910Write(1, 1, d);
			return;
				   }
		case 0x80: {
			AY8910Write(2, 0, d);
			return;
				   }
		case 0x81: {
			AY8910Write(2, 1, d);
			return;
				   }
	}
}

INT32 BjZInit()
{
	// Init the z80
	ZetInit(0);
	// Main CPU setup
	ZetOpen(0);

	ZetMapArea    (0x0000,0x7fff,0,BjRom+0x0000); // Direct Read from ROM
	ZetMapArea    (0x0000,0x7fff,2,BjRom+0x0000); // Direct Fetch from ROM
	ZetMapArea    (0xc000,0xdfff,0,BjRom+0x8000); // Direct Read from ROM
	ZetMapArea    (0xc000,0xdfff,2,BjRom+0x8000); // Direct Fetch from ROM

	ZetMapArea    (0x8000,0x8fff,0,BjRam+0x8000);
	ZetMapArea    (0x8000,0x8fff,1,BjRam+0x8000);

	ZetMapArea    (0x9000,0x93ff,0,BjVidRam);
	ZetMapArea    (0x9000,0x93ff,1,BjVidRam);

	ZetMapArea    (0x9400,0x97ff,0,BjColRam);
	ZetMapArea    (0x9400,0x97ff,1,BjColRam);

//	ZetMapArea    (0x9820,0x987f,0,BjSprRam);
//	ZetMapArea    (0x9820,0x987f,1,BjSprRam);

	ZetMapArea    (0x9c00,0x9cff,0,BjPalSrc);
	ZetMapArea    (0x9c00,0x9cff,1,BjPalSrc);

	ZetMapArea    (0x9e00,0x9e00,0,BjRam+0x9e00);
	ZetMapArea    (0x9e00,0x9e00,1,BjRam+0x9e00);

	//	ZetMapArea    (0xb000,0xb000,0,BjRam+0xb000);
	//	ZetMapArea    (0xb000,0xb000,1,BjRam+0xb000);

	//	ZetMapArea    (0xb800,0xb800,0,BjRam+0xb800);
	//	ZetMapArea    (0xb800,0xb800,1,BjRam+0xb800);

	ZetSetReadHandler(BjMemRead);
	ZetSetWriteHandler(BjMemWrite);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea    (0x0000,0x1fff,0,SndRom); // Direct Read from ROM
	ZetMapArea    (0x0000,0x1fff,2,SndRom); // Direct Fetch from ROM
	ZetMapArea    (0x4000,0x43ff,0,SndRam);
	ZetMapArea    (0x4000,0x43ff,1,SndRam);
	ZetMapArea    (0x4000,0x43ff,2,SndRam); // fetch from ram?
	ZetMapArea    (0xff00,0xffff,0,SndRam);
	ZetMapArea    (0xff00,0xffff,1,SndRam);
	ZetMapArea    (0xff00,0xffff,2,SndRam); // more fetch from ram? What the hell . .

	//	ZetMapArea    (0x6000,0x6000,0,BjRam+0xb800);
	//	ZetMapArea    (0x6000,0x6000,1,BjRam+0xb800);
	ZetSetReadHandler(SndMemRead);
	ZetSetOutHandler(SndPortWrite);
	ZetMemEnd();
	ZetClose();

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;
	pAY8910Buffer[6] = pFMBuffer + nBurnSoundLen * 6;
	pAY8910Buffer[7] = pFMBuffer + nBurnSoundLen * 7;
	pAY8910Buffer[8] = pFMBuffer + nBurnSoundLen * 8;

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(2, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910SetAllRoutes(0, 0.13, BURN_SND_ROUTE_BOTH);
	AY8910SetAllRoutes(1, 0.13, BURN_SND_ROUTE_BOTH);
	AY8910SetAllRoutes(2, 0.13, BURN_SND_ROUTE_BOTH);

	// remember to do ZetReset() in main driver

	DrvDoReset();
	return 0;
}



void DecodeTiles(UINT8 *TilePointer, INT32 num,INT32 off1,INT32 off2, INT32 off3)
{
	INT32 c,y,x,dat1,dat2,dat3,col;
	for (c=0;c<num;c++)
	{
		for (y=0;y<8;y++)
		{
			dat1=BjGfx[off1 + (c * 8) + y];
			dat2=BjGfx[off2 + (c * 8) + y];
			dat3=BjGfx[off3 + (c * 8) + y];
			for (x=0;x<8;x++)
			{
				col=0;
				if (dat1&1){ col |= 4;}
				if (dat2&1){ col |= 2;}
				if (dat3&1){ col |= 1;}
				TilePointer[(c * 64) + ((7-x) * 8) + (7 - y)]=col;
				dat1>>=1;
				dat2>>=1;
				dat3>>=1;
			}
		}
	}
}


static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	BjRom		  = Next; Next += 0x10000;
	BjGfx		  = Next; Next += 0x0f000;
	BjMap		  = Next; Next += 0x01000;
	SndRom	  = Next; Next += 0x02000;
	RamStart  = Next;
	BjRam		  = Next; Next += 0x10000;
	SndRam	  = Next; Next += 0x01000;
	BjPalSrc  = Next; Next += 0x00100;
	BjVidRam  = Next; Next += 0x00400;
	BjColRam  = Next; Next += 0x00400;
	BjSprRam  = Next; Next += 0x00060;
	RamEnd	  = Next;
	text		  = Next; Next += 512 * 8 * 8;
	sprites	  = Next; Next += 1024 * 8 * 8;
	tiles		  = Next; Next += 1024 * 8 * 8;
	pFMBuffer	= (INT16*)Next; Next += nBurnSoundLen * 9 * sizeof(INT16);
	BjPalReal	= (UINT32*)Next; Next += 0x0080 * sizeof(UINT32);
	MemEnd	  = Next;

	return 0;
}


INT32 BjInit()
{
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	for (INT32 i =0; i<5 ; i++)
	{
		BurnLoadRom(BjRom+(0x2000*i),i,1); // load code roms
	}

	for (INT32 i=0;i<3;i++)
	{
		BurnLoadRom(BjGfx+(0x1000*i),i+5,1);
	}

	BurnLoadRom(BjGfx+0x3000,8,1);
	BurnLoadRom(BjGfx+0x5000,9,1);
	BurnLoadRom(BjGfx+0x7000,10,1);

	BurnLoadRom(BjGfx+0x9000,11,1);
	BurnLoadRom(BjGfx+0xB000,12,1);
	BurnLoadRom(BjGfx+0xD000,13,1);

	BurnLoadRom(BjMap,14,1); // load Background tile maps
	BurnLoadRom(SndRom,15,1); // load Sound CPU

	// Set memory access & Init
	BjZInit();

	DecodeTiles(text,512,0,0x1000,0x2000);
	DecodeTiles(sprites,1024,0x7000,0x5000,0x3000);
	DecodeTiles(tiles,1024,0x9000,0xB000,0xD000);
	// done

	GenericTilesInit();

	DrvDoReset();
	return 0;
}

INT32 BjExit()
{
	ZetExit();

	for (INT32 i = 0; i < 3; i++) {
		AY8910Exit(i);
	}

	GenericTilesExit();
	BurnFree(Mem);
	return 0;
}

static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour >> 0) & 0x0f;
	g = (nColour >> 4) & 0x0f;
	b = (nColour >> 8) & 0x0f;

	r = (r << 4) | r;
	g = (g << 4) | g;
	b = (b << 4) | b;

	return BurnHighCol(r, g, b, 0);
}

INT32 CalcAll()
{
	for (INT32 i = 0; i < 0x100; i++) {
		BjPalReal[i / 2] = CalcCol(BjPalSrc[i & ~1] | (BjPalSrc[i | 1] << 8));
	}

	return 0;
}

void BjRenderFgLayer()
{
	for (INT32 tileCount = 0; tileCount < 1024 ;tileCount++) 
	{
		INT32 code = BjVidRam[tileCount] + 16 * (BjColRam[tileCount] & 0x10);
		INT32 color = BjColRam[tileCount] & 0x0f;
		INT32 sy = (tileCount % 32);
		INT32 sx = 31 - (tileCount / 32);

		sx<<=3;
		sx-=16;
		sy<<=3;
		if (sx >= 0 && sx < 215 && sy >= 0 && sy < 246)
		{
			Render8x8Tile_Mask(pTransDraw, code,sx,sy,color, 3, 0, 0, text);
		}
		else
		{
			Render8x8Tile_Mask_Clip(pTransDraw, code,sx,sy,color, 3, 0, 0, text);
		}
	}
}


void BjRenderBgLayer()
{
	for (INT32 tileCount = 0; tileCount < 256;tileCount++) {
		INT32 FlipX;

		INT32 BgSel=BjRam[0x9e00];

		INT32 offs = (BgSel & 0x07) * 0x200 + tileCount;
		INT32 Code = (BgSel & 0x10) ? BjMap[offs] : 0;

		INT32 attr = BjMap[offs + 0x100];
		INT32 Colour = attr & 0x0f;
		//INT32 flags = (attr & 0x80) ? TILE_FLIPY : 0;


		INT32 sy = (tileCount % 16);
		INT32 sx = 15 - (tileCount / 16);
		FlipX = attr & 0x80;

		/*if (SolomonFlipScreen) {
		sx = 31 - sx;
		sy = 31 - sy;
		FlipX = !FlipX;
		FlipY = !FlipY;
		}*/

		sx <<= 4;
		sx -=16;
		sy <<= 4;
		Code <<= 2;
		if (sx >= 0 && sx < 215 && sy >= 0 && sy < 246) {

			if (FlipX&0x80)
			{
				Render8x8Tile_Mask(pTransDraw, Code+0,sx+8,sy+0, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask(pTransDraw, Code+1,sx+8,sy+8, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask(pTransDraw, Code+2,sx+0,sy+0, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask(pTransDraw, Code+3,sx+0,sy+8, Colour, 3, 0, 0, tiles);

			}
			else
			{
				Render8x8Tile_Mask(pTransDraw, Code+0,sx+8,sy+0, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask(pTransDraw, Code+1,sx+8,sy+8, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask(pTransDraw, Code+2,sx+0,sy+0, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask(pTransDraw, Code+3,sx+0,sy+8, Colour, 3, 0, 0, tiles);
			}

		} else {

			if (FlipX&0x80)
			{
				Render8x8Tile_Mask_Clip(pTransDraw, Code+0,sx+8,sy+0, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask_Clip(pTransDraw, Code+1,sx+8,sy+8, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask_Clip(pTransDraw, Code+2,sx+0,sy+0, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask_Clip(pTransDraw, Code+3,sx+0,sy+8, Colour, 3, 0, 0, tiles);

			}
			else
			{
				Render8x8Tile_Mask_Clip(pTransDraw, Code+0,sx+8,sy+0, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask_Clip(pTransDraw, Code+1,sx+8,sy+8, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask_Clip(pTransDraw, Code+2,sx+0,sy+0, Colour, 3, 0, 0, tiles);
				Render8x8Tile_Mask_Clip(pTransDraw, Code+3,sx+0,sy+8, Colour, 3, 0, 0, tiles);
			}

		}
	}
}


static void BjDrawSprites()
{
	INT32 offs;

	for (offs = 0x60 - 4; offs >= 0; offs -= 4)
	{

		/*
		abbbbbbb cdefgggg hhhhhhhh iiiiiiii

		a        use big sprites (32x32 instead of 16x16)
		bbbbbbb  sprite code
		c        x flip
		d        y flip (used only in death sequence?)
		e        ? (set when big sprites are selected)
		f        ? (set only when the bonus (B) materializes?)
		gggg     color
		hhhhhhhh x position
		iiiiiiii y position
		*/
		INT32 sx,sy,flipx,flipy, code, colour, big;


		sy = BjSprRam[offs+3];
		if (BjSprRam[offs] & 0x80)
			sx = BjSprRam[offs+2];
		else
			sx = BjSprRam[offs+2];
		flipx = BjSprRam[offs+1] & 0x80;
		flipy =	BjSprRam[offs+1] & 0x40;

		code = BjSprRam[offs] & 0x7f;
		colour = (BjSprRam[offs+1] & 0x0f);
		big = (BjSprRam[offs] & 0x80);

		//sy -= 32;

		sx -=16;
		if (!big)
		{
			code <<= 2;
			if (sx >= 0 && sx < 215 && sy >= 0 && sy < 246) {
				if (!flipy) {
					if (!flipx) {
						Render8x8Tile_Mask(pTransDraw, code + 0, sx + 8, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code + 1, sx + 8, sy + 8, colour, 3, 0, 0, sprites);					
						Render8x8Tile_Mask(pTransDraw, code + 2, sx + 0, sy + 0, colour, 3, 0, 0, sprites);		
						Render8x8Tile_Mask(pTransDraw, code + 3, sx + 0, sy + 8, colour, 3, 0, 0, sprites);	
					} else {
						Render8x8Tile_Mask_FlipX(pTransDraw, code + 2, sx + 8, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipX(pTransDraw, code + 3, sx + 8, sy + 8, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipX(pTransDraw, code + 0, sx + 0, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipX(pTransDraw, code + 1, sx + 0, sy + 8, colour, 3, 0, 0, sprites);
					}
				} else {
					if (!flipx) {
						Render8x8Tile_Mask_FlipY(pTransDraw, code + 1, sx + 8, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipY(pTransDraw, code + 0, sx + 8, sy + 8, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipY(pTransDraw, code + 3, sx + 0, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipY(pTransDraw, code + 2, sx + 0, sy + 8, colour, 3, 0, 0, sprites);
					} else {
						Render8x8Tile_Mask_FlipXY(pTransDraw, code + 3, sx + 8, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipXY(pTransDraw, code + 2, sx + 8, sy + 8, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipXY(pTransDraw, code + 1, sx + 0, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipXY(pTransDraw, code + 0, sx + 0, sy + 8, colour, 3, 0, 0, sprites);
					}
				}
			} else {
				if (!flipy) {
					if (!flipx) {
						Render8x8Tile_Mask_Clip(pTransDraw, code + 0, sx + 8, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code + 1, sx + 8, sy + 8, colour, 3, 0, 0, sprites);					
						Render8x8Tile_Mask_Clip(pTransDraw, code + 2, sx + 0, sy + 0, colour, 3, 0, 0, sprites);		
						Render8x8Tile_Mask_Clip(pTransDraw, code + 3, sx + 0, sy + 8, colour, 3, 0, 0, sprites);	
					} else {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + 2, sx + 8, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + 3, sx + 8, sy + 8, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + 0, sx + 0, sy + 0, colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + 1, sx + 0, sy + 8, colour, 3, 0, 0, sprites);
					}
				} else {
					if (!flipx) {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + 1, sx + 0, sy + 0, colour, 4, 0, 0, sprites);
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + 0, sx + 0, sy + 8, colour, 4, 0, 0, sprites);
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + 3, sx + 8, sy + 0, colour, 4, 0, 0, sprites);
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + 2, sx + 8, sy + 8, colour, 4, 0, 0, sprites);
					} else {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + 3, sx + 8, sy + 0, colour, 4, 0, 0, sprites);
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + 2, sx + 8, sy + 8, colour, 4, 0, 0, sprites);
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + 1, sx + 0, sy + 0, colour, 4, 0, 0, sprites);
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + 0, sx + 0, sy + 8, colour, 4, 0, 0, sprites);
					}
				}
			}
		}
		else
		{	
			code&=31;
			code <<= 4;
			sx-=1;
			if (sx >= 0 && sx < 215 && sy >= 0 && sy < 246) 
			{
				if (!flipy) 
				{
					if (!flipx) {
						Render8x8Tile_Mask(pTransDraw, code+512,sx+8+16,sy,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+513,sx+8+16,sy+8,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+514,sx+16,sy,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+515,sx+16,sy+8,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+516,sx+8+16,sy+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+517,sx+8+16,sy+8+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+518,sx+16,sy+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+519,sx+16,sy+8+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+520,sx+8,sy,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+521,sx+8,sy+8,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+522,sx,sy,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+523,sx,sy+8,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+524,sx+8,sy+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+525,sx+8,sy+8+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+526,sx,sy+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask(pTransDraw, code+527,sx,sy+8+16,colour, 3, 0, 0, sprites);
					}
				}
			}
			else
			{
				if (!flipy) 
				{
					if (!flipx) {
						Render8x8Tile_Mask_Clip(pTransDraw, code+512,sx+8+16,sy,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+513,sx+8+16,sy+8,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+514,sx+16,sy,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+515,sx+16,sy+8,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+516,sx+8+16,sy+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+517,sx+8+16,sy+8+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+518,sx+16,sy+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+519,sx+16,sy+8+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+520,sx+8,sy,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+521,sx+8,sy+8,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+522,sx,sy,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+523,sx,sy+8,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+524,sx+8,sy+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+525,sx+8,sy+8+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+526,sx,sy+16,colour, 3, 0, 0, sprites);
						Render8x8Tile_Mask_Clip(pTransDraw, code+527,sx,sy+8+16,colour, 3, 0, 0, sprites);
					}
				}
			}
		}
	}
}

INT32 BjFrame()
{
	if (DrvReset) {	// Reset machine
		DrvDoReset();
	}

	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;

	nCyclesTotal[0] = 4000000 / 60;
	nCyclesTotal[1] = 3000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 1) 
		{
			if(bombjackIRQ)
			{
				ZetNmi();
			}
		}
		ZetClose();

		// Run Z80 #2
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();

		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Render(&pAY8910Buffer[0], pSoundBuf, nSegmentLength, 0);
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Render(&pAY8910Buffer[0], pSoundBuf, nSegmentLength, 0);
		}
	}
	/*ZetOpen(0);
	if (BjRam[0xb000])
	ZetNmi();
	ZetClose();*/

	ZetOpen(1);
	ZetNmi();
	ZetClose();


	if (pBurnDraw != NULL)
	{
		BurnTransferClear();
		CalcAll();

		BjRenderBgLayer();
		BjRenderFgLayer();
		BjDrawSprites();
		BurnTransferCopy(BjPalReal);
	}
	return 0;
}

static INT32 BjScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);			// Scan Z80

		// Scan critical driver variables
		SCAN_VAR(bombjackIRQ);
		SCAN_VAR(latch);
		SCAN_VAR(DrvJoy1);
		SCAN_VAR(DrvJoy2);
		SCAN_VAR(BjDip);
	}

	return 0;
}

struct BurnDriver BurnDrvBombjack = {
	"bombjack", NULL, NULL, NULL, "1984",
	"Bomb Jack (set 1)\0", NULL, "Tehkan", "Bomb Jack",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, BombjackRomInfo,BombjackRomName, NULL, NULL,DrvInputInfo,BjDIPInfo,
	BjInit,BjExit,BjFrame,NULL,BjScan,
	NULL,0x80,224,256,3,4
};

struct BurnDriver BurnDrvBombjac2 = {
	"bombjack2", "bombjack", NULL, NULL, "1984",
	"Bomb Jack (set 2)\0", NULL, "Tehkan", "Bomb Jack",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, Bombjac2RomInfo,Bombjac2RomName, NULL, NULL,DrvInputInfo,BjDIPInfo,
	BjInit,BjExit,BjFrame,NULL,BjScan,
	NULL,0x80,224,256,3,4
};
