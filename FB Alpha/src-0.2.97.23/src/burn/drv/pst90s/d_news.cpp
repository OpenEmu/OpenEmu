#include "tiles_generic.h"
#include "z80_intf.h"
#include "msm6295.h"

// FB Alpha - "News" Driver

// Input Related Variables
static UINT8 NewsInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 NewsDip[1]        = {0};
static UINT8 NewsInput[1]      = {0x00};
static UINT8 NewsReset         = 0;

// Memory Holders
static UINT8 *Mem              = NULL;
static UINT8 *MemEnd           = NULL;
static UINT8 *RamStart         = NULL;
static UINT8 *RamEnd           = NULL;
static UINT8 *NewsRom          = NULL;
static UINT8 *NewsRam          = NULL;
static UINT8 *NewsFgVideoRam   = NULL;
static UINT8 *NewsBgVideoRam   = NULL;
static UINT8 *NewsPaletteRam   = NULL;
static UINT32 *NewsPalette     = NULL;
static UINT8 *NewsTiles        = NULL;
static UINT8 *NewsTempGfx      = NULL;

// Misc Variables, system control values, etc.
static INT32 BgPic;

// Dip Switch and Input Definitions
static struct BurnInputInfo NewsInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , NewsInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , NewsInputPort0 + 1, "p1 start"  },

	{"Up"                , BIT_DIGITAL  , NewsInputPort0 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , NewsInputPort0 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , NewsInputPort0 + 4, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , NewsInputPort0 + 5, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , NewsInputPort0 + 6, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , NewsInputPort0 + 7, "p1 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &NewsReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, NewsDip + 0       , "dip"       },
};

STDINPUTINFO(News)

inline void NewsClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
	if ((*nJoystickInputs & 0x30) == 0x30) {
		*nJoystickInputs &= ~0x30;
	}
}

inline void NewsMakeInputs()
{
	// Reset Inputs
	NewsInput[0] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		NewsInput[0] |= (NewsInputPort0[i] & 1) << i;
	}

	// Clear Opposites
	NewsClearOpposites(&NewsInput[0]);
}

static struct BurnDIPInfo NewsDIPList[]=
{
	// Default Values
	{0x09, 0xff, 0xff, 0xdf, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x09, 0x01, 0x03, 0x00, "3 Coins 1 Credit"       },
	{0x09, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },
	{0x09, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x09, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x09, 0x01, 0x0c, 0x0c, "Easy"                   },
	{0x09, 0x01, 0x0c, 0x08, "Medium"                 },
	{0x09, 0x01, 0x0c, 0x04, "Hard"                   },
	{0x09, 0x01, 0x0c, 0x00, "Hardest"                },

	{0   , 0xfe, 0   , 2   , "Helps"                  },
	{0x09, 0x01, 0x10, 0x10, "1"                      },
	{0x09, 0x01, 0x10, 0x00, "2"                      },

	{0   , 0xfe, 0   , 2   , "Copyright"              },
	{0x09, 0x01, 0x20, 0x00, "Poby"                   },
	{0x09, 0x01, 0x20, 0x20, "Virus"                  },
};

STDDIPINFO(News)

static struct BurnDIPInfo NewsaDIPList[]=
{
	// Default Values
	{0x09, 0xff, 0xff, 0xdf, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x09, 0x01, 0x03, 0x00, "3 Coins 1 Credit"       },
	{0x09, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },
	{0x09, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x09, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x09, 0x01, 0x0c, 0x0c, "Easy"                   },
	{0x09, 0x01, 0x0c, 0x08, "Medium"                 },
	{0x09, 0x01, 0x0c, 0x04, "Hard"                   },
	{0x09, 0x01, 0x0c, 0x00, "Hardest"                },

	{0   , 0xfe, 0   , 2   , "Helps"                  },
	{0x09, 0x01, 0x10, 0x10, "1"                      },
	{0x09, 0x01, 0x10, 0x00, "2"                      },
};

STDDIPINFO(Newsa)

// Rom Definitions
static struct BurnRomInfo NewsRomDesc[] = {
	{ "virus.4",       0x08000, 0xaa005dfb, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code

