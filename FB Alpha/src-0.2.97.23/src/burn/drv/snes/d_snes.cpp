#include "snes.h"

unsigned char DoSnesReset = 0;

unsigned char SnesJoy1[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static struct BurnInputInfo SnesInputList[] =
{
	{"P1 Start",	  BIT_DIGITAL,   SnesJoy1 +  7, "p1 start"  },
	{"P1 Up",	  	  BIT_DIGITAL,   SnesJoy1 +  0, "p1 up"     },
	{"P1 Down",	  	  BIT_DIGITAL,   SnesJoy1 +  1, "p1 down"   },
	{"P1 Left",		  BIT_DIGITAL,   SnesJoy1 +  2, "p1 left"   },
	{"P1 Right",	  BIT_DIGITAL,   SnesJoy1 +  3, "p1 right"  },
	{"P1 Button A",	  BIT_DIGITAL,   SnesJoy1 +  6, "p1 fire 1" },
	{"P1 Button B",	  BIT_DIGITAL,   SnesJoy1 +  4, "p1 fire 2" },
	{"P1 Button C",	  BIT_DIGITAL,   SnesJoy1 +  5, "p1 fire 3" },
	{"P1 Button X",	  BIT_DIGITAL,   SnesJoy1 + 10, "p1 fire 4" },
	{"P1 Button Y",	  BIT_DIGITAL,   SnesJoy1 +  9, "p1 fire 5" },
	{"P1 Button Z",	  BIT_DIGITAL,   SnesJoy1 +  8, "p1 fire 6" },
	{"P1 Mode",	 	  BIT_DIGITAL,   SnesJoy1 + 11, "p1 select" },
	{"Reset"             , BIT_DIGITAL  , &DoSnesReset        , "reset"     },
};

STDINPUTINFO(Snes)

static struct BurnDIPInfo SnesDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x02, NULL                     },
};

STDDIPINFO(Snes)

static struct BurnRomInfo rSuperMarioWorldEuropeRev1RomDesc[] = {
	{ "smw.sfc",          524288, 0xb47f5f20, BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(rSuperMarioWorldEuropeRev1)
STD_ROM_FN(rSuperMarioWorldEuropeRev1)

struct BurnDriverD BurnDrvrSMW = {
	"smw", NULL, NULL, NULL, "1986",
	"Super Mario World (Europe) (Rev 1)\0", NULL, "Nintendo", "Miscellaneous",
	L"Super Mario World (Europe) (Rev 1)", NULL, NULL, NULL,
	BDF_GAME_WORKING|BDF_16BIT_ONLY , 2, HARDWARE_NINTENDO_SNES, GBF_SHOOT,0 ,
	NULL, rSuperMarioWorldEuropeRev1RomInfo, rSuperMarioWorldEuropeRev1RomName,NULL,NULL, SnesInputInfo, SnesDIPInfo,
	SnesInit, SnesExit, SnesFrame, NULL, SnesScan,
	NULL,0x6000,512 , 244, 4, 3
};

static struct BurnRomInfo rSuperMarioWorldEuropeRomDesc[] = {
	{ "smwa.sfc",          524288, 0x3c41070f, BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(rSuperMarioWorldEurope)
STD_ROM_FN(rSuperMarioWorldEurope)

struct BurnDriverD BurnDrvrSuperMarioWorldEurope = {
	"smwa", NULL, NULL, NULL, "1986",
	"Super Mario World (Europe)\0", NULL, "Nintendo", "Miscellaneous",
	L"Super Mario World (Europe)", NULL, NULL, NULL,
	BDF_GAME_WORKING|BDF_16BIT_ONLY , 2, HARDWARE_NINTENDO_SNES, GBF_SHOOT,0 ,
	NULL, rSuperMarioWorldEuropeRomInfo, rSuperMarioWorldEuropeRomName,NULL,NULL, SnesInputInfo, SnesDIPInfo,
	SnesInit, SnesExit, SnesFrame, NULL, SnesScan,
	NULL,0x6000,512 , 244, 4, 3
};

static struct BurnRomInfo rSuperMarioWorldJapanRomDesc[] = {
	{ "smwj.sfc",          524288, 0xec0ddac, BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(rSuperMarioWorldJapan)
STD_ROM_FN(rSuperMarioWorldJapan)

struct BurnDriverD BurnDrvrSuperMarioWorldJapan = {
	"smwj", NULL, NULL, NULL, "1986",
	"Super Mario World (Japan)\0", NULL, "Nintendo", "Miscellaneous",
	L"Super Mario World (Japan)", NULL, NULL, NULL,
	BDF_GAME_WORKING|BDF_16BIT_ONLY , 2, HARDWARE_NINTENDO_SNES, GBF_SHOOT,0 ,
	NULL, rSuperMarioWorldJapanRomInfo, rSuperMarioWorldJapanRomName,NULL,NULL, SnesInputInfo, SnesDIPInfo,
	SnesInit, SnesExit, SnesFrame, NULL, SnesScan,
	NULL,0x6000,512 , 244, 4, 3
};

static struct BurnRomInfo rSuperMarioWorldUSARomDesc[] = {
	{ "smwu.sfc",          524288, 0xb19ed489, BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(rSuperMarioWorldUSA)
STD_ROM_FN(rSuperMarioWorldUSA)

struct BurnDriverD BurnDrvrSuperMarioWorldUSA = {
	"smwu", NULL, NULL, NULL, "1986",
	"Super Mario World (USA)\0", NULL, "Nintendo", "Miscellaneous",
	L"Super Mario World (USA)", NULL, NULL, NULL,
	BDF_GAME_WORKING|BDF_16BIT_ONLY , 2, HARDWARE_NINTENDO_SNES, GBF_SHOOT,0 ,
	NULL, rSuperMarioWorldUSARomInfo, rSuperMarioWorldUSARomName,NULL,NULL, SnesInputInfo, SnesDIPInfo,
	SnesInit, SnesExit, SnesFrame, NULL, SnesScan,
	NULL,0x6000,512 , 244, 4, 3
};
