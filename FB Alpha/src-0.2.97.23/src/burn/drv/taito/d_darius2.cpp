#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "taito.h"
#include "taito_ic.h"
#include "burn_ym2610.h"

static INT32 Ninjaw;

static double Darius2YM2610Route1MasterVol;
static double Darius2YM2610Route2MasterVol;

static void Darius2Draw();
static void Darius2dDraw();
static void WarriorbDraw();

static struct BurnInputInfo Darius2InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 6, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 6, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 7, "p2 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "tilt"      },
	{"Freeze"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "freeze"    },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Darius2)

static struct BurnInputInfo Darius2dInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 6, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 6, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 7, "p2 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "tilt"      },
	{"Freeze"            , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "freeze"    },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Darius2d)

static struct BurnInputInfo WarriorbInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort2 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort2 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 6, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 5, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 6, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort2 + 6, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort2 + 7, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p2 fire 3" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "tilt"      },
	{"Freeze"            , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "freeze"    },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Warriorb)

static void Darius2MakeInputs()
{
	TC0220IOCInput[0] = 0xe2;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;
	
	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] |= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] |= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] |= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] |= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;
	
	for (INT32 i = 0; i < 8; i++) {
		TC0220IOCInput[1] -= (TC0220IOCInputPort1[i] & 1) << i;
		TC0220IOCInput[2] -= (TC0220IOCInputPort2[i] & 1) << i;
	}
}

static void Darius2dMakeInputs()
{
	TC0220IOCInput[0] = 0xff;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xf7;
	
	if (TC0220IOCInputPort2[0]) TC0220IOCInput[2] -= 0x01;
	if (TC0220IOCInputPort2[1]) TC0220IOCInput[2] -= 0x02;
	if (TC0220IOCInputPort2[2]) TC0220IOCInput[2] -= 0x04;
	if (TC0220IOCInputPort2[3]) TC0220IOCInput[2] |= 0x08;
	if (TC0220IOCInputPort2[4]) TC0220IOCInput[2] -= 0x10;
	if (TC0220IOCInputPort2[5]) TC0220IOCInput[2] -= 0x20;
	if (TC0220IOCInputPort2[6]) TC0220IOCInput[2] -= 0x40;
	if (TC0220IOCInputPort2[7]) TC0220IOCInput[2] -= 0x80;
	
	for (INT32 i = 0; i < 8; i++) {
		TC0220IOCInput[0] -= (TC0220IOCInputPort0[i] & 1) << i;
		TC0220IOCInput[1] -= (TC0220IOCInputPort1[i] & 1) << i;
	}
}

static void WarriorbMakeInputs()
{
	TC0510NIOInput[0] = 0xff;
	TC0510NIOInput[1] = 0xff;
	TC0510NIOInput[2] = 0xf7;
	
	if (TC0510NIOInputPort2[0]) TC0510NIOInput[2] -= 0x01;
	if (TC0510NIOInputPort2[1]) TC0510NIOInput[2] -= 0x02;
	if (TC0510NIOInputPort2[2]) TC0510NIOInput[2] -= 0x04;
	if (TC0510NIOInputPort2[3]) TC0510NIOInput[2] |= 0x08;
	if (TC0510NIOInputPort2[4]) TC0510NIOInput[2] -= 0x10;
	if (TC0510NIOInputPort2[5]) TC0510NIOInput[2] -= 0x20;
	if (TC0510NIOInputPort2[6]) TC0510NIOInput[2] -= 0x40;
	if (TC0510NIOInputPort2[7]) TC0510NIOInput[2] -= 0x80;
	
	for (INT32 i = 0; i < 8; i++) {
		TC0510NIOInput[0] -= (TC0510NIOInputPort0[i] & 1) << i;
		TC0510NIOInput[1] -= (TC0510NIOInputPort1[i] & 1) << i;
	}
}

static struct BurnDIPInfo Darius2DIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xfe, NULL                             },
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Auto Fire"                      },
	{0x14, 0x01, 0x02, 0x02, "Normal"                         },
	{0x14, 0x01, 0x02, 0x00, "Fast"                           },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x14, 0x01, 0x04, 0x04, "Off"                            },
	{0x14, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x14, 0x01, 0x08, 0x00, "Off"                            },
	{0x14, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x15, 0x01, 0x03, 0x02, "Easy"                           },
	{0x15, 0x01, 0x03, 0x03, "Medium"                         },
	{0x15, 0x01, 0x03, 0x01, "Hard"                           },
	{0x15, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x15, 0x01, 0x0c, 0x0c, "every 700k"                     },
	{0x15, 0x01, 0x0c, 0x08, "every 800k"                     },
	{0x15, 0x01, 0x0c, 0x04, "every 900k"                     },
	{0x15, 0x01, 0x0c, 0x00, "every 1000K"                    },
	
	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x15, 0x01, 0x30, 0x30, "3"                              },
	{0x15, 0x01, 0x30, 0x20, "4"                              },
	{0x15, 0x01, 0x30, 0x10, "5"                              },
	{0x15, 0x01, 0x30, 0x00, "6"                              },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"                },
	{0x15, 0x01, 0x40, 0x40, "Off"                            },
	{0x15, 0x01, 0x40, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x15, 0x01, 0x80, 0x00, "Off"                            },
	{0x15, 0x01, 0x80, 0x80, "On"                             },
};

STDDIPINFO(Darius2)

static struct BurnDIPInfo Darius2dDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xfe, NULL                             },
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Auto Fire"                      },
	{0x14, 0x01, 0x02, 0x02, "Normal"                         },
	{0x14, 0x01, 0x02, 0x00, "Fast"                           },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x14, 0x01, 0x04, 0x04, "Off"                            },
	{0x14, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x14, 0x01, 0x08, 0x00, "Off"                            },
	{0x14, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x15, 0x01, 0x03, 0x02, "Easy"                           },
	{0x15, 0x01, 0x03, 0x03, "Medium"                         },
	{0x15, 0x01, 0x03, 0x01, "Hard"                           },
	{0x15, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x15, 0x01, 0x0c, 0x0c, "every 700k"                     },
	{0x15, 0x01, 0x0c, 0x08, "every 800k"                     },
	{0x15, 0x01, 0x0c, 0x04, "every 900k"                     },
	{0x15, 0x01, 0x0c, 0x00, "every 1000K"                    },
	
	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x15, 0x01, 0x30, 0x30, "3"                              },
	{0x15, 0x01, 0x30, 0x20, "4"                              },
	{0x15, 0x01, 0x30, 0x10, "5"                              },
	{0x15, 0x01, 0x30, 0x00, "6"                              },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x15, 0x01, 0x80, 0x00, "Off"                            },
	{0x15, 0x01, 0x80, 0x80, "On"                             },
};

STDDIPINFO(Darius2d)

static struct BurnDIPInfo NinjawDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xff, NULL                             },
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x01, 0x00, "Off"                            },
	{0x14, 0x01, 0x01, 0x01, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Auto Fire"                      },
	{0x14, 0x01, 0x02, 0x02, "Normal"                         },
	{0x14, 0x01, 0x02, 0x00, "Fast"                           },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x14, 0x01, 0x04, 0x04, "Off"                            },
	{0x14, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x14, 0x01, 0x08, 0x00, "Off"                            },
	{0x14, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x15, 0x01, 0x03, 0x02, "Easy"                           },
	{0x15, 0x01, 0x03, 0x03, "Medium"                         },
	{0x15, 0x01, 0x03, 0x01, "Hard"                           },
	{0x15, 0x01, 0x03, 0x00, "Hardest"                        },	
};

