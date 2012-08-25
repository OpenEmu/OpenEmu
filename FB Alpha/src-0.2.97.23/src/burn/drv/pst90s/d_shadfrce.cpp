/********************************************************************************
 Shadow Force (c)1993 Technos
 Preliminary Driver by David Haywood
 Based on the Various Other Technos Games
 ********************************************************************************
 port to Finalburn Alpha by OopsWare. 2007
 ********************************************************************************
 
 2007.07.05
   bug fix: no reset while init driver, 68k boot from pc addr 0x000000 ...
   add pri-buf (RamPri) for render
   
 ********************************************************************************/

#include "burnint.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "burn_ym2151.h"
#include "msm6295.h"

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;

static UINT8 *Rom68K;
static UINT8 *RomZ80;
static UINT8 *RomGfx01;
static UINT8 *RomGfx02;
static UINT8 *RomGfx03;

static UINT8 *Ram68K;
static UINT16 *RamBg00;
static UINT16 *RamBg01;
static UINT16 *RamFg;
static UINT16 *RamSpr;
static UINT16 *RamPal;
static UINT8 *RamZ80;

static UINT16 *RamCurPal;
static UINT8 *RamPri;

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDipBtn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static UINT8 DrvInput[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvReset = 0;
static UINT8 nBrightness = 0xFF;

static UINT8 bVBlink = 0;
static UINT16 bg0scrollx, bg0scrolly, bg1scrollx, bg1scrolly;
static UINT8 nSoundlatch = 0;
static UINT8 bRecalcPalette = 0;

static INT32 nZ80Cycles;

inline static void CalcCol(INT32 idx)
{
	/* xBBBBBGGGGGRRRRR */
	UINT16 nColour = RamPal[idx];
	INT32 r = (nColour & 0x001F) << 3;	r |= r >> 5;	// Red
	INT32 g = (nColour & 0x03E0) >> 2;	g |= g >> 5;	// Green
	INT32 b = (nColour & 0x7C00) >> 7;	b |= b >> 5;	// Blue
	r = (r * nBrightness) >> 8;
	g = (g * nBrightness) >> 8;
	b = (b * nBrightness) >> 8;
	RamCurPal[idx] = BurnHighCol(r, g, b, 0);
}

static struct BurnInputInfo shadfrceInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvDipBtn + 0,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"},

	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},
	{"P1 Button 4",	BIT_DIGITAL,	DrvButton + 0,	"p1 fire 4"},
	{"P1 Button 5",	BIT_DIGITAL,	DrvButton + 1,	"p1 fire 5"},
	{"P1 Button 6",	BIT_DIGITAL,	DrvButton + 2,	"p1 fire 6"},

	{"P2 Coin",		BIT_DIGITAL,	DrvDipBtn + 1,	"p2 coin"},		// only available when you are in "test mode"
	{"P2 Start",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"},

	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},
	{"P2 Button 4",	BIT_DIGITAL,	DrvButton + 3,	"p2 fire 4"},
	{"P2 Button 5",	BIT_DIGITAL,	DrvButton + 4,	"p2 fire 5"},
	{"P2 Button 6",	BIT_DIGITAL,	DrvButton + 5,	"p2 fire 6"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 1,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
	{"Dip D",		BIT_DIPSWITCH,	DrvInput + 7,	"dip"},
};

STDINPUTINFO(shadfrce)

static struct BurnDIPInfo shadfrceDIPList[] = {

	// Defaults
	{0x19,	0xFF, 0xFF,	0x00, NULL},
	{0x1A,	0xFF, 0xFF,	0x00, NULL},
	{0x1B,	0xFF, 0xFF,	0x00, NULL},
	{0x1C,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	// coin 1
	// coin 2
	{0,		0xFE, 0,	2,	  "Service1"},	// only available when you are in "test mode"
	{0x19,	0x01, 0x04,	0x00, "Off"},
	{0x19,	0x01, 0x04,	0x04, "On"},
	
	// DIP 2
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x1A,	0x01, 0x03, 0x00, "Normal"},
	{0x1A,	0x01, 0x03, 0x01, "Hard"},
	{0x1A,	0x01, 0x03, 0x02, "Easy"},
	{0x1A,	0x01, 0x03, 0x03, "Hardest"},
	{0,		0xFE, 0,	4,	  "Stage Clear Energy Regain"},
	{0x1A,	0x01, 0x0C, 0x00, "25%"}, 
	{0x1A,	0x01, 0x0C, 0x04, "10%"},	
	{0x1A,	0x01, 0x0C, 0x08, "50%"}, 
	{0x1A,	0x01, 0x0C, 0x0C, "0%"},	
	
	// DIP 3
	{0,		0xFE, 0,	4,	  "Coinage"},	
	{0x1B,	0x01, 0x06, 0x00, "1C_1C"},
	{0x1B,	0x01, 0x06, 0x02, "1C_2C"},
	{0x1B,	0x01, 0x06, 0x04, "2C_1C"},
	{0x1B,	0x01, 0x06, 0x06, "3C_1C"},
	
	{0,		0xFE, 0,	2,	  "Continue Price"},
	{0x1B,	0x01, 0x08, 0x00, "Off"},
	{0x1B,	0x01, 0x08, 0x80, "On"},
	{0,		0xFE, 0,	2,	  "Free Play"},
	{0x1B,	0x01, 0x10, 0x00, "Off"}, 
	{0x1B,	0x01, 0x10, 0x10, "On"},
	{0,		0xFE, 0,	2,	  "Flip Screen"},
	{0x1B,	0x01, 0x20, 0x00, "Off"}, 
	{0x1B,	0x01, 0x20, 0x20, "On"},
	
	// DIP 4
	{0,		0xFE, 0,	2,	  "Demo Sounds"},
	{0x1C,	0x01, 0x01,	0x00, "Off"},
	{0x1C,	0x01, 0x01,	0x01, "On"},
	{0,		0xFE, 0,	2,	  "Test Mode"},
	{0x1C,	0x01, 0x02,	0x00, "Off"},
	{0x1C,	0x01, 0x02,	0x02, "On"},
//	{0,		0xFE, 0,	2,	  "VBlink"},	// ???
//	{0x1C,	0x01, 0x04,	0x00, "Off"},
//	{0x1C,	0x01, 0x04,	0x04, "On"},

};

STDDIPINFO(shadfrce)

// Rom information
static struct BurnRomInfo shadfrceRomDesc[] = {
	{ "32a12-01.34",  0x040000, 0x04501198, BRF_ESS | BRF_PRG },	// 68000 code 
	{ "32a13-01.26",  0x040000, 0xb8f8a05c, BRF_ESS | BRF_PRG }, 
	{ "32a14-0.33",   0x040000, 0x08279be9, BRF_ESS | BRF_PRG }, 
	{ "32a15-0.14",	  0x040000, 0xbfcadfea, BRF_ESS | BRF_PRG }, 
	
	{ "32j10-0.42",   0x010000, 0x65daf475, BRF_ESS | BRF_PRG },	// Z80 code
	
	{ "32a11-0.55",	  0x020000, 0xcfaf5e77, BRF_GRA }, 				// gfx 1 chars
	
	{ "32j4-0.12", 	  0x200000, 0x1ebea5b6, BRF_GRA }, 				// gfx 2 sprite
	{ "32j5-0.13",	  0x200000, 0x600026b5, BRF_GRA }, 
	{ "32j6-0.24", 	  0x200000, 0x6cde8ebe, BRF_GRA }, 
	{ "32j7-0.25",	  0x200000, 0xbcb37922, BRF_GRA }, 
	{ "32j8-0.32",	  0x200000, 0x201bebf6, BRF_GRA }, 

