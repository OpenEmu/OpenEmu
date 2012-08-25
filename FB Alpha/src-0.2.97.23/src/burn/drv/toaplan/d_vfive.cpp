#include "toaplan.h"
#include "nec_intf.h"

// V-Five & Grind Stormer

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static INT32 v25_reset = 0;

// Rom information
static struct BurnRomInfo vfiveRomDesc[] = {
	{ "tp027_01.bin", 0x080000, 0x731D50F4, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "tp027_02.bin", 0x100000, 0x877B45E8, BRF_GRA },			 //  1 GP9001 Tile data
	{ "tp027_03.bin", 0x100000, 0xB1FC6362, BRF_GRA },			 //  2
};


STD_ROM_PICK(vfive)
STD_ROM_FN(vfive)

static struct BurnRomInfo grindstmRomDesc[] = {
	{ "01.bin",       0x080000, 0x4923F790, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "tp027_02.bin", 0x100000, 0x877B45E8, BRF_GRA },			 //  1 GP9001 Tile data
	{ "tp027_03.bin", 0x100000, 0xB1FC6362, BRF_GRA },			 //  2
};


STD_ROM_PICK(grindstm)
STD_ROM_FN(grindstm)

static struct BurnRomInfo grindstaRomDesc[] = {
	{ "tp027-01.rom", 0x080000, 0x8D8C0392, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "tp027_02.bin", 0x100000, 0x877B45E8, BRF_GRA },			 //  1 GP9001 Tile data
	{ "tp027_03.bin", 0x100000, 0xB1FC6362, BRF_GRA },			 //  2
};


STD_ROM_PICK(grindsta)
STD_ROM_FN(grindsta)

static struct BurnInputInfo vfiveInputList[] = {
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

STDINPUTINFO(vfive)

static struct BurnDIPInfo vfiveDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x01,	0x00, "Upright"},
	{0x14,	0x01, 0x01,	0x01, "Table type"},
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
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x82, 0x30,	0x10, "1 coin 2 plays"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x82, 0x30,	0x20, "2 coins 1 play"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x82, 0x30,	0x30, "2 coins 3 plays"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x82, 0xC0,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x82, 0xC0,	0x40, "1 coin 2 plays"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x82, 0xC0,	0x80, "2 coins 1 play"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x82, 0xC0,	0xC0, "2 coins 3 plays"},
	{0x16,	0x00, 0x0E, 0x08, NULL},

	// European coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x02, 0x30,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x02, 0x30,	0x10, "2 coins 1 play"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x02, 0x30,	0x20, "3 coins 1 play"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x02, 0x30,	0x30, "3 coins 1 play"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x02, 0xC0,	0x00, "1 coin 2 plays"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x02, 0xC0,	0x40, "1 coin 3 plays"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x02, 0xC0,	0x80, "1 coin 4 play"},
	{0x16,	0x00, 0x0E, 0x08, NULL},
	{0x14,	0x02, 0xC0,	0xC0, "1 coin 6 plays"},
	{0x16,	0x00, 0x0E, 0x08, NULL},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B (normal)"},
	{0x15,	0x01, 0x03,	0x01, "A (easy)"},
	{0x15,	0x01, 0x03,	0x02, "C (hard)"},
	{0x15,	0x01, 0x03,	0x03, "D (very hard)"},
	{0,		0xFE, 0,	4,	  "Extend"},
	{0x15,	0x01, 0x0C,	0x00, "300,000pts 800,000pts"},
	{0x15,	0x01, 0x0C,	0x04, "300,000 800,000 every"},
	{0x15,	0x01, 0x0C,	0x08, "200,000pts only"},
	{0x15,	0x01, 0x0C,	0x0C, "No extend"},
	{0,		0xFE, 0,	4,	  "Hero counts"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x01, "5"},
	{0x15,	0x01, 0x30,	0x02, "2"},
	{0x15,	0x01, 0x30,	0x03, "1"},
	{0,		0xFE, 0,	2,	  NULL},
    {0x15,	0x01, 0x40,	0x00, "Normal game"},
    {0x15,	0x01, 0x40,	0x40, "Debug mode"},
	{0,		0xFE, 0,	2,	  "Continue"},
    {0x15,	0x01, 0x80,	0x00, "On"},
	{0x15,	0x01, 0x80,	0x80, "Off"},
};

