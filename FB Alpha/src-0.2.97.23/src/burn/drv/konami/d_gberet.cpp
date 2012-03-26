// FB Alpha Green Beret driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "zet.h"
#include "sn76496.h"

static UINT8 *Mem, *MemEnd, *Rom, *Gfx0, *Gfx1, *Prom;
static UINT32 *Palette, *DrvPalette;
static UINT8 DrvRecalcPal;

static UINT8 DrvJoy1[8], DrvJoy2[8], DrvJoy3[8], DrvDips[4], DrvReset;

static UINT8 nmi_enable, irq_enable;
static UINT16 gberetb_scroll;
static UINT8 flipscreen;
static UINT8 gberet_spritebank;
static INT32 mrgoemon_bank;

static INT32 game_type = 0; // 0 gberet / rushatck, 1 gberetb, 2 mrgoemon

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy3 + 0,	"p1 coin"   },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy3 + 1,	"p2 coin"   },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy3 + 3,	"p1 start"  },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 start"  },

	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, "p1 left"   },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, "p1 right"  },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 2, "p1 up"     },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 3, "p1 down"   },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1" },
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	"p1 fire 2" },

	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 0, "p2 left"   },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 1, "p2 right"  },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy2 + 2, "p2 up"     },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy2 + 3, "p2 down"   },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 1" },
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p2 fire 2" },

	{"Service"      , BIT_DIGITAL  , DrvJoy3 + 2,	"service"   },
 
	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"     },
	{"Dip 1"        , BIT_DIPSWITCH, DrvDips + 0, "dip"       },
	{"Dip 2"        , BIT_DIPSWITCH, DrvDips + 1, "dip"       },
	{"Dip 3"        , BIT_DIPSWITCH, DrvDips + 2, "dip"       },

};

STDINPUTINFO(Drv)

static struct BurnInputInfo gberetbInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy3 + 7,	"p1 coin"   },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy3 + 6,	"p2 coin"   },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy3 + 5,	"p1 start"  },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 start"  },

	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, "p1 left"   },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, "p1 right"  },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 2, "p1 up"     },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 3, "p1 down"   },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1" },
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	"p1 fire 2" },

	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"     },
	{"Dip 1"        , BIT_DIPSWITCH, DrvDips + 3, "dip"       },
	{"Dip 2"        , BIT_DIPSWITCH, DrvDips + 2, "dip"       },
};

STDINPUTINFO(gberetb)

static struct BurnDIPInfo gberetDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL },

	{0   , 0xfe, 0   , 16   , "Coin A" },
	{0x12, 0x01, 0x0f, 0x02,  "4 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x05,  "3 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x08,  "2 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x04,  "3 Coins 2 Credits" },
	{0x12, 0x01, 0x0f, 0x01,  "4 Coins 3 Credits" },
	{0x12, 0x01, 0x0f, 0x0f,  "1 Coin 1 Credit"   },
	{0x12, 0x01, 0x0f, 0x03,  "3 Coins 4 Credits" },
	{0x12, 0x01, 0x0f, 0x07,  "2 Coins 3 Credits" },
	{0x12, 0x01, 0x0f, 0x0e,  "1 Coin 2 Credits"  },
	{0x12, 0x01, 0x0f, 0x06,  "2 Coins 5 Credits" },
	{0x12, 0x01, 0x0f, 0x0d,  "1 Coin 3 Credits"  },
	{0x12, 0x01, 0x0f, 0x0c,  "1 Coin 4 Credits"  },
	{0x12, 0x01, 0x0f, 0x0b,  "1 Coin 5 Credits"  },
	{0x12, 0x01, 0x0f, 0x0a,  "1 Coin 6 Credits"  },
	{0x12, 0x01, 0x0f, 0x09,  "1 Coin 7 Credits"  },
	{0x12, 0x01, 0x0f, 0x00,  "Free Play"   	    },

	{0   , 0xfe, 0   , 15   , "Coin B" },
	{0x12, 0x82, 0xf0, 0x20,  "4 Coins 1 Credit"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x50,  "3 Coins 1 Credit"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x80,  "2 Coins 1 Credit"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x40,  "3 Coins 2 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x10,  "4 Coins 3 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xf0,  "1 Coin 1 Credit"   },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x30,  "3 Coins 4 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x70,  "2 Coins 3 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xe0,  "1 Coin 2 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x60,  "2 Coins 5 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xd0,  "1 Coin 3 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xc0,  "1 Coin 4 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xb0,  "1 Coin 5 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xa0,  "1 Coin 6 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x90,  "1 Coin 7 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },

	{0x13, 0xff, 0xff, 0x4a, NULL },

	{0   , 0xfe, 0   , 4   , "Lives" },
	{0x13, 0x01, 0x03, 0x03, "2" },
	{0x13, 0x01, 0x03, 0x02, "3" },
	{0x13, 0x01, 0x03, 0x01, "5" },
	{0x13, 0x01, 0x03, 0x00, "7" },

	{0   , 0xfe, 0   , 2   , "Cabinet" },
	{0x13, 0x01, 0x04, 0x00, "Upright"  },
	{0x13, 0x01, 0x04, 0x04, "Cocktail" },

	{0   , 0xfe, 0   , 4   , "Bonus Life"	},
	{0x13, 0x01, 0x18, 0x18, "30K 70K 70K+"   },
	{0x13, 0x01, 0x18, 0x10, "40K 80K 80K+"	  },
	{0x13, 0x01, 0x18, 0x08, "50K 100K 100K+" },
	{0x13, 0x01, 0x18, 0x00, "50K 200K 200K+"	},

	{0   , 0xfe, 0   , 4   , "Difficulty" },
	{0x13, 0x01, 0x60, 0x60, "Easy"       		},
	{0x13, 0x01, 0x60, 0x40, "Normal"       	},
	{0x13, 0x01, 0x60, 0x20, "Difficult"      },
	{0x13, 0x01, 0x60, 0x00, "Very Difficult" },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x13, 0x01, 0x80, 0x80, "Off" },
	{0x13, 0x01, 0x80, 0x00, "On"  },

	{0x14, 0xff, 0xff, 0x0f, NULL },

	{0   , 0xfe, 0   , 2   , "Flip Screen" },
	{0x14, 0x01, 0x01, 0x01, "Off" },
	{0x14, 0x01, 0x01, 0x00, "On"  },

	{0   , 0xfe, 0   , 2   , "Upright Controls" },
	{0x14, 0x01, 0x02, 0x02, "Single" },
	{0x14, 0x01, 0x02, 0x00, "Dual"   },
};

