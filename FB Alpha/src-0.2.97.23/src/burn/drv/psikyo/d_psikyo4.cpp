// FB Alpha Psikyo PS4 harware driver module
// Based on MAME driver by David Haywood and Paul Priest

#include "tiles_generic.h"
#include "burn_ymf278b.h"
#include "eeprom.h"
#include "sh2.h"

static UINT8 *AllMem;
static UINT8 *DrvSh2ROM;
static UINT8 *DrvGfxROM;
static UINT8 *DrvSndROM;
static UINT8 *DrvSndBanks;
static UINT8 *AllRam;
static UINT8 *DrvSh2RAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRegs;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT32  *DrvPalette;

static UINT16 *pTempDraw;

static UINT8 *DrvBrightVal;
static UINT16 *tile_bank;
static UINT8 *ioselect;

static INT32 sample_offs;
static UINT32  pcmbank_previous;

static UINT8 DrvJoy1[32];
static UINT8 DrvJoy2[32];
static UINT8 DrvMah1[8];
static UINT8 DrvMah2[8];
static UINT8 DrvMah3[8];
static UINT8 DrvMah4[8];
static UINT8 DrvMah5[8];
static UINT8 DrvMah6[8];
static UINT8 DrvMah7[8];
static UINT8 DrvMah8[8];
static UINT8 DrvMah9[8];
static UINT8 DrvMahs[10];
static UINT32 DrvInputs[2];
static UINT8 DrvDips[2];
static UINT8 DrvReset;

static INT32 mahjong = 0;
static INT32 nGfxMask;

static UINT32 speedhack_address = ~0;
static UINT32 speedhack_pc[4] = { 0, 0, 0, 0 };

static struct BurnInputInfo LoderndfInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 31,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 24,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 25,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 26,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 27,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 28,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 29,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 30,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 23,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 16,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 17,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 18,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 19,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 20,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 21,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 22,	"p2 fire 3"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy1 + 2,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy2 + 31,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy2 + 24,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy2 + 25,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy2 + 26,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy2 + 27,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy2 + 28,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy2 + 29,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy2 + 30,	"p3 fire 3"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy1 + 3,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	DrvJoy2 + 23,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy2 + 16,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy2 + 17,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy2 + 18,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy2 + 19,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy2 + 20,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy2 + 21,	"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	DrvJoy2 + 22,	"p4 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 4,	"service"	},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Loderndf)

