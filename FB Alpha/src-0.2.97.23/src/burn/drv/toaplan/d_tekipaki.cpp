#include "toaplan.h"
// Teki Paki

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;


// Rom information
static struct BurnRomInfo drvRomDesc[] = {
	{ "tp020-1.bin",  0x010000, 0xD8420BD5, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "tp020-2.bin",  0x010000, 0x7222DE8E, BRF_ESS | BRF_PRG }, //  1

	{ "tp020-4.bin",  0x080000, 0x3EBBE41E, BRF_GRA },	     //  2 GP9001 Tile data
	{ "tp020-3.bin",  0x080000, 0x2D5E2201, BRF_GRA },	     //  3

	{ "hd647180.020", 0x008000, 0x00000000, 0x10 | BRF_NODUMP }, //  4 Sound CPU
};

STD_ROM_PICK(drv)
STD_ROM_FN(drv)


// Rom information
static struct BurnRomInfo whoopeeRomDesc[] = {
	{ "whoopee.1",    0x020000, 0x28882e7e, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "whoopee.2",    0x020000, 0x6796f133, BRF_ESS | BRF_PRG }, //  1

	{ "tp025-4.bin",  0x100000, 0xab97f744, BRF_GRA },	     //  2 GP9001 Tile data
	{ "tp025-3.bin",  0x100000, 0x7b16101e, BRF_GRA },	     //  3

	{ "hd647180.025", 0x008000, 0x00000000, 0x10 | BRF_NODUMP }, //  4 Sound CPU
};

STD_ROM_PICK(whoopee)
STD_ROM_FN(whoopee)

static struct BurnInputInfo tekipakiInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 5,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 6,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL,	DrvButton + 0,	"diag"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(tekipaki)

static struct BurnDIPInfo tekipakiDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFE,	0x00,	  NULL},
	{0x15,	0xFF, 0x43,	0x00,	  NULL},
	{0x16,	0xFF, 0x0F,	0x00,	  NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x02,	0x00, "Normal screen"},
	{0x14,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},

	// Normal coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x82, 0x30,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0x30,	0x10, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0x30,	0x20, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0x30,	0x30, "2 coins 3 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x82, 0xC0,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0xC0,	0x40, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0xC0,	0x80, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0xC0,	0xC0, "2 coins 3 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},

	// European coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x02, 0x30,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0x30,	0x10, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0x30,	0x20, "3 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0x30,	0x30, "3 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x02, 0xC0,	0x00, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0xC0,	0x40, "1 coin 3 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0xC0,	0x80, "1 coin 4 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0xC0,	0xC0, "1 coin 6 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B"},
	{0x15,	0x01, 0x03,	0x01, "A"},
	{0x15,	0x01, 0x03,	0x02, "C"},
	{0x15,	0x01, 0x03,	0x03, "D"},
	{0,		0xFE, 0,	2,	  NULL},
    {0x15,	0x01, 0x40,	0x00, "Normal game"},
    {0x15,	0x01, 0x40,	0x40, "Stop mode"},

	// Region
	{0,		0xFE, 0,	9,	  "Game difficulty"},
	{0x16,	0x01, 0x0F,	0x00, "Japan"},
	{0x16,	0x01, 0x0F,	0x01, "U.S.A."},
	{0x16,	0x01, 0x0F,	0x02, "Europe"},
	{0x16,	0x01, 0x0F,	0x03, "Hong Kong"},
	{0x16,	0x01, 0x0F,	0x04, "Korea"},
	{0x16,	0x01, 0x0F,	0x05, "Taiwan"},
	{0x16,	0x01, 0x0F,	0x06, "Taiwan (Spacey Co, ltd.)"},
	{0x16,	0x01, 0x0F,	0x07, "U.S.A. (Romstar inc.)"},
	{0x16,	0x01, 0x0F,	0x08, "Hong Kong (Honest Trading Co.)"},
	{0x16,	0x01, 0x0F,	0x09, "Japan"},
	{0x16,	0x01, 0x0F,	0x0A, "Japan"},
	{0x16,	0x01, 0x0F,	0x0B, "Japan"},
	{0x16,	0x01, 0x0F,	0x0C, "Japan"},
	{0x16,	0x01, 0x0F,	0x0D, "Japan"},
	{0x16,	0x01, 0x0F,	0x0E, "Japan"},
	{0x16,	0x01, 0x0F,	0x0F, "Japan"},
};

STDDIPINFO(tekipaki)

