#include "sys16.h"
#include "bitswap.h"

/*====================================================
Input Defs
====================================================*/

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo OutrunInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 6, "p1 coin"    },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 3, "p1 start"   },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 7, "p2 coin"    },

	A("Steering"         , BIT_ANALOG_REL, &System16AnalogPort0,   "p1 x-axis"  ),
	A("Accelerate"       , BIT_ANALOG_REL, &System16AnalogPort1,   "p1 fire 1"  ),
	A("Brake"            , BIT_ANALOG_REL, &System16AnalogPort2,   "p1 fire 2"  ),
	{"Gear"              , BIT_DIGITAL   , &System16Gear       ,   "p1 fire 3"  },
	
	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 2 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 1 , "diag"      },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Outrun)

static struct BurnInputInfo ShangonInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"    },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 4, "p1 start"   },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"    },

	A("Steering"         , BIT_ANALOG_REL, &System16AnalogPort0,   "p1 x-axis"  ),
	A("Accelerate"       , BIT_ANALOG_REL, &System16AnalogPort1,   "p1 fire 1"  ),
	A("Brake"            , BIT_ANALOG_REL, &System16AnalogPort2,   "p1 fire 2"  ),
	{"Super Charger"     , BIT_DIGITAL   , System16InputPort0 + 5, "p1 fire 3"  },
	
	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Shangon)

static struct BurnInputInfo ToutrunInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 6, "p1 coin"    },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 3, "p1 start"   },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 7, "p2 coin"    },

	A("Steering"         , BIT_ANALOG_REL, &System16AnalogPort0,   "p1 x-axis"  ),
	A("Accelerate"       , BIT_ANALOG_REL, &System16AnalogPort1,   "p1 fire 1"  ),
	A("Brake"            , BIT_ANALOG_REL, &System16AnalogPort2,   "p1 fire 2"  ),
	{"Gear"              , BIT_DIGITAL   , &System16Gear       ,   "p1 fire 3"  },
	{"Nitro"             , BIT_DIGITAL   , System16InputPort0 + 5, "p1 fire 4"  },
	
	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 2 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 1 , "diag"      },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Toutrun)

#undef A

/*====================================================
Dip Defs
====================================================*/

#define OUTRUN_COINAGE(dipval)								\
	{0   , 0xfe, 0   , 16  , "Coin A"                               },			\
	{dipval, 0x01, 0x0f, 0x07, "4 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x08, "3 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x09, "2 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x05, "2 Coins 1 Credit 5/3 6/4"           },			\
	{dipval, 0x01, 0x0f, 0x04, "2 Coins 1 Credit 4/3"               },			\
	{dipval, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x01, "1 Coin  1 Credit 2/3"               },			\
	{dipval, 0x01, 0x0f, 0x02, "1 Coin  1 Credit 4/5"               },			\
	{dipval, 0x01, 0x0f, 0x03, "1 Coin  1 Credit 5/6"               },			\
	{dipval, 0x01, 0x0f, 0x06, "2 Coins 3 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x00, "Free Play (if coin B too) or 1C/1C" },			\
												\
	{0   , 0xfe, 0   , 16  , "Coin B"                               },			\
	{dipval, 0x01, 0xf0, 0x70, "4 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x80, "3 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x90, "2 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x50, "2 Coins 1 Credit 5/3 6/4"           },			\
	{dipval, 0x01, 0xf0, 0x40, "2 Coins 1 Credit 4/3"               },			\
	{dipval, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x10, "1 Coin  1 Credit 2/3"               },			\
	{dipval, 0x01, 0xf0, 0x20, "1 Coin  1 Credit 4/5"               },			\
	{dipval, 0x01, 0xf0, 0x30, "1 Coin  1 Credit 5/6"               },			\
	{dipval, 0x01, 0xf0, 0x60, "2 Coins 3 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0x00, "Free Play (if coin A too) or 1C/1C" },
	
static struct BurnDIPInfo OutrunDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0xff, NULL                                 },
	{0x0b, 0xff, 0xff, 0xf9, NULL                                 },

	// Dip 1
	OUTRUN_COINAGE(0x0a)

	// Dip 2
	{0   , 0xfe, 0   , 3   , "Cabinet"                            },
	{0x0b, 0x01, 0x03, 0x03, "Moving"                             },
	{0x0b, 0x01, 0x03, 0x02, "Up Cockpit"                         },
	{0x0b, 0x01, 0x03, 0x01, "Mini Up"                            },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0b, 0x01, 0x04, 0x04, "Off"                                },
	{0x0b, 0x01, 0x04, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Time Adjust"                        },
	{0x0b, 0x01, 0x30, 0x20, "Easy"                               },
	{0x0b, 0x01, 0x30, 0x30, "Normal"                             },
	{0x0b, 0x01, 0x30, 0x10, "Hard"                               },
	{0x0b, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0b, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x0b, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x0b, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x0b, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Outrun)

static struct BurnDIPInfo OutrundxDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0xff, NULL                                 },
	{0x0b, 0xff, 0xff, 0xfc, NULL                                 },

	// Dip 1
	OUTRUN_COINAGE(0x0a)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x0b, 0x01, 0x01, 0x00, "Not Moving"                         },
	{0x0b, 0x01, 0x01, 0x01, "Moving"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0b, 0x01, 0x02, 0x02, "Off"                                },
	{0x0b, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Time Adjust"                        },
	{0x0b, 0x01, 0x30, 0x20, "Easy"                               },
	{0x0b, 0x01, 0x30, 0x30, "Normal"                             },
	{0x0b, 0x01, 0x30, 0x10, "Hard"                               },
	{0x0b, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0b, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x0b, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x0b, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x0b, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Outrundx)

static struct BurnDIPInfo ShangonDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0xff, NULL                                 },
	{0x0b, 0xff, 0xff, 0x1e, NULL                                 },

	// Dip 1
	OUTRUN_COINAGE(0x0a)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0b, 0x01, 0x01, 0x01, "Off"                                },
	{0x0b, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0b, 0x01, 0x06, 0x04, "Easy"                               },
	{0x0b, 0x01, 0x06, 0x06, "Normal"                             },
	{0x0b, 0x01, 0x06, 0x02, "Hard"                               },
	{0x0b, 0x01, 0x06, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Time Adjust"                        },
	{0x0b, 0x01, 0x18, 0x10, "Easy"                               },
	{0x0b, 0x01, 0x18, 0x18, "Normal"                             },
	{0x0b, 0x01, 0x18, 0x08, "Hard"                               },
	{0x0b, 0x01, 0x18, 0x00, "Hardest"                            },
};

STDDIPINFO(Shangon)