static struct BurnDIPInfo grindstmRegionDIPList[] = {

	{0x16,	0xFF, 0x0F,	0x00, NULL},
	{0,		0xFE, 0,	11,	  "Region"},
	{0x16,	0x01, 0x0F, 0x00, "Korea (Unite Trading)"},
	{0x16,	0x01, 0x0F, 0x01, "Korea"},
	{0x16,	0x01, 0x0F, 0x02, "Hong Kong (Charterfield)"},
	{0x16,	0x01, 0x0F, 0x03, "Hong Kong"},
	{0x16,	0x01, 0x0F, 0x04, "Taiwan (Anamoto International inc.)"},
	{0x16,	0x01, 0x0F, 0x05, "Taiwan"},
	{0x16,	0x01, 0x0F, 0x06, "South East Asia (Charterfield)"},
	{0x16,	0x01, 0x0F, 0x07, "South East Asia"},
	{0x16,	0x01, 0x0F, 0x08, "Europe"},
	{0x16,	0x01, 0x0F, 0x0A, "U.S.A (American Sammy corp.)"},
	{0x16,	0x01, 0x0F, 0x0B, "U.S.A"},
	{0x16,	0x01, 0x0F, 0x09, "Europe"},
	{0x16,	0x01, 0x0F, 0x0C, "U.S.A (American Sammy corp.)"},
	{0x16,	0x01, 0x0F, 0x0D, "U.S.A"},
	{0x16,	0x01, 0x0F, 0x0E, "Korea"},
	{0x16,	0x01, 0x0F, 0x0F, "Korea"},

};

STDDIPINFO(vfive)
STDDIPINFOEXT(grindstm, vfive, grindstmRegion)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal;
static UINT8 *ShareRAM;

static INT32 nColCount = 0x0800;

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x080000;		//
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	RamStart	= Next;
	Ram01		= Next; Next += 0x004000;		// CPU #0 work RAM
	ShareRAM	= Next; Next += 0x010000;
	RamPal		= Next; Next += 0x001000;		// palette
	GP9001RAM[0]= Next; Next += 0x004000;
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
		VezScan(nAction);
		BurnYM2151Scan(nAction);

		ToaScanGP9001(nAction, pnMin);
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01, 0, 1);

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 2, nGP9001ROMSize[0]);

	return 0;
}

UINT8 __fastcall vfiveReadByte(UINT32 sekAddress)
{
	if ((sekAddress & 0xff0000) == 0x210000) {
		return ShareRAM[(sekAddress / 2) & 0x7fff];
	}

	switch (sekAddress)
	{
		case 0x200011:								// Player 1 inputs
			return DrvInput[0];
		case 0x200015:								// Player 2 inputs
			return DrvInput[1];
		case 0x200019:								// Other inputs
			return DrvInput[2];

		case 0x30000D:								// VBlank
			return ToaVBlankRegister();

//		default:
//			printf("Attempt to read byte value of location %x\n", sekAddress);
	}

	return 0;
}

UINT16 __fastcall vfiveReadWord(UINT32 sekAddress)
{
	if ((sekAddress & 0xff0000) == 0x210000) {
		return ShareRAM[(sekAddress / 2) & 0x7fff];
	}

	switch (sekAddress)
	{
		case 0x200010:								// Player 1 inputs
			return DrvInput[0];
		case 0x200014:								// Player 2 inputs
			return DrvInput[1];
		case 0x200018:								// Other inputs
			return DrvInput[2];

		case 0x300004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x300006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x30000C:
			return ToaVBlankRegister();

		case 0x700000:
			return ToaScanlineRegister();

//		default:
//			printf("Attempt to read word value of location %x\n", sekAddress);
	}

	return 0;
}

void __fastcall vfiveWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
//printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);

	if ((sekAddress & 0xff0000) == 0x210000) {
		ShareRAM[(sekAddress / 2) & 0x7fff] = byteValue;
		return;
	}

	switch (sekAddress)
	{
		case 0x20001c:
		case 0x20001d:
			if (!v25_reset && (~byteValue & 0x10)) VezReset();
			v25_reset = (~byteValue & 0x10);
			break;

//		default:
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
	}
}