static struct BurnInputInfo HotdebutInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 31,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 24,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 25,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 26,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 27,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 23,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 16,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 17,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 18,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy1 + 19,	"p2 fire 4"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy1 + 2,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy2 + 31,	"p3 start"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy2 + 24,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy2 + 25,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy2 + 26,	"p3 fire 3"	},
	{"P3 Button 4",		BIT_DIGITAL,	DrvJoy2 + 27,	"p3 fire 4"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy1 + 3,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	DrvJoy2 + 23,	"p4 start"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy2 + 16,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy2 + 17,	"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	DrvJoy2 + 18,	"p4 fire 3"	},
	{"P4 Button 4",		BIT_DIGITAL,	DrvJoy2 + 19,	"p4 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 4,	"service"	},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Hotdebut)

static struct BurnInputInfo HotgmckInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvMah1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvMah2 + 5,	"p1 start"	},
	{"A",			BIT_DIGITAL,	DrvMah2 + 0,	"mah a"		},
	{"B",			BIT_DIGITAL,	DrvMah3 + 0,	"mah b"		},
	{"C",			BIT_DIGITAL,	DrvMah4 + 0,	"mah c"		},
	{"D",			BIT_DIGITAL,	DrvMah5 + 0,	"mah d"		},
	{"E",			BIT_DIGITAL,	DrvMah2 + 1,	"mah e"		},
	{"F",			BIT_DIGITAL,	DrvMah3 + 1,	"mah f"		},
	{"G",			BIT_DIGITAL,	DrvMah4 + 1,	"mah g"		},
	{"H",			BIT_DIGITAL,	DrvMah5 + 1,	"mah h"		},
	{"I",			BIT_DIGITAL,	DrvMah2 + 2,	"mah i"		},
	{"J",			BIT_DIGITAL,	DrvMah3 + 2,	"mah j"		},
	{"K",			BIT_DIGITAL,	DrvMah2 + 4,	"mah k"		},
	{"L",			BIT_DIGITAL,	DrvMah5 + 2,	"mah l"		},
	{"M",			BIT_DIGITAL,	DrvMah2 + 3,	"mah m"		},
	{"N",			BIT_DIGITAL,	DrvMah3 + 3,	"mah n"		},
	{"Pon",			BIT_DIGITAL,	DrvMah5 + 3,	"mah pon"	},
	{"Chi",			BIT_DIGITAL,	DrvMah4 + 3,	"mah chi"	},
	{"Kan",			BIT_DIGITAL,	DrvMah2 + 4,	"mah kan"	},
	{"Ron",			BIT_DIGITAL,	DrvMah4 + 4,	"mah ron"	},
	{"Reach",		BIT_DIGITAL,	DrvMah3 + 4,	"mah reach"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvMah1 + 2,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvMah6 + 5,	"p2 start"	},
	{"A",			BIT_DIGITAL,	DrvMah6 + 0,	"mah a"		},
	{"B",			BIT_DIGITAL,	DrvMah7 + 0,	"mah b"		},
	{"C",			BIT_DIGITAL,	DrvMah8 + 0,	"mah c"		},
	{"D",			BIT_DIGITAL,	DrvMah9 + 0,	"mah d"		},
	{"E",			BIT_DIGITAL,	DrvMah6 + 1,	"mah e"		},
	{"F",			BIT_DIGITAL,	DrvMah7 + 1,	"mah f"		},
	{"G",			BIT_DIGITAL,	DrvMah8 + 1,	"mah g"		},
	{"H",			BIT_DIGITAL,	DrvMah9 + 1,	"mah h"		},
	{"I",			BIT_DIGITAL,	DrvMah6 + 2,	"mah i"		},
	{"J",			BIT_DIGITAL,	DrvMah7 + 2,	"mah j"		},
	{"K",			BIT_DIGITAL,	DrvMah6 + 4,	"mah k"		},
	{"L",			BIT_DIGITAL,	DrvMah9 + 2,	"mah l"		},
	{"M",			BIT_DIGITAL,	DrvMah6 + 3,	"mah m"		},
	{"N",			BIT_DIGITAL,	DrvMah7 + 3,	"mah n"		},
	{"Pon",			BIT_DIGITAL,	DrvMah9 + 3,	"mah pon"	},
	{"Chi",			BIT_DIGITAL,	DrvMah8 + 3,	"mah chi"	},
	{"Kan",			BIT_DIGITAL,	DrvMah6 + 4,	"mah kan"	},
	{"Ron",			BIT_DIGITAL,	DrvMah8 + 4,	"mah ron"	},
	{"Reach",		BIT_DIGITAL,	DrvMah7 + 4,	"mah reach"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvMah1 + 4,	"service"	},
	{"Service",		BIT_DIGITAL,	DrvMah1 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Hotgmck)

static struct BurnDIPInfo LoderndfDIPList[]=
{
	{0x27, 0xff, 0xff, 0x60, NULL					},
	{0x28, 0xff, 0xff, 0x01, NULL					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x27, 0x01, 0x20, 0x20, "Off"					},
	{0x27, 0x01, 0x20, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Debug"				},
	{0x27, 0x01, 0x40, 0x40, "Off"					},
	{0x27, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Region"				},
	{0x28, 0x01, 0x03, 0x00, "Japan (Shows Version Number)"		},
	{0x28, 0x01, 0x03, 0x01, "World (Does Not Show Version Number)"	},
};

STDDIPINFO(Loderndf)

static struct BurnDIPInfo HotdebutDIPList[]=
{
	{0x1b, 0xff, 0xff, 0x60, NULL		},

	{0   , 0xfe, 0   ,    2, "Service Mode"	},
	{0x1b, 0x01, 0x20, 0x20, "Off"		},
	{0x1b, 0x01, 0x20, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Debug"	},
	{0x1b, 0x01, 0x40, 0x40, "Off"		},
	{0x1b, 0x01, 0x40, 0x00, "On"		},
};

STDDIPINFO(Hotdebut)

static struct BurnDIPInfo HotgmckDIPList[]=
{
	{0x2d, 0xff, 0xff, 0x60, NULL		},

	{0   , 0xfe, 0   ,    2, "Service Mode"	},
	{0x2d, 0x01, 0x20, 0x20, "Off"		},
	{0x2d, 0x01, 0x20, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Debug"	},
	{0x2d, 0x01, 0x40, 0x40, "Off"		},
	{0x2d, 0x01, 0x40, 0x00, "On"		},
};

STDDIPINFO(Hotgmck)

#ifndef LSB_FIRST
static void le_to_be(unsigned char * p, int size)
{
        unsigned char c;

        for(int i=0; i<size; i+=4, p+=4) {
                c = *(p+0);     *(p+0) = *(p+3);        *(p+3) = c;
                c = *(p+1);     *(p+1) = *(p+2);        *(p+2) = c;
        }
}
#endif

static void set_pcm_bank()
{
	if (mahjong) {
		if (pcmbank_previous != ((UINT32)ioselect[0] & 0x77)) {
			INT32 bank0 = ((ioselect[0] >> 0) & 7) << 20;
			INT32 bank1 = ((ioselect[0] >> 4) & 7) << 20;
	
			pcmbank_previous = ioselect[0] & 0x77;
			memcpy (DrvSndROM + 0x200000, DrvSndBanks + bank0, 0x100000);
			memcpy (DrvSndROM + 0x300000, DrvSndBanks + bank1, 0x100000);
		}
	}
}

static UINT32 psikyo4_read_inputs(INT32 bsel)
{
	if (mahjong) {
		UINT32 ret = 0xff;

		if (ioselect[2] & 1) ret &= bsel ? DrvMahs[5] : DrvMahs[1];
		if (ioselect[2] & 2) ret &= bsel ? DrvMahs[6] : DrvMahs[2];
		if (ioselect[2] & 4) ret &= bsel ? DrvMahs[7] : DrvMahs[3];
		if (ioselect[2] & 8) ret &= bsel ? DrvMahs[8] : DrvMahs[4];

		return (ret << 24) | 0x00ffff00 | DrvMahs[0];
	} else {
		return DrvInputs[bsel];
	}

	return 0;
}

UINT32 __fastcall ps4_read_long(UINT32 address)
{
	address &= 0xc7fffffc;

	if (address >= 0x03000000 && address <= 0x030037ff) {
		return *((UINT32 *)(DrvSprRAM + (address & 0x3ffc)));
	}

	if ((address & 0xffffe000) == 0x03004000) {
		return *((UINT32 *)(DrvPalRAM + (address & 0x1ffc)));
	}

	if ((address & 0xffffe000) == 0x03006000) {
		INT32 bank  = (tile_bank[0] & 0x1fff) << 13;
		    bank |= address & 0x1ffc;
		if (bank >= ((nGfxMask + 1) << 8)) return 0;

		return *((UINT32 *)(DrvGfxROM + bank));
	}

	return 0;
}

void __fastcall ps4_write_long(UINT32 address, UINT32 data)
{
	address &= 0xc7fffffc;

	if (address >= 0x03000000 && address <= 0x030037ff) {
		*((UINT32 *)(DrvSprRAM + (address & 0x3ffc))) = data;
		return;
	}

	if ((address & 0xffffe000) == 0x03004000) {
		*((UINT32 *)(DrvPalRAM + (address & 0x1ffc))) = data;
		return;
	}

	if (address >= 0x03003fe4 && address <= 0x03003fef) {
		address -= 0x3003fe4;
		*((UINT32 *)(DrvVidRegs + address)) = data;
		return;
	}
}

UINT16 __fastcall ps4_read_word(UINT32 address)
{
	address &= 0xc7fffffe;
#ifdef LSB_FIRST
	address ^= 2;
#endif
	if (address >= 0x03000000 && address <= 0x030037ff) {
		return *((UINT16 *)(DrvSprRAM + (address & 0x3ffe)));
	}

	if ((address & 0xffffe000) == 0x03004000) {
		return *((UINT16 *)(DrvPalRAM + (address & 0x1ffe)));
	}

	switch (address)
	{
		case 0x03003fe2:
			return DrvDips[1] | ((EEPROMRead() & 1) << 4);

		case 0x05800000: // used?
			return psikyo4_read_inputs(0);

		case 0x05800002:
			return psikyo4_read_inputs(0) >> 16;

		case 0x05800004: // used?
			return psikyo4_read_inputs(1);

		case 0x05800006:
			return psikyo4_read_inputs(1) >> 16;
	}

	return 0;
}

UINT8 __fastcall ps4_read_byte(UINT32 address)
{
	address &= 0xc7ffffff;


	if (address >= 0x03000000 && address <= 0x030037ff) {
#ifdef LSB_FIRST
		address ^= 3;
#endif
		return DrvSprRAM[address & 0x3fff];
	}

	if ((address & 0xffffe000) == 0x03004000) {
#ifdef LSB_FIRST
		address ^= 3;
#endif
		return DrvPalRAM[address & 0x1fff];
	}

	switch (address)
	{
		case 0x03003fe1:
			return DrvDips[1] | ((EEPROMRead() & 1) << 4);
		
		case 0x05000000:
			return BurnYMF278BReadStatus();

		case 0x05000005:
			return DrvSndROM[sample_offs++ & 0x3fffff];

		case 0x05800000:
		case 0x05800001:
		case 0x05800002:
		case 0x05800003:
			return psikyo4_read_inputs(0) >> ((~address & 3) << 3);

		case 0x05800004:
		case 0x05800005:
		case 0x05800006:
		case 0x05800007:
			return psikyo4_read_inputs(1) >> ((~address & 3) << 3);
	}

	return 0;
}

void __fastcall ps4_write_word(UINT32 address, UINT16 data)
{
	address &= 0xc7fffffe;
#ifdef LSB_FIRST
	address ^= 2;
#endif

	if (address >= 0x03000000 && address <= 0x030037ff) {
		*((UINT16 *)(DrvSprRAM + (address & 0x3ffe))) = data;
		return;
	}

	if ((address & 0xffffe000) == 0x03004000) {
		*((UINT16 *)(DrvPalRAM + (address & 0x1ffe))) = data;
		return;
	}

	if (address >= 0x03003fe4 && address <= 0x03003fef) {
		address -= 0x03003fe4;
		*((UINT16 *)(DrvVidRegs + address)) = data;
		return;
	}

	if (address == 0x3003fe2) {
		EEPROMWrite((data & 0x40), (data & 0x80), (data & 0x20));
		return;
	}
}

void __fastcall ps4_write_byte(UINT32 address, UINT8 data)
{
	address &= 0xc7ffffff;

//	bprintf (0, _T("%8.8x, wb\n"), address);

	if (address >= 0x03000000 && address <= 0x030037ff) {
#ifdef LSB_FIRST
		DrvSprRAM[(address ^ 3) & 0x3fff] = data;
#else
		DrvSprRAM[(address) & 0x3fff] = data;
#endif
		return;
	}

	if ((address & 0xffffe000) == 0x03004000) {
#ifdef LSB_FIRST
		DrvPalRAM[(address ^ 3) & 0x1fff] = data;
#else
		DrvPalRAM[(address) & 0x1fff] = data;
#endif
		return;
	}

	if (address >= 0x03003fe4 && address <= 0x03003fef) {
#ifdef LSB_FIRST
		DrvVidRegs[(address ^ 3) - 0x03003fe4] = data;
#else
		DrvVidRegs[(address) - 0x03003fe4] = data;
#endif
		return;
	}

	switch (address)
	{
		case 0x03003fe1:
	//		EEPROMWrite((data & 0x40), (data & 0x80), (data & 0x20));
		return;

		case 0x03003ff3:
			DrvBrightVal[0] = data;
			if (DrvBrightVal[0] > 0x7f) DrvBrightVal[0] = 0x7f;
			DrvBrightVal[0] ^= 0x7f;
		return;

		case 0x03003ff4:
		case 0x03003ff5:
		case 0x03003ff6:
		case 0x03003ff7:
			DrvPalRAM[0x2000 + (~address & 3)] = data;
		return;

		case 0x03003ffb:
			DrvBrightVal[1] = data;
			if (DrvBrightVal[1] > 0x7f) DrvBrightVal[1] = 0x7f;
			DrvBrightVal[1] ^= 0x7f;
		return;

		case 0x03003ffc:
		case 0x03003ffd:
		case 0x03003ffe:
		case 0x03003fff:
			DrvPalRAM[0x2004 + (~address & 3)] = data;
		return;

		case 0x05000000:
		case 0x05000002:
		case 0x05000004:
		case 0x05000006:
			BurnYMF278BSelectRegister((address >> 1) & 3, data);
		return;

		case 0x05000001:
		case 0x05000003:
		case 0x05000005:
		case 0x05000007:
			BurnYMF278BWriteRegister((address >> 1) & 3, data);
		return;

		case 0x05800008:
			set_pcm_bank();
		case 0x05800009:
		case 0x0580000a:
		case 0x0580000b:
			ioselect[address & 3] = data;
		return;
	}
}

UINT32 __fastcall ps4hack_read_long(UINT32 a)
{
	a &= 0xffffc;

	if (a == speedhack_address) {
		UINT32 pc = Sh2GetPC(0);

		if (pc == speedhack_pc[0]) {
			Sh2StopRun();
		} else if (pc == speedhack_pc[1]) {
			Sh2StopRun();
		}
	}

	return *((UINT32*)(DrvSh2RAM + a));
}

UINT16 __fastcall ps4hack_read_word(UINT32 a)
{
#ifdef LSB_FIRST
	return *((UINT16 *)(DrvSh2RAM + ((a & 0xffffe) ^ 2)));
#else
	return *((UINT16 *)(DrvSh2RAM + ((a & 0xffffe))));
#endif
}

UINT8 __fastcall ps4hack_read_byte(UINT32 a)
{
#ifdef LSB_FIRST
	return DrvSh2RAM[(a & 0xfffff) ^ 3];
#else
	return DrvSh2RAM[(a & 0xfffff)];
#endif
}

static INT32 MemIndex(INT32 gfx_len)
{
	UINT8 *Next; Next = AllMem;

	DrvSh2ROM		= Next; Next += 0x0300000;

	DrvGfxROM		= Next; Next += gfx_len;

	DrvSndROM		= Next; Next += 0x0400000;

	if (mahjong) {
		DrvSndBanks	= Next; Next += 0x0800000;
	}

	AllRam			= Next;

	DrvVidRegs		= Next; Next += 0x000000c;

	DrvSh2RAM		= Next; Next += 0x0100000;
	DrvSprRAM		= Next; Next += 0x0003800;
	DrvPalRAM		= Next; Next += 0x0002008;

	DrvBrightVal		= Next; Next += 0x0000002;
	ioselect		= Next; Next += 0x0000004;

	RamEnd			= Next;

	pTempDraw		= (UINT16 *)Next; Next += 320 * 256 * sizeof(INT16);
	DrvPalette		= (UINT32   *)Next; Next += 0x1002 * sizeof(INT32);

	tile_bank		= (UINT16 *)(DrvVidRegs + 0x0008);

	MemEnd			= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	Sh2Reset();

	memset (AllRam, 0, RamEnd - AllRam);

	EEPROMReset();

	if (EEPROMAvailable() == 0) {
		EEPROMFill(DrvSh2RAM /*0 fill*/, 0, 0x100);
	}

	BurnYMF278BReset();

	sample_offs = 0;
	memset (ioselect, 0xff, 4);
	ioselect[2] = 0x32;
	
	if (mahjong) {
		pcmbank_previous = ~0;
		set_pcm_bank();
	}

	return 0;
}

static const eeprom_interface eeprom_interface_93C56 =
{
	8,			// address bits 8
	8,			// data bits    8
	"*110x",		// read         110x aaaaaaaa
	"*101x",		// write        101x aaaaaaaa dddddddd
	"*111x",		// erase        111x aaaaaaaa
	"*10000xxxxxxx",	// lock         100x 00xxxx
	"*10011xxxxxxx",	// unlock       100x 11xxxx
	0,
	0
};

static void BurnSwap32(UINT8 *src, INT32 len)
{
	for (INT32 i = 0; i < len; i+=4) {
		INT32 t = src[i+1];
		src[i+1] = src[i+2];
		src[i+2] = t;
	}
}

static void BurnSwapEndian()
{
	BurnSwap32(DrvSh2ROM, 0x100000);
	BurnByteswap(DrvSh2ROM, 0x300000);

	for (INT32 i = 0; i < 0x300000; i+=4) {
		INT32 t = DrvSh2ROM[i + 0];
		DrvSh2ROM[i + 0] = DrvSh2ROM[i+3];
		DrvSh2ROM[i + 3] = t;
		t = DrvSh2ROM[i + 1];
		DrvSh2ROM[i + 1] = DrvSh2ROM[i + 2];
		DrvSh2ROM[i + 2] = t;
	}
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (long long)Sh2TotalCycles() * nSoundRate / 28636350;
}

static void DrvIRQCallback(INT32, INT32 nStatus)
{
	if (nStatus)
		Sh2SetIRQLine(12, SH2_IRQSTATUS_AUTO);
	else
		Sh2SetIRQLine(12, SH2_IRQSTATUS_NONE);
}

static INT32 DrvInit(INT32 (*LoadCallback)(), INT32 gfx_len)
{
	AllMem = NULL;
	MemIndex(gfx_len);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex(gfx_len);

	{
		if (LoadCallback) {
			if (LoadCallback()) return 1;
		}

		BurnSwapEndian();
#ifndef LSB_FIRST
		le_to_be(DrvSh2ROM,0x0300000);
#endif
		BurnSwap32(DrvGfxROM, gfx_len);

		if (mahjong) {
			memcpy (DrvSndROM, DrvSndBanks, 0x200000);
		}
	}

	Sh2Init(1);
	Sh2Open(0);
	Sh2MapMemory(DrvSh2ROM,			0x00000000, 0x000fffff, SH2_ROM);
	Sh2MapMemory(DrvSh2ROM + 0x100000,	0x02000000, 0x021fffff, SH2_ROM);
	Sh2MapMemory(DrvSh2RAM,			0x06000000, 0x060fffff, SH2_RAM);
	Sh2SetReadByteHandler (0,		ps4_read_byte);
	Sh2SetReadWordHandler (0,		ps4_read_word);
	Sh2SetReadLongHandler (0,		ps4_read_long);
	Sh2SetWriteByteHandler(0,		ps4_write_byte);
	Sh2SetWriteWordHandler(0,		ps4_write_word);
	Sh2SetWriteLongHandler(0,		ps4_write_long);

	Sh2MapHandler(1, 0x06000000 | speedhack_address, 0x0600ffff | speedhack_address, SH2_ROM);
	Sh2SetReadByteHandler (1,		ps4hack_read_byte);
	Sh2SetReadWordHandler (1,		ps4hack_read_word);
	Sh2SetReadLongHandler (1,		ps4hack_read_long);

	BurnYMF278BInit(0, DrvSndROM, &DrvIRQCallback, DrvSynchroniseStream);
	BurnTimerAttachSh2(28636350);

	EEPROMInit(&eeprom_interface_93C56);

	nGfxMask = (gfx_len - 1) >> 8;

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	Sh2Exit();
	BurnYMF278BExit();

	EEPROMExit();

	BurnFree(AllMem);

	mahjong = 0;

	return 0;
}

static void draw_sprites(UINT16 *dest, UINT32 scr)
{
	UINT32 *psikyo4_vidregs = (UINT32*)DrvVidRegs;
	UINT32 *source = (UINT32 *)DrvSprRAM;
	UINT16 *list = (UINT16 *)source + 0x2c00/2 + 0x04/2;
	UINT16 listlen=(0xc00/2 - 0x04/2), listcntr=0;

	INT32 flipscreen1 = psikyo4_vidregs[1] & 0x40000000;
	INT32 flipscreen2 = psikyo4_vidregs[1] & 0x00400000;

	while (listcntr < listlen)
	{
		UINT16 listdat = list[listcntr ^ 1]; 
		UINT16 sprnum = (listdat & 0x03ff) << 1;

		UINT16 thisscreen = 0;
		if ((listdat & 0x2000) == (UINT16)scr) thisscreen = 1;

		if (!(listdat & 0x8000) && thisscreen)
		{
			INT32 xstart, ystart, xend, yend, xinc, yinc, loopnum=0;

			INT32 ypos =  (source[sprnum+0] & 0x03ff0000) >> 16;
			INT32 xpos =  (source[sprnum+0] & 0x000003ff);
			INT32 high =  (source[sprnum+0]               >> 28) + 1;
			INT32 wide = ((source[sprnum+0] & 0x0000f000) >> 12) + 1;
			INT32 tnum =  (source[sprnum+1] & 0x0007ffff);
			INT32 colr =  (source[sprnum+1] & 0x3f000000) >> 24;

			UINT32 flipx = (source[sprnum+1] & 0x40000000);
			UINT32 flipy = (source[sprnum+1] & 0x80000000);

			if (ypos & 0x200) ypos -= 0x400;
			if (xpos & 0x200) xpos -= 0x400;
   			if (scr)          colr += 0x040;

			if ((!scr && flipscreen1) || (scr && flipscreen2))
			{
				ypos = nScreenHeight - ypos - high*16;
				xpos = 40*8 - xpos - wide*16;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (flipx)	{ xstart = wide-1;  xend = -1;    xinc = -1; }
			else		{ xstart = 0;       xend = wide;  xinc = +1; }

			if (flipy)	{ ystart = high-1;  yend = -1;    yinc = -1; }
			else		{ ystart = 0;       yend = high;  yinc = +1; }

			for (INT32 j = ystart; j != yend; j += yinc) {
				for (INT32 i = xstart; i != xend; i += xinc, loopnum++) {
					INT32 sx = xpos+16*i;
					INT32 sy = ypos+16*j;
					INT32 code = tnum + loopnum;

					if (sx < -15 || sx >= 320 || sy < -15 || sy >= nScreenHeight || nGfxMask < code) continue;

					if (flipy) {
						if (flipx) {
							Render16x16Tile_Mask_FlipXY_Clip(dest, code, sx, sy, colr, 5, 0, 0, DrvGfxROM);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(dest, code, sx, sy, colr, 5, 0, 0, DrvGfxROM);
						}
					} else {
						if (flipx) {
							Render16x16Tile_Mask_FlipX_Clip(dest, code, sx, sy, colr, 5, 0, 0, DrvGfxROM);
						} else {
							Render16x16Tile_Mask_Clip(dest, code, sx, sy, colr, 5, 0, 0, DrvGfxROM);
						}
					}
				}
			}
		}

		listcntr++;
		if (listdat & 0x4000) break;
	}
}

static void DrvDoPalette(UINT32 *dst, INT32 c)
{
	UINT8 r,g,b;
	UINT32 *p = (UINT32*)DrvPalRAM;
	for (INT32 i = 0; i < 0x2000 / 4; i++) {
		r = p[i] >> 24;
		g = p[i] >> 16;
		b = p[i] >>  8;

		if (c == 0x7f) {
			dst[i] = BurnHighCol(r, g, b, 0);
		} else {
			dst[i] = BurnHighCol((r * c) / 0x7f, (g * c) / 0x7f, (b * c) / 0x7f, 0);
		}
	}
}

static void DrvRecalcPalette()
{
	UINT32 *p = (UINT32*)DrvPalRAM;

	DrvDoPalette(DrvPalette + 0x000, DrvBrightVal[0]);
	DrvDoPalette(DrvPalette + 0x800, DrvBrightVal[1]);

	DrvPalette[0x1000] = BurnHighCol(p[0x800] >> 24, p[0x800] >> 16, p[0x800] >> 8, 0);
	DrvPalette[0x1001] = BurnHighCol(p[0x801] >> 24, p[0x801] >> 16, p[0x801] >> 8, 0);
}

static INT32 DrvDraw()
{
	DrvRecalcPalette();
	BurnTransferClear();

	for (INT32 y = 0; y < nScreenHeight; y++) {
		for (INT32 x = 0; x < 320; x++) {
			pTransDraw[y * 640 + x] = 0x1000;
			pTempDraw[y * 320+x] = 0x1001;
		}
	}

	nScreenWidth = 640;
	draw_sprites(pTransDraw, 0x0000);	// left screen

	nScreenWidth = 320;
	draw_sprites(pTempDraw, 0x2000);	// right screen

	nScreenWidth = 640;
	for (INT32 y = 0; y < nScreenHeight; y++) {
		memcpy (pTransDraw + y * 640 + 320, pTempDraw + y * 320, 320 * sizeof(INT16));
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	Sh2NewFrame();

	{
		if (mahjong) {
			memset (DrvMahs, 0xff, 10);
			DrvMahs[0] = (~0x60) | (DrvDips[0] & 0x60);
			for (INT32 i = 0; i < 8; i++) {
				DrvMahs[0] ^= (DrvMah1[i] & 1) << i;
				DrvMahs[1] ^= (DrvMah2[i] & 1) << i;
				DrvMahs[2] ^= (DrvMah3[i] & 1) << i;
				DrvMahs[3] ^= (DrvMah4[i] & 1) << i;
				DrvMahs[4] ^= (DrvMah5[i] & 1) << i;
				DrvMahs[5] ^= (DrvMah6[i] & 1) << i;
				DrvMahs[6] ^= (DrvMah7[i] & 1) << i;
				DrvMahs[7] ^= (DrvMah8[i] & 1) << i;
				DrvMahs[8] ^= (DrvMah9[i] & 1) << i;
			}
		} else {
			DrvInputs[0] = (~0x60) | (DrvDips[0] & 0x60);
			DrvInputs[1] = ~0;
			for (INT32 i = 0; i < 32; i++) {
				DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
				DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			}
		}
	}

	BurnTimerEndFrame(28636350 / 60);
	Sh2SetIRQLine(4, SH2_IRQSTATUS_AUTO);
	Sh2Run(0);
	Sh2SetIRQLine(4, SH2_IRQSTATUS_NONE);
	BurnYMF278BUpdate(nBurnSoundLen);

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {
		*pnMin =  0x029707;
	}
	
	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= AllRam;
		ba.nLen		= RamEnd - AllRam;
		ba.nAddress	= 0;
		ba.szName	= "All RAM";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		Sh2Scan(nAction);
		BurnYMF278BScan(nAction, pnMin);
		
		SCAN_VAR(sample_offs);
	}

	if (nAction & ACB_WRITE) {
		set_pcm_bank();
	}

	return 0;
}


// Taisen Hot Gimmick (Japan)

static struct BurnRomInfo hotgmckRomDesc[] = {
	{ "2-u23.bin",	0x080000, 0x23ed4aa5, 1 | BRF_PRG | BRF_ESS }, //  0 Sh2 Code
	{ "1-u22.bin",	0x080000, 0x5db3649f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "prog.bin",	0x200000, 0x500f6b1b, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "0l.bin",	0x400000, 0x91f9ba60, 2 | BRF_GRA },           //  3 Sprites
	{ "0h.bin",	0x400000, 0xbfa800b7, 2 | BRF_GRA },           //  4
	{ "1l.bin",	0x400000, 0x4b670809, 2 | BRF_GRA },           //  5
	{ "1h.bin",	0x400000, 0xab513a4d, 2 | BRF_GRA },           //  6
	{ "2l.bin",	0x400000, 0x1a7d51e9, 2 | BRF_GRA },           //  7
	{ "2h.bin",	0x400000, 0xbf866222, 2 | BRF_GRA },           //  8
	{ "3l.bin",	0x400000, 0xa8a646f7, 2 | BRF_GRA },           //  9
	{ "3h.bin",	0x400000, 0x8c32becd, 2 | BRF_GRA },           // 10

	{ "snd0.bin",	0x400000, 0xc090d51a, 3 | BRF_SND },           // 11 Samples
	{ "snd1.bin",	0x400000, 0xc24243b5, 3 | BRF_SND },           // 12
};

STD_ROM_PICK(hotgmck)
STD_ROM_FN(hotgmck)

static INT32 HotgmckLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0100000,  2, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM  + 0x0000000,  3, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0000001,  4, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0800000,  5, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0800001,  6, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000000,  7, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000001,  8, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1800000,  9, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1800001, 10, 2)) return 1;

	if (BurnLoadRom(DrvSndBanks+ 0x0000000, 11, 1)) return 1;
	if (BurnLoadRom(DrvSndBanks+ 0x0400000, 12, 1)) return 1;

	return 0;
}

static INT32 HotgmckInit()
{
	mahjong = 1;

	return DrvInit(HotgmckLoadCallback, 0x2000000);
}

struct BurnDriver BurnDrvHotgmck = {
	"hotgmck", NULL, NULL, NULL, "1997",
	"Taisen Hot Gimmick (Japan)\0", NULL, "Psikyo", "PS4",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_MAHJONG, 0,
	NULL, hotgmckRomInfo, hotgmckRomName, NULL, NULL, HotgmckInputInfo, HotgmckDIPInfo,
	HotgmckInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1002,
	640, 224, 8, 3
};


// Taisen Hot Gimmick Kairakuten (Japan)

static struct BurnRomInfo hgkairakRomDesc[] = {
	{ "2.u23",	0x080000, 0x1c1a034d, 1 | BRF_PRG | BRF_ESS }, //  0 Sh2 Code
	{ "1.u22",	0x080000, 0x24b04aa2, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "prog.u1",	0x100000, 0x83cff542, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "0l.u2",	0x400000, 0xf7472212, 2 | BRF_GRA },           //  3 Sprites
	{ "0h.u11",	0x400000, 0x30019d0f, 2 | BRF_GRA },           //  4
	{ "1l.u3",	0x400000, 0xf46d5002, 2 | BRF_GRA },           //  5
	{ "1h.u12",	0x400000, 0x210592b6, 2 | BRF_GRA },           //  6
	{ "2l.u4",	0x400000, 0xb98adf21, 2 | BRF_GRA },           //  7
	{ "2h.u13",	0x400000, 0x8e3da1e1, 2 | BRF_GRA },           //  8
	{ "3l.u5",	0x400000, 0xfa7ba4ed, 2 | BRF_GRA },           //  9
	{ "3h.u14",	0x400000, 0xa5d400ea, 2 | BRF_GRA },           // 10
	{ "4l.u6",	0x400000, 0x76c10026, 2 | BRF_GRA },           // 11
	{ "4h.u15",	0x400000, 0x799f0889, 2 | BRF_GRA },           // 12
	{ "5l.u7",	0x400000, 0x4639ef36, 2 | BRF_GRA },           // 13
	{ "5h.u16",	0x400000, 0x549e9e9e, 2 | BRF_GRA },           // 14

	{ "snd0.u10",	0x400000, 0x0e8e5fdf, 3 | BRF_SND },           // 15 Samples
	{ "snd1.u19",	0x400000, 0xd8057d2f, 3 | BRF_SND },           // 16
};

STD_ROM_PICK(hgkairak)
STD_ROM_FN(hgkairak)

static INT32 HgkairakLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0100000,  2, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM  + 0x0000000,  3, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0000001,  4, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0800000,  5, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0800001,  6, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000000,  7, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000001,  8, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1800000,  9, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1800001, 10, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x2000000, 11, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x2000001, 12, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x2800000, 13, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x2800001, 14, 2)) return 1;

	if (BurnLoadRom(DrvSndBanks+ 0x0000000, 15, 1)) return 1;
	if (BurnLoadRom(DrvSndBanks+ 0x0400000, 16, 1)) return 1;

	return 0;
}

