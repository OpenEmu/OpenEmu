#include "tiles_generic.h"
#include "taito.h"
#include "taito_ic.h"
#include "burn_ym2610.h"
#include "eeprom.h"
#include "burn_gun.h"

static INT32 Sci;
static INT32 Dblaxle;
static INT32 OldSteer; // Hack to centre the steering in SCI
static INT32 SciSpriteFrame;

static void AquajackDraw();
static void BsharkDraw();
static void ChasehqDraw();
static void ContcircDraw();
static void EnforceDraw();
static void DblaxleDraw();
static void SciDraw();
static void SpacegunDraw();

static bool bUseAsm68KCoreOldValue = false;

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo AquajackInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p2 coin"   },

	{"Up"                , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p1 right"  },
	{"Jump"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p1 fire 1" },
	{"Accelerate"        , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 fire 2" },
	{"Vulcan"            , BIT_DIGITAL   , TC0220IOCInputPort1 + 6, "p1 fire 3" },
	{"Missile"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p1 fire 4" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Aquajack)

static struct BurnInputInfo BsharkInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "p2 start"  },

	A("Crosshair X"      , BIT_ANALOG_REL, &TaitoAnalogPort0     , "p1 x-axis" ),
	A("Crosshair Y"      , BIT_ANALOG_REL, &TaitoAnalogPort1     , "p1 y-axis" ),
	{"Fire 1"            , BIT_DIGITAL   , TC0220IOCInputPort2 + 6, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , TC0220IOCInputPort2 + 7, "p1 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Bshark)

static struct BurnInputInfo BsharkjjsInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , TC0220IOCInputPort2 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , TC0220IOCInputPort2 + 6, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , TC0220IOCInputPort2 + 7, "p1 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Bsharkjjs)

static struct BurnInputInfo ChasehqInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p2 coin"   },

	A("Steering"         , BIT_ANALOG_REL, &TaitoAnalogPort0     , "p1 x-axis" ),
	{"Brake"             , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 fire 1" },
	{"Accelerate"        , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p1 fire 2" },
	{"Turbo"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p1 fire 3" },
	{"Gear"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p1 fire 4" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Chasehq)

static struct BurnInputInfo ContcircInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p2 coin"   },

	A("Steering"         , BIT_ANALOG_REL, &TaitoAnalogPort0     , "p1 x-axis" ),
	{"Brake"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p1 fire 1" },
	{"Accelerate"        , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p1 fire 2" },
	{"Gear"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p1 fire 3" },
	{"Brake 2"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p1 fire 4" },
	{"Brake 3"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 6, "p1 fire 5" },
	{"Accelerate 2"      , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 fire 6" },
	{"Accelerate 3"      , BIT_DIGITAL   , TC0220IOCInputPort0 + 6, "p1 fire 7" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Contcirc)

static struct BurnInputInfo DblaxleInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p2 coin"   },

	A("Steering"         , BIT_ANALOG_REL, &TaitoAnalogPort0     , "p1 x-axis" ),
	{"Brake"             , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p1 fire 1" },
	{"Accelerate"        , BIT_DIGITAL   , TC0510NIOInputPort1 + 5, "p1 fire 2" },
	{"Nitro"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p1 fire 3" },
	{"Gear"              , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 fire 4" },
	{"Reverse"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 fire 5" },
	{"Centre"            , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p1 fire 6" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Dblaxle)

static struct BurnInputInfo EnforceInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p2 coin"   },

	{"Up"                , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , TC0220IOCInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p1 right"  },
	{"Laser"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p1 fire 1" },
	{"Bomb"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p1 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Enforce)

static struct BurnInputInfo NightstrInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "p2 start"  },

	A("Stick X"          , BIT_ANALOG_REL, &TaitoAnalogPort0     , "p1 x-axis" ),
	A("Stick Y"          , BIT_ANALOG_REL, &TaitoAnalogPort1     , "p1 y-axis" ),
	{"Fire 1"            , BIT_DIGITAL   , TC0220IOCInputPort2 + 6, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , TC0220IOCInputPort2 + 7, "p1 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 6, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Nightstr)

static struct BurnInputInfo SciInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p2 coin"   },

	A("Steering"         , BIT_ANALOG_REL, &TaitoAnalogPort0     , "p1 x-axis" ),
	{"Brake"             , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 fire 1" },
	{"Accelerate"        , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p1 fire 2" },
	{"Fire"              , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 fire 3" },
	{"Turbo"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p1 fire 4" },
	{"Gear"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p1 fire 5" },
	{"Centre"            , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p1 fire 6" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Sci)

static struct BurnInputInfo SpacegunInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p2 start"  },

	A("P1 Gun X"         , BIT_ANALOG_REL, &TaitoAnalogPort0     , "mouse x-axis" ),
	A("P1 Gun Y"         , BIT_ANALOG_REL, &TaitoAnalogPort1     , "mouse y-axis" ),
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "mouse button 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "mouse button 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p1 fire 1" },
	
	A("P2 Gun X"         , BIT_ANALOG_REL, &TaitoAnalogPort2     , "p2 x-axis" ),
	A("P2 Gun Y"         , BIT_ANALOG_REL, &TaitoAnalogPort3     , "p2 y-axis" ),
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p2 fire 3" },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset           , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Spacegun)

#undef A

static void AquajackMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xff;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] -= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] -= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] -= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] -= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;
	
	if (TC0220IOCInputPort1[0]) TC0220IOCInput[1] -= 0x01;
	if (TC0220IOCInputPort1[1]) TC0220IOCInput[1] -= 0x02;
	if (TC0220IOCInputPort1[2]) TC0220IOCInput[1] -= 0x04;
	if (TC0220IOCInputPort1[3]) TC0220IOCInput[1] -= 0x08;
	if (TC0220IOCInputPort1[4]) TC0220IOCInput[1] -= 0x10;
	if (TC0220IOCInputPort1[5]) TC0220IOCInput[1] -= 0x20;
	if (TC0220IOCInputPort1[6]) TC0220IOCInput[1] -= 0x40;
	if (TC0220IOCInputPort1[7]) TC0220IOCInput[1] -= 0x80;
}

static void BsharkMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xff;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] -= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] -= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] -= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] -= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;
	
	if (TC0220IOCInputPort2[0]) TC0220IOCInput[2] -= 0x01;
	if (TC0220IOCInputPort2[1]) TC0220IOCInput[2] -= 0x02;
	if (TC0220IOCInputPort2[2]) TC0220IOCInput[2] -= 0x04;
	if (TC0220IOCInputPort2[3]) TC0220IOCInput[2] -= 0x08;
	if (TC0220IOCInputPort2[4]) TC0220IOCInput[2] -= 0x10;
	if (TC0220IOCInputPort2[5]) TC0220IOCInput[2] -= 0x20;
	if (TC0220IOCInputPort2[6]) TC0220IOCInput[2] -= 0x40;
	if (TC0220IOCInputPort2[7]) TC0220IOCInput[2] -= 0x80;
}

static void ChasehqMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xf3;
	TC0220IOCInput[1] = 0xef;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] -= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] |= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] |= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] -= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;
	
	if (TC0220IOCInputPort1[0]) TC0220IOCInput[1] -= 0x01;
	if (TC0220IOCInputPort1[1]) TC0220IOCInput[1] -= 0x02;
	if (TC0220IOCInputPort1[2]) TC0220IOCInput[1] -= 0x04;
	if (TC0220IOCInputPort1[3]) TC0220IOCInput[1] -= 0x08;
	if (TC0220IOCInputPort1[4]) TC0220IOCInput[1] |= 0x10;
	if (TC0220IOCInputPort1[5]) TC0220IOCInput[1] -= 0x20;
	if (TC0220IOCInputPort1[6]) TC0220IOCInput[1] -= 0x40;
	if (TC0220IOCInputPort1[7]) TC0220IOCInput[1] -= 0x80;
}

static void ContcircMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0x13;
	TC0220IOCInput[1] = 0x0f;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] -= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] |= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] |= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] -= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] |= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] |= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] |= 0x80;
	
	if (TC0220IOCInputPort1[0]) TC0220IOCInput[1] -= 0x01;
	if (TC0220IOCInputPort1[1]) TC0220IOCInput[1] -= 0x02;
	if (TC0220IOCInputPort1[2]) TC0220IOCInput[1] -= 0x04;
	if (TC0220IOCInputPort1[3]) TC0220IOCInput[1] -= 0x08;
	if (TC0220IOCInputPort1[4]) TC0220IOCInput[1] |= 0x10;
	if (TC0220IOCInputPort1[5]) TC0220IOCInput[1] |= 0x20;
	if (TC0220IOCInputPort1[6]) TC0220IOCInput[1] |= 0x40;
	if (TC0220IOCInputPort1[7]) TC0220IOCInput[1] |= 0x80;
}

static void DblaxleMakeInputs()
{
	// Reset Inputs
	TC0510NIOInput[0] = 0xff;
	TC0510NIOInput[1] = 0xff;
	TC0510NIOInput[2] = 0xff;

	if (TC0510NIOInputPort0[0]) TC0510NIOInput[0] -= 0x01;
	if (TC0510NIOInputPort0[1]) TC0510NIOInput[0] -= 0x02;
	if (TC0510NIOInputPort0[2]) TC0510NIOInput[0] -= 0x04;
	if (TC0510NIOInputPort0[3]) TC0510NIOInput[0] -= 0x08;
	if (TC0510NIOInputPort0[4]) TC0510NIOInput[0] -= 0x10;
	if (TC0510NIOInputPort0[5]) TC0510NIOInput[0] -= 0x20;
	if (TC0510NIOInputPort0[6]) TC0510NIOInput[0] -= 0x40;
	if (TC0510NIOInputPort0[7]) TC0510NIOInput[0] -= 0x80;
	
	if (TC0510NIOInputPort1[0]) TC0510NIOInput[1] -= 0x01;
	if (TC0510NIOInputPort1[1]) TC0510NIOInput[1] -= 0x02;
	if (TC0510NIOInputPort1[2]) TC0510NIOInput[1] -= 0x04;
	if (TC0510NIOInputPort1[3]) TC0510NIOInput[1] -= 0x08;
	if (TC0510NIOInputPort1[4]) TC0510NIOInput[1] -= 0x10;
	if (TC0510NIOInputPort1[5]) TC0510NIOInput[1] -= 0x20;
	if (TC0510NIOInputPort1[6]) TC0510NIOInput[1] -= 0x40;
	if (TC0510NIOInputPort1[7]) TC0510NIOInput[1] -= 0x80;
}

static void EnforceMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xf3;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] -= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] |= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] |= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] -= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;
	
	if (TC0220IOCInputPort1[0]) TC0220IOCInput[1] -= 0x01;
	if (TC0220IOCInputPort1[1]) TC0220IOCInput[1] -= 0x02;
	if (TC0220IOCInputPort1[2]) TC0220IOCInput[1] -= 0x04;
	if (TC0220IOCInputPort1[3]) TC0220IOCInput[1] -= 0x08;
	if (TC0220IOCInputPort1[4]) TC0220IOCInput[1] -= 0x10;
	if (TC0220IOCInputPort1[5]) TC0220IOCInput[1] -= 0x20;
	if (TC0220IOCInputPort1[6]) TC0220IOCInput[1] -= 0x40;
	if (TC0220IOCInputPort1[7]) TC0220IOCInput[1] -= 0x80;
}

static void NightstrMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xff;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] -= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] -= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] -= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] -= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;
	
	if (TC0220IOCInputPort2[0]) TC0220IOCInput[2] -= 0x01;
	if (TC0220IOCInputPort2[1]) TC0220IOCInput[2] -= 0x02;
	if (TC0220IOCInputPort2[2]) TC0220IOCInput[2] -= 0x04;
	if (TC0220IOCInputPort2[3]) TC0220IOCInput[2] -= 0x08;
	if (TC0220IOCInputPort2[4]) TC0220IOCInput[2] -= 0x10;
	if (TC0220IOCInputPort2[5]) TC0220IOCInput[2] -= 0x20;
	if (TC0220IOCInputPort2[6]) TC0220IOCInput[2] -= 0x40;
	if (TC0220IOCInputPort2[7]) TC0220IOCInput[2] -= 0x80;
}

static void SciMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xff;
	TC0220IOCInput[1] = 0xef;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] -= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] -= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] -= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] -= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;
	
	if (TC0220IOCInputPort1[0]) TC0220IOCInput[1] -= 0x01;
	if (TC0220IOCInputPort1[1]) TC0220IOCInput[1] -= 0x02;
	if (TC0220IOCInputPort1[2]) TC0220IOCInput[1] -= 0x04;
	if (TC0220IOCInputPort1[3]) TC0220IOCInput[1] -= 0x08;
	if (TC0220IOCInputPort1[4]) TC0220IOCInput[1] |= 0x10;
	if (TC0220IOCInputPort1[5]) TC0220IOCInput[1] -= 0x20;
	if (TC0220IOCInputPort1[6]) TC0220IOCInput[1] -= 0x40;
	if (TC0220IOCInputPort1[7]) TC0220IOCInput[1] -= 0x80;
}

static void SpacegunMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xff;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] -= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] -= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] -= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] -= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;
	
	if (TC0220IOCInputPort2[0]) TC0220IOCInput[2] -= 0x01;
	if (TC0220IOCInputPort2[1]) TC0220IOCInput[2] -= 0x02;
	if (TC0220IOCInputPort2[2]) TC0220IOCInput[2] -= 0x04;
	if (TC0220IOCInputPort2[3]) TC0220IOCInput[2] -= 0x08;
	if (TC0220IOCInputPort2[4]) TC0220IOCInput[2] -= 0x10;
	if (TC0220IOCInputPort2[5]) TC0220IOCInput[2] -= 0x20;
	if (TC0220IOCInputPort2[6]) TC0220IOCInput[2] -= 0x40;
	if (TC0220IOCInputPort2[7]) TC0220IOCInput[2] -= 0x80;
	
	BurnGunMakeInputs(0, (INT16)TaitoAnalogPort0, (INT16)TaitoAnalogPort1);
	BurnGunMakeInputs(1, (INT16)TaitoAnalogPort2, (INT16)TaitoAnalogPort3);
}

static struct BurnDIPInfo AquajackDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x7f, NULL                             },
	{0x0f, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0e, 0x01, 0x80, 0x80, "Cockpit"                        },
	{0x0e, 0x01, 0x80, 0x00, "Upright"                        },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0e, 0x01, 0x20, 0x20, "Off"                            },
	{0x0e, 0x01, 0x20, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0e, 0x01, 0x10, 0x00, "Off"                            },
	{0x0e, 0x01, 0x10, 0x10, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0e, 0x01, 0x0c, 0x00, "4 Coins 1 Credit"               },
	{0x0e, 0x01, 0x0c, 0x08, "3 Coins 1 Credit"               },
	{0x0e, 0x01, 0x0c, 0x04, "2 Coins 1 Credit"               },
	{0x0e, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0e, 0x01, 0x03, 0x03, "1 Coin 2 Credits"               },
	{0x0e, 0x01, 0x03, 0x01, "1 Coin 3 Credits"               },
	{0x0e, 0x01, 0x03, 0x02, "1 Coin 4 Credits"               },
	{0x0e, 0x01, 0x03, 0x00, "1 Coin 6 Credits"               },	
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0f, 0x01, 0xc0, 0x40, "Easy"                           },
	{0x0f, 0x01, 0xc0, 0xc0, "Normal"                         },
	{0x0f, 0x01, 0xc0, 0x80, "Hard"                           },
	{0x0f, 0x01, 0xc0, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x0f, 0x01, 0x30, 0x00, "30000"                          },
	{0x0f, 0x01, 0x30, 0x30, "50000"                          },
	{0x0f, 0x01, 0x30, 0x10, "80000"                          },
	{0x0f, 0x01, 0x30, 0x20, "100000"                         },
	
	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x0f, 0x01, 0x0c, 0x08, "1"                              },
	{0x0f, 0x01, 0x0c, 0x04, "2"                              },
	{0x0f, 0x01, 0x0c, 0x0c, "3"                              },
	{0x0f, 0x01, 0x0c, 0x00, "5"                              },
};

STDDIPINFO(Aquajack)

static struct BurnDIPInfo AquajackjDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x7f, NULL                             },
	{0x0f, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0e, 0x01, 0x80, 0x80, "Cockpit"                        },
	{0x0e, 0x01, 0x80, 0x00, "Upright"                        },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0e, 0x01, 0x20, 0x20, "Off"                            },
	{0x0e, 0x01, 0x20, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0e, 0x01, 0x10, 0x00, "Off"                            },
	{0x0e, 0x01, 0x10, 0x10, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0e, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"               },
	{0x0e, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"               },
	{0x0e, 0x01, 0x0c, 0x00, "2 Coins 3 Credits"              },
	{0x0e, 0x01, 0x0c, 0x04, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0e, 0x01, 0x03, 0x02, "2 Coins 1 Credit"               },
	{0x0e, 0x01, 0x03, 0x03, "1 Coin  1 Credit"               },
	{0x0e, 0x01, 0x03, 0x00, "2 Coins 3 Credits"              },
	{0x0e, 0x01, 0x03, 0x01, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0f, 0x01, 0xc0, 0x40, "Easy"                           },
	{0x0f, 0x01, 0xc0, 0xc0, "Normal"                         },
	{0x0f, 0x01, 0xc0, 0x80, "Hard"                           },
	{0x0f, 0x01, 0xc0, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x0f, 0x01, 0x30, 0x00, "30000"                          },
	{0x0f, 0x01, 0x30, 0x30, "50000"                          },
	{0x0f, 0x01, 0x30, 0x10, "80000"                          },
	{0x0f, 0x01, 0x30, 0x20, "100000"                         },
	
	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x0f, 0x01, 0x0c, 0x08, "1"                              },
	{0x0f, 0x01, 0x0c, 0x04, "2"                              },
	{0x0f, 0x01, 0x0c, 0x0c, "3"                              },
	{0x0f, 0x01, 0x0c, 0x00, "5"                              },
};

STDDIPINFO(Aquajackj)

static struct BurnDIPInfo BsharkDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                             },
	{0x0c, 0xff, 0xff, 0xf7, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Mirror Screen"                  },
	{0x0b, 0x01, 0x01, 0x01, "Off"                            },
	{0x0b, 0x01, 0x01, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0b, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0b, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x0b, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x0b, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Speed of Sight"                 },
	{0x0c, 0x01, 0x0c, 0x0c, "Slow"                           },
	{0x0c, 0x01, 0x0c, 0x08, "Medium"                         },
	{0x0c, 0x01, 0x0c, 0x04, "Fast"                           },
	{0x0c, 0x01, 0x0c, 0x00, "Fastest"                        },
};

STDDIPINFO(Bshark)

static struct BurnDIPInfo BsharkjDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                             },
	{0x0c, 0xff, 0xff, 0xf7, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Mirror Screen"                  },
	{0x0b, 0x01, 0x01, 0x01, "Off"                            },
	{0x0b, 0x01, 0x01, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0b, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0b, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0b, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Speed of Sight"                 },
	{0x0c, 0x01, 0x0c, 0x0c, "Slow"                           },
	{0x0c, 0x01, 0x0c, 0x08, "Medium"                         },
	{0x0c, 0x01, 0x0c, 0x04, "Fast"                           },
	{0x0c, 0x01, 0x0c, 0x00, "Fastest"                        },
};

STDDIPINFO(Bsharkj)

static struct BurnDIPInfo BsharkjjsDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0xff, NULL                             },
	{0x0e, 0xff, 0xff, 0xf7, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Mirror Screen"                  },
	{0x0d, 0x01, 0x01, 0x01, "Off"                            },
	{0x0d, 0x01, 0x01, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0d, 0x01, 0x04, 0x04, "Off"                            },
	{0x0d, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0d, 0x01, 0x08, 0x00, "Off"                            },
	{0x0d, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0d, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0d, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0e, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0e, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0e, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0e, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Speed of Sight"                 },
	{0x0e, 0x01, 0x0c, 0x0c, "Slow"                           },
	{0x0e, 0x01, 0x0c, 0x08, "Medium"                         },
	{0x0e, 0x01, 0x0c, 0x04, "Fast"                           },
	{0x0e, 0x01, 0x0c, 0x00, "Fastest"                        },
};

STDDIPINFO(Bsharkjjs)

static struct BurnDIPInfo BsharkuDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                             },
	{0x0c, 0xff, 0xff, 0xf7, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Mirror Screen"                  },
	{0x0b, 0x01, 0x01, 0x01, "Off"                            },
	{0x0b, 0x01, 0x01, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x0b, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x0b, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0xc0, 0xc0, "Same as Start"                  },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Speed of Sight"                 },
	{0x0c, 0x01, 0x0c, 0x0c, "Slow"                           },
	{0x0c, 0x01, 0x0c, 0x08, "Medium"                         },
	{0x0c, 0x01, 0x0c, 0x04, "Fast"                           },
	{0x0c, 0x01, 0x0c, 0x00, "Fastest"                        },
};

