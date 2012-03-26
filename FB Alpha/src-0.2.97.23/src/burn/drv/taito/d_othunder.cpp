#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "taito.h"
#include "taito_ic.h"
#include "burn_ym2610.h"
#include "eeprom.h"
#include "burn_gun.h"

static void OthunderDraw();

static bool bUseAsm68KCoreOldValue = false;

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo OthunderInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 coin"        },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 6, "p1 start"       },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p2 coin"        },
	{"Start 2"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p2 start"       },

	A("P1 Gun X"         , BIT_ANALOG_REL, &TaitoAnalogPort0      , "mouse x-axis"   ),
	A("P1 Gun Y"         , BIT_ANALOG_REL, &TaitoAnalogPort1      , "mouse y-axis"   ),
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "mouse button 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "mouse button 2" },
	
	A("P2 Gun X"         , BIT_ANALOG_REL, &TaitoAnalogPort2      , "p2 x-axis"      ),
	A("P2 Gun Y"         , BIT_ANALOG_REL, &TaitoAnalogPort3      , "p2 y-axis"      ),	
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "p2 fire 1"      },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 fire 2"      },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"          },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "service"        },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"            },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"            },
	{"Dip 3"             , BIT_DIPSWITCH , TaitoDip + 0           , "dip"            },
};

STDINPUTINFO(Othunder)

#undef A

static void TC0220IOCMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xff;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;
	
	for (INT32 i = 0; i < 8; i++) {
		TC0220IOCInput[0] -= (TC0220IOCInputPort0[i] & 1) << i;
		TC0220IOCInput[1] -= (TC0220IOCInputPort1[i] & 1) << i;
		TC0220IOCInput[2] -= (TC0220IOCInputPort2[i] & 1) << i;
	}
}

static void OthunderMakeInputs()
{
	TC0220IOCMakeInputs();
	
	BurnGunMakeInputs(0, (INT16)TaitoAnalogPort0, (INT16)TaitoAnalogPort1);
	BurnGunMakeInputs(1, (INT16)TaitoAnalogPort2, (INT16)TaitoAnalogPort3);
}

static struct BurnDIPInfo OthunderDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0xff, NULL                             },
	{0x0f, 0xff, 0xff, 0x7f, NULL                             },
	{0x10, 0xff, 0xff, 0x07, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0e, 0x01, 0x02, 0x00, "Off"                            },
	{0x0e, 0x01, 0x02, 0x02, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0e, 0x01, 0x04, 0x04, "Off"                            },
	{0x0e, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0e, 0x01, 0x08, 0x00, "Off"                            },
	{0x0e, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0e, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0e, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0e, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0e, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0e, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x0e, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x0e, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x0e, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0f, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0f, 0x01, 0x03, 0x03, "Medium"                         },
	{0x0f, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0f, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Magazines/Rockets"              },
	{0x0f, 0x01, 0x0c, 0x0c, "5/3"                            },
	{0x0f, 0x01, 0x0c, 0x08, "6/4"                            },
	{0x0f, 0x01, 0x0c, 0x04, "7/5"                            },
	{0x0f, 0x01, 0x0c, 0x00, "8/6"                            },
	
	{0   , 0xfe, 0   , 4   , "Bullets per Magazine"           },
	{0x0f, 0x01, 0x30, 0x00, "30"                             },
	{0x0f, 0x01, 0x30, 0x10, "35"                             },
	{0x0f, 0x01, 0x30, 0x30, "40"                             },
	{0x0f, 0x01, 0x30, 0x20, "50"                             },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x0f, 0x01, 0x80, 0x00, "English"                        },
	{0x0f, 0x01, 0x80, 0x80, "Japanese"                       },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Stereo Seperation"              },
	{0x10, 0x01, 0x07, 0x07, "Maximum"                        },
	{0x10, 0x01, 0x07, 0x03, "High"                           },
	{0x10, 0x01, 0x07, 0x01, "Medium"                         },
	{0x10, 0x01, 0x07, 0x00, "Low"                            },
};

STDDIPINFO(Othunder)