static INT32 HgkairakInit()
{
	mahjong = 1;

	return DrvInit(HgkairakLoadCallback, 0x3000000);
}

struct BurnDriver BurnDrvHgkairak = {
	"hgkairak", NULL, NULL, NULL, "1998",
	"Taisen Hot Gimmick Kairakuten (Japan)\0", NULL, "Psikyo", "PS4",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_MAHJONG, 0,
	NULL, hgkairakRomInfo, hgkairakRomName, NULL, NULL, HotgmckInputInfo, HotgmckDIPInfo,
	HgkairakInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1002,
	640, 224, 8, 3
};


// Taisen Hot Gimmick 3 Digital Surfing (Japan)

static struct BurnRomInfo hotgmck3RomDesc[] = {
	{ "2.u22",	0x080000, 0x3b06a4a3, 1 | BRF_PRG | BRF_ESS }, //  0 Sh2 Code
	{ "1.u23",	0x080000, 0x7aad6b24, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "prog.u1",	0x100000, 0x316c3356, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "0l.u2",	0x400000, 0xd4bbd035, 2 | BRF_GRA },           //  3 Sprites
	{ "0h.u11",	0x400000, 0xe8832b0b, 2 | BRF_GRA },           //  4
	{ "1l.u3",	0x400000, 0x08426cb2, 2 | BRF_GRA },           //  5
	{ "1h.u12",	0x400000, 0x112c6eea, 2 | BRF_GRA },           //  6
	{ "2l.u4",	0x400000, 0x0f197cd4, 2 | BRF_GRA },           //  7
	{ "2h.u13",	0x400000, 0xfc37808c, 2 | BRF_GRA },           //  8
	{ "3l.u5",	0x400000, 0xc4d094dc, 2 | BRF_GRA },           //  9
	{ "3h.u14",	0x400000, 0xef0dad0a, 2 | BRF_GRA },           // 10
	{ "4l.u6",	0x400000, 0x5186790f, 2 | BRF_GRA },           // 11
	{ "4h.u15",	0x400000, 0x187a6f43, 2 | BRF_GRA },           // 12
	{ "5l.u7",	0x400000, 0xecf151f3, 2 | BRF_GRA },           // 13
	{ "5h.u16",	0x400000, 0x720bf4ec, 2 | BRF_GRA },           // 14
	{ "6l.u8",	0x400000, 0xe490404d, 2 | BRF_GRA },           // 15
	{ "6h.u17",	0x400000, 0x7e8a141a, 2 | BRF_GRA },           // 16
	{ "7l.u9",	0x400000, 0x2ec78fb2, 2 | BRF_GRA },           // 17
	{ "7h.u18",	0x400000, 0xc1735612, 2 | BRF_GRA },           // 18

