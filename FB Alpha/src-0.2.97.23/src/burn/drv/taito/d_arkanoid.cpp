// FB Alpha Arkanoid driver module
// Based on MAME driver by Brad Oliver and MANY others.

#include "tiles_generic.h"
#include "taito_m68705.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvMcuROM;
static UINT8 *DrvGfxROM;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvMcuRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvSprRAM;
static INT16 *pAY8910Buffer[3];

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *flipscreen;
static UINT8 *gfxbank;
static UINT8 *palettebank;
static UINT8 *paddleselect;
static UINT8 *bankselect;

static UINT8 DrvInputs[4];
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[1];
static UINT8 DrvReset;
static UINT16 DrvAxis[2];
static UINT32 nAnalogAxis[2] = {0,0};

static INT32 arkanoid_bootleg_id;
static INT32 use_mcu;

static UINT8 arkanoid_bootleg_cmd;

enum {
	ARKUNK=0,
	ARKANGC,
	ARKANGC2,
	ARKBLOCK,
	ARKBLOC2,
	ARKGCBL,
	PADDLE2,
	HEXA,
	TETRSARK
};

#define A(a, b, c, d) { a, b, (UINT8*)(c), d }

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 coin"  },
	{"P1 start"  ,    BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 0,	"p1 fire 1"},

	A("P1 Right / left",	BIT_ANALOG_REL, DrvAxis + 0,	"p1 x-axis"),

	{"P2 Coin"      , BIT_DIGITAL  , DrvJoy1 + 5,	"p2 coin"  },
	{"P2 start"  ,    BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 2,	"p2 fire 1"},

	A("P2 Right / left",	BIT_ANALOG_REL, DrvAxis + 1,	"p1 x-axis"),

	{"Tilt",	  BIT_DIGITAL,   DrvJoy1 + 3,	"tilt"     },
	{"Service",	  BIT_DIGITAL,   DrvJoy1 + 2,   "diag"     },
	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo tetrsarkInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL,   DrvJoy1 + 6,	"p1 coin"  },
	{"P1 Start"  ,    BIT_DIGITAL,   DrvJoy1 + 4,	"p1 start" },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy1 + 0,   "p1 down"  },
	{"P1 Left",	  BIT_DIGITAL,	 DrvJoy1 + 1,   "p1 left"  },
	{"P1 Right",	  BIT_DIGITAL,   DrvJoy1 + 2,   "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL,   DrvJoy1 + 3,	"p1 fire 1"},

	{"P2 Coin"      , BIT_DIGITAL,   DrvJoy1 + 7,	"p2 coin"  },
	{"P2 start"  ,    BIT_DIGITAL,   DrvJoy1 + 5,	"p2 start" },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy2 + 0,   "p2 down"  },
	{"P2 Left",	  BIT_DIGITAL,	 DrvJoy2 + 1,   "p2 left"  },
	{"P2 Right",	  BIT_DIGITAL,   DrvJoy2 + 2,   "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL,   DrvJoy2 + 3,	"p2 fire 1"},

	{"Reset",	  BIT_DIGITAL,   &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
};

STDINPUTINFO(tetrsark)

static struct BurnInputInfo HexaInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(Hexa)

static struct BurnDIPInfo arkanoidDIPList[]=
{
	{0x0b, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x0b, 0x01, 0x01, 0x01, "No"       		  },
	{0x0b, 0x01, 0x01, 0x00, "Yes"       		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0b, 0x01, 0x02, 0x02, "Off"			  },
	{0x0b, 0x01, 0x02, 0x00, "On"			  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0b, 0x01, 0x08, 0x08, "Easy"     		  },
	{0x0b, 0x01, 0x08, 0x00, "Hard"		          },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0b, 0x01, 0x10, 0x10, "20K 60K 60K+" 	  },
	{0x0b, 0x01, 0x10, 0x00, "20K"    		  },

	{0   , 0xfe, 0   , 2   , "Lives" 	          },
	{0x0b, 0x01, 0x20, 0x20, "3"			  },
	{0x0b, 0x01, 0x20, 0x00, "5"    		  },

	{0   , 0xfe, 0   , 4   , "Coinage" 	          },
	{0x0b, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"	  },
	{0x0b, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"    	  },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	  },
	{0x0b, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	  },
};

STDDIPINFO(arkanoid)

static struct BurnDIPInfo arknoidjDIPList[]=
{
	{0x0b, 0xff, 0xff, 0x7f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x0b, 0x01, 0x01, 0x01, "No"       		  },
	{0x0b, 0x01, 0x01, 0x00, "Yes"       		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0b, 0x01, 0x02, 0x02, "Off"			  },
	{0x0b, 0x01, 0x02, 0x00, "On"			  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0b, 0x01, 0x08, 0x08, "Easy"     		  },
	{0x0b, 0x01, 0x08, 0x00, "Hard"		          },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0b, 0x01, 0x10, 0x10, "20K 60K 60K+" 	  },
	{0x0b, 0x01, 0x10, 0x00, "20K"    		  },

	{0   , 0xfe, 0   , 2   , "Lives" 	          },
	{0x0b, 0x01, 0x20, 0x20, "3"			  },
	{0x0b, 0x01, 0x20, 0x00, "5"    		  },

	{0   , 0xfe, 0   , 2   , "Coinage" 	          },
	{0x0b, 0x01, 0x40, 0x40, "1 Coin  1 Credit"	  },
	{0x0b, 0x01, 0x40, 0x00, "1 Coin  2 Credits"      },

	{0   , 0xfe, 0   , 2   , "Cabinet" 	          },
	{0x0b, 0x01, 0x80, 0x00, "Upright"		  },
	{0x0b, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(arknoidj)

static struct BurnDIPInfo ark1ballDIPList[]=
{
	{0x0b, 0xff, 0xff, 0x7f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x0b, 0x01, 0x01, 0x01, "No"       		  },
	{0x0b, 0x01, 0x01, 0x00, "Yes"       		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0b, 0x01, 0x02, 0x02, "Off"			  },
	{0x0b, 0x01, 0x02, 0x00, "On"			  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0b, 0x01, 0x08, 0x08, "Easy"     		  },
	{0x0b, 0x01, 0x08, 0x00, "Hard"		          },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0b, 0x01, 0x10, 0x10, "60K 100K 60K+" 	  },
	{0x0b, 0x01, 0x10, 0x00, "60K"    		  },

	{0   , 0xfe, 0   , 2   , "Lives" 	          },
	{0x0b, 0x01, 0x20, 0x20, "1"			  },
	{0x0b, 0x01, 0x20, 0x00, "2"    		  },

	{0   , 0xfe, 0   , 2   , "Coinage" 	          },
	{0x0b, 0x01, 0x40, 0x40, "1 Coin  1 Credit"	  },
	{0x0b, 0x01, 0x40, 0x00, "1 Coin  2 Credits"      },

	{0   , 0xfe, 0   , 2   , "Cabinet" 	          },
	{0x0b, 0x01, 0x80, 0x00, "Upright"		  },
	{0x0b, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(ark1ball)

static struct BurnDIPInfo arkangcDIPList[]=
{
	{0x0b, 0xff, 0xff, 0x7f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Ball Speed"             },
	{0x0b, 0x01, 0x01, 0x01, "Normal"       	  },
	{0x0b, 0x01, 0x01, 0x00, "Faster"      		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0b, 0x01, 0x02, 0x02, "Off"			  },
	{0x0b, 0x01, 0x02, 0x00, "On"			  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0b, 0x01, 0x08, 0x08, "Easy"     		  },
	{0x0b, 0x01, 0x08, 0x00, "Hard"		          },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0b, 0x01, 0x10, 0x10, "20K 60K 60K+" 	  },
	{0x0b, 0x01, 0x10, 0x00, "20K"    		  },

	{0   , 0xfe, 0   , 2   , "Lives" 	          },
	{0x0b, 0x01, 0x20, 0x20, "3"			  },
	{0x0b, 0x01, 0x20, 0x00, "5"    		  },

	{0   , 0xfe, 0   , 2   , "Coinage" 	          },
	{0x0b, 0x01, 0x40, 0x40, "1 Coin  1 Credit"	  },
	{0x0b, 0x01, 0x40, 0x00, "1 Coin  2 Credits"      },

	{0   , 0xfe, 0   , 2   , "Cabinet" 	          },
	{0x0b, 0x01, 0x80, 0x00, "Upright"		  },
	{0x0b, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(arkangc)

static struct BurnDIPInfo arkangc2DIPList[]=
{
	{0x0b, 0xff, 0xff, 0x7f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Ball Speed"             },
	{0x0b, 0x01, 0x01, 0x01, "Slower"       	  },
	{0x0b, 0x01, 0x01, 0x00, "Normal"      		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0b, 0x01, 0x02, 0x02, "Off"			  },
	{0x0b, 0x01, 0x02, 0x00, "On"			  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0b, 0x01, 0x08, 0x08, "Easy"     		  },
	{0x0b, 0x01, 0x08, 0x00, "Hard"		          },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0b, 0x01, 0x10, 0x10, "20K 60K 60K+" 	  },
	{0x0b, 0x01, 0x10, 0x00, "20K"    		  },

	{0   , 0xfe, 0   , 2   , "Lives" 	          },
	{0x0b, 0x01, 0x20, 0x20, "3"			  },
	{0x0b, 0x01, 0x20, 0x00, "5"    		  },

	{0   , 0xfe, 0   , 2   , "Coinage" 	          },
	{0x0b, 0x01, 0x40, 0x40, "1 Coin  1 Credit"	  },
	{0x0b, 0x01, 0x40, 0x00, "1 Coin  2 Credits"      },

	{0   , 0xfe, 0   , 2   , "Cabinet" 	          },
	{0x0b, 0x01, 0x80, 0x00, "Upright"		  },
	{0x0b, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(arkangc2)

static struct BurnDIPInfo arkgcblDIPList[]=
{
	{0x0b, 0xff, 0xff, 0x5f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Round Select"           },
	{0x0b, 0x01, 0x01, 0x01, "Off"       		  },
	{0x0b, 0x01, 0x01, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0b, 0x01, 0x02, 0x02, "Off"			  },
	{0x0b, 0x01, 0x02, 0x00, "On"			  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0b, 0x01, 0x08, 0x08, "Easy"     		  },
	{0x0b, 0x01, 0x08, 0x00, "Hard"		          },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0b, 0x01, 0x10, 0x10, "60K 100K 60K+" 	  },
	{0x0b, 0x01, 0x10, 0x00, "60K"    		  },

	{0   , 0xfe, 0   , 2   , "Lives" 	          },
	{0x0b, 0x01, 0x20, 0x00, "2"			  },
	{0x0b, 0x01, 0x20, 0x20, "3"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet" 	          },
	{0x0b, 0x01, 0x80, 0x00, "Upright"		  },
	{0x0b, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(arkgcbl)

static struct BurnDIPInfo paddle2DIPList[]=
{
	{0x0b, 0xff, 0xff, 0x5f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Round Select"           },
	{0x0b, 0x01, 0x01, 0x01, "Off"       		  },
	{0x0b, 0x01, 0x01, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0b, 0x01, 0x02, 0x02, "Off"			  },
	{0x0b, 0x01, 0x02, 0x00, "On"			  },

	{0   , 0xfe, 0   , 2   , "Controls ?"             },
	{0x0b, 0x01, 0x04, 0x04, "Normal"		  },
	{0x0b, 0x01, 0x04, 0x00, "Alternate"		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0b, 0x01, 0x08, 0x08, "Easy"     		  },
	{0x0b, 0x01, 0x08, 0x00, "Hard"		          },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0b, 0x01, 0x10, 0x10, "60K 60K 60K+" 	  },
	{0x0b, 0x01, 0x10, 0x00, "60K"    		  },

	{0   , 0xfe, 0   , 2   , "Lives" 	          },
	{0x0b, 0x01, 0x20, 0x00, "2"			  },
	{0x0b, 0x01, 0x20, 0x20, "3"    		  },

	{0   , 0xfe, 0   , 2   , "Coinage" 	          },
	{0x0b, 0x01, 0x40, 0x40, "1 Coin  1 Credit"	  },
	{0x0b, 0x01, 0x40, 0x00, "1 Coin  2 Credits"      },

	{0   , 0xfe, 0   , 2   , "Cabinet" 	          },
	{0x0b, 0x01, 0x80, 0x00, "Upright"		  },
	{0x0b, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(paddle2)

static struct BurnDIPInfo arktayt2DIPList[]=
{
	{0x0b, 0xff, 0xff, 0x3f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x0b, 0x01, 0x01, 0x01, "No"       		  },
	{0x0b, 0x01, 0x01, 0x00, "Yes"       		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0b, 0x01, 0x02, 0x02, "Off"			  },
	{0x0b, 0x01, 0x02, 0x00, "On"			  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0b, 0x01, 0x08, 0x08, "Easy"     		  },
	{0x0b, 0x01, 0x08, 0x00, "Hard"		          },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0b, 0x01, 0x10, 0x10, "60K 60K 60K+" 	  },
	{0x0b, 0x01, 0x10, 0x00, "60K"    		  },

	{0   , 0xfe, 0   , 2   , "Lives" 	          },
	{0x0b, 0x01, 0x20, 0x20, "2"			  },
	{0x0b, 0x01, 0x20, 0x00, "3"    		  },

	{0   , 0xfe, 0   , 2   , "Coinage" 	          },
	{0x0b, 0x01, 0x40, 0x00, "2 Coins 1 Credit"	  },
	{0x0b, 0x01, 0x40, 0x40, "1 Coin  1 Credit"    	  },

	{0   , 0xfe, 0   , 2   , "Cabinet" 	          },
	{0x0b, 0x01, 0x80, 0x00, "Upright"		  },
	{0x0b, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(arktayt2)

static struct BurnDIPInfo tetrsarkDIPList[]=
{
	{0x0d, 0xff, 0xff, 0xf0, NULL                     },

	{0   , 0xfe, 0   , 2   , "Cabinet"	          },
	{0x0d, 0x01, 0x10, 0x10, "Upright"       	  },
	{0x0d, 0x01, 0x10, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0d, 0x01, 0x20, 0x20, "Off"			  },
	{0x0d, 0x01, 0x20, 0x00, "On"			  },

	{0   , 0xfe, 0   , 4   , "Coinage" 	          },
	{0x0d, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"	  },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"      },
	{0x0d, 0x01, 0xc0, 0x40, "1C 3C"		  },
	{0x0d, 0x01, 0xc0, 0x00, "1C 5C"    		  },
};

STDDIPINFO(tetrsark)

static struct BurnDIPInfo HexaDIPList[]=
{
	{0x09, 0xff, 0xff, 0xfb, NULL			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x09, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x09, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x09, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x09, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Naughty Pics"		},
	{0x09, 0x01, 0x04, 0x04, "Off"			},
	{0x09, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x09, 0x01, 0x08, 0x08, "Off"			},
	{0x09, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty?"		},
	{0x09, 0x01, 0x30, 0x30, "Easy?"		},
	{0x09, 0x01, 0x30, 0x20, "Medium?"		},
	{0x09, 0x01, 0x30, 0x10, "Hard?"		},
	{0x09, 0x01, 0x30, 0x00, "Hardest?"		},

	{0   , 0xfe, 0   ,    2, "Pobys"		},
	{0x09, 0x01, 0x40, 0x40, "2"			},
	{0x09, 0x01, 0x40, 0x00, "4"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x09, 0x01, 0x80, 0x00, "Off"			},
	{0x09, 0x01, 0x80, 0x80, "On"			},
};

STDDIPINFO(Hexa)

static UINT8 arkanoid_bootleg_f002_read()
{
	UINT8 arkanoid_bootleg_val = 0x00;

	switch (arkanoid_bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
		case ARKBLOC2:
		case ARKANGC2:
		break;

		case ARKGCBL:
			switch (arkanoid_bootleg_cmd)
			{
				case 0x8a:
					arkanoid_bootleg_val = 0xa5;
					break;
				case 0xff:
					arkanoid_bootleg_val = 0xe2;
					break;
				default:
					break;
			}
			break;

		case PADDLE2:
			switch (arkanoid_bootleg_cmd)
			{
				case 0x24:  // Avoid bad jump to 0x0066
					arkanoid_bootleg_val = 0x9b;
					break;
				case 0x36:  // Avoid "BAD HARDWARE    " message
					arkanoid_bootleg_val = 0x2d;
					break;
				case 0x38:  // Start of levels table (fixed offset)
					arkanoid_bootleg_val = 0xf3;
					break;
				case 0x8a:  // Current level (fixed routine)
					arkanoid_bootleg_val = 0xa5;
					break;
				case 0xc3:  // Avoid bad jump to 0xf000
					arkanoid_bootleg_val = 0x1d;
					break;
				case 0xe3:  // Number of bricks left (fixed offset)
					arkanoid_bootleg_val = 0x61;
					break;
				case 0xf7:  // Avoid "U69" message
					arkanoid_bootleg_val = 0x00;
					break;
				case 0xff:  // Avoid "BAD HARDWARE    " message (fixed routine)
					arkanoid_bootleg_val = 0xe2;
					break;
				default:
					break;
			}
			break;
	}

	return arkanoid_bootleg_val;
}

static void arkanoid_bootleg_d018_write(UINT8 data)
{
	arkanoid_bootleg_cmd = 0x00;

	switch (arkanoid_bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			switch (data)
			{
				case 0x36:  // unneeded value : no call 0x2050, unused A and overwritten HL (0x0313 -> 0x0340)
					if (ZetPc(-1) == 0x7c47)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x38:  // unneeded value : no call 0x2050, unused A and fixed HL (0x7bd5)
					if (ZetPc(-1) == 0x7b87)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x8a:  // unneeded value : no call 0x2050, unused A and overwritten HL (0x7b77 -> 0x7c1c)
					if (ZetPc(-1) == 0x9661)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xe3:  // unneeded value : call 0x2050 but fixed A (0x00) and fixed HL (0xed83)
					if (ZetPc(-1) == 0x67e3)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xf7:  // unneeded value : 3 * 'NOP' at 0x034f + 2 * 'NOP' at 0x35b
					if (ZetPc(-1) == 0x0349)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xff:  // unneeded value : no call 0x2050, unused A and overwritten HL (0x7c4f -> 0x7d31)
					if (ZetPc(-1) == 0x9670)
						arkanoid_bootleg_cmd = 0x00;
					break;
				default:
					arkanoid_bootleg_cmd = 0x00;
					break;
			}
			break;

		case ARKANGC2:
			switch (data)
			{
				case 0x36:  // unneeded value : call 0x2050 but fixed A (0x2d)
					if (ZetPc(-1) == 0x7c4c)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x38:  // unneeded value : call 0x2050 but fixed A (0xf3)
					if (ZetPc(-1) == 0x7b87)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x88:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e3)
						arkanoid_bootleg_cmd = 0x00;
					if (ZetPc(-1) == 0x7c47)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x89:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e5)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x8a:  // unneeded value : call 0x2050 but fixed A (0xa5)
					if (ZetPc(-1) == 0x9661)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xc0:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e7)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xe3:  // unneeded value : call 0x2050 but fixed A (0x61)
					if (ZetPc(-1) == 0x67e9)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xff:  // unneeded value : call 0x2050 but fixed A (0xe2)
					if (ZetPc(-1) == 0x9670)
						arkanoid_bootleg_cmd = 0x00;
					break;
				default:
					arkanoid_bootleg_cmd = 0x00;
					break;
			}
			break;

		case ARKBLOC2:
			switch (data)
			{
				case 0x36:  // unneeded value : call 0x2050 but fixed A (0x2d)
					if (ZetPc(-1) == 0x7c4c)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x38:  // unneeded value : call 0x2050 but fixed A (0xf3)
					if (ZetPc(-1) == 0x7b87)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x88:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e3)
						arkanoid_bootleg_cmd = 0x00;
					if (ZetPc(-1) == 0x7c47)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x89:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e5)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x8a:  // unneeded value : call 0x2050 but unused HL and fixed DE (0x7c1c)
					if (ZetPc(-1) == 0x9661)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xc0:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e7)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xe3:  // unneeded value : call 0x2050 but fixed A (0x61)
					if (ZetPc(-1) == 0x67e9)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xf7:  // unneeded value : call 0x2050 but never called (check code at 0x0340)
					if (ZetPc(-1) == 0x0349)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xff:  // unneeded value : no call 0x2050, unused A and fixed HL (0x7d31)
					if (ZetPc(-1) == 0x9670)
						arkanoid_bootleg_cmd = 0x00;
					break;
				default:
					arkanoid_bootleg_cmd = 0x00;
					break;
			}
			break;

		case ARKGCBL:
			switch (data)
			{
				case 0x36:  // unneeded value : call 0x2050 but fixed A (0x2d)
					if (ZetPc(-1) == 0x7c4c)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x38:  // unneeded value : call 0x2050 but fixed A (0xf3)
					if (ZetPc(-1) == 0x7b87)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x88:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e3)
						arkanoid_bootleg_cmd = 0x00;
					if (ZetPc(-1) == 0x7c47)
						arkanoid_bootleg_cmd = 0x00;
				case 0x89:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e5)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x8a:  // call 0x2050 with A read from 0xf002 and wrong HL
					if (ZetPc(-1) == 0x9661)
						arkanoid_bootleg_cmd = data;
					break;
				case 0xc0:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e7)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xe3:  // unneeded value : call 0x2050 but fixed A (0x61)
					if (ZetPc(-1) == 0x67e9)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xf7:  // unneeded value : 3 * 'NOP' at 0x034f + 'JR NZ,$035D' at 0x35b
					if (ZetPc(-1) == 0x0349)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xff:  // call 0x2050 with A read from 0xf002 and wrong HL
					if (ZetPc(-1) == 0x9670)
						arkanoid_bootleg_cmd = data;
					break;
				default:
					arkanoid_bootleg_cmd = 0x00;
					break;
			}
			break;

		case PADDLE2:
			switch (data)
			{
				case 0x24:  // A read from 0xf002 (expected to be 0x9b)
					if (ZetPc(-1) == 0xbd7a)
						arkanoid_bootleg_cmd = data;
					break;
				case 0x36:  // call 0x2050 with A read from 0xf002 and wrong HL
					if (ZetPc(-1) == 0x7c4c)
						arkanoid_bootleg_cmd = data;
					break;
				case 0x38:  // call 0x2050 with A read from 0xf002 and wrong HL
					if (ZetPc(-1) == 0x7b87)
						arkanoid_bootleg_cmd = data;
					break;
				case 0x88:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e3)
						arkanoid_bootleg_cmd = 0x00;
					if (ZetPc(-1) == 0x7c47)
						arkanoid_bootleg_cmd = 0x00;
				case 0x89:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e5)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0x8a:  // call 0x2050 with A read from 0xf002 and wrong HL
					if (ZetPc(-1) == 0x9661)
						arkanoid_bootleg_cmd = data;
					break;
				case 0xc0:  // unneeded value : no read back
					if (ZetPc(-1) == 0x67e7)
						arkanoid_bootleg_cmd = 0x00;
					break;
				case 0xc3:  // A read from 0xf002 (expected to be 0x1d)
					if (ZetPc(-1) == 0xbd8a)
						arkanoid_bootleg_cmd = data;
					break;
				case 0xe3:  // call 0x2050 with A read from 0xf002 and wrong HL
					if (ZetPc(-1) == 0x67e9)
						arkanoid_bootleg_cmd = data;
					break;
				case 0xf7:  // call 0x2050 with A read from 0xf002 and wrong HL
					if (ZetPc(-1) == 0x0349)
						arkanoid_bootleg_cmd = data;
					break;
				case 0xff:  // call 0x2050 with A read from 0xf002 and wrong HL
					if (ZetPc(-1) == 0x9670)
						arkanoid_bootleg_cmd = data;
					break;
				default:
					arkanoid_bootleg_cmd = 0x00;
					break;
			}
			break;

		default:
			break;
	}
}

static UINT8 arkanoid_bootleg_d008_read()
{
	switch (arkanoid_bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			return 0x00;

		case ARKANGC2:
			return 0x02;

		case ARKBLOC2:
			return (DrvInputs[2] < 0x40) << 5;

		case ARKGCBL:
			return ((DrvInputs[2] < 0x40) << 5) | 0x02;

		case PADDLE2:
			return ((DrvInputs[2] < 0x40) << 5) | 0x0f;

		default:
			return 0x00;
	}

	return 0;
}

UINT8 __fastcall arkanoid_read(UINT16 address)
{
	switch (address)
	{
		case 0xd001:
			return AY8910Read(0);

		case 0xd008:
			return arkanoid_bootleg_d008_read();

		case 0xd00c:
		{
			INT32 ret = DrvInputs[0];
			if (use_mcu) {
				ret &= 0x3f;

				if (!main_sent) ret |= 0x40;
				if (!mcu_sent ) ret |= 0x80;
			}
			return ret;
		}

		case 0xd010:
			return DrvInputs[1];

		case 0xd018:
			if (use_mcu) {
				return standard_taito_mcu_read();
			} else {
				return DrvInputs[2];
			}

		case 0xf002:
			return arkanoid_bootleg_f002_read();
	}

	if (address >= 0xf000) return DrvZ80ROM[address];

	return 0;
}

void __fastcall arkanoid_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xd000:
		case 0xd001:
			AY8910Write(0, address & 1, data);
		break;

		case 0xd008:
		{
			*flipscreen  = (data >> 0) & 3;
			*gfxbank     = (data >> 5) & 1;
			*palettebank = (data >> 6) & 1;
			*paddleselect= (data >> 2) & 1;
		}
		break;
	
		case 0xd010: // watchdog
		break;

		case 0xd018:
			if (use_mcu) {
				from_main = data;
				main_sent = 1;
			} else {
				arkanoid_bootleg_d018_write(data);
			}
		break;
	}
}

static void bankswitch(INT32 data)
{
	bankselect[0] = data;

	INT32 bank = 0x8000 + ((data & 0x10) >> 4) * 0x4000;

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + bank);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + bank);
}

void __fastcall hexa_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xd000:
		case 0xd001:
			AY8910Write(0, address & 1, data);
		return;

		case 0xd008:
			*flipscreen = data & 0x03; // flipx & flipy
			*gfxbank    =(data & 0x20) >> 5;
			bankswitch(data);
		return;
	}
}