void __fastcall vfiveWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	if ((sekAddress & 0xff0000) == 0x210000) {
		ShareRAM[(sekAddress / 2) & 0x7fff] = wordValue;
		return;
	}

	switch (sekAddress)
	{
		case 0x300000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x300004:
			ToaGP9001WriteRAM(wordValue, 0);
			break;
		case 0x300006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x300008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x30000C:
			ToaGP9001WriteRegister(wordValue);
			break;

//		default:
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);
	}
}

void __fastcall vfive_v25_write(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x00000:
			BurnYM2151SelectRegister(data);
		return;

		case 0x00001:
			BurnYM2151WriteRegister(data);
		return;
	}
}

UINT8 __fastcall vfive_v25_read(UINT32 address)
{
	switch (address)
	{
		case 0x00001:
			return BurnYM2151ReadStatus();
	}

	return 0;
}

UINT8 __fastcall vfive_v25_read_port(UINT32 port)
{
	switch (port)
	{
		case V25_PORT_PT:
			return DrvInput[3]^0xff;

		case V25_PORT_P0:
			return DrvInput[4]^0xff;

		case V25_PORT_P1:
			return DrvInput[5]^0xff;
	}

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	VezOpen(0);
	VezReset();
	VezClose();

	BurnYM2151Reset();

	v25_reset = 1;

	return 0;
}

static UINT8 nitro_decryption_table[256] = {
	0x1b,0x56,0x75,0x88,0x8c,0x06,0x58,0x72, 0x83,0x86,0x36,0x1a,0x5f,0xd3,0x8c,0xe9, /* 00 */
	0x22,0x0f,0x03,0x2a,0xeb,0x2a,0xf9,0x0f, 0xa4,0xbd,0x75,0xf3,0x4f,0x53,0x8e,0xfe, /* 10 */
	0x87,0xe8,0xb1,0x8d,0x36,0xb5,0x43,0x73, 0x2a,0x5b,0xf9,0x02,0x24,0x8a,0x03,0x80, /* 20 */
	0x86,0x8b,0xd1,0x3e,0x8d,0x3e,0x58,0xfb, 0xc3,0x79,0xbd,0xb7,0x8a,0xe8,0x0f,0x81, /* 30 */
	0xb7,0xd0,0x8b,0xeb,0xff,0xb8,0x90,0x8b, 0x5e,0xa2,0x90,0x90,0xab,0xb4,0x80,0x59, /* 40 */
	0x87,0x72,0xb5,0xbd,0xb0,0x88,0x50,0x0f, 0xfe,0xd2,0xc3,0x90,0x8a,0x90,0xf9,0x75, /* 50 */
	0x1a,0xb3,0x74,0x0a,0x68,0x24,0xbb,0x90, 0x75,0x47,0xfe,0x2c,0xbe,0xc3,0x88,0xd2, /* 60 */
	0x3e,0xc1,0x8c,0x33,0x0f,0x90,0x8b,0x90, 0xb9,0x1e,0xff,0xa2,0x3e,0x22,0xbe,0x57, /* 70 */
	0x81,0x3a,0xf6,0x88,0xeb,0xb1,0x89,0x8a, 0x32,0x80,0x0f,0xb1,0x48,0xc3,0x68,0x72, /* 80 */
	0x53,0x02,0xc0,0x02,0xe8,0xb4,0x74,0xbc, 0x90,0x58,0x0a,0xf3,0x75,0xc6,0x90,0xe8, /* 90 */
	0x26,0x50,0xfc,0x8c,0x90,0xb1,0xc3,0xd1, 0xeb,0x83,0xa4,0xbf,0x26,0x4b,0x46,0xfe, /* a0 */
	0xe2,0x89,0xb3,0x88,0x03,0x56,0x0f,0x38, 0xbb,0x0c,0x90,0x0f,0x07,0x8a,0x8a,0x33, /* b0 */
	0xfe,0xf9,0xb1,0xa0,0x45,0x36,0x22,0x5e, 0x8a,0xbe,0xc6,0xea,0x3c,0xb2,0x1e,0xe8, /* c0 */
	0x90,0xeb,0x55,0xf6,0x8a,0xb0,0x5d,0xc0, 0xbb,0x8d,0xf6,0xd0,0xd1,0x88,0x4d,0x90, /* d0 */
	0x51,0x51,0x74,0xbd,0x32,0xd1,0x90,0xd2, 0x53,0xc7,0xab,0x36,0x50,0xe9,0x33,0xb3, /* e0 */
	0x2e,0x05,0x88,0x59,0x74,0x74,0x22,0x8e, 0x8a,0x8a,0x36,0x08,0x0f,0x45,0x90,0x2e, /* f0 */
};

