#include "tiles_generic.h"
#include "burn_ym3812.h"
#include "upd7759.h"

// To Do:
// 	Analog Inputs
//	Fix savestates (crash due to snd?)

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvInputs[8];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT32 nAnalogAxis[2] = {0,0};
static UINT16 DrvAxis[2];

static INT32 Rotary1 = 0;
static INT32 Rotary1OldVal = 0;
static INT32 Rotary2 = 0;
static INT32 Rotary2OldVal = 0;

static UINT8 *Mem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KRom;
static UINT8 *Drv68KRomBank;
static UINT8 *DrvZ80Rom;
static UINT8 *Drv68KRam;
static UINT8 *DrvVidRam;
static UINT8 *DrvSprRam;
static UINT8 *DrvPalRam;
static UINT8 *DrvZ80Ram;
static UINT8 *DrvGfx0;
static UINT8 *DrvGfx0Trans;
static UINT8 *DrvGfx1;
static UINT8 *DrvSnd0;

static UINT32 *Palette;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static INT32 game_select;

static INT32 invert_controls;
static INT32 soundlatch;
static INT32 flipscreen;
static INT32 sprite_flip;
static INT32 pow_charbase;

#define A(a, b, c, d) { a, b, (UINT8*)(c), d }

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy3 + 4,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy3 + 5,	"p2 coin"  },

	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 7,	"p1 start" },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy1 + 0,   "p1 up",   },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy1 + 1,   "p1 down", },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 2, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 3, 	"p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 fire 3"},

	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy2 + 7,	"p2 start" },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy2 + 0,   "p2 up",   },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy2 + 1,   "p2 down", },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 2, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 3, 	"p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 6,	"p2 fire 3"},

	{"Service 1",	  BIT_DIGITAL,   DrvJoy3 + 0,   "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo IkariInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy3 + 4,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy3 + 5,	"p2 coin"  },

	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 7,	"p1 start" },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy1 + 0,   "p1 up",   },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy1 + 1,   "p1 down", },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 2, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 3, 	"p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 fire 3"},

	A("P1 Right / left",	BIT_ANALOG_REL, DrvAxis + 0,	"p1 z-axis"),

	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy2 + 7,	"p2 start" },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy2 + 0,   "p2 up",   },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy2 + 1,   "p2 down", },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 2, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 3, 	"p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 6,	"p2 fire 3"},

	A("P2 Right / left",	BIT_ANALOG_REL, DrvAxis + 1,	"p2 z-axis"),

	{"Service 1",	  BIT_DIGITAL,   DrvJoy3 + 0,   "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	   },
};

STDINPUTINFO(Ikari)


static struct BurnDIPInfo PowDIPList[]=
{
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x14, 0x01, 0x03, 0x00, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x03, 0x01, "1 Coin  3 Credits"      },
	{0x14, 0x01, 0x03, 0x03, "1 Coin  4 Credits"      },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x14, 0x01, 0x0c, 0x0c, "4 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x04, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x00, "1 Coin  1 Credit"       },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x14, 0x01, 0x10, 0x00, "2"                      },
	{0x14, 0x01, 0x10, 0x10, "3"                      },

	{0   , 0xfe, 0   , 2   , "Bonus Ocurrence"        },
	{0x14, 0x01, 0x14, 0x00, "1st & 2nd only"         },
	{0x14, 0x01, 0x14, 0x14, "1st & every 2nd"        },

	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x14, 0x01, 0x40, 0x00, "English"                },
	{0x14, 0x01, 0x40, 0x40, "Japanese"               },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x80, 0x00, "Off"                    },
	{0x14, 0x01, 0x80, 0x80, "On"                     },

	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x15, 0x01, 0x01, 0x00, "Off"                    },
	{0x15, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x15, 0x01, 0x02, 0x02, "No"                     },
	{0x15, 0x01, 0x02, 0x00, "Yes"                    },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x0c, 0x00, "20k 50k"                },
	{0x15, 0x01, 0x0c, 0x08, "40k 100k"               },
	{0x15, 0x01, 0x0c, 0x04, "60k 150k"               },
	{0x15, 0x01, 0x0c, 0x0c, "None"                   },

	{0   , 0xfe, 0   , 4   , "Game Mode"              },
	{0x15, 0x01, 0x30, 0x00, "Demo Sounds On"         },
	{0x15, 0x01, 0x30, 0x14, "Demo Sounds Off"        },
	{0x15, 0x01, 0x30, 0x30, "Freeze"                 },
	{0x15, 0x01, 0x30, 0x10, "Infinite Lives (Cheat)" },	

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0xc0, 0x80, "Easy"                   },
	{0x15, 0x01, 0xc0, 0x00, "Normal"                 },
	{0x15, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x15, 0x01, 0xc0, 0xc0, "Hardest"                },
};

STDDIPINFO(Pow)


static struct BurnDIPInfo PowjDIPList[]=
{
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x14, 0x01, 0x03, 0x00, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x03, 0x01, "1 Coin  3 Credits"      },
	{0x14, 0x01, 0x03, 0x03, "1 Coin  4 Credits"      },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x14, 0x01, 0x0c, 0x0c, "4 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x04, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x00, "1 Coin  1 Credit"       },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x14, 0x01, 0x10, 0x00, "2"                      },
	{0x14, 0x01, 0x10, 0x10, "3"                      },

	{0   , 0xfe, 0   , 2   , "Bonus Ocurrence"        },
	{0x14, 0x01, 0x14, 0x00, "1st & 2nd only"         },
	{0x14, 0x01, 0x14, 0x14, "1st & every 2nd"        },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x80, 0x00, "Off"                    },
	{0x14, 0x01, 0x80, 0x80, "On"                     },

	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x15, 0x01, 0x01, 0x00, "Off"                    },
	{0x15, 0x01, 0x01, 0x01, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x15, 0x01, 0x02, 0x02, "No"                     },
	{0x15, 0x01, 0x02, 0x00, "Yes"                    },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x0c, 0x00, "20k 50k"                },
	{0x15, 0x01, 0x0c, 0x08, "40k 100k"               },
	{0x15, 0x01, 0x0c, 0x04, "60k 150k"               },
	{0x15, 0x01, 0x0c, 0x0c, "None"                   },

	{0   , 0xfe, 0   , 4   , "Game Mode"              },
	{0x15, 0x01, 0x30, 0x00, "Demo Sounds On"         },
	{0x15, 0x01, 0x30, 0x14, "Demo Sounds Off"        },
	{0x15, 0x01, 0x30, 0x30, "Freeze"                 },
	{0x15, 0x01, 0x30, 0x10, "Infinite Lives (Cheat)" },	

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0xc0, 0x80, "Easy"                   },
	{0x15, 0x01, 0xc0, 0x00, "Normal"                 },
	{0x15, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x15, 0x01, 0xc0, 0xc0, "Hardest"                },
};

STDDIPINFO(Powj)

static struct BurnDIPInfo StreetsmDIPList[]=
{
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x02, "1"                      },
	{0x14, 0x01, 0x03, 0x00, "2"                      },
	{0x14, 0x01, 0x03, 0x01, "3"                      },
	{0x14, 0x01, 0x03, 0x03, "4"                      },

	{0   , 0xfe, 0   , 4   , "Coin A & B"             },
	{0x14, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x00, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x0c, 0x04, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x0c, 0x0c, "Free Play"              },

	{0   , 0xfe, 0   , 2   , "Bonus Ocurrence"        },
	{0x14, 0x01, 0x14, 0x00, "1st & 2nd only"         },
	{0x14, 0x01, 0x14, 0x14, "1st & every 2nd"        },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x80, 0x00, "Off"                    },
	{0x14, 0x01, 0x80, 0x80, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x15, 0x01, 0x01, 0x00, "Off"                    },
	{0x15, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x15, 0x01, 0x02, 0x02, "No"                     },
	{0x15, 0x01, 0x02, 0x00, "Yes"                    },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x0c, 0x00, "200k 400k"              },
	{0x15, 0x01, 0x0c, 0x08, "400k 500k"              },
	{0x15, 0x01, 0x0c, 0x04, "600k 800k"              },
	{0x15, 0x01, 0x0c, 0x0c, "None"                   },

	{0   , 0xfe, 0   , 4   , "Game Mode"              },
	{0x15, 0x01, 0x30, 0x14, "Demo Sounds Off"        },
	{0x15, 0x01, 0x30, 0x00, "Demo Sounds On"         },
	{0x15, 0x01, 0x30, 0x30, "Freeze"                 },
	{0x15, 0x01, 0x30, 0x10, "Infinite Lives (Cheat)" },	

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0xc0, 0x80, "Easy"                   },
	{0x15, 0x01, 0xc0, 0x00, "Normal"                 },
	{0x15, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x15, 0x01, 0xc0, 0xc0, "Hardest"                },
};