	{ "32j1-0.4",     0x100000, 0xf1cca740, BRF_GRA },				// gfx 3 bg
	{ "32j2-0.5",     0x100000, 0x5fac3e01, BRF_GRA },			
	{ "32j3-0.6",     0x100000, 0xd297925e, BRF_GRA },
	
	{ "32j9-0.76",    0x080000, 0x16001e81, BRF_SND },				// PCM
};

STD_ROM_PICK(shadfrce)
STD_ROM_FN(shadfrce)

static struct BurnRomInfo shadfrcjRomDesc[] = {
	{ "32a12-011.34", 0x040000, 0x0c041e08, BRF_ESS | BRF_PRG },	// 68000 code 
	{ "32a13-010.26", 0x040000, 0x00985361, BRF_ESS | BRF_PRG }, 
	{ "32a14-010.33", 0x040000, 0xea03ca25, BRF_ESS | BRF_PRG }, 
	{ "32j15-01.14",  0x040000, 0x3dc3a84a, BRF_ESS | BRF_PRG }, 
	
	{ "32j10-0.42",   0x010000, 0x65daf475, BRF_ESS | BRF_PRG },	// Z80 code
	
	{ "32j11-0.55",   0x020000, 0x7252d993, BRF_GRA }, 				// gfx 1 chars
	
	{ "32j4-0.12", 	  0x200000, 0x1ebea5b6, BRF_GRA }, 				// gfx 2 sprite
	{ "32j5-0.13",	  0x200000, 0x600026b5, BRF_GRA }, 
	{ "32j6-0.24", 	  0x200000, 0x6cde8ebe, BRF_GRA }, 
	{ "32j7-0.25",	  0x200000, 0xbcb37922, BRF_GRA }, 
	{ "32j8-0.32",	  0x200000, 0x201bebf6, BRF_GRA }, 

	{ "32j1-0.4",     0x100000, 0xf1cca740, BRF_GRA },				// gfx 3 bg
	{ "32j2-0.5",     0x100000, 0x5fac3e01, BRF_GRA },			
	{ "32j3-0.6",     0x100000, 0xd297925e, BRF_GRA },
	
	{ "32j9-0.76",    0x080000, 0x16001e81, BRF_SND },				// PCM
};

STD_ROM_PICK(shadfrcj)
STD_ROM_FN(shadfrcj)

static struct BurnRomInfo shadfrcjv2RomDesc[] = {
	{ "32j12-01.34",  0x040000, 0x38fdbe1d, BRF_ESS | BRF_PRG },	// 68000 code 
	{ "32j13-01.26",  0x040000, 0x6e1df6f1, BRF_ESS | BRF_PRG }, 
	{ "32j14-01.33",  0x040000, 0x89e3fb60, BRF_ESS | BRF_PRG }, 
	{ "32j15-01.14",  0x040000, 0x3dc3a84a, BRF_ESS | BRF_PRG }, 
	
	{ "32j10-0.42",   0x010000, 0x65daf475, BRF_ESS | BRF_PRG },	// Z80 code
	
	{ "32j11-0.55",   0x020000, 0x7252d993, BRF_GRA }, 				// gfx 1 chars
	
	{ "32j4-0.12", 	  0x200000, 0x1ebea5b6, BRF_GRA }, 				// gfx 2 sprite
	{ "32j5-0.13",	  0x200000, 0x600026b5, BRF_GRA }, 
	{ "32j6-0.24", 	  0x200000, 0x6cde8ebe, BRF_GRA }, 
	{ "32j7-0.25",	  0x200000, 0xbcb37922, BRF_GRA }, 
	{ "32j8-0.32",	  0x200000, 0x201bebf6, BRF_GRA }, 

	{ "32j1-0.4",     0x100000, 0xf1cca740, BRF_GRA },				// gfx 3 bg
	{ "32j2-0.5",     0x100000, 0x5fac3e01, BRF_GRA },			
	{ "32j3-0.6",     0x100000, 0xd297925e, BRF_GRA },
	
	{ "32j9-0.76",    0x080000, 0x16001e81, BRF_SND },				// PCM
};

STD_ROM_PICK(shadfrcjv2)
STD_ROM_FN(shadfrcjv2)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom68K 		= Next; Next += 0x100000;			// 68000 ROM
	RomZ80		= Next; Next += 0x010000;			// Z80 ROM
	RomGfx01	= Next; Next += 0x020000 / 4 * 8;	// fg 8x8x4
	RomGfx02	= Next; Next += 0xA00000 / 5 * 8;	// spr 16x16x5 sprite
	RomGfx03	= Next; Next += 0x300000 / 6 * 8;	// bg 16x16x6 tile
	MSM6295ROM	= Next; Next += 0x080000;
	
	RamStart	= Next;
	RamBg00		= (UINT16 *) Next; Next += 0x001000 * sizeof(UINT16);
	RamBg01		= (UINT16 *) Next; Next += 0x001000 * sizeof(UINT16);
	RamFg		= (UINT16 *) Next; Next += 0x001000 * sizeof(UINT16);
	RamSpr		= (UINT16 *) Next; Next += 0x001000 * sizeof(UINT16);
	RamPal		= (UINT16 *) Next; Next += 0x004000 * sizeof(UINT16);
	Ram68K		= Next; Next += 0x010000;
	RamZ80		= Next; Next += 0x001800;
	RamEnd		= Next;
	
	RamCurPal	= (UINT16 *) Next; Next += 0x004000 * sizeof(UINT16);
	RamPri		= Next; Next += 0x014000;			// 320x256 Priority Buffer
	
	MemEnd		= Next;
	return 0;
}

UINT8 __fastcall shadfrceReadByte(UINT32 sekAddress)
{
/*
	INP0 : --AABBBB CCCCCCCC :  DIP2-8 DIP2-7,  coin 1 coin 2 service 1 , player 1
	INP1 : --DDDDDD EEEEEEEE :  Difficulty but DIP2-8 DIP2-7,  player 2
	INP2 : --FFFFFF GGGGGGGG :  Coinage ~ Flip_Screen but Demo_Sounds service, button 4~6
	INP3 : ----HHII ???????? :  v-blink? ,  service Demo_Sounds
*/	
	switch (sekAddress) {
		case 0x1D0020:
			return (~DrvInput[1] & 0x3F);
		case 0x1D0021:
			return ~DrvInput[0];
		case 0x1D0022:
			return (~DrvInput[3] & 0x3F);
		case 0x1D0023:
			return ~DrvInput[2];
		case 0x1D0024:
			return (~DrvInput[5] & 0x3F);
		case 0x1D0025:
			return ~DrvInput[4];
		case 0x1D0026:
			return ~(DrvInput[7] | (bVBlink << 2)) /*& 0x0F*/;
		case 0x1D0027:
			return ~DrvInput[6];
			
		case 0x1C000B:
			return 0;
		case 0x1D000D:
			return nBrightness;
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall shadfrceReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x1D0020:
			return ~(DrvInput[0] | (DrvInput[1] << 8)) & 0x3FFF;
		case 0x1D0022:
			return ~(DrvInput[2] | (DrvInput[3] << 8)) & 0x3FFF;
		case 0x1D0024:
			return ~(DrvInput[4] | (DrvInput[5] << 8)) & 0x3FFF;
		case 0x1D0026:
			return ~(DrvInput[6] | ( (DrvInput[7] | (bVBlink << 2))<< 8)) /*& 0x0FFF*/;
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall shadfrceWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x1C000B:
			// shadfrce_flip_screen
			break;
		case 0x1D000C:
//			bprintf(PRINT_NORMAL, _T("sound write %02x\n"), byteValue);
			nSoundlatch = byteValue;
			ZetNmi();
			break;
		case 0x1D000D:
			//bprintf(PRINT_NORMAL, _T("Brightness set to %02x\n"), byteValue);
			nBrightness = byteValue;
			for(INT32 i=0;i<0x4000;i++) CalcCol(i);
			break;

		case 0x1C0009:
		case 0x1C000D:
		case 0x1D0007:
		case 0x1D0009:
		case 0x1D0011:
		case 0x1D0013:
		case 0x1D0015:
		case 0x1D0017:	// NOP 
			break;
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
		
	}
}

