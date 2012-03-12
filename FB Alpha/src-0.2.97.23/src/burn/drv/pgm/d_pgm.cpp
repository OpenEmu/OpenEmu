// FB Alpha IGS PolyGameMaster driver module
// Original FB Alpha port by OopsWare
// Based on MAME driver by ElSemi

#include "pgm.h"

static struct BurnRomInfo emptyRomDesc[] = {
	{ "",                    0,          0, 0 },
};

static struct BurnInputInfo pgmInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	PgmBtn1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	PgmJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	PgmJoy1 + 1,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	PgmJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	PgmJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	PgmJoy1 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	PgmJoy1 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	PgmJoy1 + 6,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	PgmJoy1 + 7,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	PgmBtn2 + 0,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	PgmBtn1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	PgmJoy2 + 0,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	PgmJoy2 + 1,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	PgmJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	PgmJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	PgmJoy2 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	PgmJoy2 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	PgmJoy2 + 6,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	PgmJoy2 + 7,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	PgmBtn2 + 1,	"p2 fire 4"	},

	{"P3 Coin",		BIT_DIGITAL,	PgmBtn1 + 2,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	PgmJoy3 + 0,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	PgmJoy3 + 1,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	PgmJoy3 + 2,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	PgmJoy3 + 3,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	PgmJoy3 + 4,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	PgmJoy3 + 5,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	PgmJoy3 + 6,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	PgmJoy3 + 7,	"p3 fire 3"	},
	{"P3 Button 4",		BIT_DIGITAL,	PgmBtn2 + 2,	"p3 fire 4"	},

	{"P4 Coin",		BIT_DIGITAL,	PgmBtn1 + 3,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	PgmJoy4 + 0,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	PgmJoy4 + 1,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	PgmJoy4 + 2,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	PgmJoy4 + 3,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	PgmJoy4 + 4,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	PgmJoy4 + 5,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	PgmJoy4 + 6,	"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	PgmJoy4 + 7,	"p4 fire 3"	},
	{"P4 Button 4",		BIT_DIGITAL,	PgmBtn2 + 3,	"p4 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&PgmReset,	"reset"		},
	{"Diagnostics 1",	BIT_DIGITAL,	PgmBtn1 + 4,	"diag"		},
	{"Diagnostics 2",	BIT_DIGITAL,	PgmBtn1 + 6,	""		},
	{"Service 1",		BIT_DIGITAL,	PgmBtn1 + 5,	"service"	},
	{"Service 2",		BIT_DIGITAL,	PgmBtn1 + 7,	"service2"	},

	{"Dip A",		BIT_DIPSWITCH,	PgmInput + 6,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	PgmInput + 7,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,  PgmInput + 8,   "dip"           },
};

STDINPUTINFO(pgm)

static struct BurnDIPInfo pgmDIPList[] = {
	{0x2D,	0xFF, 0xFF,	0x00, NULL			},
	{0x2F,  0xFF, 0x01,     0x01, NULL			},

	{0,	0xFE, 0,	2,    "Test mode"		},
	{0x2D,	0x01, 0x01,	0x00, "Off"			},
	{0x2D,	0x01, 0x01,	0x01, "On"			},

	{0,	0xFE, 0,	2,    "Music"			},
	{0x2D,	0x01, 0x02,	0x02, "Off"			},
	{0x2D,	0x01, 0x02,	0x00, "On"			},

	{0,	0xFE, 0,	2,    "Voice"			},
	{0x2D,	0x01, 0x04,	0x04, "Off"			},
	{0x2D,	0x01, 0x04,	0x00, "On"			},

	{0,	0xFE, 0,	2,    "Free play"		},
	{0x2D,	0x01, 0x08,	0x00, "Off"			},
	{0x2D,	0x01, 0x08,	0x08, "On"			},

	{0,	0xFE, 0,	2,    "Stop mode"		},
	{0x2D,	0x01, 0x10,	0x00, "Off"			},
	{0x2D,	0x01, 0x10,	0x10, "On"			},

	{0,     0xFE, 0,        4,    "Bios select (Fake)"	},
	{0x2F,  0x01, 0x0f,     0x00, "Older"			},
	{0x2F,  0x01, 0x0f,     0x01, "Newer"			},
	{0x2F,  0x01, 0x0f,     0x02, "Newer (no intro, calendar)" },
	{0x2F,  0x01, 0x0f,     0x03, "Newer (no intro)"	},
};

STDDIPINFO(pgm)

static struct BurnDIPInfo jammaDIPList[] = {
	{0x2D,	0xFF, 0xFF,	0x00, NULL			},
	{0x2F,  0xFF, 0x01,     0x00, NULL			},

	{0,	0xFE, 0,	2,    "Test mode"		},
	{0x2D,	0x01, 0x01,	0x00, "Off"			},
	{0x2D,	0x01, 0x01,	0x01, "On"			},

	{0,	0xFE, 0,	2,    "Music"			},
	{0x2D,	0x01, 0x02,	0x02, "Off"			},
	{0x2D,	0x01, 0x02,	0x00, "On"			},

	{0,	0xFE, 0,	2,    "Voice"			},
	{0x2D,	0x01, 0x04,	0x04, "Off"			},
	{0x2D,	0x01, 0x04,	0x00, "On"			},

	{0,	0xFE, 0,	2,    "Free play"		},
	{0x2D,	0x01, 0x08,	0x00, "Off"			},
	{0x2D,	0x01, 0x08,	0x08, "On"			},

	{0,	0xFE, 0,	2,    "Stop mode"		},
	{0x2D,	0x01, 0x10,	0x00, "Off"			},
	{0x2D,	0x01, 0x10,	0x10, "On"			},
};

STDDIPINFO(jamma)

static struct BurnDIPInfo orlegendDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x00, NULL			},

	{0,	0xFE, 0,	2,    "Disable Ba-Chieh"	},
	{0x2E,	0x02, 0x01,	0x00, "Off"			},
	{0x2E,	0x02, 0x01,	0x01, "On"			},

	{0,	0xFE, 0,	3,    "Region (Fake)"		},
	{0x2E,	0x01, 0x03,	0x00, "World"			},
	{0x2E,	0x01, 0x03,	0x02, "Korea"			},
	{0x2E,	0x01, 0x03,	0x03, "China"			},
};

static struct BurnDIPInfo orlegend112cDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x03, NULL			},

	{0,	0xFE, 0,	2,    "Disable Ba-Chieh"	},
	{0x2E,	0x02, 0x01,	0x00, "Off"			},
	{0x2E,	0x02, 0x01,	0x01, "On"			},

	{0,	0xFE, 0,	3,    "Region (Fake)"		},
	{0x2E,	0x01, 0x03,	0x00, "World"			},
	{0x2E,	0x01, 0x03,	0x02, "Korea"			},
	{0x2E,	0x01, 0x03,	0x03, "China"			},
};

static struct BurnDIPInfo orld111cDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x02, NULL			},

	{0,	0xFE, 0,	2,    "Region (Fake)"		},
	{0x2E,	0x01, 0x03,	0x00, "Hong Kong"		},
	{0x2E,	0x01, 0x03,	0x02, "China"			},
};

static struct BurnDIPInfo orld105kDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x02, NULL			},
};

static struct BurnDIPInfo sangoDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x05, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0x0F,	0x00, "China"			},
	{0x2E,	0x01, 0x0F,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x0F,	0x02, "Japan"			},
	{0x2E,	0x01, 0x0F,	0x03, "Korea"			},
	{0x2E,	0x01, 0x0F,	0x04, "Hong Kong"		},
	{0x2E,	0x01, 0x0F,	0x05, "World"			},
};

static struct BurnDIPInfo oldsDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x04, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0x0F,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x0F,	0x02, "China"			},
	{0x2E,	0x01, 0x0F,	0x03, "Japan"			},
	{0x2E,	0x01, 0x0F,	0x04, "Korea"			},
	{0x2E,	0x01, 0x0F,	0x05, "Hong Kong"		},
	{0x2E,	0x01, 0x0F,	0x06, "World"			},
};

static struct BurnDIPInfo olds100DIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x05, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0x0F,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x0F,	0x02, "China"			},
	{0x2E,	0x01, 0x0F,	0x03, "Japan"			},
	{0x2E,	0x01, 0x0F,	0x04, "Korea"			},
	{0x2E,	0x01, 0x0F,	0x05, "Hong Kong"		},
	{0x2E,	0x01, 0x0F,	0x06, "World"			},
};

static struct BurnDIPInfo kov100DIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x02, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0x0F,	0x00, "China"			},
	{0x2E,	0x01, 0x0F,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x0F,	0x02, "Japan"			},
	{0x2E,	0x01, 0x0F,	0x03, "Korea"			},
	{0x2E,	0x01, 0x0F,	0x04, "Hong Kong"		},
	{0x2E,	0x01, 0x0F,	0x05, "World"			},
};

static struct BurnDIPInfo killbldDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x21, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0xFF,	0x16, "Taiwan"			},
	{0x2E,	0x01, 0xFF,	0x17, "China"			},
	{0x2E,	0x01, 0xFF,	0x18, "Hong Kong"		},
	{0x2E,	0x01, 0xFF,	0x19, "Japan"			},
	{0x2E,	0x01, 0xFF,	0x20, "Korea"			},
	{0x2E,	0x01, 0xFF,	0x21, "World"			},
};

static struct BurnDIPInfo photoy2kDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x03, NULL			},

	{0,	0xFE, 0,	7,    "Region (Fake)"		},
	{0x2E,	0x01, 0x0F,	0x00, "Taiwan"			},
	{0x2E,	0x01, 0x0F,	0x01, "China"			},
	{0x2E,	0x01, 0x0F,	0x02, "Japan"			},
	{0x2E,	0x01, 0x0F,	0x03, "World"			},
	{0x2E,	0x01, 0x0F,	0x04, "Korea"			},
	{0x2E,	0x01, 0x0F,	0x05, "Hong Kong"		},
	{0x2E,	0x01, 0x0F,	0x06, "Singapore / Malaysia"	},
};

static struct BurnDIPInfo raf102jDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x02, NULL			},

	{0,	0xFE, 0,	7,    "Region (Fake)"		},
	{0x2E,	0x01, 0x0F,	0x00, "Taiwan"			},
	{0x2E,	0x01, 0x0F,	0x01, "China"			},
	{0x2E,	0x01, 0x0F,	0x02, "Japan"			},
	{0x2E,	0x01, 0x0F,	0x03, "World"			},
	{0x2E,	0x01, 0x0F,	0x04, "Korea"			},
	{0x2E,	0x01, 0x0F,	0x05, "Hong Kong"		},
	{0x2E,	0x01, 0x0F,	0x06, "Singapore / Malaysia"	},
};

static struct BurnDIPInfo puzzli2DIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x05, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0x0F,	0x00, "Taiwan"			},
	{0x2E,	0x01, 0x0F,	0x01, "China"			},
	{0x2E,	0x01, 0x0F,	0x02, "Japan"			},
	{0x2E,	0x01, 0x0F,	0x03, "Korea"			},
	{0x2E,	0x01, 0x0F,	0x04, "Hong Kong"		},
	{0x2E,	0x01, 0x0F,	0x05, "World"			},
};

static struct BurnDIPInfo kov2DIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x04, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0x07,	0x00, "China"			},
	{0x2E,	0x01, 0x07,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x07,	0x02, "Japan"			},
	{0x2E,	0x01, 0x07,	0x03, "Korea"			},
	{0x2E,	0x01, 0x07,	0x04, "Hong Kong"		},
	{0x2E,	0x01, 0x07,	0x05, "World"			},
};

static struct BurnDIPInfo martmastDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x06, NULL			},

	{0,	0xFE, 0,	7,    "Region (Fake)"		},
	{0x2E,	0x01, 0x07,	0x00, "China"			},
	{0x2E,	0x01, 0x07,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x07,	0x02, "China"			},
	{0x2E,	0x01, 0x07,	0x03, "Korea"			},
	{0x2E,	0x01, 0x07,	0x04, "Hong Kong"		},
	{0x2E,	0x01, 0x07,	0x05, "World"			},
	{0x2E,	0x01, 0x07,	0x06, "USA"			},
};

static struct BurnDIPInfo martmastcDIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x00, NULL			},

	{0,	0xFE, 0,	7,    "Region (Fake)"		},
	{0x2E,	0x01, 0x07,	0x00, "China"			},
	{0x2E,	0x01, 0x07,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x07,	0x02, "China"			},
	{0x2E,	0x01, 0x07,	0x03, "Korea"			},
	{0x2E,	0x01, 0x07,	0x04, "Hong Kong"		},
	{0x2E,	0x01, 0x07,	0x05, "World"			},
	{0x2E,	0x01, 0x07,	0x06, "USA"			},
};

static struct BurnDIPInfo martmastc102DIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x00, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0x07,	0x00, "China"			},
	{0x2E,	0x01, 0x07,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x07,	0x02, "Japan"			},
	{0x2E,	0x01, 0x07,	0x03, "Korea"			},
	{0x2E,	0x01, 0x07,	0x04, "Hong Kong"		},
	{0x2E,	0x01, 0x07,	0x05, "World"			},
};

static struct BurnDIPInfo ddp2DIPList[] = {
	{0x2E,	0xFF, 0xFF,	0x00, NULL			},

	{0,	0xFE, 0,	6,    "Region (Fake)"		},
	{0x2E,	0x01, 0x07,	0x00, "China"			},
	{0x2E,	0x01, 0x07,	0x01, "Taiwan"			},
	{0x2E,	0x01, 0x07,	0x02, "Japan (Cave License)"	},
	{0x2E,	0x01, 0x07,	0x03, "Korea"			},
	{0x2E,	0x01, 0x07,	0x04, "Hong Kong"		},
	{0x2E,	0x01, 0x07,	0x05, "World"			},
};

STDDIPINFOEXT(orlegend,		pgm, orlegend		)
STDDIPINFOEXT(orlegend112c, 	pgm, orlegend112c	)
STDDIPINFOEXT(orld111c, 	pgm, orld111c		)
STDDIPINFOEXT(orld105k, 	pgm, orld105k		)
STDDIPINFOEXT(sango,    	pgm, sango		)
STDDIPINFOEXT(kov100,     	pgm, kov100		)
STDDIPINFOEXT(killbld,	 	pgm, killbld		)
STDDIPINFOEXT(photoy2k, 	pgm, photoy2k		)
STDDIPINFOEXT(raf102j,  	pgm, raf102j		)
STDDIPINFOEXT(puzzli2,  	pgm, puzzli2		)
STDDIPINFOEXT(kov2,     	pgm, kov2		)
STDDIPINFOEXT(martmast, 	pgm, martmast		)
STDDIPINFOEXT(martmastc, 	pgm, martmastc		)
STDDIPINFOEXT(martmastc102, 	pgm, martmastc102	)
STDDIPINFOEXT(olds,     	pgm, olds		)
STDDIPINFOEXT(olds100,  	pgm, olds100		)
STDDIPINFOEXT(ddp2,     	pgm, ddp2		)

// -----------------------------------------------------------------------------
// BIOS


// PGM (Polygame Master) System BIOS

static struct BurnRomInfo pgmRomDesc[] = {
	{ "pgm_t01s.rom", 0x200000, 0x1a7123a0, BRF_GRA | BRF_BIOS }, 	// 0x80 - 8x8 Text Layer Tiles
	{ "pgm_m01s.rom", 0x200000, 0x45ae7159, BRF_SND | BRF_BIOS },	// 0x81 - Samples

	{ "pgm_p01s.u20", 0x020000, 0xe42b166e, BRF_PRG | BRF_BIOS },	// 0x82 - 68K BIOS (V0001,     older  - 02/26/97 - 11:14:09)
	{ "pgm_p02s.u20", 0x020000, 0x78C15FA2, BRF_PRG | BRF_BIOS },	// 0x83 - 68K BIOS (V0001,     newer  - 07/10/97 - 16:36:08)
#if !defined (ROM_VERIFY)
	{ "ddp3_bios.u37",0x080000, 0xB3CC5C8F, BRF_PRG | BRF_BIOS },	// 0x84 - 68K BIOS (V0001,     custom - 07/17/97 - 19:44:59)
	{ "bios.u42",     0x020000, 0x517CF7A2, BRF_PRG | BRF_BIOS },   // 0x85 - 68K BIOS (V0001-01J, custom - 02/25/03 - 17:51:01)
#else
	{ "",             0x000000, 0x00000000, 0                  },   // 0x84
	{ "",             0x000000, 0x00000000, 0                  },   // 0x85
#endif
};

STD_ROM_PICK(pgm)
STD_ROM_FN(pgm)

struct BurnDriver BurnDrvPgm = {
	"pgm", NULL, NULL, NULL, "1997",
	"PGM (Polygame Master) System BIOS\0", "BIOS only", "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_BOARDROM, 0, HARDWARE_IGS_PGM, GBF_BIOS, 0,
	NULL, pgmRomInfo, pgmRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	pgmInit, pgmExit, pgmFrame, pgmDraw, pgmScan,  &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


static struct BurnRomInfo thegladBIOSRomDesc[] = {
	{ "pgm_t01s.rom", 0x200000, 0x1a7123a0, BRF_GRA | BRF_BIOS }, 	// 0x80 - 8x8 Text Layer Tiles

	{ "pgm_m01s.rom", 0x200000, 0x45ae7159, BRF_SND | BRF_BIOS },	// 0x81 - Samples

	{ "bios.u42",     0x020000, 0x517CF7A2, BRF_PRG | BRF_BIOS },	// 0x82 - 68K BIOS (V0001-01J, custom? 02/25/03 17:51:01)

	{ "wave.u29",     0x200000, 0x51ACB395, BRF_SND | BRF_BIOS },	// 0x83 - More samples
};

static struct BurnRomInfo dmnfrntBIOSRomDesc[] = {
	{ "pgm_t01s.rom", 0x200000, 0x1a7123a0, BRF_GRA | BRF_BIOS }, 	// 0x80 - 8x8 Text Layer Tiles

	{ "pgm_m01s.rom", 0x200000, 0x45ae7159, BRF_SND | BRF_BIOS },	// 0x81 - Samples

	{ "bios.u42",     0x020000, 0x517CF7A2, BRF_PRG | BRF_BIOS },	// 0x82 - 68K BIOS (V0001-01J, custom? 02/25/03 17:51:01)
};

static struct BurnRomInfo ketsuiBiosRomDesc[] = {
	{ "pgm_t01s.rom", 0x200000, 0x1a7123a0, BRF_GRA | BRF_BIOS }, 	// 0x80 - 8x8 Text Layer Tiles
	{ "",		  0,        0,          0 },			// 0x81 - Samples

	{ "",		  0,        0,          0 },			// 0x82 - 68K BIOS
	{ "",		  0,        0,          0 },			// 0x83 - 68K BIOS
};


static struct BurnRomInfo espgalBiosRomDesc[] = {
	{ "t01s.u18", 0x200000, 0x1a7123a0, BRF_GRA | BRF_BIOS }, 	// 0x80 - 8x8 Text Layer Tiles
	{ "",		  0,        0,          0 },			// 0x81 - Samples

	{ "",		  0,        0,          0 },			// 0x82 - 68K BIOS
	{ "",		  0,        0,          0 },			// 0x83 - 68K BIOS
};


static struct BurnRomInfo ddp3BiosRomDesc[] = {
	{ "pgm_t01s.rom", 0x200000, 0x1a7123a0, BRF_GRA | BRF_BIOS }, 	// 0x80 - 8x8 Text Layer Tiles

	{ "pgm_m01s.rom", 0x200000, 0x45ae7159, BRF_SND | BRF_BIOS },	// 0x81 - Samples

	{ "ddp3_bios.u37",0x080000, 0xB3CC5C8F, BRF_PRG | BRF_BIOS },	// 0x82 - 68K BIOS (V0001, custom? 07/17/97 19:44:59)
};


// -----------------------------------------------------------------------------
// Normal Games


// Oriental Legend / Xi Yo Gi Shi Re Zuang (V126)

static struct BurnRomInfo orlegendRomDesc[] = {
	{ "p0103.rom",			0x200000, 0xd5e93543, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t0100.rom",			0x400000, 0x61425e1e, 2 | BRF_GRA },		//  1 Tile data