STDDIPINFO(gberet)

static struct BurnDIPInfo gberetbDIPList[]=
{
	{0x0b, 0xff, 0xff, 0xff, NULL },

	{0   , 0xfe, 0   , 16   , "Coin A" },
	{0x0b, 0x01, 0x0f, 0x02,  "4 Coins 1 Credit"  },
	{0x0b, 0x01, 0x0f, 0x05,  "3 Coins 1 Credit"  },
	{0x0b, 0x01, 0x0f, 0x08,  "2 Coins 1 Credit"  },
	{0x0b, 0x01, 0x0f, 0x04,  "3 Coins 2 Credits" },
	{0x0b, 0x01, 0x0f, 0x01,  "4 Coins 3 Credits" },
	{0x0b, 0x01, 0x0f, 0x0f,  "1 Coin 1 Credit"   },
	{0x0b, 0x01, 0x0f, 0x03,  "3 Coins 4 Credits" },
	{0x0b, 0x01, 0x0f, 0x07,  "2 Coins 3 Credits" },
	{0x0b, 0x01, 0x0f, 0x0e,  "1 Coin 2 Credits"  },
	{0x0b, 0x01, 0x0f, 0x06,  "2 Coins 5 Credits" },
	{0x0b, 0x01, 0x0f, 0x0d,  "1 Coin 3 Credits"  },
	{0x0b, 0x01, 0x0f, 0x0c,  "1 Coin 4 Credits"  },
	{0x0b, 0x01, 0x0f, 0x0b,  "1 Coin 5 Credits"  },
	{0x0b, 0x01, 0x0f, 0x0a,  "1 Coin 6 Credits"  },
	{0x0b, 0x01, 0x0f, 0x09,  "1 Coin 7 Credits"  },
	{0x0b, 0x01, 0x0f, 0x00,  "Free Play"   	    },