static void arkanoid_m68705_portC_write(UINT8 *data)
{
	if ((ddrC & 0x04) && (~*data & 0x04) && (portC_out & 0x04))
	{
		main_sent = 0;
		portA_in = from_main;
	}
	if ((ddrC & 0x08) && (~*data & 0x08) && (portC_out & 0x08))
	{
		mcu_sent = 1;
		from_mcu = portA_out;
	}

	portC_out = *data;
}

static void arkanoid_m68705_portB_read()
{
	ddrB = 0xff;
	portB_out = (*paddleselect) ? DrvInputs[3] : DrvInputs[2];
}

static m68705_interface arkanoid_m68705_interface = {
	NULL,
	NULL,
	arkanoid_m68705_portC_write,
	NULL,
	NULL,
	NULL,
	NULL,
	arkanoid_m68705_portB_read,
	standard_m68705_portC_in
};

static UINT8 ay8910_read_port_4(UINT32)
{
	INT32 ret = DrvDips[0];
	if (arkanoid_bootleg_id == TETRSARK) ret |= DrvInputs[1];

	return ret;
}

static UINT8 ay8910_read_port_5(UINT32)
{
	return DrvInputs[0];
}

static void DrvPaletteInit()
{
	INT32 len = BurnDrvGetPaletteEntries();

	for (INT32 i = 0; i < len; i++)
	{
		INT32 bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (DrvColPROM[i + len * 0] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + len * 0] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + len * 0] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + len * 0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + len * 1] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + len * 1] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + len * 1] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + len * 1] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + len * 2] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + len * 2] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + len * 2] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + len * 2] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void DrvGfxDecode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x40000);
	for (INT32 i = 0; i < 0x40000; i++) {
		tmp[i]  = ((DrvGfxROM[(i / 8) + 0x00000] >> (i & 7)) & 1) << 0;
		tmp[i] |= ((DrvGfxROM[(i / 8) + 0x08000] >> (i & 7)) & 1) << 1;
		tmp[i] |= ((DrvGfxROM[(i / 8) + 0x10000] >> (i & 7)) & 1) << 2;
	}

	memcpy (DrvGfxROM, tmp, 0x40000);
	BurnFree (tmp);
}