	{ "a0100.rom",			0x400000, 0x8b3bd88a, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a0101.rom",			0x400000, 0x3b9e9644, 3 | BRF_GRA },		//  3	  
	{ "a0102.rom",			0x400000, 0x069e2c38, 3 | BRF_GRA },		//  4
	{ "a0103.rom",			0x400000, 0x4460a3fd, 3 | BRF_GRA },		//  5
	{ "a0104.rom",			0x400000, 0x5f8abb56, 3 | BRF_GRA },		//  6
	{ "a0105.rom",			0x400000, 0xa17a7147, 3 | BRF_GRA },		//  7

	{ "b0100.rom",			0x400000, 0x69d2e48c, 4 | BRF_GRA },		//  8 Sprite Masks & Color Indexes
	{ "b0101.rom",			0x400000, 0x0d587bf3, 4 | BRF_GRA },		//  9
	{ "b0102.rom",			0x400000, 0x43823c1e, 4 | BRF_GRA },		// 10

	{ "m0100.rom",			0x200000, 0xe5c36c83, 5 | BRF_SND },		// 11 Samples
};

STDROMPICKEXT(orlegend, orlegend, pgm)
STD_ROM_FN(orlegend)

static INT32 orlegendInit()
{
	pPgmProtCallback = install_protection_asic3_orlegend;

	return pgmInit();
}

struct BurnDriver BurnDrvOrlegend = {
	"orlegend", NULL, "pgm", NULL, "1997",
	"Oriental Legend - Xi Yo Gi Shi Re Zuang (V126)\0", NULL, "IGS", "PolyGameMaster",
	L"Oriental Legend\0\u897F\u6E38\u91CA\u5384\u4F20 (V126)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, orlegendRomInfo, orlegendRomName, NULL, NULL, pgmInputInfo, orlegendDIPInfo,
	orlegendInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend / Xi Yo Gi Shi Re Zuang (V112)

static struct BurnRomInfo orlegend112eRomDesc[] = {
	{ "p0102.rom",			0x200000, 0x4d0f6cc5, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t0100.rom",			0x400000, 0x61425e1e, 2 | BRF_GRA },		//  1 Tile data

	{ "a0100.rom",			0x400000, 0x8b3bd88a, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a0101.rom",			0x400000, 0x3b9e9644, 3 | BRF_GRA },		//  3	  
	{ "a0102.rom",			0x400000, 0x069e2c38, 3 | BRF_GRA },		//  4
	{ "a0103.rom",			0x400000, 0x4460a3fd, 3 | BRF_GRA },		//  5
	{ "a0104.rom",			0x400000, 0x5f8abb56, 3 | BRF_GRA },		//  6
	{ "a0105.rom",			0x400000, 0xa17a7147, 3 | BRF_GRA },		//  7

	{ "b0100.rom",			0x400000, 0x69d2e48c, 4 | BRF_GRA },		//  8 Sprite Masks & Color Indexes
	{ "b0101.rom",			0x400000, 0x0d587bf3, 4 | BRF_GRA },		//  9
	{ "b0102.rom",			0x400000, 0x43823c1e, 4 | BRF_GRA },		// 10

	{ "m0100.rom",			0x200000, 0xe5c36c83, 5 | BRF_SND },		// 11 Samples
};

STDROMPICKEXT(orlegend112e, orlegend112e, pgm)
STD_ROM_FN(orlegend112e)

struct BurnDriver BurnDrvOrlegend112e = {
	"orlegende", "orlegend", "pgm", NULL, "1997",
	"Oriental Legend - Xi Yo Gi Shi Re Zuang (V112)\0", NULL, "IGS", "PolyGameMaster",
	L"Oriental Legend\0\u897F\u6E38\u91CA\u5384\u4F20 (V112)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, orlegend112eRomInfo, orlegend112eRomName, NULL, NULL, pgmInputInfo, orlegendDIPInfo,
	orlegendInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend / Xi Yo Gi Shi Re Zuang (V112, China)

static struct BurnRomInfo orlegend112cRomDesc[] = {
	{ "p0101.160",			0x200000, 0xb24f0c1e, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t0100.rom",			0x400000, 0x61425e1e, 2 | BRF_GRA },		//  1 Tile data

	{ "a0100.rom",			0x400000, 0x8b3bd88a, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a0101.rom",			0x400000, 0x3b9e9644, 3 | BRF_GRA },		//  3	  
	{ "a0102.rom",			0x400000, 0x069e2c38, 3 | BRF_GRA },		//  4
	{ "a0103.rom",			0x400000, 0x4460a3fd, 3 | BRF_GRA },		//  5
	{ "a0104.rom",			0x400000, 0x5f8abb56, 3 | BRF_GRA },		//  6
	{ "a0105.rom",			0x400000, 0xa17a7147, 3 | BRF_GRA },		//  7

	{ "b0100.rom",			0x400000, 0x69d2e48c, 4 | BRF_GRA },		//  8 Sprite Masks & Color Indexes
	{ "b0101.rom",			0x400000, 0x0d587bf3, 4 | BRF_GRA },		//  9
	{ "b0102.rom",			0x400000, 0x43823c1e, 4 | BRF_GRA },		// 10

	{ "m0100.rom",			0x200000, 0xe5c36c83, 5 | BRF_SND },		// 11 Samples
};

STDROMPICKEXT(orlegend112c, orlegend112c, pgm)
STD_ROM_FN(orlegend112c)

struct BurnDriver BurnDrvOrlegend112c = {
	"orlegendc", "orlegend", "pgm", NULL, "1997",
	"Oriental Legend - Xi Yo Gi Shi Re Zuang (V112, China)\0", NULL, "IGS", "PolyGameMaster",
	L"Oriental Legend\0\u897F\u6E38\u91CA\u5384\u4F20 (V112, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, orlegend112cRomInfo, orlegend112cRomName, NULL, NULL, pgmInputInfo, orlegend112cDIPInfo,
	orlegendInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend / Xi Yo Gi Shi Re Zuang (V112 alt,China)

static struct BurnRomInfo orlegend112caRomDesc[] = {
	{ "p0101.102",			0x200000, 0x7a22e1cb, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t0100.rom",			0x400000, 0x61425e1e, 2 | BRF_GRA },		//  1 Tile data

	{ "a0100.rom",			0x400000, 0x8b3bd88a, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a0101.rom",			0x400000, 0x3b9e9644, 3 | BRF_GRA },		//  3	  
	{ "a0102.rom",			0x400000, 0x069e2c38, 3 | BRF_GRA },		//  4
	{ "a0103.rom",			0x400000, 0x4460a3fd, 3 | BRF_GRA },		//  5
	{ "a0104.rom",			0x400000, 0x5f8abb56, 3 | BRF_GRA },		//  6
	{ "a0105.rom",			0x400000, 0xa17a7147, 3 | BRF_GRA },		//  7

	{ "b0100.rom",			0x400000, 0x69d2e48c, 4 | BRF_GRA },		//  8 Sprite Masks & Color Indexes
	{ "b0101.rom",			0x400000, 0x0d587bf3, 4 | BRF_GRA },		//  9
	{ "b0102.rom",			0x400000, 0x43823c1e, 4 | BRF_GRA },		// 10

	{ "m0100.rom",			0x200000, 0xe5c36c83, 5 | BRF_SND },		// 11 Samples
};

STDROMPICKEXT(orlegend112ca, orlegend112ca, pgm)
STD_ROM_FN(orlegend112ca)

struct BurnDriver BurnDrvOrlegend112ca = {
	"orlegendca", "orlegend", "pgm", NULL, "1997",
	"Oriental Legend - Xi Yo Gi Shi Re Zuang (V112 alt, China)\0", NULL, "IGS", "PolyGameMaster",
	L"Oriental Legend\0\u897F\u6E38\u91CA\u5384\u4F20 (V112 alt, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, orlegend112caRomInfo, orlegend112caRomName, NULL, NULL, pgmInputInfo, orlegend112cDIPInfo,
	orlegendInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend / Xi Yo Gi Shi Re Zuang (V111, China)

static struct BurnRomInfo orlegend111cRomDesc[] = {
	{ "olv111ch.u6",		0x080000, 0x5fb86373, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code
	{ "olv111ch.u9",		0x080000, 0x83cf09c8, 1 | BRF_PRG | BRF_ESS },	//  1
	{ "olv111ch.u7",		0x080000, 0x6ee79faf, 1 | BRF_PRG | BRF_ESS },	//  2
	{ "olv111ch.u11",		0x080000, 0xb80ddd3c, 1 | BRF_PRG | BRF_ESS },	//  3

	{ "t0100.rom",			0x400000, 0x61425e1e, 2 | BRF_GRA },		//  1 Tile data

	{ "a0100.rom",			0x400000, 0x8b3bd88a, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a0101.rom",			0x400000, 0x3b9e9644, 3 | BRF_GRA },		//  3	  
	{ "a0102.rom",			0x400000, 0x069e2c38, 3 | BRF_GRA },		//  4
	{ "a0103.rom",			0x400000, 0x4460a3fd, 3 | BRF_GRA },		//  5
	{ "a0104.rom",			0x400000, 0x5f8abb56, 3 | BRF_GRA },		//  6
	{ "a0105.rom",			0x400000, 0xa17a7147, 3 | BRF_GRA },		//  7

	{ "b0100.rom",			0x400000, 0x69d2e48c, 4 | BRF_GRA },		//  8 Sprite Masks & Color Indexes
	{ "b0101.rom",			0x400000, 0x0d587bf3, 4 | BRF_GRA },		//  9
	{ "b0102.rom",			0x400000, 0x43823c1e, 4 | BRF_GRA },		// 10

	{ "m0100.rom",			0x200000, 0xe5c36c83, 5 | BRF_SND },		// 11 Samples
};

STDROMPICKEXT(orlegend111c, orlegend111c, pgm)
STD_ROM_FN(orlegend111c)

struct BurnDriver BurnDrvOrlegend111c = {
	"orlegend111c", "orlegend", "pgm", NULL, "1997",
	"Oriental Legend - Xi Yo Gi Shi Re Zuang (V111, China)\0", NULL, "IGS", "PolyGameMaster",
	L"Oriental Legend\0\u897F\u6E38\u91CA\u5384\u4F20 (V111, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, orlegend111cRomInfo, orlegend111cRomName, NULL, NULL, pgmInputInfo, orld111cDIPInfo,
	orlegendInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend (V105, Korea)

static struct BurnRomInfo orlegend105kRomDesc[] = {
	{ "olv105ko.u6",		0x080000, 0xb86703fe, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code
	{ "olv105ko.u9",		0x080000, 0x5a108e39, 1 | BRF_PRG | BRF_ESS },	//  1
	{ "olv105ko.u7",		0x080000, 0x5712facc, 1 | BRF_PRG | BRF_ESS },	//  2
	{ "olv105ko.u11",		0x080000, 0x40ae4d9e, 1 | BRF_PRG | BRF_ESS },	//  3

	{ "t0100.rom",			0x400000, 0x61425e1e, 2 | BRF_GRA },		//  1 Tile data

	{ "a0100.rom",			0x400000, 0x8b3bd88a, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a0101.rom",			0x400000, 0x3b9e9644, 3 | BRF_GRA },		//  3	  
	{ "a0102.rom",			0x400000, 0x069e2c38, 3 | BRF_GRA },		//  4
	{ "a0103.rom",			0x400000, 0x4460a3fd, 3 | BRF_GRA },		//  5
	{ "a0104.rom",			0x400000, 0x5f8abb56, 3 | BRF_GRA },		//  6
	{ "a0105.rom",			0x400000, 0xa17a7147, 3 | BRF_GRA },		//  7

	{ "b0100.rom",			0x400000, 0x69d2e48c, 4 | BRF_GRA },		//  8 Sprite Masks & Color Indexes
	{ "b0101.rom",			0x400000, 0x0d587bf3, 4 | BRF_GRA },		//  9
	{ "b0102.rom",			0x400000, 0x43823c1e, 4 | BRF_GRA },		// 10

	{ "m0100.rom",			0x200000, 0xe5c36c83, 5 | BRF_SND },		// 11 Samples
};

STDROMPICKEXT(orlegend105k, orlegend105k, pgm)
STD_ROM_FN(orlegend105k)

struct BurnDriver BurnDrvOrlegend105k = {
	"orlegend105k", "orlegend", "pgm", NULL, "1997",
	"Oriental Legend (V105, Korea)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, orlegend105kRomInfo, orlegend105kRomName, NULL, NULL, pgmInputInfo, orld105kDIPInfo,
	orlegendInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Dragon World II (V110X, World)

static struct BurnRomInfo drgw2RomDesc[] = {
	{ "v-110x.u2",			0x080000, 0x1978106b, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "pgmt0200.u7",		0x400000, 0xb0f6534d, 2 | BRF_GRA },		//  1 Tile data

	{ "pgma0200.u5",		0x400000, 0x13b95069, 3 | BRF_GRA },		//  2 Sprite Color Data