static struct BurnDIPInfo OthunderjDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0xff, NULL                             },
	{0x0f, 0xff, 0xff, 0x7f, NULL                             },
	{0x10, 0xff, 0xff, 0x07, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0e, 0x01, 0x02, 0x00, "Off"                            },
	{0x0e, 0x01, 0x02, 0x02, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0e, 0x01, 0x04, 0x04, "Off"                            },
	{0x0e, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0e, 0x01, 0x08, 0x00, "Off"                            },
	{0x0e, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0e, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0e, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0e, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0e, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0e, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0e, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0e, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0e, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0f, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0f, 0x01, 0x03, 0x03, "Medium"                         },
	{0x0f, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0f, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Magazines/Rockets"              },
	{0x0f, 0x01, 0x0c, 0x0c, "5/3"                            },
	{0x0f, 0x01, 0x0c, 0x08, "6/4"                            },
	{0x0f, 0x01, 0x0c, 0x04, "7/5"                            },
	{0x0f, 0x01, 0x0c, 0x00, "8/6"                            },
	
	{0   , 0xfe, 0   , 4   , "Bullets per Magazine"           },
	{0x0f, 0x01, 0x30, 0x00, "30"                             },
	{0x0f, 0x01, 0x30, 0x10, "35"                             },
	{0x0f, 0x01, 0x30, 0x30, "40"                             },
	{0x0f, 0x01, 0x30, 0x20, "50"                             },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x0f, 0x01, 0x80, 0x00, "English"                        },
	{0x0f, 0x01, 0x80, 0x80, "Japanese"                       },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Stereo Seperation"              },
	{0x10, 0x01, 0x07, 0x07, "Maximum"                        },
	{0x10, 0x01, 0x07, 0x03, "High"                           },
	{0x10, 0x01, 0x07, 0x01, "Medium"                         },
	{0x10, 0x01, 0x07, 0x00, "Low"                            },
};

STDDIPINFO(Othunderj)

static struct BurnDIPInfo OthunderuDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0xff, NULL                             },
	{0x0f, 0xff, 0xff, 0x7f, NULL                             },
	{0x10, 0xff, 0xff, 0x07, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0e, 0x01, 0x02, 0x00, "Off"                            },
	{0x0e, 0x01, 0x02, 0x02, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0e, 0x01, 0x04, 0x04, "Off"                            },
	{0x0e, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0e, 0x01, 0x08, 0x00, "Off"                            },
	{0x0e, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0e, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0e, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0e, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0e, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0e, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0e, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0e, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0e, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0f, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0f, 0x01, 0x03, 0x03, "Medium"                         },
	{0x0f, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0f, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Magazines/Rockets"              },
	{0x0f, 0x01, 0x0c, 0x0c, "5/3"                            },
	{0x0f, 0x01, 0x0c, 0x08, "6/4"                            },
	{0x0f, 0x01, 0x0c, 0x04, "7/5"                            },
	{0x0f, 0x01, 0x0c, 0x00, "8/6"                            },
	
	{0   , 0xfe, 0   , 4   , "Bullets per Magazine"           },
	{0x0f, 0x01, 0x30, 0x00, "30"                             },
	{0x0f, 0x01, 0x30, 0x10, "35"                             },
	{0x0f, 0x01, 0x30, 0x30, "40"                             },
	{0x0f, 0x01, 0x30, 0x20, "50"                             },
	
	{0   , 0xfe, 0   , 2   , "Continue Price"                 },
	{0x0f, 0x01, 0x40, 0x00, "1 Coin 1 Credit"                },
	{0x0f, 0x01, 0x40, 0x40, "Same as Start"                  },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x0f, 0x01, 0x80, 0x00, "English"                        },
	{0x0f, 0x01, 0x80, 0x80, "Japanese"                       },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Stereo Seperation"              },
	{0x10, 0x01, 0x07, 0x07, "Maximum"                        },
	{0x10, 0x01, 0x07, 0x03, "High"                           },
	{0x10, 0x01, 0x07, 0x01, "Medium"                         },
	{0x10, 0x01, 0x07, 0x00, "Low"                            },
};

STDDIPINFO(Othunderu)