STDDIPINFO(Streetsm)

static struct BurnDIPInfo StreetsjDIPList[]=
{
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x02, "1"                      },
	{0x14, 0x01, 0x03, 0x00, "2"                      },
	{0x14, 0x01, 0x03, 0x01, "3"                      },
	{0x14, 0x01, 0x03, 0x03, "4"                      },

	{0   , 0xfe, 0   , 4   , "Coin A & B"             },
	{0x14, 0x01, 0x0c, 0x0c, "A 4/1 B 1/4"            },
	{0x14, 0x01, 0x0c, 0x04, "A 3/1 B 1/3"            },
	{0x14, 0x01, 0x0c, 0x08, "A 2/1 B 1/2"            },
	{0x14, 0x01, 0x0c, 0x00, "1 Coin 1 Credit"        },

	{0   , 0xfe, 0   , 2   , "Bonus Ocurrence"        },
	{0x14, 0x01, 0x14, 0x00, "1st & 2nd only"         },
	{0x14, 0x01, 0x14, 0x14, "1st & every 2nd"        },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x80, 0x00, "Off"                    },
	{0x14, 0x01, 0x80, 0x80, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x15, 0x01, 0x01, 0x00, "Off"                    },
	{0x15, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x15, 0x01, 0x02, 0x02, "No"                     },
	{0x15, 0x01, 0x02, 0x00, "Yes"                    },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x0c, 0x00, "200k 400k"              },
	{0x15, 0x01, 0x0c, 0x08, "400k 500k"              },
	{0x15, 0x01, 0x0c, 0x04, "600k 800k"              },
	{0x15, 0x01, 0x0c, 0x0c, "None"                   },

	{0   , 0xfe, 0   , 4   , "Game Mode"              },
	{0x15, 0x01, 0x30, 0x14, "Demo Sounds Off"        },
	{0x15, 0x01, 0x30, 0x00, "Demo Sounds On"         },
	{0x15, 0x01, 0x30, 0x30, "Freeze"                 },
	{0x15, 0x01, 0x30, 0x10, "Infinite Lives (Cheat)" },	

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0xc0, 0x80, "Easy"                   },
	{0x15, 0x01, 0xc0, 0x00, "Normal"                 },
	{0x15, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x15, 0x01, 0xc0, 0xc0, "Hardest"                },
};

STDDIPINFO(Streetsj)

static struct BurnDIPInfo SarDIPList[]=
{
	{0x16, 0xff, 0xff, 0x00, NULL                     },
	{0x17, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 2   , "Joystick"               },
	{0x16, 0x01, 0x01, 0x00, "Rotary"                 },
	{0x16, 0x01, 0x01, 0x01, "Standard"               },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x0c, 0x08, "2"                      },
	{0x16, 0x01, 0x0c, 0x00, "3"                      },
	{0x16, 0x01, 0x0c, 0x04, "4"                      },
	{0x16, 0x01, 0x0c, 0x0c, "5"                      },

	{0   , 0xfe, 0   , 4   , "Coin A & B"             },
	{0x16, 0x01, 0x30, 0x16, "2 Coins 1 Credit"       },
	{0x16, 0x01, 0x30, 0x00, "1 Coin  1 Credit"       },
	{0x16, 0x01, 0x30, 0x10, "1 Coin  2 Credits"      },
	{0x16, 0x01, 0x30, 0x30, "Free Play"              },

	{0   , 0xfe, 0   , 2   , "Bonus Ocurrence"        },
	{0x16, 0x01, 0x40, 0x00, "1st & 2nd only"         },
	{0x16, 0x01, 0x40, 0x40, "1st & every 2nd"        },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x16, 0x01, 0x80, 0x00, "Off"                    },
	{0x16, 0x01, 0x80, 0x80, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x17, 0x01, 0x01, 0x00, "Off"                    },
	{0x17, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x02, 0x02, "No"                     },
	{0x17, 0x01, 0x02, 0x00, "Yes"                    },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x17, 0x01, 0x0c, 0x00, "50k 200k"               },
	{0x17, 0x01, 0x0c, 0x08, "70k 270k"               },
	{0x17, 0x01, 0x0c, 0x04, "90k 350k"               },
	{0x17, 0x01, 0x0c, 0x0c, "None"                   },

	{0   , 0xfe, 0   , 4   , "Game Mode"              },
	{0x17, 0x01, 0x30, 0x16, "Demo Sounds Off"        },
	{0x17, 0x01, 0x30, 0x00, "Demo Sounds On"         },
	{0x17, 0x01, 0x30, 0x30, "Freeze"                 },
	{0x17, 0x01, 0x30, 0x10, "Infinite Lives (Cheat)" },	

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x17, 0x01, 0xc0, 0x80, "Easy"                   },
	{0x17, 0x01, 0xc0, 0x00, "Normal"                 },
	{0x17, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x17, 0x01, 0xc0, 0xc0, "Hardest"                },
};

STDDIPINFO(Sar)

static struct BurnDIPInfo IkariDIPList[]=
{
	{0x16, 0xff, 0xff, 0x00, NULL                     },
	{0x17, 0xff, 0xff, 0x00, NULL                     },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x03, 0x02, "2"                      },
	{0x16, 0x01, 0x03, 0x00, "3"                      },
	{0x16, 0x01, 0x03, 0x01, "4"                      },
	{0x16, 0x01, 0x03, 0x03, "5"                      },

	{0   , 0xfe, 0   , 4   , "Coin A & B"             },
	{0x16, 0x01, 0x0c, 0x08, "First 2C_1C, then 1C_1C"},
	{0x16, 0x01, 0x0c, 0x00, "1 Coin 1 Credit"        },
	{0x16, 0x01, 0x0c, 0x04, "First 1C_2C, then 1C_1C"},
	{0x16, 0x01, 0x0c, 0x0c, "Free Play"              },

	{0   , 0xfe, 0   , 2   , "Bonus Ocurrence"        },
	{0x16, 0x01, 0x16, 0x00, "1st & 2nd only"         },
	{0x16, 0x01, 0x16, 0x16, "1st & every 2nd"        },

	{0   , 0xfe, 0   , 2   , "Blood"                  },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x16, 0x01, 0x80, 0x00, "Off"                    },
	{0x16, 0x01, 0x80, 0x80, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x17, 0x01, 0x01, 0x00, "Off"                    },
	{0x17, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x02, 0x02, "No"                     },
	{0x17, 0x01, 0x02, 0x00, "Yes"                    },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x17, 0x01, 0x0c, 0x00, "20k 50k"                },
	{0x17, 0x01, 0x0c, 0x08, "40k 100k"               },
	{0x17, 0x01, 0x0c, 0x04, "60k 150k"               },
	{0x17, 0x01, 0x0c, 0x0c, "None"                   },

	{0   , 0xfe, 0   , 4   , "Game Mode"              },
	{0x17, 0x01, 0x30, 0x16, "Demo Sounds Off"        },
	{0x17, 0x01, 0x30, 0x00, "Demo Sounds On"         },
	{0x17, 0x01, 0x30, 0x30, "Freeze"                 },
	{0x17, 0x01, 0x30, 0x10, "Infinite Lives (Cheat)" },	

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x17, 0x01, 0xc0, 0x00, "Easy"                   },
	{0x17, 0x01, 0xc0, 0x80, "Normal"                 },
	{0x17, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x17, 0x01, 0xc0, 0xc0, "Hardest"                },
};

STDDIPINFO(Ikari)


static INT32 DrvDoReset()
{
	DrvReset = 0;

	DrvRecalc = 1;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM3812Reset();
	UPD7759Reset();

	soundlatch = 0;
	flipscreen = 0;
	sprite_flip = 0;
	pow_charbase = 0;
	invert_controls = 0;

	nAnalogAxis[1] = 0;
	nAnalogAxis[0] = 0;

	return 0;
}

void pow_paletteram16_word_w(UINT32 address)
{
	INT32 r,g,b;
	UINT16 data = *((UINT16*)(DrvPalRam + (address & 0x0ffe)));

	r = ((data >> 7) & 0x1e) | ((data >> 14) & 0x01);
	g = ((data >> 3) & 0x1e) | ((data >> 13) & 0x01);
	b = ((data << 1) & 0x1e) | ((data >> 12) & 0x01);

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	Palette[(address >> 1) & 0x7ff] = (r << 16) | (g << 8) | b;
	DrvPalette[(address >> 1) & 0x7ff] = BurnHighCol(r, g, b, 0);
}