	{0   , 0xfe, 0   , 15   , "Coin B" },
	{0x0b, 0x82, 0xf0, 0x20,  "4 Coins 1 Credit"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0x50,  "3 Coins 1 Credit"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0x80,  "2 Coins 1 Credit"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0x40,  "3 Coins 2 Credits" },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0x10,  "4 Coins 3 Credits" },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0xf0,  "1 Coin 1 Credit"   },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0x30,  "3 Coins 4 Credits" },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0x70,  "2 Coins 3 Credits" },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0xe0,  "1 Coin 2 Credits"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0x60,  "2 Coins 5 Credits" },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0xd0,  "1 Coin 3 Credits"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0xc0,  "1 Coin 4 Credits"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0xb0,  "1 Coin 5 Credits"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0xa0,  "1 Coin 6 Credits"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },
	{0x0b, 0x82, 0xf0, 0x90,  "1 Coin 7 Credits"  },
	{0x0b, 0x00, 0x0f, 0x00,  NULL },

	{0x0c, 0xff, 0xff, 0x4a, NULL },

	{0   , 0xfe, 0   , 4   , "Lives" },
	{0x0c, 0x01, 0x03, 0x03, "2" },
	{0x0c, 0x01, 0x03, 0x02, "3" },
	{0x0c, 0x01, 0x03, 0x01, "5" },
	{0x0c, 0x01, 0x03, 0x00, "7" },

	{0   , 0xfe, 0   , 2   , "Cabinet" },
	{0x0c, 0x01, 0x04, 0x00, "Upright"  },
	{0x0c, 0x01, 0x04, 0x04, "Cocktail" },

	{0   , 0xfe, 0   , 4   , "Bonus Life"	},
	{0x0c, 0x01, 0x18, 0x18, "30K 70K 70K+"   },
	{0x0c, 0x01, 0x18, 0x10, "40K 80K 80K+"	  },
	{0x0c, 0x01, 0x18, 0x08, "50K 100K 100K+" },
	{0x0c, 0x01, 0x18, 0x00, "50K 200K 200K+"	},

	{0   , 0xfe, 0   , 4   , "Difficulty" },
	{0x0c, 0x01, 0x60, 0x60, "Easy"       	  },
	{0x0c, 0x01, 0x60, 0x40, "Normal"       	},
	{0x0c, 0x01, 0x60, 0x20, "Difficult"      },
	{0x0c, 0x01, 0x60, 0x00, "Very Difficult" },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x0c, 0x01, 0x80, 0x80, "Off" },
	{0x0c, 0x01, 0x80, 0x00, "On"  },
};

STDDIPINFO(gberetb)

static struct BurnDIPInfo mrgoemonDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL },

	{0   , 0xfe, 0   , 16   , "Coin A" },
	{0x12, 0x01, 0x0f, 0x02,  "4 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x05,  "3 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x08,  "2 Coins 1 Credit"  },
	{0x12, 0x01, 0x0f, 0x04,  "3 Coins 2 Credits" },
	{0x12, 0x01, 0x0f, 0x01,  "4 Coins 3 Credits" },
	{0x12, 0x01, 0x0f, 0x0f,  "1 Coin 1 Credit"   },
	{0x12, 0x01, 0x0f, 0x03,  "3 Coins 4 Credits" },
	{0x12, 0x01, 0x0f, 0x07,  "2 Coins 3 Credits" },
	{0x12, 0x01, 0x0f, 0x0e,  "1 Coin 2 Credits"  },
	{0x12, 0x01, 0x0f, 0x06,  "2 Coins 5 Credits" },
	{0x12, 0x01, 0x0f, 0x0d,  "1 Coin 3 Credits"  },
	{0x12, 0x01, 0x0f, 0x0c,  "1 Coin 4 Credits"  },
	{0x12, 0x01, 0x0f, 0x0b,  "1 Coin 5 Credits"  },
	{0x12, 0x01, 0x0f, 0x0a,  "1 Coin 6 Credits"  },
	{0x12, 0x01, 0x0f, 0x09,  "1 Coin 7 Credits"  },
	{0x12, 0x01, 0x0f, 0x00,  "Free Play"   	    },

	{0   , 0xfe, 0   , 15   , "Coin B" },
	{0x12, 0x82, 0xf0, 0x20,  "4 Coins 1 Credit"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x50,  "3 Coins 1 Credit"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x80,  "2 Coins 1 Credit"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x40,  "3 Coins 2 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x10,  "4 Coins 3 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xf0,  "1 Coin 1 Credit"   },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x30,  "3 Coins 4 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x70,  "2 Coins 3 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xe0,  "1 Coin 2 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x60,  "2 Coins 5 Credits" },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xd0,  "1 Coin 3 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xc0,  "1 Coin 4 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xb0,  "1 Coin 5 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0xa0,  "1 Coin 6 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },
	{0x12, 0x82, 0xf0, 0x90,  "1 Coin 7 Credits"  },
	{0x12, 0x00, 0x0f, 0x00,  NULL },

	{0x13, 0xff, 0xff, 0x4a, NULL },

	{0   , 0xfe, 0   , 4   , "Lives" },
	{0x13, 0x01, 0x03, 0x03, "2" },
	{0x13, 0x01, 0x03, 0x02, "3" },
	{0x13, 0x01, 0x03, 0x01, "5" },
	{0x13, 0x01, 0x03, 0x00, "7" },

	{0   , 0xfe, 0   , 2   , "Cabinet" },
	{0x13, 0x01, 0x04, 0x00, "Upright"  },
	{0x13, 0x01, 0x04, 0x04, "Cocktail" },

	{0   , 0xfe, 0   , 4   , "Bonus Life"	},
	{0x13, 0x01, 0x18, 0x18, "20K 60K+" },
	{0x13, 0x01, 0x18, 0x10, "30K 70K+"	},
	{0x13, 0x01, 0x18, 0x08, "40K 80K+" },
	{0x13, 0x01, 0x18, 0x00, "50K 90K+"	},

	{0   , 0xfe, 0   , 4   , "Difficulty" },
	{0x13, 0x01, 0x60, 0x60, "Easy"       	  },
	{0x13, 0x01, 0x60, 0x40, "Normal"       	},
	{0x13, 0x01, 0x60, 0x20, "Difficult"      },
	{0x13, 0x01, 0x60, 0x00, "Very Difficult" },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x13, 0x01, 0x80, 0x80, "Off" },
	{0x13, 0x01, 0x80, 0x00, "On"  },

	{0x14, 0xff, 0xff, 0x0f, NULL },

	{0   , 0xfe, 0   , 2   , "Flip Screen" },
	{0x14, 0x01, 0x01, 0x01, "Off" },
	{0x14, 0x01, 0x01, 0x00, "On"  },

	{0   , 0xfe, 0   , 2   , "Upright Controls" },
	{0x14, 0x01, 0x02, 0x02, "Single" },
	{0x14, 0x01, 0x02, 0x00, "Dual"   },

	{0   , 0xfe, 0   , 2   , "Service Mode" },
	{0x14, 0x01, 0x04, 0x04, "Off" },
	{0x14, 0x01, 0x04, 0x00, "On"  },
};