static struct BurnRomInfo OthunderRomDesc[] = {
	{ "b67-20.63",             0x20000, 0x21439ea2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-23.64",             0x20000, 0x789e9daa, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-14.61",             0x20000, 0x7f3dd724, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-15.62",             0x20000, 0xe84f62d0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b67-13.40",             0x10000, 0x2936b4b1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b67-06.66",             0x80000, 0xb9a38d64, BRF_GRA | TAITO_CHARS},
	
	{ "b67-01",                0x80000, 0x81ad9acb, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-02",                0x80000, 0xc20cd2fb, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-03",                0x80000, 0xbc9019ed, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-04",                0x80000, 0x2af4c8af, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b67-05.43",             0x80000, 0x9593e42b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b67-08",                0x80000, 0x458f41fb, BRF_SND | TAITO_YM2610A },
	
	{ "b67-07",                0x80000, 0x4f834357, BRF_SND | TAITO_YM2610B },
	
	{ "eeprom-othunder.bin",   0x00080, 0x3729b844, BRF_PRG | TAITO_DEFAULT_EEPROM },
	
	{ "plhs18p8b-b67-09.ic15", 0x00149, 0x62035487, BRF_OPT },
	{ "pal16l8a-b67-11.ic36",  0x00104, 0x3177fb06, BRF_OPT },
	{ "pal20l8b-b67-12.ic37",  0x00144, 0xa47c2798, BRF_OPT },
	{ "pal20l8b-b67-10.ic33",  0x00144, 0x4ced09c7, BRF_OPT },
};

STD_ROM_PICK(Othunder)
STD_ROM_FN(Othunder)

static struct BurnRomInfo OthunderuRomDesc[] = {
	{ "b67-20-1.63",           0x20000, 0x851a453b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-22-1.64",           0x20000, 0x19480dc0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-14.61",             0x20000, 0x7f3dd724, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-15.62",             0x20000, 0xe84f62d0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b67-13.40",             0x10000, 0x2936b4b1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b67-06.66",             0x80000, 0xb9a38d64, BRF_GRA | TAITO_CHARS},
	
	{ "b67-01",                0x80000, 0x81ad9acb, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-02",                0x80000, 0xc20cd2fb, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-03",                0x80000, 0xbc9019ed, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-04",                0x80000, 0x2af4c8af, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b67-05.43",             0x80000, 0x9593e42b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b67-08",                0x80000, 0x458f41fb, BRF_SND | TAITO_YM2610A },
	
	{ "b67-07",                0x80000, 0x4f834357, BRF_SND | TAITO_YM2610B },
	
	{ "eeprom-othunder.bin",   0x00080, 0x3729b844, BRF_PRG | TAITO_DEFAULT_EEPROM },
	
	{ "plhs18p8b-b67-09.ic15", 0x00149, 0x62035487, BRF_OPT },
	{ "pal16l8a-b67-11.ic36",  0x00104, 0x3177fb06, BRF_OPT },
	{ "pal20l8b-b67-12.ic37",  0x00144, 0xa47c2798, BRF_OPT },
	{ "pal20l8b-b67-10.ic33",  0x00144, 0x4ced09c7, BRF_OPT },
};

STD_ROM_PICK(Othunderu)
STD_ROM_FN(Othunderu)

static struct BurnRomInfo OthunderuoRomDesc[] = {
	{ "b67-20.63",             0x20000, 0x21439ea2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-22.64",             0x20000, 0x0f99ad3c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-14.61",             0x20000, 0x7f3dd724, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-15.62",             0x20000, 0xe84f62d0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b67-13.40",             0x10000, 0x2936b4b1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b67-06.66",             0x80000, 0xb9a38d64, BRF_GRA | TAITO_CHARS},
	