void __fastcall pow_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xffff8000) == 0x100000 && game_select & 1) {
		if (!(address & 2))
		data |= 0xff00;

		*((UINT16 *)(DrvSprRam + (address & 0x7fff))) = data;

		return;
	}

	if ((address & 0xfffff000) == 0x400000) {
		*((UINT16 *)(DrvPalRam + (address & 0x0ffe))) = data;

		pow_paletteram16_word_w(address);

		return;
	}
}

void __fastcall pow_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xffff8000) == 0x100000 && game_select == 1) {
		if ((address & 3) == 3) data = 0xff;

		DrvSprRam[address & 0x7fff] = data;

		return;
	}

	if ((address & 0xfffff000) == 0x400000) {
		DrvPalRam[address & 0x0fff] = data;

		pow_paletteram16_word_w(address);

		return;
	}

	switch (address)
	{
		case 0x080000:
			soundlatch = data;
			ZetNmi();
		return;

		case 0x080007:
			invert_controls = ((data & 0xff) == 0x07) ? 0xff : 0x00;
		return;

		case 0x0c0001:
	   		flipscreen   = data & 8;
	    		sprite_flip  = data & 4;
	    		pow_charbase = (data & 0x70) << 4;
		return;
	}
}


UINT16 __fastcall pow_read_word(UINT32 address)
{
	bprintf (PRINT_NORMAL, _T("read %x, w\n"), address);

	return 0;
}

UINT8 __fastcall pow_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x080000:
			return DrvInputs[1];

		case 0x080001:
			return DrvInputs[0];

		case 0x0c0000:
		case 0x0c0001:
			return DrvInputs[2];

		case 0x0e0000:
		case 0x0e0001:
		case 0x0e8000:
		case 0x0e8001:
			return 0xff;

		case 0x0f0000:
		case 0x0f0001:
			return DrvDips[0];

		case 0x0f0008:
		case 0x0f0009:
			return DrvDips[1];
	}

	return 0;
}

UINT8 __fastcall sar_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x080001:
		case 0x080003:
		case 0x080005:
			return DrvInputs[(address >> 1) & 3] ^ invert_controls;

		case 0x0c0000: {
			INT32 RetVal = Rotary1;
			RetVal = (~(1 << RetVal)) & 0xff;
			return (UINT8)RetVal;
		}

		case 0x0c8000: {
			INT32 RetVal = Rotary2;
			RetVal = (~(1 << RetVal)) & 0xff;
			return (UINT8)RetVal;
		}

		case 0x0d0000: {
			INT32 RetVal = 0xff;
			
			if (Rotary1 ==  8) RetVal -= 0x01;
			if (Rotary1 ==  9) RetVal -= 0x02;
			if (Rotary1 == 10) RetVal -= 0x04;
			if (Rotary1 == 11) RetVal -= 0x08;
			
			if (Rotary2 ==  8) RetVal -= 0x10;
			if (Rotary2 ==  9) RetVal -= 0x20;
			if (Rotary2 == 10) RetVal -= 0x40;
			if (Rotary2 == 11) RetVal -= 0x80;
			return (UINT8)RetVal;
		}

		case 0x0f0000:
		case 0x0f0001:
			return DrvDips[0];

		case 0x0f0008:
		case 0x0f0009:
			return DrvDips[1];

		case 0x0f8000:
			return 1;
	}

	return 0;
}

void __fastcall pow_sound_write(UINT16, UINT8)
{
}

void __fastcall pow_sound_out(UINT16 address, UINT8 data)
{
	switch (address & 0xff)
	{
		case 0x00:
		case 0x20:
			BurnYM3812Write((address >> 5) & 1, data);
		return;

		case 0x40:
			UPD7759PortWrite(0, data);
			UPD7759StartWrite(0, 0);
			UPD7759StartWrite(0, 1);
		return;

		case 0x80:
			UPD7759ResetWrite(0, data);
		return;
	}

}

UINT8 __fastcall pow_sound_read(UINT16 address)
{
	if (address == 0xf800) return soundlatch;

	return 0;
}

UINT8 __fastcall pow_sound_in(UINT16 address)
{
	address &= 0xff;

	if (address == 0x0000) return BurnYM3812Read(0);

	return 0;
}

static void powFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 powSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static INT32 DrvGfxDecode(INT32 *spriPlanes, INT32 *spriXOffs, INT32 *spriYOffs, INT32 modulo)
{
	static INT32 tilePlanes[4] = { 0x00000, 0x00004, 0x40000, 0x40004 };
	static INT32 tileXOffs[8]  = { 0x43, 0x42, 0x41, 0x40, 0x03, 0x02, 0x01, 0x00 };
	static INT32 tileYOffs[8]  = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x300000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfx0, 0x010000);

	GfxDecode(0x0800, 4,  8,  8, tilePlanes, tileXOffs, tileYOffs, 0x0080, tmp, DrvGfx0);

	memcpy (tmp, DrvGfx1, 0x300000);

	GfxDecode(0x6000, 4, 16, 16, spriPlanes, spriXOffs, spriYOffs, modulo, tmp, DrvGfx1);

	BurnFree (tmp);

	memset (DrvGfx0Trans, 1, 0x800);
	for (INT32 i = 0; i < 0x20000; i++) {
		if (DrvGfx0[i]) {
			DrvGfx0Trans[i>>6] = 0;
			i|=0x3f;
		}
	}

	return 0;
}

static INT32 PowGfxDecode()
{
	static INT32 spriPlanes[4] = { 0x000000, 0x400000, 0x800000, 0xc00000 };
	static INT32 spriXOffs[16] = { 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80,
				     0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };
	static INT32 spriYOffs[16] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
				     0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78 };

	return DrvGfxDecode(spriPlanes, spriXOffs, spriYOffs, 0x100);
}

static INT32 SarGfxDecode()
{
	static INT32 spriPlanes[4] = { 0x000000, 0x000008,0xc00000, 0xc00008 };
	static INT32 spriXOffs[16] = { 0x107, 0x106, 0x105, 0x104, 0x103, 0x102, 0x101, 0x100, 
				     0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000 };
	static INT32 spriYOffs[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
				     0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0 };

	return DrvGfxDecode(spriPlanes, spriXOffs, spriYOffs, 0x200);
}

static INT32 IkariGfxDecode()
{
	static INT32 spriPlanes[4] = { 0xa00000, 0x000000, 0x500000, 0xf00000 };
	static INT32 spriXOffs[16] = { 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80,
				     0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };
	static INT32 spriYOffs[16] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
				     0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78 };

	return DrvGfxDecode(spriPlanes, spriXOffs, spriYOffs, 0x100);
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KRom	= Next; Next += 0x040000;
	Drv68KRomBank	= Next; Next += 0x040000;
	DrvZ80Rom	= Next; Next += 0x010000;

	DrvGfx0		= Next; Next += 0x020000;
	DrvGfx1		= Next; Next += 0x600000;

	DrvGfx0Trans	= Next; Next += 0x000800;

	DrvSnd0		= Next; Next += 0x020000;

	AllRam		= Next;

	Drv68KRam	= Next; Next += 0x004000;
	DrvVidRam	= Next; Next += 0x001000;
	DrvSprRam	= Next; Next += 0x008000;
	DrvPalRam	= Next; Next += 0x001000;

	DrvZ80Ram	= Next; Next += 0x000800;

	Palette		= (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	RamEnd		= Next;

	DrvPalette	= (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	MemEnd		= Next;

	return 0;
}


static void pow_map_68k()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRam,		0x040000, 0x043fff, SM_RAM);
	SekMapMemory(DrvVidRam,		0x100000, 0x100fff, SM_RAM); // video ram
	SekMapMemory(DrvSprRam,		0x200000, 0x207fff, SM_RAM); // sprite ram
	SekMapMemory(DrvPalRam,		0x400000, 0x400fff, SM_ROM); // palette ram
	SekSetWriteByteHandler(0,	pow_write_byte);
	SekSetWriteWordHandler(0,	pow_write_word);
	SekSetReadByteHandler(0,	pow_read_byte);
	SekSetReadWordHandler(0,	pow_read_word);
	SekClose();
}

static void sar_map_68k()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRam,		0x040000, 0x043fff, SM_RAM);
	SekMapMemory(DrvSprRam,		0x100000, 0x107fff, SM_ROM); // sprite ram
	SekMapMemory(DrvVidRam,		0x200000, 0x200fff, SM_RAM); // video ram
	SekMapMemory(DrvVidRam,		0x201000, 0x201fff, SM_WRITE); // video ram mirror
	SekMapMemory(Drv68KRomBank,	0x300000, 0x33ffff, SM_ROM); // extra rom
	SekMapMemory(DrvPalRam,		0x400000, 0x400fff, SM_ROM); // palette ram
	SekSetWriteByteHandler(0,	pow_write_byte);
	SekSetWriteWordHandler(0,	pow_write_word);
	SekSetReadByteHandler(0,	sar_read_byte);
	SekSetReadWordHandler(0,	pow_read_word);
	SekClose();
}