STDDIPINFO(Ninjaw)

static struct BurnDIPInfo NinjawjDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xff, NULL                             },
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x01, 0x00, "Off"                            },
	{0x14, 0x01, 0x01, 0x01, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Auto Fire"                      },
	{0x14, 0x01, 0x02, 0x02, "Normal"                         },
	{0x14, 0x01, 0x02, 0x00, "Fast"                           },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x14, 0x01, 0x04, 0x04, "Off"                            },
	{0x14, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x14, 0x01, 0x08, 0x00, "Off"                            },
	{0x14, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x14, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x14, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x14, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x14, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x15, 0x01, 0x03, 0x02, "Easy"                           },
	{0x15, 0x01, 0x03, 0x03, "Medium"                         },
	{0x15, 0x01, 0x03, 0x01, "Hard"                           },
	{0x15, 0x01, 0x03, 0x00, "Hardest"                        },	
};

STDDIPINFO(Ninjawj)

static struct BurnDIPInfo WarriorbDIPList[]=
{
	// Default Values
	{0x16, 0xff, 0xff, 0xff, NULL                             },
	{0x17, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Vitality Recovery"              },
	{0x16, 0x01, 0x03, 0x02, "Less"                           },
	{0x16, 0x01, 0x03, 0x03, "Normal"                         },
	{0x16, 0x01, 0x03, 0x01, "More"                           },
	{0x16, 0x01, 0x03, 0x00, "Most"                           },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x16, 0x01, 0x04, 0x04, "Off"                            },
	{0x16, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x16, 0x01, 0x08, 0x00, "Off"                            },
	{0x16, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x16, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x16, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x16, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x16, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x16, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x16, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x16, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x16, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x17, 0x01, 0x03, 0x02, "Easy"                           },
	{0x17, 0x01, 0x03, 0x03, "Medium"                         },
	{0x17, 0x01, 0x03, 0x01, "Hard"                           },
	{0x17, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 2   , "Gold Sheep at"                  },
	{0x17, 0x01, 0x04, 0x04, "50k only"                       },
	{0x17, 0x01, 0x04, 0x00, "50k, then every 70k"            },
	
	{0   , 0xfe, 0   , 2   , "Magic Energy Loss"              },
	{0x17, 0x01, 0x08, 0x08, "Always Player"                  },
	{0x17, 0x01, 0x08, 0x00, "Player or Magician"             },
	
	{0   , 0xfe, 0   , 2   , "Player Starting Strength"       },
	{0x17, 0x01, 0x10, 0x10, "Normal"                         },
	{0x17, 0x01, 0x10, 0x00, "Full"                           },
	
	{0   , 0xfe, 0   , 2   , "Magician appears"               },
	{0x17, 0x01, 0x20, 0x20, "When you get a crystal"         },
	{0x17, 0x01, 0x20, 0x00, "Always"                         },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x17, 0x01, 0x40, 0x00, "Off"                            },
	{0x17, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Rounds"                         },
	{0x17, 0x01, 0x80, 0x80, "Normal (10-14, depends on skill"},
	{0x17, 0x01, 0x80, 0x00, "Long (14)"                      },
};

STDDIPINFO(Warriorb)

static struct BurnRomInfo Darius2RomDesc[] = {
	{ "c07-32-1",      0x10000, 0x216c8f6a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-29-1",      0x10000, 0x48de567f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-31-1",      0x10000, 0x8279d2f8, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-30-1",      0x10000, 0x6122e400, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-27",        0x20000, 0x0a6f7b6c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-25",        0x20000, 0x059f40ce, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-26",        0x20000, 0x1f411242, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-24",        0x20000, 0x486c9c20, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c07-35-1",      0x10000, 0xdd8c4723, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-38-1",      0x10000, 0x46afb85c, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-34-1",      0x10000, 0x296984b8, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-37-1",      0x10000, 0x8b7d461f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-33-1",      0x10000, 0x2da03a3f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-36-1",      0x10000, 0x02cf2b1c, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },

	{ "c07-28",        0x20000, 0xda304bc5, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c07-03.12",     0x80000, 0x189bafce, BRF_GRA | TAITO_CHARS },
	{ "c07-04.11",     0x80000, 0x50421e81, BRF_GRA | TAITO_CHARS },
	
	{ "c07-01",        0x80000, 0x3cf0f050, BRF_GRA | TAITO_SPRITESA },
	{ "c07-02",        0x80000, 0x75d16d4b, BRF_GRA | TAITO_SPRITESA },
	
	{ "c07-10.95",     0x80000, 0x4bbe0ed9, BRF_SND | TAITO_YM2610A },
	{ "c07-11.96",     0x80000, 0x3c815699, BRF_SND | TAITO_YM2610A },
	
	{ "c07-12.107",    0x80000, 0xe0b71258, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Darius2)
STD_ROM_FN(Darius2)

static struct BurnRomInfo Darius2dRomDesc[] = {
	{ "c07_20-2.74",   0x20000, 0xa0f345b8, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07_19-2.73",   0x20000, 0x925412c6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07_21-2.76",   0x20000, 0xbdd60e37, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07_18-2.71",   0x20000, 0x23fcd89b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-09.75",     0x80000, 0xcc69c2ce, BRF_ESS | BRF_PRG | TAITO_68KROM1          },

	{ "c07_17.69",     0x20000, 0xae16c905, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c07-03.12",     0x80000, 0x189bafce, BRF_GRA | TAITO_CHARS },
	{ "c07-04.11",     0x80000, 0x50421e81, BRF_GRA | TAITO_CHARS },
	
	{ "c07-06.27",     0x80000, 0x5eebbcd6, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c07-05.24",     0x80000, 0xfb6d0550, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c07-08.25",     0x80000, 0xa07dc846, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c07-07.26",     0x80000, 0xfd9f9e74, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c07-10.95",     0x80000, 0x4bbe0ed9, BRF_SND | TAITO_YM2610A },
	{ "c07-11.96",     0x80000, 0x3c815699, BRF_SND | TAITO_YM2610A },
	
	{ "c07-12.107",    0x80000, 0xe0b71258, BRF_SND | TAITO_YM2610B },
	
	{ "c07-13.37",     0x00400, 0x3ca18eb3, BRF_OPT },
	{ "c07-14.38",     0x00400, 0xbaf2a193, BRF_OPT },
};

STD_ROM_PICK(Darius2d)
STD_ROM_FN(Darius2d)

static struct BurnRomInfo Darius2doRomDesc[] = {
	{ "c07_20-1.74",   0x20000, 0x48b0804a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07_19-1.73",   0x20000, 0x1f9a4f83, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07_21-1.76",   0x20000, 0xb491b0ca, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07_18-1.71",   0x20000, 0xc552e42f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-09.75",     0x80000, 0xcc69c2ce, BRF_ESS | BRF_PRG | TAITO_68KROM1          },

	{ "c07_17.69",     0x20000, 0xae16c905, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c07-03.12",     0x80000, 0x189bafce, BRF_GRA | TAITO_CHARS },
	{ "c07-04.11",     0x80000, 0x50421e81, BRF_GRA | TAITO_CHARS },
	
	{ "c07-06.27",     0x80000, 0x5eebbcd6, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c07-05.24",     0x80000, 0xfb6d0550, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c07-08.25",     0x80000, 0xa07dc846, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c07-07.26",     0x80000, 0xfd9f9e74, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c07-10.95",     0x80000, 0x4bbe0ed9, BRF_SND | TAITO_YM2610A },
	{ "c07-11.96",     0x80000, 0x3c815699, BRF_SND | TAITO_YM2610A },
	
	{ "c07-12.107",    0x80000, 0xe0b71258, BRF_SND | TAITO_YM2610B },
	
	{ "c07-13.37",     0x00400, 0x3ca18eb3, BRF_OPT },
	{ "c07-14.38",     0x00400, 0xbaf2a193, BRF_OPT },
};

STD_ROM_PICK(Darius2do)
STD_ROM_FN(Darius2do)

static struct BurnRomInfo NinjawRomDesc[] = {
	{ "b31_45.35",     0x10000, 0x107902c3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_47.32",     0x10000, 0xbd536b1e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_29.34",     0x10000, 0xf2941a37, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_27.31",     0x10000, 0x2f3ff642, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_41.5",      0x20000, 0x0daef28a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_39.2",      0x20000, 0xe9197c3c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_40.6",      0x20000, 0x2ce0f24e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_38.3",      0x20000, 0xbc68cd99, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b31_33.87",     0x10000, 0x6ce9af44, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_36.97",     0x10000, 0xba20b0d4, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_32.86",     0x10000, 0xe6025fec, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_35.96",     0x10000, 0x70d9a89f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_31.85",     0x10000, 0x837f47e2, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_34.95",     0x10000, 0xd6b5fb2a, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },

	{ "b31_37.11",     0x20000, 0x0ca5799d, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b31-01.23",     0x80000, 0x8e8237a7, BRF_GRA | TAITO_CHARS },
	{ "b31-02.24",     0x80000, 0x4c3b4e33, BRF_GRA | TAITO_CHARS },
	
	{ "b31-07.176",    0x80000, 0x33568cdb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-06.175",    0x80000, 0x0d59439e, BRF_GRA | TAITO_SPRITESA },
	{ "b31-05.174",    0x80000, 0x0a1fc9fb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-04.173",    0x80000, 0x2e1e4cb5, BRF_GRA | TAITO_SPRITESA },
	
	{ "b31-09.18",     0x80000, 0x60a73382, BRF_SND | TAITO_YM2610A },
	{ "b31-10.17",     0x80000, 0xc6434aef, BRF_SND | TAITO_YM2610A },
	{ "b31-11.16",     0x80000, 0x8da531d4, BRF_SND | TAITO_YM2610A },
	
	{ "b31-08.19",     0x80000, 0xa0a1f87d, BRF_SND | TAITO_YM2610B },
	
	{ "b31-25.38",     0x00200, 0xa0b4ba48, BRF_OPT },
	{ "b31-26.58",     0x00200, 0x13e5fe15, BRF_OPT },
};

STD_ROM_PICK(Ninjaw)
STD_ROM_FN(Ninjaw)

static struct BurnRomInfo NinjawjRomDesc[] = {
	{ "b31_30.35",     0x10000, 0x056edd9f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_28.32",     0x10000, 0xcfa7661c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_29.34",     0x10000, 0xf2941a37, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_27.31",     0x10000, 0x2f3ff642, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_41.5",      0x20000, 0x0daef28a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_39.2",      0x20000, 0xe9197c3c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_40.6",      0x20000, 0x2ce0f24e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_38.3",      0x20000, 0xbc68cd99, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b31_33.87",     0x10000, 0x6ce9af44, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_36.97",     0x10000, 0xba20b0d4, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_32.86",     0x10000, 0xe6025fec, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_35.96",     0x10000, 0x70d9a89f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_31.85",     0x10000, 0x837f47e2, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_34.95",     0x10000, 0xd6b5fb2a, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },

	{ "b31_37.11",     0x20000, 0x0ca5799d, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b31-01.23",     0x80000, 0x8e8237a7, BRF_GRA | TAITO_CHARS },
	{ "b31-02.24",     0x80000, 0x4c3b4e33, BRF_GRA | TAITO_CHARS },
	
	{ "b31-07.176",    0x80000, 0x33568cdb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-06.175",    0x80000, 0x0d59439e, BRF_GRA | TAITO_SPRITESA },
	{ "b31-05.174",    0x80000, 0x0a1fc9fb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-04.173",    0x80000, 0x2e1e4cb5, BRF_GRA | TAITO_SPRITESA },
	
	{ "b31-09.18",     0x80000, 0x60a73382, BRF_SND | TAITO_YM2610A },
	{ "b31-10.17",     0x80000, 0xc6434aef, BRF_SND | TAITO_YM2610A },
	{ "b31-11.16",     0x80000, 0x8da531d4, BRF_SND | TAITO_YM2610A },
	
	{ "b31-08.19",     0x80000, 0xa0a1f87d, BRF_SND | TAITO_YM2610B },
	
	{ "b31-25.38",     0x00200, 0xa0b4ba48, BRF_OPT },
	{ "b31-26.58",     0x00200, 0x13e5fe15, BRF_OPT },
};

STD_ROM_PICK(Ninjawj)
STD_ROM_FN(Ninjawj)

static struct BurnRomInfo NinjawuRomDesc[] = {
	{ "b31_49.35",     0x10000, 0xd38b6391, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_48.32",     0x10000, 0x4b5bb3d8, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_29.34",     0x10000, 0xf2941a37, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_27.31",     0x10000, 0x2f3ff642, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_41.5",      0x20000, 0x0daef28a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_39.2",      0x20000, 0xe9197c3c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_40.6",      0x20000, 0x2ce0f24e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_38.3",      0x20000, 0xbc68cd99, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b31_33.87",     0x10000, 0x6ce9af44, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_36.97",     0x10000, 0xba20b0d4, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_32.86",     0x10000, 0xe6025fec, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_35.96",     0x10000, 0x70d9a89f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_31.85",     0x10000, 0x837f47e2, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_34.95",     0x10000, 0xd6b5fb2a, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },

	{ "b31_37.11",     0x20000, 0x0ca5799d, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b31-01.23",     0x80000, 0x8e8237a7, BRF_GRA | TAITO_CHARS },
	{ "b31-02.24",     0x80000, 0x4c3b4e33, BRF_GRA | TAITO_CHARS },
	
	{ "b31-07.176",    0x80000, 0x33568cdb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-06.175",    0x80000, 0x0d59439e, BRF_GRA | TAITO_SPRITESA },
	{ "b31-05.174",    0x80000, 0x0a1fc9fb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-04.173",    0x80000, 0x2e1e4cb5, BRF_GRA | TAITO_SPRITESA },
	
	{ "b31-09.18",     0x80000, 0x60a73382, BRF_SND | TAITO_YM2610A },
	{ "b31-10.17",     0x80000, 0xc6434aef, BRF_SND | TAITO_YM2610A },
	{ "b31-11.16",     0x80000, 0x8da531d4, BRF_SND | TAITO_YM2610A },
	
	{ "b31-08.19",     0x80000, 0xa0a1f87d, BRF_SND | TAITO_YM2610B },
	
	{ "b31-25.38",     0x00200, 0xa0b4ba48, BRF_OPT },
	{ "b31-26.58",     0x00200, 0x13e5fe15, BRF_OPT },
};

STD_ROM_PICK(Ninjawu)
STD_ROM_FN(Ninjawu)

static struct BurnRomInfo WarriorbRomDesc[] = {
	{ "d24_20-1.74",   0x040000, 0x4452dc25, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d24_19-1.73",   0x040000, 0x15c16016, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d24_21-1.76",   0x040000, 0x783ef8e1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d24_18-1.71",   0x040000, 0x4502db60, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },	
	{ "d24-09.75",     0x100000, 0xece5cc59, BRF_ESS | BRF_PRG | TAITO_68KROM1          },

	{ "d24_17.69",     0x020000, 0xe41e4aae, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "d24-02.12",     0x100000, 0x9f50c271, BRF_GRA | TAITO_CHARS },
	{ "d24-01.11",     0x100000, 0x326dcca9, BRF_GRA | TAITO_CHARS },
	
	{ "d24-07.47",     0x100000, 0x9f50c271, BRF_GRA | TAITO_CHARSB },
	{ "d24-08.48",     0x100000, 0x1e6d1528, BRF_GRA | TAITO_CHARSB },
	
	{ "d24-06.27",     0x100000, 0x918486fe, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "d24-03.24",     0x100000, 0x46db9fd7, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "d24-04.25",     0x100000, 0x148e0493, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "d24-05.26",     0x100000, 0x9f414317, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },	
	
	{ "d24-12.107",    0x100000, 0x279203a1, BRF_SND | TAITO_YM2610A },
	{ "d24-10.95",     0x100000, 0x0e0c716d, BRF_SND | TAITO_YM2610A },
	{ "d24-11.118",    0x100000, 0x15362573, BRF_SND | TAITO_YM2610A },
	
	{ "d24-13.37",     0x000400, 0x3ca18eb3, BRF_OPT },
	{ "d24-14.38",     0x000400, 0xbaf2a193, BRF_OPT },
};

STD_ROM_PICK(Warriorb)
STD_ROM_FN(Warriorb)

static INT32 MemIndex()
{
	UINT8 *Next; Next = TaitoMem;

	Taito68KRom1                        = Next; Next += Taito68KRom1Size;
	Taito68KRom2                        = Next; Next += Taito68KRom2Size;
	TaitoZ80Rom1                        = Next; Next += TaitoZ80Rom1Size;
	TaitoYM2610ARom                     = Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom                     = Next; Next += TaitoYM2610BRomSize;
	
	TaitoRamStart                       = Next;

	Taito68KRam1                        = Next; Next += 0x010000;
	Taito68KRam2                        = Next; Next += 0x010000;
	TaitoZ80Ram1                        = Next; Next += 0x002000;
	TaitoSharedRam                      = Next; Next += 0x010000;
	TaitoSpriteRam                      = Next; Next += 0x004000;	
		
	TaitoRamEnd                         = Next;

	TaitoChars                          = Next; Next += TaitoNumChar * TaitoCharWidth * TaitoCharHeight;
	TaitoSpritesA                       = Next; Next += TaitoNumSpriteA * TaitoSpriteAWidth * TaitoSpriteAHeight;

	TaitoMemEnd                         = Next;

	return 0;
}

static INT32 Darius2dMemIndex()
{
	UINT8 *Next; Next = TaitoMem;

	Taito68KRom1                        = Next; Next += Taito68KRom1Size;
	TaitoZ80Rom1                        = Next; Next += TaitoZ80Rom1Size;
	TaitoYM2610ARom                     = Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom                     = Next; Next += TaitoYM2610BRomSize;
	
	TaitoRamStart                       = Next;

	Taito68KRam1                        = Next; Next += 0x010000;
	TaitoZ80Ram1                        = Next; Next += 0x002000;
	TaitoSpriteRam                      = Next; Next += 0x001400;	
		
	TaitoRamEnd                         = Next;

	TaitoChars                          = Next; Next += TaitoNumChar * TaitoCharWidth * TaitoCharHeight;
	TaitoSpritesA                       = Next; Next += TaitoNumSpriteA * TaitoSpriteAWidth * TaitoSpriteAHeight;

	TaitoMemEnd                         = Next;

	return 0;
}

static INT32 WarriorbMemIndex()
{
	UINT8 *Next; Next = TaitoMem;

	Taito68KRom1                        = Next; Next += Taito68KRom1Size;
	TaitoZ80Rom1                        = Next; Next += TaitoZ80Rom1Size;
	TaitoYM2610ARom                     = Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom                     = Next; Next += TaitoYM2610BRomSize;
	
	TaitoRamStart                       = Next;

	Taito68KRam1                        = Next; Next += 0x014000;
	TaitoZ80Ram1                        = Next; Next += 0x002000;
	TaitoSpriteRam                      = Next; Next += 0x001400;	
		
	TaitoRamEnd                         = Next;

	TaitoChars                          = Next; Next += TaitoNumChar * TaitoCharWidth * TaitoCharHeight;
	TaitoCharsB                         = Next; Next += TaitoNumCharB * TaitoCharBWidth * TaitoCharBHeight;
	TaitoSpritesA                       = Next; Next += TaitoNumSpriteA * TaitoSpriteAWidth * TaitoSpriteAHeight;

	TaitoMemEnd                         = Next;

	return 0;
}

static void Darius2CpuAReset(UINT16 d)
{
	TaitoCpuACtrl = d;
	if (!(TaitoCpuACtrl & 1)) {
		SekClose();
		SekOpen(1);
		SekReset();
		SekClose();
		SekOpen(0);
	}
}

UINT8 __fastcall Darius268K1ReadByte(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Darius268K1WriteByte(UINT32 a, UINT8 d)
{
	TC0100SCNTripleScreenByteWrite_Map(0x280000, 0x293fff)
	TC0100SCN1ByteWrite_Map(0x2c0000, 0x2d3fff)
	TC0100SCN2ByteWrite_Map(0x300000, 0x313fff)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Darius268K1ReadWord(UINT32 a)
{
	switch (a) {
		case 0x200000: {
			return TC0220IOCPortRegRead();
		}
		
		case 0x200002: {
			return TC0220IOCHalfWordPortRead();
		}
		
		case 0x220002: {
			return TC0140SYTCommRead();
		}
		
		case 0x340002: {
			return TC0110PCRWordRead(0);
		}
		
		case 0x350002: {
			return TC0110PCRWordRead(1);
		}
		
		case 0x360002: {
			return TC0110PCRWordRead(2);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Darius268K1WriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0CtrlWordWrite_Map(0x2a0000)
	TC0100SCN1CtrlWordWrite_Map(0x2e0000)
	TC0100SCN2CtrlWordWrite_Map(0x320000)
	TC0100SCNTripleScreenWordWrite_Map(0x280000, 0x293fff)
	TC0100SCN1WordWrite_Map(0x2c0000, 0x2d3fff)
	TC0100SCN2WordWrite_Map(0x300000, 0x313fff)
	
	switch (a) {
		case 0x200000: {
			TC0220IOCHalfWordPortRegWrite(d);
			return;
		}
		
		case 0x200002: {
			TC0220IOCHalfWordPortWrite(d);
			return;
		}
		
		case 0x210000: {
			Darius2CpuAReset(d);
			return;
		}
		
		case 0x220000: {
			TC0140SYTPortWrite(d);
			return;
		}
		
		case 0x220002: {
			TC0140SYTCommWrite(d);
			return;
		}
		
		case 0x340000:
		case 0x340002: {
			TC0110PCRStep1WordWrite(0, (a - 0x340000) >> 1, d);
			return;
		}
		
		case 0x350000:
		case 0x350002: {
			TC0110PCRStep1WordWrite(1, (a - 0x350000) >> 1, d);
			return;
		}
		
		case 0x360000:
		case 0x360002: {
			TC0110PCRStep1WordWrite(2, (a - 0x360000) >> 1, d);
			return;
		}
		
		case 0x340004:
		case 0x350004:
		case 0x360004: {
			//nop
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Darius268K2ReadByte(UINT32 a)
{
	switch (a) {		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Darius268K2WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Darius268K2ReadWord(UINT32 a)
{
	switch (a) {
		case 0x200000: {
			return TC0220IOCPortRegRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Darius268K2WriteWord(UINT32 a, UINT16 d)
{
	TC0100SCNTripleScreenWordWrite_Map(0x280000, 0x293fff)
	
	switch (a) {
		case 0x210000: {
			//???
			return;
		}
		
		case 0x340000:
		case 0x340002: {
			TC0110PCRStep1WordWrite(0, (a - 0x340000) >> 1, d);
			return;
		}
		
		case 0x350000:
		case 0x350002: {
			TC0110PCRStep1WordWrite(1, (a - 0x350000) >> 1, d);
			return;
		}
		
		case 0x360000:
		case 0x360002: {
			TC0110PCRStep1WordWrite(2, (a - 0x360000) >> 1, d);
			return;
		}
		
		case 0x340004:
		case 0x350004:
		case 0x360004: {
			//nop
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Darius2d68KReadByte(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Darius2d68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCNDualScreenByteWrite_Map(0x200000, 0x213fff)
	TC0100SCN1ByteWrite_Map(0x240000, 0x25ffff)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Darius2d68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x400002: {
			return TC0110PCRWordRead(0);
		}
		
		case 0x420002: {
			return TC0110PCRWordRead(1);
		}
		
		case 0x800000:
		case 0x800002:
		case 0x800004:
		case 0x800006:
		case 0x800008:
		case 0x80000a:
		case 0x80000c:
		case 0x80000e: {
			return TC0220IOCRead((a - 0x800000) >> 1);
		}
		
		case 0x830002: {
			return TC0140SYTCommRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Darius2d68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCNDualScreenWordWrite_Map(0x200000, 0x213fff)
	
	if (a >= 0x214000 && a <= 0x2141ff) return;
	
	TC0100SCN0CtrlWordWrite_Map(0x220000)
	TC0100SCN1CtrlWordWrite_Map(0x260000)
	TC0100SCN1WordWrite_Map(0x240000, 0x25ffff)
	
	switch (a) {
		case 0x400000:
		case 0x400002:
		case 0x400004: {
			TC0110PCRStep1WordWrite(0, (a - 0x400000) >> 1, d);
			return;
		}
		
		case 0x420000:
		case 0x420002:
		case 0x420004: {
			TC0110PCRStep1WordWrite(1, (a - 0x420000) >> 1, d);
			return;
		}
		
		case 0x800000:
		case 0x800002:
		case 0x800004:
		case 0x800006:
		case 0x800008:
		case 0x80000a:
		case 0x80000c:
		case 0x80000e: {
			TC0220IOCWrite((a - 0x800000) >> 1, d);
			return;
		}
		
		case 0x820000: {
			// nop
			return;
		}
		
		case 0x830000: {
			TC0140SYTPortWrite(d);
			return;
		}
		
		case 0x830002: {
			TC0140SYTCommWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Warriorb68KReadByte(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Warriorb68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCNDualScreenByteWrite_Map(0x300000, 0x313fff)
	TC0100SCN1ByteWrite_Map(0x340000, 0x35ffff)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Warriorb68KReadWord(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x800000)
	
	switch (a) {
		case 0x400002: {
			return TC0110PCRWordRead(0);
		}
		
		case 0x420002: {
			return TC0110PCRWordRead(1);
		}
		
		case 0x830002: {
			return TC0140SYTCommRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Warriorb68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCNDualScreenWordWrite_Map(0x300000, 0x313fff)
	TC0100SCN1WordWrite_Map(0x340000, 0x35ffff)
	
	TC0100SCN0CtrlWordWrite_Map(0x320000)
	TC0100SCN1CtrlWordWrite_Map(0x360000)
	
	TC0510NIOHalfWordWrite_Map(0x800000)
	
	switch (a) {
		case 0x400000:
		case 0x400002:
		case 0x400004: {
			TC0110PCRStep1WordWrite(0, (a - 0x400000) >> 1, d);
			return;
		}
		
		case 0x420000:
		case 0x420002:
		case 0x420004: {
			TC0110PCRStep1WordWrite(1, (a - 0x420000) >> 1, d);
			return;
		}
		
		case 0x820000: {
			// nop
			return;
		}
		
		case 0x830000: {
			TC0140SYTPortWrite(d);
			return;
		}
		
		case 0x830002: {
			TC0140SYTCommWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Darius2Z80Read(UINT16 a)
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
			// NOP
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Darius2Z80Write(UINT16 a, UINT8 d)
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
		
		case 0xe400: {
			BurnYM2610SetLeftVolume(BURN_SND_YM2610_YM2610_ROUTE_1, Darius2YM2610Route1MasterVol * d / 255.0);
			return;
		}
		
		case 0xe401: {
			BurnYM2610SetRightVolume(BURN_SND_YM2610_YM2610_ROUTE_1, Darius2YM2610Route1MasterVol * d / 255.0);
			return;
		}
		
		case 0xe402: {
			BurnYM2610SetLeftVolume(BURN_SND_YM2610_YM2610_ROUTE_2, Darius2YM2610Route1MasterVol * d / 255.0);
			return;
		}
		
		case 0xe403: {
			BurnYM2610SetRightVolume(BURN_SND_YM2610_YM2610_ROUTE_2, Darius2YM2610Route1MasterVol * d / 255.0);
			return;
		}
		
		case 0xe600: {
			//???
			return;
		}
		
		case 0xee00: {
			//nop
			return;
		}
		
		case 0xf000: {
			//nop
			return;
		}
		
		case 0xf200: {
			TaitoZ80Bank = (d - 1) & 7;
			ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write %04X, %02X\n"), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[4]           = { 0, 1, 2, 3 };
static INT32 CharXOffsets[8]               = { 8, 12, 0, 4, 24, 28, 16, 20 };
static INT32 CharYOffsets[8]               = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 SpritePlaneOffsets[4]         = { 8, 12, 0, 4 };
static INT32 SpriteXOffsets[16]            = { 3, 2, 1, 0, 19, 18, 17, 16, 259, 258, 257, 256, 275, 274, 273, 272 };
static INT32 SpriteYOffsets[16]            = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };
static INT32 Darius2dSpritePlaneOffsets[4] = { 0, 1, 2, 3 };
static INT32 Darius2dSpriteXOffsets[16]    = { 12, 8, 44, 40, 4, 0, 36, 32, 28, 24, 60, 56, 20, 16, 52, 48 };
static INT32 Darius2dSpriteYOffsets[16]    = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };

static void Darius2FMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 Darius2SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / (16000000 / 4);
}

static double Darius2GetTime()
{
	return (double)ZetTotalCycles() / (16000000 / 4);
}

static INT32 Darius2Init()
{
	INT32 nLen;
	
	TaitoCharModulo = 0x100;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 8;
	TaitoCharHeight = 8;
	TaitoCharPlaneOffsets = CharPlaneOffsets;
	TaitoCharXOffsets = CharXOffsets;
	TaitoCharYOffsets = CharYOffsets;
	TaitoNumChar = 0x8000;
	
	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x2000;
	
	if (Ninjaw) TaitoNumSpriteA = 0x4000;
	
	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
		
	TaitoLoadRoms(0);
	
	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 22, 16, 0, NULL);
	TC0100SCNSetClipArea(0, 288, nScreenHeight, 0);
	TC0100SCNInit(1, TaitoNumChar, 24, 16, 0, NULL);
	TC0100SCNSetClipArea(1, 288, nScreenHeight, 288);
	TC0100SCNSetPaletteOffset(1, 0x1000);
	TC0100SCNInit(2, TaitoNumChar, 26, 16, 0, NULL);
	TC0100SCNSetClipArea(2, 288, nScreenHeight, 576);
	TC0100SCNSetPaletteOffset(2, 0x2000);
	TC0110PCRInit(3, 0x3000);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1           , 0x000000, 0x0bffff, SM_ROM);
	SekMapMemory(Taito68KRam1           , 0x0c0000, 0x0cffff, SM_RAM);
	SekMapMemory(TaitoSharedRam         , 0x240000, 0x24ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam         , 0x260000, 0x263fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]        , 0x280000, 0x293fff, SM_READ);
	SekMapMemory(TC0100SCNRam[1]        , 0x2c0000, 0x2d3fff, SM_READ);
	SekMapMemory(TC0100SCNRam[2]        , 0x300000, 0x313fff, SM_READ);
	SekSetReadByteHandler(0, Darius268K1ReadByte);
	SekSetWriteByteHandler(0, Darius268K1WriteByte);
	SekSetReadWordHandler(0, Darius268K1ReadWord);	
	SekSetWriteWordHandler(0, Darius268K1WriteWord);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2           , 0x000000, 0x05ffff, SM_ROM);
	SekMapMemory(Taito68KRam2           , 0x080000, 0x08ffff, SM_RAM);
	SekMapMemory(TaitoSharedRam         , 0x240000, 0x24ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam         , 0x260000, 0x263fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]        , 0x280000, 0x293fff, SM_READ);
	SekSetReadByteHandler(0, Darius268K2ReadByte);
	SekSetWriteByteHandler(0, Darius268K2WriteByte);
	SekSetReadWordHandler(0, Darius268K2ReadWord);	
	SekSetWriteWordHandler(0, Darius268K2WriteWord);
	SekClose();
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Darius2Z80Read);
	ZetSetWriteHandler(Darius2Z80Write);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1               );
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1               );
	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1               );
	ZetMemEnd();
	ZetClose();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &Darius2FMIRQHandler, Darius2SynchroniseStream, Darius2GetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetLeftVolume(BURN_SND_YM2610_AY8910_ROUTE, 0.25);
	BurnYM2610SetRightVolume(BURN_SND_YM2610_AY8910_ROUTE, 0.25);
	Darius2YM2610Route1MasterVol = 1.00;
	Darius2YM2610Route2MasterVol = 1.00;
	bYM2610UseSeperateVolumes = 1;
	
	TaitoDrawFunction = Darius2Draw;
	TaitoMakeInputsFunction = Darius2MakeInputs;
	TaitoIrqLine = 4;
	
	nTaitoCyclesTotal[0] = (16000000 / 2) / 60;
	nTaitoCyclesTotal[1] = (16000000 / 2) / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;

	TaitoResetFunction = TaitoDoReset;
	TaitoResetFunction();

	return 0;
}

static INT32 Darius2dInit()
{
	INT32 nLen;
	
	TaitoCharModulo = 0x100;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 8;
	TaitoCharHeight = 8;
	TaitoCharPlaneOffsets = CharPlaneOffsets;
	TaitoCharXOffsets = CharXOffsets;
	TaitoCharYOffsets = CharYOffsets;
	TaitoNumChar = 0x8000;
	
	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = Darius2dSpritePlaneOffsets;
	TaitoSpriteAXOffsets = Darius2dSpriteXOffsets;
	TaitoSpriteAYOffsets = Darius2dSpriteYOffsets;
	TaitoNumSpriteA = 0x4000;
	
	TaitoNum68Ks = 1;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
		
	TaitoLoadRoms(0);
	
	// Allocate and Blank all required memory
	TaitoMem = NULL;
	Darius2dMemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	Darius2dMemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 4, 24, 0, NULL);
	TC0100SCNSetClipArea(0, 320, nScreenHeight, 0);
	TC0100SCNInit(1, TaitoNumChar, 4, 24, 0, NULL);
	TC0100SCNSetClipArea(1, 320, nScreenHeight, 320);
	TC0100SCNSetPaletteOffset(1, 0x1000);
	TC0110PCRInit(2, 0x2000);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1           , 0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Taito68KRam1           , 0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]        , 0x200000, 0x213fff, SM_READ);
	SekMapMemory(TC0100SCNRam[1]        , 0x240000, 0x253fff, SM_READ);
	SekMapMemory(TaitoSpriteRam         , 0x600000, 0x6013ff, SM_RAM);
	SekSetReadByteHandler(0, Darius2d68KReadByte);
	SekSetWriteByteHandler(0, Darius2d68KWriteByte);
	SekSetReadWordHandler(0, Darius2d68KReadWord);	
	SekSetWriteWordHandler(0, Darius2d68KWriteWord);
	SekClose();
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Darius2Z80Read);
	ZetSetWriteHandler(Darius2Z80Write);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1               );
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1               );
	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1               );
	ZetMemEnd();
	ZetClose();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &Darius2FMIRQHandler, Darius2SynchroniseStream, Darius2GetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetLeftVolume(BURN_SND_YM2610_AY8910_ROUTE, 0.25);
	BurnYM2610SetRightVolume(BURN_SND_YM2610_AY8910_ROUTE, 0.25);
	Darius2YM2610Route1MasterVol = 12.00;
	Darius2YM2610Route2MasterVol = 12.00;
	bYM2610UseSeperateVolumes = 1;
	
	TaitoDrawFunction = Darius2dDraw;
	TaitoMakeInputsFunction = Darius2dMakeInputs;
	TaitoIrqLine = 4;
	
	nTaitoCyclesTotal[0] = 12000000 / 60;
	nTaitoCyclesTotal[1] = (16000000 / 4) / 60;

	TaitoResetFunction = TaitoDoReset;
	TaitoResetFunction();

	return 0;
}

static INT32 NinjawInit()
{
	Ninjaw = 1;
	return Darius2Init();
}

static INT32 WarriorbInit()
{
	INT32 nLen;
	
	TaitoCharModulo = 0x100;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 8;
	TaitoCharHeight = 8;
	TaitoCharPlaneOffsets = CharPlaneOffsets;
	TaitoCharXOffsets = CharXOffsets;
	TaitoCharYOffsets = CharYOffsets;
	TaitoNumChar = 0x10000;
	
	TaitoCharBModulo = 0x100;
	TaitoCharBNumPlanes = 4;
	TaitoCharBWidth = 8;
	TaitoCharBHeight = 8;
	TaitoCharBPlaneOffsets = CharPlaneOffsets;
	TaitoCharBXOffsets = CharXOffsets;
	TaitoCharBYOffsets = CharYOffsets;
	TaitoNumCharB = 0x10000;
	
	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = Darius2dSpritePlaneOffsets;
	TaitoSpriteAXOffsets = Darius2dSpriteXOffsets;
	TaitoSpriteAYOffsets = Darius2dSpriteYOffsets;
	TaitoNumSpriteA = 0x8000;
	
	TaitoNum68Ks = 1;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
		
	TaitoLoadRoms(0);
	
	// Allocate and Blank all required memory
	TaitoMem = NULL;
	WarriorbMemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	WarriorbMemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 4, 8, 0, NULL);
	TC0100SCNSetClipArea(0, 320, nScreenHeight, 0);
	TC0100SCNInit(1, TaitoNumCharB, 5, 8, 0, NULL);
	TC0100SCNSetClipArea(1, 320, nScreenHeight, 320);
	TC0100SCNSetPaletteOffset(1, 0x1000);
	TC0110PCRInit(2, 0x2000);
	TC0140SYTInit();
	TC0510NIOInit();
	
	if (TaitoLoadRoms(1)) return 1;
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1           , 0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Taito68KRam1           , 0x200000, 0x213fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]        , 0x300000, 0x313fff, SM_READ);
	SekMapMemory(TC0100SCNRam[1]        , 0x340000, 0x353fff, SM_READ);
	SekMapMemory(TaitoSpriteRam         , 0x600000, 0x6013ff, SM_RAM);
	SekSetReadByteHandler(0, Warriorb68KReadByte);
	SekSetWriteByteHandler(0, Warriorb68KWriteByte);
	SekSetReadWordHandler(0, Warriorb68KReadWord);	
	SekSetWriteWordHandler(0, Warriorb68KWriteWord);
	SekClose();
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Darius2Z80Read);
	ZetSetWriteHandler(Darius2Z80Write);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1               );
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1               );
	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1               );
	ZetMemEnd();
	ZetClose();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &Darius2FMIRQHandler, Darius2SynchroniseStream, Darius2GetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetLeftVolume(BURN_SND_YM2610_AY8910_ROUTE, 0.25);
	BurnYM2610SetRightVolume(BURN_SND_YM2610_AY8910_ROUTE, 0.25);
	Darius2YM2610Route1MasterVol = 12.00;
	Darius2YM2610Route2MasterVol = 12.00;
	bYM2610UseSeperateVolumes = 1;
	
	TaitoDrawFunction = WarriorbDraw;
	TaitoMakeInputsFunction = WarriorbMakeInputs;
	TaitoIrqLine = 4;
	
	nTaitoCyclesTotal[0] = 16000000 / 60;
	nTaitoCyclesTotal[1] = (16000000 / 4) / 60;

	TaitoResetFunction = TaitoDoReset;
	TaitoResetFunction();

	return 0;
}

static INT32 Darius2Exit()
{
	Ninjaw = 0;
	
	return TaitoExit();
}

static void Darius2RenderSprites(INT32 PriorityDraw)
{
	INT32 Offset, Data, Code, Colour, xFlip, yFlip;
	INT32 x, y, Priority;
	
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;

	for (Offset = 0x2000 - 4; Offset >=0; Offset -= 4) {
		Data = SpriteRam[Offset + 2];
		Code = Data & (TaitoNumSpriteA - 1);

		if (!Code) continue;

		Data = SpriteRam[Offset + 0];
		x = (Data - 32) & 0x3ff;

		Data = SpriteRam[Offset + 1];
		y = (Data - 0) & 0x1ff;
		
		Data = SpriteRam[Offset + 3];
		xFlip = (Data & 0x1);
		yFlip = (Data & 0x2) >> 1;
		Priority = (Data & 0x4) >> 2;
		
		if (Priority != PriorityDraw) continue;
		
		Colour = (Data & 0x7f00) >> 8;

		y -= 16;

		if (x > 0x3c0) x -= 0x400;
		if (y > 0x180) y -= 0x200;

		if (x > 16 && x < (nScreenWidth - 16) && y > 16 && y < (nScreenHeight - 16)) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			}
		}
	}
}

static void Darius2dRenderSprites(INT32 PriorityDraw)
{
	INT32 Offset, Data, Code, Colour, xFlip, yFlip;
	INT32 x, y, Priority;
	
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;

//	for (Offset = 0; Offset < 0x001400 / 2; Offset += 4) {
	for (Offset = (0x1400 / 2) - 4; Offset >= 0; Offset -= 4) {
		Data = SpriteRam[Offset + 1];
		Code = Data & (TaitoNumSpriteA - 1);

		Data = SpriteRam[Offset + 0];
		y = (-(Data & 0x1ff) - 24) & 0x1ff;
		yFlip = (Data & 0x200) >> 9;

		Data = SpriteRam[Offset + 2];
		Priority = (Data & 0x100) >> 8;
		Colour = Data & 0x7f;
		
		Data = SpriteRam[Offset + 3];
		x = Data & 0x3ff;
		xFlip = (Data & 0x400) >> 10;
		
		if (Priority != PriorityDraw) continue;

		y -= 16;

		if (x > 0x3c0) x -= 0x400;
		if (y > 0x180) y -= 0x200;
		
		if (x > 16 && x < (nScreenWidth - 16) && y > 16 && y < (nScreenHeight - 16)) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			}
		}
	}
}

static void Darius2Draw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	INT32 Disable2 = TC0100SCNCtrl[1][6] & 0xf7;
	INT32 Disable3 = TC0100SCNCtrl[2][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x02)) TC0100SCNRenderFgLayer(1, 0, TaitoChars);
		if (!(Disable3 & 0x02)) TC0100SCNRenderFgLayer(2, 0, TaitoChars);
		Darius2RenderSprites(1);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x01)) TC0100SCNRenderBgLayer(1, 0, TaitoChars);
		if (!(Disable3 & 0x01)) TC0100SCNRenderBgLayer(2, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x01)) TC0100SCNRenderBgLayer(1, 0, TaitoChars);
		if (!(Disable3 & 0x01)) TC0100SCNRenderBgLayer(2, 0, TaitoChars);
		Darius2RenderSprites(1);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x02)) TC0100SCNRenderFgLayer(1, 0, TaitoChars);
		if (!(Disable3 & 0x02)) TC0100SCNRenderFgLayer(2, 0, TaitoChars);
	}
	
	Darius2RenderSprites(0);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	if (!(Disable2 & 0x04)) TC0100SCNRenderCharLayer(1);
	if (!(Disable3 & 0x04)) TC0100SCNRenderCharLayer(2);
	BurnTransferCopy(TC0110PCRPalette);
}

static void Darius2dDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	INT32 Disable2 = TC0100SCNCtrl[1][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x02)) TC0100SCNRenderFgLayer(1, 0, TaitoChars);
		Darius2dRenderSprites(1);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x01)) TC0100SCNRenderBgLayer(1, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x01)) TC0100SCNRenderBgLayer(1, 0, TaitoChars);
		Darius2dRenderSprites(1);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x02)) TC0100SCNRenderFgLayer(1, 0, TaitoChars);
	}
	
	Darius2dRenderSprites(0);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	if (!(Disable2 & 0x04)) TC0100SCNRenderCharLayer(1);
	BurnTransferCopy(TC0110PCRPalette);
}