	{ "b67-01",                0x80000, 0x81ad9acb, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-02",                0x80000, 0xc20cd2fb, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-03",                0x80000, 0xbc9019ed, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-04",                0x80000, 0x2af4c8af, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b67-05.43",             0x80000, 0x9593e42b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b67-08",                0x80000, 0x458f41fb, BRF_SND | TAITO_YM2610A },
	
	{ "b67-07",                0x80000, 0x4f834357, BRF_SND | TAITO_YM2610B },
	
	{ "eeprom-othunder.bin",   0x00080, 0x3729b844, BRF_PRG | TAITO_DEFAULT_EEPROM },
	
	{ "plhs18p8b-b67-09.ic15", 0x00149, 0x62035487, BRF_OPT },
	{ "pal16l8a-b67-11.ic36",  0x00104, 0x3177fb06, BRF_OPT },
	{ "pal20l8b-b67-12.ic37",  0x00144, 0xa47c2798, BRF_OPT },
	{ "pal20l8b-b67-10.ic33",  0x00144, 0x4ced09c7, BRF_OPT },
};

STD_ROM_PICK(Othunderuo)
STD_ROM_FN(Othunderuo)

static struct BurnRomInfo OthunderjRomDesc[] = {
	{ "b67-20.63",             0x20000, 0x21439ea2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-21.64",             0x20000, 0x9690fc86, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-14.61",             0x20000, 0x7f3dd724, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b67-15.62",             0x20000, 0xe84f62d0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b67-13.40",             0x10000, 0x2936b4b1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b67-06.66",             0x80000, 0xb9a38d64, BRF_GRA | TAITO_CHARS},
	
	{ "b67-01",                0x80000, 0x81ad9acb, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-02",                0x80000, 0xc20cd2fb, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-03",                0x80000, 0xbc9019ed, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b67-04",                0x80000, 0x2af4c8af, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b67-05.43",             0x80000, 0x9593e42b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b67-08",                0x80000, 0x458f41fb, BRF_SND | TAITO_YM2610A },
	
	{ "b67-07",                0x80000, 0x4f834357, BRF_SND | TAITO_YM2610B },
	
	{ "eeprom-othunder.bin",   0x00080, 0x3729b844, BRF_PRG | TAITO_DEFAULT_EEPROM },
	
	{ "plhs18p8b-b67-09.ic15", 0x00149, 0x62035487, BRF_OPT },
	{ "pal16l8a-b67-11.ic36",  0x00104, 0x3177fb06, BRF_OPT },
	{ "pal20l8b-b67-12.ic37",  0x00144, 0xa47c2798, BRF_OPT },
	{ "pal20l8b-b67-10.ic33",  0x00144, 0x4ced09c7, BRF_OPT },
};

STD_ROM_PICK(Othunderj)
STD_ROM_FN(Othunderj)

static INT32 MemIndex()
{
	UINT8 *Next; Next = TaitoMem;

	Taito68KRom1                   = Next; Next += Taito68KRom1Size;
	TaitoZ80Rom1                   = Next; Next += TaitoZ80Rom1Size;
	TaitoSpriteMapRom              = Next; Next += TaitoSpriteMapRomSize;
	TaitoYM2610ARom                = Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom                = Next; Next += TaitoYM2610BRomSize;
	TaitoDefaultEEProm             = Next; Next += TaitoDefaultEEPromSize;
	
	TaitoRamStart                  = Next;

	Taito68KRam1                   = Next; Next += 0x10000;
	TaitoZ80Ram1                   = Next; Next += 0x02000;
	TaitoSpriteRam                 = Next; Next += 0x00600;
	
	TaitoRamEnd                    = Next;

	TaitoChars                     = Next; Next += TaitoNumChar * TaitoCharWidth * TaitoCharHeight;
	TaitoSpritesA                  = Next; Next += TaitoNumSpriteA * TaitoSpriteAWidth * TaitoSpriteAHeight;
	TaitoPalette                   = (UINT32*)Next; Next += 0x01000 * sizeof(UINT32);

	TaitoMemEnd                    = Next;

	return 0;
}

static INT32 OthunderDoReset()
{
	TaitoDoReset();
	
	return 0;
}

static UINT8 OthunderInputBypassRead(INT32 Offset)
{
	switch (Offset) {
		case 0x03: {
			return (EEPROMRead() & 1)  << 7;
		}
	
		default: {
			return TC0220IOCRead(Offset);
		}
	}
	
	return 0;
}

static void OthunderInputBypassWrite(INT32 Offset, UINT16 Data)
{
	switch (Offset) {
		case 0x03: {
			EEPROMWrite(Data & 0x20, Data & 0x10, Data & 0x40);
			return;
		}
	
		default: {
			TC0220IOCWrite(Offset, Data & 0xff);
		}
	}
}

UINT8 __fastcall Othunder68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x500001: {
			return ~BurnGunReturnX(0);
		}
		