	{ "snd0.u10",	0x400000, 0xd62a0dba, 3 | BRF_SND },           // 19 Samples
	{ "snd1.u19",	0x400000, 0x1df91fb4, 3 | BRF_SND },           // 20
};

STD_ROM_PICK(hotgmck3)
STD_ROM_FN(hotgmck3)

static INT32 Hotgmck3LoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  0, 2)) return 1; // swapped
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  1, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0100000,  2, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM  + 0x0000000,  3, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0000001,  4, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0800000,  5, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0800001,  6, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000000,  7, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000001,  8, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1800000,  9, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1800001, 10, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x2000000, 11, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x2000001, 12, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x2800000, 13, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x2800001, 14, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x3000000, 15, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x3000001, 16, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x3800000, 17, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x3800001, 18, 2)) return 1;

	if (BurnLoadRom(DrvSndBanks+ 0x0000000, 19, 1)) return 1;
	if (BurnLoadRom(DrvSndBanks+ 0x0400000, 20, 1)) return 1;

	return 0;
}

static INT32 Hotgmck3Init()
{
	mahjong = 1;

	return DrvInit(Hotgmck3LoadCallback, 0x4000000);
}

struct BurnDriver BurnDrvHotgmck3 = {
	"hotgmck3", NULL, NULL, NULL, "1999",
	"Taisen Hot Gimmick 3 Digital Surfing (Japan)\0", NULL, "Psikyo", "PS4",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_MAHJONG, 0,
	NULL, hotgmck3RomInfo, hotgmck3RomName, NULL, NULL, HotgmckInputInfo, HotgmckDIPInfo,
	Hotgmck3Init, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1002,
	640, 224, 8, 3
};