static void WarriorbDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	INT32 Disable2 = TC0100SCNCtrl[1][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x02)) TC0100SCNRenderFgLayer(1, 0, TaitoCharsB);
		Darius2dRenderSprites(1);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x01)) TC0100SCNRenderBgLayer(1, 0, TaitoCharsB);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x01)) TC0100SCNRenderBgLayer(1, 0, TaitoCharsB);
		Darius2dRenderSprites(1);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable2 & 0x02)) TC0100SCNRenderFgLayer(1, 0, TaitoCharsB);
	}
	
	Darius2dRenderSprites(0);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	if (!(Disable2 & 0x04)) TC0100SCNRenderCharLayer(1);
	BurnTransferCopy(TC0110PCRPalette);
}

static INT32 Darius2Frame()
{
	INT32 nInterleave = 100;

	if (TaitoReset) TaitoResetFunction();

	TaitoMakeInputsFunction();
	
	nTaitoCyclesDone[0] = nTaitoCyclesDone[1] = nTaitoCyclesDone[2] = 0;

	SekNewFrame();
	ZetNewFrame();
		
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 # 1
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
		if (i == nInterleave - 1) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		// Run 68000 # 2
		if ((TaitoCpuACtrl & 0x01)) {
			nCurrentCPU = 1;
			SekOpen(1);
			nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
			nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
			nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
			if (i == nInterleave - 1) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
			SekClose();
		}
		
		ZetOpen(0);
		BurnTimerUpdate(i * (nTaitoCyclesTotal[2] / nInterleave));
		ZetClose();
	}
	
	ZetOpen(0);
	BurnTimerEndFrame(nTaitoCyclesTotal[2]);
	BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();
	
	if (pBurnDraw) TaitoDrawFunction();
	
	return 0;
}

