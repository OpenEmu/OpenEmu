#include "toaplan.h"
// Shippu Mahou Daisakusen

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

// Rom information
static struct BurnRomInfo shippumdRomDesc[] = {
	{ "ma02rom1.bin", 0x080000, 0xA678B149, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "ma02rom0.bin", 0x080000, 0xF226A212, BRF_ESS | BRF_PRG }, //  1				(odd)

	{ "ma02rom3.bin", 0x200000, 0x0E797142, BRF_GRA },			 //  2 GP9001 Tile data
	{ "ma02rom4.bin", 0x200000, 0x72A6FA53, BRF_GRA },			 //  3

	{ "ma02rom5.bin", 0x008000, 0x116AE559, BRF_GRA },			 //  4 Extra text layer tile data

	{ "ma02rom2.bin", 0x010000, 0xDDE8A57E, BRF_ESS | BRF_PRG }, //  5 Z80 program

	{ "ma02rom6.bin", 0x080000, 0x199E7CAE, BRF_SND },			 //  6 MSM6295 ADPCM data
};


STD_ROM_PICK(shippumd)
STD_ROM_FN(shippumd)

static struct BurnRomInfo kingdmgpRomDesc[] = {
	{ "ma02rom1.bin", 0x080000, 0xA678B149, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "ma02rom0.bin", 0x080000, 0xF226A212, BRF_ESS | BRF_PRG }, //  1				(odd)

	{ "ma02rom3.bin", 0x200000, 0x0E797142, BRF_GRA },			 //  2 GP9001 Tile data
	{ "ma02rom4.bin", 0x200000, 0x72A6FA53, BRF_GRA },			 //  3

	{ "ma02rom5.eng", 0x008000, 0x8C28460B, BRF_GRA },			 //  4 Extra text layer tile data

	{ "ma02rom2.bin", 0x010000, 0xDDE8A57E, BRF_ESS | BRF_PRG }, //  5 Z80 program

	{ "ma02rom6.bin", 0x080000, 0x199E7CAE, BRF_SND },			 //  6 MSM6295 ADPCM data
};


STD_ROM_PICK(kingdmgp)
STD_ROM_FN(kingdmgp)