static INT32 DrvGetRoms()
{
	char* pRomName;
	struct BurnRomInfo ri;
	UINT8 *LoadP0 = Drv68KRom;
	UINT8 *LoadP1 = DrvZ80Rom;
	UINT8 *LoadG0 = DrvGfx0;
	UINT8 *LoadG1 = DrvGfx1;
	UINT8 *LoadS0 = DrvSnd0;

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {
		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & 7) == 1) {
			if (BurnLoadRom(LoadP0 + 1, i + 0, 2)) return 1;
			if (BurnLoadRom(LoadP0 + 0, i + 1, 2)) return 1;
			LoadP0 += ri.nLen * 2;
			i++;
			continue;
		}

		if ((ri.nType & 7) == 2) {
			if (BurnLoadRom(LoadP1, i, 1)) return 1;
			LoadP1 += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 3) {
			if (BurnLoadRom(LoadG0, i, 1)) return 1;
			LoadG0 += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 4) {
			if (BurnLoadRom(LoadG1, i, 1)) return 1;
			LoadG1 += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 5) {
			if (BurnLoadRom(LoadS0, i, 1)) return 1;
			LoadS0 += ri.nLen;
			continue;
		}		
	}

	return 0;
}

static INT32 DrvInit(INT32 game)
{
	game_select = game;

	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	if (DrvGetRoms()) return 1;

	switch (game_select)
	{
		case 0: // Pow
			if (PowGfxDecode()) return 1;
			pow_map_68k();
		break;

		case 1: // searchar
			if (SarGfxDecode()) return 1;
			sar_map_68k();
		break;

		case 2: // streets
			if (SarGfxDecode()) return 1;
			pow_map_68k();
		break;

		case 3: // ikari
			if (IkariGfxDecode()) return 1;
			sar_map_68k();
		break;
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80Rom);
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80Rom);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80Ram);
	ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80Ram);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80Ram);
	ZetSetWriteHandler(pow_sound_write);
	ZetSetReadHandler(pow_sound_read);
	ZetSetInHandler(pow_sound_in);
	ZetSetOutHandler(pow_sound_out);
	ZetMemEnd();
	ZetClose();

	BurnYM3812Init(4000000, &powFMIRQHandler, &powSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);
	
	UPD7759Init(0, UPD7759_STANDARD_CLOCK, DrvSnd0);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	BurnYM3812Exit();
	UPD7759Exit();

	GenericTilesExit();
	SekExit();
	ZetExit();

	BurnFree (Mem);

	game_select = 0;

	return 0;
}


static void pow_sprites(INT32 j, INT32 pos)
{
	INT32 offs,mx,my,color,tile,fx,fy,i;

	UINT16 *spriteram16 = (UINT16*)(DrvSprRam);

	for (offs = pos; offs < pos+0x800; offs += 0x80 )
	{
		mx=(spriteram16[(offs+4+(4*j))>>1]&0xff)<<4;
		my=spriteram16[(offs+6+(4*j))>>1];
		mx=mx+(my>>12);
		mx=((mx+16)&0x1ff)-16;

		mx=((mx+0x100)&0x1ff)-0x100;
		my=((my+0x100)&0x1ff)-0x100;

		my=0x200 - my;
		my-=0x200;

		if (flipscreen) {
			mx=240-mx;
			my=240-my;
		}

		my -= 16;

		for (i = 0; i < 0x80; i+=4)
		{
			color = spriteram16[(offs+i+(0x1000*j)+0x1000)>>1]&0x7f;

			if (color)
			{
				tile=spriteram16[(offs+2+i+(0x1000*j)+0x1000)>>1];
				fy=tile&0x8000;
				fx=tile&0x4000;
				tile&=0x3fff;

				if (flipscreen) {
					if (fx) fx=0; else fx=1;
					if (fy) fy=0; else fy=1;
				}

				if (mx > -16 && mx < 256 && my > -16 && my < 224) {
					if (fy) {
						if (fx) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, tile, mx, my, color, 4, 0, 0, DrvGfx1);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, tile, mx, my, color, 4, 0, 0, DrvGfx1);
						}
					} else {
						if (fx) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, tile, mx, my, color, 4, 0, 0, DrvGfx1);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, tile, mx, my, color, 4, 0, 0, DrvGfx1);
						}
					}
				}
			}

			if (flipscreen) {
				my-=16;
				if (my < -0x100) my+=0x200;
			}
			else {
				my+=16;
				if (my > 0x100) my-=0x200;
			}
		}
	}
}

static void sar_sprites(INT32 j, INT32 z, INT32 pos)
{
	INT32 offs,mx,my,color,tile,fx,fy,i;

	UINT16 *spriteram16 = (UINT16*)(DrvSprRam);

	for (offs = pos; offs < pos+0x800 ; offs += 0x80 )
	{
		mx=spriteram16[(offs+j)>>1];
		my=spriteram16[(offs+j+2)>>1];

		mx=mx<<4;
		mx=mx|((my>>12)&0xf);
		my=my&0x1ff;

		mx=(mx+0x100)&0x1ff;
		my=(my+0x100)&0x1ff;
		mx-=0x100;
		my-=0x100;
		my=0x200 - my;
		my-=0x200;

		if (flipscreen) {
			mx=240-mx;
			my=240-my;
		}

		my -= 16;

		for (i = 0; i < 0x80; i += 4)
		{
			color = spriteram16[(offs+i+z)>>1] & 0x7f;

			if (color)
			{
				tile = spriteram16[(offs+2+i+z)>>1];

				if (sprite_flip) {
					fx=0;
					fy=tile&0x8000;
				} else {
					fy=0;
					fx=tile&0x8000;
				}

				if (flipscreen) {
					if (fx) fx=0; else fx=1;
					if (fy) fy=0; else fy=1;
				}

				tile&=0x7fff;
				if (tile>0x5fff) break;

				if (mx > -16 && mx < 256 && my > -16 && my < 224) {
					if (fy) {
						if (fx) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, tile, mx, my, color, 4, 0, 0, DrvGfx1);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, tile, mx, my, color, 4, 0, 0, DrvGfx1);
						}
					} else {
						if (fx) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, tile, mx, my, color, 4, 0, 0, DrvGfx1);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, tile, mx, my, color, 4, 0, 0, DrvGfx1);
						}
					}
				}
			}

			if (flipscreen) {
				my-=16;
				if (my < -0x100) my+=0x200;
			}
			else {
				my+=16;
				if (my > 0x100) my-=0x200;
			}
		}
	}
}

static void pow_foreground()
{
	UINT16 *vidram = (UINT16*)DrvVidRam;

	for (INT32 offs = 0; offs < 0x1000 / 2; offs+=2)
	{
		INT32 sy = (offs << 2) & 0xf8;
		INT32 sx = (offs >> 3) & 0xf8;

		if (flipscreen) {
			sy ^= 0xf8;
			sx ^= 0xf8;
		}

		if (sy < 16 || sy > 239) continue;
		sy -= 16;

		INT32 code  = (vidram[offs | 0] & 0xff) | pow_charbase;
		INT32 color = vidram[offs | 1] & 0x0f;

		if (DrvGfx0Trans[code]) continue;

		if (flipscreen) {
			Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfx0);
		} else {
			Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfx0);
		}
	}
}