static struct BurnInputInfo whoopeeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvButton + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvButton + 6,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service",		BIT_DIGITAL,	DrvButton + 0,	"service"},
	{"Tilt",		BIT_DIGITAL,	DrvButton + 1,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(whoopee)

static struct BurnDIPInfo whoopeeDIPList[]=
{
	{0x13, 0xff, 0xff, 0x00, NULL		},
	{0x14, 0xff, 0xff, 0x00, NULL		},
	{0x15, 0xff, 0xff, 0xf6, NULL		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x00, "Off"		},
	{0x13, 0x01, 0x02, 0x02, "On"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x00, "Off"		},
	{0x13, 0x01, 0x04, 0x04, "On"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x08, "Off"		},
	{0x13, 0x01, 0x08, 0x00, "On"		},

	{0   , 0xfe, 0   ,    7, "Coin A"		},
	{0x13, 0x01, 0x30, 0x30, "4 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x20, "3 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x00, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0x30, 0x30, "2 Coins 3 Credits"		},
	{0x13, 0x01, 0x30, 0x10, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x13, 0x01, 0xc0, 0x80, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0xc0, 0xc0, "2 Coins 3 Credits"		},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  2 Credits"		},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  2 Credits"		},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"		},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  4 Credits"		},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   ,    0, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x03, "Hardest"		},
	{0x14, 0x01, 0x03, 0x02, "Hard"		},
	{0x14, 0x01, 0x03, 0x00, "Normal"		},
	{0x14, 0x01, 0x03, 0x01, "Easy"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "None"		},
	{0x14, 0x01, 0x0c, 0x08, "200k only"		},
	{0x14, 0x01, 0x0c, 0x00, "200k and every 300k"		},
	{0x14, 0x01, 0x0c, 0x04, "150k and every 200k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x30, "1"		},
	{0x14, 0x01, 0x30, 0x20, "2"		},
	{0x14, 0x01, 0x30, 0x00, "3"		},
	{0x14, 0x01, 0x30, 0x10, "5"		},

	{0   , 0xfe, 0   ,    4, "Invulnerability"		},
	{0x14, 0x01, 0x40, 0x00, "Off"		},
	{0x14, 0x01, 0x40, 0x40, "On"		},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x15, 0x01, 0x07, 0x06, "Europe"		},
	{0x15, 0x01, 0x07, 0x07, "Europe (Nova Apparate GMBH & Co)"		},
	{0x15, 0x01, 0x07, 0x04, "USA"		},
	{0x15, 0x01, 0x07, 0x05, "USA (Romstar)"		},
	{0x15, 0x01, 0x07, 0x00, "Japan"		},
	{0x15, 0x01, 0x07, 0x01, "Asia"		},
	{0x15, 0x01, 0x07, 0x02, "Hong Kong (Honest Trading Co.)"		},
	{0x15, 0x01, 0x07, 0x03, "Taiwan"		},

	{0   , 0xfe, 0   ,    0, "Nudity"		},
	{0x15, 0x01, 0x08, 0x08, "Low"		},
	{0x15, 0x01, 0x08, 0x00, "High, but censored"		},
};

STDDIPINFO(whoopee)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal;

static INT32 nColCount = 0x0800;

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x040000;		// 68000 ROM
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	RamStart	= Next;
	Ram01		= Next; Next += 0x003000;		// CPU #0 work RAM
	RamPal		= Next; Next += 0x001000;		// palette
	GP9001RAM[0]= Next; Next += 0x008000;		// Double size, as the game tests too much memory during POST
	GP9001Reg[0]= (UINT16*)Next; Next += 0x0100 * sizeof(UINT16);
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x020997;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd-RamStart;
		ba.szName	= "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states

		ToaScanGP9001(nAction, pnMin);
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	ToaLoadCode(Rom01, 0, 2);

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 2, 2, nGP9001ROMSize[0]);

	return 0;
}

UINT8 __fastcall tekipakiReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x180051:								// Player 1 inputs
			return DrvInput[0];
		case 0x180061:								// Player 2 inputs
			return DrvInput[1];
		case 0x180021:								// Other inputs
			return DrvInput[2];

		case 0x180001:								// Dipswitch 1
			return DrvInput[3];
		case 0x180011:			   					// Dipswitch 2
			return DrvInput[4];
		case 0x180031:								// Dipswitch 3 - Territory
			return (DrvInput[5] & 0x0F) | 0x10;

		case 0x14000D:								// VBlank
			return ToaVBlankRegister();