	{ "pgmb0200.u9",		0x400000, 0x932d0f13, 4 | BRF_GRA },		//  3 Sprite Masks & Color Indexes
};

STDROMPICKEXT(drgw2, drgw2, pgm)
STD_ROM_FN(drgw2)

static void drgw2_patch()
{
	pgm_decrypt_dw2();

	*((UINT16*)(PGM68KROM + 0x031098)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
	*((UINT16*)(PGM68KROM + 0x03113e)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
	*((UINT16*)(PGM68KROM + 0x0311ce)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
}

static INT32 drgw2Init()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = drgw2_patch;
	pPgmProtCallback = install_protection_asic25_asic12_dw2;

	return pgmInit();
}

struct BurnDriver BurnDrvDrgw2 = {
	"drgw2", NULL, "pgm", NULL, "1997",
	"Dragon World II (V110X, World)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, drgw2RomInfo, drgw2RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	drgw2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Zhong Guo Long II (V100C, China)

static struct BurnRomInfo drgw2cRomDesc[] = {
	{ "v-100c.u2",			0x080000, 0x67467981, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "pgmt0200.u7",		0x400000, 0xb0f6534d, 2 | BRF_GRA },		//  1 Tile data

	{ "pgma0200.u5",		0x400000, 0x13b95069, 3 | BRF_GRA },		//  2 Sprite Color Data

	{ "pgmb0200.u9",		0x400000, 0x932d0f13, 4 | BRF_GRA },		//  3 Sprite Masks & Color Indexes
};

STDROMPICKEXT(drgw2c, drgw2c, pgm)
STD_ROM_FN(drgw2c)

static void drgw2100c_patch()
{
	pgm_decrypt_dw2();

	*((UINT16*)(PGM68KROM + 0x0303bc)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
	*((UINT16*)(PGM68KROM + 0x030462)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
	*((UINT16*)(PGM68KROM + 0x0304F2)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
}

static INT32 drgw2100cInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = drgw2100c_patch;
	pPgmProtCallback = install_protection_asic25_asic12_dw2;

	return pgmInit();
}

struct BurnDriver BurnDrvDrgw2c = {
	"drgw2c", "drgw2", "pgm", NULL, "1997",
	"Zhong Guo Long II (V100C, China)\0", NULL, "IGS", "PolyGameMaster",
	L"\u4E2D\u570B\u9F8D II (V100C, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, drgw2cRomInfo, drgw2cRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	drgw2100cInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	448, 224, 4, 3
};


// Chuugokuryuu II (V100J, Japan)

static struct BurnRomInfo drgw2jRomDesc[] = {
	{ "v-100j.u2",			0x080000, 0xf8f8393e, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "pgmt0200.u7",		0x400000, 0xb0f6534d, 2 | BRF_GRA },		//  1 Tile data

	{ "pgma0200.u5",		0x400000, 0x13b95069, 3 | BRF_GRA },		//  2 Sprite Color Data

	{ "pgmb0200.u9",		0x400000, 0x932d0f13, 4 | BRF_GRA },		//  3 Sprite Masks & Color Indexes
};

STDROMPICKEXT(drgw2j, drgw2j, pgm)
STD_ROM_FN(drgw2j)

static void drgw2100j_patch()
{
	pgm_decrypt_dw2();

	*((UINT16*)(PGM68KROM + 0x0302C0)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
	*((UINT16*)(PGM68KROM + 0x030366)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
	*((UINT16*)(PGM68KROM + 0x0303F6)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
}

static INT32 drgw2100jInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = drgw2100j_patch;
	pPgmProtCallback = install_protection_asic25_asic12_dw2;

	return pgmInit();
}

struct BurnDriver BurnDrvDrgw2j = {
	"drgw2j", "drgw2", "pgm", NULL, "1997",
	"Chuugokuryuu II (V100J, Japan)\0", NULL, "IGS", "PolyGameMaster",
	L"\u4E2D\u570B\u9F8D II (V100J, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, drgw2jRomInfo, drgw2jRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	drgw2100jInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	448, 224, 4, 3
};


// Dragon World II (V100X, World)

static struct BurnRomInfo drgw2xRomDesc[] = {
	{ "dragonv100x.bin",		0x080000, 0x5e71851d, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "pgmt0200.u7",		0x400000, 0xb0f6534d, 2 | BRF_GRA },		//  1 Tile data

	{ "pgma0200.u5",		0x400000, 0x13b95069, 3 | BRF_GRA },		//  2 Sprite Color Data

	{ "pgmb0200.u9",		0x400000, 0x932d0f13, 4 | BRF_GRA },		//  3 Sprite Masks & Color Indexes
};

STDROMPICKEXT(drgw2x, drgw2x, pgm)
STD_ROM_FN(drgw2x)

static void drgw2100x_patch()
{
	pgm_decrypt_dw2();

	*((UINT16*)(PGM68KROM + 0x031084)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
	*((UINT16*)(PGM68KROM + 0x03112A)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
	*((UINT16*)(PGM68KROM + 0x0311BA)) = BURN_ENDIAN_SWAP_INT16(0x4e93);
}

static INT32 drgw2100xInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = drgw2100x_patch;
	pPgmProtCallback = install_protection_asic25_asic12_dw2;

	return pgmInit();
}

struct BurnDriver BurnDrvDrgw2x = {
	"dw2v100x", "drgw2", "pgm", NULL, "1997",
	"Dragon World II (V100X, World)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, drgw2xRomInfo, drgw2xRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	drgw2100xInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	448, 224, 4, 3
};


// The Killing Blade (V109, China)

static struct BurnRomInfo killbldRomDesc[] = {
	{ "p0300_v109.u9",		0x200000, 0x2fcee215, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t0300.u14",			0x400000, 0x0922f7d9, 2 | BRF_GRA },		//  1 Tile data

	{ "a0300.u9",			0x400000, 0x3f9455d3, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a0301.u10",			0x400000, 0x92776889, 3 | BRF_GRA },		//  3
	{ "a0303.u11",			0x400000, 0x33f5cc69, 3 | BRF_GRA },		//  4
	{ "a0306.u12",			0x400000, 0xcc018a8e, 3 | BRF_GRA },		//  5
	{ "a0307.u2",			0x400000, 0xbc772e39, 3 | BRF_GRA },		//  6

	{ "b0300.u13",			0x400000, 0x7f876981, 4 | BRF_GRA },		//  7 Sprite Masks & Color Indexes
	{ "b0302.u14",			0x400000, 0xeea9c502, 4 | BRF_GRA },		//  8
	{ "b0303.u15",			0x200000, 0x77a9652e, 4 | BRF_GRA },		//  9

	{ "m0300.u1",			0x400000, 0x93159695, 5 | BRF_SND },		// 10 Samples

	{ "kb_u2.rom",			0x010000, 0xde3eae63, 0 | BRF_PRG | BRF_ESS },	// 11 Protection Data
};

STDROMPICKEXT(killbld, killbld, pgm)
STD_ROM_FN(killbld)

static INT32 killbldInit()
{
	pPgmInitCallback = pgm_decrypt_killbld;
	pPgmProtCallback = install_protection_asic25_asic22_killbld;

	return pgmInit();
}

struct BurnDriver BurnDrvKillbld = {
	"killbld", NULL, "pgm", NULL, "1998",
	"The Killing Blade (V109, China)\0", NULL, "IGS", "PolyGameMaster",
	L"The Killing Blade\0\u50B2\u5251\u72C2\u5200\0\u50B2\u528D\u72C2\u5200 (V109, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM, GBF_VSFIGHT, 0,
	NULL, killbldRomInfo, killbldRomName, NULL, NULL, pgmInputInfo, killbldDIPInfo,
	killbldInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// The Killing Blade (V104)

static struct BurnRomInfo killbld104RomDesc[] = {
	{ "kb_u3_v104.u3",		0x080000, 0x6db1d719, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code
	{ "kb_u6_v104.u6",		0x080000, 0x31ecc978, 1 | BRF_PRG | BRF_ESS },	//  1
	{ "kb_u4_v104.u4",		0x080000, 0x1ed8b2e7, 1 | BRF_PRG | BRF_ESS },	//  2
	{ "kb_u5_v104.u5",		0x080000, 0xa0bafc29, 1 | BRF_PRG | BRF_ESS },	//  3

	{ "t0300.u14",			0x400000, 0x0922f7d9, 2 | BRF_GRA },		//  4 Tile data

	{ "a0300.u9",			0x400000, 0x3f9455d3, 3 | BRF_GRA },		//  5 Sprite Color Data
	{ "a0301.u10",			0x400000, 0x92776889, 3 | BRF_GRA },		//  6
	{ "a0303.u11",			0x400000, 0x33f5cc69, 3 | BRF_GRA },		//  7
	{ "a0306.u12",			0x400000, 0xcc018a8e, 3 | BRF_GRA },		//  8
	{ "a0307.u2",			0x400000, 0xbc772e39, 3 | BRF_GRA },		//  9

	{ "b0300.u13",			0x400000, 0x7f876981, 4 | BRF_GRA },		// 10 Sprite Masks & Color Indexes
	{ "b0302.u14",			0x400000, 0xeea9c502, 4 | BRF_GRA },		// 11
	{ "b0303.u15",			0x200000, 0x77a9652e, 4 | BRF_GRA },		// 12

	{ "m0300.u1",			0x400000, 0x93159695, 5 | BRF_SND },		// 13 Samples

	{ "kb_u2_v104.u2",		0x010000, 0xc970f6d5, 0 | BRF_PRG | BRF_ESS },	// 14 Protection Data
};

STDROMPICKEXT(killbld104, killbld104, pgm)
STD_ROM_FN(killbld104)

struct BurnDriver BurnDrvKillbld104 = {
	"killbld104", "killbld", "pgm", NULL, "1998",
	"The Killing Blade (V104)\0", NULL, "IGS", "PolyGameMaster",
	L"The Killing Blade\0\u50B2\u5251\u72C2\u5200\0\u50B2\u528D\u72C2\u5200 (V104)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_VSFIGHT, 0,
	NULL, killbld104RomInfo, killbld104RomName, NULL, NULL, pgmInputInfo, killbldDIPInfo,
	killbldInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Dragon World 3 (V106, Korea)

static struct BurnRomInfo drgw3RomDesc[] = {
	{ "dw3_v106.u12",  		0x080000, 0xc3f6838b, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "dw3_v106.u13",  		0x080000, 0x28284e22, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "dw3t0400.u18",  		0x400000, 0xb70f3357, 2 | BRF_GRA },	       //  2 Tile data

	{ "dw3a0400.u9",   		0x400000, 0xdd7bfd40, 3 | BRF_GRA },	       //  3 Sprite Color Data
	{ "dw3a0401.u10",  		0x400000, 0xcab6557f, 3 | BRF_GRA },	       //  4

	{ "dw3b0400.u13",  		0x400000, 0x4bb87cc0, 4 | BRF_GRA },	       //  5 Sprite Masks & Color Indexes

	{ "dw3m0400.u1",   		0x400000, 0x031eb9ce, 5 | BRF_SND },	       //  6 Samples

	{ "dw3_v100.u15",  		0x010000, 0x03dc4fdf, 0 | BRF_PRG | BRF_ESS }, //  7 Protection data
};

STDROMPICKEXT(drgw3, drgw3, pgm)
STD_ROM_FN(drgw3)

static INT32 drgw3Init()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_dw3;

	return pgmInit();
}

struct BurnDriverD BurnDrvDrgw3 = {
	"drgw3", NULL, "pgm", NULL, "1998",
	"Dragon World 3 (V106, Korea)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, drgw3RomInfo, drgw3RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	drgw3Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Dragon World 3 (ver. 105)

static struct BurnRomInfo drgw3105RomDesc[] = {
	{ "dw3_v105.u12",  		0x080000, 0xc5e24318, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "dw3_v105.u13",  		0x080000, 0x8d6c9d39, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "dw3t0400.u18",  		0x400000, 0xb70f3357, 2 | BRF_GRA },	       //  2 Tile data

	{ "dw3a0400.u9",   		0x400000, 0xdd7bfd40, 3 | BRF_GRA },	       //  3 Sprite Color Data
	{ "dw3a0401.u10",  		0x400000, 0xcab6557f, 3 | BRF_GRA },	       //  4

	{ "dw3b0400.u13",  		0x400000, 0x4bb87cc0, 4 | BRF_GRA },	       //  5 Sprite Masks & Color Indexes

	{ "dw3m0400.u1",   		0x400000, 0x031eb9ce, 5 | BRF_SND },	       //  6 Samples

	{ "dw3_v100.u15",  		0x010000, 0x03dc4fdf, 0 | BRF_PRG | BRF_ESS }, //  7 Protection data
};

STDROMPICKEXT(drgw3105, drgw3105, pgm)
STD_ROM_FN(drgw3105)

struct BurnDriverD BurnDrvDrgw3105 = {
	"drgw3105", "drgw3", "pgm", NULL, "1998",
	"Dragon World 3 (ver. 105)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, drgw3105RomInfo, drgw3105RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	drgw3Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Dragon World 3 (ver. 100)

static struct BurnRomInfo drgw3100RomDesc[] = {
	{ "dw3_v100.u12",  		0x080000, 0x47243906, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "dw3_v100.u13",  		0x080000, 0xb7cded21, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "dw3t0400.u18",  		0x400000, 0xb70f3357, 2 | BRF_GRA },	       //  2 Tile data

	{ "dw3a0400.u9",   		0x400000, 0xdd7bfd40, 3 | BRF_GRA },	       //  3 Sprite Color Data
	{ "dw3a0401.u10",  		0x400000, 0xcab6557f, 3 | BRF_GRA },	       //  4

	{ "dw3b0400.u13",  		0x400000, 0x4bb87cc0, 4 | BRF_GRA },	       //  5 Sprite Masks & Color Indexes

	{ "dw3m0400.u1",   		0x400000, 0x031eb9ce, 5 | BRF_SND },	       //  6 Samples

	{ "dw3_v100.u15",  		0x010000, 0x03dc4fdf, 0 | BRF_PRG | BRF_ESS }, //  7 Protection data
};

STDROMPICKEXT(drgw3100, drgw3100, pgm)
STD_ROM_FN(drgw3100)

struct BurnDriverD BurnDrvDrgw3100 = {
	"drgw3100", "drgw3", "pgm", NULL, "1998",
	"Dragon World 3 (ver. 100)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, drgw3100RomInfo, drgw3100RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	drgw3Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Dragon World 3 EX (ver. 100)

static struct BurnRomInfo dwexRomDesc[] = {
	{ "ex_v100.u12",   		0x080000, 0xbc171799, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "ex_v100.u13",   		0x080000, 0x7afe6322, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "ex_t0400.u18",  		0x200000, 0x9ecc950d, 2 | BRF_GRA },		  //  2 Tile data

	{ "ex_a0400.u9",   		0x400000, 0xdd7bfd40, 3 | BRF_GRA },		  //  3 Sprite Color Data
	{ "ex_a0401.u10",  		0x200000, 0xd36c06a4, 3 | BRF_GRA },		  //  4

	{ "ex_b0400.u13",  		0x400000, 0x4bb87cc0, 4 | BRF_GRA },		  //  5 Sprite Masks & Color Indexes

	{ "ex_m0400.u1",   		0x200000, 0x42d54fd5, 5 | BRF_SND },		  //  6 Samples

	{ "ex_data.u15",   		0x010000, 0x03dc4fdf, 0 | BRF_PRG | BRF_ESS }, //  7 Protection data
};

STDROMPICKEXT(dwex, dwex, pgm)
STD_ROM_FN(dwex)

struct BurnDriverD BurnDrvDwex = {
	"dwex", NULL, "pgm", NULL, "1998",
	"Dragon World 3 EX (ver. 100)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, dwexRomInfo, dwexRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	drgw3Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend Super (V101, Korea)

static struct BurnRomInfo oldsRomDesc[] = {
	{ "sp_v101.u2",	   0x080000, 0x08eb9661, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "sp_v101.u3",	   0x080000, 0x0a358c1e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sp_v101.u4",	   0x080000, 0x766570e0, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sp_v101.u5",	   0x080000, 0x58662e12, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sp_v101.u1",	   0x080000, 0x2b2f4f1e, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "t0500.rom",	   0x400000, 0xd881726c, 2 | BRF_GRA },		  //  5 Tile data
	{ "t0501.rom",	   0x200000, 0xd2106864, 2 | BRF_GRA },		  //  6

	{ "a0500.rom",	   0x400000, 0x80a59197, 3 | BRF_GRA },		  //  7 Sprite Color Data
	{ "a0501.rom",	   0x400000, 0x98c931b1, 3 | BRF_GRA },		  //  8
	{ "a0502.rom",	   0x400000, 0xc3fcdf1d, 3 | BRF_GRA },		  //  9
	{ "a0503.rom",	   0x400000, 0x066dffec, 3 | BRF_GRA },		  // 10
	{ "a0504.rom",	   0x400000, 0x45337583, 3 | BRF_GRA },		  // 11
	{ "a0505.rom",	   0x400000, 0x5b8cf3a5, 3 | BRF_GRA },		  // 12
	{ "a0506.rom",	   0x400000, 0x087ac60c, 3 | BRF_GRA },		  // 13

	{ "b0500.rom",	   0x400000, 0xcde07f74, 4 | BRF_GRA },		  // 14 Sprite Masks & Color Indexes
	{ "b0501.rom",	   0x400000, 0x1546c2e9, 4 | BRF_GRA },		  // 15
	{ "b0502.rom",	   0x400000, 0xe97b31c3, 4 | BRF_GRA },		  // 16
	{ "b0503.u16",	   0x400000, 0xe41d98e4, 4 | BRF_GRA },		  // 17

	{ "m0500.rom",	   0x200000, 0x37928cdd, 5 | BRF_SND },		  // 18 Samples

	{ "sp_v101.u6",	   0x010000, 0x097046bc, 0 | BRF_PRG | BRF_ESS }, // 19 Protection Rom

	{ "ram_dump", 	   0x004000, 0x280cfb4e, 0 | BRF_PRG | BRF_ESS }, // 20 ram dump
};

STDROMPICKEXT(olds, olds, pgm)
STD_ROM_FN(olds)

static INT32 oldsInit()
{
	pPgmProtCallback = install_protection_asic25_asic28_olds;

	return pgmInit();
}

struct BurnDriver BurnDrvOlds = {
	"olds", NULL, "pgm", NULL, "1999",
	"Oriental Legend Super (V101, Korea)\0", "Imperfect Protection Emulation", "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, oldsRomInfo, oldsRomName, NULL, NULL, pgmInputInfo, oldsDIPInfo,
	oldsInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend Special / Xi Yo Gi Shi Re Zuang Super (V100)

static struct BurnRomInfo olds100RomDesc[] = {
	{ "v100-u2.040",   0x080000, 0x517c2a06, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "v100-u3.040",   0x080000, 0xd0e2b741, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "v100-u4.040",   0x080000, 0x32a6bdbd, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "v100-u5.040",   0x080000, 0xb4a1cafb, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "v100-u1.040",   0x080000, 0x37ea4e75, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "t0500.rom",	   0x400000, 0xd881726c, 2 | BRF_GRA },		  //  5 Tile data
	{ "t0501.rom",	   0x200000, 0xd2106864, 2 | BRF_GRA },		  //  6

	{ "a0500.rom",	   0x400000, 0x80a59197, 3 | BRF_GRA },		  //  7 Sprite Color Data
	{ "a0501.rom",	   0x400000, 0x98c931b1, 3 | BRF_GRA },		  //  8
	{ "a0502.rom",	   0x400000, 0xc3fcdf1d, 3 | BRF_GRA },		  //  9
	{ "a0503.rom",	   0x400000, 0x066dffec, 3 | BRF_GRA },		  // 10
	{ "a0504.rom",	   0x400000, 0x45337583, 3 | BRF_GRA },		  // 11
	{ "a0505.rom",	   0x400000, 0x5b8cf3a5, 3 | BRF_GRA },		  // 12
	{ "a0506.rom",	   0x400000, 0x087ac60c, 3 | BRF_GRA },		  // 13

	{ "b0500.rom",	   0x400000, 0xcde07f74, 4 | BRF_GRA },		  // 14 Sprite Masks & Color Indexes
	{ "b0501.rom",	   0x400000, 0x1546c2e9, 4 | BRF_GRA },		  // 15
	{ "b0502.rom",	   0x400000, 0xe97b31c3, 4 | BRF_GRA },		  // 16
	{ "b0503.u16",	   0x400000, 0xe41d98e4, 4 | BRF_GRA },		  // 17

	{ "m0500.rom",	   0x200000, 0x37928cdd, 5 | BRF_SND },		  // 18 Samples

	{ "kd-u6.512",	   0x010000, 0xe7613dda, 0 | BRF_PRG | BRF_ESS }, // 19 Protection Rom

	{ "ram_dump", 	   0x004000, 0x280cfb4e, 0 | BRF_PRG | BRF_ESS }, // 20 ram dump
};

STDROMPICKEXT(olds100, olds100, pgm)
STD_ROM_FN(olds100)

struct BurnDriver BurnDrvOlds100 = {
	"olds100", "olds", "pgm", NULL, "1998",
	"Oriental Legend Special - Xi Yo Gi Shi Re Zuang Super (V100)\0", "Imperfect Protection Emulation", "IGS", "PolyGameMaster",
	L"Oriental Legend Super\0\u897F\u6E38\u91CA\u5384\u4F20 Super (V100)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, olds100RomInfo, olds100RomName, NULL, NULL, pgmInputInfo, olds100DIPInfo,
	oldsInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend Special / Xi Yo Gi Shi Re Zuang Super (V100 alt)

static struct BurnRomInfo olds100aRomDesc[] = {
	{ "p0500.v10",	   0x400000, 0x8981fc87, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0500.rom",	   0x400000, 0xd881726c, 2 | BRF_GRA },		  //  1 Tile data
	{ "t0501.rom",	   0x200000, 0xd2106864, 2 | BRF_GRA },		  //  2

	{ "a0500.rom",	   0x400000, 0x80a59197, 3 | BRF_GRA },		  //  3 Sprite Color Data
	{ "a0501.rom",	   0x400000, 0x98c931b1, 3 | BRF_GRA },		  //  4
	{ "a0502.rom",	   0x400000, 0xc3fcdf1d, 3 | BRF_GRA },		  //  5
	{ "a0503.rom",	   0x400000, 0x066dffec, 3 | BRF_GRA },		  //  6
	{ "a0504.rom",	   0x400000, 0x45337583, 3 | BRF_GRA },		  //  7
	{ "a0505.rom",	   0x400000, 0x5b8cf3a5, 3 | BRF_GRA },		  //  8
	{ "a0506.rom",	   0x400000, 0x087ac60c, 3 | BRF_GRA },		  //  9

	{ "b0500.rom",	   0x400000, 0xcde07f74, 4 | BRF_GRA },		  // 10 Sprite Masks & Color Indexes
	{ "b0501.rom",	   0x400000, 0x1546c2e9, 4 | BRF_GRA },		  // 11
	{ "b0502.rom",	   0x400000, 0xe97b31c3, 4 | BRF_GRA },		  // 12
	{ "b0503.u16",	   0x400000, 0xe41d98e4, 4 | BRF_GRA },		  // 13

	{ "m0500.rom",	   0x200000, 0x37928cdd, 5 | BRF_SND },		  // 14 Samples
	
#if !defined (ROM_VERIFY)
	{ "protection_data.u6",	  0x010000, 0, 0 | BRF_PRG | BRF_NODUMP }, // 15 Protection Rom
#else
	{ "",                     0x000000, 0, 0                        },   // 15
#endif

	{ "ram_dump", 	   0x004000, 0x280cfb4e, 0 | BRF_PRG | BRF_ESS }, // 16 ram dump
};

STDROMPICKEXT(olds100a, olds100a, pgm)
STD_ROM_FN(olds100a)

struct BurnDriver BurnDrvOlds100a = {
	"olds100a", "olds", "pgm", NULL, "1998",
	"Oriental Legend Special - Xi Yo Gi Shi Re Zuang Super (V100 alt)\0", "Imperfect Protection Emulation", "IGS", "PolyGameMaster",
	L"Oriental Legend Super\0\u897F\u6E38\u91CA\u5384\u4F20 Super (V100 alt)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM, GBF_SCRFIGHT, 0,
	NULL, olds100aRomInfo, olds100aRomName, NULL, NULL, pgmInputInfo, olds100DIPInfo,
	oldsInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour / Sangoku Senki (V117)

static struct BurnRomInfo kovRomDesc[] = {
	{ "p0600.117",     0x400000, 0xc4d19fe6, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  3
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  4
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },		  //  5
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  //  6 Sprite Masks & Color Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },		  //  7
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  //  8 Samples
	
	{ "kov_igs027a.bin", 0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, //  9 Internal ARM7 Rom
};

STDROMPICKEXT(kov, kov, pgm)
STD_ROM_FN(kov)

static INT32 kovInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_kov;
	pPgmProtCallback = install_protection_asic27_kov; // simulation

	return pgmInit();
}

struct BurnDriver BurnDrvKov = {
	"kov", NULL, "pgm", NULL, "1999",
	"Knights of Valour - Sangoku Senki (V117)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour\0\u4E09\u56FD\u6226\u7D00\0\u4E09\u56FD\u6218\u7EAA (V117)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovRomInfo, kovRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour / Sangoku Senki (V115)

static struct BurnRomInfo kov115RomDesc[] = {
	{ "p0600.115",     0x400000, 0x527a2924, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  3
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  4
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },		  //  5
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  //  6 Sprite Masks & Color Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },		  //  7
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  //  8 Samples
	
	{ "kov_igs027a.bin", 0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, //  9 Internal ARM7 Rom
};

STDROMPICKEXT(kov115, kov115, pgm)
STD_ROM_FN(kov115)

struct BurnDriver BurnDrvKov115 = {
	"kov115", "kov", "pgm", NULL, "1999",
	"Knights of Valour - Sangoku Senki (V115)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour\0\u4E09\u56FD\u6226\u7D00\0\u4E09\u56FD\u6218\u7EAA (V115)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kov115RomInfo, kov115RomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour Plus / Sangoku Senki (V100, Japan)

static struct BurnRomInfo kov100RomDesc[] = {
	{ "sav111.u4",     0x080000, 0xae2f1b4e, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "sav111.u7",     0x080000, 0x95eedf0e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sav111.u5",     0x080000, 0x5fdd4aa8, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sav111.u8",     0x080000, 0x003cbf49, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sav111.u10",    0x080000, 0xd5536107, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  5 Tile data
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  //  6 Sprite Color Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  7
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  8
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },		  //  9
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // 10 Sprite Masks & Color Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },		  // 11
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // 12 Samples	
	
	{ "kov_igs027a.bin", 0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 13 Internal ARM7 Rom
};

STDROMPICKEXT(kov100, kov100, pgm)
STD_ROM_FN(kov100)

struct BurnDriver BurnDrvKov100 = {
	"kov100", "kov", "pgm", NULL, "1999",
	"Knights of Valour - Sangoku Senki (V100, Japan)\0", NULL, "IGS (Alta Co., LTD License)", "PolyGameMaster",
	L"Knights of Valour\0\u4E09\u56FD\u6226\u7D00\0\u4E09\u56FD\u6218\u7EAA (V100, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kov100RomInfo, kov100RomName, NULL, NULL, pgmInputInfo, kov100DIPInfo,
	kovInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour Plus / Sangoku Senki Plus (V119)

static struct BurnRomInfo kovplusRomDesc[] = {
	{ "p0600.119",     0x400000, 0xe4b0875d, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  3
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  4
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },		  //  5
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  //  6 Sprite Masks & Color Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },		  //  7
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  //  8 Samples	
	
	{ "kov_igs027a.bin", 0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, //  9 Internal ARM7 Rom
};

STDROMPICKEXT(kovplus, kovplus, pgm)
STD_ROM_FN(kovplus)

struct BurnDriver BurnDrvKovplus = {
	"kovplus", NULL, "pgm", NULL, "1999",
	"Knights of Valour Plus - Sangoku Senki Plus (V119)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour Plus\0\u4E09\u56FD\u6226\u7D00 Plus\0\u4E09\u56FD\u6218\u7EAA Plus (V119)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovplusRomInfo, kovplusRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour Plus / Sangoku Senki Plus (V119 alt)

static struct BurnRomInfo kovplusaRomDesc[] = {
	{ "v119.u3",       0x080000, 0x6750388f, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "v119.u5",       0x080000, 0xd4101ffd, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "v119.u4",       0x080000, 0x8200ece6, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "v119.u6",       0x080000, 0x71e28f27, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "v119.u2",	   0x080000, 0x29588ef2, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  5 Tile data
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  //  6 Sprite Color Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  7
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  8
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },		  //  9
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // 10 Sprite Masks & Color Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },		  // 11
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // 12 Samples	
	
	{ "kov_igs027a.bin", 0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 13 Internal ARM7 Rom
};

STDROMPICKEXT(kovplusa, kovplusa, pgm)
STD_ROM_FN(kovplusa)

struct BurnDriver BurnDrvKovplusa = {
	"kovplusa", "kovplus", "pgm", NULL, "1999",
	"Knights of Valour Plus - Sangoku Senki Plus (V119 alt)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour Plus\0\u4E09\u56FD\u6226\u7D00 Plus\0\u4E09\u56FD\u6218\u7EAA Plus (V119 alt)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovplusaRomInfo, kovplusaRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour Superheroes / Sangoku Senki Superheroes (V104, China)

static struct BurnRomInfo kovshRomDesc[] = {
	{ "p0600.322",	   0x400000, 0x7c78e5f3, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0600.rom",	   0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",	   0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  3
	{ "a0602.rom",	   0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  4
	{ "a0603.rom",	   0x400000, 0xec31abda, 3 | BRF_GRA },		  //  5
	{ "a0604.rom",	   0x400000, 0x26b59fd3, 3 | BRF_GRA },		  //  6

	{ "b0600.rom",	   0x800000, 0x7d3cd059, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b0601.rom",	   0x400000, 0xa0bb1c2f, 4 | BRF_GRA },		  //  8
	{ "b0602.rom",	   0x100000, 0x9df77934, 4 | BRF_GRA },		  //  9

	{ "m0600.rom",	   0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // 10 Samples

	{ "kovsh_v100_china.asic", 0x04000, 0x0f09a5c1, 7 | BRF_ESS | BRF_PRG }, // 11 Internal ARM7 Rom
};

STDROMPICKEXT(kovsh, kovsh, pgm)
STD_ROM_FN(kovsh)

static INT32 kovshInit()
{
	pPgmInitCallback = pgm_decrypt_kovsh;
	pPgmProtCallback = install_protection_asic27a_kovsh;
	
	INT32 nRet = pgmInit();

	Arm7SetIdleLoopAddress(0x00000260);

	return nRet;
}

struct BurnDriver BurnDrvKovsh = {
	"kovsh", NULL, "pgm", NULL, "1999",
	"Knights of Valour Superheroes / Sangoku Senki Superheroes (V104, China)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour Superheroes\0\u4E09\u56FD\u6218\u7EAA\0\u98CE\u4E91\u518D\u8D77 (V104, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kovshRomInfo, kovshRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovshInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour Superheroes / Sangoku Senki Superheroes (V103)

static struct BurnRomInfo kovsh103RomDesc[] = {
	{ "p0600.103",	   0x400000, 0xf0b3da82, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0600.rom",	   0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",	   0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  3
	{ "a0602.rom",	   0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  4
	{ "a0603.rom",	   0x400000, 0xec31abda, 3 | BRF_GRA },		  //  5
	{ "a0604.rom",	   0x400000, 0x26b59fd3, 3 | BRF_GRA },		  //  6

	{ "b0600.rom",	   0x800000, 0x7d3cd059, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b0601.rom",	   0x400000, 0xa0bb1c2f, 4 | BRF_GRA },		  //  8
	{ "b0602.rom",	   0x100000, 0x9df77934, 4 | BRF_GRA },		  //  9

	{ "m0600.rom",	   0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // 10 Samples

	{ "kovsh_v100_china.asic", 0x04000, 0x0f09a5c1, 7 | BRF_ESS | BRF_PRG }, // 11 Internal ARM7 Rom
};

STDROMPICKEXT(kovsh103, kovsh103, pgm)
STD_ROM_FN(kovsh103)

struct BurnDriver BurnDrvKovsh103 = {
	"kovsh103", "kovsh", "pgm", NULL, "1999",
	"Knights of Valour Superheroes / Sangoku Senki Superheroes (V103)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kovsh103RomInfo, kovsh103RomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovshInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Photo Y2K (ver. 105)

static struct BurnRomInfo photoy2kRomDesc[] = {
	{ "p0701_v105.u2", 0x200000, 0xfab142e0, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0700.rom",     0x080000, 0x93943b4d, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0700.l",       0x400000, 0x26a9ae9c, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0700.h",       0x400000, 0x79bc1fc1, 3 | BRF_GRA },		  //  3
	{ "a0701.l",       0x400000, 0x23607f81, 3 | BRF_GRA },		  //  4
	{ "a0701.h",       0x400000, 0x5f2efd37, 3 | BRF_GRA },		  //  5
	{ "a0702.rom",     0x080000, 0x42239e1b, 3 | BRF_GRA },		  //  6

	{ "b0700.l",       0x400000, 0xaf096904, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b0700.h",       0x400000, 0x6d53de26, 4 | BRF_GRA },		  //  8
	{ "cgv101.rom",    0x020000, 0xda02ec3e, 4 | BRF_GRA },		  //  9

	{ "m0700.rom",     0x080000, 0xacc7afce, 5 | BRF_SND },		  // 10 Samples

	{ "igs027a_photoy2k_v100_china.asic",	0x004000, 0x1a0b68f6, 7 | BRF_PRG | BRF_ESS }, // 11 Internal ARM7 Rom
};

STDROMPICKEXT(photoy2k, photoy2k, pgm)
STD_ROM_FN(photoy2k)

static INT32 photoy2kInit()
{
	pPgmInitCallback = pgm_decrypt_photoy2k;
	pPgmProtCallback = install_protection_asic27a_kovsh;

	INT32 nRet = pgmInit();
	
	Arm7SetIdleLoopAddress(0x000001f4);

	return nRet;
}

struct BurnDriver BurnDrvPhotoy2k = {
	"photoy2k", NULL, "pgm", NULL, "1999",
	"Photo Y2K / Real and Fake (V105)\0", NULL, "IGS", "PolyGameMaster",
	L"Photo Y2K\0\u30EA\u30A2\u30EB\u30A2\u30F3\u30C9 \u30D5\u30A7\u30A4\u30AF\0\u5927\u5BB6\u6765 \u627E\u78B4\0\u8D85\u7EA7 \u6BD4\u4E00\u6BD4 (V105)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_PUZZLE, 0,
	NULL, photoy2kRomInfo, photoy2kRomName, NULL, NULL, pgmInputInfo, photoy2kDIPInfo,
	photoy2kInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Photo Y2K / Real and Fake (V104)

static struct BurnRomInfo photoy2k104RomDesc[] = {
	{ "v104.16m",      0x200000, 0xe051070f, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0700.rom",     0x080000, 0x93943b4d, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0700.l",       0x400000, 0x26a9ae9c, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0700.h",       0x400000, 0x79bc1fc1, 3 | BRF_GRA },		  //  3
	{ "a0701.l",       0x400000, 0x23607f81, 3 | BRF_GRA },		  //  4
	{ "a0701.h",       0x400000, 0x5f2efd37, 3 | BRF_GRA },		  //  5
	{ "a0702.rom",     0x080000, 0x42239e1b, 3 | BRF_GRA },		  //  6

	{ "b0700.l",       0x400000, 0xaf096904, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b0700.h",       0x400000, 0x6d53de26, 4 | BRF_GRA },		  //  8
	{ "cgv101.rom",    0x020000, 0xda02ec3e, 4 | BRF_GRA },		  //  9

	{ "m0700.rom",     0x080000, 0xacc7afce, 5 | BRF_SND },		  // 10 Samples

	{ "igs027a_photoy2k_v100_china_alt.asic", 0x004000,  0x6dd7f257, 7 | BRF_PRG | BRF_ESS }, // 11 Internal ARM7 Rom
};

STDROMPICKEXT(photoy2k104, photoy2k104, pgm)
STD_ROM_FN(photoy2k104)

struct BurnDriver BurnDrvPhotoy2k104 = {
	"photoy2k104", "photoy2k", "pgm", NULL, "1999",
	"Photo Y2K / Real and Fake (V104)\0", NULL, "IGS", "PolyGameMaster",
	L"Photo Y2K\0\u30EA\u30A2\u30EB\u30A2\u30F3\u30C9 \u30D5\u30A7\u30A4\u30AF\0\u5927\u5BB6\u6765 \u627E\u78B4\0\u8D85\u7EA7 \u6BD4\u4E00\u6BD4 (V104)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_PUZZLE, 0,
	NULL, photoy2k104RomInfo, photoy2k104RomName, NULL, NULL, pgmInputInfo, photoy2kDIPInfo,
	photoy2kInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Real and Fake / Photo Y2K (V102, Japan)

static struct BurnRomInfo photoy2k102RomDesc[] = {
	{ "v102.u4",       0x080000, 0xa65eda9f, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "v102.u6",       0x080000, 0xb9ca5504, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "v102.u5",       0x080000, 0x9201621b, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "v102.u8",       0x080000, 0x3be22b8f, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "t0700.rom",     0x080000, 0x93943b4d, 2 | BRF_GRA },		  //  4 Tile data

	{ "a0700.l",       0x400000, 0x26a9ae9c, 3 | BRF_GRA },		  //  5 Sprite Color Data
	{ "a0700.h",       0x400000, 0x79bc1fc1, 3 | BRF_GRA },		  //  6
	{ "a0701.l",       0x400000, 0x23607f81, 3 | BRF_GRA },		  //  7
	{ "a0701.h",       0x400000, 0x5f2efd37, 3 | BRF_GRA },		  //  8
	{ "a0702.rom",     0x080000, 0x42239e1b, 3 | BRF_GRA },		  //  9

	{ "b0700.l",       0x400000, 0xaf096904, 4 | BRF_GRA },		  // 10 Sprite Masks & Color Indexes
	{ "b0700.h",       0x400000, 0x6d53de26, 4 | BRF_GRA },		  // 11
	{ "cgv101.rom",    0x020000, 0xda02ec3e, 4 | BRF_GRA },		  // 12

	{ "m0700.rom",     0x080000, 0xacc7afce, 5 | BRF_SND },		  // 13 Samples

	{ "igs027a_photoy2k_v100_china_alt.asic", 0x0004000,  0x6dd7f257, 7 | BRF_PRG | BRF_ESS }, // 14 Internal ARM7 Rom
};

STDROMPICKEXT(photoy2k102, photoy2k102, pgm)
STD_ROM_FN(photoy2k102)

struct BurnDriver BurnDrvphotoy2k102 = {
	"photoy2k102", "photoy2k", "pgm", NULL, "1999",
	"Real and Fake\0Photo Y2K (V102, Japan)\0", NULL, "IGS", "PolyGameMaster",
	L"\u30EA\u30A2\u30EB\u30A2\u30F3\u30C9 \u30D5\u30A7\u30A4\u30AF\0Photo Y2K\0\u5927\u5BB6\u6765 \u627E\u78B4\0\u8D85\u7EA7 \u6BD4\u4E00\u6BD4 (V102, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING  | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_PUZZLE, 0,
	NULL, photoy2k102RomInfo, photoy2k102RomName, NULL, NULL, pgmInputInfo, raf102jDIPInfo,
	photoy2kInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Puzzle Star (V100MG)

static struct BurnRomInfo puzlstarRomDesc[] = {
	{ "v100mg.u2",	   0x080000, 0x4c79d979, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "v100mg.u1",	   0x080000, 0x5788b77d, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "t0800.u5",	   0x200000, 0xf9d84e59, 2 | BRF_GRA }, 	  //  2 Tile data

	{ "a0800.u1",	   0x400000, 0xe1e6ec40, 3 | BRF_GRA },		  //  3 Sprite Color Data

	{ "b0800.u3",	   0x200000, 0x52e7bef5, 4 | BRF_GRA }, 	  //  4 Sprite Masks & Color Indexes

	{ "m0800.u2",	   0x400000, 0xe1a46541, 5 | BRF_SND },		  //  5 Samples

	{ "puzlstar_igs027a.bin", 0x004000,   0, 7 | BRF_PRG | BRF_NODUMP }, //  6 Internal ARM7 Rom
};

STDROMPICKEXT(puzlstar, puzlstar, pgm)
STD_ROM_FN(puzlstar)

static INT32 puzlstarInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_puzlstar;
	pPgmProtCallback = install_protection_asic27a_puzlstar; // simulation
//	pPgmProtCallback = install_protection_asic27a_kovsh;

	return pgmInit();
}

struct BurnDriver BurnDrvPuzlstar = {
	"puzlstar", NULL, "pgm", NULL, "1999",
	"Puzzle Star (V100MG)\0", "Incomplete Protection Emulation", "IGS", "PolyGameMaster",
	L"Puzzle Star\0\u30D1\u30BA\u30EB\u30B9\u30BF\u30FC\0\u9B54\u5E7B\u661F\u5EA7 (V100MG)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_PUZZLE, 0,
	NULL, puzlstarRomInfo, puzlstarRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	puzlstarInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Puzzli 2 Super (V200)

static struct BurnRomInfo puzzli2RomDesc[] = {
	{ "2sp_v200.u4",   0x080000, 0xfa5c86c1, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "2sp_v200.u3",   0x080000, 0x2a5ba8a6, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "t0900.u9",	   0x200000, 0x70615611, 2 | BRF_GRA },		  //  2 Tile data

	{ "a0900.u3",	   0x400000, 0x14911251, 3 | BRF_GRA },		  //  3 Sprite Color Data

	{ "b0900.u4",	   0x200000, 0x6f0638b6, 4 | BRF_GRA },		  //  4 Sprite Masks & Color Indexes

	{ "m0900.u2",	   0x400000, 0x9ea7af2e, 5 | BRF_SND },		  //  5 Samples

	{ "puzzli2_igs027a.bin", 0x004000, 0,    7 | BRF_PRG | BRF_NODUMP }, //  6 Internal ARM7 Rom
};

STDROMPICKEXT(puzzli2, puzzli2, pgm)
STD_ROM_FN(puzzli2)

static INT32 puzzli2Init()
{
	pPgmInitCallback = pgm_decrypt_puzzli2;
	pPgmProtCallback = install_protection_asic27a_puzzli2; // simulation
//	pPgmProtCallback = install_protection_asic27a_kovsh;

	return pgmInit();
}

struct BurnDriverD BurnDrvPuzzli2 = {
	"puzzli2", NULL, "pgm", NULL, "2001",
	"Puzzli 2 Super (V200)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_PUZZLE, 0,
	NULL, puzzli2RomInfo, puzzli2RomName, NULL, NULL, pgmInputInfo, puzzli2DIPInfo,
	puzzli2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Martial Masters / Xing Yi (V104, 102, 102, USA)

static struct BurnRomInfo martmastRomDesc[] = {
	{ "v104_32m.u9",   0x400000, 0xcfd9dff4, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1000.u3",	   0x800000, 0xbbf879b5, 2 | BRF_GRA },	          //  1 Tile data

	{ "a1000.u3",      0x800000, 0x43577ac8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1001.u4",      0x800000, 0xfe7a476f, 3 | BRF_GRA },		  //  3
	{ "a1002.u6",      0x800000, 0x62e33d38, 3 | BRF_GRA },		  //  4
	{ "a1003.u8",      0x800000, 0xb2c4945a, 3 | BRF_GRA },		  //  5
	{ "a1004.u10",     0x400000, 0x9fd3f5fd, 3 | BRF_GRA },		  //  6

	{ "b1000.u9",	   0x800000, 0xc5961f6f, 4 | BRF_GRA },	          //  7 Sprite Masks & Color Indexes
	{ "b1001.u11",	   0x800000, 0x0b7e1c06, 4 | BRF_GRA },		  //  8

	{ "m1000.u5",      0x800000, 0xed407ae8, 5 | BRF_SND },	          //  9 Samples
	{ "m1001.u7",      0x400000, 0x662d2d48, 5 | BRF_SND },		  // 10 

	{ "martial_masters_v102_usa.asic", 0x4000,  0xa6c0828c, 7 | BRF_PRG | BRF_ESS }, // 11 Internal ARM7 Rom

	{ "v102_16m.u10",  0x200000, 0x18b745e6, 8 | BRF_PRG | BRF_ESS }, // 12 External ARM7 Rom
};

STDROMPICKEXT(martmast, martmast, pgm)
STD_ROM_FN(martmast)

static INT32 martmastInit()
{
	pPgmInitCallback = pgm_decrypt_martmast;
	pPgmProtCallback = install_protection_asic27a_martmast;
	
	INT32 nRet = pgmInit();
	
	Arm7SetIdleLoopAddress(0x800039e);

	return nRet;
}

struct BurnDriver BurnDrvMartmast = {
	"martmast", NULL, "pgm", NULL, "2001",
	"Martial Masters (V104, 102, 102, USA)\0", NULL, "IGS", "PolyGameMaster",
	L"Martial Masters\0\u5f62\u610f\u62f3 (V104, 102, 102, USA)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_VSFIGHT, 0,
	NULL, martmastRomInfo, martmastRomName, NULL, NULL, pgmInputInfo, martmastDIPInfo,
	martmastInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Martial Masters / Xing Yi (V104, 102, 101, China)

static struct BurnRomInfo martmastcRomDesc[] = {
	{ "v104_32m.u9",   0x400000, 0xcfd9dff4, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1000.u3",	   0x800000, 0xbbf879b5, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1000.u3",      0x800000, 0x43577ac8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1001.u4",      0x800000, 0xfe7a476f, 3 | BRF_GRA },		  //  3
	{ "a1002.u6",      0x800000, 0x62e33d38, 3 | BRF_GRA },		  //  4
	{ "a1003.u8",      0x800000, 0xb2c4945a, 3 | BRF_GRA },		  //  5
	{ "a1004.u10",     0x400000, 0x9fd3f5fd, 3 | BRF_GRA },		  //  6

	{ "b1000.u9",	   0x800000, 0xc5961f6f, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b1001.u11",	   0x800000, 0x0b7e1c06, 4 | BRF_GRA },		  //  8

	{ "m1000.u5",      0x800000, 0xed407ae8, 5 | BRF_SND },		  //  9 Samples
	{ "m1001.u7",      0x400000, 0x662d2d48, 5 | BRF_SND },		  // 10

	{ "martial_masters_v101_cn.asic", 0x4000, 0xb3e25b7d, 7 | BRF_PRG | BRF_ESS }, // 11 Internal ARM7 Rom

	{ "v102_16m.u10",  0x200000, 0x18b745e6, 8 | BRF_PRG | BRF_ESS },  // 12 External ARM7 Rom
};

STDROMPICKEXT(martmastc, martmastc, pgm)
STD_ROM_FN(martmastc)

struct BurnDriver BurnDrvmartmastc = {
	"martmastc", "martmast", "pgm", NULL, "2001",
	"Martial Masters (V104, 102, 101, China)\0", NULL, "IGS", "PolyGameMaster",
	L"Martial Masters\0\u5f62\u610f\u62f3 (V104, 102, 101, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_VSFIGHT, 0,
	NULL, martmastcRomInfo, martmastcRomName, NULL, NULL, pgmInputInfo, martmastcDIPInfo,
	martmastInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Martial Masters / Xing Yi (V102, 101, 101, China)

static struct BurnRomInfo martmastc102RomDesc[] = {
	{ "martmast_u9-v102.322",		0x400000, 0xBB24B92A, 1 | BRF_PRG | BRF_ESS },  //  0 68K Code

	{ "t1000.u3",	 			0x800000, 0xbbf879b5, 2 | BRF_GRA },		//  1 Tile data

	{ "a1000.u3",    			0x800000, 0x43577ac8, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a1001.u4",    			0x800000, 0xfe7a476f, 3 | BRF_GRA },		//  3
	{ "a1002.u6",    			0x800000, 0x62e33d38, 3 | BRF_GRA },		//  4
	{ "a1003.u8",    			0x800000, 0xb2c4945a, 3 | BRF_GRA },		//  5
	{ "a1004.u10",   			0x400000, 0x9fd3f5fd, 3 | BRF_GRA },		//  6

	{ "b1000.u9",	  			0x800000, 0xc5961f6f, 4 | BRF_GRA },		//  7 Sprite Masks & Color Indexes
	{ "b1001.u11",	  			0x800000, 0x0b7e1c06, 4 | BRF_GRA },		//  8

	{ "m1000.u5",     			0x800000, 0xed407ae8, 5 | BRF_SND },		//  9 Samples
	{ "m1001.u7",      			0x400000, 0x662d2d48, 5 | BRF_SND },		// 10

	{ "martial_masters_v101_cn.asic",	0x004000, 0xb3e25b7d, 7 | BRF_PRG | BRF_ESS },  // 11 Internal ARM7 Rom

	{ "martmast_u10-v101.160",		0x200000, 0xD5D93294, 8 | BRF_PRG | BRF_ESS },  // 12 External ARM7 Rom
};

STDROMPICKEXT(martmastc102, martmastc102, pgm)
STD_ROM_FN(martmastc102)

struct BurnDriver BurnDrvmartmastc102 = {
	"martmastc102", "martmast", "pgm", NULL, "2001",
	"Martial Masters (V102, 101, 101, China)\0", NULL, "IGS", "PolyGameMaster",
	L"Martial Masters\0\u5f62\u610f\u62f3 (V102, 101, 101, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_VSFIGHT, 0,
	NULL, martmastc102RomInfo, martmastc102RomName, NULL, NULL, pgmInputInfo, martmastc102DIPInfo,
	martmastInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Photo Y2K 2

static struct BurnRomInfo py2k2RomDesc[] = {
	{ "y2k2_m-101xx.u1",0x200000, 0xc47795f1, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "y2k2_a1100.u6", 0x800000, 0xe32ce499, 3 | BRF_GRA },		  //  1 Sprite Color Data
	{ "y2k2_a1101.u7", 0x800000, 0x4e7568bc, 3 | BRF_GRA },		  //  2
	{ "y2k2_a1102.u8", 0x800000, 0x6da7c143, 3 | BRF_GRA },		  //  3
	{ "y2k2_a1103.u9", 0x800000, 0x0ebebfdc, 3 | BRF_GRA },		  //  4

	{ "y2k2_b1100.u4", 0x800000, 0xfa53d6f6, 4 | BRF_GRA },		  //  5 Sprite Masks & Color Indexes
	{ "y2k2_b1101.u5", 0x800000, 0x001e4c81, 4 | BRF_GRA },		  //  6

	{ "y2k2_m1100.u3", 0x200000, 0xfb1515f8, 5 | BRF_SND },		  //  7 Samples

	{ "igs027a_photoy2k2.asic",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_ESS| BRF_NODUMP }, //  8 Internal ARM7 Rom
};

STDROMPICKEXT(py2k2, py2k2, pgm)
STD_ROM_FN(py2k2)

static INT32 py2k2Init()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_py2k2;
//	pPgmProtCallback = install_protection_asic27a_kovsh;

	return pgmInit();
}

struct BurnDriverD BurnDrvPy2k2 = {
	"py2k2", NULL, "pgm", NULL, "2001",
	"Photo Y2K 2\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_PUZZLE, 0,
	NULL, py2k2RomInfo, py2k2RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	py2k2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour 2 (V107, 102, 100, Hong Kong)

static struct BurnRomInfo kov2RomDesc[] = {
	{ "u18.107",	  0x400000, 0x661a5b2c, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1200.rom",	   0x800000, 0xd7e26609, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1200.rom",	   0x800000, 0xceeb81d8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1201.rom",	   0x800000, 0x82f0a878, 3 | BRF_GRA },		  //  3
	{ "a1202.rom",	   0x800000, 0x4bb92fae, 3 | BRF_GRA },		  //  4
	{ "a1203.rom",	   0x800000, 0xe73cb627, 3 | BRF_GRA },		  //  5
	{ "a1204.rom",	   0x800000, 0x27527099, 3 | BRF_GRA },		  //  6

	{ "b1200.rom",	   0x800000, 0xbed7d994, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b1201.rom",	   0x800000, 0xf251eb57, 4 | BRF_GRA },		  //  8

	{ "m1200.rom",	   0x800000, 0xb0d88720, 5 | BRF_SND },		  //  9 Samples

	{ "kov2_v100_hongkong.asic", 0x4000, 0xe0d7679f, 7 | BRF_PRG | BRF_ESS }, // 10 Internal ARM7 Rom

	{ "u19.102",	   0x200000, 0x462e2980, 8 | BRF_PRG | BRF_ESS },  // 11 External ARM7 Rom
};

STDROMPICKEXT(kov2, kov2, pgm)
STD_ROM_FN(kov2)

static INT32 kov2Init()
{
	pPgmInitCallback = pgm_decrypt_kov2;
	pPgmProtCallback = install_protection_asic27a_martmast;
	
	INT32 nRet = pgmInit();
	
	Arm7SetIdleLoopAddress(0x80000ae);

	return nRet;
}

struct BurnDriver BurnDrvKov2 = {
	"kov2", NULL, "pgm", NULL, "2000",
	"Knights of Valour 2 (V107, 102, 100, Hong Kong)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour 2\0\u4e09\u56fd\u6218\u7eaa 2 (V107, 102, 100, Hong Kong)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kov2RomInfo, kov2RomName, NULL, NULL, pgmInputInfo, kov2DIPInfo,
	kov2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour 2 (V106, 102, 100 Hong Kong)

static struct BurnRomInfo kov2106RomDesc[] = {
	{ "u18.106",	   0x400000, 0x40051ad9, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1200.rom",	   0x800000, 0xd7e26609, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1200.rom",	   0x800000, 0xceeb81d8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1201.rom",	   0x800000, 0x82f0a878, 3 | BRF_GRA },		  //  3
	{ "a1202.rom",	   0x800000, 0x4bb92fae, 3 | BRF_GRA },		  //  4
	{ "a1203.rom",	   0x800000, 0xe73cb627, 3 | BRF_GRA },		  //  5
	{ "a1204.rom",	   0x800000, 0x27527099, 3 | BRF_GRA },		  //  6

	{ "b1200.rom",	   0x800000, 0xbed7d994, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b1201.rom",	   0x800000, 0xf251eb57, 4 | BRF_GRA },		  //  8

	{ "m1200.rom",	   0x800000, 0xb0d88720, 5 | BRF_SND },		  //  9 Samples

	{ "kov2_v100_hongkong.asic", 0x4000, 0xe0d7679f, 7 | BRF_PRG | BRF_ESS }, // 10 Internal ARM7 Rom

	{ "u19.102",	   0x200000, 0x462e2980, 8 | BRF_PRG | BRF_ESS }, // 11 External ARM7 Rom
};

STDROMPICKEXT(kov2106, kov2106, pgm)
STD_ROM_FN(kov2106)

struct BurnDriver BurnDrvKov2106 = {
	"kov2106", "kov2", "pgm", NULL, "2000",
	"Knights of Valour 2 (V106, 102, 100 Hong Kong)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour 2\0\u4e09\u56fd\u6218\u7eaa 2 (V106, 102, 100 Hong Kong)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kov2106RomInfo, kov2106RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	kov2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour 2 (V103, 101, 100, Hong Kong)

static struct BurnRomInfo kov2103RomDesc[] = {
	{ "u18.103",	   0x400000, 0x98c32f76, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1200.rom",	   0x800000, 0xd7e26609, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1200.rom",	   0x800000, 0xceeb81d8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1201.rom",	   0x800000, 0x82f0a878, 3 | BRF_GRA },		  //  3
	{ "a1202.rom",	   0x800000, 0x4bb92fae, 3 | BRF_GRA },		  //  4
	{ "a1203.rom",	   0x800000, 0xe73cb627, 3 | BRF_GRA },		  //  5
	{ "a1204.rom",	   0x800000, 0x27527099, 3 | BRF_GRA },		  //  6

	{ "b1200.rom",	   0x800000, 0xbed7d994, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b1201.rom",	   0x800000, 0xf251eb57, 4 | BRF_GRA },		  //  8

	{ "m1200.rom",	   0x800000, 0xb0d88720, 5 | BRF_SND },		  //  9 Samples

	{ "kov2_v100_hongkong.asic",	0x004000, 0xe0d7679f, 7 | BRF_PRG | BRF_ESS }, // 10 Internal ARM7 Rom

	{ "u19.101",	   0x200000, 0x8c35f2fe, 8 | BRF_PRG | BRF_ESS }, // 11 External ARM7 Rom
};

STDROMPICKEXT(kov2103, kov2103, pgm)
STD_ROM_FN(kov2103)

struct BurnDriver BurnDrvKov2103 = {
	"kov2103", "kov2", "pgm", NULL, "2000",
	"Knights of Valour 2 (V103, 101, 100, Hong Kong)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour 2\0\u4e09\u56fd\u6218\u7eaa 2 (V103, 101, 100, Hong Kong)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kov2103RomInfo, kov2103RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	kov2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour 2 (V102, 101, 100, Hong Kong)

static struct BurnRomInfo kov2102RomDesc[] = {
	{ "u18.102",	   0x400000, 0xa2489c37, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1200.rom",	   0x800000, 0xd7e26609, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1200.rom",	   0x800000, 0xceeb81d8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1201.rom",	   0x800000, 0x82f0a878, 3 | BRF_GRA },		  //  3
	{ "a1202.rom",	   0x800000, 0x4bb92fae, 3 | BRF_GRA },		  //  4
	{ "a1203.rom",	   0x800000, 0xe73cb627, 3 | BRF_GRA },		  //  5
	{ "a1204.rom",	   0x800000, 0x27527099, 3 | BRF_GRA },		  //  6

	{ "b1200.rom",	   0x800000, 0xbed7d994, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b1201.rom",	   0x800000, 0xf251eb57, 4 | BRF_GRA },		  //  8

	{ "m1200.rom",	   0x800000, 0xb0d88720, 5 | BRF_SND },		  //  9 Samples

	{ "kov2_v100_hongkong.asic",	0x004000, 0xe0d7679f, 7 | BRF_PRG | BRF_ESS }, // 10 Internal ARM7 Rom

	{ "u19.101",	   0x200000, 0x8c35f2fe, 8 | BRF_PRG | BRF_ESS }, // 11 External ARM7 Rom
};

STDROMPICKEXT(kov2102, kov2102, pgm)
STD_ROM_FN(kov2102)

struct BurnDriver BurnDrvKov2102 = {
	"kov2102", "kov2", "pgm", NULL, "2000",
	"Knights of Valour 2 (V102, 101, 100, Hong Kong)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour 2\0\u4e09\u56fd\u6218\u7eaa 2 (V102, 101, 100, Hong Kong)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kov2102RomInfo, kov2102RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	kov2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour 2 (V100, 100, 100, Hong Kong)

static struct BurnRomInfo kov2100RomDesc[] = {
	{ "igs_u18.rom",   0x400000, 0x86205879, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1200.rom",	   0x800000, 0xd7e26609, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1200.rom",	   0x800000, 0xceeb81d8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1201.rom",	   0x800000, 0x82f0a878, 3 | BRF_GRA },		  //  3
	{ "a1202.rom",	   0x800000, 0x4bb92fae, 3 | BRF_GRA },		  //  4
	{ "a1203.rom",	   0x800000, 0xe73cb627, 3 | BRF_GRA },		  //  5
	{ "a1204.rom",	   0x800000, 0x27527099, 3 | BRF_GRA },		  //  6

	{ "b1200.rom",	   0x800000, 0xbed7d994, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b1201.rom",	   0x800000, 0xf251eb57, 4 | BRF_GRA },		  //  8

	{ "m1200.rom",	   0x800000, 0xb0d88720, 5 | BRF_SND },		  //  9 Samples

	{ "kov2_v100_hongkong.asic",	0x004000, 0xe0d7679f, 7 | BRF_PRG | BRF_ESS }, // 10 Internal ARM7 Rom

	{ "u19.100",	   0x200000, 0xedd59922, 8 | BRF_PRG | BRF_ESS }, // 11 External ARM7 Rom
};

STDROMPICKEXT(kov2100, kov2100, pgm)
STD_ROM_FN(kov2100)

struct BurnDriver BurnDrvKov2100 = {
	"kov2100", "kov2", "pgm", NULL, "2000",
	"Knights of Valour 2 (V100, 100, 100, Hong Kong)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour 2\0\u4e09\u56fd\u6218\u7eaa 2 (V100, 100, 100, Hong Kong)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kov2100RomInfo, kov2100RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	kov2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour 2 Plus - Nine Dragons (VM204XX)

static struct BurnRomInfo kov2pRomDesc[] = {
	{ "v204-32m.rom",  0x400000, 0x583e0650, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1200.rom",	   0x800000, 0xd7e26609, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1200.rom",	   0x800000, 0xceeb81d8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1201.rom_p",   0x800000, 0x21063ca7, 3 | BRF_GRA },		  //  3
	{ "a1202.rom",	   0x800000, 0x4bb92fae, 3 | BRF_GRA },		  //  4
	{ "a1203.rom",	   0x800000, 0xe73cb627, 3 | BRF_GRA },		  //  5
	{ "a1204.rom_p",   0x200000, 0x14b4b5bb, 3 | BRF_GRA },		  //  6

	{ "b1200.rom",	   0x800000, 0xbed7d994, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b1201.rom",	   0x800000, 0xf251eb57, 4 | BRF_GRA },		  //  8

	{ "m1200.rom",	   0x800000, 0xb0d88720, 5 | BRF_SND },		  //  9 Samples

	{ "kov2p_igs027a.bin",	   0x004000, 0xe0d7679f, 7 | BRF_PRG | BRF_ESS },  // 10 Internal ARM7 Rom

	{ "v200-16.rom",   0x200000, 0x16a0c11f, 8 | BRF_PRG | BRF_ESS }, // 11 External ARM7 Rom
};

STDROMPICKEXT(kov2p, kov2p, pgm)
STD_ROM_FN(kov2p)

static void kov2pCallback()
{
	pgm_decrypt_kov2p();

	*((UINT16*)(PGMUSER0 + 0x0000de)) = BURN_ENDIAN_SWAP_INT16(0x46c0);
	*((UINT16*)(PGMUSER0 + 0x1ffffc)) = BURN_ENDIAN_SWAP_INT16(0x9933);

	for (INT32 i = 0x4ed8; i <= 0x4f0c; i+=4) {
		PGMUSER0[i] -= 0x08;
	}
}

static INT32 kov2pInit()
{
	pPgmInitCallback = kov2pCallback;
	pPgmProtCallback = install_protection_asic27a_martmast;
	
	INT32 nRet = pgmInit();
	
	Arm7SetIdleLoopAddress(0x80000a6);

	return nRet;
}

struct BurnDriver BurnDrvKov2p = {
	"kov2p", NULL, "pgm", NULL, "2001",
	"Knights of Valour 2 Plus - Nine Dragons (VM204XX)\0", "Incorrect Asic dump", "IGS", "PolyGameMaster",
	L"Knights of Valour 2 Plus\0\u4e09\u56fd\u6218\u7eaa 2 - \u7fa4\u96c4\u4e89\u9738 (VM204XX)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kov2pRomInfo, kov2pRomName, NULL, NULL, pgmInputInfo, kov2DIPInfo,
	kov2pInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour 2 Plus - Nine Dragons (VM205XX, China)

static struct BurnRomInfo kov2p205RomDesc[] = {
	{ "u8-27322.rom",  0x400000, 0x3a2cc0de, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1200.rom",	   0x800000, 0xd7e26609, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1200.rom",	   0x800000, 0xceeb81d8, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1201.rom_p",   0x800000, 0x21063ca7, 3 | BRF_GRA },		  //  3
	{ "a1202.rom",	   0x800000, 0x4bb92fae, 3 | BRF_GRA },		  //  4
	{ "a1203.rom",	   0x800000, 0xe73cb627, 3 | BRF_GRA },		  //  5
	{ "a1204.rom_p",   0x200000, 0x14b4b5bb, 3 | BRF_GRA },		  //  6

	{ "b1200.rom",	   0x800000, 0xbed7d994, 4 | BRF_GRA },		  //  7 Sprite Masks & Color Indexes
	{ "b1201.rom",	   0x800000, 0xf251eb57, 4 | BRF_GRA },		  //  8

	{ "m1200.rom",	   0x800000, 0xb0d88720, 5 | BRF_SND },		  //  9 Samples

	{ "kov2p_igs027a.bin",	   0x004000, 0xe0d7679f, 7 | BRF_PRG | BRF_ESS },  // 10 Internal ARM7 Rom

	{ "v200-16.rom",   0x200000, 0x16a0c11f, 8 | BRF_PRG | BRF_ESS }, // 11 External ARM7 Rom
};

STDROMPICKEXT(kov2p205, kov2p205, pgm)
STD_ROM_FN(kov2p205)

struct BurnDriver BurnDrvKov2p205 = {
	"kov2p205", "kov2p", "pgm", NULL, "2001",
	"Knights of Valour 2 Plus - Nine Dragons (VM205XX, China)\0", "Incorrect Asic dump", "IGS", "PolyGameMaster",
	L"Knights of Valour 2 Plus\0\u4e09\u56fd\u6218\u7eaa 2 - \u7fa4\u96c4\u4e89\u9738 (VM205XX, China)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kov2p205RomInfo, kov2p205RomName, NULL, NULL, pgmInputInfo, kov2DIPInfo,
	kov2pInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Bee Storm - DoDonPachi II (V102)

static struct BurnRomInfo ddp2RomDesc[] = {
	{ "v102.u8",       0x200000, 0x5a9ea040, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1300.u21",	   0x800000, 0xe748f0cb, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1300.u1",	   0x800000, 0xfc87a405, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1301.u2",	   0x800000, 0x0c8520da, 3 | BRF_GRA },		  //  3

	{ "b1300.u7",	   0x800000, 0xef646604, 4 | BRF_GRA },		  //  4 Sprite Masks & Color Indexes

	{ "m1300.u5",	   0x400000, 0x82d4015d, 5 | BRF_SND },		  //  5 Samples

	{ "ddp2_igs027a.bin", 0x004000, 0,	 7 | BRF_PRG | BRF_NODUMP },//  6 Internal ARM7 Rom

	{ "v100.u23", 	   0x020000, 0x06c3dd29, 8 | BRF_PRG | BRF_ESS }, //  7 External ARM7 Rom
};

STDROMPICKEXT(ddp2, ddp2, pgm)
STD_ROM_FN(ddp2)

static INT32 ddp2Init()
{
	pPgmInitCallback = pgm_decrypt_ddp2;
	pPgmProtCallback = install_protection_asic27a_ddp2; // simulation
//	pPgmProtCallback = install_protection_asic27a_martmast;

	return pgmInit();
}

struct BurnDriverD BurnDrvDdp2 = {
	"ddp2", NULL, "pgm", NULL, "2001",
	"Bee Storm - DoDonPachi II (V102)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"Bee Storm - DoDonPatchi II\0\u6012\u9996\u9886\u8702\uff0d\u8702\u66b4 (V102)\0", NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU */, GBF_VERSHOOT, 0,
	NULL, ddp2RomInfo, ddp2RomName, NULL, NULL, pgmInputInfo, ddp2DIPInfo,
	ddp2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	224,448,3,4
};

// Bee Storm - DoDonPachi II (V100)

static struct BurnRomInfo ddp2100RomDesc[] = {
	{ "v100.u8",	   0x200000, 0x0c8aa8ea, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1300.u21",	   0x800000, 0xe748f0cb, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1300.u1",	   0x800000, 0xfc87a405, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1301.u2",	   0x800000, 0x0c8520da, 3 | BRF_GRA },		  //  3

	{ "b1300.u7",	   0x800000, 0xef646604, 4 | BRF_GRA },		  //  4 Sprite Masks & Color Indexes

	{ "m1300.u5",	   0x400000, 0x82d4015d, 5 | BRF_SND },		  //  5 Samples

	{ "ddp2_igs027a.bin", 0x004000, 0,	 7 | BRF_PRG | BRF_NODUMP },//  6 Internal ARM7 Rom

	{ "v100.u23", 	   0x020000, 0x06c3dd29, 8 | BRF_PRG | BRF_ESS }, //  7 External ARM7 Rom
};

STDROMPICKEXT(ddp2100, ddp2100, pgm)
STD_ROM_FN(ddp2100)

struct BurnDriverD BurnDrvDdp2100 = {
	"ddp2100", "ddp2", "pgm", NULL, "2001",
	"Bee Storm - DoDonPachi II (V100)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"Bee Storm - DoDonPatchi II\0\u6012\u9996\u9886\u8702\uff0d\u8702\u66b4 (V100)\0", NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL | BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_VERSHOOT, 0,
	NULL, ddp2100RomInfo, ddp2100RomName, NULL, NULL, pgmInputInfo, ddp2DIPInfo,
	ddp2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	224,448,3,4
};

// Bee Storm - DoDonPachi II (V101)

static struct BurnRomInfo ddp2101RomDesc[] = {
	{ "v101_16m.u8",   0x200000, 0x5e5786fd, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t1300.u21",	   0x800000, 0xe748f0cb, 2 | BRF_GRA },		  //  1 Tile data

	{ "a1300.u1",	   0x800000, 0xfc87a405, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a1301.u2",	   0x800000, 0x0c8520da, 3 | BRF_GRA },		  //  3

	{ "b1300.u7",	   0x800000, 0xef646604, 4 | BRF_GRA },		  //  4 Sprite Masks & Color Indexes

	{ "m1300.u5",	   0x400000, 0x82d4015d, 5 | BRF_SND },		  //  5 Samples

	{ "ddp2_igs027a.bin", 0x004000, 0,	 7 | BRF_PRG | BRF_NODUMP },//  6 Internal ARM7 Rom

	{ "v100.u23", 	   0x020000, 0x06c3dd29, 8 | BRF_PRG | BRF_ESS }, //  7 External ARM7 Rom
};

STDROMPICKEXT(ddp2101, ddp2101, pgm)
STD_ROM_FN(ddp2101)

struct BurnDriverD BurnDrvDdp2101 = {
	"ddp2101", "ddp2", "pgm", NULL, "2001",
	"Bee Storm - DoDonPachi II (V101)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"Bee Storm - DoDonPatchi II\0\u6012\u9996\u9886\u8702\uff0d\u8702\u66b4 (V101)\0", NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL | BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU */, GBF_VERSHOOT, 0,
	NULL, ddp2101RomInfo, ddp2101RomName, NULL, NULL, pgmInputInfo, ddp2DIPInfo,
	ddp2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	224,448,3,4
};


// Dragon World 2001 (V100, Taiwan)

static struct BurnRomInfo dw2001RomDesc[] = {
	{ "2001.u22",	   0x080000, 0x5cabed92, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "2001.u11",	   0x200000, 0x1dd8d1e9, 2 | BRF_GRA },		  //  1 Tile data

	{ "2001.u2",	   0x200000, 0xd11c733c, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "2001.u3",	   0x200000, 0x1435aef2, 3 | BRF_GRA },		  //  4

	{ "2001.u9",	   0x200000, 0xccbca572, 4 | BRF_GRA },		  //  5 Sprite Masks & Color Indexes

	{ "2001.u7",	   0x200000, 0x4ea62f21, 5 | BRF_SND },		  //  6 Samples

	{ "dw2001_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_ESS | BRF_NODUMP }, //  7 Internal ARM7 rom

	{ "2001.u12",	   0x080000, 0x973db1ab, 8 | BRF_PRG | BRF_ESS }, // 8 External ARM7 rom
};

STDROMPICKEXT(dw2001, dw2001, pgm)
STD_ROM_FN(dw2001)

static INT32 dw2001Init()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_martmast; // the same
//	pPgmProtCallback = install_protection_asic27a_martmast;

	return pgmInit();
}

struct BurnDriverD BurnDrvDw2001 = {
	"dw2001", NULL, "pgm", NULL, "2001",
	"Dragon World 2001 (V100, Taiwan)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_PUZZLE, 0,
	NULL, dw2001RomInfo, dw2001RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	dw2001Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Demon Front (V102)

static struct BurnRomInfo dmnfrntRomDesc[] = {
	{ "v102_16m.u5",   0x200000, 0x3d4d481a, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t04501.u29",	   0x800000, 0x900eaaac, 2 | BRF_GRA },		  //  1 Tile data

	{ "a04501.u3",     0x800000, 0x9741bea6, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a04502.u4",     0x800000, 0xe104f405, 3 | BRF_GRA },		  //  3
	{ "a04503.u6",     0x800000, 0xbfd5cfe3, 3 | BRF_GRA },		  //  4

	{ "b04501.u9",	   0x800000, 0x29320b7d, 4 | BRF_GRA },		  //  5 Sprite Masks & Color Indexes
	{ "b04502.u11",	   0x200000, 0x578c00e9, 4 | BRF_GRA },		  //  6

	{ "w04501.u5",     0x800000, 0x3ab58137, 5 | BRF_SND },		  //  7 Samples

	{ "dmnfrnt_igs027a.bin", 0x004000, 0,    7 | BRF_PRG | BRF_NODUMP }, //  8 Internal ARM7 Rom

	{ "v101_32m.u26",  0x400000, 0x93965281, 8 | BRF_PRG | BRF_ESS }, //  9 External ARM7 Rom
};

STDROMPICKEXT(dmnfrnt, dmnfrnt, pgm)
STD_ROM_FN(dmnfrnt)

static INT32 dmnfrntInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_dfront;
//	pPgmProtCallback = install_protection_asic27a_svg;

	return pgmInit();
}

struct BurnDriverD BurnDrvDmnfrnt = {
	"dmnfrnt", NULL, "pgm", NULL, "2002",
	"Demon Front (V102)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"Demon Front\0\u9B54\u57DF\u6218\u7EBF\0\u9B54\u57DF\u6230\u7DDA (V102)\0", NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_PLATFORM, 0,
	NULL, dmnfrntRomInfo, dmnfrntRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	dmnfrntInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Demon Front (V105, China)

static struct BurnRomInfo dmnfrnt105RomDesc[] = {
	{ "v105_16m.u5",   0x200000, 0xbda083bd, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t04501.u29",	   0x800000, 0x900eaaac, 2 | BRF_GRA },		  //  1 Tile data

	{ "a04501.u3",     0x800000, 0x9741bea6, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a04502.u4",     0x800000, 0xe104f405, 3 | BRF_GRA },		  //  3
	{ "a04503.u6",     0x800000, 0xbfd5cfe3, 3 | BRF_GRA },		  //  4

	{ "b04501.u9",	   0x800000, 0x29320b7d, 4 | BRF_GRA },		  //  5 Sprite Masks & Color Indexes
	{ "b04502.u11",	   0x200000, 0x578c00e9, 4 | BRF_GRA },		  //  6

	{ "w04501.u5",     0x800000, 0x3ab58137, 5 | BRF_SND },		  //  7 Samples

	{ "dmnfrnt_igs027a.bin", 0x004000, 0,    7 | BRF_PRG | BRF_NODUMP }, //  8 Internal ARM7 Rom

	{ "v105_32m.u26",  0x400000, 0xc798c2ef, 8 | BRF_PRG | BRF_ESS }, //  9 External ARM7 Rom
};

STDROMPICKEXT(dmnfrnt105, dmnfrnt105, pgm)
STD_ROM_FN(dmnfrnt105)

struct BurnDriverD BurnDrvDmnfrnt105 = {
	"dmnfrnt105", "dmnfrnt", "pgm", NULL, "2002",
	"Demon Front (V105, China)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"Demon Front\0\u9B54\u57DF\u6218\u7EBF\0\u9B54\u57DF\u6230\u7DDA (V105, China)\0", NULL, NULL, NULL,
	BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_PLATFORM, 0,
	NULL, dmnfrnt105RomInfo, dmnfrnt105RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	dmnfrntInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// The Gladiator / Shen Jian (V100)

static struct BurnRomInfo thegladRomDesc[] = {
	{ "u6.rom",			0x080000, 0x14c85212, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t04601.u33",			0x800000, 0xe5dab371, 2 | BRF_GRA },		//  1 Tile data

	{ "a04601.u2",			0x800000, 0xd9b2e004, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a04602.u4",			0x800000, 0x14f22308, 3 | BRF_GRA },		//  3
	{ "a04603.u6",			0x800000, 0x8f621e17, 3 | BRF_GRA },		//  4

	{ "b04601.u11",			0x800000, 0xee72bccf, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes
	{ "b04602.u12",			0x400000, 0x7dba9c38, 4 | BRF_GRA },		//  6

	{ "w04601.u1",			0x800000, 0x5f15ddb3, 5 | BRF_SND },		//  7 Samples

	{ "theglad_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP },  //  8 Internal ARM7 Rom

	{ "u2.rom",			0x200000, 0xc7bcf2ae, 8 | BRF_PRG | BRF_ESS },	//  9 External ARM7 Rom
};

STDROMPICKEXT(theglad, theglad, pgm)
STD_ROM_FN(theglad)

static INT32 thegladInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_theglad;
//	pPgmProtCallback = install_protection_asic27a_svg;

	return pgmInit();
}

struct BurnDriverD BurnDrvTheglad = {
	"theglad", NULL, "pgm", NULL, "2003",
	"The Gladiator / Shen Jian (V100)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"The Gladiator (V100)\0\u795E\u5251\u98CE\u4E91\0\u795E\u528D\u98A8\u96F2\0", NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, thegladRomInfo, thegladRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	thegladInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// The Gladiator / Shen Jian (V101, Taiwan)

static struct BurnRomInfo theglad101RomDesc[] = {
	{ "v101.u6",			0x080000, 0xf799e866, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t04601.u33",			0x800000, 0xe5dab371, 2 | BRF_GRA },		//  1 Tile data

	{ "a04601.u2",			0x800000, 0xd9b2e004, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a04602.u4",			0x800000, 0x14f22308, 3 | BRF_GRA },		//  3
	{ "a04603.u6",			0x800000, 0x8f621e17, 3 | BRF_GRA },		//  4

	{ "b04601.u11",			0x800000, 0xee72bccf, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes
	{ "b04602.u12",			0x400000, 0x7dba9c38, 4 | BRF_GRA },		//  6

	{ "w04601.u1",			0x800000, 0x5f15ddb3, 5 | BRF_SND },		//  7 Samples

	{ "theglad_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, //  8 Internal ARM7 Rom

	{ "v107.u26",			0x200000, 0xf7c61357, 8 | BRF_PRG | BRF_ESS },	//  9 External ARM7 Rom
};

STDROMPICKEXT(theglad101, theglad101, pgm)
STD_ROM_FN(theglad101)

struct BurnDriverD BurnDrvTheglad101 = {
	"theglada", "theglad", "pgm", NULL, "2003",
	"The Gladiator / Shen Jian (V101, Taiwan)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"The Gladiator (V101, Taiwan)\0\u795E\u5251\u98CE\u4E91\0\u795E\u528D\u98A8\u96F2\0", NULL, NULL, NULL,
	BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, theglad101RomInfo, theglad101RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	thegladInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Happy 6in1 (V100 - V100MK)

static struct BurnRomInfo happy6RomDesc[] = {
	{ "happy6in1_v100cn.u5",	0x080000, 0xa25418e8, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t01w64m.u29",   		0x800000, 0x2d3feb8b, 2 | BRF_GRA },		//  1 Tile data

	{ "a01w64m.u5",	  		0x800000, 0xbbaa3df3, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a02w64m.u6",	  		0x800000, 0xf8c9cd36, 3 | BRF_GRA },		//  3

	{ "b01w64m.u19", 		0x800000, 0x73f5f225, 4 | BRF_GRA },		//  4 Sprite Masks & Color Indexes

	{ "w01w64m.u17",	  	0x800000, 0x7e23e2be, 5 | BRF_SND },		//  5 Samples

	{ "happy6_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, //  6 Internal ARM7 Rom

	{ "happy6in1_v101cn.u26",	0x400000, 0x4a48ca1c, 8 | BRF_PRG | BRF_ESS },	// 7 External ARM7 Rom
};

STDROMPICKEXT(happy6, happy6, pgm)
STD_ROM_FN(happy6)

static INT32 happy6in1Init()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_happy6in1;
//	pPgmProtCallback = install_protection_asic27a_svg;

	return pgmInit();
}

struct BurnDriverD BurnDrvhappy6 = {
	"happy6", NULL, "pgm", NULL, "2004",
	"Happy 6in1 (V100 - V100MK)\0", "Missing Internal ARM7 Rom", "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_MISC, 0,
	NULL, happy6RomInfo, happy6RomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	happy6in1Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	448, 224, 4, 3
};


// Knights of Valour Super Heroes Plus / Sangoku Senki Super Heroes Plus (ver. 100)

static struct BurnRomInfo kovshpRomDesc[] = {
	{ "p0600h.rom",	   0x400000, 0xe251e8e4, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0600.rom",	   0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",	   0x800000, 0xff7a4373, 3 | BRF_GRA }, 	  //  3 
	{ "a0602.rom",	   0x800000, 0xe7a32959, 3 | BRF_GRA }, 	  //  4
	{ "a0540.rom",	   0x800000, 0x4fd3413e, 3 | BRF_GRA }, 	  //  5

	{ "b0600.rom",	   0x800000, 0x7d3cd059, 4 | BRF_GRA },		  //  6 Sprite Masks & Color Indexes
	{ "b0540.rom",	   0x800000, 0x60999757, 4 | BRF_GRA },		  //  7

	{ "m0600.rom",	   0x400000, 0x3ada4fd6, 5 | BRF_SND },		  //  8 Samples

	{ "kovshp_igs027a.bin", 0x004000, 0x00000000, 0 | BRF_ESS | BRF_PRG | BRF_NODUMP }, //  9 Internal ARM7 Rom
};

STDROMPICKEXT(kovshp, kovshp, pgm)
STD_ROM_FN(kovshp)

static INT32 kovshpInit()
{
	pPgmInitCallback = pgm_decrypt_kovshp;
	pPgmProtCallback = install_protection_asic27_kov; // simulation
//	pPgmProtCallback = install_protection_asic27a_kovsh;

	return pgmInit();
}

struct BurnDriverD BurnDrvkovshp = {
	"kovshp", NULL, "pgm", NULL, "2004",
	"Knights of Valour Super Heroes Plus / Sangoku Senki Super Heroes Plus (ver. 100)\0", "Incomplete dump", "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM /* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovshpRomInfo, kovshpRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovshpInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Oriental Legend Special Plus / Xi You Shi E Zhuan Super Plus (ver. 205)

static struct BurnRomInfo oldsplusRomDesc[] = {
	{ "p05301.rom",			0x400000, 0x923f7246, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t05301.rom",			0x800000, 0x8257bbb0, 2 | BRF_GRA },		//  1 Tile data

	{ "a05301.rom",			0x800000, 0x57946fd2, 3 | BRF_GRA },	 	//  2 Sprite Color Data
	{ "a05302.rom",			0x800000, 0x3459a0b8, 3 | BRF_GRA },		//  3
	{ "a05303.rom",			0x800000, 0x13475d85, 3 | BRF_GRA },		//  4
	{ "a05304.rom",			0x800000, 0xf03ef7a6, 3 | BRF_GRA },		//  5

	{ "b05301.rom",			0x800000, 0xfd98f503, 4 | BRF_GRA },		//  6 Sprite Masks & Color Indexes
	{ "b05302.rom",			0x800000, 0x9f6094a8, 4 | BRF_GRA },		//  7

	{ "m05301.rom",			0x400000, 0x86ec83bc, 5 | BRF_SND },		//  8 Samples

	{ "oldsplus_igs027a.bin",	0x004000, 0,          7 | BRF_PRG | BRF_NODUMP }, //  9 Internal ARM7 Rom
};

STDROMPICKEXT(oldsplus, oldsplus, pgm)
STD_ROM_FN(oldsplus)

static INT32 oldsplusInit()
{
	pPgmInitCallback = pgm_decrypt_oldsplus;
	pPgmProtCallback = install_protection_asic27a_oldsplus;
//	pPgmProtCallback = install_protection_asic27a_kovsh;

	return pgmInit();
}

struct BurnDriver BurnDrvoldsplus = {
	"oldsplus", NULL, "pgm", NULL, "2004",
	"Oriental Legend Special Plus / Xi You Shi E Zhuan Super Plus (ver. 205)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"Oriental Legend Special Plus\0\u897F\u6E38\u91CA\u5384\u4F20 Plus\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, oldsplusRomInfo, oldsplusRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	oldsplusInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// The Killing Blade Plus (V300)

static struct BurnRomInfo killbldpRomDesc[] = {
	{ "v300x.u6",       		0x080000, 0xb7fb8ec9, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code
	
	{ "t05701w032.bin", 		0x400000, 0x567c714f, 2 | BRF_GRA },		//  1 Tile data

	{ "a05701w064.bin", 		0x800000, 0x8c0c992c, 3 | BRF_GRA },		//  3 Sprite Color Data
	{ "a05702w064.bin", 		0x800000, 0x7e5b0f27, 3 | BRF_GRA },		//  4
	{ "a05703w064.bin", 		0x800000, 0xaccbdb44, 3 | BRF_GRA },		//  5
	
	{ "b05701w064.bin", 		0x800000, 0xa20cdcef, 4 | BRF_GRA },		//  6 Sprite Masks & Color Indexes
	{ "b05702w016.bin", 		0x200000, 0xfe7457df, 4 | BRF_GRA },		//  7
	
	{ "w05701b032.bin", 		0x400000, 0x2d3ae593, 5 | BRF_SND },		//  8 Samples

	{ "killbldp_igs027a.bin",	0x004000, 0x9a73bf7d, 7 | BRF_PRG }, //  9 Internal ARM7 Rom
	
	{ "v300x.u26",			0x200000, 0x144388c8, 8 | BRF_PRG | BRF_ESS },	// 10 External ARM7 Rom
};

STDROMPICKEXT(killbldp, killbldp, pgm)
STD_ROM_FN(killbldp)

INT32 killbldpInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_killbldp;
	pPgmProtCallback = install_protection_asic27a_svg;
	
	INT32 nRet = pgmInit();
	
	Arm7SetIdleLoopAddress(0x00007c4);

	return nRet;
}

struct BurnDriverD BurnDrvKillbldp = {
	"killbldp", NULL, "pgm", NULL, "2005",
	"The Killing Blade Plus (V300)\0", NULL, "IGS", "PolyGameMaster",
	L"The Killing Blade Plus \0\u50B2\u5251\u72C2\u5200\0\u50B2\u528D\u72C2\u5200 (V300)\0", NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_VSFIGHT, 0,
	NULL, killbldpRomInfo, killbldpRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	killbldpInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// S.V.G. - Spectral vs Generation (V200, China)

static struct BurnRomInfo svgRomDesc[] = {
	{ "u30.bin",			0x080000, 0x34c18f3f, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t05601w016.bin",		0x200000, 0x03e110dc, 2 | BRF_GRA },		//  1 Tile data

	{ "a05601w064.bin",		0x800000, 0xea6453e4, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a05602w064.bin",		0x800000, 0x6d00621b, 3 | BRF_GRA },		//  3
	{ "a05603w064.bin",		0x800000, 0x7b71c64f, 3 | BRF_GRA },		//  4
	{ "a05604w032.bin",		0x400000, 0x9452a567, 3 | BRF_GRA },		//  5

	{ "b05601w064.bin",		0x800000, 0x35c0a489, 4 | BRF_GRA },		//  6 Sprite Masks & Color Indexes
	{ "b05602w064.bin",		0x800000, 0x8aad3f85, 4 | BRF_GRA },		//  7

	{ "w05601b064.bin",		0x800000, 0xbfe61a71, 5 | BRF_SND },		//  8 Samples
	{ "w05602b032.bin",		0x400000, 0x0685166d, 5 | BRF_SND },		//  9

	{ "svg_igs027a.bin",		0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 10 Internal ARM7 Rom

	{ "u26.bin",			0x400000, 0x46826ec8, 8 | BRF_PRG | BRF_ESS },	// 11 External ARM7 Rom
	{ "u29.bin",			0x400000, 0xfa5f3901, 8 | BRF_PRG | BRF_ESS },	// 12
};

STDROMPICKEXT(svg, svg, pgm)
STD_ROM_FN(svg)

static INT32 svgInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_svg;
//	pPgmProtCallback = install_protection_asic27a_svg;

	return pgmInit();
}

struct BurnDriverD BurnDrvSvg = {
	"svg", NULL, "pgm", NULL, "2005",
	"S.V.G. - Spectral vs Generation (V200, China)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, svgRomInfo, svgRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	svgInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// -----------------------------------------------------------------------------
// PCB Versions!


// DoDonPachi Dai-Ou-Jou (V101, Japan)

static struct BurnRomInfo ddp3RomDesc[] = {
	{ "ddp3_v101.u36",		0x200000, 0x195b5c1e, 1 | BRF_PRG | BRF_ESS },		//  0 68K Code

	{ "t04401w064.u19",		0x800000, 0x3a95f19c, 2 | BRF_GRA },	//  1 Tile data

	{ "a04401w064.u7",		0x800000, 0xed229794, 3 | BRF_GRA },	//  2 Sprite Color Data
	{ "a04402w064.u8",		0x800000, 0x752167b0, 3 | BRF_GRA },	//  3

	{ "b04401w064.u1",		0x800000, 0x8cbff066, 4 | BRF_GRA },			//  4 Sprite Masks & Color Indexes

	{ "m04401b032.u17",		0x400000, 0x5a0dbd76, 5 | BRF_SND },	//  5 Samples

	{ "ddp3_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP },	//  6 Internal ARM7 Rom
	
	{ "ddp3_defaults.nv",	0x020000, 0x571e96c0, 0 | BRF_OPT },			//  7 NV RAM
};

STDROMPICKEXT(ddp3, ddp3, ddp3Bios) /* custom bios */
STD_ROM_FN(ddp3)

static INT32 ddp3Init()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_py2k2; // the same.
	pPgmProtCallback = install_protection_asic27a_ddp3; // simulation

	return pgmInit();
}

struct BurnDriver BurnDrvDdp3 = {
	"ddpdoj", NULL, NULL,  NULL, "2002",
	"DoDonPachi Dai-Ou-Jou (V101, Japan)\0", "Incomplete Protection Simulation", "Cave / AMI", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 4, HARDWARE_IGS_PGM /* | HARDWARE_IGS_USE_ARM_CPU */, GBF_SCRFIGHT, 0,
	NULL, ddp3RomInfo, ddp3RomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	ddp3Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	224, 448, 3, 4
};


// DoDonPachi Dai-Ou-Jou (V100 (second version), Japan)

static struct BurnRomInfo ddp3aRomDesc[] = {
	{ "ddp3_d_d_1_0.u36",		0x200000, 0x5D3F85BA, 1 | BRF_PRG | BRF_ESS },		//  0 68K Code

	{ "t04401w064.u19",		0x800000, 0x3a95f19c, 2 | BRF_GRA },	//  1 Tile data

	{ "a04401w064.u7",		0x800000, 0xed229794, 3 | BRF_GRA },	//  2 Sprite Color Data
	{ "a04402w064.u8",		0x800000, 0x752167b0, 3 | BRF_GRA },	//  3

	{ "b04401w064.u1",		0x800000, 0x8cbff066, 4 | BRF_GRA },			//  4 Sprite Masks & Color Indexes

	{ "m04401b032.u17",		0x400000, 0x5a0dbd76, 5 | BRF_SND },	//  5 Samples

	{ "ddp3_igs027a.bin",		0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP },	//  6 Internal ARM7 Rom
	
	{ "ddp3_defaults.nv",	0x020000, 0x571e96c0, 0 | BRF_OPT },			//  7 NV RAM
};

STDROMPICKEXT(ddp3a, ddp3a, ddp3Bios) /* custom bios */
STD_ROM_FN(ddp3a)

struct BurnDriver BurnDrvDdp3a = {
	"ddpdoja", "ddpdoj", NULL,  NULL, "2002",
	"DoDonPachi Dai-Ou-Jou (V100 (second version), Japan)\0", "Incomplete Protection Simulation", "Cave / AMI", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 4, HARDWARE_IGS_PGM /* | HARDWARE_IGS_USE_ARM_CPU */, GBF_SCRFIGHT, 0,
	NULL, ddp3aRomInfo, ddp3aRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	ddp3Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	224, 448, 3, 4
};


// DoDonPachi Dai-Ou-Jou (V100 (first version), Japan)

static struct BurnRomInfo ddp3bRomDesc[] = {
	{ "dd v100.bin",		0x200000, 0x7da0c1e4, 1 | BRF_PRG | BRF_ESS },		//  0 68K Code

	{ "t04401w064.u19",		0x800000, 0x3a95f19c, 2 | BRF_GRA },	//  1 Tile data

	{ "a04401w064.u7",		0x800000, 0xed229794, 3 | BRF_GRA },	//  2 Sprite Color Data
	{ "a04402w064.u8",		0x800000, 0x752167b0, 3 | BRF_GRA },	//  3

	{ "b04401w064.u1",		0x800000, 0x8cbff066, 4 | BRF_GRA },			//  4 Sprite Masks & Color Indexes

	{ "m04401b032.u17",		0x400000, 0x5a0dbd76, 5 | BRF_SND },	//  5 Samples

	{ "ddp3_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP },	//  6 Internal ARM7 Rom
	
	{ "ddp3_defaults.nv",	0x020000, 0x571e96c0, 0 | BRF_OPT },			//  7 NV RAM
};

STDROMPICKEXT(ddp3b, ddp3b, ddp3Bios) /* custom bios */
STD_ROM_FN(ddp3b)

struct BurnDriver BurnDrvDdp3b = {
	"ddpdojb", "ddpdoj", NULL,  NULL, "2002",
	"DoDonPachi Dai-Ou-Jou (V100 (first version), Japan)\0", "Incomplete Protection Simulation", "Cave / AMI", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 4, HARDWARE_IGS_PGM /* | HARDWARE_IGS_USE_ARM_CPU */, GBF_SCRFIGHT, 0,
	NULL, ddp3bRomInfo, ddp3bRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	ddp3Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x1200,
	224, 448, 3, 4
};


// DoDonPachi Dai-Ou-Jou Black Label (V100, Japan)

static struct BurnRomInfo ddp3blkRomDesc[] = {
	{ "ddb10.u45",			0x200000, 0x72b35510, 1 | BRF_PRG | BRF_ESS },		//  0 68K Code

	{ "t04401w064.u19",		0x800000, 0x3a95f19c, 2 | BRF_GRA },			//  1 Tile data

	{ "a04401w064.u7",		0x800000, 0xed229794, 3 | BRF_GRA },			//  2 Sprite Color Data
	{ "a04402w064.u8",		0x800000, 0x752167b0, 3 | BRF_GRA },	//  3

	{ "b04401w064.u1",		0x800000, 0x8cbff066, 4 | BRF_GRA },			//  4 Sprite Masks & Color Indexes

	{ "m04401b032.u17",		0x400000, 0x5a0dbd76, 5 | BRF_SND },	//  5 Samples

	{ "ddp3_igs027a.bin",		0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP },	//  6 Internal ARM7 Rom

	{ "ddp3blk_defaults.nv",	0x020000, 0xa1651904, 0 | BRF_OPT },			//  7 NV RAM (patch below instead)
};

STDROMPICKEXT(ddp3blk, ddp3blk, ddp3Bios) /* custom bios */
STD_ROM_FN(ddp3blk)

/*
	Black Label has some values set in the NV ram. If the battery dies,
	the pcb must be sent back to Cave for repair.
*/

static void ddp3blkPatchRAM()
{
	SekOpen(0);
	SekWriteLong(0x803800, 0x95804803); // cmpi.l  #$95804803,($803800).l
	SekWriteLong(0x803804, 0x23879065); // cmpi.l  #$23879065,($803804).l
	SekClose();

	// enable asic test
//	*((UINT16*)(PGM68KROM + 0x03c0f4)) = BURN_ENDIAN_SWAP_INT16(0x0012);
}

static INT32 ddp3blkInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_py2k2; // the same
	pPgmProtCallback = install_protection_asic27a_ddp3; // simulation

	INT32 nRet = pgmInit();

	if (nRet == 0) {
		ddp3blkPatchRAM();
	}

	return nRet;
}

struct BurnDriver BurnDrvDdp3blk = {
	"ddpdojblk", "ddpdoj", NULL, NULL, "2002",
	"DoDonPachi Dai-Ou-Jou Black Label (V100, Japan)\0", "Incomplete Protection Simulation", "Cave / AMI", "PolyGameMaster",
	L"DoDonPachi Dai-Ou-Jou Black Label\0\u6012\u9996\u9818\u8702 \u5927\u5F80\u751F Black Label (V100, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 4, HARDWARE_IGS_PGM /* | HARDWARE_IGS_USE_ARM_CPU */, GBF_SCRFIGHT, 0,
	NULL, ddp3blkRomInfo, ddp3blkRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	ddp3blkInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	224, 448, 3, 4
};


// Ketsui Kizuna Jigoku Tachi (Revision 2?, Japan)

static struct BurnRomInfo ketRomDesc[] = {
	{ "ketsui_v100.u38",	0x200000, 0xdfe62f3b, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t04701w064.u19", 	0x800000, 0x2665b041, 2 | BRF_GRA },		//  1 Tile data

	{ "a04701w064.u7", 		0x800000, 0x5ef1b94b, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a04702w064.u8", 		0x800000, 0x26d6da7f, 3 | BRF_GRA },		//  3

	{ "b04701w064.u1",		0x800000, 0x1bec008d, 4 | BRF_GRA },		//  4 Sprite Masks & Color Indexes

	{ "m04701b032.u17",		0x400000, 0xb46e22d1, 5 | BRF_SND },		//  5 Samples

	{ "ket_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 6 Internal ARM7 Rom
	
	{ "ket_defaults.nv",	0x020000, 0x3ca892d8, 0 | BRF_OPT },			//  7 NV RAM
};

STDROMPICKEXT(ket, ket, ketsuiBios) /* custom bios */
STD_ROM_FN(ket)

static INT32 ketsuiInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_ketsui;
	pPgmProtCallback = install_protection_asic27a_ketsui;  // simulation

	return pgmInit();
}

struct BurnDriver BurnDrvKet = {
	"ket", NULL, NULL, NULL, "2002",
	"Ketsui Kizuna Jigoku Tachi (Revision 2?, Japan)\0", NULL, "CAVE / AMI", "PolyGameMaster based",
	L"Ketsui Kizuna Jigoku Tachi\0\u30B1\u30C4\u30A4~\u7D46\u5730\u7344\u305F\u3061 (Revision 2?, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_JAMMAPCB/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_VERSHOOT, 0,
	NULL, ketRomInfo, ketRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	ketsuiInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	224, 448, 3, 4
};


// Ketsui Kizuna Jigoku Tachi (Revision 1?, Japan)

static struct BurnRomInfo ketaRomDesc[] = {
	{ "ketsui_prg_revised.bin",	0x200000, 0x69fcf5eb, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t04701w064.u19", 	0x800000, 0x2665b041, 2 | BRF_GRA },		//  1 Tile data

	{ "a04701w064.u7", 		0x800000, 0x5ef1b94b, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a04702w064.u8", 		0x800000, 0x26d6da7f, 3 | BRF_GRA },		//  3

	{ "b04701w064.u1",		0x800000, 0x1bec008d, 4 | BRF_GRA },		//  4 Sprite Masks & Color Indexes

	{ "m04701b032.u17",		0x400000, 0xb46e22d1, 5 | BRF_SND },		//  5 Samples

	{ "ket_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 6 Internal ARM7 Rom
	
	{ "ket_defaults.nv",	0x020000, 0x3ca892d8, 0 | BRF_OPT },			//  7 NV RAM
};

STDROMPICKEXT(keta, keta, ketsuiBios) /* custom bios */
STD_ROM_FN(keta)

struct BurnDriver BurnDrvKeta = {
	"keta", "ket", NULL, NULL, "2002",
	"Ketsui Kizuna Jigoku Tachi (Revision 1?, Japan)\0", NULL, "CAVE / AMI", "PolyGameMaster based",
	L"Ketsui Kizuna Jigoku Tachi\0\u30B1\u30C4\u30A4~\u7D46\u5730\u7344\u305F\u3061 (Revision 1?, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_JAMMAPCB/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_VERSHOOT, 0,
	NULL, ketaRomInfo, ketaRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	ketsuiInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	224, 448, 3, 4
};


// Ketsui Kizuna Jigoku Tachi (Original?, Japan)

static struct BurnRomInfo ketbRomDesc[] = {
	{ "ketsui_prg_original.bin",	0x200000, 0xcca5e153, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	{ "t04701w064.u19", 	0x800000, 0x2665b041, 2 | BRF_GRA },		//  1 Tile data

	{ "a04701w064.u7", 		0x800000, 0x5ef1b94b, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a04702w064.u8", 		0x800000, 0x26d6da7f, 3 | BRF_GRA },		//  3

	{ "b04701w064.u1",		0x800000, 0x1bec008d, 4 | BRF_GRA },		//  4 Sprite Masks & Color Indexes

	{ "m04701b032.u17",		0x400000, 0xb46e22d1, 5 | BRF_SND },		//  5 Samples

	{ "ket_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 6 Internal ARM7 Rom
	
	{ "ket_defaults.nv",	0x020000, 0x3ca892d8, 0 | BRF_OPT },			//  7 NV RAM
};

STDROMPICKEXT(ketb, ketb, ketsuiBios) /* custom bios */
STD_ROM_FN(ketb)

struct BurnDriver BurnDrvKetb = {
	"ketb", "ket", NULL, NULL, "2002",
	"Ketsui Kizuna Jigoku Tachi (Original?, Japan)\0", NULL, "CAVE / AMI", "PolyGameMaster based",
	L"Ketsui Kizuna Jigoku Tachi\0\u30B1\u30C4\u30A4~\u7D46\u5730\u7344\u305F\u3061 (Original?, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_JAMMAPCB/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_VERSHOOT, 0,
	NULL, ketbRomInfo, ketbRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	ketsuiInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	224, 448, 3, 4
};


// Espgaluda (V100, Japan)

static struct BurnRomInfo espgalRomDesc[] = {
	{ "espgaluda_v100.u38", 0x200000, 0x08ecec34, 1 | BRF_PRG | BRF_ESS },		//  0 68K Code

	{ "t04801w064.u19",	   	0x800000, 0x6021c79e, 2 | BRF_GRA },	//  1 Tile data

	{ "a04801w064.u7",	    0x800000, 0x26dd4932, 3 | BRF_GRA },	//  2 Sprite Color Data
	{ "a04802w064.u8",	   	0x800000, 0x0e6bf7a9, 3 | BRF_GRA },	//  3

	{ "b04801w064.u1",	 	0x800000, 0x98dce13a, 4 | BRF_GRA },	//  4 Sprite Masks & Color Indexes

	{ "w04801b032.u17",		0x400000, 0x60298536, 5 | BRF_SND },	//  5 Samples

	{ "espgal_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP },	//  6 Internal ARM7 Rom
};

STDROMPICKEXT(espgal, espgal, espgalBios) /* custom bios */
STD_ROM_FN(espgal)

static INT32 espgalInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmInitCallback = pgm_decrypt_espgaluda;
	pPgmProtCallback = install_protection_asic27a_ketsui; // simulation

	return pgmInit();
}

struct BurnDriver BurnDrvEspgal = {
	"espgal", NULL, NULL, NULL, "2003",
	"Espgaluda (V100, Japan)\0", NULL, "CAVE / AMI", "PolyGameMaster based",
	L"Espgaluda\0\u30A8\u30B9\u30D7\u30AC\u30EB\u30FC\u30C0 (V100, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_JAMMAPCB/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_VERSHOOT, 0,
	NULL, espgalRomInfo, espgalRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	espgalInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	224, 448, 3, 4
};
	

// The Gladiator / Shen Jian (V100, Japan, PCB Version)

static struct BurnRomInfo thegladpcbRomDesc[] = {
	{ "v100.u43",			0x080000, 0xBCF3B172, 1 | BRF_PRG | BRF_ESS },	//  0 68K Code

	// u71 on this board
	{ "t04601.u33",			0x800000, 0xe5dab371, 2 | BRF_GRA },		//  1 Tile data

	// u30, u31 & u32
	{ "a04601.u2",			0x800000, 0xd9b2e004, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "a04602.u4",			0x800000, 0x14f22308, 3 | BRF_GRA },		//  3
	{ "a04603.u6",			0x800000, 0x8f621e17, 3 | BRF_GRA },		//  4

	// u40 & u41
	{ "b04601.u11",			0x800000, 0xee72bccf, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes
	{ "b04602.u12",			0x400000, 0x7dba9c38, 4 | BRF_GRA },		//  6

	{ "w04601.u1",			0x800000, 0x5f15ddb3, 5 | BRF_SND },		//  7 Samples

	{ "thegladpcb_igs027a.bin",	0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP },  //  8 Internal ARM7 Rom

	{ "v100.u62",			0x200000, 0x0F3F511E, 8 | BRF_PRG | BRF_ESS },	//  9 External ARM7 Rom
};

STDROMPICKEXT(thegladpcb, thegladpcb, thegladBIOS) /* custom bios */
STD_ROM_FN(thegladpcb)

static INT32 thegladpcbInit()
{
	INT32 nRet = thegladInit();

	if (nRet == 0) {
		if (BurnLoadRom(ICSSNDROM + 0x200000, 0x83, 1)) return 1; // load extra 'wave' bios sample rom
	}

	return nRet;
}

struct BurnDriverD BurnDrvThegladpcb = {
	"thegladpcb", NULL, NULL, NULL, "2003",
	"The Gladiator / Shen Jian (V100, Japan, Single PCB Version)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"The Gladiator (V100, Japan, PCB Version)\0\u795E\u5251\u98CE\u4E91\0\u795E\u528D\u98A8\u96F2\0", NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM /* | HARDWARE_IGS_USE_ARM_CPU */, GBF_SCRFIGHT, 0,
	NULL, thegladpcbRomInfo, thegladpcbRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	thegladpcbInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Demon Front (V107, Korea, PCB Version)

static struct BurnRomInfo dmnfrntpcbRomDesc[] = {
	{ "v107-u43.bin",  0x200000, 0x671d8a31, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t04501.u29",	   0x800000, 0x900eaaac, 2 | BRF_GRA },		  //  1 Tile data

	{ "a04501.u3",     0x800000, 0x9741bea6, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a04502.u4",     0x800000, 0xe104f405, 3 | BRF_GRA },		  //  3
	{ "a04503.u6",     0x800000, 0xbfd5cfe3, 3 | BRF_GRA },		  //  4

	{ "b04501.u9",	   0x800000, 0x29320b7d, 4 | BRF_GRA },		  //  5 Sprite Masks & Color Indexes
	{ "b04502.u11",	   0x200000, 0x578c00e9, 4 | BRF_GRA },		  //  6

	{ "w04501.u5",     0x800000, 0x3ab58137, 5 | BRF_SND },		  //  7 Samples

	{ "dmnfrntpcb_igs027a.bin", 0x004000, 0,    7 | BRF_PRG | BRF_NODUMP }, //  8 Internal ARM7 Rom

	{ "v107-u62.bin",  0x400000, 0xcb94772e, 8 | BRF_PRG | BRF_ESS }, //  9 External ARM7 Rom
};

STDROMPICKEXT(dmnfrntpcb, dmnfrntpcb, dmnfrntBIOS)
STD_ROM_FN(dmnfrntpcb)

struct BurnDriverD BurnDrvDmnfrntpcb = {
	"dmnfrntpcb", NULL, NULL, NULL, "2002",
	"Demon Front (V107, Korea, Single PCB Version)\0", "Incomplete Dump", "IGS", "PolyGameMaster",
	L"Demon Front\0\u9B54\u57DF\u6218\u7EBF\0\u9B54\u57DF\u6230\u7DDA (V107, Korea, Single PCB Version)\0", NULL, NULL, NULL,
	0, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_PLATFORM, 0,
	NULL, dmnfrntpcbRomInfo, dmnfrntpcbRomName, NULL, NULL, pgmInputInfo, jammaDIPInfo,
	dmnfrntInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// -----------------------------------------------------------------------------
// Homebrew


// Chaos (PGM Demo)

static struct BurnRomInfo pgmdemoRomDesc[] = {
	{ "p0103.rom",     0x200000, 0xd3f6ec45, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0100.rom",     0x400000, 0x0596a59a, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0100.rom",     0x400000, 0x5d0e8fa1, 3 | BRF_GRA },		  //  2 Sprite Color Data

	{ "b0100.rom",     0x400000, 0x15dd191f, 4 | BRF_GRA },		  //  3 Sprite Masks & Color Indexes

	{ "m0100.rom",     0x200000, 0x8d89877e, 5 | BRF_SND },		  //  4 Samples
};

STDROMPICKEXT(pgmdemo, pgmdemo, pgm)
STD_ROM_FN(pgmdemo)

struct BurnDriver BurnDrvPgmdemo = {
	"pgmdemo", NULL, "pgm", NULL, "2005",
	"Chaos (PGM Demo)\0", "Demo Game", "Raster", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING  | BDF_DEMO, 4, HARDWARE_IGS_PGM, GBF_MISC, 0,
	NULL, pgmdemoRomInfo, pgmdemoRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,NULL, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Frog Feast (PGM)

static struct BurnRomInfo pgmfrogRomDesc[] = {
	{ "p0103.rom",     0x200000, 0xcdec9e8d, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0100.rom",     0x400000, 0x8f58b6d8, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0100.rom",     0x400000, 0xdc1eafe6, 3 | BRF_GRA },		  //  2 Sprite Color Data

	{ "b0100.rom",     0x400000, 0x3d44b66f, 4 | BRF_GRA },		  //  3 Sprite Masks & Color Indexes

	{ "m0100.rom",     0x200000, 0x05e2f761, 5 | BRF_SND },		  //  4 Samples
};

STDROMPICKEXT(pgmfrog, pgmfrog, pgm)
STD_ROM_FN(pgmfrog)

struct BurnDriver BurnDrvPgmfrog = {
	"pgmfrog", NULL, "pgm", NULL, "2006",
	"Frog Feast (PGM)\0", NULL, "RasterSoft", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING  | BDF_HOMEBREW, 4, HARDWARE_IGS_PGM, GBF_MISC, 0,
	NULL, pgmfrogRomInfo, pgmfrogRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	pgmInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// P-GeMeni (060123)

static struct BurnRomInfo pgemeniRomDesc[] = {
	{ "p0103.rom",     0x200000, 0x6cafa56b, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0100.rom",     0x400000, 0x42b979dd, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0100.rom",     0x400000, 0x105d7cee, 3 | BRF_GRA },		  //  2 Sprite Color Data

	{ "b0100.rom",     0x400000, 0xb4127373, 4 | BRF_GRA },		  //  3 Sprite Masks & Color Indexes

	{ "m0100.rom",     0x200000, 0x8d89877e, 5 | BRF_SND },		  //  4 Samples
};

STDROMPICKEXT(pgemeni, pgemeni, pgm)
STD_ROM_FN(pgemeni)

struct BurnDriver BurnDrvPgemeni = {
	"pgemeni", NULL, "pgm", NULL, "2006",
	"P-GeMeni (060123)\0", NULL, "blastar@gmx.net", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING  | BDF_HOMEBREW, 4, HARDWARE_IGS_PGM, GBF_PUZZLE, 0,
	NULL, pgemeniRomInfo, pgemeniRomName, NULL, NULL, pgmInputInfo, pgmDIPInfo,
	pgmInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


//-----------------------------------------------------------------------------------------------------------------
// Bootlegs


// Knights of Valour: Quan Huang San Guo Special / Sangoku Senki: Quan Huang San Guo Special (ver. 303CN)

static struct BurnRomInfo kovqhsgsRomDesc[] = {
	{ "qhsg_c51.rom",  	0x0400000, 0xe5cbac85, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "lsqh2_t01.rom",      0x1000000, 0xd498d97f, 2 | BRF_GRA },		//  1 Tile data

	{ "lsqh2_a01.rom",	0x1000000, 0x25ae3efd, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "lsqh2_a23.rom",	0x1000000, 0x7a779112, 3 | BRF_GRA },		//  3
	{ "lsqh2_a45.rom",	0x1000000, 0x5d7de052, 3 | BRF_GRA },		//  4

	{ "lsqh2_b01.rom",	0x1000000, 0xdf7ca696, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes

	{ "lsqh2_m01.rom",	0x1000000, 0x01af1b65, 5 | BRF_SND },		//  6 Samples

	{ "qhsg_prot.c51",	0x0004000, 0x0f09a5c1, 7 | BRF_ESS | BRF_PRG },	//  7 Internal ARM7 Rom
};

STDROMPICKEXT(kovqhsgs, kovqhsgs, pgm)
STD_ROM_FN(kovqhsgs)

static INT32 kovqhsgsInit()
{
	pPgmInitCallback = pgm_decrypt_kovqhsgs;
	pPgmProtCallback = install_protection_asic27a_kovsh;
	
	INT32 nRet = pgmInit();
	
	Arm7SetIdleLoopAddress(0x00000260);

	return nRet;
}

struct BurnDriver BurnDrvkovqhsgs = {
	"kovqhsgs", "kovsh", "pgm", NULL, "1999",
	"Knights of Valour: Quan Huang San Guo Special / Sangoku Senki: Quan Huang San Guo Special (ver. 303CN)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour: Quan Huang San Guo Special\0\u4E09\u56FD\u6218\u7EAA\0\u62F3\u7687\u4E09\u56FD\u7279\u522B\u7248 (ver. 303CN)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kovqhsgsRomInfo, kovqhsgsRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovqhsgsInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: Luan Shi Quan Huang 2 / Sangoku Senki: Luan Shi Quan Huang 2 (ver. 200CN)

static struct BurnRomInfo kovlsqh2RomDesc[] = {
	{ "lsqh2_prg.rom",  	0x0400000, 0xd71e3d50, 1 | BRF_ESS | BRF_PRG },	//  0 68K Code

	{ "lsqh2_t01.rom",      0x1000000, 0xd498d97f, 2 | BRF_GRA },		//  1 Tile data

	{ "lsqh2_a01.rom",	0x1000000, 0x25ae3efd, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "lsqh2_a23.rom",	0x1000000, 0x7a779112, 3 | BRF_GRA },		//  3
	{ "lsqh2_a45.rom",	0x1000000, 0x5d7de052, 3 | BRF_GRA },		//  4

	{ "lsqh2_b01.rom",	0x1000000, 0xdf7ca696, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes

	{ "lsqh2_m01.rom",	0x1000000, 0x01af1b65, 5 | BRF_SND },		//  6 Samples

	{ "qhsg_prot.c51",	0x0004000, 0x0f09a5c1, 7 | BRF_ESS | BRF_PRG },	//  7 Internal ARM7 Rom
};

STDROMPICKEXT(kovlsqh2, kovlsqh2, pgm)
STD_ROM_FN(kovlsqh2)

static INT32 kovlsqh2Init()
{
	pPgmInitCallback = pgm_decrypt_kovlsqh2;
//	pPgmProtCallback = install_protection_asic27a_kovsh;

	return pgmInit();
}

struct BurnDriverD BurnDrvkovlsqh2 = {
	"kovlsqh2", "kovshp", "pgm", NULL, "2009",
	"Knights of Valour: Luan Shi Quan Huang 2 / Sangoku Senki: Luan Shi Quan Huang 2 (ver. 200CN)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU */, GBF_SCRFIGHT, 0,
	NULL, kovlsqh2RomInfo, kovlsqh2RomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovlsqh2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: Luan Shi Quan Huang 2 / Sangoku Senki: Luan Shi Quan Huang (ver. 200CN)

static struct BurnRomInfo kovlsqhRomDesc[] = {
	{ "lsqh_v200cn.rom",  	0x0400000, 0x9935a27a, 1 | BRF_ESS | BRF_PRG },	//  0 68K Code

	{ "lsqh2_t01.rom",      0x1000000, 0xd498d97f, 2 | BRF_GRA },		//  1 Tile data

	{ "lsqh2_a01.rom",	0x1000000, 0x25ae3efd, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "lsqh2_a23.rom",	0x1000000, 0x7a779112, 3 | BRF_GRA },		//  3
	{ "lsqh2_a45.rom",	0x1000000, 0x5d7de052, 3 | BRF_GRA },		//  4

	{ "lsqh2_b01.rom",	0x1000000, 0xdf7ca696, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes

	{ "lsqh2_m01.rom",	0x1000000, 0x01af1b65, 5 | BRF_SND },		//  6 Samples

	{ "qhsg_prot.c51",	0x0004000, 0x0f09a5c1, 7 | BRF_ESS | BRF_PRG },	//  7 Internal ARM7 Rom
};

STDROMPICKEXT(kovlsqh, kovlsqh, pgm)
STD_ROM_FN(kovlsqh)

static INT32 kovlsqhInit()
{
	pPgmInitCallback = pgm_decrypt_kovlsqh2;
//	pPgmProtCallback = install_protection_asic27a_kovsh;
//	pPgmProtCallback = install_protection_asic27a_simulation_debug;

	return pgmInit();
}

struct BurnDriverD BurnDrvkovlsqh = {
	"kovlsqh", "kovshp", "pgm", NULL, "2009",
	"Knights of Valour: Luan Shi Quan Huang / Sangoku Senki: Luan Shi Quan Huang (ver. 200CN)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU */, GBF_SCRFIGHT, 0,
	NULL, kovlsqhRomInfo, kovlsqhRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovlsqhInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: Luan Shi Jie Ba / Sangoku Senki: Luan Shi Jie Ba (ver. 200CN)

static struct BurnRomInfo kovlsjbRomDesc[] = {
	{ "lsjb_prg.rom",  	0x0400000, 0xadf06b37, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "lsqh2_t01.rom",      0x1000000, 0xd498d97f, 2 | BRF_GRA },		//  1 Tile data

	{ "lsqh2_a01.rom",	0x1000000, 0x25ae3efd, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "lsqh2_a23.rom",	0x1000000, 0x7a779112, 3 | BRF_GRA },		//  3
	{ "lsqh2_a45.rom",	0x1000000, 0x5d7de052, 3 | BRF_GRA },		//  4

	{ "lsqh2_b01.rom",	0x1000000, 0xdf7ca696, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes

	{ "lsqh2_m01.rom",	0x1000000, 0x01af1b65, 5 | BRF_SND },		//  6 Samples

	{ "qhsg_prot.c51",	0x0004000, 0x0f09a5c1, 7 | BRF_ESS | BRF_PRG },	//  7 Internal ARM7 Rom
};

STDROMPICKEXT(kovlsjb, kovlsjb, pgm)
STD_ROM_FN(kovlsjb)

struct BurnDriverD BurnDrvkovlsjb = {
	"kovlsjb", "kovshp", "pgm", NULL, "2009",
	"Knights of Valour: Luan Shi Jie Ba / Sangoku Senki: Luan Shi Jie Ba (ver. 200CN)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovlsjbRomInfo, kovlsjbRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovlsqh2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: Luan Shi Jie Ba / Sangoku Senki: Luan Shi Jie Ba (alt ver. 200CN)

static struct BurnRomInfo kovlsjbaRomDesc[] = {
	{ "lsjba_prg.rom",  	0x0400000, 0x8b42f456, 1 | BRF_ESS | BRF_PRG },	//  0 68K Code

	{ "lsqh2_t01.rom",      0x1000000, 0xd498d97f, 2 | BRF_GRA },		//  1 Tile data

	{ "lsqh2_a01.rom",	0x1000000, 0x25ae3efd, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "lsqh2_a23.rom",	0x1000000, 0x7a779112, 3 | BRF_GRA },		//  3
	{ "lsqh2_a45.rom",	0x1000000, 0x5d7de052, 3 | BRF_GRA },		//  4

	{ "lsqh2_b01.rom",	0x1000000, 0xdf7ca696, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes

	{ "lsqh2_m01.rom",	0x1000000, 0x01af1b65, 5 | BRF_SND },		//  6 Samples

	{ "qhsg_prot.c51",	0x0004000, 0x0f09a5c1, 7 | BRF_ESS | BRF_PRG },	//  7 Internal ARM7 Rom
};

STDROMPICKEXT(kovlsjba, kovlsjba, pgm)
STD_ROM_FN(kovlsjba)

struct BurnDriverD BurnDrvkovlsjba = {
	"kovlsjba", "kovshp", "pgm", NULL, "2009",
	"Knights of Valour: Luan Shi Jie Ba / Sangoku Senki: Luan Shi Jie Ba (alt ver. 200CN)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovlsjbaRomInfo, kovlsjbaRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovlsqh2Init, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: Ao Shi San Guo / Sangoku Senki: Ao Shi San Guo (ver. 315CN)

static struct BurnRomInfo kovassgRomDesc[] = {
	{ "assg_v315cn.rom",  	0x0400000, 0xb9885d5c, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "lsqh2_t01.rom",      0x1000000, 0xd498d97f, 2 | BRF_GRA },		//  1 Tile data

	{ "lsqh2_a01.rom",	0x1000000, 0x25ae3efd, 3 | BRF_GRA },		//  2 Sprite Color Data
	{ "lsqh2_a23.rom",	0x1000000, 0x7a779112, 3 | BRF_GRA },		//  3
	{ "lsqh2_a45.rom",	0x1000000, 0x5d7de052, 3 | BRF_GRA },		//  4

	{ "lsqh2_b01.rom",	0x1000000, 0xdf7ca696, 4 | BRF_GRA },		//  5 Sprite Masks & Color Indexes

	{ "lsqh2_m01.rom",	0x1000000, 0x01af1b65, 5 | BRF_SND },		//  6 Samples

	{ "assg_prot.arm", 	0x0010000, 0x00000000, 7 | BRF_ESS | BRF_PRG | BRF_NODUMP } // 7 Internal ARM7 Rom
};

STDROMPICKEXT(kovassg, kovassg, pgm)
STD_ROM_FN(kovassg)

static INT32 kovassgInit()
{
	pPgmInitCallback = pgm_decrypt_kovassg;
//	pPgmProtCallback = install_protection_asic27a_kovsh;

	return pgmInit();
}

struct BurnDriverD BurnDrvkovassg = {
	"kovassg", "kovshp", "pgm", NULL, "2009",
	"Knights of Valour: Ao Shi San Guo / Sangoku Senki: Ao Shi San Guo (ver. 315CN)\0", NULL, "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_IGS_PGM /* | HARDWARE_IGS_USE_ARM_CPU */, GBF_SCRFIGHT, 0,
	NULL, kovassgRomInfo, kovassgRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovassgInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: Aoshi Sanguo / Sangoku Senki: Aoshi Sanguo (ver. 202CN)

static struct BurnRomInfo kovshxasRomDesc[] = {
	{ "de_p0609.322",  0x400000, 0x3b7b627f, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data

	{ "a0600.rom",	   0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",	   0x800000, 0xff7a4373, 3 | BRF_GRA }, 	  //  3 
	{ "a0602.rom",	   0x800000, 0xe7a32959, 3 | BRF_GRA }, 	  //  4
	{ "a0603xas.rom",  0x800000, 0x7057b37e, 3 | BRF_GRA }, 	  //  5

	{ "b0600.rom",	   0x800000, 0x7d3cd059, 4 | BRF_GRA },		  //  6 Sprite Masks & Color Indexes
	{ "b0601xas.rom",  0x800000, 0x3784fb49, 4 | BRF_GRA },		  //  7

	{ "m0600.rom",	   0x400000, 0x3ada4fd6, 5 | BRF_SND },		  //  8 Samples

	{ "kovshxas_v100_china.asic", 0x004000, 0x00000000, 0 | BRF_ESS | BRF_PRG | BRF_NODUMP }, //  9 Internal ARM7 Rom
};

STDROMPICKEXT(kovshxas, kovshxas, pgm)
STD_ROM_FN(kovshxas)

static INT32 kovshxasInit()
{
	pPgmProtCallback = install_protection_asic27a_kovsh;

	return pgmInit();
}

struct BurnDriverD BurnDrvkovshxas = {
	"kovshxas", "kovshp", "pgm", NULL, "2004",
	"Knights of Valour: Aoshi Sanguo / Sangoku Senki: Aoshi Sanguo (ver. 202CN)\0", "Incomplete dump", "IGS", "PolyGameMaster",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU, GBF_SCRFIGHT, 0,
	NULL, kovshxasRomInfo, kovshxasRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovshxasInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (set 1)

static struct BurnRomInfo kovsgqyzRomDesc[] = {
	{ "pgm_sgqyz.rom", 0x400000, 0x18e1eed9, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  3
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  4
	{ "qyza0603.rom",  0x800000, 0xc8b92220, 3 | BRF_GRA },		  //  5

	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // 10 Sprite Masks & Color Indexes
	{ "qyzb0601.rom",  0x800000, 0x64f55882, 4 | BRF_GRA },		  // 11
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // 12 Samples	
	
	{ "kovsgqyz_igs027a.bin", 0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 13 Internal ARM7 Rom
};

STDROMPICKEXT(kovsgqyz, kovsgqyz, pgm)
STD_ROM_FN(kovsgqyz)

static INT32 kovsgqyzInit()
{
	nPGMEnableIRQ4 = 1;
	pPgmProtCallback = install_protection_asic27_kov; // simulation

	return pgmInit();
}

struct BurnDriver BurnDrvKovsgqyz = {
	"kovsgqyz", "kovplus", "pgm", NULL, "1999",
	"Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (set 1)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour: SanGuo QunYingZhuan\0\u4E09\u56FD\u6218\u7EAA\0\u4E09\u56FD\u7FA4\u82F1\u4F20 (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovsgqyzRomInfo, kovsgqyzRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovsgqyzInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (set 2)

static struct BurnRomInfo kovsgqyzaRomDesc[] = {
	{ "pgm_sgqyza.rom",0x400000, 0x5a30dcb7, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  3
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  4
	{ "qyza0603.rom",  0x800000, 0xc8b92220, 3 | BRF_GRA },		  //  5

	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // 10 Sprite Masks & Color Indexes
	{ "qyzb0601.rom",  0x800000, 0x64f55882, 4 | BRF_GRA },		  // 11
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // 12 Samples	
	
	{ "kovsgqyza_igs027a.bin", 0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 13 Internal ARM7 Rom
};

STDROMPICKEXT(kovsgqyza, kovsgqyza, pgm)
STD_ROM_FN(kovsgqyza)

struct BurnDriver BurnDrvKovsgqyza = {
	"kovsgqyza", "kovplus", "pgm", NULL, "1999",
	"Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (set 2)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour: SanGuo QunYingZhuan\0\u4E09\u56FD\u6218\u7EAA\0\u4E09\u56FD\u7FA4\u82F1\u4F20 (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovsgqyzaRomInfo, kovsgqyzaRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovsgqyzInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};


// Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (set 3)

static struct BurnRomInfo kovsgqyzbRomDesc[] = {
	{ "pgm_sgqyzb.rom",0x400000, 0x18b8b9c0, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  //  1 Tile data
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  //  2 Sprite Color Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },		  //  3
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },		  //  4
	{ "qyza0603.rom",  0x800000, 0xc8b92220, 3 | BRF_GRA },		  //  5

	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // 10 Sprite Masks & Color Indexes
	{ "qyzb0601.rom",  0x800000, 0x64f55882, 4 | BRF_GRA },		  // 11
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // 12 Samples	
	
	{ "kovsgqyzb_igs027a.bin", 0x004000, 0x00000000, 7 | BRF_PRG | BRF_NODUMP }, // 13 Internal ARM7 Rom
};

STDROMPICKEXT(kovsgqyzb, kovsgqyzb, pgm)
STD_ROM_FN(kovsgqyzb)

struct BurnDriver BurnDrvKovsgqyzb = {
	"kovsgqyzb", "kovplus", "pgm", NULL, "1999",
	"Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (set 3)\0", NULL, "IGS", "PolyGameMaster",
	L"Knights of Valour: SanGuo QunYingZhuan\0\u4E09\u56FD\u6218\u7EAA\0\u4E09\u56FD\u7FA4\u82F1\u4F20 (set 3)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IGS_PGM/* | HARDWARE_IGS_USE_ARM_CPU*/, GBF_SCRFIGHT, 0,
	NULL, kovsgqyzbRomInfo, kovsgqyzbRomName, NULL, NULL, pgmInputInfo, sangoDIPInfo,
	kovsgqyzInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc, 0x600,
	448, 224, 4, 3
};