static void sar_foreground()
{
	UINT16 *vidram = (UINT16*)DrvVidRam;

	for (INT32 offs = 0; offs < 0x1000 / 2; offs+=2)
	{
		INT32 sy = (offs << 2) & 0xf8;
		INT32 sx = (offs >> 3) & 0xf8;

		if (flipscreen) {
			sy ^= 0xf8;
			sx ^= 0xf8;
		}

		if (sy < 16 || sy > 239) continue;
		sy -= 16;

		INT32 code = vidram[offs];
		INT32 color = code >> 12;

		// kludge for bad tile (ikari)
		if (code == 0x80ff) {
			code  = 0x02ca;
			color = 0x07;
		}

		code &= 0x0fff;

		if (DrvGfx0Trans[code]) continue;

		if (flipscreen) {
			Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfx0);
		} else {
			Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfx0);
		}		
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x800; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	for (INT32 offs = 0; offs < nScreenHeight * nScreenWidth; offs++) {
		pTransDraw[offs] = 0x7ff;
	}

	if (!game_select) {
		pow_sprites(1, 0x000);
		pow_sprites(1, 0x800);
		pow_sprites(2, 0x000);
		pow_sprites(2, 0x800);
		pow_sprites(0, 0x000);
		pow_sprites(0, 0x800);
	} else {
		sar_sprites(8, 0x2000, 0x000);
		sar_sprites(8, 0x2000, 0x800);
		sar_sprites(12,0x3000, 0x000);
		sar_sprites(12,0x3000, 0x800);
		sar_sprites(4, 0x1000, 0x000);
		sar_sprites(4, 0x1000, 0x800);
	}

	if (game_select & 1) {
		sar_foreground();
	} else {
		pow_foreground();
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
		DrvInputs[0] = DrvInputs[1] = DrvInputs[2] = 0xff;

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= DrvJoy1[i] << i;
			DrvInputs[1] ^= DrvJoy2[i] << i;
			DrvInputs[2] ^= DrvJoy3[i] << i;
		}

		nAnalogAxis[0] -= DrvAxis[0];
		DrvInputs[6] = (~nAnalogAxis[0] >> 8) & 0xfe;

		nAnalogAxis[1] -= DrvAxis[1];
		DrvInputs[7] = (~nAnalogAxis[1] >> 8) & 0xfe;
	}
	
	if (game_select == 1 || game_select == 3) {
		if ((DrvInputs[6] >> 4) < Rotary1OldVal) {
			Rotary1++;
		}

		if ((DrvInputs[6] >> 4) > Rotary1OldVal) {
			Rotary1--;
		}
		Rotary1OldVal = DrvInputs[6] >> 4;
		if (Rotary1 > 11) Rotary1 = 0;
		if (Rotary1 < 0) Rotary1 = 11;
		
		if ((DrvInputs[7] >> 4) < Rotary2OldVal) {
			Rotary2++;
		}

		if ((DrvInputs[7] >> 4) > Rotary2OldVal) {
			Rotary2--;
		}
		Rotary2OldVal = DrvInputs[7] >> 4;
		if (Rotary2 > 11) Rotary2 = 0;
		if (Rotary2 < 0) Rotary2 = 11;
	}

	INT32 nTotalCycles[2] =  { ((game_select == 1) ? 9000000 : 10000000) / 60, 4000000 / 60 };

	SekOpen(0);
	ZetOpen(0);

	SekNewFrame();
	ZetNewFrame();

	SekRun(nTotalCycles[0]);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

	BurnTimerEndFrameYM3812(nTotalCycles[1]);
	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		UPD7759Update(0, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}


static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029682;
	}

	if (nAction & ACB_MEMORY_RAM) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		if (nAction & ACB_WRITE)
			DrvRecalc = 1;
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		BurnYM3812Scan(nAction, pnMin);
		UPD7759Scan(0, nAction, pnMin);

		SCAN_VAR(invert_controls);
		SCAN_VAR(soundlatch);
		SCAN_VAR(flipscreen);
		SCAN_VAR(sprite_flip);
		SCAN_VAR(pow_charbase);
		SCAN_VAR(nAnalogAxis[0]);
		SCAN_VAR(nAnalogAxis[1]);
		SCAN_VAR(Rotary1);
		SCAN_VAR(Rotary1OldVal);
		SCAN_VAR(Rotary2);
		SCAN_VAR(Rotary2OldVal);
	}

	return 0;
}


// P.O.W. - Prisoners of War (US version 1)

static struct BurnRomInfo powRomDesc[] = {
	{ "dg1ver1.j14", 0x20000, 0x8e71a8af, 1 | BRF_PRG }, //  0 68k COde
	{ "dg2ver1.l14", 0x20000, 0x4287affc, 1 | BRF_PRG }, //  1

	{ "dg8.e25",     0x10000, 0xd1d61da3, 2 | BRF_PRG }, //  2 Z80 Code

	{ "dg9.l25",     0x08000, 0xdf864a08, 3 | BRF_GRA }, //  3 Characters
	{ "dg10.m25",    0x08000, 0x9e470d53, 3 | BRF_GRA }, //  4

	{ "snk880.11a",  0x20000, 0xe70fd906, 4 | BRF_GRA }, //  5 Sprites
	{ "snk880.12a",  0x20000, 0x628b1aed, 4 | BRF_GRA }, //  6
	{ "snk880.13a",  0x20000, 0x19dc8868, 4 | BRF_GRA }, //  7
	{ "snk880.14a",  0x20000, 0x47cd498b, 4 | BRF_GRA }, //  8
	{ "snk880.15a",  0x20000, 0x7a90e957, 4 | BRF_GRA }, //  9
	{ "snk880.16a",  0x20000, 0xe40a6c13, 4 | BRF_GRA }, // 10
	{ "snk880.17a",  0x20000, 0xc7931cc2, 4 | BRF_GRA }, // 11
	{ "snk880.18a",  0x20000, 0xeed72232, 4 | BRF_GRA }, // 12
	{ "snk880.19a",  0x20000, 0x1775b8dd, 4 | BRF_GRA }, // 13
	{ "snk880.20a",  0x20000, 0xf8e752ec, 4 | BRF_GRA }, // 14
	{ "snk880.21a",  0x20000, 0x27e9fffe, 4 | BRF_GRA }, // 15
	{ "snk880.22a",  0x20000, 0xaa9c00d8, 4 | BRF_GRA }, // 16
	{ "snk880.23a",  0x20000, 0xadb6ad68, 4 | BRF_GRA }, // 17
	{ "snk880.24a",  0x20000, 0xdd41865a, 4 | BRF_GRA }, // 18
	{ "snk880.25a",  0x20000, 0x055759ad, 4 | BRF_GRA }, // 19
	{ "snk880.26a",  0x20000, 0x9bc261c5, 4 | BRF_GRA }, // 20

	{ "dg7.d20",     0x10000, 0xaba9a9d3, 5 | BRF_SND }, // 21 upd7759 samples

	{ "pal20l10.a6", 0x000cc, 0xc3d9e729, 0 | BRF_OPT }, // 22 pld
};

STD_ROM_PICK(pow)
STD_ROM_FN(pow)

static INT32 powInit()
{
	return DrvInit(0);
}

struct BurnDriver BurnDrvpow = {
	"pow", NULL, NULL, NULL, "1988",
	"P.O.W. - Prisoners of War (US version 1)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, powRomInfo, powRomName, NULL, NULL, DrvInputInfo, PowDIPInfo,
	powInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};


// Datsugoku - Prisoners of War (Japan)

static struct BurnRomInfo powjRomDesc[] = {
	{ "1-2",         0x20000, 0x2f17bfb0, 1 | BRF_PRG }, //  0 68k Code
	{ "2-2",         0x20000, 0xbaa32354, 1 | BRF_PRG }, //  1

	{ "dg8.e25",     0x10000, 0xd1d61da3, 2 | BRF_PRG }, //  2 Z80 Code

	{ "dg9.l25",     0x08000, 0xdf864a08, 3 | BRF_GRA }, //  3 Characters
	{ "dg10.m25",    0x08000, 0x9e470d53, 3 | BRF_GRA }, //  4

	{ "snk880.11a",  0x20000, 0xe70fd906, 4 | BRF_GRA }, //  5 Sprites
	{ "snk880.12a",  0x20000, 0x628b1aed, 4 | BRF_GRA }, //  6
	{ "snk880.13a",  0x20000, 0x19dc8868, 4 | BRF_GRA }, //  7
	{ "snk880.14a",  0x20000, 0x47cd498b, 4 | BRF_GRA }, //  8
	{ "snk880.15a",  0x20000, 0x7a90e957, 4 | BRF_GRA }, //  9
	{ "snk880.16a",  0x20000, 0xe40a6c13, 4 | BRF_GRA }, // 10
	{ "snk880.17a",  0x20000, 0xc7931cc2, 4 | BRF_GRA }, // 11
	{ "snk880.18a",  0x20000, 0xeed72232, 4 | BRF_GRA }, // 12
	{ "snk880.19a",  0x20000, 0x1775b8dd, 4 | BRF_GRA }, // 13
	{ "snk880.20a",  0x20000, 0xf8e752ec, 4 | BRF_GRA }, // 14
	{ "snk880.21a",  0x20000, 0x27e9fffe, 4 | BRF_GRA }, // 15
	{ "snk880.22a",  0x20000, 0xaa9c00d8, 4 | BRF_GRA }, // 16
	{ "snk880.23a",  0x20000, 0xadb6ad68, 4 | BRF_GRA }, // 17
	{ "snk880.24a",  0x20000, 0xdd41865a, 4 | BRF_GRA }, // 18
	{ "snk880.25a",  0x20000, 0x055759ad, 4 | BRF_GRA }, // 19
	{ "snk880.26a",  0x20000, 0x9bc261c5, 4 | BRF_GRA }, // 20