STDDIPINFO(mrgoemon)

static void mrgoemon_bankswitch(INT32 nBank)
{
	mrgoemon_bank = nBank;
	ZetMapArea(0xf800, 0xffff, 0, Rom + 0x10000 + mrgoemon_bank);
	ZetMapArea(0xf800, 0xffff, 2, Rom + 0x10000 + mrgoemon_bank);
}

void __fastcall gberet_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe040:
		case 0xe041:
		case 0xe042:
		return;

		case 0xe043:
			gberet_spritebank = data;
		return;

		case 0xe044:
		{
			nmi_enable = data & 0x01;
			irq_enable = data & ((game_type & 2) ? 0x02 : 0x04);

			flipscreen = data & 0x08;
		}
		return;

		case 0xf000:
		{
			// Coin counters

			if (game_type == 2) {
				mrgoemon_bankswitch((data >> 5) << 11);
			}
		}
		return;

		case 0xf400:
			SN76496Write(0, data);
		return;

		case 0xf600:	// watchdog
		return;

		case 0xf900:
		case 0xf901:
			gberetb_scroll = (((address & 1) << 8) | data) + 0x38;
		return;
	}

	if (address >= 0xe000 && address <= 0xe03f) {
		Rom[address] = data;
		return;
	}

	if (game_type != 1) return;

	if (address >= 0xe900 && address <= 0xe9ff) {
		Rom[address] = data;
		return;
	}
}

UINT8 __fastcall gberet_read(UINT16 address)
{
	UINT8 nRet = 0xff;

	switch (address)
	{
		case 0xf200:
			return DrvDips[1];

		case 0xf400:
			return DrvDips[2];

		case 0xf600:
			return DrvDips[0];

		case 0xf601:
		{
			if (game_type & 1) return DrvDips[3];

			for (INT32 i = 0; i < 8; i++) nRet ^= DrvJoy2[i] << i;

			return nRet;
		}


		case 0xf602:
		{
			for (INT32 i = 0; i < 8; i++) nRet ^= DrvJoy1[i] << i;

			return nRet;
		}


		case 0xf603:
		{
			for (INT32 i = 0; i < 8; i++) nRet ^= DrvJoy3[i] << i;

			return nRet;
		}
	}

	if (address >= 0xe000 && address <= 0xe03f) {
		return Rom[address];
	}

	if (game_type != 1) return 0;

	if (address >= 0xe900 && address <= 0xe9ff) {
		return Rom[address];
	}

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	flipscreen = 0;
	nmi_enable = 0;
	irq_enable = 0;
	mrgoemon_bank = 0;
	gberetb_scroll = 0;
	gberet_spritebank = 0;

	ZetOpen(0);
	ZetReset();

	if (game_type & 2) {
		mrgoemon_bankswitch(0);
	}

	ZetClose();

	return 0;
}