STDDIPINFO(Bsharku)

static struct BurnDIPInfo ChasehqDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                             },
	{0x0c, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Cabinet"                        },
	{0x0b, 0x01, 0x03, 0x03, "Upright / Steering Lock"        },
	{0x0b, 0x01, 0x03, 0x02, "Upright / No Steering Lock"     },
	{0x0b, 0x01, 0x03, 0x01, "Full Throttle Convert, Cockpit" },
	{0x0b, 0x01, 0x03, 0x00, "Full Throttle Convert, Deluxe"  },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0b, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0b, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x0b, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x0b, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },	
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Timer Setting"                  },
	{0x0c, 0x01, 0x0c, 0x08, "70 Seconds"                     },
	{0x0c, 0x01, 0x0c, 0x04, "65 Seconds"                     },
	{0x0c, 0x01, 0x0c, 0x0c, "60 Seconds"                     },
	{0x0c, 0x01, 0x0c, 0x00, "55 Seconds"                     },
	
	{0   , 0xfe, 0   , 2   , "Turbos Stocked"                 },
	{0x0c, 0x01, 0x10, 0x10, "3"                              },
	{0x0c, 0x01, 0x10, 0x00, "5"                              },
	
	{0   , 0xfe, 0   , 2   , "Discounted Continue Play"       },
	{0x0c, 0x01, 0x20, 0x20, "Off"                            },
	{0x0c, 0x01, 0x20, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Damage Cleared at Continue"     },
	{0x0c, 0x01, 0x40, 0x00, "Off"                            },
	{0x0c, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0c, 0x01, 0x80, 0x00, "Off"                            },
	{0x0c, 0x01, 0x80, 0x80, "On"                             },
};

STDDIPINFO(Chasehq)

static struct BurnDIPInfo ChasehqjDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                             },
	{0x0c, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Cabinet"                        },
	{0x0b, 0x01, 0x03, 0x03, "Upright / Steering Lock"        },
	{0x0b, 0x01, 0x03, 0x02, "Upright / No Steering Lock"     },
	{0x0b, 0x01, 0x03, 0x01, "Full Throttle Convert, Cockpit" },
	{0x0b, 0x01, 0x03, 0x00, "Full Throttle Convert, Deluxe"  },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0b, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0b, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0b, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Timer Setting"                  },
	{0x0c, 0x01, 0x0c, 0x08, "70 Seconds"                     },
	{0x0c, 0x01, 0x0c, 0x04, "65 Seconds"                     },
	{0x0c, 0x01, 0x0c, 0x0c, "60 Seconds"                     },
	{0x0c, 0x01, 0x0c, 0x00, "55 Seconds"                     },
	
	{0   , 0xfe, 0   , 2   , "Turbos Stocked"                 },
	{0x0c, 0x01, 0x10, 0x10, "3"                              },
	{0x0c, 0x01, 0x10, 0x00, "5"                              },
	
	{0   , 0xfe, 0   , 2   , "Discounted Continue Play"       },
	{0x0c, 0x01, 0x20, 0x20, "Off"                            },
	{0x0c, 0x01, 0x20, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Damage Cleared at Continue"     },
	{0x0c, 0x01, 0x40, 0x00, "Off"                            },
	{0x0c, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0c, 0x01, 0x80, 0x00, "Off"                            },
	{0x0c, 0x01, 0x80, 0x80, "On"                             },
};

STDDIPINFO(Chasehqj)

static struct BurnDIPInfo ContcircDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0xff, NULL                             },
	{0x0f, 0xff, 0xff, 0xdf, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0e, 0x01, 0x01, 0x01, "Upright"                        },
	{0x0e, 0x01, 0x01, 0x00, "Cockpit"                        },
	
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
	{0   , 0xfe, 0   , 4   , "Time Difficulty"                },
	{0x0f, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0f, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0f, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0f, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Opponent Difficulty"            },
	{0x0f, 0x01, 0x0c, 0x08, "Easy"                           },
	{0x0f, 0x01, 0x0c, 0x0c, "Normal"                         },
	{0x0f, 0x01, 0x0c, 0x04, "Hard"                           },
	{0x0f, 0x01, 0x0c, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 2   , "Steering Wheel"                 },
	{0x0f, 0x01, 0x10, 0x10, "Free"                           },
	{0x0f, 0x01, 0x10, 0x00, "Locked"                         },
	
	{0   , 0xfe, 0   , 2   , "Enable 3D Alternate Frames"     },
	{0x0f, 0x01, 0x20, 0x00, "Off"                            },
	{0x0f, 0x01, 0x20, 0x20, "On"                             },
};

STDDIPINFO(Contcirc)

static struct BurnDIPInfo ContcircuDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0xff, NULL                             },
	{0x0f, 0xff, 0xff, 0xdf, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0e, 0x01, 0x01, 0x01, "Upright"                        },
	{0x0e, 0x01, 0x01, 0x00, "Cockpit"                        },
	
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
	{0   , 0xfe, 0   , 4   , "Time Difficulty"                },
	{0x0f, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0f, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0f, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0f, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Opponent Difficulty"            },
	{0x0f, 0x01, 0x0c, 0x08, "Easy"                           },
	{0x0f, 0x01, 0x0c, 0x0c, "Normal"                         },
	{0x0f, 0x01, 0x0c, 0x04, "Hard"                           },
	{0x0f, 0x01, 0x0c, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 2   , "Steering Wheel"                 },
	{0x0f, 0x01, 0x10, 0x10, "Free"                           },
	{0x0f, 0x01, 0x10, 0x00, "Locked"                         },
	
	{0   , 0xfe, 0   , 2   , "Enable 3D Alternate Frames"     },
	{0x0f, 0x01, 0x20, 0x00, "Off"                            },
	{0x0f, 0x01, 0x20, 0x20, "On"                             },
};

STDDIPINFO(Contcircu)

static struct BurnDIPInfo DblaxleDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0xff, NULL                             },
	{0x0e, 0xff, 0xff, 0xfb, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Gear Shift"                     },
	{0x0d, 0x01, 0x02, 0x02, "Normal"                         },
	{0x0d, 0x01, 0x02, 0x00, "Inverted"                       },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0d, 0x01, 0x04, 0x04, "Off"                            },
	{0x0d, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0d, 0x01, 0x08, 0x00, "Off"                            },
	{0x0d, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0d, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0d, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0e, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0e, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0e, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0e, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 2   , "Multi-machine hookup"           },
	{0x0e, 0x01, 0x04, 0x00, "Off"                            },
	{0x0e, 0x01, 0x04, 0x04, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Player Truck"                   },
	{0x0e, 0x01, 0x08, 0x08, "Red"                            },
	{0x0e, 0x01, 0x08, 0x00, "Blue"                           },
	
	{0   , 0xfe, 0   , 2   , "Reverse"                        },
	{0x0e, 0x01, 0x10, 0x10, "Normal"                         },
	{0x0e, 0x01, 0x10, 0x00, "Inverted"                       },
};

STDDIPINFO(Dblaxle)

static struct BurnDIPInfo PwheelsjDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0xff, NULL                             },
	{0x0e, 0xff, 0xff, 0xfb, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Gear Shift"                     },
	{0x0d, 0x01, 0x02, 0x02, "Normal"                         },
	{0x0d, 0x01, 0x02, 0x00, "Inverted"                       },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0d, 0x01, 0x04, 0x04, "Off"                            },
	{0x0d, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0d, 0x01, 0x08, 0x00, "Off"                            },
	{0x0d, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0d, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0d, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0e, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0e, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0e, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0e, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 2   , "Multi-machine hookup"           },
	{0x0e, 0x01, 0x04, 0x00, "Off"                            },
	{0x0e, 0x01, 0x04, 0x04, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Player Truck"                   },
	{0x0e, 0x01, 0x08, 0x08, "Red"                            },
	{0x0e, 0x01, 0x08, 0x00, "Blue"                           },
	
	{0   , 0xfe, 0   , 2   , "Reverse"                        },
	{0x0e, 0x01, 0x10, 0x10, "Normal"                         },
	{0x0e, 0x01, 0x10, 0x00, "Inverted"                       },
};

STDDIPINFO(Pwheelsj)

static struct BurnDIPInfo EnforceDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0xff, NULL                             },
	{0x0d, 0xff, 0xff, 0xef, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0c, 0x01, 0x04, 0x04, "Off"                            },
	{0x0c, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0c, 0x01, 0x08, 0x00, "Off"                            },
	{0x0c, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0c, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0c, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0c, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0c, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0c, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0c, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0c, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0c, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Background Scenery"             },
	{0x0d, 0x01, 0x10, 0x10, "Crazy Scolling"                 },
	{0x0d, 0x01, 0x10, 0x00, "Normal"                         },
};

STDDIPINFO(Enforce)

static struct BurnDIPInfo NightstrDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xfe, NULL                             },
	{0x0c, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0b, 0x01, 0x01, 0x01, "Cockpit"                        },
	{0x0b, 0x01, 0x01, 0x00, "Upright"                        },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0b, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0b, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x0b, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x0b, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },	
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Bonus Shields"                  },
	{0x0c, 0x01, 0x0c, 0x08, "3"                              },
	{0x0c, 0x01, 0x0c, 0x04, "2"                              },
	{0x0c, 0x01, 0x0c, 0x0c, "1"                              },
	{0x0c, 0x01, 0x0c, 0x00, "None"                           },
	
	{0   , 0xfe, 0   , 4   , "Shields"                        },
	{0x0c, 0x01, 0x30, 0x00, "3"                              },
	{0x0c, 0x01, 0x30, 0x10, "4"                              },
	{0x0c, 0x01, 0x30, 0x30, "5"                              },
	{0x0c, 0x01, 0x30, 0x20, "6"                              },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0c, 0x01, 0x40, 0x00, "Off"                            },
	{0x0c, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Trigger Turbo"                  },
	{0x0c, 0x01, 0x80, 0x80, "7 shots / seconds"              },
	{0x0c, 0x01, 0x80, 0x00, "10 shots / seconds"             },
};

STDDIPINFO(Nightstr)
	
static struct BurnDIPInfo NightstrjDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xfe, NULL                             },
	{0x0c, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0b, 0x01, 0x01, 0x01, "Cockpit"                        },
	{0x0b, 0x01, 0x01, 0x00, "Upright"                        },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0b, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0b, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0b, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Bonus Shields"                  },
	{0x0c, 0x01, 0x0c, 0x08, "3"                              },
	{0x0c, 0x01, 0x0c, 0x04, "2"                              },
	{0x0c, 0x01, 0x0c, 0x0c, "1"                              },
	{0x0c, 0x01, 0x0c, 0x00, "None"                           },
	
	{0   , 0xfe, 0   , 4   , "Shields"                        },
	{0x0c, 0x01, 0x30, 0x00, "3"                              },
	{0x0c, 0x01, 0x30, 0x10, "4"                              },
	{0x0c, 0x01, 0x30, 0x30, "5"                              },
	{0x0c, 0x01, 0x30, 0x20, "6"                              },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0c, 0x01, 0x40, 0x00, "Off"                            },
	{0x0c, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Trigger Turbo"                  },
	{0x0c, 0x01, 0x80, 0x80, "7 shots / seconds"              },
	{0x0c, 0x01, 0x80, 0x00, "10 shots / seconds"             },
};

STDDIPINFO(Nightstrj)


static struct BurnDIPInfo NightstruDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xfe, NULL                             },
	{0x0c, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0b, 0x01, 0x01, 0x01, "Cockpit"                        },
	{0x0b, 0x01, 0x01, 0x00, "Upright"                        },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x0b, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x0b, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0xc0, 0xc0, "Same as Start"                  },	
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Bonus Shields"                  },
	{0x0c, 0x01, 0x0c, 0x08, "3"                              },
	{0x0c, 0x01, 0x0c, 0x04, "2"                              },
	{0x0c, 0x01, 0x0c, 0x0c, "1"                              },
	{0x0c, 0x01, 0x0c, 0x00, "None"                           },
	
	{0   , 0xfe, 0   , 4   , "Shields"                        },
	{0x0c, 0x01, 0x30, 0x00, "3"                              },
	{0x0c, 0x01, 0x30, 0x10, "4"                              },
	{0x0c, 0x01, 0x30, 0x30, "5"                              },
	{0x0c, 0x01, 0x30, 0x20, "6"                              },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0c, 0x01, 0x40, 0x00, "Off"                            },
	{0x0c, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Trigger Turbo"                  },
	{0x0c, 0x01, 0x80, 0x80, "7 shots / seconds"              },
	{0x0c, 0x01, 0x80, 0x00, "10 shots / seconds"             },
};

STDDIPINFO(Nightstru)

static struct BurnDIPInfo SciDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0xfe, NULL                             },
	{0x0e, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0d, 0x01, 0x01, 0x01, "Cockpit"                        },
	{0x0d, 0x01, 0x01, 0x00, "Upright"                        },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0d, 0x01, 0x04, 0x04, "Off"                            },
	{0x0d, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0d, 0x01, 0x08, 0x00, "Off"                            },
	{0x0d, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0d, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0d, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x0d, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x0d, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },	
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0e, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0e, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0e, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0e, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Timer Setting"                  },
	{0x0e, 0x01, 0x0c, 0x08, "70 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x04, "65 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x0c, "60 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x00, "55 Seconds"                     },
	
	{0   , 0xfe, 0   , 2   , "Turbos Stocked"                 },
	{0x0e, 0x01, 0x10, 0x10, "3"                              },
	{0x0e, 0x01, 0x10, 0x00, "5"                              },
	
	{0   , 0xfe, 0   , 2   , "Steering Radius"                },
	{0x0e, 0x01, 0x20, 0x00, "270 Degree"                     },
	{0x0e, 0x01, 0x20, 0x20, "360 Degree"                     },

	{0   , 0xfe, 0   , 2   , "Damage Cleared at Continue"     },
	{0x0e, 0x01, 0x40, 0x00, "Off"                            },
	{0x0e, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Siren Volume"                   },
	{0x0e, 0x01, 0x80, 0x80, "Normal"                         },
	{0x0e, 0x01, 0x80, 0x00, "Low"                            },
};

STDDIPINFO(Sci)

static struct BurnDIPInfo ScijDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0xfe, NULL                             },
	{0x0e, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0d, 0x01, 0x01, 0x01, "Cockpit"                        },
	{0x0d, 0x01, 0x01, 0x00, "Upright"                        },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0d, 0x01, 0x04, 0x04, "Off"                            },
	{0x0d, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0d, 0x01, 0x08, 0x00, "Off"                            },
	{0x0d, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0d, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0d, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0e, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0e, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0e, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0e, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Timer Setting"                  },
	{0x0e, 0x01, 0x0c, 0x08, "70 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x04, "65 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x0c, "60 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x00, "55 Seconds"                     },
	
	{0   , 0xfe, 0   , 2   , "Turbos Stocked"                 },
	{0x0e, 0x01, 0x10, 0x10, "3"                              },
	{0x0e, 0x01, 0x10, 0x00, "5"                              },
	
	{0   , 0xfe, 0   , 2   , "Steering Radius"                },
	{0x0e, 0x01, 0x20, 0x00, "270 Degree"                     },
	{0x0e, 0x01, 0x20, 0x20, "360 Degree"                     },

	{0   , 0xfe, 0   , 2   , "Damage Cleared at Continue"     },
	{0x0e, 0x01, 0x40, 0x00, "Off"                            },
	{0x0e, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Siren Volume"                   },
	{0x0e, 0x01, 0x80, 0x80, "Normal"                         },
	{0x0e, 0x01, 0x80, 0x00, "Low"                            },
};

STDDIPINFO(Scij)

static struct BurnDIPInfo SciuDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0xfe, NULL                             },
	{0x0e, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0d, 0x01, 0x01, 0x01, "Cockpit"                        },
	{0x0d, 0x01, 0x01, 0x00, "Upright"                        },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0d, 0x01, 0x04, 0x04, "Off"                            },
	{0x0d, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0d, 0x01, 0x08, 0x00, "Off"                            },
	{0x0d, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x0d, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x0d, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  1 Credit"               },
	{0x0d, 0x01, 0xc0, 0xc0, "Same as Start"                  },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0e, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0e, 0x01, 0x03, 0x03, "Normal"                         },
	{0x0e, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0e, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 4   , "Timer Setting"                  },
	{0x0e, 0x01, 0x0c, 0x08, "70 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x04, "65 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x0c, "60 Seconds"                     },
	{0x0e, 0x01, 0x0c, 0x00, "55 Seconds"                     },
	
	{0   , 0xfe, 0   , 2   , "Turbos Stocked"                 },
	{0x0e, 0x01, 0x10, 0x10, "3"                              },
	{0x0e, 0x01, 0x10, 0x00, "5"                              },
	
	{0   , 0xfe, 0   , 2   , "Steering Radius"                },
	{0x0e, 0x01, 0x20, 0x00, "270 Degree"                     },
	{0x0e, 0x01, 0x20, 0x20, "360 Degree"                     },

	{0   , 0xfe, 0   , 2   , "Damage Cleared at Continue"     },
	{0x0e, 0x01, 0x40, 0x00, "Off"                            },
	{0x0e, 0x01, 0x40, 0x40, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Siren Volume"                   },
	{0x0e, 0x01, 0x80, 0x80, "Normal"                         },
	{0x0e, 0x01, 0x80, 0x00, "Low"                            },
};

STDDIPINFO(Sciu)

static struct BurnDIPInfo SpacegunDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0xff, NULL                             },
	{0x11, 0xff, 0xff, 0xff, NULL                             },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Always have gunsight power up"  },
	{0x10, 0x01, 0x02, 0x02, "Off"                            },
	{0x10, 0x01, 0x02, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x10, 0x01, 0x04, 0x04, "Off"                            },
	{0x10, 0x01, 0x04, 0x00, "On"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x10, 0x01, 0x08, 0x00, "Off"                            },
	{0x10, 0x01, 0x08, 0x08, "On"                             },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x10, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x10, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x10, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x10, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x10, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x10, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x10, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x10, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },	
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x11, 0x01, 0x03, 0x02, "Easy"                           },
	{0x11, 0x01, 0x03, 0x03, "Normal"                         },
	{0x11, 0x01, 0x03, 0x01, "Hard"                           },
	{0x11, 0x01, 0x03, 0x00, "Hardest"                        },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x11, 0x01, 0x40, 0x00, "Off"                            },
	{0x11, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Disable Pedal"                  },
	{0x11, 0x01, 0x80, 0x80, "Off"                            },
	{0x11, 0x01, 0x80, 0x00, "On"                             },	
};