	{ "dg7.d20",     0x10000, 0xaba9a9d3, 5 | BRF_SND }, // 21 upd7759 samples

	{ "pal20l10.a6", 0x000cc, 0xc3d9e729, 0 | BRF_OPT }, // 22 pld
};

STD_ROM_PICK(powj)
STD_ROM_FN(powj)

struct BurnDriver BurnDrvpowj = {
	"powj", "pow", NULL, NULL, "1988",
	"Datsugoku - Prisoners of War (Japan)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, powjRomInfo, powjRomName, NULL, NULL, DrvInputInfo, PowjDIPInfo,
	powInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};


// SAR - Search And Rescue (World)

static struct BurnRomInfo searcharRomDesc[] = {
	{ "bhw.2", 	 0x20000, 0xe1430138, 1 | BRF_PRG }, //  0 68k Code
	{ "bhw.3", 	 0x20000, 0xee1f9374, 1 | BRF_PRG }, //  1
	{ "bhw.1", 	 0x20000, 0x62b60066, 1 | BRF_PRG }, //  2
	{ "bhw.4", 	 0x20000, 0x16d8525c, 1 | BRF_PRG }, //  3

	{ "bh.5",        0x10000, 0x53e2fa76, 2 | BRF_PRG }, //  4 Z80 Code

	{ "bh.7",        0x08000, 0xb0f1b049, 3 | BRF_GRA }, //  5 Characters
	{ "bh.8",        0x08000, 0x174ddba7, 3 | BRF_GRA }, //  6

	{ "bh.c1",       0x80000, 0x1fb8f0ae, 4 | BRF_GRA }, //  7 Sprites
	{ "bh.c3",       0x80000, 0xfd8bc407, 4 | BRF_GRA }, //  8
	{ "bh.c5",       0x80000, 0x1d30acc3, 4 | BRF_GRA }, //  9
	{ "bh.c2",       0x80000, 0x7c803767, 4 | BRF_GRA }, // 10
	{ "bh.c4",       0x80000, 0xeede7c43, 4 | BRF_GRA }, // 11
	{ "bh.c6",       0x80000, 0x9f785cd9, 4 | BRF_GRA }, // 12

	{ "bh.v1",       0x20000, 0x07a6114b, 5 | BRF_SND }, // 13 upd7759 samples
};

STD_ROM_PICK(searchar)
STD_ROM_FN(searchar)

static INT32 searcharInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvsearchar = {
	"searchar", NULL, NULL, NULL, "1989",
	"SAR - Search And Rescue (World)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, searcharRomInfo, searcharRomName, NULL, NULL, IkariInputInfo, SarDIPInfo,
	searcharInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 224, 256, 3, 4
};


// SAR - Search And Rescue (US)

static struct BurnRomInfo sercharuRomDesc[] = {
	{ "bh.2",  	 0x20000, 0xc852e2e2, 1 | BRF_PRG }, //  0 68k Code
	{ "bh.3",  	 0x20000, 0xbc04a4a1, 1 | BRF_PRG }, //  1
	{ "bh.1",  	 0x20000, 0xba9ca70b, 1 | BRF_PRG }, //  2
	{ "bh.4",  	 0x20000, 0xeabc5ddf, 1 | BRF_PRG }, //  3

	{ "bh.5",        0x10000, 0x53e2fa76, 2 | BRF_PRG }, //  4 Z80 Code

	{ "bh.7",        0x08000, 0xb0f1b049, 3 | BRF_GRA }, //  5 Characters
	{ "bh.8",        0x08000, 0x174ddba7, 3 | BRF_GRA }, //  6

	{ "bh.c1",       0x80000, 0x1fb8f0ae, 4 | BRF_GRA }, //  7 Sprites
	{ "bh.c3",       0x80000, 0xfd8bc407, 4 | BRF_GRA }, //  8
	{ "bh.c5",       0x80000, 0x1d30acc3, 4 | BRF_GRA }, //  9
	{ "bh.c2",       0x80000, 0x7c803767, 4 | BRF_GRA }, // 10
	{ "bh.c4",       0x80000, 0xeede7c43, 4 | BRF_GRA }, // 11
	{ "bh.c6",       0x80000, 0x9f785cd9, 4 | BRF_GRA }, // 12

	{ "bh.v1",       0x20000, 0x07a6114b, 5 | BRF_SND }, // 13 upd7759 samples
};

STD_ROM_PICK(sercharu)
STD_ROM_FN(sercharu)

struct BurnDriver BurnDrvsercharu = {
	"searcharu", "searchar", NULL, NULL, "1989",
	"SAR - Search And Rescue (US)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, sercharuRomInfo, sercharuRomName, NULL, NULL, IkariInputInfo, SarDIPInfo,
	searcharInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 224, 256, 3, 4
};


// SAR - Search And Rescue (Japan)

static struct BurnRomInfo sercharjRomDesc[] = {
	{ "bh2ver3j.9c", 0x20000, 0x7ef7b172, 1 | BRF_PRG }, //  0 68k Code
	{ "bh3ver3j.10c",0x20000, 0x3fdea793, 1 | BRF_PRG }, //  1
	{ "bhw.1", 	 0x20000, 0x62b60066, 1 | BRF_PRG }, //  2
	{ "bhw.4", 	 0x20000, 0x16d8525c, 1 | BRF_PRG }, //  3

	{ "bh.5",        0x10000, 0x53e2fa76, 2 | BRF_PRG }, //  4 Z80 Code

	{ "bh.7",        0x08000, 0xb0f1b049, 3 | BRF_GRA }, //  5 Characters
	{ "bh.8",        0x08000, 0x174ddba7, 3 | BRF_GRA }, //  6

	{ "bh.c1",       0x80000, 0x1fb8f0ae, 4 | BRF_GRA }, //  7 Sprites
	{ "bh.c3",       0x80000, 0xfd8bc407, 4 | BRF_GRA }, //  8
	{ "bh.c5",       0x80000, 0x1d30acc3, 4 | BRF_GRA }, //  9
	{ "bh.c2",       0x80000, 0x7c803767, 4 | BRF_GRA }, // 10
	{ "bh.c4",       0x80000, 0xeede7c43, 4 | BRF_GRA }, // 11
	{ "bh.c6",       0x80000, 0x9f785cd9, 4 | BRF_GRA }, // 12

	{ "bh.v1",       0x20000, 0x07a6114b, 5 | BRF_SND }, // 13 upd7759 samples
};

STD_ROM_PICK(sercharj)
STD_ROM_FN(sercharj)

struct BurnDriver BurnDrvsercharj = {
	"searcharj", "searchar", NULL, NULL, "1989",
	"SAR - Search And Rescue (Japan)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, sercharjRomInfo, sercharjRomName, NULL, NULL, IkariInputInfo, SarDIPInfo,
	searcharInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 224, 256, 3, 4
};



// Street Smart (US version 2)

static struct BurnRomInfo streetsmRomDesc[] = {
	{ "s2-1ver2.14h", 0x20000, 0x655f4773, 1 | BRF_PRG }, //  0 68k Code
	{ "s2-2ver2.14k", 0x20000, 0xefae4823, 1 | BRF_PRG }, //  1

	{ "s2-5.16c",     0x10000, 0xca4b171e, 2 | BRF_PRG }, //  2 Z80 Code

	{ "s2-9.25l",     0x08000, 0x09b6ac67, 3 | BRF_GRA }, //  3 Characters
	{ "s2-10.25m",    0x08000, 0x89e4ee6f, 3 | BRF_GRA }, //  4

	{ "stsmart.900",  0x80000, 0xa8279a7e, 4 | BRF_GRA }, //  5 Sprites
	{ "stsmart.902",  0x80000, 0x2f021aa1, 4 | BRF_GRA }, //  6
	{ "stsmart.904",  0x80000, 0x167346f7, 4 | BRF_GRA }, //  7
	{ "stsmart.901",  0x80000, 0xc305af12, 4 | BRF_GRA }, //  8
	{ "stsmart.903",  0x80000, 0x73c16d35, 4 | BRF_GRA }, //  9
	{ "stsmart.905",  0x80000, 0xa5beb4e2, 4 | BRF_GRA }, // 10

	{ "s2-6.18d",     0x20000, 0x47db1605, 5 | BRF_SND }, // 11 upd7759 samples

	{ "pl20l10a.1n",  0x000cc, 0x00000000, 0 | BRF_NODUMP }, /* PAL is read protected */
};

STD_ROM_PICK(streetsm)
STD_ROM_FN(streetsm)

static INT32 streetsmInit()
{
	return DrvInit(2);
}