// Taisen Hot Gimmick 4 Ever (Japan)

static struct BurnRomInfo hotgm4evRomDesc[] = {
	{ "2.u22",	0x080000, 0x3334c21e, 1 | BRF_PRG | BRF_ESS }, //  0 Sh2 Code
	{ "1.u23",	0x080000, 0xb1a1c643, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "prog.u1",	0x400000, 0xad556d8e, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "0l.u2",	0x400000, 0xf65986f7, 2 | BRF_GRA },           //  3 Sprites
	{ "0h.u11",	0x400000, 0x51fd07a9, 2 | BRF_GRA },           //  4
	{ "1l.u3",	0x400000, 0xf59d21d7, 2 | BRF_GRA },           //  5
	{ "1h.u12",	0x400000, 0x60ea4797, 2 | BRF_GRA },           //  6
	{ "2l.u4",	0x400000, 0xfbaf05e3, 2 | BRF_GRA },           //  7
	{ "2h.u13",	0x400000, 0x61281612, 2 | BRF_GRA },           //  8
	{ "3l.u5",	0x400000, 0xe2e1bd9f, 2 | BRF_GRA },           //  9
	{ "3h.u14",	0x400000, 0xc4426542, 2 | BRF_GRA },           // 10
	{ "4l.u6",	0x400000, 0x7298a242, 2 | BRF_GRA },           // 11
	{ "4h.u15",	0x400000, 0xfe91b459, 2 | BRF_GRA },           // 12
	{ "5l.u7",	0x400000, 0xcc714a7d, 2 | BRF_GRA },           // 13
	{ "5h.u16",	0x400000, 0x2f149cf9, 2 | BRF_GRA },           // 14
	{ "6l.u8",	0x400000, 0xbfe97dfe, 2 | BRF_GRA },           // 15
	{ "6h.u17",	0x400000, 0x3473052a, 2 | BRF_GRA },           // 16
	{ "7l.u9",	0x400000, 0x022a8a31, 2 | BRF_GRA },           // 17
	{ "7h.u18",	0x400000, 0x77e47409, 2 | BRF_GRA },           // 18