static INT32 GetRoms()
{
	char* pRomName;
	struct BurnRomInfo ri;
	UINT8 *RomLoad = DrvZ80ROM;
	UINT8 *GfxLoad = DrvGfxROM;
	UINT8 *PrmLoad = DrvColPROM;
	use_mcu = 0;

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & 7) == 1) {
			if (BurnLoadRom(RomLoad, i, 1)) return 1;
			RomLoad += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 2) {
			if (BurnLoadRom(DrvMcuROM, i, 1)) return 1;
			use_mcu = 1;
			continue;
		}

		if ((ri.nType & 7) == 3) {
			if (BurnLoadRom(GfxLoad, i, 1)) return 1;
			GfxLoad += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 4) {
			if (BurnLoadRom(PrmLoad, i, 1)) return 1;
			PrmLoad += ri.nLen;
			continue;
		}
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

	m67805_taito_reset();

	AY8910Reset(0);

	nAnalogAxis[0] = 0;
	nAnalogAxis[1] = 0;
	arkanoid_bootleg_cmd = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM		= Next; Next += 0x010000;
	DrvMcuROM		= Next; Next += 0x000800;

	DrvGfxROM		= Next; Next += 0x040000;

	DrvColPROM		= Next; Next += 0x000800;

	DrvPalette		= (UINT32*)Next; Next += 0x0200 * sizeof(UINT32);

	AllRam			= Next;

	DrvZ80RAM		= Next; Next += 0x000800;
	DrvMcuRAM		= Next; Next += 0x000080;
	DrvVidRAM		= Next; Next += 0x000800;
	DrvSprRAM		= Next; Next += 0x000800;

	flipscreen		= Next; Next += 0x000001;
	gfxbank			= Next; Next += 0x000001;
	palettebank		= Next; Next += 0x000001;
	paddleselect		= Next; Next += 0x000001;
	bankselect		= Next; Next += 0x000001;

	RamEnd			= Next;

	pAY8910Buffer[0]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[1]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[2]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);

	MemEnd			= Next;

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

	if (GetRoms()) return 1;

	DrvGfxDecode();
	DrvPaletteInit();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvVidRAM);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvVidRAM);
	ZetMapArea(0xe000, 0xe7ff, 2, DrvVidRAM);
	ZetMapArea(0xe800, 0xefff, 0, DrvSprRAM);
	ZetMapArea(0xe800, 0xefff, 1, DrvSprRAM);
	ZetMapArea(0xe800, 0xefff, 2, DrvSprRAM);
	ZetMapArea(0xf000, 0xffff, 2, DrvZ80ROM + 0xf000);
	if (arkanoid_bootleg_id == HEXA) {
		ZetSetWriteHandler(hexa_write);
	} else {
		ZetSetWriteHandler(arkanoid_write);
	}
	ZetSetReadHandler(arkanoid_read);
	ZetMemEnd();
	ZetClose();

	m67805_taito_init(DrvMcuROM, DrvMcuRAM, &arkanoid_m68705_interface);

	AY8910Init(0, 1500000, nBurnSoundRate, &ay8910_read_port_5, &ay8910_read_port_4, NULL, NULL);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	m67805_taito_exit();

	AY8910Exit(0);

	BurnFree (AllMem);

	arkanoid_bootleg_id = 0;

	return 0;
}