static struct BurnInputInfo shippumdInputList[] = {
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

STDINPUTINFO(shippumd)

static struct BurnDIPInfo shippumdDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x01,	0x00, "Coin play"},
	{0x14,	0x01, 0x01,	0x01, "Free play"},
	{0,		0xFE, 0,	2,	  "Screen"},
	{0x14,	0x01, 0x02,	0x00, "Normal"},
	{0x14,	0x01, 0x02,	0x02, "Invert"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "1 coin 2 plays"},
	{0x14,	0x01, 0x30,	0x20, "2 coins 1 play"},
	{0x14,	0x01, 0x30,	0x30, "2 coins 3 plays"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 2 plays"},
	{0x14,	0x01, 0xC0,	0x80, "2 coins 1 play"},
	{0x14,	0x01, 0xC0,	0xC0, "2 coins 3 plays"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B (normal)"},
	{0x15,	0x01, 0x03,	0x01, "A (easy)"},
	{0x15,	0x01, 0x03,	0x02, "C (hard)"},
	{0x15,	0x01, 0x03,	0x03, "D (more hard)"},
	{0,		0xFE, 0,	4,	  "Extend bonus"},
	{0x15,	0x01, 0x0C,	0x00, "300000 pts every"},
	{0x15,	0x01, 0x0C,	0x04, "200000 & 500000 pts"},
	{0x15,	0x01, 0x0C,	0x08, "200000 only"},
	{0x15,	0x01, 0x0C,	0x0C, "No extend"},
	{0,		0xFE, 0,	4,	  "Number of heroes"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x10, "5"},
	{0x15,	0x01, 0x30,	0x20, "2"},
	{0x15,	0x01, 0x30,	0x30, "1"},
	{0,		0xFE, 0,	2,	  "No death & stop mode"},
    {0x15,	0x01, 0x40,	0x00, "Off"},
    {0x15,	0x01, 0x40,	0x40, "On"},
	{0,		0xFE, 0,	2,	  NULL},
    {0x15,	0x01, 0x80,	0x00, "Continue play"},
	{0x15,	0x01, 0x80,	0x80, "Continue impossible"},
};

static struct BurnDIPInfo shippumdRegionDIPList[] = {
	// Defaults
	{0x16,	0xFF, 0x0E,	0x00, NULL},

	// Region
	{0,		0xFE, 0,	1,	  "Region"},
    {0x16,	0x01, 0x0E,	0x00, "Japan"},
    {0x16,	0x01, 0x0E,	0x02, "U.S.A."},
    {0x16,	0x01, 0x0E,	0x04, "Europe"},
    {0x16,	0x01, 0x0E,	0x06, "South East Asia"},
    {0x16,	0x01, 0x0E,	0x08, "China"},
    {0x16,	0x01, 0x0E,	0x0A, "Korea"},
    {0x16,	0x01, 0x0E,	0x0C, "Hong Kong"},
    {0x16,	0x01, 0x0E,	0x0E, "Taiwan"},
};

static struct BurnDIPInfo kingdmgpRegionDIPList[] = {
	// Defaults
	{0x16,	0xFF, 0x0E,	0x04, NULL},

	// Region
	{0,		0xFE, 0,	7,	  "Region"},
    {0x16,	0x01, 0x0E,	0x02, "U.S.A."},
    {0x16,	0x01, 0x0E,	0x04, "Europe"},
    {0x16,	0x01, 0x0E,	0x06, "South East Asia"},
    {0x16,	0x01, 0x0E,	0x08, "China"},
    {0x16,	0x01, 0x0E,	0x0A, "Korea"},
    {0x16,	0x01, 0x0E,	0x0C, "Hong Kong"},
    {0x16,	0x01, 0x0E,	0x0E, "Taiwan"},
    {0x16,	0x01, 0x0E,	0x00, "Japan"},
};

STDDIPINFOEXT(shippumd, shippumd, shippumdRegion)
STDDIPINFOEXT(kingdmgp, shippumd, kingdmgpRegion)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *Ram02, *RamPal;

static INT32 nColCount = 0x0800;

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x100000;		//
	RomZ80		= Next; Next += 0x010000;		// Z80 ROM
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	ExtraTROM	= Next; Next += 0x008000;		// Extra Text layer tile data
	MSM6295ROM	= Next; Next += 0x080000;
	RamStart	= Next;
	Ram01		= Next; Next += 0x010000;		// CPU #0 work RAM
	Ram02		= Next; Next += 0x000800;		//
	ExtraTRAM	= Next; Next += 0x002000;		// Extra tile layer
	ExtraTScroll= Next; Next += 0x001000;		//
	ExtraTSelect= Next; Next += 0x001000;		//
	RamPal		= Next; Next += 0x001000;		// palette
	RamZ80		= Next; Next += 0x004000;		// Z80 RAM
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
		*pnMin = 0x029497;
	}
	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd-RamStart;
		ba.szName	= "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);				// Scan 68000
		ZetScan(nAction);				// Scan Z80

		MSM6295Scan(0, nAction);
		BurnYM2151Scan(nAction);

		ToaScanGP9001(nAction, pnMin);

		SCAN_VAR(DrvInput);
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	if (ToaLoadCode(Rom01, 0, 2)) {
		return 1;
	}

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 2, 2, nGP9001ROMSize[0]);

	// Load Extra text layer tile data
	BurnLoadRom(ExtraTROM, 4, 1);

	// Load Z80 ROM
	BurnLoadRom(RomZ80, 5, 1);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM, 6, 1);

	return 0;
}

UINT8 __fastcall shippumdZ80Read(UINT16 nAddress)
{
	if (nAddress == 0xE001) {
		return BurnYM2151ReadStatus();
	}
	if (nAddress == 0xE004) {
		return MSM6295ReadStatus(0);
	}
	return 0;
}

void __fastcall shippumdZ80Write(UINT16 nAddress, UINT8 nValue)
{
	switch (nAddress) {
		case 0xE000:
			BurnYM2151SelectRegister(nValue);
			break;
		case 0xE001:
			BurnYM2151WriteRegister(nValue);
			break;
		case 0xE004:
			MSM6295Command(0, nValue);
			break;
	}
}

static INT32 DrvZ80Init()
{
	// Init the Z80
	ZetInit(0);
	ZetOpen(0);

	ZetSetReadHandler(shippumdZ80Read);
	ZetSetWriteHandler(shippumdZ80Write);

	// ROM
	ZetMapArea    (0x0000, 0xFFFF, 0, RomZ80);	// Direct Read from ROM
	ZetMapArea    (0x0000, 0xFFFF, 2, RomZ80);	// Direct Fetch from ROM
	// RAM
	ZetMapArea    (0xC000, 0xDFFF, 0, RamZ80);	// Direct Read from RAM
	ZetMapArea    (0xC000, 0xDFFF, 1, RamZ80);	// Direct Write to RAM
	ZetMapArea    (0xC000, 0xDFFF, 2, RamZ80);	//
	// Callbacks
	ZetMemCallback(0xE000, 0xE0FF, 0);			// Read
	ZetMemCallback(0xE000, 0xE0FF, 1);			// Write

	ZetMemEnd();
	ZetClose();

	return 0;
}