struct BurnDriver BurnDrvstreetsm = {
	"streetsm", NULL, NULL, NULL, "1989",
	"Street Smart (US version 2)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_VSFIGHT, 0,
	NULL, streetsmRomInfo, streetsmRomName, NULL, NULL, DrvInputInfo, StreetsmDIPInfo,
	streetsmInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};


// Street Smart (US version 1)

static struct BurnRomInfo streets1RomDesc[] = {
	{ "s2-1ver1.9c",  0x20000, 0xb59354c5, 1 | BRF_PRG }, //  0 68k Code
	{ "s2-2ver1.10c", 0x20000, 0xe448b68b, 1 | BRF_PRG }, //  1

	{ "s2-5.16c",     0x10000, 0xca4b171e, 2 | BRF_PRG }, //  2 Z80 Code

	{ "s2-7.15l",     0x08000, 0x22bedfe5, 3 | BRF_GRA }, //  3 Characters
	{ "s2-8.15m",     0x08000, 0x6a1c70ab, 3 | BRF_GRA }, //  4

	{ "stsmart.900",  0x80000, 0xa8279a7e, 4 | BRF_GRA }, //  5 Sprites
	{ "stsmart.902",  0x80000, 0x2f021aa1, 4 | BRF_GRA }, //  6
	{ "stsmart.904",  0x80000, 0x167346f7, 4 | BRF_GRA }, //  7
	{ "stsmart.901",  0x80000, 0xc305af12, 4 | BRF_GRA }, //  8
	{ "stsmart.903",  0x80000, 0x73c16d35, 4 | BRF_GRA }, //  9
	{ "stsmart.905",  0x80000, 0xa5beb4e2, 4 | BRF_GRA }, // 10

	{ "s2-6.18d",     0x20000, 0x47db1605, 5 | BRF_SND }, // 11 upd7759 samples
};

STD_ROM_PICK(streets1)
STD_ROM_FN(streets1)

struct BurnDriver BurnDrvstreets1 = {
	"streetsm1", "streetsm", NULL, NULL, "1989",
	"Street Smart (US version 1)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_VSFIGHT, 0,
	NULL, streets1RomInfo, streets1RomName, NULL, NULL, DrvInputInfo, StreetsmDIPInfo,
	searcharInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};


// Street Smart (World version 1)

static struct BurnRomInfo streetswRomDesc[] = {
	{ "s-smart1.bin", 0x20000, 0xa1f5ceab, 1 | BRF_PRG }, //  0 68k Code
	{ "s-smart2.bin", 0x20000, 0x263f615d, 1 | BRF_PRG }, //  1

	{ "s2-5.16c",     0x10000, 0xca4b171e, 2 | BRF_PRG }, //  2 Z80 Code

	{ "s2-7.15l",     0x08000, 0x22bedfe5, 3 | BRF_GRA }, //  3 Characters
	{ "s2-8.15m",     0x08000, 0x6a1c70ab, 3 | BRF_GRA }, //  4

	{ "stsmart.900",  0x80000, 0xa8279a7e, 4 | BRF_GRA }, //  5 Sprites
	{ "stsmart.902",  0x80000, 0x2f021aa1, 4 | BRF_GRA }, //  6
	{ "stsmart.904",  0x80000, 0x167346f7, 4 | BRF_GRA }, //  7
	{ "stsmart.901",  0x80000, 0xc305af12, 4 | BRF_GRA }, //  8
	{ "stsmart.903",  0x80000, 0x73c16d35, 4 | BRF_GRA }, //  9
	{ "stsmart.905",  0x80000, 0xa5beb4e2, 4 | BRF_GRA }, // 10

	{ "s2-6.18d",     0x20000, 0x47db1605, 5 | BRF_SND }, // 11 upd7759 samples
};

STD_ROM_PICK(streetsw)
STD_ROM_FN(streetsw)

struct BurnDriver BurnDrvstreetsw = {
	"streetsmw", "streetsm", NULL, NULL, "1989",
	"Street Smart (World version 1)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_VSFIGHT, 0,
	NULL, streetswRomInfo, streetswRomName, NULL, NULL, DrvInputInfo, StreetsjDIPInfo,
	searcharInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};


// Street Smart (Japan version 1)

static struct BurnRomInfo streetsjRomDesc[] = {
	{ "s2v1j_01.bin", 0x20000, 0xf031413c, 1 | BRF_PRG }, //  0 68k Code
	{ "s2v1j_02.bin", 0x20000, 0xe403a40b, 1 | BRF_PRG }, //  1

	{ "s2-5.16c",     0x10000, 0xca4b171e, 2 | BRF_PRG }, //  2 Z80 Code

	{ "s2-7.15l",     0x08000, 0x22bedfe5, 3 | BRF_GRA }, //  3 Characters
	{ "s2-8.15m",     0x08000, 0x6a1c70ab, 3 | BRF_GRA }, //  4

	{ "stsmart.900",  0x80000, 0xa8279a7e, 4 | BRF_GRA }, //  5 Sprites
	{ "stsmart.902",  0x80000, 0x2f021aa1, 4 | BRF_GRA }, //  6
	{ "stsmart.904",  0x80000, 0x167346f7, 4 | BRF_GRA }, //  7
	{ "stsmart.901",  0x80000, 0xc305af12, 4 | BRF_GRA }, //  8
	{ "stsmart.903",  0x80000, 0x73c16d35, 4 | BRF_GRA }, //  9
	{ "stsmart.905",  0x80000, 0xa5beb4e2, 4 | BRF_GRA }, // 10

	{ "s2-6.18d",     0x20000, 0x47db1605, 5 | BRF_SND }, // 11 upd7759 samples
};

STD_ROM_PICK(streetsj)
STD_ROM_FN(streetsj)

struct BurnDriver BurnDrvstreetsj = {
	"streetsmj", "streetsm", NULL, NULL, "1989",
	"Street Smart (Japan version 1)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_VSFIGHT, 0,
	NULL, streetsjRomInfo, streetsjRomName, NULL, NULL, DrvInputInfo, StreetsjDIPInfo,
	searcharInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};

// Ikari III - The Rescue (8-Way Joystick)

static struct BurnRomInfo ikari3RomDesc[] = {
	{ "ik3-2-ver1.c10", 0x20000, 0x1bae8023, 1 | BRF_PRG }, //  0 68k Code
	{ "ik3-3-ver1.c9",  0x20000, 0x10e38b66, 1 | BRF_PRG }, //  1
	{ "ik3-1.c8",       0x10000, 0x47e4d256, 1 | BRF_PRG }, //  2
	{ "ik3-4.c12",      0x10000, 0xa43af6b5, 1 | BRF_PRG }, //  3

	{ "ik3-5.bin",      0x10000, 0xce6706fc, 2 | BRF_PRG }, //  4 Z80 Code

	{ "ik3-7.bin",      0x08000, 0x0b4804df, 3 | BRF_GRA }, //  5 Characters
	{ "ik3-8.bin",      0x08000, 0x10ab4e50, 3 | BRF_GRA }, //  6

	{ "ik3-13.bin",     0x20000, 0x9a56bd32, 4 | BRF_GRA }, //  7 Sprites
	{ "ik3-12.bin",     0x20000, 0x0ce6a10a, 4 | BRF_GRA }, //  8
	{ "ik3-11.bin",     0x20000, 0xe4e2be43, 4 | BRF_GRA }, //  9
	{ "ik3-10.bin",     0x20000, 0xac222372, 4 | BRF_GRA }, // 10
	{ "ik3-9.bin",      0x20000, 0xc33971c2, 4 | BRF_GRA }, // 11
	{ "ik3-14.bin",     0x20000, 0x453bea77, 4 | BRF_GRA }, // 12
	{ "ik3-15.bin",     0x20000, 0x781a81fc, 4 | BRF_GRA }, // 13
	{ "ik3-16.bin",     0x20000, 0x80ba400b, 4 | BRF_GRA }, // 14
	{ "ik3-17.bin",     0x20000, 0x0cc3ce4a, 4 | BRF_GRA }, // 15
	{ "ik3-18.bin",     0x20000, 0xba106245, 4 | BRF_GRA }, // 16
	{ "ik3-23.bin",     0x20000, 0xd0fd5c77, 4 | BRF_GRA }, // 17
	{ "ik3-22.bin",     0x20000, 0x4878d883, 4 | BRF_GRA }, // 18
	{ "ik3-21.bin",     0x20000, 0x50d0fbf0, 4 | BRF_GRA }, // 19
	{ "ik3-20.bin",     0x20000, 0x9a851efc, 4 | BRF_GRA }, // 20
	{ "ik3-19.bin",     0x20000, 0x4ebdba89, 4 | BRF_GRA }, // 21
	{ "ik3-24.bin",     0x20000, 0xe9b26d68, 4 | BRF_GRA }, // 22
	{ "ik3-25.bin",     0x20000, 0x073b03f1, 4 | BRF_GRA }, // 23
	{ "ik3-26.bin",     0x20000, 0x9c613561, 4 | BRF_GRA }, // 24
	{ "ik3-27.bin",     0x20000, 0x16dd227e, 4 | BRF_GRA }, // 25
	{ "ik3-28.bin",     0x20000, 0x711715ae, 4 | BRF_GRA }, // 26