static INT32 Darius2dFrame()
{
	INT32 nInterleave = 100;

	if (TaitoReset) TaitoResetFunction();

	TaitoMakeInputsFunction();
	
	nTaitoCyclesDone[0] = nTaitoCyclesDone[2] = 0;

	SekNewFrame();
	ZetNewFrame();
		
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 # 1
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
		if (i == nInterleave - 1) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
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

static INT32 Darius2Scan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029684;
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

		if (TaitoNumYM2610) BurnYM2610Scan(nAction, pnMin);

		SCAN_VAR(TaitoInput);
		SCAN_VAR(TaitoZ80Bank);
		SCAN_VAR(nTaitoCyclesDone);
		SCAN_VAR(nTaitoCyclesSegment);
	}
	
	if (nAction & ACB_WRITE && TaitoZ80Bank) {
		ZetOpen(0);
		ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
		ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
		ZetClose();
	}
	
	return 0;
}

struct BurnDriver BurnDrvDarius2 = {
	"darius2", NULL, NULL, NULL, "1989",
	"Darius II (Japan)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_HORSHOOT, 0,
	NULL, Darius2RomInfo, Darius2RomName, NULL, NULL, Darius2InputInfo, Darius2DIPInfo,
	Darius2Init, Darius2Exit, Darius2Frame, NULL, Darius2Scan,
	NULL, 0x3000, 864, 224, 12, 3
};