UINT8 __fastcall shippumdReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x21C021:								// Player 1 inputs
			return DrvInput[0];
		case 0x21C025:								// Player 2 inputs
			return DrvInput[1];
		case 0x21C029:								// Other inputs
			return DrvInput[2];
		case 0x21C02D:								// Dipswitch A
			return DrvInput[3];
		case 0x21C031:			   					// Dipswitch B
			return DrvInput[4];
		case 0x21C035:								// Dipswitch C - Territory
			return DrvInput[5];

		case 0x30000D:
			return ToaVBlankRegister();

		default: {
//			printf("Attempt to read byte value of location %x\n", sekAddress);

			if ((sekAddress & 0x00FFC000) == 0x00218000) {
				return RamZ80[(sekAddress & 0x3FFF) >> 1];
			}
		}
	}
	return 0;
}

UINT16 __fastcall shippumdReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x21C020:								// Player 1 inputs
			return DrvInput[0];
		case 0x21C024:								// Player 2 inputs
			return DrvInput[1];
		case 0x21C028:								// Other inputs
			return DrvInput[2];
		case 0x21C02C:								// Dipswitch A
			return DrvInput[3];
		case 0x21C030:								// Dipswitch B
			return DrvInput[4];
		case 0x21C034:								// Dipswitch C - Territory
			return DrvInput[5];

		case 0x21C03C:
			return ToaScanlineRegister();

		case 0x300004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x300006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x30000C:
			return ToaVBlankRegister();

		default: {
			if ((sekAddress & 0x00FFC000) == 0x00218000) {
				return RamZ80[(sekAddress & 0x3FFF) >> 1];
			}

//			printf("Attempt to read word value of location %x\n", sekAddress);
		}
	}
	return 0;
}

void __fastcall shippumdWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x21C01D: {
			INT32 nBankOffset;
			if (byteValue & 0x10) {
				nBankOffset = 0x40000;
			} else {
				nBankOffset = 0x00000;
			}
			MSM6295SampleInfo[0][0] = MSM6295ROM + nBankOffset;
			MSM6295SampleData[0][0] = MSM6295ROM + nBankOffset;
			MSM6295SampleInfo[0][1] = MSM6295ROM + nBankOffset + 0x0100;
			MSM6295SampleData[0][1] = MSM6295ROM + nBankOffset + 0x10000;
			MSM6295SampleInfo[0][2] = MSM6295ROM + nBankOffset + 0x0200;
			MSM6295SampleData[0][2] = MSM6295ROM + nBankOffset + 0x20000;
			MSM6295SampleInfo[0][3] = MSM6295ROM + nBankOffset + 0x0300;
			MSM6295SampleData[0][3] = MSM6295ROM + nBankOffset + 0x30000;
			break;
		}

		default: {
			if ((sekAddress & 0x00FFC000) == 0x00218000) {
				RamZ80[(sekAddress & 0x3FFF) >> 1] = byteValue;
			}

//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
		}
	}
}