void __fastcall shadfrceWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x1C0000: bg0scrollx = wordValue & 0x1FF; break;
		case 0x1C0002: bg0scrolly = wordValue & 0x1FF; break;
		case 0x1C0004: bg1scrollx = wordValue & 0x1FF; break;
		case 0x1C0006: bg1scrolly = wordValue & 0x1FF; break;
		
		case 0x1D000D:
			bprintf(PRINT_NORMAL, _T("Brightness set to %04x\n"), wordValue);
			//nBrightness = byteValue;
			break;
		case 0x1D0016:
			// wait v-blink dip change call back ???
			//bprintf(PRINT_NORMAL, _T("v-blink clear at PC 0x%06x\n"), SekGetPC(0) );
			break;
			
		case 0x1D0000:	// NOP
		case 0x1D0002:
		case 0x1D0006:
		case 0x1D0008:
			break;
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);

	}
}

/*
void __fastcall shadfrceWriteBytePalette(UINT32 sekAddress, UINT8 byteValue)
{
	//CavePalWriteByte(sekAddress & 0xFFFF, byteValue);
	bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to palette %x\n"), byteValue, sekAddress);
}
*/

void __fastcall shadfrceWriteWordPalette(UINT32 sekAddress, UINT16 wordValue)
{
	sekAddress &= 0x7FFF;
	sekAddress >>= 1;
	RamPal[sekAddress] = wordValue;
	CalcCol(sekAddress);
}

UINT8 __fastcall shadfrceZRead(UINT16 a)
{
	switch (a) {
	case 0xC801:	// YM2151_status_port_0_r
		//bprintf(PRINT_NORMAL, _T("YM2151_status_port_0_r\n"));
		return BurnYM2151ReadStatus();
	case 0xD800:	// OKIM6295_status_0_r
		//bprintf(PRINT_NORMAL, _T("OKIM6295_status_0_r\n"));
		return MSM6295ReadStatus(0);
	case 0xE000:	// soundlatch_r
		//bprintf(PRINT_NORMAL, _T("Soundlatch_r\n"));
		return nSoundlatch;
//	default:
//		bprintf(PRINT_NORMAL, _T("Z80 address %04X read.\n"), a);
	}
	return 0;
}

void __fastcall shadfrceZWrite(UINT16 a, UINT8 d)
{
	switch (a) {
	case 0xC800:	// YM2151_register_port_0_w
		//bprintf(PRINT_NORMAL, _T("BurnYM2151SelectRegister(%02x)\n"), d);
		BurnYM2151SelectRegister(d);
		break;
	case 0xC801:	// YM2151_data_port_0_w
		//bprintf(PRINT_NORMAL, _T("BurnYM2151WriteRegister(%02x)\n"), d);
		BurnYM2151WriteRegister(d);
		break;
	case 0xD800:	// OKIM6295_data_0_w
		//bprintf(PRINT_NORMAL, _T("MSM6295Command(%02x)\n"), d);
		MSM6295Command(0, d);
		break;
	case 0xE800:	// oki_bankswitch_w
		bprintf(PRINT_NORMAL, _T("oki_bankswitch_w(%02X)\n"), d);
		break;
//	default:
//		bprintf(PRINT_NORMAL, _T("Z80 address %04X -> %02X.\n"), a, d);
	}
}