//		default:
//			printf("Attempt to read byte value of location %x\n", sekAddress);
	}

	return 0;
}

UINT16 __fastcall tekipakiReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x180050:								// Player 1 inputs
			return DrvInput[0];
		case 0x180060:								// Player 2 inputs
			return DrvInput[1];
		case 0x180020:								// Other inputs
			return DrvInput[2];

		case 0x180000:								// Dipswitch 1
			return DrvInput[3];
		case 0x180010:								// Dipswitch 2
			return DrvInput[4];
		case 0x180030:								// Dipswitch 3 - Territory
			return (DrvInput[5] & 0x0F) | 0x10;

		case 0x140004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x140006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x14000C:
			return ToaVBlankRegister();

//		default:
//			printf("Attempt to read word value of location %x\n", sekAddress);
	}

	return 0;
}

void __fastcall tekipakiWriteByte(UINT32 /*sekAddress*/, UINT8 /*byteValue*/)
{
//	switch (sekAddress) {

//		default:
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
//	}
}

void __fastcall tekipakiWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x140000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x140004:
			ToaGP9001WriteRAM(wordValue, 0);
			break;
		case 0x140006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x140008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x14000C:
			ToaGP9001WriteRegister(wordValue);
			break;

//		default:
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);
	}
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = false;
#endif

	nGP9001ROMSize[0] = 0x800000;

	// Find out how much memory is needed
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();													// Index the allocated memory

	// Load the roms into memory
	if (LoadRoms()) {
		return 1;
	}

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,		0x000000, 0x03FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,		0x080000, 0x082FFF, SM_RAM);
		SekMapMemory(RamPal,	0x0c0000, 0x0c0FFF, SM_RAM);	// Palette RAM

		SekSetReadWordHandler(0, tekipakiReadWord);
		SekSetReadByteHandler(0, tekipakiReadByte);
		SekSetWriteWordHandler(0, tekipakiWriteWord);
		SekSetWriteByteHandler(0, tekipakiWriteByte);

		SekClose();
	}

	nSpriteYOffset = 0x0011;

	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;

	ToaInitGP9001();

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	bDrawScreen = true;

	DrvDoReset();			// Reset machine
	return 0;
}

static INT32 DrvExit()
{
	ToaPalExit();

	ToaExitGP9001();
	SekExit();				// Deallocate 68000s

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDraw()
{
	ToaClearScreen(0x120);

	if (bDrawScreen) {
		ToaGetBitmap();
		ToaRenderGP9001();					// Render GP9001 graphics
	}

	ToaPalUpdate();							// Update the palette

	return 0;
}

inline static INT32 CheckSleep(INT32)
{
	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 4;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x00;													// Buttons
	DrvInput[1] = 0x00;													// Player 1
	DrvInput[2] = 0x00;													// Player 2
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvButton[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInput[0]);
	ToaClearOpposites(&DrvInput[1]);

	SekNewFrame();

	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesDone[0] = 0;
	
	SekOpen(0);

	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;

	for (INT32 i = 0; i < nInterleave; i++) {
    	INT32 nCurrentCPU;
		INT32 nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;


		// Trigger VBlank interrupt
		if (!bVBlank && nNext > nToaCyclesVBlankStart) {
			if (nCyclesDone[nCurrentCPU] < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - nCyclesDone[nCurrentCPU];
				nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
			}

			bVBlank = true;

			ToaBufferGP9001Sprites();

			// Trigger VBlank interrupt
			SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

	}

	SekClose();

	if (pBurnDraw) {
		DrvDraw();												// Draw screen if needed
	}

	return 0;
}

struct BurnDriver BurnDrvTekiPaki = {
	"tekipaki", NULL, NULL, NULL, "1991",
	"Teki Paki\0", "No sound (sound MCU not dumped)", "Toaplan", "Toaplan GP9001 based",
	L"Teki Paki\0\u6D17\u8133\u30B2\u30FC\u30E0\0", NULL, NULL, NULL,
	1, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_PUZZLE, 0,
	NULL, drvRomInfo, drvRomName, NULL, NULL, tekipakiInputInfo, tekipakiDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvWhoopee = {
	"whoopee", "pipibibs", NULL, NULL, "1991",
	"Pipi & Bibis / Whoopee!! (Teki Paki hardware)\0", "No sound (sound MCU not dumped)", "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	1 | BDF_CLONE, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_PLATFORM, 0,
	NULL, whoopeeRomInfo, whoopeeRomName, NULL, NULL, whoopeeInputInfo, whoopeeDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};