static void draw_background_layer()
{
	for (INT32 offs = 0; offs < 32 * 30; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		if (*flipscreen) {
			sx ^= 0xf8;
			sy ^= 0xf8;
		}

		sy -= 16;

		if (sy < 0 || sy >= nScreenHeight) continue;

		INT32 attr  = DrvVidRAM[offs * 2 + 0];
		INT32 code  = DrvVidRAM[offs * 2 + 1] | ((attr & 0x07) << 8) | (*gfxbank << 11);
		INT32 color = ((attr & 0xf8) >> 3) | (*palettebank << 5);

		if (*flipscreen) {
			Render8x8Tile_FlipY(pTransDraw, code, sx, sy, color, 3, 0, DrvGfxROM);
		} else {
			Render8x8Tile_FlipX(pTransDraw, code, sx, sy, color, 3, 0, DrvGfxROM);
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0; offs < 0x40; offs+=4)
	{
		INT32 sx    = DrvSprRAM[offs + 0];
		INT32 sy    = 248 - DrvSprRAM[offs + 1];
		INT32 attr  = DrvSprRAM[offs + 2];
		INT32 code  = DrvSprRAM[offs + 3] | ((attr & 0x03) << 8) | (*gfxbank << 10);
		INT32 color = ((attr & 0xf8) >> 3) | (*palettebank << 5);

		if (*flipscreen) {
			sx = 248 - sx;
			sy = 248 - sy;

			Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code * 2 + 0, sx, sy -  8, color, 3, 0, 0, DrvGfxROM);
			Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code * 2 + 1, sx, sy - 16, color, 3, 0, 0, DrvGfxROM);
		} else {
			Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code * 2 + 0, sx, sy - 24, color, 3, 0, 0, DrvGfxROM);
			Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code * 2 + 1, sx, sy - 16, color, 3, 0, 0, DrvGfxROM);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	draw_background_layer();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 2);

		if (arkanoid_bootleg_id != HEXA) DrvInputs[0] = 0x4f;

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		nAnalogAxis[0] -= DrvAxis[0];
		DrvInputs[2] = (~nAnalogAxis[0] >> 8) & 0xfe;

		nAnalogAxis[1] -= DrvAxis[1];
		DrvInputs[3] = (~nAnalogAxis[1] >> 8) & 0xfe;
	}

	INT32 nInterleave = 100;
	INT32 nCyclesTotal[2] = { 6000000 / 60, 3000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	ZetOpen(0);
	m6805Open(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nSegment = nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += ZetRun(nSegment);

		if (use_mcu) {
			nSegment = nCyclesTotal[1] / nInterleave;
			nCyclesDone[1] += m6805Run(nSegment);
		}
	}

	ZetRaiseIrq(0);

	m6805Close();
	ZetClose();

	if (pBurnSoundOut) {
		INT32 nSample;
		AY8910Update(0, &pAY8910Buffer[0], nBurnSoundLen);
		for (INT32 n = 0; n < nBurnSoundLen; n++) {
			nSample  = pAY8910Buffer[0][n];
			nSample += pAY8910Buffer[1][n];
			nSample += pAY8910Buffer[2][n];

			nSample /= 4;

			nSample = BURN_SND_CLIP(nSample);

			pBurnSoundOut[(n << 1) + 0] = nSample;
			pBurnSoundOut[(n << 1) + 1] = nSample;
		}
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		if (use_mcu) {
			m68705_taito_scan(nAction);
		}

		AY8910Scan(nAction, pnMin);

		SCAN_VAR(nAnalogAxis[0]);
		SCAN_VAR(nAnalogAxis[1]);
		SCAN_VAR(arkanoid_bootleg_cmd);
	}

	return 0;
}