static void shadfrceYM2151IRQHandler(INT32 nStatus)
{
	if (nStatus) {
		//ZetRaiseIrq(255);
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
		ZetRun(0x0400);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvDoReset()
{
	SekOpen(0);
    SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
	SekReset();
	SekClose();
	
	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	BurnYM2151Reset();

	return 0;
}

static INT32 loadDecodeGfx01()
{
/*
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
*/	
	UINT8 *buf = NULL;
	
	if ((buf = (UINT8*)BurnMalloc(0x20000)) == NULL) {
	    return 1;
    }	
	
	memset(buf, 0, 0x20000);
	BurnLoadRom(buf,  5, 1);	
	
	UINT8 *tmp = buf;
	UINT8 *pgfx = RomGfx01;
	
	for (INT32 i=0; i<(0x20000/32); i++) {
		for( INT32 y=0;y<8;y++) {
			
			pgfx[0] = (((tmp[ 0]>>0)&1)<<0) | (((tmp[ 0]>>2)&1)<<1) | (((tmp[ 0]>>4)&1)<<2) | (((tmp[ 0]>>6)&1)<<3);
			pgfx[1] = (((tmp[ 0]>>1)&1)<<0) | (((tmp[ 0]>>3)&1)<<1) | (((tmp[ 0]>>5)&1)<<2) | (((tmp[ 0]>>7)&1)<<3);
			pgfx[2] = (((tmp[ 8]>>0)&1)<<0) | (((tmp[ 8]>>2)&1)<<1) | (((tmp[ 8]>>4)&1)<<2) | (((tmp[ 8]>>6)&1)<<3);
			pgfx[3] = (((tmp[ 8]>>1)&1)<<0) | (((tmp[ 8]>>3)&1)<<1) | (((tmp[ 8]>>5)&1)<<2) | (((tmp[ 8]>>7)&1)<<3);
			pgfx[4] = (((tmp[16]>>0)&1)<<0) | (((tmp[16]>>2)&1)<<1) | (((tmp[16]>>4)&1)<<2) | (((tmp[16]>>6)&1)<<3);
			pgfx[5] = (((tmp[16]>>1)&1)<<0) | (((tmp[16]>>3)&1)<<1) | (((tmp[16]>>5)&1)<<2) | (((tmp[16]>>7)&1)<<3);
			pgfx[6] = (((tmp[24]>>0)&1)<<0) | (((tmp[24]>>2)&1)<<1) | (((tmp[24]>>4)&1)<<2) | (((tmp[24]>>6)&1)<<3);
			pgfx[7] = (((tmp[24]>>1)&1)<<0) | (((tmp[24]>>3)&1)<<1) | (((tmp[24]>>5)&1)<<2) | (((tmp[24]>>7)&1)<<3);
					
			tmp += 1;
			pgfx += 8;
		}
		tmp += 24;
	}
	
	BurnFree(buf);
	return 0;
}

static INT32 loadDecodeGfx02()
{
/*
	16,16,
	RGN_FRAC(1,5),
	5,
	{ 0x800000*8, 0x600000*8, 0x400000*8, 0x200000*8, 0x000000*8 },
	{ 0,1,2,3,4,5,6,7,16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*16
*/	
	UINT8 *buf = NULL;
	
	if ((buf = (UINT8*)BurnMalloc(0xA00000)) == NULL) {
	    return 1;
    }	
	memset(buf, 0, 0xA00000);	
	
	BurnLoadRom(buf + 0x000000,  6, 1);
	BurnLoadRom(buf + 0x200000,  7, 1);
	BurnLoadRom(buf + 0x400000,  8, 1);
	BurnLoadRom(buf + 0x600000,  9, 1);
	BurnLoadRom(buf + 0x800000, 10, 1);
	
	UINT8 *tmp1 = buf;
	UINT8 *tmp2 = tmp1 + 0x200000;
	UINT8 *tmp3 = tmp2 + 0x200000;
	UINT8 *tmp4 = tmp3 + 0x200000;
	UINT8 *tmp5 = tmp4 + 0x200000;
	UINT8 *pgfx = RomGfx02;

//	TODO: be lazy to research how BurnProgresser work, so ...

//	BurnSetProgressRange(1.0);
	
	for (INT32 i=0; i<(0x200000/32); i++) {
		for( INT32 y=0;y<16;y++) {
			
			for(INT32 x=0;x<8;x++) {
				pgfx[(7-x)+0] =	(((tmp1[ 0] >> x) & 0x01) << 0) | 
								(((tmp2[ 0] >> x) & 0x01) << 1) | 
								(((tmp3[ 0] >> x) & 0x01) << 2) | 
								(((tmp4[ 0] >> x) & 0x01) << 3) | 
								(((tmp5[ 0] >> x) & 0x01) << 4);
				pgfx[(7-x)+8] =	(((tmp1[16] >> x) & 0x01) << 0) | 
								(((tmp2[16] >> x) & 0x01) << 1) | 
								(((tmp3[16] >> x) & 0x01) << 2) | 
								(((tmp4[16] >> x) & 0x01) << 3) | 
								(((tmp5[16] >> x) & 0x01) << 4);
			}
			tmp1 += 1;
			tmp2 += 1;
			tmp3 += 1;
			tmp4 += 1;
			tmp5 += 1;
			pgfx += 16;
		}
		
//		if ((i & 0xFFF) == 0)
//			BurnUpdateProgress( 1.0 / 16, i ? NULL : _T("Decodeing graphics..."), 1);
		
		tmp1 += 16;
		tmp2 += 16;
		tmp3 += 16;
		tmp4 += 16;
		tmp5 += 16;
	}

	BurnFree(buf);
	return 0;
}

static INT32 loadDecodeGfx03()
{
/*
	16,16,
	RGN_FRAC(1,3),
	6,
	{ 0x000000*8+8, 0x000000*8+0, 0x100000*8+8, 0x100000*8+0, 0x200000*8+8, 0x200000*8+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,16*16+0,16*16+1,16*16+2,16*16+3,16*16+4,16*16+5,16*16+6,16*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	64*8
*/	
	UINT8 *buf = NULL;
	
	if ((buf = (UINT8*)BurnMalloc(0x300000)) == NULL) {
	    return 1;
    }	
	memset(buf, 0, 0x300000);	
	
	BurnLoadRom(buf + 0x000000, 11, 1);
	BurnLoadRom(buf + 0x100000, 12, 1);
	BurnLoadRom(buf + 0x200000, 13, 1);
	
	UINT8 *tmp1 = buf;
	UINT8 *tmp2 = tmp1 + 0x100000;
	UINT8 *tmp3 = tmp2 + 0x100000;	
	UINT8 *pgfx = RomGfx03;
	
	for (INT32 i=0; i<(0x100000/64); i++) {
		for( INT32 y=0;y<16;y++) {
			
			for(INT32 x=0;x<8;x++) {
				pgfx[(7-x)+0] =	(((tmp3[ 0] >> x) & 0x01) << 0) | (((tmp3[ 1] >> x) & 0x01) << 1) | 
								(((tmp2[ 0] >> x) & 0x01) << 2) | (((tmp2[ 1] >> x) & 0x01) << 3) | 
								(((tmp1[ 0] >> x) & 0x01) << 4) | (((tmp1[ 1] >> x) & 0x01) << 5);
				pgfx[(7-x)+8] =	(((tmp3[32] >> x) & 0x01) << 0) | (((tmp3[33] >> x) & 0x01) << 1) | 
								(((tmp2[32] >> x) & 0x01) << 2) | (((tmp2[33] >> x) & 0x01) << 3) | 
								(((tmp1[32] >> x) & 0x01) << 4) | (((tmp1[33] >> x) & 0x01) << 5);
			}
			tmp1 += 2;
			tmp2 += 2;
			tmp3 += 2;
			pgfx += 16;
		}
		tmp1 += 32;
		tmp2 += 32;
		tmp3 += 32;
	}
	
	BurnFree(buf);
	
	return 0;
}

static INT32 shadfrceInit()
{
	INT32 nRet;
	
	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();	
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000001, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x080000, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x080001, 3, 2); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(RomZ80 + 0x000000, 4, 1); if (nRet != 0) return 1;
	
	loadDecodeGfx01();
	loadDecodeGfx02();
	loadDecodeGfx03();

	BurnLoadRom(MSM6295ROM, 14, 1);	

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM

		SekMapMemory((UINT8 *)RamBg00,
									0x100000, 0x101FFF, SM_RAM);	// b ground 0
		SekMapMemory((UINT8 *)RamBg01,		
									0x102000, 0x103FFF, SM_RAM);	// b ground 1
		SekMapMemory((UINT8 *)RamFg,			
									0x140000, 0x141FFF, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamSpr,
									0x142000, 0x143FFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,
									0x180000, 0x187FFF, SM_ROM);	// palette
		SekMapMemory(Ram68K,		0x1F0000, 0x1FFFFF, SM_RAM);	// 68K RAM

		SekMapHandler(1,			0x180000, 0x187FFF, SM_WRITE);
		
		
		SekSetReadWordHandler(0, shadfrceReadWord);
		SekSetReadByteHandler(0, shadfrceReadByte);
		SekSetWriteWordHandler(0, shadfrceWriteWord);
		SekSetWriteByteHandler(0, shadfrceWriteByte);
		
		//SekSetWriteByteHandler(1, shadfrceWriteBytePalette);
		SekSetWriteWordHandler(1, shadfrceWriteWordPalette);

		SekClose();
	}

	{
		ZetInit(0);
		ZetOpen(0);
	
		ZetSetReadHandler(shadfrceZRead);
		ZetSetWriteHandler(shadfrceZWrite);
		//ZetSetInHandler(shadfrceZIn);
		//ZetSetOutHandler(shadfrceZOut);
	
		// ROM bank 1
		ZetMapArea(0x0000, 0xBFFF, 0, RomZ80);
		ZetMapArea(0x0000, 0xBFFF, 2, RomZ80);

		// RAM
		ZetMapArea(0xC000, 0xC7FF, 0, RamZ80);
		ZetMapArea(0xC000, 0xC7FF, 1, RamZ80);
		ZetMapArea(0xC000, 0xC7FF, 2, RamZ80);

		ZetMapArea(0xF000, 0xFFFF, 0, RamZ80+0x0800);
		ZetMapArea(0xF000, 0xFFFF, 1, RamZ80+0x0800);
		ZetMapArea(0xF000, 0xFFFF, 2, RamZ80+0x0800);
		
		ZetMemEnd();
		ZetClose();
	}
	
	BurnYM2151Init(3579545);		// 3.5795 MHz
	YM2151SetIrqHandler(0, &shadfrceYM2151IRQHandler);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.50, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.50, BURN_SND_ROUTE_RIGHT);
	MSM6295Init(0, 12000, 1);		// 12.000 KHz
	MSM6295SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);
	nZ80Cycles = 3579545 * 100 / nBurnFPS;

	DrvDoReset();
	return 0;
}

static INT32 shadfrceExit()
{
	MSM6295Exit(0);
	BurnYM2151Exit();
	
	SekExit();
	ZetExit();
	
	BurnFree(Mem);
	
	return 0;
}

#define	TILE_BG0(x)				\
	if (d[x]) { p[x]=pal[d[x]|c]; pp[x]=2; }

#define	TILE_BG0_FLIPX(x)		\
	if (d[x]) { p[15-x]=pal[d[x]|c]; pp[15-x]=2; }