	{ "snd0.u10",	0x400000, 0x051e2fed, 3 | BRF_SND },           // 19 Samples
	{ "snd1.u19",	0x400000, 0x0de0232d, 3 | BRF_SND },           // 20
};

STD_ROM_PICK(hotgm4ev)
STD_ROM_FN(hotgm4ev)

struct BurnDriver BurnDrvHotgm4ev = {
	"hotgm4ev", NULL, NULL, NULL, "2000",
	"Taisen Hot Gimmick 4 Ever (Japan)\0", NULL, "Psikyo", "PS4",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_MAHJONG, 0,
	NULL, hotgm4evRomInfo, hotgm4evRomName, NULL, NULL, HotgmckInputInfo, HotgmckDIPInfo,
	Hotgmck3Init, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1002,
	640, 224, 8, 3
};


// Mahjong Hot Gimmick Integral (Japan)

static struct BurnRomInfo hotgmckiRomDesc[] = {
	{ "2.u22",	0x080000, 0xabc192dd, 1 | BRF_PRG | BRF_ESS }, //  0 Sh2 Code
	{ "1.u23",	0x080000, 0x8be896d0, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "prog.u1",	0x200000, 0x9017ae8e, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "0l.u2",	0x200000, 0x58ae45eb, 2 | BRF_GRA },           //  3 Sprites
	{ "0h.u11",	0x200000, 0xd7bbb929, 2 | BRF_GRA },           //  4
	{ "1l.u3",	0x200000, 0x27576360, 2 | BRF_GRA },           //  5
	{ "1h.u12",	0x200000, 0x7439a63f, 2 | BRF_GRA },           //  6
	{ "2l.u4",	0x200000, 0xfda64e24, 2 | BRF_GRA },           //  7
	{ "2h.u13",	0x200000, 0x8be54ea6, 2 | BRF_GRA },           //  8
	{ "3l.u5",	0x200000, 0x92507b3f, 2 | BRF_GRA },           //  9
	{ "3h.u14",	0x200000, 0x042bef5e, 2 | BRF_GRA },           // 10
	{ "4l.u6",	0x200000, 0x023b6d70, 2 | BRF_GRA },           // 11
	{ "4h.u15",	0x200000, 0x9be7e8b1, 2 | BRF_GRA },           // 12
	{ "5l.u7",	0x200000, 0x7aa54306, 2 | BRF_GRA },           // 13
	{ "5h.u16",	0x200000, 0xe6b48e52, 2 | BRF_GRA },           // 14
	{ "6l.u8",	0x200000, 0xdfe675e9, 2 | BRF_GRA },           // 15
	{ "6h.u17",	0x200000, 0x45919576, 2 | BRF_GRA },           // 16
	{ "7l.u9",	0x200000, 0xcd3af598, 2 | BRF_GRA },           // 17
	{ "7h.u18",	0x200000, 0xa3fd4ae5, 2 | BRF_GRA },           // 18