static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Rom            = Next; Next += 0x14000;

	Gfx0           = Next; Next += 0x08000;
	Gfx1           = Next; Next += 0x20000;

	Prom           = Next; Next += 0x00220;

	Palette	       = (UINT32*)Next; Next += 0x00200 * sizeof(UINT32);
	DrvPalette     = (UINT32*)Next; Next += 0x00200 * sizeof(UINT32);

	MemEnd         = Next;

	return 0;
}

static void DrvCreatePalette()
{
	UINT32 tmp[0x20];

	for (INT32 i = 0; i < 0x20; i++)
	{
		INT32 bit0, bit1, bit2;
		INT32 r, g, b;

		bit0 = (Prom[i] >> 0) & 0x01;
		bit1 = (Prom[i] >> 1) & 0x01;
		bit2 = (Prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (Prom[i] >> 3) & 0x01;
		bit1 = (Prom[i] >> 4) & 0x01;
		bit2 = (Prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = (Prom[i] >> 6) & 0x01;
		bit2 = (Prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		tmp[i] = (r << 16) | (g << 8) | b;
	}

	Prom += 0x20;

	for (INT32 i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry;

		ctabentry = (Prom[0x000 + i] & 0x0f) | 0x10;
		Palette[0x000 + i] = tmp[ctabentry];

		ctabentry = (Prom[0x100 + i] & 0x0f);
		Palette[0x100 + i] = tmp[ctabentry];
	}
}

static void Graphics_Decode(INT32 *CharPlanes, INT32 *CharXOffs, INT32 *CharYOffs, INT32 *SprPlanes, INT32 *SprXOffs, INT32 *SprYOffs, INT32 SprMod)
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x10000);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, Gfx0, 0x4000);

	GfxDecode(0x200, 4,  8,  8, CharPlanes, CharXOffs, CharYOffs, 0x100, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0x10000);

	GfxDecode(0x200, 4, 16, 16, SprPlanes, SprXOffs, SprYOffs, SprMod, tmp, Gfx1);

	BurnFree (tmp);
}

static void DrvGfxDecode()
{
	static INT32 Planes[4] = { 0, 1, 2, 3 };
	static INT32 XOffs[16] = { 0, 4, 8, 12, 16, 20, 24, 28, 256, 260, 264, 268, 272, 276, 280, 284 };
	static INT32 YOffs[16] = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };

	Graphics_Decode(Planes, XOffs, YOffs, Planes, XOffs, YOffs, 0x400);
}

static void BootGfxDecode()
{
	static INT32 CharPlanes[4] = { 0, 1, 2, 3 };
	static INT32 CharXOffs[8]  = { 24, 28, 0, 4, 8, 12, 16, 20 };
	static INT32 CharYOffs[8]  = { 0, 32, 64, 96, 128, 160, 192, 224 };
	static INT32 SpriPlanes[4] = { 0, 0x20000, 0x40000, 0x60000 };
	static INT32 SpriXOffs[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
	static INT32 SpriYOffs[16] = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

	Graphics_Decode(CharPlanes, CharXOffs, CharYOffs, SpriPlanes, SpriXOffs, SpriYOffs, 0x100);
}

static INT32 GetRoms()
{
	char* pRomName;
	struct BurnRomInfo ri;
	UINT8 *Load0 = Rom;
	UINT8 *Load1 = Gfx0;
	UINT8 *Load2 = Gfx1;
	UINT8 *Load3 = Prom;

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & 7) == 1) {
			if (BurnLoadRom(Load0, i, 1)) return 1;
			Load0 += ri.nLen;

			continue;
		}

		if ((ri.nType & 7) == 2) {
			if (BurnLoadRom(Load1, i, 1)) return 1;
			Load1 += ri.nLen;

			continue;
		}

		if ((ri.nType & 7) == 3) {
			if (BurnLoadRom(Load2, i, 1)) return 1;
			Load2 += ri.nLen;

			continue;
		}

		if ((ri.nType & 7) == 4) {
			if (BurnLoadRom(Load3, i, 1)) return 1;
			Load3 += ri.nLen;

			continue;
		}
	}

	if (game_type == 2) {
		memcpy (Rom + 0x10000, Rom + 0x0c000, 0x04000);
		memset (Rom + 0x0c000, 0, 0x04000);
	} 

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	if (GetRoms()) return 1;

	if (game_type == 1) {
		BootGfxDecode();
	} else {
		DrvGfxDecode();
	}
	DrvCreatePalette();

	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(gberet_read);
	ZetSetWriteHandler(gberet_write);
	ZetMapArea(0x0000, 0xbfff, 0, Rom + 0x0000);
	ZetMapArea(0x0000, 0xbfff, 2, Rom + 0x0000);
	ZetMapArea(0xc000, 0xcfff, 0, Rom + 0xc000);
	ZetMapArea(0xc000, 0xcfff, 1, Rom + 0xc000);
	ZetMapArea(0xd000, 0xd0ff, 0, Rom + 0xd000);
	ZetMapArea(0xd000, 0xd0ff, 1, Rom + 0xd000);
	ZetMapArea(0xd100, 0xd1ff, 0, Rom + 0xd100);
	ZetMapArea(0xd100, 0xd1ff, 1, Rom + 0xd100);
	ZetMapArea(0xd200, 0xdfff, 0, Rom + 0xd200);
	ZetMapArea(0xd200, 0xdfff, 1, Rom + 0xd200);
	ZetMapArea(0xd200, 0xdfff, 2, Rom + 0xd200);
	if (game_type == 2) {
		ZetMapArea(0xe800, 0xe8ff, 0, Rom + 0xe800);
		ZetMapArea(0xe800, 0xe8ff, 1, Rom + 0xe800);
		ZetMapArea(0xe800, 0xe8ff, 2, Rom + 0xe800);
	}
	ZetMemEnd();
	ZetClose();

	SN76489Init(0, 18432000 / 12, 0);

	DrvDoReset();

	if (game_type == 0) {
		BurnSetRefreshRate(30.0);
	}

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	SN76496Exit();

	BurnFree (Rom);

	game_type = 0;


	return 0;
}