#define	TILE_BG0_E(z)			\
	if (d[z]&&(x+z)>=0 && (x+z)<320) { p[z]=pal[d[z]|c]; pp[z]=2; }

#define	TILE_BG0_FLIPX_E(z)		\
	if (d[z]&&(x+15-z)>=0&&(x+15-z)<320) { p[15-z]=pal[d[z]|c]; pp[15-z]=2; }

#define	TILE_BG0_LINE			\
	TILE_BG0( 0)				\
	TILE_BG0( 1)				\
	TILE_BG0( 2)				\
	TILE_BG0( 3)				\
	TILE_BG0( 4)				\
	TILE_BG0( 5)				\
	TILE_BG0( 6)				\
	TILE_BG0( 7)				\
	TILE_BG0( 8)				\
	TILE_BG0( 9)				\
	TILE_BG0(10)				\
	TILE_BG0(11)				\
	TILE_BG0(12)				\
	TILE_BG0(13)				\
	TILE_BG0(14)				\
	TILE_BG0(15)

#define	TILE_BG0_FLIPX_LINE		\
	TILE_BG0_FLIPX( 0)			\
	TILE_BG0_FLIPX( 1)			\
	TILE_BG0_FLIPX( 2)			\
	TILE_BG0_FLIPX( 3)			\
	TILE_BG0_FLIPX( 4)			\
	TILE_BG0_FLIPX( 5)			\
	TILE_BG0_FLIPX( 6)			\
	TILE_BG0_FLIPX( 7)			\
	TILE_BG0_FLIPX( 8)			\
	TILE_BG0_FLIPX( 9)			\
	TILE_BG0_FLIPX(10)			\
	TILE_BG0_FLIPX(11)			\
	TILE_BG0_FLIPX(12)			\
	TILE_BG0_FLIPX(13)			\
	TILE_BG0_FLIPX(14)			\
	TILE_BG0_FLIPX(15)

#define	TILE_BG0_LINE_E						\
	if (((y+k)>=0) && ((y+k)<256)) {		\
		TILE_BG0_E( 0)						\
		TILE_BG0_E( 1)						\
		TILE_BG0_E( 2)						\
		TILE_BG0_E( 3)						\
		TILE_BG0_E( 4)						\
		TILE_BG0_E( 5)						\
		TILE_BG0_E( 6)						\
		TILE_BG0_E( 7)						\
		TILE_BG0_E( 8)						\
		TILE_BG0_E( 9)						\
		TILE_BG0_E(10)						\
		TILE_BG0_E(11)						\
		TILE_BG0_E(12)						\
		TILE_BG0_E(13)						\
		TILE_BG0_E(14)						\
		TILE_BG0_E(15)						\
	}

#define	TILE_BG0_FLIPX_LINE_E				\
	if (((y+k)>=0) && ((y+k)<256)) {		\
		TILE_BG0_FLIPX_E( 0)				\
		TILE_BG0_FLIPX_E( 1)				\
		TILE_BG0_FLIPX_E( 2)				\
		TILE_BG0_FLIPX_E( 3)				\
		TILE_BG0_FLIPX_E( 4)				\
		TILE_BG0_FLIPX_E( 5)				\
		TILE_BG0_FLIPX_E( 6)				\
		TILE_BG0_FLIPX_E( 7)				\
		TILE_BG0_FLIPX_E( 8)				\
		TILE_BG0_FLIPX_E( 9)				\
		TILE_BG0_FLIPX_E(10)				\
		TILE_BG0_FLIPX_E(11)				\
		TILE_BG0_FLIPX_E(12)				\
		TILE_BG0_FLIPX_E(13)				\
		TILE_BG0_FLIPX_E(14)				\
		TILE_BG0_FLIPX_E(15)				\
	}

static void tileBackground_0()
{
	/* ---- ----  YXcc CCCC  --TT TTTT TTTT TTTT */
	
	UINT16 * pal = RamCurPal + 0x2000;
	INT32 offs, mx, my, x, y;
	
	mx = -1;
	my = 0;
	for (offs = 0; offs < 32*32*2; offs+=2) {
		mx++;
		if (mx == 32) {
			mx = 0;
			my++;
		}

		x = mx * 16 - bg0scrollx;
		if (x <= (320-512)) x += 512;
		
		y = my * 16 - bg0scrolly;
		if (y <= (256-512)) y += 512;
		
		if ( x<=-16 || x>=320 || y<=-16 || y>= 256 ) 
			continue;
		else 
		if ( x >=0 && x <= (320-16) && y >= 0 && y <= (256-16)) {

			UINT32 tileno = RamBg00[offs+1] & 0x3FFF;
			if (tileno == 0) continue;
			
 			UINT32 c = (RamBg00[offs] & 0x1F);
 			if (c & 0x10) c ^= 0x30;	// skip hole 
 			c <<= 6;
 			
 			UINT16 * p = (UINT16 *) pBurnDraw + y * 320 + x;
			UINT8 *d = RomGfx03 + (tileno << 8);
			
			UINT8 * pp = RamPri + y * 320 + x;
			
			if (RamBg00[offs] & 0x0080) {
				p += 320 * 15;
				pp += 320 * 15;
				if (RamBg00[offs] & 0x0040) {
	 				for (INT32 k=0;k<16;k++) {
	 					
		 				TILE_BG0_FLIPX_LINE
		
		 				d += 16;
		 				p -= 320;
		 				pp -= 320;
		 			}
	 			} else {
					for (INT32 k=0;k<16;k++) {
		 				
		 				TILE_BG0_LINE
		
		 				d += 16;
		 				p -= 320;
		 				pp -= 320;
		 			}
	 			}
 			} else {
 				if (RamBg00[offs] & 0x0040) {
	 				for (INT32 k=0;k<16;k++) {
	 					
		 				TILE_BG0_FLIPX_LINE
		
		 				d += 16;
		 				p += 320;
		 				pp += 320;
		 			}
	 			} else {
					for (INT32 k=0;k<16;k++) {
						
		 				TILE_BG0_LINE

		 				d += 16;
		 				p += 320;
		 				pp += 320;
		 			}
	 			}			
 			}
 			
		} else {

			UINT32 tileno = RamBg00[offs+1] & 0x3FFF;
			if (tileno == 0) continue;
			
 			UINT32 c = (RamBg00[offs] & 0x1F);
 			if (c & 0x10) c ^= 0x30;	// skip hole
 			c <<= 6;
 			
 			UINT16 * p = (UINT16 *) pBurnDraw + y * 320 + x;
			UINT8 *d = RomGfx03 + (tileno << 8);
			UINT8 * pp = RamPri + y * 320 + x;
			
			if (RamBg00[offs] & 0x0080) {
				p += 320 * 15;
				pp += 320 * 15;
				if (RamBg00[offs] & 0x0040) {
	 				for (INT32 k=0;k<16;k++) {
	 					
		 				TILE_BG0_FLIPX_LINE_E
		 						
		 				d += 16;
		 				p -= 320;
		 				pp -= 320;
		 			}
	 			} else {
					for (INT32 k=0;k<16;k++) {
		 				
		 				TILE_BG0_LINE_E
		
		 				d += 16;
		 				p -= 320;
		 				pp -= 320;
		 			}
	 			}
 			} else {
 				if (RamBg00[offs] & 0x0040) {
	 				for (INT32 k=0;k<16;k++) {
	 					
		 				TILE_BG0_FLIPX_LINE_E
		
		 				d += 16;
		 				p += 320;
		 				pp += 320;
		 			}
	 			} else {
					for (INT32 k=0;k<16;k++) {
						
		 				TILE_BG0_LINE_E

		 				d += 16;
		 				p += 320;
		 				pp += 320;
		 			}
	 			}			
 			}
			
		}
	}
	
}