	{ "virus.2",       0x40000, 0xb5af58d8, BRF_GRA },			 //  1	Tiles
	{ "virus.3",       0x40000, 0xa4b1c175, BRF_GRA },			 //  2	Tiles

	{ "virus.1",       0x40000, 0x41f5935a, BRF_SND },			 //  3	Samples
};


STD_ROM_PICK(News)
STD_ROM_FN(News)

static struct BurnRomInfo NewsaRomDesc[] = {
	{ "newsa_virus.4", 0x10000, 0x74a257da, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code

	{ "virus.2",       0x40000, 0xb5af58d8, BRF_GRA },			 //  1	Tiles
	{ "virus.3",       0x40000, 0xa4b1c175, BRF_GRA },			 //  2	Tiles

	{ "virus.1",       0x40000, 0x41f5935a, BRF_SND },			 //  3	Samples
};


STD_ROM_PICK(Newsa)
STD_ROM_FN(Newsa)

// Misc Driver Functions and Memory Handlers
INT32 NewsDoReset()
{
	BgPic = 0;

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);

	return 0;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour >> 8) & 0x0f;
	g = (nColour >> 4) & 0x0f;
	b = (nColour >> 0) & 0x0f;

	r = (r << 4) | r;
	g = (g << 4) | g;
	b = (b << 4) | b;

	return BurnHighCol(r, g, b, 0);
}

UINT8 __fastcall NewsRead(UINT16 a)
{
	switch (a) {
		case 0xc000: {
			return NewsDip[0];
		}

		case 0xc001: {
			return 0xff - NewsInput[0];
		}

		case 0xc002: {
			return MSM6295ReadStatus(0);
		}
	}

	return 0;
}

void __fastcall NewsWrite(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xc002: {
			MSM6295Command(0, d);
			return;
		}

		case 0xc003: {
			BgPic = d;
			return;
		}
	}

	if (a >= 0x9000 && a <= 0x91ff) {
		INT32 Offset = a - 0x9000;

		NewsPaletteRam[Offset] = d;
		NewsPalette[Offset / 2] = CalcCol(NewsPaletteRam[Offset | 1] | (NewsPaletteRam[Offset & ~1] << 8));
		return;
	}
}



// Function to Allocate and Index required memory
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	NewsRom              = Next; Next += 0x10000;
	MSM6295ROM           = Next; Next += 0x40000;

	RamStart = Next;

	NewsRam              = Next; Next += 0x02000;
	NewsFgVideoRam       = Next; Next += 0x00800;
	NewsBgVideoRam       = Next; Next += 0x00800;
	NewsPaletteRam       = Next; Next += 0x00200;

	RamEnd = Next;

	NewsTiles            = Next; Next += (16384 * 8 * 8);
	NewsPalette          = (UINT32*)Next; Next += 0x00100 * sizeof(UINT32);
	MemEnd = Next;

	return 0;
}

static INT32 TilePlaneOffsets[4]   = { 0, 1, 2, 3 };
static INT32 TileXOffsets[8]       = { 0, 4, 8, 12, 16, 20, 24, 28 };
static INT32 TileYOffsets[8]       = { 0, 32, 64, 96, 128, 160, 192, 224 };