		case 0x500003: {
			return BurnGunReturnY(0);
		}
		
		case 0x500005: {
			return ~BurnGunReturnX(1);
		}
		
		case 0x500007: {
			return BurnGunReturnY(1);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Othunder68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x200000, 0x20ffff)
	
	switch (a) {
		case 0x500001:
		case 0x500003:
		case 0x500005:
		case 0x500007: {
			nTaitoCyclesDone[0] += SekRun(10);
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Othunder68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x090000:
		case 0x090002:
		case 0x090004:
		case 0x090006:
		case 0x090008:
		case 0x09000a:
		case 0x09000c:
		case 0x09000e: {
			return OthunderInputBypassRead((a - 0x090000) >> 1);
		}
		
		case 0x100002: {
			return TC0110PCRWordRead(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Othunder68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x200000, 0x20ffff)
	TC0100SCN0CtrlWordWrite_Map(0x220000)
	
	switch (a) {
		case 0x090000:
		case 0x090002:
		case 0x090004:
		case 0x090006:
		case 0x090008:
		case 0x09000a:
		case 0x09000c:
		case 0x09000e: {
			OthunderInputBypassWrite((a - 0x090000) >> 1, d);
			return;
		}
		
		case 0x100000:
		case 0x100002:
		case 0x100004: {
			TC0110PCRStep1RBSwapWordWrite(0, (a - 0x100000) >> 1, d);
			return;
		}
		
		case 0x300000: {
			TC0140SYTPortWrite(d & 0xff);
			return;
		}
		
		case 0x300002: {
			TC0140SYTCommWrite(d & 0xff);
			return;
		}
		
		case 0x500000:
		case 0x500002:
		case 0x500004:
		case 0x500006: {
			nTaitoCyclesDone[0] += SekRun(10);
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
			return;
		}
		
		case 0x600000:
		case 0x600002: {
			// irq ack
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall OthunderZ80Read(UINT16 a)
{
	switch (a) {
		case 0xe000: {
			return BurnYM2610Read(0);
		}
		
		case 0xe002: {
			return BurnYM2610Read(2);
		}
		
		case 0xe201: {
			return TC0140SYTSlaveCommRead();
		}
		
		case 0xea00: {
			return TaitoDip[0];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall OthunderZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xe000: {
			BurnYM2610Write(0, d);
			return;
		}
		
		case 0xe001: {
			BurnYM2610Write(1, d);
			return;
		}
		
		case 0xe002: {
			BurnYM2610Write(2, d);
			return;
		}
		
		case 0xe003: {
			BurnYM2610Write(3, d);
			return;
		}
		
		case 0xe200: {
			TC0140SYTSlavePortWrite(d);
			return;
		}
		
		case 0xe201: {
			TC0140SYTSlaveCommWrite(d);
			return;
		}
		
		case 0xe400:
		case 0xe401:
		case 0xe402:
		case 0xe403: {
			return;
		}
		
		case 0xe600: {
			// nop
			return;
		}
		
		case 0xee00: {
			// nop
			return;
		}
		
		case 0xf000: {
			// nop
			return;
		}
		
		case 0xf200: {
			TaitoZ80Bank = (d - 1) & 7;
			ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static const eeprom_interface othunder_eeprom_interface = {
	6,				/* address bits */
	16,				/* data bits */
	"0110",			/* read command */
	"0101",			/* write command */
	"0111",			/* erase command */
	"0100000000",	/* lock command */
	"0100111111",	/* unlock command */
	0,
	0
};

static const UINT8 othunder_default_eeprom[128] = {
	0x00, 0x00, 0x00, 0xff, 0x00, 0x01, 0x41, 0x41, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xf0, 0xf0,
	0x00, 0x00, 0x00, 0xff, 0x00, 0x01, 0x41, 0x41, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xf0, 0xf0,
	0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0xf0, 0x00,
	0x00, 0x01, 0x42, 0x85, 0x00, 0x00, 0xf1, 0xe3, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0xf0, 0x00,
	0x00, 0x01, 0x42, 0x85, 0x00, 0x00, 0xf1, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static void OthunderFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 OthunderSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / (16000000 / 4);
}

static double OthunderGetTime()
{
	return (double)ZetTotalCycles() / (16000000 / 4);
}

static INT32 CharPlaneOffsets[4]   = { 0, 1, 2, 3 };
static INT32 CharXOffsets[8]       = { 8, 12, 0, 4, 24, 28, 16, 20 };
static INT32 CharYOffsets[8]       = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 SpritePlaneOffsets[4] = { 0, 8, 16, 24 };
static INT32 SpriteXOffsets[16]    = { 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 SpriteYOffsets[8]     = { 0, 64, 128, 192, 256, 320, 384, 448 };

static void SwitchToMusashi()
{
	if (bBurnUseASMCPUEmulation) {
#if 1 && defined FBA_DEBUG
		bprintf(PRINT_NORMAL, _T("Switching to Musashi 68000 core\n"));
#endif
		bUseAsm68KCoreOldValue = bBurnUseASMCPUEmulation;
		bBurnUseASMCPUEmulation = false;
	}
}

static INT32 OthunderInit()
{
	INT32 nLen;
	
	TaitoCharModulo = 0x100;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 8;
	TaitoCharHeight = 8;
	TaitoCharPlaneOffsets = CharPlaneOffsets;
	TaitoCharXOffsets = CharXOffsets;
	TaitoCharYOffsets = CharYOffsets;
	TaitoNumChar = 0x4000;
	
	TaitoSpriteAModulo = 0x200;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 8;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x8000;
	
	TaitoNum68Ks = 1;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	TaitoNumEEPROM = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 4, 8, 1, NULL);
	TC0110PCRInit(1, 0x1000);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;

	SwitchToMusashi();
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x080000, 0x08ffff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0x200000, 0x20ffff, SM_READ);
	SekMapMemory(TaitoSpriteRam          , 0x400000, 0x4005ff, SM_RAM);
	SekSetReadWordHandler(0, Othunder68KReadWord);
	SekSetWriteWordHandler(0, Othunder68KWriteWord);
	SekSetReadByteHandler(0, Othunder68KReadByte);
	SekSetWriteByteHandler(0, Othunder68KWriteByte);
	SekClose();	
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(OthunderZ80Read);
	ZetSetWriteHandler(OthunderZ80Write);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1               );
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1               );
	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1               );
	ZetMemEnd();
	ZetClose();	
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &OthunderFMIRQHandler, OthunderSynchroniseStream, OthunderGetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetSoundMixMode(1);
	
	EEPROMInit(&othunder_eeprom_interface);
	if (!EEPROMAvailable()) EEPROMFill(TaitoDefaultEEProm, 0, 128);
	
	TaitoMakeInputsFunction = OthunderMakeInputs;
	TaitoDrawFunction = OthunderDraw;
	TaitoIrqLine = 5;
	TaitoFrameInterleave = 100;
	TaitoFlipScreenX = 1;

	nTaitoCyclesTotal[0] = 13000000 / 60;
	nTaitoCyclesTotal[1] = (16000000 / 4) / 60;
	
	BurnGunInit(2, true);
	
	// Reset the driver
	OthunderDoReset();

	return 0;
}

static INT32 OthunderExit()
{
	BurnYM2610SetSoundMixMode(0);
	
	TaitoExit();
	
	// Switch back CPU core if needed
	if (bUseAsm68KCoreOldValue) {
#if 1 && defined FBA_DEBUG
		bprintf(PRINT_NORMAL, _T("Switching back to A68K core\n"));
#endif
		bUseAsm68KCoreOldValue = false;
		bBurnUseASMCPUEmulation = true;
	}
	
	return 0;
}

static void RenderSpriteZoom(INT32 Code, INT32 sx, INT32 sy, INT32 Colour, INT32 xFlip, INT32 yFlip, INT32 xScale, INT32 yScale, UINT8* pSource)
{
	// We can use sprite A for sizes, etc. as only Chase HQ uses sprite B and it has the same sizes and count
	
	UINT8 *SourceBase = pSource + ((Code % TaitoNumSpriteA) * TaitoSpriteAWidth * TaitoSpriteAHeight);
	
	INT32 SpriteScreenHeight = (yScale * TaitoSpriteAHeight + 0x8000) >> 16;
	INT32 SpriteScreenWidth = (xScale * TaitoSpriteAWidth + 0x8000) >> 16;
	
	Colour = 0x10 * (Colour % 0x100);
	
	if (TaitoFlipScreenX) {
		xFlip = !xFlip;
		sx = 320 - sx - (xScale >> 12);
	}	
		
	if (SpriteScreenWidth && SpriteScreenHeight) {
		INT32 dx = (TaitoSpriteAWidth << 16) / SpriteScreenWidth;
		INT32 dy = (TaitoSpriteAHeight << 16) / SpriteScreenHeight;
		
		INT32 ex = sx + SpriteScreenWidth;
		INT32 ey = sy + SpriteScreenHeight;
		
		INT32 xIndexBase;
		INT32 yIndex;
		
		if (xFlip) {
			xIndexBase = (SpriteScreenWidth - 1) * dx;
			dx = -dx;
		} else {
			xIndexBase = 0;
		}
		
		if (yFlip) {
			yIndex = (SpriteScreenHeight - 1) * dy;
			dy = -dy;
		} else {
			yIndex = 0;
		}
		
		if (sx < 0) {
			INT32 Pixels = 0 - sx;
			sx += Pixels;
			xIndexBase += Pixels * dx;
		}
		
		if (sy < 0) {
			INT32 Pixels = 0 - sy;
			sy += Pixels;
			yIndex += Pixels * dy;
		}
		
		if (ex > nScreenWidth) {
			INT32 Pixels = ex - nScreenWidth;
			ex -= Pixels;
		}
		
		if (ey > nScreenHeight) {
			INT32 Pixels = ey - nScreenHeight;
			ey -= Pixels;	
		}
		
		if (ex > sx) {
			INT32 y;
			
			for (y = sy; y < ey; y++) {
				UINT8 *Source = SourceBase + ((yIndex >> 16) * TaitoSpriteAWidth);
				UINT16* pPixel = pTransDraw + (y * nScreenWidth);
				
				INT32 x, xIndex = xIndexBase;
				for (x = sx; x < ex; x++) {
					INT32 c = Source[xIndex >> 16];
					if (c != 0) {
						pPixel[x] = c | Colour;
					}
					xIndex += dx;
				}
				
				yIndex += dy;
			}
		}
	}
}

static void OthunderRenderSprites(INT32 PriorityDraw)
{
	UINT16 *SpriteMap = (UINT16*)TaitoSpriteMapRom;
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	INT32 Offset, Data, Tile, Colour, xFlip, yFlip;
	INT32 x, y, Priority, xCur, yCur;
	INT32 xZoom, yZoom, zx, zy;
	INT32 SpriteChunk, MapOffset, Code, j, k, px, py;
	INT32 BadChunks;
	
	for (Offset = 0x300 - 4; Offset >= 0; Offset -= 4) {
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 1]);
		Priority = (Data & 0x8000) >> 15;
		
		if (Priority != 0 && Priority != 1) bprintf(PRINT_NORMAL, _T("Unused Priority %x\n"), Priority);
		if (Priority != PriorityDraw) continue;
		
		xFlip = (Data & 0x4000) >> 14;
		x = Data & 0x1ff;

		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 3]);
		yFlip = (Data & 0x8000) >> 15;
		Tile = Data & 0x1fff;
		if (!Tile) continue;
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 0]);
		yZoom = (Data & 0xfe00) >> 9;
		y = Data & 0x1ff;
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 2]);
		Colour = (Data & 0xff00) >> 8;
		xZoom = (Data & 0x7f);