static inline void put_pixel(INT32 x, INT32 y, INT32 src, INT32 color)
{
	INT32 pxl = color | src;

	if (y < 0 || x < 0 || x >= nScreenWidth || y >= nScreenHeight || !Prom[pxl]) return;

	pTransDraw[(y * nScreenWidth) + x] = pxl;
}

static void gberet_draw_16x16(INT32 num, INT32 sx, INT32 sy, INT32 color, INT32 flipx, INT32 flipy)
{
	color |= 0x100;
	UINT8 *src = Gfx1 + (num << 8);

	if (flipy) {
		if (flipx) {
			for (INT32 y = sy + 15; y >= sy; y--) {
				for (INT32 x = sx + 15; x >= sx; x--, src++) {
					put_pixel(x, y, *src, color);
				}
			}
		} else {
			for (INT32 y = sy + 15; y >= sy; y--) {
				for (INT32 x = sx; x < sx + 16; x++, src++) {
					put_pixel(x, y, *src, color);
				}
			}
		}
	} else {
		if (flipx) {
			for (INT32 y = sy; y < sy + 16; y++) {
				for (INT32 x = sx + 15; x >= sx; x--, src++) {
					put_pixel(x, y, *src, color);
				}
			}
		} else {
			for (INT32 y = sy; y < sy + 16; y++) {
				for (INT32 x = sx; x < sx + 16; x++, src++) {
					put_pixel(x, y, *src, color);
				}
			}
		}
	}
}