void __fastcall shippumdWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x21C01C: {
			INT32 nBankOffset;
			if (wordValue & 0x10) {
				nBankOffset = 0x40000;
			} else {
				nBankOffset = 0x00000;
			}
			MSM6295SampleInfo[0][0] = MSM6295ROM + nBankOffset;
			MSM6295SampleData[0][0] = MSM6295ROM + nBankOffset;
			MSM6295SampleInfo[0][1] = MSM6295ROM + nBankOffset + 0x0100;
			MSM6295SampleData[0][1] = MSM6295ROM + nBankOffset + 0x10000;
			MSM6295SampleInfo[0][2] = MSM6295ROM + nBankOffset + 0x0200;
			MSM6295SampleData[0][2] = MSM6295ROM + nBankOffset + 0x20000;
			MSM6295SampleInfo[0][3] = MSM6295ROM + nBankOffset + 0x0300;
			MSM6295SampleData[0][3] = MSM6295ROM + nBankOffset + 0x30000;
			break;
		}

		case 0x300000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x300004:
		case 0x300006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x300008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x30000C:
			ToaGP9001WriteRegister(wordValue);
			break;

		default: {
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);

			if ((sekAddress & 0x00FFC000) == 0x00218000) {
				RamZ80[(sekAddress & 0x3FFF) >> 1] = wordValue;
			}
		}
	}
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	BurnYM2151Reset();

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	nGP9001ROMSize[0] = 0x400000;

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
		SekMapMemory(Rom01,			0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,			0x100000, 0x10FFFF, SM_RAM);
		SekMapMemory(RamPal,		0x400000, 0x400FFF, SM_RAM);	// Palette RAM
		SekMapMemory(Ram02,			0x401000, 0x4017FF, SM_RAM);	// Unused
		SekMapMemory(ExtraTRAM,		0x500000, 0x502FFF, SM_RAM);
		SekMapMemory(ExtraTSelect,	0x502000, 0x502FFF, SM_RAM);	// 0x502000 - Scroll; 0x502200 - RAM
		SekMapMemory(ExtraTScroll,	0x503000, 0x503FFF, SM_RAM);	// 0x203000 - Offset; 0x503200 - RAM

		SekSetReadWordHandler(0, shippumdReadWord);
		SekSetReadByteHandler(0, shippumdReadByte);
		SekSetWriteWordHandler(0, shippumdWriteWord);
		SekSetWriteByteHandler(0, shippumdWriteByte);

		SekClose();
	}

 	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;
	ToaInitGP9001();

	nExtraTXOffset = 0x2C;
	ToaExtraTextInit();

	DrvZ80Init();				// Initialize Z80

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	BurnYM2151Init(27000000 / 8);
	BurnYM2151SetAllRoutes(1.00, BURN_SND_ROUTE_BOTH);
	MSM6295Init(0, 32000000 / 32 / 132, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	bDrawScreen = true;

	DrvDoReset();				// Reset machine
	return 0;
}

static INT32 DrvExit()
{
	MSM6295Exit(0);
	BurnYM2151Exit();

	ToaPalExit();

	ToaExitGP9001();
	ToaExtraTextExit();
	ToaZExit();				// Z80 exit
	SekExit();				// Deallocate 68000s

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDraw()
{
	ToaClearScreen(0);

	if (bDrawScreen) {
		ToaGetBitmap();
		ToaRenderGP9001();					// Render GP9001 graphics
		ToaExtraTextLayer();				// Render extra text layer
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

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = TOA_Z80_SPEED / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	SekOpen(0);

	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;

	INT32 nSoundBufferPos = 0;

	ZetOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
    	INT32 nCurrentCPU;
		INT32 nNext;

		// Run 68000

		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;

		// Trigger VBlank interrupt
		if (nNext > nToaCyclesVBlankStart) {
			if (nCyclesDone[nCurrentCPU] < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - nCyclesDone[nCurrentCPU];
				nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
			}

			ToaBufferGP9001Sprites();

			bVBlank = true;
			SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

		// Run Z80
		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);

		{
			// Render sound segment
			if (pBurnSoundOut) {
				INT32 nSegmentLength = nBurnSoundLen / nInterleave;
				INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				MSM6295Render(0, pSoundBuf, nSegmentLength);
				nSoundBufferPos += nSegmentLength;
			}
		}
	}

	{
		// Make sure the buffer is entirely filled.
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (nSegmentLength) {
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				MSM6295Render(0, pSoundBuf, nSegmentLength);
			}
		}
	}
	
	SekClose();
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();														// Draw screen if needed
	}

	return 0;
}

struct BurnDriver BurnDrvShippuMD = {
	"shippumd", "kingdmgp", NULL, NULL, "1994",
	"Shippu Mahou Daisakusen - kingdom grandprix\0", NULL, "Raizing / 8ing", "Toaplan GP9001 based",
	L"\u75BE\u98A8\u9B54\u6CD5\u5927\u4F5C\u6226 - kingdom grandprix (Japan)\0Shippu Mahou Daisakusen (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, shippumdRomInfo, shippumdRomName, NULL, NULL, shippumdInputInfo, shippumdDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvKingdmGP = {
	"kingdmgp", NULL, NULL, NULL, "1994",
	"Kingdom Grandprix (World)\0", NULL, "Raizing / 8ing", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, kingdmgpRomInfo, kingdmgpRomName, NULL, NULL, shippumdInputInfo, kingdmgpDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};