static void tileBackground_1()
{
	/* cccc TTTT TTTT TTTT */
	
	UINT16 * pal = RamCurPal + 0x2000;
	INT32 offs, mx, my, x, y;
	
	mx = -1;
	my = 0;
	for (offs = 0; offs < 32*32; offs++) {
		mx++;
		if (mx == 32) {
			mx = 0;
			my++;
		}

		x = mx * 16 - bg1scrollx;
		if (x <= (320-512)) x += 512;
		
		y = my * 16 - bg1scrolly;
		if (y <= (256-512)) y += 512;
		
		if ( x<=-16 || x>=320 || y<=-16 || y>= 256 ) 
			continue;
		else
		if ( x >=0 && x <= (320-16) && y >= 0 && y <= (256-16)) {

			UINT32 tileno = RamBg01[offs] & 0x0FFF;
 			UINT32 c = ((RamBg01[offs] & 0xF000) >> 6) + (64 << 6);
 			UINT16 * p = (UINT16 *) pBurnDraw + y * 320 + x;
			UINT8 *d = RomGfx03 + (tileno << 8);
			
			for (INT32 k=0;k<16;k++) {
 				p[ 0] = pal[ d[ 0] | c ];
 				p[ 1] = pal[ d[ 1] | c ];
 				p[ 2] = pal[ d[ 2] | c ];
 				p[ 3] = pal[ d[ 3] | c ];
 				p[ 4] = pal[ d[ 4] | c ];
 				p[ 5] = pal[ d[ 5] | c ];
 				p[ 6] = pal[ d[ 6] | c ];
 				p[ 7] = pal[ d[ 7] | c ];
 				p[ 8] = pal[ d[ 8] | c ];
 				p[ 9] = pal[ d[ 9] | c ];
 				p[10] = pal[ d[10] | c ];
 				p[11] = pal[ d[11] | c ];
 				p[12] = pal[ d[12] | c ];
 				p[13] = pal[ d[13] | c ];
 				p[14] = pal[ d[14] | c ];
 				p[15] = pal[ d[15] | c ];

 				d += 16;
 				p += 320;
 			}
		} else {
			UINT32 tileno = RamBg01[offs] & 0x0FFF;
 			UINT32 c = ((RamBg01[offs] & 0xF000) >> 6) + (64 << 6);
 			UINT16 * p = (UINT16 *) pBurnDraw + y * 320 + x;
			UINT8 *d = RomGfx03 + (tileno << 8);
			
			for (INT32 k=0;k<16;k++) {
				if ( (y+k)>=0 && (y+k)<256 ) {
	 				if ((x +  0) >= 0 && (x +  0)<320) p[ 0] = pal[ d[ 0] | c ];
	 				if ((x +  1) >= 0 && (x +  1)<320) p[ 1] = pal[ d[ 1] | c ];
	 				if ((x +  2) >= 0 && (x +  2)<320) p[ 2] = pal[ d[ 2] | c ];
	 				if ((x +  3) >= 0 && (x +  3)<320) p[ 3] = pal[ d[ 3] | c ];
	 				if ((x +  4) >= 0 && (x +  4)<320) p[ 4] = pal[ d[ 4] | c ];
	 				if ((x +  5) >= 0 && (x +  5)<320) p[ 5] = pal[ d[ 5] | c ];
	 				if ((x +  6) >= 0 && (x +  6)<320) p[ 6] = pal[ d[ 6] | c ];
	 				if ((x +  7) >= 0 && (x +  7)<320) p[ 7] = pal[ d[ 7] | c ];
	 				if ((x +  8) >= 0 && (x +  8)<320) p[ 8] = pal[ d[ 8] | c ];
	 				if ((x +  9) >= 0 && (x +  9)<320) p[ 9] = pal[ d[ 9] | c ];
	 				if ((x + 10) >= 0 && (x + 10)<320) p[10] = pal[ d[10] | c ];
	 				if ((x + 11) >= 0 && (x + 11)<320) p[11] = pal[ d[11] | c ];
	 				if ((x + 12) >= 0 && (x + 12)<320) p[12] = pal[ d[12] | c ];
	 				if ((x + 13) >= 0 && (x + 13)<320) p[13] = pal[ d[13] | c ];
	 				if ((x + 14) >= 0 && (x + 14)<320) p[14] = pal[ d[14] | c ];
	 				if ((x + 15) >= 0 && (x + 15)<320) p[15] = pal[ d[15] | c ];
	 			}
 				d += 16;
 				p += 320;
 			}
		}
	}
}

static void tileForeground()
{
	/* ---- ----  tttt tttt  ---- ----  pppp TTTT */
	
	UINT16 * pal = RamCurPal;
	INT32 offs, mx, my, x, y;
	
	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*32*2; offs+=2) {
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}

		x = mx * 8;
		y = my * 8;
		
		if ( x<=-8 || x>=320 || y<=-8 || y>= 256 ) 
			continue;
		else
		if ( x >=0 && x < (320-8) && y >= 0 && y < (256-8)) {

			UINT32 tileno = (RamFg[offs] & 0x00FF) | ((RamFg[offs+1] & 0x000F) << 8);
			if (tileno == 0) continue;
			
 			UINT32 c = (RamFg[offs+1] & 0x00F0) << 2;
 			UINT16 * p = (UINT16 *) pBurnDraw + y * 320 + x;
			UINT8 *d = RomGfx01 + (tileno << 6);
			
			for (INT32 k=0;k<8;k++) {
 				if (d[0]) p[0] = pal[d[0]|c];
 				if (d[1]) p[1] = pal[d[1]|c];
 				if (d[2]) p[2] = pal[d[2]|c];
 				if (d[3]) p[3] = pal[d[3]|c];
 				if (d[4]) p[4] = pal[d[4]|c];
 				if (d[5]) p[5] = pal[d[5]|c];
 				if (d[6]) p[6] = pal[d[6]|c];
 				if (d[7]) p[7] = pal[d[7]|c];

 				d += 8;
 				p += 320;
 			}
		} else {

			UINT32 tileno = (RamFg[offs] & 0x00FF) | ((RamFg[offs+1] & 0x000F) << 8);
			if (tileno == 0) continue;
			
 			UINT32 c = (RamFg[offs+1] & 0x00F0) << 2;
 			UINT16 * p = (UINT16 *) pBurnDraw + y * 320 + x;
			UINT8 *d = RomGfx01 + (tileno << 6);
			
			for (INT32 k=0;k<8;k++) {
				if ( (y+k)>=0 && (y+k)<256 ) {
	 				if ((x + 0) >= 0 && (x + 0)<320) p[0] = pal[ d[0] | c ];
	 				if ((x + 1) >= 0 && (x + 1)<320) p[1] = pal[ d[1] | c ];
	 				if ((x + 2) >= 0 && (x + 2)<320) p[2] = pal[ d[2] | c ];
	 				if ((x + 3) >= 0 && (x + 3)<320) p[3] = pal[ d[3] | c ];
	 				if ((x + 4) >= 0 && (x + 4)<320) p[4] = pal[ d[4] | c ];
	 				if ((x + 5) >= 0 && (x + 5)<320) p[5] = pal[ d[5] | c ];
	 				if ((x + 6) >= 0 && (x + 6)<320) p[6] = pal[ d[6] | c ];
	 				if ((x + 7) >= 0 && (x + 7)<320) p[7] = pal[ d[7] | c ];
	 			}
 				d += 8;
 				p += 320;
 			}
		}
	}
}

#define TILE_SPR_NORMAL(x)				\
	if (q[x]) { if((pp[x]&(0x05|pri))==0) p[x]=pal[q[x]|color]; pp[x]|=(1<<pri); }