		MapOffset = Tile << 5;

		xZoom += 1;
		yZoom += 1;

		y += 3;

		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		BadChunks = 0;
				
		for (SpriteChunk = 0; SpriteChunk < 32; SpriteChunk++) {
			k = SpriteChunk % 4;
			j = SpriteChunk / 4;

			px = xFlip ? (3 - k) : k;
			py = yFlip ? (7 - j) : j;

			Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 2)]);
			Code &= (TaitoNumSpriteA - 1);	
			
			if (Code == 0xffff) {
				BadChunks += 1;
				continue;
			}		

			xCur = x + ((k * xZoom) / 4);
			yCur = y + ((j * yZoom) / 8);

			zx = x + (((k + 1) * xZoom) / 4) - xCur;
			zy = y + (((j + 1) * yZoom) / 8) - yCur;
			
			yCur -= 16;

			RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 13, TaitoSpritesA);
		}

	}
}

static void OthunderDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 1, TaitoChars);
		OthunderRenderSprites(1);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 1, TaitoChars);
		OthunderRenderSprites(1);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}
	
	OthunderRenderSprites(0);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TC0110PCRPalette);
	
	for (INT32 i = 0; i < nBurnGunNumPlayers; i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}
}

static INT32 OthunderFrame()
{
	INT32 nInterleave = TaitoFrameInterleave;

	if (TaitoReset) OthunderDoReset();

	TaitoMakeInputsFunction();
	
	nTaitoCyclesDone[0] = nTaitoCyclesDone[1] = 0;

	SekNewFrame();
	ZetNewFrame();
		
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
		if (i == (TaitoFrameInterleave - 1)) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		ZetOpen(0);
		BurnTimerUpdate(i * (nTaitoCyclesTotal[1] / nInterleave));
		ZetClose();
	}
	
	ZetOpen(0);
	BurnTimerEndFrame(nTaitoCyclesTotal[1]);
	BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();
	
	if (pBurnDraw) TaitoDrawFunction();

	return 0;
}