	{ "ik3-6.bin",      0x20000, 0x59d256a4, 5 | BRF_SND }, // 27 upd7759 samples
};

STD_ROM_PICK(ikari3)
STD_ROM_FN(ikari3)

static INT32 ikari3Init()
{
	return DrvInit(3);
}

struct BurnDriver BurnDrvikari3 = {
	"ikari3", NULL, NULL, NULL, "1989",
	"Ikari III - The Rescue (8-Way Joystick)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, ikari3RomInfo, ikari3RomName, NULL, NULL, IkariInputInfo, IkariDIPInfo,
	ikari3Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};

// Ikari III - The Rescue (US, Rotary Joystick)

static struct BurnRomInfo ikari3uRomDesc[] = {
	{ "ik3-2.c10",  0x20000, 0xa7b34dcd, 1 | BRF_PRG }, //  0 68k Code
	{ "ik3-3.c9",   0x20000, 0x50f2b83d, 1 | BRF_PRG }, //  1
	{ "ik3-1.c8",   0x10000, 0x47e4d256, 1 | BRF_PRG }, //  2
	{ "ik3-4.c12",  0x10000, 0xa43af6b5, 1 | BRF_PRG }, //  3

	{ "ik3-5.bin",  0x10000, 0xce6706fc, 2 | BRF_PRG }, //  4 Z80 Code

	{ "ik3-7.bin",  0x08000, 0x0b4804df, 3 | BRF_GRA }, //  5 Characters
	{ "ik3-8.bin",  0x08000, 0x10ab4e50, 3 | BRF_GRA }, //  6

	{ "ik3-13.bin", 0x20000, 0x9a56bd32, 4 | BRF_GRA }, //  7 Sprites
	{ "ik3-12.bin", 0x20000, 0x0ce6a10a, 4 | BRF_GRA }, //  8
	{ "ik3-11.bin", 0x20000, 0xe4e2be43, 4 | BRF_GRA }, //  9
	{ "ik3-10.bin", 0x20000, 0xac222372, 4 | BRF_GRA }, // 10
	{ "ik3-9.bin",  0x20000, 0xc33971c2, 4 | BRF_GRA }, // 11
	{ "ik3-14.bin", 0x20000, 0x453bea77, 4 | BRF_GRA }, // 12
	{ "ik3-15.bin", 0x20000, 0x781a81fc, 4 | BRF_GRA }, // 13
	{ "ik3-16.bin", 0x20000, 0x80ba400b, 4 | BRF_GRA }, // 14
	{ "ik3-17.bin", 0x20000, 0x0cc3ce4a, 4 | BRF_GRA }, // 15
	{ "ik3-18.bin", 0x20000, 0xba106245, 4 | BRF_GRA }, // 16
	{ "ik3-23.bin", 0x20000, 0xd0fd5c77, 4 | BRF_GRA }, // 17
	{ "ik3-22.bin", 0x20000, 0x4878d883, 4 | BRF_GRA }, // 18
	{ "ik3-21.bin", 0x20000, 0x50d0fbf0, 4 | BRF_GRA }, // 19
	{ "ik3-20.bin", 0x20000, 0x9a851efc, 4 | BRF_GRA }, // 20
	{ "ik3-19.bin", 0x20000, 0x4ebdba89, 4 | BRF_GRA }, // 21
	{ "ik3-24.bin", 0x20000, 0xe9b26d68, 4 | BRF_GRA }, // 22
	{ "ik3-25.bin", 0x20000, 0x073b03f1, 4 | BRF_GRA }, // 23
	{ "ik3-26.bin", 0x20000, 0x9c613561, 4 | BRF_GRA }, // 24
	{ "ik3-27.bin", 0x20000, 0x16dd227e, 4 | BRF_GRA }, // 25
	{ "ik3-28.bin", 0x20000, 0x711715ae, 4 | BRF_GRA }, // 26

	{ "ik3-6.bin",  0x20000, 0x59d256a4, 5 | BRF_SND }, // 27 upd7759 samples
};

STD_ROM_PICK(ikari3u)
STD_ROM_FN(ikari3u)

struct BurnDriver BurnDrvikari3u = {
	"ikari3u", "ikari3", NULL, NULL, "1989",
	"Ikari III - The Rescue (US, Rotary Joystick)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, ikari3uRomInfo, ikari3uRomName, NULL, NULL, IkariInputInfo, IkariDIPInfo,
	ikari3Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};

// Ikari Three - The Rescue (Japan, Rotary Joystick)

static struct BurnRomInfo ikari3jRomDesc[] = {
	{ "ik3-2-j.c10",  0x20000, 0x7b1b4be4, 1 | BRF_PRG }, //  0 68k Code
	{ "ik3-3-j.c9",   0x20000, 0x8e6e2aa9, 1 | BRF_PRG }, //  1
	{ "ik3-1.c8",     0x10000, 0x47e4d256, 1 | BRF_PRG }, //  2
	{ "ik3-4.c12",    0x10000, 0xa43af6b5, 1 | BRF_PRG }, //  3

	{ "ik3-5.bin",    0x10000, 0xce6706fc, 2 | BRF_PRG }, //  4 Z80 Code

	{ "ik3-7.bin",    0x08000, 0x0b4804df, 3 | BRF_GRA }, //  5 Characters
	{ "ik3-8.bin",    0x08000, 0x10ab4e50, 3 | BRF_GRA }, //  6

	{ "ik3-13.bin",   0x20000, 0x9a56bd32, 4 | BRF_GRA }, //  7 Sprites
	{ "ik3-12.bin",   0x20000, 0x0ce6a10a, 4 | BRF_GRA }, //  8
	{ "ik3-11.bin",   0x20000, 0xe4e2be43, 4 | BRF_GRA }, //  9
	{ "ik3-10.bin",   0x20000, 0xac222372, 4 | BRF_GRA }, // 10
	{ "ik3-9.bin",    0x20000, 0xc33971c2, 4 | BRF_GRA }, // 11
	{ "ik3-14.bin",   0x20000, 0x453bea77, 4 | BRF_GRA }, // 12
	{ "ik3-15.bin",   0x20000, 0x781a81fc, 4 | BRF_GRA }, // 13
	{ "ik3-16.bin",   0x20000, 0x80ba400b, 4 | BRF_GRA }, // 14
	{ "ik3-17.bin",   0x20000, 0x0cc3ce4a, 4 | BRF_GRA }, // 15
	{ "ik3-18.bin",   0x20000, 0xba106245, 4 | BRF_GRA }, // 16
	{ "ik3-23.bin",   0x20000, 0xd0fd5c77, 4 | BRF_GRA }, // 17
	{ "ik3-22.bin",   0x20000, 0x4878d883, 4 | BRF_GRA }, // 18
	{ "ik3-21.bin",   0x20000, 0x50d0fbf0, 4 | BRF_GRA }, // 19
	{ "ik3-20.bin",   0x20000, 0x9a851efc, 4 | BRF_GRA }, // 20
	{ "ik3-19.bin",   0x20000, 0x4ebdba89, 4 | BRF_GRA }, // 21
	{ "ik3-24.bin",   0x20000, 0xe9b26d68, 4 | BRF_GRA }, // 22
	{ "ik3-25.bin",   0x20000, 0x073b03f1, 4 | BRF_GRA }, // 23
	{ "ik3-26.bin",   0x20000, 0x9c613561, 4 | BRF_GRA }, // 24
	{ "ik3-27.bin",   0x20000, 0x16dd227e, 4 | BRF_GRA }, // 25
	{ "ik3-28.bin",   0x20000, 0x711715ae, 4 | BRF_GRA }, // 26

	{ "ik3-6.bin",    0x20000, 0x59d256a4, 5 | BRF_SND }, // 27 upd7759 samples
};

STD_ROM_PICK(ikari3j)
STD_ROM_FN(ikari3j)

struct BurnDriver BurnDrvikari3j = {
	"ikari3j", "ikari3", NULL, NULL, "1989",
	"Ikari Three - The Rescue (Japan, Rotary Joystick)\0", NULL, "SNK", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, ikari3jRomInfo, ikari3jRomName, NULL, NULL, IkariInputInfo, IkariDIPInfo,
	ikari3Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 256, 224, 4, 3
};