#define TILE_SPR_FLIP_X(x)				\
	if (q[x]) { if((pp[15-x]&(0x05|pri))==0) p[15-x]=pal[q[x]|color]; pp[15-x]|=(1<<pri); }

#define TILE_SPR_NORMAL_E(x)			\
	if (q[x]&&((sx+x)>=0)&&((sx+x)<320)) { if((pp[x]&(0x05|pri))==0) p[x]=pal[q[x]|color]; pp[x]|=(1<<pri); }

#define TILE_SPR_FLIP_X_E(x)			\
	if (q[x]&&((sx+15-x)>=0)&&((sx+15-x)<320)) { if((pp[15-x]&(0x05|pri))==0) p[15-x]=pal[q[x]|color]; pp[15-x]|=(1<<pri); }

#define TILE_SPR_NORMAL_LINE			\
	TILE_SPR_NORMAL( 0)					\
	TILE_SPR_NORMAL( 1)					\
	TILE_SPR_NORMAL( 2)					\
	TILE_SPR_NORMAL( 3)					\
	TILE_SPR_NORMAL( 4)					\
	TILE_SPR_NORMAL( 5)					\
	TILE_SPR_NORMAL( 6)					\
	TILE_SPR_NORMAL( 7)					\
	TILE_SPR_NORMAL( 8)					\
	TILE_SPR_NORMAL( 9)					\
	TILE_SPR_NORMAL(10)					\
	TILE_SPR_NORMAL(11)					\
	TILE_SPR_NORMAL(12)					\
	TILE_SPR_NORMAL(13)					\
	TILE_SPR_NORMAL(14)					\
	TILE_SPR_NORMAL(15)

#define TILE_SPR_FLIP_X_LINE			\
	TILE_SPR_FLIP_X( 0)					\
	TILE_SPR_FLIP_X( 1)					\
	TILE_SPR_FLIP_X( 2)					\
	TILE_SPR_FLIP_X( 3)					\
	TILE_SPR_FLIP_X( 4)					\
	TILE_SPR_FLIP_X( 5)					\
	TILE_SPR_FLIP_X( 6)					\
	TILE_SPR_FLIP_X( 7)					\
	TILE_SPR_FLIP_X( 8)					\
	TILE_SPR_FLIP_X( 9)					\
	TILE_SPR_FLIP_X(10)					\
	TILE_SPR_FLIP_X(11)					\
	TILE_SPR_FLIP_X(12)					\
	TILE_SPR_FLIP_X(13)					\
	TILE_SPR_FLIP_X(14)					\
	TILE_SPR_FLIP_X(15)

#define TILE_SPR_NORMAL_LINE_E			\
	TILE_SPR_NORMAL_E( 0)				\
	TILE_SPR_NORMAL_E( 1)				\
	TILE_SPR_NORMAL_E( 2)				\
	TILE_SPR_NORMAL_E( 3)				\
	TILE_SPR_NORMAL_E( 4)				\
	TILE_SPR_NORMAL_E( 5)				\
	TILE_SPR_NORMAL_E( 6)				\
	TILE_SPR_NORMAL_E( 7)				\
	TILE_SPR_NORMAL_E( 8)				\
	TILE_SPR_NORMAL_E( 9)				\
	TILE_SPR_NORMAL_E(10)				\
	TILE_SPR_NORMAL_E(11)				\
	TILE_SPR_NORMAL_E(12)				\
	TILE_SPR_NORMAL_E(13)				\
	TILE_SPR_NORMAL_E(14)				\
	TILE_SPR_NORMAL_E(15)

#define TILE_SPR_FLIP_X_LINE_E			\
	TILE_SPR_FLIP_X_E( 0)				\
	TILE_SPR_FLIP_X_E( 1)				\
	TILE_SPR_FLIP_X_E( 2)				\
	TILE_SPR_FLIP_X_E( 3)				\
	TILE_SPR_FLIP_X_E( 4)				\
	TILE_SPR_FLIP_X_E( 5)				\
	TILE_SPR_FLIP_X_E( 6)				\
	TILE_SPR_FLIP_X_E( 7)				\
	TILE_SPR_FLIP_X_E( 8)				\
	TILE_SPR_FLIP_X_E( 9)				\
	TILE_SPR_FLIP_X_E(10)				\
	TILE_SPR_FLIP_X_E(11)				\
	TILE_SPR_FLIP_X_E(12)				\
	TILE_SPR_FLIP_X_E(13)				\
	TILE_SPR_FLIP_X_E(14)				\
	TILE_SPR_FLIP_X_E(15)

static void pdrawgfx(UINT32 code,UINT32 color,INT32 flipx,INT32 flipy,INT32 sx,INT32 sy,INT32 pri)
{
	UINT16 * p	= (UINT16 *) pBurnDraw;
	UINT8 * pp = RamPri;
	UINT8 * q	= RomGfx02 + (code << 8);
	UINT16 *pal	= RamCurPal + 0x1000;

	p += sy * 320 + sx;
	pp += sy * 320 + sx;
		
	if (sx < 0 || sx >= (320-16) || sy < 0 || sy >= (256-16) ) {
		
		if ((sx <= -16) || (sx >= 320) || (sy <= -16) || (sy >= 256))
			return;
			
		if (flipy) {
		
			p += 320 * 15;
			pp += 320 * 15;
			
			if (flipx) {
			
				for (INT32 i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<256)) {
						
						TILE_SPR_FLIP_X_LINE_E
						
					}
					p -= 320;
					pp -= 320;
					q += 16;
				}	
			
			} else {
	
				for (INT32 i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<256)) {
						
						TILE_SPR_NORMAL_LINE_E

					}
					p -= 320;
					pp -= 320;
					q += 16;
				}		
			}
			
		} else {
			
			if (flipx) {
			
				for (INT32 i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<256)) {
						
						TILE_SPR_FLIP_X_LINE_E
						
					}
					p += 320;
					pp += 320;
					q += 16;
				}		
			
			} else {
	
				for (INT32 i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<256)) {

						TILE_SPR_NORMAL_LINE_E

					}
					p += 320;
					pp += 320;
					q += 16;
				}	
	
			}
			
		}
		
		return;
	}

	if (flipy) {
		
		p += 320 * 15;
		pp += 320 * 15;
		
		if (flipx) {
		
			for (INT32 i=0;i<16;i++) {
				
				TILE_SPR_FLIP_X_LINE
	
				p -= 320;
				pp -= 320;
				q += 16;
			}	
		
		} else {

			for (INT32 i=0;i<16;i++) {
				
				TILE_SPR_NORMAL_LINE
	
				p -= 320;
				pp -= 320;
				q += 16;
			}		
		}
		
	} else {
		
		if (flipx) {
		
			for (INT32 i=0;i<16;i++) {
				
				TILE_SPR_FLIP_X_LINE
	
				p += 320;
				pp += 320;
				q += 16;
			}		
		
		} else {

			for (INT32 i=0;i<16;i++) {
				
				TILE_SPR_NORMAL_LINE
	
				p += 320;
				pp += 320;
				q += 16;
			}	

		}
		
	}

}