STDDIPINFO(Spacegun)

static struct BurnRomInfo AquajackRomDesc[] = {
	{ "b77-22.ic31",   0x20000, 0x67400dde, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b77-26.ic17",   0x20000, 0xcd4d0969, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b77-24.ic69",   0x20000, 0x95e643ed, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b77-23.ic67",   0x20000, 0x395a7d1c, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b77-20.ic54",   0x10000, 0x84ba54b7, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b77-05.ic105",  0x80000, 0x7238f0ff, BRF_GRA | TAITO_CHARS },
	
	{ "b77-04.ic16",   0x80000, 0xbed0be6c, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-03.ic15",   0x80000, 0x9a3030a7, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-02.ic14",   0x80000, 0xdaea0d2e, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-01.ic13",   0x80000, 0xcdab000d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b77-07.ic33",   0x80000, 0x7db1fc5e, BRF_GRA | TAITO_ROAD },
	
	{ "b77-06.ic39",   0x80000, 0xce2aed00, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b77-09.ic58",   0x80000, 0x948e5ad9, BRF_SND | TAITO_YM2610A },
	
	{ "b77-08.ic57",   0x80000, 0x119b9485, BRF_SND | TAITO_YM2610B },
	
	{ "b77-17.ic1",    0x00100, 0xfbf81f30, BRF_OPT },
	{ "b77-18.ic37",   0x00100, 0x7b7d8ff4, BRF_OPT },
};

STD_ROM_PICK(Aquajack)
STD_ROM_FN(Aquajack)

static struct BurnRomInfo AquajackuRomDesc[] = {
	{ "b77-22.ic31",   0x20000, 0x67400dde, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b77-25.ic17",   0x20000, 0xba4a39ff, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b77-24.ic69",   0x20000, 0x95e643ed, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b77-23.ic67",   0x20000, 0x395a7d1c, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b77-20.ic54",   0x10000, 0x84ba54b7, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b77-05.ic105",  0x80000, 0x7238f0ff, BRF_GRA | TAITO_CHARS },
	
	{ "b77-04.ic16",   0x80000, 0xbed0be6c, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-03.ic15",   0x80000, 0x9a3030a7, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-02.ic14",   0x80000, 0xdaea0d2e, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-01.ic13",   0x80000, 0xcdab000d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b77-07.ic33",   0x80000, 0x7db1fc5e, BRF_GRA | TAITO_ROAD },
	
	{ "b77-06.ic39",   0x80000, 0xce2aed00, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b77-09.ic58",   0x80000, 0x948e5ad9, BRF_SND | TAITO_YM2610A },
	
	{ "b77-08.ic57",   0x80000, 0x119b9485, BRF_SND | TAITO_YM2610B },
	
	{ "b77-17.ic1",    0x00100, 0xfbf81f30, BRF_OPT },
	{ "b77-18.ic37",   0x00100, 0x7b7d8ff4, BRF_OPT },
};

STD_ROM_PICK(Aquajacku)
STD_ROM_FN(Aquajacku)

static struct BurnRomInfo AquajackjRomDesc[] = {
	{ "b77-22.ic31",   0x20000, 0x67400dde, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b77-21.ic17",   0x20000, 0x23436845, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b77-24.ic69",   0x20000, 0x95e643ed, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b77-23.ic67",   0x20000, 0x395a7d1c, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b77-20.ic54",   0x10000, 0x84ba54b7, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b77-05.ic105",  0x80000, 0x7238f0ff, BRF_GRA | TAITO_CHARS },
	
	{ "b77-04.ic16",   0x80000, 0xbed0be6c, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-03.ic15",   0x80000, 0x9a3030a7, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-02.ic14",   0x80000, 0xdaea0d2e, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b77-01.ic13",   0x80000, 0xcdab000d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b77-07.ic33",   0x80000, 0x7db1fc5e, BRF_GRA | TAITO_ROAD },
	
	{ "b77-06.ic39",   0x80000, 0xce2aed00, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b77-09.ic58",   0x80000, 0x948e5ad9, BRF_SND | TAITO_YM2610A },
	
	{ "b77-08.ic57",   0x80000, 0x119b9485, BRF_SND | TAITO_YM2610B },
	
	{ "b77-17.ic1",    0x00100, 0xfbf81f30, BRF_OPT },
	{ "b77-18.ic37",   0x00100, 0x7b7d8ff4, BRF_OPT },
};

STD_ROM_PICK(Aquajackj)
STD_ROM_FN(Aquajackj)

static struct BurnRomInfo BsharkRomDesc[] = {
	{ "c34_71.98",     0x20000, 0xdf1fa629, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_69.75",     0x20000, 0xa54c137a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_70.97",     0x20000, 0xd77d81e2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_68.74",     0x20000, 0x4e374ce2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c34_74.128",    0x20000, 0x6869fa99, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_72.112",    0x20000, 0xc09c0f91, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_75.129",    0x20000, 0x6ba65542, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_73.113",    0x20000, 0xf2fe62b5, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c34_05.3",      0x80000, 0x596b83da, BRF_GRA | TAITO_CHARS },
	
	{ "c34_04.17",     0x80000, 0x2446b0da, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_03.16",     0x80000, 0xa18eab78, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_02.15",     0x80000, 0x8488ba10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_01.14",     0x80000, 0x3ebe8c63, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c34_07.42",     0x80000, 0xedb07808, BRF_GRA | TAITO_ROAD },
	