static struct BurnDIPInfo ToutrunDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                                 },
	{0x0c, 0xff, 0xff, 0xd9, NULL                                 },

	// Dip 1
	OUTRUN_COINAGE(0x0b)
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Cabinet"                            },
	{0x0c, 0x01, 0x03, 0x03, "Moving"                             },
	{0x0c, 0x01, 0x03, 0x02, "Cockpit Conversion"                 },
	{0x0c, 0x01, 0x03, 0x01, "Mini Up"                            },
	{0x0c, 0x01, 0x03, 0x00, "Cockpit"                            },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0c, 0x01, 0x04, 0x04, "Off"                                },
	{0x0c, 0x01, 0x04, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Turbo"                              },
	{0x0c, 0x01, 0x08, 0x00, "Use start button"                   },
	{0x0c, 0x01, 0x08, 0x08, "Use turbo shifter"                  },
	
	{0   , 0xfe, 0   , 4   , "Credits"                            },
	{0x0c, 0x01, 0x30, 0x20, "3 to start/2 to continue"           },
	{0x0c, 0x01, 0x30, 0x30, "2 to start/1 to continue"           },
	{0x0c, 0x01, 0x30, 0x10, "1 to start/1 to continue"           },
	{0x0c, 0x01, 0x30, 0x00, "2 to start/2 to continue"           },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0c, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x0c, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x0c, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x0c, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Toutrun)

static struct BurnDIPInfo ToutruncDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                                 },
	{0x0c, 0xff, 0xff, 0xd8, NULL                                 },

	// Dip 1
	OUTRUN_COINAGE(0x0b)
	
	// Dip 2
	{0   , 0xfe, 0   , 1   , "Cabinet"                            },
	{0x0c, 0x01, 0x03, 0x00, "Cockpit"                            },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0c, 0x01, 0x04, 0x04, "Off"                                },
	{0x0c, 0x01, 0x04, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 1   , "Turbo"                              },
	{0x0c, 0x01, 0x08, 0x08, "Use turbo shifter"                  },
	
	{0   , 0xfe, 0   , 4   , "Credits"                            },
	{0x0c, 0x01, 0x30, 0x20, "3 to start/2 to continue"           },
	{0x0c, 0x01, 0x30, 0x30, "2 to start/1 to continue"           },
	{0x0c, 0x01, 0x30, 0x10, "1 to start/1 to continue"           },
	{0x0c, 0x01, 0x30, 0x00, "2 to start/2 to continue"           },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0c, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x0c, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x0c, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x0c, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Toutrunc)

static struct BurnDIPInfo ToutrunmDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                                 },
	{0x0c, 0xff, 0xff, 0xdb, NULL                                 },

	// Dip 1
	OUTRUN_COINAGE(0x0b)
	
	// Dip 2
	{0   , 0xfe, 0   , 1   , "Cabinet"                            },
	{0x0c, 0x01, 0x03, 0x03, "Moving"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0c, 0x01, 0x04, 0x04, "Off"                                },
	{0x0c, 0x01, 0x04, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Turbo"                              },
	{0x0c, 0x01, 0x08, 0x00, "Use start button"                   },
	{0x0c, 0x01, 0x08, 0x08, "Use turbo shifter"                  },
	
	{0   , 0xfe, 0   , 4   , "Credits"                            },
	{0x0c, 0x01, 0x30, 0x20, "3 to start/2 to continue"           },
	{0x0c, 0x01, 0x30, 0x30, "2 to start/1 to continue"           },
	{0x0c, 0x01, 0x30, 0x10, "1 to start/1 to continue"           },
	{0x0c, 0x01, 0x30, 0x00, "2 to start/2 to continue"           },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0c, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x0c, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x0c, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x0c, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Toutrunm)

#undef OUTRUN_COINAGE

/*====================================================
Rom Defs
====================================================*/