	{ "snd0.u10",	0x400000, 0x5f275f35, 3 | BRF_SND },           // 19 Samples
	{ "snd1.u19",	0x400000, 0x98608779, 3 | BRF_SND },           // 20
};

STD_ROM_PICK(hotgmcki)
STD_ROM_FN(hotgmcki)

static INT32 HotgmckiLoadCallback()
{
	if (Hotgmck3LoadCallback()) return 1;

	// mirror gfx roms
	for (INT32 i = 0; i < 0x4000000; i += 0x0800000) {
		memcpy (DrvGfxROM + 0x0400000 + i, DrvGfxROM + i, 0x0400000);
	}

	return 0;
}

static INT32 HotgmckiInit()
{
	mahjong = 1;

	return DrvInit(HotgmckiLoadCallback, 0x4000000);
}

struct BurnDriver BurnDrvHotgmcki = {
	"hotgmcki", NULL, NULL, NULL, "2001",
	"Mahjong Hot Gimmick Integral (Japan)\0", NULL, "Psikyo", "PS4",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_MAHJONG, 0,
	NULL, hotgmckiRomInfo, hotgmckiRomName, NULL, NULL, HotgmckInputInfo, HotgmckDIPInfo,
	HotgmckiInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1002,
	640, 224, 8, 3
};