struct BurnDriver BurnDrvDarius2d = {
	"darius2d", "darius2", NULL, NULL, "1989",
	"Darius II (dual screen) (Japan)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_HORSHOOT, 0,
	NULL, Darius2dRomInfo, Darius2dRomName, NULL, NULL, Darius2dInputInfo, Darius2dDIPInfo,
	Darius2dInit, Darius2Exit, Darius2dFrame, NULL, Darius2Scan,
	NULL, 0x2000, 640, 224, 8, 3
};

struct BurnDriver BurnDrvDarius2do = {
	"darius2do", "darius2", NULL, NULL, "1989",
	"Darius II (dual screen) (Japan old version)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_HORSHOOT, 0,
	NULL, Darius2doRomInfo, Darius2doRomName, NULL, NULL, Darius2dInputInfo, Darius2dDIPInfo,
	Darius2dInit, Darius2Exit, Darius2dFrame, NULL, Darius2Scan,
	NULL, 0x2000, 640, 224, 8, 3
};

struct BurnDriver BurnDrvNinjaw = {
	"ninjaw", NULL, NULL, NULL, "1987",
	"The Ninja Warriors (World)\0", NULL, "Taito Corporation Japan", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, NinjawRomInfo, NinjawRomName, NULL, NULL, Darius2InputInfo, NinjawDIPInfo,
	NinjawInit, Darius2Exit, Darius2Frame, NULL, Darius2Scan,
	NULL, 0x3000, 864, 224, 12, 3
};

