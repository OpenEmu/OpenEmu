#include "cps.h"

#define CPS1_68K_PROGRAM_BYTESWAP		1
#define CPS1_68K_PROGRAM_NO_BYTESWAP	2
#define CPS1_Z80_PROGRAM				3
#define CPS1_TILES						4
#define CPS1_OKIM6295_SAMPLES			5
#define CPS1_QSOUND_SAMPLES				6
#define CPS1_PIC						7

typedef INT32 (*Cps1Callback)(INT32);
static Cps1Callback Cps1GfxLoadCallbackFunction = NULL;

// Input Definitions

static struct BurnInputInfo NTFOInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(NTFO)

static struct BurnInputInfo ThreeWondersInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Button 1"      , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Button 2"      , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Button 1"      , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Button 2"      , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(ThreeWonders)

static struct BurnInputInfo CaptcommInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"P4 Coin"          , BIT_DIGITAL  , CpsInp179+6, "p4 coin"   },
	{"P4 Start"         , BIT_DIGITAL  , CpsInp179+7, "p4 start"  },
	{"P4 Up"            , BIT_DIGITAL  , CpsInp179+3, "p4 up"     },
	{"P4 Down"          , BIT_DIGITAL  , CpsInp179+2, "p4 down"   },
	{"P4 Left"          , BIT_DIGITAL  , CpsInp179+1, "p4 left"   },
	{"P4 Right"         , BIT_DIGITAL  , CpsInp179+0, "p4 right"  },
	{"P4 Attack"        , BIT_DIGITAL  , CpsInp179+4, "p4 fire 1" },
	{"P4 Jump"          , BIT_DIGITAL  , CpsInp179+5, "p4 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Captcomm)

static struct BurnInputInfo CawingInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Cawing)

static struct BurnInputInfo Cworld2jInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Answer 1"      , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Answer 2"      , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Answer 3"      , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Answer 4"      , BIT_DIGITAL  , CpsInp001+7, "p1 fire 4" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Answer 1"      , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Answer 2"      , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Answer 3"      , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Answer 4"      , BIT_DIGITAL  , CpsInp000+7, "p2 fire 4" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Cworld2j)

static struct BurnInputInfo DinoInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0 , "p1 coin"  },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4 , "p1 start" },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3 , "p1 up"    },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2 , "p1 down"  },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1 , "p1 left"  },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0 , "p1 right" },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4 , "p1 fire 1"},
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5 , "p1 fire 2"},

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1 , "p2 coin"  },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5 , "p2 start" },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3 , "p2 up"    },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2 , "p2 down"  },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1 , "p2 left"  },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0 , "p2 right" },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4 , "p2 fire 1"},
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5 , "p2 fire 2"},

	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc001+6, "p3 coin"  },
	{"P3 Start"         , BIT_DIGITAL  , CpsInpc001+7, "p3 start" },
	{"P3 Up"            , BIT_DIGITAL  , CpsInpc001+3, "p3 up"    },
	{"P3 Down"          , BIT_DIGITAL  , CpsInpc001+2, "p3 down"  },
	{"P3 Left"          , BIT_DIGITAL  , CpsInpc001+1, "p3 left"  },
	{"P3 Right"         , BIT_DIGITAL  , CpsInpc001+0, "p3 right" },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc001+4, "p3 fire 1"},
	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc001+5, "p3 fire 2"},

	{"Reset"            , BIT_DIGITAL  , &CpsReset   ,  "reset"   },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6 ,  "diag"    },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2 ,  "service" },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E     ,  "dip"     },
};

STDINPUTINFO(Dino)

static struct BurnInputInfo DinohInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0 , "p1 coin"  },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4 , "p1 start" },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3 , "p1 up"    },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2 , "p1 down"  },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1 , "p1 left"  },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0 , "p1 right" },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4 , "p1 fire 1"},
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5 , "p1 fire 2"},
	{"P1 Bomb"          , BIT_DIGITAL  , CpsInp001+6 , "p1 fire 3"},

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1 , "p2 coin"  },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5 , "p2 start" },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3 , "p2 up"    },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2 , "p2 down"  },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1 , "p2 left"  },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0 , "p2 right" },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4 , "p2 fire 1"},
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5 , "p2 fire 2"},
	{"P2 Bomb"          , BIT_DIGITAL  , CpsInp000+6 , "p2 fire 3"},

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset   ,  "reset"   },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6 ,  "diag"    },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2 ,  "service" },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E     ,  "dip"     },
};

STDINPUTINFO(Dinoh)

static struct BurnInputInfo DynwarInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack Left"   , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Attack Right"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack Left"   , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Attack Right"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Dynwar)

static struct BurnInputInfo FfightInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Ffight)

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo ForgottnInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL,    CpsInp018+0, "p1 coin"  },
	{"P1 Start"         , BIT_DIGITAL,    CpsInp018+4, "p1 start" },
	{"P1 Up"            , BIT_DIGITAL,    CpsInp001+3, "p1 up"    },
	{"P1 Down"          , BIT_DIGITAL,    CpsInp001+2, "p1 down"  },
	{"P1 Left"          , BIT_DIGITAL,    CpsInp001+1, "p1 left"  },
	{"P1 Right"         , BIT_DIGITAL,    CpsInp001+0, "p1 right" },
	{"P1 Attack"        , BIT_DIGITAL,    CpsInp001+4, "p1 fire 1"},
	A("P1 Turn"         , BIT_ANALOG_REL, &CpsInp055,  "p1 z-axis"),

	{"P2 Coin"          , BIT_DIGITAL,    CpsInp018+1, "p2 coin"  },
	{"P2 Start"         , BIT_DIGITAL,    CpsInp018+5, "p2 start" },
	{"P2 Up"            , BIT_DIGITAL,    CpsInp000+3, "p2 up"    },
	{"P2 Down"          , BIT_DIGITAL,    CpsInp000+2, "p2 down"  },
	{"P2 Left"          , BIT_DIGITAL,    CpsInp000+1, "p2 left"  },
	{"P2 Right"         , BIT_DIGITAL,    CpsInp000+0, "p2 right" },
	{"P2 Attack"        , BIT_DIGITAL,    CpsInp000+4, "p2 fire 1"},
	A("P2 Turn"         , BIT_ANALOG_REL, &CpsInp05d,  "p2 z-axis"),

	{"Reset"            , BIT_DIGITAL,    &CpsReset,   "reset"    },
	{"Service"          , BIT_DIGITAL,    CpsInp018+2, "service"  },

	{"Dip A"            , BIT_DIPSWITCH,  &Cpi01A    , "dip"      },
	{"Dip B"            , BIT_DIPSWITCH,  &Cpi01C    , "dip"      },
	{"Dip C"            , BIT_DIPSWITCH,  &Cpi01E    , "dip"      },
};

#undef A

STDINPUTINFO(Forgottn)

static struct BurnInputInfo GhoulsInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Fire"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	
	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Fire"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Ghouls)

static struct BurnInputInfo KnightsInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Knights)

static struct BurnInputInfo KodInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Kod)

static struct BurnInputInfo KodhInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc000+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInpc000+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInpc000+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInpc000+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInpc000+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInpc000+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc000+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc000+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Kodh)

static struct BurnInputInfo MegamanInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Fire"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Select"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Fire"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Select"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Megaman)

static struct BurnInputInfo MercsInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Shot"          , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Special"       , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Mercs)

static struct BurnInputInfo MswordInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Msword)

static struct BurnInputInfo MtwinsInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Mtwins)

static struct BurnInputInfo NemoInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Nemo)

static struct BurnInputInfo Pang3InputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot 1"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Shot 2"        , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot 1"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Shot 2"        , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Pang3)

static struct BurnInputInfo PnickjInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Turn 1"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Turn 2"        , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Turn 1"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Turn 2"        , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Pnickj)

static struct BurnInputInfo PunisherInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Punisher)

static struct BurnInputInfo PunisherbzInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Super"         , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Super"         , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
};

STDINPUTINFO(Punisherbz)

static struct BurnInputInfo QadInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Answer 1"      , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Answer 2"      , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Answer 3"      , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Answer 4"      , BIT_DIGITAL  , CpsInp001+7, "p1 fire 4" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Answer 1"      , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Answer 2"      , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Answer 3"      , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Answer 4"      , BIT_DIGITAL  , CpsInp000+7, "p2 fire 4" },
	
	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Qad)

static struct BurnInputInfo Qtono2jInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Answer 1"      , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Answer 2"      , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Answer 3"      , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Answer 4"      , BIT_DIGITAL  , CpsInp001+7, "p1 fire 4" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Answer 1"      , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Answer 2"      , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Answer 3"      , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Answer 4"      , BIT_DIGITAL  , CpsInp000+7, "p2 fire 4" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Qtono2j)

static struct BurnInputInfo Sf2InputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp177+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp177+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp177+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp177+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp177+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp177+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2)

static struct BurnInputInfo Sf2ueInputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp1fd+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp1fd+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp1fd+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp1fd+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp1fd+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp1fd+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2ue)

static struct BurnInputInfo Sf2yycInputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp019+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp019+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp176+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp176+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp176+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp019+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp019+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp176+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp176+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp176+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp019+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp019+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2yyc)

static struct BurnInputInfo Sf2m1InputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp012+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp012+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp012+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp012+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp012+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp012+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2m1)

static struct BurnInputInfo Sf2m3InputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp029+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp029+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp011+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp011+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp011+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp011+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp011+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp011+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp011+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp186+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp186+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp186+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp029+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp029+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp010+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp010+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp010+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp010+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp010+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp010+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp010+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp186+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp186+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp186+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp029+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp029+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2m3)

static struct BurnInputInfo SfzchInputList[] =
{
	{"P1 Pause"         , BIT_DIGITAL  , CpsInp018+2, ""          },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
 	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp001+7, "p1 fire 4" },
 	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp018+0, "p1 fire 5" },
 	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp018+6, "p1 fire 6" },

 	{"P2 Pause"         , BIT_DIGITAL  , CpsInp018+3, ""          },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
 	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp000+7, "p2 fire 4" },
 	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp018+1, "p2 fire 5" },
 	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp018+7, "p2 fire 6" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
};

STDINPUTINFO(Sfzch)

static struct BurnInputInfo SlammastInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Pin"           , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Pin"           , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc001+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInpc001+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInpc001+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInpc001+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInpc001+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInpc001+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc001+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc001+5, "p3 fire 2"},
 	{"P3 Pin"           , BIT_DIGITAL  , CpsInp001+7 , "p3 fire 3"},

 	{"P4 Coin"          , BIT_DIGITAL  , CpsInpc003+6, "p4 coin"  },
 	{"P4 Start"         , BIT_DIGITAL  , CpsInpc003+7, "p4 start" },
 	{"P4 Up"            , BIT_DIGITAL  , CpsInpc003+3, "p4 up"    },
 	{"P4 Down"          , BIT_DIGITAL  , CpsInpc003+2, "p4 down"  },
 	{"P4 Left"          , BIT_DIGITAL  , CpsInpc003+1, "p4 left"  },
 	{"P4 Right"         , BIT_DIGITAL  , CpsInpc003+0, "p4 right" },
 	{"P4 Attack"        , BIT_DIGITAL  , CpsInpc003+4, "p4 fire 1"},
 	{"P4 Jump"          , BIT_DIGITAL  , CpsInpc003+5, "p4 fire 2"},
 	{"P4 Pin"           , BIT_DIGITAL  , CpsInp000+7 , "p4 fire 3"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset   , "reset"    },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6 , "diag"     },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2 , "service"  },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E     , "dip"      },
};

STDINPUTINFO(Slammast)

static struct BurnInputInfo StriderInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Strider)

static struct BurnInputInfo UnsquadInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },

};

STDINPUTINFO(Unsquad)

static struct BurnInputInfo VarthInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Bomb"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Bomb"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Varth)

static struct BurnInputInfo WillowInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
 
 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Willow)

static struct BurnInputInfo WofInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc001+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInpc001+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInpc001+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInpc001+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInpc001+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInpc001+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc001+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc001+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Wof)

static struct BurnInputInfo WofchInputList[] =
{
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

 	{"P3 Start"         , BIT_DIGITAL  , CpsInpc001+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInpc001+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInpc001+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInpc001+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInpc001+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc001+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc001+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
};

STDINPUTINFO(Wofch)

static struct BurnInputInfo WofhfhInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Wofhfh)

static struct BurnInputInfo WofhInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   }, // doesn't work in some sets?
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Wofh)

// don't know where this one reads config data from (if at all), reads values at 0x80001a, 0x80001e, 0x80001f (but values make no difference)
// if I force it to use EEPROM, the config data gets forgotten
static struct BurnInputInfo WofsjbInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp01B+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp01B+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp01B+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp01B+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp01B+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp01B+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp01B+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp01B+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Wofsjb)

// Dip Switch Definitions

#define CPS1_COINAGE_1(dipval) \
	{0     , 0xfe, 0   , 8   , "Coin A"                 }, \
	{dipval, 0x01, 0x07, 0x07, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x06, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x05, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x01, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x07, 0x02, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x07, 0x03, "1 Coin  4 Credits"      }, \
	{dipval, 0x01, 0x07, 0x04, "1 Coin  6 Credits"      }, \
							       \
	{0     , 0xfe, 0   , 8   , "Coin B"                 }, \
	{dipval, 0x01, 0x38, 0x38, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x30, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x28, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x08, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x38, 0x10, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x38, 0x18, "1 Coin  4 Credits"      }, \
	{dipval, 0x01, 0x38, 0x20, "1 Coin  6 Credits"      }, 
	
#define CPS1_COINAGE_2(dipval) \
	{0     , 0xfe, 0   , 8   , "Coinage"                }, \
	{dipval, 0x01, 0x07, 0x07, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x06, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x05, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x01, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x07, 0x02, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x07, 0x03, "1 Coin  4 Credits"      }, \
	{dipval, 0x01, 0x07, 0x04, "1 Coin  6 Credits"      }, 
	
#define CPS1_COINAGE_3(dipval) \
	{0     , 0xfe, 0   , 8   , "Coin A"                 }, \
	{dipval, 0x01, 0x07, 0x06, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x05, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x04, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x07, "2 Coins 1 Credit (1 to cont)"}, \
	{dipval, 0x01, 0x07, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x01, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x07, 0x02, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x07, 0x03, "1 Coin  4 Credits"      }, \
							       \
	{0     , 0xfe, 0   , 8   , "Coin B"                 }, \
	{dipval, 0x01, 0x38, 0x30, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x28, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x20, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x38, "2 Coins 1 Credit (1 to cont)"}, \
	{dipval, 0x01, 0x38, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x08, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x38, 0x10, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x38, 0x18, "1 Coin  4 Credits"      }, 
	
#define CPS1_DIFFICULTY_1(dipval) \
	{0   , 0xfe, 0   , 8   , "Difficulty"               }, \
	{dipval, 0x01, 0x07, 0x00, "1 (Easiest)"            }, \
	{dipval, 0x01, 0x07, 0x01, "2"                      }, \
	{dipval, 0x01, 0x07, 0x02, "3"                      }, \
	{dipval, 0x01, 0x07, 0x03, "4 (Normal)"             }, \
	{dipval, 0x01, 0x07, 0x04, "5"                      }, \
	{dipval, 0x01, 0x07, 0x05, "6"                      }, \
	{dipval, 0x01, 0x07, 0x06, "7"                      }, \
	{dipval, 0x01, 0x07, 0x07, "8 (Hardest)"            },
	
#define CPS1_DIFFICULTY_2(dipval) \
	{0   , 0xfe, 0   , 8   , "Difficulty"               }, \
	{dipval, 0x01, 0x07, 0x03, "1 (Easiest)"            }, \
	{dipval, 0x01, 0x07, 0x02, "2"                      }, \
	{dipval, 0x01, 0x07, 0x01, "3"                      }, \
	{dipval, 0x01, 0x07, 0x00, "4 (Normal)"             }, \
	{dipval, 0x01, 0x07, 0x04, "5"                      }, \
	{dipval, 0x01, 0x07, 0x05, "6"                      }, \
	{dipval, 0x01, 0x07, 0x06, "7"                      }, \
	{dipval, 0x01, 0x07, 0x07, "8 (Hardest)"            },

static struct BurnDIPInfo NTFODIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x12, 0x01, 0x40, 0x00, "Off"                    },
	{0x12, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x13)
	
	{0   , 0xfe, 0   , 4   , "Level Up Timer"         },
	{0x13, 0x01, 0x18, 0x00, "More Slowly"            },
	{0x13, 0x01, 0x18, 0x08, "Slowly"                 },
	{0x13, 0x01, 0x18, 0x10, "Quickly"                },
	{0x13, 0x01, 0x18, 0x18, "More Quickly"           },

	{0   , 0xfe, 0   , 4   , "Bullet's Speed"         },
	{0x13, 0x01, 0x60, 0x00, "Very Slow"              },
	{0x13, 0x01, 0x60, 0x20, "Slow"                   },
	{0x13, 0x01, 0x60, 0x40, "Fast"                   },
	{0x13, 0x01, 0x60, 0x60, "Very Fast"              },

	{0   , 0xfe, 0   , 2   , "Initital Vitality"      },
	{0x13, 0x01, 0x80, 0x00, "3 bars"                 },
	{0x13, 0x01, 0x80, 0x80, "4 bars"                 },
	
	// Dip C
	{0   , 0xfe, 0   , 2   , "Throttle Game Speed"    },
	{0x14, 0x01, 0x01, 0x01, "Off"                    },
	{0x14, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x04, 0x00, "Off"                    },
	{0x14, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x14, 0x01, 0x08, 0x00, "Off"                    },
	{0x14, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x14, 0x01, 0x20, 0x00, "Off"                    },
	{0x14, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x40, 0x00, "Off"                    },
	{0x14, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x14, 0x01, 0x80, 0x00, "Game"                   },
	{0x14, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(NTFO)

static struct BurnDIPInfo ThreeWondersDIPList[]=
{
	// Defaults
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x65, NULL                     },
	{0x15, 0xff, 0xff, 0x66, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x13)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x13, 0x01, 0x40, 0x00, "Off"                    },
	{0x13, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 4   , "Action Lives"           },
	{0x14, 0x01, 0x03, 0x00, "1"                      },
	{0x14, 0x01, 0x03, 0x01, "2"                      },
	{0x14, 0x01, 0x03, 0x02, "3"                      },
	{0x14, 0x01, 0x03, 0x03, "5"                      },

	{0   , 0xfe, 0   , 4   , "Action Game Level"      },
	{0x14, 0x01, 0x0c, 0x00, "Easy"                   },
	{0x14, 0x01, 0x0c, 0x04, "Normal"                 },
	{0x14, 0x01, 0x0c, 0x08, "Hard"                   },
	{0x14, 0x01, 0x0c, 0x0c, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Shooting Lives"         },
	{0x14, 0x01, 0x30, 0x00, "1"                      },
	{0x14, 0x01, 0x30, 0x10, "2"                      },
	{0x14, 0x01, 0x30, 0x20, "3"                      },
	{0x14, 0x01, 0x30, 0x30, "5"                      },

	{0   , 0xfe, 0   , 4   , "Shooting Game Level"    },
	{0x14, 0x01, 0xc0, 0x00, "Easy"                   },
	{0x14, 0x01, 0xc0, 0x40, "Normal"                 },
	{0x14, 0x01, 0xc0, 0x80, "Hard"                   },
	{0x14, 0x01, 0xc0, 0xc0, "Hardest"                },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Puzzle Lives"           },
	{0x15, 0x01, 0x03, 0x00, "1"                      },
	{0x15, 0x01, 0x03, 0x01, "2"                      },
	{0x15, 0x01, 0x03, 0x02, "3"                      },
	{0x15, 0x01, 0x03, 0x03, "5"                      },

	{0   , 0xfe, 0   , 4   , "Puzzle Game Level"      },
	{0x15, 0x01, 0x0c, 0x00, "Easy"                   },
	{0x15, 0x01, 0x0c, 0x04, "Normal"                 },
	{0x15, 0x01, 0x0c, 0x08, "Hard"                   },
	{0x15, 0x01, 0x0c, 0x0c, "Hardest"                },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x15, 0x01, 0x10, 0x00, "Off"                    },
	{0x15, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x15, 0x01, 0x20, 0x00, "Off"                    },
	{0x15, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x15, 0x01, 0x80, 0x00, "Game"                   },
	{0x15, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(ThreeWonders)

static struct BurnDIPInfo CaptcommDIPList[]=
{
	// Defaults
	{0x23, 0xff, 0xff, 0x00, NULL                     },
	{0x24, 0xff, 0xff, 0x0b, NULL                     },
	{0x25, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x23)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x23, 0x01, 0x40, 0x00, "Off"                    },
	{0x23, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x24)

	{0   , 0xfe, 0   , 4   , "Difficulty 2"           },
	{0x24, 0x01, 0x18, 0x00, "Easy"                   },
	{0x24, 0x01, 0x18, 0x08, "Normal"                 },
	{0x24, 0x01, 0x18, 0x10, "Hard"                   },
	{0x24, 0x01, 0x18, 0x18, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Play Mode"              },
	{0x24, 0x01, 0xc0, 0x80, "1 Player"               },
	{0x24, 0x01, 0xc0, 0x00, "2 Player"               },
	{0x24, 0x01, 0xc0, 0x40, "3 Player"               },
	{0x24, 0x01, 0xc0, 0xc0, "4 Player"               },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x25, 0x01, 0x03, 0x03, "1"                      },
	{0x25, 0x01, 0x03, 0x00, "2"                      },
	{0x25, 0x01, 0x03, 0x01, "3"                      },
	{0x25, 0x01, 0x03, 0x02, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x25, 0x01, 0x04, 0x00, "Off"                    },
	{0x25, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x25, 0x01, 0x08, 0x00, "Off"                    },
	{0x25, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x25, 0x01, 0x10, 0x00, "Off"                    },
	{0x25, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x25, 0x01, 0x20, 0x00, "Off"                    },
	{0x25, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x25, 0x01, 0x40, 0x00, "Off"                    },
	{0x25, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x25, 0x01, 0x80, 0x00, "Game"                   },
	{0x25, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Captcomm)

static struct BurnDIPInfo CawingDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Difficulty (Enemys strength)"},
	{0x16, 0x01, 0x07, 0x00, "1 (Easiest)"            },
	{0x16, 0x01, 0x07, 0x01, "2"                      },
	{0x16, 0x01, 0x07, 0x02, "3"                      },
	{0x16, 0x01, 0x07, 0x03, "4 (Normal)"             },
	{0x16, 0x01, 0x07, 0x04, "5"                      },
	{0x16, 0x01, 0x07, 0x05, "6"                      },
	{0x16, 0x01, 0x07, 0x06, "7"                      },
	{0x16, 0x01, 0x07, 0x07, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 4   , "Difficulty (Players strength)"},
	{0x16, 0x01, 0x18, 0x08, "Easy"                   },
	{0x16, 0x01, 0x18, 0x00, "Normal"                 },
	{0x16, 0x01, 0x18, 0x10, "Difficult"              },
	{0x16, 0x01, 0x18, 0x18, "Very Difficult"         },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Sound"                  },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Cawing)

static struct BurnDIPInfo Cworld2jDIPList[]=
{
	// Defaults
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x01, NULL                     },
	{0x11, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x0f)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x0f, 0x01, 0x40, 0x00, "Off"                    },
	{0x0f, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Extended Test Mode"     },
	{0x0f, 0x01, 0x80, 0x00, "Off"                    },
	{0x0f, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0x07, 0x01, "0"                      },
	{0x10, 0x01, 0x07, 0x02, "1"                      },
	{0x10, 0x01, 0x07, 0x03, "2"                      },
	{0x10, 0x01, 0x07, 0x04, "3"                      },
	{0x10, 0x01, 0x07, 0x05, "4"                      },

	{0   , 0xfe, 0   , 3   , "Extend"                 },
	{0x10, 0x01, 0x18, 0x18, "D"                      },
	{0x10, 0x01, 0x18, 0x08, "E"                      },
	{0x10, 0x01, 0x18, 0x00, "N"                      },

	{0   , 0xfe, 0   , 5   , "Lives"                  },
	{0x10, 0x01, 0xe0, 0xe0, "1"                      },
	{0x10, 0x01, 0xe0, 0x60, "2"                      },
	{0x10, 0x01, 0xe0, 0x00, "3"                      },
	{0x10, 0x01, 0xe0, 0x40, "4"                      },
	{0x10, 0x01, 0xe0, 0x20, "5"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x04, 0x00, "Off"                    },
	{0x11, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x08, 0x00, "Off"                    },
	{0x11, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x11, 0x01, 0x20, 0x00, "Off"                    },
	{0x11, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x11, 0x01, 0x40, 0x00, "Off"                    },
	{0x11, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x11, 0x01, 0x80, 0x00, "Game"                   },
	{0x11, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Cworld2j)

static struct BurnDIPInfo DinoDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1b, 0x01, 0x08, 0x00, "Off"                    },
	{0x1b, 0x01, 0x08, 0x08, "On"                     },
};

STDDIPINFO(Dino)

static struct BurnDIPInfo DinohDIPList[]=
{
	// Defaults
	{0x1d, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },
};

STDDIPINFO(Dinoh)

static struct BurnDIPInfo DynwarDIPList[]=
{
	// Defaults
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
        CPS1_COINAGE_3(0x14)

        {0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x80, 0x00, "Off"                    },
	{0x14, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_2(0x15)
	
	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x16, 0x01, 0x01, 0x00, "Off"                    },
	{0x16, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Turbo Mode"             },
	{0x16, 0x01, 0x02, 0x00, "Off"                    },
	{0x16, 0x01, 0x02, 0x02, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x16, 0x01, 0x10, 0x00, "Off"                    },
	{0x16, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x16, 0x01, 0x20, 0x20, "Off"                    },
	{0x16, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x16, 0x01, 0x80, 0x00, "Game"                   },
	{0x16, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Dynwar)

static struct BurnDIPInfo FfightDIPList[]=
{
	// Defaults
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x0b, NULL                     },
	{0x16, 0xff, 0xff, 0x60, NULL                     },
		
	// Dip A
	CPS1_COINAGE_1(0x14)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x14, 0x01, 0x40, 0x00, "Off"                    },
	{0x14, 0x01, 0x40, 0x40, "On"                     },
		
	// Dip B
	CPS1_DIFFICULTY_1(0x15)

	{0   , 0xfe, 0   , 4   , "Difficulty 2"           },
	{0x15, 0x01, 0x18, 0x00, "Easy"                   },
	{0x15, 0x01, 0x18, 0x08, "Normal"                 },
	{0x15, 0x01, 0x18, 0x10, "Hard"                   },
	{0x15, 0x01, 0x18, 0x18, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x60, 0x00, "100k"                   },
	{0x15, 0x01, 0x60, 0x20, "200k"                   },
	{0x15, 0x01, 0x60, 0x40, "100k and every 200k"    },
	{0x15, 0x01, 0x60, 0x60, "None"                   },
	
	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x03, 0x03, "1"                      },
	{0x16, 0x01, 0x03, 0x00, "2"                      },
	{0x16, 0x01, 0x03, 0x01, "3"                      },
	{0x16, 0x01, 0x03, 0x02, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x16, 0x01, 0x04, 0x00, "Off"                    },
	{0x16, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x16, 0x01, 0x08, 0x00, "Off"                    },
	{0x16, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x16, 0x01, 0x10, 0x00, "Off"                    },
	{0x16, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x16, 0x01, 0x20, 0x00, "Off"                    },
	{0x16, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x40, 0x00, "Off"                    },
	{0x16, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x16, 0x01, 0x80, 0x00, "Game"                   },
	{0x16, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Ffight)

static struct BurnDIPInfo ForgottnDIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x03, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x12, 0x01, 0x40, 0x40, "Off"                    },
	{0x12, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x12, 0x01, 0x80, 0x00, "Off"                    },
	{0x12, 0x01, 0x80, 0x80, "On"                     },
	
	// Dip B
	CPS1_DIFFICULTY_1(0x13)
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x40, 0x00, "Off"                    },
	{0x13, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(Forgottn)

static struct BurnDIPInfo GhoulsDIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x12, 0x01, 0xc0, 0x00, "Upright 1 Player"       },
	{0x12, 0x01, 0xc0, 0x40, "Upright 2 Players"      },
	{0x12, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x13)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x30, 0x10, "10k, 30k and every 30k" },
	{0x13, 0x01, 0x30, 0x20, "20k, 50k and every 70k" },
	{0x13, 0x01, 0x30, 0x00, "30k, 60k and every 70k" },
	{0x13, 0x01, 0x30, 0x30, "40k, 70k and every 80k" },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x00, "3"                      },
	{0x14, 0x01, 0x03, 0x01, "4"                      },
	{0x14, 0x01, 0x03, 0x02, "5"                      },
	{0x14, 0x01, 0x03, 0x03, "6"                      },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x14, 0x01, 0x80, 0x00, "Game"                   },
	{0x14, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Ghouls)

static struct BurnDIPInfo GhoulsuDIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x12, 0x01, 0xc0, 0x00, "Upright 1 Player"       },
	{0x12, 0x01, 0xc0, 0x40, "Upright 2 Players"      },
	{0x12, 0x01, 0xc0, 0xc0, "Cocktail"               },
	
	// Dip B
	CPS1_DIFFICULTY_2(0x13)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x30, 0x10, "10k, 30k and every 30k" },
	{0x13, 0x01, 0x30, 0x20, "20k, 50k and every 70k" },
	{0x13, 0x01, 0x30, 0x00, "30k, 60k and every 70k" },
	{0x13, 0x01, 0x30, 0x30, "40k, 70k and every 80k" },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },
	
	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x03, "2"                      },
	{0x14, 0x01, 0x03, 0x00, "3"                      },
	{0x14, 0x01, 0x03, 0x01, "4"                      },
	{0x14, 0x01, 0x03, 0x02, "5"                      },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x14, 0x01, 0x80, 0x00, "Game"                   },
	{0x14, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Ghoulsu)

static struct BurnDIPInfo DaimakaiDIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x12, 0x01, 0xc0, 0x00, "Upright 1 Player"       },
	{0x12, 0x01, 0xc0, 0x40, "Upright 2 Players"      },
	{0x12, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x13)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x30, 0x10, "10k, 30k and every 30k" },
	{0x13, 0x01, 0x30, 0x20, "20k, 50k and every 70k" },
	{0x13, 0x01, 0x30, 0x00, "30k, 60k and every 70k" },
	{0x13, 0x01, 0x30, 0x30, "40k, 70k and every 80k" },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x00, "3"                      },
	{0x14, 0x01, 0x03, 0x01, "4"                      },
	{0x14, 0x01, 0x03, 0x02, "5"                      },
	{0x14, 0x01, 0x03, 0x03, "6"                      },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x14, 0x01, 0x80, 0x00, "Game"                   },
	{0x14, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Daimakai)

static struct BurnDIPInfo KnightsDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Enemys Attack Frequency"},
	{0x1c, 0x01, 0x07, 0x00, "1 (Easiest)"            },
	{0x1c, 0x01, 0x07, 0x01, "2"                      },
	{0x1c, 0x01, 0x07, 0x02, "3"                      },
	{0x1c, 0x01, 0x07, 0x03, "4 (Normal)"             },
	{0x1c, 0x01, 0x07, 0x04, "5"                      },
	{0x1c, 0x01, 0x07, 0x05, "6"                      },
	{0x1c, 0x01, 0x07, 0x06, "7"                      },
	{0x1c, 0x01, 0x07, 0x07, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 8   , "Enemys Attack Power"    },
	{0x1c, 0x01, 0x38, 0x38, "1 (Easiest)"            },
	{0x1c, 0x01, 0x38, 0x30, "2"                      },
	{0x1c, 0x01, 0x38, 0x28, "3"                      },
	{0x1c, 0x01, 0x38, 0x00, "4 (Normal)"             },
	{0x1c, 0x01, 0x38, 0x08, "5"                      },
	{0x1c, 0x01, 0x38, 0x10, "6"                      },
	{0x1c, 0x01, 0x38, 0x18, "7"                      },
	{0x1c, 0x01, 0x38, 0x20, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1c, 0x01, 0x40, 0x40, "1"                      },
	{0x1c, 0x01, 0x40, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1c, 0x01, 0x80, 0x80, "2P"                     },
	{0x1c, 0x01, 0x80, 0x00, "3P"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x1d, 0x01, 0x03, 0x03, "1"                      },
	{0x1d, 0x01, 0x03, 0x00, "2"                      },
	{0x1d, 0x01, 0x03, 0x01, "3"                      },
	{0x1d, 0x01, 0x03, 0x02, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Knights)

static struct BurnDIPInfo KodDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x1b)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1b, 0x01, 0x08, 0x08, "1"                      },
	{0x1b, 0x01, 0x08, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1b, 0x01, 0x10, 0x10, "2 Players"              },
	{0x1b, 0x01, 0x10, 0x00, "3 Players"              },

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1c, 0x01, 0x38, 0x08, "1"                      },
	{0x1c, 0x01, 0x38, 0x00, "2"                      },
	{0x1c, 0x01, 0x38, 0x10, "3"                      },
	{0x1c, 0x01, 0x38, 0x18, "4"                      },
	{0x1c, 0x01, 0x38, 0x20, "5"                      },
	{0x1c, 0x01, 0x38, 0x28, "6"                      },
	{0x1c, 0x01, 0x38, 0x30, "7"                      },
	{0x1c, 0x01, 0x38, 0x38, "8"                      },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x1c, 0x01, 0xc0, 0x40, "80k and every 400k"     },
	{0x1c, 0x01, 0xc0, 0x80, "160k and every 450k"    },
	{0x1c, 0x01, 0xc0, 0x00, "200k and every 450k"    },
	{0x1c, 0x01, 0xc0, 0xc0, "None"                   },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Kod)

static struct BurnDIPInfo Kodr1DIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x1b)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1b, 0x01, 0x08, 0x08, "1"                      },
	{0x1b, 0x01, 0x08, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1b, 0x01, 0x10, 0x10, "2 Players"              },
	{0x1b, 0x01, 0x10, 0x00, "3 Players"              },

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1c, 0x01, 0x38, 0x08, "1"                      },
	{0x1c, 0x01, 0x38, 0x00, "2"                      },
	{0x1c, 0x01, 0x38, 0x10, "3"                      },
	{0x1c, 0x01, 0x38, 0x18, "4"                      },
	{0x1c, 0x01, 0x38, 0x20, "5"                      },
	{0x1c, 0x01, 0x38, 0x28, "6"                      },
	{0x1c, 0x01, 0x38, 0x30, "7"                      },
	{0x1c, 0x01, 0x38, 0x38, "8"                      },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x1c, 0x01, 0xc0, 0x40, "80k and every 400k"     },
	{0x1c, 0x01, 0xc0, 0x00, "160k and every 450k"    },
	{0x1c, 0x01, 0xc0, 0x80, "200k and every 450k"    },
	{0x1c, 0x01, 0xc0, 0xc0, "None"                   },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Kodr1)

static struct BurnDIPInfo MegamanDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x01, NULL                     },
	{0x17, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	{0   , 0xfe, 0   , 19  , "Coinage"                },
	{0x15, 0x01, 0x1f, 0x00, "1 Coin  1 Credit"       },
	{0x15, 0x01, 0x1f, 0x01, "1 Coin  2 Credits"      },
	{0x15, 0x01, 0x1f, 0x02, "1 Coin  3 Credits"      },
	{0x15, 0x01, 0x1f, 0x03, "1 Coin  4 Credits"      },
	{0x15, 0x01, 0x1f, 0x04, "1 Coin  5 Credits"      },
	{0x15, 0x01, 0x1f, 0x05, "1 Coin  6 Credits"      },
	{0x15, 0x01, 0x1f, 0x06, "1 Coin  7 Credits"      },
	{0x15, 0x01, 0x1f, 0x07, "1 Coin  8 Credits"      },
	{0x15, 0x01, 0x1f, 0x08, "1 Coin  9 Credits"      },
	{0x15, 0x01, 0x1f, 0x09, "2 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0a, "3 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0b, "4 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0c, "5 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0d, "6 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0e, "7 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0f, "8 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x10, "9 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x11, "2 Coins Start 1 Coin Continue"},
	{0x15, 0x01, 0x1f, 0x12, "Free Play"              },

	{0   , 0xfe, 0   , 4   , "Coin Slots"             },
	{0x15, 0x01, 0x60, 0x40, "1, Common"              },
	{0x15, 0x01, 0x60, 0x00, "2, Common"              },
	{0x15, 0x01, 0x60, 0x20, "2, Individual"          },

	// Dip B
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x16, 0x01, 0x03, 0x00, "Easy"                   },
	{0x16, 0x01, 0x03, 0x01, "Normal"                 },
	{0x16, 0x01, 0x03, 0x02, "Hard"                   },
	{0x16, 0x01, 0x03, 0x03, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Time"                   },
	{0x16, 0x01, 0x0c, 0x00, "100"                    },
	{0x16, 0x01, 0x0c, 0x04, "90"                     },
	{0x16, 0x01, 0x0c, 0x08, "70"                     },
	{0x16, 0x01, 0x0c, 0x0c, "60"                     },

	{0   , 0xfe, 0   , 2   , "Voice"                  },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },
	
	// Dip C
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x17, 0x01, 0x01, 0x00, "Off"                    },
	{0x17, 0x01, 0x01, 0x01, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x02, 0x02, "Off"                    },
	{0x17, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x04, 0x04, "Off"                    },
	{0x17, 0x01, 0x04, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },	
};

STDDIPINFO(Megaman)

static struct BurnDIPInfo RockmanjDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x01, NULL                     },
	{0x17, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	{0   , 0xfe, 0   , 19  , "Coinage"                },
	{0x15, 0x01, 0x1f, 0x00, "1 Coin  1 Credit"       },
	{0x15, 0x01, 0x1f, 0x01, "1 Coin  2 Credits"      },
	{0x15, 0x01, 0x1f, 0x02, "1 Coin  3 Credits"      },
	{0x15, 0x01, 0x1f, 0x03, "1 Coin  4 Credits"      },
	{0x15, 0x01, 0x1f, 0x04, "1 Coin  5 Credits"      },
	{0x15, 0x01, 0x1f, 0x05, "1 Coin  6 Credits"      },
	{0x15, 0x01, 0x1f, 0x06, "1 Coin  7 Credits"      },
	{0x15, 0x01, 0x1f, 0x07, "1 Coin  8 Credits"      },
	{0x15, 0x01, 0x1f, 0x08, "1 Coin  9 Credits"      },
	{0x15, 0x01, 0x1f, 0x09, "2 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0a, "3 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0b, "4 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0c, "5 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0d, "6 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0e, "7 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0f, "8 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x10, "9 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x11, "2 Coins Start 1 Coin Continue"},
	{0x15, 0x01, 0x1f, 0x12, "Free Play"              },

	{0   , 0xfe, 0   , 4   , "Coin Slots"             },
	{0x15, 0x01, 0x60, 0x40, "1, Common"              },
	{0x15, 0x01, 0x60, 0x00, "2, Common"              },
	{0x15, 0x01, 0x60, 0x20, "2, Individual"          },

	// Dip B
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x16, 0x01, 0x03, 0x00, "Easy"                   },
	{0x16, 0x01, 0x03, 0x01, "Normal"                 },
	{0x16, 0x01, 0x03, 0x02, "Hard"                   },
	{0x16, 0x01, 0x03, 0x03, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Time"                   },
	{0x16, 0x01, 0x0c, 0x00, "100"                    },
	{0x16, 0x01, 0x0c, 0x04, "90"                     },
	{0x16, 0x01, 0x0c, 0x08, "70"                     },
	{0x16, 0x01, 0x0c, 0x0c, "60"                     },
	
	// Dip C
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x17, 0x01, 0x01, 0x00, "Off"                    },
	{0x17, 0x01, 0x01, 0x01, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x02, 0x02, "Off"                    },
	{0x17, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x04, 0x04, "Off"                    },
	{0x17, 0x01, 0x04, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },	
};

STDDIPINFO(Rockmanj)

static struct BurnDIPInfo MercsDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1c, 0x01, 0x08, 0x08, "1"                      },
	{0x1c, 0x01, 0x08, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1c, 0x01, 0x10, 0x10, "2 Player"               },
	{0x1c, 0x01, 0x10, 0x00, "3 Player"               },
	
	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x1d, 0x01, 0x80, 0x00, "Off"                    },
	{0x1d, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(Mercs)

static struct BurnDIPInfo MswordDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Players vitality consumption"},
	{0x16, 0x01, 0x07, 0x00, "1 (Easiest)"            },
	{0x16, 0x01, 0x07, 0x01, "2"                      },
	{0x16, 0x01, 0x07, 0x02, "3"                      },
	{0x16, 0x01, 0x07, 0x03, "4 (Normal)"             },
	{0x16, 0x01, 0x07, 0x04, "5"                      },
	{0x16, 0x01, 0x07, 0x05, "6"                      },
	{0x16, 0x01, 0x07, 0x06, "7"                      },
	{0x16, 0x01, 0x07, 0x07, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 8   , "Level 2"                },
	{0x16, 0x01, 0x38, 0x18, "1 (Easiest)"            },
	{0x16, 0x01, 0x38, 0x10, "2"                      },
	{0x16, 0x01, 0x38, 0x08, "3"                      },
	{0x16, 0x01, 0x38, 0x00, "4 (Normal)"             },
	{0x16, 0x01, 0x38, 0x20, "5"                      },
	{0x16, 0x01, 0x38, 0x28, "6"                      },
	{0x16, 0x01, 0x38, 0x30, "7"                      },
	{0x16, 0x01, 0x38, 0x38, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 2   , "Stage Select"           },
	{0x16, 0x01, 0x40, 0x00, "Off"                    },
	{0x16, 0x01, 0x40, 0x40, "On"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Vitality"               },
	{0x17, 0x01, 0x03, 0x03, "1"                      },
	{0x17, 0x01, 0x03, 0x00, "2"                      },
	{0x17, 0x01, 0x03, 0x01, "3 (2 when continue)"    },
	{0x17, 0x01, 0x03, 0x02, "4 (3 when continue)"    },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Msword)

static struct BurnDIPInfo MtwinsDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x15)

	// Dip B
	CPS1_DIFFICULTY_1(0x16)

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x38, 0x28, "1"                      },
	{0x16, 0x01, 0x38, 0x20, "2"                      },
	{0x16, 0x01, 0x38, 0x30, "3"                      },
	{0x16, 0x01, 0x38, 0x38, "4"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Mtwins)

static struct BurnDIPInfo NemoDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x16)

	{0   , 0xfe, 0   , 3   , "Life Bar"               },
	{0x16, 0x01, 0x18, 0x18, "Minimum"                },
	{0x16, 0x01, 0x18, 0x00, "Medium"                 },
	{0x16, 0x01, 0x18, 0x10, "Maximum"                },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x17, 0x01, 0x03, 0x01, "1"                      },
	{0x17, 0x01, 0x03, 0x00, "2"                      },
	{0x17, 0x01, 0x03, 0x02, "3"                      },
	{0x17, 0x01, 0x03, 0x03, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Nemo)

static struct BurnDIPInfo Pang3DIPList[]=
{
	// Defaults
	{0x13, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x08, 0x00, "Off"                    },
	{0x13, 0x01, 0x08, 0x08, "On"                     },
};

STDDIPINFO(Pang3)

static struct BurnDIPInfo PnickjDIPList[]=
{
	// Defaults
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x03, NULL                     },
	{0x15, 0xff, 0xff, 0x20, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x13)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x13, 0x01, 0x08, 0x00, "1"                      },
	{0x13, 0x01, 0x08, 0x08, "2"                      },
	
	// Dip B
	CPS1_DIFFICULTY_1(0x14)

	{0   , 0xfe, 0   , 4   , "Vs. Play Mode"          },
	{0x14, 0x01, 0xc0, 0x00, "1 Game  Match"          },
	{0x14, 0x01, 0xc0, 0x40, "3 Games Match"          },
	{0x14, 0x01, 0xc0, 0x80, "5 Games Match"          },
	{0x14, 0x01, 0xc0, 0xc0, "7 Games Match"          },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x15, 0x01, 0x08, 0x00, "Off"                    },
	{0x15, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x15, 0x01, 0x10, 0x00, "Off"                    },
	{0x15, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x15, 0x01, 0x20, 0x00, "Off"                    },
	{0x15, 0x01, 0x20, 0x20, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x15, 0x01, 0x80, 0x00, "Game"                   },
	{0x15, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Pnickj)

static struct BurnDIPInfo PunisherDIPList[]=
{
	// Defaults
	{0x13, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x08, 0x00, "Off"                    },
	{0x13, 0x01, 0x08, 0x08, "On"                     },
};

STDDIPINFO(Punisher)

static struct BurnDIPInfo PunisherbzDIPList[]=
{
	// Defaults
 	{0x15, 0xff, 0xff, 0x10, NULL                     },
 	{0x16, 0xff, 0xff, 0x6B, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x08, 0x00, "Off"                    },
	{0x15, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x15, 0x01, 0x30, 0x00, "1"                      },
	{0x15, 0x01, 0x30, 0x10, "2"                      },
	{0x15, 0x01, 0x30, 0x20, "3"                      },
	{0x15, 0x01, 0x30, 0x30, "4"                      },

	{0   , 0xfe, 0   , 2   , "Sound"                  },
	{0x15, 0x01, 0x40, 0x00, "Q Sound"                },
	{0x15, 0x01, 0x40, 0x40, "Monaural"               },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x15, 0x01, 0x80, 0x00, "Off"                    },
	{0x15, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x16, 0x01, 0x07, 0x00, "0 (Easiest)"            },
	{0x16, 0x01, 0x07, 0x01, "1"                      },
	{0x16, 0x01, 0x07, 0x02, "2"                      },
	{0x16, 0x01, 0x07, 0x03, "3 (Normal)"             },
	{0x16, 0x01, 0x07, 0x04, "4"                      },
	{0x16, 0x01, 0x07, 0x05, "5"                      },
	{0x16, 0x01, 0x07, 0x06, "6"                      },
	{0x16, 0x01, 0x07, 0x07, "7 (Hardest)"            },

	{0   , 0xfe, 0   , 4   , "Extend"                 },
	{0x16, 0x01, 0x18, 0x00, "800000"                 },
	{0x16, 0x01, 0x18, 0x08, "1800000"                },
	{0x16, 0x01, 0x18, 0x10, "2800000"                },
	{0x16, 0x01, 0x18, 0x18, "No Extend"              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x20, 0x00, "Off"                    },
	{0x16, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x16, 0x01, 0x40, 0x00, "Off"                    },
	{0x16, 0x01, 0x40, 0x40, "On"                     },
};

STDDIPINFO(Punisherbz)

static struct BurnDIPInfo QadDIPList[]=
{
	// Defaults
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x0b, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x0f)
	
	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x0f, 0x01, 0x40, 0x00, "Off"                    },
	{0x0f, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 5   , "Difficulty"             },
	{0x10, 0x01, 0x07, 0x01, "Easiest"                },
	{0x10, 0x01, 0x07, 0x02, "Easy"                   },
	{0x10, 0x01, 0x07, 0x03, "Normal"                 },
	{0x10, 0x01, 0x07, 0x04, "Hard"                   },
	{0x10, 0x01, 0x07, 0x05, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Wisdom of Enemy"        },
	{0x10, 0x01, 0x18, 0x00, "Easy"                   },
	{0x10, 0x01, 0x18, 0x08, "Normal"                 },
	{0x10, 0x01, 0x18, 0x10, "Hard"                   },
	{0x10, 0x01, 0x18, 0x18, "Hardest"                },

	{0   , 0xfe, 0   , 5   , "Lives"                  },
	{0x10, 0x01, 0xe0, 0x80, "1"                      },
	{0x10, 0x01, 0xe0, 0x60, "2"                      },
	{0x10, 0x01, 0xe0, 0x40, "3"                      },
	{0x10, 0x01, 0xe0, 0x20, "4"                      },
	{0x10, 0x01, 0xe0, 0x00, "5"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x04, 0x00, "Off"                    },
	{0x11, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x08, 0x00, "Off"                    },
	{0x11, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x11, 0x01, 0x20, 0x20, "Off"                    },
	{0x11, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x11, 0x01, 0x40, 0x40, "Off"                    },
	{0x11, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x11, 0x01, 0x80, 0x00, "Game"                   },
	{0x11, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Qad)

static struct BurnDIPInfo QadjDIPList[]=
{
	// Defaults
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x20, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x0f)
	
	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x0f, 0x01, 0x40, 0x00, "Off"                    },
	{0x0f, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 5   , "Difficulty"             },
	{0x10, 0x01, 0x07, 0x00, "0"                      },
	{0x10, 0x01, 0x07, 0x01, "1"                      },
	{0x10, 0x01, 0x07, 0x02, "2"                      },
	{0x10, 0x01, 0x07, 0x03, "3"                      },
	{0x10, 0x01, 0x07, 0x04, "4"                      },

	{0   , 0xfe, 0   , 3   , "Lives"                  },
	{0x10, 0x01, 0xe0, 0x40, "1"                      },
	{0x10, 0x01, 0xe0, 0x20, "2"                      },
	{0x10, 0x01, 0xe0, 0x00, "3"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x04, 0x00, "Off"                    },
	{0x11, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x08, 0x00, "Off"                    },
	{0x11, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x11, 0x01, 0x20, 0x00, "Off"                    },
	{0x11, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x11, 0x01, 0x40, 0x40, "Off"                    },
	{0x11, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x11, 0x01, 0x80, 0x00, "Game"                   },
	{0x11, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Qadj)

static struct BurnDIPInfo Qtono2jDIPList[]=
{
	// Defaults
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x03, NULL                     },
	{0x11, 0xff, 0xff, 0x20, NULL                     },
	
	// Dip A
	CPS1_COINAGE_2(0x0f)
	
	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x0f, 0x01, 0x40, 0x00, "Off"                    },
	{0x0f, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x10)

	{0   , 0xfe, 0   , 5   , "Lives"                  },
	{0x10, 0x01, 0xe0, 0x80, "1"                      },
	{0x10, 0x01, 0xe0, 0x60, "2"                      },
	{0x10, 0x01, 0xe0, 0x00, "3"                      },
	{0x10, 0x01, 0xe0, 0x40, "4"                      },
	{0x10, 0x01, 0xe0, 0x20, "5"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Infinite Lives"         },
	{0x11, 0x01, 0x02, 0x00, "Off"                    },
	{0x11, 0x01, 0x02, 0x02, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x04, 0x00, "Off"                    },
	{0x11, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x08, 0x00, "Off"                    },
	{0x11, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x11, 0x01, 0x20, 0x00, "Off"                    },
	{0x11, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x11, 0x01, 0x40, 0x40, "Off"                    },
	{0x11, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x11, 0x01, 0x80, 0x00, "Game"                   },
	{0x11, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Qtono2j)

static struct BurnDIPInfo Sf2DIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
        {0x1d, 0x01, 0x04, 0x00, "Off"                    },
        {0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
        {0x1d, 0x01, 0x08, 0x00, "Off"                    },
        {0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
        {0x1d, 0x01, 0x10, 0x00, "Off"                    },
        {0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
        {0x1d, 0x01, 0x20, 0x00, "Off"                    },
        {0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
        {0x1d, 0x01, 0x40, 0x00, "Off"                    },
        {0x1d, 0x01, 0x40, 0x40, "On"                     },
        
        {0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Sf2)

static struct BurnDIPInfo Sf2jDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 2   , "2 Players Game"         },
	{0x1c, 0x01, 0x08, 0x00, "1 Credit/No Continue"   },
	{0x1c, 0x01, 0x08, 0x08, "2 Credits/Winner Continue"},

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
        {0x1d, 0x01, 0x04, 0x00, "Off"                    },
        {0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
        {0x1d, 0x01, 0x08, 0x00, "Off"                    },
        {0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
        {0x1d, 0x01, 0x10, 0x00, "Off"                    },
        {0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
        {0x1d, 0x01, 0x20, 0x00, "Off"                    },
        {0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
        {0x1d, 0x01, 0x40, 0x00, "Off"                    },
        {0x1d, 0x01, 0x40, 0x40, "On"                     },
        
        {0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Sf2j)

static struct BurnDIPInfo SlammastDIPList[]=
{
	// Defaults
	{0x23, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x23, 0x01, 0x08, 0x08, "Off"                    },
	{0x23, 0x01, 0x08, 0x00, "On"                     },
};

STDDIPINFO(Slammast)

static struct BurnDIPInfo StriderDIPList[]=
{
	// Defaults
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x14)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x14, 0x01, 0xc0, 0x00, "Upright 1P"             },
	{0x14, 0x01, 0xc0, 0x40, "Upright 2P"             },
	{0x14, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x15)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x30, 0x00, "20k, 40k and every 60k" },
	{0x15, 0x01, 0x30, 0x10, "30k, 50k and every 70k" },
	{0x15, 0x01, 0x30, 0x20, "20k & 60k only"         },
	{0x15, 0x01, 0x30, 0x30, "30k & 60k only"         },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x03, 0x03, "2"                      },
	{0x16, 0x01, 0x03, 0x00, "3"                      },
	{0x16, 0x01, 0x03, 0x01, "4"                      },
	{0x16, 0x01, 0x03, 0x02, "5"                      },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x16, 0x01, 0x04, 0x00, "Off"                    },
	{0x16, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x16, 0x01, 0x08, 0x00, "Off"                    },
	{0x16, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x16, 0x01, 0x10, 0x00, "Off"                    },
	{0x16, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Sound"                  },
	{0x16, 0x01, 0x20, 0x20, "Off"                    },
	{0x16, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x16, 0x01, 0x80, 0x00, "Game"                   },
	{0x16, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Strider)

static struct BurnDIPInfo StrideruaDIPList[]=
{
	// Defaults
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x14)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x14, 0x01, 0xc0, 0x00, "Upright 1P"             },
	{0x14, 0x01, 0xc0, 0x40, "Upright 2P"             },
	{0x14, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x15)
	
	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x08, 0x00, "Off"                    },
	{0x15, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x30, 0x00, "20k, 40k and every 60k" },
	{0x15, 0x01, 0x30, 0x10, "30k, 50k and every 70k" },
	{0x15, 0x01, 0x30, 0x20, "20k & 60k only"         },
	{0x15, 0x01, 0x30, 0x30, "30k & 60k only"         },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x03, 0x03, "2"                      },
	{0x16, 0x01, 0x03, 0x00, "3"                      },
	{0x16, 0x01, 0x03, 0x01, "4"                      },
	{0x16, 0x01, 0x03, 0x02, "5"                      },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x16, 0x01, 0x04, 0x00, "Off"                    },
	{0x16, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x16, 0x01, 0x08, 0x00, "Off"                    },
	{0x16, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x16, 0x01, 0x10, 0x00, "Off"                    },
	{0x16, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Sound"                  },
	{0x16, 0x01, 0x20, 0x20, "Off"                    },
	{0x16, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x16, 0x01, 0x80, 0x00, "Game"                   },
	{0x16, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Striderua)

static struct BurnDIPInfo UnsquadDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
        CPS1_COINAGE_3(0x15)

	// Dip B
	CPS1_DIFFICULTY_1(0x16)
	
	{0   , 0xfe, 0   , 4   , "Damage"                 },
	{0x16, 0x01, 0x18, 0x08, "Small"                  },
	{0x16, 0x01, 0x18, 0x00, "Normal"                 },
	{0x16, 0x01, 0x18, 0x10, "Big"                    },
	{0x16, 0x01, 0x18, 0x18, "Biggest"                },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Unsquad)

static struct BurnDIPInfo VarthDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x0b, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },
	
	// Dip A
	CPS1_COINAGE_1(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x16)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x16, 0x01, 0x18, 0x00, "600k and every 1400k"   },
	{0x16, 0x01, 0x18, 0x08, "600k, 2000k and every 4500k"},
	{0x16, 0x01, 0x18, 0x10, "1200k, 3500k"           },
	{0x16, 0x01, 0x18, 0x18, "2000k only"             },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x17, 0x01, 0x03, 0x01, "1"                      },
	{0x17, 0x01, 0x03, 0x02, "2"                      },
	{0x17, 0x01, 0x03, 0x00, "3"                      },
	{0x17, 0x01, 0x03, 0x03, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Varth)

static struct BurnDIPInfo WillowDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },
	{0x17, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
        CPS1_COINAGE_3(0x15)

        {0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x15, 0x01, 0xc0, 0x00, "Upright 1P"             },
	{0x15, 0x01, 0xc0, 0x40, "Upright 2P"             },
	{0x15, 0x01, 0xc0, 0xc0, "Cocktail"               },
	
	// Dip B
	CPS1_DIFFICULTY_2(0x16)

	{0   , 0xfe, 0   , 4   , "Nando Speed"            },
	{0x16, 0x01, 0x18, 0x08, "Slow"                   },
	{0x16, 0x01, 0x18, 0x00, "Normal"                 },	
	{0x16, 0x01, 0x18, 0x10, "Fast"                   },
	{0x16, 0x01, 0x18, 0x18, "Very Fast"              },

	{0   , 0xfe, 0   , 2   , "Stage Magic Continue"   },
	{0x16, 0x01, 0x80, 0x00, "Off"                    },
	{0x16, 0x01, 0x80, 0x80, "On"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x17, 0x01, 0x03, 0x01, "1"                      },
	{0x17, 0x01, 0x03, 0x00, "2"                      },
	{0x17, 0x01, 0x03, 0x02, "3"                      },
	{0x17, 0x01, 0x03, 0x03, "4"                      },

	{0   , 0xfe, 0   , 4   , "Vitality"               },
	{0x17, 0x01, 0x0c, 0x0c, "2"                      },
	{0x17, 0x01, 0x0c, 0x00, "3"                      },
	{0x17, 0x01, 0x0c, 0x04, "4"                      },
	{0x17, 0x01, 0x0c, 0x08, "5"                      },

	{0   , 0xfe, 0   , 2   , "Screen"                 },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x17, 0x01, 0x20, 0x20, "Off"                    },
	{0x17, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x40, "Off"                    },
	{0x17, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(Willow)

static struct BurnDIPInfo WofDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1b, 0x01, 0x08, 0x08, "Off"                    },
	{0x1b, 0x01, 0x08, 0x00, "On"                     },
};

STDDIPINFO(Wof)

static struct BurnDIPInfo WofhfhDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x13, NULL                     },
	{0x1d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x1b, 0x01, 0x03, 0x00, "1 Coin 1 Credit"        },
	{0x1b, 0x01, 0x03, 0x01, "1 Coin 2 Credits"       },
	{0x1b, 0x01, 0x03, 0x02, "1 Coin 3 Credits"       },
	{0x1b, 0x01, 0x03, 0x03, "1 Coin 4 Credits"       },
	
	// Dip B
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x1c, 0x01, 0x07, 0x00, "Extra Easy"             },
	{0x1c, 0x01, 0x07, 0x01, "Very Easy"              },
	{0x1c, 0x01, 0x07, 0x02, "Easy"                   },
	{0x1c, 0x01, 0x07, 0x03, "Normal"                 },
	{0x1c, 0x01, 0x07, 0x04, "Hard"                   },
	{0x1c, 0x01, 0x07, 0x05, "Very Hard"              },
	{0x1c, 0x01, 0x07, 0x06, "Extra Hard"             },
	{0x1c, 0x01, 0x07, 0x07, "Hardest"                },
	
	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1c, 0x01, 0x70, 0x70, "Start 4 Continue 5"     },
	{0x1c, 0x01, 0x70, 0x60, "Start 3 Continue 4"     },
	{0x1c, 0x01, 0x70, 0x50, "Start 2 Continue 3"     },
	{0x1c, 0x01, 0x70, 0x40, "Start 1 Continue 2"     },
	{0x1c, 0x01, 0x70, 0x30, "Start 4 Continue 4"     },
	{0x1c, 0x01, 0x70, 0x20, "Start 3 Continue 3"     },
	{0x1c, 0x01, 0x70, 0x10, "Start 2 Continue 2"     },
	{0x1c, 0x01, 0x70, 0x00, "Start 1 Continue 1"     },
	
	// Dip C
	{0   , 0xfe, 0   , 3   , "Coin Slots"             },
	{0x1d, 0x01, 0x03, 0x02, "2 Players 1 Shooter"    },
	{0x1d, 0x01, 0x03, 0x01, "3 Players 1 Shooter"    },
	{0x1d, 0x01, 0x03, 0x00, "3 Players 3 Shooters"   },
};

STDDIPINFO(Wofhfh)

static struct BurnDIPInfo WofhDIPList[]=
{
	// Defaults
	{0x1a, 0xff, 0xff, 0x00, NULL                     },
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x13, NULL                     },
	
	// Dip A
	{0   , 0xfe, 0   , 3   , "Coin Slots"             },
	{0x1a, 0x01, 0x03, 0x02, "2 Players 1 Shooter"    },
	{0x1a, 0x01, 0x03, 0x01, "3 Players 1 Shooter"    },
	{0x1a, 0x01, 0x03, 0x00, "3 Players 3 Shooters"   },
	
	// Dip B
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x1b, 0x01, 0x03, 0x00, "1 Coin 1 Credit"        },
	{0x1b, 0x01, 0x03, 0x01, "1 Coin 2 Credits"       },
	{0x1b, 0x01, 0x03, 0x02, "1 Coin 3 Credits"       },
	{0x1b, 0x01, 0x03, 0x03, "1 Coin 4 Credits"       },
	
	// Dip C
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x1c, 0x01, 0x07, 0x00, "Extra Easy"             },
	{0x1c, 0x01, 0x07, 0x01, "Very Easy"              },
	{0x1c, 0x01, 0x07, 0x02, "Easy"                   },
	{0x1c, 0x01, 0x07, 0x03, "Normal"                 },
	{0x1c, 0x01, 0x07, 0x04, "Hard"                   },
	{0x1c, 0x01, 0x07, 0x05, "Very Hard"              },
	{0x1c, 0x01, 0x07, 0x06, "Extra Hard"             },
	{0x1c, 0x01, 0x07, 0x07, "Hardest"                },
	
	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1c, 0x01, 0x70, 0x70, "Start 4 Continue 5"     },
	{0x1c, 0x01, 0x70, 0x60, "Start 3 Continue 4"     },
	{0x1c, 0x01, 0x70, 0x50, "Start 2 Continue 3"     },
	{0x1c, 0x01, 0x70, 0x40, "Start 1 Continue 2"     },
	{0x1c, 0x01, 0x70, 0x30, "Start 4 Continue 4"     },
	{0x1c, 0x01, 0x70, 0x20, "Start 3 Continue 3"     },
	{0x1c, 0x01, 0x70, 0x10, "Start 2 Continue 2"     },
	{0x1c, 0x01, 0x70, 0x00, "Start 1 Continue 1"     },
};

STDDIPINFO(Wofh)

static struct BurnDIPInfo Wof3jsDIPList[]=
{
	// Defaults
	{0x1a, 0xff, 0xff, 0x00, NULL                     },
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x1a, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },
	{0x1a, 0x01, 0x03, 0x00, "1 Coin  1 Credit"       },	
	{0x1a, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	{0x1a, 0x01, 0x03, 0x03, "1 Coin  3 Credits"      },
	
	// Dip B
	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1b, 0x01, 0x01, 0x01, "3 Players 1 Shooter"    },
	{0x1b, 0x01, 0x01, 0x00, "3 Players 3 Shooters"   },
	
	// Dip C
	// not sure about difficulty (no way to test)
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x1c, 0x01, 0x07, 0x00, "Value 0"                },
	{0x1c, 0x01, 0x07, 0x01, "Value 1"                },
	{0x1c, 0x01, 0x07, 0x02, "Value 2"                },
	{0x1c, 0x01, 0x07, 0x03, "Value 3"                },
	{0x1c, 0x01, 0x07, 0x04, "Value 4"                },
	{0x1c, 0x01, 0x07, 0x05, "Value 5"                },
	{0x1c, 0x01, 0x07, 0x06, "Value 6"                },
	{0x1c, 0x01, 0x07, 0x07, "Value 7"                },
	
	{0   , 0xfe, 0   , 5   , "Lives"                  },
	{0x1c, 0x01, 0x70, 0x40, "Start 1 Continue 1"     },
	{0x1c, 0x01, 0x70, 0x30, "Start 4 Continue 3"     },
	{0x1c, 0x01, 0x70, 0x20, "Start 4 Continue 4"     },
	{0x1c, 0x01, 0x70, 0x10, "Start 3 Continue 3"     },
	{0x1c, 0x01, 0x70, 0x00, "Start 2 Continue 2"     },
};

STDDIPINFO(Wof3js)

static struct BurnDIPInfo Wof3sjDIPList[]=
{
	// Defaults
	{0x1a, 0xff, 0xff, 0x00, NULL                     },
	{0x1b, 0xff, 0xff, 0x13, NULL                     },
	{0x1c, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip A
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x1a, 0x01, 0x03, 0x00, "1 Coin 1 Credit"        },
	{0x1a, 0x01, 0x03, 0x01, "1 Coin 2 Credits"       },
	{0x1a, 0x01, 0x03, 0x02, "1 Coin 3 Credits"       },
	{0x1a, 0x01, 0x03, 0x03, "1 Coin 4 Credits"       },
	
	// Dip B
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x1b, 0x01, 0x07, 0x00, "Extra Easy"             },
	{0x1b, 0x01, 0x07, 0x01, "Very Easy"              },
	{0x1b, 0x01, 0x07, 0x02, "Easy"                   },
	{0x1b, 0x01, 0x07, 0x03, "Normal"                 },
	{0x1b, 0x01, 0x07, 0x04, "Hard"                   },
	{0x1b, 0x01, 0x07, 0x05, "Very Hard"              },
	{0x1b, 0x01, 0x07, 0x06, "Extra Hard"             },
	{0x1b, 0x01, 0x07, 0x07, "Hardest"                },
	
	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1b, 0x01, 0x70, 0x70, "Start 4 Continue 5"     },
	{0x1b, 0x01, 0x70, 0x60, "Start 3 Continue 4"     },
	{0x1b, 0x01, 0x70, 0x50, "Start 2 Continue 3"     },
	{0x1b, 0x01, 0x70, 0x40, "Start 1 Continue 2"     },
	{0x1b, 0x01, 0x70, 0x30, "Start 4 Continue 4"     },
	{0x1b, 0x01, 0x70, 0x20, "Start 3 Continue 3"     },
	{0x1b, 0x01, 0x70, 0x10, "Start 2 Continue 2"     },
	{0x1b, 0x01, 0x70, 0x00, "Start 1 Continue 1"     },
	
	// Dip C
	{0   , 0xfe, 0   , 3   , "Coin Slots"             },
	{0x1c, 0x01, 0x03, 0x02, "2 Players 1 Shooter"    },
	{0x1c, 0x01, 0x03, 0x01, "3 Players 1 Shooter"    },
	{0x1c, 0x01, 0x03, 0x00, "3 Players 3 Shooters"   },
};

STDDIPINFO(Wof3sj)

// Rom Definitions

#define A_BOARD_PLDS						\
	{ "buf1",          0x000117, 0xeb122de7, BRF_OPT },	\
	{ "ioa1",          0x000117, 0x59c7ee3b, BRF_OPT },	\
	{ "prg1",          0x000117, 0xf1129744, BRF_OPT },	\
	{ "rom1",          0x000117, 0x41dc73b9, BRF_OPT },	\
	{ "sou1",          0x000117, 0x84f4b2fe, BRF_OPT },
	
#define A_BOARD_QSOUND_PLDS					\
	{ "buf1",          0x000117, 0xeb122de7, BRF_OPT },	\
	{ "ioa1",          0x000117, 0x59c7ee3b, BRF_OPT },	\
	{ "prg2",          0x000117, 0x4386879a, BRF_OPT },	\
	{ "rom1",          0x000117, 0x41dc73b9, BRF_OPT },	\

static struct BurnRomInfo NTFODrvRomDesc[] = {
	{ "41em_30.11f",   0x020000, 0x4249ec61, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41em_35.11h",   0x020000, 0xddbee5eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41em_31.12f",   0x020000, 0x584e88e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41em_36.12h",   0x020000, 0x3cfc31d0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41-32m.8h",     0x080000, 0x4e9648ca, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
 
	{ "41-5m.7a",      0x080000, 0x01d1cb11, BRF_GRA | CPS1_TILES },
	{ "41-7m.9a",      0x080000, 0xaeaa3509, BRF_GRA | CPS1_TILES },
	{ "41-1m.3a",      0x080000, 0xff77985a, BRF_GRA | CPS1_TILES },
	{ "41-3m.5a",      0x080000, 0x983be58f, BRF_GRA | CPS1_TILES },

	{ "41_9.12b",      0x010000, 0x0f9d8527, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "41_18.11c",     0x020000, 0xd1f15aeb, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "41_19.12c",     0x020000, 0x15aec3a6, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "yi24b.1a",      0x000117, 0x3004dcdf, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(NTFODrv)
STD_ROM_FN(NTFODrv)

static struct BurnRomInfo NTFOR1DrvRomDesc[] = {
	{ "41e_30.11f",    0x020000, 0x9deb1e75, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41e_35.11h",    0x020000, 0xd63942b3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41e_31.12f",    0x020000, 0xdf201112, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41e_36.12h",    0x020000, 0x816a818f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41-32m.8h",     0x080000, 0x4e9648ca, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
 
	{ "41-5m.7a",      0x080000, 0x01d1cb11, BRF_GRA | CPS1_TILES },
	{ "41-7m.9a",      0x080000, 0xaeaa3509, BRF_GRA | CPS1_TILES },
	{ "41-1m.3a",      0x080000, 0xff77985a, BRF_GRA | CPS1_TILES },
	{ "41-3m.5a",      0x080000, 0x983be58f, BRF_GRA | CPS1_TILES },

	{ "41_09.12b",     0x010000, 0x0f9d8527, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "41_18.11c",     0x020000, 0xd1f15aeb, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "41_19.12c",     0x020000, 0x15aec3a6, BRF_SND | CPS1_OKIM6295_SAMPLES },	
	
	A_BOARD_PLDS
	
	{ "yi24b.1a",      0x000117, 0x3004dcdf, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(NTFOR1Drv)
STD_ROM_FN(NTFOR1Drv)

static struct BurnRomInfo NTFOUDrvRomDesc[] = {
	{ "41u_30.11f",    0x020000, 0xbe5439d0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41u_35.11h",    0x020000, 0x6ac96595, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41u_31.12f",    0x020000, 0x9811d6eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41u_36.12h",    0x020000, 0xa87e6137, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41-32m.8h",     0x080000, 0x4e9648ca, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
 
	{ "41-5m.7a",      0x080000, 0x01d1cb11, BRF_GRA | CPS1_TILES },
	{ "41-7m.9a",      0x080000, 0xaeaa3509, BRF_GRA | CPS1_TILES },
	{ "41-1m.3a",      0x080000, 0xff77985a, BRF_GRA | CPS1_TILES },
	{ "41-3m.5a",      0x080000, 0x983be58f, BRF_GRA | CPS1_TILES },

	{ "41_9.12b",      0x010000, 0x0f9d8527, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "41e_18.11c",    0x020000, 0xd1f15aeb, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "41_19.12c",     0x020000, 0x15aec3a6, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "yi24b.1a",      0x000117, 0x3004dcdf, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(NTFOUDrv)
STD_ROM_FN(NTFOUDrv)

static struct BurnRomInfo NTFOJDrvRomDesc[] = {
	{ "41_36.12f",     0x020000, 0x7fbd42ab, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41_42.12h",     0x020000, 0xc7781f89, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41_37.13f",     0x020000, 0xc6464b0b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41_43.13h",     0x020000, 0x440fc0b5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41_34.10f",     0x020000, 0xb5f341ec, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41_40.10h",     0x020000, 0x3979837d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41_35.11f",     0x020000, 0x95cc979a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41_41.11h",     0x020000, 0x57496819, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "41_09.4b",      0x020000, 0xbe1b6bc2, BRF_GRA | CPS1_TILES },
	{ "41_01.4a",      0x020000, 0xd8946fc1, BRF_GRA | CPS1_TILES },
	{ "41_13.9b",      0x020000, 0x2e06d0ec, BRF_GRA | CPS1_TILES },
	{ "41_05.9a",      0x020000, 0xd8ba28e0, BRF_GRA | CPS1_TILES },
	{ "41_24.5e",      0x020000, 0x5aa43cee, BRF_GRA | CPS1_TILES },
	{ "41_17.5c",      0x020000, 0xbbeff902, BRF_GRA | CPS1_TILES },
	{ "41_38.8h",      0x020000, 0x8889c0aa, BRF_GRA | CPS1_TILES },
	{ "41_32.8f",      0x020000, 0xf0168249, BRF_GRA | CPS1_TILES },
	{ "41_10.5b",      0x020000, 0xb7eb6a6d, BRF_GRA | CPS1_TILES },
	{ "41_02.5a",      0x020000, 0x802e8153, BRF_GRA | CPS1_TILES },
	{ "41_14.10b",     0x020000, 0x5a33f676, BRF_GRA | CPS1_TILES },
	{ "41_06.10a",     0x020000, 0x4e53650b, BRF_GRA | CPS1_TILES },
	{ "41_25.7e",      0x020000, 0x94add360, BRF_GRA | CPS1_TILES },
	{ "41_18.7c",      0x020000, 0xa5e1c1f3, BRF_GRA | CPS1_TILES },
	{ "41_39.9h",      0x020000, 0x5b5c3949, BRF_GRA | CPS1_TILES },
	{ "41_33.9f",      0x020000, 0x7a31b0e2, BRF_GRA | CPS1_TILES },

	{ "41_23.13b",     0x010000, 0x0f9d8527, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "41_30.12c",     0x020000, 0xd1f15aeb, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "41_31.13c",     0x020000, 0x15aec3a6, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "yi22b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(NTFOJDrv)
STD_ROM_FN(NTFOJDrv)

static struct BurnRomInfo ThreeWondersRomDesc[] = {
	{ "rte_30a.11f",   0x020000, 0xef5b8b33, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rte_35a.11h",   0x020000, 0x7d705529, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rte_31a.12f",   0x020000, 0x32835e5e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rte_36a.12h",   0x020000, 0x7637975f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rt_28a.9f",     0x020000, 0x054137c8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rt_33a.9h",     0x020000, 0x7264cb1b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rte_29a.10f",   0x020000, 0xcddaa919, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rte_34a.10h",   0x020000, 0xed52e7e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "rt-5m.7a",      0x080000, 0x86aef804, BRF_GRA | CPS1_TILES },
	{ "rt-7m.9a",      0x080000, 0x4f057110, BRF_GRA | CPS1_TILES },
	{ "rt-1m.3a",      0x080000, 0x902489d0, BRF_GRA | CPS1_TILES },
	{ "rt-3m.5a",      0x080000, 0xe35ce720, BRF_GRA | CPS1_TILES },
	{ "rt-6m.8a",      0x080000, 0x13cb0e7c, BRF_GRA | CPS1_TILES },
	{ "rt-8m.10a",     0x080000, 0x1f055014, BRF_GRA | CPS1_TILES },
	{ "rt-2m.4a",      0x080000, 0xe9a034f4, BRF_GRA | CPS1_TILES },
	{ "rt-4m.6a",      0x080000, 0xdf0eea8b, BRF_GRA | CPS1_TILES },

	{ "rt_9.12b",      0x010000, 0xabfca165, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "rt_18.11c",     0x020000, 0x26b211ab, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "rt_19.12c",     0x020000, 0xdbe64ad0, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "rt24b.1a",      0x000117, 0x54b85159, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(ThreeWonders)
STD_ROM_FN(ThreeWonders)

static struct BurnRomInfo Wonder3uRomDesc[] = {
	{ "rtu_30a.11f",   0x020000, 0x0b156fd8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtu_35a.11h",   0x020000, 0x57350bf4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtu_31a.12f",   0x020000, 0x0e723fcc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtu_36a.12h",   0x020000, 0x523a45dc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rt_28a.9f",     0x020000, 0x054137c8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rt_33a.9h",     0x020000, 0x7264cb1b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtu_29a.10f",   0x020000, 0x37ba3e20, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtu_34a.10h",   0x020000, 0xf99f46c0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "rt-5m.7a",      0x080000, 0x86aef804, BRF_GRA | CPS1_TILES },
	{ "rt-7m.9a",      0x080000, 0x4f057110, BRF_GRA | CPS1_TILES },
	{ "rt-1m.3a",      0x080000, 0x902489d0, BRF_GRA | CPS1_TILES },
	{ "rt-3m.5a",      0x080000, 0xe35ce720, BRF_GRA | CPS1_TILES },
	{ "rt-6m.8a",      0x080000, 0x13cb0e7c, BRF_GRA | CPS1_TILES },
	{ "rt-8m.10a",     0x080000, 0x1f055014, BRF_GRA | CPS1_TILES },
	{ "rt-2m.4a",      0x080000, 0xe9a034f4, BRF_GRA | CPS1_TILES },
	{ "rt-4m.6a",      0x080000, 0xdf0eea8b, BRF_GRA | CPS1_TILES },

	{ "rt_9.12b",      0x010000, 0xabfca165, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "rt_18.11c",     0x020000, 0x26b211ab, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "rt_19.12c",     0x020000, 0xdbe64ad0, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "rt24b.1a",      0x000117, 0x54b85159, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wonder3u)
STD_ROM_FN(Wonder3u)

static struct BurnRomInfo Wonder3RomDesc[] = {
	{ "rtj_36.12f",    0x020000, 0xe3741247, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtj_42.12h",    0x020000, 0xb4baa117, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtj_37.13f",    0x020000, 0xa1f677b0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtj_43.13h",    0x020000, 0x85337a47, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rt_34.10f",     0x020000, 0x054137c8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rt_40.10h",     0x020000, 0x7264cb1b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtj_35.11f",    0x020000, 0xe72f9ea3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rtj_41.11h",    0x020000, 0xa11ee998, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "rt_09.4b",      0x020000, 0x2c40e480, BRF_GRA | CPS1_TILES },
	{ "rt_01.4a",      0x020000, 0x3e11f8cd, BRF_GRA | CPS1_TILES },
	{ "rt_13.9b",      0x020000, 0x51009117, BRF_GRA | CPS1_TILES },
	{ "rt_05.9a",      0x020000, 0x283fd470, BRF_GRA | CPS1_TILES },
	{ "rt_24.5e",      0x020000, 0xee4484ce, BRF_GRA | CPS1_TILES },
	{ "rt_17.5c",      0x020000, 0xe5dcddeb, BRF_GRA | CPS1_TILES },
	{ "rt_38.8h",      0x020000, 0xb2940c2d, BRF_GRA | CPS1_TILES },
	{ "rt_32.8f",      0x020000, 0x08e2b758, BRF_GRA | CPS1_TILES },
	{ "rt_10.5b",      0x020000, 0xe3f3ff94, BRF_GRA | CPS1_TILES },
	{ "rt_02.5a",      0x020000, 0xfcffd73c, BRF_GRA | CPS1_TILES },
	{ "rt_14.10b",     0x020000, 0x5c546d9a, BRF_GRA | CPS1_TILES },
	{ "rt_06.10a",     0x020000, 0xd9650bc4, BRF_GRA | CPS1_TILES },
	{ "rt_25.7e",      0x020000, 0x11b28831, BRF_GRA | CPS1_TILES },
	{ "rt_18.7c",      0x020000, 0xce1afb7c, BRF_GRA | CPS1_TILES },
	{ "rt_39.9h",      0x020000, 0xea7ac9ee, BRF_GRA | CPS1_TILES },
	{ "rt_33.9f",      0x020000, 0xd6a99384, BRF_GRA | CPS1_TILES },
	{ "rt_11.7b",      0x020000, 0x04f3c298, BRF_GRA | CPS1_TILES },
	{ "rt_03.7a",      0x020000, 0x98087e08, BRF_GRA | CPS1_TILES },
	{ "rt_15.11b",     0x020000, 0xb6aba565, BRF_GRA | CPS1_TILES },
	{ "rt_07.11a",     0x020000, 0xc62defa1, BRF_GRA | CPS1_TILES },
	{ "rt_26.8e",      0x020000, 0x532f542e, BRF_GRA | CPS1_TILES },
	{ "rt_19.8c",      0x020000, 0x1f0f72bd, BRF_GRA | CPS1_TILES },
	{ "rt_28.10e",     0x020000, 0x6064e499, BRF_GRA | CPS1_TILES },
	{ "rt_21.10c",     0x020000, 0x20012ddc, BRF_GRA | CPS1_TILES },
	{ "rt_12.8b",      0x020000, 0xe54664cc, BRF_GRA | CPS1_TILES },
	{ "rt_04.8a",      0x020000, 0x4d7b9a1a, BRF_GRA | CPS1_TILES },
	{ "rt_16.12b",     0x020000, 0x37c96cfc, BRF_GRA | CPS1_TILES },
	{ "rt_08.12a",     0x020000, 0x75f4975b, BRF_GRA | CPS1_TILES },
	{ "rt_27.9e",      0x020000, 0xec6edc0f, BRF_GRA | CPS1_TILES },
	{ "rt_20.9c",      0x020000, 0x4fe52659, BRF_GRA | CPS1_TILES },
	{ "rt_29.11e",     0x020000, 0x8fa77f9f, BRF_GRA | CPS1_TILES },
	{ "rt_22.11c",     0x020000, 0x228a0d4a, BRF_GRA | CPS1_TILES },	

	{ "rt_23.13b",     0x010000, 0xabfca165, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "rt_30.12c",     0x020000, 0x26b211ab, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "rt_31.13c",     0x020000, 0xdbe64ad0, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "rt22b.1a",      0x000117, 0x89560d6a, BRF_OPT },	// b-board PLDs
	{ "iob1.12e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wonder3)
STD_ROM_FN(Wonder3)

static struct BurnRomInfo Wonder3hRomDesc[] = {
	{ "22.bin",        0x020000, 0xe6071884, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "26.bin",        0x020000, 0xa28447b7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "23.bin",        0x020000, 0xfd3d6509, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "27.bin",        0x020000, 0x999cba3d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rt_28a.9f",     0x020000, 0x054137c8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rt_33a.9h",     0x020000, 0x7264cb1b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rte_29a.10f",   0x020000, 0xcddaa919, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "rte_34a.10h",   0x020000, 0xed52e7e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "16.bin",        0x040000, 0xc997bca2, BRF_GRA | CPS1_TILES },
	{ "6.bin",         0x040000, 0x3eea321a, BRF_GRA | CPS1_TILES },
	{ "18.bin",        0x040000, 0x98acdfd4, BRF_GRA | CPS1_TILES },
	{ "8.bin",         0x040000, 0xdc9ca6f9, BRF_GRA | CPS1_TILES },
	{ "12.bin",        0x040000, 0x0d8a6007, BRF_GRA | CPS1_TILES },
	{ "2.bin",         0x040000, 0xd75563b9, BRF_GRA | CPS1_TILES },
	{ "14.bin",        0x040000, 0x84369a28, BRF_GRA | CPS1_TILES },
	{ "4.bin",         0x040000, 0xd4831578, BRF_GRA | CPS1_TILES },
	{ "17.bin",        0x040000, 0x040edff5, BRF_GRA | CPS1_TILES },
	{ "7.bin",         0x040000, 0xc7c0468c, BRF_GRA | CPS1_TILES },
	{ "19.bin",        0x040000, 0x9fef114f, BRF_GRA | CPS1_TILES },
	{ "9.bin",         0x040000, 0x48cbfba5, BRF_GRA | CPS1_TILES },
	{ "13.bin",        0x040000, 0x8fc3d7d1, BRF_GRA | CPS1_TILES },
	{ "3.bin",         0x040000, 0xc65e9a86, BRF_GRA | CPS1_TILES },
	{ "15.bin",        0x040000, 0xf239341a, BRF_GRA | CPS1_TILES },
	{ "5.bin",         0x040000, 0x947ac944, BRF_GRA | CPS1_TILES },
	
	{ "rt_9.12b",      0x010000, 0xabfca165, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "rt_18.11c",     0x020000, 0x26b211ab, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "rt_19.12c",     0x020000, 0xdbe64ad0, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "rt24b.1a",      0x000117, 0x54b85159, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wonder3h)
STD_ROM_FN(Wonder3h)

static struct BurnRomInfo CaptcommRomDesc[] = {
	{ "cce_23f.8f",    0x080000, 0x42c814c5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cc_22f.7f",     0x080000, 0x0fd34195, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cc_24f.9e",     0x020000, 0x3a794f25, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cc_28f.9f",     0x020000, 0xfc3c2906, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "cc-5m.3a",      0x080000, 0x7261d8ba, BRF_GRA | CPS1_TILES },
	{ "cc-7m.5a",      0x080000, 0x6a60f949, BRF_GRA | CPS1_TILES },
	{ "cc-1m.4a",      0x080000, 0x00637302, BRF_GRA | CPS1_TILES },
	{ "cc-3m.6a",      0x080000, 0xcc87cf61, BRF_GRA | CPS1_TILES },
	{ "cc-6m.7a",      0x080000, 0x28718bed, BRF_GRA | CPS1_TILES },
	{ "cc-8m.9a",      0x080000, 0xd4acc53a, BRF_GRA | CPS1_TILES },
	{ "cc-2m.8a",      0x080000, 0x0c69f151, BRF_GRA | CPS1_TILES },
	{ "cc-4m.10a",     0x080000, 0x1f9ebb97, BRF_GRA | CPS1_TILES },

	{ "cc_09.11a",     0x010000, 0x698e8b58, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cc_18.11c",     0x020000, 0x6de2c2db, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "cc_19.12c",     0x020000, 0xb99091ae, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cc63b.1a",      0x000117, 0xcae8f0f9, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ccprg1.11d",    0x000117, 0xe1c225c4, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632b.ic1",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP},
};

STD_ROM_PICK(Captcomm)
STD_ROM_FN(Captcomm)

static struct BurnRomInfo Captcommr1RomDesc[] = {
	{ "cce_23d.8f",    0x080000, 0x19c58ece, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cc_22d.7f",     0x080000, 0xa91949b7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cc_24d.9e",     0x020000, 0x680e543f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cc_28d.9f",     0x020000, 0x8820039f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "cc-5m.3a",      0x080000, 0x7261d8ba, BRF_GRA | CPS1_TILES },
	{ "cc-7m.5a",      0x080000, 0x6a60f949, BRF_GRA | CPS1_TILES },
	{ "cc-1m.4a",      0x080000, 0x00637302, BRF_GRA | CPS1_TILES },
	{ "cc-3m.6a",      0x080000, 0xcc87cf61, BRF_GRA | CPS1_TILES },
	{ "cc-6m.7a",      0x080000, 0x28718bed, BRF_GRA | CPS1_TILES },
	{ "cc-8m.9a",      0x080000, 0xd4acc53a, BRF_GRA | CPS1_TILES },
	{ "cc-2m.8a",      0x080000, 0x0c69f151, BRF_GRA | CPS1_TILES },
	{ "cc-4m.10a",     0x080000, 0x1f9ebb97, BRF_GRA | CPS1_TILES },

	{ "cc_09.11a",     0x010000, 0x698e8b58, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cc_18.11c",     0x020000, 0x6de2c2db, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "cc_19.12c",     0x020000, 0xb99091ae, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cc63b.1a",      0x000117, 0xcae8f0f9, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ccprg.11d",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Captcommr1)
STD_ROM_FN(Captcommr1)

static struct BurnRomInfo CaptcommuRomDesc[] = {
	{ "ccu_23b.8f",    0x080000, 0x03da44fd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "ccu_22c.7f",    0x080000, 0x9b82a052, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "ccu_24b.9e",    0x020000, 0x84ff99b2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ccu_28b.9f",    0x020000, 0xfbcec223, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "cc-5m.3a",      0x080000, 0x7261d8ba, BRF_GRA | CPS1_TILES },
	{ "cc-7m.5a",      0x080000, 0x6a60f949, BRF_GRA | CPS1_TILES },
	{ "cc-1m.4a",      0x080000, 0x00637302, BRF_GRA | CPS1_TILES },
	{ "cc-3m.6a",      0x080000, 0xcc87cf61, BRF_GRA | CPS1_TILES },
	{ "cc-6m.7a",      0x080000, 0x28718bed, BRF_GRA | CPS1_TILES },
	{ "cc-8m.9a",      0x080000, 0xd4acc53a, BRF_GRA | CPS1_TILES },
	{ "cc-2m.8a",      0x080000, 0x0c69f151, BRF_GRA | CPS1_TILES },
	{ "cc-4m.10a",     0x080000, 0x1f9ebb97, BRF_GRA | CPS1_TILES },

	{ "ccu_09.11a",    0x010000, 0x698e8b58, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ccu_18.11c",    0x020000, 0x6de2c2db, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ccu_19.12c",    0x020000, 0xb99091ae, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cc63b.1a",      0x000117, 0xcae8f0f9, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ccprg1.11d",    0x000117, 0xe1c225c4, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Captcommu)
STD_ROM_FN(Captcommu)

static struct BurnRomInfo CaptcommjRomDesc[] = {
	{ "ccj_23f.8f",    0x080000, 0x5b482b62, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "ccj_22f.7f",    0x080000, 0x0fd34195, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "ccj_24f.9e",    0x020000, 0x3a794f25, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ccj_28f.9f",    0x020000, 0xfc3c2906, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "cc_01.3a",      0x080000, 0x7261d8ba, BRF_GRA | CPS1_TILES },
	{ "cc_02.4a",      0x080000, 0x6a60f949, BRF_GRA | CPS1_TILES },
	{ "cc_03.5a",      0x080000, 0x00637302, BRF_GRA | CPS1_TILES },
	{ "cc_04.6a",      0x080000, 0xcc87cf61, BRF_GRA | CPS1_TILES },
	{ "cc_05.7a",      0x080000, 0x28718bed, BRF_GRA | CPS1_TILES },
	{ "cc_06.8a",      0x080000, 0xd4acc53a, BRF_GRA | CPS1_TILES },
	{ "cc_07.9a",      0x080000, 0x0c69f151, BRF_GRA | CPS1_TILES },
	{ "cc_08.10a",     0x080000, 0x1f9ebb97, BRF_GRA | CPS1_TILES },

	{ "ccj_09.12a",    0x010000, 0x698e8b58, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ccj_18.11c",    0x020000, 0x6de2c2db, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ccj_19.12c",    0x020000, 0xb99091ae, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cc63b.1a",      0x000117, 0xcae8f0f9, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ccprg1.11d",    0x000117, 0xe1c225c4, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632b.ic1",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STD_ROM_PICK(Captcommj)
STD_ROM_FN(Captcommj)

static struct BurnRomInfo Captcommjr1RomDesc[] = {
	{ "ccj_23b.8f",    0x080000, 0xe2a2d80e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "ccj_22c.7f",    0x080000, 0x9b82a052, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "ccj_24b.9e",    0x020000, 0x84ff99b2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ccj_28b.9f",    0x020000, 0xfbcec223, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "cc_01.3a",      0x080000, 0x7261d8ba, BRF_GRA | CPS1_TILES },
	{ "cc_02.4a",      0x080000, 0x6a60f949, BRF_GRA | CPS1_TILES },
	{ "cc_03.5a",      0x080000, 0x00637302, BRF_GRA | CPS1_TILES },
	{ "cc_04.6a",      0x080000, 0xcc87cf61, BRF_GRA | CPS1_TILES },
	{ "cc_05.7a",      0x080000, 0x28718bed, BRF_GRA | CPS1_TILES },
	{ "cc_06.8a",      0x080000, 0xd4acc53a, BRF_GRA | CPS1_TILES },
	{ "cc_07.9a",      0x080000, 0x0c69f151, BRF_GRA | CPS1_TILES },
	{ "cc_08.10a",     0x080000, 0x1f9ebb97, BRF_GRA | CPS1_TILES },

	{ "ccj_09.12a",    0x010000, 0x698e8b58, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ccj_18.11c",    0x020000, 0x6de2c2db, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ccj_19.12c",    0x020000, 0xb99091ae, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cc63b.1a",      0x000117, 0xcae8f0f9, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ccprg.11d",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Captcommjr1)
STD_ROM_FN(Captcommjr1)

static struct BurnRomInfo CaptcommbRomDesc[] = {
	{ "25.bin",        0x080000, 0xcb71ed7a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "27.bin",        0x080000, 0x47cb2e87, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "24.bin",        0x040000, 0x79794279, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "26.bin",        0x040000, 0xb01077ba, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "c91e-01.bin",   0x100000, 0xf863071c, BRF_GRA | CPS1_TILES },
	{ "c91e-02.bin",   0x100000, 0x4b03c308, BRF_GRA | CPS1_TILES },
	{ "c91e-03.bin",   0x100000, 0x3383ea96, BRF_GRA | CPS1_TILES },
	{ "c91e-04.bin",   0x100000, 0xb8e1f4cf, BRF_GRA | CPS1_TILES },

	{ "l.bin",         0x010000, 0x698e8b58, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "c91e-05.bin",   0x040000, 0x096115fb, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Captcommb)
STD_ROM_FN(Captcommb)

static struct BurnRomInfo Captcommb2RomDesc[] = {
	{ "5.bin",     	   0x080000, 0xc3a6ed28, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "4.bin",         0x080000, 0x28729335, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "3.bin",         0x040000, 0x1b526d73, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "2.bin",         0x040000, 0x73c99709, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "cap.bin",       0x080000, 0x7261d8ba, BRF_GRA | CPS1_TILES },
	{ "cbp.bin",       0x080000, 0x6a60f949, BRF_GRA | CPS1_TILES },
	{ "ccp.bin",       0x080000, 0x00637302, BRF_GRA | CPS1_TILES },
	{ "cdp.bin",       0x080000, 0xcc87cf61, BRF_GRA | CPS1_TILES },
	{ "cai.bin",       0x080000, 0x28718bed, BRF_GRA | CPS1_TILES },
	{ "cbi.bin",       0x080000, 0xd4acc53a, BRF_GRA | CPS1_TILES },
	{ "cci.bin",       0x080000, 0x0c69f151, BRF_GRA | CPS1_TILES },
	{ "cdi.bin",       0x080000, 0x1f9ebb97, BRF_GRA | CPS1_TILES },
				
	{ "1.bin",         0x040000, 0xaed2f4bd, BRF_PRG | CPS1_Z80_PROGRAM },
};

STD_ROM_PICK(Captcommb2)
STD_ROM_FN(Captcommb2)

static struct BurnRomInfo CawingRomDesc[] = {
	{ "cae_30a.11f",   0x020000, 0x91fceacd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cae_35a.11h",   0x020000, 0x3ef03083, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cae_31a.12f",   0x020000, 0xe5b75caf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cae_36a.12h",   0x020000, 0xc73fd713, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ca-32m.8h",     0x080000, 0x0c4837d4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ca-5m.7a",      0x080000, 0x66d4cc37, BRF_GRA | CPS1_TILES },
	{ "ca-7m.9a",      0x080000, 0xb6f896f2, BRF_GRA | CPS1_TILES },
	{ "ca-1m.3a",      0x080000, 0x4d0620fd, BRF_GRA | CPS1_TILES },
	{ "ca-3m.5a",      0x080000, 0x0b0341c3, BRF_GRA | CPS1_TILES },

	{ "ca_9.12b",      0x010000, 0x96fe7485, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ca_18.11c",     0x020000, 0x4a613a2c, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ca_19.12c",     0x020000, 0x74584493, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ca24b.1a",      0x000117, 0x76ec0b1c, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Cawing)
STD_ROM_FN(Cawing)

static struct BurnRomInfo Cawingr1RomDesc[] = {
	{ "cae_30.11f",    0x020000, 0x23305cd5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cae_35.11h",    0x020000, 0x69419113, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cae_31.12f",    0x020000, 0x9008dfb3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cae_36.12h",    0x020000, 0x4dbf6f8e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ca-32m.8h",     0x080000, 0x0c4837d4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ca-5m.7a",      0x080000, 0x66d4cc37, BRF_GRA | CPS1_TILES },
	{ "ca-7m.9a",      0x080000, 0xb6f896f2, BRF_GRA | CPS1_TILES },
	{ "ca-1m.3a",      0x080000, 0x4d0620fd, BRF_GRA | CPS1_TILES },
	{ "ca-3m.5a",      0x080000, 0x0b0341c3, BRF_GRA | CPS1_TILES },

	{ "cae_09.12b",    0x010000, 0x96fe7485, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cae_18.11c",    0x020000, 0x4a613a2c, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "cae_19.12c",    0x020000, 0x74584493, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ca24b.1a",      0x000117, 0x76ec0b1c, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Cawingr1)
STD_ROM_FN(Cawingr1)

static struct BurnRomInfo CawinguRomDesc[] = {
	{ "cau_30a.11f",   0x020000, 0x91fceacd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cau_35a.11h",   0x020000, 0xf090d9b2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cau_31a.12f",   0x020000, 0xe5b75caf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cau_36a.12h",   0x020000, 0xc73fd713, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ca-32m.8h",     0x080000, 0x0c4837d4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ca-5m.7a",      0x080000, 0x66d4cc37, BRF_GRA | CPS1_TILES },
	{ "ca-7m.9a",      0x080000, 0xb6f896f2, BRF_GRA | CPS1_TILES },
	{ "ca-1m.3a",      0x080000, 0x4d0620fd, BRF_GRA | CPS1_TILES },
	{ "ca-3m.5a",      0x080000, 0x0b0341c3, BRF_GRA | CPS1_TILES },

	{ "cau_09.12b",    0x010000, 0x96fe7485, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cau_18.11c",    0x020000, 0x4a613a2c, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "cau_19.12c",    0x020000, 0x74584493, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ca24b.1a",      0x000117, 0x76ec0b1c, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Cawingu)
STD_ROM_FN(Cawingu)

static struct BurnRomInfo CawingjRomDesc[] = {
	{ "caj_36a.12f",   0x020000, 0x91fceacd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "caj_42a.12h",   0x020000, 0x039f8362, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "caj_37a.13f",   0x020000, 0xe5b75caf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "caj_43a.13h",   0x020000, 0xc73fd713, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "caj_34.10f",    0x020000, 0x51ea57f4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "caj_40.10h",    0x020000, 0x2ab71ae1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "caj_35.11f",    0x020000, 0x01d71973, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "caj_41.11h",    0x020000, 0x3a43b538, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "caj_09.4b",     0x020000, 0x41b0f9a6, BRF_GRA | CPS1_TILES },
	{ "caj_01.4a",     0x020000, 0x1002d0b8, BRF_GRA | CPS1_TILES },
	{ "caj_13.9b",     0x020000, 0x6f3948b2, BRF_GRA | CPS1_TILES },
	{ "caj_05.9a",     0x020000, 0x207373d7, BRF_GRA | CPS1_TILES },
	{ "caj_24.5e",     0x020000, 0xe356aad7, BRF_GRA | CPS1_TILES },
	{ "caj_17.5c",     0x020000, 0x540f2fd8, BRF_GRA | CPS1_TILES },
	{ "caj_38.8h",     0x020000, 0x2464d4ab, BRF_GRA | CPS1_TILES },
	{ "caj_32.8f",     0x020000, 0x9b5836b3, BRF_GRA | CPS1_TILES },
	{ "caj_10.5b",     0x020000, 0xbf8a5f52, BRF_GRA | CPS1_TILES },
	{ "caj_02.5a",     0x020000, 0x125b018d, BRF_GRA | CPS1_TILES },
	{ "caj_14.10b",    0x020000, 0x8458e7d7, BRF_GRA | CPS1_TILES },
	{ "caj_06.10a",    0x020000, 0xcf80e164, BRF_GRA | CPS1_TILES },
	{ "caj_25.7e",     0x020000, 0xcdd0204d, BRF_GRA | CPS1_TILES },
	{ "caj_18.7c",     0x020000, 0x29c1d4b1, BRF_GRA | CPS1_TILES },
	{ "caj_39.9h",     0x020000, 0xeea23b67, BRF_GRA | CPS1_TILES },
	{ "caj_33.9f",     0x020000, 0xdde3891f, BRF_GRA | CPS1_TILES },

	{ "caj_23.13b",    0x010000, 0x96fe7485, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "caj_30.12c",    0x020000, 0x4a613a2c, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "caj_31.13c",    0x020000, 0x74584493, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ca22b.1a",      0x000117, 0x5152e678, BRF_OPT },	// b-board PLDs
	{ "iob1.12e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Cawingj)
STD_ROM_FN(Cawingj)

static struct BurnRomInfo CawingblRomDesc[] = {
	{ "caw2.bin",      0x080000, 0x8125d3f0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	// caw2
	{ "caw1.bin",      0x080000, 0xb19b10ce, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	// caw1

	{ "caw7.bin",      0x080000, 0xa045c689, BRF_GRA | CPS1_TILES },
	{ "caw6.bin",      0x080000, 0x61192f7c, BRF_GRA | CPS1_TILES },	
	{ "caw5.bin",      0x080000, 0x30dd78db, BRF_GRA | CPS1_TILES },
	{ "caw4.bin",      0x080000, 0x4937fc41, BRF_GRA | CPS1_TILES },	

	{ "caw3.bin",      0x020000, 0xffe16cdc, BRF_PRG | CPS1_Z80_PROGRAM },
};

STD_ROM_PICK(Cawingbl)
STD_ROM_FN(Cawingbl)

static struct BurnRomInfo Cps1demoRomDesc[] = {
	{ "cd30-36.bin",   0x020000, 0x5eb617d6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cd35-42.bin",   0x020000, 0x1f1c0a62, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cd31-37.bin",   0x020000, 0x7ee8cdcd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP }, // sound rom from cps1frog ?
	{ "cd36-43.bin",   0x020000, 0x7ee8cdcd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP }, // sound rom from cps1frog ?
	{ "ff-32m.8h",     0x080000, 0xc747696e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "cd05-05m.bin",  0x080000, 0x35dc6153, BRF_GRA | CPS1_TILES },
	{ "cd07-07m.bin",  0x080000, 0xbaf08a38, BRF_GRA | CPS1_TILES },
	{ "cd01-01m.bin",  0x080000, 0x3ad8e790, BRF_GRA | CPS1_TILES },
	{ "cd03-03m.bin",  0x080000, 0xfb5186f3, BRF_GRA | CPS1_TILES },

	{ "ff_09.12b",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_18.11c",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_19.12c",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Cps1demo)
STD_ROM_FN(Cps1demo)

static struct BurnRomInfo Cworld2jRomDesc[] = {
	{ "q5_36.12f",     0x020000, 0x38a08099, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "q5_42.12h",     0x020000, 0x4d29b3a4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "q5_37.13f",     0x020000, 0xeb547ebc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "q5_43.13h",     0x020000, 0x3ef65ea8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "q5_34.10f",     0x020000, 0x7fcc1317, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "q5_40.10h",     0x020000, 0x7f14b7b4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "q5_35.11f",     0x020000, 0xabacee26, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "q5_41.11h",     0x020000, 0xd3654067, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "q5_09.4b",      0x020000, 0x48496d80, BRF_GRA | CPS1_TILES },
	{ "q5_01.4a",      0x020000, 0xc5453f56, BRF_GRA | CPS1_TILES },
	{ "q5_13.9b",      0x020000, 0xc741ac52, BRF_GRA | CPS1_TILES },
	{ "q5_05.9a",      0x020000, 0x143e068f, BRF_GRA | CPS1_TILES },
	{ "q5_24.5e",      0x020000, 0xb419d139, BRF_GRA | CPS1_TILES },
	{ "q5_17.5c",      0x020000, 0xbd3b4d11, BRF_GRA | CPS1_TILES },
	{ "q5_38.8h",      0x020000, 0x9c24670c, BRF_GRA | CPS1_TILES },
	{ "q5_32.8f",      0x020000, 0x3ef9c7c2, BRF_GRA | CPS1_TILES },
	{ "q5_10.5b",      0x020000, 0x119e5e93, BRF_GRA | CPS1_TILES },
	{ "q5_02.5a",      0x020000, 0xa2cadcbe, BRF_GRA | CPS1_TILES },
	{ "q5_14.10b",     0x020000, 0xa8755f82, BRF_GRA | CPS1_TILES },
	{ "q5_06.10a",     0x020000, 0xc92a91fc, BRF_GRA | CPS1_TILES },
	{ "q5_25.7e",      0x020000, 0x979237cb, BRF_GRA | CPS1_TILES },
	{ "q5_18.7c",      0x020000, 0xc57da03c, BRF_GRA | CPS1_TILES },
	{ "q5_39.9h",      0x020000, 0xa5839b25, BRF_GRA | CPS1_TILES },
	{ "q5_33.9f",      0x020000, 0x04d03930, BRF_GRA | CPS1_TILES },

	{ "q5_23.13b",     0x010000, 0xe14dc524, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "q5_30.12c",     0x020000, 0xd10c1b68, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "q5_31.13c",     0x020000, 0x7d17e496, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "q522b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP},	// b-board PLDs
	{ "lwio.12e",      0x000117, 0xad52b90c, BRF_OPT },	
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Cworld2j)
STD_ROM_FN(Cworld2j)

static struct BurnRomInfo DinoRomDesc[] = {
	{ "cde_23a.8f",    0x080000, 0x8f4e585e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cde_22a.7f",    0x080000, 0x9278aa12, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cde_21a.6f",    0x080000, 0x66d23de2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "cd-1m.3a",      0x080000, 0x8da4f917, BRF_GRA | CPS1_TILES },
	{ "cd-3m.5a",      0x080000, 0x6c40f603, BRF_GRA | CPS1_TILES },
	{ "cd-2m.4a",      0x080000, 0x09c8fc2d, BRF_GRA | CPS1_TILES },
	{ "cd-4m.6a",      0x080000, 0x637ff38f, BRF_GRA | CPS1_TILES },
	{ "cd-5m.7a",      0x080000, 0x470befee, BRF_GRA | CPS1_TILES },
	{ "cd-7m.9a",      0x080000, 0x22bfb7a3, BRF_GRA | CPS1_TILES },
	{ "cd-6m.8a",      0x080000, 0xe7599ac4, BRF_GRA | CPS1_TILES },
	{ "cd-8m.10a",     0x080000, 0x211b4b15, BRF_GRA | CPS1_TILES },

	{ "cd_q.5k",       0x020000, 0x605fdb0b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cd-q1.1k",      0x080000, 0x60927775, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q2.2k",      0x080000, 0x770f4c47, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q3.3k",      0x080000, 0x2f273ffc, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q4.4k",      0x080000, 0x2c67821d, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "cd63b.1a",      0x000117, 0xef72e902, BRF_OPT },		// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },		// c-board PLDs
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },		// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Dino)
STD_ROM_FN(Dino)

static struct BurnRomInfo DinojRomDesc[] = {
	{ "cdj_23a.8f",    0x080000, 0x5f3ece96, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cdj_22a.7f",    0x080000, 0xa0d8de29, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cdj_21a.6f",    0x080000, 0x66d23de2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "cd_01.3a",      0x080000, 0x8da4f917, BRF_GRA | CPS1_TILES },
	{ "cd_02.4a",      0x080000, 0x6c40f603, BRF_GRA | CPS1_TILES },
	{ "cd_03.5a",      0x080000, 0x09c8fc2d, BRF_GRA | CPS1_TILES },
	{ "cd_04.6a",      0x080000, 0x637ff38f, BRF_GRA | CPS1_TILES },
	{ "cd_05.7a",      0x080000, 0x470befee, BRF_GRA | CPS1_TILES },
	{ "cd_06.8a",      0x080000, 0x22bfb7a3, BRF_GRA | CPS1_TILES },
	{ "cd_07.9a",      0x080000, 0xe7599ac4, BRF_GRA | CPS1_TILES },
	{ "cd_08.10a",     0x080000, 0x211b4b15, BRF_GRA | CPS1_TILES },

	{ "cd_q.5k",       0x020000, 0x605fdb0b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cd-q1.1k",      0x080000, 0x60927775, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q2.2k",      0x080000, 0x770f4c47, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q3.3k",      0x080000, 0x2f273ffc, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q4.4k",      0x080000, 0x2c67821d, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "cd63b.1a",      0x000117, 0xef72e902, BRF_OPT },		// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },		// c-board PLDs
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },		// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Dinoj)
STD_ROM_FN(Dinoj)

static struct BurnRomInfo DinouRomDesc[] = {
	{ "cdu_23a.8f",    0x080000, 0x7c2543cd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cdu_22a.7f",    0x080000, 0xfab740a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cdu_21a.6f",    0x080000, 0x66d23de2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "cd-1m.3a",      0x080000, 0x8da4f917, BRF_GRA | CPS1_TILES },
	{ "cd-3m.5a",      0x080000, 0x6c40f603, BRF_GRA | CPS1_TILES },
	{ "cd-2m.4a",      0x080000, 0x09c8fc2d, BRF_GRA | CPS1_TILES },
	{ "cd-4m.6a",      0x080000, 0x637ff38f, BRF_GRA | CPS1_TILES },
	{ "cd-5m.7a",      0x080000, 0x470befee, BRF_GRA | CPS1_TILES },
	{ "cd-7m.9a",      0x080000, 0x22bfb7a3, BRF_GRA | CPS1_TILES },
	{ "cd-6m.8a",      0x080000, 0xe7599ac4, BRF_GRA | CPS1_TILES },
	{ "cd-8m.10a",     0x080000, 0x211b4b15, BRF_GRA | CPS1_TILES },

	{ "cd_q.5k",       0x020000, 0x605fdb0b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cd-q1.1k",      0x080000, 0x60927775, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q2.2k",      0x080000, 0x770f4c47, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q3.3k",      0x080000, 0x2f273ffc, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd-q4.4k",      0x080000, 0x2c67821d, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "cd63b.1a",      0x000117, 0xef72e902, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },	// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Dinou)
STD_ROM_FN(Dinou)

static struct BurnRomInfo DinopicRomDesc[] = {
	// This set is also known as dinob
	{ "5.bin",         0x080000, 0x96dfcbf1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "3.bin",         0x080000, 0x13dfeb08, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "7.bin",         0x080000, 0x6133f349, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "2.bin",         0x080000, 0x0e4058ba, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "4.bin",         0x080000, 0xf3c2c98d, BRF_GRA | CPS1_TILES },
	{ "8.bin",         0x080000, 0xd574befc, BRF_GRA | CPS1_TILES },
	{ "9.bin",         0x080000, 0x55ef0adc, BRF_GRA | CPS1_TILES },
	{ "6.bin",         0x080000, 0xcc0805fc, BRF_GRA | CPS1_TILES },
	{ "13.bin",        0x080000, 0x1371f714, BRF_GRA | CPS1_TILES },
	{ "12.bin",        0x080000, 0xb284c4a7, BRF_GRA | CPS1_TILES },
	{ "11.bin",        0x080000, 0xb7ad3394, BRF_GRA | CPS1_TILES },
	{ "10.bin",        0x080000, 0x88847705, BRF_GRA | CPS1_TILES },
	
	{ "pic16c57-rp",   0x002d4c, 0x5a6d393c, BRF_PRG | CPS1_PIC },

	{ "1.bin",         0x080000, 0x7d921309, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Dinopic)
STD_ROM_FN(Dinopic)

static struct BurnRomInfo Dinopic2RomDesc[] = {
	{ "27c4000-m12481.bin",    0x080000, 0x96dfcbf1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "27c4000-m12374r-2.bin", 0x080000, 0x13dfeb08, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "27c4000-m12374r-3.bin", 0x080000, 0x6133f349, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "27c4000-m12374r-1.bin", 0x080000, 0x0e4058ba, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "27c4000-m12481-4.bin",  0x080000, 0xf3c2c98d, BRF_GRA | CPS1_TILES },
#if !defined ROM_VERIFY
	{ "27c4000-m12481-3.bin",  0x080000, 0xd574befc, BRF_GRA | CPS1_TILES },
#else
	{ "27c4000-m12481-3.bin",  0x080000, 0x417a4816, BRF_GRA | CPS1_TILES },
#endif
	{ "27c4000-m12481-2.bin",  0x080000, 0x55ef0adc, BRF_GRA | CPS1_TILES },
	{ "27c4000-m12481-1.bin",  0x080000, 0xcc0805fc, BRF_GRA | CPS1_TILES },
	{ "27c4000-m12481-8.bin",  0x080000, 0x1371f714, BRF_GRA | CPS1_TILES },
	{ "27c4000-m12481-7.bin",  0x080000, 0xb284c4a7, BRF_GRA | CPS1_TILES },
	{ "27c4000-m12481-6.bin",  0x080000, 0xb7ad3394, BRF_GRA | CPS1_TILES },
	{ "27c4000-m12481-5.bin",  0x080000, 0x88847705, BRF_GRA | CPS1_TILES },
	
	{ "pic16c57-xt.hex",       0x0026cc, 0xa6a5eac4, BRF_PRG | CPS1_PIC },

	{ "27c4000-m12623.bin",    0x080000, 0x7d921309, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	{ "cat93c46p.bin",         0x000080, 0xd49fa351, BRF_OPT },
	{ "gal20v8a-1.bin",        0x000157, 0xcd99ca47, BRF_OPT },
	{ "gal20v8a-2.bin",        0x000157, 0x60d016b9, BRF_OPT },
	{ "gal20v8a-3.bin",        0x000157, 0x049b7f4f, BRF_OPT },
	{ "palce16v8h-1.bin",      0x000117, 0x48253c66, BRF_OPT },
	{ "palce16v8h-2.bin",      0x000117, 0x9ae375ba, BRF_OPT },
};

STD_ROM_PICK(Dinopic2)
STD_ROM_FN(Dinopic2)

static struct BurnRomInfo DinohRomDesc[] = {
	{ "cda_23h.rom",   0x080000, 0x8e2a9cf0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cda_22h.rom",   0x080000, 0xf72cd219, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cda_21h.rom",   0x080000, 0xbc275b76, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cda_20h.rom",   0x080000, 0x8987c975, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "cd_gfx01.rom",  0x080000, 0x8da4f917, BRF_GRA | CPS1_TILES },
	{ "cd_gfx03.rom",  0x080000, 0x6c40f603, BRF_GRA | CPS1_TILES },
	{ "cd_gfx02.rom",  0x080000, 0x09c8fc2d, BRF_GRA | CPS1_TILES },
	{ "cd_gfx04.rom",  0x080000, 0x637ff38f, BRF_GRA | CPS1_TILES },
	{ "cd_gfx05.rom",  0x080000, 0x470befee, BRF_GRA | CPS1_TILES },
	{ "cd_gfx07.rom",  0x080000, 0x22bfb7a3, BRF_GRA | CPS1_TILES },
	{ "cd_gfx06.rom",  0x080000, 0xe7599ac4, BRF_GRA | CPS1_TILES },
	{ "cd_gfx08.rom",  0x080000, 0x211b4b15, BRF_GRA | CPS1_TILES },
	
	// This doesn't work at all without the Q-Sound data present - does it really use Q-Sound??	
	{ "cd_q.rom",      0x020000, 0x605fdb0b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cd_q1.rom",     0x080000, 0x60927775, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q2.rom",     0x080000, 0x770f4c47, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q3.rom",     0x080000, 0x2f273ffc, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q4.rom",     0x080000, 0x2c67821d, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Dinoh)
STD_ROM_FN(Dinoh)

static struct BurnRomInfo DinotRomDesc[] = {
	// This set is also known as Kl2d
	{ "kl2-r1.800",    0x100000, 0x4c70dca7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "kl2-l2.800",    0x100000, 0xc6ae7338, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "cd-a.160",      0x200000, 0x7e4f9fb3, BRF_GRA | CPS1_TILES },
	{ "cd-b.160",      0x200000, 0x89532d85, BRF_GRA | CPS1_TILES },
	
	{ "cd_q.rom",      0x020000, 0x605fdb0b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cd_q1.rom",     0x080000, 0x60927775, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q2.rom",     0x080000, 0x770f4c47, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q3.rom",     0x080000, 0x2f273ffc, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q4.rom",     0x080000, 0x2c67821d, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Dinot)
STD_ROM_FN(Dinot)

static struct BurnRomInfo DinotpicRomDesc[] = {
	{ "cd-d.800",      0x100000, 0x2a7b2915, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cd-e.800",      0x100000, 0xe8370226, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "cd-a.160",      0x200000, 0x7e4f9fb3, BRF_GRA | CPS1_TILES },
	{ "cd-b.160",      0x200000, 0x89532d85, BRF_GRA | CPS1_TILES },
};

STD_ROM_PICK(Dinotpic)
STD_ROM_FN(Dinotpic)

static struct BurnRomInfo DinohuntRomDesc[] = {
	{ "u23",           0x080000, 0x8d5ddc5d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "u22",           0x080000, 0xf72cd219, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "u21",           0x080000, 0xbc275b76, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "u20",           0x080000, 0x8987c975, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "u1",            0x080000, 0xa01a9fb5, BRF_GRA | CPS1_TILES },
 	{ "u2",            0x080000, 0xbdf02c17, BRF_GRA | CPS1_TILES },
	{ "u3",            0x080000, 0x058beefa, BRF_GRA | CPS1_TILES },
	{ "u4",            0x080000, 0x5028a9f1, BRF_GRA | CPS1_TILES },
	{ "u5",            0x080000, 0xd77f89ea, BRF_GRA | CPS1_TILES },
	{ "u6",            0x080000, 0xbfbcb034, BRF_GRA | CPS1_TILES },
	{ "u7",            0x080000, 0xa2544d4e, BRF_GRA | CPS1_TILES },
	{ "u8",            0x080000, 0x8869bbb1, BRF_GRA | CPS1_TILES },

	{ "u9",            0x010000, 0x2eb16a83, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "u18",           0x020000, 0x8d2899ba, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "u19",           0x020000, 0xb34a4b42, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Dinohunt)
STD_ROM_FN(Dinohunt)

static struct BurnRomInfo DinoehRomDesc[] = {
	{ "dinoeh.23",     0x080000, 0xa6b88364, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "cde_22a.rom",   0x080000, 0x9278aa12, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "dinoeh.21",     0x080000, 0xb89a0548, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "cd_gfx01.rom",  0x080000, 0x8da4f917, BRF_GRA | CPS1_TILES },
	{ "cd_gfx03.rom",  0x080000, 0x6c40f603, BRF_GRA | CPS1_TILES },
	{ "cd_gfx02.rom",  0x080000, 0x09c8fc2d, BRF_GRA | CPS1_TILES },
	{ "cd_gfx04.rom",  0x080000, 0x637ff38f, BRF_GRA | CPS1_TILES },
	{ "cd_gfx05.rom",  0x080000, 0x470befee, BRF_GRA | CPS1_TILES },
	{ "cd_gfx07.rom",  0x080000, 0x22bfb7a3, BRF_GRA | CPS1_TILES },
	{ "cd_gfx06.rom",  0x080000, 0xe7599ac4, BRF_GRA | CPS1_TILES },
	{ "cd_gfx08.rom",  0x080000, 0x211b4b15, BRF_GRA | CPS1_TILES },

	{ "cd_q.rom",      0x020000, 0x605fdb0b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "cd_q1.rom",     0x080000, 0x60927775, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q2.rom",     0x080000, 0x770f4c47, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q3.rom",     0x080000, 0x2f273ffc, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "cd_q4.rom",     0x080000, 0x2c67821d, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Dinoeh)
STD_ROM_FN(Dinoeh)

static struct BurnRomInfo DynwarRomDesc[] = {
	{ "30.11f",        0x020000, 0xf9ec6d68, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "35.11h",        0x020000, 0xe41fff2f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "31.12f",        0x020000, 0xe3de76ff, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "36.12h",        0x020000, 0x7a13cfbf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tkm-9.8h",      0x080000, 0x93654bcf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tkm-5.7a",      0x080000, 0xf64bb6a0, BRF_GRA | CPS1_TILES },
	{ "tkm-8.9a",      0x080000, 0x21fe6274, BRF_GRA | CPS1_TILES },
	{ "tkm-6.3a",      0x080000, 0x0bf228cb, BRF_GRA | CPS1_TILES },
	{ "tkm-7.5a",      0x080000, 0x1255dfb1, BRF_GRA | CPS1_TILES },
	{ "tkm-1.8a",      0x080000, 0x44f7661e, BRF_GRA | CPS1_TILES },
	{ "tkm-4.10a",     0x080000, 0xa54c515d, BRF_GRA | CPS1_TILES },
	{ "tkm-2.4a",      0x080000, 0xca5c687c, BRF_GRA | CPS1_TILES },
	{ "tkm-3.6a",      0x080000, 0xf9fe6591, BRF_GRA | CPS1_TILES },

	{ "tke_17.12b",    0x010000, 0xb3b79d4f, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tke_18.11c",    0x020000, 0xac6e307d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "tke_19.12c",    0x020000, 0x068741db, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "tk24b1.1a",     0x000117, 0xae4a7645, BRF_OPT},	// b-board PLDs
	{ "lwio.11e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Dynwar)
STD_ROM_FN(Dynwar)

static struct BurnRomInfo DynwaraRomDesc[] = {
	{ "tke_36.12f",    0x020000, 0x895991d1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tke_42.12h",    0x020000, 0xc898d2e8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tke_37.13f",    0x020000, 0xb228d58c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tke_43.13h",    0x020000, 0x1a14375a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "34.10f",        0x020000, 0x8f663d00, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "40.10h",        0x020000, 0x1586dbf3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "35.11f",        0x020000, 0x9db93d7a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41.11h",        0x020000, 0x1aae69a4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "09.4b",         0x020000, 0xc3e83c69, BRF_GRA | CPS1_TILES },
	{ "01.4a",         0x020000, 0x187b2886, BRF_GRA | CPS1_TILES },
	{ "13.9b",         0x020000, 0x0273d87d, BRF_GRA | CPS1_TILES },
	{ "05.9a",         0x020000, 0x339378b8, BRF_GRA | CPS1_TILES },
	{ "24.5e",         0x020000, 0xc6909b6f, BRF_GRA | CPS1_TILES },
	{ "17.5c",         0x020000, 0x2e2f8320, BRF_GRA | CPS1_TILES },
	{ "38.8h",         0x020000, 0xcd7923ed, BRF_GRA | CPS1_TILES },
	{ "32.8f",         0x020000, 0x21a0a453, BRF_GRA | CPS1_TILES },
	{ "10.5b",         0x020000, 0xff28f8d0, BRF_GRA | CPS1_TILES },
	{ "02.5a",         0x020000, 0xcc83c02f, BRF_GRA | CPS1_TILES },
	{ "14.10b",        0x020000, 0x58d9b32f, BRF_GRA | CPS1_TILES },
	{ "06.10a",        0x020000, 0x6f9edd75, BRF_GRA | CPS1_TILES },
	{ "25.7e",         0x020000, 0x152ea74a, BRF_GRA | CPS1_TILES },
	{ "18.7c",         0x020000, 0x1833f932, BRF_GRA | CPS1_TILES },
	{ "39.9h",         0x020000, 0xbc09b360, BRF_GRA | CPS1_TILES },
	{ "33.9f",         0x020000, 0x89de1533, BRF_GRA | CPS1_TILES },
	{ "11.7b",         0x020000, 0x29eaf490, BRF_GRA | CPS1_TILES },
	{ "03.7a",         0x020000, 0x7bf51337, BRF_GRA | CPS1_TILES },
	{ "15.11b",        0x020000, 0xd36cdb91, BRF_GRA | CPS1_TILES },
	{ "07.11a",        0x020000, 0xe04af054, BRF_GRA | CPS1_TILES },
	{ "26.8e",         0x020000, 0x07fc714b, BRF_GRA | CPS1_TILES },
	{ "19.8c",         0x020000, 0x7114e5c6, BRF_GRA | CPS1_TILES },
	{ "28.10e",        0x020000, 0xaf62bf07, BRF_GRA | CPS1_TILES },
	{ "21.10c",        0x020000, 0x523f462a, BRF_GRA | CPS1_TILES },
	{ "12.8b",         0x020000, 0x38652339, BRF_GRA | CPS1_TILES },
	{ "04.8a",         0x020000, 0x4951bc0f, BRF_GRA | CPS1_TILES },
	{ "16.12b",        0x020000, 0x381608ae, BRF_GRA | CPS1_TILES },
	{ "08.12a",        0x020000, 0xb475d4e9, BRF_GRA | CPS1_TILES },
	{ "27.9e",         0x020000, 0xa27e81fa, BRF_GRA | CPS1_TILES },
	{ "20.9c",         0x020000, 0x002796dc, BRF_GRA | CPS1_TILES },
	{ "29.11e",        0x020000, 0x6b41f82d, BRF_GRA | CPS1_TILES },
	{ "22.11c",        0x020000, 0x52145369, BRF_GRA | CPS1_TILES },

	{ "23.13c",        0x010000, 0xb3b79d4f, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tke_30.12e",    0x020000, 0xac6e307d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "tke_31.13e",    0x020000, 0x068741db, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "tk22b.1a",      0x000117, 0x1a1ab6d7, BRF_OPT },	// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Dynwara)
STD_ROM_FN(Dynwara)

static struct BurnRomInfo DynwarjRomDesc[] = {
	{ "36.12f",        0x020000, 0x1a516657, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "42.12h",        0x020000, 0x12a290a0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "37.13f",        0x020000, 0x932fc943, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "43.13h",        0x020000, 0x872ad76d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "34.10f",        0x020000, 0x8f663d00, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "40.10h",        0x020000, 0x1586dbf3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "35.11f",        0x020000, 0x9db93d7a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "41.11h",        0x020000, 0x1aae69a4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "09.4b",         0x020000, 0xc3e83c69, BRF_GRA | CPS1_TILES },
	{ "01.4a",         0x020000, 0x187b2886, BRF_GRA | CPS1_TILES },
	{ "13.9b",         0x020000, 0x0273d87d, BRF_GRA | CPS1_TILES },
	{ "05.9a",         0x020000, 0x339378b8, BRF_GRA | CPS1_TILES },
	{ "24.5e",         0x020000, 0xc6909b6f, BRF_GRA | CPS1_TILES },
	{ "17.5c",         0x020000, 0x2e2f8320, BRF_GRA | CPS1_TILES },
	{ "38.8h",         0x020000, 0xcd7923ed, BRF_GRA | CPS1_TILES },
	{ "32.8f",         0x020000, 0x21a0a453, BRF_GRA | CPS1_TILES },
	{ "10.5b",         0x020000, 0xff28f8d0, BRF_GRA | CPS1_TILES },
	{ "02.5a",         0x020000, 0xcc83c02f, BRF_GRA | CPS1_TILES },
	{ "14.10b",        0x020000, 0x58d9b32f, BRF_GRA | CPS1_TILES },
	{ "06.10a",        0x020000, 0x6f9edd75, BRF_GRA | CPS1_TILES },
	{ "25.7e",         0x020000, 0x152ea74a, BRF_GRA | CPS1_TILES },
	{ "18.7c",         0x020000, 0x1833f932, BRF_GRA | CPS1_TILES },
	{ "39.9h",         0x020000, 0xbc09b360, BRF_GRA | CPS1_TILES },
	{ "33.9f",         0x020000, 0x89de1533, BRF_GRA | CPS1_TILES },
	{ "11.7b",         0x020000, 0x29eaf490, BRF_GRA | CPS1_TILES },
	{ "03.7a",         0x020000, 0x7bf51337, BRF_GRA | CPS1_TILES },
	{ "15.11b",        0x020000, 0xd36cdb91, BRF_GRA | CPS1_TILES },
	{ "07.11a",        0x020000, 0xe04af054, BRF_GRA | CPS1_TILES },
	{ "26.8e",         0x020000, 0x07fc714b, BRF_GRA | CPS1_TILES },
	{ "19.8c",         0x020000, 0x7114e5c6, BRF_GRA | CPS1_TILES },
	{ "28.10e",        0x020000, 0xaf62bf07, BRF_GRA | CPS1_TILES },
	{ "21.10c",        0x020000, 0x523f462a, BRF_GRA | CPS1_TILES },
	{ "12.8b",         0x020000, 0x38652339, BRF_GRA | CPS1_TILES },
	{ "04.8a",         0x020000, 0x4951bc0f, BRF_GRA | CPS1_TILES },
	{ "16.12b",        0x020000, 0x381608ae, BRF_GRA | CPS1_TILES },
	{ "08.12a",        0x020000, 0xb475d4e9, BRF_GRA | CPS1_TILES },
	{ "27.9e",         0x020000, 0xa27e81fa, BRF_GRA | CPS1_TILES },
	{ "20.9c",         0x020000, 0x002796dc, BRF_GRA | CPS1_TILES },
	{ "29.11e",        0x020000, 0x6b41f82d, BRF_GRA | CPS1_TILES },
	{ "22.11c",        0x020000, 0x52145369, BRF_GRA | CPS1_TILES },

	{ "23.13c",        0x010000, 0xb3b79d4f, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "30.12e",        0x020000, 0x7e5f6cb4, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "31.13e",        0x020000, 0x4a30c737, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "tk22b.1a",      0x000117, 0x1a1ab6d7, BRF_OPT },	// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Dynwarj)
STD_ROM_FN(Dynwarj)

static struct BurnRomInfo DynwarjrRomDesc[] = {
	{ "tk1j_23.8f",    0x080000, 0x088a3009, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk1j_22.7f",    0x080000, 0x93654bcf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk1_01.3a",     0x080000, 0xf64bb6a0, BRF_GRA | CPS1_TILES },
	{ "tk1_02.4a",     0x080000, 0x21fe6274, BRF_GRA | CPS1_TILES },
	{ "tk1_03.5a",     0x080000, 0x0bf228cb, BRF_GRA | CPS1_TILES },
	{ "tk1_04.6a",     0x080000, 0x1255dfb1, BRF_GRA | CPS1_TILES },
	{ "tk1_05.7a",     0x080000, 0x44f7661e, BRF_GRA | CPS1_TILES },
	{ "tk1_06.8a",     0x080000, 0xa54c515d, BRF_GRA | CPS1_TILES },
	{ "tk1_07.9a",     0x080000, 0xca5c687c, BRF_GRA | CPS1_TILES },
	{ "tk1_08.10a",    0x080000, 0xf9fe6591, BRF_GRA | CPS1_TILES },

	{ "tk1_09.12a",    0x020000, 0xdb77d899, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk1_18.11c",    0x020000, 0x7e5f6cb4, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "tk1_19.12c",    0x020000, 0x4a30c737, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "tk163b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },		// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Dynwarjr)
STD_ROM_FN(Dynwarjr)

static struct BurnRomInfo FfightRomDesc[] = {
	{ "ff_36.11f",     0x020000, 0xf9a5ce83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_42.11h",     0x020000, 0x65f11215, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_37.12f",     0x020000, 0xe1033784, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffe_43.12h",    0x020000, 0x995e968a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff-32m.8h",     0x080000, 0xc747696e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ff-5m.7a",      0x080000, 0x9c284108, BRF_GRA | CPS1_TILES },
	{ "ff-7m.9a",      0x080000, 0xa7584dfb, BRF_GRA | CPS1_TILES },
	{ "ff-1m.3a",      0x080000, 0x0b605e44, BRF_GRA | CPS1_TILES },
	{ "ff-3m.5a",      0x080000, 0x52291cd2, BRF_GRA | CPS1_TILES },

	{ "ff_09.12b",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_18.11c",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_19.12c",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s224b.1a",      0x000117, 0x4e85b158, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Ffight)
STD_ROM_FN(Ffight)

static struct BurnRomInfo FfightaRomDesc[] = {
	{ "ffe_30.11f",    0x020000, 0x2347bf51, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffe_35.11h",    0x020000, 0x5f694ecc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffe_31.12f",    0x020000, 0x6dc6b792, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffe_36.12h",    0x020000, 0xb36a0b99, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff-32m.8h",     0x080000, 0xc747696e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ff-5m.7a",      0x080000, 0x9c284108, BRF_GRA | CPS1_TILES },
	{ "ff-7m.9a",      0x080000, 0xa7584dfb, BRF_GRA | CPS1_TILES },
	{ "ff-1m.3a",      0x080000, 0x0b605e44, BRF_GRA | CPS1_TILES },
	{ "ff-3m.5a",      0x080000, 0x52291cd2, BRF_GRA | CPS1_TILES },

	{ "ffe_23.12b",    0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_18.11c",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_19.12c",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s224b.1a",      0x000117, 0x4e85b158, BRF_OPT },	// b-board PLDs
	{ "lwio.11e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Ffighta)
STD_ROM_FN(Ffighta)

static struct BurnRomInfo FfightuRomDesc[] = {
	{ "ff_36.11f",     0x020000, 0xf9a5ce83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_42.11h",     0x020000, 0x65f11215, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_37.12f",     0x020000, 0xe1033784, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffu_43.12h",    0x020000, 0x4ca65947, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff-32m.8h",     0x080000, 0xc747696e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ff-5m.7a",      0x080000, 0x9c284108, BRF_GRA | CPS1_TILES },
	{ "ff-7m.9a",      0x080000, 0xa7584dfb, BRF_GRA | CPS1_TILES },
	{ "ff-1m.3a",      0x080000, 0x0b605e44, BRF_GRA | CPS1_TILES },
	{ "ff-3m.5a",      0x080000, 0x52291cd2, BRF_GRA | CPS1_TILES },

	{ "ff_09.12b",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_18.11c",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_19.12c",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s224b.1a",      0x000117, 0x4e85b158, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Ffightu)
STD_ROM_FN(Ffightu)

static struct BurnRomInfo Ffightu1RomDesc[] = {
	{ "ff_36.11f",     0x020000, 0xf9a5ce83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_42.11h",     0x020000, 0x65f11215, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_37.12f",     0x020000, 0xe1033784, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffu_43.12h",    0x020000, 0x4ca65947, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_34.9f",      0x020000, 0x0c8dc3fc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_40.9h",      0x020000, 0x8075bab9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_35.10f",     0x020000, 0x4a934121, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_41.10h",     0x020000, 0x2af68154, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "ff-5m.7a",      0x080000, 0x9c284108, BRF_GRA | CPS1_TILES },
	{ "ff-7m.9a",      0x080000, 0xa7584dfb, BRF_GRA | CPS1_TILES },
	{ "ff-1m.3a",      0x080000, 0x0b605e44, BRF_GRA | CPS1_TILES },
	{ "ff-3m.5a",      0x080000, 0x52291cd2, BRF_GRA | CPS1_TILES },

	{ "ff_09.12b",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_18.11c",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_19.12c",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s224b.1a",      0x000117, 0x4e85b158, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Ffightu1)
STD_ROM_FN(Ffightu1)

static struct BurnRomInfo FfightuaRomDesc[] = {
	{ "ffu_36.11f",    0x020000, 0xe2a48af9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffu_42.11h",    0x020000, 0xf4bb480e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffu_37.12f",    0x020000, 0xc371c667, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffu_43.36",     0x020000, 0x2f5771f9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff-32m.8h",     0x080000, 0xc747696e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ff-5m.7a",      0x080000, 0x9c284108, BRF_GRA | CPS1_TILES },
	{ "ff-7m.9a",      0x080000, 0xa7584dfb, BRF_GRA | CPS1_TILES },
	{ "ff-1m.3a",      0x080000, 0x0b605e44, BRF_GRA | CPS1_TILES },
	{ "ff-3m.5a",      0x080000, 0x52291cd2, BRF_GRA | CPS1_TILES },
	
	{ "ff_09.12b",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_18.11c",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_19.12c",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s224b.1a",      0x000117, 0x4e85b158, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Ffightua)
STD_ROM_FN(Ffightua)

static struct BurnRomInfo FfightubRomDesc[] = {
	{ "ffu_30.11f",    0x020000, 0xed988977, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffu_35.11h",    0x020000, 0x07bf1c21, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffu_31.12f",    0x020000, 0xdba5a476, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffu_36.12h",    0x020000, 0x4d89f542, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff-32m.8h",     0x080000, 0xc747696e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ff-5m.7a",      0x080000, 0x9c284108, BRF_GRA | CPS1_TILES },
	{ "ff-7m.9a",      0x080000, 0xa7584dfb, BRF_GRA | CPS1_TILES },
	{ "ff-1m.3a",      0x080000, 0x0b605e44, BRF_GRA | CPS1_TILES },
	{ "ff-3m.5a",      0x080000, 0x52291cd2, BRF_GRA | CPS1_TILES },

	{ "ff_23.12b",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_18.11c",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_19.12c",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s224b.1a",      0x000117, 0x4e85b158, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Ffightub)
STD_ROM_FN(Ffightub)

static struct BurnRomInfo FfightjRomDesc[] = {
	{ "ff36.bin",      0x020000, 0xf9a5ce83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff42.bin",      0x020000, 0x65f11215, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff37.bin",      0x020000, 0xe1033784, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff43.bin",      0x020000, 0xb6dee1c3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_34.10f",    0x020000, 0x0c8dc3fc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_40.10h",    0x020000, 0x8075bab9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_35.11f",    0x020000, 0x4a934121, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_41.11h",    0x020000, 0x2af68154, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "ffj_09.4b",     0x020000, 0x5b116d0d, BRF_GRA | CPS1_TILES },
	{ "ffj_01.4a",     0x020000, 0x815b1797, BRF_GRA | CPS1_TILES },
	{ "ffj_13.9b",     0x020000, 0x8721a7da, BRF_GRA | CPS1_TILES },
	{ "ffj_05.9a",     0x020000, 0xd0fcd4b5, BRF_GRA | CPS1_TILES },
	{ "ffj_24.5e",     0x020000, 0xa1ab607a, BRF_GRA | CPS1_TILES },
	{ "ffj_17.5c",     0x020000, 0x2dc18cf4, BRF_GRA | CPS1_TILES },
	{ "ffj_38.8h",     0x020000, 0x6535a57f, BRF_GRA | CPS1_TILES },
	{ "ffj_32.8f",     0x020000, 0xc8bc4a57, BRF_GRA | CPS1_TILES },
	{ "ffj_10.5b",     0x020000, 0x624a924a, BRF_GRA | CPS1_TILES },
	{ "ffj_02.5a",     0x020000, 0x5d91f694, BRF_GRA | CPS1_TILES },
	{ "ffj_14.10b",    0x020000, 0x0a2e9101, BRF_GRA | CPS1_TILES },
	{ "ffj_06.10a",    0x020000, 0x1c18f042, BRF_GRA | CPS1_TILES },
	{ "ffj_25.7e",     0x020000, 0x6e8181ea, BRF_GRA | CPS1_TILES },
	{ "ffj_18.7c",     0x020000, 0xb19ede59, BRF_GRA | CPS1_TILES },
	{ "ffj_39.9h",     0x020000, 0x9416b477, BRF_GRA | CPS1_TILES },
	{ "ffj_33.9f",     0x020000, 0x7369fa07, BRF_GRA | CPS1_TILES },

	{ "ff_23.bin",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ffj_30.bin",    0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ffj_31.bin",    0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s222b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Ffightj)
STD_ROM_FN(Ffightj)

static struct BurnRomInfo Ffightj1RomDesc[] = {
	{ "ffj_36.12f",    0x020000, 0xe2a48af9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_42.12h",    0x020000, 0xf4bb480e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_37.13f",    0x020000, 0xc371c667, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_43.13h",    0x020000, 0x6f81f194, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_34.10f",    0x020000, 0x0c8dc3fc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_40.10h",    0x020000, 0x8075bab9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_35.11f",    0x020000, 0x4a934121, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_41.11h",    0x020000, 0x2af68154, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "ffj_09.4b",     0x020000, 0x5b116d0d, BRF_GRA | CPS1_TILES },
	{ "ffj_01.4a",     0x020000, 0x815b1797, BRF_GRA | CPS1_TILES },
	{ "ffj_13.9b",     0x020000, 0x8721a7da, BRF_GRA | CPS1_TILES },
	{ "ffj_05.9a",     0x020000, 0xd0fcd4b5, BRF_GRA | CPS1_TILES },
	{ "ffj_24.5e",     0x020000, 0xa1ab607a, BRF_GRA | CPS1_TILES },
	{ "ffj_17.5c",     0x020000, 0x2dc18cf4, BRF_GRA | CPS1_TILES },
	{ "ffj_38.8h",     0x020000, 0x6535a57f, BRF_GRA | CPS1_TILES },
	{ "ffj_32.8f",     0x020000, 0xc8bc4a57, BRF_GRA | CPS1_TILES },
	{ "ffj_10.5b",     0x020000, 0x624a924a, BRF_GRA | CPS1_TILES },
	{ "ffj_02.5a",     0x020000, 0x5d91f694, BRF_GRA | CPS1_TILES },
	{ "ffj_14.10b",    0x020000, 0x0a2e9101, BRF_GRA | CPS1_TILES },
	{ "ffj_06.10a",    0x020000, 0x1c18f042, BRF_GRA | CPS1_TILES },
	{ "ffj_25.7e",     0x020000, 0x6e8181ea, BRF_GRA | CPS1_TILES },
	{ "ffj_18.7c",     0x020000, 0xb19ede59, BRF_GRA | CPS1_TILES },
	{ "ffj_39.9h",     0x020000, 0x9416b477, BRF_GRA | CPS1_TILES },
	{ "ffj_33.9f",     0x020000, 0x7369fa07, BRF_GRA | CPS1_TILES },

	{ "ff_23.13b",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ffj_30.12c",    0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ffj_31.13c",    0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s222b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Ffightj1)
STD_ROM_FN(Ffightj1)

static struct BurnRomInfo Ffightj2RomDesc[] = {
	{ "ffj_36a.12f",   0x020000, 0x088ed1c9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_42a.12h",   0x020000, 0xc4c491e6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_37a.13f",   0x020000, 0x708557ff, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ffj_43a.13h",   0x020000, 0xc004004a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_34.10f",     0x020000, 0x0c8dc3fc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_40.10h",     0x020000, 0x8075bab9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_35.11f",     0x020000, 0x4a934121, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ff_41.11h",     0x020000, 0x2af68154, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "ff_09.4b",      0x020000, 0x5b116d0d, BRF_GRA | CPS1_TILES },
	{ "ff_01.4a",      0x020000, 0x815b1797, BRF_GRA | CPS1_TILES },
	{ "ff_13.9b",      0x020000, 0x8721a7da, BRF_GRA | CPS1_TILES },
	{ "ff_05.9a",      0x020000, 0xd0fcd4b5, BRF_GRA | CPS1_TILES },
	{ "ff_24.5e",      0x020000, 0xa1ab607a, BRF_GRA | CPS1_TILES },
	{ "ff_17.5c",      0x020000, 0x2dc18cf4, BRF_GRA | CPS1_TILES },
	{ "ff_38.8h",      0x020000, 0x6535a57f, BRF_GRA | CPS1_TILES },
	{ "ff_32.8f",      0x020000, 0xc8bc4a57, BRF_GRA | CPS1_TILES },
	{ "ff_10.5b",      0x020000, 0x624a924a, BRF_GRA | CPS1_TILES },
	{ "ff_02.5a",      0x020000, 0x5d91f694, BRF_GRA | CPS1_TILES },
	{ "ff_14.10b",     0x020000, 0x0a2e9101, BRF_GRA | CPS1_TILES },
	{ "ff_06.10a",     0x020000, 0x1c18f042, BRF_GRA | CPS1_TILES },
	{ "ff_25.7e",      0x020000, 0x6e8181ea, BRF_GRA | CPS1_TILES },
	{ "ff_18.7c",      0x020000, 0xb19ede59, BRF_GRA | CPS1_TILES },
	{ "ff_39.9h",      0x020000, 0x9416b477, BRF_GRA | CPS1_TILES },
	{ "ff_33.9f",      0x020000, 0x7369fa07, BRF_GRA | CPS1_TILES },

	{ "ff_23.13c",     0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_30.12e",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_31.13e",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s222b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Ffightj2)
STD_ROM_FN(Ffightj2)

static struct BurnRomInfo FfightjhRomDesc[] = {
	{ "ff_23.8f",      0x080000, 0xae3dda7f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "ff_22.7f",      0x080000, 0xb2d5a3aa, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ff_1.3a",       0x080000, 0x969d18e2, BRF_GRA | CPS1_TILES },
	{ "ff_2.4a",       0x080000, 0x02b59f99, BRF_GRA | CPS1_TILES },
	{ "ff_3.5a",       0x080000, 0x01d507ae, BRF_GRA | CPS1_TILES },
	{ "ff_4.6a",       0x080000, 0xf7c4ceb0, BRF_GRA | CPS1_TILES },

	{ "ff_9.12a",      0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ff_18.11c",     0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ff_19.12c",     0x020000, 0x1ef137f9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "gal16v8a-15lp.1a", 0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Ffightjh)
STD_ROM_FN(Ffightjh)

static struct BurnRomInfo FcrashRomDesc[] = {
	{ "9.bin",         0x020000, 0xc6854c91, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "5.bin",         0x020000, 0x77f7c2b3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "8.bin",         0x020000, 0x1895b3df, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "4.bin",         0x020000, 0xbbd411ee, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "7.bin",         0x020000, 0x5b23ebf2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "3.bin",         0x020000, 0xaba2aebe, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "6.bin",         0x020000, 0xd4bf37f6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "2.bin",         0x020000, 0x07ac8f43, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "18.bin",        0x020000, 0xf1eee6d9, BRF_GRA | CPS1_TILES },
	{ "20.bin",        0x020000, 0x675f4537, BRF_GRA | CPS1_TILES },
	{ "22.bin",        0x020000, 0xdb8a32ac, BRF_GRA | CPS1_TILES },
	{ "24.bin",        0x020000, 0xf4113e57, BRF_GRA | CPS1_TILES },
	{ "10.bin",        0x020000, 0xd478853e, BRF_GRA | CPS1_TILES },
	{ "12.bin",        0x020000, 0x25055642, BRF_GRA | CPS1_TILES },
	{ "14.bin",        0x020000, 0xb77d0328, BRF_GRA | CPS1_TILES },
	{ "16.bin",        0x020000, 0xea111a79, BRF_GRA | CPS1_TILES },
	{ "19.bin",        0x020000, 0xb3aa1f48, BRF_GRA | CPS1_TILES },
	{ "21.bin",        0x020000, 0x04d175c9, BRF_GRA | CPS1_TILES },
	{ "23.bin",        0x020000, 0xe592ba4f, BRF_GRA | CPS1_TILES },
	{ "25.bin",        0x020000, 0xb89a740f, BRF_GRA | CPS1_TILES },
	{ "11.bin",        0x020000, 0xd4457a60, BRF_GRA | CPS1_TILES },
	{ "13.bin",        0x020000, 0x3b26a37d, BRF_GRA | CPS1_TILES },
	{ "15.bin",        0x020000, 0x6d837e09, BRF_GRA | CPS1_TILES },
	{ "17.bin",        0x020000, 0xc59a4d6c, BRF_GRA | CPS1_TILES },

	{ "1.bin",         0x020000, 0x5b276c14, BRF_PRG | CPS1_Z80_PROGRAM },
};

STD_ROM_PICK(Fcrash)
STD_ROM_FN(Fcrash)

static struct BurnRomInfo ForgottnRomDesc[] = {
	{ "lw11.12f",      0x020000, 0x73e920b7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw15.12h",      0x020000, 0x50d7012d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw10.13f",      0x020000, 0xbea45994, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw14.13h",      0x020000, 0x539b2339, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw-07.10g",     0x080000, 0xfd252a26, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "lw_2.2b",       0x020000, 0x4bd75fee, BRF_GRA | CPS1_TILES },
	{ "lw_1.2a",       0x020000, 0x65f41485, BRF_GRA | CPS1_TILES },
	{ "lw-08.9b",      0x080000, 0x25a8e43c, BRF_GRA | CPS1_TILES },
	{ "lw-05.6d",      0x080000, 0xe4552fd7, BRF_GRA | CPS1_TILES },
	{ "lw_30.8h",      0x020000, 0xb385954e, BRF_GRA | CPS1_TILES },
	{ "lw_29.8f",      0x020000, 0x7bda1ac6, BRF_GRA | CPS1_TILES },
	{ "lw_4.3b",       0x020000, 0x50cf757f, BRF_GRA | CPS1_TILES },
	{ "lw_3.3a",       0x020000, 0xc03ef278, BRF_GRA | CPS1_TILES },
	{ "lw_32.9h",      0x020000, 0x30967a15, BRF_GRA | CPS1_TILES },
	{ "lw_31.9f",      0x020000, 0xc49d37fb, BRF_GRA | CPS1_TILES },
	{ "lw-02.6b",      0x080000, 0x43e6c5c8, BRF_GRA | CPS1_TILES },
	{ "lw_14.10b",     0x020000, 0x82862cce, BRF_GRA | CPS1_TILES },
	{ "lw_13.10a",     0x020000, 0xb81c0e96, BRF_GRA | CPS1_TILES },
	{ "lw-06.9d",      0x080000, 0x5b9edffc, BRF_GRA | CPS1_TILES },
	{ "lw_26.10e",     0x020000, 0x57bcd032, BRF_GRA | CPS1_TILES },
	{ "lw_25.10c",     0x020000, 0xbac91554, BRF_GRA | CPS1_TILES },	
	{ "lw_16.11b",     0x020000, 0x40b26554, BRF_GRA | CPS1_TILES },
	{ "lw_15.11a",     0x020000, 0x1b7d2e07, BRF_GRA | CPS1_TILES },
	{ "lw_28.11e",     0x020000, 0xa805ad30, BRF_GRA | CPS1_TILES },
	{ "lw_27.11c",     0x020000, 0x103c1bd2, BRF_GRA | CPS1_TILES },
	
	{ "lw_00.13c",     0x010000, 0x59df2a63, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "lw-03u.12e",    0x020000, 0x807d051f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "lw-04u.13e",    0x020000, 0xe6cd098e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "lw621.1a",      0x000117, 0x5eec6ce9, BRF_OPT },	// b-board PLDs
	{ "lwio.12b",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Forgottn)
STD_ROM_FN(Forgottn)

static struct BurnRomInfo ForgottnuRomDesc[] = {
	{ "lw11c.12f",     0x020000, 0xe62742b6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw15c.12h",     0x020000, 0x1b70f216, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw10c.13f",     0x020000, 0x8f5ea3f5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw14c.13h",     0x020000, 0x708e7472, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw-07.10g",     0x080000, 0xfd252a26, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "lw_2.2b",       0x020000, 0x4bd75fee, BRF_GRA | CPS1_TILES },
	{ "lw_1.2a",       0x020000, 0x65f41485, BRF_GRA | CPS1_TILES },
	{ "lw-08.9b",      0x080000, 0x25a8e43c, BRF_GRA | CPS1_TILES },
	{ "lw-05.6d",      0x080000, 0xe4552fd7, BRF_GRA | CPS1_TILES },
	{ "lw_30.8h",      0x020000, 0xb385954e, BRF_GRA | CPS1_TILES },
	{ "lw_29.8f",      0x020000, 0x7bda1ac6, BRF_GRA | CPS1_TILES },
	{ "lw_4.3b",       0x020000, 0x50cf757f, BRF_GRA | CPS1_TILES },
	{ "lw_3.3a",       0x020000, 0xc03ef278, BRF_GRA | CPS1_TILES },
	{ "lw_32.9h",      0x020000, 0x30967a15, BRF_GRA | CPS1_TILES },
	{ "lw_31.9f",      0x020000, 0xc49d37fb, BRF_GRA | CPS1_TILES },
	{ "lw-02.6b",      0x080000, 0x43e6c5c8, BRF_GRA | CPS1_TILES },
	{ "lw_14.10b",     0x020000, 0x82862cce, BRF_GRA | CPS1_TILES },
	{ "lw_13.10a",     0x020000, 0xb81c0e96, BRF_GRA | CPS1_TILES },
	{ "lw-06.9d",      0x080000, 0x5b9edffc, BRF_GRA | CPS1_TILES },
	{ "lw-13.10d",     0x080000, 0x8e058ef5, BRF_GRA | CPS1_TILES },
	{ "lw_16.11b",     0x020000, 0x40b26554, BRF_GRA | CPS1_TILES },
	{ "lw_15.11a",     0x020000, 0x1b7d2e07, BRF_GRA | CPS1_TILES },

	{ "lw_00.13c",     0x010000, 0x59df2a63, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "lw-03u.12e",    0x020000, 0x807d051f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "lw-04u.13e",    0x020000, 0xe6cd098e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "lw621.1a",      0x000117, 0x5eec6ce9, BRF_OPT },	// b-board PLDs
	{ "lwio.12b",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Forgottnu)
STD_ROM_FN(Forgottnu)

static struct BurnRomInfo Forgottnu1RomDesc[] = {
	{ "lw11c.14f",     0x020000, 0xe62742b6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw15c.14g",     0x020000, 0x1b70f216, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw10c.13f",     0x020000, 0x8f5ea3f5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw14c.13g",     0x020000, 0x708e7472, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw-07.13e",     0x080000, 0xfd252a26, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "lw-01.9d",      0x080000, 0x0318f298, BRF_GRA | CPS1_TILES },
	{ "lw-08.9f",      0x080000, 0x25a8e43c, BRF_GRA | CPS1_TILES },
	{ "lw-05.9e",      0x080000, 0xe4552fd7, BRF_GRA | CPS1_TILES },
	{ "lw-12.9g",      0x080000, 0x8e6a832b, BRF_GRA | CPS1_TILES },
	{ "lw-02.12d",     0x080000, 0x43e6c5c8, BRF_GRA | CPS1_TILES },
	{ "lw-09.12f",     0x080000, 0x899cb4ad, BRF_GRA | CPS1_TILES },
	{ "lw-06.12e",     0x080000, 0x5b9edffc, BRF_GRA | CPS1_TILES },
	{ "lw-13.12g",     0x080000, 0x8e058ef5, BRF_GRA | CPS1_TILES },

	{ "lw_00.14a",     0x010000, 0x59df2a63, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "lw-03u.14c",    0x020000, 0x807d051f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "lw-04u.13c",    0x020000, 0xe6cd098e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "lwchr.3a",      0x000117, 0x54ed4c39, BRF_OPT },	// b-board PLDs
	{ "lwio.15e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Forgottnu1)
STD_ROM_FN(Forgottnu1)

static struct BurnRomInfo ForgottnuaRomDesc[] = {
	{ "lwu_11a.14f",   0x020000, 0xddf78831, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lwu_15a.14g",   0x020000, 0xf7ce2097, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lwu_10a.13f",   0x020000, 0x8cb38c81, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lwu_14a.13g",   0x020000, 0xd70ef9fd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw-07.13e",     0x080000, 0xfd252a26, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "lw-01.9d",      0x080000, 0x0318f298, BRF_GRA | CPS1_TILES },
	{ "lw-08.9f",      0x080000, 0x25a8e43c, BRF_GRA | CPS1_TILES },
	{ "lw-05.9e",      0x080000, 0xe4552fd7, BRF_GRA | CPS1_TILES },
	{ "lw-12.9g",      0x080000, 0x8e6a832b, BRF_GRA | CPS1_TILES },
	{ "lw-02.12d",     0x080000, 0x43e6c5c8, BRF_GRA | CPS1_TILES },
	{ "lw-09.12f",     0x080000, 0x899cb4ad, BRF_GRA | CPS1_TILES },
	{ "lw-06.12e",     0x080000, 0x5b9edffc, BRF_GRA | CPS1_TILES },
	{ "lw-13.12g",     0x080000, 0x8e058ef5, BRF_GRA | CPS1_TILES },

	{ "lwu_00.14a",    0x010000, 0x59df2a63, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "lw-03u.14c",    0x020000, 0x807d051f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "lw-04u.13c",    0x020000, 0xe6cd098e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "lwchr.3a",      0x000117, 0x54ed4c39, BRF_OPT },	// b-board PLDs
	{ "lwio.15e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Forgottnua)
STD_ROM_FN(Forgottnua)

static struct BurnRomInfo LostwrldRomDesc[] = {
	{ "lw_11c.14f",    0x020000, 0x67e42546, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw_15c.14g",    0x020000, 0x402e2a46, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw_10c.13f",    0x020000, 0xc46479d7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw_14c.13g",    0x020000, 0x97670f4a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw-07.13e",     0x080000, 0xfd252a26, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "lw-01.9d",      0x080000, 0x0318f298, BRF_GRA | CPS1_TILES },
	{ "lw-08.9f",      0x080000, 0x25a8e43c, BRF_GRA | CPS1_TILES },
	{ "lw-05.9e",      0x080000, 0xe4552fd7, BRF_GRA | CPS1_TILES },
	{ "lw-12.9g",      0x080000, 0x8e6a832b, BRF_GRA | CPS1_TILES },
	{ "lw-02.12d",     0x080000, 0x43e6c5c8, BRF_GRA | CPS1_TILES },
	{ "lw-09.12f",     0x080000, 0x899cb4ad, BRF_GRA | CPS1_TILES },
	{ "lw-06.12e",     0x080000, 0x5b9edffc, BRF_GRA | CPS1_TILES },
	{ "lw-13.12g",     0x080000, 0x8e058ef5, BRF_GRA | CPS1_TILES },

	{ "lw_00b.14a",    0x010000, 0x59df2a63, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "lw-03.14c",     0x020000, 0xce2159e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "lw-04.13c",     0x020000, 0x39305536, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "lwchr.3a",      0x000117, 0x54ed4c39, BRF_OPT },	// b-board PLDs
	{ "lwio.15e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Lostwrld)
STD_ROM_FN(Lostwrld)

static struct BurnRomInfo LostwrldoRomDesc[] = {
	{ "lw_11.14f",     0x020000, 0x61e2cc56, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw_15.14g",     0x020000, 0x8a0c18d3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw_10.13f",     0x020000, 0x23bca4d5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw_14.13g",     0x020000, 0x3a023771, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "lw-07.13e",     0x080000, 0xfd252a26, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "lw-01.9d",      0x080000, 0x0318f298, BRF_GRA | CPS1_TILES },
	{ "lw-08.9f",      0x080000, 0x25a8e43c, BRF_GRA | CPS1_TILES },
	{ "lw-05.9e",      0x080000, 0xe4552fd7, BRF_GRA | CPS1_TILES },
	{ "lw-12.9g",      0x080000, 0x8e6a832b, BRF_GRA | CPS1_TILES },
	{ "lw-02.12d",     0x080000, 0x43e6c5c8, BRF_GRA | CPS1_TILES },
	{ "lw-09.12f",     0x080000, 0x899cb4ad, BRF_GRA | CPS1_TILES },
	{ "lw-06.12e",     0x080000, 0x5b9edffc, BRF_GRA | CPS1_TILES },
	{ "lw-13.12g",     0x080000, 0x8e058ef5, BRF_GRA | CPS1_TILES },

	{ "lw_00b.14a",    0x010000, 0x59df2a63, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "lw-03.14c",     0x020000, 0xce2159e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "lw-04.13c",     0x020000, 0x39305536, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "lwchr.3a",      0x000117, 0x54ed4c39, BRF_OPT },	// b-board PLDs
	{ "lwio.15e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Lostwrldo)
STD_ROM_FN(Lostwrldo)

static struct BurnRomInfo GhoulsRomDesc[] = {
	{ "dme_29.10h",    0x020000, 0x166a58a2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dme_30.10j",    0x020000, 0x7ac8407a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dme_27.9h",     0x020000, 0xf734b2be, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dme_28.9j",     0x020000, 0x03d3e714, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dm-17.7j",      0x080000, 0x3ea1b0f2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "dm-05.3a",      0x080000, 0x0ba9c0b0, BRF_GRA | CPS1_TILES },
	{ "dm-07.3f",      0x080000, 0x5d760ab9, BRF_GRA | CPS1_TILES },
	{ "dm-06.3c",      0x080000, 0x4ba90b59, BRF_GRA | CPS1_TILES },
	{ "dm-08.3g",      0x080000, 0x4bdee9de, BRF_GRA | CPS1_TILES },
	{ "09.4a",         0x010000, 0xae24bb19, BRF_GRA | CPS1_TILES },
	{ "18.7a",         0x010000, 0xd34e271a, BRF_GRA | CPS1_TILES },
	{ "13.4e",         0x010000, 0x3f70dd37, BRF_GRA | CPS1_TILES },
	{ "22.7e",         0x010000, 0x7e69e2e6, BRF_GRA | CPS1_TILES },
	{ "11.4c",         0x010000, 0x37c9b6c6, BRF_GRA | CPS1_TILES },
	{ "20.7c",         0x010000, 0x2f1345b4, BRF_GRA | CPS1_TILES },
	{ "15.4g",         0x010000, 0x3c2a212a, BRF_GRA | CPS1_TILES },
	{ "24.7g",         0x010000, 0x889aac05, BRF_GRA | CPS1_TILES },
	{ "10.4b",         0x010000, 0xbcc0f28c, BRF_GRA | CPS1_TILES },
	{ "19.7b",         0x010000, 0x2a40166a, BRF_GRA | CPS1_TILES },
	{ "14.4f",         0x010000, 0x20f85c03, BRF_GRA | CPS1_TILES },
	{ "23.7f",         0x010000, 0x8426144b, BRF_GRA | CPS1_TILES },
	{ "12.4d",         0x010000, 0xda088d61, BRF_GRA | CPS1_TILES },
	{ "21.7d",         0x010000, 0x17e11df0, BRF_GRA | CPS1_TILES },
	{ "16.4h",         0x010000, 0xf187ba1c, BRF_GRA | CPS1_TILES },
	{ "25.7h",         0x010000, 0x29f79c78, BRF_GRA | CPS1_TILES },

	{ "26.10a",        0x010000, 0x3692f6e5, BRF_PRG | CPS1_Z80_PROGRAM },
	
	A_BOARD_PLDS
	
	{ "dm620.2a",      0x000117, 0xf6e5f727, BRF_OPT },	// b-board PLDs
	{ "lwio.8i",       0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Ghouls)
STD_ROM_FN(Ghouls)

static struct BurnRomInfo GhoulsuRomDesc[] = {
	{ "dmu_29.10h",    0x020000, 0x334d85b2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dmu_30.10j",    0x020000, 0xcee8ceb5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dmu_27.9h",     0x020000, 0x4a524140, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dmu_28.9j",     0x020000, 0x94aae205, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dm-17.7j",      0x080000, 0x3ea1b0f2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "dm-05.3a",      0x080000, 0x0ba9c0b0, BRF_GRA | CPS1_TILES },
	{ "dm-07.3f",      0x080000, 0x5d760ab9, BRF_GRA | CPS1_TILES },
	{ "dm-06.3c",      0x080000, 0x4ba90b59, BRF_GRA | CPS1_TILES },
	{ "dm-08.3g",      0x080000, 0x4bdee9de, BRF_GRA | CPS1_TILES },
	{ "09.4a",         0x010000, 0xae24bb19, BRF_GRA | CPS1_TILES },
	{ "18.7a",         0x010000, 0xd34e271a, BRF_GRA | CPS1_TILES },
	{ "13.4e",         0x010000, 0x3f70dd37, BRF_GRA | CPS1_TILES },
	{ "22.7e",         0x010000, 0x7e69e2e6, BRF_GRA | CPS1_TILES },
	{ "11.4c",         0x010000, 0x37c9b6c6, BRF_GRA | CPS1_TILES },
	{ "20.7c",         0x010000, 0x2f1345b4, BRF_GRA | CPS1_TILES },
	{ "15.4g",         0x010000, 0x3c2a212a, BRF_GRA | CPS1_TILES },
	{ "24.7g",         0x010000, 0x889aac05, BRF_GRA | CPS1_TILES },
	{ "10.4b",         0x010000, 0xbcc0f28c, BRF_GRA | CPS1_TILES },
	{ "19.7b",         0x010000, 0x2a40166a, BRF_GRA | CPS1_TILES },
	{ "14.4f",         0x010000, 0x20f85c03, BRF_GRA | CPS1_TILES },
	{ "23.7f",         0x010000, 0x8426144b, BRF_GRA | CPS1_TILES },
	{ "12.4d",         0x010000, 0xda088d61, BRF_GRA | CPS1_TILES },
	{ "21.7d",         0x010000, 0x17e11df0, BRF_GRA | CPS1_TILES },
	{ "16.4h",         0x010000, 0xf187ba1c, BRF_GRA | CPS1_TILES },
	{ "25.7h",         0x010000, 0x29f79c78, BRF_GRA | CPS1_TILES },

	{ "26.10a",        0x010000, 0x3692f6e5, BRF_PRG | CPS1_Z80_PROGRAM },
	
	A_BOARD_PLDS
	
	{ "dm620.2a",      0x000117, 0xf6e5f727, BRF_OPT },	// b-board PLDs
	{ "lwio.8i",       0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Ghoulsu)
STD_ROM_FN(Ghoulsu)

static struct BurnRomInfo DaimakaiRomDesc[] = {
	{ "dmj_38.12f",    0x020000, 0x82fd1798, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dmj_39.12h",    0x020000, 0x35366ccc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dmj_40.13f",    0x020000, 0xa17c170a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dmj_41.13h",    0x020000, 0x6af0b391, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dm_33.10f",     0x020000, 0x384d60c4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dm_34.10h",     0x020000, 0x19abe30f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dm_35.11f",     0x020000, 0xc04b85c8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dm_36.11h",     0x020000, 0x89be83de, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "dm_02.4b",      0x020000, 0x8b98dc48, BRF_GRA | CPS1_TILES },
	{ "dm_01.4a",      0x020000, 0x80896c33, BRF_GRA | CPS1_TILES },
	{ "dm_10.9b",      0x020000, 0xc2e7d9ef, BRF_GRA | CPS1_TILES },
	{ "dm_09.9a",      0x020000, 0xc9c4afa5, BRF_GRA | CPS1_TILES },
	{ "dm_18.5e",      0x020000, 0x1aa0db99, BRF_GRA | CPS1_TILES },
	{ "dm_17.5c",      0x020000, 0xdc6ed8ad, BRF_GRA | CPS1_TILES },
	{ "dm_30.8h",      0x020000, 0xd9d3f8bd, BRF_GRA | CPS1_TILES },
	{ "dm_29.8f",      0x020000, 0x49a48796, BRF_GRA | CPS1_TILES },
	{ "dm_04.5b",      0x020000, 0xa4f4f8f0, BRF_GRA | CPS1_TILES },
	{ "dm_03.5a",      0x020000, 0xb1033e62, BRF_GRA | CPS1_TILES },
	{ "dm_12.10b",     0x020000, 0x10fdd76a, BRF_GRA | CPS1_TILES },
	{ "dm_11.10a",     0x020000, 0x9040cb04, BRF_GRA | CPS1_TILES },
	{ "dm_20.7e",      0x020000, 0x281d0b3e, BRF_GRA | CPS1_TILES },
	{ "dm_19.7c",      0x020000, 0x2623b52f, BRF_GRA | CPS1_TILES },
	{ "dm_32.9h",      0x020000, 0x99692344, BRF_GRA | CPS1_TILES },
	{ "dm_31.9f",      0x020000, 0x54acb729, BRF_GRA | CPS1_TILES },
	{ "dm_06.7b",      0x010000, 0xae24bb19, BRF_GRA | CPS1_TILES },
	{ "dm_05.7a",      0x010000, 0xd34e271a, BRF_GRA | CPS1_TILES },
	{ "dm_14.11b",     0x010000, 0x3f70dd37, BRF_GRA | CPS1_TILES },
	{ "dm_13.11a",     0x010000, 0x7e69e2e6, BRF_GRA | CPS1_TILES },
	{ "dm_22.8e",      0x010000, 0x37c9b6c6, BRF_GRA | CPS1_TILES },
	{ "dm_21.8c",      0x010000, 0x2f1345b4, BRF_GRA | CPS1_TILES },
	{ "dm_26.10e",     0x010000, 0x3c2a212a, BRF_GRA | CPS1_TILES },
	{ "dm_25.10c",     0x010000, 0x889aac05, BRF_GRA | CPS1_TILES },
	{ "dm_08.8b",      0x010000, 0xbcc0f28c, BRF_GRA | CPS1_TILES },
	{ "dm_07.8a",      0x010000, 0x2a40166a, BRF_GRA | CPS1_TILES },
	{ "dm_16.12b",     0x010000, 0x20f85c03, BRF_GRA | CPS1_TILES },
	{ "dm_15.12a",     0x010000, 0x8426144b, BRF_GRA | CPS1_TILES },
	{ "dm_24.9e",      0x010000, 0xda088d61, BRF_GRA | CPS1_TILES },
	{ "dm_23.9c",      0x010000, 0x17e11df0, BRF_GRA | CPS1_TILES },
	{ "dm_28.11e",     0x010000, 0xf187ba1c, BRF_GRA | CPS1_TILES },
	{ "dm_27.11c",     0x010000, 0x29f79c78, BRF_GRA | CPS1_TILES },

	{ "37.13c",        0x010000, 0x3692f6e5, BRF_PRG | CPS1_Z80_PROGRAM },
	
	A_BOARD_PLDS
	
	{ "dm22a.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Daimakai)
STD_ROM_FN(Daimakai)

static struct BurnRomInfo DaimakairRomDesc[] = {
	{ "damj_23.8f",    0x080000, 0xc3b248ec, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "damj_22.7f",    0x080000, 0x595ff2f3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "dam_01.3a",     0x080000, 0x0ba9c0b0, BRF_GRA | CPS1_TILES },
	{ "dam_02.4a",     0x080000, 0x5d760ab9, BRF_GRA | CPS1_TILES },
	{ "dam_03.5a",     0x080000, 0x4ba90b59, BRF_GRA | CPS1_TILES },
	{ "dam_04.6a",     0x080000, 0x4bdee9de, BRF_GRA | CPS1_TILES },
	{ "dam_05.7a",     0x080000, 0x7dc61b94, BRF_GRA | CPS1_TILES },
	{ "dam_06.8a",     0x080000, 0xfde89758, BRF_GRA | CPS1_TILES },
	{ "dam_07.9a",     0x080000, 0xec351d78, BRF_GRA | CPS1_TILES },
	{ "dam_08.10a",    0x080000, 0xee2acc1e, BRF_GRA | CPS1_TILES },

	{ "dam_09.12a",    0x020000, 0x0656ff53, BRF_PRG | CPS1_Z80_PROGRAM },
	
	A_BOARD_PLDS
	
	{ "dam63b.1a",     0x000117, 0x474b3c8a, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Daimakair)
STD_ROM_FN(Daimakair)

static struct BurnRomInfo DaimakaibRomDesc[] = {
	{ "dmjb2.bin",     0x080000, 0x7d5f9f84, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "dmjb1.bin",     0x080000, 0x9b945cc4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	

	{ "dm_02.4b",      0x020000, 0x8b98dc48, BRF_GRA | CPS1_TILES },
	{ "dm_01.4a",      0x020000, 0x80896c33, BRF_GRA | CPS1_TILES },
	{ "dm_10.9b",      0x020000, 0xc2e7d9ef, BRF_GRA | CPS1_TILES },
	{ "dm_09.9a",      0x020000, 0xc9c4afa5, BRF_GRA | CPS1_TILES },
	{ "dm_18.5e",      0x020000, 0x1aa0db99, BRF_GRA | CPS1_TILES },
	{ "dm_17.5c",      0x020000, 0xdc6ed8ad, BRF_GRA | CPS1_TILES },
	{ "dm_30.8h",      0x020000, 0xd9d3f8bd, BRF_GRA | CPS1_TILES },
	{ "dm_29.8f",      0x020000, 0x49a48796, BRF_GRA | CPS1_TILES },
	{ "dm_04.5b",      0x020000, 0xa4f4f8f0, BRF_GRA | CPS1_TILES },
	{ "dm_03.5a",      0x020000, 0xb1033e62, BRF_GRA | CPS1_TILES },
	{ "dm_12.10b",     0x020000, 0x10fdd76a, BRF_GRA | CPS1_TILES },
	{ "dm_11.10a",     0x020000, 0x9040cb04, BRF_GRA | CPS1_TILES },
	{ "dm_20.7e",      0x020000, 0x281d0b3e, BRF_GRA | CPS1_TILES },
	{ "dm_19.7c",      0x020000, 0x2623b52f, BRF_GRA | CPS1_TILES },
	{ "dm_32.9h",      0x020000, 0x99692344, BRF_GRA | CPS1_TILES },
	{ "dm_31.9f",      0x020000, 0x54acb729, BRF_GRA | CPS1_TILES },
	{ "dm_06.7b",      0x010000, 0xae24bb19, BRF_GRA | CPS1_TILES },
	{ "dm_05.7a",      0x010000, 0xd34e271a, BRF_GRA | CPS1_TILES },
	{ "dm_14.11b",     0x010000, 0x3f70dd37, BRF_GRA | CPS1_TILES },
	{ "dm_13.11a",     0x010000, 0x7e69e2e6, BRF_GRA | CPS1_TILES },
	{ "dm_22.8e",      0x010000, 0x37c9b6c6, BRF_GRA | CPS1_TILES },
	{ "dm_21.8c",      0x010000, 0x2f1345b4, BRF_GRA | CPS1_TILES },
	{ "dm_26.10e",     0x010000, 0x3c2a212a, BRF_GRA | CPS1_TILES },
	{ "dm_25.10c",     0x010000, 0x889aac05, BRF_GRA | CPS1_TILES },
	{ "dm_08.8b",      0x010000, 0xbcc0f28c, BRF_GRA | CPS1_TILES },
	{ "dm_07.8a",      0x010000, 0x2a40166a, BRF_GRA | CPS1_TILES },
	{ "dm_16.12b",     0x010000, 0x20f85c03, BRF_GRA | CPS1_TILES },
	{ "dm_15.12a",     0x010000, 0x8426144b, BRF_GRA | CPS1_TILES },
	{ "dm_24.9e",      0x010000, 0xda088d61, BRF_GRA | CPS1_TILES },
	{ "dm_23.9c",      0x010000, 0x17e11df0, BRF_GRA | CPS1_TILES },
	{ "dm_28.11e",     0x010000, 0xf187ba1c, BRF_GRA | CPS1_TILES },
	{ "dm_27.11c",     0x010000, 0x29f79c78, BRF_GRA | CPS1_TILES },

	{ "dm_37.13c",     0x010000, 0x3692f6e5, BRF_PRG | CPS1_Z80_PROGRAM },
};

STD_ROM_PICK(Daimakaib)
STD_ROM_FN(Daimakaib)

static struct BurnRomInfo KnightsRomDesc[] = {
	{ "kr_23e.8f",     0x080000, 0x1b3997eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "kr_22.7f",      0x080000, 0xd0b671a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "kr-5m.3a",      0x080000, 0x9e36c1a4, BRF_GRA | CPS1_TILES },
	{ "kr-7m.5a",      0x080000, 0xc5832cae, BRF_GRA | CPS1_TILES },
	{ "kr-1m.4a",      0x080000, 0xf095be2d, BRF_GRA | CPS1_TILES },
	{ "kr-3m.6a",      0x080000, 0x179dfd96, BRF_GRA | CPS1_TILES },
	{ "kr-6m.7a",      0x080000, 0x1f4298d2, BRF_GRA | CPS1_TILES },
	{ "kr-8m.9a",      0x080000, 0x37fa8751, BRF_GRA | CPS1_TILES },
	{ "kr-2m.8a",      0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "kr-4m.10a",     0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "kr_09.11a",     0x010000, 0x5e44d9ee, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kr_18.11c",     0x020000, 0xda69d15f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kr_19.12c",     0x020000, 0xbfc654e9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kr63b.1a",      0x000117, 0xfd5b6522, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },	
};

STD_ROM_PICK(Knights)
STD_ROM_FN(Knights)

static struct BurnRomInfo KnightsuRomDesc[] = {
	{ "kr_23u.8f",     0x080000, 0x252bc2ba, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "kr_22.7f",      0x080000, 0xd0b671a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "kr-5m.3a",      0x080000, 0x9e36c1a4, BRF_GRA | CPS1_TILES },
	{ "kr-7m.5a",      0x080000, 0xc5832cae, BRF_GRA | CPS1_TILES },
	{ "kr-1m.4a",      0x080000, 0xf095be2d, BRF_GRA | CPS1_TILES },
	{ "kr-3m.6a",      0x080000, 0x179dfd96, BRF_GRA | CPS1_TILES },
	{ "kr-6m.7a",      0x080000, 0x1f4298d2, BRF_GRA | CPS1_TILES },
	{ "kr-8m.9a",      0x080000, 0x37fa8751, BRF_GRA | CPS1_TILES },
	{ "kr-2m.8a",      0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "kr-4m.10a",     0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "kr_09.11a",     0x010000, 0x5e44d9ee, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kr_18.11c",     0x020000, 0xda69d15f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kr_19.12c",     0x020000, 0xbfc654e9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kr63b.1a",      0x000117, 0xfd5b6522, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },	
};

STD_ROM_PICK(Knightsu)
STD_ROM_FN(Knightsu)

static struct BurnRomInfo KnightsjRomDesc[] = {
	{ "kr_23j.8f",     0x080000, 0xeae7417f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "kr_22.7f",      0x080000, 0xd0b671a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "kr_01.3a",      0x080000, 0x9e36c1a4, BRF_GRA | CPS1_TILES },
	{ "kr_02.4a",      0x080000, 0xc5832cae, BRF_GRA | CPS1_TILES },
	{ "kr_03.5a",      0x080000, 0xf095be2d, BRF_GRA | CPS1_TILES },
	{ "kr_04.6a",      0x080000, 0x179dfd96, BRF_GRA | CPS1_TILES },
	{ "kr_05.7a",      0x080000, 0x1f4298d2, BRF_GRA | CPS1_TILES },
	{ "kr_06.8a",      0x080000, 0x37fa8751, BRF_GRA | CPS1_TILES },
	{ "kr_07.9a",      0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "kr_08.10a",     0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "kr_09.12a",     0x010000, 0x5e44d9ee, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kr_18.11c",     0x020000, 0xda69d15f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kr_19.12c",     0x020000, 0xbfc654e9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kr63b.1a",      0x000117, 0xfd5b6522, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Knightsj)
STD_ROM_FN(Knightsj)

static struct BurnRomInfo KnightsjaRomDesc[] = {
	{ "krj_36.12f",    0x020000, 0xad3d1a8e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "krj_42.12h",    0x020000, 0xe694a491, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "krj_37.13f",    0x020000, 0x85596094, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "krj_43.13h",    0x020000, 0x9198bf8f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kr_34.10f",     0x020000, 0xfe6eb08d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kr_40.10h",     0x020000, 0x1172806d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kr_35.11f",     0x020000, 0xf854b020, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kr_41.11h",     0x020000, 0xeb52e78d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "kr_09.4b",      0x020000, 0x08b76e10, BRF_GRA | CPS1_TILES },
	{ "kr_01.4a",      0x020000, 0x40cecf5c, BRF_GRA | CPS1_TILES },
	{ "kr_13.9b",      0x020000, 0x435aaa03, BRF_GRA | CPS1_TILES },
	{ "kr_05.9a",      0x020000, 0x5b8a615b, BRF_GRA | CPS1_TILES },
	{ "kr_24.5e",      0x020000, 0xde65153e, BRF_GRA | CPS1_TILES },
	{ "kr_17.5c",      0x020000, 0xb171c968, BRF_GRA | CPS1_TILES },
	{ "kr_38.8h",      0x020000, 0xf4466bf4, BRF_GRA | CPS1_TILES },
	{ "kr_32.8f",      0x020000, 0x87380ddd, BRF_GRA | CPS1_TILES },
	{ "kr_10.5b",      0x020000, 0x37006d66, BRF_GRA | CPS1_TILES },
	{ "kr_02.5a",      0x020000, 0xb54612e3, BRF_GRA | CPS1_TILES },
	{ "kr_14.10b",     0x020000, 0x0ae88766, BRF_GRA | CPS1_TILES },
	{ "kr_06.10a",     0x020000, 0xecb1a09a, BRF_GRA | CPS1_TILES },
	{ "kr_25.7e",      0x020000, 0x9aace189, BRF_GRA | CPS1_TILES },
	{ "kr_18.7c",      0x020000, 0x09fa14a5, BRF_GRA | CPS1_TILES },
	{ "kr_39.9h",      0x020000, 0xfd8a9aeb, BRF_GRA | CPS1_TILES },
	{ "kr_33.9f",      0x020000, 0x11803e95, BRF_GRA | CPS1_TILES },
	{ "kr_11.7b",      0x020000, 0xa967ceb3, BRF_GRA | CPS1_TILES },
	{ "kr_03.7a",      0x020000, 0xea10db07, BRF_GRA | CPS1_TILES },
	{ "kr_15.11b",     0x020000, 0x8140b83b, BRF_GRA | CPS1_TILES },
	{ "kr_07.11a",     0x020000, 0x6af10648, BRF_GRA | CPS1_TILES },
	{ "kr_26.8e",      0x020000, 0x8865d86b, BRF_GRA | CPS1_TILES },
	{ "kr_19.8c",      0x020000, 0x029f4abe, BRF_GRA | CPS1_TILES },
	{ "kr_28.10e",     0x020000, 0x5f84f92f, BRF_GRA | CPS1_TILES },
	{ "kr_21.10c",     0x020000, 0x01b35065, BRF_GRA | CPS1_TILES },
	{ "kr_12.8b",      0x020000, 0x03d945b1, BRF_GRA | CPS1_TILES },
	{ "kr_04.8a",      0x020000, 0xe30c8388, BRF_GRA | CPS1_TILES },
	{ "kr_16.12b",     0x020000, 0x40c39d1b, BRF_GRA | CPS1_TILES },
	{ "kr_08.12a",     0x020000, 0xd310c9e8, BRF_GRA | CPS1_TILES },
	{ "kr_27.9e",      0x020000, 0x3e041444, BRF_GRA | CPS1_TILES },
	{ "kr_20.9c",      0x020000, 0xbd4bffb8, BRF_GRA | CPS1_TILES },
	{ "kr_29.11e",     0x020000, 0x1387a076, BRF_GRA | CPS1_TILES },
	{ "kr_22.11c",     0x020000, 0xfd351922, BRF_GRA | CPS1_TILES },
	
	{ "kr_23.13b",     0x010000, 0x5e44d9ee, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kr_30.12c",     0x020000, 0xda69d15f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kr_31.13c",     0x020000, 0xbfc654e9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kr22b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12e",      0x000117, 0xad52b90c, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Knightsja)
STD_ROM_FN(Knightsja)

static struct BurnRomInfo KnightsbRomDesc[] = {
	{ "5.ic172",       0x040000, 0x7fd91118, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "3.ic173",       0x040000, 0xc9c6e720, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "4.ic176",       0x040000, 0xaf352703, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "2.ic175",       0x040000, 0x1eb91343, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	

	{ "kr_gfx1.rom",   0x080000, 0x9e36c1a4, BRF_GRA | CPS1_TILES },
	{ "kr_gfx3.rom",   0x080000, 0xc5832cae, BRF_GRA | CPS1_TILES },
	{ "kr_gfx2.rom",   0x080000, 0xf095be2d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx4.rom",   0x080000, 0x179dfd96, BRF_GRA | CPS1_TILES },
	{ "kr_gfx5.rom",   0x080000, 0x1f4298d2, BRF_GRA | CPS1_TILES },
	{ "kr_gfx7.rom",   0x080000, 0x37fa8751, BRF_GRA | CPS1_TILES },
	{ "kr_gfx6.rom",   0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx8.rom",   0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "1.ic26",        0x040000, 0xbd6f9cc1, BRF_PRG | CPS1_Z80_PROGRAM },
};

STD_ROM_PICK(Knightsb)
STD_ROM_FN(Knightsb)

static struct BurnRomInfo Knightsb2RomDesc[] = {
	{ "040-z.02",      0x080000, 0x95d00a7e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "040-r.02",      0x080000, 0x5a9d0b64, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "kr_gfx1.rom",   0x080000, 0x9e36c1a4, BRF_GRA | CPS1_TILES },
	{ "kr_gfx3.rom",   0x080000, 0xc5832cae, BRF_GRA | CPS1_TILES },
	{ "kr_gfx2.rom",   0x080000, 0xf095be2d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx4.rom",   0x080000, 0x179dfd96, BRF_GRA | CPS1_TILES },
	{ "kr_gfx5.rom",   0x080000, 0x1f4298d2, BRF_GRA | CPS1_TILES },
	{ "kr_gfx7.rom",   0x080000, 0x37fa8751, BRF_GRA | CPS1_TILES },
	{ "kr_gfx6.rom",   0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx8.rom",   0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "kr_09.rom",     0x010000, 0x5e44d9ee, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kr_18.rom",     0x020000, 0xda69d15f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kr_19.rom",     0x020000, 0xbfc654e9, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Knightsb2)
STD_ROM_FN(Knightsb2)

static struct BurnRomInfo Knightsb3RomDesc[] = {
	{ "23.096",        0x080000, 0x7733b8a6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "22.096",        0x080000, 0xd0b671a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "1.096",         0x080000, 0x91541596, BRF_GRA | CPS1_TILES },
	{ "2.096",         0x080000, 0x5d332114, BRF_GRA | CPS1_TILES },
	{ "3.096",         0x080000, 0x60488eba, BRF_GRA | CPS1_TILES },
	{ "4.096",         0x080000, 0x394bdd11, BRF_GRA | CPS1_TILES },
	{ "5.096",         0x080000, 0xb52af98b, BRF_GRA | CPS1_TILES },
	{ "6.096",         0x080000, 0xe44e5eac, BRF_GRA | CPS1_TILES },
	{ "7.096",         0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "8.096",         0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "9.512",         0x010000, 0x5e44d9ee, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18.010",        0x020000, 0x9b9be3ab, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19.010",        0x020000, 0xbfc654e9, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Knightsb3)
STD_ROM_FN(Knightsb3)

static struct BurnRomInfo Knightsb4RomDesc[] = {
	{ "3.bin",         0x080000, 0xb818272c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "2.bin",         0x080000, 0xb0b9a4c2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
    	
	{ "kr_gfx1.rom",   0x080000, 0x9e36c1a4, BRF_GRA | CPS1_TILES },
	{ "kr_gfx3.rom",   0x080000, 0xc5832cae, BRF_GRA | CPS1_TILES },
	{ "kr_gfx2.rom",   0x080000, 0xf095be2d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx4.rom",   0x080000, 0x179dfd96, BRF_GRA | CPS1_TILES },
	{ "kr_gfx5.rom",   0x080000, 0x1f4298d2, BRF_GRA | CPS1_TILES },
	{ "kr_gfx7.rom",   0x080000, 0x37fa8751, BRF_GRA | CPS1_TILES },
	{ "kr_gfx6.rom",   0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx8.rom",   0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "1.bin",         0x040000, 0xbd6f9cc1, BRF_PRG | CPS1_Z80_PROGRAM },
};

STD_ROM_PICK(Knightsb4)
STD_ROM_FN(Knightsb4)

static struct BurnRomInfo KnightshRomDesc[] = {
	{ "krh_23.rom",    0x080000, 0xfa2ff63d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "krh_22.rom",    0x080000, 0x1438d070, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "kr_gfx1.rom",   0x080000, 0x9e36c1a4, BRF_GRA | CPS1_TILES },
	{ "kr_gfx3.rom",   0x080000, 0xc5832cae, BRF_GRA | CPS1_TILES },
	{ "kr_gfx2.rom",   0x080000, 0xf095be2d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx4.rom",   0x080000, 0x179dfd96, BRF_GRA | CPS1_TILES },
	{ "kr_gfx5.rom",   0x080000, 0x1f4298d2, BRF_GRA | CPS1_TILES },
	{ "kr_gfx7.rom",   0x080000, 0x37fa8751, BRF_GRA | CPS1_TILES },
	{ "kr_gfx6.rom",   0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx8.rom",   0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "kr_09.rom",     0x010000, 0x5e44d9ee, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kr_18.rom",     0x020000, 0xda69d15f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kr_19.rom",     0x020000, 0xbfc654e9, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Knightsh)
STD_ROM_FN(Knightsh)

static struct BurnRomInfo Knightsh2RomDesc[] = {
	{ "krha_23.rom",   0x080000, 0xa7fd309a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "krh_22.rom",    0x080000, 0x1438d070, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "kr_gfx1.rom",   0x080000, 0x9e36c1a4, BRF_GRA | CPS1_TILES },
	{ "kr_gfx3.rom",   0x080000, 0xc5832cae, BRF_GRA | CPS1_TILES },
	{ "kr_gfx2.rom",   0x080000, 0xf095be2d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx4.rom",   0x080000, 0x179dfd96, BRF_GRA | CPS1_TILES },
	{ "kr_gfx5.rom",   0x080000, 0x1f4298d2, BRF_GRA | CPS1_TILES },
	{ "kr_gfx7.rom",   0x080000, 0x37fa8751, BRF_GRA | CPS1_TILES },
	{ "kr_gfx6.rom",   0x080000, 0x0200bc3d, BRF_GRA | CPS1_TILES },
	{ "kr_gfx8.rom",   0x080000, 0x0bb2b4e7, BRF_GRA | CPS1_TILES },

	{ "kr_09.rom",     0x010000, 0x5e44d9ee, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kr_18.rom",     0x020000, 0xda69d15f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kr_19.rom",     0x020000, 0xbfc654e9, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Knightsh2)
STD_ROM_FN(Knightsh2)

static struct BurnRomInfo KodRomDesc[] = {
	{ "kde_30a.11e",   0x020000, 0xfcb5efe2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_37a.11f",   0x020000, 0xf22e5266, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_31a.12e",   0x020000, 0xc710d722, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_38a.12f",   0x020000, 0x57d6ed3a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_28.9e",      0x020000, 0x9367bcd9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_35.9f",      0x020000, 0x4ca6a48a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_29.10e",     0x020000, 0x0360fa72, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_36a.10f",    0x020000, 0x95a3cef8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "kd-5m.4a",      0x080000, 0xe45b8701, BRF_GRA | CPS1_TILES },
	{ "kd-7m.6a",      0x080000, 0xa7750322, BRF_GRA | CPS1_TILES },
	{ "kd-1m.3a",      0x080000, 0x5f74bf78, BRF_GRA | CPS1_TILES },
	{ "kd-3m.5a",      0x080000, 0x5e5303bf, BRF_GRA | CPS1_TILES },
	{ "kd-6m.4c",      0x080000, 0x113358f3, BRF_GRA | CPS1_TILES },
	{ "kd-8m.6c",      0x080000, 0x38853c44, BRF_GRA | CPS1_TILES },
	{ "kd-2m.3c",      0x080000, 0x9ef36604, BRF_GRA | CPS1_TILES },
	{ "kd-4m.5c",      0x080000, 0x402b9b4f, BRF_GRA | CPS1_TILES },

	{ "kd_9.12a",      0x010000, 0xbac6ec26, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kd_18.11c",     0x020000, 0x4c63181d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kd_19.12c",     0x020000, 0x92941b80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kd29b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Kod)
STD_ROM_FN(Kod)

static struct BurnRomInfo Kodr1RomDesc[] = {
	{ "kde_30.11e",    0x020000, 0xc7414fd4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_37.11f",    0x020000, 0xa5bf40d2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_31.12e",    0x020000, 0x1fffc7bd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_38.12f",    0x020000, 0x89e57a82, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_28.9e",     0x020000, 0x9367bcd9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_35.9f",     0x020000, 0x4ca6a48a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_29.10e",    0x020000, 0x6a0ba878, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kde_36.10f",    0x020000, 0xb509b39d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "kd-5m.4a",      0x080000, 0xe45b8701, BRF_GRA | CPS1_TILES },
	{ "kd-7m.6a",      0x080000, 0xa7750322, BRF_GRA | CPS1_TILES },
	{ "kd-1m.3a",      0x080000, 0x5f74bf78, BRF_GRA | CPS1_TILES },
	{ "kd-3m.5a",      0x080000, 0x5e5303bf, BRF_GRA | CPS1_TILES },
	{ "kd-6m.4c",      0x080000, 0x113358f3, BRF_GRA | CPS1_TILES },
	{ "kd-8m.6c",      0x080000, 0x38853c44, BRF_GRA | CPS1_TILES },
	{ "kd-2m.3c",      0x080000, 0x9ef36604, BRF_GRA | CPS1_TILES },
	{ "kd-4m.5c",      0x080000, 0x402b9b4f, BRF_GRA | CPS1_TILES },

	{ "kd_9.9",        0x010000, 0xf5514510, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kd_18.18",      0x020000, 0x69ecb2c8, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kd_19.19",      0x020000, 0x02d851c1, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kd29b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Kodr1)
STD_ROM_FN(Kodr1)

static struct BurnRomInfo KoduRomDesc[] = {
	{ "kdu_30b.11e",   0x020000, 0x825817f9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdu_37b.11f",   0x020000, 0xd2422dfb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdu_31b.12e",   0x020000, 0x9af36039, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdu_38b.12f",   0x020000, 0xbe8405a1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdu_28.9e",     0x020000, 0x9367bcd9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdu_35.9f",     0x020000, 0x4ca6a48a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdu_29.10e",    0x020000, 0x0360fa72, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdu_36a.10f",   0x020000, 0x95a3cef8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "kd-5m.4a",      0x080000, 0xe45b8701, BRF_GRA | CPS1_TILES },
	{ "kd-7m.6a",      0x080000, 0xa7750322, BRF_GRA | CPS1_TILES },
	{ "kd-1m.3a",      0x080000, 0x5f74bf78, BRF_GRA | CPS1_TILES },
	{ "kd-3m.5a",      0x080000, 0x5e5303bf, BRF_GRA | CPS1_TILES },
	{ "kd-6m.4c",      0x080000, 0x113358f3, BRF_GRA | CPS1_TILES },
	{ "kd-8m.6c",      0x080000, 0x38853c44, BRF_GRA | CPS1_TILES },
	{ "kd-2m.3c",      0x080000, 0x9ef36604, BRF_GRA | CPS1_TILES },
	{ "kd-4m.5c",      0x080000, 0x402b9b4f, BRF_GRA | CPS1_TILES },

	{ "kd_09.12a",     0x010000, 0xbac6ec26, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kd_18.11c",     0x020000, 0x4c63181d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kd_19.12c",     0x020000, 0x92941b80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kd29b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Kodu)
STD_ROM_FN(Kodu)

static struct BurnRomInfo KodjRomDesc[] = {
	{ "kdj_30a.11e",   0x020000, 0xebc788ad, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdj_37a.11f",   0x020000, 0xe55c3529, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdj_31a.12e",   0x020000, 0xc710d722, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdj_38a.12f",   0x020000, 0x57d6ed3a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_33.6f",      0x080000, 0x9bd7ad4b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "kd_06.8a",      0x080000, 0xe45b8701, BRF_GRA | CPS1_TILES },
	{ "kd_08.10a",     0x080000, 0xa7750322, BRF_GRA | CPS1_TILES },
	{ "kd_05.7a",      0x080000, 0x5f74bf78, BRF_GRA | CPS1_TILES },
	{ "kd_07.9a",      0x080000, 0x5e5303bf, BRF_GRA | CPS1_TILES },
	{ "kd_15.8c",      0x080000, 0x113358f3, BRF_GRA | CPS1_TILES },
	{ "kd_17.10c",     0x080000, 0x38853c44, BRF_GRA | CPS1_TILES },
	{ "kd_14.7c",      0x080000, 0x9ef36604, BRF_GRA | CPS1_TILES },
	{ "kd_16.9c",      0x080000, 0x402b9b4f, BRF_GRA | CPS1_TILES },

	{ "kd_09.12a",     0x010000, 0xbac6ec26, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kd_18.11c",     0x020000, 0x4c63181d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kd_19.12c",     0x020000, 0x92941b80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kd29b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Kodj)
STD_ROM_FN(Kodj)

static struct BurnRomInfo KodjaRomDesc[] = {
	{ "kdj_36a.12f",   0x020000, 0xebc788ad, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdj_42a.12h",   0x020000, 0xe55c3529, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdj_37a.13f",   0x020000, 0xc710d722, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kdj_43a.13h",   0x020000, 0x57d6ed3a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_34.10f",     0x020000, 0x9367bcd9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_40.10h",     0x020000, 0x4ca6a48a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_35.11f",     0x020000, 0x0360fa72, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "kd_41a.11h",    0x020000, 0x95a3cef8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "kd_9.4b",       0x020000, 0x401a98e3, BRF_GRA | CPS1_TILES },
	{ "kd_1.4a",       0x020000, 0x5894399a, BRF_GRA | CPS1_TILES },
	{ "kd_13.9b",      0x020000, 0xb6685131, BRF_GRA | CPS1_TILES },
	{ "kd_5.9a",       0x020000, 0xc29b9ab3, BRF_GRA | CPS1_TILES },
	{ "kd_24.5e",      0x020000, 0x97008fdb, BRF_GRA | CPS1_TILES },
	{ "kd_17.5c",      0x020000, 0xdc9a83d3, BRF_GRA | CPS1_TILES },
	{ "kd_38.8h",      0x020000, 0x9c3dd2d1, BRF_GRA | CPS1_TILES },
	{ "kd_32.8f",      0x020000, 0x1b2a802a, BRF_GRA | CPS1_TILES },
	{ "kd_10.5b",      0x020000, 0xe788ae96, BRF_GRA | CPS1_TILES },
	{ "kd_2.5a",       0x020000, 0xb022e3e3, BRF_GRA | CPS1_TILES },
	{ "kd_14.10b",     0x020000, 0x4840c5ef, BRF_GRA | CPS1_TILES },
	{ "kd_6.10a",      0x020000, 0x519faee4, BRF_GRA | CPS1_TILES },
	{ "kd_25.7e",      0x020000, 0x5d0fa853, BRF_GRA | CPS1_TILES },
	{ "kd_18.7c",      0x020000, 0x6ad3b2bb, BRF_GRA | CPS1_TILES },
	{ "kd_39.9h",      0x020000, 0xd7920213, BRF_GRA | CPS1_TILES },
	{ "kd_33.9f",      0x020000, 0x65c2bed6, BRF_GRA | CPS1_TILES },
	{ "kd_11.7b",      0x020000, 0x147e3310, BRF_GRA | CPS1_TILES },
	{ "kd_3.7a",       0x020000, 0x5d18bc83, BRF_GRA | CPS1_TILES },
	{ "kd_15.11b",     0x020000, 0x57359746, BRF_GRA | CPS1_TILES },
	{ "kd_7.11a",      0x020000, 0x7fe03079, BRF_GRA | CPS1_TILES },
	{ "kd_26.8e",      0x020000, 0x57e5fab5, BRF_GRA | CPS1_TILES },
	{ "kd_19.8c",      0x020000, 0xb1f30f7c, BRF_GRA | CPS1_TILES },
	{ "kd_28.10e",     0x020000, 0x3a424135, BRF_GRA | CPS1_TILES },
	{ "kd_21.10c",     0x020000, 0xce10d2c3, BRF_GRA | CPS1_TILES },
	{ "kd_12.8b",      0x020000, 0xa6042aa2, BRF_GRA | CPS1_TILES },
	{ "kd_4.8a",       0x020000, 0x0ce0ba30, BRF_GRA | CPS1_TILES },
	{ "kd_16.12b",     0x020000, 0x63dcb7e0, BRF_GRA | CPS1_TILES },
	{ "kd_8.12a",      0x020000, 0xc69b77ae, BRF_GRA | CPS1_TILES },
	{ "kd_27.9e",      0x020000, 0x40d7bfed, BRF_GRA | CPS1_TILES },
	{ "kd_20.9c",      0x020000, 0x01c1f399, BRF_GRA | CPS1_TILES },
	{ "kd_29.11e",     0x020000, 0xa1eeac03, BRF_GRA | CPS1_TILES },
	{ "kd_22.11c",     0x020000, 0x5ade98eb, BRF_GRA | CPS1_TILES },

	{ "kd_23.13b",     0x010000, 0xbac6ec26, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kd_30.12c",     0x020000, 0x4c63181d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kd_31.13c",     0x020000, 0x92941b80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kd22b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },// b-board PLDs
	{ "iob1.12e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Kodja)
STD_ROM_FN(Kodja)

static struct BurnRomInfo KodbRomDesc[] = {
	{ "3.ic172",       0x080000, 0x036dd74c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "4.ic171",       0x080000, 0x3e4b7295, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "cp.ic90",       0x080000, 0xe3b8589e, BRF_GRA | CPS1_TILES },
	{ "dp.ic89",       0x080000, 0x3eec9580, BRF_GRA | CPS1_TILES },
	{ "ap.ic88",       0x080000, 0xfdf5f163, BRF_GRA | CPS1_TILES },
	{ "bp.ic87",       0x080000, 0x4e1c52b7, BRF_GRA | CPS1_TILES },
	{ "ci.ic91",       0x080000, 0x22228bc5, BRF_GRA | CPS1_TILES },
	{ "di.ic92",       0x080000, 0xab031763, BRF_GRA | CPS1_TILES },
	{ "ai.ic93",       0x080000, 0xcffbf4be, BRF_GRA | CPS1_TILES },
	{ "bi.ic94",       0x080000, 0x4a1b43fe, BRF_GRA | CPS1_TILES },

	{ "1.ic28",        0x010000, 0x01cae60c, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "2.ic19",        0x040000, 0xa2db1575, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Kodb)
STD_ROM_FN(Kodb)

static struct BurnRomInfo KodhRomDesc[] = {
	{ "23.096",        0x080000, 0xdaf89cfb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "22.096",        0x080000, 0xc83e19d8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "1.096",         0x080000, 0x09261881, BRF_GRA | CPS1_TILES },
	{ "2.096",         0x080000, 0xbc121ff2, BRF_GRA | CPS1_TILES },
	{ "3.096",         0x080000, 0xf463ae22, BRF_GRA | CPS1_TILES },
	{ "4.096",         0x080000, 0x01308733, BRF_GRA | CPS1_TILES },
	{ "kod11.rom",     0x080000, 0x113358f3, BRF_GRA | CPS1_TILES },
	{ "kod13.rom",     0x080000, 0x38853c44, BRF_GRA | CPS1_TILES },
	{ "7.096",         0x080000, 0xc7ab4704, BRF_GRA | CPS1_TILES },
	{ "kod12.rom",     0x080000, 0x402b9b4f, BRF_GRA | CPS1_TILES },

	{ "kd09.bin",      0x010000, 0xbac6ec26, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kd18.bin",      0x020000, 0x4c63181d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kd19.bin",      0x020000, 0x92941b80, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Kodh)
STD_ROM_FN(Kodh)

// King Of Dragons 910731 ETC Phoenix set

static struct BurnRomInfo KoddaRomDesc[] = {
	{ "8f",            0x080000, 0x0a6ab826, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "7f",            0x080000, 0x9bd7ad4b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	
	{ "kd-5m.4a",      0x080000, 0xe45b8701, BRF_GRA | CPS1_TILES },
	{ "kd-7m.6a",      0x080000, 0xa7750322, BRF_GRA | CPS1_TILES },
	{ "kd-1m.3a",      0x080000, 0x5f74bf78, BRF_GRA | CPS1_TILES },
	{ "kdda-7m.6a",    0x080000, 0x0e2cb76f, BRF_GRA | CPS1_TILES },
	{ "kd-6m.4c",      0x080000, 0x113358f3, BRF_GRA | CPS1_TILES },
	{ "kd-8m.6c",      0x080000, 0x38853c44, BRF_GRA | CPS1_TILES },
	{ "kd-2m.3c",      0x080000, 0x9ef36604, BRF_GRA | CPS1_TILES },
	{ "kd-4m.5c",      0x080000, 0x402b9b4f, BRF_GRA | CPS1_TILES },
	
	{ "kd_9.12a",      0x010000, 0xbac6ec26, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "kd_18.11c",     0x020000, 0x4c63181d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "kd_19.12c",     0x020000, 0x92941b80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "kd29b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Kodda)
STD_ROM_FN(Kodda)

static struct BurnRomInfo MegamanRomDesc[] = {
	{ "rcmu_23b.8f",   0x080000, 0x1cd33c7a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "rcmu_22b.7f",   0x080000, 0x708268c4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "rcmu_21a.6f",   0x080000, 0x4376ea95, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "rcm_01.3a",     0x080000, 0x6ecdf13f, BRF_GRA | CPS1_TILES },
	{ "rcm_02.4a",     0x080000, 0x944d4f0f, BRF_GRA | CPS1_TILES },
	{ "rcm_03.5a",     0x080000, 0x36f3073c, BRF_GRA | CPS1_TILES },
	{ "rcm_04.6a",     0x080000, 0x54e622ff, BRF_GRA | CPS1_TILES },
	{ "rcm_05.7a",     0x080000, 0x5dd131fd, BRF_GRA | CPS1_TILES },
	{ "rcm_06.8a",     0x080000, 0xf0faf813, BRF_GRA | CPS1_TILES },
	{ "rcm_07.9a",     0x080000, 0x826de013, BRF_GRA | CPS1_TILES },
	{ "rcm_08.10a",    0x080000, 0xfbff64cf, BRF_GRA | CPS1_TILES },
	{ "rcm_10.3c",     0x080000, 0x4dc8ada9, BRF_GRA | CPS1_TILES },
	{ "rcm_11.4c",     0x080000, 0xf2b9ee06, BRF_GRA | CPS1_TILES },
	{ "rcm_12.5c",     0x080000, 0xfed5f203, BRF_GRA | CPS1_TILES },
	{ "rcm_13.6c",     0x080000, 0x5069d4a9, BRF_GRA | CPS1_TILES },
	{ "rcm_14.7c",     0x080000, 0x303be3bd, BRF_GRA | CPS1_TILES },
	{ "rcm_15.8c",     0x080000, 0x4f2d372f, BRF_GRA | CPS1_TILES },
	{ "rcm_16.9c",     0x080000, 0x93d97fde, BRF_GRA | CPS1_TILES },
	{ "rcm_17.10c",    0x080000, 0x92371042, BRF_GRA | CPS1_TILES },

	{ "rcm_09.11a",    0x010000, 0x22ac8f5f, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "rcm_18.11c",    0x020000, 0x80f1f8aa, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "rcm_19.12c",    0x020000, 0xf257dbe1, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "rcm63b.1a",     0x000117, 0x84acd494, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Megaman)
STD_ROM_FN(Megaman)

static struct BurnRomInfo MegamanaRomDesc[] = {
	{ "rcma_23b.8f",   0x080000, 0x61e4a397, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "rcma_22b.7f",   0x080000, 0x708268c4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "rcma_21a.6f",   0x080000, 0x4376ea95, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "rcm_01.3a",     0x080000, 0x6ecdf13f, BRF_GRA | CPS1_TILES },
	{ "rcm_02.4a",     0x080000, 0x944d4f0f, BRF_GRA | CPS1_TILES },
	{ "rcm_03.5a",     0x080000, 0x36f3073c, BRF_GRA | CPS1_TILES },
	{ "rcm_04.6a",     0x080000, 0x54e622ff, BRF_GRA | CPS1_TILES },
	{ "rcm_05.7a",     0x080000, 0x5dd131fd, BRF_GRA | CPS1_TILES },
	{ "rcm_06.8a",     0x080000, 0xf0faf813, BRF_GRA | CPS1_TILES },
	{ "rcm_07.9a",     0x080000, 0x826de013, BRF_GRA | CPS1_TILES },
	{ "rcm_08.10a",    0x080000, 0xfbff64cf, BRF_GRA | CPS1_TILES },
	{ "rcm_10.3c",     0x080000, 0x4dc8ada9, BRF_GRA | CPS1_TILES },
	{ "rcm_11.4c",     0x080000, 0xf2b9ee06, BRF_GRA | CPS1_TILES },
	{ "rcm_12.5c",     0x080000, 0xfed5f203, BRF_GRA | CPS1_TILES },
	{ "rcm_13.6c",     0x080000, 0x5069d4a9, BRF_GRA | CPS1_TILES },
	{ "rcm_14.7c",     0x080000, 0x303be3bd, BRF_GRA | CPS1_TILES },
	{ "rcm_15.8c",     0x080000, 0x4f2d372f, BRF_GRA | CPS1_TILES },
	{ "rcm_16.9c",     0x080000, 0x93d97fde, BRF_GRA | CPS1_TILES },
	{ "rcm_17.10c",    0x080000, 0x92371042, BRF_GRA | CPS1_TILES },

	{ "rcm_09.12a",    0x020000, 0x9632d6ef, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "rcm_18.11c",    0x020000, 0x80f1f8aa, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "rcm_19.12c",    0x020000, 0xf257dbe1, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "rcm63b.1a",     0x000117, 0x84acd494, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Megamana)
STD_ROM_FN(Megamana)

static struct BurnRomInfo RockmanjRomDesc[] = {
	{ "rcm_23a.8f",    0x080000, 0xefd96cb2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "rcm_22a.7f",    0x080000, 0x8729a689, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "rcm_21a.6f",    0x080000, 0x517ccde2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "rcm_01.3a",     0x080000, 0x6ecdf13f, BRF_GRA | CPS1_TILES },
	{ "rcm_02.4a",     0x080000, 0x944d4f0f, BRF_GRA | CPS1_TILES },
	{ "rcm_03.5a",     0x080000, 0x36f3073c, BRF_GRA | CPS1_TILES },
	{ "rcm_04.6a",     0x080000, 0x54e622ff, BRF_GRA | CPS1_TILES },
	{ "rcm_05.7a",     0x080000, 0x5dd131fd, BRF_GRA | CPS1_TILES },
	{ "rcm_06.8a",     0x080000, 0xf0faf813, BRF_GRA | CPS1_TILES },
	{ "rcm_07.9a",     0x080000, 0x826de013, BRF_GRA | CPS1_TILES },
	{ "rcm_08.10a",    0x080000, 0xfbff64cf, BRF_GRA | CPS1_TILES },
	{ "rcm_10.3c",     0x080000, 0x4dc8ada9, BRF_GRA | CPS1_TILES },
	{ "rcm_11.4c",     0x080000, 0xf2b9ee06, BRF_GRA | CPS1_TILES },
	{ "rcm_12.5c",     0x080000, 0xfed5f203, BRF_GRA | CPS1_TILES },
	{ "rcm_13.6c",     0x080000, 0x5069d4a9, BRF_GRA | CPS1_TILES },
	{ "rcm_14.7c",     0x080000, 0x303be3bd, BRF_GRA | CPS1_TILES },
	{ "rcm_15.8c",     0x080000, 0x4f2d372f, BRF_GRA | CPS1_TILES },
	{ "rcm_16.9c",     0x080000, 0x93d97fde, BRF_GRA | CPS1_TILES },
	{ "rcm_17.10c",    0x080000, 0x92371042, BRF_GRA | CPS1_TILES },

	{ "rcm_09.12a",    0x020000, 0x9632d6ef, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "rcm_18.11c",    0x020000, 0x80f1f8aa, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "rcm_19.12c",    0x020000, 0xf257dbe1, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "rcm63b.1a",     0x000117, 0x84acd494, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Rockmanj)
STD_ROM_FN(Rockmanj)

static struct BurnRomInfo MercsRomDesc[] = {
	{ "so2_30e.11f",   0x020000, 0xe17f9bf7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_35e.11h",   0x020000, 0x78e63575, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_31e.12f",   0x020000, 0x51204d36, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_36e.12h",   0x020000, 0x9cfba8b4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2-32m.8h",    0x080000, 0x2eb5cf0c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "so2-6m.8a",     0x080000, 0xaa6102af, BRF_GRA | CPS1_TILES },
	{ "so2-8m.10a",    0x080000, 0x839e6869, BRF_GRA | CPS1_TILES },
	{ "so2-2m.4a",     0x080000, 0x597c2875, BRF_GRA | CPS1_TILES },
	{ "so2-4m.6a",     0x080000, 0x912a9ca0, BRF_GRA | CPS1_TILES },
	{ "so2_24.7d",     0x020000, 0x3f254efe, BRF_GRA | CPS1_TILES },
	{ "so2_14.7c",     0x020000, 0xf5a8905e, BRF_GRA | CPS1_TILES },
	{ "so2_26.9d",     0x020000, 0xf3aa5a4a, BRF_GRA | CPS1_TILES },
	{ "so2_16.9c",     0x020000, 0xb43cd1a8, BRF_GRA | CPS1_TILES },
	{ "so2_20.3d",     0x020000, 0x8ca751a3, BRF_GRA | CPS1_TILES },
	{ "so2_10.3c",     0x020000, 0xe9f569fd, BRF_GRA | CPS1_TILES },
	{ "so2_22.5d",     0x020000, 0xfce9a377, BRF_GRA | CPS1_TILES },
	{ "so2_12.5c",     0x020000, 0xb7df8a06, BRF_GRA | CPS1_TILES },

	{ "so2_09.12b",    0x010000, 0xd09d7c7a, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "so2_18.11c",    0x020000, 0xbbea1643, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "so2_19.12c",    0x020000, 0xac58aa71, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "o224b.1a",      0x000117, 0xc211c8cd, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c628",          0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Mercs)
STD_ROM_FN(Mercs)

static struct BurnRomInfo MercsuRomDesc[] = {
	{ "so2_30a.11f",   0x020000, 0xe4e725d7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_35a.11h",   0x020000, 0xe7843445, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_31a.12f",   0x020000, 0xc0b91dea, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_36a.12h",   0x020000, 0x591edf6c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2-32m.8h",    0x080000, 0x2eb5cf0c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "so2-6m.8a",     0x080000, 0xaa6102af, BRF_GRA | CPS1_TILES },
	{ "so2-8m.10a",    0x080000, 0x839e6869, BRF_GRA | CPS1_TILES },
	{ "so2-2m.4a",     0x080000, 0x597c2875, BRF_GRA | CPS1_TILES },
	{ "so2-4m.6a",     0x080000, 0x912a9ca0, BRF_GRA | CPS1_TILES },
	{ "so2_24.7d",     0x020000, 0x3f254efe, BRF_GRA | CPS1_TILES },
	{ "so2_14.7c",     0x020000, 0xf5a8905e, BRF_GRA | CPS1_TILES },
	{ "so2_26.9d",     0x020000, 0xf3aa5a4a, BRF_GRA | CPS1_TILES },
	{ "so2_16.9c",     0x020000, 0xb43cd1a8, BRF_GRA | CPS1_TILES },
	{ "so2_20.3d",     0x020000, 0x8ca751a3, BRF_GRA | CPS1_TILES },
	{ "so2_10.3c",     0x020000, 0xe9f569fd, BRF_GRA | CPS1_TILES },
	{ "so2_22.5d",     0x020000, 0xfce9a377, BRF_GRA | CPS1_TILES },
	{ "so2_12.5c",     0x020000, 0xb7df8a06, BRF_GRA | CPS1_TILES },

	{ "so2_09.12b",    0x010000, 0xd09d7c7a, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "so2_18.11c",    0x020000, 0xbbea1643, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "so2_19.12c",    0x020000, 0xac58aa71, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "o224b.1a",      0x000117, 0xc211c8cd, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c628",          0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Mercsu)
STD_ROM_FN(Mercsu)

static struct BurnRomInfo Mercsur1RomDesc[] = {
	{ "so2_30.11f",    0x020000, 0xe17f9bf7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_35.11h",    0x020000, 0x4477df61, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_31.12f",    0x020000, 0x51204d36, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_36.12h",    0x020000, 0x9cfba8b4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2-32m.8h",    0x080000, 0x2eb5cf0c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "so2-6m.8a",     0x080000, 0xaa6102af, BRF_GRA | CPS1_TILES },
	{ "so2-8m.10a",    0x080000, 0x839e6869, BRF_GRA | CPS1_TILES },
	{ "so2-2m.4a",     0x080000, 0x597c2875, BRF_GRA | CPS1_TILES },
	{ "so2-4m.6a",     0x080000, 0x912a9ca0, BRF_GRA | CPS1_TILES },
	{ "so2_24.7d",     0x020000, 0x3f254efe, BRF_GRA | CPS1_TILES },
	{ "so2_14.7c",     0x020000, 0xf5a8905e, BRF_GRA | CPS1_TILES },
	{ "so2_26.9d",     0x020000, 0xf3aa5a4a, BRF_GRA | CPS1_TILES },
	{ "so2_16.9c",     0x020000, 0xb43cd1a8, BRF_GRA | CPS1_TILES },
	{ "so2_20.3d",     0x020000, 0x8ca751a3, BRF_GRA | CPS1_TILES },
	{ "so2_10.3c",     0x020000, 0xe9f569fd, BRF_GRA | CPS1_TILES },
	{ "so2_22.5d",     0x020000, 0xfce9a377, BRF_GRA | CPS1_TILES },
	{ "so2_12.5c",     0x020000, 0xb7df8a06, BRF_GRA | CPS1_TILES },

	{ "so2_09.12b",    0x010000, 0xd09d7c7a, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "so2_18.11c",    0x020000, 0xbbea1643, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "so2_19.12c",    0x020000, 0xac58aa71, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "o224b.1a",      0x000117, 0xc211c8cd, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c628",          0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Mercsur1)
STD_ROM_FN(Mercsur1)

static struct BurnRomInfo MercsjRomDesc[] = {
	{ "so2_36.12f",    0x020000, 0xe17f9bf7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_42.12h",    0x020000, 0x2c3884c6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_37.13f",    0x020000, 0x51204d36, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_43.13h",    0x020000, 0x9cfba8b4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_34.10f",    0x020000, 0xb8dae95f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_40.10h",    0x020000, 0xde37771c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_35.11f",    0x020000, 0x7d24394d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "so2_41.11h",    0x020000, 0x914f85e0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "so2_09.4b",     0x020000, 0x690c261d, BRF_GRA | CPS1_TILES },
	{ "so2_01.4a",     0x020000, 0x31fd2715, BRF_GRA | CPS1_TILES },
	{ "so2_13.9b",     0x020000, 0xb5e48282, BRF_GRA | CPS1_TILES },
	{ "so2_05.9a",     0x020000, 0x54bed82c, BRF_GRA | CPS1_TILES },
	{ "so2_24.5e",     0x020000, 0x78b6f0cb, BRF_GRA | CPS1_TILES },
	{ "so2_17.5c",     0x020000, 0xe78bb308, BRF_GRA | CPS1_TILES },
	{ "so2_38.8h",     0x020000, 0x0010a9a2, BRF_GRA | CPS1_TILES },
	{ "so2_32.8f",     0x020000, 0x75dffc9a, BRF_GRA | CPS1_TILES },
	{ "so2_10.5b",     0x020000, 0x2f871714, BRF_GRA | CPS1_TILES },
	{ "so2_02.5a",     0x020000, 0xb4b2a0b7, BRF_GRA | CPS1_TILES },
	{ "so2_14.10b",    0x020000, 0x737a744b, BRF_GRA | CPS1_TILES },
	{ "so2_06.10a",    0x020000, 0x9d756f51, BRF_GRA | CPS1_TILES },
	{ "so2_25.7e",     0x020000, 0x6d0e05d6, BRF_GRA | CPS1_TILES },
	{ "so2_18.7c",     0x020000, 0x96f61f4e, BRF_GRA | CPS1_TILES },
	{ "so2_39.9h",     0x020000, 0xd52ba336, BRF_GRA | CPS1_TILES },
	{ "so2_33.9f",     0x020000, 0x39b90d25, BRF_GRA | CPS1_TILES },
	{ "so2_11.7b",     0x020000, 0x3f254efe, BRF_GRA | CPS1_TILES },
	{ "so2_03.7a",     0x020000, 0xf5a8905e, BRF_GRA | CPS1_TILES },
	{ "so2_15.11b",    0x020000, 0xf3aa5a4a, BRF_GRA | CPS1_TILES },
	{ "so2_07.11a",    0x020000, 0xb43cd1a8, BRF_GRA | CPS1_TILES },
	{ "so2_26.8e",     0x020000, 0x8ca751a3, BRF_GRA | CPS1_TILES },
	{ "so2_19.8c",     0x020000, 0xe9f569fd, BRF_GRA | CPS1_TILES },
	{ "so2_28.10e",    0x020000, 0xfce9a377, BRF_GRA | CPS1_TILES },
	{ "so2_21.10c",    0x020000, 0xb7df8a06, BRF_GRA | CPS1_TILES },

	{ "so2_23.13b",    0x010000, 0xd09d7c7a, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "so2_30.12c",    0x020000, 0xbbea1643, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "so2_31.13c",    0x020000, 0xac58aa71, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "o222b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12e",      0x000117, 0xad52b90c, BRF_OPT },
	{ "c628",          0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Mercsj)
STD_ROM_FN(Mercsj)

static struct BurnRomInfo MswordRomDesc[] = {
	{ "mse_30.11f",    0x020000, 0x03fc8dbc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mse_35.11h",    0x020000, 0xd5bf66cd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mse_31.12f",    0x020000, 0x30332bcf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mse_36.12h",    0x020000, 0x8f7d6ce9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms-32m.8h",     0x080000, 0x2475ddfc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ms-5m.7a",      0x080000, 0xc00fe7e2, BRF_GRA | CPS1_TILES },
	{ "ms-7m.9a",      0x080000, 0x4ccacac5, BRF_GRA | CPS1_TILES },
	{ "ms-1m.3a",      0x080000, 0x0d2bbe00, BRF_GRA | CPS1_TILES },
	{ "ms-3m.5a",      0x080000, 0x3a1a5bf4, BRF_GRA | CPS1_TILES },

	{ "ms_09.12b",     0x010000, 0x57b29519, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ms_18.11c",     0x020000, 0xfb64e90d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ms_19.12c",     0x020000, 0x74f892b9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ms24b.1a",      0x000117, 0x636dbe6d, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Msword)
STD_ROM_FN(Msword)

static struct BurnRomInfo Mswordr1RomDesc[] = {
	{ "ms_30.11f",     0x020000, 0x21c1f078, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms_35.11h",     0x020000, 0xa540a73a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms_31.12f",     0x020000, 0xd7e762b5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms_36.12h",     0x020000, 0x66f2dcdb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms-32m.8h",     0x080000, 0x2475ddfc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ms-5m.7a",      0x080000, 0xc00fe7e2, BRF_GRA | CPS1_TILES },
	{ "ms-7m.9a",      0x080000, 0x4ccacac5, BRF_GRA | CPS1_TILES },
	{ "ms-1m.3a",      0x080000, 0x0d2bbe00, BRF_GRA | CPS1_TILES },
	{ "ms-3m.5a",      0x080000, 0x3a1a5bf4, BRF_GRA | CPS1_TILES },

	{ "ms_09.12b",     0x010000, 0x57b29519, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ms_18.11c",     0x020000, 0xfb64e90d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ms_19.12c",     0x020000, 0x74f892b9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ms24b.1a",      0x000117, 0x636dbe6d, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Mswordr1)
STD_ROM_FN(Mswordr1)

static struct BurnRomInfo MswordjRomDesc[] = {
	{ "msj_36.12f",    0x020000, 0x04f0ef50, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "msj_42.12h",    0x020000, 0x9fcbb9cd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "msj_37.13f",    0x020000, 0x6c060d70, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "msj_43.13h",    0x020000, 0xaec77787, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms_34.10f",     0x020000, 0x0e59a62d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms_40.10h",     0x020000, 0xbabade3a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms_35.11f",     0x020000, 0x03da99d1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms_41.11h",     0x020000, 0xfadf99ea, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "ms_09.4b",      0x020000, 0x4adee6f6, BRF_GRA | CPS1_TILES },
	{ "ms_01.4a",      0x020000, 0xf7ab1b88, BRF_GRA | CPS1_TILES },
	{ "ms_13.9b",      0x020000, 0xe01adc4b, BRF_GRA | CPS1_TILES },
	{ "ms_05.9a",      0x020000, 0xf62c2369, BRF_GRA | CPS1_TILES },
	{ "ms_24.5e",      0x020000, 0xbe64a3a1, BRF_GRA | CPS1_TILES },
	{ "ms_17.5c",      0x020000, 0x0bc1665f, BRF_GRA | CPS1_TILES },
	{ "ms_38.8h",      0x020000, 0x904a2ed5, BRF_GRA | CPS1_TILES },
	{ "ms_32.8f",      0x020000, 0x3d89c530, BRF_GRA | CPS1_TILES },
	{ "ms_10.5b",      0x020000, 0xf02c0718, BRF_GRA | CPS1_TILES },
	{ "ms_02.5a",      0x020000, 0xd071a405, BRF_GRA | CPS1_TILES },
	{ "ms_14.10b",     0x020000, 0xdfb2e4df, BRF_GRA | CPS1_TILES },
	{ "ms_06.10a",     0x020000, 0xd3ce2a91, BRF_GRA | CPS1_TILES },
	{ "ms_25.7e",      0x020000, 0x0f199d56, BRF_GRA | CPS1_TILES },
	{ "ms_18.7c",      0x020000, 0x1ba76df2, BRF_GRA | CPS1_TILES },
	{ "ms_39.9h",      0x020000, 0x01efce86, BRF_GRA | CPS1_TILES },
	{ "ms_33.9f",      0x020000, 0xce25defc, BRF_GRA | CPS1_TILES },

	{ "ms_23.13b",     0x010000, 0x57b29519, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ms_30.12c",     0x020000, 0xfb64e90d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ms_31.13c",     0x020000, 0x74f892b9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ms22b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Mswordj)
STD_ROM_FN(Mswordj)

static struct BurnRomInfo MsworduRomDesc[] = {
	{ "msu_30.11f",    0x020000, 0xd963c816, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "msu_35.11h",    0x020000, 0x72f179b3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "msu_31.12f",    0x020000, 0x20cd7904, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "msu_36.12h",    0x020000, 0xbf88c080, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ms-32m.8h",     0x080000, 0x2475ddfc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ms-5m.7a",      0x080000, 0xc00fe7e2, BRF_GRA | CPS1_TILES },
	{ "ms-7m.9a",      0x080000, 0x4ccacac5, BRF_GRA | CPS1_TILES },
	{ "ms-1m.3a",      0x080000, 0x0d2bbe00, BRF_GRA | CPS1_TILES },
	{ "ms-3m.5a",      0x080000, 0x3a1a5bf4, BRF_GRA | CPS1_TILES },

	{ "ms_09.12b",     0x010000, 0x57b29519, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ms_18.11c",     0x020000, 0xfb64e90d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ms_19.12c",     0x020000, 0x74f892b9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ms24b.1a",      0x000117, 0x636dbe6d, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Mswordu)
STD_ROM_FN(Mswordu)

static struct BurnRomInfo MtwinsRomDesc[] = {
	{ "che_30.11f",    0x020000, 0x9a2a2db1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "che_35.11h",    0x020000, 0xa7f96b02, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "che_31.12f",    0x020000, 0xbbff8a99, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "che_36.12h",    0x020000, 0x0fa00c39, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ck-32m.8h",     0x080000, 0x9b70bd41, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ck-5m.7a",      0x080000, 0x4ec75f15, BRF_GRA | CPS1_TILES },
	{ "ck-7m.9a",      0x080000, 0xd85d00d6, BRF_GRA | CPS1_TILES },
	{ "ck-1m.3a",      0x080000, 0xf33ca9d4, BRF_GRA | CPS1_TILES },
	{ "ck-3m.5a",      0x080000, 0x0ba2047f, BRF_GRA | CPS1_TILES },

	{ "ch_09.12b",     0x010000, 0x4d4255b7, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ch_18.11c",     0x020000, 0xf909e8de, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ch_19.12c",     0x020000, 0xfc158cf7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ck24b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Mtwins)
STD_ROM_FN(Mtwins)

static struct BurnRomInfo ChikijRomDesc[] = {
	{ "chj_36a.12f",   0x020000, 0xec1328d8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "chj_42a.12h",   0x020000, 0x4ae13503, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "chj_37a.13f",   0x020000, 0x46d2cf7b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "chj_43a.13h",   0x020000, 0x8d387fe8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ch_34.10f",     0x020000, 0x609ed2f9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ch_40.10h",     0x020000, 0xbe0d8301, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ch_35.11f",     0x020000, 0xb810867f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ch_41.11h",     0x020000, 0x8ad96155, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "ch_09.4b",      0x020000, 0x567ab3ca, BRF_GRA | CPS1_TILES },
	{ "ch_01.4a",      0x020000, 0x7f3b7b56, BRF_GRA | CPS1_TILES },
	{ "ch_13.9b",      0x020000, 0x12a7a8ba, BRF_GRA | CPS1_TILES },
	{ "ch_05.9a",      0x020000, 0x6c1afb9a, BRF_GRA | CPS1_TILES },
	{ "ch_24.5e",      0x020000, 0x9cb6e6bc, BRF_GRA | CPS1_TILES },
	{ "ch_17.5c",      0x020000, 0xfe490846, BRF_GRA | CPS1_TILES },
	{ "ch_38.8h",      0x020000, 0x6e5c8cb6, BRF_GRA | CPS1_TILES },
	{ "ch_32.8f",      0x020000, 0x317d27b0, BRF_GRA | CPS1_TILES },
	{ "ch_10.5b",      0x020000, 0xe8251a9b, BRF_GRA | CPS1_TILES },
	{ "ch_02.5a",      0x020000, 0x7c8c88fb, BRF_GRA | CPS1_TILES },
	{ "ch_14.10b",     0x020000, 0x4012ec4b, BRF_GRA | CPS1_TILES },
	{ "ch_06.10a",     0x020000, 0x81884b2b, BRF_GRA | CPS1_TILES },
	{ "ch_25.7e",      0x020000, 0x1dfcbac5, BRF_GRA | CPS1_TILES },
	{ "ch_18.7c",      0x020000, 0x516a34d1, BRF_GRA | CPS1_TILES },
	{ "ch_39.9h",      0x020000, 0x872fb2a4, BRF_GRA | CPS1_TILES },
	{ "ch_33.9f",      0x020000, 0x30dc5ded, BRF_GRA | CPS1_TILES },

	{ "ch_23.13b",     0x010000, 0x4d4255b7, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ch_30.12c",     0x020000, 0xf909e8de, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "ch_31.13c",     0x020000, 0xfc158cf7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ck22b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Chikij)
STD_ROM_FN(Chikij)

static struct BurnRomInfo NemoRomDesc[] = {
	{ "nme_30a.11f",   0x020000, 0xd2c03e56, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nme_35a.11h",   0x020000, 0x5fd31661, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nme_31a.12f",   0x020000, 0xb2bd4f6f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nme_36a.12h",   0x020000, 0xee9450e3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nm-32m.8h",     0x080000, 0xd6d1add3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "nm-5m.7a",      0x080000, 0x487b8747, BRF_GRA | CPS1_TILES },
	{ "nm-7m.9a",      0x080000, 0x203dc8c6, BRF_GRA | CPS1_TILES },
	{ "nm-1m.3a",      0x080000, 0x9e878024, BRF_GRA | CPS1_TILES },
	{ "nm-3m.5a",      0x080000, 0xbb01e6b6, BRF_GRA | CPS1_TILES },

	{ "nme_09.12b",    0x010000, 0x0f4b0581, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "nme_18.11c",    0x020000, 0xbab333d4, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "nme_19.12c",    0x020000, 0x2650a0a8, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "nm24b.1a",      0x000117, 0x7b25bac6, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Nemo)
STD_ROM_FN(Nemo)

static struct BurnRomInfo NemojRomDesc[] = {
	{ "nmj_36a.12f",   0x020000, 0xdaeceabb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nmj_42a.12h",   0x020000, 0x55024740, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nmj_37a.13f",   0x020000, 0x619068b6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nmj_43a.13h",   0x020000, 0xa948a53b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nm_34.10f",     0x020000, 0x5737feed, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nm_40.10h",     0x020000, 0x8a4099f3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nm_35.11f",     0x020000, 0xbd11a7f8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "nm_41.11h",     0x020000, 0x6309603d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "nm_09.4b",      0x020000, 0x9d60d286, BRF_GRA | CPS1_TILES },
	{ "nm_01.4a",      0x020000, 0x8a83f7c4, BRF_GRA | CPS1_TILES },
	{ "nm_13.9b",      0x020000, 0xa4909fe0, BRF_GRA | CPS1_TILES },
	{ "nm_05.9a",      0x020000, 0x16db1e61, BRF_GRA | CPS1_TILES },
	{ "nm_24.5e",      0x020000, 0x3312c648, BRF_GRA | CPS1_TILES },
	{ "nm_17.5c",      0x020000, 0xccfc50e2, BRF_GRA | CPS1_TILES },
	{ "nm_38.8h",      0x020000, 0xae98a997, BRF_GRA | CPS1_TILES },
	{ "nm_32.8f",      0x020000, 0xb3704dde, BRF_GRA | CPS1_TILES },
	{ "nm_10.5b",      0x020000, 0x33c1388c, BRF_GRA | CPS1_TILES },
	{ "nm_02.5a",      0x020000, 0x84c69469, BRF_GRA | CPS1_TILES },
	{ "nm_14.10b",     0x020000, 0x66612270, BRF_GRA | CPS1_TILES },
	{ "nm_06.10a",     0x020000, 0x8b9bcf95, BRF_GRA | CPS1_TILES },
	{ "nm_25.7e",      0x020000, 0xacfc84d2, BRF_GRA | CPS1_TILES },
	{ "nm_18.7c",      0x020000, 0x4347deed, BRF_GRA | CPS1_TILES },
	{ "nm_39.9h",      0x020000, 0x6a274ecd, BRF_GRA | CPS1_TILES },
	{ "nm_33.9f",      0x020000, 0xc469dc74, BRF_GRA | CPS1_TILES },
	
	{ "nm_23.13b",     0x010000, 0x8d3c5a42, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "nm_30.12c",     0x020000, 0xbab333d4, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "nm_31.13c",     0x020000, 0x2650a0a8, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "nm22b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Nemoj)
STD_ROM_FN(Nemoj)

static struct BurnRomInfo Pang3RomDesc[] = {
	{ "pa3e_17a.11l",  0x080000, 0xa213fa80, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "pa3e_16a.10l",  0x080000, 0x7169ea67, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "pa3-01m.2c",    0x200000, 0x068a152c, BRF_GRA | CPS1_TILES },
	{ "pa3-07m.2f",    0x200000, 0x3a4a619d, BRF_GRA | CPS1_TILES },

	{ "pa3_11.11f",    0x020000, 0xcb1423a2, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "pa3_05.10d",    0x020000, 0x73a10d5d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "pa3_06.11d",    0x020000, 0xaffa4f82, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cp1b1f.1f",     0x000117, 0x3979b8e3, BRF_OPT },	// b-board PLDs
	{ "cp1b8k.8k",     0x000117, 0x8a52ea7a, BRF_OPT },
	{ "cp1b9ka.9k",    0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Pang3)
STD_ROM_FN(Pang3)

static struct BurnRomInfo Pang3r1RomDesc[] = {
	{ "pa3e_17.11l",   0x080000, 0xd7041d32, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "pa3e_16.10l",   0x080000, 0x1be9a483, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "pa3-01m.2c",    0x200000, 0x068a152c, BRF_GRA | CPS1_TILES },
	{ "pa3-07m.2f",    0x200000, 0x3a4a619d, BRF_GRA | CPS1_TILES },

	{ "pa3_11.11f",    0x020000, 0xcb1423a2, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "pa3_05.10d",    0x020000, 0x73a10d5d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "pa3_06.11d",    0x020000, 0xaffa4f82, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cp1b1f.1f",     0x000117, 0x3979b8e3, BRF_OPT },	// b-board PLDs
	{ "cp1b8k.8k",     0x000117, 0x8a52ea7a, BRF_OPT },
	{ "cp1b9k.9k",     0x000117, 0xa754bdc3, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Pang3r1)
STD_ROM_FN(Pang3r1)

static struct BurnRomInfo Pang3bRomDesc[] = {
	{ "pa3w_17.11l",   0x080000, 0x12138234, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "pa3w_16.10l",   0x080000, 0xd1ba585c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "pa3-01m.2c",    0x200000, 0x068a152c, BRF_GRA | CPS1_TILES },
	{ "pa3-07m.2f",    0x200000, 0x3a4a619d, BRF_GRA | CPS1_TILES },

	{ "pa3_11.11",     0x008000, 0x90a08c46, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "pa3_05.10d",    0x020000, 0x73a10d5d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "pa3_06.11d",    0x020000, 0xaffa4f82, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cp1b1f.1f",     0x000117, 0x3979b8e3, BRF_OPT },	// b-board PLDs
	{ "cp1b8k.8k",     0x000117, 0x8a52ea7a, BRF_OPT },
	{ "cp1b9k.9k",     0x000117, 0xa754bdc3, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Pang3b)
STD_ROM_FN(Pang3b)

static struct BurnRomInfo Pang3b2RomDesc[] = {
	{ "pa3_17bl.11l",  0x080000, 0x3b5d99de, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "pa3_16bl.10l",  0x080000, 0x1be9a483, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "pa3-01m.2c",    0x200000, 0x068a152c, BRF_GRA | CPS1_TILES },
	{ "pa3-07m.2f",    0x200000, 0x3a4a619d, BRF_GRA | CPS1_TILES },

	{ "pa3_11.11f",    0x020000, 0xcb1423a2, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "pa3_05.10d",    0x020000, 0x73a10d5d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "pa3_06.11d",    0x020000, 0xaffa4f82, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cp1b1f.1f",     0x000117, 0x3979b8e3, BRF_OPT },	// b-board PLDs
	{ "cp1b8k.8k",     0x000117, 0x8a52ea7a, BRF_OPT },
	{ "cp1b9k.9k",     0x000117, 0xa754bdc3, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Pang3b2)
STD_ROM_FN(Pang3b2)

static struct BurnRomInfo Pang3jRomDesc[] = {
	{ "pa3j_17.11l",   0x080000, 0x21f6e51f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "pa3j_16.10l",   0x080000, 0xca1d7897, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "pa3-01m.2c",    0x200000, 0x068a152c, BRF_GRA | CPS1_TILES },
	{ "pa3-07m.2f",    0x200000, 0x3a4a619d, BRF_GRA | CPS1_TILES },

	{ "pa3_11.11f",    0x020000, 0xcb1423a2, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "pa3_05.10d",    0x020000, 0x73a10d5d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "pa3_06.11d",    0x020000, 0xaffa4f82, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "cp1b1f.1f",     0x000117, 0x3979b8e3, BRF_OPT },	// b-board PLDs
	{ "cp1b8k.8k",     0x000117, 0x8a52ea7a, BRF_OPT },
	{ "cp1b9k.9k",     0x000117, 0xa754bdc3, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Pang3j)
STD_ROM_FN(Pang3j)

static struct BurnRomInfo PnickjRomDesc[] = {
	{ "pnij_36.12f",   0x020000, 0x2d4ffb2b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pnij_42.12h",   0x020000, 0xc085dfaf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "pnij_09.4b",    0x020000, 0x48177b0a, BRF_GRA | CPS1_TILES },
	{ "pnij_01.4a",    0x020000, 0x01a0f311, BRF_GRA | CPS1_TILES },
	{ "pnij_13.9b",    0x020000, 0x406451b0, BRF_GRA | CPS1_TILES },
	{ "pnij_05.9a",    0x020000, 0x8c515dc0, BRF_GRA | CPS1_TILES },
	{ "pnij_26.5e",    0x020000, 0xe2af981e, BRF_GRA | CPS1_TILES },
	{ "pnij_18.5c",    0x020000, 0xf17a0e56, BRF_GRA | CPS1_TILES },
	{ "pnij_38.8h",    0x020000, 0xeb75bd8c, BRF_GRA | CPS1_TILES },
	{ "pnij_32.8f",    0x020000, 0x84560bef, BRF_GRA | CPS1_TILES },
	{ "pnij_10.5b",    0x020000, 0xc2acc171, BRF_GRA | CPS1_TILES },
	{ "pnij_02.5a",    0x020000, 0x0e21fc33, BRF_GRA | CPS1_TILES },
	{ "pnij_14.10b",   0x020000, 0x7fe59b19, BRF_GRA | CPS1_TILES },
	{ "pnij_06.10a",   0x020000, 0x79f4bfe3, BRF_GRA | CPS1_TILES },
	{ "pnij_27.7e",    0x020000, 0x83d5cb0e, BRF_GRA | CPS1_TILES },
	{ "pnij_19.7c",    0x020000, 0xaf08b230, BRF_GRA | CPS1_TILES },
	{ "pnij_39.9h",    0x020000, 0x70fbe579, BRF_GRA | CPS1_TILES },
	{ "pnij_33.9f",    0x020000, 0x3ed2c680, BRF_GRA | CPS1_TILES },

	{ "pnij_17.13b",   0x010000, 0xe86f787a, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "pnij_24.12c",   0x020000, 0x5092257d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "pnij_25.13c",   0x020000, 0x22109aaa, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "pkb10b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },					// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Pnickj)
STD_ROM_FN(Pnickj)

static struct BurnRomInfo PunisherRomDesc[] = {
	{ "pse_26.11e",    0x020000, 0x389a99d2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pse_30.11f",    0x020000, 0x68fb06ac, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pse_27.12e",    0x020000, 0x3eb181c3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pse_31.12f",    0x020000, 0x37108e7b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pse_24.9e",     0x020000, 0x0f434414, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pse_28.9f",     0x020000, 0xb732345d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pse_25.10e",    0x020000, 0xb77102e2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pse_29.10f",    0x020000, 0xec037bce, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ps_21.6f",      0x080000, 0x8affa5a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ps-1m.3a",      0x080000, 0x77b7ccab, BRF_GRA | CPS1_TILES },
	{ "ps-3m.5a",      0x080000, 0x0122720b, BRF_GRA | CPS1_TILES },
	{ "ps-2m.4a",      0x080000, 0x64fa58d4, BRF_GRA | CPS1_TILES },
	{ "ps-4m.6a",      0x080000, 0x60da42c8, BRF_GRA | CPS1_TILES },
	{ "ps-5m.7a",      0x080000, 0xc54ea839, BRF_GRA | CPS1_TILES },
	{ "ps-7m.9a",      0x080000, 0x04c5acbd, BRF_GRA | CPS1_TILES },
	{ "ps-6m.8a",      0x080000, 0xa544f4cc, BRF_GRA | CPS1_TILES },
	{ "ps-8m.10a",     0x080000, 0x8f02f436, BRF_GRA | CPS1_TILES },

	{ "ps_q.5k",       0x020000, 0x49ff4446, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ps-q1.1k",      0x080000, 0x31fd8726, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q2.2k",      0x080000, 0x980a9eef, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q3.3k",      0x080000, 0x0dd44491, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q4.4k",      0x080000, 0xbed42f03, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "ps63b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },					// c-board PLDs
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },					// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Punisher)
STD_ROM_FN(Punisher)

static struct BurnRomInfo PunisheruRomDesc[] = {
	{ "psu_26.11e",    0x020000, 0x9236d121, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psu_30.11f",    0x020000, 0x8320e501, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psu_27.12e",    0x020000, 0x61c960a1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psu_31.12f",    0x020000, 0x78d4c298, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psu_24.9e",     0x020000, 0x1cfecad7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psu_28.9f",     0x020000, 0xbdf921c1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psu_25.10e",    0x020000, 0xc51acc94, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psu_29.10f",    0x020000, 0x52dce1ca, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ps_21.6f",      0x080000, 0x8affa5a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ps-1m.3a",      0x080000, 0x77b7ccab, BRF_GRA | CPS1_TILES },
	{ "ps-3m.5a",      0x080000, 0x0122720b, BRF_GRA | CPS1_TILES },
	{ "ps-2m.4a",      0x080000, 0x64fa58d4, BRF_GRA | CPS1_TILES },
	{ "ps-4m.6a",      0x080000, 0x60da42c8, BRF_GRA | CPS1_TILES },
	{ "ps-5m.7a",      0x080000, 0xc54ea839, BRF_GRA | CPS1_TILES },
	{ "ps-7m.9a",      0x080000, 0x04c5acbd, BRF_GRA | CPS1_TILES },
	{ "ps-6m.8a",      0x080000, 0xa544f4cc, BRF_GRA | CPS1_TILES },
	{ "ps-8m.10a",     0x080000, 0x8f02f436, BRF_GRA | CPS1_TILES },

	{ "ps_q.5k",       0x020000, 0x49ff4446, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ps-q1.1k",      0x080000, 0x31fd8726, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q2.2k",      0x080000, 0x980a9eef, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q3.3k",      0x080000, 0x0dd44491, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q4.4k",      0x080000, 0xbed42f03, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "ps63b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },					// c-board PLDs
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },					// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Punisheru)
STD_ROM_FN(Punisheru)

static struct BurnRomInfo PunisherjRomDesc[] = {
	{ "psj_23.8f",     0x080000, 0x6b2fda52, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "psj_22.7f",     0x080000, 0xe01036bc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "psj_21.6f",     0x080000, 0x8affa5a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ps_01.3a",      0x080000, 0x77b7ccab, BRF_GRA | CPS1_TILES },
	{ "ps_02.4a",      0x080000, 0x0122720b, BRF_GRA | CPS1_TILES },
	{ "ps_03.5a",      0x080000, 0x64fa58d4, BRF_GRA | CPS1_TILES },
	{ "ps_04.6a",      0x080000, 0x60da42c8, BRF_GRA | CPS1_TILES },
	{ "ps_05.7a",      0x080000, 0xc54ea839, BRF_GRA | CPS1_TILES },
	{ "ps_06.8a",      0x080000, 0x04c5acbd, BRF_GRA | CPS1_TILES },
	{ "ps_07.9a",      0x080000, 0xa544f4cc, BRF_GRA | CPS1_TILES },
	{ "ps_08.10a",     0x080000, 0x8f02f436, BRF_GRA | CPS1_TILES },

	{ "ps_q.5k",       0x020000, 0x49ff4446, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ps-q1.1k",      0x080000, 0x31fd8726, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q2.2k",      0x080000, 0x980a9eef, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q3.3k",      0x080000, 0x0dd44491, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "ps-q4.4k",      0x080000, 0xbed42f03, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "ps63b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },			// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Punisherj)
STD_ROM_FN(Punisherj)

static struct BurnRomInfo PunipicRomDesc[] = {
	{ "cpu5.bin",      0x080000, 0xc3151563, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cpu3.bin",      0x080000, 0x8c2593ac, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cpu4.bin",      0x080000, 0x665a5485, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cpu2.bin",      0x080000, 0xd7b13f39, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "gfx9.bin",      0x080000, 0x9b9a887a, BRF_GRA | CPS1_TILES },
	{ "gfx8.bin",      0x080000, 0x2b94287a, BRF_GRA | CPS1_TILES },
	{ "gfx7.bin",      0x080000, 0xe9bd74f5, BRF_GRA | CPS1_TILES },
	{ "gfx6.bin",      0x080000, 0xa5e1c8a4, BRF_GRA | CPS1_TILES },
	{ "gfx13.bin",     0x080000, 0x6d75a193, BRF_GRA | CPS1_TILES },
	{ "gfx12.bin",     0x080000, 0xa3c205c1, BRF_GRA | CPS1_TILES },
	{ "gfx11.bin",     0x080000, 0x22f2ec92, BRF_GRA | CPS1_TILES },
	{ "gfx10.bin",     0x080000, 0x763974c9, BRF_GRA | CPS1_TILES },
	
	{ "pic16c57",      0x004000, 0x00000000, BRF_PRG | BRF_NODUMP | CPS1_PIC },

	{ "sound.bin",     0x080000, 0xaeec9dc6, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Punipic)
STD_ROM_FN(Punipic)

static struct BurnRomInfo Punipic2RomDesc[] = {
	{ "prg4.bin",      0x080000, 0xc3151563, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "prg3.bin",      0x080000, 0x8c2593ac, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "prg2.bin",      0x080000, 0x665a5485, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "prg1.bin",      0x080000, 0xd7b13f39, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "pu11256.bin",   0x200000, 0x6581faea, BRF_GRA | CPS1_TILES },
	{ "pu13478.bin",   0x200000, 0x61613de4, BRF_GRA | CPS1_TILES },
	
	{ "pic16c57",      0x004000, 0x00000000, BRF_PRG | BRF_NODUMP | CPS1_PIC },

	{ "sound.bin",     0x080000, 0xaeec9dc6, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	{ "93c46.bin",     0x000080, 0x36ab4e7d, BRF_OPT },
};

STD_ROM_PICK(Punipic2)
STD_ROM_FN(Punipic2)

static struct BurnRomInfo Punipic3RomDesc[] = {
	{ "psb5b.rom",     0x080000, 0x58f42c05, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psb3b.rom",     0x080000, 0x90113db4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psb4a.rom",     0x080000, 0x665a5485, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "psb2a.rom",     0x080000, 0xd7b13f39, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "psb-a.rom",     0x200000, 0x57f0f5e3, BRF_GRA | CPS1_TILES },
	{ "psb-b.rom",     0x200000, 0xd9eb867e, BRF_GRA | CPS1_TILES },
};

STD_ROM_PICK(Punipic3)
STD_ROM_FN(Punipic3)

static struct BurnRomInfo PunisherbzRomDesc[] = {
	{ "23.096",        0x080000, 0xbfa45d23, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "22.096",        0x080000, 0x092578a4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "21.096",        0x080000, 0xd21ccddb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "20.096",        0x080000, 0xf9f334ce, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "1.096",         0x080000, 0xad468e07, BRF_GRA | CPS1_TILES },
	{ "2.096",         0x080000, 0xb9fdb6b5, BRF_GRA | CPS1_TILES },
	{ "3.096",         0x080000, 0xbe0b1a78, BRF_GRA | CPS1_TILES },
	{ "4.096",         0x080000, 0xbba67a43, BRF_GRA | CPS1_TILES },
	{ "ps_gfx5.rom",   0x080000, 0xc54ea839, BRF_GRA | CPS1_TILES },
	{ "ps_gfx7.rom",   0x080000, 0x04c5acbd, BRF_GRA | CPS1_TILES },
	{ "ps_gfx6.rom",   0x080000, 0xa544f4cc, BRF_GRA | CPS1_TILES },
	{ "ps_gfx8.rom",   0x080000, 0x8f02f436, BRF_GRA | CPS1_TILES },

	{ "9.512",         0x010000, 0xb8367eb5, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18.010",        0x020000, 0x375c66e7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19.010",        0x020000, 0xeb5ca884, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Punisherbz)
STD_ROM_FN(Punisherbz)

static struct BurnRomInfo QadRomDesc[] = {
	{ "qdu_36a.12f",   0x020000, 0xde9c24a0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "qdu_42a.12h",   0x020000, 0xcfe36f0c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "qdu_37a.13f",   0x020000, 0x10d22320, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "qdu_43a.13h",   0x020000, 0x15e6beb9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "qd_09.4b",      0x020000, 0x8c3f9f44, BRF_GRA | CPS1_TILES },
	{ "qd_01.4a",      0x020000, 0xf688cf8f, BRF_GRA | CPS1_TILES },
	{ "qd_13.9b",      0x020000, 0xafbd551b, BRF_GRA | CPS1_TILES },
	{ "qd_05.9a",      0x020000, 0xc3db0910, BRF_GRA | CPS1_TILES },
	{ "qd_24.5e",      0x020000, 0x2f1bd0ec, BRF_GRA | CPS1_TILES },
	{ "qd_17.5c",      0x020000, 0xa812f9e2, BRF_GRA | CPS1_TILES },
	{ "qd_38.8h",      0x020000, 0xccdddd1f, BRF_GRA | CPS1_TILES },
	{ "qd_32.8f",      0x020000, 0xa8d295d3, BRF_GRA | CPS1_TILES },

	{ "qd_23.13b",     0x010000, 0xcfb5264b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "qdu_30.12c",    0x020000, 0xf190da84, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "qdu_31.13c",    0x020000, 0xb7583f73, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "qd22b.1a",      0x000117, 0x783c53ab, BRF_OPT },	// b-board PLDs
	{ "iob1.12e",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Qad)
STD_ROM_FN(Qad)

static struct BurnRomInfo QadjRomDesc[] = {
	{ "qad_23a.8f",    0x080000, 0x4d3553de, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "qad_22a.7f",    0x080000, 0x3191ddd0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "qad_01.3a",     0x080000, 0x9d853b57, BRF_GRA | CPS1_TILES },
	{ "qad_02.4a",     0x080000, 0xb35976c4, BRF_GRA | CPS1_TILES },
	{ "qad_03.5a",     0x080000, 0xcea4ca8c, BRF_GRA | CPS1_TILES },
	{ "qad_04.6a",     0x080000, 0x41b74d1b, BRF_GRA | CPS1_TILES },

	{ "qad_09.12a",    0x010000, 0x733161cc, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "qad_18.11c",    0x020000, 0x2bfe6f6a, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "qad_19.12c",    0x020000, 0x13d3236b, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "qd63b.1a",      0x000117, 0xb3312b13, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Qadj)
STD_ROM_FN(Qadj)

static struct BurnRomInfo Qtono2jRomDesc[] = {
	{ "tn2j_30.11e",   0x020000, 0x9226eb5e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tn2j_37.11f",   0x020000, 0xd1d30da1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tn2j_31.12e",   0x020000, 0x015e6a8a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tn2j_38.12f",   0x020000, 0x1f139bcc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tn2j_28.9e",    0x020000, 0x86d27f71, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tn2j_35.9f",    0x020000, 0x7a1ab87d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tn2j_29.10e",   0x020000, 0x9c384e99, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tn2j_36.10f",   0x020000, 0x4c4b2a0a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "tn2-02m.4a",    0x080000, 0xf2016a34, BRF_GRA | CPS1_TILES },
	{ "tn2-04m.6a",    0x080000, 0x094e0fb1, BRF_GRA | CPS1_TILES },
	{ "tn2-01m.3a",    0x080000, 0xcb950cf9, BRF_GRA | CPS1_TILES },
	{ "tn2-03m.5a",    0x080000, 0x18a5bf59, BRF_GRA | CPS1_TILES },
	{ "tn2-11m.4c",    0x080000, 0xd0edd30b, BRF_GRA | CPS1_TILES },
	{ "tn2-13m.6c",    0x080000, 0x426621c3, BRF_GRA | CPS1_TILES },
	{ "tn2-10m.3c",    0x080000, 0xa34ece70, BRF_GRA | CPS1_TILES },
	{ "tn2-12m.5c",    0x080000, 0xe04ff2f4, BRF_GRA | CPS1_TILES },

	{ "tn2j_09.12a",   0x008000, 0x6d8edcef, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tn2j_18.11c",   0x020000, 0xa40bf9a7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "tn2j_19.12c",   0x020000, 0x5b3b931e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "qtono2j.1a",    0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Qtono2j)
STD_ROM_FN(Qtono2j)

static struct BurnRomInfo Sf2RomDesc[] = {
	{ "sf2e_30g.11e",  0x020000, 0xfe39ee33, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_37g.11f",  0x020000, 0xfb92cd74, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_31g.12e",  0x020000, 0x69a0a301, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_38g.12f",  0x020000, 0x5e22db70, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_28g.9e",   0x020000, 0x8bf9f1e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_35g.9f",   0x020000, 0x626ef934, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2)
STD_ROM_FN(Sf2)

static struct BurnRomInfo Sf2ebRomDesc[] = {
	{ "sf2e_30b.11e",  0x020000, 0x57bd7051, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_37b.11f",  0x020000, 0x62691cdd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_31b.12e",  0x020000, 0xa673143d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_38b.12f",  0x020000, 0x4c2ccef7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_28b.9e",    0x020000, 0x4009955e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_35b.9f",    0x020000, 0x8c1f3994, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2eb)
STD_ROM_FN(Sf2eb)

static struct BurnRomInfo Sf2eeRomDesc[] = {
	{ "sf2e_30e.11e",  0x020000, 0xf37cd088, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_37e.11f",  0x020000, 0xc39468e6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_31e.12e",  0x020000, 0x7c4771b4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_38e.12f",  0x020000, 0xa4bd0cd9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_28e.9e",   0x020000, 0xe3b95625, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2e_35e.9f",   0x020000, 0x3648769a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob2.11d",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "c632b.ic1",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Sf2ee)
STD_ROM_FN(Sf2ee)

static struct BurnRomInfo Sf2ebblRomDesc[] = {
	{ "12.bin",        0x040000, 0xa258b4d5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "09.bin",        0x040000, 0x59ccd474, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "11.bin",        0x040000, 0x82097d63, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "10.bin",        0x040000, 0x0c83844d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "1b_yf082.bin",  0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "1d_yf028.bin",  0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "1a_yf087.bin",  0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "1c_yf088.bin",  0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "1f_yf085.bin",  0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "1h_yf115.bin",  0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "1e_yf111.bin",  0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "1g_yf002.bin",  0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "1j_yf117.bin",  0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "1l_ye040.bin",  0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "1i_yf038.bin",  0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "1k_ye039.bin",  0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "03.bin",        0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "02.bin",        0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "01.bin",        0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	{ "05.bin",        0x020000, 0xa505621e, BRF_GRA }, // extra graphics - map over normal roms to remove CAPCOM logo
	{ "07.bin",        0x020000, 0xde6271fb, BRF_GRA },
	{ "06.bin",        0x020000, 0x23775344, BRF_GRA },
	{ "08.bin",        0x020000, 0x81c9550f, BRF_GRA },
	
	{ "04.bin",        0x010000, 0x13ea1c44, BRF_OPT }, // unknown
};

STD_ROM_PICK(Sf2ebbl)
STD_ROM_FN(Sf2ebbl)

static struct BurnRomInfo Sf2sttRomDesc[] = {
	{ "12.bin",        0x040000, 0xa258b4d5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP }, // from sf2ebbl - missing from dump (empty space on pcb near ce91e-b and ce91e-a)?
	{ "09.bin",        0x040000, 0x59ccd474, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP }, // from sf2ebbl - missing from dump (empty space on pcb near ce91e-b and ce91e-a)?
	{ "ce91e-b",       0x040000, 0x0862386e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ce91e-a",       0x040000, 0x0c83844d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "stt-a-910913",  0x100000, 0x10a7036d, BRF_GRA | CPS1_TILES }, 
	{ "stt-d-910913",  0x100000, 0x3580b124, BRF_GRA | CPS1_TILES }, 
	{ "stt-b-910913",  0x100000, 0x7a09224e, BRF_GRA | CPS1_TILES }, 
	{ "stt-e-910913",  0x100000, 0x382a612c, BRF_GRA | CPS1_TILES }, 
	{ "stt-c-910913",  0x100000, 0x11701b8f, BRF_GRA | CPS1_TILES }, 
	{ "stt-f-910913",  0x100000, 0x101a0b72, BRF_GRA | CPS1_TILES },

	{ "stt3",          0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "stt2",          0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "stt1",          0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	{ "stt5",          0x020000, 0xa505621e, BRF_GRA }, // extra graphics - map over normal roms to remove CAPCOM logo
	{ "stt7",          0x020000, 0xde6271fb, BRF_GRA },
	{ "stt6",          0x020000, 0x23775344, BRF_GRA },
	{ "stt8",          0x020000, 0x81c9550f, BRF_GRA },
	
	{ "stt4",          0x010000, 0x13ea1c44, BRF_OPT }, // unknown
};

STD_ROM_PICK(Sf2stt)
STD_ROM_FN(Sf2stt)

static struct BurnRomInfo Sf2uaRomDesc[] = {
	{ "sf2u_30a.11e",  0x020000, 0x08beb861, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37a.11f",  0x020000, 0xb7638d69, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31a.12e",  0x020000, 0x0d5394e0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38a.12f",  0x020000, 0x42d6a79e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28a.9e",   0x020000, 0x387a175c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35a.9f",   0x020000, 0xa1a5adcc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2ua)
STD_ROM_FN(Sf2ua)

static struct BurnRomInfo Sf2ubRomDesc[] = {
	{ "sf2u_30b.11e",  0x020000, 0x57bd7051, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37b.11f",  0x020000, 0x4a54d479, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31b.12e",  0x020000, 0xa673143d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38b.12f",  0x020000, 0x4c2ccef7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28b.9e",   0x020000, 0x4009955e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35b.9f",   0x020000, 0x8c1f3994, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2ub)
STD_ROM_FN(Sf2ub)

static struct BurnRomInfo Sf2ucRomDesc[] = {
	{ "sf2u_30c.11e",  0x020000, 0x6cb59385, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37c.11f",  0x020000, 0x32e2c278, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31c.12e",  0x020000, 0xc4fff4a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38c.12f",  0x020000, 0x8210fc0e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28c.9e",   0x020000, 0x6eddd5e8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35c.9f",   0x020000, 0x6bcb404c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },
	
	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2uc)
STD_ROM_FN(Sf2uc)

static struct BurnRomInfo Sf2udRomDesc[] = {
	{ "sf2u_30d.11e",  0x020000, 0x4bb2657c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37d.11f",  0x020000, 0xb33b42f2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31d.12e",  0x020000, 0xd57b67d7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38d.12f",  0x020000, 0x9c8916ef, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28d.9e",   0x020000, 0x175819d1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35d.9f",   0x020000, 0x82060da4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2ud)
STD_ROM_FN(Sf2ud)

static struct BurnRomInfo Sf2ueRomDesc[] = {
	{ "sf2u_30e.11e",  0x020000, 0xf37cd088, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37e.11f",  0x020000, 0x6c61a513, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31e.12e",  0x020000, 0x7c4771b4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38e.12f",  0x020000, 0xa4bd0cd9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28e.9e",   0x020000, 0xe3b95625, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35e.9f",   0x020000, 0x3648769a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob2.11d",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "c632b.ic1",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Sf2ue)
STD_ROM_FN(Sf2ue)

static struct BurnRomInfo Sf2ufRomDesc[] = {
	{ "sf2u_30f.11e",  0x020000, 0xfe39ee33, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37f.11f",  0x020000, 0x169e7388, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31f.12e",  0x020000, 0x69a0a301, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38f.12f",  0x020000, 0x1510e4e2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28f.9e",   0x020000, 0xacd8175b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35f.9f",   0x020000, 0xc0a80bd1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2uf)
STD_ROM_FN(Sf2uf)

static struct BurnRomInfo Sf2ugRomDesc[] = {
	{ "sf2u_30g.11e",  0x020000, 0xfe39ee33, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37g.11f",  0x020000, 0x5886cae7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31g.12e",  0x020000, 0x69a0a301, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38g.12f",  0x020000, 0x5e22db70, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28g.9e",   0x020000, 0x8bf9f1e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35g.9f",   0x020000, 0x626ef934, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2ug)
STD_ROM_FN(Sf2ug)

static struct BurnRomInfo Sf2uiRomDesc[] = {
	{ "sf2u_30i.11e",  0x020000, 0xfe39ee33, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37i.11f",  0x020000, 0x9df707dd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31i.12e",  0x020000, 0x69a0a301, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38i.12f",  0x020000, 0x4cb46daf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_28i.9e",    0x020000, 0x1580be4c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35i.9f",   0x020000, 0x1468d185, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29b.10e",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36b.10f",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_9.12a",     0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2ui)
STD_ROM_FN(Sf2ui)

static struct BurnRomInfo Sf2ukRomDesc[] = {
	{ "sf2u_30k.11e",  0x020000, 0x8f66076c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_37k.11f",  0x020000, 0x4e1f6a83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31k.12e",  0x020000, 0xf9f89f60, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38k.12f",  0x020000, 0x6ce0a85a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28k.9e",   0x020000, 0x8e958f31, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35k.9f",   0x020000, 0xfce76fad, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_29a.10e",  0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_36a.10f",  0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2-5m.4a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2-7m.6a",     0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2-1m.3a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2-3m.5a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2-6m.4c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2-8m.6c",     0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2-2m.3c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2-4m.5c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2-13m.4d",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2-15m.6d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2-9m.3d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2-11m.5d",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_09.12a",    0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2uk)
STD_ROM_FN(Sf2uk)

static struct BurnRomInfo Sf2jRomDesc[] = {
	{ "sf2j30.bin",    0x020000, 0x79022b31, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j37.bin",    0x020000, 0x516776ec, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j31.bin",    0x020000, 0xfe15cb39, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j38.bin",    0x020000, 0x38614d70, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j28.bin",    0x020000, 0xd283187a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j35.bin",    0x020000, 0xd28158e4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_29a.bin",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36a.bin",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2_06.8a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2_08.10a",    0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2_05.7a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2_07.9a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2_15.8c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2_17.10c",    0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2_14.7c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2_16.9c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2_25.8d",     0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2_27.10d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2_24.7d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2_26.9d",     0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_09.bin",    0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2j)
STD_ROM_FN(Sf2j)

static struct BurnRomInfo Sf2jaRomDesc[] = {
	{ "sf2j_30a.11e",  0x020000, 0x57bd7051, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_37a.11f",  0x020000, 0x1e1f6844, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_31a.12e",  0x020000, 0xa673143d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_38a.12f",  0x020000, 0x4c2ccef7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_28a.9e",   0x020000, 0x4009955e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_35a.9f",   0x020000, 0x8c1f3994, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_29a.10e",  0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_36a.10f",  0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2_06.8a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2_08.10a",    0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2_05.7a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2_07.9a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2_15.8c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2_17.10c",    0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2_14.7c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2_16.9c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2_25.8d",     0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2_27.10d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2_24.7d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2_26.9d",     0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2j_09.12a",   0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2j_18.11c",   0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2j_19.12c",   0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2ja)
STD_ROM_FN(Sf2ja)

static struct BurnRomInfo Sf2jcRomDesc[] = {
	{ "sf2j_30c.11e",  0x020000, 0x8add35ec, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_37c.11f",  0x020000, 0x0d74a256, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_31c.12e",  0x020000, 0xc4fff4a9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_38c.12f",  0x020000, 0x8210fc0e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_28c.9e",   0x020000, 0x6eddd5e8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_35c.9f",   0x020000, 0x6bcb404c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_29a.10e",  0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2j_36a.10f",  0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2_06.8a",     0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2_08.10a",    0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2_05.7a",     0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2_07.9a",     0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2_15.8c",     0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2_17.10c",    0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2_14.7c",     0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2_16.9c",     0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2_25.8d",     0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2_27.10d",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2_24.7d",     0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2_26.9d",     0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_09.12a",    0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2j_18.11c",   0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2j_19.12c",   0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "stf29.1a",      0x000117, 0x043309c5, BRF_OPT },			// b-board PLDs
	{ "iob1.11d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Sf2jc)
STD_ROM_FN(Sf2jc)

static struct BurnRomInfo Sf2qp1RomDesc[] = {
	{ "stfii-qkn-cps-17.33", 0x080000, 0x3a9458ee, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "stfii-qkn-cps-17.34", 0x080000, 0x4ed215d8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sf2_06.bin",    0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2_08.bin",    0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2_05.bin",    0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2_07.bin",    0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2_15.bin",    0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2_17.bin",    0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2_14.bin",    0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2_16.bin",    0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2_25.bin",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2_27.bin",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2_24.bin",    0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2_26.bin",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_09.bin",    0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2qp1)
STD_ROM_FN(Sf2qp1)

static struct BurnRomInfo Sf2thndrRomDesc[] = {
	{ "17_30.11e",     0x020000, 0xd3cd6d18, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "17_37.11f",     0x020000, 0xe892716e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_31b.12e",  0x020000, 0xa673143d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_38b.12f",  0x020000, 0x4c2ccef7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_28b.9e",   0x020000, 0x4009955e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2u_35b.9f",   0x020000, 0x8c1f3994, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "17_29.10e",     0x020000, 0x8830b54d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "17_36.10f",     0x020000, 0x3f13ada3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sf2_06.bin",    0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2_08.bin",    0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2_05.bin",    0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2_07.bin",    0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2_15.bin",    0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2_17.bin",    0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2_14.bin",    0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2_16.bin",    0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2_25.bin",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2_27.bin",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2_24.bin",    0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2_26.bin",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_09.bin",    0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2thndr)
STD_ROM_FN(Sf2thndr)

static struct BurnRomInfo Sf2bRomDesc[] = {
	{ "u3.bin",        0x020000, 0x2c9ece7c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u4.bin",        0x020000, 0x4efb4c7a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "sf2_31a.bin",   0x020000, 0xa673143d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_38a.bin",   0x020000, 0x4c2ccef7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

//	{ "sf2-4.bin",     0x020000, 0x76f9f91f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	
//	{ "sf2-3.bin",     0x020000, 0xe8f14362, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "u6.bin",        0x020000, 0x5cfc3f39, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u5.bin",        0x020000, 0x47dd24b6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
		
	{ "sf2_29a.bin",   0x020000, 0xbb4af315, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf2_36a.bin",   0x020000, 0xc02a13eb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
//	{ "sf2-1.bin",     0x020000, 0x6de44671, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
//	{ "sf2-2.bin",     0x020000, 0xbf0cd819, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	
	
	{ "sf2_06.bin",    0x080000, 0x22c9cc8e, BRF_GRA | CPS1_TILES },
	{ "sf2_08.bin",    0x080000, 0x57213be8, BRF_GRA | CPS1_TILES },
	{ "sf2_05.bin",    0x080000, 0xba529b4f, BRF_GRA | CPS1_TILES },
	{ "sf2_07.bin",    0x080000, 0x4b1b33a8, BRF_GRA | CPS1_TILES },
	{ "sf2_15.bin",    0x080000, 0x2c7e2229, BRF_GRA | CPS1_TILES },
	{ "sf2_17.bin",    0x080000, 0xb5548f17, BRF_GRA | CPS1_TILES },
	{ "sf2_14.bin",    0x080000, 0x14b84312, BRF_GRA | CPS1_TILES },
	{ "sf2_16.bin",    0x080000, 0x5e9cd89a, BRF_GRA | CPS1_TILES },
	{ "sf2_25.bin",    0x080000, 0x994bfa58, BRF_GRA | CPS1_TILES },
	{ "sf2_27.bin",    0x080000, 0x3e66ad9d, BRF_GRA | CPS1_TILES },
	{ "sf2_24.bin",    0x080000, 0xc1befaa8, BRF_GRA | CPS1_TILES },
	{ "sf2_26.bin",    0x080000, 0x0627c831, BRF_GRA | CPS1_TILES },

	{ "sf2_09.bin",    0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sf2_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sf2_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2b)
STD_ROM_FN(Sf2b)

static struct BurnRomInfo Sf2ceRomDesc[] = {
	{ "s92e_23b.8f",   0x080000, 0x0aaa1a3a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_22b.7f",    0x080000, 0x2bbe15ed, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.6f",    0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92-1m.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92-3m.5a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92-2m.4a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92-4m.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92-5m.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92-7m.9a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92-6m.8a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92-8m.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92-10m.3c",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92-12m.5c",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92-11m.4c",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92-13m.6c",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.11a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2ce)
STD_ROM_FN(Sf2ce)

static struct BurnRomInfo Sf2ceeaRomDesc[] = {
	{ "s92e_23a.8f",   0x080000, 0x3f846b74, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_22a.7f",    0x080000, 0x99f1cca4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.6f",    0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92-1m.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92-3m.5a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92-2m.4a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92-4m.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92-5m.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92-7m.9a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92-6m.8a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92-8m.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92-10m.3c",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92-12m.5c",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92-11m.4c",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92-13m.6c",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.11a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2ceea)
STD_ROM_FN(Sf2ceea)

static struct BurnRomInfo Sf2cejaRomDesc[] = {
	{ "s92j_23a.8f",   0x080000, 0x4f42bb5a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92j_22a.7f",   0x080000, 0xc4f64bcd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.6f",    0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.4a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.5a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.8a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.9a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.3c",     0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.4c",     0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.5c",     0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.6c",     0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.12a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2ceja)
STD_ROM_FN(Sf2ceja)

static struct BurnRomInfo Sf2cejbRomDesc[] = {
	{ "s92j_23b.8f",   0x080000, 0x140876c5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92j_22b.7f",   0x080000, 0x2fbb3bfe, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.6f",    0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.4a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.5a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.8a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.9a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.3c",     0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.4c",     0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.5c",     0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.6c",     0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.12a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2cejb)
STD_ROM_FN(Sf2cejb)

static struct BurnRomInfo Sf2cejcRomDesc[] = {
	{ "s92j_23c.8f",   0x080000, 0xf0120635, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92j_22c.7f",   0x080000, 0x8c0b2ed6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92j_21a.6f",   0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.4a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.5a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.8a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.9a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.3c",     0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.4c",     0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.5c",     0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.6c",     0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.12a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2cejc)
STD_ROM_FN(Sf2cejc)

static struct BurnRomInfo Sf2ceuaRomDesc[] = {
	{ "s92u_23a.8f",   0x080000, 0xac44415b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_22a.7f",    0x080000, 0x99f1cca4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.6f",    0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92-1m.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92-3m.5a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92-2m.4a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92-4m.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92-5m.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92-7m.9a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92-6m.8a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92-8m.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92-10m.3c",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92-12m.5c",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92-11m.4c",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92-13m.6c",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.11a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2ceua)
STD_ROM_FN(Sf2ceua)

static struct BurnRomInfo Sf2ceubRomDesc[] = {
	{ "s92u_23b.8f",   0x080000, 0x996a3015, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_22b.7f",    0x080000, 0x2bbe15ed, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.6f",    0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92-1m.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92-3m.5a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92-2m.4a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92-4m.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92-5m.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92-7m.9a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92-6m.8a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92-8m.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92-10m.3c",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92-12m.5c",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92-11m.4c",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92-13m.6c",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.11a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2ceub)
STD_ROM_FN(Sf2ceub)

static struct BurnRomInfo Sf2ceucRomDesc[] = {
	{ "s92u_23c.8f",   0x080000, 0x0a8b6aa2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_22c.7f",    0x080000, 0x5fd8630b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.6f",    0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92-1m.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92-3m.5a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92-2m.4a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92-4m.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92-5m.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92-7m.9a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92-6m.8a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92-8m.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92-10m.3c",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92-12m.5c",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92-11m.4c",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92-13m.6c",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.11a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2ceuc)
STD_ROM_FN(Sf2ceuc)

static struct BurnRomInfo Sf2accRomDesc[] = {
	{ "sf2ca_23-c.bin", 0x080000, 0xe7c8c5a6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2ca_22-c.bin", 0x080000, 0x99f1cca4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2ca_21-c.bin", 0x040000, 0xcf7fcc8c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",     0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",     0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",     0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",     0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",     0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",     0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",     0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",     0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2acc)
STD_ROM_FN(Sf2acc)

static struct BurnRomInfo Sf2accaRomDesc[] = {
	{ "sf2ca_23-c.8f",  0x080000, 0x35f9517b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2ca_22-c.7f",  0x080000, 0x99f1cca4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2ca_21-c.6f",  0x080000, 0x2ab2034f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",     0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",     0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",     0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",     0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",     0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",     0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",     0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",     0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2acca)
STD_ROM_FN(Sf2acca)

static struct BurnRomInfo Sf2accp2RomDesc[] = {
	{ "sf2ca-23.bin",  0x080000, 0x36c3ba2f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2ca-22.bin",  0x080000, 0x0550453d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2ca-21.bin",  0x040000, 0x4c1c43ba, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2accp2)
STD_ROM_FN(Sf2accp2)

static struct BurnRomInfo Sf2dkot2RomDesc[] = {
	{ "turboii.23",    0x080000, 0x9bbfe420, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "turboii.22",    0x080000, 0x3e57ba19, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "turboii.21",    0x080000, 0xed4186bd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2dkot2)
STD_ROM_FN(Sf2dkot2)

static struct BurnRomInfo Sf2rbRomDesc[] = {
	{ "sf2d__23.rom",  0x080000, 0x450532b0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2d__22.rom",  0x080000, 0xfe9d9cf5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.bin",   0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2rb)
STD_ROM_FN(Sf2rb)

static struct BurnRomInfo Sf2rb2RomDesc[] = {
	{ "27.bin",        0x020000, 0x40296ecd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "31.bin",        0x020000, 0x87954a41, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "26.bin",        0x020000, 0xa6974195, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "30.bin",        0x020000, 0x8141fe32, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "25.bin",        0x020000, 0x9ef8f772, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "29.bin",        0x020000, 0x7d9c479c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "24.bin",        0x020000, 0x93579684, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "28.bin",        0x020000, 0xff728865, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "s92_21a.bin",   0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2rb2)
STD_ROM_FN(Sf2rb2)

static struct BurnRomInfo Sf2rb3RomDesc[] = {
	{ "sf2_ce_rb.23",  0x080000, 0x202f9e50, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2_ce_rb.22",  0x080000, 0x145e5219, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.bin",   0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2rb3)
STD_ROM_FN(Sf2rb3)

static struct BurnRomInfo Sf2redRomDesc[] = {
	{ "sf2red.23",     0x080000, 0x40276abb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2red.22",     0x080000, 0x18daf387, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2red.21",     0x080000, 0x52c486bb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2red)
STD_ROM_FN(Sf2red)

static struct BurnRomInfo Sf2v004RomDesc[] = {
	{ "sf2v004.23",    0x080000, 0x52d19f2c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2v004.22",    0x080000, 0x4b26fde7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2red.21",     0x080000, 0x52c486bb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2v004)
STD_ROM_FN(Sf2v004)

static struct BurnRomInfo Sf2hfRomDesc[] = {
	{ "s2te_23.8f",    0x080000, 0x2dd72514, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s2te_22.7f",    0x080000, 0xaea6e035, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s2te_21.6f",    0x080000, 0xfd200288, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92-1m.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92-3m.5a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92-2m.4a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92-4m.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92-5m.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92-7m.9a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92-6m.8a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92-8m.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92-10m.3c",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92-12m.5c",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92-11m.4c",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92-13m.6c",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.11a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2hf)
STD_ROM_FN(Sf2hf)

static struct BurnRomInfo Sf2hfuRomDesc[] = {
	{ "s2tu_23.8f",    0x080000, 0x89a1fc38, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s2tu_22.7f",    0x080000, 0xaea6e035, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s2tu_21.6f",    0x080000, 0xfd200288, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92-1m.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92-3m.5a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92-2m.4a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92-4m.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92-5m.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92-7m.9a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92-6m.8a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92-8m.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92-10m.3c",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92-12m.5c",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92-11m.4c",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92-13m.6c",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.11a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2hfu)
STD_ROM_FN(Sf2hfu)

static struct BurnRomInfo Sf2hfjRomDesc[] = {
	{ "s2tj_23.8f",    0x080000, 0xea73b4dc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s2tj_22.7f",    0x080000, 0xaea6e035, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s2tj_21.6f",    0x080000, 0xfd200288, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.3a",     0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.4a",     0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.5a",     0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.6a",     0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.7a",     0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.8a",     0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.9a",     0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.10a",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s2t_10.3c",     0x080000, 0x3c042686, BRF_GRA | CPS1_TILES },
	{ "s2t_11.4c",     0x080000, 0x8b7e7183, BRF_GRA | CPS1_TILES },
	{ "s2t_12.5c",     0x080000, 0x293c888c, BRF_GRA | CPS1_TILES },
	{ "s2t_13.6c",     0x080000, 0x842b35a4, BRF_GRA | CPS1_TILES },

	{ "s92_09.12a",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.11c",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.12c",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "s9263b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Sf2hfj)
STD_ROM_FN(Sf2hfj)

static struct BurnRomInfo Sf2hfjbRomDesc[] = {
	// This set is also known as sf2ce13
	{ "222-040.13",    0x080000, 0xec6f5cb3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "196-040.13",    0x080000, 0x0e9ac52b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "221-010.13",    0x020000, 0x8226c11c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "195-010.13",    0x020000, 0x924c6ce2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s2t_10.bin",    0x080000, 0x3c042686, BRF_GRA | CPS1_TILES },
	{ "s2t_11.bin",    0x080000, 0x8b7e7183, BRF_GRA | CPS1_TILES },
	{ "s2t_12.bin",    0x080000, 0x293c888c, BRF_GRA | CPS1_TILES },
	{ "s2t_13.bin",    0x080000, 0x842b35a4, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2hfjb)
STD_ROM_FN(Sf2hfjb)

static struct BurnRomInfo Sf2hfjb2RomDesc[] = {
	{ "sf12-1.040",    0x080000, 0xec6f5cb3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf12-3.040",    0x080000, 0x0e9ac52b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf12-2.010",    0x020000, 0xd1707134, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf12-4.010",    0x020000, 0xcd1d5666, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s2t_10.bin",    0x080000, 0x3c042686, BRF_GRA | CPS1_TILES },
	{ "s2t_11.bin",    0x080000, 0x8b7e7183, BRF_GRA | CPS1_TILES },
	{ "s2t_12.bin",    0x080000, 0x293c888c, BRF_GRA | CPS1_TILES },
	{ "s2t_13.bin",    0x080000, 0x842b35a4, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2hfjb2)
STD_ROM_FN(Sf2hfjb2)

static struct BurnRomInfo Sf2yycRomDesc[] = {
	{ "b12.rom",      0x0080000, 0x8f742fd5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "b14.rom",      0x0080000, 0x8831ec7f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "b11.rom",      0x0020000, 0x94a46525, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "b13.rom",      0x0020000, 0x8fb3dd47, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2yyc)
STD_ROM_FN(Sf2yyc)

static struct BurnRomInfo Sf2koryuRomDesc[] = {
	{ "u222.rom",     0x0080000, 0x9236a79a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u196.rom",     0x0080000, 0xb23a869d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u221.rom",     0x0020000, 0x64e6e091, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u195.rom",     0x0020000, 0xc95e4443, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

#if !defined ROM_VERIFY
	{ "u70.rom",      0x0100000, 0xbaa0f81f, BRF_GRA | CPS1_TILES },
	{ "u68.rom",      0x0100000, 0x8edff95a, BRF_GRA | CPS1_TILES },
	{ "u69.rom",      0x0100000, 0x468962b1, BRF_GRA | CPS1_TILES },
	{ "u64.rom",      0x0100000, 0x8165f536, BRF_GRA | CPS1_TILES },
	{ "u19.rom",      0x0100000, 0x39d763d3, BRF_GRA | CPS1_TILES },
	{ "u18.rom",      0x0100000, 0x93ec42ae, BRF_GRA | CPS1_TILES },
#else
	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },
#endif

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2koryu)
STD_ROM_FN(Sf2koryu)

static struct BurnRomInfo Sf2koryu2RomDesc[] = {
    { "sf10-3040",    0x0080000, 0x9236a79a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf10-1040",    0x0080000, 0xb23a869d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf10-2010",    0x0020000, 0x8226c11c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
    { "sf10-4010",    0x0020000, 0x924c6ce2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2koryu2)
STD_ROM_FN(Sf2koryu2)

static struct BurnRomInfo Sf2amfRomDesc[] = {
	{ "amf5",          0x080000, 0x03991fba, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "amf4",          0x080000, 0x39f15a1e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "stf2th-6.bin",  0x020000, 0x64e6e091, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP }, // wasn't in original dump (set seems to be same as sf2th but with different graphics roms)
	{ "stf2th-4.bin",  0x020000, 0xc95e4443, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP }, // wasn't in original dump (set seems to be same as sf2th but with different graphics roms)
	
	{ "y.c.e.c-m.k.r-001", 0x100000, 0xa258de13, BRF_GRA | CPS1_TILES }, 
	{ "y.c.e.c-m.k.r-003", 0x100000, 0xc781bf87, BRF_GRA | CPS1_TILES }, 
	{ "y.c.e.c-m.k.r-002", 0x100000, 0x5726cab8, BRF_GRA | CPS1_TILES }, 
	{ "y.c.e.c-d.w.c-011", 0x100000, 0xbc90c12f, BRF_GRA | CPS1_TILES }, 
	{ "y.c.e.c-d.w.c-012", 0x100000, 0x187667cc, BRF_GRA | CPS1_TILES }, 
	{ "y.c.e.c-d.w.c-013", 0x100000, 0x5b585071, BRF_GRA | CPS1_TILES }, 

	{ "amf3",          0x010000, 0xa4823a1b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "amf2",          0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "amf1",          0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	{ "amf6",          0x020000, 0x3a85a275, BRF_GRA }, // extra graphics - map over normal roms to remove CAPCOM logo
	{ "amf9",          0x020000, 0x9156472f, BRF_GRA },
	{ "amf8",          0x020000, 0xecdb083b, BRF_GRA },
	{ "amf10",         0x020000, 0x8fea8384, BRF_GRA },

	{ "amf7",          0x010000, 0x13ea1c44, BRF_OPT }, // unknown
};

STD_ROM_PICK(Sf2amf)
STD_ROM_FN(Sf2amf)

static struct BurnRomInfo Sf2mdtRomDesc[] = {
    { "3.ic172",      0x0080000, 0x5301b41f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "1.ic171",      0x0080000, 0xc1c803f6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "4.ic176",      0x0020000, 0x1073b7b6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
    { "2.ic175",      0x0020000, 0x924c6ce2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "7.ic90",       0x0080000, 0x896eaf48, BRF_GRA | CPS1_TILES },
	{ "13.ic89",      0x0080000, 0x305dd72a, BRF_GRA | CPS1_TILES },	
	{ "10.ic88",      0x0080000, 0xef3f5be8, BRF_GRA | CPS1_TILES },	
	{ "16.ic87",      0x0080000, 0xe57f6db9, BRF_GRA | CPS1_TILES },
	{ "6.ic91",       0x0080000, 0x054cd5c4, BRF_GRA | CPS1_TILES },
	{ "12.ic92",      0x0080000, 0x87e069e8, BRF_GRA | CPS1_TILES },
	{ "9.ic93",       0x0080000, 0x818ca33d, BRF_GRA | CPS1_TILES },	
	{ "15.ic94",      0x0080000, 0x5dfb44d1, BRF_GRA | CPS1_TILES },
	{ "8.ic86",       0x0080000, 0x34bbb3fa, BRF_GRA | CPS1_TILES },
	{ "14.ic85",      0x0080000, 0x7d9f1a67, BRF_GRA | CPS1_TILES },
	{ "11.ic84",      0x0080000, 0xcea6d1d6, BRF_GRA | CPS1_TILES },	
	{ "17.ic83",      0x0080000, 0x91a9a05d, BRF_GRA | CPS1_TILES },

	{ "5.ic26",       0x0020000, 0x17d5ba8a, BRF_PRG | CPS1_Z80_PROGRAM },
};

STD_ROM_PICK(Sf2mdt)
STD_ROM_FN(Sf2mdt)

static struct BurnRomInfo Sf2mdtaRomDesc[] = {
    { "3.bin",          0x0080000, 0x9f544ef4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "5.bin",          0x0080000, 0xd76d6621, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "2.bin",          0x0020000, 0x74844192, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
    { "4.bin",          0x0020000, 0xbd98ff15, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	// is there a bad dump here? some of the sprites have vertical lines, whilst others are fine
	{ "pf4-sh058.ic89", 0x0100000, 0x40fdf624, BRF_GRA | CPS1_TILES }, // f205v has redumped this with the same result, it reports an error on pin 30, assumed bad
	{ "pf7-sh072.ic92", 0x0100000, 0xfb78022e, BRF_GRA | CPS1_TILES },
	{ "pf5-sh036.ic90", 0x0100000, 0x0a6be48b, BRF_GRA | CPS1_TILES },
	{ "pf8-sh074.ic93", 0x0100000, 0x6258c7cf, BRF_GRA | CPS1_TILES },
	{ "pf6-sh070.ic88", 0x0100000, 0x9b5b09d7, BRF_GRA | CPS1_TILES },
	{ "pf9-sh001.ic91", 0x0100000, 0x9f25090e, BRF_GRA | CPS1_TILES },

	{ "1.ic28",         0x0020000, 0xd5bee9cc, BRF_PRG | CPS1_Z80_PROGRAM },
};

STD_ROM_PICK(Sf2mdta)
STD_ROM_FN(Sf2mdta)

static struct BurnRomInfo Sf2m1RomDesc[] = {
	{ "222e",         0x0080000, 0x1e20d0a3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "196e",         0x0080000, 0x88cc38a3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "s92_21a.bin",  0x0080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m1)
STD_ROM_FN(Sf2m1)

static struct BurnRomInfo Sf2m2RomDesc[] = {
	{ "ch222esp",     0x0080000, 0x9e6d058a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ch196esp",     0x0080000, 0xed2ff437, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "s92_21a.bin",  0x0080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m2)
STD_ROM_FN(Sf2m2)

static struct BurnRomInfo Sf2m3RomDesc[] = {
	{ "u222chp",      0x0080000, 0xdb567b66, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u196chp",      0x0080000, 0x95ea597e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "s92_21a.bin",  0x0080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m3)
STD_ROM_FN(Sf2m3)

static struct BurnRomInfo Sf2m4RomDesc[] = {
	{ "u222ne",       0x0080000, 0x7133489e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u196ne",       0x0080000, 0xb07a4f90, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "s92_21a.bin",  0x0080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m4)
STD_ROM_FN(Sf2m4)

static struct BurnRomInfo Sf2m5RomDesc[] = {
	{ "u222",         0x0080000, 0x03991fba, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u196",         0x0080000, 0x39f15a1e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "s92_21a.bin",  0x0080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m5)
STD_ROM_FN(Sf2m5)

static struct BurnRomInfo Sf2m6RomDesc[] = {
	{ "u222-6b",      0x0080000, 0x0a3692be, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u196-6b",      0x0080000, 0x80454da7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "s92_21a.bin",  0x0080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m6)
STD_ROM_FN(Sf2m6)

static struct BurnRomInfo Sf2m7RomDesc[] = {
	{ "u222-2i",      0x0040000, 0x1ca7adbd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u196-2i",      0x0040000, 0xf758408c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u222-2s",      0x0040000, 0x720cea3e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u196-2s",      0x0040000, 0x9932832c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "s92_21a.bin",  0x0080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m7)
STD_ROM_FN(Sf2m7)

static struct BurnRomInfo Sf2m8RomDesc[] = {
//	This was the original dump which was bad
//	{ "u221.epr",     0x0020000, 0xd8276822, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "k4-222",       0x0080000, 0xdb567b66, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "k4-196",       0x0080000, 0x95ea597e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u221.epr",     0x0020000, 0x1073b7b6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u195.epr",     0x0020000, 0x924c6ce2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m8)
STD_ROM_FN(Sf2m8)

static struct BurnRomInfo Sf2m9RomDesc[] = {
	{ "sf3a-1.040",   0x0080000, 0x9e6d058a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3a-3.040",   0x0080000, 0x518d8404, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3a-2",       0x0020000, 0xfca4fc1e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3a-4",       0x0020000, 0xcfdd6f54, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m9)
STD_ROM_FN(Sf2m9)

static struct BurnRomInfo Sf2m10RomDesc[] = {
	{ "sfu9-1.040",   0x0080000, 0x95306baf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu9-3.040",   0x0080000, 0x21024d5f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu9-2.020",   0x0040000, 0x0b3fe5dd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu9-4.020",   0x0040000, 0xdbee7b18, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m10)
STD_ROM_FN(Sf2m10)

static struct BurnRomInfo Sf2m11RomDesc[] = {
	{ "sfu7-1.040",   0x0080000, 0x866a9b31, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu7-3.040",   0x0080000, 0xf3a45593, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu7-2.010",   0x0020000, 0xd1707134, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu7-4.010",   0x0020000, 0xcd1d5666, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m11)
STD_ROM_FN(Sf2m11)

static struct BurnRomInfo Sf2m12RomDesc[] = {
	{ "sfu8-1.040",   0x0080000, 0x10ec67fe, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu8-3.040",   0x0080000, 0x92eb3a1c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu8-2.010",   0x0020000, 0x1073b7b6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sfu8-4.010",   0x0020000, 0x924c6ce2, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m12)
STD_ROM_FN(Sf2m12)

static struct BurnRomInfo Sf2m13RomDesc[] = {
	{ "sf3d5-1040",   0x0080000, 0xbcdd2c3a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3d5-3040",   0x0080000, 0x01965987, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3d5-2010",   0x0020000, 0xdd2e1d31, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3d5-4010",   0x0020000, 0xc95e4443, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m13)
STD_ROM_FN(Sf2m13)

static struct BurnRomInfo Sf2m14RomDesc[] = {
	{ "sf6-1.040",    0x0080000, 0xccd74822, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf6-3.040",    0x0080000, 0x2a48b557, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf6-2.010",    0x0020000, 0x64e6e091, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf6-4.010",    0x0020000, 0xc95e4443, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m14)
STD_ROM_FN(Sf2m14)

static struct BurnRomInfo Sf2m15RomDesc[] = {
	{ "sf3g-1.040",   0x0080000, 0xc90559a5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3g-3.040",   0x0080000, 0x81f36682, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3g-2.010",   0x0020000, 0x9eee20a1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf3g-4.010",   0x0020000, 0x6ac3d875, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2m15)
STD_ROM_FN(Sf2m15)

static struct BurnRomInfo Sf2tlonaRomDesc[] = {
	// There is a set known as sf2tlond and sf2turyu which contains the same data, but has double-size program roms
	{ "tl4mt.1",       0x080000, 0x158635ca, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tl4m.2",        0x080000, 0x882cd1c4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u221t.1m",      0x020000, 0x1073b7b6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u195t.1m",      0x020000, 0xdb7e1f72, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2tlona)
STD_ROM_FN(Sf2tlona)

static struct BurnRomInfo Sf2tlonbRomDesc[] = {
	// This set is also known as sf2tury2
	{ "tl4m.1",        0x080000, 0xafc7bd18, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "tl4m.2",        0x080000, 0x882cd1c4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u221.1m",       0x020000, 0xd1707134, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "u195.1m",       0x020000, 0xcd1d5666, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2tlonb)
STD_ROM_FN(Sf2tlonb)

static struct BurnRomInfo Sf2tloncRomDesc[] = {
	{ "sf11-1.040",    0x080000, 0xafc7bd18, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf11-3.040",    0x080000, 0x882cd1c4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf11-2.010",    0x020000, 0xf8a5cd53, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sf11-4.010",    0x020000, 0xdb7e1f72, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2tlonc)
STD_ROM_FN(Sf2tlonc)

static struct BurnRomInfo Sf2thRomDesc[] = {
//	This was the original dump (JacKc found that if you split it into 4 then parts 2, 3 and 4 match u196 from sf2m5)
//	The first 128Kb of the original dump appears to be missing data, and we are assuming that the dump is bad and using
//	the u196 dump instead
//	{ "stf2th-5.bin", 0x0080000, 0x3127302c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "stf2th-7.bin", 0x0080000, 0x03991fba, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "stf2th-5.bin", 0x0080000, 0x39f15a1e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "stf2th-6.bin", 0x0020000, 0x64e6e091, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "stf2th-4.bin", 0x0020000, 0xc95e4443, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "s92_01.bin",   0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",   0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",   0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",   0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",   0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",   0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",   0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",   0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",   0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",   0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",   0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",   0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",   0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",   0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",   0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	{ "stf2th-10.bin", 0x020000, 0x84427d1b, BRF_GRA }, // extra graphics - map over normal roms (add turbo graphics in place of Asian chars?)
	{ "stf2th-12.bin", 0x020000, 0x55bc790c, BRF_GRA },
	{ "stf2th-9.bin",  0x020000, 0xf8725add, BRF_GRA },
	{ "stf2th-11.bin", 0x020000, 0xc2a5373e, BRF_GRA },
	
	{ "sf2th-8.bin",   0x010000, 0x13ea1c44, BRF_OPT }, // unknown
};

STD_ROM_PICK(Sf2th)
STD_ROM_FN(Sf2th)

static struct BurnRomInfo Sf2thaRomDesc[] = {
	{ "t-2.u52",        0x0080000, 0x0d305e8b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "t-1.u36",        0x0080000, 0x137d8665, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "t-4.u54",        0x0040000, 0x0b3fe5dd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "t-3.u38",        0x0040000, 0xdbee7b18, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "w03.bin",        0x0080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "w05.bin",        0x0080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "w04.bin",        0x0080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "w06.bin",        0x0080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "w07.bin",        0x0080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "w09.bin",        0x0080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "w08.bin",        0x0080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "w10.bin",        0x0080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "w11.bin",        0x0080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "w13.bin",        0x0080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "w12.bin",        0x0080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "w14.bin",        0x0080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",     0x0010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",     0x0020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",     0x0020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	{ "r04.bin",        0x020000, 0x84427d1b, BRF_GRA }, // extra graphics - map over normal roms (add turbo graphics in place of Asian chars?)
	{ "r06.bin",        0x020000, 0x55bc790c, BRF_GRA },
	{ "r03.bin",        0x020000, 0xf8725add, BRF_GRA },
	{ "r05.bin",        0x020000, 0xc2a5373e, BRF_GRA },
	
	{ "g01.bin",        0x010000, 0x13ea1c44, BRF_OPT }, // unknown
		
	{ "palce16v8h-7.u35",   0x006462, 0x4ce28f4a, BRF_OPT },	
	{ "plhs16l8an-8.u1",    0x001946, 0x50852ce7, BRF_OPT },
	{ "palce16v8h-9.u34",   0x003254, 0xfeb26c41, BRF_OPT },
	{ "plhs16l8an-x.u19",   0x001946, 0x63d7d7ba, BRF_OPT },			
};

STD_ROM_PICK(Sf2tha)
STD_ROM_FN(Sf2tha)

static struct BurnRomInfo SfzchRomDesc[] = {
	{ "sfzch23",       0x080000, 0x1140743f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza22",        0x080000, 0x8d9b2480, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfzch21",       0x080000, 0x5435225d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza20",        0x080000, 0x806e8f38, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sfz_01.3a",     0x080000, 0x0dd53e62, BRF_GRA | CPS1_TILES },
	{ "sfz_02.4a",     0x080000, 0x94c31e3f, BRF_GRA | CPS1_TILES },
	{ "sfz_03.5a",     0x080000, 0x9584ac85, BRF_GRA | CPS1_TILES },
	{ "sfz_04.6a",     0x080000, 0xb983624c, BRF_GRA | CPS1_TILES },
	{ "sfz_05.7a",     0x080000, 0x2b47b645, BRF_GRA | CPS1_TILES },
	{ "sfz_06.8a",     0x080000, 0x74fd9fb1, BRF_GRA | CPS1_TILES },
	{ "sfz_07.9a",     0x080000, 0xbb2c734d, BRF_GRA | CPS1_TILES },
	{ "sfz_08.10a",    0x080000, 0x454f7868, BRF_GRA | CPS1_TILES },
	{ "sfz_10.3c",     0x080000, 0x2a7d675e, BRF_GRA | CPS1_TILES },
	{ "sfz_11.4c",     0x080000, 0xe35546c8, BRF_GRA | CPS1_TILES },
	{ "sfz_12.5c",     0x080000, 0xf122693a, BRF_GRA | CPS1_TILES },
	{ "sfz_13.6c",     0x080000, 0x7cf942c8, BRF_GRA | CPS1_TILES },
	{ "sfz_14.7c",     0x080000, 0x09038c81, BRF_GRA | CPS1_TILES },
	{ "sfz_15.8c",     0x080000, 0x1aa17391, BRF_GRA | CPS1_TILES },
	{ "sfz_16.9c",     0x080000, 0x19a5abd6, BRF_GRA | CPS1_TILES },
	{ "sfz_17.10c",    0x080000, 0x248b3b73, BRF_GRA | CPS1_TILES },
	
	{ "sfz_09.12a",    0x010000, 0xc772628b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "sfz_18.11c",    0x020000, 0x61022b2d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sfz_19.12c",    0x020000, 0x3b5886d5, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sfzch)
STD_ROM_FN(Sfzch)

static struct BurnRomInfo SlammastRomDesc[] = {
	{ "mbe_23e.8f",    0x080000, 0x5394057a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbe_24b.9e",    0x020000, 0x95d5e729, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_28b.9f",    0x020000, 0xb1c7cbcb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_25b.10e",   0x020000, 0xa50d3fd4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	
	{ "mbe_29b.10f",   0x020000, 0x08e32e56, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_21a.6f",    0x080000, 0xd5007b05, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbe_20a.5f",    0x080000, 0xaeb557b0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb-1m.3a",      0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb-3m.5a",      0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb-2m.4a",      0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb-4m.6a",      0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb-5m.7a",      0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb-7m.9a",      0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb-6m.8a",      0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb-8m.10a",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb-10m.3c",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb-12m.5c",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb-11m.4c",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb-13m.6c",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },
	
	{ "mb_qa.5k",      0x020000, 0xe21a03c4, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb-q1.1k",     0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q2.2k",     0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q3.3k",     0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q4.4k",     0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q5.1m",     0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q6.2m",     0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q7.3m",     0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q8.4m",     0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "mb63b.1a",      0x000117, 0xb8392f02, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },	// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Slammast)
STD_ROM_FN(Slammast)

static struct BurnRomInfo SlammastuRomDesc[] = {
	{ "mbu_23e.8f",    0x080000, 0x224f0062, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbu_24b.9e",    0x020000, 0x95d5e729, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbu_28b.9f",    0x020000, 0xb1c7cbcb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbu_25b.10e",   0x020000, 0xa50d3fd4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbu_29b.10f",   0x020000, 0x08e32e56, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbu_21a.6f",    0x080000, 0xd5007b05, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbu_20a.5f",    0x080000, 0xfc848af5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb-1m.3a",      0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb-3m.5a",      0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb-2m.4a",      0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb-4m.6a",      0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb-5m.7a",      0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb-7m.9a",      0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb-6m.8a",      0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb-8m.10a",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb-10m.3c",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb-12m.5c",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb-11m.4c",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb-13m.6c",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },
	
	{ "mb_qa.5k",      0x020000, 0xe21a03c4, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb-q1.1k",     0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q2.2k",     0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q3.3k",     0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q4.4k",     0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q5.1m",     0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q6.2m",     0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q7.3m",     0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q8.4m",     0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "mb63b.1a",      0x000117, 0xb8392f02, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },	// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Slammastu)
STD_ROM_FN(Slammastu)

static struct BurnRomInfo MbomberjRomDesc[] = {
	{ "mbj23e",        0x080000, 0x0d06036a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbe_24b.rom",   0x020000, 0x95d5e729, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_28b.rom",   0x020000, 0xb1c7cbcb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_25b.rom",   0x020000, 0xa50d3fd4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_29b.rom",   0x020000, 0x08e32e56, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_21a.rom",   0x080000, 0xd5007b05, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbe_20a.rom",   0x080000, 0xaeb557b0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mbj_01.bin",    0x080000, 0xa53b1c81, BRF_GRA | CPS1_TILES },
	{ "mbj_03.bin",    0x080000, 0x23fe10f6, BRF_GRA | CPS1_TILES },
	{ "mbj_02.bin",    0x080000, 0xcb866c2f, BRF_GRA | CPS1_TILES },
	{ "mbj_04.bin",    0x080000, 0xc9143e75, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },
	
	{ "mb_qa.5k",      0x020000, 0xe21a03c4, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb-q1.1k",      0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q2.2k",      0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q3.3k",      0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q4.4k",      0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q5.1m",      0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q6.2m",      0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q7.3m",      0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q8.4m",      0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "mb63b.1a",      0x000117, 0xb8392f02, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },	// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Mbomberj)
STD_ROM_FN(Mbomberj)

static struct BurnRomInfo MbombrdRomDesc[] = {
	{ "mbd_26.bin",    0x020000, 0x72b7451c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbde_30.rom",   0x020000, 0xa036dc16, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_27.bin",    0x020000, 0x4086f534, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_31.bin",    0x020000, 0x085f47f0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_24.bin",    0x020000, 0xc20895a5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_28.bin",    0x020000, 0x2618d5e1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_25.bin",    0x020000, 0x9bdb6b11, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_29.bin",    0x020000, 0x3f52d5e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_21.bin",    0x080000, 0x690c026a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbd_20.bin",    0x080000, 0xb8b2139b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb_gfx01.rom",  0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb_gfx03.rom",  0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb_gfx02.rom",  0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb_gfx04.rom",  0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },

	{ "mb_q.5k",       0x020000, 0xd6fa76d1, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb-q1.1k",      0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q2.2k",      0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q3.3k",      0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q4.4k",      0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q5.1m",      0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q6.2m",      0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q7.3m",      0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q8.4m",      0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "mb63b.1a",      0x000117, 0xb8392f02, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },	// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Mbombrd)
STD_ROM_FN(Mbombrd)

static struct BurnRomInfo MbombrdjRomDesc[] = {
	{ "mbd_26.bin",    0x020000, 0x72b7451c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbdj_30.bin",   0x020000, 0xbeff31cf, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_27.bin",    0x020000, 0x4086f534, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_31.bin",    0x020000, 0x085f47f0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_24.bin",    0x020000, 0xc20895a5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_28.bin",    0x020000, 0x2618d5e1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_25.bin",    0x020000, 0x9bdb6b11, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_29.bin",    0x020000, 0x3f52d5e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_21.bin",    0x080000, 0x690c026a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbd_20.bin",    0x080000, 0xb8b2139b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mbj_01.bin",    0x080000, 0xa53b1c81, BRF_GRA | CPS1_TILES },
	{ "mbj_03.bin",    0x080000, 0x23fe10f6, BRF_GRA | CPS1_TILES },
	{ "mbj_02.bin",    0x080000, 0xcb866c2f, BRF_GRA | CPS1_TILES },
	{ "mbj_04.bin",    0x080000, 0xc9143e75, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },

	{ "mb_q.5k",       0x020000, 0xd6fa76d1, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb-q1.1k",      0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q2.2k",      0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q3.3k",      0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q4.4k",      0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q5.1m",      0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q6.2m",      0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q7.3m",      0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb-q8.4m",      0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "mb63b.1a",      0x000117, 0xb8392f02, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },	// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k2.9k",       0x000117, 0xcd85a156, BRF_OPT },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Mbombrdj)
STD_ROM_FN(Mbombrdj)

static struct BurnRomInfo StriderRomDesc[] = {
	{ "30.11f",        0x020000, 0xda997474, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "35.11h",        0x020000, 0x5463aaa3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "31.12f",        0x020000, 0xd20786db, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "36.12h",        0x020000, 0x21aa2863, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "st-14.8h",      0x080000, 0x9b3cfc08, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "st-2.8a",       0x080000, 0x4eee9aea, BRF_GRA | CPS1_TILES },
	{ "st-11.10a",     0x080000, 0x2d7f21e4, BRF_GRA | CPS1_TILES },
	{ "st-5.4a",       0x080000, 0x7705aa46, BRF_GRA | CPS1_TILES },
	{ "st-9.6a",       0x080000, 0x5b18b722, BRF_GRA | CPS1_TILES },
	{ "st-1.7a",       0x080000, 0x005f000b, BRF_GRA | CPS1_TILES },
	{ "st-10.9a",      0x080000, 0xb9441519, BRF_GRA | CPS1_TILES },
	{ "st-4.3a",       0x080000, 0xb7d04e8b, BRF_GRA | CPS1_TILES },
	{ "st-8.5a",       0x080000, 0x6b4713b4, BRF_GRA | CPS1_TILES },

	{ "09.12b",        0x010000, 0x2ed403bc, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18.11c",        0x020000, 0x4386bc80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19.12c",        0x020000, 0x444536d7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "st24m1.1a",     0x000117, 0xa80d357e, BRF_OPT },	// b-board PLDs
	{ "lwio.11e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Strider)
STD_ROM_FN(Strider)

static struct BurnRomInfo StrideruaRomDesc[] = {
	{ "30.30",         0x020000, 0x66aec273, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "35.35",         0x020000, 0x50e0e865, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "31.31",         0x020000, 0xeae93bd1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "36.36",         0x020000, 0xb904a31d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "st-14.8h",      0x080000, 0x9b3cfc08, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "st-2.8a",       0x080000, 0x4eee9aea, BRF_GRA | CPS1_TILES },
	{ "st-11.10a",     0x080000, 0x2d7f21e4, BRF_GRA | CPS1_TILES },
	{ "st-5.4a",       0x080000, 0x7705aa46, BRF_GRA | CPS1_TILES },
	{ "st-9.6a",       0x080000, 0x5b18b722, BRF_GRA | CPS1_TILES },
	{ "st-1.7a",       0x080000, 0x005f000b, BRF_GRA | CPS1_TILES },
	{ "st-10.9a",      0x080000, 0xb9441519, BRF_GRA | CPS1_TILES },
	{ "st-4.3a",       0x080000, 0xb7d04e8b, BRF_GRA | CPS1_TILES },
	{ "st-8.5a",       0x080000, 0x6b4713b4, BRF_GRA | CPS1_TILES },

	{ "09.9",          0x010000, 0x08d63519, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18.11c",        0x020000, 0x4386bc80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19.12c",        0x020000, 0x444536d7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "st24b2.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.11e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Striderua)
STD_ROM_FN(Striderua)

static struct BurnRomInfo StriderjrRomDesc[] = {
	{ "sthj_23.8f",    0x080000, 0x046e7b12, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sthj_22.7f",    0x080000, 0x9b3cfc08, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sth_01.3a",     0x080000, 0x4eee9aea, BRF_GRA | CPS1_TILES },
	{ "sth_02.4a",     0x080000, 0x2d7f21e4, BRF_GRA | CPS1_TILES },
	{ "sth_03.5a",     0x080000, 0x7705aa46, BRF_GRA | CPS1_TILES },
	{ "sth_04.6a",     0x080000, 0x5b18b722, BRF_GRA | CPS1_TILES },
	{ "sth_05.7a",     0x080000, 0x005f000b, BRF_GRA | CPS1_TILES },
	{ "sth_06.8a",     0x080000, 0xb9441519, BRF_GRA | CPS1_TILES },
	{ "sth_07.9a",     0x080000, 0xb7d04e8b, BRF_GRA | CPS1_TILES },
	{ "sth_08.10a",    0x080000, 0x6b4713b4, BRF_GRA | CPS1_TILES },

	{ "sth_09.12a",    0x010000, 0x08d63519, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sth_18.11c",    0x020000, 0x4386bc80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sth_19.12c",    0x020000, 0x444536d7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "sth63b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Striderjr)
STD_ROM_FN(Striderjr)

static struct BurnRomInfo StriderjRomDesc[] = {
	{ "sth36.bin",     0x020000, 0x53c7b006, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sth42.bin",     0x020000, 0x4037f65f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sth37.bin",     0x020000, 0x80e8877d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sth43.bin",     0x020000, 0x6b3fa466, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sth34.bin",     0x020000, 0xbea770b5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sth40.bin",     0x020000, 0x43b922dc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sth35.bin",     0x020000, 0x5cc429da, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sth41.bin",     0x020000, 0x50af457f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "sth09.bin",     0x020000, 0x1ef6bfbd, BRF_GRA | CPS1_TILES },
	{ "sth01.bin",     0x020000, 0x1e21b0c1, BRF_GRA | CPS1_TILES },
	{ "sth13.bin",     0x020000, 0x063263ae, BRF_GRA | CPS1_TILES },
	{ "sth05.bin",     0x020000, 0xec9f8714, BRF_GRA | CPS1_TILES },	
	{ "sth24.bin",     0x020000, 0x6356f4d2, BRF_GRA | CPS1_TILES },	
	{ "sth17.bin",     0x020000, 0xb4f73d86, BRF_GRA | CPS1_TILES },
	{ "sth38.bin",     0x020000, 0xee5abfc2, BRF_GRA | CPS1_TILES },
	{ "sth32.bin",     0x020000, 0x44a206a3, BRF_GRA | CPS1_TILES },	
	{ "sth10.bin",     0x020000, 0xdf3dd3bc, BRF_GRA | CPS1_TILES },	
	{ "sth02.bin",     0x020000, 0xc75f9ea0, BRF_GRA | CPS1_TILES },
	{ "sth14.bin",     0x020000, 0x6c03e19d, BRF_GRA | CPS1_TILES },
	{ "sth06.bin",     0x020000, 0xd84f5478, BRF_GRA | CPS1_TILES },	
	{ "sth25.bin",     0x020000, 0x921e506a, BRF_GRA | CPS1_TILES },
	{ "sth18.bin",     0x020000, 0x5b318956, BRF_GRA | CPS1_TILES },
	{ "sth39.bin",     0x020000, 0x9321d6aa, BRF_GRA | CPS1_TILES },
	{ "sth33.bin",     0x020000, 0xb47ddfc7, BRF_GRA | CPS1_TILES },	
	{ "sth11.bin",     0x020000, 0x2484f241, BRF_GRA | CPS1_TILES },
	{ "sth03.bin",     0x020000, 0xaaa07245, BRF_GRA | CPS1_TILES },
	{ "sth15.bin",     0x020000, 0xe415d943, BRF_GRA | CPS1_TILES },
	{ "sth07.bin",     0x020000, 0x97d072d2, BRF_GRA | CPS1_TILES },
	{ "sth26.bin",     0x020000, 0x0ebfcb02, BRF_GRA | CPS1_TILES },
	{ "sth19.bin",     0x020000, 0x257ce683, BRF_GRA | CPS1_TILES },
	{ "sth28.bin",     0x020000, 0x98ac8cd1, BRF_GRA | CPS1_TILES },
	{ "sth21.bin",     0x020000, 0x538d9423, BRF_GRA | CPS1_TILES },
	{ "sth12.bin",     0x020000, 0xf670a477, BRF_GRA | CPS1_TILES },
	{ "sth04.bin",     0x020000, 0x853d3e01, BRF_GRA | CPS1_TILES },
	{ "sth16.bin",     0x020000, 0x4092019f, BRF_GRA | CPS1_TILES },
	{ "sth08.bin",     0x020000, 0x2ce9b4c7, BRF_GRA | CPS1_TILES },
	{ "sth27.bin",     0x020000, 0xf82c88d9, BRF_GRA | CPS1_TILES },
	{ "sth20.bin",     0x020000, 0xeb584dd4, BRF_GRA | CPS1_TILES },
	{ "sth29.bin",     0x020000, 0x34ae2997, BRF_GRA | CPS1_TILES },
	{ "sth22.bin",     0x020000, 0x78dd9c48, BRF_GRA | CPS1_TILES },

	{ "sth23.bin",     0x010000, 0x2ed403bc, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sth30.bin",     0x020000, 0x4386bc80, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sth31.bin",     0x020000, 0x444536d7, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "st22b.1a",      0x000117, 0x68fecc55, BRF_OPT },	// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Striderj)
STD_ROM_FN(Striderj)

static struct BurnRomInfo UnsquadRomDesc[] = {
	{ "aru_30.11f",    0x020000, 0x24d8f88d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "aru_35.11h",    0x020000, 0x8b954b59, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "aru_31.12f",    0x020000, 0x33e9694b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "aru_36.12h",    0x020000, 0x7cc8fb9e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ar-32m.8h",     0x080000, 0xae1d7fb0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ar-5m.7a",      0x080000, 0xbf4575d8, BRF_GRA | CPS1_TILES },
	{ "ar-7m.9a",      0x080000, 0xa02945f4, BRF_GRA | CPS1_TILES },
	{ "ar-1m.3a",      0x080000, 0x5965ca8d, BRF_GRA | CPS1_TILES },
	{ "ar-3m.5a",      0x080000, 0xac6db17d, BRF_GRA | CPS1_TILES },

	{ "ar_09.12b",     0x010000, 0xf3dd1367, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "aru_18.11c",    0x020000, 0x584b43a9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ar24b.1a",      0x000117, 0x09a51271, BRF_OPT },	// b-board PLDs
	{ "lwio.11e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Unsquad)
STD_ROM_FN(Unsquad)

static struct BurnRomInfo Area88RomDesc[] = {
	{ "ar_36.12f",     0x020000, 0x65030392, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ar_42.12h",     0x020000, 0xc48170de, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ar_37.13f",     0x020000, 0x33e9694b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ar_43.13h",     0x020000, 0x7cc8fb9e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ar_34.10f",     0x020000, 0xf6e80386, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ar_40.10h",     0x020000, 0xbe36c145, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ar_35.11f",     0x020000, 0x86d98ff3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "ar_41.11h",     0x020000, 0x758893d3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "ar_09.4b",      0x020000, 0xdb9376f8, BRF_GRA | CPS1_TILES },
	{ "ar_01.4a",      0x020000, 0x392151b4, BRF_GRA | CPS1_TILES },
	{ "ar_13.9b",      0x020000, 0x81436481, BRF_GRA | CPS1_TILES },
	{ "ar_05.9a",      0x020000, 0xe246ed9f, BRF_GRA | CPS1_TILES },
	{ "ar_24.5e",      0x020000, 0x9cd6e2a3, BRF_GRA | CPS1_TILES },
	{ "ar_17.5c",      0x020000, 0x0b8e0df4, BRF_GRA | CPS1_TILES },
	{ "ar_38.8h",      0x020000, 0x8b9e75b9, BRF_GRA | CPS1_TILES },
	{ "ar_32.8f",      0x020000, 0xdb6acdcf, BRF_GRA | CPS1_TILES },
	{ "ar_10.5b",      0x020000, 0x4219b622, BRF_GRA | CPS1_TILES },
	{ "ar_02.5a",      0x020000, 0xbac5dec5, BRF_GRA | CPS1_TILES },
	{ "ar_14.10b",     0x020000, 0xe6bae179, BRF_GRA | CPS1_TILES },
	{ "ar_06.10a",     0x020000, 0xc8f04223, BRF_GRA | CPS1_TILES },
	{ "ar_25.7e",      0x020000, 0x15ccf981, BRF_GRA | CPS1_TILES },
	{ "ar_18.7c",      0x020000, 0x9336db6a, BRF_GRA | CPS1_TILES },
	{ "ar_39.9h",      0x020000, 0x9b8e1363, BRF_GRA | CPS1_TILES },
	{ "ar_33.9f",      0x020000, 0x3968f4b5, BRF_GRA | CPS1_TILES },

	{ "ar_23.13c",     0x010000, 0xf3dd1367, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ar_30.12e",     0x020000, 0x584b43a9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ar22b.1a",      0x000117, 0xf1db9030, BRF_OPT },	// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Area88)
STD_ROM_FN(Area88)

static struct BurnRomInfo Area88rRomDesc[] = {
	{ "araj_23.8f",    0x080000, 0x7045d6cb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "araj_22.7f",    0x080000, 0x9913002e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "ara_01.3a",     0x080000, 0xbf4575d8, BRF_GRA | CPS1_TILES },
	{ "ara_02.4a",     0x080000, 0xa02945f4, BRF_GRA | CPS1_TILES },
	{ "ara_03.5a",     0x080000, 0x5965ca8d, BRF_GRA | CPS1_TILES },
	{ "ara_04.6a",     0x080000, 0xac6db17d, BRF_GRA | CPS1_TILES },

	{ "ara_09.12a",    0x020000, 0xaf88359c, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "ara_18.11c",    0x020000, 0x584b43a9, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "ara63b.1a",     0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic7",      0x000117, 0x0d182081, BRF_OPT },		// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Area88r)
STD_ROM_FN(Area88r)

static struct BurnRomInfo VarthRomDesc[] = {
	{ "vae_30b.11f",   0x020000, 0xadb8d391, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_35b.11h",   0x020000, 0x44e5548f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_31b.12f",   0x020000, 0x1749a71c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_36b.12h",   0x020000, 0x5f2e2450, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_28b.9f",    0x020000, 0xe524ca50, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_33b.9h",    0x020000, 0xc0bbf8c9, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_29b.10f",   0x020000, 0x6640996a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_34b.10h",   0x020000, 0xfa59be8a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "va-5m.7a",      0x080000, 0xb1fb726e, BRF_GRA | CPS1_TILES },
	{ "va-7m.9a",      0x080000, 0x4c6588cd, BRF_GRA | CPS1_TILES },
	{ "va-1m.3a",      0x080000, 0x0b1ace37, BRF_GRA | CPS1_TILES },
	{ "va-3m.5a",      0x080000, 0x44dfe706, BRF_GRA | CPS1_TILES },

	{ "va_09.12b",     0x010000, 0x7a99446e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "va_18.11c",     0x020000, 0xde30510e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "va_19.12c",     0x020000, 0x0610a4ac, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "va24b.1a",      0x000117, 0xcc476650, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Varth)
STD_ROM_FN(Varth)

static struct BurnRomInfo Varthr1RomDesc[] = {
	{ "vae_30a.11f",   0x020000, 0x7fcd0091, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_35a.11h",   0x020000, 0x35cf9509, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_31a.12f",   0x020000, 0x15e5ee81, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_36a.12h",   0x020000, 0x153a201e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_28a.9f",    0x020000, 0x7a0e0d25, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_33a.9h",    0x020000, 0xf2365922, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_29a.10f",   0x020000, 0x5e2cd2c3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vae_34a.10h",   0x020000, 0x3d9bdf83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "va-5m.7a",      0x080000, 0xb1fb726e, BRF_GRA | CPS1_TILES },
	{ "va-7m.9a",      0x080000, 0x4c6588cd, BRF_GRA | CPS1_TILES },
	{ "va-1m.3a",      0x080000, 0x0b1ace37, BRF_GRA | CPS1_TILES },
	{ "va-3m.5a",      0x080000, 0x44dfe706, BRF_GRA | CPS1_TILES },

	{ "va_09.12b",     0x010000, 0x7a99446e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "va_18.11c",     0x020000, 0xde30510e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "va_19.12c",     0x020000, 0x0610a4ac, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "va24b.1a",      0x000117, 0xcc476650, BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, BRF_OPT },
};

STD_ROM_PICK(Varthr1)
STD_ROM_FN(Varthr1)

static struct BurnRomInfo VarthjRomDesc[] = {
	{ "vaj_36b.12f",   0x020000, 0x1d798d6a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vaj_42b.12h",   0x020000, 0x0f720233, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vaj_37b.13f",   0x020000, 0x24414b17, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vaj_43b.13h",   0x020000, 0x34b4b06c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vaj_34b.10f",   0x020000, 0x87c79aed, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vaj_40b.10h",   0x020000, 0x210b4bd0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vaj_35b.11f",   0x020000, 0x6b0da69f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "vaj_41b.11h",   0x020000, 0x6542c8a4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	
	{ "va_09.4b",      0x020000, 0x183dfaa8, BRF_GRA | CPS1_TILES },
	{ "va_01.4a",      0x020000, 0xc41312b5, BRF_GRA | CPS1_TILES },
	{ "va_13.9b",      0x020000, 0x45537e69, BRF_GRA | CPS1_TILES },
	{ "va_05.9a",      0x020000, 0x7065d4e9, BRF_GRA | CPS1_TILES },
	{ "va_24.5e",      0x020000, 0x57191ccf, BRF_GRA | CPS1_TILES },
	{ "va_17.5c",      0x020000, 0x054f5a5b, BRF_GRA | CPS1_TILES },
	{ "va_38.8h",      0x020000, 0xe117a17e, BRF_GRA | CPS1_TILES },
	{ "va_32.8f",      0x020000, 0x3b4f40b2, BRF_GRA | CPS1_TILES },
	{ "va_10.5b",      0x020000, 0xd62750cd, BRF_GRA | CPS1_TILES },
	{ "va_02.5a",      0x020000, 0x11590325, BRF_GRA | CPS1_TILES },
	{ "va_14.10b",     0x020000, 0xdc2f4783, BRF_GRA | CPS1_TILES },
	{ "va_06.10a",     0x020000, 0x06e833ac, BRF_GRA | CPS1_TILES },
	{ "va_25.7e",      0x020000, 0x51d90690, BRF_GRA | CPS1_TILES },
	{ "va_18.7c",      0x020000, 0xa17817c0, BRF_GRA | CPS1_TILES },
	{ "va_39.9h",      0x020000, 0xb0b12f51, BRF_GRA | CPS1_TILES },
	{ "va_33.9f",      0x020000, 0x4b003af7, BRF_GRA | CPS1_TILES },

	{ "va_23.13c",     0x010000, 0x7a99446e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "va_30.12e",     0x020000, 0xde30510e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "va_31.13e",     0x020000, 0x0610a4ac, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "va22b.1a",      0x000117, 0xbd7cd574, BRF_OPT },			// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },			// c-board PLDs
};

STD_ROM_PICK(Varthj)
STD_ROM_FN(Varthj)

static struct BurnRomInfo VarthuRomDesc[] = {
	{ "vau_23a.8f",    0x080000, 0xfbe68726, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "vau_22a.7f",    0x080000, 0x0ed71bbd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "va-5m.3a",      0x080000, 0xb1fb726e, BRF_GRA | CPS1_TILES },
	{ "va-7m.5a",      0x080000, 0x4c6588cd, BRF_GRA | CPS1_TILES },
	{ "va-1m.4a",      0x080000, 0x0b1ace37, BRF_GRA | CPS1_TILES },
	{ "va-3m.6a",      0x080000, 0x44dfe706, BRF_GRA | CPS1_TILES },

	{ "va_09.11a",     0x010000, 0x7a99446e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "va_18.11c",     0x020000, 0xde30510e, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "va_19.12c",     0x020000, 0x0610a4ac, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "va63b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
};

STD_ROM_PICK(Varthu)
STD_ROM_FN(Varthu)

static struct BurnRomInfo WillowRomDesc[] = {
	{ "wlu_30.11f",    0x020000, 0xd604dbb1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "35.11h",        0x020000, 0x7a791e77, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wlu_31.12f",    0x020000, 0x0eb48a83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wlu_36.12h",    0x020000, 0x36100209, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wlm-32.8h",     0x080000, 0xdfd9f643, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "wlm-7.7a",      0x080000, 0xafa74b73, BRF_GRA | CPS1_TILES },
	{ "wlm-5.9a",      0x080000, 0x12a0dc0b, BRF_GRA | CPS1_TILES },
	{ "wlm-3.3a",      0x080000, 0xc6f2abce, BRF_GRA | CPS1_TILES },
	{ "wlm-1.5a",      0x080000, 0x4aa4c6d3, BRF_GRA | CPS1_TILES },
	{ "wl_24.7d",      0x020000, 0x6f0adee5, BRF_GRA | CPS1_TILES },
	{ "wl_14.7c",      0x020000, 0x9cf3027d, BRF_GRA | CPS1_TILES },
	{ "wl_26.9d",      0x020000, 0xf09c8ecf, BRF_GRA | CPS1_TILES },
	{ "wl_16.9c",      0x020000, 0xe35407aa, BRF_GRA | CPS1_TILES },
	{ "wl_20.3d",      0x020000, 0x84992350, BRF_GRA | CPS1_TILES },
	{ "wl_10.3c",      0x020000, 0xb87b5a36, BRF_GRA | CPS1_TILES },
	{ "wl_22.5d",      0x020000, 0xfd3f89f0, BRF_GRA | CPS1_TILES },
	{ "wl_12.5c",      0x020000, 0x7da49d69, BRF_GRA | CPS1_TILES },

	{ "wl_09.12b",     0x010000, 0xf6b3d060, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "wl_18.11c",     0x020000, 0xbde23d4d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "wl_19.12c",     0x020000, 0x683898f5, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "wl24b.1a",      0x000117, 0x7101cdf1, BRF_OPT },	// b-board PLDs
	{ "lwio.11e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Willow)
STD_ROM_FN(Willow)

static struct BurnRomInfo WillowoRomDesc[] = {
	{ "wlu_30.11f",    0x020000, 0xd604dbb1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wlu_35.11h",    0x020000, 0xdaee72fe, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wlu_31.12f",    0x020000, 0x0eb48a83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wlu_36.12h",    0x020000, 0x36100209, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wlm-32.8h",     0x080000, 0xdfd9f643, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "wlm-7.7a",      0x080000, 0xafa74b73, BRF_GRA | CPS1_TILES },
	{ "wlm-5.9a",      0x080000, 0x12a0dc0b, BRF_GRA | CPS1_TILES },
	{ "wlm-3.3a",      0x080000, 0xc6f2abce, BRF_GRA | CPS1_TILES },
	{ "wlm-1.5a",      0x080000, 0x4aa4c6d3, BRF_GRA | CPS1_TILES },
	{ "wl_24.7d",      0x020000, 0x6f0adee5, BRF_GRA | CPS1_TILES },
	{ "wl_14.7c",      0x020000, 0x9cf3027d, BRF_GRA | CPS1_TILES },
	{ "wl_26.9d",      0x020000, 0xf09c8ecf, BRF_GRA | CPS1_TILES },
	{ "wl_16.9c",      0x020000, 0xe35407aa, BRF_GRA | CPS1_TILES },
	{ "wl_20.3d",      0x020000, 0x84992350, BRF_GRA | CPS1_TILES },
	{ "wl_10.3c",      0x020000, 0xb87b5a36, BRF_GRA | CPS1_TILES },
	{ "wl_22.5d",      0x020000, 0xfd3f89f0, BRF_GRA | CPS1_TILES },
	{ "wl_12.5c",      0x020000, 0x7da49d69, BRF_GRA | CPS1_TILES },

	{ "wl_09.12b",     0x010000, 0xf6b3d060, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "wl_18.11c",     0x020000, 0xbde23d4d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "wl_19.12c",     0x020000, 0x683898f5, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "wl24b.1a",      0x000117, 0x7101cdf1, BRF_OPT },	// b-board PLDs
	{ "lwio.11e",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Willowo)
STD_ROM_FN(Willowo)

static struct BurnRomInfo WillowjRomDesc[] = {
	{ "wl_36.12f",     0x020000, 0x2b0d7cbc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wl_42.12h",     0x020000, 0x1ac39615, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wl_37.13f",     0x020000, 0x30a717fa, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wl_43.13h",     0x020000, 0xd0dddc9e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wl_34.10f",     0x020000, 0x23a84f7a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wl_40.10h",     0x020000, 0xc7a0ed21, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wl_35.11f",     0x020000, 0x5eff7951, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "wl_41.11h",     0x020000, 0x8d6477a3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "wl_09.4b",      0x020000, 0x05aa71b4, BRF_GRA | CPS1_TILES },
	{ "wl_01.4a",      0x020000, 0x08c2df12, BRF_GRA | CPS1_TILES },
	{ "wl_13.9b",      0x020000, 0x1f7c87cd, BRF_GRA | CPS1_TILES },
	{ "wl_05.9a",      0x020000, 0xf5254bf2, BRF_GRA | CPS1_TILES },
	{ "wl_24.5e",      0x020000, 0xd9d73ba1, BRF_GRA | CPS1_TILES },
	{ "wl_17.5c",      0x020000, 0xa652f30c, BRF_GRA | CPS1_TILES },
	{ "wl_38.8h",      0x020000, 0xf6f9111b, BRF_GRA | CPS1_TILES },
	{ "wl_32.8f",      0x020000, 0x10f64027, BRF_GRA | CPS1_TILES },
	{ "wl_10.5b",      0x020000, 0xdbba0a3f, BRF_GRA | CPS1_TILES },
	{ "wl_02.5a",      0x020000, 0x86fba7a5, BRF_GRA | CPS1_TILES },
	{ "wl_14.10b",     0x020000, 0x7d5798b2, BRF_GRA | CPS1_TILES },
	{ "wl_06.10a",     0x020000, 0x1f052948, BRF_GRA | CPS1_TILES },
	{ "wl_25.7e",      0x020000, 0x857d17d2, BRF_GRA | CPS1_TILES },
	{ "wl_18.7c",      0x020000, 0x316c7fbc, BRF_GRA | CPS1_TILES },
	{ "wl_39.9h",      0x020000, 0xe6fce9b0, BRF_GRA | CPS1_TILES },
	{ "wl_33.9f",      0x020000, 0xa15d5517, BRF_GRA | CPS1_TILES },
		
	{ "wl_11.7b",      0x020000, 0x6f0adee5, BRF_GRA | CPS1_TILES },
	{ "wl_03.7a",      0x020000, 0x9cf3027d, BRF_GRA | CPS1_TILES },
	{ "wl_15.11b",     0x020000, 0xf09c8ecf, BRF_GRA | CPS1_TILES },
	{ "wl_07.11a",     0x020000, 0xe35407aa, BRF_GRA | CPS1_TILES },
	{ "wl_26.8e",      0x020000, 0x84992350, BRF_GRA | CPS1_TILES },
	{ "wl_19.8c",      0x020000, 0xb87b5a36, BRF_GRA | CPS1_TILES },
	{ "wl_28.10e",     0x020000, 0xfd3f89f0, BRF_GRA | CPS1_TILES },
	{ "wl_21.10c",     0x020000, 0x7da49d69, BRF_GRA | CPS1_TILES },

	{ "wl_23.13c",     0x010000, 0xf6b3d060, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "wl_30.12e",     0x020000, 0xbde23d4d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "wl_31.13e",     0x020000, 0x683898f5, BRF_SND | CPS1_OKIM6295_SAMPLES },
	
	A_BOARD_PLDS
	
	{ "wl22b.1a",      0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",      0x000117, 0xad52b90c, BRF_OPT },
};

STD_ROM_PICK(Willowj)
STD_ROM_FN(Willowj)

static struct BurnRomInfo WofRomDesc[] = {
	{ "tk2e_23c.8f",   0x080000, 0x0d708505, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2e_22c.7f",   0x080000, 0x608c17e3, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2-1m.3a",     0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2-3m.5a",     0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2-2m.4a",     0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2-4m.6a",     0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2-5m.7a",     0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2-7m.9a",     0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2-6m.8a",     0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2-8m.10a",    0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.5k",     0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2-q1.1k",     0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q2.2k",     0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q3.3k",     0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q4.4k",     0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "tk263b.1a",     0x000117, 0xc4b0349b, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },					// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k1.9k",       0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Wof)
STD_ROM_FN(Wof)

static struct BurnRomInfo Wofr1RomDesc[] = {
	{ "tk2e_23b.8f",   0x080000, 0x11fb2ed1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2e_22b.7f",   0x080000, 0x479b3f24, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2-1m.3a",     0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2-3m.5a",     0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2-2m.4a",     0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2-4m.6a",     0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2-5m.7a",     0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2-7m.9a",     0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2-6m.8a",     0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2-8m.10a",    0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.5k",     0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2-q1.1k",     0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q2.2k",     0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q3.3k",     0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q4.4k",     0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "tk263b.1a",     0x000117, 0xc4b0349b, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },					// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k1.9k",       0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Wofr1)
STD_ROM_FN(Wofr1)

static struct BurnRomInfo WofuRomDesc[] = {
	{ "tk2u_23c.8f",   0x080000, 0x29b89c12, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2u_22c.7f",   0x080000, 0xf5af4774, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2-1m.3a",     0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2-3m.5a",     0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2-2m.4a",     0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2-4m.6a",     0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2-5m.7a",     0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2-7m.9a",     0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2-6m.8a",     0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2-8m.10a",    0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.5k",     0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2-q1.1k",     0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q2.2k",     0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q3.3k",     0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q4.4k",     0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "tk263b.1a",     0x000117, 0xc4b0349b, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },	// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k1.9k",       0x000117, 0x00000000, BRF_OPT | BRF_NODUMP},
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Wofu)
STD_ROM_FN(Wofu)

static struct BurnRomInfo WofjRomDesc[] = {
	{ "tk2j_23c.8f",   0x080000, 0x9b215a68, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2j_22c.7f",   0x080000, 0xb74b09ac, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2_01.3a",     0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_02.4a",     0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_03.5a",     0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_04.6a",     0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2_05.7a",     0x080000, 0xe4a44d53, BRF_GRA | CPS1_TILES },
	{ "tk2_06.8a",     0x080000, 0x58066ba8, BRF_GRA | CPS1_TILES },
	{ "tk2_07.9a",     0x080000, 0xd706568e, BRF_GRA | CPS1_TILES },
	{ "tk2_08.10a",    0x080000, 0xd4a19a02, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.5k",     0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2-q1.1k",     0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q2.2k",     0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q3.3k",     0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q4.4k",     0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "tk263b.1a",     0x000117, 0xc4b0349b, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },					// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k1.9k",       0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Wofj)
STD_ROM_FN(Wofj)

static struct BurnRomInfo WofaRomDesc[] = {
	{ "tk2a_23c.8f",   0x080000, 0x2e024628, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2a_22c.7f",   0x080000, 0x900ad4cd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2-1m.3a",     0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2-3m.5a",     0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2-2m.4a",     0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2-4m.6a",     0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2-5m.7a",     0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2-7m.9a",     0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2-6m.8a",     0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2-8m.10a",    0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.5k",     0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2-q1.1k",     0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q2.2k",     0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q3.3k",     0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q4.4k",     0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "tk263b.1a",     0x000117, 0xc4b0349b, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },					// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k1.9k",       0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Wofa)
STD_ROM_FN(Wofa)

static struct BurnRomInfo WofchRomDesc[] = {
	{ "tk2(ch)_23.8f", 0x080000, 0x4e0b8dee, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2(ch)_22.7f", 0x080000, 0xd0937a8d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2-1m.3a",     0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2-3m.5a",     0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2-2m.4a",     0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2-4m.6a",     0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2(ch)_05.7a", 0x080000, 0xe4a44d53, BRF_GRA | CPS1_TILES },
	{ "tk2(ch)_06.8a", 0x080000, 0x58066ba8, BRF_GRA | CPS1_TILES },
	{ "tk2(ch)_07.9a", 0x080000, 0xd706568e, BRF_GRA | CPS1_TILES },
	{ "tk2(ch)_08.10a",0x080000, 0xd4a19a02, BRF_GRA | CPS1_TILES },
	
	{ "tk2_qa.5k",     0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2-q1.1k",     0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q2.2k",     0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q3.3k",     0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2-q4.4k",     0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
	
	A_BOARD_QSOUND_PLDS
	
	{ "tk263b.1a",     0x000117, 0xc4b0349b, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",      0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",     0x000117, 0x31793da7, BRF_OPT },
	
	{ "ioc1.ic1",      0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",      0x000117, 0x0fbd9270, BRF_OPT },
	
	{ "d7l1.7l",       0x000117, 0x27b7410d, BRF_OPT },					// d-board PLDs
	{ "d8l1.8l",       0x000117, 0x539fc7da, BRF_OPT },
	{ "d9k1.9k",       0x000117, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "d10f1.10f",     0x000117, 0x6619c494, BRF_OPT },
};

STD_ROM_PICK(Wofch)
STD_ROM_FN(Wofch)

static struct BurnRomInfo WofhfhRomDesc[] = {
	{ "23",            0x080000, 0x6ae4b312, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "22",            0x080000, 0x94e8d01a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "1",             0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "2",             0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "3",             0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "4",             0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "5",             0x080000, 0x34949d7b, BRF_GRA | CPS1_TILES },
	{ "6",             0x080000, 0xdfa70971, BRF_GRA | CPS1_TILES },
	{ "7",             0x080000, 0x073686a6, BRF_GRA | CPS1_TILES },
	{ "8",             0x080000, 0x5300f8db, BRF_GRA | CPS1_TILES },

	{ "9",             0x020000, 0x86fe8a97, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18",            0x020000, 0xc04be720, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19",            0x020000, 0xfbb8d8c1, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wofhfh)
STD_ROM_FN(Wofhfh)

static struct BurnRomInfo WofhRomDesc[] = {
	{ "sgyx.800",       0x100000, 0x3703a650, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sgyx-1.160",     0x200000, 0xa60be9f6, BRF_GRA | CPS1_TILES },
	{ "sgyx-2.160",     0x200000, 0x6ad9d048, BRF_GRA | CPS1_TILES },
	
	// These sound roms are taken from sgyxz
	{ "sgyxz_snd2.bin", 0x010000, 0x210c376f, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sgyxz_snd1.bin", 0x040000, 0xc15ac0f2, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wofh)
STD_ROM_FN(Wofh)

static struct BurnRomInfo WofhaRomDesc[] = {
	{ "fg-c.040",       0x080000, 0xd046fc86, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "fg-a.040",       0x080000, 0xf176ee8f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sgyx-1.160",     0x200000, 0xa60be9f6, BRF_GRA | CPS1_TILES },
	{ "sgyx-2.160",     0x200000, 0x6ad9d048, BRF_GRA | CPS1_TILES },
	
	// These sound roms are taken from sgyxz
	{ "sgyxz_snd2.bin", 0x010000, 0x210c376f, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sgyxz_snd1.bin", 0x040000, 0xc15ac0f2, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wofha)
STD_ROM_FN(Wofha)

static struct BurnRomInfo SgyxzRomDesc[] = {
	{ "sgyxz_prg2.bin", 0x080000, 0x95429c83, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "sgyxz_prg1.bin", 0x080000, 0xd8511929, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "sgyxz_gfx1.bin", 0x200000, 0xa60be9f6, BRF_GRA | CPS1_TILES },
	{ "sgyxz_gfx2.bin", 0x200000, 0x6ad9d048, BRF_GRA | CPS1_TILES },

	{ "sgyxz_snd2.bin", 0x010000, 0x210c376f, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "sgyxz_snd1.bin", 0x040000, 0xc15ac0f2, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sgyxz)
STD_ROM_FN(Sgyxz)

static struct BurnRomInfo Wof3jsRomDesc[] = {
	{ "3js_23.rom",    0x080000, 0x1ebb76da, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "3js_22.rom",    0x080000, 0xf41d6153, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP }, // First and second halves identical
	{ "3js_24.rom",    0x020000, 0x06ead409, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "3js_28.rom",    0x020000, 0x8ba934e6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "3js_gfx5.rom",  0x080000, 0x94b58f82, BRF_GRA | CPS1_TILES },
	{ "3js_gfx7.rom",  0x080000, 0xdf4fb386, BRF_GRA | CPS1_TILES },
	{ "3js_gfx6.rom",  0x080000, 0xc22c5bd8, BRF_GRA | CPS1_TILES },
	{ "3js_gfx8.rom",  0x080000, 0xf9cfd08b, BRF_GRA | CPS1_TILES },

	{ "3js_09.rom",    0x010000, 0x21ce044c, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "3js_18.rom",    0x020000, 0xac6e307d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "3js_19.rom",    0x020000, 0x068741db, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wof3js)
STD_ROM_FN(Wof3js)

static struct BurnRomInfo Wof3jsaRomDesc[] = {
	{ "cx2.040",       0x080000, 0xc01a6d2f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "cx1.040",       0x080000, 0xfd95e677, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "tx-a.160",      0x200000, 0xae348da2, BRF_GRA | CPS1_TILES },
	{ "tx-b.160",      0x200000, 0x384a6db0, BRF_GRA | CPS1_TILES },

	// These sound roms are taken from wofhfh, this set looks it uses psound
	{ "9",             0x020000, 0x86fe8a97, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18",            0x020000, 0xc04be720, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19",            0x020000, 0xfbb8d8c1, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wof3jsa)
STD_ROM_FN(Wof3jsa)

static struct BurnRomInfo Wof3sjRomDesc[] = {
	{ "k6b.040",       0x080000, 0x7b365108, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "k6a.040",       0x080000, 0x10488a51, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "k6a.160",       0x200000, 0xa121180d, BRF_GRA | CPS1_TILES },
	{ "k6b.160",       0x200000, 0xa4db96c4, BRF_GRA | CPS1_TILES },
	
	// These sound roms are taken from wofhfh, this set looks it uses psound
	{ "9",             0x020000, 0x86fe8a97, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18",            0x020000, 0xc04be720, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19",            0x020000, 0xfbb8d8c1, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wof3sj)
STD_ROM_FN(Wof3sj)

static struct BurnRomInfo Wof3sjaRomDesc[] = {
	{ "3js.800",       0x100000, 0x812f9200, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "k6a.160",       0x200000, 0xa121180d, BRF_GRA | CPS1_TILES },
	{ "k6b.160",       0x200000, 0xa4db96c4, BRF_GRA | CPS1_TILES },
	
	// These sound roms are taken from wofhfh, this set looks it uses psound
	{ "9",             0x020000, 0x86fe8a97, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18",            0x020000, 0xc04be720, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19",            0x020000, 0xfbb8d8c1, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wof3sja)
STD_ROM_FN(Wof3sja)

static struct BurnRomInfo WofsjRomDesc[] = {
	{ "c-c47b.040",    0x080000, 0xb1809761, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "a-2402.040",    0x080000, 0x4fab4232, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx5.rom",  0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx7.rom",  0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx6.rom",  0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx8.rom",  0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },
	
	// These sound roms are taken from wofhfh, this set looks it uses psound
	{ "9",             0x020000, 0x86fe8a97, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18",            0x020000, 0xc04be720, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19",            0x020000, 0xfbb8d8c1, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wofsj)
STD_ROM_FN(Wofsj)

static struct BurnRomInfo WofsjaRomDesc[] = {
	{ "one.800",       0x100000, 0x0507584d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx5.rom",  0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx7.rom",  0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx6.rom",  0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx8.rom",  0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },
	
	// These sound roms are taken from wofhfh, this set looks it uses psound
	{ "9",             0x020000, 0x86fe8a97, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "18",            0x020000, 0xc04be720, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "19",            0x020000, 0xfbb8d8c1, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wofsja)
STD_ROM_FN(Wofsja)

static struct BurnRomInfo WofsjbRomDesc[] = {
	{ "c-d140.040",    0x080000, 0xe6d933a6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "a-0050.040",    0x080000, 0x403eaead, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx5.rom",  0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx7.rom",  0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx6.rom",  0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx8.rom",  0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },
	
	// using the sound roms from wofhfh or wof3js doesn't give the right result
	// it doesn't use Q-Sound either
};

STD_ROM_PICK(Wofsjb)
STD_ROM_FN(Wofsjb)

static struct BurnRomInfo WofbRomDesc[] = {
	{ "3-f2ab.040",    0x080000, 0x61fd0a01, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "1-9207.040",    0x080000, 0x7f59e24c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "4-d4d2.010",    0x020000, 0xfe5eee87, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "2-6c41.010",    0x020000, 0x739379be, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	

	{ "5-caf3.040",    0x080000, 0xc8dcaa95, BRF_GRA | CPS1_TILES },
	{ "6-034f.040",    0x080000, 0x1ab0000c, BRF_GRA | CPS1_TILES },
	{ "7-b0fa.040",    0x080000, 0x8425ff6b, BRF_GRA | CPS1_TILES },
	{ "8-a6b7.040",    0x080000, 0x24ce197b, BRF_GRA | CPS1_TILES },
	{ "9-8a2c.040",    0x080000, 0x9d20ef9b, BRF_GRA | CPS1_TILES },
	{ "10-7d24.040",   0x080000, 0x90c93dd2, BRF_GRA | CPS1_TILES },
	{ "11-4171.040",   0x080000, 0x219fd7e2, BRF_GRA | CPS1_TILES },
	{ "12-f56b.040",   0x080000, 0xefc17c9a, BRF_GRA | CPS1_TILES },
	
	// These sound roms are taken from wof3js, this set looks it uses psound
	{ "3js_09.rom",    0x010000, 0x21ce044c, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "3js_18.rom",    0x020000, 0xac6e307d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "3js_19.rom",    0x020000, 0x068741db, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Wofb)
STD_ROM_FN(Wofb)

static struct BurnRomInfo WofjhRomDesc[] = {
	{ "tk2j23h.bin",   0x080000, 0x84d38575, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2j22c.bin",   0x080000, 0xb74b09ac, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk205.bin",     0x080000, 0xe4a44d53, BRF_GRA | CPS1_TILES },
	{ "tk206.bin",     0x080000, 0x58066ba8, BRF_GRA | CPS1_TILES },
	{ "tk207.bin",     0x080000, 0xd706568e, BRF_GRA | CPS1_TILES },
	{ "tk208.bin",     0x080000, 0xd4a19a02, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.rom",    0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2_q1.rom",    0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q2.rom",    0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q3.rom",    0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q4.rom",    0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Wofjh)
STD_ROM_FN(Wofjh)

static struct BurnRomInfo WofahRomDesc[] = {
	{ "htk2a_23b.rom", 0x080000, 0x1b17fc85, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2a_22b.rom",  0x080000, 0x900ad4cd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx5.rom",  0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx7.rom",  0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx6.rom",  0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx8.rom",  0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.rom",    0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2_q1.rom",    0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q2.rom",    0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q3.rom",    0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q4.rom",    0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Wofah)
STD_ROM_FN(Wofah)

static struct BurnRomInfo WofahaRomDesc[] = {
	{ "htk2ah_23b.rom",0x080000, 0x6e3ff382, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	// Taken from wofah, possibly not correct, although does match original Asia set as well
	{ "tk2a_22b.rom",  0x080000, 0x900ad4cd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx5.rom",  0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx7.rom",  0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx6.rom",  0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx8.rom",  0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.rom",    0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2_q1.rom",    0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q2.rom",    0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q3.rom",    0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q4.rom",    0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Wofaha)
STD_ROM_FN(Wofaha)

static struct BurnRomInfo WofahbRomDesc[] = {
	{ "tk2ah_23b.rom", 0x080000, 0x48fd83c6, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	// Taken from wofah, possibly not correct, although does match original Asia set as well
	{ "tk2a_22b.rom",  0x080000, 0x900ad4cd, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx5.rom",  0x080000, 0x291f0f0b, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx7.rom",  0x080000, 0x3edeb949, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx6.rom",  0x080000, 0x1abd14d6, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx8.rom",  0x080000, 0xb27948e3, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.rom",    0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2_q1.rom",    0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q2.rom",    0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q3.rom",    0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q4.rom",    0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Wofahb)
STD_ROM_FN(Wofahb)

static struct BurnRomInfo Cps1frogRomDesc[] = {
	{ "frog30-36.bin", 0x020000, 0x8eb8ddbe, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "frog35-42.bin", 0x020000, 0xacb5a988, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "frog31-37.bin", 0x020000, 0x154803cc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "frog36-43.bin", 0x020000, 0x154803cc, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "frog32-32m.bin",0x080000, 0x75660aac, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "frog05-05m.bin",0x080000, 0x7bbc4b54, BRF_GRA | CPS1_TILES },
	{ "frog07-07m.bin",0x080000, 0x0beadd80, BRF_GRA | CPS1_TILES },
	{ "frog01-01m.bin",0x080000, 0xf84d02bb, BRF_GRA | CPS1_TILES },
	{ "frog03-03m.bin",0x080000, 0xd784a5b0, BRF_GRA | CPS1_TILES },

	{ "frog09-09.bin", 0x010000, 0x3ac7fb5c, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "frog18-18.bin", 0x020000, 0x65d1ef07, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "frog19-19.bin", 0x020000, 0x7ee8cdcd, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Cps1frog)
STD_ROM_FN(Cps1frog)

// Game config

typedef void (*KabukiDecode)();
static KabukiDecode KabukiDecodeFunction;

static INT32 GameHasStars = 0;

struct GameConfig {
	const char *DriverName;
	INT32 CpsBId;
	INT32 CpsMapperId;
	INT32 CpsStars;
	KabukiDecode DecodeFunction;
};

static const struct GameConfig ConfigTable[] =
{
	{ "1941"        , CPS_B_05    , mapper_YI24B , 0, NULL                },
	{ "1941r1"      , CPS_B_05    , mapper_YI24B , 0, NULL                },
	{ "1941u"       , CPS_B_05    , mapper_YI24B , 0, NULL                },
	{ "1941j"       , CPS_B_05    , mapper_YI24B , 0, NULL                },
	{ "3wonders"    , CPS_B_21_BT1, mapper_RT24B , 0, NULL                },
	{ "3wondersu"   , CPS_B_21_BT1, mapper_RT24B , 0, NULL                },
	{ "wonder3"     , CPS_B_21_BT1, mapper_RT22B , 0, NULL                },
	{ "3wondersh"   , CPS_B_02    , mapper_RT24B , 0, NULL                },
	{ "captcomm"    , CPS_B_21_BT3, mapper_CC63B , 0, NULL                },
	{ "captcommr1"  , CPS_B_21_BT3, mapper_CC63B , 0, NULL                },
	{ "captcommu"   , CPS_B_21_BT3, mapper_CC63B , 0, NULL                },
	{ "captcommj"   , CPS_B_21_BT3, mapper_CC63B , 0, NULL                },
	{ "captcommjr1" , CPS_B_21_BT3, mapper_CC63B , 0, NULL                },
	{ "captcommb"   , CPS_B_21_BT3, mapper_CC63B , 0, NULL                },
	{ "captcommb2"  , CPS_B_21_BT3, mapper_CC63B , 0, NULL                },
	{ "cawing"      , CPS_B_16    , mapper_CA24B , 0, NULL                },
	{ "cawingr1"    , CPS_B_16    , mapper_CA24B , 0, NULL                },
	{ "cawingu"     , CPS_B_16    , mapper_CA24B , 0, NULL                },
	{ "cawingj"     , CPS_B_16    , mapper_CA22B , 0, NULL                },
	{ "cawingbl"    , CPS_B_16    , mapper_CA24B , 0, NULL                },
	{ "cworld2j"    , CPS_B_21_BT6, mapper_Q522B , 0, NULL                },
	{ "dino"        , CPS_B_21_QS2, mapper_CD63B , 0, dino_decode         },
	{ "dinou"       , CPS_B_21_QS2, mapper_CD63B , 0, dino_decode         },
	{ "dinoj"       , CPS_B_21_QS2, mapper_CD63B , 0, dino_decode         },
	{ "dinopic"     , CPS_B_21_QS2, mapper_CD63B , 0, NULL                },
	{ "dinopic2"    , CPS_B_21_QS2, mapper_CD63B , 0, NULL                },
	{ "dinoeh"      , CPS_B_21_QS2, mapper_CD63B , 0, dino_decode         },
	{ "dinoh"       , CPS_B_21_DEF, mapper_CD63B , 0, dino_decode         },
	{ "dinot"       , CPS_B_21_DEF, mapper_CD63B , 0, dino_decode         },
	{ "dinotpic"    , CPS_B_21_QS2, mapper_CD63B , 0, dino_decode         },
	{ "dinohunt"    , CPS_B_21_DEF, mapper_CD63B , 0, NULL                },
	{ "dynwar"      , CPS_B_02    , mapper_TK22B , 0, NULL                },
	{ "dynwara"     , CPS_B_02    , mapper_TK22B , 0, NULL                },
	{ "dynwarj"     , CPS_B_02    , mapper_TK22B , 0, NULL                },
	{ "dynwarjr"    , CPS_B_21_DEF, mapper_TK22B , 0, NULL                },
	{ "ffight"      , CPS_B_04    , mapper_S224B , 0, NULL                },
	{ "ffighta"     , CPS_B_04    , mapper_S224B , 0, NULL                },
	{ "ffightu"     , CPS_B_04    , mapper_S224B , 0, NULL                },
	{ "ffightu1"    , CPS_B_04    , mapper_S224B , 0, NULL                },
	{ "ffightua"    , CPS_B_01    , mapper_S224B , 0, NULL                },
	{ "ffightub"    , CPS_B_05    , mapper_S224B , 0, NULL                },
	{ "ffightj"     , CPS_B_04    , mapper_S224B , 0, NULL                },
	{ "ffightj1"    , CPS_B_01    , mapper_S224B , 0, NULL                },
	{ "ffightj2"    , CPS_B_02    , mapper_S224B , 0, NULL                },
	{ "ffightjh"    , CPS_B_01    , mapper_S224B , 0, NULL                },
	{ "fcrash"      , HACK_B_5    , mapper_S224B , 0, NULL                }, // doesn't really have an ID, but this used to give the relevant values to our rendering functions
	{ "forgottn"    , CPS_B_01    , mapper_LW621 , 1, NULL                },
	{ "forgottnu"   , CPS_B_01    , mapper_LW621 , 1, NULL                },
	{ "forgottnu1"  , CPS_B_01    , mapper_LWCHR , 1, NULL                },
	{ "forgottnua"  , CPS_B_01    , mapper_LWCHR , 1, NULL                },
	{ "lostwrld"    , CPS_B_01    , mapper_LWCHR , 1, NULL                },
	{ "lostwrldo"   , CPS_B_01    , mapper_LWCHR , 1, NULL                },
	{ "ghouls"      , CPS_B_01    , mapper_DM620 , 0, NULL                },
	{ "ghoulsu"     , CPS_B_01    , mapper_DM620 , 0, NULL                },
	{ "daimakai"    , CPS_B_01    , mapper_DM22A , 0, NULL                },
	{ "daimakair"   , CPS_B_21_DEF, mapper_DAM63B, 0, NULL                },
	{ "daimakaib"   , CPS_B_21_DEF, mapper_DAM63B, 0, NULL                }, // game controls layers at 0x98000c
	{ "knights"     , CPS_B_21_BT4, mapper_KR63B , 0, NULL                },
	{ "knightsu"    , CPS_B_21_BT4, mapper_KR63B , 0, NULL                },
	{ "knightsj"    , CPS_B_21_BT4, mapper_KR63B , 0, NULL                },
	{ "knightsja"   , CPS_B_21_BT4, mapper_KR63B , 0, NULL                },
	{ "knightsb"    , CPS_B_21_BT4, mapper_KR63B , 0, NULL                }, // game controls layers (and priorities?) at 0x98000c
	{ "knightsb2"   , CPS_B_21_BT4, mapper_KR63B , 0, NULL                },
	{ "knightsb3"   , CPS_B_21_DEF, mapper_KR63B , 0, NULL                },
	{ "knightsb4"   , CPS_B_21_BT4, mapper_KR63B , 0, NULL                }, // game controls layers (and priorities?) at 0x98000c
	{ "knightsh"    , CPS_B_21_DEF, mapper_KR63B , 0, NULL                },
	{ "knightsh2"   , CPS_B_21_DEF, mapper_KR63B , 0, NULL                },
	{ "kod"         , CPS_B_21_BT2, mapper_KD29B , 0, NULL                },
	{ "kodr1"       , CPS_B_21_BT2, mapper_KD29B , 0, NULL                },
	{ "kodu"        , CPS_B_21_BT2, mapper_KD29B , 0, NULL                },
	{ "kodj"        , CPS_B_21_BT2, mapper_KD29B , 0, NULL                },
	{ "kodja"       , CPS_B_21_BT2, mapper_KD29B , 0, NULL                },
	{ "kodb"        , CPS_B_21_BT2, mapper_KD29B , 0, NULL                }, // game writes priority masks 1&2 at 0x980020-22 range, 0&3 assumed hard-coded at boot
	{ "kodh"        , CPS_B_21_DEF, mapper_KD29B , 0, NULL                },
	{ "kodda"       , CPS_B_21_DEF, mapper_KD29B , 0, NULL                },
	{ "megaman"     , CPS_B_21_DEF, mapper_RCM63B, 0, NULL                },
	{ "megamana"    , CPS_B_21_DEF, mapper_RCM63B, 0, NULL                },
	{ "rockmanj"    , CPS_B_21_DEF, mapper_RCM63B, 0, NULL                },
	{ "mercs"       , CPS_B_12    , mapper_O224B , 0, NULL                },
	{ "mercsu"      , CPS_B_12    , mapper_O224B , 0, NULL                },	
	{ "mercsur1"    , CPS_B_12    , mapper_O224B , 0, NULL                },
	{ "mercsj"      , CPS_B_12    , mapper_O224B , 0, NULL                },
	{ "msword"      , CPS_B_13    , mapper_MS24B , 0, NULL                },
	{ "mswordr1"    , CPS_B_13    , mapper_MS24B , 0, NULL                },
	{ "mswordu"     , CPS_B_13    , mapper_MS24B , 0, NULL                },
	{ "mswordj"     , CPS_B_13    , mapper_MS24B , 0, NULL                },
	{ "mtwins"      , CPS_B_14    , mapper_CK24B , 0, NULL                },
	{ "chikij"      , CPS_B_14    , mapper_CK24B , 0, NULL                },
	{ "nemo"        , CPS_B_15    , mapper_NM24B , 0, NULL                },
	{ "nemoj"       , CPS_B_15    , mapper_NM24B , 0, NULL                },
	{ "pang3"       , CPS_B_21_DEF, mapper_pang3 , 0, NULL                },
	{ "pang3r1"     , CPS_B_21_DEF, mapper_pang3 , 0, NULL                },
	{ "pang3b"      , CPS_B_21_DEF, mapper_pang3 , 0, NULL                },
	{ "pang3b2"     , CPS_B_04    , mapper_pang3 , 0, NULL                }, // hacked to run on Final Fight C-Board
	{ "pang3j"      , CPS_B_21_DEF, mapper_pang3 , 0, NULL                },
	{ "pnickj"      , CPS_B_21_DEF, mapper_PKB10B, 0, NULL                },
	{ "punisher"    , CPS_B_21_QS3, mapper_PS63B , 0, punisher_decode     },
	{ "punisheru"   , CPS_B_21_QS3, mapper_PS63B , 0, punisher_decode     },
	{ "punisherj"   , CPS_B_21_QS3, mapper_PS63B , 0, punisher_decode     },
	{ "punipic"     , CPS_B_21_QS3, mapper_PS63B , 0, NULL                }, // game controls layers at 0x98000c
	{ "punipic2"    , CPS_B_21_QS3, mapper_PS63B , 0, NULL                }, // game controls layers at 0x98000c
	{ "punipic3"    , CPS_B_21_QS3, mapper_PS63B , 0, NULL                }, // game controls layers at 0x98000c
	{ "punisherbz"  , CPS_B_21_DEF, mapper_PS63B , 0, NULL                },
	{ "qad"         , CPS_B_21_BT7, mapper_QD22B , 0, NULL                },
	{ "qadj"        , CPS_B_21_DEF, mapper_QD63B , 0, NULL                },
	{ "qtono2j"     , CPS_B_21_DEF, mapper_qtono2, 0, NULL                },
	{ "sf2"         , CPS_B_11    , mapper_STF29 , 0, NULL                },
	{ "sf2eb"       , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2ee"       , CPS_B_18    , mapper_STF29 , 0, NULL                },
	{ "sf2ebbl"     , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2stt"      , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2ua"       , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2ub"       , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2uc"       , CPS_B_12    , mapper_STF29 , 0, NULL                },
	{ "sf2ud"       , CPS_B_05    , mapper_STF29 , 0, NULL                },
	{ "sf2ue"       , CPS_B_18    , mapper_STF29 , 0, NULL                },
	{ "sf2uf"       , CPS_B_15    , mapper_STF29 , 0, NULL                },
	{ "sf2ug"       , CPS_B_11    , mapper_STF29 , 0, NULL                },
	{ "sf2ui"       , CPS_B_14    , mapper_STF29 , 0, NULL                },
	{ "sf2uk"       , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2j"        , CPS_B_13    , mapper_STF29 , 0, NULL                },
	{ "sf2ja"       , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2jc"       , CPS_B_12    , mapper_STF29 , 0, NULL                },
	{ "sf2qp1"      , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2thndr"    , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2b"        , CPS_B_17    , mapper_STF29 , 0, NULL                },
	{ "sf2ce"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2ceea"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2ceua"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2ceub"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2ceuc"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2ceja"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2cejb"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2cejc"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2rb"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2rb2"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2rb3"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2red"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2v004"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2acc"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2acca"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2accp2"    , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2dkot2"    , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2mdt"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2mdta"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m1"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                }, // game controls layers at 0x98000c
	{ "sf2m2"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m3"       , HACK_B_2    , mapper_S9263B, 0, NULL                },
	{ "sf2m4"       , HACK_B_1    , mapper_S9263B, 0, NULL                },
	{ "sf2m5"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m6"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m7"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m8"       , HACK_B_2    , mapper_S9263B, 0, NULL                },
	{ "sf2m9"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m10"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m11"      , HACK_B_1    , mapper_S9263B, 0, NULL                },
	{ "sf2m12"      , HACK_B_1    , mapper_S9263B, 0, NULL                },
	{ "sf2m13"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m14"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2m15"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2tlona"    , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2tlonb"    , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2tlonc"    , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2th"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2tha"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2yyc"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2koryu"    , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2koryu2"   , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2amf"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2hf"       , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2hfu"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2hfj"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },
	{ "sf2hfjb"     , HACK_B_4    , mapper_S9263B, 0, NULL                },
	{ "sf2hfjb2"    , HACK_B_4    , mapper_S9263B, 0, NULL                },
	{ "slammast"    , CPS_B_21_QS4, mapper_MB63B , 0, slammast_decode     },
	{ "slammastu"   , CPS_B_21_QS4, mapper_MB63B , 0, slammast_decode     },
	{ "mbomberj"    , CPS_B_21_QS4, mapper_MB63B , 0, slammast_decode     },
	{ "mbombrd"     , CPS_B_21_QS5, mapper_MB63B , 0, slammast_decode     },
	{ "mbombrdj"    , CPS_B_21_QS5, mapper_MB63B , 0, slammast_decode     },
	{ "strider"     , CPS_B_01    , mapper_ST24M1, 1, NULL                },
	{ "striderua"   , CPS_B_01    , mapper_ST24M1, 1, NULL                },
	{ "striderjr"   , CPS_B_21_DEF, mapper_ST24M1, 1, NULL                },
	{ "striderj"    , CPS_B_01    , mapper_ST22B , 1, NULL                },
	{ "unsquad"     , CPS_B_11    , mapper_AR24B , 0, NULL                },
	{ "area88"      , CPS_B_11    , mapper_AR22B , 0, NULL                },
	{ "area88r"     , CPS_B_21_DEF, mapper_AR22B , 0, NULL                },
	{ "varth"       , CPS_B_04    , mapper_VA63B , 0, NULL                },
	{ "varthr1"     , CPS_B_04    , mapper_VA63B , 0, NULL                },
	{ "varthu"      , CPS_B_04    , mapper_VA63B , 0, NULL                },
	{ "varthj"      , CPS_B_21_BT5, mapper_VA22B , 0, NULL                },
	{ "willow"      , CPS_B_03    , mapper_WL24B , 0, NULL                },
	{ "willowj"     , CPS_B_03    , mapper_WL24B , 0, NULL                },
	{ "willowo"     , CPS_B_03    , mapper_WL24B , 0, NULL                },
	{ "wof"         , CPS_B_21_QS1, mapper_TK263B, 0, wof_decode          },
	{ "wofr1"       , CPS_B_21_DEF, mapper_TK263B, 0, wof_decode          },
	{ "wofu"        , CPS_B_21_QS1, mapper_TK263B, 0, wof_decode          },
	{ "wofj"        , CPS_B_21_QS1, mapper_TK263B, 0, wof_decode          },
	{ "wofa"        , CPS_B_21_DEF, mapper_TK263B, 0, wof_decode          },
	{ "wofhfh"      , CPS_B_21_DEF, mapper_TK263B, 0, NULL                },
	{ "wofh"        , HACK_B_6    , mapper_TK263B, 0, NULL                },
	{ "wofha"       , HACK_B_6    , mapper_TK263B, 0, NULL                },
	{ "sgyxz"       , HACK_B_6    , mapper_TK263B, 0, NULL                },
	{ "wof3js"      , CPS_B_21_DEF, mapper_TK263B, 0, NULL                },
	{ "wof3jsa"     , HACK_B_6    , mapper_TK263B, 0, NULL                },
	{ "wof3sj"      , HACK_B_6    , mapper_TK263B, 0, NULL                },
	{ "wof3sja"     , HACK_B_6    , mapper_TK263B, 0, NULL                },
	{ "wofsj"       , HACK_B_6    , mapper_TK263B, 0, NULL                },
	{ "wofsja"      , HACK_B_6    , mapper_TK263B, 0, NULL                },
	{ "wofsjb"      , CPS_B_21_DEF, mapper_TK263B, 0, NULL                },
	{ "wofb"        , CPS_B_21_DEF, mapper_TK263B, 0, NULL                }, // game controls layers at 0x98000c
	{ "wofjh"       , CPS_B_21_QS1, mapper_TK263B, 0, wof_decode          },
	{ "wofah"       , CPS_B_21_DEF, mapper_TK263B, 0, wof_decode          },
	{ "wofaha"      , CPS_B_21_DEF, mapper_TK263B, 0, wof_decode          },
	{ "wofahb"      , CPS_B_21_DEF, mapper_TK263B, 0, wof_decode          },
	{ "sfzch"       , CPS_B_21_DEF, mapper_sfzch , 0, NULL                },
	{ "wofch"       , CPS_B_21_DEF, mapper_sfzch , 0, wof_decode          },
	{ "cps1demo"    , CPS_B_04    , mapper_sfzch , 0, NULL                },
	{ "cps1frog"    , CPS_B_04    , mapper_frog  , 0, NULL                },
	
	// Not included games
	{ "pnicku"      , CPS_B_21_DEF, mapper_PKB10B, 0, NULL                },	// IPS
	{ "sf2cebr"     , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },	// IPS
	{ "sf2ceh"      , CPS_B_21_DEF, mapper_S9263B, 0, NULL                },	// IPS
	{ "slammasa"    , CPS_B_21_QS4, mapper_MB63B , 0, slammast_decode     },	// IPS
	{ "slammash"    , CPS_B_21_QS4, mapper_MB63B , 0, slammast_decode     },	// IPS
	{ "mbombdje"    , CPS_B_21_QS5, mapper_MB63B , 0, slammast_decode     },	// IPS
	{ "mbombrda"    , CPS_B_21_QS5, mapper_MB63B , 0, slammast_decode     },	// IPS
	{ "mbombrdh"    , CPS_B_21_QS5, mapper_MB63B , 0, slammast_decode     },	// IPS
	{ "mbombrdu"    , CPS_B_21_QS5, mapper_MB63B , 0, slammast_decode     },	// IPS
	{ "woffr"       , CPS_B_21_DEF, mapper_TK263B, 0, wof_decode          },	// IPS
	
	// CPS Changer Region Hacks (not included)
	{ "sfach"       , CPS_B_21_DEF, mapper_sfzch , 0, NULL                },
	{ "sfabch"      , CPS_B_21_DEF, mapper_sfzch , 0, NULL                },
	{ "sfzbch"      , CPS_B_21_DEF, mapper_sfzch , 0, NULL                },
	{ "sfzech"      , CPS_B_21_DEF, mapper_sfzch , 0, NULL                },
	{ "sfzhch"      , CPS_B_21_DEF, mapper_sfzch , 0, NULL                },

	{ 0             , 0           , 0            , 0, 0                   }
};

static void SetGameConfig()
{
	const char *GameName = BurnDrvGetTextA(DRV_NAME);
	const struct GameConfig *k = &ConfigTable[0];

	while (k->DriverName) {
		if (strcmp(k->DriverName, GameName) == 0) {
			break;
		}
		++k;
	}

	if (k->DriverName) {
		GameHasStars = k->CpsStars;
		SetCpsBId(k->CpsBId, GameHasStars);
		SetGfxMapper(k->CpsMapperId);
		KabukiDecodeFunction = k->DecodeFunction;
	} else {
#if 1 && defined FBA_DEBUG
		bprintf(PRINT_IMPORTANT, _T("Missing Config Data\n"));
#endif
	}
}

// Driver functions

static UINT32 nCps68KByteswapRomNum = 0;
static UINT32 nCps68KNoByteswapRomNum = 0;
static UINT32 nCpsZ80RomNum = 0;
static UINT32 nCpsTilesRomNum = 0;
static UINT32 nCpsOkim6295RomNum = 0;
static UINT32 nCpsQsoundRomNum = 0;
static UINT32 nCpsPicRomNum = 0;

static INT32 Cps1LoadRoms(INT32 bLoad)
{
	struct BurnRomInfo ri;
	ri.nType = 0;
	ri.nLen = 0;
	INT32 nOffset = -1;
	UINT32 i = 0;
	INT32 nRet = 0;
	
	if (!bLoad) {
		do {
			ri.nLen = 0;
			ri.nType = 0;
			BurnDrvGetRomInfo(&ri, ++nOffset);
			if ((ri.nType & 0xff) == CPS1_68K_PROGRAM_BYTESWAP) {
				nCpsRomLen += ri.nLen;
				nCps68KByteswapRomNum++;
			}
			if ((ri.nType & 0xff) == CPS1_68K_PROGRAM_NO_BYTESWAP) {
				nCpsRomLen += ri.nLen;
				nCps68KNoByteswapRomNum++;
			}
			if ((ri.nType & 0xff) == CPS1_Z80_PROGRAM) {
				nCpsZRomLen += ri.nLen;
				nCpsZ80RomNum++;
			}
			if ((ri.nType & 0xff) == CPS1_TILES) {
				nCpsGfxLen += ri.nLen;
				nCpsTilesRomNum++;
			}
			if ((ri.nType & 0xff) == CPS1_PIC) {
				nCpsPicRomNum++;
			}
			if ((ri.nType & 0xff) == CPS1_OKIM6295_SAMPLES) {
				nCpsAdLen += ri.nLen;
				nCpsOkim6295RomNum++;
			}
			if ((ri.nType & 0xff) == CPS1_QSOUND_SAMPLES) {
				Cps1Qs = 1;
				nCpsQSamLen += ri.nLen;
				nCpsQsoundRomNum++;
			}
			i++;
			
		} while (ri.nLen);
		
		if (Cps1Qs) nCpsZRomLen *= 2;
		if (GameHasStars) nCpsGfxLen += 0x2000;
		if (PangEEP) nCpsGfxLen *= 2;
		if (nCpsPicRomNum) Cps1DisablePSnd = 1;
		
#if 1 && defined FBA_DEBUG
		if (nCpsRomLen) bprintf(PRINT_IMPORTANT, _T("68K Rom Length %06X, (%i roms byteswapped, %i roms not byteswapped)\n"), nCpsRomLen, nCps68KByteswapRomNum, nCps68KNoByteswapRomNum);
		if (nCpsZRomLen) bprintf(PRINT_IMPORTANT, _T("Z80 Rom Length %06X, (%i roms)\n"), nCpsZRomLen, nCpsZ80RomNum);
		if (nCpsGfxLen) bprintf(PRINT_IMPORTANT, _T("Tile Rom Length %08X, (%i roms)\n"), nCpsGfxLen, nCpsTilesRomNum);
		if (nCpsAdLen) bprintf(PRINT_IMPORTANT, _T("OKIM6295 Rom Length %08X, (%i roms)\n"), nCpsAdLen, nCpsOkim6295RomNum);
		if (nCpsQSamLen) bprintf(PRINT_IMPORTANT, _T("QSound Rom Length %08X, (%i roms)\n"), nCpsQSamLen, nCpsQsoundRomNum);
#endif
	}

	if (bLoad) {
		INT32 Offset = 0;
		
		i = 0;
		while (i < nCps68KByteswapRomNum + nCps68KNoByteswapRomNum) {
			BurnDrvGetRomInfo(&ri, i + 0);
			if ((ri.nType & 0xff) == CPS1_68K_PROGRAM_BYTESWAP) {
				nRet = BurnLoadRom(CpsRom + Offset + 1, i + 0, 2); if (nRet) return 1;
				nRet = BurnLoadRom(CpsRom + Offset + 0, i + 1, 2); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 1);
				Offset += ri.nLen;
				
				i += 2;
			}
			
			if ((ri.nType & 0xff) == CPS1_68K_PROGRAM_NO_BYTESWAP) {
				nRet = BurnLoadRom(CpsRom + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
				
				i++;
			}
		}
		
		// Graphics
		if (nCpsGfxLen) {
			Offset = 0;
			i = nCps68KByteswapRomNum + nCps68KNoByteswapRomNum;
			while (i < nCps68KByteswapRomNum + nCps68KNoByteswapRomNum + nCpsTilesRomNum) {
				if (Cps1GfxLoadCallbackFunction) {
					Cps1GfxLoadCallbackFunction(i);
				
					i += nCpsTilesRomNum;
				} else {
					BurnDrvGetRomInfo(&ri, i + 0);

					if (ri.nLen < 0x80000) {
						CpsLoadTilesByte(CpsGfx + Offset, i);
		
						BurnDrvGetRomInfo(&ri, i + 0);
						Offset += ri.nLen;
						BurnDrvGetRomInfo(&ri, i + 1);
						Offset += ri.nLen;
						BurnDrvGetRomInfo(&ri, i + 2);
						Offset += ri.nLen;
						BurnDrvGetRomInfo(&ri, i + 3);
						Offset += ri.nLen;
						BurnDrvGetRomInfo(&ri, i + 4);
						Offset += ri.nLen;
						BurnDrvGetRomInfo(&ri, i + 5);
						Offset += ri.nLen;
						BurnDrvGetRomInfo(&ri, i + 6);
						Offset += ri.nLen;
						BurnDrvGetRomInfo(&ri, i + 7);
						Offset += ri.nLen;
						
						i += 8;
					} else {
						if (PangEEP) {
							CpsLoadTilesPang(CpsGfx + Offset, i);
			
							BurnDrvGetRomInfo(&ri, i + 0);
							Offset += ri.nLen;
							BurnDrvGetRomInfo(&ri, i + 1);
							Offset += ri.nLen;
							BurnDrvGetRomInfo(&ri, i + 2);
							Offset += ri.nLen;
							BurnDrvGetRomInfo(&ri, i + 3);
							Offset += ri.nLen;
					
							i += 4;
						} else {
							if (nCpsTilesRomNum < 4) {
								// Handle this seperately
								i += nCpsTilesRomNum;
							} else {
								CpsLoadTiles(CpsGfx + Offset, i);
					
								BurnDrvGetRomInfo(&ri, i + 0);
								Offset += ri.nLen;
								BurnDrvGetRomInfo(&ri, i + 1);
								Offset += ri.nLen;
								BurnDrvGetRomInfo(&ri, i + 2);
								Offset += ri.nLen;
								BurnDrvGetRomInfo(&ri, i + 3);
								Offset += ri.nLen;
				
								i += 4;
							}
						}
					}
				}
			}			
		}
		
		// Z80 Program
		if (nCpsZRomLen) {
			Offset = 0;
			for (i = nCps68KByteswapRomNum + nCps68KNoByteswapRomNum + nCpsTilesRomNum; i < nCps68KByteswapRomNum + nCps68KNoByteswapRomNum + nCpsTilesRomNum + nCpsZ80RomNum; i++) {
				BurnLoadRom(CpsZRom + Offset, i, 1);
				
				if (Cps1Qs) BurnLoadRom(CpsEncZRom + Offset, i, 1);
			
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
		
		// Pic (skip for now)
		if (nCpsPicRomNum) {
			BurnDrvGetRomInfo(&ri, i);
		}
		
		// OKIM6295 Samples
		if (nCpsAdLen) {
			Offset = 0;
			for (i = nCps68KByteswapRomNum + nCps68KNoByteswapRomNum + nCpsTilesRomNum + nCpsZ80RomNum; i < nCps68KByteswapRomNum + nCps68KNoByteswapRomNum + nCpsTilesRomNum + nCpsZ80RomNum + nCpsOkim6295RomNum; i++) {
				BurnLoadRom(CpsAd + Offset, i, 1);
			
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
		
		// QSound Samples
		if (nCpsQSamLen) {
			Offset = 0;
			for (i = nCps68KByteswapRomNum + nCps68KNoByteswapRomNum + nCpsTilesRomNum + nCpsZ80RomNum; i < nCps68KByteswapRomNum + nCps68KNoByteswapRomNum + nCpsTilesRomNum + nCpsZ80RomNum + nCpsQsoundRomNum; i++) {
				BurnLoadRom((UINT8*)CpsQSam + Offset, i, 1);
			
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
	}

	return nRet;
}

typedef void (*AmendProgRom)();
static AmendProgRom AmendProgRomCallback;

static INT32 DrvInit()
{
	INT32 nRet = 0;
	
	SetGameConfig();
    
	Cps1LoadRoms(0);

	Cps = 1;
	nRet = CpsInit(); if (nRet != 0) return 1;
	
	Cps1LoadRoms(1);
	
	if (AmendProgRomCallback) AmendProgRomCallback();
	
	SetGameConfig();
	
	if (Cps1Qs) {
		KabukiDecodeFunction();
	}
	
	nRet = CpsRunInit(); if (nRet != 0) return 1;
	
	Cps1VBlankIRQLine = 2;
	
	return 0;
}

static INT32 DrvExit()
{
	CpsRunExit();
	CpsExit();
	
	nCpsRomLen = 0;
	nCpsZRomLen = 0;
	nCpsGfxLen = 0;
	nCpsAdLen = 0;
	nCpsQSamLen = 0;
	nCpsPicRomNum = 0;
	
	nCps68KByteswapRomNum = 0;
	nCps68KNoByteswapRomNum = 0;
	nCpsZ80RomNum = 0;
	nCpsTilesRomNum = 0;
	nCpsOkim6295RomNum = 0;
	nCpsQsoundRomNum = 0;
	KabukiDecodeFunction = NULL;
	GameHasStars = 0;
	
	AmendProgRomCallback = NULL;
	
	CpsLayer1XOffs = 0;
	CpsLayer2XOffs = 0;
	CpsLayer3XOffs = 0;
	CpsLayer1YOffs = 0;
	CpsLayer2YOffs = 0;
	CpsLayer3YOffs = 0;
	CpsDrawSpritesInReverse = 0;
	Cps1OverrideLayers = 0;
	nCps1Layers[0] = -1;
	nCps1Layers[1] = -1;
	nCps1Layers[2] = -1;
	nCps1Layers[3] = -1;

	Cps = 0;
	Cps1Qs = 0;
	Cps1DisablePSnd = 0;
	Forgottn = 0;
	Ghouls = 0;
	Kodb = 0;
	PangEEP = 0;
	Cps1LockSpriteList910000 = 0;
	Cps1DisableBgHi = 0;
	Dinohunt = 0;
	Sf2thndr = 0;
	Port6SoundWrite = 0;
	
	Cps1QsHack = 0;
	
	Cps1GfxLoadCallbackFunction = NULL;
	
	BurnFree(CpsBootlegSpriteRam);
	
	return 0;
}

static INT32 TwelveMhzInit()
{
	nCPS68KClockspeed = 12000000;
	
	return DrvInit();
}

static INT32 CpsBootlegSpriteRamScanCallback(INT32 nAction, INT32*)
{
	if (nAction & ACB_MEMORY_RAM) {
		struct BurnArea ba;
		memset(&ba, 0, sizeof(ba));

		ba.Data = CpsBootlegSpriteRam;
		ba.nLen = 0x4000;
		ba.szName = "CpsBootlegSpriteRam";
		BurnAcb(&ba);
	}
	
	return 0;
}

static INT32 CaptcommbInit()
{
	CpsLayer1XOffs = -8;
	CpsLayer2XOffs = -11;
	CpsLayer3XOffs = -12;
	CpsDrawSpritesInReverse = 1;
	Cps1DetectEndSpriteList8000 = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesCaptcommb;
	
	return DrvInit();
}

static INT32 Captcommb2Init()
{
	Cps1DisablePSnd = 1;
	CpsRunInitCallbackFunction = Sf2mdtSoundInit;
	CpsRunResetCallbackFunction = Sf2mdtSoundReset;
	CpsRunExitCallbackFunction = Sf2mdtSoundExit;
	CpsRunFrameStartCallbackFunction = Sf2mdtSoundFrameStart;
	CpsRunFrameEndCallbackFunction = Sf2mdtSoundFrameEnd;
	CpsRWSoundCommandCallbackFunction = Sf2mdtSoundCommand;
	CpsMemScanCallbackFunction = Sf2mdtScanSound;
	
	return DrvInit();
}

UINT8 __fastcall CawingblInputReadByte(UINT32 a)
{
	switch (a) {
		case 0x882000: {
			return ~Inp000;
		}
		
		case 0x882001: {
			return ~Inp001;
		}
		
		case 0x882008: {
			return ~Inp018;
		}
		
		case 0x88200a: {
			return ~Cpi01A;
		}
		
		case 0x88200c: {
			return ~Cpi01C;
		}
		
		case 0x88200e: {
			return ~Cpi01E;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Read Byte %x\n"), a);
		}
	}

	return 0;
}

UINT16 __fastcall CawingblInputReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Input Read Word %x\n"), a);
		}
	}

	return 0;
}

void __fastcall CawingblInputWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x882006: {
			FcrashSoundCommand(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Write Byte %x, %x\n"), a, d);
		}
	}
}

void __fastcall CawingblInputWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x882006: {
			FcrashSoundCommand(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Write Word %x, %x\n"), a, d);
		}
	}
}

static INT32 CawingblInit()
{
	INT32 nRet = 0;
	
	Cps1DisablePSnd = 1;
	bCpsUpdatePalEveryFrame = 1;
	
	CpsLayer1XOffs = -63;
	CpsLayer2XOffs = -62;
	CpsLayer3XOffs = -65;

	Cps1ObjGetCallbackFunction = WofhObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesCawingbl;
	CpsRunInitCallbackFunction = FcrashSoundInit;
	CpsRunResetCallbackFunction = FcrashSoundReset;
	CpsRunExitCallbackFunction = FcrashSoundExit;
	CpsRunFrameStartCallbackFunction = FcrashSoundFrameStart;
	CpsRunFrameEndCallbackFunction = FcrashSoundFrameEnd;
	CpsMemScanCallbackFunction = FcrashScanSound;
	
	nRet = DrvInit();
	
	SekOpen(0);
	SekMapHandler(1, 0x882000, 0x882fff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(1, CawingblInputReadByte);
	SekSetReadWordHandler(1, CawingblInputReadWord);
	SekSetWriteByteHandler(1, CawingblInputWriteByte);
	SekSetWriteWordHandler(1, CawingblInputWriteWord);
	SekClose();
	
	Cps1VBlankIRQLine = 6;
	
	return 0;
}

static INT32 Cps1demoInit()
{
	bCpsUpdatePalEveryFrame = 1;

	return DrvInit();
}

void __fastcall DinopicScrollWrite(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x980000: {
			// scroll1 y
			*((UINT16*)(CpsReg + 0x0e)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980002: {
			// scroll1 x
			*((UINT16*)(CpsReg + 0x0c)) = BURN_ENDIAN_SWAP_INT16(d - 0x40);
			return;
		}
		
		case 0x980004: {
			// scroll2 y
			*((UINT16*)(CpsReg + 0x12)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980006: {
			// scroll2 x
			*((UINT16*)(CpsReg + 0x10)) = BURN_ENDIAN_SWAP_INT16(d - 0x40);
			return;
		}
		
		case 0x980008: {
			// scroll3 y
			*((UINT16*)(CpsReg + 0x16)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x98000a: {
			// scroll3 x
			*((UINT16*)(CpsReg + 0x14)) = BURN_ENDIAN_SWAP_INT16(d - 0x40);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write Word %x, %x\n"), a, d);
		}
	}
}

void __fastcall DinopicLayerWrite(UINT32 a, UINT16 d)
{
	if (a == 0x800222) {
		*((UINT16*)(CpsReg + 0x06)) = d;
		return;
	}
	
	// Send anything else through the main handler
	SEK_DEF_WRITE_WORD(0, a, d);
}

static INT32 DinopicInit()
{
	INT32 nRet = 0;
	
	Cps1DisablePSnd = 1;
	CpsBootlegEEPROM = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesDinopic;
	Cps1ObjGetCallbackFunction = DinopicObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsMemScanCallbackFunction = CpsBootlegSpriteRamScanCallback;
		
	nRet = TwelveMhzInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x991fff, SM_RAM);
	SekMapHandler(1, 0x980000, 0x98000f, SM_WRITE);
	SekSetWriteWordHandler(1, DinopicScrollWrite);
	SekMapHandler(2, 0x800200, 0x8002ff, SM_WRITE);
	SekSetWriteWordHandler(2, DinopicLayerWrite);
	SekClose();
	
	return nRet;
}

static void DinohCallback()
{
	// Patch Q-Sound Test ???
	*((UINT16*)(CpsRom + 0xaacf4)) = 0x4e71;
}

static INT32 DinohInit()
{
	Cps1QsHack = 1;
	AmendProgRomCallback = DinohCallback;
	
	return TwelveMhzInit();
}

static void DinotCallback()
{
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x200000);
	if (TempRom) {
		memcpy(TempRom, CpsRom, 0x200000);
		memset(CpsRom, 0, 0x200000);
		memcpy(CpsRom + 0x080000, TempRom + 0x000000, 0x80000);
		memcpy(CpsRom + 0x000000, TempRom + 0x080000, 0x80000);
		memcpy(CpsRom + 0x180000, TempRom + 0x100000, 0x80000);
		memcpy(CpsRom + 0x100000, TempRom + 0x180000, 0x80000);
		BurnFree(TempRom);
	}
	
	// Patch Q-Sound Test ???
	*((UINT16*)(CpsRom + 0xaacf4)) = 0x4e71;
}

static INT32 DinotInit()
{
	Cps1QsHack = 1;
	AmendProgRomCallback = DinotCallback;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesHack160;
	
	return TwelveMhzInit();
}

static INT32 DinotpicInit()
{
	INT32 nRet = 0;
	
	Cps1DisablePSnd = 1;
	CpsBootlegEEPROM = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesHack160;
	Cps1ObjGetCallbackFunction = DinopicObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsMemScanCallbackFunction = CpsBootlegSpriteRamScanCallback;
	
	nRet = TwelveMhzInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x991fff, SM_RAM);
	SekMapHandler(1, 0x980000, 0x98000f, SM_WRITE);
	SekSetWriteWordHandler(1, DinopicScrollWrite);
	SekMapHandler(2, 0x800200, 0x8002ff, SM_WRITE);
	SekSetWriteWordHandler(2, DinopicLayerWrite);
	SekClose();
	
	return nRet;
}

UINT8 __fastcall DinohuntQSharedRamRead(UINT32 /*a*/)
{
	return 0xff;
}

static INT32 DinohuntInit()
{
	INT32 nRet = 0;
	
	Dinohunt = 1;
	CpsBootlegEEPROM = 1;
	
	nRet = TwelveMhzInit();
	
	SekOpen(0);
	SekMapHandler(1, 0xf18000, 0xf19fff, SM_READ);
	SekSetReadByteHandler(1, DinohuntQSharedRamRead);
	SekClose();
	
	return nRet;
}

UINT8 __fastcall FcrashInputReadByte(UINT32 a)
{
	switch (a) {
		case 0x880000: {
			return ~Inp000;
		}
		
		case 0x880008: {
			return ~Inp018;
		}
		
		case 0x88000a: {
			return ~Cpi01A;
		}
		
		case 0x88000c: {
			return ~Cpi01C;
		}
		
		case 0x88000e: {
			return ~Cpi01E;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Read Byte %x\n"), a);
		}
	}
	
	return 0;
}

UINT16 __fastcall FcrashInputReadWord(UINT32 a)
{
	switch (a) {
		case 0x880000: {
			return (~Inp000 << 8) | ~Inp001;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Read Word %x\n"), a);
		}
	}
	
	return 0;
}

void __fastcall FcrashInputWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Input Write Byte %x, %x\n"), a, d);
		}
	}
}

void __fastcall FcrashInputWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x880006: {
			FcrashSoundCommand(d);
			return;
		}
		
		case 0x890000: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Write word %x, %x\n"), a, d);
		}
	}
}

static INT32 FcrashInit()
{
	Cps1DisablePSnd = 1;
	bCpsUpdatePalEveryFrame = 1;
	CpsLayer1XOffs = -0x3f;
	CpsLayer2XOffs = -0x3c;
	CpsLayer3XOffs = 0xffc0;
	
	Cps1GfxLoadCallbackFunction = CpsLoadTilesFcrash;
	Cps1ObjGetCallbackFunction = FcrashObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsRunInitCallbackFunction = FcrashSoundInit;
	CpsRunResetCallbackFunction = FcrashSoundReset;
	CpsRunExitCallbackFunction = FcrashSoundExit;
	CpsRunFrameStartCallbackFunction = FcrashSoundFrameStart;
	CpsRunFrameEndCallbackFunction = FcrashSoundFrameEnd;
	CpsMemScanCallbackFunction = FcrashScanSound;
	
	INT32 nRet = DrvInit();
	
	SekOpen(0);
	SekMapHandler(1, 0x880000, 0x89ffff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(1, FcrashInputReadByte);
	SekSetReadWordHandler(1, FcrashInputReadWord);
	SekSetWriteByteHandler(1, FcrashInputWriteByte);
	SekSetWriteWordHandler(1, FcrashInputWriteWord);
	SekClose();
	
	return nRet;
}

static INT32 ForgottnInit()
{
	INT32 nRet = 0;
	
	Forgottn = 1;
	
	nRet = DrvInit();
	
	CpsStar = CpsGfx + nCpsGfxLen - 0x2000;
	
	CpsLoadStars(CpsStar, 9);
	
	return nRet;
}

static INT32 ForgottnAltGfxInit()
{
	INT32 nRet = 0;
	
	Forgottn = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesForgottn;
	
	nRet = DrvInit();
	
	CpsStar = CpsGfx + nCpsGfxLen - 0x2000;
	
	CpsLoadStarsForgottnAlt(CpsStar, 15);
	
	return nRet;
}

static INT32 ForgottnAltGfxuInit()
{
	INT32 nRet = 0;
	
	Forgottn = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesForgottnu;
	
	nRet = DrvInit();
	
	CpsStar = CpsGfx + nCpsGfxLen - 0x2000;
	
	CpsLoadStarsForgottnAlt(CpsStar, 15);
	
	return nRet;
}

static void GhoulsCallback()
{
	BurnByteswap(CpsRom + 0x080000, 0x080000);
}

static INT32 GhoulsInit()
{
	Ghouls = 1;
	AmendProgRomCallback = GhoulsCallback;

	return DrvInit();
}

static INT32 DaimakaiInit()
{
	Ghouls = 1;
	
	return DrvInit();
}

void __fastcall Daimakaib88WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x880000: {
			*((UINT16*)(CpsReg + nCpsPalCtrlReg)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Write word %x, %x\n"), a, d);
}

void __fastcall Daimakaib98WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x980000: {
			// scroll1 y
			*((UINT16*)(CpsReg + 0x0e)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980002: {
			// scroll1 x
			*((UINT16*)(CpsReg + 0x0c)) = BURN_ENDIAN_SWAP_INT16(d - 0x40);
			return;
		}
		
		case 0x980004: {
			// scroll2 y
			*((UINT16*)(CpsReg + 0x12)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980006: {
			// scroll2 x
			*((UINT16*)(CpsReg + 0x10)) = BURN_ENDIAN_SWAP_INT16(d - 0x40);
			return;
		}
		
		case 0x980008: {
			// scroll3 y
			*((UINT16*)(CpsReg + 0x16)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x98000a: {
			// scroll3 x
			*((UINT16*)(CpsReg + 0x14)) = BURN_ENDIAN_SWAP_INT16(d - 0x40);
			return;
		}
		
		case 0x98000c: {
			// This seems to control layer order and enable
			switch (d) {
				case 0: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 3;
					break;
				}
				
				case 1: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = -1;
					nCps1Layers[3] = 3;
					break;
				}
				
				case 2: {
					nCps1Layers[0] = 3;
					nCps1Layers[1] = -1;
					nCps1Layers[2] = -1;
					nCps1Layers[3] = 1;
					break;
				}
				
				case 6: {
					nCps1Layers[0] = -1;
					nCps1Layers[1] = -1;
					nCps1Layers[2] = -1;
					nCps1Layers[3] = -1;
					break;
				}
				
				default: {
					nCps1Layers[0] = 0;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 0;
					nCps1Layers[3] = 0;
					bprintf(PRINT_IMPORTANT, _T("Unknown value written at 0x98000c %x\n"), d);
				}
			}
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Write word %x, %x\n"), a, d);
}

void __fastcall DaimakaibFFWriteByte(UINT32 a, UINT8 d)
{
	CpsRamFF[((a & 0xffff) ^ 1)] = d;
}

void __fastcall DaimakaibFFWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xff0680: {
			*((UINT16*)(CpsReg + MaskAddr[1])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff0682: {
			*((UINT16*)(CpsReg + MaskAddr[2])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff0684: {
			*((UINT16*)(CpsReg + MaskAddr[3])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
	}
	
	UINT16 *RAM = (UINT16*)CpsRamFF;
	RAM[((a & 0xffff) >> 1)] = d;
}

static INT32 DaimakaibInit()
{
	Ghouls = 1;
	Port6SoundWrite = 1;
	Cps1OverrideLayers = 1;
	Cps1ObjGetCallbackFunction = DaimakaibObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsMemScanCallbackFunction = CpsBootlegSpriteRamScanCallback;
	
	INT32 nRet = DrvInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x991fff, SM_RAM);
	SekMapHandler(1, 0x880000, 0x88ffff, SM_WRITE);
	SekSetWriteWordHandler(1, Daimakaib88WriteWord);
	SekMapHandler(2, 0x980000, 0x98ffff, SM_WRITE);
	SekSetWriteWordHandler(2, Daimakaib98WriteWord);
	SekMapHandler(3, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(3, DaimakaibFFWriteByte);
	SekSetWriteWordHandler(3, DaimakaibFFWriteWord);
	SekClose();
	
	return nRet;
}

void __fastcall Knightsb98WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x980000: {
			// scroll1 y
			*((UINT16*)(CpsReg + 0x0e)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980002: {
			// scroll1 x
			*((UINT16*)(CpsReg + 0x0c)) = BURN_ENDIAN_SWAP_INT16(d - 0x3e);
			return;
		}
		
		case 0x980004: {
			// scroll2 y
			*((UINT16*)(CpsReg + 0x12)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980006: {
			// scroll2 x
			*((UINT16*)(CpsReg + 0x10)) = BURN_ENDIAN_SWAP_INT16(d - 0x3c);
			return;
		}
		
		case 0x980008: {
			// scroll3 y
			*((UINT16*)(CpsReg + 0x16)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x98000a: {
			// scroll3 x
			*((UINT16*)(CpsReg + 0x14)) = BURN_ENDIAN_SWAP_INT16(d - 0x40);
			return;
		}
		
		case 0x98000c: {
			// This seems to control layer order and enable
			switch (d) {
				case 0x0000:
				case 0x001f:
				case 0x00ff:
				case 0x07ff:
				case 0x5800:
				case 0x5f00: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 3;
					break;
				}
				
				case 0x2000: {
					nCps1Layers[0] = 0;
					nCps1Layers[1] = 1;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 3;
					break;
				}
				
				case 0x80ff:
				case 0x87ff:
				case 0xd800: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 3;
					nCps1Layers[3] = 2;
					break;
				}
				
				case 0xa000: {
					nCps1Layers[0] = 2;
					nCps1Layers[1] = 1;
					nCps1Layers[2] = 0;
					nCps1Layers[3] = 3;
					break;
				}
				
				default: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 3;
					bprintf(PRINT_IMPORTANT, _T("Unknown value written at 0x98000c %x\n"), d);
				}
			}
			return;
		}
		
		case 0x980020: {
			*((UINT16*)(CpsReg + MaskAddr[1])) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980022: {
			*((UINT16*)(CpsReg + MaskAddr[2])) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980024: {
			*((UINT16*)(CpsReg + MaskAddr[3])) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write word %x, %x\n"), a, d);
		}
	}
}

static INT32 KnightsbScanCallback(INT32 nAction, INT32*pnMin)
{
	CpsBootlegSpriteRamScanCallback(nAction, pnMin);
	Sf2mdtScanSound(nAction, pnMin);
	
	return 0;
}

static INT32 KnightsbInit()
{
	Cps1DisablePSnd = 1;
	bCpsUpdatePalEveryFrame = 1;
	Cps1OverrideLayers = 1;
	Port6SoundWrite = 1;
	
	Cps1ObjGetCallbackFunction = DinopicObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsRunInitCallbackFunction = Sf2mdtSoundInit;
	CpsRunResetCallbackFunction = Sf2mdtSoundReset;
	CpsRunExitCallbackFunction = Sf2mdtSoundExit;
	CpsRunFrameStartCallbackFunction = Sf2mdtSoundFrameStart;
	CpsRunFrameEndCallbackFunction = Sf2mdtSoundFrameEnd;
	CpsRWSoundCommandCallbackFunction = Sf2mdtSoundCommand;
	CpsMemScanCallbackFunction = Sf2mdtScanSound;
	CpsMemScanCallbackFunction = KnightsbScanCallback;
	
	INT32 nRet = DrvInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x993fff, SM_RAM);
	SekMapHandler(1, 0x980000, 0x98ffff, SM_WRITE);
	SekSetWriteWordHandler(1, Knightsb98WriteWord);
	SekClose();
	
	return nRet;
}

static INT32 Knightsb2Init()
{
	CpsDrawSpritesInReverse = 1;
	Cps1DetectEndSpriteList8000 = 1;
	
	return DrvInit();
}

void __fastcall Kodb98WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x98000c: {
			*((UINT16*)(CpsReg + nCpsLcReg)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980020: {
			*((UINT16*)(CpsReg + MaskAddr[1])) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980022: {
			*((UINT16*)(CpsReg + MaskAddr[2])) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
	}
	
	bprintf(PRINT_IMPORTANT, _T("Unknown value written at %x %x\n"), a, d);
}

static INT32 KodbInit()
{
	INT32 nRet = 0;

	Kodb = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesKodb;
	Cps1ObjGetCallbackFunction = KodbObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	
	nRet = DrvInit();
	
	SekOpen(0);
	SekMapHandler(1, 0x980000, 0x99ffff, SM_WRITE);
	SekSetWriteWordHandler(1, Kodb98WriteWord);
	SekClose();
	
	*((UINT16*)(CpsReg + MaskAddr[0])) = 0x0000;
	*((UINT16*)(CpsReg + MaskAddr[3])) = BURN_ENDIAN_SWAP_INT16(0xff00);

	return nRet;
}

static INT32 KodhInit()
{
	Kodh = 1;
	
	return DrvInit();
}

static INT32 MercsInit()
{
	Cps1DisableBgHi = 1;

	return DrvInit();
}

static INT32 Pang3bInit()
{
	PangEEP = 1;

	return TwelveMhzInit();
}

static void Pang3Callback()
{
	for (INT32 i = 0x80000; i < 0x100000; i += 2) {
		INT32 src = CpsRom[i];
		INT32 dst = src & 0xff00;
		if ( src & 0x01) dst ^= 0x04;
		if ( src & 0x02) dst ^= 0x21;
		if ( src & 0x04) dst ^= 0x01;
		if (~src & 0x08) dst ^= 0x50;
		if ( src & 0x10) dst ^= 0x40;
		if ( src & 0x20) dst ^= 0x06;
		if ( src & 0x40) dst ^= 0x08;
		if (~src & 0x80) dst ^= 0x88;
		CpsRom[i] = (UINT8)dst;
  	}
}

static INT32 Pang3Init()
{
	AmendProgRomCallback = Pang3Callback;
	
	return Pang3bInit();
}

static UINT16 PunipicPriorityValue = 0;

UINT8 __fastcall PunipicF18Read(UINT32)
{
	return 0xff;
}

void __fastcall Punipic98WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x980000: {
			// scroll1 y
			*((UINT16*)(CpsReg + 0x0e)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980002: {
			// scroll1 x
			*((UINT16*)(CpsReg + 0x0c)) = BURN_ENDIAN_SWAP_INT16(d - 0x46);
			return;
		}
		
		case 0x980004: {
			// scroll2 y
			*((UINT16*)(CpsReg + 0x12)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980006: {
			// scroll2 x
			*((UINT16*)(CpsReg + 0x10)) = BURN_ENDIAN_SWAP_INT16(d - 0x40);
			return;
		}
		
		case 0x980008: {
			// scroll3 y
			*((UINT16*)(CpsReg + 0x16)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x98000a: {
			// scroll3 x
			*((UINT16*)(CpsReg + 0x14)) = BURN_ENDIAN_SWAP_INT16(d - 0x46);
			return;
		}
		
		case 0x98000c: {
			PunipicPriorityValue = d;
			return;
		}
		
		case 0x98000e: {
			// layer enable and order appears to be handled by writes here and at 0x98000c
			if (d == 0x0000) {
				switch (PunipicPriorityValue) {
					case 0x24: {
						nCps1Layers[0] = 1;
						nCps1Layers[1] = 0;
						nCps1Layers[2] = 3;
						nCps1Layers[3] = 2;
						return;
					}
					
					case 0x54:{
						nCps1Layers[0] = 1;
						nCps1Layers[1] = 0;
						nCps1Layers[2] = 2;
						nCps1Layers[3] = 3;
						return;
					}
					
					case 0x64: {
						nCps1Layers[0] = 1;
						nCps1Layers[1] = 0;
						nCps1Layers[2] = 2;
						nCps1Layers[3] = 3;
						return;
					}
					
					case 0x7c: {
						nCps1Layers[0] = 0;
						nCps1Layers[1] = 1;
						nCps1Layers[2] = 3;
						nCps1Layers[3] = 2;
						return;
					}
					
					default: {
						bprintf(PRINT_NORMAL, _T("Unknown PunipicPriorityValue %x when 0x98000e is %x\n"), PunipicPriorityValue, d);
						return;
					}
				}
			}
			
			if (d == 0xffff) {
				switch (PunipicPriorityValue) {
					case 0x24: {
						nCps1Layers[0] = 1;
						nCps1Layers[1] = 0;
						nCps1Layers[2] = -1;
						nCps1Layers[3] = 3;
						return;
					}
					
					case 0x54: {
						nCps1Layers[0] = 1;
						nCps1Layers[1] = 0;
						nCps1Layers[2] = 2;
						nCps1Layers[3] = -1;
						return;
					}
					
					case 0x64: {
						nCps1Layers[0] = 1;
						nCps1Layers[1] = 0;
						nCps1Layers[2] = 2;
						nCps1Layers[3] = -1;
						return;
					}
					
					default: {
						bprintf(PRINT_NORMAL, _T("Unknown PunipicPriorityValue %x when 0x98000e is %x\n"), PunipicPriorityValue, d);
						return;
					}
				}
			}
			
			bprintf(PRINT_NORMAL, _T("Unknown value written to 0x98000e %x\n"), d);
			
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Write Word %x, %x\n"), a, d);
}

void __fastcall PunipicFFWriteByte(UINT32 a, UINT8 d)
{
	CpsRamFF[((a & 0xffff) ^ 1)] = d;
}

void __fastcall PunipicFFWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xff5b30: {
			*((UINT16*)(CpsReg + MaskAddr[1])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff5b32: {
			*((UINT16*)(CpsReg + MaskAddr[2])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff5b34: {
			*((UINT16*)(CpsReg + MaskAddr[3])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff5b8a: {
			// scroll 1 ram offset
			*((UINT16*)(CpsReg + 0x02)) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff5b8c: {
			// scroll 2 ram offset
			*((UINT16*)(CpsReg + 0x04)) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff5b8e: {
			// scroll 3 ram offset
			*((UINT16*)(CpsReg + 0x06)) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
	}
	
	UINT16 *RAM = (UINT16*)CpsRamFF;
	RAM[((a & 0xffff) >> 1)] = d;
}

static INT32 PunipicScanCallback(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(PunipicPriorityValue);
	}

	return CpsBootlegSpriteRamScanCallback(nAction, pnMin);
}

static INT32 PunipicInit()
{
	Cps1DisablePSnd = 1;
	bCpsUpdatePalEveryFrame = 1;
	Cps1OverrideLayers = 1;
	CpsBootlegEEPROM = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesDinopic;
	Cps1ObjGetCallbackFunction = DinopicObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsMemScanCallbackFunction = PunipicScanCallback;
	
	INT32 nRet = TwelveMhzInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x993fff, SM_RAM);
	SekMapHandler(1, 0xf18000, 0xf19fff, SM_READ);
	SekSetReadByteHandler(1, PunipicF18Read);
	SekMapHandler(2, 0x980000, 0x980fff, SM_WRITE);
	SekSetWriteWordHandler(2, Punipic98WriteWord);
	SekMapHandler(3, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(3, PunipicFFWriteByte);
	SekSetWriteWordHandler(3, PunipicFFWriteWord);
	SekClose();
	
	return nRet;
}

static INT32 PunipicExit()
{
	PunipicPriorityValue = 0;
	
	return DrvExit();
}

static INT32 Punipic2Init()
{
	Cps1DisablePSnd = 1;
	bCpsUpdatePalEveryFrame = 1;
	Cps1OverrideLayers = 1;
	CpsBootlegEEPROM = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesHack160Alt;
	Cps1ObjGetCallbackFunction = DinopicObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsMemScanCallbackFunction = PunipicScanCallback;
	
	INT32 nRet = TwelveMhzInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x993fff, SM_RAM);
	SekMapHandler(1, 0xf18000, 0xf19fff, SM_READ);
	SekSetReadByteHandler(1, PunipicF18Read);
	SekMapHandler(2, 0x980000, 0x980fff, SM_WRITE);
	SekSetWriteWordHandler(2, Punipic98WriteWord);
	SekMapHandler(3, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(3, PunipicFFWriteByte);
	SekSetWriteWordHandler(3, PunipicFFWriteWord);
	SekClose();
	
	return nRet;
}

static INT32 Punipic3Init()
{
	Cps1DisablePSnd = 1;
	bCpsUpdatePalEveryFrame = 1;
	Cps1OverrideLayers = 1;
	CpsBootlegEEPROM = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesHack160;
	Cps1ObjGetCallbackFunction = DinopicObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsMemScanCallbackFunction = PunipicScanCallback;
	
	INT32 nRet = TwelveMhzInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x993fff, SM_RAM);
	SekMapHandler(1, 0xf18000, 0xf19fff, SM_READ);
	SekSetReadByteHandler(1, PunipicF18Read);
	SekMapHandler(2, 0x980000, 0x980fff, SM_WRITE);
	SekSetWriteWordHandler(2, Punipic98WriteWord);
	SekMapHandler(3, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(3, PunipicFFWriteByte);
	SekSetWriteWordHandler(3, PunipicFFWriteWord);
	SekClose();
	
	return nRet;
}

static INT32 QadInit()
{
	Cps1DisableBgHi = 1;

	return TwelveMhzInit();
}

static INT32 Sf2ebblInit()
{
	INT32 nRet = DrvInit();
	
	// load bootleg tiles over original tiles
	memset(CpsGfx + 0x400000, 0, 0x80000);
	CpsLoadTilesSf2ebbl(CpsGfx + 0x400000, 19);
	
	Cps1LockSpriteList910000 = 1;
	
	CpsLayer1XOffs = -12;
	CpsLayer1YOffs = 1;
	CpsLayer2XOffs = -14;
	CpsLayer2YOffs = 1;
	CpsLayer3XOffs = -16;
	CpsLayer3YOffs = 1;
	CpsDrawSpritesInReverse = 1;
	
	return nRet;
}

static INT32 Sf2sttInit()
{
	Cps1GfxLoadCallbackFunction = CpsLoadTilesSf2stt;
	
	INT32 nRet = DrvInit();
	
	// load bootleg tiles over original tiles
	memset(CpsGfx + 0x400000, 0, 0x80000);
	CpsLoadTilesSf2ebbl(CpsGfx + 0x400000, 13);
	
	Cps1LockSpriteList910000 = 1;
	
	CpsLayer1XOffs = -12;
	CpsLayer1YOffs = 1;
	CpsLayer2XOffs = -14;
	CpsLayer2YOffs = 1;
	CpsLayer3XOffs = -16;
	CpsLayer3YOffs = 1;
	CpsDrawSpritesInReverse = 1;
	
	return nRet;
}

static INT32 Sf2jcInit()
{
	Cps1DisableBgHi = 1;

	return DrvInit();
}

static void Sf2qp1Callback()
{
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x100000);
	if (TempRom) {
		memcpy(TempRom, CpsRom, 0x100000);
		memset(CpsRom, 0, 0x100000);
		memcpy(CpsRom + 0x000000, TempRom + 0x000000, 0x40000);
		memcpy(CpsRom + 0x0c0000, TempRom + 0x040000, 0x40000);
		memcpy(CpsRom + 0x080000, TempRom + 0x080000, 0x40000);
		memcpy(CpsRom + 0x040000, TempRom + 0x0c0000, 0x40000);
		BurnFree(TempRom);
	}
}

static INT32 Sf2qp1Init()
{
	AmendProgRomCallback = Sf2qp1Callback;
	
	return DrvInit();
}

static INT32 Sf2thndrInit()
{
	Sf2thndr = 1;
	
	return DrvInit();
}

static INT32 Sf2ceInit()
{
	nCPS68KClockspeed = 7000000;
	
	return DrvInit();
}

static void Sf2accp2Callback()
{
	CpsRom[0x11755] = 0x4e;
	CpsRom[0x11756] = 0x71;
}

static INT32 Sf2accp2Init()
{
	AmendProgRomCallback = Sf2accp2Callback;
	
	return Sf2ceInit();
}

static INT32 Sf2dkot2Init()
{
	INT32 nRet;

	nRet = Sf2ceInit();

	SekOpen(0);
	SekMapMemory(CpsRom + 0x000000, 0x280000, 0x2fffff, SM_ROM);
	SekMapMemory(CpsRom + 0x080000, 0x200000, 0x27ffff, SM_ROM);
	SekClose();

	return nRet;
}

static void Sf2rbCallback()
{
	BurnByteswap(CpsRom, 0x100000);
	CpsRom[0xe5465] = 0x60;
	CpsRom[0xe5464] = 0x12;
}

static INT32 Sf2rbInit()
{
	AmendProgRomCallback = Sf2rbCallback;
	
	return Sf2ceInit();
}

static void Sf2rb2Callback()
{
	CpsRom[0xe5333] = 0x60;
	CpsRom[0xe5332] = 0x14;
}

static INT32 Sf2rb2Init()
{
	AmendProgRomCallback = Sf2rb2Callback;
	
	return Sf2ceInit();
}

static INT32 Sf2thInit()
{
	INT32 nRet = Sf2ceInit();
	
	memset(CpsGfx + 0x400000, 0, 0x80000);
	CpsLoadTiles(CpsGfx + 0x400000, 19);
	
	Cps1LockSpriteList910000 = 1;
	
	CpsLayer1XOffs = -12;
	CpsLayer2XOffs = -14;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	return nRet;
}

static void Sf2yycCallback()
{
	memcpy(CpsRom + 0x140000, CpsRom + 0x100000, 0x40000);
        
	*((UINT16*)(CpsRom + 0xe55be)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55ca)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55cc)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55ce)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55d0)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55d2)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55d4)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55d6)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55d8)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55da)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55de)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55e2)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55e4)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55e6)) = 0x4e71;
	*((UINT16*)(CpsRom + 0xe55e8)) = 0x4e71;
}

static INT32 Sf2yycInit()
{
	nCpsRomLen = 0x40000;
	
	Cps1LockSpriteList910000 = 1;
	AmendProgRomCallback = Sf2yycCallback;
	
	CpsLayer1XOffs = -12;
	CpsLayer2XOffs = -14;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	return Sf2ceInit();
}

static INT32 Sf2koryuInit()
{
	Cps1GfxLoadCallbackFunction = CpsLoadTilesSf2koryu;
	
	Cps1LockSpriteList910000 = 1;
	
	CpsLayer1XOffs = -12;
	CpsLayer2XOffs = -14;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	return Sf2ceInit();
}

static INT32 Sf2koryu2Init()
{
	Cps1LockSpriteList910000 = 1;
	
	CpsLayer1XOffs = -12;
	CpsLayer2XOffs = -14;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	return Sf2ceInit();
}

static INT32 Sf2amfInit()
{
	Cps1GfxLoadCallbackFunction = CpsLoadTilesSf2koryu;
	
	INT32 nRet = Sf2ceInit();
	
	// load bootleg tiles over original tiles
	memset(CpsGfx + 0x400000, 0, 0x80000);
	CpsLoadTilesSf2ebbl(CpsGfx + 0x400000, 13);
	
	Cps1LockSpriteList910000 = 1;
	
	CpsLayer1XOffs = -12;
	CpsLayer2XOffs = -14;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	return nRet;
}

UINT8 __fastcall Sf2mdtReadByte(UINT32 a)
{
	switch (a) {
		case 0x70c000: {
			return ~Inp000;
		}
		
		case 0x70c001: {
			return ~Inp001;
		}
		
		case 0x70c008: {
			return 0xff;
		}
		
		case 0x70c009: {
			return ~Inp177;
		}
		
		case 0x70c018: {
			return ~Inp018;
		}
		
		case 0x70c01a: {
			return ~Cpi01A;
		}
		
		case 0x70c01c: {
			return ~Cpi01C;
		}
		
		case 0x70c01e: {
			return ~Cpi01E;
		}		
		
		default: {
			bprintf(PRINT_NORMAL, _T("Read Byte %x\n"), a);
		}
	}

	return 0;
}

UINT16 __fastcall Sf2mdtReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Read Word %x\n"), a);
		}
	}

	return 0;
}

void __fastcall Sf2mdtWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x70c106: {
			Sf2mdtSoundCommand(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write Byte %x, %x\n"), a, d);
		}
	}
}

void __fastcall Sf2mdtWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x70810c: {
			// scroll3 x
			*((UINT16*)(CpsReg + 0x14)) = BURN_ENDIAN_SWAP_INT16(d + 0xffbe);
			return;
		}
		
		case 0x70810e: {
			// scroll3 y
			*((UINT16*)(CpsReg + 0x16)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x708110: {
			// scroll2 x
			*((UINT16*)(CpsReg + 0x10)) = BURN_ENDIAN_SWAP_INT16(d + 0xffc0);
			return;
		}
		
		case 0x708112: {
			// scroll1 x
			*((UINT16*)(CpsReg + 0x0c)) = BURN_ENDIAN_SWAP_INT16(d + 0xffbe);
			return;
		}
		
		case 0x708114: {
			// scroll2 y
			*((UINT16*)(CpsReg + 0x12)) = BURN_ENDIAN_SWAP_INT16(d);
			// update the row scroll start reg here as well
			*((UINT16*)(CpsReg + 0x20)) = BURN_ENDIAN_SWAP_INT16(d);
			// get the row scroll table address
			*((UINT16*)(CpsReg + 0x08)) = *((UINT16*)(CpsRamFF + 0x802e));
			return;
		}
		
		case 0x708116: {
			// scroll1 y
			*((UINT16*)(CpsReg + 0x0e)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x70814c: {
			*((UINT16*)(CpsReg + nCpsLcReg)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x70d000: {
			// nop?
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write Word %x, %x\n"), a, d);
		}
	}
}

void __fastcall Sf2mdtaWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x70810c: {
			// scroll1 x
			*((UINT16*)(CpsReg + 0x0c)) = BURN_ENDIAN_SWAP_INT16(d + 0xffbe);
			return;
		}
		
		case 0x70810e: {
			// scroll1 y
			*((UINT16*)(CpsReg + 0x0e)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x708110: {
			// scroll3 x
			*((UINT16*)(CpsReg + 0x14)) = BURN_ENDIAN_SWAP_INT16(d + 0xffbe);
			return;
		}
		
		case 0x708112: {
			// scroll2 y
			*((UINT16*)(CpsReg + 0x12)) = BURN_ENDIAN_SWAP_INT16(d);
			// update the row scroll start reg here as well
			*((UINT16*)(CpsReg + 0x20)) = BURN_ENDIAN_SWAP_INT16(d);
			// get the row scroll table address
			*((UINT16*)(CpsReg + 0x08)) = *((UINT16*)(CpsRamFF + 0x802e));
			return;
		}
		
		case 0x708114: {
			// ???
			return;
		}
		
		case 0x708116: {
			// scroll3 y
			*((UINT16*)(CpsReg + 0x16)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x70814c: {
			*((UINT16*)(CpsReg + nCpsLcReg)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x70d000: {
			// nop?
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write Word %x, %x\n"), a, d);
		}
	}
}

static INT32 Sf2mdtScanCallback(INT32 nAction, INT32*pnMin)
{
	CpsBootlegSpriteRamScanCallback(nAction, pnMin);
	Sf2mdtScanSound(nAction, pnMin);
	
	return 0;
}

static INT32 Sf2mdtInit()
{
	bCpsUpdatePalEveryFrame = 1;	
	Cps1DisablePSnd = 1;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesSf2mdt;
	Cps1ObjGetCallbackFunction = Sf2mdtObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsRunInitCallbackFunction = Sf2mdtSoundInit;
	CpsRunResetCallbackFunction = Sf2mdtSoundReset;
	CpsRunExitCallbackFunction = Sf2mdtSoundExit;
	CpsRunFrameStartCallbackFunction = Sf2mdtSoundFrameStart;
	CpsRunFrameEndCallbackFunction = Sf2mdtSoundFrameEnd;
	CpsRWSoundCommandCallbackFunction = Sf2mdtSoundCommand;
	CpsMemScanCallbackFunction = Sf2mdtScanCallback;
	
	INT32 nRet = Sf2ceInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x700000, 0x703fff, SM_RAM);
	SekMapMemory(CpsBootlegSpriteRam, 0x704000, 0x707fff, SM_RAM); // mirror? can use either of this - seems to make no difference
	SekMapHandler(1, 0x708000, 0x7fffff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(1, Sf2mdtReadByte);
	SekSetReadWordHandler(1, Sf2mdtReadWord);
	SekSetWriteByteHandler(1, Sf2mdtWriteByte);
	SekSetWriteWordHandler(1, Sf2mdtWriteWord);
	SekClose();
	
	Cps1VBlankIRQLine = 4; // triggers the sprite ram and layer enable/scroll writes at 0x700000
	
	return nRet;
}

static INT32 Sf2mdtaInit()
{
	bCpsUpdatePalEveryFrame = 1;	
	Cps1DisablePSnd = 1;
	CpsLayer2XOffs = 0xffc0; // layer 2 scrolling seems to be taken care of by row scroll tables
	Cps1GfxLoadCallbackFunction = CpsLoadTilesSf2mdta;
	Cps1ObjGetCallbackFunction = Sf2mdtObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsRunInitCallbackFunction = Sf2mdtSoundInit;
	CpsRunResetCallbackFunction = Sf2mdtSoundReset;
	CpsRunExitCallbackFunction = Sf2mdtSoundExit;
	CpsRunFrameStartCallbackFunction = Sf2mdtSoundFrameStart;
	CpsRunFrameEndCallbackFunction = Sf2mdtSoundFrameEnd;
	CpsRWSoundCommandCallbackFunction = Sf2mdtSoundCommand;
	CpsMemScanCallbackFunction = Sf2mdtScanCallback;
	
	INT32 nRet = Sf2ceInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x700000, 0x703fff, SM_RAM);
	SekMapMemory(CpsBootlegSpriteRam, 0x704000, 0x707fff, SM_RAM); // mirror? can use either of this - seems to make no difference
	SekMapMemory(CpsRamFF, 0xfc0000, 0xfcffff, SM_RAM);	
	SekMapHandler(1, 0x708000, 0x7fffff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(1, Sf2mdtReadByte);
	SekSetReadWordHandler(1, Sf2mdtReadWord);
	SekSetWriteByteHandler(1, Sf2mdtWriteByte);
	SekSetWriteWordHandler(1, Sf2mdtaWriteWord);
	SekClose();
	
	Cps1VBlankIRQLine = 4; // triggers the sprite ram and layer enable/scroll writes at 0x700000
	
	return nRet;
}

void __fastcall Sf2hfjbWriteByte(UINT32 a, UINT8 d)
{
	CpsWritePort(a & 0x1ff, d);
}

void __fastcall Sf2hfjbWriteWord(UINT32 a, UINT16 d)
{
	if (a == 0x800124) {
		// row scroll start register moved in this set
		*((UINT16*)(CpsReg + 0x20)) = d;
		return;
	}
	
	SEK_DEF_WRITE_WORD(1, a, d);
}

static INT32 Sf2hfjbInit()
{
	CpsLayer1XOffs = -12;
	CpsLayer2XOffs = -14;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	INT32 nRet = Sf2ceInit();
	
	SekOpen(0);
	SekMapHandler(1, 0x800000, 0x807fff, SM_WRITE);
	SekSetWriteByteHandler(1, Sf2hfjbWriteByte);
	SekSetWriteWordHandler(1, Sf2hfjbWriteWord);
	SekClose();
	
	return nRet;
}

static INT32 Sf2m4Init()
{
	Cps1LockSpriteList910000 = 1;
	
	CpsLayer1XOffs = -12;
	CpsLayer2XOffs = -14;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	return Sf2ceInit();
}

void __fastcall Sf2m1ScrollWrite(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x980000: {
			// scroll1 y
			*((UINT16*)(CpsReg + 0x0e)) = d;
			return;
		}
		
		case 0x980002: {
			// scroll1 x
			*((UINT16*)(CpsReg + 0x0c)) = d - 0x40;
			return;
		}
		
		case 0x980004: {
			// scroll2 y
			*((UINT16*)(CpsReg + 0x12)) = d;
			return;
		}
		
		case 0x980006: {
			// scroll2 x
			*((UINT16*)(CpsReg + 0x10)) = d - 0x3c;
			return;
		}
		
		case 0x980008: {
			// scroll3 y
			*((UINT16*)(CpsReg + 0x16)) = d;
			return;
		}
		
		case 0x98000a: {
			// scroll3 x
			*((UINT16*)(CpsReg + 0x14)) = d - 0x40;
			return;
		}
		
		case 0x98000c: {
			// This seems to control layer order and enable
			switch (d) {
				case 0x00: {
					nCps1Layers[0] = 0;
					nCps1Layers[1] = 1;
					nCps1Layers[2] = 3;
					nCps1Layers[3] = 2;
					break;
				}
				
				case 0x01: {
					nCps1Layers[0] = 0;
					nCps1Layers[1] = 3;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 1;
					break;
				}
				
				case 0x02: {
					nCps1Layers[0] = 0;
					nCps1Layers[1] = 1;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 3;
					break;
				}
				
				case 0x03: {
					nCps1Layers[0] = 0;
					nCps1Layers[1] = 2;
					nCps1Layers[2] = 1;
					nCps1Layers[3] = 3;
					break;
				}
				
				case 0x04: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 3;
					break;
				}
				
				case 0x05: {
					nCps1Layers[0] = 0;
					nCps1Layers[1] = 2;
					nCps1Layers[2] = 3;
					nCps1Layers[3] = 1;
					break;
				}
				
				default: {
					nCps1Layers[0] = 0;
					nCps1Layers[1] = 3;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 1;
					bprintf(PRINT_IMPORTANT, _T("Unknown value written at 0x98000c %x\n"), d);
				}
			}
			return;
		}
		
		case 0x980016: {
			// scroll3 ram offset
			*((UINT16*)(CpsReg + 0x06)) = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write Word %x, %x\n"), a, d);
		}
	}
}

static INT32 Sf2m1Init()
{
	Port6SoundWrite = 1;
	bCpsUpdatePalEveryFrame = 1;
	Cps1OverrideLayers = 1;
	Cps1ObjGetCallbackFunction = DinopicObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	
	INT32 nRet = Sf2ceInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x993fff, SM_RAM);
	SekMapHandler(1, 0x980000, 0x980fff, SM_WRITE);
	SekSetWriteWordHandler(1, Sf2m1ScrollWrite);
	SekClose();
	
	return nRet;
}

static void Sf2m2Callback()
{
	*((UINT16*)(CpsRom + 0xC0680)) = 0x4E71;
	*((UINT16*)(CpsRom + 0xC0682)) = 0x4E71;
	*((UINT16*)(CpsRom + 0xC0684)) = 0x4E71;
}

static INT32 Sf2m2Init()
{
	Cps1LockSpriteList910000 = 1;
	AmendProgRomCallback = Sf2m2Callback;
	
	CpsLayer1XOffs = -12;
	CpsLayer2XOffs = -14;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	return Sf2ceInit();
}

UINT8 __fastcall Sf2m3ReadByte(UINT32 a)
{
	UINT8 d = 0xff;
	
	switch (a) {
		case 0x800010: {
			d = (UINT8)~Inp010;
			return d;
		}
		
		case 0x800011: {
			d = (UINT8)~Inp011;
			return d;
		}
		
		case 0x800029: {
			d = (UINT8)~Inp029;
			return d;
		}
		
		case 0x800186: {
			d = (UINT8)~Inp186;
			return d;
		}
		
		case 0x80002a: {
			d = (UINT8)~Cpi01A;
			return d;
		}
		
		case 0x80002c: {
			d = (UINT8)~Cpi01C;
			return d;
		}
		
		case 0x80002e: {
			d = (UINT8)~Cpi01E;
			return d;
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Read byte %x\n"), a);
	
	return 0;
}

void __fastcall Sf2m3WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x800191: {
			PsndSyncZ80((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles);

			PsndCode = d;
			return;
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Write byte %x, %x\n"), a, d);
}

void __fastcall Sf2m3WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x800100: {
			CpsReg[0x00] = d & 0xff;
			CpsReg[0x01] = d >> 8;
			return;
		}
		
		case 0x800102: {
			CpsReg[0x02] = d & 0xff;
			CpsReg[0x03] = d >> 8;
			return;
		}
		
		case 0x800104: {
			CpsReg[0x04] = d & 0xff;
			CpsReg[0x05] = d >> 8;
			return;
		}
		
		case 0x800106: {
			CpsReg[0x06] = d & 0xff;
			CpsReg[0x07] = d >> 8;
			return;
		}
		
		case 0x80010a: {
			CpsReg[0x0a] = d & 0xff;
			CpsReg[0x0b] = d >> 8;
			
			GetPalette(0, 6);
			CpsPalUpdate(CpsSavePal);
			return;
		}
		
		case 0x800122: {
			CpsReg[0x22] = d & 0xff;
			CpsReg[0x23] = d >> 8;
			return;
		}
		
		case 0x80014a: {
			CpsReg[0x4a] = d & 0xff;
			CpsReg[0x4b] = d >> 8;
			return;
		}
		
		case 0x80014c: {
			CpsReg[0x4c] = d & 0xff;
			CpsReg[0x4d] = d >> 8;
			return;
		}
		
		case 0x80014e: {
			CpsReg[0x4e] = d & 0xff;
			CpsReg[0x4f] = d >> 8;
			return;
		}
		
		case 0x800150: {
			CpsReg[0x50] = d & 0xff;
			CpsReg[0x51] = d >> 8;
			return;
		}
		
		case 0x800152: {
			CpsReg[0x52] = d & 0xff;
			CpsReg[0x53] = d >> 8;
			return;
		}
		
		case 0x8001a8: {
			CpsReg[0x08] = d & 0xff;
			CpsReg[0x09] = d >> 8;
			return;
		}
		
		case 0x8001ac: {
			CpsReg[0x0c] = d & 0xff;
			CpsReg[0x0d] = d >> 8;
			return;
		}
		
		case 0x8001ae: {
			CpsReg[0x0e] = d & 0xff;
			CpsReg[0x0f] = d >> 8;
			return;
		}
		
		case 0x8001b0: {
			CpsReg[0x10] = d & 0xff;
			CpsReg[0x11] = d >> 8;
			return;
		}
		
		case 0x8001b2: {
			CpsReg[0x12] = d & 0xff;
			CpsReg[0x13] = d >> 8;
			return;
		}
		
		case 0x8001b4: {
			CpsReg[0x14] = d & 0xff;
			CpsReg[0x15] = d >> 8;
			return;
		}
		
		case 0x8001b6: {
			CpsReg[0x16] = d & 0xff;
			CpsReg[0x17] = d >> 8;
			return;
		}
		
		case 0x8001c0: {
			CpsReg[0x20] = d & 0xff;
			CpsReg[0x21] = d >> 8;
			return;
		}	
		
		case 0x8001c4: {
			CpsReg[0xc4] = d & 0xff;
			CpsReg[0xc5] = d >> 8;
			return;
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Write word %x, %x\n"), a, d);
}

static INT32 Sf2m3Init()
{
	INT32 nRet = 0;
	
	CpsLayer1XOffs = -16;
	CpsLayer2XOffs = -16;
	CpsLayer3XOffs = -16;
	CpsDrawSpritesInReverse = 1;
	
	nRet = Sf2ceInit();
	
	SekOpen(0);
	SekMapHandler(1, 0x800000, 0x800200, SM_RAM);
	SekSetReadByteHandler(1, Sf2m3ReadByte);
	SekSetWriteByteHandler(1, Sf2m3WriteByte);
	SekSetWriteWordHandler(1, Sf2m3WriteWord);
	
	SekClose();
	
	return nRet;
}

static INT32 Sf2m8Init()
{
	Scroll1TileMask = 0x4fff;
	Scroll3TileMask = 0x1fff;
	
	return Sf2m3Init();
}

static INT32 SfzchInit()
{
	INT32 nRet = 0;
	
	nRet = TwelveMhzInit();
	
	Cps = 3;
	
	return nRet;
}

static INT32 StriderInit()
{
	INT32 nRet = 0;
	
	nRet = DrvInit();
	
	CpsStar = CpsGfx + nCpsGfxLen - 0x2000;
	
	CpsLoadStars(CpsStar, 5);
	
	return nRet;
}

static INT32 StriderjrInit()
{
	INT32 nRet = 0;
	
	nRet = TwelveMhzInit();
	
	CpsStar = CpsGfx + nCpsGfxLen - 0x2000;
	
	CpsLoadStars(CpsStar, 2);
	
	return nRet;
}

static INT32 StriderjInit()
{
	INT32 nRet = 0;
	
	nRet = DrvInit();
	
	CpsStar = CpsGfx + nCpsGfxLen - 0x2000;
	
	CpsLoadStarsByte(CpsStar, 8);
	
	return nRet;
}

static INT32 WofchInit()
{
	INT32 nRet = 0;

	Cps1Qs = 1;
	
	nRet = TwelveMhzInit();
	
	Cps = 3;
	
	return nRet;
}

UINT8 __fastcall Wofh135ReadByte(UINT32)
{
	return 0xff;
}

UINT16 __fastcall Wofh135ReadWord(UINT32)
{
	return 0xffff;
}

UINT8 __fastcall WofhInputReadByte(UINT32 a)
{
	switch (a) {
		case 0x880000: {
			return ~Inp000;
		}
		
		case 0x880001: {
			return ~Inp001;
		}
		
		case 0x880006: {
			return ~Inp018;
		}
		
		case 0x880007: {
			return ~Inp177;
		}
		
		case 0x880008: {
			return ~Cpi01A;
		}
		
		case 0x88000a: {
			return ~Cpi01C;
		}
		
		case 0x88000c: {
			return ~Cpi01E;
		}
		
		case 0x880e78: {
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Read Byte %x\n"), a);
		}
	}
	
	return 0x00;
}

UINT16 __fastcall WofhInputReadWord(UINT32 a)
{
	SEK_DEF_READ_WORD(3, a);
}

void __fastcall WofhInputWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x88000e: {
			PsndSyncZ80((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles);

			PsndCode = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Write Byte %x, %x\n"), a, d);
		}
	}
}

void __fastcall WofhInputWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x88000e: {
			PsndSyncZ80((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles);

			PsndCode = d & 0xff;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Write word %x, %x\n"), a, d);
		}
	}
}

void __fastcall WofbFFWriteByte(UINT32 a, UINT8 d)
{
	CpsRamFF[((a & 0xffff) ^ 1)] = d;
}

void __fastcall WofbFFWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xff639a: {
			*((UINT16*)(CpsReg + MaskAddr[1])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff639c: {
			*((UINT16*)(CpsReg + MaskAddr[2])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
		
		case 0xff639e: {
			*((UINT16*)(CpsReg + MaskAddr[3])) = BURN_ENDIAN_SWAP_INT16(d);
			break;
		}
	}
	
	UINT16 *RAM = (UINT16*)CpsRamFF;
	RAM[((a & 0xffff) >> 1)] = d;
}

static void WofhCallback()
{
	// Patch protection? check
	CpsRom[0xf11ed] = 0x4e;
	CpsRom[0xf11ec] = 0x71;
//	CpsRom[0xf11ef] = 0x4e;
//	CpsRom[0xf11ee] = 0x71;
}

static INT32 WofhInit()
{
	INT32 nRet = 0;
	
	bCpsUpdatePalEveryFrame = 1;
	CpsLayer1XOffs = 0xffc0;
	CpsLayer2XOffs = 0xffc0;
	CpsLayer3XOffs = 0xffc0;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesHack160;
	AmendProgRomCallback = WofhCallback;
	Cps1ObjGetCallbackFunction = WofhObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	
	nRet = TwelveMhzInit();
	
	SekOpen(0);
	SekMapHandler(3, 0x880000, 0x89ffff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(3, WofhInputReadByte);
	SekSetReadWordHandler(3, WofhInputReadWord);
	SekSetWriteByteHandler(3, WofhInputWriteByte);
	SekSetWriteWordHandler(3, WofhInputWriteWord);
	SekMapHandler(4, 0x135000, 0x135fff, SM_READ);
	SekSetReadByteHandler(4, Wofh135ReadByte);
	SekSetReadWordHandler(4, Wofh135ReadWord);
	SekMapHandler(5, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(5, WofbFFWriteByte);
	SekSetWriteWordHandler(5, WofbFFWriteWord);
	SekClose();
	
	return nRet;
}

static void SgyxzCallback()
{
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x40000);
	
	if (pTemp) {
		memcpy(pTemp           , CpsRom + 0x40000, 0x40000);
		memcpy(CpsRom + 0x40000, CpsRom + 0x80000, 0x40000);
		memcpy(CpsRom + 0x80000, pTemp           , 0x40000);
	}
	
	BurnFree(pTemp);
	
	WofhCallback();
	
	// This set has more protection?
	*((UINT16*)(CpsRom + 0x708be)) = BURN_ENDIAN_SWAP_INT16(0x4e71);
	*((UINT16*)(CpsRom + 0x708c0)) = BURN_ENDIAN_SWAP_INT16(0x4e71);
}

static INT32 SgyxzInit()
{
	INT32 nRet = 0;
	
	bCpsUpdatePalEveryFrame = 1;
	CpsLayer1XOffs = 0xffc0;
	CpsLayer2XOffs = 0xffc0;
	CpsLayer3XOffs = 0xffc0;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesHack160;
	AmendProgRomCallback = SgyxzCallback;
	Cps1ObjGetCallbackFunction = WofhObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	
	nRet = TwelveMhzInit();
	
	SekOpen(0);
	SekMapHandler(3, 0x880000, 0x89ffff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(3, WofhInputReadByte);
	SekSetReadWordHandler(3, WofhInputReadWord);
	SekSetWriteByteHandler(3, WofhInputWriteByte);
	SekSetWriteWordHandler(3, WofhInputWriteWord);
	SekMapHandler(4, 0x135000, 0x135fff, SM_READ);
	SekSetReadByteHandler(4, Wofh135ReadByte);
	SekSetReadWordHandler(4, Wofh135ReadWord);
	SekMapHandler(5, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(5, WofbFFWriteByte);
	SekSetWriteWordHandler(5, WofbFFWriteWord);
	SekClose();
	
	return nRet;
}

static void Wof3jsCallback()
{
	memcpy(CpsRom + 0x0c0000, CpsRom + 0x100000, 0x40000);
	memset(CpsRom + 0x100000, 0, 0x40000);
}

static INT32 Wof3jsInit()
{
	AmendProgRomCallback = Wof3jsCallback;
	
	return TwelveMhzInit();
}

static void Wof3jsaCallback()
{
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x40000);
	
	if (pTemp) {
		memcpy(pTemp           , CpsRom + 0x40000, 0x40000);
		memcpy(CpsRom + 0x40000, CpsRom + 0x80000, 0x40000);
		memcpy(CpsRom + 0x80000, pTemp           , 0x40000);
	}
	
	BurnFree(pTemp);
	
	// Patch protection?
	*((UINT16*)(CpsRom + 0xe7ad0)) = BURN_ENDIAN_SWAP_INT16(0x4e71);
}

static INT32 Wof3jsaInit()
{
	AmendProgRomCallback = Wof3jsaCallback;
	
	bCpsUpdatePalEveryFrame = 1;
	CpsLayer1XOffs = 0xffc0;
	CpsLayer2XOffs = 0xffc0;
	CpsLayer3XOffs = 0xffc0;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesHack160;
	Cps1ObjGetCallbackFunction = WofhObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	
	INT32 nRet = TwelveMhzInit();
	
	SekOpen(0);
	SekMapHandler(3, 0x880000, 0x89ffff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(3, WofhInputReadByte);
	SekSetReadWordHandler(3, WofhInputReadWord);
	SekSetWriteByteHandler(3, WofhInputWriteByte);
	SekSetWriteWordHandler(3, WofhInputWriteWord);
	SekMapHandler(4, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(4, WofbFFWriteByte);
	SekSetWriteWordHandler(4, WofbFFWriteWord);
	SekClose();
	
	return nRet;
}

UINT8 __fastcall Wof3sjInputReadByte(UINT32 a)
{
	switch (a) {
		case 0x880000: {
			return ~Inp000;
		}
		
		case 0x880001: {
			return ~Inp001;
		}
		
		case 0x880008: {
			return ~Inp018;
		}
		
		case 0x880009: {
			return ~Inp177;
		}
		
		case 0x88000a: {
			return ~Cpi01A;
		}
		
		case 0x88000c: {
			return ~Cpi01C;
		}
		
		case 0x88000e: {
			return ~Cpi01E;
		}
		
		case 0x880c1e: {
			return 0xff;
		}
		
		case 0x880e7e: {
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Read Byte %x\n"), a);
		}
	}
	
	return 0;
}

UINT16 __fastcall Wof3sjInputReadWord(UINT32 a)
{
	SEK_DEF_READ_WORD(3, a);
}

void __fastcall Wof3sjInputWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x880006: {
			PsndSyncZ80((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles);

			PsndCode = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Write Byte %x, %x\n"), a, d);
		}
	}
}

void __fastcall Wof3sjInputWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x880006: {
			PsndSyncZ80((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles);

			PsndCode = d & 0xff;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Input Write word %x, %x\n"), a, d);
		}
	}
}

static INT32 Wof3sjInit()
{
	bCpsUpdatePalEveryFrame = 1;
	CpsLayer1XOffs = 0xffc0;
	CpsLayer2XOffs = 0xffc0;
	CpsLayer3XOffs = 0xffc0;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesHack160;
	Cps1ObjGetCallbackFunction = WofhObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	
	INT32 nRet = TwelveMhzInit();
	
	SekOpen(0);
	SekMapHandler(3, 0x880000, 0x89ffff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(3, Wof3sjInputReadByte);
	SekSetReadWordHandler(3, Wof3sjInputReadWord);
	SekSetWriteByteHandler(3, Wof3sjInputWriteByte);
	SekSetWriteWordHandler(3, Wof3sjInputWriteWord);
	SekMapHandler(4, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(4, WofbFFWriteByte);
	SekSetWriteWordHandler(4, WofbFFWriteWord);
	SekClose();
	
	return nRet;
}

static INT32 WofsjInit()
{
	bCpsUpdatePalEveryFrame = 1;
	CpsLayer1XOffs = 0xffc0;
	CpsLayer2XOffs = 0xffc0;
	CpsLayer3XOffs = 0xffc0;
	
	Cps1ObjGetCallbackFunction = WofhObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	
	INT32 nRet = TwelveMhzInit();
	
	SekOpen(0);
	SekMapHandler(3, 0x880000, 0x89ffff, SM_READ | SM_WRITE);
	SekSetReadByteHandler(3, Wof3sjInputReadByte);
	SekSetReadWordHandler(3, Wof3sjInputReadWord);
	SekSetWriteByteHandler(3, Wof3sjInputWriteByte);
	SekSetWriteWordHandler(3, Wof3sjInputWriteWord);
	SekMapHandler(4, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(4, WofbFFWriteByte);
	SekSetWriteWordHandler(4, WofbFFWriteWord);
	SekClose();
	
	return nRet;
}

static INT32 WofsjbInit()
{
	CpsLayer1XOffs = 2;
	CpsLayer2XOffs = 4;
	CpsLayer3XOffs = 8;
	
	Cps1DisablePSnd = 1;
	
	return TwelveMhzInit();
}

void __fastcall Wofb98WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x980000: {
			// scroll1 y
			*((UINT16*)(CpsReg + 0x0e)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980002: {
			// scroll1 x
			*((UINT16*)(CpsReg + 0x0c)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980004: {
			// scroll2 y
			*((UINT16*)(CpsReg + 0x12)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980006: {
			// scroll2 x
			*((UINT16*)(CpsReg + 0x10)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x980008: {
			// scroll3 y
			*((UINT16*)(CpsReg + 0x16)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x98000a: {
			// scroll3 x
			*((UINT16*)(CpsReg + 0x14)) = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x98000c: {
			switch (d) {
				case 0x00:
				case 0x04:
				case 0x06:
				case 0x08:
				case 0x0a:
				case 0x0b:
				case 0x0e: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 3;
					break;
				}
				
				case 0x01:
				case 0x05:
				case 0x0f: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 3;
					nCps1Layers[3] = 2;
					break;
				}
				
				case 0x03:
				case 0x07:
				case 0x09: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 3;
					nCps1Layers[2] = 0;
					nCps1Layers[3] = 2;
					break;
				}
				
				default: {
					nCps1Layers[0] = 1;
					nCps1Layers[1] = 0;
					nCps1Layers[2] = 2;
					nCps1Layers[3] = 3;
					bprintf(PRINT_IMPORTANT, _T("Unknown value written at 0x98000c %x\n"), d);
				}
			}
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write word %x, %x\n"), a, d);
		}
	}
}

static INT32 WofbInit()
{
	bCpsUpdatePalEveryFrame = 1;
	Cps1OverrideLayers = 1;
	Port6SoundWrite = 1;
	CpsBootlegEEPROM = 1;
	CpsLayer1XOffs = 0xffc0;
	CpsLayer2XOffs = 0xffc0;
	CpsLayer3XOffs = 0xffc0;
	Cps1GfxLoadCallbackFunction = CpsLoadTilesDinopic;
	Cps1ObjGetCallbackFunction = DinopicObjGet;
	Cps1ObjDrawCallbackFunction = FcrashObjDraw;
	CpsMemScanCallbackFunction = CpsBootlegSpriteRamScanCallback;
	
	INT32 nRet = TwelveMhzInit();
	
	CpsBootlegSpriteRam = (UINT8*)BurnMalloc(0x4000);
	
	SekOpen(0);
	SekMapMemory(CpsBootlegSpriteRam, 0x990000, 0x993fff, SM_RAM);
	SekMapHandler(1, 0x980000, 0x98ffff, SM_WRITE);
	SekSetWriteWordHandler(1, Wofb98WriteWord);
	SekMapHandler(2, 0xff0000, 0xffffff, SM_WRITE);
	SekSetWriteByteHandler(2, WofbFFWriteByte);
	SekSetWriteWordHandler(2, WofbFFWriteWord);
	SekClose();
	
	// scroll3 ram offset
	*((UINT16*)(CpsReg + 0x06)) = BURN_ENDIAN_SWAP_INT16(0x9100);
	
	return nRet;	
}

// Driver Definitions

struct BurnDriver BurnDrvCps1941 = {
	"1941", NULL, NULL, NULL, "1990",
	"1941 - Counter Attack (900227 World)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, FBF_19XX,
	NULL, NTFODrvRomInfo, NTFODrvRomName, NULL, NULL, NTFOInputInfo, NTFODIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCps1941r1 = {
	"1941r1", "1941", NULL, NULL, "1990",
	"1941 - Counter Attack (World)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, FBF_19XX,
	NULL, NTFOR1DrvRomInfo, NTFOR1DrvRomName, NULL, NULL, NTFOInputInfo, NTFODIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCps1941u = {
	"1941u", "1941", NULL, NULL, "1990",
	"1941 - Counter Attack (900227 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, FBF_19XX,
	NULL, NTFOUDrvRomInfo, NTFOUDrvRomName, NULL, NULL, NTFOInputInfo, NTFODIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCps1941j = {
	"1941j", "1941", NULL, NULL, "1990",
	"1941 - Counter Attack (Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, FBF_19XX,
	NULL, NTFOJDrvRomInfo, NTFOJDrvRomName, NULL, NULL, NTFOInputInfo, NTFODIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCps3wonders = {
	"3wonders", NULL, NULL, NULL, "1991",
	"Three Wonders (wonder 3 910520 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_MINIGAMES, 0,
	NULL, ThreeWondersRomInfo, ThreeWondersRomName, NULL, NULL, ThreeWondersInputInfo, ThreeWondersDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCps3wondersu = {
	"3wondersu", "3wonders", NULL, NULL, "1991",
	"Three Wonders (wonder 3 910520 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_MINIGAMES, 0,
	NULL, Wonder3uRomInfo, Wonder3uRomName, NULL, NULL, ThreeWondersInputInfo, ThreeWondersDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWonder3 = {
	"wonder3", "3wonders", NULL, NULL, "1991",
	"Wonder 3 (910520 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_MINIGAMES, 0,
	NULL, Wonder3RomInfo, Wonder3RomName, NULL, NULL, ThreeWondersInputInfo, ThreeWondersDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCps3wondersh = {
	"3wondersh", "3wonders", NULL, NULL, "1991",
	"Three Wonders (bootleg, wonder 3 910520 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_MINIGAMES, 0,
	NULL, Wonder3hRomInfo, Wonder3hRomName, NULL, NULL, ThreeWondersInputInfo, ThreeWondersDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCaptcomm = {
	"captcomm", NULL, NULL, NULL, "1991",
	"Captain Commando (911202 other country)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, CaptcommRomInfo, CaptcommRomName, NULL, NULL, CaptcommInputInfo, CaptcommDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCaptcommr1 = {
	"captcommr1", "captcomm", NULL, NULL, "1991",
	"Captain Commando (911014 other country)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Captcommr1RomInfo, Captcommr1RomName, NULL, NULL, CaptcommInputInfo, CaptcommDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCaptcommu = {
	"captcommu", "captcomm", NULL, NULL, "1991",
	"Captain Commando (910928 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, CaptcommuRomInfo, CaptcommuRomName, NULL, NULL, CaptcommInputInfo, CaptcommDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCaptcommj = {
	"captcommj", "captcomm", NULL, NULL, "1991",
	"Captain Commando (911202 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, CaptcommjRomInfo, CaptcommjRomName, NULL, NULL, CaptcommInputInfo, CaptcommDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCaptcommjr1 = {
	"captcommjr1", "captcomm", NULL, NULL, "1991",
	"Captain Commando (910928 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Captcommjr1RomInfo, Captcommjr1RomName, NULL, NULL, CaptcommInputInfo, CaptcommDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCaptcommb = {
	"captcommb", "captcomm", NULL, NULL, "1991",
	"Captain Commando (bootleg set 1, 911014 other country)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, CaptcommbRomInfo, CaptcommbRomName, NULL, NULL, CaptcommInputInfo, CaptcommDIPInfo,
	CaptcommbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCaptcommb2 = {
	"captcommb2", "captcomm", NULL, NULL, "1991",
	"Captain Commando (bootleg set 2 (with YM2151 + 2xMSM5205), 911014 other country)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Captcommb2RomInfo, Captcommb2RomName, NULL, NULL, CaptcommInputInfo, CaptcommDIPInfo,
	Captcommb2Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCawing = {
	"cawing", NULL, NULL, NULL, "1990",
	"Carrier Air Wing (U.S. navy 901012 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_HORSHOOT, 0,
	NULL, CawingRomInfo, CawingRomName, NULL, NULL, CawingInputInfo, CawingDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCawingr1 = {
	"cawingr1", "cawing", NULL, NULL, "1990",
	"Carrier Air Wing (U.S. navy 901009 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_HORSHOOT, 0,
	NULL, Cawingr1RomInfo, Cawingr1RomName, NULL, NULL, CawingInputInfo, CawingDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCawingu = {
	"cawingu", "cawing", NULL, NULL, "1990",
	"Carrier Air Wing (U.S. navy 901012 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_HORSHOOT, 0,
	NULL, CawinguRomInfo, CawinguRomName, NULL, NULL, CawingInputInfo, CawingDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCawingj = {
	"cawingj", "cawing", NULL, NULL, "1990",
	"U.S. Navy (901012 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_HORSHOOT, 0,
	NULL, CawingjRomInfo, CawingjRomName, NULL, NULL, CawingInputInfo, CawingDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCawingbl = {
	"cawingbl", "cawing", NULL, NULL, "1990",
	"Carrier Air Wing (bootleg (with 2xYM2203 + 2xMSM5205), U.S. navy 901012 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_HORSHOOT, 0,
	NULL, CawingblRomInfo, CawingblRomName, NULL, NULL, CawingInputInfo, CawingDIPInfo,
	CawingblInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCps1demo = {
	"cps1demo", NULL, NULL, NULL, "2000",
	"Chaos Demo (CPS-1)\0", NULL, "Chaos", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_CAPCOM_CPS1, GBF_MISC, 0,
	NULL, Cps1demoRomInfo, Cps1demoRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	Cps1demoInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCworld2j = {
	"cworld2j", NULL, NULL, NULL, "1992",
	"Capcom World 2 (920611 Japan)\0", NULL, "Capcom", "CPS1",
	L"Capcom World 2 (\u30AF\u30A4\u30BA\uFF15 \u3042\u3069\u3079\u3093\u3061\u3083\u30FC\u304F\u3044\u305A \u304B\u3077\u3053\u3093\u308F\u30FC\u308B\u3069\uFF12 920611 Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_PUZZLE, 0,
	NULL, Cworld2jRomInfo, Cworld2jRomName, NULL, NULL, Cworld2jInputInfo, Cworld2jDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDino = {
	"dino", NULL, NULL, NULL, "1993",
	"Cadillacs & Dinosaurs (930201 etc)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, DinoRomInfo, DinoRomName, NULL, NULL, DinoInputInfo, DinoDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinoj = {
	"dinoj", "dino", NULL, NULL, "1993",
	"Cadillacs Kyouryuu-Shinseiki (Cadillacs 930201 Japan)\0", NULL, "Capcom", "CPS1 / QSound",
	L"Cadillacs \u6050\u7ADC\u65B0\u4E16\u7D00\0Cadillacs Kyouryuu-Shinseiki (Cadillacs 930201 Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, DinojRomInfo, DinojRomName, NULL, NULL, DinoInputInfo, DinoDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinou = {
	"dinou", "dino", NULL, NULL, "1993",
	"Cadillacs & Dinosaurs (930201 USA)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, DinouRomInfo, DinouRomName, NULL, NULL, DinoInputInfo, DinoDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinopic = {
	"dinopic", "dino", NULL, NULL, "1993",
	"Cadillacs and Dinosaurs (bootleg set 1 (with PIC16c57), 930201 etc)\0", "No sound", "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, DinopicRomInfo, DinopicRomName, NULL, NULL, DinoInputInfo, DinoDIPInfo,
	DinopicInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinopic2 = {
	"dinopic2", "dino", NULL, NULL, "1993",
	"Cadillacs and Dinosaurs (bootleg set 2 (with PIC16c57), 930201 etc)\0", "No sound", "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Dinopic2RomInfo, Dinopic2RomName, NULL, NULL, DinoInputInfo, DinoDIPInfo,
	DinopicInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinoh = {
	"dinoh", "dino", NULL, NULL, "1993",
	"Cadillacs and Dinosaurs (bootleg set 3, 930223 Asia TW)\0", NULL, "bootleg", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, DinohRomInfo, DinohRomName, NULL, NULL, DinohInputInfo, DinohDIPInfo,
	DinohInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinot = {
	"dinot", "dino", NULL, NULL, "1993",
	"Cadillacs and Dinosaurs Turbo (bootleg set 1, 930223 Asia TW)\0", NULL, "bootleg", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, DinotRomInfo, DinotRomName, NULL, NULL, DinohInputInfo, DinohDIPInfo,
	DinotInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinotpic = {
	"dinotpic", "dino", NULL, NULL, "1993",
	"Cadillacs and Dinosaurs Turbo (bootleg set 2 (with PIC16c57), 930201 etc)\0", "No sound", "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, DinotpicRomInfo, DinotpicRomName, NULL, NULL, DinoInputInfo, DinoDIPInfo,
	DinotpicInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinohunt = {
	"dinohunt", "dino", NULL, NULL, "1993",
	"Dinosaur Hunter (Chinese bootleg, 930223 Asia TW)\0", NULL, "bootleg", "CPS1",
	L"Dinosaur Hunter \u6050\u9F8D\u7375\u4EBA (Chinese bootleg, 930223 Asia TW)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, DinohuntRomInfo, DinohuntRomName, NULL, NULL, DinoInputInfo, DinoDIPInfo,
	DinohuntInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDinoeh = {
	"dinoeh", "dino", NULL, NULL, "1993",
	"Cadillacs and Dinosaurs (hack, 930201 etc)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, DinoehRomInfo, DinoehRomName, NULL, NULL, DinoInputInfo, DinoDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDynwar = {
	"dynwar", NULL, NULL, NULL, "1989",
	"Dynasty Wars (US set 1)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, DynwarRomInfo, DynwarRomName, NULL, NULL, DynwarInputInfo, DynwarDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDynwara = {
	"dynwara", "dynwar", NULL, NULL, "1989",
	"Dynasty Wars (US set 2)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, DynwaraRomInfo, DynwaraRomName, NULL, NULL, DynwarInputInfo, DynwarDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDynwarj = {
	"dynwarj", "dynwar", NULL, NULL, "1989",
	"Tenchi wo Kurau (Japan)\0", NULL, "Capcom", "CPS1",
	L"\u5929\u5730\u3092\u55B0\u3089\u3046\0Tenchi wo Kurau (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, DynwarjRomInfo, DynwarjRomName, NULL, NULL, DynwarInputInfo, DynwarDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDynwarjr = {
	"dynwarjr", "dynwar", NULL, NULL, "1989",
	"Tenchi wo Kurau (Japan Resale Ver.)\0", NULL, "Capcom", "CPS1",
	L"\u5929\u5730\u3092\u55B0\u3089\u3046\0Tenchi wo Kurau (Japan Resale Ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, DynwarjrRomInfo, DynwarjrRomName, NULL, NULL, DynwarInputInfo, DynwarDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfight = {
	"ffight", NULL, NULL, NULL, "1989",
	"Final Fight (World, set 1)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, FfightRomInfo, FfightRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfighta = {
	"ffighta", "ffight", NULL, NULL, "1989",
	"Final Fight (World, set 2)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, FfightaRomInfo, FfightaRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfightu = {
	"ffightu", "ffight", NULL, NULL, "1989",
	"Final Fight (USA, set 1)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, FfightuRomInfo, FfightuRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfightu1 = {
	"ffightu1", "ffight", NULL, NULL, "1989",
	"Final Fight (USA, set 2)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Ffightu1RomInfo, Ffightu1RomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfightua = {
	"ffightua", "ffight", NULL, NULL, "1989",
	"Final Fight (900112 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, FfightuaRomInfo, FfightuaRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfightub = {
	"ffightub", "ffight", NULL, NULL, "1989",
	"Final Fight (900613 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, FfightubRomInfo, FfightubRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfightj = {
	"ffightj", "ffight", NULL, NULL, "1989",
	"Final Fight (Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, FfightjRomInfo, FfightjRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfightj1 = {
	"ffightj1", "ffight", NULL, NULL, "1989",
	"Final Fight (900112 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Ffightj1RomInfo, Ffightj1RomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfightj2 = {
	"ffightj2", "ffight", NULL, NULL, "1989",
	"Final Fight (900305 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Ffightj2RomInfo, Ffightj2RomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFfightjh = {
	"ffightjh", "ffight", NULL, NULL, "1989",
	"Street Smart / Final Fight (Japan, hack)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, FfightjhRomInfo, FfightjhRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsFcrash = {
	"fcrash", "ffight", NULL, NULL, "1990",
	"Final Crash (bootleg (with 2xYM2203 + 2xMSM5205))\0", NULL, "Playmark", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, FcrashRomInfo, FcrashRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	FcrashInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsForgottn = {
	"forgottn", NULL, NULL, NULL, "1988",
	"Forgotten Worlds (World)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_HORSHOOT, 0,
	NULL, ForgottnRomInfo, ForgottnRomName, NULL, NULL, ForgottnInputInfo, ForgottnDIPInfo,
	ForgottnAltGfxInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsForgottnu = {
	"forgottnu", "forgottn", NULL, NULL, "1988",
	"Forgotten Worlds (US, B-Board 88621B-2, rev C)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_HORSHOOT, 0,
	NULL, ForgottnuRomInfo, ForgottnuRomName, NULL, NULL, ForgottnInputInfo, ForgottnDIPInfo,
	ForgottnAltGfxuInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsForgottnu1 = {
	"forgottnu1", "forgottn", NULL, NULL, "1988",
	"Forgotten Worlds (US, B-Board 88618B-2, Rev C)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_HORSHOOT, 0,
	NULL, Forgottnu1RomInfo, Forgottnu1RomName, NULL, NULL, ForgottnInputInfo, ForgottnDIPInfo,
	ForgottnInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsForgottnua = {
	"forgottnua", "forgottn", NULL, NULL, "1988",
	"Forgotten Worlds (US, B-Board 88618B-2, rev A)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_HORSHOOT, 0,
	NULL, ForgottnuaRomInfo, ForgottnuaRomName, NULL, NULL, ForgottnInputInfo, ForgottnDIPInfo,
	ForgottnInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsLostwrld = {
	"lostwrld", "forgottn", NULL, NULL, "1988",
	"Lost Worlds (Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_HORSHOOT, 0,
 	NULL, LostwrldRomInfo, LostwrldRomName, NULL, NULL, ForgottnInputInfo, ForgottnDIPInfo,
	ForgottnInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsLostwrldo = {
	"lostwrldo", "forgottn", NULL, NULL, "1988",
	"Lost Worlds (Japan Old ver.)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_HORSHOOT, 0,
 	NULL, LostwrldoRomInfo, LostwrldoRomName, NULL, NULL, ForgottnInputInfo, ForgottnDIPInfo,
	ForgottnInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsGhouls = {
	"ghouls", NULL, NULL, NULL, "1988",
	"Ghouls'n Ghosts (World)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, GhoulsRomInfo, GhoulsRomName, NULL, NULL, GhoulsInputInfo, GhoulsDIPInfo,
	GhoulsInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsGhoulsu = {
	"ghoulsu", "ghouls", NULL, NULL, "1988",
	"Ghouls'n Ghosts (US)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, GhoulsuRomInfo, GhoulsuRomName, NULL, NULL, GhoulsInputInfo, GhoulsuDIPInfo,
	GhoulsInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDaimakai = {
	"daimakai", "ghouls", NULL, NULL, "1988",
	"Dai Makai-Mura (Japan)\0", NULL, "Capcom", "CPS1",
	L"\u5927\u9B54\u754C\u6751\0Dai Makai-Mura (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, DaimakaiRomInfo, DaimakaiRomName, NULL, NULL, GhoulsInputInfo, DaimakaiDIPInfo,
	DaimakaiInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDaimakair = {
	"daimakair", "ghouls", NULL, NULL, "1988",
	"Dai Makai-Mura (Japan Resale Ver.)\0", NULL, "Capcom", "CPS1",
	L"\u5927\u9B54\u754C\u6751\0Dai Makai-Mura (Japan Resale Ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, DaimakairRomInfo, DaimakairRomName, NULL, NULL, GhoulsInputInfo, DaimakaiDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsDaimakaib = {
	"daimakaib", "ghouls", NULL, NULL, "1988",
	"Dai Makai-Mura (bootleg, Japan)\0", NULL, "Capcom", "CPS1",
	L"\u5927\u9B54\u754C\u6751\0Dai Makai-Mura (bootleg, Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, DaimakaibRomInfo, DaimakaibRomName, NULL, NULL, GhoulsInputInfo, DaimakaiDIPInfo,
	DaimakaibInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnights = {
	"knights", NULL, NULL, NULL, "1991",
	"Knights of the Round (911127 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KnightsRomInfo, KnightsRomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsu = {
	"knightsu", "knights", NULL, NULL, "1991",
	"Knights of the Round (911127 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KnightsuRomInfo, KnightsuRomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsj = {
	"knightsj", "knights", NULL, NULL, "1991",
	"Knights of the Round (911127 Japan, B-Board 91634B-2)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KnightsjRomInfo, KnightsjRomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsja = {
	"knightsja", "knights", NULL, NULL, "1991",
	"Knights of the Round (911127 Japan, B-Board 89625B-1)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KnightsjaRomInfo, KnightsjaRomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsb = {
	"knightsb", "knights", NULL, NULL, "1991",
	"Knights of the Round (bootleg set 1 (with YM2151 + 2xMSM5205), 911127 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KnightsbRomInfo, KnightsbRomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	KnightsbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsb2 = {
	"knightsb2", "knights", NULL, NULL, "1991",
	"Knights of the Round (bootleg set 2, 911127 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Knightsb2RomInfo, Knightsb2RomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	Knightsb2Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsb3 = {
	"knightsb3", "knights", NULL, NULL, "1991",
	"Knights of the Round (bootleg set 3, 911127 Japan)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Knightsb3RomInfo, Knightsb3RomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsb4 = {
	"knightsb4", "knights", NULL, NULL, "1991",
	"Knights of the Round (bootleg set 4 (with YM2151 + 2xMSM5205), 911127 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Knightsb4RomInfo, Knightsb4RomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	KnightsbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsh = {
	"knightsh", "knights", NULL, NULL, "1991",
	"Knights of the Round (hack set 1)\0", NULL, "hack", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KnightshRomInfo, KnightshRomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKnightsh2 = {
	"knightsh2", "knights", NULL, NULL, "1991",
	"Knights of the Round (hack set 2, 911127 etc)\0", NULL, "hack", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Knightsh2RomInfo, Knightsh2RomName, NULL, NULL, KnightsInputInfo, KnightsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKod = {
	"kod", NULL, NULL, NULL, "1991",
	"The King of Dragons (910805 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KodRomInfo, KodRomName, NULL, NULL, KodInputInfo, KodDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKodr1 = {
	"kodr1", "kod", NULL, NULL, "1991",
	"The King of Dragons (910711 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Kodr1RomInfo, Kodr1RomName, NULL, NULL, KodInputInfo, Kodr1DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKodu = {
	"kodu", "kod", NULL, NULL, "1991",
	"The King of Dragons (US 910910)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KoduRomInfo, KoduRomName, NULL, NULL, KodInputInfo, KodDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKodj = {
	"kodj", "kod", NULL, NULL, "1991",
	"The King of Dragons (Japan 910805, B-Board 90629B-3)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KodjRomInfo, KodjRomName, NULL, NULL, KodInputInfo, KodDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKodja = {
	"kodja", "kod", NULL, NULL, "1991",
	"The King of Dragons (Japan 910805, B-Board 89625B-1)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KodjaRomInfo, KodjaRomName, NULL, NULL, KodInputInfo, KodDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKodb = {
	"kodb", "kod", NULL, NULL, "1991",
	"The King of Dragons (bootleg, 910731 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KodbRomInfo, KodbRomName, NULL, NULL, KodInputInfo, KodDIPInfo,
	KodbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKodh = {
	"kodh", "kod", NULL, NULL, "2002",
	"The King of Dragons (hack)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KodhRomInfo, KodhRomName, NULL, NULL, KodhInputInfo, KodDIPInfo,
	KodhInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsKodda = {
	"kodda", "kod", NULL, NULL, "1991",
	"The King of Dragons (Phoenix bootleg, 910731 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, KoddaRomInfo, KoddaRomName, NULL, NULL, KodInputInfo, KodDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMegaman = {
	"megaman", NULL, NULL, NULL, "1995",
	"Mega Man - the power battle (951006 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MegamanRomInfo, MegamanRomName, NULL, NULL, MegamanInputInfo, MegamanDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMegamana = {
	"megamana", "megaman", NULL, NULL, "1995",
	"Mega Man - the power battle (951006 Asia)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MegamanaRomInfo, MegamanaRomName, NULL, NULL, MegamanInputInfo, MegamanDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsRockmanj = {
	"rockmanj", "megaman", NULL, NULL, "1995",
	"Rockman - the power battle (950922 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, RockmanjRomInfo, RockmanjRomName, NULL, NULL, MegamanInputInfo, RockmanjDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMercs = {
	"mercs", NULL, NULL, NULL, "1990",
	"Mercs (900302 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, 0,
	NULL, MercsRomInfo, MercsRomName, NULL, NULL, MercsInputInfo, MercsDIPInfo,
	MercsInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCpsMercsu = {
	"mercsu", "mercs", NULL, NULL, "1990",
	"Mercs (900608 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, 0,
	NULL, MercsuRomInfo, MercsuRomName, NULL, NULL, MercsInputInfo, MercsDIPInfo,
	MercsInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCpsMercsur1 = {
	"mercsur1", "mercs", NULL, NULL, "1990",
	"Mercs (900302 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, 0,
	NULL, Mercsur1RomInfo, Mercsur1RomName, NULL, NULL, MercsInputInfo, MercsDIPInfo,
	MercsInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCpsMercsj = {
	"mercsj", "mercs", NULL, NULL, "1990",
	"Senjo no Ookami II (Ookami 2 900302 Japan)\0", NULL, "Capcom", "CPS1",
	L"\u6226\u5834\u306E\u72FC II\0Senjo no Ookami II (Ookami 2 900302 Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, 0,
	NULL, MercsjRomInfo, MercsjRomName, NULL, NULL, MercsInputInfo, MercsDIPInfo,
	MercsInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCpsMsword = {
	"msword", NULL, NULL, NULL, "1990",
	"Magic Sword - heroic fantasy (25.07.1990 other country)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, MswordRomInfo, MswordRomName, NULL, NULL, MswordInputInfo, MswordDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMswordr1 = {
	"mswordr1", "msword", NULL, NULL, "1990",
	"Magic Sword - heroic fantasy (23.06.1990 other country)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Mswordr1RomInfo, Mswordr1RomName, NULL, NULL, MswordInputInfo, MswordDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMswordj = {
	"mswordj", "msword", NULL, NULL, "1990",
	"Magic Sword (23.06.1990 Japan)\0", NULL, "Capcom", "CPS1",
	L"Magic Sword (23.06.1990 Japan)\0Magic Sword\u30DE\u30B8\u30C3\u30AF\uFF65\u30BD\u30FC\u30C9\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, MswordjRomInfo, MswordjRomName, NULL, NULL, MswordInputInfo, MswordDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMswordu = {
	"mswordu", "msword", NULL, NULL, "1990",
	"Magic Sword - heroic fantasy (25.07.1990 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, MsworduRomInfo, MsworduRomName, NULL, NULL, MswordInputInfo, MswordDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMtwins = {
	"mtwins", NULL, NULL, NULL, "1990",
	"Mega Twins (chiki chiki boys 900619 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, MtwinsRomInfo, MtwinsRomName, NULL, NULL, MtwinsInputInfo, MtwinsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsChikij = {
	"chikij", "mtwins", NULL, NULL, "1990",
	"Chiki Chiki Boys (900619 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, ChikijRomInfo, ChikijRomName, NULL, NULL, MtwinsInputInfo, MtwinsDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsNemo = {
	"nemo", NULL, NULL, NULL, "1990",
	"Nemo (90 11 30 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, NemoRomInfo, NemoRomName, NULL, NULL, NemoInputInfo, NemoDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsNemoj = {
	"nemoj", "nemo", NULL, NULL, "1990",
	"Nemo (90 11 20 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2 ,HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, NemojRomInfo, NemojRomName, NULL, NULL, NemoInputInfo, NemoDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPang3 = {
	"pang3", NULL, NULL, NULL, "1995",
	"Pang! 3 (950601 Euro)\0", NULL, "Mitchell", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_PUZZLE, 0,
	NULL, Pang3RomInfo, Pang3RomName, NULL, NULL, Pang3InputInfo, Pang3DIPInfo,
	Pang3Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPang3r1 = {
	"pang3r1", "pang3", NULL, NULL, "1995",
	"Pang! 3 (950511 Euro)\0", NULL, "Mitchell", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_PUZZLE, 0,
	NULL, Pang3r1RomInfo, Pang3r1RomName, NULL, NULL, Pang3InputInfo, Pang3DIPInfo,
	Pang3Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPang3b = {
	"pang3b", "pang3", NULL, NULL, "1995",
	"Pang! 3 (bootleg set 1, 950511 Euro)\0", NULL, "Mitchell", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_PUZZLE, 0,
	NULL, Pang3bRomInfo, Pang3bRomName, NULL, NULL, Pang3InputInfo, Pang3DIPInfo,
	Pang3bInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPang3b2 = {
	"pang3b2", "pang3", NULL, NULL, "1995",
	"Pang! 3 (bootleg set 2, 950511 Euro)\0", NULL, "Mitchell", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_PUZZLE, 0,
	NULL, Pang3b2RomInfo, Pang3b2RomName, NULL, NULL, Pang3InputInfo, Pang3DIPInfo,
	Pang3Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPang3j = {
	"pang3j", "pang3", NULL, NULL, "1995",
	"Pang! 3: Kaitou Tachi no Karei na Gogo (950511 Japan)\0", NULL, "Mitchell", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_GENERIC, GBF_PUZZLE, 0,
	NULL, Pang3jRomInfo, Pang3jRomName, NULL, NULL, Pang3InputInfo, Pang3DIPInfo,
	Pang3Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPnickj = {
	"pnickj", NULL, NULL, NULL, "1994",
	"Pnickies (940608 Japan)\0", NULL, "Compile (Capcom license)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_PUZZLE, 0,
	NULL, PnickjRomInfo, PnickjRomName, NULL, NULL, PnickjInputInfo, PnickjDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPunisher = {
	"punisher", NULL, NULL, NULL, "1993",
	"The Punisher (930422 etc)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, PunisherRomInfo, PunisherRomName, NULL, NULL, PunisherInputInfo, PunisherDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPunisheru = {
	"punisheru", "punisher", NULL, NULL, "1993",
	"The Punisher (930422 USA)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, PunisheruRomInfo, PunisheruRomName, NULL, NULL, PunisherInputInfo, PunisherDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPunisherj = {
	"punisherj", "punisher", NULL, NULL, "1993",
	"The Punisher (930422 Japan)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, PunisherjRomInfo, PunisherjRomName, NULL, NULL, PunisherInputInfo, PunisherDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPunipic = {
	"punipic", "punisher", NULL, NULL, "1993",
	"The Punisher (bootleg set 1 (with PIC16c57), 930422 etc)\0", "No sound", "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, PunipicRomInfo, PunipicRomName, NULL, NULL, PunisherInputInfo, PunisherDIPInfo,
	PunipicInit, PunipicExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPunipic2 = {
	"punipic2", "punisher", NULL, NULL, "1993",
	"The Punisher (bootleg set 2 (with PIC16c57), 930422 etc)\0", "No sound", "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Punipic2RomInfo, Punipic2RomName, NULL, NULL, PunisherInputInfo, PunisherDIPInfo,
	Punipic2Init, PunipicExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPunipic3 = {
	"punipic3", "punisher", NULL, NULL, "1993",
	"The Punisher (bootleg set 3 (with PIC16c57), 930422 etc)\0", "No sound", "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Punipic3RomInfo, Punipic3RomName, NULL, NULL, PunisherInputInfo, PunisherDIPInfo,
	Punipic3Init, PunipicExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsPunisherbz = {
	"punisherbz", "punisher", NULL, NULL, "2002",
	"Biaofeng Zhanjing (Chinese bootleg)\0", NULL, "bootleg", "CPS1",
	L"\uFEFF\u98C6\u98A8\u6230\u8B66\0The Punisher: Biaofeng Zhanjing (Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, PunisherbzRomInfo, PunisherbzRomName, NULL, NULL, PunisherbzInputInfo, PunisherbzDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsQad = {
	"qad", NULL, NULL, NULL, "1992",
	"Quiz & Dragons (capcom quiz game 920701 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_QUIZ, 0,
	NULL, QadRomInfo, QadRomName, NULL, NULL, QadInputInfo, QadDIPInfo,
	QadInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsQadj = {
	"qadj", "qad", NULL, NULL, "1992",
	"Quiz & Dragons (940921 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_QUIZ, 0,
	NULL, QadjRomInfo, QadjRomName, NULL, NULL, QadInputInfo, QadjDIPInfo,
	QadInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsQtono2j = {
	"qtono2j", NULL, NULL, NULL, "1995",
	"Quiz Tonosama no Yabou 2 Zenkoku-ban (tonosama 2 950123 Japan)\0", NULL, "Capcom", "CPS1",
	L"\u30AF\u30A4\u30BA\u6BBF\u69D8\u306E\u91CE\u671B\uFF12 (\u3068\u306E\u3055\u307E\u306E\u3084\u307C\u3046 2)\0Quiz Tonosama no Yabou 2 Zenkoku-ban (tonosama 2 950123 Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_QUIZ, 0,
	NULL, Qtono2jRomInfo, Qtono2jRomName, NULL, NULL, Qtono2jInputInfo, Qtono2jDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2 = {
	"sf2", NULL, NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910522 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2RomInfo, Sf2RomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2eb = {
	"sf2eb", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910214 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ebRomInfo, Sf2ebRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ee = {
	"sf2ee", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910228 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2eeRomInfo, Sf2eeRomName, NULL, NULL, Sf2ueInputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ebbl = {
	"sf2ebbl", "sf2", NULL, NULL, "1992",
	"Street Fighter II - the world warrior (TAB Austria bootleg, 910214 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ebblRomInfo, Sf2ebblRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2ebblInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2stt = {
	"sf2stt", "sf2", NULL, NULL, "1992",
	"Street Fighter II - the world warrior (STT, TAB Austria bootleg, 910214 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2sttRomInfo, Sf2sttRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2sttInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ua = {
	"sf2ua", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910206 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2uaRomInfo, Sf2uaRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ub = {
	"sf2ub", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910214 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ubRomInfo, Sf2ubRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2uc = {
	"sf2uc", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910306 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ucRomInfo, Sf2ucRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ud = {
	"sf2ud", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910318 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2udRomInfo, Sf2udRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ue = {
	"sf2ue", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910228 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ueRomInfo, Sf2ueRomName, NULL, NULL, Sf2ueInputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2uf = {
	"sf2uf", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910411 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ufRomInfo, Sf2ufRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ug = {
	"sf2ug", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910522 USA, rev G)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ugRomInfo, Sf2ugRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ui = {
	"sf2ui", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910522 USA, rev I)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2uiRomInfo, Sf2uiRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2uk = {
	"sf2uk", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (911101 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ukRomInfo, Sf2ukRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2j = {
	"sf2j", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (911210 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2jRomInfo, Sf2jRomName, NULL, NULL, Sf2InputInfo, Sf2jDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ja = {
	"sf2ja", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910214 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2jaRomInfo, Sf2jaRomName, NULL, NULL, Sf2InputInfo, Sf2jDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2jc = {
	"sf2jc", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (910306 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2jcRomInfo, Sf2jcRomName, NULL, NULL, Sf2InputInfo, Sf2jDIPInfo,
	Sf2jcInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2qp1 = {
	"sf2qp1", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (Quicken Pt-I bootleg, 910214 USA)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2qp1RomInfo, Sf2qp1RomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2qp1Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2thndr = {
	"sf2thndr", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (Thunder Edition bootleg, 910214 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2thndrRomInfo, Sf2thndrRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2thndrInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriverX BurnDrvCpsSf2b = {
	"sf2b", "sf2", NULL, NULL, "1991",
	"Street Fighter II - the world warrior (bootleg)\0", "Bad dump?, Resets itself", "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2bRomInfo, Sf2bRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ce = {
	"sf2ce", NULL, NULL, NULL, "1992",
	"Street Fighter II' - champion edition (street fighter 2' 920513 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ceRomInfo, Sf2ceRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ceea = {
	"sf2ceea", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (street fighter 2' 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ceeaRomInfo, Sf2ceeaRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ceja = {
	"sf2ceja", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (street fighter 2' 920322 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2cejaRomInfo, Sf2cejaRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2cejb = {
	"sf2cejb", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (street fighter 2' 920513 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2cejbRomInfo, Sf2cejbRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2cejc = {
	"sf2cejc", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (street fighter 2' 920803 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2cejcRomInfo, Sf2cejcRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ceua = {
	"sf2ceua", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (street fighter 2' 920313 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ceuaRomInfo, Sf2ceuaRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ceub = {
	"sf2ceub", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (street fighter 2' 920513 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ceubRomInfo, Sf2ceubRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2ceuc = {
	"sf2ceuc", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (street fighter 2' 920803 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2ceucRomInfo, Sf2ceucRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2acc = {
	"sf2acc", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Accelerator! bootleg set 1, 920313 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2,HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2accRomInfo, Sf2accRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2acca = {
	"sf2acca", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Accelerator! bootleg set 2, 920310 Accelerator!)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2,HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2accaRomInfo, Sf2accaRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2accp2 = {
	"sf2accp2", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Accelerator Pt.II bootleg, 920313 Testron)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2,HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2accp2RomInfo, Sf2accp2RomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2accp2Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2dkot2 = {
	"sf2dkot2", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Double K.O. Turbo II bootleg, 902140 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2,HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2dkot2RomInfo, Sf2dkot2RomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2dkot2Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2rb = {
	"sf2rb", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Rainbow bootleg set 1, 920322 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2,HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2rbRomInfo, Sf2rbRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2rbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2rb2 = {
	"sf2rb2", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Rainbow bootleg set 2, 920322 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2,HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2rb2RomInfo, Sf2rb2RomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2rb2Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2rb3 = {
	"sf2rb3", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Rainbow bootleg set 3, 920322 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2,HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2rb3RomInfo, Sf2rb3RomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2red = {
	"sf2red", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Red Wave bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2redRomInfo, Sf2redRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2v004 = {
	"sf2v004", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (V004 bootleg, 102092 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2v004RomInfo, Sf2v004RomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2hf = {
	"sf2hf", NULL, NULL, NULL, "1992",
	"Street Fighter II' - hyper fighting (street fighter 2' T 921209 ETC)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2hfRomInfo, Sf2hfRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2hfu = {
	"sf2hfu", "sf2hf", NULL, NULL, "1992",
	"Street Fighter II' - hyper fighting (street fighter 2' T 921209 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2hfuRomInfo, Sf2hfuRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2hfj = {
	"sf2hfj", "sf2hf", NULL, NULL, "1992",
	"Street Fighter II' Turbo - hyper fighting (street fighter 2' T 921209 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2hfjRomInfo, Sf2hfjRomName, NULL, NULL, Sf2InputInfo, Sf2jDIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2hfjb = {
	"sf2hfjb", "sf2hf", NULL, NULL, "1992",
	"Street Fighter II' Turbo - hyper fighting (bootleg set 1, 921209 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2hfjbRomInfo, Sf2hfjbRomName, NULL, NULL, Sf2yycInputInfo, Sf2jDIPInfo,
	Sf2hfjbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2hfjb2 = {
	"sf2hfjb2", "sf2hf", NULL, NULL, "1992",
	"Street Fighter II' Turbo - hyper fighting (bootleg set 2, 921209 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2hfjb2RomInfo, Sf2hfjb2RomName, NULL, NULL, Sf2yycInputInfo, Sf2jDIPInfo,
	Sf2hfjbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2yyc = {
	"sf2yyc", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (YYC bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2yycRomInfo, Sf2yycRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2yycInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2koryu = {
	"sf2koryu", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - Xiang Long (bootleg set 1, 811102 001)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2koryuRomInfo, Sf2koryuRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2koryuInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2koryu2 = {
	"sf2koryu2", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - Xiang Long (bootleg set 2, 811102 001)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2koryu2RomInfo, Sf2koryu2RomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2koryu2Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2amf = {
	"sf2amf", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II - champion edition (Alpha Magic-F bootleg, 920313 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2amfRomInfo, Sf2amfRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2amfInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2mdt = {
	"sf2mdt", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - Magic Delta Turbo (bootleg set 1 (with YM2151 + 2xMSM5205), 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2mdtRomInfo, Sf2mdtRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2mdtInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2mdta = {
	"sf2mdta", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - Magic Delta Turbo (bootleg set 2 (with YM2151 + 2xMSM5205), 920313 etc)\0", "Bad graphics rom dump", "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2mdtaRomInfo, Sf2mdtaRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2mdtaInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m1 = {
	"sf2m1", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M1 bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m1RomInfo, Sf2m1RomName, NULL, NULL, Sf2m1InputInfo, Sf2DIPInfo,
	Sf2m1Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m2 = {
	"sf2m2", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M2 bootleg (Mega Co), 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m2RomInfo, Sf2m2RomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m2Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m3 = {
	"sf2m3", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M3 bootleg, 920313 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m3RomInfo, Sf2m3RomName, NULL, NULL, Sf2m3InputInfo, Sf2DIPInfo,
	Sf2m3Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m4 = {
	"sf2m4", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M4 bootleg, 920322 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m4RomInfo, Sf2m4RomName, NULL, NULL, Sf2yycInputInfo, Sf2jDIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m5 = {
	"sf2m5", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M5 bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m5RomInfo, Sf2m5RomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m6 = {
	"sf2m6", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M6 bootleg, 811102 001)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m6RomInfo, Sf2m6RomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m7 = {
	"sf2m7", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M7 bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m7RomInfo, Sf2m7RomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m8 = {
	"sf2m8", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M8 bootleg, 920313 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m8RomInfo, Sf2m8RomName, NULL, NULL, Sf2m3InputInfo, Sf2DIPInfo,
	Sf2m8Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m9 = {
	"sf2m9", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M9 bootleg (Mega Co), 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m9RomInfo, Sf2m9RomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m2Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m10 = {
	"sf2m10", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M10 bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m10RomInfo, Sf2m10RomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m11 = {
	"sf2m11", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M11 bootleg, 920322 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m11RomInfo, Sf2m11RomName, NULL, NULL, Sf2yycInputInfo, Sf2jDIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m12 = {
	"sf2m12", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M12 bootleg, 920322 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m12RomInfo, Sf2m12RomName, NULL, NULL, Sf2yycInputInfo, Sf2jDIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m13 = {
	"sf2m13", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M13 bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m13RomInfo, Sf2m13RomName, NULL, NULL, Sf2yycInputInfo, Sf2jDIPInfo,
	Sf2yycInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m14 = {
	"sf2m14", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M14 bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m14RomInfo, Sf2m14RomName, NULL, NULL, Sf2yycInputInfo, Sf2jDIPInfo,
	Sf2yycInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2m15 = {
	"sf2m15", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (M15 bootleg, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2m15RomInfo, Sf2m15RomName, NULL, NULL, Sf2yycInputInfo, Sf2jDIPInfo,
	Sf2yycInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2tlona = {
	"sf2tlona", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Tu Long bootleg set 1, 811102 001)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2tlonaRomInfo, Sf2tlonaRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2tlonb = {
	"sf2tlonb", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Tu Long bootleg set 2, 811102 001)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2tlonbRomInfo, Sf2tlonbRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2tlonc = {
	"sf2tlonc", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Tu Long bootleg set 3, 811102 001)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2tloncRomInfo, Sf2tloncRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2m4Init, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2th = {
	"sf2th", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Turbo bootleg set 1, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2thRomInfo, Sf2thRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2thInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSf2tha = {
	"sf2tha", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (Turbo bootleg set 2, 920313 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2thaRomInfo, Sf2thaRomName, NULL, NULL, Sf2yycInputInfo, Sf2DIPInfo,
	Sf2thInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSfzch = {
	"sfzch", NULL, NULL, NULL, "1995",
	"Street Fighter Zero (CPS Changer, 951020 Japan)\0", NULL, "Capcom", "CPS Changer",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPSCHANGER, GBF_VSFIGHT, FBF_SF,
	NULL, SfzchRomInfo, SfzchRomName, NULL, NULL, SfzchInputInfo, NULL,
	SfzchInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSlammast = {
	"slammast", NULL, NULL, NULL, "1993",
	"Saturday Night Slam Masters (Slam Masters 930713 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, SlammastRomInfo, SlammastRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSlammastu = {
	"slammastu", "slammast", NULL, NULL, "1993",
	"Saturday Night Slam Masters (slam masters 930713 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, SlammastuRomInfo, SlammastuRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMbomberj = {
	"mbomberj", "slammast", NULL, NULL, "1993",
	"Muscle Bomber - the body explosion (930713 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MbomberjRomInfo, MbomberjRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMbombrd = {
	"mbombrd", NULL, NULL, NULL, "1993",
	"Muscle Bomber Duo - ultimate team battle (931206 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MbombrdRomInfo, MbombrdRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsMbombrdj = {
	"mbombrdj", "mbombrd", NULL, NULL, "1993",
	"Muscle Bomber Duo - heat up warriors (931206 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MbombrdjRomInfo, MbombrdjRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsStrider = {
	"strider", NULL, NULL, NULL, "1989",
	"Strider (US set 1)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, StriderRomInfo, StriderRomName, NULL, NULL, StriderInputInfo, StriderDIPInfo,
	StriderInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsStriderua = {
	"striderua", "strider", NULL, NULL, "1989",
	"Strider (US set 2)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, StrideruaRomInfo, StrideruaRomName, NULL, NULL, StriderInputInfo, StrideruaDIPInfo,
	StriderInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsStriderjr = {
	"striderjr", "strider", NULL, NULL, "1989",
	"Strider Hiryu (Japan Resale Ver.)\0", NULL, "Capcom", "CPS1",
	L"\u30B9\u30C8\u30E9\u30A4\u30C0\u30FC\u98DB\u7ADC\0Strider Hiryu (Japan Resale Ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, StriderjrRomInfo, StriderjrRomName, NULL, NULL, StriderInputInfo, StriderDIPInfo,
	StriderjrInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsStriderj = {
	"striderj", "strider", NULL, NULL, "1989",
	"Strider Hiryu (Japan)\0", NULL, "Capcom", "CPS1",
	L"\u30B9\u30C8\u30E9\u30A4\u30C0\u30FC\u98DB\u7ADC\0Strider Hiryu (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, StriderjRomInfo, StriderjRomName, NULL, NULL, StriderInputInfo, StriderDIPInfo,
	StriderjInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsUnsquad = {
	"unsquad", NULL, NULL, NULL, "1989",
	"U.N. Squadron (US)\0", NULL, "Daipro / Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_HORSHOOT, 0,
	NULL, UnsquadRomInfo, UnsquadRomName, NULL, NULL, UnsquadInputInfo, UnsquadDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsArea88 = {
	"area88", "unsquad", NULL, NULL, "1989",
	"Area 88 (Japan)\0", NULL, "Daipro / Capcom", "CPS1",
	L"\u30A8\u30EA\u30A2\uFF18\uFF18\0Area 88 (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_HORSHOOT, 0,
	NULL, Area88RomInfo, Area88RomName, NULL, NULL, UnsquadInputInfo, UnsquadDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsArea88r = {
	"area88r", "unsquad", NULL, NULL, "1989",
	"Area 88 (Japan Resale ver.)\0", NULL, "Daipro / Capcom", "CPS1",
	L"\u30A8\u30EA\u30A2\uFF18\uFF18\0Area 88 (Japan Resale ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_HORSHOOT, 0,
	NULL, Area88rRomInfo, Area88rRomName, NULL, NULL, UnsquadInputInfo, UnsquadDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsVarth = {
	"varth", NULL, NULL, NULL, "1992",
	"Varth - operation thunderstorm (920714 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, 0,
	NULL, VarthRomInfo, VarthRomName, NULL, NULL, VarthInputInfo, VarthDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCpsVarthr1 = {
	"varthr1", "varth", NULL, NULL, "1992",
	"Varth - operation thunderstorm (920612 etc)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, 0,
	NULL, Varthr1RomInfo, Varthr1RomName, NULL, NULL, VarthInputInfo, VarthDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCpsVarthj = {
	"varthj", "varth", NULL, NULL, "1992",
	"Varth - operation thunderstorm (920714 Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2 ,HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, 0,
	NULL, VarthjRomInfo, VarthjRomName, NULL, NULL, VarthInputInfo, VarthDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCpsVarthu = {
	"varthu", "varth", NULL, NULL, "1992",
	"Varth - operation thunderstorm (920612 USA)\0", NULL, "Capcom (Romstar license)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1, GBF_VERSHOOT, 0,
	NULL, VarthuRomInfo, VarthuRomName, NULL, NULL, VarthInputInfo, VarthDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 224, 384, 3, 4
};

struct BurnDriver BurnDrvCpsWillow = {
	"willow", NULL, NULL, NULL, "1989",
	"Willow (US)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, WillowRomInfo, WillowRomName, NULL, NULL, WillowInputInfo, WillowDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWillowo = {
	"willowo", "willow", NULL, NULL, "1989",
	"Willow (US old ver.)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, WillowoRomInfo, WillowoRomName, NULL, NULL, WillowInputInfo, WillowDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWillowj = {
	"willowj", "willow", NULL, NULL, "1989",
	"Willow (Japan, Japanese)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1, GBF_PLATFORM, 0,
	NULL, WillowjRomInfo, WillowjRomName, NULL, NULL, WillowInputInfo, WillowDIPInfo,
	DrvInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWof = {
	"wof", NULL, NULL, NULL, "1992",
	"Warriors of Fate (921031 etc)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WofRomInfo, WofRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofr1 = {
	"wofr1", "wof", NULL, NULL, "1992",
	"Warriors of Fate (921002 etc)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, Wofr1RomInfo, Wofr1RomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofu = {
	"wofu", "wof", NULL, NULL, "1992",
	"Warriors of Fate (921031 USA)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WofuRomInfo, WofuRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofj = {
	"wofj", "wof", NULL, NULL, "1992",
	"Tenchi wo Kurau II - Sekiheki no Tatakai (921031 Japan)\0", NULL, "Capcom", "CPS1 / QSound",
	L"\u5929\u5730\u3092\u55B0\u3089\u3046II - \u8D64\u58C1\u306E\u6226\u3044\0Tenchi wo Kurau II - Sekiheki no Tatakai (921031 Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WofjRomInfo, WofjRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofa = {
	"wofa", "wof", NULL, NULL, "1992",
	"Sangokushi II (921005 Asia)\0", NULL, "Capcom", "CPS1 / QSound",
	L"\u4E09\u56FD\u5FD7II\0Sangokushi II (921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WofaRomInfo, WofaRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofch = {
	"wofch", NULL, NULL, NULL, "1992",
	"Tenchi wo Kurau II - Sekiheki no Tatakai (CPS Changer, 921031 Japan)\0", NULL, "Capcom", "CPS Changer",
	L"\u5929\u5730\u3092\u55B0\u3089\u3046II - \u8D64\u58C1\u306E\u6226\u3044\0Tenchi wo Kurau II - Sekiheki no Tatakai (CPS Changer, 921031 Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_CAPCOM_CPSCHANGER, GBF_SCRFIGHT, 0,
	NULL, WofchRomInfo, WofchRomName, NULL, NULL, WofchInputInfo, NULL,
	WofchInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofhfh = {
	"wofhfh", "wof", NULL, NULL, "1992",
	"Sangokushi II: Huo Fenghuang (Chinese bootleg, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u706B\u9CF3\u51F0\0Sangokushi II: Huo Feng Huang (Chinese bootleg, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, WofhfhRomInfo, WofhfhRomName, NULL, NULL, WofhfhInputInfo, WofhfhDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofh = {
	"wofh", "wof", NULL, NULL, "1992",
	"Sangokushi II: Sanguo Yingxiong Zhuan (Chinese bootleg set 1, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u570B\u82F1\u96C4\u50B3\0Sangokushi II: Sanguo YingXiongZhuan (Chinese bootleg set 1, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, WofhRomInfo, WofhRomName, NULL, NULL, WofhInputInfo, WofhDIPInfo,
	WofhInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofha = {
	"wofha", "wof", NULL, NULL, "1992",
	"Sangokushi II: Sanguo Yingxiong Zhuan (Chinese bootleg set 2, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u570B\u82F1\u96C4\u50B3\0Sangokushi II: Sanguo YingXiongZhuan (Chinese bootleg set 2, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, WofhaRomInfo, WofhaRomName, NULL, NULL, WofhInputInfo, WofhDIPInfo,
	WofhInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsSgyxz = {
	"sgyxz", "wof", NULL, NULL, "1992",
	"Sangokushi II: Sanguo Yingxiong Zhuan (Chinese bootleg set 3, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u570B\u82F1\u96C4\u50B3\0Sangokushi II: Sanguo YingXiongZhuan (Chinese bootleg set 3, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, SgyxzRomInfo, SgyxzRomName, NULL, NULL, WofhInputInfo, WofhDIPInfo,
	SgyxzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWof3js = {
	"wof3js", "wof", NULL, NULL, "1992",
	"Sangokushi II: San Jian Sheng (Chinese bootleg set 1, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u528D\u8056\0Sangokushi II: San Jian Sheng (Chinese bootleg set 1, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Wof3jsRomInfo, Wof3jsRomName, NULL, NULL, WofhInputInfo, Wof3jsDIPInfo,
	Wof3jsInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWof3jsa = {
	"wof3jsa", "wof", NULL, NULL, "1992",
	"Sangokushi II: San Jian Sheng (Chinese bootleg set 2, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u528D\u8056\0Sangokushi II: San Jian Sheng (Chinese bootleg set 2, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Wof3jsaRomInfo, Wof3jsaRomName, NULL, NULL, WofhInputInfo, WofhDIPInfo,
	Wof3jsaInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWof3sj = {
	"wof3sj", "wof", NULL, NULL, "1992",
	"Sangokushi II: San Sheng Jian (Chinese bootleg set 1, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u5723\u5251\0Sangokushi II: San Sheng Jian (Chinese bootleg set 1, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Wof3sjRomInfo, Wof3sjRomName, NULL, NULL, WofhInputInfo, Wof3sjDIPInfo,
	Wof3sjInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWof3sja = {
	"wof3sja", "wof", NULL, NULL, "1992",
	"Sangokushi II: San Sheng Jian (Chinese bootleg set 2, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u5723\u5251\0Sangokushi II: San Sheng Jian (Chinese bootleg set 2, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, Wof3sjaRomInfo, Wof3sjaRomName, NULL, NULL, WofhInputInfo, Wof3sjDIPInfo,
	Wof3sjInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofsj = {
	"wofsj", "wof", NULL, NULL, "1992",
	"Sangokushi II: Sheng Jian Sanguo (Chinese bootleg set 1, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u5723\u5251\u4E09\0Sangokushi II: Sheng Jian Sanguo (Chinese bootleg set 1, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, WofsjRomInfo, WofsjRomName, NULL, NULL, WofhInputInfo, Wof3sjDIPInfo,
	WofsjInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofsja = {
	"wofsja", "wof", NULL, NULL, "1992",
	"Sangokushi II: Sheng Jian Sanguo (Chinese bootleg set 2, 921005 Asia)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u5723\u5251\u4E09\0Sangokushi II: Sheng Jian Sanguo (Chinese bootleg set 2, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, WofsjaRomInfo, WofsjaRomName, NULL, NULL, WofhInputInfo, Wof3sjDIPInfo,
	WofsjInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofsjb = {
	"wofsjb", "wof", NULL, NULL, "1992",
	"Sangokushi II: Sheng Jian Sanguo (Chinese bootleg set 3, 921005 Asia)\0", "No sound", "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u5723\u5251\u4E09\0Sangokushi II: Sheng Jian Sanguo (Chinese bootleg set 3, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, WofsjbRomInfo, WofsjbRomName, NULL, NULL, WofsjbInputInfo, NULL,
	WofsjbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofb = {
	"wofb", "wof", NULL, NULL, "1992",
	"Warriors of Fate (bootleg, 921002 etc)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_CAPCOM_CPS1, GBF_SCRFIGHT, 0,
	NULL, WofbRomInfo, WofbRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	WofbInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofjh = {
	"wofjh", "wof", NULL, NULL, "1992",
	"Tenchi wo Kurau II - Sekiheki no Tatakai (hack, 921031 Japan)\0", NULL, "hack", "CPS1 / QSound",
	L"\u5929\u5730\u3092\u55B0\u3089\u3046II - \u8D64\u58C1\u306E\u6226\u3044\0Tenchi wo Kurau II - Sekiheki no Tatakai (hack, 921031 Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WofjhRomInfo, WofjhRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofah = {
	"wofah", "wof", NULL, NULL, "1992",
	"Sangokushi II (hack set 1, 921005 Asia)\0", NULL, "hack", "CPS1 / QSound",
	L"\u4E09\u56FD\u5FD7II\0Sangokushi II (hack set 1, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WofahRomInfo, WofahRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofaha = {
	"wofaha", "wof", NULL, NULL, "1992",
	"Sangokushi II (hack set 2, 921005 Asia)\0", NULL, "hack", "CPS1 / QSound",
	L"\u4E09\u56FD\u5FD7II\0Sangokushi II (hack set 2, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WofahaRomInfo, WofahaRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsWofahb = {
	"wofahb", "wof", NULL, NULL, "1992",
	"Sangokushi II (hack set 3, 921005 Asia)\0", NULL, "hack", "CPS1 / QSound",
	L"\u4E09\u56FD\u5FD7II\0Sangokushi II (hack set 3, 921005 Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WofahbRomInfo, WofahbRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

struct BurnDriver BurnDrvCpsCps1frog = {
	"cps1frog", NULL, NULL, NULL, "2006",
	"Frog Feast (CPS-1)\0", NULL, "Rastersoft (Homebrew)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_CAPCOM_CPS1, GBF_MISC, 0,
	NULL, Cps1frogRomInfo, Cps1frogRomName, NULL, NULL, FfightInputInfo, FfightDIPInfo,
	Cps1demoInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Not included games

// Pnickies (USA)
// Seems like a region hack
static struct BurnRomInfo PnickuRomDesc[] = {
	{ "pnij36.bin",    0x020000, 0x2d4ffb2b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "pniu42.bin",    0x020000, 0x22d20227, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },

	{ "pnij09.bin",    0x020000, 0x48177b0a, BRF_GRA | CPS1_TILES },
	{ "pnij01.bin",    0x020000, 0x01a0f311, BRF_GRA | CPS1_TILES },
	{ "pnij13.bin",    0x020000, 0x406451b0, BRF_GRA | CPS1_TILES },
	{ "pnij05.bin",    0x020000, 0x8c515dc0, BRF_GRA | CPS1_TILES },
	{ "pnij26.bin",    0x020000, 0xe2af981e, BRF_GRA | CPS1_TILES },
	{ "pnij18.bin",    0x020000, 0xf17a0e56, BRF_GRA | CPS1_TILES },
	{ "pnij38.bin",    0x020000, 0xeb75bd8c, BRF_GRA | CPS1_TILES },
	{ "pnij32.bin",    0x020000, 0x84560bef, BRF_GRA | CPS1_TILES },
	{ "pnij10.bin",    0x020000, 0xc2acc171, BRF_GRA | CPS1_TILES },
	{ "pnij02.bin",    0x020000, 0x0e21fc33, BRF_GRA | CPS1_TILES },
	{ "pnij14.bin",    0x020000, 0x7fe59b19, BRF_GRA | CPS1_TILES },
	{ "pnij06.bin",    0x020000, 0x79f4bfe3, BRF_GRA | CPS1_TILES },
	{ "pnij27.bin",    0x020000, 0x83d5cb0e, BRF_GRA | CPS1_TILES },
	{ "pnij19.bin",    0x020000, 0xaf08b230, BRF_GRA | CPS1_TILES },
	{ "pnij39.bin",    0x020000, 0x70fbe579, BRF_GRA | CPS1_TILES },
	{ "pnij33.bin",    0x020000, 0x3ed2c680, BRF_GRA | CPS1_TILES },

	{ "pnij17.bin",    0x010000, 0xe86f787a, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "pnij24.bin",    0x020000, 0x5092257d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "pnij25.bin",    0x020000, 0x22109aaa, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Pnicku)
STD_ROM_FN(Pnicku)

struct BurnDriverX BurnDrvCpsPnicku = {
	"pnicku", "pnickj", NULL, NULL, "1994",
	"Pnickies (940608 USA)\0", NULL, "Compile (Capcom license)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_CAPCOM_CPS1, GBF_PUZZLE, 0,
	NULL, PnickuRomInfo, PnickuRomName, NULL, NULL, PnickjInputInfo, PnickjDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Saturday Night Slam Masters (Asia)
// Seems to be just a region hack

static struct BurnRomInfo SlammasaRomDesc[] = {
	{ "mba_23e.rom",   0x080000, 0x027e49db, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbe_24b.rom",   0x020000, 0x95d5e729, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_28b.rom",   0x020000, 0xb1c7cbcb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_25b.rom",   0x020000, 0xa50d3fd4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	
	{ "mbe_29b.rom",   0x020000, 0x08e32e56, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_21a.rom",   0x080000, 0xd5007b05, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbe_20a.rom",   0x080000, 0xaeb557b0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb_gfx01.rom",  0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb_gfx03.rom",  0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb_gfx02.rom",  0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb_gfx04.rom",  0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },

	{ "mb_qa.rom",     0x020000, 0xe21a03c4, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb_q1.bin",     0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q2.bin",     0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q3.bin",     0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q4.bin",     0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q5.bin",     0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q6.bin",     0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q7.bin",     0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q8.bin",     0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Slammasa)
STD_ROM_FN(Slammasa)

struct BurnDriverX BurnDrvCpsSlammasa = {
	"slammasa", "slammast", NULL, NULL, "1993",
	"Saturday Night Slam Masters (Slam Masters 930301 Asia)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, SlammasaRomInfo, SlammasaRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Saturday Night Slam Masters (Hispanic)
// Seems to be just a region hack
static struct BurnRomInfo SlammashRomDesc[] = {
	{ "mbh_23e.rom",   0x080000, 0xb0220f4a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbe_24b.rom",   0x020000, 0x95d5e729, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_28b.rom",   0x020000, 0xb1c7cbcb, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_25b.rom",   0x020000, 0xa50d3fd4, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },	
	{ "mbe_29b.rom",   0x020000, 0x08e32e56, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbe_21a.rom",   0x080000, 0xd5007b05, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbe_20a.rom",   0x080000, 0xaeb557b0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb_gfx01.rom",  0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb_gfx03.rom",  0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb_gfx02.rom",  0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb_gfx04.rom",  0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },

	{ "mb_qa.rom",     0x020000, 0xe21a03c4, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb_q1.bin",     0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q2.bin",     0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q3.bin",     0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q4.bin",     0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q5.bin",     0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q6.bin",     0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q7.bin",     0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q8.bin",     0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Slammash)
STD_ROM_FN(Slammash)

struct BurnDriverX BurnDrvCpsSlammash = {
	"slammash", "slammast", NULL, NULL, "1993",
	"Saturday Night Slam Masters (Slam Masters 930713 Hispanic)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, SlammashRomInfo, SlammashRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Muscle Bomber Duo (Japan E)
// Seems to be just a region hack
static struct BurnRomInfo MbombdjeRomDesc[] = {
	{ "mbd_26.bin",    0x020000, 0x72b7451c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbdje_30.rom",  0x020000, 0x61b09fb1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_27.bin",    0x020000, 0x4086f534, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_31.bin",    0x020000, 0x085f47f0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_24.bin",    0x020000, 0xc20895a5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_28.bin",    0x020000, 0x2618d5e1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_25.bin",    0x020000, 0x9bdb6b11, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_29.bin",    0x020000, 0x3f52d5e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_21.bin",    0x080000, 0x690c026a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbd_20.bin",    0x080000, 0xb8b2139b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb_gfx01.rom",  0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb_gfx03.rom",  0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb_gfx02.rom",  0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb_gfx04.rom",  0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },

	{ "mb_q.bin",      0x020000, 0xd6fa76d1, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb_q1.bin",     0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q2.bin",     0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q3.bin",     0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q4.bin",     0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q5.bin",     0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q6.bin",     0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q7.bin",     0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q8.bin",     0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Mbombdje)
STD_ROM_FN(Mbombdje)

struct BurnDriverX BurnDrvCpsMbombdje = {
	"mbombdje", "mbombrd", NULL, NULL, "1993",
	"Muscle Bomber Duo - heat up warriors (931206 Japan E)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MbombdjeRomInfo, MbombdjeRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Muscle Bomber Duo (Asia)
// Seems to be just a region hack
static struct BurnRomInfo MbombrdaRomDesc[] = {
	{ "mbd_26.bin",    0x020000, 0x72b7451c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbda_30.rom",   0x020000, 0x42eaa9da, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_27.bin",    0x020000, 0x4086f534, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_31.bin",    0x020000, 0x085f47f0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_24.bin",    0x020000, 0xc20895a5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_28.bin",    0x020000, 0x2618d5e1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_25.bin",    0x020000, 0x9bdb6b11, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_29.bin",    0x020000, 0x3f52d5e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_21.bin",    0x080000, 0x690c026a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbd_20.bin",    0x080000, 0xb8b2139b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb_gfx01.rom",  0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb_gfx03.rom",  0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb_gfx02.rom",  0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb_gfx04.rom",  0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },

	{ "mb_q.bin",      0x020000, 0xd6fa76d1, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb_q1.bin",     0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q2.bin",     0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q3.bin",     0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q4.bin",     0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q5.bin",     0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q6.bin",     0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q7.bin",     0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q8.bin",     0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Mbombrda)
STD_ROM_FN(Mbombrda)

struct BurnDriverX BurnDrvCpsMbombrda = {
	"mbombrda", "mbombrd", NULL, NULL, "1993",
	"Muscle Bomber Duo - ultimate team battle (931206 Asia)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MbombrdaRomInfo, MbombrdaRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Muscle Bomber Duo (Hispanic)
// Seems to be just a region hack
static struct BurnRomInfo MbombrdhRomDesc[] = {
	{ "mbd_26.bin",    0x020000, 0x72b7451c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbdh_30.rom",   0x020000, 0x836cea7d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_27.bin",    0x020000, 0x4086f534, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_31.bin",    0x020000, 0x085f47f0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_24.bin",    0x020000, 0xc20895a5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_28.bin",    0x020000, 0x2618d5e1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_25.bin",    0x020000, 0x9bdb6b11, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_29.bin",    0x020000, 0x3f52d5e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_21.bin",    0x080000, 0x690c026a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbd_20.bin",    0x080000, 0xb8b2139b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb_gfx01.rom",  0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb_gfx03.rom",  0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb_gfx02.rom",  0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb_gfx04.rom",  0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },

	{ "mb_q.bin",      0x020000, 0xd6fa76d1, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb_q1.bin",     0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q2.bin",     0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q3.bin",     0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q4.bin",     0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q5.bin",     0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q6.bin",     0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q7.bin",     0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q8.bin",     0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Mbombrdh)
STD_ROM_FN(Mbombrdh)

struct BurnDriverX BurnDrvCpsMbombrdh = {
	"mbombrdh", "mbombrd", NULL, NULL, "1993",
	"Muscle Bomber Duo - ultimate team battle (931206 Hispanic)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MbombrdhRomInfo, MbombrdhRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Muscle Bomber Duo (USA)
// Seems to be just a region hack
static struct BurnRomInfo MbombrduRomDesc[] = {
	{ "mbd_26.bin",    0x020000, 0x72b7451c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbdu_30.rom",   0x020000, 0x5c234403, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_27.bin",    0x020000, 0x4086f534, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_31.bin",    0x020000, 0x085f47f0, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_24.bin",    0x020000, 0xc20895a5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_28.bin",    0x020000, 0x2618d5e1, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_25.bin",    0x020000, 0x9bdb6b11, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_29.bin",    0x020000, 0x3f52d5e5, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_BYTESWAP },
	{ "mbd_21.bin",    0x080000, 0x690c026a, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "mbd_20.bin",    0x080000, 0xb8b2139b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "mb_gfx01.rom",  0x080000, 0x41468e06, BRF_GRA | CPS1_TILES },
	{ "mb_gfx03.rom",  0x080000, 0xf453aa9e, BRF_GRA | CPS1_TILES },
	{ "mb_gfx02.rom",  0x080000, 0x2ffbfea8, BRF_GRA | CPS1_TILES },
	{ "mb_gfx04.rom",  0x080000, 0x1eb9841d, BRF_GRA | CPS1_TILES },
	{ "mb_05.bin",     0x080000, 0x506b9dc9, BRF_GRA | CPS1_TILES },
	{ "mb_07.bin",     0x080000, 0xaff8c2fb, BRF_GRA | CPS1_TILES },
	{ "mb_06.bin",     0x080000, 0xb76c70e9, BRF_GRA | CPS1_TILES },
	{ "mb_08.bin",     0x080000, 0xe60c9556, BRF_GRA | CPS1_TILES },
	{ "mb_10.bin",     0x080000, 0x97976ff5, BRF_GRA | CPS1_TILES },
	{ "mb_12.bin",     0x080000, 0xb350a840, BRF_GRA | CPS1_TILES },
	{ "mb_11.bin",     0x080000, 0x8fb94743, BRF_GRA | CPS1_TILES },
	{ "mb_13.bin",     0x080000, 0xda810d5f, BRF_GRA | CPS1_TILES },

	{ "mb_q.bin",      0x020000, 0xd6fa76d1, BRF_ESS | BRF_PRG | CPS1_Z80_PROGRAM },

	{ "mb_q1.bin",     0x080000, 0x0630c3ce, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q2.bin",     0x080000, 0x354f9c21, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q3.bin",     0x080000, 0x7838487c, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q4.bin",     0x080000, 0xab66e087, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q5.bin",     0x080000, 0xc789fef2, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q6.bin",     0x080000, 0xecb81b61, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q7.bin",     0x080000, 0x041e49ba, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "mb_q8.bin",     0x080000, 0x59fe702a, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Mbombrdu)
STD_ROM_FN(Mbombrdu)

struct BurnDriverX BurnDrvCpsMbombrdu = {
	"mbombrdu", "mbombrd", NULL, NULL, "1993",
	"Muscle Bomber Duo - ultimate team battle (931206 USA)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 4, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, 0,
	NULL, MbombrduRomInfo, MbombrduRomName, NULL, NULL, SlammastInputInfo, SlammastDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

static struct BurnRomInfo Sf2cebrRomDesc[] = {
	{ "sf2cebr.23",    0x080000, 0x74e848ee, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2cebr.22",    0x080000, 0xc3c49626, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.bin",   0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92br_10.bin",  0x080000, 0xb3e1dd5f, BRF_GRA | CPS1_TILES },
	{ "s92br_11.bin",  0x080000, 0xf13af812, BRF_GRA | CPS1_TILES },
	{ "s92br_12.bin",  0x080000, 0x10ce42af, BRF_GRA | CPS1_TILES },
	{ "s92br_13.bin",  0x080000, 0x32cf5af3, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2cebr)
STD_ROM_FN(Sf2cebr)

struct BurnDriverX BurnDrvCpsSf2cebr = {
	"sf2cebr", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (bootleg, Brazil)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2cebrRomInfo, Sf2cebrRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

static struct BurnRomInfo Sf2cehRomDesc[] = {
	{ "sf2ceh.23",     0x080000, 0x25dc14c8, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sf2ceh.22",     0x080000, 0x1c9dd91c, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "s92_21a.bin",   0x080000, 0x925a7877, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "s92_01.bin",    0x080000, 0x03b0d852, BRF_GRA | CPS1_TILES },
	{ "s92_02.bin",    0x080000, 0x840289ec, BRF_GRA | CPS1_TILES },
	{ "s92_03.bin",    0x080000, 0xcdb5f027, BRF_GRA | CPS1_TILES },
	{ "s92_04.bin",    0x080000, 0xe2799472, BRF_GRA | CPS1_TILES },
	{ "s92_05.bin",    0x080000, 0xba8a2761, BRF_GRA | CPS1_TILES },
	{ "s92_06.bin",    0x080000, 0xe584bfb5, BRF_GRA | CPS1_TILES },
	{ "s92_07.bin",    0x080000, 0x21e3f87d, BRF_GRA | CPS1_TILES },
	{ "s92_08.bin",    0x080000, 0xbefc47df, BRF_GRA | CPS1_TILES },
	{ "s92_10.bin",    0x080000, 0x960687d5, BRF_GRA | CPS1_TILES },
	{ "s92_11.bin",    0x080000, 0x978ecd18, BRF_GRA | CPS1_TILES },
	{ "s92_12.bin",    0x080000, 0xd6ec9a0a, BRF_GRA | CPS1_TILES },
	{ "s92_13.bin",    0x080000, 0xed2c67f6, BRF_GRA | CPS1_TILES },

	{ "s92_09.bin",    0x010000, 0x08f6b60e, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "s92_18.bin",    0x020000, 0x7f162009, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "s92_19.bin",    0x020000, 0xbeade53f, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sf2ceh)
STD_ROM_FN(Sf2ceh)

struct BurnDriverX BurnDrvCpsSf2ceh = {
	"sf2ceh", "sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - champion edition (bootleg, Hispanic)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_CAPCOM_CPS1, GBF_VSFIGHT, FBF_SF,
	NULL, Sf2cehRomInfo, Sf2cehRomName, NULL, NULL, Sf2InputInfo, Sf2DIPInfo,
	Sf2ceInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Warriors of Fate (French Translation)
// Very similar to wof
static struct BurnRomInfo WoffrRomDesc[] = {
	{ "tk2e_23bfr.rom",0x080000, 0xe006e81e, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "tk2e_22b.rom",  0x080000, 0x479b3f24, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "tk2_gfx1.rom",  0x080000, 0x0d9cb9bf, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx3.rom",  0x080000, 0x45227027, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx2.rom",  0x080000, 0xc5ca2460, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx4.rom",  0x080000, 0xe349551c, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx5fr.rom",0x080000, 0x1b6bc2b2, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx7fr.rom",0x080000, 0x3f1e5334, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx6fr.rom",0x080000, 0x28c9d96f, BRF_GRA | CPS1_TILES },
	{ "tk2_gfx8fr.rom",0x080000, 0xb3b9a29e, BRF_GRA | CPS1_TILES },

	{ "tk2_qa.rom",    0x020000, 0xc9183a0d, BRF_PRG | CPS1_Z80_PROGRAM },

	{ "tk2_q1.rom",    0x080000, 0x611268cf, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q2.rom",    0x080000, 0x20f55ca9, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q3.rom",    0x080000, 0xbfcf6f52, BRF_SND | CPS1_QSOUND_SAMPLES },
	{ "tk2_q4.rom",    0x080000, 0x36642e88, BRF_SND | CPS1_QSOUND_SAMPLES },
};

STD_ROM_PICK(Woffr)
STD_ROM_FN(Woffr)

struct BurnDriverX BurnDrvCpsWoffr = {
	"woffr", "wof", NULL, NULL, "1992",
	"Warriors of Fate (French Translation)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 3, HARDWARE_CAPCOM_CPS1_QSOUND, GBF_SCRFIGHT, 0,
	NULL, WoffrRomInfo, WoffrRomName, NULL, NULL, WofInputInfo, WofDIPInfo,
	TwelveMhzInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// CPS Changer Region Hacks (not included)

// Street Fighter Alpha (CPS Changer, 950727 Publicity US)
static struct BurnRomInfo SfachRomDesc[] = {
	{ "sfach23",       0x080000, 0x02a1a853, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza22",        0x080000, 0x8d9b2480, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfzch21",       0x080000, 0x5435225d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza20",        0x080000, 0x806e8f38, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sfz_01.3a",     0x080000, 0x0dd53e62, BRF_GRA | CPS1_TILES },
	{ "sfz_02.4a",     0x080000, 0x94c31e3f, BRF_GRA | CPS1_TILES },
	{ "sfz_03.5a",     0x080000, 0x9584ac85, BRF_GRA | CPS1_TILES },
	{ "sfz_04.6a",     0x080000, 0xb983624c, BRF_GRA | CPS1_TILES },
	{ "sfz_05.7a",     0x080000, 0x2b47b645, BRF_GRA | CPS1_TILES },
	{ "sfz_06.8a",     0x080000, 0x74fd9fb1, BRF_GRA | CPS1_TILES },
	{ "sfz_07.9a",     0x080000, 0xbb2c734d, BRF_GRA | CPS1_TILES },
	{ "sfz_08.10a",    0x080000, 0x454f7868, BRF_GRA | CPS1_TILES },
	{ "sfz_10.3c",     0x080000, 0x2a7d675e, BRF_GRA | CPS1_TILES },
	{ "sfz_11.4c",     0x080000, 0xe35546c8, BRF_GRA | CPS1_TILES },
	{ "sfz_12.5c",     0x080000, 0xf122693a, BRF_GRA | CPS1_TILES },
	{ "sfz_13.6c",     0x080000, 0x7cf942c8, BRF_GRA | CPS1_TILES },
	{ "sfz_14.7c",     0x080000, 0x09038c81, BRF_GRA | CPS1_TILES },
	{ "sfz_15.8c",     0x080000, 0x1aa17391, BRF_GRA | CPS1_TILES },
	{ "sfz_16.9c",     0x080000, 0x19a5abd6, BRF_GRA | CPS1_TILES },
	{ "sfz_17.10c",    0x080000, 0x248b3b73, BRF_GRA | CPS1_TILES },
	
	{ "sfz_09.12a",    0x010000, 0xc772628b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "sfz_18.11c",    0x020000, 0x61022b2d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sfz_19.12c",    0x020000, 0x3b5886d5, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sfach)
STD_ROM_FN(Sfach)

struct BurnDriverX BurnDrvCpsSfach = {
	"sfach", "sfzch", NULL, NULL, "1995",
	"Street Fighter Alpha (CPS Changer, 950727 Publicity US)\0", NULL, "Capcom", "CPS Changer",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPSCHANGER, GBF_VSFIGHT, FBF_SF,
	NULL, SfachRomInfo, SfachRomName, NULL, NULL, SfzchInputInfo, NULL,
	SfzchInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Street Fighter Zero (CPS Changer, 950925 Asia)
static struct BurnRomInfo SfzachRomDesc[] = {
	{ "sfzach23",      0x080000, 0x2562108b, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza22",        0x080000, 0x8d9b2480, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfzch21",       0x080000, 0x5435225d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza20",        0x080000, 0x806e8f38, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sfz_01.3a",     0x080000, 0x0dd53e62, BRF_GRA | CPS1_TILES },
	{ "sfz_02.4a",     0x080000, 0x94c31e3f, BRF_GRA | CPS1_TILES },
	{ "sfz_03.5a",     0x080000, 0x9584ac85, BRF_GRA | CPS1_TILES },
	{ "sfz_04.6a",     0x080000, 0xb983624c, BRF_GRA | CPS1_TILES },
	{ "sfz_05.7a",     0x080000, 0x2b47b645, BRF_GRA | CPS1_TILES },
	{ "sfz_06.8a",     0x080000, 0x74fd9fb1, BRF_GRA | CPS1_TILES },
	{ "sfz_07.9a",     0x080000, 0xbb2c734d, BRF_GRA | CPS1_TILES },
	{ "sfz_08.10a",    0x080000, 0x454f7868, BRF_GRA | CPS1_TILES },
	{ "sfz_10.3c",     0x080000, 0x2a7d675e, BRF_GRA | CPS1_TILES },
	{ "sfz_11.4c",     0x080000, 0xe35546c8, BRF_GRA | CPS1_TILES },
	{ "sfz_12.5c",     0x080000, 0xf122693a, BRF_GRA | CPS1_TILES },
	{ "sfz_13.6c",     0x080000, 0x7cf942c8, BRF_GRA | CPS1_TILES },
	{ "sfz_14.7c",     0x080000, 0x09038c81, BRF_GRA | CPS1_TILES },
	{ "sfz_15.8c",     0x080000, 0x1aa17391, BRF_GRA | CPS1_TILES },
	{ "sfz_16.9c",     0x080000, 0x19a5abd6, BRF_GRA | CPS1_TILES },
	{ "sfz_17.10c",    0x080000, 0x248b3b73, BRF_GRA | CPS1_TILES },
	
	{ "sfz_09.12a",    0x010000, 0xc772628b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "sfz_18.11c",    0x020000, 0x61022b2d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sfz_19.12c",    0x020000, 0x3b5886d5, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sfzach)
STD_ROM_FN(Sfzach)

struct BurnDriverX BurnDrvCpsSfzach = {
	"sfzach", "sfzch", NULL, NULL, "1995",
	"Street Fighter Zero (CPS Changer, 950925 Asia)\0", NULL, "Capcom", "CPS Changer",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPSCHANGER, GBF_VSFIGHT, FBF_SF,
	NULL, SfzachRomInfo, SfzachRomName, NULL, NULL, SfzchInputInfo, NULL,
	SfzchInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Street Fighter Zero (CPS Changer, 950727 Brazil)
static struct BurnRomInfo SfzbchRomDesc[] = {
	{ "sfbch23",       0x080000, 0x53699f68, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza22",        0x080000, 0x8d9b2480, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfzch21",       0x080000, 0x5435225d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza20",        0x080000, 0x806e8f38, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sfz_01.3a",     0x080000, 0x0dd53e62, BRF_GRA | CPS1_TILES },
	{ "sfz_02.4a",     0x080000, 0x94c31e3f, BRF_GRA | CPS1_TILES },
	{ "sfz_03.5a",     0x080000, 0x9584ac85, BRF_GRA | CPS1_TILES },
	{ "sfz_04.6a",     0x080000, 0xb983624c, BRF_GRA | CPS1_TILES },
	{ "sfz_05.7a",     0x080000, 0x2b47b645, BRF_GRA | CPS1_TILES },
	{ "sfz_06.8a",     0x080000, 0x74fd9fb1, BRF_GRA | CPS1_TILES },
	{ "sfz_07.9a",     0x080000, 0xbb2c734d, BRF_GRA | CPS1_TILES },
	{ "sfz_08.10a",    0x080000, 0x454f7868, BRF_GRA | CPS1_TILES },
	{ "sfz_10.3c",     0x080000, 0x2a7d675e, BRF_GRA | CPS1_TILES },
	{ "sfz_11.4c",     0x080000, 0xe35546c8, BRF_GRA | CPS1_TILES },
	{ "sfz_12.5c",     0x080000, 0xf122693a, BRF_GRA | CPS1_TILES },
	{ "sfz_13.6c",     0x080000, 0x7cf942c8, BRF_GRA | CPS1_TILES },
	{ "sfz_14.7c",     0x080000, 0x09038c81, BRF_GRA | CPS1_TILES },
	{ "sfz_15.8c",     0x080000, 0x1aa17391, BRF_GRA | CPS1_TILES },
	{ "sfz_16.9c",     0x080000, 0x19a5abd6, BRF_GRA | CPS1_TILES },
	{ "sfz_17.10c",    0x080000, 0x248b3b73, BRF_GRA | CPS1_TILES },
	
	{ "sfz_09.12a",    0x010000, 0xc772628b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "sfz_18.11c",    0x020000, 0x61022b2d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sfz_19.12c",    0x020000, 0x3b5886d5, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sfzbch)
STD_ROM_FN(Sfzbch)

struct BurnDriverX BurnDrvCpsSfzbch = {
	"sfzbch", "sfzch", NULL, NULL, "1995",
	"Street Fighter Zero (CPS Changer, 950727 Brazil)\0", NULL, "Capcom", "CPS Changer",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPSCHANGER, GBF_VSFIGHT, FBF_SF,
	NULL, SfzbchRomInfo, SfzbchRomName, NULL, NULL, SfzchInputInfo, NULL,
	SfzchInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Street Fighter Zero (CPS Changer, 950727 Euro)
static struct BurnRomInfo SfzechRomDesc[] = {
	{ "sfzech23",      0x080000, 0x5ec7058f, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza22",        0x080000, 0x8d9b2480, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfzch21",       0x080000, 0x5435225d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza20",        0x080000, 0x806e8f38, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sfz_01.3a",     0x080000, 0x0dd53e62, BRF_GRA | CPS1_TILES },
	{ "sfz_02.4a",     0x080000, 0x94c31e3f, BRF_GRA | CPS1_TILES },
	{ "sfz_03.5a",     0x080000, 0x9584ac85, BRF_GRA | CPS1_TILES },
	{ "sfz_04.6a",     0x080000, 0xb983624c, BRF_GRA | CPS1_TILES },
	{ "sfz_05.7a",     0x080000, 0x2b47b645, BRF_GRA | CPS1_TILES },
	{ "sfz_06.8a",     0x080000, 0x74fd9fb1, BRF_GRA | CPS1_TILES },
	{ "sfz_07.9a",     0x080000, 0xbb2c734d, BRF_GRA | CPS1_TILES },
	{ "sfz_08.10a",    0x080000, 0x454f7868, BRF_GRA | CPS1_TILES },
	{ "sfz_10.3c",     0x080000, 0x2a7d675e, BRF_GRA | CPS1_TILES },
	{ "sfz_11.4c",     0x080000, 0xe35546c8, BRF_GRA | CPS1_TILES },
	{ "sfz_12.5c",     0x080000, 0xf122693a, BRF_GRA | CPS1_TILES },
	{ "sfz_13.6c",     0x080000, 0x7cf942c8, BRF_GRA | CPS1_TILES },
	{ "sfz_14.7c",     0x080000, 0x09038c81, BRF_GRA | CPS1_TILES },
	{ "sfz_15.8c",     0x080000, 0x1aa17391, BRF_GRA | CPS1_TILES },
	{ "sfz_16.9c",     0x080000, 0x19a5abd6, BRF_GRA | CPS1_TILES },
	{ "sfz_17.10c",    0x080000, 0x248b3b73, BRF_GRA | CPS1_TILES },
	
	{ "sfz_09.12a",    0x010000, 0xc772628b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "sfz_18.11c",    0x020000, 0x61022b2d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sfz_19.12c",    0x020000, 0x3b5886d5, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sfzech)
STD_ROM_FN(Sfzech)

struct BurnDriverX BurnDrvCpsSfzech = {
	"sfzech", "sfzch", NULL, NULL, "1995",
	"Street Fighter Zero (CPS Changer, 950727 Euro)\0", NULL, "Capcom", "CPS Changer",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPSCHANGER, GBF_VSFIGHT, FBF_SF,
	NULL, SfzechRomInfo, SfzechRomName, NULL, NULL, SfzchInputInfo, NULL,
	SfzchInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

// Street Fighter Zero (CPS Changer, 950727 Hispanic)
static struct BurnRomInfo SfzhchRomDesc[] = {
	{ "sfzhch23",      0x080000, 0x3683cce7, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza22",        0x080000, 0x8d9b2480, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfzch21",       0x080000, 0x5435225d, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },
	{ "sfza20",        0x080000, 0x806e8f38, BRF_ESS | BRF_PRG | CPS1_68K_PROGRAM_NO_BYTESWAP },

	{ "sfz_01.3a",     0x080000, 0x0dd53e62, BRF_GRA | CPS1_TILES },
	{ "sfz_02.4a",     0x080000, 0x94c31e3f, BRF_GRA | CPS1_TILES },
	{ "sfz_03.5a",     0x080000, 0x9584ac85, BRF_GRA | CPS1_TILES },
	{ "sfz_04.6a",     0x080000, 0xb983624c, BRF_GRA | CPS1_TILES },
	{ "sfz_05.7a",     0x080000, 0x2b47b645, BRF_GRA | CPS1_TILES },
	{ "sfz_06.8a",     0x080000, 0x74fd9fb1, BRF_GRA | CPS1_TILES },
	{ "sfz_07.9a",     0x080000, 0xbb2c734d, BRF_GRA | CPS1_TILES },
	{ "sfz_08.10a",    0x080000, 0x454f7868, BRF_GRA | CPS1_TILES },
	{ "sfz_10.3c",     0x080000, 0x2a7d675e, BRF_GRA | CPS1_TILES },
	{ "sfz_11.4c",     0x080000, 0xe35546c8, BRF_GRA | CPS1_TILES },
	{ "sfz_12.5c",     0x080000, 0xf122693a, BRF_GRA | CPS1_TILES },
	{ "sfz_13.6c",     0x080000, 0x7cf942c8, BRF_GRA | CPS1_TILES },
	{ "sfz_14.7c",     0x080000, 0x09038c81, BRF_GRA | CPS1_TILES },
	{ "sfz_15.8c",     0x080000, 0x1aa17391, BRF_GRA | CPS1_TILES },
	{ "sfz_16.9c",     0x080000, 0x19a5abd6, BRF_GRA | CPS1_TILES },
	{ "sfz_17.10c",    0x080000, 0x248b3b73, BRF_GRA | CPS1_TILES },
	
	{ "sfz_09.12a",    0x010000, 0xc772628b, BRF_PRG | CPS1_Z80_PROGRAM },
	
	{ "sfz_18.11c",    0x020000, 0x61022b2d, BRF_SND | CPS1_OKIM6295_SAMPLES },
	{ "sfz_19.12c",    0x020000, 0x3b5886d5, BRF_SND | CPS1_OKIM6295_SAMPLES },
};

STD_ROM_PICK(Sfzhch)
STD_ROM_FN(Sfzhch)

struct BurnDriverX BurnDrvCpsSfzhch = {
	"sfzhch", "sfzch", NULL, NULL, "1995",
	"Street Fighter Zero (CPS Changer, 950727 Hispanic)\0", NULL, "Capcom", "CPS Changer",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPSCHANGER, GBF_VSFIGHT, FBF_SF,
	NULL, SfzhchRomInfo, SfzhchRomName, NULL, NULL, SfzchInputInfo, NULL,
	SfzchInit, DrvExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 0x1000, 384, 224, 4, 3
};

#undef CPS1_68K_PROGRAM_BYTESWAP
#undef CPS1_68K_PROGRAM_NO_BYTESWAP
#undef CPS1_Z80_PROGRAM
#undef CPS1_TILES
#undef CPS1_OKIM6295_SAMPLES
#undef CPS1_QSOUND_SAMPLES
#undef CPS1_PIC