// Lode Runner - The Dig Fight (ver. B)

static struct BurnRomInfo loderndfRomDesc[] = {
	{ "1b.u23",	0x080000, 0xfae92286, 1 | BRF_PRG | BRF_ESS }, //  0 Sh2 Code
	{ "2b.u22",	0x080000, 0xfe2424c0, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "0l.u2",	0x800000, 0xccae855d, 2 | BRF_GRA },           //  2 Sprites
	{ "0h.u11",	0x800000, 0x7a146c59, 2 | BRF_GRA },           //  3
	{ "1l.u3",	0x800000, 0x7a9cd21e, 2 | BRF_GRA },           //  4
	{ "1h.u12",	0x800000, 0x78f40d0d, 2 | BRF_GRA },           //  5

	{ "snd0.u10",	0x800000, 0x2da3788f, 3 | BRF_SND },           //  6 Samples
};

STD_ROM_PICK(loderndf)
STD_ROM_FN(loderndf)

static INT32 LoderndfLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(DrvGfxROM  + 0x0000000,  6, 1)) return 1;
	memcpy (DrvSndROM, DrvGfxROM, 0x400000); // otherwise we have to waste 4mb

	if (BurnLoadRom(DrvGfxROM  + 0x0000000,  2, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0000001,  3, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000000,  4, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000001,  5, 2)) return 1;

	return 0;
}

static INT32 LoderndfInit()
{
	speedhack_address = 0x0020;
	speedhack_pc[0] = 0x00001B3E;
	speedhack_pc[1] = 0x00001B40;

	return DrvInit(LoderndfLoadCallback, 0x2000000);
}

struct BurnDriver BurnDrvLoderndf = {
	"loderndf", NULL, NULL, NULL, "2000",
	"Lode Runner - The Dig Fight (ver. B)\0", NULL, "Psikyo", "PS4",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_PLATFORM, 0,
	NULL, loderndfRomInfo, loderndfRomName, NULL, NULL, LoderndfInputInfo, LoderndfDIPInfo,
	LoderndfInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1002,
	640, 240, 8, 3
};


// Lode Runner - The Dig Fight (ver. A)

static struct BurnRomInfo loderndfaRomDesc[] = {
	{ "12.u23",	0x080000, 0x661d372e, 1 | BRF_PRG | BRF_ESS }, //  0 Sh2 Code
	{ "3.u22",	0x080000, 0x0a63529f, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "0l.u2",	0x800000, 0xccae855d, 2 | BRF_GRA },           //  2 Sprites
	{ "0h.u11",	0x800000, 0x7a146c59, 2 | BRF_GRA },           //  3
	{ "1l.u3",	0x800000, 0x7a9cd21e, 2 | BRF_GRA },           //  4
	{ "1h.u12",	0x800000, 0x78f40d0d, 2 | BRF_GRA },           //  5

	{ "snd0.u10",	0x800000, 0x2da3788f, 3 | BRF_SND },           //  6 Samples
};

STD_ROM_PICK(loderndfa)
STD_ROM_FN(loderndfa)

static INT32 LoderndfaInit()
{
	speedhack_address = 0x0020;
	speedhack_pc[0] = 0x00001B4A;
	speedhack_pc[1] = 0x00001B4C;

	return DrvInit(LoderndfLoadCallback, 0x2000000);
}

struct BurnDriver BurnDrvLoderndfa = {
	"loderndfa", "loderndf", NULL, NULL, "2000",
	"Lode Runner - The Dig Fight (ver. A)\0", NULL, "Psikyo", "PS4",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PSIKYO, GBF_PLATFORM, 0,
	NULL, loderndfaRomInfo, loderndfaRomName, NULL, NULL, LoderndfInputInfo, LoderndfDIPInfo,
	LoderndfaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1002,
	640, 240, 8, 3
};


// Quiz de Idol! Hot Debut (Japan)

static struct BurnRomInfo hotdebutRomDesc[] = {
	{ "1.u23",	0x080000, 0x0b0d0027, 1 | BRF_PRG | BRF_ESS }, //  0 Sh2 Code
	{ "2.u22",	0x080000, 0xc3b5180b, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "0l.u2",	0x400000, 0x15da9983, 2 | BRF_GRA },           //  2 Sprites
	{ "0h.u11",	0x400000, 0x76d7b73f, 2 | BRF_GRA },           //  3
	{ "1l.u3",	0x400000, 0x76ea3498, 2 | BRF_GRA },           //  4
	{ "1h.u12",	0x400000, 0xa056859f, 2 | BRF_GRA },           //  5
	{ "2l.u4",	0x400000, 0x9d2d1bb1, 2 | BRF_GRA },           //  6
	{ "2h.u13",	0x400000, 0xa7753c4d, 2 | BRF_GRA },           //  7

	{ "snd0.u10",	0x400000, 0xeef28aa7, 3 | BRF_SND },           //  8 Samples
};

STD_ROM_PICK(hotdebut)
STD_ROM_FN(hotdebut)

static INT32 HotdebutLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(DrvGfxROM  + 0x0000000,  2, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0000001,  3, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0800000,  4, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x0800001,  5, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000000,  6, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM  + 0x1000001,  7, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000,  8, 1)) return 1;

	return 0;
}

static INT32 HotdebutInit()
{
	speedhack_address = 0x0001c;
	speedhack_pc[0] = 0x000029EE;
	speedhack_pc[1] = 0x000029F0;

	return DrvInit(HotdebutLoadCallback, 0x1800000);
}

struct BurnDriver BurnDrvHotdebut = {
	"hotdebut", NULL, NULL, NULL, "2000",
	"Quiz de Idol! Hot Debut (Japan)\0", NULL, "Psikyo / Moss", "PS4",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_QUIZ, 0,
	NULL, hotdebutRomInfo, hotdebutRomName, NULL, NULL, HotdebutInputInfo, HotdebutDIPInfo,
	HotdebutInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1002,
	640, 240, 8, 3
};
