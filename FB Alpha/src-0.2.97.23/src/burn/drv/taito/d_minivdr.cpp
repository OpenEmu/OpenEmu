// FB Alpha Minivader driver module
//Based on MAME Driver by Takahiro Nogi

#include "tiles_generic.h"
#include "zet.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvZ80RAM;

static UINT8 DrvJoy1[4];
static UINT8 DrvInputs[1];
static UINT8 DrvReset;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 3,	"p1 coin"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 2,	"p1 fire 1"},

	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"    },
};

STDINPUTINFO(Drv)

UINT8 __fastcall minivdr_read(UINT16 address)
{
	if (address == 0xe008) {
		return DrvInputs[0];
	}

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM	= Next; Next += 0x010000;

	AllRam		= Next;

	DrvZ80RAM	= Next; Next += 0x002000;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
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
		if (BurnLoadRom(DrvZ80ROM, 0, 1)) return 1;
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea (0x0000, 0x1fff, 0, DrvZ80ROM);
	ZetMapArea (0x0000, 0x1fff, 2, DrvZ80ROM);
	ZetMapArea (0xa000, 0xbfff, 0, DrvZ80RAM);
	ZetMapArea (0xa000, 0xbfff, 1, DrvZ80RAM);
	ZetMapArea (0xa000, 0xbfff, 2, DrvZ80RAM);
	ZetSetReadHandler(minivdr_read);
	ZetMemEnd();
	ZetClose();

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();

	BurnFree (AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	UINT32 DrvPalette[2];

	DrvPalette[0] = 0;
	DrvPalette[1] = BurnHighCol(0xff, 0xff, 0xff, 0);

	for (INT32 offs = 0x200; offs < 0x1e00; offs++)
	{
		INT32 sx = (offs << 3) & 0xf8;
		INT32 sy = ((offs >> 5) - 0x10) << 8;
		INT32 d = DrvZ80RAM[offs];

		for (INT32 i = 0; i < 8; i++, sx++)
		{
			pTransDraw[sx + sy] = (d >> (7 - i)) & 1;
		}
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = 0xff;
		for (INT32 i = 0; i < 4; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		}
	}

	ZetOpen(0);
	ZetRun(4000000 / 60);
	ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
	}

	return 0;
}


// Minivader

static struct BurnRomInfo minivadrRomDesc[] = {
	{ "d26-01.bin",	0x2000, 0xa96c823d, BRF_ESS | BRF_PRG }, //  Z80 code
};

STD_ROM_PICK(minivadr)
STD_ROM_FN(minivadr)

struct BurnDriver BurnDrvminivadr = {
	"minivadr", NULL, NULL, NULL, "1990",
	"Minivader\0", NULL, "Taito Corporation", "Minivader",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_TAITO_MISC, GBF_SHOOT, 0,
	NULL, minivadrRomInfo, minivadrRomName, NULL, NULL, DrvInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 224, 4, 3
};