static void gberet_draw_sprites()
{
	UINT8 *sr = Rom + 0xd000 + ((~gberet_spritebank & 8) << 5);

	for (INT32 offs = 0; offs < 0xc0; offs += 4)
	{
		if (sr[offs + 3])
		{
			INT32 attr = sr[offs + 1];
			INT32 code = sr[offs + 0] + ((attr & 0x40) << 2);
			INT32 sx = sr[offs + 2] - 2 * (attr & 0x80);
			INT32 sy = sr[offs + 3];
			INT32 color = (attr & 0x0f) << 4;
			INT32 flipx = attr & 0x10;
			INT32 flipy = attr & 0x20;

			if (flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			sy -= 16;
			sx -= 8;

			gberet_draw_16x16(code, sx, sy, color, flipx, flipy);
		}
	}
}

static void gberetb_draw_sprites()
{
	for (INT32 offs = 0x100 - 4; offs >= 0; offs -= 4)
	{
		if (Rom[0xe901 + offs])
		{
			INT32 attr = Rom[0xe903 + offs];
			INT32 code = Rom[0xe900 + offs] + ((attr & 0x40) << 2);
			INT32 sx = Rom[0xe902 + offs] - 2 * (attr & 0x80);
			INT32 sy = 240 - Rom[0xe901 + offs];
			INT32 color = (attr & 0x0f) << 4;
			INT32 flipx = attr & 0x10;
			INT32 flipy = attr & 0x20;

			if (flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			sy -= 16;
			sx -= 8;

			gberet_draw_16x16(code, sx, sy, color, flipx, flipy);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalcPal) {
		for (INT32 i = 0; i < 0x200; i++) {
			DrvPalette[i] = BurnHighCol(Palette[i] >> 16, Palette[i] >> 8, Palette[i], 0);
		}	
	}

	for (INT32 offs = 0x40; offs < 0x7c0; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 3) & 0xf8;
		INT32 attr = Rom[0xc000 + offs];
		INT32 code = Rom[0xc800 + offs] + ((attr & 0x40) << 2);
		INT32 color = attr & 0x0f;
		INT32 flipy = attr & 0x20;
		INT32 flipx = attr & 0x10;

		INT32 scroll = 0;

		if (game_type & 1) {
			if (sy > 0x2f && sy < 0xe8) {
				scroll = gberetb_scroll;
			}
		} else {
			scroll = Rom[0xe000 | (sy >> 3)] | (Rom[0xe020 | (sy >> 3)] << 8);
		}

		sx -= scroll;
		sx += (sx >> 21) & 0x200;

		if (flipscreen) {
			sx = 248 - sx;
			sy = 248 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sy -= 16;
		sx -= 8;

		if (sx < -7 || sx >= 0x100 || sy < 0 || sy > 223) continue;

		if (flipy) {
			if (flipx) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, Gfx0);
			} else {
				Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, Gfx0);
			}
		} else {
			if (flipx) {
				Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, Gfx0);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, Gfx0);

			}
		}
	}

	if (game_type & 1) {
		gberetb_draw_sprites();
	} else {
		gberet_draw_sprites();
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = game_type ? 16 : 32;

	if (DrvReset) {
		DrvDoReset();
	}

	ZetOpen(0);

	INT32 nCyclesDone, nCyclesTotal;

	nCyclesDone = 0;
	nCyclesTotal = 3072000 / (nBurnFPS / 256);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nCyclesSegment = (nCyclesTotal - nCyclesDone) / (nInterleave - i);

		nCyclesDone = ZetRun(nCyclesSegment);

		if (irq_enable && i == (nInterleave - 1)) {
			ZetRaiseIrq(0);
		}

		if (nmi_enable && (i & 1)) {
			ZetNmi();
		}
	}

	ZetClose();

	if (pBurnSoundOut) {
		SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029675;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = Rom + 0xc000;
		ba.nLen	  = 0x4000;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		SCAN_VAR(nmi_enable);
		SCAN_VAR(irq_enable);
		SCAN_VAR(flipscreen);
		SCAN_VAR(mrgoemon_bank);
		SCAN_VAR(gberetb_scroll);
		SCAN_VAR(gberet_spritebank);

		ZetOpen(0);
		mrgoemon_bankswitch(mrgoemon_bank);
		ZetClose();
	}

	return 0;
}


// Green Beret

static struct BurnRomInfo gberetRomDesc[] = {
	{ "577l03.10c",   0x4000, 0xae29e4ff, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "577l02.8c",    0x4000, 0x240836a5, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "577l01.7c",    0x4000, 0x41fa3e1f, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "577l07.3f",    0x4000, 0x4da7bd1b, 2 | BRF_GRA },	       //  3 Characters

	{ "577l06.5e",    0x4000, 0x0f1cb0ca, 3 | BRF_GRA },	       //  4 Sprites
	{ "577l05.4e",    0x4000, 0x523a8b66, 3 | BRF_GRA },	       //  5
	{ "577l08.4f",    0x4000, 0x883933a4, 3 | BRF_GRA },	       //  6
	{ "577l04.3e",    0x4000, 0xccecda4c, 3 | BRF_GRA },	       //  7

	{ "577h09.2f",    0x0020, 0xc15e7c80, 4 | BRF_GRA },	       //  8 Color Proms
	{ "577h11.6f",    0x0100, 0x2a1a992b, 4 | BRF_GRA },	       //  9
	{ "577h10.5f",    0x0100, 0xe9de1e53, 4 | BRF_GRA },	       // 10
};

STD_ROM_PICK(gberet)
STD_ROM_FN(gberet)

struct BurnDriver BurnDrvGberet = {
	"gberet", NULL, NULL, NULL, "1985",
	"Green Beret\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM, 0,
	NULL, gberetRomInfo, gberetRomName, NULL, NULL, DrvInputInfo, gberetDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPal, 0x200,
	240, 224, 4, 3
};


// Rush'n Attack (US)

static struct BurnRomInfo rushatckRomDesc[] = {
	{ "577h03.10c",   0x4000, 0x4d276b52, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "577h02.8c",    0x4000, 0xb5802806, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "577h01.7c",    0x4000, 0xda7c8f3d, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "577h07.3f",    0x4000, 0x03f9815f, 2 | BRF_GRA },	       //  3 Characters