static void drawSprites()
{
	/* | ---- ---- hhhf Fe-Y | ---- ---- yyyy yyyy | ---- ---- TTTT TTTT | ---- ---- tttt tttt |
       | ---- ---- -pCc cccX | ---- ---- xxxx xxxx | ---- ---- ---- ---- | ---- ---- ---- ---- | */

	UINT16 * finish = RamSpr;
	UINT16 * source = finish + 0x2000/2 - 8;
	INT32 hcount;
	
	while( source>=finish ) {
		INT32 ypos = 0x100 - (((source[0] & 0x0003) << 8) | (source[1] & 0x00ff));
		INT32 xpos = (((source[4] & 0x0001) << 8) | (source[5] & 0x00ff)) + 1;
		INT32 tile = ((source[2] & 0x00ff) << 8) | (source[3] & 0x00ff);
		INT32 height = (source[0] & 0x00e0) >> 5;
		INT32 enable = ((source[0] & 0x0004));
		INT32 flipx = ((source[0] & 0x0010) >> 4);
		INT32 flipy = ((source[0] & 0x0008) >> 3);
		INT32 pal = ((source[4] & 0x003e));
		INT32 pri_mask = (source[4] & 0x0040) ? 0x02 : 0x00;
			
//		if ( (1 << pri_mask) & nSpriteEnable) {
			
			if (pal & 0x20) pal ^= 0x60;	// skip hole
			pal <<= 5;
			
			height++;
			if (enable)	{
				for (hcount=0;hcount<height;hcount++) {
					pdrawgfx(tile+hcount,pal,flipx,flipy,xpos,ypos-hcount*16-16,pri_mask);
					pdrawgfx(tile+hcount,pal,flipx,flipy,xpos-0x200,ypos-hcount*16-16,pri_mask);
					pdrawgfx(tile+hcount,pal,flipx,flipy,xpos,ypos-hcount*16-16+0x200,pri_mask);
					pdrawgfx(tile+hcount,pal,flipx,flipy,xpos-0x200,ypos-hcount*16-16+0x200,pri_mask);
				}
			}
			
//		}
		source-=8;
	}

}

static void DrvDraw()
{
	memset( RamPri, 0, 320*256);

//	if (nBurnLayer & 2)
	tileBackground_1();
		
//	if (nBurnLayer & 4)
	tileBackground_0();
	
	drawSprites();
	
//	if (nBurnLayer & 8)
	tileForeground();
}

static INT32 shadfrceFrame()
{
	if (DrvReset)														// Reset machine
		DrvDoReset();
		
	if (bRecalcPalette) {
		for(INT32 i=0;i<0x4000;i++) CalcCol(i);
		bRecalcPalette = 0;
	}

	DrvInput[0] = 0x00;													// Joy1
	DrvInput[2] = 0x00;													// Joy2
	DrvInput[4] = 0x00;													// Buttons
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[2] |= (DrvJoy2[i] & 1) << i;
		DrvInput[4] |= (DrvButton[i] & 1) << i;
	}
	DrvInput[1] = (DrvInput[1] & 0xFC) | (DrvDipBtn[0] & 1) | ((DrvDipBtn[1] & 1) << 1); 

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);
	
/*	
	005344	rts
	005346	btst    #$2, $1d0026.l
	00534e	bne     5346
	005350	clr.w   $1d0016.l
	005356	btst    #$2, $1d0026.l
	00535e	beq     5356
	005360	movea.l #$142000, A0
	
	0053b2	dbra    D0, 53b0
	0053b6	btst    #$2, $1d0026.l
	0053be	bne     53b6
	0053c0	clr.w   $1d0016.l
	0053c6	btst    #$2, $1d0026.l
	0053ce	beq     53c6
	0053d0	jmp     (A6)
	
	it will check v-blink dip (in 1d0016.l)
	
 */			
 	
 	INT32 nSoundBufferPos = 0;
 	
	bVBlink = 1;
	SekRun(3500000 / 60);
	
	ZetRun(nZ80Cycles >> 2);
	if (pBurnSoundOut) {
		INT32 nSegmentLength = (nBurnSoundLen * 1 / 4) - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		BurnYM2151Render(pSoundBuf, nSegmentLength);
		MSM6295Render(0, pSoundBuf, nSegmentLength);
		nSoundBufferPos += nSegmentLength;
	}
	
	bVBlink = 0;
	SekRun(3500000 / 60);
	SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);

	ZetRun(nZ80Cycles >> 2);
	if (pBurnSoundOut) {
		INT32 nSegmentLength = (nBurnSoundLen * 2 / 4) - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		BurnYM2151Render(pSoundBuf, nSegmentLength);
		MSM6295Render(0, pSoundBuf, nSegmentLength);
		nSoundBufferPos += nSegmentLength;
	}

	bVBlink = 1;
	SekRun(3500000 / 60);
	
	ZetRun(nZ80Cycles >> 2);
	if (pBurnSoundOut) {
		INT32 nSegmentLength = (nBurnSoundLen * 3 / 4) - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		BurnYM2151Render(pSoundBuf, nSegmentLength);
		MSM6295Render(0, pSoundBuf, nSegmentLength);
		nSoundBufferPos += nSegmentLength;
	}
	
	
	bVBlink = 0;
	SekRun(3500000 / 60);
	SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
	
	ZetRun(nZ80Cycles >> 2);
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		BurnYM2151Render(pSoundBuf, nSegmentLength);
		MSM6295Render(0, pSoundBuf, nSegmentLength);
		nSoundBufferPos += nSegmentLength;
	}

/*
	ZetRun(nZ80Cycles);
	if (pBurnSoundOut) {
		BurnYM2151Render(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}
*/

	ZetClose();
	SekClose();
	
	if (pBurnDraw)
		DrvDraw();												// Draw screen if needed
	
	

	return 0;
}

static INT32 shadfrceScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin)						// Return minimum compatible version
		*pnMin =  0x029671;

	if (nAction & ACB_MEMORY_RAM) {								// Scan all memory, devices & variables
		memset(&ba, 0, sizeof(ba));
    	ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);										// Scan 68000 state
		ZetScan(nAction);

		MSM6295Scan(0, nAction);
		BurnYM2151Scan(nAction);
		
		SCAN_VAR(DrvInput);
		SCAN_VAR(nBrightness);
		SCAN_VAR(bg0scrollx);
		SCAN_VAR(bg0scrolly);
		SCAN_VAR(bg1scrollx);
		SCAN_VAR(bg1scrolly);
		SCAN_VAR(nSoundlatch);
	}
	
	if (nAction & ACB_WRITE) {
		// recalc palette and brightness
		for(INT32 i=0;i<0x4000;i++) CalcCol(i);
	}
	
	return 0;
}

struct BurnDriver BurnDrvShadfrce = {
	"shadfrce", NULL, NULL, NULL, "1993",
	"Shadow Force (US Version 2)\0", NULL, "Technos Japan", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, shadfrceRomInfo, shadfrceRomName, NULL, NULL, shadfrceInputInfo, shadfrceDIPInfo,
	shadfrceInit, shadfrceExit, shadfrceFrame, NULL, shadfrceScan, &bRecalcPalette, 0x8000,
	320, 256, 4, 3
};

struct BurnDriver BurnDrvShadfrcj = {
	"shadfrcej", "shadfrce", NULL, NULL, "1993",
	"Shadow Force (Japan Version 3)\0", NULL, "Technos Japan", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, shadfrcjRomInfo, shadfrcjRomName, NULL, NULL, shadfrceInputInfo, shadfrceDIPInfo,
	shadfrceInit, shadfrceExit, shadfrceFrame, NULL, shadfrceScan, &bRecalcPalette, 0x8000,
	320, 256, 4, 3
};

struct BurnDriver BurnDrvShadfrcjv2 = {
	"shadfrcejv2", "shadfrce", NULL, NULL, "1993",
	"Shadow Force (Japan Version 2)\0", NULL, "Technos Japan", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, shadfrcjv2RomInfo, shadfrcjv2RomName, NULL, NULL, shadfrceInputInfo, shadfrceDIPInfo,
	shadfrceInit, shadfrceExit, shadfrceFrame, NULL, shadfrceScan, &bRecalcPalette, 0x8000,
	320, 256, 4, 3
};