	{ "c34_06.12",     0x80000, 0xd200b6eb, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c34_08.127",    0x80000, 0x89a30450, BRF_SND | TAITO_YM2610A },
	
	{ "c34_09.126",    0x80000, 0x39d12b50, BRF_SND | TAITO_YM2610B },
	
	{ "c34_18.22",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c34_19.72",     0x00100, 0x2ee9c404, BRF_OPT },
	{ "c34_20.89",     0x00100, 0xfbf81f30, BRF_OPT },
	{ "c34_21.7",      0x00400, 0x10728853, BRF_OPT },
	{ "c34_22.8",      0x00400, 0x643e8bfc, BRF_OPT },
};

STD_ROM_PICK(Bshark)
STD_ROM_FN(Bshark)

static struct BurnRomInfo BsharkjRomDesc[] = {
	{ "c34_71.98",     0x20000, 0xdf1fa629, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_69.75",     0x20000, 0xa54c137a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_70.97",     0x20000, 0xd77d81e2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_66.74",     0x20000, 0xa0392dce, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c34_74.128",    0x20000, 0x6869fa99, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_72.112",    0x20000, 0xc09c0f91, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_75.129",    0x20000, 0x6ba65542, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_73.113",    0x20000, 0xf2fe62b5, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c34_05.3",      0x80000, 0x596b83da, BRF_GRA | TAITO_CHARS },
	
	{ "c34_04.17",     0x80000, 0x2446b0da, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_03.16",     0x80000, 0xa18eab78, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_02.15",     0x80000, 0x8488ba10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_01.14",     0x80000, 0x3ebe8c63, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c34_07.42",     0x80000, 0xedb07808, BRF_GRA | TAITO_ROAD },
	
	{ "c34_06.12",     0x80000, 0xd200b6eb, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c34_08.127",    0x80000, 0x89a30450, BRF_SND | TAITO_YM2610A },
	
	{ "c34_09.126",    0x80000, 0x39d12b50, BRF_SND | TAITO_YM2610B },
	
	{ "c34_18.22",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c34_19.72",     0x00100, 0x2ee9c404, BRF_OPT },
	{ "c34_20.89",     0x00100, 0xfbf81f30, BRF_OPT },
	{ "c34_21.7",      0x00400, 0x10728853, BRF_OPT },
	{ "c34_22.8",      0x00400, 0x643e8bfc, BRF_OPT },
};

STD_ROM_PICK(Bsharkj)
STD_ROM_FN(Bsharkj)

static struct BurnRomInfo BsharkjjsRomDesc[] = {
	{ "c34_79.98",     0x20000, 0xbc3f2e93, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_77.75",     0x20000, 0x917916d0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_78.97",     0x20000, 0xf2fcc880, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_76.74",     0x20000, 0xde97fac0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c34_82.128",    0x20000, 0x6869fa99, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_80.112",    0x20000, 0xe1783eb4, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_83.129",    0x20000, 0xeec0b364, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_81.113",    0x20000, 0x23ce6bcf, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c34_05.3",      0x80000, 0x596b83da, BRF_GRA | TAITO_CHARS },
	
	{ "c34_04.17",     0x80000, 0x2446b0da, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_03.16",     0x80000, 0xa18eab78, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_02.15",     0x80000, 0x8488ba10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_01.14",     0x80000, 0x3ebe8c63, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c34_07.42",     0x80000, 0xedb07808, BRF_GRA | TAITO_ROAD },
	
	{ "c34_06.12",     0x80000, 0xd200b6eb, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c34_08.127",    0x80000, 0x89a30450, BRF_SND | TAITO_YM2610A },
	
	{ "c34_09.126",    0x80000, 0x39d12b50, BRF_SND | TAITO_YM2610B },
	
	{ "c34_18.22",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c34_19.72",     0x00100, 0x2ee9c404, BRF_OPT },
	{ "c34_20.89",     0x00100, 0xfbf81f30, BRF_OPT },
	{ "c34_21.7",      0x00400, 0x10728853, BRF_OPT },
	{ "c34_22.8",      0x00400, 0x643e8bfc, BRF_OPT },
};

STD_ROM_PICK(Bsharkjjs)
STD_ROM_FN(Bsharkjjs)

static struct BurnRomInfo BsharkuRomDesc[] = {
	{ "c34_71.98",     0x20000, 0xdf1fa629, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_69.75",     0x20000, 0xa54c137a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_70.97",     0x20000, 0xd77d81e2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c34_67.74",     0x20000, 0x39307c74, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c34_74.128",    0x20000, 0x6869fa99, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_72.112",    0x20000, 0xc09c0f91, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_75.129",    0x20000, 0x6ba65542, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c34_73.113",    0x20000, 0xf2fe62b5, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c34_05.3",      0x80000, 0x596b83da, BRF_GRA | TAITO_CHARS },
	
	{ "c34_04.17",     0x80000, 0x2446b0da, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_03.16",     0x80000, 0xa18eab78, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_02.15",     0x80000, 0x8488ba10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c34_01.14",     0x80000, 0x3ebe8c63, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c34_07.42",     0x80000, 0xedb07808, BRF_GRA | TAITO_ROAD },
	
	{ "c34_06.12",     0x80000, 0xd200b6eb, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c34_08.127",    0x80000, 0x89a30450, BRF_SND | TAITO_YM2610A },
	
	{ "c34_09.126",    0x80000, 0x39d12b50, BRF_SND | TAITO_YM2610B },
	
	{ "c34_18.22",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c34_19.72",     0x00100, 0x2ee9c404, BRF_OPT },
	{ "c34_20.89",     0x00100, 0xfbf81f30, BRF_OPT },
	{ "c34_21.7",      0x00400, 0x10728853, BRF_OPT },
	{ "c34_22.8",      0x00400, 0x643e8bfc, BRF_OPT },
};

STD_ROM_PICK(Bsharku)
STD_ROM_FN(Bsharku)

static struct BurnRomInfo ChasehqRomDesc[] = {
	{ "b52-130.36",    0x20000, 0x4e7beb46, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-136.29",    0x20000, 0x2f414df0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-131.37",    0x20000, 0xaa945d83, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-129.30",    0x20000, 0x0eaebc08, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b52-132.39",    0x10000, 0xa2f54789, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b52-133.55",    0x10000, 0x12232f95, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b52-137.51",    0x10000, 0x37abb74a, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b52-29.27",     0x80000, 0x8366d27c, BRF_GRA | TAITO_CHARS},
	
	{ "b52-34.5",      0x80000, 0x7d8dce36, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-35.7",      0x80000, 0x78eeec0d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-36.9",      0x80000, 0x61e89e91, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-37.11",     0x80000, 0xf02e47b9, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b52-30.4",      0x80000, 0x1b8cc647, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-31.6",      0x80000, 0xf1998e20, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-32.8",      0x80000, 0x8620780c, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-33.10",     0x80000, 0xe6f4b8c4, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	
	{ "b52-28.4",      0x80000, 0x963bc82b, BRF_GRA | TAITO_ROAD },
	
	{ "b52-38.34",     0x80000, 0x5b5bf7f6, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b52-115.71",    0x80000, 0x4e117e93, BRF_SND | TAITO_YM2610A },
	{ "b52-114.72",    0x80000, 0x3a73d6b1, BRF_SND | TAITO_YM2610A },
	{ "b52-113.73",    0x80000, 0x2c6a3a05, BRF_SND | TAITO_YM2610A },
	
	{ "b52-116.70",    0x80000, 0xad46983c, BRF_SND | TAITO_YM2610B },
	
	{ "b52-01.7",      0x00100, 0x89719d17, BRF_OPT },
	{ "b52-03.135",    0x00400, 0xa3f8490d, BRF_OPT },
	{ "b52-06.24",     0x00100, 0xfbf81f30, BRF_OPT },
	{ "b52-18.93",     0x00100, 0x60bdaf1a, BRF_OPT },
	{ "b52-18a",       0x00100, 0x6271be0d, BRF_OPT },
	{ "b52-49.68",     0x02000, 0x60dd2ed1, BRF_OPT },
	{ "b52-50.66",     0x10000, 0xc189781c, BRF_OPT },
	{ "b52-51.65",     0x10000, 0x30cc1f79, BRF_OPT },
	{ "b52-126.136",   0x00400, 0xfa2f840e, BRF_OPT },
	{ "b52-127.156",   0x00400, 0x77682a4f, BRF_OPT },
	
	{ "pal20l8b-b52-17.ic18",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic16",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic53",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic55",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal16l8b-b52-19.ic33",   0x00104, 0x3ba292dc, BRF_OPT },
	{ "pal16l8b-b52-20.ic35",   0x00104, 0xbd39ad73, BRF_OPT },
	{ "pal16l8b-b52-21.ic51",   0x00104, 0x2fe76aa4, BRF_OPT },
	{ "pal20l8b-b52-25.ic123",  0x00144, 0x372b632d, BRF_OPT },
	{ "pal20l8b-b52-26.ic15",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic18",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic52",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic54",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-27.ic64",   0x00144, 0x61c2ab26, BRF_OPT },
	{ "pal20l8b-b52-118.ic20",  0x00144, 0x9c5fe4af, BRF_OPT },
	{ "pal20l8b-b52-119.ic21",  0x00144, 0x8b8e2106, BRF_OPT },
	{ "pal16l8b-b52-120.ic56",  0x00104, 0x3e7effa0, BRF_OPT },
	{ "pal20l8b-b52-121.ic57",  0x00144, 0x7056fd1d, BRF_OPT },
	{ "pal16l8b-b52-122.ic124", 0x00104, 0x04c0fb04, BRF_OPT },
	{ "pal16l8b-b52-123.ic125", 0x00104, 0x3865d1c8, BRF_OPT },
	{ "pal16l8b-b52-124.ic180", 0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "pal16l8b-b52-125.ic112", 0x00104, 0x7628c557, BRF_OPT },
};

STD_ROM_PICK(Chasehq)
STD_ROM_FN(Chasehq)

static struct BurnRomInfo ChasehqjRomDesc[] = {
	{ "b52-140.36",    0x20000, 0xc1298a4b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-139.29",    0x20000, 0x997f732e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-131.37",    0x20000, 0xaa945d83, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-129.30",    0x20000, 0x0eaebc08, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b52-132.39",    0x10000, 0xa2f54789, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b52-133.55",    0x10000, 0x12232f95, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b52-134.51",    0x10000, 0x91faac7f, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b52-29.27",     0x80000, 0x8366d27c, BRF_GRA | TAITO_CHARS },
	
	{ "b52-34.5",      0x80000, 0x7d8dce36, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-35.7",      0x80000, 0x78eeec0d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-36.9",      0x80000, 0x61e89e91, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-37.11",     0x80000, 0xf02e47b9, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b52-30.4",      0x80000, 0x1b8cc647, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-31.6",      0x80000, 0xf1998e20, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-32.8",      0x80000, 0x8620780c, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-33.10",     0x80000, 0xe6f4b8c4, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	
	{ "b52-28.4",      0x80000, 0x963bc82b, BRF_GRA | TAITO_ROAD },
	
	{ "b52-38.34",     0x80000, 0x5b5bf7f6, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b52-41.71",     0x80000, 0x8204880c, BRF_SND | TAITO_YM2610A },
	{ "b52-40.72",     0x80000, 0xf0551055, BRF_SND | TAITO_YM2610A },
	{ "b52-39.73",     0x80000, 0xac9cbbd3, BRF_SND | TAITO_YM2610A },
	
	{ "b52-42.70",     0x80000, 0x6e617df1, BRF_SND | TAITO_YM2610B },
	
	{ "b52-01.7",      0x00100, 0x89719d17, BRF_OPT },
	{ "b52-03.135",    0x00400, 0xa3f8490d, BRF_OPT },
	{ "b52-06.24",     0x00100, 0xfbf81f30, BRF_OPT },
	{ "b52-18.93",     0x00100, 0x60bdaf1a, BRF_OPT },
	{ "b52-18a",       0x00100, 0x6271be0d, BRF_OPT },
	{ "b52-49.68",     0x02000, 0x60dd2ed1, BRF_OPT },
	{ "b52-50.66",     0x10000, 0xc189781c, BRF_OPT },
	{ "b52-51.65",     0x10000, 0x30cc1f79, BRF_OPT },
	{ "b52-126.136",   0x00400, 0xfa2f840e, BRF_OPT },
	{ "b52-127.156",   0x00400, 0x77682a4f, BRF_OPT },
	
	{ "pal20l8b-b52-17.ic18",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic16",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic53",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic55",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal16l8b-b52-19.ic33",   0x00104, 0x3ba292dc, BRF_OPT },
	{ "pal16l8b-b52-20.ic35",   0x00104, 0xbd39ad73, BRF_OPT },
	{ "pal16l8b-b52-21.ic51",   0x00104, 0x2fe76aa4, BRF_OPT },
	{ "pal20l8b-b52-25.ic123",  0x00144, 0x372b632d, BRF_OPT },
	{ "pal20l8b-b52-26.ic15",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic18",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic52",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic54",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-27.ic64",   0x00144, 0x61c2ab26, BRF_OPT },
	{ "pal20l8b-b52-118.ic20",  0x00144, 0x9c5fe4af, BRF_OPT },
	{ "pal20l8b-b52-119.ic21",  0x00144, 0x8b8e2106, BRF_OPT },
	{ "pal16l8b-b52-120.ic56",  0x00104, 0x3e7effa0, BRF_OPT },
	{ "pal20l8b-b52-121.ic57",  0x00144, 0x7056fd1d, BRF_OPT },
	{ "pal16l8b-b52-122.ic124", 0x00104, 0x04c0fb04, BRF_OPT },
	{ "pal16l8b-b52-123.ic125", 0x00104, 0x3865d1c8, BRF_OPT },
	{ "pal16l8b-b52-124.ic180", 0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "pal16l8b-b52-125.ic112", 0x00104, 0x7628c557, BRF_OPT },
};

STD_ROM_PICK(Chasehqj)
STD_ROM_FN(Chasehqj)

static struct BurnRomInfo ChasehquRomDesc[] = {
	{ "b52-138.36",    0x20000, 0x8b71fe51, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-135.29",    0x20000, 0x5ba56a7c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-131.37",    0x20000, 0xaa945d83, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b52-129.30",    0x20000, 0x0eaebc08, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b52-132.39",    0x10000, 0xa2f54789, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b52-133.55",    0x10000, 0x12232f95, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b52-137.51",    0x10000, 0x37abb74a, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b52-29.27",     0x80000, 0x8366d27c, BRF_GRA | TAITO_CHARS},
	
	{ "b52-34.5",      0x80000, 0x7d8dce36, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-35.7",      0x80000, 0x78eeec0d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-36.9",      0x80000, 0x61e89e91, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b52-37.11",     0x80000, 0xf02e47b9, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b52-30.4",      0x80000, 0x1b8cc647, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-31.6",      0x80000, 0xf1998e20, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-32.8",      0x80000, 0x8620780c, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b52-33.10",     0x80000, 0xe6f4b8c4, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	
	{ "b52-28.4",      0x80000, 0x963bc82b, BRF_GRA | TAITO_ROAD },
	
	{ "b52-38.34",     0x80000, 0x5b5bf7f6, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b52-115.71",    0x80000, 0x4e117e93, BRF_SND | TAITO_YM2610A },
	{ "b52-114.72",    0x80000, 0x3a73d6b1, BRF_SND | TAITO_YM2610A },
	{ "b52-113.73",    0x80000, 0x2c6a3a05, BRF_SND | TAITO_YM2610A },
	
	{ "b52-116.70",    0x80000, 0xad46983c, BRF_SND | TAITO_YM2610B },
	
	{ "b52-01.7",      0x00100, 0x89719d17, BRF_OPT },
	{ "b52-03.135",    0x00400, 0xa3f8490d, BRF_OPT },
	{ "b52-06.24",     0x00100, 0xfbf81f30, BRF_OPT },
	{ "b52-18.93",     0x00100, 0x60bdaf1a, BRF_OPT },
	{ "b52-18a",       0x00100, 0x6271be0d, BRF_OPT },
	{ "b52-49.68",     0x02000, 0x60dd2ed1, BRF_OPT },
	{ "b52-50.66",     0x10000, 0xc189781c, BRF_OPT },
	{ "b52-51.65",     0x10000, 0x30cc1f79, BRF_OPT },
	{ "b52-126.136",   0x00400, 0xfa2f840e, BRF_OPT },
	{ "b52-127.156",   0x00400, 0x77682a4f, BRF_OPT },
	
	{ "pal20l8b-b52-17.ic18",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic16",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic53",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal20l8b-b52-17.ic55",   0x00144, 0x4851316d, BRF_OPT },
	{ "pal16l8b-b52-19.ic33",   0x00104, 0x3ba292dc, BRF_OPT },
	{ "pal16l8b-b52-20.ic35",   0x00104, 0xbd39ad73, BRF_OPT },
	{ "pal16l8b-b52-21.ic51",   0x00104, 0x2fe76aa4, BRF_OPT },
	{ "pal20l8b-b52-25.ic123",  0x00144, 0x372b632d, BRF_OPT },
	{ "pal20l8b-b52-26.ic15",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic18",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic52",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-26.ic54",   0x00144, 0xd94f2bc2, BRF_OPT },
	{ "pal20l8b-b52-27.ic64",   0x00144, 0x61c2ab26, BRF_OPT },
	{ "pal20l8b-b52-118.ic20",  0x00144, 0x9c5fe4af, BRF_OPT },
	{ "pal20l8b-b52-119.ic21",  0x00144, 0x8b8e2106, BRF_OPT },
	{ "pal16l8b-b52-120.ic56",  0x00104, 0x3e7effa0, BRF_OPT },
	{ "pal20l8b-b52-121.ic57",  0x00144, 0x7056fd1d, BRF_OPT },
	{ "pal16l8b-b52-122.ic124", 0x00104, 0x04c0fb04, BRF_OPT },
	{ "pal16l8b-b52-123.ic125", 0x00104, 0x3865d1c8, BRF_OPT },
	{ "pal16l8b-b52-124.ic180", 0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "pal16l8b-b52-125.ic112", 0x00104, 0x7628c557, BRF_OPT },
};

STD_ROM_PICK(Chasehqu)
STD_ROM_FN(Chasehqu)

static struct BurnRomInfo ContcircRomDesc[] = {
	{ "ic25",          0x20000, 0xf5c92e42, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic26",          0x20000, 0xe7c1d1fa, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "ic35",          0x20000, 0x16522f2d, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "cc_36.bin",     0x20000, 0xa1732ea5, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b33-30.11",     0x10000, 0xd8746234, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b33-02.57",     0x80000, 0xf6fb3ba2, BRF_GRA | TAITO_CHARS },
	
	{ "b33-06",        0x80000, 0x2cb40599, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-05",        0x80000, 0xbddf9eea, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-04",        0x80000, 0x8df866a2, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-03",        0x80000, 0x4f6c36d9, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b33-01.3",      0x80000, 0xf11f2be8, BRF_GRA | TAITO_ROAD },
	
	{ "b33-07.64",     0x80000, 0x151e1f52, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b33-09.18",     0x80000, 0x1e6724b5, BRF_SND | TAITO_YM2610A },
	{ "b33-10.17",     0x80000, 0xe9ce03ab, BRF_SND | TAITO_YM2610A },
	
	{ "b33-08.19",     0x80000, 0xcaa1c4c8, BRF_SND | TAITO_YM2610B },
	
	{ "b14-30.97",     0x10000, 0xdccb0c7f, BRF_OPT },
	{ "b14-31.50",     0x02000, 0x5c6b013d, BRF_OPT },
	{ "b33-17.16",     0x00100, 0x7b7d8ff4, BRF_OPT },
	{ "b33-18.17",     0x00100, 0xfbf81f30, BRF_OPT },
};

STD_ROM_PICK(Contcirc)
STD_ROM_FN(Contcirc)

static struct BurnRomInfo ContcircuRomDesc[] = {
	{ "ic25",          0x20000, 0xf5c92e42, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic26",          0x20000, 0xe7c1d1fa, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "ic35",          0x20000, 0x16522f2d, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "ic36",          0x20000, 0xd6741e33, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b33-30.11",     0x10000, 0xd8746234, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b33-02.57",     0x80000, 0xf6fb3ba2, BRF_GRA | TAITO_CHARS },
	
	{ "b33-06",        0x80000, 0x2cb40599, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-05",        0x80000, 0xbddf9eea, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-04",        0x80000, 0x8df866a2, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-03",        0x80000, 0x4f6c36d9, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b33-01.3",      0x80000, 0xf11f2be8, BRF_GRA | TAITO_ROAD },
	
	{ "b33-07.64",     0x80000, 0x151e1f52, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b33-09.18",     0x80000, 0x1e6724b5, BRF_SND | TAITO_YM2610A },
	{ "b33-10.17",     0x80000, 0xe9ce03ab, BRF_SND | TAITO_YM2610A },
	
	{ "b33-08.19",     0x80000, 0xcaa1c4c8, BRF_SND | TAITO_YM2610B },
	
	{ "b14-30.97",     0x10000, 0xdccb0c7f, BRF_OPT },
	{ "b14-31.50",     0x02000, 0x5c6b013d, BRF_OPT },
	{ "b33-17.16",     0x00100, 0x7b7d8ff4, BRF_OPT },
	{ "b33-18.17",     0x00100, 0xfbf81f30, BRF_OPT },
};

STD_ROM_PICK(Contcircu)
STD_ROM_FN(Contcircu)

static struct BurnRomInfo ContcircuaRomDesc[] = {
	{ "b33-34.ic25",   0x20000, 0xe1e016c1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b33-33.ic26",   0x20000, 0xf539d44b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "21-2.ic35",     0x20000, 0x2723f9e3, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "31-1.ic36",     0x20000, 0x438431f7, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b33-30.11",     0x10000, 0xd8746234, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b33-02.57",     0x80000, 0xf6fb3ba2, BRF_GRA | TAITO_CHARS },
	
	{ "b33-06",        0x80000, 0x2cb40599, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-05",        0x80000, 0xbddf9eea, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-04",        0x80000, 0x8df866a2, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b33-03",        0x80000, 0x4f6c36d9, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b33-01.3",      0x80000, 0xf11f2be8, BRF_GRA | TAITO_ROAD },
	
	{ "b33-07.64",     0x80000, 0x151e1f52, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b33-09.18",     0x80000, 0x1e6724b5, BRF_SND | TAITO_YM2610A },
	{ "b33-10.17",     0x80000, 0xe9ce03ab, BRF_SND | TAITO_YM2610A },
	
	{ "b33-08.19",     0x80000, 0xcaa1c4c8, BRF_SND | TAITO_YM2610B },
	
	{ "b14-30.97",     0x10000, 0xdccb0c7f, BRF_OPT },
	{ "b14-31.50",     0x02000, 0x5c6b013d, BRF_OPT },
	{ "b33-17.16",     0x00100, 0x7b7d8ff4, BRF_OPT },
	{ "b33-18.17",     0x00100, 0xfbf81f30, BRF_OPT },
};

STD_ROM_PICK(Contcircua)
STD_ROM_FN(Contcircua)

static struct BurnRomInfo DblaxleRomDesc[] = {
	{ "c78-41-1.2",    0x020000, 0xcf297fe4, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c78-43-1.4",    0x020000, 0x38a8bad6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c78-42-1.3",    0x020000, 0x4124ab2b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c78-44-1.5",    0x020000, 0x50a55b6e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c78-30-1.35",   0x020000, 0x026aac18, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c78-31-1.36",   0x020000, 0x67ce23e8, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c78-34.c42",    0x020000, 0xf2186943, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c78-10.12",     0x080000, 0x44b1897c, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "c78-11.11",     0x080000, 0x7db3d4a3, BRF_GRA | TAITO_CHARS_BYTESWAP },
	
	{ "c78-08.25",     0x100000, 0x6c725211, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c78-07.33",     0x100000, 0x9da00d5b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c78-06.23",     0x100000, 0x8309e91b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c78-05.31",     0x100000, 0x90001f68, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c78-09.12",     0x080000, 0x0dbde6f5, BRF_GRA | TAITO_ROAD },
	
	{ "c78-04.3",      0x080000, 0xcc1aa37c, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c78-12.33",     0x100000, 0xb0267404, BRF_SND | TAITO_YM2610A },
	{ "c78-13.46",     0x080000, 0x1b363aa2, BRF_SND | TAITO_YM2610A },
	
	{ "c78-14.31",     0x080000, 0x9cad4dfb, BRF_SND | TAITO_YM2610B },
	
	{ "c78-25.15",     0x010000, 0x7245a6f6, BRF_OPT },
	{ "c78-15.22",     0x000100, 0xfbf81f30, BRF_OPT },
	{ "c78-21.74",     0x000100, 0x2926bf27, BRF_OPT },
	{ "c84-10.16",     0x000400, 0x643e8bfc, BRF_OPT },
	{ "c84-11.17",     0x000400, 0x10728853, BRF_OPT },
};

STD_ROM_PICK(Dblaxle)
STD_ROM_FN(Dblaxle)

static struct BurnRomInfo PwheelsjRomDesc[] = {
	{ "c78-26-2.2",    0x020000, 0x25c8eb2e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c78-28-2.4",    0x020000, 0xa9500eb1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c78-27-2.3",    0x020000, 0x08d2cffb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c78-29-2.5",    0x020000, 0xe1608004, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c78-30-1.35",   0x020000, 0x026aac18, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c78-31-1.36",   0x020000, 0x67ce23e8, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c78-32.42",     0x020000, 0x1494199c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c78-10.12",     0x080000, 0x44b1897c, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "c78-11.11",     0x080000, 0x7db3d4a3, BRF_GRA | TAITO_CHARS_BYTESWAP },
	
	{ "c78-08.25",     0x100000, 0x6c725211, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c78-07.33",     0x100000, 0x9da00d5b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c78-06.23",     0x100000, 0x8309e91b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c78-05.31",     0x100000, 0x90001f68, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c78-09.12",     0x080000, 0x0dbde6f5, BRF_GRA | TAITO_ROAD },
	
	{ "c78-04.3",      0x080000, 0xcc1aa37c, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c78-01.33",     0x100000, 0x90ff1e72, BRF_SND | TAITO_YM2610A },
	{ "c78-02.46",     0x080000, 0x8882d2b7, BRF_SND | TAITO_YM2610A },
	
	{ "c78-03.31",     0x080000, 0x9b926a2f, BRF_SND | TAITO_YM2610B },
	
	{ "c78-25.15",     0x010000, 0x7245a6f6, BRF_OPT },
	{ "c78-15.22",     0x000100, 0xfbf81f30, BRF_OPT },
	{ "c78-21.74",     0x000100, 0x2926bf27, BRF_OPT },
	{ "c84-10.16",     0x000400, 0x643e8bfc, BRF_OPT },
	{ "c84-11.17",     0x000400, 0x10728853, BRF_OPT },
};

STD_ROM_PICK(Pwheelsj)
STD_ROM_FN(Pwheelsj)

static struct BurnRomInfo EnforceRomDesc[] = {
	{ "b58-27.27",     0x20000, 0xa1aa0191, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b58-19.19",     0x20000, 0x40f43da3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b58-26.26",     0x20000, 0xe823c85c, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b58-18.18",     0x20000, 0x65328a3e, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b58-32.41",     0x10000, 0xf3fd8eca, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b58-09.13",     0x80000, 0x9ffd5b31, BRF_GRA | TAITO_CHARS },
	
	{ "b58-04.7",      0x80000, 0x9482f08d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b58-03.6",      0x80000, 0x158bc440, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b58-02.2",      0x80000, 0x6a6e307c, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b58-01.1",      0x80000, 0x01e9f0a8, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b58-06.116",    0x80000, 0xb3495d70, BRF_GRA | TAITO_ROAD },
	
	{ "b58-05.71",     0x80000, 0xd1f4991b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b58-07.11",     0x80000, 0xeeb5ba08, BRF_SND | TAITO_YM2610A },
	{ "b58-08.12",     0x80000, 0x049243cf, BRF_SND | TAITO_YM2610A },
	
	{ "b58-10.14",     0x80000, 0xedce0cc1, BRF_SND | TAITO_YM2610B },
	
	{ "b58-26a.104",   0x10000, 0xdccb0c7f, BRF_OPT },
	{ "b58-27.56",     0x02000, 0x5c6b013d, BRF_OPT },
	{ "b58-23.52",     0x00100, 0x7b7d8ff4, BRF_OPT },
	{ "b58-24.51",     0x00100, 0xfbf81f30, BRF_OPT },
	{ "b58-25.75",     0x00100, 0xde547342, BRF_OPT },
};

STD_ROM_PICK(Enforce)
STD_ROM_FN(Enforce)

static struct BurnRomInfo NightstrRomDesc[] = {
	{ "b91-45.bin",    0x20000, 0x7ad63421, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-44.bin",    0x20000, 0x4bc30adf, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-43.bin",    0x20000, 0x3e6f727a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-47.bin",    0x20000, 0x9f778e03, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b91-39.bin",    0x20000, 0x725b23ae, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b91-40.bin",    0x20000, 0x81fb364d, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b91-41.bin",    0x20000, 0x2694bb42, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b91-11.bin",    0x80000, 0xfff8ce31, BRF_GRA | TAITO_CHARS },
	
	{ "b91-04.bin",    0x80000, 0x8ca1970d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-03.bin",    0x80000, 0xcd5fed39, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-02.bin",    0x80000, 0x457c64b8, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-01.bin",    0x80000, 0x3731d94f, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b91-08.bin",    0x80000, 0x66f35c34, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-07.bin",    0x80000, 0x4d8ec6cf, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-06.bin",    0x80000, 0xa34dc839, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-05.bin",    0x80000, 0x5e72ac90, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	
	{ "b91-10.bin",    0x80000, 0x1d8f05b4, BRF_GRA | TAITO_ROAD },
	
	{ "b91-09.bin",    0x80000, 0x5f247ca2, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b91-13.bin",    0x80000, 0x8c7bf0f5, BRF_SND | TAITO_YM2610A },
	{ "b91-12.bin",    0x80000, 0xda77c7af, BRF_SND | TAITO_YM2610A },
	
	{ "b91-14.bin",    0x80000, 0x6bc314d3, BRF_SND | TAITO_YM2610B },
	
	{ "b91-26.bin",    0x00400, 0x77682a4f, BRF_OPT },
	{ "b91-27.bin",    0x00400, 0xa3f8490d, BRF_OPT },
	{ "b91-28.bin",    0x00400, 0xfa2f840e, BRF_OPT },
	{ "b91-29.bin",    0x02000, 0xad685be8, BRF_OPT },
	{ "b91-30.bin",    0x10000, 0x30cc1f79, BRF_OPT },
	{ "b91-31.bin",    0x10000, 0xc189781c, BRF_OPT },
	{ "b91-32.bin",    0x00100, 0xfbf81f30, BRF_OPT },
	{ "b91-33.bin",    0x00100, 0x89719d17, BRF_OPT },
};

STD_ROM_PICK(Nightstr)
STD_ROM_FN(Nightstr)

static struct BurnRomInfo NightstrjRomDesc[] = {
	{ "b91-45.bin",    0x20000, 0x7ad63421, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-44.bin",    0x20000, 0x4bc30adf, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-43.bin",    0x20000, 0x3e6f727a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-42.bin",    0x20000, 0x7179ef2f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b91-39.bin",    0x20000, 0x725b23ae, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b91-40.bin",    0x20000, 0x81fb364d, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b91-41.bin",    0x20000, 0x2694bb42, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b91-11.bin",    0x80000, 0xfff8ce31, BRF_GRA | TAITO_CHARS },
	
	{ "b91-04.bin",    0x80000, 0x8ca1970d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-03.bin",    0x80000, 0xcd5fed39, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-02.bin",    0x80000, 0x457c64b8, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-01.bin",    0x80000, 0x3731d94f, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b91-08.bin",    0x80000, 0x66f35c34, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-07.bin",    0x80000, 0x4d8ec6cf, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-06.bin",    0x80000, 0xa34dc839, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-05.bin",    0x80000, 0x5e72ac90, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	
	{ "b91-10.bin",    0x80000, 0x1d8f05b4, BRF_GRA | TAITO_ROAD },
	
	{ "b91-09.bin",    0x80000, 0x5f247ca2, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b91-13.bin",    0x80000, 0x8c7bf0f5, BRF_SND | TAITO_YM2610A },
	{ "b91-12.bin",    0x80000, 0xda77c7af, BRF_SND | TAITO_YM2610A },
	
	{ "b91-14.bin",    0x80000, 0x6bc314d3, BRF_SND | TAITO_YM2610B },
	
	{ "b91-26.bin",    0x00400, 0x77682a4f, BRF_OPT },
	{ "b91-27.bin",    0x00400, 0xa3f8490d, BRF_OPT },
	{ "b91-28.bin",    0x00400, 0xfa2f840e, BRF_OPT },
	{ "b91-29.bin",    0x02000, 0xad685be8, BRF_OPT },
	{ "b91-30.bin",    0x10000, 0x30cc1f79, BRF_OPT },
	{ "b91-31.bin",    0x10000, 0xc189781c, BRF_OPT },
	{ "b91-32.bin",    0x00100, 0xfbf81f30, BRF_OPT },
	{ "b91-33.bin",    0x00100, 0x89719d17, BRF_OPT },
};

STD_ROM_PICK(Nightstrj)
STD_ROM_FN(Nightstrj)

static struct BurnRomInfo NightstruRomDesc[] = {
	{ "b91-45.bin",    0x20000, 0x7ad63421, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-44.bin",    0x20000, 0x4bc30adf, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-43.bin",    0x20000, 0x3e6f727a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b91-46.bin",    0x20000, 0xe870be95, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "b91-39.bin",    0x20000, 0x725b23ae, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b91-40.bin",    0x20000, 0x81fb364d, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "b91-41.bin",    0x20000, 0x2694bb42, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "b91-11.bin",    0x80000, 0xfff8ce31, BRF_GRA | TAITO_CHARS },
	
	{ "b91-04.bin",    0x80000, 0x8ca1970d, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-03.bin",    0x80000, 0xcd5fed39, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-02.bin",    0x80000, 0x457c64b8, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "b91-01.bin",    0x80000, 0x3731d94f, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "b91-08.bin",    0x80000, 0x66f35c34, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-07.bin",    0x80000, 0x4d8ec6cf, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-06.bin",    0x80000, 0xa34dc839, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	{ "b91-05.bin",    0x80000, 0x5e72ac90, BRF_GRA | TAITO_SPRITESB_BYTESWAP32 },
	
	{ "b91-10.bin",    0x80000, 0x1d8f05b4, BRF_GRA | TAITO_ROAD },
	
	{ "b91-09.bin",    0x80000, 0x5f247ca2, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "b91-13.bin",    0x80000, 0x8c7bf0f5, BRF_SND | TAITO_YM2610A },
	{ "b91-12.bin",    0x80000, 0xda77c7af, BRF_SND | TAITO_YM2610A },
	
	{ "b91-14.bin",    0x80000, 0x6bc314d3, BRF_SND | TAITO_YM2610B },
	
	{ "b91-26.bin",    0x00400, 0x77682a4f, BRF_OPT },
	{ "b91-27.bin",    0x00400, 0xa3f8490d, BRF_OPT },
	{ "b91-28.bin",    0x00400, 0xfa2f840e, BRF_OPT },
	{ "b91-29.bin",    0x02000, 0xad685be8, BRF_OPT },
	{ "b91-30.bin",    0x10000, 0x30cc1f79, BRF_OPT },
	{ "b91-31.bin",    0x10000, 0xc189781c, BRF_OPT },
	{ "b91-32.bin",    0x00100, 0xfbf81f30, BRF_OPT },
	{ "b91-33.bin",    0x00100, 0x89719d17, BRF_OPT },
};

STD_ROM_PICK(Nightstru)
STD_ROM_FN(Nightstru)

static struct BurnRomInfo SciRomDesc[] = {
	{ "c09-37.43",     0x20000, 0x0fecea17, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-38.40",     0x20000, 0xe46ebd9b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-42.38",     0x20000, 0xf4404f87, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-39.41",     0x20000, 0xde87bcb9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c09-33.6",      0x10000, 0xcf4e6c5b, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c09-32.5",      0x10000, 0xa4713719, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c09-34.31",     0x20000, 0xa21b3151, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c09-05.16",     0x80000, 0x890b38f0, BRF_GRA | TAITO_CHARS },
	
	{ "c09-04.52",     0x80000, 0x2cbb3c9b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-02.53",     0x80000, 0xa83a0389, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-03.54",     0x80000, 0xa31d0e80, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-01.55",     0x80000, 0x64bfea10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c09-07.15",     0x80000, 0x963bc82b, BRF_GRA | TAITO_ROAD },
	
	{ "c09-06.37",     0x80000, 0x12df6d7b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c09-14.42",     0x80000, 0xad78bf46, BRF_SND | TAITO_YM2610A },
	{ "c09-13.43",     0x80000, 0xd57c41d3, BRF_SND | TAITO_YM2610A },
	{ "c09-12.44",     0x80000, 0x56c99fa5, BRF_SND | TAITO_YM2610A },
	
	{ "c09-15.29",     0x80000, 0xe63b9095, BRF_SND | TAITO_YM2610B },
	
	{ "c09-16.17",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c09-20.71",     0x00100, 0xcd8ffd80, BRF_OPT },
	{ "c09-23.14",     0x00100, 0xfbf81f30, BRF_OPT },
};

STD_ROM_PICK(Sci)
STD_ROM_FN(Sci)

static struct BurnRomInfo SciaRomDesc[] = {
	{ "c09-28.43",     0x20000, 0x630dbaad, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-30.40",     0x20000, 0x68b1a97d, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-36.38",     0x20000, 0x59e47cba, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-31.41",     0x20000, 0x962b1fbf, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c09-33.6",      0x10000, 0xcf4e6c5b, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c09-32.5",      0x10000, 0xa4713719, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c09-34.31",     0x20000, 0xa21b3151, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c09-05.16",     0x80000, 0x890b38f0, BRF_GRA | TAITO_CHARS },
	
	{ "c09-04.52",     0x80000, 0x2cbb3c9b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-02.53",     0x80000, 0xa83a0389, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-03.54",     0x80000, 0xa31d0e80, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-01.55",     0x80000, 0x64bfea10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c09-07.15",     0x80000, 0x963bc82b, BRF_GRA | TAITO_ROAD },
	
	{ "c09-06.37",     0x80000, 0x12df6d7b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c09-14.42",     0x80000, 0xad78bf46, BRF_SND | TAITO_YM2610A },
	{ "c09-13.43",     0x80000, 0xd57c41d3, BRF_SND | TAITO_YM2610A },
	{ "c09-12.44",     0x80000, 0x56c99fa5, BRF_SND | TAITO_YM2610A },
	
	{ "c09-15.29",     0x80000, 0xe63b9095, BRF_SND | TAITO_YM2610B },
	
	{ "c09-16.17",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c09-20.71",     0x00100, 0xcd8ffd80, BRF_OPT },
	{ "c09-23.14",     0x00100, 0xfbf81f30, BRF_OPT },
};

STD_ROM_PICK(Scia)
STD_ROM_FN(Scia)

static struct BurnRomInfo ScijRomDesc[] = {
	{ "c09-37.43",     0x20000, 0x0fecea17, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-38.40",     0x20000, 0xe46ebd9b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-40.38",     0x20000, 0x1a4e2eab, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-39.41",     0x20000, 0xde87bcb9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c09-33.6",      0x10000, 0xcf4e6c5b, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c09-32.5",      0x10000, 0xa4713719, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c09-27.31",     0x20000, 0xcd161dca, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c09-05.16",     0x80000, 0x890b38f0, BRF_GRA | TAITO_CHARS },
	
	{ "c09-04.52",     0x80000, 0x2cbb3c9b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-02.53",     0x80000, 0xa83a0389, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-03.54",     0x80000, 0xa31d0e80, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-01.55",     0x80000, 0x64bfea10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c09-07.15",     0x80000, 0x963bc82b, BRF_GRA | TAITO_ROAD },
	
	{ "c09-06.37",     0x80000, 0x12df6d7b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c09-10.42",     0x80000, 0xad78bf46, BRF_SND | TAITO_YM2610A },
	{ "c09-09.43",     0x80000, 0x6a655c00, BRF_SND | TAITO_YM2610A },
	{ "c09-08.44",     0x80000, 0x7ddfc316, BRF_SND | TAITO_YM2610A },
	
	{ "c09-11.29",     0x80000, 0x6b1a11e1, BRF_SND | TAITO_YM2610B },
	
	{ "c09-16.17",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c09-20.71",     0x00100, 0xcd8ffd80, BRF_OPT },
	{ "c09-23.14",     0x00100, 0xfbf81f30, BRF_OPT },
};

STD_ROM_PICK(Scij)
STD_ROM_FN(Scij)

static struct BurnRomInfo SciuRomDesc[] = {
	{ "c09-43.43",     0x20000, 0x20a9343e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-44.40",     0x20000, 0x7524338a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-41.38",     0x20000, 0x83477f11, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c09-39.41",     0x20000, 0xde87bcb9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c09-33.6",      0x10000, 0xcf4e6c5b, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c09-32.5",      0x10000, 0xa4713719, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c09-34.31",     0x20000, 0xa21b3151, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c09-05.16",     0x80000, 0x890b38f0, BRF_GRA | TAITO_CHARS },
	
	{ "c09-04.52",     0x80000, 0x2cbb3c9b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-02.53",     0x80000, 0xa83a0389, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-03.54",     0x80000, 0xa31d0e80, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-01.55",     0x80000, 0x64bfea10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c09-07.15",     0x80000, 0x963bc82b, BRF_GRA | TAITO_ROAD },
	
	{ "c09-06.37",     0x80000, 0x12df6d7b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c09-14.42",     0x80000, 0xad78bf46, BRF_SND | TAITO_YM2610A },
	{ "c09-13.43",     0x80000, 0xd57c41d3, BRF_SND | TAITO_YM2610A },
	{ "c09-12.44",     0x80000, 0x56c99fa5, BRF_SND | TAITO_YM2610A },
	
	{ "c09-15.29",     0x80000, 0xe63b9095, BRF_SND | TAITO_YM2610B },
	
	{ "c09-16.17",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c09-20.71",     0x00100, 0xcd8ffd80, BRF_OPT },
	{ "c09-23.14",     0x00100, 0xfbf81f30, BRF_OPT },
};

STD_ROM_PICK(Sciu)
STD_ROM_FN(Sciu)

static struct BurnRomInfo ScinegroRomDesc[] = {
	{ "ic37.37",       0x20000, 0x33fb159c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic40.38",       0x20000, 0x657df3f2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic38.42",       0x20000, 0x0a09b90b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },	
	{ "ic41.39",       0x20000, 0x43167b2a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },	
	
	{ "c09-33.6",      0x10000, 0xcf4e6c5b, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c09-32.5",      0x10000, 0xa4713719, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c09-27.31",     0x20000, 0xcd161dca, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "c09-05.16",     0x80000, 0x890b38f0, BRF_GRA | TAITO_CHARS },
	
	{ "c09-04.52",     0x80000, 0x2cbb3c9b, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-02.53",     0x80000, 0xa83a0389, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-03.54",     0x80000, 0xa31d0e80, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c09-01.55",     0x80000, 0x64bfea10, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c09-07.15",     0x80000, 0x963bc82b, BRF_GRA | TAITO_ROAD },
	
	{ "c09-06.37",     0x80000, 0x12df6d7b, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c09-10.42",     0x80000, 0xad78bf46, BRF_SND | TAITO_YM2610A },
	{ "c09-09.43",     0x80000, 0x6a655c00, BRF_SND | TAITO_YM2610A },
	{ "c09-08.44",     0x80000, 0x7ddfc316, BRF_SND | TAITO_YM2610A },
	
	{ "c09-11.29",     0x80000, 0x6b1a11e1, BRF_SND | TAITO_YM2610B },
	
	{ "c09-16.17",     0x10000, 0x7245a6f6, BRF_OPT },
	{ "c09-20.71",     0x00100, 0xcd8ffd80, BRF_OPT },
	{ "c09-23.14",     0x00100, 0xfbf81f30, BRF_OPT },
};

STD_ROM_PICK(Scinegro)
STD_ROM_FN(Scinegro)

static struct BurnRomInfo SpacegunRomDesc[] = {
	{ "c57-18.62",         0x020000, 0x19d7d52e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c57-20.74",         0x020000, 0x2e58253f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c57-17.59",         0x020000, 0xe197edb8, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c57-22.73",         0x020000, 0x5855fde3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "c57-15+.27",        0x020000, 0xb36eb8f1, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c57-16+.29",        0x020000, 0xbfb5d1e7, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	
	{ "c57-06.52",         0x080000, 0x4ebadd5b, BRF_GRA | TAITO_CHARS },
	
	{ "c57-01.25",         0x100000, 0xf901b04e, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c57-02.24",         0x100000, 0x21ee4633, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c57-03.12",         0x100000, 0xfafca86f, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	{ "c57-04.11",         0x100000, 0xa9787090, BRF_GRA | TAITO_SPRITESA_BYTESWAP32 },
	
	{ "c57-05.36",         0x080000, 0x6a70eb2e, BRF_GRA | TAITO_SPRITEMAP },
	
	{ "c57-07.76",         0x080000, 0xad653dc1, BRF_SND | TAITO_YM2610A },
	
	{ "c57-08.75",         0x080000, 0x22593550, BRF_SND | TAITO_YM2610B },
	
	{ "pal16l8-c57-09.9",  0x000104, 0xea93161e, BRF_OPT },
	{ "pal20l8-c57-10.47", 0x000144, 0x3ee56888, BRF_OPT },
	{ "pal16l8-c57-11.48", 0x000104, 0x6bb4372e, BRF_OPT },
	{ "pal20l8-c57-12.61", 0x000144, 0xdebddb13, BRF_OPT },
	{ "pal16l8-c57-13.72", 0x000104, 0x1369f23e, BRF_OPT },
	{ "pal16r4-c57-14.96", 0x000104, 0x75e1bf61, BRF_OPT },
	
};

STD_ROM_PICK(Spacegun)
STD_ROM_FN(Spacegun)

static INT32 MemIndex()
{
	UINT8 *Next; Next = TaitoMem;

	Taito68KRom1                   = Next; Next += Taito68KRom1Size;
	Taito68KRom2                   = Next; Next += Taito68KRom2Size;
	TaitoZ80Rom1                   = Next; Next += TaitoZ80Rom1Size;
	TaitoSpriteMapRom              = Next; Next += TaitoSpriteMapRomSize;
	TaitoYM2610ARom                = Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom                = Next; Next += TaitoYM2610BRomSize;
	
	TaitoRamStart                  = Next;

	Taito68KRam1                   = Next; Next += 0x10000;
	Taito68KRam2                   = Next; Next += 0x08000;
	TaitoSharedRam                 = Next; Next += 0x10000;
	TaitoZ80Ram1                   = Next; Next += 0x02000;
	TaitoSpriteRam                 = Next; Next += 0x04000;
	TaitoPaletteRam                = Next; Next += 0x02000;
	
	TaitoRamEnd                    = Next;

	TaitoChars                     = Next; Next += TaitoNumChar * TaitoCharWidth * TaitoCharHeight;
	TaitoSpritesA                  = Next; Next += TaitoNumSpriteA * TaitoSpriteAWidth * TaitoSpriteAHeight;
	TaitoSpritesB                  = Next; Next += TaitoNumSpriteB * TaitoSpriteBWidth * TaitoSpriteBHeight;
	TaitoPalette                   = (UINT32*)Next; Next += 0x01000 * sizeof(UINT32);

	TaitoMemEnd                    = Next;

	return 0;
}

static INT32 TaitoZDoReset()
{
	TaitoDoReset();

	SciSpriteFrame = 0;
	OldSteer = 0;
	
	return 0;
}

static void TaitoZCpuAReset(UINT16 d)
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

void __fastcall Aquajack68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0xa00000, 0xa0ffff)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

void __fastcall Aquajack68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0xa00000, 0xa0ffff)
	TC0100SCN0CtrlWordWrite_Map(0xa20000)
	
	switch (a) {
		case 0x200000: {
			TaitoZCpuAReset(d);
			return;
		}
		
		case 0x300000:
		case 0x300002: {
			TC0110PCRStep1WordWrite(0, (a - 0x300000) >> 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Aquajack68K2ReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x200000)
	
	switch (a) {
		case 0x300003: {
			return TC0140SYTCommRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

UINT16 __fastcall Aquajack68K2ReadWord(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x200000)
	
	switch (a) {
		case 0x900000:
		case 0x900002:
		case 0x900004:
		case 0x900006: {
			// nop
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Aquajack68K2WriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x200000)
	
	switch (a) {
		case 0x300000: {
			TC0140SYTPortWrite(d & 0xff);
			return;
		}
		
		case 0x300002: {
			TC0140SYTCommWrite(d & 0xff);
			return;
		}
		
		case 0x900000:
		case 0x900002:
		case 0x900004:
		case 0x900006: {
			// nop
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 BsharkStickRead(INT32 Offset)
{
	switch (Offset) {
		case 0x00: {
			INT32 Temp = (TaitoAnalogPort0 >> 4) & 0xfff;
			Temp = 0xfff - Temp;
			Temp += 1;
			if (Temp == 0x1000) Temp = 0;
			return Temp;
		}
		
		case 0x01: {
			return 0xff;
		}
		
		case 0x02: {
			return TaitoAnalogPort1 >> 4;
		}
		
		case 0x03: {
			return 0xff;
		}
	}
	
	return 0;
}

UINT8 __fastcall Bshark68K1ReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x400000)
	
	switch (a) {
		case 0x800001:
		case 0x800003:
		case 0x800005:
		case 0x800007: {
			return BsharkStickRead((a - 0x800000) >> 1);
		}
		
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Bshark68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x400000)
	TC0100SCN0ByteWrite_Map(0xd00000, 0xd0ffff)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

void __fastcall Bshark68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x400000)
	TC0100SCN0WordWrite_Map(0xd00000, 0xd0ffff)
	TC0100SCN0CtrlWordWrite_Map(0xd20000)
	
	switch (a) {
		case 0x600000: {
			TaitoZCpuAReset(d);
			return;
		}
		
		case 0x800000:
		case 0x800002:
		case 0x800004:
		case 0x800006: {
			nTaitoCyclesDone[0] += SekRun(10000);
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT16 __fastcall Bshark68K2ReadWord(UINT32 a)
{
	switch (a) {
		case 0x40000a: {
			// ???
			return 0;
		}
		
		case 0x600000: {
			return BurnYM2610Read(0);
		}
		
		case 0x600004: {
			return BurnYM2610Read(2);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Bshark68K2WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x400000:
		case 0x400002:
		case 0x400004:
		case 0x400006:
		case 0x400008: {
			// nop
			return;
		}
		
		case 0x600000: {
			BurnYM2610Write(0, d & 0xff);
			return;
		}
		
		case 0x600002: {
			BurnYM2610Write(1, d & 0xff);
			return;
		}
		
		case 0x600004: {
			BurnYM2610Write(2, d & 0xff);
			return;
		}
		
		case 0x600006: {
			BurnYM2610Write(3, d & 0xff);
			return;
		}
		
		case 0x60000c:
		case 0x60000e: {
			// nop
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 ChasehqInputBypassRead()
{
	UINT8 Port = TC0220IOCPortRead();
	
	INT32 Steer = (TaitoAnalogPort0 >> 4);
	
	switch (Port) {
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b: {
			return 0xff;
		}
		
		case 0x0c: {
			return Steer & 0xff;
		}
		
		case 0x0d: {
			return Steer >> 8;
		}
	
		default: {
			return TC0220IOCPortRegRead();
		}
	}
}

UINT8 __fastcall Chasehq68K1ReadByte(UINT32 a)
{
	switch (a) {
		case 0x400001: {
			return ChasehqInputBypassRead();
		}
		
		case 0x820003: {
			return TC0140SYTCommRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Chasehq68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0xc00000, 0xc0ffff)
	
	switch (a) {
		case 0x400001: {
			TC0220IOCHalfWordPortRegWrite(d);
			return;
		}
		
		case 0x400003: {
			TC0220IOCHalfWordPortWrite(d);
			return;
		}
		
		case 0x800001: {
			TaitoZCpuAReset(d);
			return;
		}
		
		case 0x820001: {
			TC0140SYTPortWrite(d);
			return;
		}
		
		case 0x820003: {
			TC0140SYTCommWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Chasehq68K1ReadWord(UINT32 a)
{
	switch (a) {
		case 0x400002: {
			return TC0220IOCHalfWordPortRead();
		}
		
		case 0xa00002: {
			return TC0110PCRWordRead(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Chasehq68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0xc00000, 0xc0ffff)
	TC0100SCN0CtrlWordWrite_Map(0xc20000)

	switch (a) {
		case 0x400000: {
			TC0220IOCHalfWordPortRegWrite(d);
			return;
		}
		
		case 0x400002: {
			TC0220IOCHalfWordPortWrite(d);
			return;
		}
		
		case 0xa00000:
		case 0xa00002: {
			TC0110PCRStep1WordWrite(0, (a - 0xa00000) >> 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 ContcircInputBypassRead()
{
	UINT8 Port = TC0220IOCPortRead();
	
	INT32 Steer = (TaitoAnalogPort0 >> 4) & 0xfff;
	Steer = 0xfff - Steer;
	if (Steer == 0xfff) Steer = 0;
	if (Steer > 0x5f && Steer < 0x80) Steer = 0x5f;
	if (Steer > 0xf7f && Steer < 0xfa0) Steer = 0xfa0;
	if (Steer > 0xf7f) Steer |= 0xf000;
	
	switch (Port) {
		case 0x08: {
			return Steer & 0xff;
		}
		
		case 0x09: {
			return Steer >> 8;
		}
	
		default: {
			return TC0220IOCPortRegRead();
		}
	}
}

void __fastcall Contcirc68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x200000, 0x20ffff)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

void __fastcall Contcirc68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x200000, 0x20ffff)
	TC0100SCN0CtrlWordWrite_Map(0x220000)
	
	switch (a) {
		case 0x090000: {
			TaitoRoadPalBank = (d & 0xc0) >> 6;
			TaitoZCpuAReset(d);
			return;
		}
		
		case 0x100000:
		case 0x100002: {
			TC0110PCRStep1RBSwapWordWrite(0, (a - 0x100000) >> 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Contcirc68K2ReadByte(UINT32 a)
{
	switch (a) {
		case 0x100001: {
			return ContcircInputBypassRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Contcirc68K2WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x100001: {
			TC0220IOCHalfWordPortRegWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Contcirc68K2ReadWord(UINT32 a)
{
	switch (a) {
		case 0x100000: {
			return ContcircInputBypassRead();
		}
		
		case 0x100002: {
			return TC0220IOCHalfWordPortRead();
		}
		
		case 0x200002: {
			return TC0140SYTCommRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Contcirc68K2WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x100000: {
			TC0220IOCHalfWordPortRegWrite(d);
			return;
		}
		
		case 0x100002: {
			TC0220IOCHalfWordPortWrite(d);
			return;
		}
		
		case 0x200000: {
			TC0140SYTPortWrite(d & 0xff);
			return;
		}
		
		case 0x200002: {
			TC0140SYTCommWrite(d & 0xff);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT16 DblaxleSteerRead(INT32 Offset)
{
	int Steer = TaitoAnalogPort0 >> 5;
	if (Steer > 0x3f) Steer |= 0xf800;

	switch (Offset) {
		case 0x04: {
			return Steer >> 8;
		}
		
		case 0x05: {
			return Steer & 0xff;
		}
	}
	
	return 0x00;
}

UINT8 __fastcall Dblaxle68K1ReadByte(UINT32 a)
{
	TC0510NIOHalfWordSwapRead_Map(0x400000)
	
	switch (a) {
		case 0x620003: {
			return TC0140SYTCommRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Dblaxle68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordSwapWrite_Map(0x400000)
	
	switch (a) {
		case 0x600001: {
			TaitoZCpuAReset(d);
			return;
		}
		
		case 0x620001: {
			TC0140SYTPortWrite(d);
			return;
		}
		
		case 0x620003: {
			TC0140SYTCommWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Dblaxle68K1ReadWord(UINT32 a)
{
	TC0510NIOHalfWordSwapRead_Map(0x400000)
	
	switch (a) {
		case 0x400018:
		case 0x40001a: {
			return DblaxleSteerRead((a - 0x400010) >> 1);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Dblaxle68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0510NIOHalfWordSwapWrite_Map(0x400000)
	TC0480SCPCtrlWordWrite_Map(0xa30000)
	
	switch (a) {
		case 0xc08000: {
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

void __fastcall Enforce68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x600000, 0x60ffff)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Enforce68K1ReadWord(UINT32 a)
{
	switch (a) {
		case 0x500002: {
			return TC0110PCRWordRead(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Enforce68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x600000, 0x60ffff)
	TC0100SCN0CtrlWordWrite_Map(0x620000)
	
	switch (a) {
		case 0x200000: {
			TaitoZCpuAReset(d);
			return;
		}
		
		case 0x500000:
		case 0x500002: {
			TC0110PCRStep1RBSwapWordWrite(0, (a - 0x500000) >> 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Enforce68K2ReadByte(UINT32 a)
{
	switch (a) {
		case 0x300001: {
			return TC0220IOCPortRegRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Enforce68K2WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x300001: {
			TC0220IOCHalfWordPortRegWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Enforce68K2ReadWord(UINT32 a)
{
	switch (a) {
		case 0x200002: {
			return TC0140SYTCommRead();
		}
		
		case 0x300000: {
			return TC0220IOCPortRegRead();
		}
		
		case 0x300002: {
			return TC0220IOCHalfWordPortRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Enforce68K2WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x200000: {
			TC0140SYTPortWrite(d & 0xff);
			return;
		}
		
		case 0x200002: {
			TC0140SYTCommWrite(d & 0xff);
			return;
		}
		
		case 0x300000: {
			TC0220IOCHalfWordPortRegWrite(d);
			return;
		}
		
		case 0x300002: {
			TC0220IOCHalfWordPortWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static const UINT8 nightstr_stick[128]=
{
	0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
	0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,
	0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,
	0xe8,0x00,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,
	0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,
	0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x42,0x43,0x44,0x45,
	0x46,0x47,0x48,0x49,0xb8
};

static UINT8 NightstrStickRead(INT32 Offset)
{
	switch (Offset) {
		case 0x00: {
			UINT8 Temp = 0x7f + (TaitoAnalogPort0 >> 4);
			if (Temp < 0x01) Temp = 0x01;
			if (Temp > 0xfe) Temp = 0xfe;
			return nightstr_stick[(Temp * 0x64) / 0x100];
		}
		
		case 0x01: {
			UINT8 Temp = 0x7f - (TaitoAnalogPort1 >> 4);
			if (Temp < 0x01) Temp = 0x01;
			if (Temp > 0xfe) Temp = 0xfe;
			return nightstr_stick[(Temp * 0x64) / 0x100];
		}
		
		case 0x02: {
			return 0xff;
		}
		
		case 0x03: {
			return 0xff;
		}
	}
	
	return 0xff;
}

UINT8 __fastcall Nightstr68K1ReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x400000)
	
	switch (a) {
		case 0xe40001:
		case 0xe40003:
		case 0xe40005:
		case 0xe40007: {
			return NightstrStickRead((a - 0xe40000) >> 1);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Nightstr68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x400000)
	TC0100SCN0ByteWrite_Map(0xc00000, 0xc0ffff)
	
	switch (a) {
		case 0xe00000:
		case 0xe00008:
		case 0xe00010: {
			// nop
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Nightstr68K1ReadWord(UINT32 a)
{
	switch (a) {
		case 0x820002: {
			return TC0140SYTCommRead();
		}
		
		case 0xa00002: {
			return TC0110PCRWordRead(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Nightstr68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x400000)
	TC0100SCN0WordWrite_Map(0xc00000, 0xc0ffff)
	TC0100SCN0CtrlWordWrite_Map(0xc20000)
	
	switch (a) {
		case 0x800000: {
			TaitoZCpuAReset(d);
			return;
		}
		
		case 0x820000: {
			TC0140SYTPortWrite(d & 0xff);
			return;
		}
		
		case 0x820002: {
			TC0140SYTCommWrite(d & 0xff);
			return;
		}
		
		case 0xa00000:
		case 0xa00002: {
			TC0110PCRStep1WordWrite(0, (a - 0xa00000) >> 1, d);
			return;
		}
		
		case 0xe00000:
		case 0xe00008:
		case 0xe00010: {
			// nop
			return;
		}
		
		case 0xe40000:
		case 0xe40002:
		case 0xe40004:
		case 0xe40006:
		case 0xe40008:
		case 0xe4000a:
		case 0xe4000c:
		case 0xe4000e: {
			nTaitoCyclesDone[0] += SekRun(10000);
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 SciSteerRead(INT32 Offset)
{
	INT32 Steer = TaitoAnalogPort0 >> 4;
	if (Steer > 0x5f && Steer < 0x80) Steer = 0x5f;
	if (Steer > 0xf80 && Steer < 0xfa0) Steer = 0xfa0;
	if ((OldSteer < Steer) && (Steer > 0xfc0)) Steer = 0;
	OldSteer = Steer;
	
	switch (Offset) {
		case 0x04: {
			return Steer & 0xff;
		}
		
		case 0x05: {
			return (Steer & 0xff00) >> 8;
		}
	}
	
	return 0xff;
}

UINT8 __fastcall Sci68K1ReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x200000)
	
	switch (a) {
		case 0x200019:
		case 0x20001b: {
			return SciSteerRead((a - 0x200010) >> 1);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Sci68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x200000)
	TC0100SCN0ByteWrite_Map(0xa00000, 0xa0ffff)
	
	switch (a) {
		case 0x400001: {
			TaitoZCpuAReset(d);
			return;
		}
		
		case 0x420001: {
			TC0140SYTPortWrite(d);
			return;
		}
		
		case 0x420003: {
			TC0140SYTCommWrite(d);
			return;
		}
		
		case 0xc08000: {
			SciSpriteFrame = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

void __fastcall Sci68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0xa00000, 0xa0ffff)
	TC0100SCN0CtrlWordWrite_Map(0xa20000)
	
	switch (a) {		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static const UINT8 spacegun_default_eeprom[128]=
{
	0x00,0x00,0x00,0xff,0x00,0x01,0x41,0x41,0x00,0x00,0x00,0xff,0x00,0x00,0xf0,0xf0,
	0x00,0x00,0x00,0xff,0x00,0x01,0x41,0x41,0x00,0x00,0x00,0xff,0x00,0x00,0xf0,0xf0,
	0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x01,0x40,0x00,0x00,0x00,0xf0,0x00,
	0x00,0x01,0x42,0x85,0x00,0x00,0xf1,0xe3,0x00,0x01,0x40,0x00,0x00,0x00,0xf0,0x00,
	0x00,0x01,0x42,0x85,0x00,0x00,0xf1,0xe3,0xcc,0xcb,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

static const eeprom_interface spacegun_eeprom_intf =
{
	6,				/* address bits */
	16,				/* data bits */
	"0110",			/* read command */
	"0101",			/* write command */
	"0111",			/* erase command */
	"0100000000",	/* lock command */
	"0100111111",	/* unlock command */
	0,				/* multi-read disabled */
	1				/* reset delay */
};

static UINT8 SpacegunInputBypassRead(INT32 Offset)
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

static void SpacegunInputBypassWrite(INT32 Offset, UINT16 Data)
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

void __fastcall Spacegun68K1WriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x900000, 0x90ffff)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Spacegun68K1ReadWord(UINT32 a)
{
	switch (a) {
		case 0xb00002: {
			return TC0110PCRWordRead(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Spacegun68K1WriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x900000, 0x90ffff)
	TC0100SCN0CtrlWordWrite_Map(0x920000)
	
	switch (a) {
		case 0xb00000:
		case 0xb00002: {
			TC0110PCRStep1RBSwapWordWrite(0, (a - 0xb00000) >> 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Spacegun68K2ReadByte(UINT32 a)
{
	switch (a) {
		case 0xc0000d: {
			// nop
			return 0;
		}
		
		case 0xf00001: {
			return ~BurnGunReturnX(0);
		}
		
		case 0xf00003: {
			return BurnGunReturnY(0);
		}
		
		case 0xf00005: {
			return ~BurnGunReturnX(1);
		}
		
		case 0xf00007: {
			return BurnGunReturnY(1);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Spacegun68K2WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x800008: {
			SpacegunInputBypassWrite((a - 0x800000) >> 1, d);
			return;
		}
		
		case 0xc0000d: {
			// nop
			return;
		}
		
		case 0xe00001: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Spacegun68K2ReadWord(UINT32 a)
{
	switch (a) {
		case 0x800000:
		case 0x800002:
		case 0x800004:
		case 0x800006:
		case 0x800008:
		case 0x80000a:
		case 0x80000c:
		case 0x80000e: {
			return SpacegunInputBypassRead((a - 0x800000) >> 1);
		}
		
		case 0xc00000: {
			return BurnYM2610Read(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Spacegun68K2WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x800000:
		case 0x800002:
		case 0x800004:
		case 0x800006:
		case 0x800008:
		case 0x80000a:
		case 0x80000c:
		case 0x80000e: {
			SpacegunInputBypassWrite((a - 0x800000) >> 1, d);
			return;
		}
		
		case 0xc00000: {
			BurnYM2610Write(0, d & 0xff);
			return;
		}
		
		case 0xc00002: {
			BurnYM2610Write(1, d & 0xff);
			return;
		}
		
		case 0xc00004: {
			BurnYM2610Write(2, d & 0xff);
			return;
		}
		
		case 0xc00006: {
			BurnYM2610Write(3, d & 0xff);
			return;
		}
		
		case 0xc20000:
		case 0xc20002:
		case 0xc20004:
		case 0xc20006: {
			// ???
			return;
		}
		
		case 0xf00000:
		case 0xf00002:
		case 0xf00004:
		case 0xf00006: {
			nTaitoCyclesDone[1] += SekRun(10000);
			SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall TaitoZZ80Read(UINT16 a)
{
	switch (a) {
		case 0xe000: {
			return BurnYM2610Read(0);
		}
		
		case 0xe001: {
			return BurnYM2610Read(1);
		}
		
		case 0xe002: {
			return BurnYM2610Read(2);
		}
		
		case 0xe200: {
			// NOP
			return 0;
		}
		
		case 0xe201: {
			return TC0140SYTSlaveCommRead();
		}
		
		case 0xea00: {
			// NOP
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall TaitoZZ80Write(UINT16 a, UINT8 d)
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
			return;
		}
		
		case 0xee00: {
			return;
		}
		
		case 0xf000: {
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

static INT32 CharPlaneOffsets[4]        = { 0, 1, 2, 3 };
static INT32 CharXOffsets[8]            = { 8, 12, 0, 4, 24, 28, 16, 20 };
static INT32 CharYOffsets[8]            = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 DblaxleCharPlaneOffsets[4] = { 0, 1, 2, 3 };
static INT32 DblaxleCharXOffsets[16]    = { 4, 0, 20, 16, 12, 8, 28, 24, 36, 32, 52, 48, 44, 40, 60, 56 };
static INT32 DblaxleCharYOffsets[16]    = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
static INT32 SpritePlaneOffsets[4]      = { 0, 8, 16, 24 };
static INT32 SpriteXOffsets[16]         = { 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 SpriteYOffsets[16]         = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
static INT32 Sprite16x8YOffsets[8]      = { 0, 64, 128, 192, 256, 320, 384, 448 };

static void TaitoZFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 TaitoZSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / (16000000 / 4);
}

static double TaitoZGetTime()
{
	return (double)ZetTotalCycles() / (16000000 / 4);
}

static INT32 TaitoZ68KSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)SekTotalCycles() * nSoundRate / (nTaitoCyclesTotal[1] * 60);
}

static double TaitoZ68KGetTime()
{
	return (double)SekTotalCycles() / (nTaitoCyclesTotal[1] * 60);
}

static void TaitoZZ80Init()
{
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(TaitoZZ80Read);
	ZetSetWriteHandler(TaitoZZ80Write);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1               );
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1               );
	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1               );
	ZetMemEnd();
	ZetClose();
	
	TaitoNumZ80s = 1;
}

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

static INT32 AquajackInit()
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
	TaitoSpriteAYOffsets = Sprite16x8YOffsets;
	TaitoNumSpriteA = 0x8000;
	
	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0110PCRInit(1, 0x1000);
	TC0150RODInit(TaitoRoadRomSize, 0);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;

	SwitchToMusashi();
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x104000, 0x107fff, SM_RAM);
	SekMapMemory(TC0150RODRam            , 0x800000, 0x801fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0xa00000, 0xa0ffff, SM_READ);
	SekMapMemory(TaitoSpriteRam          , 0xc40000, 0xc403ff, SM_RAM);
	SekSetWriteByteHandler(0, Aquajack68K1WriteByte);
	SekSetWriteWordHandler(0, Aquajack68K1WriteWord);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x104000, 0x107fff, SM_RAM);
	SekSetReadWordHandler(0, Aquajack68K2ReadWord);
	SekSetWriteWordHandler(0, Aquajack68K2WriteWord);
	SekSetReadByteHandler(0, Aquajack68K2ReadByte);
	SekClose();
	
	TaitoZZ80Init();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &TaitoZFMIRQHandler, TaitoZSynchroniseStream, TaitoZGetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetSoundMixMode(1);
	
	TaitoMakeInputsFunction = AquajackMakeInputs;
	TaitoDrawFunction = AquajackDraw;
	TaitoIrqLine = 4;
	TaitoFrameInterleave = 500;

	nTaitoCyclesTotal[0] = 12000000 / 60;
	nTaitoCyclesTotal[1] = 12000000 / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;
	
	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 BsharkInit()
{
	INT32 nLen;
	
	Sci = 1;
	
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
	TaitoSpriteAYOffsets = Sprite16x8YOffsets;
	TaitoNumSpriteA = 0x8000;
	
	TaitoNum68Ks = 2;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 0, 8, 1, NULL);
	TC0150RODInit(TaitoRoadRomSize, 1);
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;
	
	SwitchToMusashi();
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x110000, 0x113fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam         , 0xa00000, 0xa01fff, SM_RAM);
	SekMapMemory(TaitoSpriteRam          , 0xc00000, 0xc00fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0xd00000, 0xd0ffff, SM_READ);
	SekSetWriteWordHandler(0, Bshark68K1WriteWord);
	SekSetReadByteHandler(0, Bshark68K1ReadByte);
	SekSetWriteByteHandler(0, Bshark68K1WriteByte);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x108000, 0x10bfff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x110000, 0x113fff, SM_RAM);
	SekMapMemory(TC0150RODRam            , 0x800000, 0x801fff, SM_RAM);
	SekSetReadWordHandler(0, Bshark68K2ReadWord);
	SekSetWriteWordHandler(0, Bshark68K2WriteWord);
	SekClose();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, NULL, TaitoZ68KSynchroniseStream, TaitoZ68KGetTime, 0);
	BurnTimerAttachSek(12000000);
	BurnYM2610SetSoundMixMode(1);
	
	TaitoMakeInputsFunction = BsharkMakeInputs;
	TaitoDrawFunction = BsharkDraw;
	TaitoIrqLine = 4;
	TaitoFrameInterleave = 100;
	TaitoFlipScreenX = 1;

	nTaitoCyclesTotal[0] = 12000000 / 60;
	nTaitoCyclesTotal[1] = 12000000 / 60;
	
	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 ChasehqInit()
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
	
	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x4000;
	
	TaitoSpriteBModulo = 0x400;
	TaitoSpriteBNumPlanes = 4;
	TaitoSpriteBWidth = 16;
	TaitoSpriteBHeight = 16;
	TaitoSpriteBPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteBXOffsets = SpriteXOffsets;
	TaitoSpriteBYOffsets = SpriteYOffsets;
	TaitoNumSpriteB = 0x4000;
	
	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0110PCRInit(1, 0x1000);
	TC0150RODInit(TaitoRoadRomSize, 0);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;

	SwitchToMusashi();
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x100000, 0x107fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x108000, 0x10bfff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0xc00000, 0xc0ffff, SM_READ);
	SekMapMemory(TaitoSpriteRam          , 0xd00000, 0xd007ff, SM_RAM);
	SekSetReadWordHandler(0, Chasehq68K1ReadWord);
	SekSetWriteWordHandler(0, Chasehq68K1WriteWord);
	SekSetReadByteHandler(0, Chasehq68K1ReadByte);
	SekSetWriteByteHandler(0, Chasehq68K1WriteByte);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x108000, 0x10bfff, SM_RAM);
	SekMapMemory(TC0150RODRam            , 0x800000, 0x801fff, SM_RAM);
	SekClose();
	
	TaitoZZ80Init();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &TaitoZFMIRQHandler, TaitoZSynchroniseStream, TaitoZGetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetSoundMixMode(1);
	
	TaitoMakeInputsFunction = ChasehqMakeInputs;
	TaitoDrawFunction = ChasehqDraw;
	TaitoIrqLine = 4;
	TaitoFrameInterleave = 100;

	nTaitoCyclesTotal[0] = 12000000 / 60;
	nTaitoCyclesTotal[1] = 12000000 / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;
	
	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 ContcircInit()
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
	TaitoSpriteAYOffsets = Sprite16x8YOffsets;
	TaitoNumSpriteA = 0x8000;
	
	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 0, 16, 0, NULL);
	TC0110PCRInit(1, 0x1000);
	TC0150RODInit(TaitoRoadRomSize, 0);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;

	SwitchToMusashi();
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x080000, 0x083fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x084000, 0x087fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0x200000, 0x20ffff, SM_READ);
	SekMapMemory(TC0150RODRam            , 0x300000, 0x301fff, SM_RAM);	
	SekMapMemory(TaitoSpriteRam          , 0x400000, 0x4006ff, SM_RAM);
	SekSetWriteByteHandler(0, Contcirc68K1WriteByte);
	SekSetWriteWordHandler(0, Contcirc68K1WriteWord);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x080000, 0x083fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x084000, 0x087fff, SM_RAM);
	SekSetReadWordHandler(0, Contcirc68K2ReadWord);
	SekSetWriteWordHandler(0, Contcirc68K2WriteWord);
	SekSetReadByteHandler(0, Contcirc68K2ReadByte);
	SekSetWriteByteHandler(0, Contcirc68K2WriteByte);
	SekClose();
	
	TaitoZZ80Init();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &TaitoZFMIRQHandler, TaitoZSynchroniseStream, TaitoZGetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetSoundMixMode(1);
	
	TaitoMakeInputsFunction = ContcircMakeInputs;
	TaitoDrawFunction = ContcircDraw;
	TaitoIrqLine = 6;
	TaitoFrameInterleave = 100;

	nTaitoCyclesTotal[0] = 12000000 / 60;
	nTaitoCyclesTotal[1] = 12000000 / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;
	
	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 DblaxleInit()
{
	INT32 nLen;
	
	Dblaxle = 1;
	
	TaitoCharModulo = 0x400;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 16;
	TaitoCharHeight = 16;
	TaitoCharPlaneOffsets = DblaxleCharPlaneOffsets;
	TaitoCharXOffsets = DblaxleCharXOffsets;
	TaitoCharYOffsets = DblaxleCharYOffsets;
	TaitoNumChar = 0x2000;
	
	TaitoSpriteAModulo = 0x200;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 8;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = Sprite16x8YOffsets;
	TaitoNumSpriteA = 0x10000;
	
	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);
	
	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	TC0150RODInit(TaitoRoadRomSize, 0);
	TC0480SCPInit(TaitoNumChar, 0, 0x21, 8, 4, 0, 0);
	TC0140SYTInit();
	TC0510NIOInit();
	
	if (TaitoLoadRoms(1)) return 1;
	
	SwitchToMusashi();
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x200000, 0x203fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x210000, 0x21ffff, SM_RAM);
	SekMapMemory(TaitoPaletteRam         , 0x800000, 0x801fff, SM_RAM);
	SekMapMemory(TC0480SCPRam            , 0x900000, 0x90ffff, SM_RAM);
	SekMapMemory(TC0480SCPRam            , 0xa00000, 0xa0ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam          , 0xc00000, 0xc03fff, SM_RAM);
	SekSetReadWordHandler(0, Dblaxle68K1ReadWord);
	SekSetWriteWordHandler(0, Dblaxle68K1WriteWord);
	SekSetReadByteHandler(0, Dblaxle68K1ReadByte);
	SekSetWriteByteHandler(0, Dblaxle68K1WriteByte);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x110000, 0x11ffff, SM_RAM);
	SekMapMemory(TC0150RODRam            , 0x300000, 0x301fff, SM_RAM);
	SekClose();
	
	TaitoZZ80Init();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &TaitoZFMIRQHandler, TaitoZSynchroniseStream, TaitoZGetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetSoundMixMode(1);
	
	TaitoMakeInputsFunction = DblaxleMakeInputs;
	TaitoDrawFunction = DblaxleDraw;
	TaitoIrqLine = 4;
	TaitoFrameInterleave = 100;

	nTaitoCyclesTotal[0] = 16000000 / 60;
	nTaitoCyclesTotal[1] = 16000000 / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;
	
	GenericTilesInit();

	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 EnforceInit()
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
	TaitoSpriteAYOffsets = Sprite16x8YOffsets;
	TaitoNumSpriteA = 0x8000;
	
	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 0, 16, 0, NULL);
	TC0110PCRInit(1, 0x1000);
	TC0150RODInit(TaitoRoadRomSize, 0);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;

	SwitchToMusashi();
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x104000, 0x107fff, SM_RAM);
	SekMapMemory(TaitoSpriteRam          , 0x300000, 0x3006ff, SM_RAM);
	SekMapMemory(TC0150RODRam            , 0x400000, 0x401fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0x600000, 0x60ffff, SM_READ);
	SekSetWriteByteHandler(0, Enforce68K1WriteByte);
	SekSetReadWordHandler(0, Enforce68K1ReadWord);
	SekSetWriteWordHandler(0, Enforce68K1WriteWord);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x104000, 0x107fff, SM_RAM);
	SekSetReadWordHandler(0, Enforce68K2ReadWord);
	SekSetWriteWordHandler(0, Enforce68K2WriteWord);
	SekSetReadByteHandler(0, Enforce68K2ReadByte);
	SekSetWriteByteHandler(0, Enforce68K2WriteByte);
	SekClose();
	
	TaitoZZ80Init();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &TaitoZFMIRQHandler, TaitoZSynchroniseStream, TaitoZGetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetSoundMixMode(1);
	
	TaitoMakeInputsFunction = EnforceMakeInputs;
	TaitoDrawFunction = EnforceDraw;
	TaitoIrqLine = 6;
	TaitoFrameInterleave = 100;

	nTaitoCyclesTotal[0] = 12000000 / 60;
	nTaitoCyclesTotal[1] = 12000000 / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;
	
	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 NightstrInit()
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
	
	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x4000;
	
	TaitoSpriteBModulo = 0x400;
	TaitoSpriteBNumPlanes = 4;
	TaitoSpriteBWidth = 16;
	TaitoSpriteBHeight = 16;
	TaitoSpriteBPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteBXOffsets = SpriteXOffsets;
	TaitoSpriteBYOffsets = SpriteYOffsets;
	TaitoNumSpriteB = 0x4000;
	
	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0110PCRInit(1, 0x1000);
	TC0150RODInit(TaitoRoadRomSize, 0);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;

	SwitchToMusashi();

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x110000, 0x113fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0xc00000, 0xc0ffff, SM_READ);
	SekMapMemory(TaitoSpriteRam          , 0xd00000, 0xd007ff, SM_RAM);
	SekSetReadWordHandler(0, Nightstr68K1ReadWord);
	SekSetWriteWordHandler(0, Nightstr68K1WriteWord);
	SekSetReadByteHandler(0, Nightstr68K1ReadByte);
	SekSetWriteByteHandler(0, Nightstr68K1WriteByte);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x104000, 0x107fff, SM_RAM);
	SekMapMemory(TC0150RODRam            , 0x800000, 0x801fff, SM_RAM);
	SekClose();
	
	TaitoZZ80Init();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &TaitoZFMIRQHandler, TaitoZSynchroniseStream, TaitoZGetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetSoundMixMode(1);
	
	TaitoMakeInputsFunction = NightstrMakeInputs;
	TaitoDrawFunction = ChasehqDraw;
	TaitoIrqLine = 4;
	TaitoFrameInterleave = 100;

	nTaitoCyclesTotal[0] = 12000000 / 60;
	nTaitoCyclesTotal[1] = 12000000 / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;
	
	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 SciInit()
{
	INT32 nLen;
	
	Sci = 1;
	
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
	TaitoSpriteAYOffsets = Sprite16x8YOffsets;
	TaitoNumSpriteA = 0x8000;
	
	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0150RODInit(TaitoRoadRomSize, 0);
	TC0140SYTInit();
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;

	SwitchToMusashi();

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x100000, 0x107fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x108000, 0x10bfff, SM_RAM);
	SekMapMemory(TaitoPaletteRam         , 0x800000, 0x801fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0xa00000, 0xa0ffff, SM_READ);
	SekMapMemory(TaitoSpriteRam          , 0xc00000, 0xc03fff, SM_RAM);
	SekSetWriteWordHandler(0, Sci68K1WriteWord);
	SekSetReadByteHandler(0, Sci68K1ReadByte);
	SekSetWriteByteHandler(0, Sci68K1WriteByte);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x200000, 0x203fff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x208000, 0x20bfff, SM_RAM);
	SekMapMemory(TC0150RODRam            , 0xa00000, 0xa01fff, SM_RAM);
	SekClose();
	
	TaitoZZ80Init();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &TaitoZFMIRQHandler, TaitoZSynchroniseStream, TaitoZGetTime, 0);
	BurnTimerAttachZet(16000000 / 4);
	BurnYM2610SetSoundMixMode(1);
	
	TaitoMakeInputsFunction = SciMakeInputs;
	TaitoDrawFunction = SciDraw;
	TaitoIrqLine = 4;
	TaitoFrameInterleave = 100;

	nTaitoCyclesTotal[0] = 12000000 / 60;
	nTaitoCyclesTotal[1] = 12000000 / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;
	
	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 SpacegunInit()
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
	TaitoSpriteAYOffsets = Sprite16x8YOffsets;
	TaitoNumSpriteA = 0x10000;
	
	TaitoNum68Ks = 2;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0100SCNInit(0, TaitoNumChar, 4, 8, 1, NULL);
	TC0110PCRInit(1, 0x1000);
	TC0220IOCInit();
	
	if (TaitoLoadRoms(1)) return 1;

	SwitchToMusashi();

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x30c000, 0x30ffff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x310000, 0x31ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam          , 0x500000, 0x5005ff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]         , 0x900000, 0x90ffff, SM_READ);
	SekSetReadWordHandler(0, Spacegun68K1ReadWord);
	SekSetWriteWordHandler(0, Spacegun68K1WriteWord);
	SekSetWriteByteHandler(0, Spacegun68K1WriteByte);
	SekClose();
	
	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2            , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Taito68KRam2            , 0x20c000, 0x20ffff, SM_RAM);
	SekMapMemory(TaitoSharedRam          , 0x210000, 0x21ffff, SM_RAM);
	SekSetReadWordHandler(0, Spacegun68K2ReadWord);
	SekSetWriteWordHandler(0, Spacegun68K2WriteWord);
	SekSetReadByteHandler(0, Spacegun68K2ReadByte);
	SekSetWriteByteHandler(0, Spacegun68K2WriteByte);
	SekClose();
	
	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, NULL, TaitoZ68KSynchroniseStream, TaitoZ68KGetTime, 0);
	BurnTimerAttachSek(16000000);
	BurnYM2610SetSoundMixMode(1);
	
	EEPROMInit(&spacegun_eeprom_intf);
	if (!EEPROMAvailable()) EEPROMFill(spacegun_default_eeprom, 0, 128);
	
	TaitoMakeInputsFunction = SpacegunMakeInputs;
	TaitoDrawFunction = SpacegunDraw;
	TaitoIrqLine = 4;
	TaitoFrameInterleave = 100;
	TaitoFlipScreenX = 1;
	TaitoNumEEPROM = 1;

	nTaitoCyclesTotal[0] = 16000000 / 60;
	nTaitoCyclesTotal[1] = 16000000 / 60;
	
	BurnGunInit(2, true);	

	// Reset the driver
	TaitoZDoReset();

	return 0;
}

static INT32 TaitoZExit()
{
	BurnYM2610SetSoundMixMode(0);
	
	TaitoExit();

	SciSpriteFrame = 0;
	OldSteer = 0;
	Dblaxle = 0;
	Sci = 0;
	
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

static inline UINT8 pal5bit(UINT8 bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal5bit(BURN_ENDIAN_SWAP_INT16(nColour) >>  0);
	g = pal5bit(BURN_ENDIAN_SWAP_INT16(nColour) >>  5);
	b = pal5bit(BURN_ENDIAN_SWAP_INT16(nColour) >> 10);

	return BurnHighCol(r, g, b, 0);
}

static void TaitoZCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)TaitoPaletteRam, pd = TaitoPalette; i < 0x1000; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
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

static void AquajackRenderSprites(INT32 PriorityDraw)
{
	UINT16 *SpriteMap = (UINT16*)TaitoSpriteMapRom;
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	INT32 Offset, Data, Tile, Colour, xFlip, yFlip;
	INT32 x, y, Priority, xCur, yCur;
	INT32 xZoom, yZoom, zx, zy;
	INT32 SpriteChunk, MapOffset, Code, j, k, px, py;
	
	for (Offset = 0x200 - 4; Offset >= 0; Offset -= 4) {
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
		yZoom = (Data & 0x7e00) >> 9;
		y = Data & 0x1ff;
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 2]);
		Colour = (Data & 0xff00) >> 8;
		xZoom = (Data & 0x3f);

		MapOffset = Tile << 5;

		xZoom += 1;
		yZoom += 1;

		y += 3;

		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		for (SpriteChunk = 0; SpriteChunk < 32; SpriteChunk++) {
			k = SpriteChunk % 4;
			j = SpriteChunk / 4;

			px = xFlip ? (3 - k) : k;
			py = yFlip ? (7 - j) : j;

			Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 2)]);
			Code &= (TaitoNumSpriteA - 1);

			xCur = x + ((k * xZoom) / 4);
			yCur = y + ((j * yZoom) / 8);

			zx = x + (((k + 1) * xZoom) / 4) - xCur;
			zy = y + (((j + 1) * yZoom) / 8) - yCur;

			yCur -= 16;

			RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 13, TaitoSpritesA);
		}


	}
}

static void BsharkRenderSprites(INT32 PriorityDraw, INT32 yOffset, INT32 SpriteRamSize)
{
	UINT16 *SpriteMap = (UINT16*)TaitoSpriteMapRom;
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	INT32 Offset, Data, Tile, Colour, xFlip, yFlip;
	INT32 x, y, Priority, xCur, yCur;
	INT32 xZoom, yZoom, zx, zy;
	INT32 SpriteChunk, MapOffset, Code, j, k, px, py;
	
	for (Offset = 0; Offset < SpriteRamSize - 4; Offset += 4) {
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 1]);
		Priority = (Data & 0x8000) >> 15;
		
		if (Priority != 0 && Priority != 1) bprintf(PRINT_NORMAL, _T("Unused Priority %x\n"), Priority);
		if (Priority != PriorityDraw) continue;
		
		Colour = (Data & 0x7f80) >> 7;
		xZoom = (Data & 0x3f);
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 3]);
		Tile = Data & 0x1fff;
		if (!Tile) continue;
				
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 0]);
		yZoom = (Data & 0x7e00) >> 9;
		y = Data & 0x1ff;

		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 2]);
		yFlip = (Data & 0x8000) >> 15;
		xFlip = (Data & 0x4000) >> 14;
		x = Data & 0x1ff;

		MapOffset = Tile << 5;

		xZoom += 1;
		yZoom += 1;

		y += yOffset;
		y += (64 - yZoom);

		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		for (SpriteChunk = 0; SpriteChunk < 32; SpriteChunk++) {
			k = SpriteChunk % 4;
			j = SpriteChunk / 4;
			
			px = xFlip ? (3 - k) : k;
			py = yFlip ? (7 - j) : j;

			Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 2)]);
			Code &= (TaitoNumSpriteA - 1);
			
			xCur = x + ((k * xZoom) / 4);
			yCur = y + ((j * yZoom) / 8);
			
			zx = x + (((k + 1) * xZoom) / 4) - xCur;
			zy = y + (((j + 1) * yZoom) / 8) - yCur;

			yCur -= 16;

			RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 13, TaitoSpritesA);
		}

	}
}

static void ChasehqRenderSprites(INT32 PriorityDraw)
{
	UINT16 *SpriteMap = (UINT16*)TaitoSpriteMapRom;
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	INT32 Offset, Data, Tile, Colour, xFlip, yFlip;
	INT32 x, y, Priority, xCur, yCur;
	INT32 xZoom, yZoom, zx, zy;
	INT32 SpriteChunk, MapOffset, Code, j, k, px, py;
	
	for (Offset = 0; Offset < 0x400; Offset += 4) {
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 1]);
		Priority = (Data & 0x8000) >> 15;
		
		if (Priority != 0 && Priority != 1) bprintf(PRINT_NORMAL, _T("Unused Priority %x\n"), Priority);
		if (Priority != PriorityDraw) continue;
		
		Colour = (Data & 0x7f80) >> 7;
		xZoom = (Data & 0x7f);
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 3]);
		Tile = Data & 0x7ff;
		if (!Tile) continue;
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 0]);
		yZoom = (Data & 0xfe00) >> 9;
		y = Data & 0x1ff;
				
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 2]);
		yFlip = (Data & 0x8000) >> 15;
		xFlip = (Data & 0x4000) >> 14;
		x = Data & 0x1ff;
		
		xZoom += 1;
		yZoom += 1;
		
		y += 7;
		y += (128 - yZoom);
		
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		if ((xZoom - 1) & 0x40) {
			MapOffset = Tile << 6;
			
			for (SpriteChunk = 0; SpriteChunk < 64; SpriteChunk++) {
				j = SpriteChunk / 8;
				k = SpriteChunk % 8;
				
				px = xFlip ? (7 - k) : k;
				py = yFlip ? (7 - j) : j;
				
				Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 3)]);
				Code &= (TaitoNumSpriteA - 1);
				
				xCur = x + ((k * xZoom) / 8);
				yCur = y + ((j * yZoom) / 8);
				
				zx = x + (((k + 1) * xZoom) / 8) - xCur;
				zy = y + (((j + 1) * yZoom) / 8) - yCur;
				
				yCur -= 16;
				
				RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 12, TaitoSpritesA);
			}
		} else if ((xZoom - 1) & 0x20) {
			MapOffset = (Tile << 5) + 0x20000;
			
			for (SpriteChunk = 0; SpriteChunk < 32; SpriteChunk++) {
				j = SpriteChunk / 4;
				k = SpriteChunk % 4;
				
				px = xFlip ? (3 - k) : k;
				py = yFlip ? (7 - j) : j;
				
				Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 2)]);
				Code &= (TaitoNumSpriteB - 1);
				
				xCur = x + ((k * xZoom) / 4);
				yCur = y + ((j * yZoom) / 8);
				
				zx = x + (((k + 1) * xZoom) / 4) - xCur;
				zy = y + (((j + 1) * yZoom) / 8) - yCur;
				
				yCur -= 16;
				
				RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 12, TaitoSpritesB);
			}
		} else if (!((xZoom - 1) & 0x60)) {
			MapOffset = (Tile << 4) + 0x30000;
			
			for (SpriteChunk = 0; SpriteChunk < 16; SpriteChunk++) {
				j = SpriteChunk / 2;
				k = SpriteChunk % 2;
				
				px = xFlip ? (1 - k) : k;
				py = yFlip ? (7 - j) : j;
				
				Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 1)]);
				Code &= (TaitoNumSpriteB - 1);
				
				xCur = x + ((k * xZoom) / 2);
				yCur = y + ((j * yZoom) / 8);
				
				zx = x + (((k + 1) * xZoom) / 2) - xCur;
				zy = y + (((j + 1) * yZoom) / 8) - yCur;
				
				yCur -= 16;
				
				RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 12, TaitoSpritesB);
			}
		}
	}
}

static void ContcircRenderSprites(INT32 PriorityDraw, INT32 VisibleYOffset)
{
	UINT16 *SpriteMap = (UINT16*)TaitoSpriteMapRom;
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	INT32 Offset, Data, Tile, Colour, xFlip, yFlip;
	INT32 x, y, Priority, xCur, yCur;
	INT32 xZoom, yZoom, zx, zy;
	INT32 SpriteChunk, MapOffset, Code, j, k, px, py;
	
	for (Offset = 0x380 - 4; Offset >= 0; Offset -= 4) {
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 2]);
		Priority = (Data & 0x8000) >> 15;
			
		if (Priority != 0 && Priority != 1) bprintf(PRINT_NORMAL, _T("Unused Priority %x\n"), Priority);
		if (Priority != PriorityDraw) continue;
		
		xFlip = (Data & 0x4000) >> 14;
		yFlip = (Data & 0x2000) >> 13;
		x = Data & 0x1ff;
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 1]);
		Tile = Data & 0x7ff;
		if (!Tile) continue;
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 0]);
		yZoom = (Data & 0xfe00) >> 9;
		y = Data & 0x1ff;
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 3]);
		Colour = (Data & 0xff00) >> 8;
		xZoom = (Data & 0x7f);		

		MapOffset = Tile << 7;

		xZoom += 1;
		yZoom += 1;

		y += 5;
		y += ( 128 - yZoom);

		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		for (SpriteChunk = 0; SpriteChunk < 128; SpriteChunk++)	{
			k = SpriteChunk % 8;
			j = SpriteChunk / 8;

			px = xFlip ? (7 - k) : k;
			py = yFlip ? (15 - j) : j;

			Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 3)]);
			Code &= (TaitoNumSpriteA - 1);

			xCur = x + ((k * xZoom) / 8);
			yCur = y + ((j * yZoom) / 16);

			zx = x + (((k + 1) * xZoom) / 8) - xCur;
			zy = y + (((j + 1) * yZoom) / 16) - yCur;

			yCur -= VisibleYOffset;

			RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 13, TaitoSpritesA);
		}
	}
}

static void SciRenderSprites(INT32 PriorityDraw, INT32 yOffs)
{
	UINT16 *SpriteMap = (UINT16*)TaitoSpriteMapRom;
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	INT32 Offset, StartOffs, Data, Tile, Colour, xFlip, yFlip;
	INT32 x, y, Priority, xCur, yCur;
	INT32 xZoom, yZoom, zx, zy;
	INT32 SpriteChunk, MapOffset, Code, j, k, px, py;
	
	StartOffs = (SciSpriteFrame & 1) ? 0x800 : 0;
	
	for (Offset = StartOffs; Offset < StartOffs + 0x800; Offset += 4) {
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 1]);
		Priority = (Data & 0x8000) >> 15;
		
		if (Priority != 0 && Priority != 1) bprintf(PRINT_NORMAL, _T("Unused Priority %x\n"), Priority);
		if (Priority != PriorityDraw) continue;
		
		Colour = (Data & 0x7f80) >> 7;
		xZoom = (Data & 0x3f);
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 3]);
		Tile = Data & 0x1fff;
		if (!Tile) continue;
		
		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 0]);
		yZoom = (Data & 0x7e00) >> 9;
		y = Data & 0x1ff;

		Data = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 2]);
		yFlip = (Data & 0x8000) >> 15;
		xFlip = (Data & 0x4000) >> 14;
		x = Data & 0x1ff;

		MapOffset = Tile << 5;

		xZoom += 1;
		yZoom += 1;

		y += yOffs;
		y += (64 - yZoom);

		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		for (SpriteChunk = 0; SpriteChunk < 32; SpriteChunk++) {
			k = SpriteChunk % 4;
			j = SpriteChunk / 4;

			px = xFlip ? (3-k) : k;
			py = yFlip ? (7-j) : j;

			Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 2)]);
			Code &= (TaitoNumSpriteA - 1);

			xCur = x + ((k * xZoom) / 4);
			yCur = y + ((j * yZoom) / 8);

			zx = x + (((k + 1) * xZoom) / 4) - xCur;
			zy = y + (((j + 1) * yZoom) / 8) - yCur;
			
			yCur -= 16;

			RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 13, TaitoSpritesA);
		}
	}
}

static void SpacegunRenderSprites(INT32 PriorityDraw)
{
	UINT16 *SpriteMap = (UINT16*)TaitoSpriteMapRom;
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	INT32 Offset, Data, Tile, Colour, xFlip, yFlip;
	INT32 x, y, Priority, xCur, yCur;
	INT32 xZoom, yZoom, zx, zy;
	INT32 SpriteChunk, MapOffset, Code, j, k, px, py;
	
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

		y += 4;

		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		for (SpriteChunk = 0; SpriteChunk < 32; SpriteChunk++) {
			k = SpriteChunk % 4;
			j = SpriteChunk / 4;

			px = xFlip ? (3 - k) : k;
			py = yFlip ? (7 - j) : j;

			Code = BURN_ENDIAN_SWAP_INT16(SpriteMap[MapOffset + px + (py << 2)]);
			Code &= (TaitoNumSpriteA - 1);			

			xCur = x + ((k * xZoom) / 4);
			yCur = y + ((j * yZoom) / 8);

			zx = x + (((k + 1) * xZoom) / 4) - xCur;
			zy = y + (((j + 1) * yZoom) / 8) - yCur;
			
			yCur -= 16;

			RenderSpriteZoom(Code, xCur, yCur, Colour, xFlip, yFlip, zx << 12, zy << 13, TaitoSpritesA);
		}

	}
}

static void AquajackDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 1, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 1, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}
	
	AquajackRenderSprites(1);
	
	TC0150RODDraw(-1, 0, 2, 1, 1, 2);
	
	AquajackRenderSprites(0);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TC0110PCRPalette);
}

static void BsharkDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	
	BurnTransferClear();
	TaitoZCalcPalette();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 1, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 1, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}
	
	BsharkRenderSprites(1, 8, 0x800);
	
	TC0150RODDraw(-1, 0xc0, 0, 1, 1, 2);
	
	BsharkRenderSprites(0, 8, 0x800);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TaitoPalette);
}

static void ChasehqDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 1, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 1, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}
	
	ChasehqRenderSprites(1);
	
	TC0150RODDraw(-1, 0xc0, 0, 0, 1, 2);
	
	ChasehqRenderSprites(0);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TC0110PCRPalette);
}

static void ContcircDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}
	
	ContcircRenderSprites(1, 24);
	
	TC0150RODDraw(-3 + 8, TaitoRoadPalBank << 6, 1, 0, 1, 2);
	
	ContcircRenderSprites(0, 24);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TC0110PCRPalette);
}

static void EnforceDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}
	
	ContcircRenderSprites(1, 24);
	
	TC0150RODDraw(-3 + 8, 0xc0, 1, 0, 1, 2);
	
	ContcircRenderSprites(0, 24);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TC0110PCRPalette);
}

static void DblaxleDraw()
{
	UINT8 Layer[4];
	UINT16 Priority = TC0480SCPGetBgPriority();
	
	Layer[0] = (Priority & 0xf000) >> 12;
	Layer[1] = (Priority & 0x0f00) >>  8;
	Layer[2] = (Priority & 0x00f0) >>  4;
	Layer[3] = (Priority & 0x000f) >>  0;
	
	BurnTransferClear();
	TaitoZCalcPalette();
	
	TC0480SCPTilemapRender(Layer[0], 1, TaitoChars);
	TC0480SCPTilemapRender(Layer[1], 0, TaitoChars);
	TC0480SCPTilemapRender(Layer[2], 0, TaitoChars);
	
	BsharkRenderSprites(1, 7, 0x2000);
	
	TC0150RODDraw(-1, 0xc0, 0, 0, 1, 2);
	
	BsharkRenderSprites(0, 7, 0x2000);
	
	TC0480SCPTilemapRender(Layer[3], 0, TaitoChars);
	
	TC0480SCPRenderCharLayer();
	BurnTransferCopy(TaitoPalette);
}

static void SciDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	
	BurnTransferClear();
	TaitoZCalcPalette();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 1, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 1, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}
	
	SciRenderSprites(1, 6);

	TC0150RODDraw(-1, 0xc0, 0, 0, 1, 2);
	
	SciRenderSprites(0, 6);

	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	
	BurnTransferCopy(TaitoPalette);
}

static void SpacegunDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	
	BurnTransferClear();
	
	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 1, TaitoChars);
		SpacegunRenderSprites(1);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 1, TaitoChars);
		SpacegunRenderSprites(1);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}
	
	SpacegunRenderSprites(0);
	
	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TC0110PCRPalette);
	
	for (INT32 i = 0; i < nBurnGunNumPlayers; i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}
}

static INT32 TaitoZFrame()
{
	INT32 nInterleave = TaitoFrameInterleave;

	if (TaitoReset) TaitoZDoReset();

	TaitoMakeInputsFunction();
	
	nTaitoCyclesDone[0] = nTaitoCyclesDone[1] = nTaitoCyclesDone[2] = 0;

	SekNewFrame();
	if (TaitoNumZ80s) ZetNewFrame();
		
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
		if (i == 10 && Sci && (GetCurrentFrame() & 1)) { SekSetIRQLine(6, SEK_IRQSTATUS_AUTO); nTaitoCyclesDone[0] += SekRun(200000 - 500); }
		if (i == 10 && Dblaxle && (GetCurrentFrame() & 1)) { SekSetIRQLine(6, SEK_IRQSTATUS_AUTO); nTaitoCyclesDone[0] += SekRun(200000 - 500); }
		if (i == 50 && Dblaxle) { SekSetIRQLine(6, SEK_IRQSTATUS_AUTO); nTaitoCyclesDone[0] += SekRun(200000 - 500); }
		if (i == (TaitoFrameInterleave - 1)) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		// Run 68000 #2
		if ((TaitoCpuACtrl & 0x01) && TaitoNumZ80s) {
			nCurrentCPU = 1;
			SekOpen(1);
			nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
			nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
			nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
			if (i == (TaitoFrameInterleave - 1)) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
			SekClose();
		}
		
		if (TaitoNumZ80s) {
			ZetOpen(0);
			BurnTimerUpdate(i * (nTaitoCyclesTotal[2] / nInterleave));
			ZetClose();
		}
	}
	
	if (TaitoNumZ80s) {
		ZetOpen(0);
		BurnTimerEndFrame(nTaitoCyclesTotal[2]);
		if (pBurnSoundOut) BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
		ZetClose();
	} else {
		SekOpen(1);
		if (TaitoCpuACtrl & 0x01) BurnTimerEndFrame(nTaitoCyclesTotal[1]);
		if (pBurnSoundOut) BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
		if (TaitoCpuACtrl & 0x01) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
		SekClose();
	}
	
	if (pBurnDraw) TaitoDrawFunction();

	return 0;
}

static INT32 TaitoZScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029674;
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
		if (TaitoNumZ80s) ZetScan(nAction);

		BurnYM2610Scan(nAction, pnMin);
		
		BurnGunScan();
				
		SCAN_VAR(TaitoAnalogPort0);
		SCAN_VAR(TaitoAnalogPort1);
		SCAN_VAR(TaitoAnalogPort2);
		SCAN_VAR(TaitoAnalogPort3);
		SCAN_VAR(TaitoInput);
		SCAN_VAR(OldSteer);
		SCAN_VAR(TaitoCpuACtrl);
		SCAN_VAR(TaitoZ80Bank);
		SCAN_VAR(SciSpriteFrame);
		SCAN_VAR(TaitoRoadPalBank);
		SCAN_VAR(nTaitoCyclesDone);
		SCAN_VAR(nTaitoCyclesSegment);
	}
	
	if (nAction & ACB_WRITE) {
		if (TaitoNumZ80s) {
			ZetOpen(0);
			ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetClose();
		}
	}

	return 0;
}

struct BurnDriver BurnDrvAquajack = {
	"aquajack", NULL, NULL, NULL, "1990",
	"Aquajack (World)\0", NULL, "Taito Corporation Japan", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_MISC, 0,
	NULL, AquajackRomInfo, AquajackRomName, NULL, NULL, AquajackInputInfo, AquajackDIPInfo,
	AquajackInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvAquajackj = {
	"aquajackj", "aquajack", NULL, NULL, "1990",
	"Aquajack (Japan)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_MISC, 0,
	NULL, AquajackjRomInfo, AquajackjRomName, NULL, NULL, AquajackInputInfo, AquajackjDIPInfo,
	AquajackInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvAquajacku = {
	"aquajacku", "aquajack", NULL, NULL, "1990",
	"Aquajack (US)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_MISC, 0,
	NULL, AquajackuRomInfo, AquajackuRomName, NULL, NULL, AquajackInputInfo, AquajackjDIPInfo,
	AquajackInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvBshark = {
	"bshark", NULL, NULL, NULL, "1989",
	"Battle Shark (World)\0", NULL, "Taito Corporation Japan", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_SHOOT, 0,
	NULL, BsharkRomInfo, BsharkRomName, NULL, NULL, BsharkInputInfo, BsharkDIPInfo,
	BsharkInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvBsharkj = {
	"bsharkj", "bshark", NULL, NULL, "1989",
	"Battle Shark (Japan)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_SHOOT, 0,
	NULL, BsharkjRomInfo, BsharkjRomName, NULL, NULL, BsharkInputInfo, BsharkjDIPInfo,
	BsharkInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvBsharkjjs = {
	"bsharkjjs", "bshark", NULL, NULL, "1989",
	"Battle Shark (Japan, Joystick)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_SHOOT, 0,
	NULL, BsharkjjsRomInfo, BsharkjjsRomName, NULL, NULL, BsharkjjsInputInfo, BsharkjjsDIPInfo,
	BsharkInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvBsharku = {
	"bsharku", "bshark", NULL, NULL, "1989",
	"Battle Shark (US)\0", NULL, "Taito America Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_SHOOT, 0,
	NULL, BsharkuRomInfo, BsharkuRomName, NULL, NULL, BsharkInputInfo, BsharkuDIPInfo,
	BsharkInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvChasehq = {
	"chasehq", NULL, NULL, NULL, "1988",
	"Chase H.Q. (World)\0", NULL, "Taito Corporation Japan", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, ChasehqRomInfo, ChasehqRomName, NULL, NULL, ChasehqInputInfo, ChasehqDIPInfo,
	ChasehqInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvChasehqj = {
	"chasehqj", "chasehq", NULL, NULL, "1988",
	"Chase H.Q. (Japan)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, ChasehqjRomInfo, ChasehqjRomName, NULL, NULL, ChasehqInputInfo, ChasehqjDIPInfo,
	ChasehqInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvChasehqu = {
	"chasehqu", "chasehq", NULL, NULL, "1988",
	"Chase H.Q. (US)\0", NULL, "Taito America Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, ChasehquRomInfo, ChasehquRomName, NULL, NULL, ChasehqInputInfo, ChasehqDIPInfo,
	ChasehqInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvContcirc = {
	"contcirc", NULL, NULL, NULL, "1987",
	"Continental Circus (World)\0", NULL, "Taito Corporation Japan", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, ContcircRomInfo, ContcircRomName, NULL, NULL, ContcircInputInfo, ContcircDIPInfo,
	ContcircInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvContcircu = {
	"contcircu", "contcirc", NULL, NULL, "1987",
	"Continental Circus (US set 1)\0", NULL, "Taito America Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, ContcircuRomInfo, ContcircuRomName, NULL, NULL, ContcircInputInfo, ContcircuDIPInfo,
	ContcircInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvContcircua = {
	"contcircua", "contcirc", NULL, NULL, "1987",
	"Continental Circus (US set 2)\0", NULL, "Taito America Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, ContcircuaRomInfo, ContcircuaRomName, NULL, NULL, ContcircInputInfo, ContcircuDIPInfo,
	ContcircInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDblaxle = {
	"dblaxle", NULL, NULL, NULL, "1991",
	"Double Axle (US)\0", "Imperfect sound, bad dump in sample rom?", "Taito America Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, DblaxleRomInfo, DblaxleRomName, NULL, NULL, DblaxleInputInfo, DblaxleDIPInfo,
	DblaxleInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvPwheelsj = {
	"pwheelsj", "dblaxle", NULL, NULL, "1991",
	"Power Wheels (Japan)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, PwheelsjRomInfo, PwheelsjRomName, NULL, NULL, DblaxleInputInfo, PwheelsjDIPInfo,
	DblaxleInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvEnforce = {
	"enforce", NULL, NULL, NULL, "1988",
	"Enforce (Japan)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_SHOOT, 0,
	NULL, EnforceRomInfo, EnforceRomName, NULL, NULL, EnforceInputInfo, EnforceDIPInfo,
	EnforceInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvNightstr = {
	"nightstr", NULL, NULL, NULL, "1989",
	"Night Striker (World)\0", NULL, "Taito Corporation Japan", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_MISC, 0,
	NULL, NightstrRomInfo, NightstrRomName, NULL, NULL, NightstrInputInfo, NightstrDIPInfo,
	NightstrInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvNightstrj = {
	"nightstrj", "nightstr", NULL, NULL, "1989",
	"Night Striker (Japan)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_MISC, 0,
	NULL, NightstrjRomInfo, NightstrjRomName, NULL, NULL, NightstrInputInfo, NightstrjDIPInfo,
	NightstrInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvNightstru = {
	"nightstru", "nightstr", NULL, NULL, "1989",
	"Night Striker (US)\0", NULL, "Taito America Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, NightstruRomInfo, NightstruRomName, NULL, NULL, NightstrInputInfo, NightstruDIPInfo,
	NightstrInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvSci = {
	"sci", NULL, NULL, NULL, "1989",
	"Special Criminal Investigation (World set 1)\0", NULL, "Taito Corporation Japan", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, SciRomInfo, SciRomName, NULL, NULL, SciInputInfo, SciDIPInfo,
	SciInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvScia = {
	"scia", "sci", NULL, NULL, "1989",
	"Special Criminal Investigation (World set 2)\0", NULL, "Taito Corporation Japan", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, SciaRomInfo, SciaRomName, NULL, NULL, SciInputInfo, SciDIPInfo,
	SciInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvScij = {
	"scij", "sci", NULL, NULL, "1989",
	"Special Criminal Investigation (Japan)\0", NULL, "Taito Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, ScijRomInfo, ScijRomName, NULL, NULL, SciInputInfo, ScijDIPInfo,
	SciInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvSciu = {
	"sciu", "sci", NULL, NULL, "1989",
	"Special Criminal Investigation (US)\0", NULL, "Taito America Corporation", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, SciuRomInfo, SciuRomName, NULL, NULL, SciInputInfo, SciuDIPInfo,
	SciInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvScinegro = {
	"scinegro", "sci", NULL, NULL, "1989",
	"Special Criminal Investigation (Negro bootleg)\0", NULL, "Negro", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_TAITOZ, GBF_RACING, 0,
	NULL, ScinegroRomInfo, ScinegroRomName, NULL, NULL, SciInputInfo, SciDIPInfo,
	SciInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvSpacegun = {
	"spacegun", NULL, NULL, NULL, "1990",
	"Space Gun (World)\0", NULL, "Taito Corporation Japan", "Taito-Z",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOZ, GBF_SHOOT, 0,
	NULL, SpacegunRomInfo, SpacegunRomName, NULL, NULL, SpacegunInputInfo, SpacegunDIPInfo,
	SpacegunInit, TaitoZExit, TaitoZFrame, NULL, TaitoZScan,
	NULL, 0x1000, 320, 240, 4, 3
};