static INT32 DrvInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	nGP9001ROMSize[0] = 0x200000;

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
		SekInit(0, 0x68000);									// Allocate 68000
		SekOpen(0);
		SekMapMemory(Rom01,		0x000000, 0x07FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,		0x100000, 0x103FFF, SM_RAM);
	//	SekMapMemory(ShareRAM,		0x210000, 0x21ffff, SM_RAM);
		SekMapMemory(RamPal,		0x400000, 0x400FFF, SM_RAM);	// Palette RAM
		SekSetReadWordHandler(0, vfiveReadWord);
		SekSetReadByteHandler(0, vfiveReadByte);
		SekSetWriteWordHandler(0, vfiveWriteWord);
		SekSetWriteByteHandler(0, vfiveWriteByte);
		SekClose();

		VezInit(0, V25_TYPE, 10000000 /*before divider*/);
		VezOpen(0);
		for (INT32 i = 0x80000; i < 0x100000; i += 0x8000) {
			VezMapArea(i, i + 0x7fff, 0, ShareRAM);
			VezMapArea(i, i + 0x7fff, 1, ShareRAM);
			VezMapArea(i, i + 0x7fff, 2, ShareRAM);
		}
		VezSetReadHandler(vfive_v25_read);
		VezSetWriteHandler(vfive_v25_write);
		VezSetReadPort(vfive_v25_read_port);
		VezSetDecode(nitro_decryption_table);
		VezClose();
	}

	BurnYM2151Init(3375000);
	BurnYM2151SetAllRoutes(1.00, BURN_SND_ROUTE_BOTH);

	nSpriteYOffset = 0x0001;

	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;

	ToaInitGP9001();

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	bDrawScreen = true;

	DrvDoReset();							// Reset machine
	return 0;
}

static INT32 DrvExit()
{
	ToaPalExit();

	ToaExitGP9001();
	BurnYM2151Exit();
	SekExit();								// Deallocate 68000s
	VezExit();

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
	INT32 nInterleave = 10;

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
	VezNewFrame();

	INT32 nSoundBufferPos = 0;

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)5000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesDone[0] = 0;
	nCyclesDone[1] = 0;
	
	SekOpen(0);

	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;

	VezOpen(0);

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
			SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

		// sound! (increase interleave?)
		if (v25_reset) {
			nCyclesDone[1] += nCyclesTotal[1] / nInterleave;
		} else {
			nCyclesDone[1] += VezRun(nCyclesTotal[1] / nInterleave);
		}
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
		}
	}

	VezClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();												// Draw screen if needed
	}

	return 0;
}

struct BurnDriver BurnDrvVFive = {
	"vfive", "grindstm", NULL, NULL, "1993",
	"V-Five (Japan)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_VERSHOOT, 0,
	NULL, vfiveRomInfo, vfiveRomName, NULL, NULL, vfiveInputInfo, vfiveDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvGrindStormer = {
	"grindstm", NULL, NULL, NULL, "1992",
	"Grind Stormer\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_VERSHOOT, 0,
	NULL, grindstmRomInfo, grindstmRomName, NULL, NULL, vfiveInputInfo, grindstmDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvGrindStormerA = {
	"grindstma", "grindstm", NULL, NULL, "1992",
	"Grind Stormer (older set)\0", NULL, "Toaplan GP9001 based", "Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_VERSHOOT, 0,
	NULL, grindstaRomInfo, grindstaRomName, NULL, NULL, vfiveInputInfo, grindstmDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