	{ "577l06.5e",    0x4000, 0x0f1cb0ca, 3 | BRF_GRA },	       //  4 Sprites
	{ "577h05.4e",    0x4000, 0x9d028e8f, 3 | BRF_GRA },	       //  5
	{ "577l08.4f",    0x4000, 0x883933a4, 3 | BRF_GRA },	       //  6
	{ "577l04.3e",    0x4000, 0xccecda4c, 3 | BRF_GRA },	       //  7

	{ "577h09.2f",    0x0020, 0xc15e7c80, 4 | BRF_GRA },	       //  8 Color Proms
	{ "577h11.6f",    0x0100, 0x2a1a992b, 4 | BRF_GRA },	       //  9
	{ "577h10.5f",    0x0100, 0xe9de1e53, 4 | BRF_GRA },	       // 10
};

STD_ROM_PICK(rushatck)
STD_ROM_FN(rushatck)

struct BurnDriver BurnDrvRushatck = {
	"rushatck", "gberet", NULL, NULL, "1985",
	"Rush'n Attack (US)\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM, 0,
	NULL, rushatckRomInfo, rushatckRomName, NULL, NULL, DrvInputInfo, gberetDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPal, 0x200,
	240, 224, 4, 3
};


// Green Beret (bootleg)

static struct BurnRomInfo gberetbRomDesc[] = {
	{ "2-ic82.10g",   0x8000, 0x6d6fb494, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "3-ic81.10f",   0x4000, 0xf1520a0a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "1-ic92.12c",   0x4000, 0xb0189c87, 2 | BRF_GRA },	       //  2 Characters

	{ "7-1c8.2b",     0x4000, 0x86334522, 3 | BRF_GRA },	       //  3 Sprites
	{ "6-ic9.2c",     0x4000, 0xbda50d3e, 3 | BRF_GRA },	       //  4
	{ "5-ic10.2d",    0x4000, 0x6a7b3881, 3 | BRF_GRA },	       //  5
	{ "4-ic11.2e",    0x4000, 0x3fb186c9, 3 | BRF_GRA },	       //  6

	{ "577h09",       0x0020, 0xc15e7c80, 4 | BRF_GRA },	       //  7 Color Proms
	{ "577h11.6f",    0x0100, 0x2a1a992b, 4 | BRF_GRA },	       //  8
	{ "577h10.5f",    0x0100, 0xe9de1e53, 4 | BRF_GRA },	       //  9
	
	{ "ic35.5h.bin",  0x0001, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STD_ROM_PICK(gberetb)
STD_ROM_FN(gberetb)

static INT32 gberetbInit()
{
	game_type = 1;

	return DrvInit();
}

struct BurnDriver BurnDrvGberetb = {
	"gberetb", "gberet", NULL, NULL, "1985",
	"Green Beret (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM, 0,
	NULL, gberetbRomInfo, gberetbRomName, NULL, NULL, gberetbInputInfo, gberetbDIPInfo,
	gberetbInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPal, 0x200,
	240, 224, 4, 3
};


// Mr. Goemon (Japan)

static struct BurnRomInfo mrgoemonRomDesc[] = {
	{ "621d01.10c",   0x8000, 0xb2219c56, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "621d02.12c",   0x8000, 0xc3337a97, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "621a05.6d",    0x4000, 0xf0a6dfc5, 2 | BRF_GRA },	       //  2 Characters

	{ "621d03.4d",    0x8000, 0x66f2b973, 3 | BRF_GRA },	       //  3 Sprites
	{ "621d04.5d",    0x8000, 0x47df6301, 3 | BRF_GRA },	       //  4

	{ "621a06.5f",    0x0020, 0x7c90de5f, 4 | BRF_GRA },	       //  5 Color Proms
	{ "621a08.7f",    0x0100, 0x2fb244dd, 4 | BRF_GRA },	       //  6
	{ "621a07.6f",    0x0100, 0x3980acdc, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(mrgoemon)
STD_ROM_FN(mrgoemon)

static INT32 mrgoemonInit()
{
	game_type = 2;

	return DrvInit();
}

struct BurnDriver BurnDrvMrgoemon = {
	"mrgoemon", NULL, NULL, NULL, "1986",
	"Mr. Goemon (Japan)\0", NULL, "Konami", "Miscellaneous",
	L"Mr. Goemon \u4E94\u53F3\u885B\u9580 (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM, 0,
	NULL, mrgoemonRomInfo, mrgoemonRomName, NULL, NULL, DrvInputInfo, mrgoemonDIPInfo,
	mrgoemonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPal, 0x200,
	240, 224, 4, 3
};