static INT32 OthunderScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029709;
	}
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TaitoRamStart;
		ba.nLen	  = TaitoRamEnd-TaitoRamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	TaitoICScan(nAction);
	
	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2610Scan(nAction, pnMin);
		
		BurnGunScan();
		
		SCAN_VAR(TaitoInput);
		SCAN_VAR(TaitoZ80Bank);
		SCAN_VAR(nTaitoCyclesDone);
		SCAN_VAR(nTaitoCyclesSegment);
	}
	
	if (nAction & ACB_WRITE) {
		if (TaitoZ80Bank) {
			ZetOpen(0);
			ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetClose();
		}		
	}
	
	return 0;
}

struct BurnDriver BurnDrvOthunder = {
	"othunder", NULL, NULL, NULL, "1988",
	"Operation Thunderbolt (World)\0", NULL, "Taito Corporation Japan", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_SHOOT, 0,
	NULL, OthunderRomInfo, OthunderRomName, NULL, NULL, OthunderInputInfo, OthunderDIPInfo,
	OthunderInit, OthunderExit, OthunderFrame, NULL, OthunderScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvOthunderu = {
	"othunderu", "othunder", NULL, NULL, "1988",
	"Operation Thunderbolt (US)\0", NULL, "Taito America Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_SHOOT, 0,
	NULL, OthunderuRomInfo, OthunderuRomName, NULL, NULL, OthunderInputInfo, OthunderuDIPInfo,
	OthunderInit, OthunderExit, OthunderFrame, NULL, OthunderScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvOthunderuo = {
	"othunderuo", "othunder", NULL, NULL, "1988",
	"Operation Thunderbolt (US, older)\0", NULL, "Taito America Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_SHOOT, 0,
	NULL, OthunderuoRomInfo, OthunderuoRomName, NULL, NULL, OthunderInputInfo, OthunderuDIPInfo,
	OthunderInit, OthunderExit, OthunderFrame, NULL, OthunderScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvOthunderj = {
	"othunderj", "othunder", NULL, NULL, "1988",
	"Operation Thunderbolt (Japan)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_SHOOT, 0,
	NULL, OthunderjRomInfo, OthunderjRomName, NULL, NULL, OthunderInputInfo, OthunderjDIPInfo,
	OthunderInit, OthunderExit, OthunderFrame, NULL, OthunderScan,
	NULL, 0x1000, 320, 240, 4, 3
};