static INT32 HexaScan(INT32 nAction, INT32 *pnMin)
{
	DrvScan(nAction, pnMin);

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		bankswitch(*bankselect);
		ZetClose();
	}

	return 0;
}


// Arkanoid (World)

static struct BurnRomInfo arkanoidRomDesc[] = {
	{ "a75-01-1.ic17",0x8000, 0x5bcda3b0, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "a75-11.ic16",  0x8000, 0xeafd7191, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-06.ic14",  0x0800, 0x515d77b6, 2 | BRF_ESS | BRF_PRG }, //  2 M68705 MCU

	{ "a75-03.ic64",  0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  3 Graphics
	{ "a75-04.ic63",  0x8000, 0x71fae199, 3 | BRF_GRA },	       //  4
	{ "a75-05.ic62",  0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  5

	{ "a75-07.ic24",  0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  6 Color Proms
	{ "a75-08.ic23",  0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  7
	{ "a75-09.ic22",  0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  8
	
	{ "arkanoid1_68705p3.ic14", 0x0800, 0x1b68e2d8, 0 | BRF_PRG | BRF_OPT },  //  9 Decapped roms
	{ "arkanoid_mcu.ic14",      0x0800, 0x4e44b50a, 0 | BRF_PRG | BRF_OPT },  // 10
	{ "arkanoid_68705p5.ic14",  0x0800, 0x0be83647, 0 | BRF_PRG | BRF_OPT },  // 11
};

STD_ROM_PICK(arkanoid)
STD_ROM_FN(arkanoid)

struct BurnDriver BurnDrvarkanoid = {
	"arkanoid", NULL, NULL, NULL, "1986",
	"Arkanoid (World)\0", NULL, "Taito Corporation Japan", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkanoidRomInfo, arkanoidRomName, NULL, NULL, DrvInputInfo, arkanoidDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (US)

static struct BurnRomInfo arknoiduRomDesc[] = {
	{ "a75-19.ic17",  0x8000, 0xd3ad37d7, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "a75-18.ic16",  0x8000, 0xcdc08301, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-20.ic14",  0x0800, 0xde518e47, 2 | BRF_ESS | BRF_PRG }, //  2 M68705 MCU

	{ "a75-03.ic64",  0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  3 Graphics
	{ "a75-04.ic63",  0x8000, 0x71fae199, 3 | BRF_GRA },	       //  4
	{ "a75-05.ic62",  0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  5

	{ "a75-07.ic24",  0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  6 Color Proms
	{ "a75-08.ic23",  0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  7
	{ "a75-09.ic22",  0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  8
};

STD_ROM_PICK(arknoidu)
STD_ROM_FN(arknoidu)

struct BurnDriver BurnDrvarknoidu = {
	"arkanoidu", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (US)\0", NULL, "Taito America Corporation (Romstar license)", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arknoiduRomInfo, arknoiduRomName, NULL, NULL, DrvInputInfo, arkanoidDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (US, older)

static struct BurnRomInfo arknoiuoRomDesc[] = {
	{ "a75-01-1.ic17",0x8000, 0x5bcda3b0, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "a75-10.ic16",  0x8000, 0xa1769e15, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-06.ic14",  0x0800, 0x515d77b6, 2 | BRF_ESS | BRF_PRG }, //  2 M68705 MCU

	{ "a75-03.ic64",  0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  3 Graphics
	{ "a75-04.ic63",  0x8000, 0x71fae199, 3 | BRF_GRA },	       //  4
	{ "a75-05.ic62",  0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  5

	{ "a75-07.ic24",  0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  6 Color Proms
	{ "a75-08.ic23",  0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  7
	{ "a75-09.ic23",  0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  8
};

STD_ROM_PICK(arknoiuo)
STD_ROM_FN(arknoiuo)

struct BurnDriver BurnDrvarknoiuo = {
	"arkanoiduo", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (US, older)\0", NULL, "Taito America Corporation (Romstar license)", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arknoiuoRomInfo, arknoiuoRomName, NULL, NULL, DrvInputInfo, arkanoidDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Tournament Arkanoid (US)

static struct BurnRomInfo arkatourRomDesc[] = {
	{ "a75-27.ic17",  0x8000, 0xe3b8faf5, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "a75-28.ic16",  0x8000, 0x326aca4d, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-32.ic14",  0x0800, 0xd3249559, 2 | BRF_ESS | BRF_PRG }, //  2 M68705 MCU

	{ "a75-29.ic64",  0x8000, 0x5ddea3cf, 3 | BRF_GRA },	       //  3 Graphics
	{ "a75-30.ic63",  0x8000, 0x5fcf2e85, 3 | BRF_GRA },	       //  4
	{ "a75-31.ic62",  0x8000, 0x7b76b192, 3 | BRF_GRA },	       //  5

	{ "a75-33.ic24",  0x0200, 0xb4bf3c81, 4 | BRF_GRA },	       //  6 Color Proms
	{ "a75-34.ic23",  0x0200, 0xde85a803, 4 | BRF_GRA },	       //  7
	{ "a75-35.ic22",  0x0200, 0x38acfd3b, 4 | BRF_GRA },	       //  8
};

STD_ROM_PICK(arkatour)
STD_ROM_FN(arkatour)

struct BurnDriver BurnDrvarkatour = {
	"arkatour", "arkanoid", NULL, NULL, "1987",
	"Tournament Arkanoid (US)\0", NULL, "Taito America Corporation (Romstar license)", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkatourRomInfo, arkatourRomName, NULL, NULL, DrvInputInfo, arkanoidDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (Japan)

static struct BurnRomInfo arknoidjRomDesc[] = {
	{ "a75-21.ic17",  0x8000, 0xbf0455fc, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "a75-22.ic16",  0x8000, 0x3a2688d3, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-23.ic14",  0x0800, 0x0a4abef6, 2 | BRF_ESS | BRF_PRG }, //  2 M68705 MCU

	{ "a75-03.ic64",  0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  3 Graphics
	{ "a75-04.ic63",  0x8000, 0x71fae199, 3 | BRF_GRA },	       //  4
	{ "a75-05.ic62",  0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  5

	{ "a75-07.ic24",  0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  6 Color Proms
	{ "a75-08.ic23",  0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  7
	{ "a75-09.ic22",  0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  8
};

STD_ROM_PICK(arknoidj)
STD_ROM_FN(arknoidj)

struct BurnDriver BurnDrvarknoidj = {
	"arkanoidj", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (Japan)\0", NULL, "Taito Corporation", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arknoidjRomInfo, arknoidjRomName, NULL, NULL, DrvInputInfo, arknoidjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (bootleg with MCU)

static struct BurnRomInfo arkmcublRomDesc[] = {
	{ "e1.6d",        0x8000, 0xdd4f2b72, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "e2.6f",        0x8000, 0xbbc33ceb, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-06.ic14",  0x0800, 0x515d77b6, 2 | BRF_ESS | BRF_PRG }, //  2 M68705 MCU

	{ "a75-03.rom",   0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  3 Graphics
	{ "a75-04.rom",   0x8000, 0x71fae199, 3 | BRF_GRA },	       //  4
	{ "a75-05.rom",   0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  5

	{ "a75-07.bpr",   0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  6 Color Proms
	{ "a75-08.bpr",   0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  7
	{ "a75-09.bpr",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  8

	{ "68705p3.6i",   0x0800, 0x389a8cfb, 0 | BRF_OPT | BRF_PRG }, //  9 Another MCU?
};

STD_ROM_PICK(arkmcubl)
STD_ROM_FN(arkmcubl)

struct BurnDriver BurnDrvarkmcubl = {
	"arkanoidjb", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (bootleg with MCU)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkmcublRomInfo, arkmcublRomName, NULL, NULL, DrvInputInfo, arknoidjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (bootleg with MCU, harder)

static struct BurnRomInfo ark1ballRomDesc[] = {
	{ "a-1.7d",       0x8000, 0xdd4f2b72, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "2palline.7f",  0x8000, 0xed6b62ab, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-06.ic14",  0x0800, 0x515d77b6, 2 | BRF_ESS | BRF_PRG }, //  2 M68705 MCU

	{ "a-3.3a",       0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  3 Graphics
	{ "a-4.3d",       0x8000, 0x71fae199, 3 | BRF_GRA },	       //  4
	{ "a-5.3f",       0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  5

	{ "a75-07.bpr",   0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  6 Color Proms
	{ "a75-08.bpr",   0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  7
	{ "a75-09.bpr",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  8
};

STD_ROM_PICK(ark1ball)
STD_ROM_FN(ark1ball)

struct BurnDriver BurnDrvark1ball = {
	"ark1ball", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (bootleg with MCU, harder)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, ark1ballRomInfo, ark1ballRomName, NULL, NULL, DrvInputInfo, ark1ballDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (Game Corporation bootleg, set 1)

static struct BurnRomInfo arkangcRomDesc[] = {
	{ "arkgc.1",      0x8000, 0xc54232e6, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "arkgc.2",      0x8000, 0x9f0d4754, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-03.rom",   0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  2 Graphics
	{ "a75-04.rom",   0x8000, 0x71fae199, 3 | BRF_GRA },	       //  3
	{ "a75-05.rom",   0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  4

	{ "a75-07.bpr",   0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  5 Color Proms
	{ "a75-08.bpr",   0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  6
	{ "a75-09.bpr",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(arkangc)
STD_ROM_FN(arkangc)

static INT32 arkangcInit()
{
	arkanoid_bootleg_id = ARKANGC;

	return DrvInit();
}

struct BurnDriver BurnDrvarkangc = {
	"arkangc", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (Game Corporation bootleg, set 1)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkangcRomInfo, arkangcRomName, NULL, NULL, DrvInputInfo, arkangcDIPInfo,
	arkangcInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (Game Corporation bootleg, set 2)

static struct BurnRomInfo arkangc2RomDesc[] = {
	{ "1.81",         0x8000, 0xbd6eb996, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "2.82",         0x8000, 0x29dbe452, 1 | BRF_ESS | BRF_PRG }, //  1
 
	{ "a75-03.rom",   0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  2 Graphics
	{ "a75-04.rom",   0x8000, 0x71fae199, 3 | BRF_GRA },	       //  3
	{ "a75-05.rom",   0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  4

	{ "a75-07.bpr",   0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  5 Color Proms
	{ "a75-08.bpr",   0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  6
	{ "a75-09.bpr",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(arkangc2)
STD_ROM_FN(arkangc2)

static INT32 arkangc2Init()
{
	arkanoid_bootleg_id = ARKANGC2;

	return DrvInit();
}

struct BurnDriver BurnDrvarkangc2 = {
	"arkangc2", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (Game Corporation bootleg, set 2)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkangc2RomInfo, arkangc2RomName, NULL, NULL, DrvInputInfo, arkangc2DIPInfo,
	arkangc2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Block (Game Corporation bootleg, set 1)

static struct BurnRomInfo arkblockRomDesc[] = {
	{ "ark-6.bin",    0x8000, 0x0be015de, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "arkgc.2",      0x8000, 0x9f0d4754, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-03.rom",   0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  2 Graphics
	{ "a75-04.rom",   0x8000, 0x71fae199, 3 | BRF_GRA },	       //  3
	{ "a75-05.rom",   0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  4

	{ "a75-07.bpr",   0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  5 Color Proms
	{ "a75-08.bpr",   0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  6
	{ "a75-09.bpr",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(arkblock)
STD_ROM_FN(arkblock)

static INT32 arkblockInit()
{
	arkanoid_bootleg_id = ARKBLOCK;

	return DrvInit();
}

struct BurnDriver BurnDrvarkblock = {
	"arkblock", "arkanoid", NULL, NULL, "1986",
	"Block (Game Corporation bootleg, set 1)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkblockRomInfo, arkblockRomName, NULL, NULL, DrvInputInfo, arkangcDIPInfo,
	arkblockInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Block (Game Corporation bootleg, set 2)

static struct BurnRomInfo arkbloc2RomDesc[] = {
	{ "block01.bin",  0x8000, 0x5be667e1, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "block02.bin",  0x8000, 0x4f883ef1, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-03.rom",   0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  2 Graphics
	{ "a75-04.rom",   0x8000, 0x71fae199, 3 | BRF_GRA },	       //  3
	{ "a75-05.rom",   0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  4

	{ "a75-07.bpr",   0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  5 Color Proms
	{ "a75-08.bpr",   0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  6
	{ "a75-09.bpr",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(arkbloc2)
STD_ROM_FN(arkbloc2)

static INT32 arkbloc2Init()
{
	arkanoid_bootleg_id = ARKBLOC2;

	return DrvInit();
}

struct BurnDriver BurnDrvarkbloc2 = {
	"arkbloc2", "arkanoid", NULL, NULL, "1986",
	"Block (Game Corporation bootleg, set 2)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkbloc2RomInfo, arkbloc2RomName, NULL, NULL, DrvInputInfo, arkangcDIPInfo,
	arkbloc2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (bootleg on Block hardware)

static struct BurnRomInfo arkgcblRomDesc[] = {
	{ "16.6e",        0x8000, 0xb0f73900, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "17.6f",        0x8000, 0x9827f297, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-03.rom",   0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  2 Graphics
	{ "a75-04.rom",   0x8000, 0x71fae199, 3 | BRF_GRA },	       //  3
	{ "a75-05.rom",   0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  4
	
	{ "82s129.5k",    0x0100, 0xfa70b64d, 4 | BRF_GRA },	       //  5 Color Proms
	{ "82s129.5jk",   0x0100, 0xcca69884, 4 | BRF_GRA },	       //  6
	{ "82s129.5l",    0x0100, 0x3e4d2bf5, 4 | BRF_GRA },	       //  7
	{ "82s129.5kl",   0x0100, 0x085d625a, 4 | BRF_GRA },	       //  8
	{ "82s129.5mn",   0x0100, 0x0fe0b108, 4 | BRF_GRA },	       //  9
	{ "63s141.5m",    0x0100, 0x5553f675, 4 | BRF_GRA },	       //  10
	
	{ "pal16r8.5f",   0x0104, 0x36471917, 0 | BRF_OPT | BRF_GRA }, //  11 PAL
};

STD_ROM_PICK(arkgcbl)
STD_ROM_FN(arkgcbl)

static INT32 arkgcblInit()
{
	arkanoid_bootleg_id = ARKGCBL;

	return DrvInit();
}

struct BurnDriver BurnDrvarkgcbl = {
	"arkgcbl", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (bootleg on Block hardware)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkgcblRomInfo, arkgcblRomName, NULL, NULL, DrvInputInfo, arkgcblDIPInfo,
	arkgcblInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Paddle 2 (bootleg on Block hardware)

static struct BurnRomInfo paddle2RomDesc[] = {
	{ "paddle2.16",   0x8000, 0xa286333c, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "paddle2.17",   0x8000, 0x04c2acb5, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "a75-03.rom",   0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  2 Graphics
	{ "a75-04.rom",   0x8000, 0x71fae199, 3 | BRF_GRA },	       //  3
	{ "a75-05.rom",   0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  4

	{ "a75-07.bpr",   0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  5 Color Proms
	{ "a75-08.bpr",   0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  6
	{ "a75-09.bpr",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(paddle2)
STD_ROM_FN(paddle2)

static INT32 paddle2Init()
{
	arkanoid_bootleg_id = PADDLE2;

	return DrvInit();
}

struct BurnDriver BurnDrvpaddle2 = {
	"paddle2", "arkanoid", NULL, NULL, "1986",
	"Paddle 2 (bootleg on Block hardware)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, paddle2RomInfo, paddle2RomName, NULL, NULL, DrvInputInfo, paddle2DIPInfo,
	paddle2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (Tayto bootleg)

static struct BurnRomInfo arkataytRomDesc[] = {
	{ "ic81-v.3f",    0x8000, 0x154e2c6f, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "ic82-w.5f",    0x8000, 0x4fa8cefa, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "1-ic33.2c",    0x8000, 0x038b74ba, 3 | BRF_GRA },	       //  2 Graphics
	{ "2-ic34.3c",    0x8000, 0x71fae199, 3 | BRF_GRA },	       //  3
	{ "3-ic35.5c",    0x8000, 0xc76374e2, 3 | BRF_GRA },	       //  4

	{ "ic73.11e",     0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  5 Color Proms
	{ "ic74.12e",     0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  6
	{ "ic75.13e",     0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(arkatayt)
STD_ROM_FN(arkatayt)

struct BurnDriver BurnDrvarkatayt = {
	"arkatayt", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (Tayto bootleg)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arkataytRomInfo, arkataytRomName, NULL, NULL, DrvInputInfo, arknoidjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Arkanoid (Tayto bootleg, harder)

static struct BurnRomInfo arktayt2RomDesc[] = {
	{ "ic81.3f",    0x8000, 0x6e0a2b6f, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "ic82.5f",    0x8000, 0x5a97dd56, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "1-ic33.2c",  0x8000, 0x038b74ba, 3 | BRF_GRA },	     //  2 Graphics
	{ "2-ic34.3c",  0x8000, 0x71fae199, 3 | BRF_GRA },	     //  3
	{ "3-ic35.5c",  0x8000, 0xc76374e2, 3 | BRF_GRA },	     //  4

	{ "ic73.11e",   0x0200, 0x0af8b289, 4 | BRF_GRA },	     //  5 Color Proms
	{ "ic74.12e",   0x0200, 0xabb002fb, 4 | BRF_GRA },	     //  6
	{ "ic75.13e",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	     //  7
};

STD_ROM_PICK(arktayt2)
STD_ROM_FN(arktayt2)

struct BurnDriver BurnDrvarktayt2 = {
	"arktayt2", "arkanoid", NULL, NULL, "1986",
	"Arkanoid (Tayto bootleg, harder)\0", NULL, "bootleg", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, arktayt2RomInfo, arktayt2RomName, NULL, NULL, DrvInputInfo, arktayt2DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 256, 3, 4
};


// Tetris (D.R. Korea)

static struct BurnRomInfo tetrsarkRomDesc[] = {
	{ "ic17.1",       0x8000, 0x1a505eda, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "ic16.2",       0x8000, 0x157bc4df, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "ic64.3",       0x8000, 0xc3e9b290, 3 | BRF_GRA },	       //  2 Graphics
	{ "ic63.4",       0x8000, 0xde9a368f, 3 | BRF_GRA },	       //  3
	{ "ic62.5",       0x8000, 0xc8e80a00, 3 | BRF_GRA },	       //  4

	{ "a75-07.bpr",   0x0200, 0x0af8b289, 4 | BRF_GRA },	       //  5 Color Proms
	{ "a75-08.bpr",   0x0200, 0xabb002fb, 4 | BRF_GRA },	       //  6
	{ "a75-09.bpr",   0x0200, 0xa7c6c277, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(tetrsark)
STD_ROM_FN(tetrsark)

static INT32 tetrsarkInit()
{
	arkanoid_bootleg_id = TETRSARK;

	INT32 nRet = DrvInit();

	if (nRet == 0) {
		for (INT32 i = 0; i < 0x8000; i++) {
			DrvZ80ROM[i] ^= 0x94;
		}
	}

	return nRet;
}

struct BurnDriver BurnDrvtetrsark = {
	"tetrsark", NULL, NULL, NULL, "198?",
	"Tetris (D.R. Korea)\0", "Wrong colors", "D.R. Korea", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_BREAKOUT, 0,
	NULL, tetrsarkRomInfo, tetrsarkRomName, NULL, NULL, tetrsarkInputInfo, tetrsarkDIPInfo,
	tetrsarkInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	256, 224, 4, 3
};


// Hexa

static struct BurnRomInfo hexaRomDesc[] = {
	{ "hexa.20",	0x8000, 0x98b00586, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "hexa.21",	0x8000, 0x3d5d006c, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "hexa.17",	0x8000, 0xf6911dd6, 3 | BRF_GRA },           //  2 Graphics
	{ "hexa.18",	0x8000, 0x6e3d95d2, 3 | BRF_GRA },           //  3
	{ "hexa.19",	0x8000, 0xffe97a31, 3 | BRF_GRA },           //  4

	{ "hexa.001",	0x0100, 0x88a055b4, 4 | BRF_GRA },           //  5 Color Proms
	{ "hexa.003",	0x0100, 0x3e9d4932, 4 | BRF_GRA },           //  6
	{ "hexa.002",	0x0100, 0xff15366c, 4 | BRF_GRA },           //  7
};

STD_ROM_PICK(hexa)
STD_ROM_FN(hexa)

static INT32 HexaInit()
{
	arkanoid_bootleg_id = HEXA;

	return DrvInit();
}

struct BurnDriver BurnDrvHexa = {
	"hexa", NULL, NULL, NULL, "199?",
	"Hexa\0", NULL, "D. R. Korea", "Arkanoid",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_TAITO_MISC, GBF_PUZZLE, 0,
	NULL, hexaRomInfo, hexaRomName, NULL, NULL, HexaInputInfo, HexaDIPInfo,
	HexaInit, DrvExit, DrvFrame, DrvDraw, HexaScan, &DrvRecalc, 0x100,
	256, 224, 4, 3
};