static struct BurnRomInfo OutrunRomDesc[] = {
	{ "epr-10380b.133",   0x10000, 0x1f6cadad, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10382b.118",   0x10000, 0xc4c3fa1a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10381b.132",   0x10000, 0xbe8c412b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10383b.117",   0x10000, 0x10a2014a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10327a.76",    0x10000, 0xe28a5baf, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10329a.58",    0x10000, 0xda131c81, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10328a.75",    0x10000, 0xd5ec5e5d, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10330a.57",    0x10000, 0xba9ec82a, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "opr-10268.99",     0x08000, 0x95344b04, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10232.102",    0x08000, 0x776ba1eb, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10267.100",    0x08000, 0xa85bb823, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10231.103",    0x08000, 0x8908bcbf, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10266.101",    0x08000, 0x9f6f1a74, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10230.104",    0x08000, 0x686f5e50, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-10371.9",      0x20000, 0x7cc86208, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10373.10",     0x20000, 0xb0d26ac9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10375.11",     0x20000, 0x59b60bd7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10377.12",     0x20000, 0x17a1b04a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10372.13",     0x20000, 0xb557078c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10374.14",     0x20000, 0x8051e517, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10376.15",     0x20000, 0xf3b8f318, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10378.16",     0x20000, 0xa1062984, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "opr-10186.47",     0x08000, 0x22794426, SYS16_ROM_ROAD | BRF_GRA },
	{ "opr-10185.11",     0x08000, 0x22794426, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10187.88",     0x08000, 0xa10abaa9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-10193.66",     0x08000, 0xbcd10dde, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10192.67",     0x08000, 0x770f1270, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10191.68",     0x08000, 0x20a284ab, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10190.69",     0x08000, 0x7cab70e2, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10189.70",     0x08000, 0x01366b54, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10188.71",     0x08000, 0xbad30ad9, SYS16_ROM_PCMDATA | BRF_SND },
};


STD_ROM_PICK(Outrun)
STD_ROM_FN(Outrun)

static struct BurnRomInfo OutrunoRomDesc[] = {
	{ "epr-10380.133",    0x10000, 0xe339e87a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10382.118",    0x10000, 0x65248dd5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10381.132",    0x10000, 0xbe8c412b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10383.117",    0x10000, 0xdcc586e7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10327.76",     0x10000, 0xda99d855, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10329.58",     0x10000, 0xfe0fa5e2, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10328.75",     0x10000, 0x3c0e9a7f, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10330.57",     0x10000, 0x59786e99, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "opr-10268.99",     0x08000, 0x95344b04, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10232.102",    0x08000, 0x776ba1eb, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10267.100",    0x08000, 0xa85bb823, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10231.103",    0x08000, 0x8908bcbf, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10266.101",    0x08000, 0x9f6f1a74, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10230.104",    0x08000, 0x686f5e50, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10194.26",     0x08000, 0xf0eda3bd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10203.38",     0x08000, 0x8445a622, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10212.52",     0x08000, 0xdee7e731, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10221.66",     0x08000, 0x43431387, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10195.27",     0x08000, 0x0de75cdd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10204.39",     0x08000, 0x5f4b5abb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10213.53",     0x08000, 0x1d1b22f0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10222.67",     0x08000, 0xa254c706, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10196.28",     0x08000, 0x8688bb59, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10205.40",     0x08000, 0x74bd93ca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10214.54",     0x08000, 0x57527e18, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10223.68",     0x08000, 0x3850690e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10197.29",     0x08000, 0x009165a6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10206.41",     0x08000, 0x954542c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10215.55",     0x08000, 0x69be5a6c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10224.69",     0x08000, 0x5cffc346, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10198.30",     0x08000, 0xd894992e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10207.42",     0x08000, 0xca61cea4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10216.56",     0x08000, 0xd394134d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10225.70",     0x08000, 0x0a5d1f2b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10199.31",     0x08000, 0x86376af6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10208.43",     0x08000, 0x6830b7fa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10217.57",     0x08000, 0xbf2c9b76, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10226.71",     0x08000, 0x5a452474, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10200.32",     0x08000, 0x1e5d4f73, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10209.44",     0x08000, 0x5c15419e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10218.58",     0x08000, 0xdb4bdb39, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10227.72",     0x08000, 0xc7def392, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10201.33",     0x08000, 0x1d9d4b9c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10210.45",     0x08000, 0x39422931, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10219.59",     0x08000, 0xe73b9224, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10228.73",     0x08000, 0x25803978, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "opr-10186.47",     0x08000, 0x22794426, SYS16_ROM_ROAD | BRF_GRA },
	{ "opr-10185.11",     0x08000, 0x22794426, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10187.88",     0x08000, 0xa10abaa9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-10193.66",     0x08000, 0xbcd10dde, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10192.67",     0x08000, 0x770f1270, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10191.68",     0x08000, 0x20a284ab, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10190.69",     0x08000, 0x7cab70e2, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10189.70",     0x08000, 0x01366b54, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10188.71",     0x08000, 0xbad30ad9, SYS16_ROM_PCMDATA | BRF_SND },
};


STD_ROM_PICK(Outruno)
STD_ROM_FN(Outruno)

static struct BurnRomInfo OutrundxRomDesc[] = {
	{ "epr-10183.115",    0x08000, 0x3d992396, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10261.130",    0x08000, 0x1d034847, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10184.116",    0x08000, 0x1a73dc46, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10262.131",    0x08000, 0x5386b6b3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10258.117",    0x08000, 0x39408e4f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10263.132",    0x08000, 0xeda65fd6, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10259.118",    0x08000, 0x95100b1a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10264.133",    0x08000, 0xcc94b102, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10173.66",     0x08000, 0x6c2775c0, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10178.86",     0x08000, 0x6d36be05, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10174.67",     0x08000, 0xaae7efad, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10179.87",     0x08000, 0x180fd041, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10175.68",     0x08000, 0x31c76063, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10180.88",     0x08000, 0x4713b264, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10176.69",     0x08000, 0xa7811f90, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10181.89",     0x08000, 0xe009a04d, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	
	{ "opr-10268.99",     0x08000, 0x95344b04, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10232.102",    0x08000, 0x776ba1eb, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10267.100",    0x08000, 0xa85bb823, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10231.103",    0x08000, 0x8908bcbf, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10266.101",    0x08000, 0x9f6f1a74, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10230.104",    0x08000, 0x686f5e50, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10194.26",     0x08000, 0xf0eda3bd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10203.38",     0x08000, 0x8445a622, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10212.52",     0x08000, 0xdee7e731, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10221.66",     0x08000, 0x43431387, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10195.27",     0x08000, 0x0de75cdd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10204.39",     0x08000, 0x5f4b5abb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10213.53",     0x08000, 0x1d1b22f0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10222.67",     0x08000, 0xa254c706, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10196.28",     0x08000, 0x8688bb59, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10205.40",     0x08000, 0x74bd93ca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10214.54",     0x08000, 0x57527e18, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10223.68",     0x08000, 0x3850690e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10197.29",     0x08000, 0x009165a6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10206.41",     0x08000, 0x954542c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10215.55",     0x08000, 0x69be5a6c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10224.69",     0x08000, 0x5cffc346, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10198.30",     0x08000, 0xd894992e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10207.42",     0x08000, 0xca61cea4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10216.56",     0x08000, 0xd394134d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10225.70",     0x08000, 0x0a5d1f2b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10199.31",     0x08000, 0x86376af6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10208.43",     0x08000, 0x6830b7fa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10217.57",     0x08000, 0xbf2c9b76, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10226.71",     0x08000, 0x5a452474, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10200.32",     0x08000, 0x1e5d4f73, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10209.44",     0x08000, 0x5c15419e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10218.58",     0x08000, 0xdb4bdb39, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10227.72",     0x08000, 0xc7def392, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10201.33",     0x08000, 0x1d9d4b9c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10210.45",     0x08000, 0x39422931, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10219.59",     0x08000, 0xe73b9224, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10228.73",     0x08000, 0x25803978, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "opr-10186.47",     0x08000, 0x22794426, SYS16_ROM_ROAD | BRF_GRA },
	{ "opr-10185.11",     0x08000, 0x22794426, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10187.88",     0x08000, 0xa10abaa9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-10193.66",     0x08000, 0xbcd10dde, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10192.67",     0x08000, 0x770f1270, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10191.68",     0x08000, 0x20a284ab, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10190.69",     0x08000, 0x7cab70e2, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10189.70",     0x08000, 0x01366b54, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10188.71",     0x08000, 0xbad30ad9, SYS16_ROM_PCMDATA | BRF_SND },
};


STD_ROM_PICK(Outrundx)
STD_ROM_FN(Outrundx)

static struct BurnRomInfo OutrunraRomDesc[] = {
	{ "epr-10380a.133",   0x10000, 0x434fadbc, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10382a.118",   0x10000, 0x1ddcc04e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10381a.132",   0x10000, 0xbe8c412b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10383a.117",   0x10000, 0xdcc586e7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10327a.76",    0x10000, 0xe28a5baf, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10329a.58",    0x10000, 0xda131c81, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10328a.75",    0x10000, 0xd5ec5e5d, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10330a.57",    0x10000, 0xba9ec82a, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "opr-10268.99",     0x08000, 0x95344b04, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10232.102",    0x08000, 0x776ba1eb, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10267.100",    0x08000, 0xa85bb823, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10231.103",    0x08000, 0x8908bcbf, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10266.101",    0x08000, 0x9f6f1a74, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-10230.104",    0x08000, 0x686f5e50, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-10371.9",      0x20000, 0x7cc86208, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10373.10",     0x20000, 0xb0d26ac9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10375.11",     0x20000, 0x59b60bd7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10377.12",     0x20000, 0x17a1b04a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10372.13",     0x20000, 0xb557078c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10374.14",     0x20000, 0x8051e517, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10376.15",     0x20000, 0xf3b8f318, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10378.16",     0x20000, 0xa1062984, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "opr-10186.47",     0x08000, 0x22794426, SYS16_ROM_ROAD | BRF_GRA },
	{ "opr-10185.11",     0x08000, 0x22794426, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10187.88",     0x08000, 0xa10abaa9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-10193.66",     0x08000, 0xbcd10dde, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10192.67",     0x08000, 0x770f1270, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10191.68",     0x08000, 0x20a284ab, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10190.69",     0x08000, 0x7cab70e2, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10189.70",     0x08000, 0x01366b54, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-10188.71",     0x08000, 0xbad30ad9, SYS16_ROM_PCMDATA | BRF_SND },
};


STD_ROM_PICK(Outrunra)
STD_ROM_FN(Outrunra)

static struct BurnRomInfo OutrunbRomDesc[] = {
	{ "a-10.bin",         0x10000, 0xcddceea2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "a-9.bin",          0x10000, 0x14e97a67, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "a-14.bin",         0x10000, 0x3092d857, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "a-13.bin",         0x10000, 0x30a1c496, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "a-8.bin",          0x10000, 0xd7f5aae0, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "a-7.bin",          0x10000, 0x88c2e78f, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "a-12.bin",         0x10000, 0xd5ec5e5d, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "a-11.bin",         0x10000, 0x74c5fbec, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "a-15.bin",         0x10000, 0x4c489133, SYS16_ROM_TILES | BRF_GRA },
	{ "a-17.bin",         0x10000, 0x899c781d, SYS16_ROM_TILES | BRF_GRA },
	{ "a-16.bin",         0x10000, 0x98dd4d15, SYS16_ROM_TILES | BRF_GRA },
	
	{ "a-18.bin",         0x10000, 0x77377e00, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-20.bin",         0x10000, 0x69ecc975, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-22.bin",         0x10000, 0xb6a8d0e2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-24.bin",         0x10000, 0xd632d8a2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-26.bin",         0x10000, 0x4f784236, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-28.bin",         0x10000, 0xee4f7154, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-30.bin",         0x10000, 0xe9880aa3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-32.bin",         0x10000, 0xdc286dc2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-19.bin",         0x10000, 0x2c0e7277, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-21.bin",         0x10000, 0x54761e57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-23.bin",         0x10000, 0xa00d0676, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-25.bin",         0x10000, 0xda398368, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-27.bin",         0x10000, 0x8d459356, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-29.bin",         0x10000, 0xa8245727, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-31.bin",         0x10000, 0xef7d06fe, SYS16_ROM_SPRITES | BRF_GRA },
	{ "a-33.bin",         0x10000, 0x1222af9f, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "a-2.bin",          0x08000, 0xed5bda9c, SYS16_ROM_ROAD | BRF_GRA },
	{ "a-3.bin",          0x08000, 0x666fe754, SYS16_ROM_ROAD | BRF_GRA },

	{ "a-1.bin",          0x08000, 0x209bb53a, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "a-6.bin",          0x10000, 0x191f98f4, SYS16_ROM_PCMDATA | BRF_SND },
	{ "a-5.bin",          0x10000, 0x374466d0, SYS16_ROM_PCMDATA | BRF_SND },
	{ "a-4.bin",          0x10000, 0x2a27d0b0, SYS16_ROM_PCMDATA | BRF_SND },
};


STD_ROM_PICK(Outrunb)
STD_ROM_FN(Outrunb)

static struct BurnRomInfo ShangonRomDesc[] = {
	{ "epr-10886.133",    0x10000, 0x8be3cd36, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10884.118",    0x10000, 0xcb06150d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10887.132",    0x10000, 0x8d248bb0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10885.117",    0x10000, 0x70795f26, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10792.76",     0x10000, 0x16299d25, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10790.58",     0x10000, 0x2246cbc1, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10793.75",     0x10000, 0xd9525427, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10791.57",     0x10000, 0x5faf4cbe, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-10652.54",     0x08000, 0x260286f9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10651.55",     0x08000, 0xc609ee7b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10650.56",     0x08000, 0xb236a403, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-10794.8",      0x20000, 0x7c958e63, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10798.16",     0x20000, 0x7d58f807, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10795.6",      0x20000, 0xd9d31f8c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10799.14",     0x20000, 0x96d90d3d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10796.4",      0x20000, 0xfb48957c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10800.12",     0x20000, 0xfeaff98e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10797.2",      0x20000, 0x27f2870d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10801.10",     0x20000, 0x12781795, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-10642.47",     0x08000, 0x7836bcc3, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10649c.88",    0x08000, 0xf6c1ce71, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10643.66",     0x08000, 0x06f55364, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10644.67",     0x08000, 0xb41d541d, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10645.68",     0x08000, 0xa60dabff, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10646.69",     0x08000, 0x473cc411, SYS16_ROM_PCMDATA | BRF_SND },
};


STD_ROM_PICK(Shangon)
STD_ROM_FN(Shangon)

static struct BurnRomInfo Shangon2RomDesc[] = {
	{ "epr-10636a.133",   0x10000, 0x74a64f4f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10634a.118",   0x10000, 0x1608cb4a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10637a.132",   0x10000, 0xad6c1308, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10635a.117",   0x10000, 0xa2415595, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10640.76",     0x10000, 0x02be68db, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10638.58",     0x10000, 0xf13e8bee, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10641.75",     0x10000, 0x38c3f808, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10639.57",     0x10000, 0x8cdbcde8, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-10652.54",     0x08000, 0x260286f9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10651.55",     0x08000, 0xc609ee7b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10650.56",     0x08000, 0xb236a403, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10675.8",      0x10000, 0xd6ac012b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10682.16",     0x10000, 0xd9d83250, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10676.7",      0x10000, 0x25ebf2c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10683.15",     0x10000, 0x6365d2e9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10677.6",      0x10000, 0x8a57b8d6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10684.14",     0x10000, 0x3aff8910, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10678.5",      0x10000, 0xaf473098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10685.13",     0x10000, 0x80bafeef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10679.4",      0x10000, 0x03bc4878, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10686.12",     0x10000, 0x274b734e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10680.3",      0x10000, 0x9f0677ed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10687.11",     0x10000, 0x508a4701, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10681.2",      0x10000, 0xb176ea72, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10688.10",     0x10000, 0x42fcd51d, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-10642.47",     0x08000, 0x7836bcc3, SYS16_ROM_ROAD | BRF_GRA },

	{ "ic88",             0x08000, 0x1254efa6, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10643.66",     0x08000, 0x06f55364, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10644.67",     0x08000, 0xb41d541d, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10645.68",     0x08000, 0xa60dabff, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10646.69",     0x08000, 0x473cc411, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "317-0034.key",     0x02000, 0x263ca773, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Shangon2)
STD_ROM_FN(Shangon2)

static struct BurnRomInfo Shangon3RomDesc[] = {
	{ "epr-10789.133",    0x10000, 0x6092c5ce, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10788.118",    0x10000, 0xc3d8a1ea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10637a.132",   0x10000, 0xad6c1308, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10635a.117",   0x10000, 0xa2415595, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10792.76",     0x10000, 0x16299d25, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10790.58",     0x10000, 0x2246cbc1, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10793.75",     0x10000, 0xd9525427, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10791.57",     0x10000, 0x5faf4cbe, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-10652.54",     0x08000, 0x260286f9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10651.55",     0x08000, 0xc609ee7b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10650.56",     0x08000, 0xb236a403, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10675.8",      0x10000, 0xd6ac012b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10682.16",     0x10000, 0xd9d83250, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10676.7",      0x10000, 0x25ebf2c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10683.15",     0x10000, 0x6365d2e9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10677.6",      0x10000, 0x8a57b8d6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10684.14",     0x10000, 0x3aff8910, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10678.5",      0x10000, 0xaf473098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10685.13",     0x10000, 0x80bafeef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10679.4",      0x10000, 0x03bc4878, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10686.12",     0x10000, 0x274b734e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10680.3",      0x10000, 0x9f0677ed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10687.11",     0x10000, 0x508a4701, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10681.2",      0x10000, 0xb176ea72, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10688.10",     0x10000, 0x42fcd51d, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-10642.47",     0x08000, 0x7836bcc3, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10649a.88",    0x08000, 0xbf38330f, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10643.66",     0x08000, 0x06f55364, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10644.67",     0x08000, 0xb41d541d, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10645.68",     0x08000, 0xa60dabff, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10646.69",     0x08000, 0x473cc411, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "317-0034.key",     0x02000, 0x263ca773, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};
	

STD_ROM_PICK(Shangon3)
STD_ROM_FN(Shangon3)

static struct BurnRomInfo ShangonleRomDesc[] = {
	{ "epr-13944.133",    0x10000, 0x989a80db, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13943.118",    0x10000, 0x426e3050, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10899.132",    0x10000, 0xbb3faa37, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10897.117",    0x10000, 0x5f087eb1, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10640.76",     0x10000, 0x02be68db, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10638.58",     0x10000, 0xf13e8bee, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10641.75",     0x10000, 0x38c3f808, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10639.57",     0x10000, 0x8cdbcde8, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-10652.54",     0x08000, 0x260286f9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10651.55",     0x08000, 0xc609ee7b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10650.56",     0x08000, 0xb236a403, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10675.8",      0x10000, 0xd6ac012b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10682.16",     0x10000, 0xd9d83250, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-13945.7",      0x10000, 0xfbb1eef9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-13946.15",     0x10000, 0x03144930, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10677.6",      0x10000, 0x8a57b8d6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10684.14",     0x10000, 0x3aff8910, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10678.5",      0x10000, 0xaf473098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10685.13",     0x10000, 0x80bafeef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10679.4",      0x10000, 0x03bc4878, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10686.12",     0x10000, 0x274b734e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10680.3",      0x10000, 0x9f0677ed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10687.11",     0x10000, 0x508a4701, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10681.2",      0x10000, 0xb176ea72, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10688.10",     0x10000, 0x42fcd51d, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-10642.47",     0x08000, 0x7836bcc3, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10649c.88",    0x08000, 0xf6c1ce71, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10643.66",     0x08000, 0x06f55364, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10644.67",     0x08000, 0xb41d541d, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10645.68",     0x08000, 0xa60dabff, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10646.69",     0x08000, 0x473cc411, SYS16_ROM_PCMDATA | BRF_SND },
};


STD_ROM_PICK(Shangonle)
STD_ROM_FN(Shangonle)

static struct BurnRomInfo ToutrunRomDesc[] = {
	{ "epr-12513.133",    0x10000, 0xae8835a5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12512.118",    0x10000, 0xf90372ad, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12515.132",    0x10000, 0x1f047df4, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12514.117",    0x10000, 0x5539e9c3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12293.131",    0x10000, 0xf4321eea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12292.116",    0x10000, 0x51d98af0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12295.76",     0x10000, 0xd43a3a84, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12294.58",     0x10000, 0x27cdcfd3, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12297.75",     0x10000, 0x1d9b5677, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12296.57",     0x10000, 0x0a513671, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "opr-12323.102",    0x10000, 0x4de43a6f, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12324.103",    0x10000, 0x24607a55, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12325.104",    0x10000, 0x1405137a, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12336.9",      0x20000, 0xdda465c7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12337.10",     0x20000, 0x828233d1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12338.11",     0x20000, 0x46b4b5f4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12339.12",     0x20000, 0x0d7e3bab, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12364.13",     0x20000, 0xa4b83e65, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12365.14",     0x20000, 0x4a80b2a9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12366.15",     0x20000, 0x385cb3ab, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12367.16",     0x20000, 0x4930254a, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12299.47",     0x08000, 0xfc9bc41b, SYS16_ROM_ROAD | BRF_GRA },
	{ "epr-12298.11",     0x08000, 0xfc9bc41b, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-12300.88",     0x10000, 0xe8ff7011, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12301.66",     0x10000, 0x6e78ad15, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12302.67",     0x10000, 0xe72928af, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12303.68",     0x10000, 0x8384205c, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12304.69",     0x10000, 0xe1762ac3, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12305.70",     0x10000, 0xba9ce677, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12306.71",     0x10000, 0xe49249fd, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "317-0118.key",     0x02000, 0x083d7d56, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Toutrun)
STD_ROM_FN(Toutrun)

static struct BurnRomInfo Toutrun1RomDesc[] = {
	{ "epr-12289.133",    0x10000, 0x812fd035, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12288.118",    0x10000, 0x2f1151bb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12291.132",    0x10000, 0x8ca284d2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12290.117",    0x10000, 0x44dbf3cb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12293.131",    0x10000, 0xf4321eea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12292.116",    0x10000, 0x51d98af0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12295.76",     0x10000, 0xd43a3a84, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12294.58",     0x10000, 0x27cdcfd3, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12297.75",     0x10000, 0x1d9b5677, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12296.57",     0x10000, 0x0a513671, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "opr-12323.102",    0x10000, 0x4de43a6f, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12324.103",    0x10000, 0x24607a55, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12325.104",    0x10000, 0x1405137a, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-12307.9",      0x10000, 0x437dcf09, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12308.10",     0x10000, 0x0de70cc2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12309.11",     0x10000, 0xdeb8c242, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12310.12",     0x10000, 0x45cf157e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12311.13",     0x10000, 0xae2bd639, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12312.14",     0x10000, 0x626000e7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12313.15",     0x10000, 0x52870c37, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12314.16",     0x10000, 0x40c461ea, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12315.17",     0x10000, 0x3ff9a3a3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12316.18",     0x10000, 0x8a1e6dc8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12317.19",     0x10000, 0x77e382d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12318.20",     0x10000, 0xd1afdea9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12319.25",     0x10000, 0xdf23baf9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12320.22",     0x10000, 0x7931e446, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12321.23",     0x10000, 0x830bacd4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12322.24",     0x10000, 0x8b812492, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12299.47",     0x08000, 0xfc9bc41b, SYS16_ROM_ROAD | BRF_GRA },
	{ "epr-12298.11",     0x08000, 0xfc9bc41b, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-12300.88",     0x10000, 0xe8ff7011, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12301.66",     0x10000, 0x6e78ad15, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12302.67",     0x10000, 0xe72928af, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12303.68",     0x10000, 0x8384205c, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12304.69",     0x10000, 0xe1762ac3, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12305.70",     0x10000, 0xba9ce677, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12306.71",     0x10000, 0xe49249fd, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "317-0109.key",     0x02000, 0xe12a6e78, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Toutrun1)
STD_ROM_FN(Toutrun1)

static struct BurnRomInfo Toutrun2RomDesc[] = {
	{ "epr-12397.133",    0x10000, 0xe4b57d7d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12396.118",    0x10000, 0x5e7115cb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12399.132",    0x10000, 0x62c77b1b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12398.117",    0x10000, 0x18e34520, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12293.131",    0x10000, 0xf4321eea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12292.116",    0x10000, 0x51d98af0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12295.76",     0x10000, 0xd43a3a84, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12294.58",     0x10000, 0x27cdcfd3, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12297.75",     0x10000, 0x1d9b5677, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12296.57",     0x10000, 0x0a513671, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "opr-12323.102",    0x10000, 0x4de43a6f, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12324.103",    0x10000, 0x24607a55, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12325.104",    0x10000, 0x1405137a, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-12307.9",      0x10000, 0x437dcf09, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12308.10",     0x10000, 0x0de70cc2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12309.11",     0x10000, 0xdeb8c242, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12310.12",     0x10000, 0x45cf157e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12311.13",     0x10000, 0xae2bd639, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12312.14",     0x10000, 0x626000e7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12313.15",     0x10000, 0x52870c37, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12314.16",     0x10000, 0x40c461ea, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12315.17",     0x10000, 0x3ff9a3a3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12316.18",     0x10000, 0x8a1e6dc8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12317.19",     0x10000, 0x77e382d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12318.20",     0x10000, 0xd1afdea9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12319.25",     0x10000, 0xdf23baf9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12320.22",     0x10000, 0x7931e446, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12321.23",     0x10000, 0x830bacd4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12322.24",     0x10000, 0x8b812492, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12299.47",     0x08000, 0xfc9bc41b, SYS16_ROM_ROAD | BRF_GRA },
	{ "epr-12298.11",     0x08000, 0xfc9bc41b, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-12300.88",     0x10000, 0xe8ff7011, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12301.66",     0x10000, 0x6e78ad15, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12302.67",     0x10000, 0xe72928af, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12303.68",     0x10000, 0x8384205c, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12304.69",     0x10000, 0xe1762ac3, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12305.70",     0x10000, 0xba9ce677, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12306.71",     0x10000, 0xe49249fd, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "317-0106.key", 	  0x02000, 0xa4e33916, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Toutrun2)
STD_ROM_FN(Toutrun2)

static struct BurnRomInfo Toutrun3RomDesc[] = {
	{ "epr-12410.133",    0x10000, 0xaa74f3e9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12409.118",    0x10000, 0xc11c8ef7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12412.132",    0x10000, 0xb0534647, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12411.117",    0x10000, 0x12bb0d83, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12293.131",    0x10000, 0xf4321eea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12292.116",    0x10000, 0x51d98af0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12295.76",     0x10000, 0xd43a3a84, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12294.58",     0x10000, 0x27cdcfd3, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12297.75",     0x10000, 0x1d9b5677, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "opr-12296.57",     0x10000, 0x0a513671, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "opr-12323.102",    0x10000, 0x4de43a6f, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12324.103",    0x10000, 0x24607a55, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12325.104",    0x10000, 0x1405137a, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-12307.9",      0x10000, 0x437dcf09, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12308.10",     0x10000, 0x0de70cc2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12309.11",     0x10000, 0xdeb8c242, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12310.12",     0x10000, 0x45cf157e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12311.13",     0x10000, 0xae2bd639, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12312.14",     0x10000, 0x626000e7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12313.15",     0x10000, 0x52870c37, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12314.16",     0x10000, 0x40c461ea, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12315.17",     0x10000, 0x3ff9a3a3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12316.18",     0x10000, 0x8a1e6dc8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12317.19",     0x10000, 0x77e382d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12318.20",     0x10000, 0xd1afdea9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12319.25",     0x10000, 0xdf23baf9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12320.22",     0x10000, 0x7931e446, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12321.23",     0x10000, 0x830bacd4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12322.24",     0x10000, 0x8b812492, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12299.47",     0x08000, 0xfc9bc41b, SYS16_ROM_ROAD | BRF_GRA },
	{ "epr-12298.11",     0x08000, 0xfc9bc41b, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-12300.88",     0x10000, 0xe8ff7011, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12301.66",     0x10000, 0x6e78ad15, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12302.67",     0x10000, 0xe72928af, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12303.68",     0x10000, 0x8384205c, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12304.69",     0x10000, 0xe1762ac3, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12305.70",     0x10000, 0xba9ce677, SYS16_ROM_PCMDATA | BRF_SND },
	{ "opr-12306.71",     0x10000, 0xe49249fd, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "317-0107.key",     0x02000, 0x33e632ae, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Toutrun3)
STD_ROM_FN(Toutrun3)

/*====================================================
Memory Handlers
====================================================*/

void OutrunPPI0WritePortC(UINT8 data)
{
	System16VideoEnable = data & 0x20;
	
	if (!(data & 0x01)) {
		ZetOpen(0);
		ZetReset();
		ZetClose();
	}
}

UINT16 __fastcall OutrunReadWord(UINT32 a)
{
	switch (a) {
		case 0x140060: {
			// watchdog read
			return 0;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Word -> 0x%06X\n"), a);
#endif

	return 0xffff;
}

UINT8 __fastcall OutrunReadByte(UINT32 a)
{
	switch (a) {
		case 0x140001: {
			ppi8255_r(0, (a - 0x140000) >> 1);
		}
		
		case 0x140011: {
			return 0xff - System16Input[0];
		}
		
		case 0x140015: {
			return System16Dip[0];
		}
		
		case 0x140017: {
			return System16Dip[1];
		}
		
		case 0x140031: {
			if (System16ProcessAnalogControlsDo) return System16ProcessAnalogControlsDo(System16AnalogSelect);
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

void __fastcall OutrunWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x100000 && a <= 0x10ffff) {
		System16BTileWordWrite(a - 0x100000, d);
		return;
	}
	
	switch (a) {
		case 0x140070: {
			UINT32 *src = (UINT32 *)System16SpriteRam;
			UINT32 *dst = (UINT32 *)System16SpriteRamBuff;

			/* swap the halves of the sprite RAM */
			for (UINT32 i = 0; i < System16SpriteRamSize/4; i++) {
				UINT32 temp = *src;
				*src++ = *dst;
				*dst++ = temp;
			}
		
			/* hack for thunderblade */
			memset(System16SpriteRam, 0xff, System16SpriteRamSize);
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

void __fastcall OutrunWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x100000 && a <= 0x10ffff) {
		System16BTileByteWrite((a - 0x100000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0x140003:
		case 0x140005:
		case 0x140007: {
			ppi8255_w(0, (a - 0x140000) >> 1, d & 0xff);
		}
		
		case 0x140031: {
			System16AnalogSelect = d;
			return;
		}
		
		case 0x140071: {
			UINT32 *src = (UINT32 *)System16SpriteRam;
			UINT32 *dst = (UINT32 *)System16SpriteRamBuff;

			/* swap the halves of the sprite RAM */
			for (UINT32 i = 0; i < System16SpriteRamSize/4; i++) {
				UINT32 temp = *src;
				*src++ = *dst;
				*dst++ = temp;
			}
		
			/* hack for thunderblade */
			memset(System16SpriteRam, 0xff, System16SpriteRamSize);
			return;
		}
		
		case 0xffff07: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetNmi();
			ZetClose();
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

UINT8 __fastcall Outrun2ReadByte(UINT32 a)
{
	switch (a) {
		case 0x090000:
		case 0x090001: {
			// swap the halves of the road RAM
			memcpy(System16RoadRamBuff, System16RoadRam, 0x1000);
			UINT32 *src = (UINT32 *)System16RoadRamBuff;
			UINT32 *dst = (UINT32 *)System16RoadRam;
			for (INT32 i = 0; i < 0x1000/4; i++) {
				UINT32 temp = *src;
				*src++ = *dst;
				*dst++ = temp;
			}
			
			return System16RoadControl;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 # 2 Read Byte -> 0x%06X\n"), a);
#endif

	return 0;
}

void __fastcall Outrun2WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x090000: {
			System16RoadControl = d & 3;
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 # 2 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

void __fastcall Outrun2WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x090001: {
			System16RoadControl = d & 3;
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 # 2 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

UINT16 __fastcall ShangonReadWord(UINT32 a)
{
	switch (a) {
		case 0x141002: {
			return 0xff - System16Input[0];
		}
		
		case 0x141004: {
			return System16Dip[0];
		}
		
		case 0x141006: {
			return System16Dip[1];
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Word -> 0x%06X\n"), a);
#endif

	return 0xffff;
}

UINT8 __fastcall ShangonReadByte(UINT32 a)
{
	switch (a) {
		case 0x141003: {
			return 0xff - System16Input[0];
		}
		
		case 0x141005: {
			return System16Dip[0];
		}
		
		case 0x141007: {
			return System16Dip[1];
		}
		
		case 0x143021: {
			if (System16ProcessAnalogControlsDo) return System16ProcessAnalogControlsDo(System16AnalogSelect);
		}
		
		case 0x290000: {
			// swap the halves of the road RAM
			memcpy(System16RoadRamBuff, System16RoadRam, 0x1000);
			UINT32 *src = (UINT32 *)System16RoadRamBuff;
			UINT32 *dst = (UINT32 *)System16RoadRam;
			for (INT32 i = 0; i < 0x1000/4; i++) {
				UINT32 temp = *src;
				*src++ = *dst;
				*dst++ = temp;
			}
			
			return System16RoadControl;
		}
		
		case 0xffff05: {
			return System16SoundLatch;
		}
	}
		
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

void __fastcall ShangonWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x100000 && a <= 0x10ffff) {
		System16BTileWordWrite(a - 0x100000, d);
		return;
	}
	
	switch (a) {
		case 0x290000: {
			System16RoadControl = d & 3;
			return;
		}
		
		case 0xffff06: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetNmi();
			nSystem16CyclesDone[2] += ZetRun(200);
			ZetClose();
			return;
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

void __fastcall ShangonWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x100000 && a <= 0x10ffff) {
		System16BTileByteWrite((a - 0x100000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0x140001: {
			System16AnalogSelect = (d >> 6) & 3;
			System16VideoEnable = (d >> 5) & 1;
			return;
		}
		
		case 0x140021: {
			if (!(d & 1)) {
				ZetOpen(0);
				ZetReset();
				ZetClose();
			}
			return;
			
		}
		
		case 0x143001: {
			// watchdog write
			return;
		}
		
		case 0x143021: {
			return;
		}
		
		case 0xffff07: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetNmi();
			nSystem16CyclesDone[2] += ZetRun(200);
			ZetClose();
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

/*====================================================
Driver Inits
====================================================*/

UINT8 OutrunProcessAnalogControls(UINT16 value)
{
	UINT8 temp = 0;
	
	switch (value) {

		// Steering
		case 0: {

			// Prevent CHAR data overflow
			if((System16AnalogPort0 >> 4) > 0x7f && (System16AnalogPort0 >> 4) <= 0x80) {
				temp = 0x80 + 0x7f;
			} else {
				temp = 0x80 + (System16AnalogPort0 >> 4);
			}

			if (temp < 0x20) temp = 0x20;
			if (temp > 0xe0) temp = 0xe0;
			return temp;
		}

		// Accelerate
		case 4: {
			if (System16AnalogPort1 > 1) return 0xff;
			return 0;
		}
		
		// Brake
		case 8: {
			if (System16AnalogPort2 > 1) return 0xff;
			return 0;
		}
	}
	
	return 0;
}

UINT8 ShangonProcessAnalogControls(UINT16 value)
{
	UINT8 temp = 0;
	
	switch (value) {

		// Steering
		case 0: {

			// Prevent CHAR data overflow
			if((System16AnalogPort0 >> 4) < 0xf82 && (System16AnalogPort0 >> 4) > 0x80) {
				temp = (UINT8)(0x80 - 0xf82);
			} else {
				temp = 0x80 - (System16AnalogPort0 >> 4);
			}

			if (temp < 0x20) temp = 0x20;
			if (temp > 0xe0) temp = 0xe0;
			return temp;
		}

		// Accelerate
		case 1: {
			if (System16AnalogPort1 > 1) return 0xff;
			return 0;
		}

		// Brake
		case 2: {
			if (System16AnalogPort2 > 1) return 0xff;
			return 0;
		}
	}
	
	return 0;
}

static INT32 OutrunInit()
{
	System16ProcessAnalogControlsDo = OutrunProcessAnalogControls;
	
	System16HasGears = true;
	
	System16PCMDataSizePreAllocate = 0x60000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x30000);
		memcpy(pTemp, System16PCMData, 0x30000);
		memset(System16PCMData, 0, 0x60000);
		memcpy(System16PCMData + 0x00000, pTemp + 0x00000, 0x8000);
		memcpy(System16PCMData + 0x08000, pTemp + 0x00000, 0x8000);
		memcpy(System16PCMData + 0x10000, pTemp + 0x08000, 0x8000);
		memcpy(System16PCMData + 0x18000, pTemp + 0x08000, 0x8000);
		memcpy(System16PCMData + 0x20000, pTemp + 0x10000, 0x8000);
		memcpy(System16PCMData + 0x28000, pTemp + 0x10000, 0x8000);
		memcpy(System16PCMData + 0x30000, pTemp + 0x18000, 0x8000);
		memcpy(System16PCMData + 0x38000, pTemp + 0x18000, 0x8000);
		memcpy(System16PCMData + 0x40000, pTemp + 0x20000, 0x8000);
		memcpy(System16PCMData + 0x48000, pTemp + 0x20000, 0x8000);
		memcpy(System16PCMData + 0x50000, pTemp + 0x28000, 0x8000);
		memcpy(System16PCMData + 0x58000, pTemp + 0x28000, 0x8000);
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 OutrunbInit()
{
	System16ProcessAnalogControlsDo = OutrunProcessAnalogControls;
	
	System16HasGears = true;
	
	System16PCMDataSizePreAllocate = 0x60000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT16 *word;
		UINT8 *byte;
		INT32 i, length;
		
		/* main CPU: swap bits 11,12 and 6,7 */
		word = (UINT16 *)System16Rom;
		length = System16RomSize / 2;
		for (i = 0; i < length; i++)
			word[i] = BITSWAP16(word[i], 15,14,11,12,13,10,9,8,6,7,5,4,3,2,1,0);
		
		memcpy(System16Code, System16Rom, System16RomSize);
		
		/* sub CPU: swap bits 14,15 and 2,3 */
		word = (UINT16 *)System16Rom2;
		length = System16Rom2Size / 2;
		for (i = 0; i < length; i++)
			word[i] = BITSWAP16(word[i], 14,15,13,12,11,10,9,8,7,6,5,4,2,3,1,0);
			
		/* road gfx */
		/* rom a-2.bin: swap bits 6,7 */
		/* rom a-3.bin: swap bits 5,6 */
		byte = System16Roads;
		length = 0x40000 / 2;
		for (i = 0; i < length; i++)
		{
			byte[i]        = BITSWAP08(byte[i],        6,7,5,4,3,2,1,0);
			byte[i+length] = BITSWAP08(byte[i+length], 7,5,6,4,3,2,1,0);
		}
			
		/* Z80 code: swap bits 5,6 */
		byte = System16Z80Rom;
		length = System16Z80RomSize;
		for (i = 0; i < length; i++) {
			byte[i] = BITSWAP08(byte[i], 7,5,6,4,3,2,1,0);
		}
			
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x30000);
		memcpy(pTemp, System16PCMData, 0x30000);
		memset(System16PCMData, 0, 0x60000);
		memcpy(System16PCMData + 0x00000, pTemp + 0x00000, 0x8000);
		memcpy(System16PCMData + 0x10000, pTemp + 0x08000, 0x8000);
		memcpy(System16PCMData + 0x20000, pTemp + 0x10000, 0x8000);
		memcpy(System16PCMData + 0x30000, pTemp + 0x18000, 0x8000);
		memcpy(System16PCMData + 0x40000, pTemp + 0x20000, 0x8000);
		memcpy(System16PCMData + 0x50000, pTemp + 0x28000, 0x8000);
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 ShangonInit()
{
	System16ProcessAnalogControlsDo = ShangonProcessAnalogControls;
	
	Shangon = true;
	
	System16PCMDataSizePreAllocate = 0x40000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadWordHandler(0, ShangonReadWord);
		SekSetWriteWordHandler(0, ShangonWriteWord);
		SekSetReadByteHandler(0, ShangonReadByte);
		SekSetWriteByteHandler(0, ShangonWriteByte);
		SekClose();
		
		System16RoadColorOffset1 = 0x7f6;
		System16RoadColorOffset2 = 0x7c0;
		System16RoadColorOffset3 = 0x7c0;
		
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x20000);
		memcpy(pTemp, System16PCMData, 0x20000);
		memset(System16PCMData, 0, 0x40000);
		memcpy(System16PCMData + 0x00000, pTemp + 0x00000, 0x8000);
		memcpy(System16PCMData + 0x08000, pTemp + 0x00000, 0x8000);
		memcpy(System16PCMData + 0x10000, pTemp + 0x08000, 0x8000);
		memcpy(System16PCMData + 0x18000, pTemp + 0x08000, 0x8000);
		memcpy(System16PCMData + 0x20000, pTemp + 0x10000, 0x8000);
		memcpy(System16PCMData + 0x28000, pTemp + 0x10000, 0x8000);
		memcpy(System16PCMData + 0x30000, pTemp + 0x18000, 0x8000);
		memcpy(System16PCMData + 0x38000, pTemp + 0x18000, 0x8000);
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Shangon2Init()
{
	System16ProcessAnalogControlsDo = ShangonProcessAnalogControls;
	
	System16PCMDataSizePreAllocate = 0x40000;
	
	return ShangonInit();
}

static INT32 ToutrunInit()
{
	System16ProcessAnalogControlsDo = OutrunProcessAnalogControls;
	
	System16HasGears = true;
	
	System16PCMDataSizePreAllocate = 0x60000;
	
	return System16Init();
}

/*====================================================
Driver Defs
====================================================*/

struct BurnDriver BurnDrvOutrun = {
	"outrun", NULL, NULL, NULL, "1986",
	"Out Run (sitdown/upright, Rev B)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, OutrunRomInfo, OutrunRomName, NULL, NULL, OutrunInputInfo, OutrunDIPInfo,
	OutrunInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvOutruno = {
	"outruno", "outrun", NULL, NULL, "1986",
	"Out Run (sitdown/upright)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, OutrunoRomInfo, OutrunoRomName, NULL, NULL, OutrunInputInfo, OutrundxDIPInfo,
	OutrunInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvOutrundx = {
	"outrundx", "outrun", NULL, NULL, "1986",
	"Out Run (deluxe sitdown)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, OutrundxRomInfo, OutrundxRomName, NULL, NULL, OutrunInputInfo, OutrundxDIPInfo,
	OutrunInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvOutrunra = {
	"outrunra", "outrun", NULL, NULL, "1986",
	"Out Run (sitdown/upright, Rev A)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, OutrunraRomInfo, OutrunraRomName, NULL, NULL, OutrunInputInfo, OutrunDIPInfo,
	OutrunInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvOutrunb = {
	"outrunb", "outrun", NULL, NULL, "1986",
	"Out Run (bootleg)\0", NULL, "bootleg", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, OutrunbRomInfo, OutrunbRomName, NULL, NULL, OutrunInputInfo, OutrunDIPInfo,
	OutrunbInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShangon = {
	"shangon", NULL, NULL, NULL, "1987",
	"Super Hang-On (sitdown/upright, unprotected)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_OUTRUN, GBF_RACING, 0,
	NULL, ShangonRomInfo, ShangonRomName, NULL, NULL, ShangonInputInfo, ShangonDIPInfo,
	ShangonInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShangon2 = {
	"shangon2", "shangon", NULL, NULL, "1987",
	"Super Hang-On (mini ride-on, Rev A, FD1089B 317-0034)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_FD1089B_ENC, GBF_RACING, 0,
	NULL, Shangon2RomInfo, Shangon2RomName, NULL, NULL, ShangonInputInfo, ShangonDIPInfo,
	Shangon2Init, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShangon3 = {
	"shangon3", "shangon", NULL, NULL, "1987",
	"Super Hang-On (sitdown/upright, FD1089B 317-0034)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_FD1089B_ENC, GBF_RACING, 0,
	NULL, Shangon3RomInfo, Shangon3RomName, NULL, NULL, ShangonInputInfo, ShangonDIPInfo,
	Shangon2Init, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShangonle = {
	"shangonle", "shangon", NULL, NULL, "1991",
	"Limited Edition Hang-On\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN, GBF_RACING, 0,
	NULL, ShangonleRomInfo, ShangonleRomName, NULL, NULL, ShangonInputInfo, ShangonDIPInfo,
	ShangonInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvToutrun = {
	"toutrun", NULL, NULL, NULL, "1989",
	"Turbo Out Run (Out Run upgrade, FD1094 317-0118)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, ToutrunRomInfo, ToutrunRomName, NULL, NULL, ToutrunInputInfo, ToutrunDIPInfo,
	ToutrunInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvToutrun1 = {
	"toutrun1", "toutrun", NULL, NULL, "1989",
	"Turbo Out Run (deluxe cockpit, FD1094 317-0109)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, Toutrun1RomInfo, Toutrun1RomName, NULL, NULL, ToutrunInputInfo, ToutrunmDIPInfo,
	ToutrunInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriverD BurnDrvToutrun2 = {
	"toutrun2", "toutrun", NULL, NULL, "1989",
	"Turbo Out Run (cockpit, FD1094 317-0106)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, Toutrun2RomInfo, Toutrun2RomName, NULL, NULL, ToutrunInputInfo, ToutrunDIPInfo,
	ToutrunInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvToutrun3 = {
	"toutrun3", "toutrun", NULL, NULL, "1989",
	"Turbo Out Run (cockpit, FD1094 317-0107)\0", NULL, "Sega", "Out Run",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_OUTRUN | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, Toutrun3RomInfo, Toutrun3RomName, NULL, NULL, ToutrunInputInfo, ToutruncDIPInfo,
	ToutrunInit, System16Exit, OutrunFrame, NULL, System16Scan,
	NULL, 0x3000, 320, 224, 4, 3
};