struct BurnDriver BurnDrvNinjawj = {
	"ninjawj", "ninjaw", NULL, NULL, "1987",
	"The Ninja Warriors (Japan)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, NinjawjRomInfo, NinjawjRomName, NULL, NULL, Darius2InputInfo, NinjawjDIPInfo,
	NinjawInit, Darius2Exit, Darius2Frame, NULL, Darius2Scan,
	NULL, 0x3000, 864, 224, 12, 3
};

struct BurnDriver BurnDrvNinjawu = {
	"ninjawu", "ninjaw", NULL, NULL, "1987",
	"The Ninja Warriors (US)\0", NULL, "Taito Corporation America (licensed to Romstar)", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, NinjawuRomInfo, NinjawuRomName, NULL, NULL, Darius2InputInfo, NinjawjDIPInfo,
	NinjawInit, Darius2Exit, Darius2Frame, NULL, Darius2Scan,
	NULL, 0x3000, 864, 224, 12, 3
};

struct BurnDriver BurnDrvWarriorb = {
	"warriorb", NULL, NULL, NULL, "1991",
	"Warrior Blade - Rastan Saga Episode III (Japan)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_HORSHOOT, 0,
	NULL, WarriorbRomInfo, WarriorbRomName, NULL, NULL, WarriorbInputInfo, WarriorbDIPInfo,
	WarriorbInit, Darius2Exit, Darius2dFrame, NULL, Darius2Scan,
	NULL, 0x2000, 640, 240, 8, 3
};