// Driver Init and Exit Functions
INT32 NewsInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	NewsTempGfx = (UINT8*)BurnMalloc(0x80000);

	// Load Z80 Program Rom
	nRet = BurnLoadRom(NewsRom, 0, 1); if (nRet != 0) return 1;

	// Load, byte-swap and decode Tile Roms
	nRet = BurnLoadRom(NewsTempGfx + 0x00000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(NewsTempGfx + 0x00001, 2, 2); if (nRet != 0) return 1;
	GfxDecode(16384, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, NewsTempGfx, NewsTiles);
	BurnFree(NewsTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 3, 1); if (nRet != 0) return 1;

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, NewsRom        );
	ZetMapArea(0x0000, 0x7fff, 2, NewsRom        );
	ZetMapArea(0x8000, 0x87ff, 0, NewsFgVideoRam );
	ZetMapArea(0x8000, 0x87ff, 1, NewsFgVideoRam );
	ZetMapArea(0x8000, 0x87ff, 2, NewsFgVideoRam );
	ZetMapArea(0x8800, 0x8fff, 0, NewsBgVideoRam );
	ZetMapArea(0x8800, 0x8fff, 1, NewsBgVideoRam );
	ZetMapArea(0x8800, 0x8fff, 2, NewsBgVideoRam );
	ZetMapArea(0xe000, 0xffff, 0, NewsRam        );
	ZetMapArea(0xe000, 0xffff, 1, NewsRam        );
	ZetMapArea(0xe000, 0xffff, 2, NewsRam        );
	ZetMemEnd();
	ZetSetReadHandler(NewsRead);
	ZetSetWriteHandler(NewsWrite);
	ZetClose();

	// Setup the OKIM6295 emulation
	MSM6295Init(0, 8000, 0);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	// Reset the driver
	NewsDoReset();

	return 0;
}

INT32 NewsExit()
{
	MSM6295Exit(0);
	ZetExit();

	GenericTilesExit();

	BurnFree(Mem);

	BgPic = 0;

	return 0;
}

// Graphics Emulation
void NewsRenderBgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Code = (NewsBgVideoRam[TileIndex * 2] << 8) | NewsBgVideoRam[TileIndex * 2 + 1];
			Colour = (Code & 0xf000) >> 12;
			Code &= 0x0fff;
			if ((Code & 0x0e00) == 0xe00) Code = (Code & 0x1ff) | (BgPic << 9);
			x = 8 * mx;
			y = 8 * my;
			y -= 16;

			if (x > 7 && x < 248 && y > 7 && y < 216) {
				Render8x8Tile(pTransDraw, Code, x, y, Colour, 4, 0, NewsTiles);
			} else {
				Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0, NewsTiles);
			}

			TileIndex++;
		}
	}
}

void NewsRenderFgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Code = (NewsFgVideoRam[TileIndex * 2] << 8) | NewsFgVideoRam[TileIndex * 2 + 1];
			Colour = (Code & 0xf000) >> 12;
			Code &= 0x0fff;
			x = 8 * mx;
			y = 8 * my;
			y -= 16;

			if (x > 7 && x < 248 && y > 7 && y < 216) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, NewsTiles);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, NewsTiles);
			}

			TileIndex++;
		}
	}
}

void NewsDraw()
{
	NewsRenderBgLayer();
	NewsRenderFgLayer();
	BurnTransferCopy(NewsPalette);
}

// Frame Function
INT32 NewsFrame()
{
	if (NewsReset) NewsDoReset();

	NewsMakeInputs();

	ZetOpen(0);
	ZetRun(8000000 / 60);
	ZetRaiseIrq(0);
	ZetClose();

	if (pBurnDraw) NewsDraw();
	if (pBurnSoundOut) MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);

	return 0;
}

// Scan RAM
static INT32 NewsScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x02942;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
		
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = NewsPalette;
		ba.nLen	  = 0x100 * sizeof(UINT32);
		ba.szName = "Palette";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);			// Scan Z80

		MSM6295Scan(0, nAction);	// Scan OKIM6295

		// Scan critical driver variables
		SCAN_VAR(NewsInput);
		SCAN_VAR(NewsDip);
		SCAN_VAR(BgPic);
	}

	return 0;
}

// Driver Declaration
struct BurnDriver BurnDrvNews = {
	"news", NULL, NULL, NULL, "1993",
	"News (set 1)\0", NULL, "Poby / Virus", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, NewsRomInfo, NewsRomName, NULL, NULL, NewsInputInfo, NewsDIPInfo,
	NewsInit, NewsExit, NewsFrame, NULL, NewsScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvNewsa = {
	"newsa", "news", NULL, NULL, "1993",
	"News (set 2)\0", NULL, "Poby / Jeansole", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 1, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, NewsaRomInfo, NewsaRomName, NULL, NULL, NewsInputInfo, NewsaDIPInfo,
	NewsInit, NewsExit, NewsFrame, NULL, NewsScan,
	NULL, 0x100, 256, 224, 4, 3
};
