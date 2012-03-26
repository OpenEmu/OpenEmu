#include "gal.h"

static INT32 ScrambleInit();

// Input definitions (alphabetical)
#define A(a, b, c, d) {a, b, (UINT8*)(c), d}
static struct BurnInputInfo Ad2083InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort1 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort1 + 2, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 0, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort1 + 4, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
};

STDINPUTINFO(Ad2083)

static struct BurnInputInfo AmidarInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
};

STDINPUTINFO(Amidar)

static struct BurnInputInfo AnteaterInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort2 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 fire 1" },

	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 2, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 3, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 6, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Anteater)

static struct BurnInputInfo AnteatergInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort2 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort2 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort2 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort2 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort2 + 3, "p1 fire 1" },

	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort2 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Anteaterg)

static struct BurnInputInfo AnteaterukInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },

	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Anteateruk)

static struct BurnInputInfo AtlantisInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 2, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Atlantis)

static struct BurnInputInfo AzurianInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 1, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 0, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 3, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 2, "p1 right"  },	
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 5, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 3, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },	
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort0 + 6, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , &GalFakeDip      , "dip"       },
};

STDINPUTINFO(Azurian)

static struct BurnInputInfo BagmanmcInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort0 + 2, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 0, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 5, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 3, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 6, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Bagmanmc)

static struct BurnInputInfo Batman2InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 fire 2" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort0 + 6, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Batman2)

static struct BurnInputInfo BlkholeInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Blkhole)

static struct BurnInputInfo BongoInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 5, "p1 up"     },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 5, "p2 up"     },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
};

STDINPUTINFO(Bongo)

static struct BurnInputInfo CalipsoInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },

	{"P1 Up"             , BIT_DIGITAL   , GalInputPort0 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , GalInputPort0 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , GalInputPort2 + 0, "p1 fire 1" },
	
	{"P2 Up"             , BIT_DIGITAL   , GalInputPort1 + 2, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , GalInputPort1 + 3, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , GalInputPort0 + 0, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Calipso)

static struct BurnInputInfo CheckmanjInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort2 + 1, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort2 + 0, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort0 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Checkmanj)

static struct BurnInputInfo CheckmanInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 coin"   },
	{"Start 1/P1 Fire 1" , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 0, "p2 coin"   },
	{"Start 2/P1 Fire 2" , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 6, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort0 + 4, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Checkman)

static struct BurnInputInfo CkonggInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Ckongg)

static struct BurnInputInfo CkongsInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Ckongs)

static struct BurnInputInfo DambustrInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort0 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 0, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort0 + 2, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort1 + 5, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort1 + 4, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort1 + 3, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort1 + 2, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 0, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort1 + 1, "p1 fire 2" },
	{"Fire 3"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Dambustr)

static struct BurnInputInfo DarkplntInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },

	{"Left"              , BIT_DIGITAL   , GalInputPort3 + 0, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort3 + 1, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 2, "p1 fire 2" },
	{"Fire 3"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Darkplnt)

static struct BurnInputInfo DevilfsgInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 6, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Devilfsg)

static struct BurnInputInfo DevilfshInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Devilfsh)

static struct BurnInputInfo DingoInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort2 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort0 + 5, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Dingo)

static struct BurnInputInfo DkongjrmInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 5, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Dkongjrm)

static struct BurnInputInfo DrivfrcgInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 0, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort1 + 1, "p1 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
};

STDINPUTINFO(Drivfrcg)

static struct BurnInputInfo ExplorerInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 0, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
};

STDINPUTINFO(Explorer)

static struct BurnInputInfo FantastcInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Fantastc)

static struct BurnInputInfo Fourin1InputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire"              , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },

	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"GM Dip 1"          , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
	{"SC Dip 2"          , BIT_DIPSWITCH , GalDip + 4       , "dip"       },
	{"GX Dip 3"          , BIT_DIPSWITCH , GalDip + 5       , "dip"       },
	{"GC Dip 4"          , BIT_DIPSWITCH , GalDip + 6       , "dip"       },
};

STDINPUTINFO(Fourin1)

static struct BurnInputInfo FroggInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 7, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Frogg)

static struct BurnInputInfo FroggerInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Frogger)

static struct BurnInputInfo FroggermcInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 7, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Froggermc)

static struct BurnInputInfo GalaxianInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 7, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Galaxian)

static struct BurnInputInfo GmgalaxInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"GM Dip 1"          , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"GM Dip 2"          , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"GM Dip 3"          , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"GX Dip 1"          , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
	{"GX Dip 2"          , BIT_DIPSWITCH , GalDip + 4       , "dip"       },
	{"GX Dip 3"          , BIT_DIPSWITCH , GalDip + 5       , "dip"       },
	{"Game Select Switch", BIT_DIPSWITCH , &GalFakeDip      , "dip"       },
};

STDINPUTINFO(Gmgalax)

static struct BurnInputInfo HotshockInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort1 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort1 + 4, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 0, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort1 + 5, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
};

STDINPUTINFO(Hotshock)

static struct BurnInputInfo HunchbkgInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 0, "p1 fire 1" },
	
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Hunchbkg)

static struct BurnInputInfo HunchbksInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 2, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Hunchbks)

static struct BurnInputInfo HustlerInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 1, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Hustler)

static struct BurnInputInfo JumpbugInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort1 + 5, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 7, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Jumpbug)

static struct BurnInputInfo KingballInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 7, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Fake Dip"          , BIT_DIPSWITCH , &GalFakeDip      , "dip"       },
};

STDINPUTINFO(Kingball)

static struct BurnInputInfo CkongmcInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 0, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 1, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Ckongmc)

static struct BurnInputInfo KongInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 coin"   },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort1 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 0, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort1 + 1, "p2 fire 1" },
	
	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Kong)

static struct BurnInputInfo LadybuggInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort2 + 5, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Ladybugg)

static struct BurnInputInfo LeversInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort1 + 5, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	
	{"P2 Up"             , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , GalInputPort1 + 7, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Levers)

static struct BurnInputInfo LosttombInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort0 + 1, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort0 + 0, "p2 start"  },

	{"Left Up"           , BIT_DIGITAL   , GalInputPort0 + 2, "p1 up"     },
	{"Left Down"         , BIT_DIGITAL   , GalInputPort0 + 3, "p1 down"   },
	{"left Left"         , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Left Right"        , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 6, "p1 fire 1" },
	
	{"Right Up"          , BIT_DIGITAL   , GalInputPort1 + 2, "p2 up"     },
	{"Right Down"        , BIT_DIGITAL   , GalInputPort1 + 3, "p2 down"   },
	{"Right Left"        , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right Right"       , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Losttomb)

static struct BurnInputInfo LuctodayInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Luctoday)

static struct BurnInputInfo MarsInputList[] =
{
	{"Coin 1"             , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"            , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"             , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"            , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"P1 Left Stick Up"   , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"P1 Left Stick Down" , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"P1 Left Stick Left" , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"P1 Left Stick Right", BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"P1 Rght Stick Up"   , BIT_DIGITAL   , GalInputPort2 + 7, "p3 up"     },
	{"P1 Rght Stick Down" , BIT_DIGITAL   , GalInputPort2 + 5, "p3 down"   },
	{"P1 Rght Stick Left" , BIT_DIGITAL   , GalInputPort0 + 3, "p3 left"   },
	{"P1 Rght Stick Right", BIT_DIGITAL   , GalInputPort0 + 2, "p3 right"  },
	
	{"P2 Left Stick Up"   , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"P2 Left Stick Down" , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"P2 Left Stick Left" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"P2 Left Stick Right", BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"P2 Rght Stick Up"   , BIT_DIGITAL   , GalInputPort3 + 7, "p4 up"     },
	{"P2 Rght Stick Down" , BIT_DIGITAL   , GalInputPort3 + 5, "p4 down"   },
	{"P2 Rght Stick Left" , BIT_DIGITAL   , GalInputPort1 + 3, "p4 left"   },
	{"P2 Rght Stick Right", BIT_DIGITAL   , GalInputPort1 + 2, "p4 right"  },

	{"Reset"              , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"            , BIT_DIGITAL   , GalInputPort0 + 1, "service"   },
	{"Dip 1"              , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"              , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"              , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"              , BIT_DIPSWITCH , &GalFakeDip      , "dip"       },
};

STDINPUTINFO(Mars)

static struct BurnInputInfo MoonwarInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 2, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 3, "p2 start"  },

	{"Hyper Flip / Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 2" },
	
	A("Dial"             , BIT_ANALOG_REL, &GalAnalogPort0  , "p1 z-axis" ),
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 7, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort1 + 6, "p1 fire 2" },
	{"Fire 3"            , BIT_DIGITAL   , GalInputPort1 + 5, "p1 fire 3" },

	A("Dial (Cocktail)"  , BIT_ANALOG_REL, &GalAnalogPort1  , "p2 z-axis" ),
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort2 + 0, "p2 fire 1" },
	{"Fire 3 (Cocktail)" , BIT_DIGITAL   , GalInputPort0 + 5, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Moonwar)

static struct BurnInputInfo MrkougarInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Mrkougar)

static struct BurnInputInfo MshuttleInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 3, "p1 down"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 5, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 2, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 3, "p2 down"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 5, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Mshuttle)

static struct BurnInputInfo OmegaInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Omega)

static struct BurnInputInfo OrbitronInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 7, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Orbitron)

static struct BurnInputInfo Ozon1InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"P1 Left"           , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	
	{"P2 Left"           , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Ozon1)

static struct BurnInputInfo PacmanblInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },	
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 6, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },	

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Pacmanbl)

static struct BurnInputInfo Phoenxp2InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },	
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 6, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Phoenxp2)

static struct BurnInputInfo PicsesInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 0, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Picses)

static struct BurnInputInfo PorterInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 0, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 7, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 1, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Porter)

static struct BurnInputInfo RacknrolInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 5, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Racknrol)

static struct BurnInputInfo RescueInputList[] =
{
	{"Coin 1"             , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"            , BIT_DIGITAL   , GalInputPort2 + 0, "p1 start"  },
	{"Coin 2"             , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"            , BIT_DIGITAL   , GalInputPort2 + 6, "p2 start"  },

	{"P1 Left Stick Up"   , BIT_DIGITAL   , GalInputPort0 + 2, "p1 up"     },
	{"P1 Left Stick Down" , BIT_DIGITAL   , GalInputPort0 + 3, "p1 down"   },
	{"P1 Left Stick Left" , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"P1 Left Stick Right", BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"P1 Rght Stick Up"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 up"     },
	{"P1 Rght Stick Down" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 down"   },
	{"P1 Rght Stick Left" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"P1 Rght Stick Right", BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1"             , BIT_DIGITAL   , GalInputPort0 + 0, "p2 fire 1" },

	{"Reset"              , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"              , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"              , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"              , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"              , BIT_DIPSWITCH , &GalFakeDip      , "dip"       },
};

STDINPUTINFO(Rescue)

static struct BurnInputInfo RockclimInputList[] =
{
	{"Coin 1"             , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"            , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"             , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"            , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"P1 Left Stick Up"   , BIT_DIGITAL   , GalInputPort1 + 4, "p1 up"     },
	{"P1 Left Stick Down" , BIT_DIGITAL   , GalInputPort1 + 5, "p1 down"   },
	{"P1 Left Stick Left" , BIT_DIGITAL   , GalInputPort1 + 2, "p1 left"   },
	{"P1 Left Stick Right", BIT_DIGITAL   , GalInputPort1 + 3, "p1 right"  },
	{"P1 Rght Stick Up"   , BIT_DIGITAL   , GalInputPort0 + 4, "p2 up"     },
	{"P1 Rght Stick Down" , BIT_DIGITAL   , GalInputPort0 + 5, "p2 down"   },
	{"P1 Rght Stick Left" , BIT_DIGITAL   , GalInputPort0 + 2, "p2 left"   },
	{"P1 Rght Stick Right", BIT_DIGITAL   , GalInputPort0 + 3, "p2 right"  },

	{"Reset"              , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"              , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"              , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"              , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"              , BIT_DIPSWITCH , &GalFakeDip      , "dip"       },
};

STDINPUTINFO(Rockclim)

static struct BurnInputInfo ScorpionmcInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 coin"   },
	{"Start 1 / Fire 1"  , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2 / Fire 2"  , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort0 + 5, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 0, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
};

STDINPUTINFO(Scorpionmc)

static struct BurnInputInfo AracnisInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p1 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 0, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH , GalDip + 3       , "dip"       },
};

STDINPUTINFO(Aracnis)

static struct BurnInputInfo Scramb2InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 2, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 2, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Scramb2)

static struct BurnInputInfo ScramblbInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 7, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 1, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Scramblb)

static struct BurnInputInfo ScrambleInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 2, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Scramble)

static struct BurnInputInfo SfxInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , GalInputPort1 + 2, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Sfx)

static struct BurnInputInfo SkybaseInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort2 + 6, "p1 fire 2" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort2 + 7, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Skybase)

static struct BurnInputInfo SmooncrsInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Smooncrs)

static struct BurnInputInfo SpdcoinInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort0 + 1, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort0 + 3, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Spdcoin)

static struct BurnInputInfo StratgyxInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 2, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Stratgyx)

static struct BurnInputInfo StreakngInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },	
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },	

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Streakng)

static struct BurnInputInfo SuperbonInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort0 + 1, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort0 + 0, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort1 + 3, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort1 + 6, "p1 fire 2" },
	
	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Superbon)

static struct BurnInputInfo TazmaniaInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort2 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 2, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 3, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 7, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 6, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Tazmania)

static struct BurnInputInfo TazzmangInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1 / Fire 2"  , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 7, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Tazzmang)

static struct BurnInputInfo TdpgalInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 7, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Tdpgal)

static struct BurnInputInfo TheendInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p1 fire 2" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 2, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Theend)

static struct BurnInputInfo ThepitmInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 7, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 5, "p1 fire 1" },	
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 4, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 6, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 5, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Thepitm)

static struct BurnInputInfo TurtlesInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort2 + 4, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort2 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 5, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 4, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 3, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 0, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort2 + 0, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 4, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 3, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , GalInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Turtles)

static struct BurnInputInfo WarofbugInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , GalInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 7, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },	
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort0 + 1, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 5, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },	
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Warofbug)

static struct BurnInputInfo ZigzagInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , GalInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , GalInputPort1 + 0, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL   , GalInputPort1 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL   , GalInputPort0 + 5, "p1 up"     },
	{"Down"              , BIT_DIGITAL   , GalInputPort0 + 6, "p1 down"   },
	{"Left"              , BIT_DIGITAL   , GalInputPort0 + 2, "p1 left"   },
	{"Right"             , BIT_DIGITAL   , GalInputPort0 + 3, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL   , GalInputPort0 + 4, "p1 fire 1" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL   , GalInputPort1 + 5, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL   , GalInputPort0 + 7, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL   , GalInputPort1 + 2, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL   , GalInputPort1 + 3, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL   , GalInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &GalReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , GalDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , GalDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH , GalDip + 2       , "dip"       },
};

STDINPUTINFO(Zigzag)

#undef A

// Dip Switch defintions (alphabetical)
static struct BurnDIPInfo Ad2083DIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x10, NULL                     },
	
	// Dip 1
	
	// Dip 2
	
	// Dip 3
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x0d, 0x01, 0x0f, 0x08, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x0f, 0x09, "2 Coins 2 Plays"        },
	{0x0d, 0x01, 0x0f, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0x0f, 0x0a, "2 Coins 3 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0b, "2 Coins 4 Plays"        },
	{0x0d, 0x01, 0x0f, 0x01, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0c, "2 Coins 5 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0d, "2 Coins 6 Plays"        },
	{0x0d, 0x01, 0x0f, 0x02, "1 Coin  3 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0e, "2 Coins 7 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0f, "2 Coins 8 Plays"        },
	{0x0d, 0x01, 0x0f, 0x03, "1 Coin  4 Plays"        },
	{0x0d, 0x01, 0x0f, 0x04, "1 Coin  5 Plays"        },
	{0x0d, 0x01, 0x0f, 0x05, "1 Coin  6 Plays"        },
	{0x0d, 0x01, 0x0f, 0x06, "1 Coin  7 Plays"        },
	{0x0d, 0x01, 0x0f, 0x07, "1 Coin  8 Plays"        },
	
	{0   , 0xfe, 0   , 16  , "Coin B"                 },
	{0x0d, 0x01, 0xf0, 0x80, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0xf0, 0x90, "2 Coins 2 Plays"        },
	{0x0d, 0x01, 0xf0, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0xf0, 0xa0, "2 Coins 3 Plays"        },
	{0x0d, 0x01, 0xf0, 0xb0, "2 Coins 4 Plays"        },
	{0x0d, 0x01, 0xf0, 0x10, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0xf0, 0xc0, "2 Coins 5 Plays"        },
	{0x0d, 0x01, 0xf0, 0xd0, "2 Coins 6 Plays"        },
	{0x0d, 0x01, 0xf0, 0x20, "1 Coin  3 Plays"        },
	{0x0d, 0x01, 0xf0, 0xe0, "2 Coins 7 Plays"        },
	{0x0d, 0x01, 0xf0, 0xf0, "2 Coins 8 Plays"        },
	{0x0d, 0x01, 0xf0, 0x30, "1 Coin  4 Plays"        },
	{0x0d, 0x01, 0xf0, 0x40, "1 Coin  5 Plays"        },
	{0x0d, 0x01, 0xf0, 0x50, "1 Coin  6 Plays"        },
	{0x0d, 0x01, 0xf0, 0x60, "1 Coin  7 Plays"        },
	{0x0d, 0x01, 0xf0, 0x70, "1 Coin  8 Plays"        },
	
	// Dip 4
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x03, 0x03, "2"                      },
	{0x0e, 0x01, 0x03, 0x02, "3"                      },
	{0x0e, 0x01, 0x03, 0x01, "4"                      },
	{0x0e, 0x01, 0x03, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x0c, 0x0c, "None"                   },
	{0x0e, 0x01, 0x0c, 0x08, "100000"                 },
	{0x0e, 0x01, 0x0c, 0x04, "150000"                 },
	{0x0e, 0x01, 0x0c, 0x00, "200000"                 },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0e, 0x01, 0x10, 0x10, "Upright"                },
	{0x0e, 0x01, 0x10, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x0e, 0x01, 0x20, 0x20, "Off"                    },
	{0x0e, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0e, 0x01, 0x80, 0x80, "Off"                    },
	{0x0e, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Ad2083)

static struct BurnDIPInfo AmidarDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x06, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x00, "3"                      },
	{0x11, 0x01, 0x03, 0x01, "4"                      },
	{0x11, 0x01, 0x03, 0x02, "5"                      },
	{0x11, 0x01, 0x03, 0x03, "255"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x02, 0x00, "Off"                    },
	{0x12, 0x01, 0x02, 0x02, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x12, 0x01, 0x04, 0x04, "30000 50000"            },
	{0x12, 0x01, 0x04, 0x00, "50000 50000"            },
	
	// Dip 4
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x13, 0x01, 0x0f, 0x0b, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x05, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x0e, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x0d, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x0f, 0x07, "4 Coins 3 Plays"        },
	{0x13, 0x01, 0x0f, 0x00, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x0f, 0x03, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x0f, 0x01, "2 Coins 3 Plays"        },
	{0x13, 0x01, 0x0f, 0x08, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 5 Plays"        },
	{0x13, 0x01, 0x0f, 0x04, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x0f, 0x02, "1 Coin  5 Plays"        },
	{0x13, 0x01, 0x0f, 0x0a, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x0f, 0x06, "1 Coin  7 Plays"        },
	{0x13, 0x01, 0x0f, 0x0f, "Free Play"              },
	
	{0   , 0xfe, 0   , 16  , "Coin B"                 },
	{0x13, 0x01, 0xf0, 0xb0, "4 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0x50, "3 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0xe0, "2 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0xd0, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0xf0, 0x70, "4 Coins 3 Plays"        },
	{0x13, 0x01, 0xf0, 0x00, "1 Coin  1 Play"         },
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0xf0, 0x10, "2 Coins 3 Plays"        },
	{0x13, 0x01, 0xf0, 0x80, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 5 Plays"        },
	{0x13, 0x01, 0xf0, 0x40, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0xf0, 0x20, "1 Coin  5 Plays"        },
	{0x13, 0x01, 0xf0, 0xa0, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0xf0, 0x60, "1 Coin  7 Plays"        },
	{0x13, 0x01, 0xf0, 0xf0, "Disable All Coins"      },
};

STDDIPINFO(Amidar)

static struct BurnDIPInfo AmidaroDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x02, NULL                     },
	{0x12, 0xff, 0xff, 0x06, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x00, "1"                      },
	{0x11, 0x01, 0x03, 0x01, "2"                      },
	{0x11, 0x01, 0x03, 0x02, "3"                      },
	{0x11, 0x01, 0x03, 0x03, "4"                      },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Level Progression"      },
	{0x12, 0x01, 0x02, 0x02, "Slow"                   },
	{0x12, 0x01, 0x02, 0x00, "Fast"                   },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x12, 0x01, 0x04, 0x04, "30000 70000"            },
	{0x12, 0x01, 0x04, 0x00, "50000 80000"            },
	
	// Dip 4
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x13, 0x01, 0x0f, 0x0b, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x05, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x0e, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x0d, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x0f, 0x07, "4 Coins 3 Plays"        },
	{0x13, 0x01, 0x0f, 0x00, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x0f, 0x03, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x0f, 0x01, "2 Coins 3 Plays"        },
	{0x13, 0x01, 0x0f, 0x08, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 5 Plays"        },
	{0x13, 0x01, 0x0f, 0x04, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x0f, 0x02, "1 Coin  5 Plays"        },
	{0x13, 0x01, 0x0f, 0x0a, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x0f, 0x06, "1 Coin  7 Plays"        },
	{0x13, 0x01, 0x0f, 0x0f, "Free Play"              },
	
	{0   , 0xfe, 0   , 16  , "Coin B"                 },
	{0x13, 0x01, 0xf0, 0xb0, "4 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0x50, "3 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0xe0, "2 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0xd0, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0xf0, 0x70, "4 Coins 3 Plays"        },
	{0x13, 0x01, 0xf0, 0x00, "1 Coin  1 Play"         },
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0xf0, 0x10, "2 Coins 3 Plays"        },
	{0x13, 0x01, 0xf0, 0x80, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 5 Plays"        },
	{0x13, 0x01, 0xf0, 0x40, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0xf0, 0x20, "1 Coin  5 Plays"        },
	{0x13, 0x01, 0xf0, 0xa0, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0xf0, 0x60, "1 Coin  7 Plays"        },
	{0x13, 0x01, 0xf0, 0xf0, "Disable All Coins"      },
};

STDDIPINFO(Amidaro)

static struct BurnDIPInfo AmidarsDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x01, NULL                     },
	{0x12, 0xff, 0xff, 0x0a, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x00, "2"                      },
	{0x11, 0x01, 0x03, 0x01, "3"                      },
	{0x11, 0x01, 0x03, 0x02, "4"                      },
	{0x11, 0x01, 0x03, 0x03, "256"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x12, 0x01, 0x02, 0x02, "A 1C/1P  B 1C/6P"       },
	{0x12, 0x01, 0x02, 0x00, "A 2C/1P  B 1C/3P"       },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x08, 0x08, "Upright"                },
	{0x12, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Amidars)

static struct BurnDIPInfo AmidaruDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x06, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x00, "3"                      },
	{0x11, 0x01, 0x03, 0x01, "4"                      },
	{0x11, 0x01, 0x03, 0x02, "5"                      },
	{0x11, 0x01, 0x03, 0x03, "255"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x02, 0x00, "Off"                    },
	{0x12, 0x01, 0x02, 0x02, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x12, 0x01, 0x04, 0x04, "30000 70000"            },
	{0x12, 0x01, 0x04, 0x00, "50000 80000"            },
	
	// Dip 4
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x13, 0x01, 0x0f, 0x0b, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x05, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x0e, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x0f, 0x0d, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x0f, 0x07, "4 Coins 3 Plays"        },
	{0x13, 0x01, 0x0f, 0x00, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x0f, 0x03, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x0f, 0x01, "2 Coins 3 Plays"        },
	{0x13, 0x01, 0x0f, 0x08, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 5 Plays"        },
	{0x13, 0x01, 0x0f, 0x04, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x0f, 0x02, "1 Coin  5 Plays"        },
	{0x13, 0x01, 0x0f, 0x0a, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x0f, 0x06, "1 Coin  7 Plays"        },
	{0x13, 0x01, 0x0f, 0x0f, "Free Play"              },
	
	{0   , 0xfe, 0   , 16  , "Coin B"                 },
	{0x13, 0x01, 0xf0, 0xb0, "4 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0x50, "3 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0xe0, "2 Coins 1 Play"         },
	{0x13, 0x01, 0xf0, 0xd0, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0xf0, 0x70, "4 Coins 3 Plays"        },
	{0x13, 0x01, 0xf0, 0x00, "1 Coin  1 Play"         },
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0xf0, 0x10, "2 Coins 3 Plays"        },
	{0x13, 0x01, 0xf0, 0x80, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 5 Plays"        },
	{0x13, 0x01, 0xf0, 0x40, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0xf0, 0x20, "1 Coin  5 Plays"        },
	{0x13, 0x01, 0xf0, 0xa0, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0xf0, 0x60, "1 Coin  7 Plays"        },
	{0x13, 0x01, 0xf0, 0xf0, "Disable All Coins"      },
};

STDDIPINFO(Amidaru)

static struct BurnDIPInfo AnteaterDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x02, NULL                     },
	{0x11, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x10, 0x01, 0x01, 0x00, "3"                      },
	{0x10, 0x01, 0x01, 0x01, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x10, 0x01, 0x02, 0x00, "Off"                    },
	{0x10, 0x01, 0x02, 0x02, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x11, 0x01, 0x06, 0x04, "1 Coin 1 Play"          },
	{0x11, 0x01, 0x06, 0x06, "A 1C/2P  B 2C/1P"       },
	{0x11, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P"       },
	{0x11, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P"       },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x08, 0x00, "Upright"                },
	{0x11, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Anteater)

static struct BurnDIPInfo AnteatergDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x02, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x0f, 0x01, 0x02, 0x00, "A 1C/1P  B 1C/5P"       },
	{0x0f, 0x01, 0x02, 0x02, "A 2C/1P  B 1C/3P"       },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x0c, 0x00, "3"                      },
	{0x0f, 0x01, 0x0c, 0x04, "4"                      },
	{0x0f, 0x01, 0x0c, 0x08, "5"                      },
	{0x0f, 0x01, 0x0c, 0x0c, "6"                      },
	
	// Dip 2	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x01, 0x00, "Upright"                },
	{0x10, 0x01, 0x01, 0x01, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x10, 0x01, 0x02, 0x00, "Off"                    },
	{0x10, 0x01, 0x02, 0x02, "On"                     },
	
	// Dip 3
};

STDDIPINFO(Anteaterg)

static struct BurnDIPInfo AnteaterukDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x02, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x01, 0x00, "Upright"                },
	{0x10, 0x01, 0x01, 0x01, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x10, 0x01, 0x02, 0x00, "Off"                    },
	{0x10, 0x01, 0x02, 0x02, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x11, 0x01, 0x02, 0x00, "A 1C/1P  B 1C/5P"       },
	{0x11, 0x01, 0x02, 0x02, "A 2C/1P  B 1C/3P"       },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x0c, 0x00, "3"                      },
	{0x11, 0x01, 0x0c, 0x04, "4"                      },
	{0x11, 0x01, 0x0c, 0x08, "5"                      },
	{0x11, 0x01, 0x0c, 0x0c, "6"                      },
};

STDDIPINFO(Anteateruk)

static struct BurnDIPInfo ArmorcarDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x02, NULL                     },
	{0x13, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x12, 0x01, 0x01, 0x00, "3"                      },
	{0x12, 0x01, 0x01, 0x01, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x02, 0x00, "Off"                    },
	{0x12, 0x01, 0x02, 0x02, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x13, 0x01, 0x06, 0x04, "1 Coin 1 Play"          },
	{0x13, 0x01, 0x06, 0x06, "A 1C/2P  B 2C/1P"       },
	{0x13, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P"       },
	{0x13, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P"       },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x08, 0x00, "Upright"                },
	{0x13, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Armorcar)

static struct BurnDIPInfo AtlantisDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x01, NULL                     },
	{0x13, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x01, 0x01, "Upright"                },
	{0x12, 0x01, 0x01, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x12, 0x01, 0x02, 0x00, "3"                      },
	{0x12, 0x01, 0x02, 0x02, "5"                      },
	
	// Dip 3
	{0   , 0xfe, 0   , 3   , "Coinage"                },
	{0x13, 0x01, 0x0e, 0x0c, "A 1C/3P  B 2C/1P"       },
	{0x13, 0x01, 0x0e, 0x0e, "A 1C/6P  B 1C/1P"       },
	{0x13, 0x01, 0x0e, 0x0a, "A 1C/99P  B 1C/99P"     },
};

STDDIPINFO(Atlantis)

static struct BurnDIPInfo AtlantisbDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x80, NULL                     },
	{0x11, 0xff, 0xff, 0x0c, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x10, 0x01, 0x40, 0x40, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x40, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x10, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x10, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x11, 0x01, 0x02, 0x00, "Easy"                   },
	{0x11, 0x01, 0x02, 0x02, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x04, 0x04, "3"                      },
	{0x11, 0x01, 0x04, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x08, 0x08, "Upright"                },
	{0x11, 0x01, 0x08, 0x00, "Cocktail"               },	
};

STDDIPINFO(Atlantisb)

static struct BurnDIPInfo AzurianDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0f, 0x01, 0x80, 0x00, "3"                      },
	{0x0f, 0x01, 0x80, 0x80, "5"                      },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x10, 0x01, 0x01, 0x01, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x01, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x10, 0x01, 0x02, 0x00, "5000"                   },
	{0x10, 0x01, 0x02, 0x02, "7000"                   },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x08, 0x00, "Upright"                },
	{0x10, 0x01, 0x08, 0x08, "Cocktail"               },
	
	// Fake Dip
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x11, 0x01, 0x03, 0x03, "Easy"                   },
	{0x11, 0x01, 0x03, 0x01, "Normal"                 },
	{0x11, 0x01, 0x03, 0x02, "Hard"                   },
	{0x11, 0x01, 0x03, 0x00, "Very Hard"              },
};

STDDIPINFO(Azurian)

static struct BurnDIPInfo BagmanmcDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x80, NULL                     },
	{0x11, 0xff, 0xff, 0x7e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x80, 0x80, "Upright"                },
	{0x10, 0x01, 0x80, 0x00, "Cocktail"               },	
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x03, "2"                      },
	{0x11, 0x01, 0x03, 0x02, "3"                      },
	{0x11, 0x01, 0x03, 0x01, "4"                      },
	{0x11, 0x01, 0x03, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x11, 0x01, 0x04, 0x00, "A 2C/1P  B 1C/1P"       },
	{0x11, 0x01, 0x04, 0x04, "A 1C/1P  B 1C/2P"       },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x11, 0x01, 0x18, 0x18, "Easy"                   },
	{0x11, 0x01, 0x18, 0x10, "Medium"                 },
	{0x11, 0x01, 0x18, 0x08, "Hard"                   },
	{0x11, 0x01, 0x18, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x11, 0x01, 0x20, 0x20, "English"                },
	{0x11, 0x01, 0x20, 0x00, "French"                 },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x11, 0x01, 0x40, 0x40, "30000"                  },
	{0x11, 0x01, 0x40, 0x00, "40000"                  },
};

STDDIPINFO(Bagmanmc)

static struct BurnDIPInfo Batman2DIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0xc0, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0e, 0x01, 0x40, 0x40, "1 Coin 1 Play"          },
	{0x0e, 0x01, 0x40, 0x00, "1 Coin 2 Plays"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0e, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x0e, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0f, 0x01, 0x01, 0x00, "30000"                  },
	{0x0f, 0x01, 0x01, 0x01, "50000"                  },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0f, 0x01, 0x04, 0x00, "3"                      },
	{0x0f, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x08, 0x00, "Upright"                },
	{0x0f, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Batman2)

static struct BurnDIPInfo BlkholeDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0x00, NULL                     },
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x02, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0b, 0x01, 0xc0, 0xc0, "2 Coins 1 Play"         },
	{0x0b, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x0b, 0x01, 0xc0, 0x40, "1 Coin  2 Plays"        },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin  3 Plays"        },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0c, 0x01, 0x01, 0x00, "5000"                   },
	{0x0c, 0x01, 0x01, 0x01, "10000"                  },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x02, 0x02, "Upright"                },
	{0x0c, 0x01, 0x02, 0x00, "Cocktail"               },
};

STDDIPINFO(Blkhole)

static struct BurnDIPInfo BongoDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0x00, NULL                     },
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x40, NULL                     },
	
	// Dip 1
	
	// Dip 2
		
	// Dip 3
		
	// Dip 4
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0d, 0x01, 0x06, 0x00, "2"                      },
	{0x0d, 0x01, 0x06, 0x02, "3"                      },
	{0x0d, 0x01, 0x06, 0x04, "4"                      },
	{0x0d, 0x01, 0x06, 0x06, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Infinite Lives"         },
	{0x0d, 0x01, 0x08, 0x00, "Off"                    },
	{0x0d, 0x01, 0x08, 0x08, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x0d, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x80, 0x00, "Upright"                },
	{0x0d, 0x01, 0x80, 0x80, "Cocktail"               },
};

STDDIPINFO(Bongo)

static struct BurnDIPInfo CalipsoDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x02, NULL                     },
	{0x0f, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x01, 0x00, "3"                      },
	{0x0e, 0x01, 0x01, 0x01, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0e, 0x01, 0x02, 0x00, "Off"                    },
	{0x0e, 0x01, 0x02, 0x02, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0f, 0x01, 0x06, 0x04, "1 Coin 1 Play"          },
	{0x0f, 0x01, 0x06, 0x06, "1 Coin 2 Plays"         },
	{0x0f, 0x01, 0x06, 0x02, "1 Coin 3 Plays"         },
	{0x0f, 0x01, 0x06, 0x00, "1 Coin 4 Plays"         },
};

STDDIPINFO(Calipso)

static struct BurnDIPInfo CatacombDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x40, NULL                     },
	{0x0d, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0c, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0c, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0c, 0x01, 0x80, 0x00, "1 Coin 3 Plays"         },
	{0x0c, 0x01, 0x80, 0x80, "1 Coin 5 Plays"         },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0d, 0x01, 0x04, 0x04, "3"                      },
	{0x0d, 0x01, 0x04, 0x00, "5"                      },
};

STDDIPINFO(Catacomb)

static struct BurnDIPInfo CavelonDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x02, NULL                     },
	{0x13, 0xff, 0xff, 0x0c, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x01, 0x00, "Upright"                },
	{0x12, 0x01, 0x01, 0x01, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x12, 0x01, 0x02, 0x02, "A 1C/1P  B 1C/6P"       },
	{0x12, 0x01, 0x02, 0x00, "A 2C/1P  B 1C/3P"       },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x06, 0x00, "2"                      },
	{0x13, 0x01, 0x06, 0x04, "3"                      },
	{0x13, 0x01, 0x06, 0x02, "4"                      },
	{0x13, 0x01, 0x06, 0x06, "5"                      },
};

STDDIPINFO(Cavelon)

static struct BurnDIPInfo CheckmanjDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x12, 0x01, 0x40, 0x40, "A 2C/1P  B 1C/3P"       },
	{0x12, 0x01, 0x40, 0x00, "A 1C/1P  B 1C/6P"       },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x80, 0x00, "Upright"                },
	{0x12, 0x01, 0x80, 0x80, "Cocktail"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x00, "3"                      },
	{0x13, 0x01, 0x03, 0x01, "4"                      },
	{0x13, 0x01, 0x03, 0x02, "5"                      },
	{0x13, 0x01, 0x03, 0x03, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x13, 0x01, 0x04, 0x00, "100000"                 },
	{0x13, 0x01, 0x04, 0x04, "200000"                 },
	
	{0   , 0xfe, 0   , 2   , "Difficulty increases at level"},
	{0x13, 0x01, 0x08, 0x08, "3"                      },
	{0x13, 0x01, 0x08, 0x00, "5"                      },
};

STDDIPINFO(Checkmanj)

static struct BurnDIPInfo CheckmanDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x10, 0x01, 0x40, 0x40, "A 2C/1P  B 1C/3P"       },
	{0x10, 0x01, 0x40, 0x00, "A 1C/1P  B 1C/6P"       },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x80, 0x00, "Upright"                },
	{0x10, 0x01, 0x80, 0x80, "Cocktail"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x00, "3"                      },
	{0x11, 0x01, 0x03, 0x01, "4"                      },
	{0x11, 0x01, 0x03, 0x02, "5"                      },
	{0x11, 0x01, 0x03, 0x03, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x11, 0x01, 0x04, 0x00, "100000"                 },
	{0x11, 0x01, 0x04, 0x04, "200000"                 },
	
	{0   , 0xfe, 0   , 2   , "Difficulty increases at level"},
	{0x11, 0x01, 0x08, 0x08, "3"                      },
	{0x11, 0x01, 0x08, 0x00, "5"                      },
};

STDDIPINFO(Checkman)

static struct BurnDIPInfo CkonggDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x80, NULL                     },
	{0x11, 0xff, 0xff, 0x0a, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x10, 0x01, 0x40, 0x00, "1 Coin 2 Plays"         },
	{0x10, 0x01, 0x40, 0x40, "1 Coin 4 Plays"         },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x00, "1"                      },
	{0x11, 0x01, 0x03, 0x01, "2"                      },
	{0x11, 0x01, 0x03, 0x02, "3"                      },
	{0x11, 0x01, 0x03, 0x03, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x11, 0x01, 0x08, 0x08, "500000"                 },
	{0x11, 0x01, 0x08, 0x00, "750000"                 },
};

STDDIPINFO(Ckongg)

static struct BurnDIPInfo CkongsDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x02, NULL                     },
	
	// Dip 1
	
	// Dip 2
		
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x02, 0x02, "Upright"                },
	{0x11, 0x01, 0x02, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x04, 0x00, "3"                      },
	{0x11, 0x01, 0x04, 0x04, "4"                      },
};

STDDIPINFO(Ckongs)

static struct BurnDIPInfo DambustrDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Clear Swear Words"      },
	{0x0c, 0x01, 0x20, 0x00, "Off"                    },
	{0x0c, 0x01, 0x20, 0x20, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Second Coin Counter"    },
	{0x0d, 0x01, 0x40, 0x00, "Off"                    },
	{0x0d, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x0d, 0x01, 0x80, 0x80, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x80, 0x00, "1 Coin  1 Play"         },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x03, 0x00, "3"                      },
	{0x0e, 0x01, 0x03, 0x01, "4"                      },
	{0x0e, 0x01, 0x03, 0x02, "5"                      },
	{0x0e, 0x01, 0x03, 0x03, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Game Test Mode"         },
	{0x0e, 0x01, 0x04, 0x00, "Off"                    },
	{0x0e, 0x01, 0x04, 0x04, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Union Jack"             },
	{0x0e, 0x01, 0x08, 0x08, "Off"                    },
	{0x0e, 0x01, 0x08, 0x00, "On"                     },
};

STDDIPINFO(Dambustr)

static struct BurnDIPInfo DarkplntDIPList[]=
{
	// Default Values
	{0x08, 0xff, 0xff, 0x00, NULL                     },
	{0x09, 0xff, 0xff, 0x02, NULL                     },
	{0x0a, 0xff, 0xff, 0x0c, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Bonus Occurrence"       },
	{0x09, 0x01, 0x01, 0x00, "Once"                   },
	{0x09, 0x01, 0x01, 0x01, "Everytime"              },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x09, 0x01, 0x02, 0x02, "1"                      },
	{0x09, 0x01, 0x02, 0x00, "2"                      },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0a, 0x01, 0x06, 0x04, "1 Coin 1 Play"          },
	{0x0a, 0x01, 0x06, 0x06, "A 1C/2P  B 2C/1P"       },
	{0x0a, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P"       },
	{0x0a, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P"       },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0a, 0x01, 0x08, 0x08, "100000"                 },
	{0x0a, 0x01, 0x08, 0x00, "200000"                 },
};

STDDIPINFO(Darkplnt)

static struct BurnDIPInfo DevilfsgDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0xc0, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x10, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x10, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x10, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x11, 0x01, 0x01, 0x00, "10000"                  },
	{0x11, 0x01, 0x01, 0x01, "15000"                  },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x04, 0x00, "4"                      },
	{0x11, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x08, 0x00, "Upright"                },
	{0x11, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Devilfsg)

static struct BurnDIPInfo DevilfshDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x03, NULL                     },
	{0x11, 0xff, 0xff, 0x02, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x10, 0x01, 0x01, 0x01, "10000"                  },
	{0x10, 0x01, 0x01, 0x00, "15000"                  },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x02, 0x02, "Upright"                },
	{0x10, 0x01, 0x02, 0x00, "Cocktail"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x02, 0x02, "4"                      },
	{0x11, 0x01, 0x02, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x11, 0x01, 0x04, 0x04, "2 Coins 1 Play"         },
	{0x11, 0x01, 0x04, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x11, 0x01, 0x08, 0x00, "1 Coin 3 Plays"         },
	{0x11, 0x01, 0x08, 0x08, "1 Coin 5 Plays"         },
};

STDDIPINFO(Devilfsh)

static struct BurnDIPInfo DingoDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x80, NULL                     },
	{0x11, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x40, 0x00, "Upright"                },
	{0x10, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x10, 0x01, 0x80, 0x40, "A 2C/1P  B 1C/3P"       },
	{0x10, 0x01, 0x80, 0x80, "A 1C/1P  B 1C/5P"       },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x11, 0x01, 0x03, 0x00, "Easy"                   },
	{0x11, 0x01, 0x03, 0x01, "Medium"                 },
	{0x11, 0x01, 0x03, 0x02, "Hard"                   },
	{0x11, 0x01, 0x03, 0x03, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x04, 0x00, "3"                      },
	{0x11, 0x01, 0x04, 0x04, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x11, 0x01, 0x08, 0x00, "20000"                  },
	{0x11, 0x01, 0x08, 0x08, "40000"                  },
};

STDDIPINFO(Dingo)

static struct BurnDIPInfo DkongjrmDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x08, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0f, 0x01, 0xc0, 0xc0, "3 Coins 1 Play"         },
	{0x0f, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Coin Multiplier"        },
	{0x10, 0x01, 0x01, 0x00, "x1"                     },
	{0x10, 0x01, 0x01, 0x01, "x2"                     },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x06, 0x00, "3"                      },
	{0x10, 0x01, 0x06, 0x02, "4"                      },
	{0x10, 0x01, 0x06, 0x04, "5"                      },
	{0x10, 0x01, 0x06, 0x06, "6"                      },	
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x08, 0x08, "Upright"                },
	{0x10, 0x01, 0x08, 0x00, "Cocktail"               },	
};

STDDIPINFO(Dkongjrm)

static struct BurnDIPInfo DrivfrcgDIPList[]=
{
	// Default Values
	{0x07, 0xff, 0xff, 0x00, NULL                     },
	{0x08, 0xff, 0xff, 0x00, NULL                     },
	{0x09, 0xff, 0xff, 0x00, NULL                     },
	{0x0a, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x08, 0x01, 0x40, 0x40, "A 2C/1P  B 1C/3P"       },
	{0x08, 0x01, 0x40, 0x00, "A 1C/1P  B 1C/5P"       },
	
	// Dip 3
	
	// Dip 4
};

STDDIPINFO(Drivfrcg)

static struct BurnDIPInfo Eagle2DIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x80, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0b, 0x01, 0x20, 0x00, "Upright"                },
	{0x0b, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0c, 0x01, 0x40, 0x00, "30000"                  },
	{0x0c, 0x01, 0x40, 0x40, "50000"                  },
	
	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x0c, 0x01, 0x80, 0x80, "English"                },
	{0x0c, 0x01, 0x80, 0x00, "Japanese"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0x03, 0x03, "4 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x02, "3 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x01, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x0d, 0x01, 0x0c, 0x00, "Off"                    },
	{0x0d, 0x01, 0x0c, 0x0c, "On"                     },
};

STDDIPINFO(Eagle2)

static struct BurnDIPInfo ExodusDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0xc0, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0b, 0x01, 0x40, 0x00, "Off"                    },
	{0x0b, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0c, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0c, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0c, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x0c, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0d, 0x01, 0x01, 0x00, "7000"                   },
	{0x0d, 0x01, 0x01, 0x01, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0d, 0x01, 0x04, 0x00, "3"                      },
	{0x0d, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x08, 0x00, "Upright"                },
	{0x0d, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Exodus)

static struct BurnDIPInfo ExplorerDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	
	// Dip 2
	
	// Dip 3
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x0d, 0x01, 0x0f, 0x08, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x0f, 0x09, "2 Coins 2 Plays"        },
	{0x0d, 0x01, 0x0f, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0x0f, 0x0a, "2 Coins 3 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0b, "2 Coins 4 Plays"        },
	{0x0d, 0x01, 0x0f, 0x01, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0c, "2 Coins 5 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0d, "2 Coins 6 Plays"        },
	{0x0d, 0x01, 0x0f, 0x02, "1 Coin  3 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0e, "2 Coins 7 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0f, "2 Coins 8 Plays"        },
	{0x0d, 0x01, 0x0f, 0x03, "1 Coin  4 Plays"        },
	{0x0d, 0x01, 0x0f, 0x04, "1 Coin  5 Plays"        },
	{0x0d, 0x01, 0x0f, 0x05, "1 Coin  6 Plays"        },
	{0x0d, 0x01, 0x0f, 0x06, "1 Coin  7 Plays"        },
	{0x0d, 0x01, 0x0f, 0x07, "1 Coin  8 Plays"        },
	
	{0   , 0xfe, 0   , 16  , "Coin B"                 },
	{0x0d, 0x01, 0xf0, 0x80, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0xf0, 0x90, "2 Coins 2 Plays"        },
	{0x0d, 0x01, 0xf0, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0xf0, 0xa0, "2 Coins 3 Plays"        },
	{0x0d, 0x01, 0xf0, 0xb0, "2 Coins 4 Plays"        },
	{0x0d, 0x01, 0xf0, 0x10, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0xf0, 0xc0, "2 Coins 5 Plays"        },
	{0x0d, 0x01, 0xf0, 0xd0, "2 Coins 6 Plays"        },
	{0x0d, 0x01, 0xf0, 0x20, "1 Coin  3 Plays"        },
	{0x0d, 0x01, 0xf0, 0xe0, "2 Coins 7 Plays"        },
	{0x0d, 0x01, 0xf0, 0xf0, "2 Coins 8 Plays"        },
	{0x0d, 0x01, 0xf0, 0x30, "1 Coin  4 Plays"        },
	{0x0d, 0x01, 0xf0, 0x40, "1 Coin  5 Plays"        },
	{0x0d, 0x01, 0xf0, 0x50, "1 Coin  6 Plays"        },
	{0x0d, 0x01, 0xf0, 0x60, "1 Coin  7 Plays"        },
	{0x0d, 0x01, 0xf0, 0x70, "1 Coin  8 Plays"        },
	
	// Dip 4
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x03, 0x00, "2"                      },
	{0x0e, 0x01, 0x03, 0x01, "3"                      },
	{0x0e, 0x01, 0x03, 0x02, "4"                      },
	{0x0e, 0x01, 0x03, 0x03, "5"                      },
	
	{0   , 0xfe, 0   , 8   , "Bonus Life"             },
	{0x0e, 0x01, 0x1c, 0x04, "10000"                  },
	{0x0e, 0x01, 0x1c, 0x0c, "15000"                  },
	{0x0e, 0x01, 0x1c, 0x14, "20000"                  },
	{0x0e, 0x01, 0x1c, 0x1c, "25000"                  },
	{0x0e, 0x01, 0x1c, 0x00, "30000"                  },
	{0x0e, 0x01, 0x1c, 0x08, "50000"                  },
	{0x0e, 0x01, 0x1c, 0x10, "70000"                  },
	{0x0e, 0x01, 0x1c, 0x18, "80000"                  },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0e, 0x01, 0x80, 0x00, "Off"                    },
	{0x0e, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(Explorer)

static struct BurnDIPInfo FantastcDIPList[]=
{
	// Default Values
	{0x07, 0xff, 0xff, 0x40, NULL                     },
	{0x08, 0xff, 0xff, 0x00, NULL                     },
	{0x09, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x07, 0x01, 0x20, 0x00, "Off"                    },
	{0x07, 0x01, 0x20, 0x20, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Extended Bonus Life"    },
	{0x07, 0x01, 0x40, 0x00, "Off"                    },
	{0x07, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x08, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x08, 0x01, 0xc0, 0x80, "2 Coins 1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x08, 0x01, 0x80, 0x00, "3"                      },
	{0x08, 0x01, 0x80, 0x80, "5"                      },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x09, 0x01, 0x03, 0x00, "Easy"                   },
	{0x09, 0x01, 0x03, 0x01, "Medium"                 },
	{0x09, 0x01, 0x03, 0x02, "Hard"                   },
	{0x09, 0x01, 0x03, 0x03, "Hardest"                },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x09, 0x01, 0x0c, 0x00, "20000 80000"            },
	{0x09, 0x01, 0x0c, 0x04, "30000 80000"            },
	{0x09, 0x01, 0x0c, 0x08, "20000 120000"           },
	{0x09, 0x01, 0x0c, 0x0c, "30000 120000"           },
};

STDDIPINFO(Fantastc)

static struct BurnDIPInfo FantaziaDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0b, 0x01, 0x20, 0x00, "Upright"                },
	{0x0b, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0c, 0x01, 0x40, 0x00, "30000"                  },
	{0x0c, 0x01, 0x40, 0x40, "50000"                  },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0d, 0x01, 0x03, 0x03, "4 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x02, "3 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x01, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0d, 0x01, 0x0c, 0x00, "1 Coin 1 Play"          },
	{0x0d, 0x01, 0x0c, 0x04, "1 Coin 2 Plays"         },
	{0x0d, 0x01, 0x0c, 0x08, "1 Coin 3 Plays"         },
	{0x0d, 0x01, 0x0c, 0x0c, "Freeplay"               },
};

STDDIPINFO(Fantazia)

static struct BurnDIPInfo Fourin1DIPList[]=
{
	// Defaults
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0e, 0x01, 0x20, 0x00, "Upright"                },
	{0x0e, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x10, 0x01, 0x04, 0x00, "3"                      },
	{0x10, 0x01, 0x04, 0x04, "5"                      },
	
	// Ghost Muncher Pt 3
	{0   , 0xfe, 0   , 4   , "Bonus Life (Ghost Muncher)"},
	{0x11, 0x01, 0x03, 0x01, "10000"                  },
	{0x11, 0x01, 0x03, 0x02, "15000"                  },
	{0x11, 0x01, 0x03, 0x03, "20000"                  },
	{0x11, 0x01, 0x03, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 4   , "Coinage (Ghost Muncher)"},
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"       },
	{0x11, 0x01, 0xc0, 0x00, "1 Coin  1 Credit"       },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"      },
	{0x11, 0x01, 0xc0, 0xc0, "Free Play"              },
	
	// Scramble Pt 2
	{0   , 0xfe, 0   , 2   , "Allow Continue (Scramble)"},
	{0x12, 0x01, 0x08, 0x08, "No"                     },
	{0x12, 0x01, 0x08, 0x00, "Yes"                    },
	
	{0   , 0xfe, 0   , 4   , "Coinage (Scramble)"     },
	{0x12, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0xc0, 0x00, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0xc0, 0xc0, "Free Play"              },
	
	// Galaxian Pt 5
	{0   , 0xfe, 0   , 4   , "Bonus Life (Galaxian)"  },
	{0x13, 0x01, 0x03, 0x01, "4000"                   },
	{0x13, 0x01, 0x03, 0x02, "5000"                   },
	{0x13, 0x01, 0x03, 0x03, "7000"                   },
	{0x13, 0x01, 0x03, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 3   , "Coinage (Galaxian)"     },
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  1 Credit"       },
	{0x13, 0x01, 0xc0, 0xc0, "Free Play"              },
	
	// Galactic Convoy
	{0   , 0xfe, 0   , 2   , "Bonus Life (Galactic Convoy)"},
	{0x14, 0x01, 0x01, 0x00, "50000"                  },
	{0x14, 0x01, 0x01, 0x01, "80000"                  },
		
	{0   , 0xfe, 0   , 3   , "Coinage (Galactic Convoy)"},
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0xc0, 0x00, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0xc0, 0xc0, "Free Play"              },
};

STDDIPINFO(Fourin1)

static struct BurnDIPInfo FroggDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0xc0, NULL                     },
	{0x10, 0xff, 0xff, 0x05, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0xc0, 0xc0, "3"                      },
	{0x0f, 0x01, 0xc0, 0x80, "5"                      },
	{0x0f, 0x01, 0xc0, 0x40, "7"                      },
	{0x0f, 0x01, 0xc0, 0x00, "256"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x01, 0x01, "Upright"                },
	{0x10, 0x01, 0x01, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x10, 0x01, 0x04, 0x00, "2 Coins 1 Credit"       },
	{0x10, 0x01, 0x04, 0x04, "1 Coin  1 Credit"       },	
};

STDDIPINFO(Frogg)

static struct BurnDIPInfo FroggerDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x03, NULL                     },
	{0x10, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x03, 0x03, "3"                      },
	{0x0f, 0x01, 0x03, 0x02, "5"                      },
	{0x0f, 0x01, 0x03, 0x01, "7"                      },
	{0x0f, 0x01, 0x03, 0x00, "256"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x06, 0x04, "A 2C/1P  B 2C/1P  C 2C/1P"},
	{0x10, 0x01, 0x06, 0x02, "A 2C/1P  B 1C/3P  C 2C/1P"},
	{0x10, 0x01, 0x06, 0x06, "A 1C/1P  B 1C/1P  C 1C/1P"},
	{0x10, 0x01, 0x06, 0x00, "A 1C/1P  B 1C/6P  C 1C/1P"},
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x08, 0x08, "Upright"                },
	{0x10, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Frogger)

static struct BurnDIPInfo FroggermcDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0xc0, NULL                     },
	{0x10, 0xff, 0xff, 0x07, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0xc0, 0xc0, "3"                      },
	{0x0f, 0x01, 0xc0, 0x80, "5"                      },
	{0x0f, 0x01, 0xc0, 0x40, "7"                      },
	{0x0f, 0x01, 0xc0, 0x00, "256"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x01, 0x01, "Upright"                },
	{0x10, 0x01, 0x01, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x06, 0x02, "A 2C/1P  B 2C/1P  C 2C/1P"},
	{0x10, 0x01, 0x06, 0x04, "A 2C/1P  B 1C/3P  C 2C/1P"},
	{0x10, 0x01, 0x06, 0x06, "A 1C/1P  B 1C/1P  C 1C/1P"},
	{0x10, 0x01, 0x06, 0x00, "A 1C/1P  B 1C/6P  C 1C/1P"},	
};

STDDIPINFO(Froggermc)

static struct BurnDIPInfo GalaxianDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x03, 0x00, "7000"                   },
	{0x0e, 0x01, 0x03, 0x01, "10000"                  },
	{0x0e, 0x01, 0x03, 0x02, "12000"                  },
	{0x0e, 0x01, 0x03, 0x03, "20000"                  },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "2"                      },
	{0x0e, 0x01, 0x04, 0x04, "3"                      },
};

STDDIPINFO(Galaxian)

static struct BurnDIPInfo GmgalaxDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x40, NULL                     },
	{0x10, 0xff, 0xff, 0x06, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x01, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	
	// Ghost Muncher Dip 1
	{0   , 0xfe, 0   , 2   , "GM - Cabinet"           },
	{0x0e, 0x01, 0x20, 0x00, "Upright"                },
	{0x0e, 0x01, 0x20, 0x20, "Cocktail"               },
		
	// Ghost Muncher Dip 2
	{0   , 0xfe, 0   , 4   , "GM - Bonus Life"        },
	{0x0f, 0x01, 0xc0, 0x40, "10000"                  },
	{0x0f, 0x01, 0xc0, 0x80, "15000"                  },
	{0x0f, 0x01, 0xc0, 0xc0, "20000"                  },
	{0x0f, 0x01, 0xc0, 0x00, "None"                   },
	
	// Ghost Muncher Dip 3
	{0   , 0xfe, 0   , 4   , "GM - Coinage"           },
	{0x10, 0x01, 0x03, 0x00, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x03, 0x02, "1 Coin  1 Play"         },
	{0x10, 0x01, 0x03, 0x01, "1 Coin  2 Plays"        },
	{0x10, 0x01, 0x03, 0x03, "Freeplay"               },

	{0   , 0xfe, 0   , 4   , "GM - Lives"             },
	{0x10, 0x01, 0x0c, 0x0c, "1"                      },
	{0x10, 0x01, 0x0c, 0x08, "2"                      },
	{0x10, 0x01, 0x0c, 0x04, "3"                      },
	{0x10, 0x01, 0x0c, 0x00, "4"                      },
	
	// Galaxian Dip 1
	{0   , 0xfe, 0   , 2   , "GX - Cabinet"           },
	{0x11, 0x01, 0x20, 0x00, "Upright"                },
	{0x11, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "GX - Service Mode"      },
	{0x11, 0x01, 0x40, 0x00, "Off"                    },
	{0x11, 0x01, 0x40, 0x40, "On"                     },
	
	// Galaxian Dip 2
	{0   , 0xfe, 0   , 4   , "GX - Coinage"           },
	{0x12, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x12, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x12, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Galaxian Dip 3
	{0   , 0xfe, 0   , 4   , "GX - Bonus Life"        },
	{0x13, 0x01, 0x03, 0x01, "4000"                   },
	{0x13, 0x01, 0x03, 0x02, "5000"                   },
	{0x13, 0x01, 0x03, 0x03, "7000"                   },
	{0x13, 0x01, 0x03, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "GX - Lives"             },
	{0x13, 0x01, 0x04, 0x00, "3"                      },
	{0x13, 0x01, 0x04, 0x04, "5"                      },
	
	// Select switch	
	{0   , 0xfe, 0   , 2   , "Game Select"            },
	{0x14, 0x01, 0x01, 0x00, "Ghost Muncher"          },
	{0x14, 0x01, 0x01, 0x01, "Galaxian"               },
};

STDDIPINFO(Gmgalax)

static struct BurnDIPInfo GteikokbDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0xc0, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0c, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0c, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0c, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x0c, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0d, 0x01, 0x01, 0x00, "7000"                   },
	{0x0d, 0x01, 0x01, 0x01, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0d, 0x01, 0x04, 0x00, "3"                      },
	{0x0d, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x08, 0x00, "Upright"                },
	{0x0d, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Gteikokb)

static struct BurnDIPInfo Gteikob2DIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0xc0, NULL                     },
	{0x0d, 0xff, 0xff, 0x08, NULL                     },
	
	// Dip 1

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0c, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0c, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0c, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x0c, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0d, 0x01, 0x01, 0x00, "7000"                   },
	{0x0d, 0x01, 0x01, 0x01, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0d, 0x01, 0x04, 0x00, "3"                      },
	{0x0d, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x08, 0x08, "Upright"                },
	{0x0d, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Gteikob2)

static struct BurnDIPInfo HotshockDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x05, NULL                     },
	
	// Dip 1
		
	// Dip 2
	
	// Dip 3
	{0   , 0xfe, 0   , 16   , "Coin A"                },
	{0x0d, 0x01, 0xf0, 0x80, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0xf0, 0x90, "2 Coins 2 Plays"        },
	{0x0d, 0x01, 0xf0, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0xf0, 0xa0, "2 Coins 3 Plays"        },
	{0x0d, 0x01, 0xf0, 0xb0, "2 Coins 4 Plays"        },
	{0x0d, 0x01, 0xf0, 0x10, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0xf0, 0xc0, "2 Coins 5 Plays"        },
	{0x0d, 0x01, 0xf0, 0xd0, "2 Coins 6 Plays"        },
	{0x0d, 0x01, 0xf0, 0x20, "1 Coin  3 Plays"        },
	{0x0d, 0x01, 0xf0, 0xe0, "2 Coins 7 Plays"        },
	{0x0d, 0x01, 0xf0, 0xf0, "2 Coins 8 Plays"        },
	{0x0d, 0x01, 0xf0, 0x30, "1 Coin  4 Plays"        },
	{0x0d, 0x01, 0xf0, 0x40, "1 Coin  5 Plays"        },
	{0x0d, 0x01, 0xf0, 0x50, "1 Coin  6 Plays"        },
	{0x0d, 0x01, 0xf0, 0x60, "1 Coin  7 Plays"        },
	{0x0d, 0x01, 0xf0, 0x70, "1 Coin  8 Plays"        },
	
	{0   , 0xfe, 0   , 16   , "Coin B"                },
	{0x0d, 0x01, 0x0f, 0x08, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x0f, 0x09, "2 Coins 2 Plays"        },
	{0x0d, 0x01, 0x0f, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0x0f, 0x0a, "2 Coins 3 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0b, "2 Coins 4 Plays"        },
	{0x0d, 0x01, 0x0f, 0x01, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0c, "2 Coins 5 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0d, "2 Coins 6 Plays"        },
	{0x0d, 0x01, 0x0f, 0x02, "1 Coin  3 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0e, "2 Coins 7 Plays"        },
	{0x0d, 0x01, 0x0f, 0x0f, "2 Coins 8 Plays"        },
	{0x0d, 0x01, 0x0f, 0x03, "1 Coin  4 Plays"        },
	{0x0d, 0x01, 0x0f, 0x04, "1 Coin  5 Plays"        },
	{0x0d, 0x01, 0x0f, 0x05, "1 Coin  6 Plays"        },
	{0x0d, 0x01, 0x0f, 0x06, "1 Coin  7 Plays"        },
	{0x0d, 0x01, 0x0f, 0x07, "1 Coin  8 Plays"        },
	
	// Dip 4
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x03, 0x00, "2"                      },
	{0x0e, 0x01, 0x03, 0x01, "3"                      },
	{0x0e, 0x01, 0x03, 0x02, "4"                      },
	{0x0e, 0x01, 0x03, 0x03, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x0e, 0x01, 0x04, 0x04, "English"                },
	{0x0e, 0x01, 0x04, 0x00, "Italian"                },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x18, 0x00, "75000"                  },
	{0x0e, 0x01, 0x18, 0x08, "150000"                 },
	{0x0e, 0x01, 0x18, 0x10, "200000"                 },
	{0x0e, 0x01, 0x18, 0x18, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0e, 0x01, 0x80, 0x00, "Upright"                },
	{0x0e, 0x01, 0x80, 0x80, "Cocktail"               },
};

STDDIPINFO(Hotshock)

static struct BurnDIPInfo HunchbkgDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
		
	// Dip 2
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0e, 0x01, 0x01, 0x00, "Upright"                },
	{0x0e, 0x01, 0x01, 0x01, "Cocktail"               },
};

STDDIPINFO(Hunchbkg)

static struct BurnDIPInfo HunchbksDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x02, NULL                     },
	{0x13, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x12, 0x01, 0x01, 0x00, "3"                      },
	{0x12, 0x01, 0x01, 0x01, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x12, 0x01, 0x02, 0x00, "A 2C/1P  B 1C/3P"       },
	{0x12, 0x01, 0x02, 0x02, "A 1C/1P  B 1C/5P"       },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x06, 0x06, "10000"                  },
	{0x13, 0x01, 0x06, 0x04, "20000"                  },
	{0x13, 0x01, 0x06, 0x02, "40000"                  },
	{0x13, 0x01, 0x06, 0x00, "80000"                  },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x08, 0x08, "Upright"                },
	{0x14, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Hunchbks)

static struct BurnDIPInfo HustlerDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x02, NULL                     },
	{0x12, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
		
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x01, 0x01, "1"                      },
	{0x11, 0x01, 0x01, 0x00, "2"                      },
	
	{0   , 0xfe, 0   , 2   , "Infinite Lives"         },
	{0x11, 0x01, 0x02, 0x02, "Off"                    },
	{0x11, 0x01, 0x02, 0x00, "On"                     },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x12, 0x01, 0x06, 0x02, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x06, 0x04, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x06, 0x06, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x06, 0x00, "1 Coin  2 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x08, 0x08, "Upright"                },
	{0x12, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Hustler)

static struct BurnDIPInfo JumpbugDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x20, 0x00, "Upright"                },
	{0x0f, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x10, 0x01, 0x40, 0x40, "Easy"                   },
	{0x10, 0x01, 0x40, 0x00, "Hard"                   },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x01, "3"                      },
	{0x11, 0x01, 0x03, 0x02, "4"                      },
	{0x11, 0x01, 0x03, 0x03, "5"                      },
	{0x11, 0x01, 0x03, 0x00, "Infinite"               },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x11, 0x01, 0x0c, 0x04, "A 2C/1P  B 2C/1P"       },
	{0x11, 0x01, 0x0c, 0x08, "A 2C/1P  B 1C/3P"       },
	{0x11, 0x01, 0x0c, 0x00, "A 1C/1P  B 1C/1P"       },
	{0x11, 0x01, 0x0c, 0x0c, "A 1C/1P  B 1C/6P"       },
};

STDDIPINFO(Jumpbug)

static struct BurnDIPInfo KingballDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x40, NULL                     },
	{0x0e, 0xff, 0xff, 0x04, NULL                     },
	{0x0f, 0xff, 0xff, 0x02, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0xc0, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0xc0, 0x40, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0xc0, 0x00, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x03, 0x00, "10000"                  },
	{0x0e, 0x01, 0x03, 0x01, "12000"                  },
	{0x0e, 0x01, 0x03, 0x02, "15000"                  },
	{0x0e, 0x01, 0x03, 0x03, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "2"                      },
	{0x0e, 0x01, 0x04, 0x04, "3"                      },
	
	// Fake dip	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0f, 0x01, 0x01, 0x00, "Off"                    },
	{0x0f, 0x01, 0x01, 0x01, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Speech"                 },
	{0x0f, 0x01, 0x02, 0x00, "Off"                    },
	{0x0f, 0x01, 0x02, 0x02, "On"                     },
};

STDDIPINFO(Kingball)

static struct BurnDIPInfo CkongmcDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x0a, NULL                     },
	
	// Dip 1
	
	// Dip 2
		
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x03, 0x00, "1"                      },
	{0x0e, 0x01, 0x03, 0x01, "2"                      },
	{0x0e, 0x01, 0x03, 0x02, "3"                      },
	{0x0e, 0x01, 0x03, 0x03, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0e, 0x01, 0x08, 0x08, "500000"                 },
	{0x0e, 0x01, 0x08, 0x00, "750000"                 },
};

STDDIPINFO(Ckongmc)

static struct BurnDIPInfo KongDIPList[]=
{
	// Default Values
	{0x08, 0xff, 0xff, 0x00, NULL                     },
	{0x09, 0xff, 0xff, 0x00, NULL                     },
	{0x0a, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x08, 0x01, 0x20, 0x00, "Off"                    },
	{0x08, 0x01, 0x20, 0x20, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "99 Men/Max Timer"       },
	{0x08, 0x01, 0x40, 0x00, "Off"                    },
	{0x08, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x09, 0x01, 0x40, 0x00, "3"                      },
	{0x09, 0x01, 0x40, 0x40, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x09, 0x01, 0x80, 0x00, "1 Coin  1 Credit"       },
	{0x09, 0x01, 0x80, 0x80, "2 Coins 1 Credit"       },
		
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0a, 0x01, 0x03, 0x00, "10000"                  },
	{0x0a, 0x01, 0x03, 0x01, "20000"                  },
	{0x0a, 0x01, 0x03, 0x02, "30000"                  },
	{0x0a, 0x01, 0x03, 0x03, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Mode"                   },
	{0x0a, 0x01, 0x04, 0x04, "Tournament (Harder)"    },
	{0x0a, 0x01, 0x04, 0x00, "Normal"                 },
};

STDDIPINFO(Kong)

static struct BurnDIPInfo LadybuggDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
		
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x40, 0x00, "Upright"                },
	{0x10, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x10, 0x01, 0x80, 0x00, "A 1C/1P  B 1C/5P"       },
	{0x10, 0x01, 0x80, 0x80, "A 2C/1P  B 1C/3P"       },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x11, 0x01, 0x03, 0x00, "Easiest"                },
	{0x11, 0x01, 0x03, 0x01, "Easy"                   },
	{0x11, 0x01, 0x03, 0x02, "Hard"                   },
	{0x11, 0x01, 0x03, 0x03, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x04, 0x00, "3"                      },
	{0x11, 0x01, 0x04, 0x04, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x11, 0x01, 0x08, 0x00, "20000"                  },
	{0x11, 0x01, 0x08, 0x08, "40000"                  },
};

STDDIPINFO(Ladybugg)

static struct BurnDIPInfo LeversDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x0f, 0x01, 0x08, 0x00, "Off"                    },
	{0x0f, 0x01, 0x08, 0x08, "On"                     },
};

STDDIPINFO(Levers)

static struct BurnDIPInfo LosttombDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x02, NULL                     },
	{0x10, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x03, 0x02, "3"                      },
	{0x0f, 0x01, 0x03, 0x01, "5"                      },
	{0x0f, 0x01, 0x03, 0x00, "Free Play"              },
	{0x0f, 0x01, 0x03, 0x03, "Invulnerability"        },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x06, 0x04, "1 Coin 1 Play"          },
	{0x10, 0x01, 0x06, 0x06, "A 1C/2P  B 2C/1P"       },
	{0x10, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P"       },
	{0x10, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P"       },
};

STDDIPINFO(Losttomb)

static struct BurnDIPInfo LuctodayDIPList[]=
{
	// Default Values
	{0x07, 0xff, 0xff, 0x00, NULL                     },
	{0x08, 0xff, 0xff, 0x00, NULL                     },
	{0x09, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	
	// Dip 3	
};

STDDIPINFO(Luctoday)

static struct BurnDIPInfo MarsDIPList[]=
{
	// Default Values
	{0x16, 0xff, 0xff, 0x00, NULL                     },
	{0x17, 0xff, 0xff, 0x03, NULL                     },
	{0x18, 0xff, 0xff, 0x0e, NULL                     },
	{0x19, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x17, 0x01, 0x02, 0x00, "2 Coins 1 Play"         },
	{0x17, 0x01, 0x02, 0x02, "1 Coin 1 Play"          },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x17, 0x01, 0x01, 0x01, "1 Coin 3 Plays"         },
	{0x17, 0x01, 0x01, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x18, 0x01, 0x02, 0x02, "Upright"                },
	{0x18, 0x01, 0x02, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x18, 0x01, 0x08, 0x08, "3"                      },
	{0x18, 0x01, 0x08, 0x00, "255"                    },
};

STDDIPINFO(Mars)

static struct BurnDIPInfo MimonkeyDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x03, NULL                     },
	{0x13, 0xff, 0xff, 0xae, NULL                     },
	
	// Dip 1
	
	// Dip 2	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },
	{0x12, 0x01, 0x03, 0x00, "6"                      },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x13, 0x01, 0x06, 0x00, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x06, 0x04, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x06, 0x06, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x06, 0x02, "1 Coin  2 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x08, 0x08, "Upright"                },
	{0x13, 0x01, 0x08, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Infinite Lives"         },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
};

STDDIPINFO(Mimonkey)

static struct BurnDIPInfo MimonscoDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x01, NULL                     },
	{0x13, 0xff, 0xff, 0xae, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "1"                      },
	{0x12, 0x01, 0x03, 0x02, "2"                      },
	{0x12, 0x01, 0x03, 0x01, "3"                      },
	{0x12, 0x01, 0x03, 0x00, "4"                      },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x13, 0x01, 0x06, 0x00, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x06, 0x04, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x06, 0x06, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x06, 0x02, "1 Coin  2 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x08, 0x08, "Upright"                },
	{0x13, 0x01, 0x08, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Infinite Lives"         },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
};

STDDIPINFO(Mimonsco)

static struct BurnDIPInfo MinefldDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x02, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Starting Level"         },
	{0x0e, 0x01, 0x02, 0x00, "1"                      },
	{0x0e, 0x01, 0x02, 0x02, "3"                      },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0f, 0x01, 0x01, 0x00, "3"                      },
	{0x0f, 0x01, 0x01, 0x01, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x02, 0x00, "Off"                    },
	{0x0f, 0x01, 0x02, 0x02, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x10, 0x01, 0x02, 0x00, "1 Coin 1 Play"          },
	{0x10, 0x01, 0x02, 0x02, "A 1C/2P  B 2C/1P"       },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0x0c, 0x00, "Easy"                   },
	{0x10, 0x01, 0x0c, 0x04, "Medium"                 },
	{0x10, 0x01, 0x0c, 0x08, "Hard"                   },
	{0x10, 0x01, 0x0c, 0x0c, "Hardest"                },
};

STDDIPINFO(Minefld)

static struct BurnDIPInfo Moonal2DIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x03, 0x01, "4000"                   },
	{0x0e, 0x01, 0x03, 0x02, "5000"                   },
	{0x0e, 0x01, 0x03, 0x03, "7000"                   },
	{0x0e, 0x01, 0x03, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "3"                      },
	{0x0e, 0x01, 0x04, 0x04, "5"                      },
};

STDDIPINFO(Moonal2)

static struct BurnDIPInfo MooncrgxDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0xc0, NULL                     },
	{0x0d, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0c, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0c, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0c, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x0c, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },	
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0d, 0x01, 0x01, 0x01, "30000"                  },
	{0x0d, 0x01, 0x01, 0x00, "50000"                  },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x08, 0x00, "Upright"                },
	{0x0d, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Mooncrgx)

static struct BurnDIPInfo MooncrsaDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x80, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0b, 0x01, 0x20, 0x00, "Upright"                },
	{0x0b, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0c, 0x01, 0x40, 0x00, "30000"                  },
	{0x0c, 0x01, 0x40, 0x40, "50000"                  },
	
	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x0c, 0x01, 0x80, 0x80, "English"                },
	{0x0c, 0x01, 0x80, 0x00, "Japanese"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0x0c, 0x00, "1 Coin 1 Play"          },
	{0x0d, 0x01, 0x0c, 0x04, "1 Coin 2 Plays"         },
	{0x0d, 0x01, 0x0c, 0x08, "1 Coin 3 Plays"         },
	{0x0d, 0x01, 0x0c, 0x0c, "Freeplay"               },
};

STDDIPINFO(Mooncrsa)

static struct BurnDIPInfo MooncrsgDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0b, 0x01, 0x20, 0x00, "Upright"                },
	{0x0b, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0c, 0x01, 0x40, 0x00, "30000"                  },
	{0x0c, 0x01, 0x40, 0x40, "50000"                  },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0d, 0x01, 0x03, 0x03, "4 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x02, "3 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x01, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0d, 0x01, 0x0c, 0x00, "1 Coin 1 Play"          },
	{0x0d, 0x01, 0x0c, 0x04, "1 Coin 2 Plays"         },
	{0x0d, 0x01, 0x0c, 0x08, "1 Coin 3 Plays"         },
	{0x0d, 0x01, 0x0c, 0x0c, "Freeplay"               },
};

STDDIPINFO(Mooncrsg)

static struct BurnDIPInfo MooncrstDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x80, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0b, 0x01, 0x20, 0x00, "Upright"                },
	{0x0b, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0c, 0x01, 0x40, 0x00, "30000"                  },
	{0x0c, 0x01, 0x40, 0x40, "50000"                  },
	
	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x0c, 0x01, 0x80, 0x80, "English"                },
	{0x0c, 0x01, 0x80, 0x00, "Japanese"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0d, 0x01, 0x03, 0x03, "4 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x02, "3 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x01, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0d, 0x01, 0x0c, 0x00, "1 Coin 1 Play"          },
	{0x0d, 0x01, 0x0c, 0x04, "1 Coin 2 Plays"         },
	{0x0d, 0x01, 0x0c, 0x08, "1 Coin 3 Plays"         },
	{0x0d, 0x01, 0x0c, 0x0c, "Freeplay"               },
};

STDDIPINFO(Mooncrst)

static struct BurnDIPInfo MoonqsrDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0b, 0x01, 0x20, 0x00, "Upright"                },
	{0x0b, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x0c, 0x01, 0xc0, 0x00, "Easy"                   },
	{0x0c, 0x01, 0xc0, 0x40, "Medium"                 },
	{0x0c, 0x01, 0xc0, 0x80, "Hard"                   },
	{0x0c, 0x01, 0xc0, 0xc0, "Hardest"                },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0d, 0x01, 0x03, 0x03, "4 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x02, "3 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x01, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0d, 0x01, 0x0c, 0x00, "1 Coin 1 Play"          },
	{0x0d, 0x01, 0x0c, 0x04, "1 Coin 2 Plays"         },
	{0x0d, 0x01, 0x0c, 0x08, "1 Coin 3 Plays"         },
	{0x0d, 0x01, 0x0c, 0x0c, "Freeplay"               },
};

STDDIPINFO(Moonqsr)

static struct BurnDIPInfo MoonwarDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x03, NULL                     },
	{0x0f, 0xff, 0xff, 0x0c, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x03, 0x03, "3"                      },
	{0x0e, 0x01, 0x03, 0x02, "4"                      },
	{0x0e, 0x01, 0x03, 0x01, "5"                      },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"              },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0f, 0x01, 0x06, 0x04, "1 Coin 1 Play"          },
	{0x0f, 0x01, 0x06, 0x06, "1 Coin 2 Plays"         },
	{0x0f, 0x01, 0x06, 0x02, "1 Coin 3 Plays"         },
	{0x0f, 0x01, 0x06, 0x00, "1 Coin 4 Plays"         },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x08, 0x08, "Upright"                },
	{0x0f, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Moonwar)

static struct BurnDIPInfo MoonwaraDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x03, NULL                     },
	{0x0f, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x03, 0x03, "3"                      },
	{0x0e, 0x01, 0x03, 0x02, "4"                      },
	{0x0e, 0x01, 0x03, 0x01, "5"                      },
	{0x0e, 0x01, 0x03, 0x00, "Free Play"              },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0f, 0x01, 0x06, 0x06, "1 Coin 1 Play"          },
	{0x0f, 0x01, 0x06, 0x04, "1 Coin 2 Plays"         },
	{0x0f, 0x01, 0x06, 0x02, "1 Coin 3 Plays"         },
	{0x0f, 0x01, 0x06, 0x00, "1 Coin 4 Plays"         },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x08, 0x08, "Upright"                },
	{0x0f, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Moonwara)

static struct BurnDIPInfo MrkougarDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x03, NULL                     },
	{0x0e, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
		
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0d, 0x01, 0x02, 0x00, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x02, 0x02, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0d, 0x01, 0x01, 0x01, "1 Coin  3 Plays"        },
	{0x0d, 0x01, 0x01, 0x00, "1 Coin  5 Plays"        },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0e, 0x01, 0x02, 0x02, "Upright"                },
	{0x0e, 0x01, 0x02, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x04, "3"                      },
	{0x0e, 0x01, 0x04, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0e, 0x01, 0x08, 0x00, "Easy"                   },
	{0x0e, 0x01, 0x08, 0x08, "Hard"                   },
};

STDDIPINFO(Mrkougar)

static struct BurnDIPInfo MshuttleDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x80, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0e, 0x01, 0x40, 0x00, "Off"                    },
	{0x0e, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0e, 0x01, 0x80, 0x80, "Upright"                },
	{0x0e, 0x01, 0x80, 0x00, "Cocktail"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x03, 0x00, "3"                      },
	{0x0f, 0x01, 0x03, 0x01, "4"                      },
	{0x0f, 0x01, 0x03, 0x02, "5"                      },
	{0x0f, 0x01, 0x03, 0x03, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0f, 0x01, 0x04, 0x00, "20000"                  },
	{0x0f, 0x01, 0x04, 0x04, "30000"                  },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0e, 0x01, 0x18, 0x18, "3 Coins 1 Play"         },
	{0x0e, 0x01, 0x18, 0x10, "3 Coins 1 Play"         },
	{0x0e, 0x01, 0x18, 0x08, "2 Coins 1 Play"         },
	{0x0e, 0x01, 0x18, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0e, 0x01, 0x60, 0x00, "1 Coin 1 Play"          },
	{0x0e, 0x01, 0x60, 0x20, "1 Coin 2 Plays"         },
	{0x0e, 0x01, 0x60, 0x40, "1 Coin 3 Plays"         },
	{0x0e, 0x01, 0x60, 0x60, "1 Coin 4 Plays"         },
};

STDDIPINFO(Mshuttle)

static struct BurnDIPInfo Newsin7DIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x0c, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x03, 0x00, "A 1C/1P  B 2C/1P"       },
	{0x10, 0x01, 0x03, 0x02, "A 1C/3P  B 3C/1P"       },
	{0x10, 0x01, 0x03, 0x01, "A 1C/2P  B 1C/1P"       },
	{0x10, 0x01, 0x03, 0x03, "A 1C/4P  B 4C/1P"       },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x02, 0x00, "Upright"                },
	{0x11, 0x01, 0x02, 0x02, "Cocktail"               },	
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x08, 0x08, "3"                      },
	{0x11, 0x01, 0x08, 0x00, "5"                      },
};

STDDIPINFO(Newsin7)

static struct BurnDIPInfo OmegaDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0xc0, NULL                     },
	{0x0d, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0c, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0c, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0c, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x0c, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0d, 0x01, 0x01, 0x00, "10000"                  },
	{0x0d, 0x01, 0x01, 0x01, "20000"                  },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0d, 0x01, 0x04, 0x00, "3"                      },
	{0x0d, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x08, 0x00, "Upright"                },
	{0x0d, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Omega)

static struct BurnDIPInfo OrbitronDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x40, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x10, 0x01, 0x60, 0x00, "A 2C/1P  B 1C/3P"       },
	{0x10, 0x01, 0x60, 0x40, "A 1C/1P  B 1C/6P"       },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x04, 0x04, "2"                      },
	{0x11, 0x01, 0x04, 0x00, "3"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x08, 0x00, "Upright"                },
	{0x11, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Orbitron)

static struct BurnDIPInfo Ozon1DIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x02, NULL                     },
	{0x0d, 0xff, 0xff, 0x06, NULL                     },
	
	// Dip 1

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0c, 0x01, 0x03, 0x03, "3"                      },
	{0x0c, 0x01, 0x03, 0x02, "4"                      },
	{0x0c, 0x01, 0x03, 0x01, "5"                      },
	{0x0c, 0x01, 0x03, 0x00, "6"                      },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0x06, 0x06, "A 1C/1P  B 2C/1P"       },
	{0x0d, 0x01, 0x06, 0x04, "A 1C/2P  B 1C/1P"       },
	{0x0d, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P"       },
	{0x0d, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P"       },
};

STDDIPINFO(Ozon1)

static struct BurnDIPInfo PacmanblDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0xc0, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0e, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0e, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0e, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x0e, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0f, 0x01, 0x01, 0x00, "15000"                  },
	{0x0f, 0x01, 0x01, 0x01, "20000"                  },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0f, 0x01, 0x02, 0x00, "Easy"                   },
	{0x0f, 0x01, 0x02, 0x02, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0f, 0x01, 0x04, 0x00, "3"                      },
	{0x0f, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x08, 0x00, "Upright"                },
	{0x0f, 0x01, 0x08, 0x08, "Cocktail"               },	
};

STDDIPINFO(Pacmanbl)

static struct BurnDIPInfo Phoenxp2DIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0xc0, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x10, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x10, 0x01, 0x80, 0x80, "1 Coin 3 Plays"         },
	{0x10, 0x01, 0x80, 0x00, "1 Coin 5 Plays"         },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x11, 0x01, 0x02, 0x00, "Easy"                   },
	{0x11, 0x01, 0x02, 0x02, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x04, 0x00, "3"                      },
	{0x11, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x08, 0x00, "Upright"                },
	{0x11, 0x01, 0x08, 0x08, "Cocktail"               },	
};

STDDIPINFO(Phoenxp2)

static struct BurnDIPInfo PiscesDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0c, 0x01, 0x40, 0x00, "3"                      },
	{0x0c, 0x01, 0x40, 0x40, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x80, 0x00, "Upright"                },
	{0x0c, 0x01, 0x80, 0x80, "Cocktail"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0d, 0x01, 0x01, 0x00, "10000"                  },
	{0x0d, 0x01, 0x01, 0x01, "20000"                  },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0x02, 0x02, "A 2C/1P  B 1C/3P"       },
	{0x0d, 0x01, 0x02, 0x00, "A 1C/1P  B 1C/6P"       },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0d, 0x01, 0x04, 0x00, "Easy"                   },
	{0x0d, 0x01, 0x04, 0x04, "Hard"                   },
};

STDDIPINFO(Pisces)

static struct BurnDIPInfo PiscesbDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0c, 0x01, 0x40, 0x00, "3"                      },
	{0x0c, 0x01, 0x40, 0x40, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x80, 0x00, "Upright"                },
	{0x0c, 0x01, 0x80, 0x80, "Cocktail"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0d, 0x01, 0x01, 0x00, "10000"                  },
	{0x0d, 0x01, 0x01, 0x01, "20000"                  },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0x02, 0x02, "A 2C/1P  B 1C/2P 2C/5P" },
	{0x0d, 0x01, 0x02, 0x00, "A 1C/1P  B 1C/5P"       },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0d, 0x01, 0x04, 0x00, "Easy"                   },
	{0x0d, 0x01, 0x04, 0x04, "Hard"                   },
};

STDDIPINFO(Piscesb)

static struct BurnDIPInfo PorterDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	
	// Dip 3
};

STDDIPINFO(Porter)

static struct BurnDIPInfo RacknrolDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0x00, NULL                     },
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
		
	// Dip 2
	
	// Dip 3
};

STDDIPINFO(Racknrol)

static struct BurnDIPInfo RedufoDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0xc0, NULL                     },
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Coin A"                 },
	{0x0d, 0x01, 0x40, 0x00, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x40, 0x40, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Coin B"                 },
	{0x0d, 0x01, 0x80, 0x80, "1 Coin  3 Plays"        },
	{0x0d, 0x01, 0x80, 0x00, "1 Coin  5 Plays"        },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0e, 0x01, 0x01, 0x00, "7000"                   },
	{0x0e, 0x01, 0x01, 0x01, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "3"                      },
	{0x0e, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0e, 0x01, 0x08, 0x00, "Upright"                },
	{0x0e, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Redufo)

static struct BurnDIPInfo RedufobDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0x40, "A 2C/1P  B 1C/3P"       },
	{0x0d, 0x01, 0xc0, 0x00, "A 1C/1P  B 1C/6P"       },
	{0x0d, 0x01, 0xc0, 0x80, "A 1C/2P  B 1C/12P"      },
	{0x0d, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x03, 0x01, "4000"                   },
	{0x0e, 0x01, 0x03, 0x02, "5000"                   },
	{0x0e, 0x01, 0x03, 0x03, "7000"                   },
	{0x0e, 0x01, 0x03, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "3"                      },
	{0x0e, 0x01, 0x04, 0x04, "5"                      },
};

STDDIPINFO(Redufob)

static struct BurnDIPInfo RescueDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x02, NULL                     },
	{0x10, 0xff, 0xff, 0x0c, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Starting Level"         },
	{0x0e, 0x01, 0x02, 0x00, "1"                      },
	{0x0e, 0x01, 0x02, 0x02, "3"                      },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0f, 0x01, 0x01, 0x00, "3"                      },
	{0x0f, 0x01, 0x01, 0x01, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x02, 0x00, "Off"                    },
	{0x0f, 0x01, 0x02, 0x02, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x06, 0x04, "1 Coin 1 Play"          },
	{0x10, 0x01, 0x06, 0x06, "A 1C/2P  B 2C/1P"       },
	{0x10, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P"       },
	{0x10, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P"       },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x10, 0x01, 0x08, 0x08, "Easy"                   },
	{0x10, 0x01, 0x08, 0x00, "Hard"                   },	
};

STDDIPINFO(Rescue)

static struct BurnDIPInfo RockclimDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0x40, NULL                     },
	{0x0e, 0xff, 0xff, 0xc0, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x0e, 0x01, 0x40, 0x00, "30000"                  },
	{0x0e, 0x01, 0x40, 0x40, "50000"                  },
	
	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x0e, 0x01, 0x80, 0x80, "1"                      },
	{0x0e, 0x01, 0x80, 0x00, "2"                      },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x03, 0x00, "3"                      },
	{0x0f, 0x01, 0x03, 0x01, "4"                      },
	{0x0f, 0x01, 0x03, 0x02, "5"                      },
	{0x0f, 0x01, 0x03, 0x03, "6"                      },
	
	// Dip 4
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x10, 0x01, 0x0f, 0x00, "9 Coins 1 Play"         },
	{0x10, 0x01, 0x0f, 0x01, "8 Coins 1 Play"         },
	{0x10, 0x01, 0x0f, 0x02, "7 Coins 1 Play"         },
	{0x10, 0x01, 0x0f, 0x03, "6 Coins 1 Play"         },
	{0x10, 0x01, 0x0f, 0x04, "5 Coins 1 Play"         },
	{0x10, 0x01, 0x0f, 0x05, "4 Coins 1 Play"         },
	{0x10, 0x01, 0x0f, 0x06, "3 Coins 1 Play"         },
	{0x10, 0x01, 0x0f, 0x07, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x0f, 0x0f, "1 Coin  1 Play"         },
	{0x10, 0x01, 0x0f, 0x0e, "1 Coin  2 Plays"        },
	{0x10, 0x01, 0x0f, 0x0d, "1 Coin  3 Plays"        },
	{0x10, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x10, 0x01, 0x0f, 0x0b, "1 Coin  5 Plays"        },
	{0x10, 0x01, 0x0f, 0x0a, "1 Coin  6 Plays"        },
	{0x10, 0x01, 0x0f, 0x09, "1 Coin  7 Plays"        },
	{0x10, 0x01, 0x0f, 0x08, "1 Coin  8 Plays"        },
	
	{0   , 0xfe, 0   , 16  , "Coin B"                 },
	{0x10, 0x01, 0xf0, 0x00, "9 Coins 1 Play"         },
	{0x10, 0x01, 0xf0, 0x10, "8 Coins 1 Play"         },
	{0x10, 0x01, 0xf0, 0x20, "7 Coins 1 Play"         },
	{0x10, 0x01, 0xf0, 0x30, "6 Coins 1 Play"         },
	{0x10, 0x01, 0xf0, 0x40, "5 Coins 1 Play"         },
	{0x10, 0x01, 0xf0, 0x50, "4 Coins 1 Play"         },
	{0x10, 0x01, 0xf0, 0x60, "3 Coins 1 Play"         },
	{0x10, 0x01, 0xf0, 0x70, "2 Coins 1 Play"         },
	{0x10, 0x01, 0xf0, 0xf0, "1 Coin  1 Play"         },
	{0x10, 0x01, 0xf0, 0xe0, "1 Coin  2 Plays"        },
	{0x10, 0x01, 0xf0, 0xd0, "1 Coin  3 Plays"        },
	{0x10, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
	{0x10, 0x01, 0xf0, 0xb0, "1 Coin  5 Plays"        },
	{0x10, 0x01, 0xf0, 0xa0, "1 Coin  6 Plays"        },
	{0x10, 0x01, 0xf0, 0x90, "1 Coin  7 Plays"        },
	{0x10, 0x01, 0xf0, 0x80, "1 Coin  8 Plays"        },
};

STDDIPINFO(Rockclim)

static struct BurnDIPInfo ScobraDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x03, NULL                     },
	{0x14, 0xff, 0xff, 0x0c, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x13, 0x01, 0x02, 0x02, "3"                      },
	{0x13, 0x01, 0x02, 0x00, "4"                      },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x14, 0x01, 0x06, 0x02, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x06, 0x00, "4 Coins 3 Plays"        },
	{0x14, 0x01, 0x06, 0x04, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x06, 0x06, "1 Coin 99 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x08, 0x08, "Upright"                },
	{0x14, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Scobra)

static struct BurnDIPInfo ScobrasDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x03, NULL                     },
	{0x14, 0xff, 0xff, 0x0c, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x13, 0x01, 0x02, 0x02, "3"                      },
	{0x13, 0x01, 0x02, 0x00, "5"                      },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x14, 0x01, 0x06, 0x02, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x06, 0x00, "4 Coins 3 Plays"        },
	{0x14, 0x01, 0x06, 0x04, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x06, 0x06, "1 Coin 99 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x08, 0x08, "Upright"                },
	{0x14, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Scobras)

static struct BurnDIPInfo ScorpionDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	
	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x04, 0x00, "3"                      },
	{0x12, 0x01, 0x04, 0x01, "4"                      },
	{0x12, 0x01, 0x04, 0x02, "5"                      },
	{0x12, 0x01, 0x04, 0x03, "255"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x04, 0x04, "Upright"                },
	{0x13, 0x01, 0x04, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Coinage"                },
	{0x13, 0x01, 0x08, 0x00, "A 1C/1P  B 1C/1P"       },
	{0x13, 0x01, 0x08, 0x08, "A 1C/1P  B 1C/3P"       },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0xa0, 0x00, "Normal"                 },
	{0x13, 0x01, 0xa0, 0x20, "Medium"                 },
	{0x13, 0x01, 0xa0, 0x80, "Hard"                   },
	{0x13, 0x01, 0xa0, 0xa0, "Very Hard"              },
};

STDDIPINFO(Scorpion)

static struct BurnDIPInfo ScorpionmcDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x40, NULL                     },
	{0x11, 0xff, 0xff, 0x04, NULL                     },
	{0x12, 0xff, 0xff, 0x02, NULL                     },
	
	// Dip 1
	
	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0xc0, 0x00, "Easy"                   },
	{0x10, 0x01, 0xc0, 0x40, "Normal"                 },
	{0x10, 0x01, 0xc0, 0x80, "Hard"                   },
	{0x10, 0x01, 0xc0, 0xc0, "Hardest"                },
	
	// Dip 3
	{0   , 0xfe, 0   , 3   , "Lives"                  },
	{0x11, 0x01, 0x0c, 0x00, "2"                      },
	{0x11, 0x01, 0x0c, 0x04, "3"                      },
	{0x11, 0x01, 0x0c, 0x08, "4"                      },
	
	// Dip 4
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x01, 0x00, "Upright"                },
	{0x12, 0x01, 0x01, 0x01, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x12, 0x01, 0x06, 0x00, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x06, 0x02, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x06, 0x04, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0x06, 0x06, "1 Coin  3 Plays"        },
};

STDDIPINFO(Scorpionmc)

static struct BurnDIPInfo AracnisDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x20, NULL                     },
	{0x0c, 0xff, 0xff, 0xf8, NULL                     },
	{0x0d, 0xff, 0xff, 0xfd, NULL                     },
	{0x0e, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	
	// Dip 2
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0x03, 0x00, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0x03, 0x01, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0x03, 0x02, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0x03, 0x03, "1 Coin  3 Plays"        },
	
	// Dip 4
};

STDDIPINFO(Aracnis)

static struct BurnDIPInfo Scramb2DIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x03, NULL                     },
	{0x13, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },
	{0x12, 0x01, 0x03, 0x00, "255"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x13, 0x01, 0x06, 0x06, "1 Coin 1 Play"          },
	{0x13, 0x01, 0x06, 0x04, "1 Coin 2 Plays"         },
	{0x13, 0x01, 0x06, 0x02, "1 Coin 3 Plays"         },
	{0x13, 0x01, 0x06, 0x00, "1 Coin 4 Plays"         },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x08, 0x08, "Upright"                },
	{0x13, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Scramb2)

static struct BurnDIPInfo ScramblbDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x40, 0x00, "Upright"                },
	{0x11, 0x01, 0x40, 0x40, "Cocktail"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x12, 0x01, 0x03, 0x00, "1 Coin 1 Play"          },
	{0x12, 0x01, 0x03, 0x01, "1 Coin 2 Plays"         },
	{0x12, 0x01, 0x03, 0x02, "1 Coin 3 Plays"         },
	{0x12, 0x01, 0x03, 0x03, "1 Coin 4 Plays"         },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x0c, 0x00, "3"                      },
	{0x12, 0x01, 0x0c, 0x04, "4"                      },
	{0x12, 0x01, 0x0c, 0x08, "5"                      },
	{0x12, 0x01, 0x0c, 0x0c, "255"                    },
};

STDDIPINFO(Scramblb)

static struct BurnDIPInfo ScrambleDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x03, NULL                     },
	{0x14, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x03, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x01, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "255"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x14, 0x01, 0x06, 0x06, "A 1C/1P  B 2C/1P  C 1C/1P"},
	{0x14, 0x01, 0x06, 0x04, "A 1C/2P  B 1C/1P  C 1C/2P"},
	{0x14, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P  C 1C/3P"},
	{0x14, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P  C 1C/4P"},
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x08, 0x08, "Upright"                },
	{0x14, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Scramble)

static struct BurnDIPInfo SfxDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x01, NULL                     },
	{0x14, 0xff, 0xff, 0x08, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x01, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x03, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "Invulerability"         },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x14, 0x01, 0x06, 0x06, "3 Coins 1 Play"         },
	{0x14, 0x01, 0x06, 0x04, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x06, 0x00, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x06, 0x02, "1 Coin  2 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x08, 0x08, "Upright"                },
	{0x14, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Sfx)

static struct BurnDIPInfo SkybaseDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin 1 Play (2 to start)"},
	{0x0d, 0x01, 0xc0, 0x00, "1 Coin 1 Play"          },
	{0x0d, 0x01, 0xc0, 0x40, "1 Coin 2 Plays"         },
	{0x0d, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x03, 0x00, "3"                      },
	{0x0e, 0x01, 0x03, 0x01, "4"                      },
	{0x0e, 0x01, 0x03, 0x02, "5"                      },
	{0x0e, 0x01, 0x03, 0x03, "Infinite"               },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x0c, 0x00, "10000"                  },
	{0x0e, 0x01, 0x0c, 0x40, "15000"                  },
	{0x0e, 0x01, 0x0c, 0x40, "20000"                  },
	{0x0e, 0x01, 0x0c, 0x40, "None"                   },
};

STDDIPINFO(Skybase)

static struct BurnDIPInfo SmooncrsDIPList[]=
{
	// Default Values
	{0x08, 0xff, 0xff, 0x00, NULL                     },
	{0x09, 0xff, 0xff, 0xc0, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x08, 0x01, 0x20, 0x00, "Upright"                },
	{0x08, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x09, 0x01, 0x40, 0x40, "Easy"                   },
	{0x09, 0x01, 0x40, 0x00, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Players Bullet Speed"   },
	{0x09, 0x01, 0x80, 0x00, "Slow"                   },
	{0x09, 0x01, 0x80, 0x80, "Fast"                   },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x10, 0x01, 0x03, 0x03, "4 Coins 1 Play"         },
	{0x10, 0x01, 0x03, 0x02, "3 Coins 1 Play"         },
	{0x10, 0x01, 0x03, 0x01, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x03, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x10, 0x01, 0x0c, 0x00, "1 Coin 1 Play"          },
	{0x10, 0x01, 0x0c, 0x04, "1 Coin 2 Plays"         },
	{0x10, 0x01, 0x0c, 0x08, "1 Coin 3 Plays"         },
	{0x10, 0x01, 0x0c, 0x0c, "Freeplay"               },
};

STDDIPINFO(Smooncrs)

static struct BurnDIPInfo SpacbattDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0x40, "A 2C/1P  B 1C/3P"       },
	{0x0d, 0x01, 0xc0, 0x00, "A 1C/1P  B 1C/6P"       },
	{0x0d, 0x01, 0xc0, 0x80, "A 1C/2P  B 1C/6P"       },
	{0x0d, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x03, 0x01, "4000"                   },
	{0x0e, 0x01, 0x03, 0x02, "5000"                   },
	{0x0e, 0x01, 0x03, 0x03, "7000"                   },
	{0x0e, 0x01, 0x03, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "3"                      },
	{0x0e, 0x01, 0x04, 0x04, "5"                      },
};

STDDIPINFO(Spacbatt)

static struct BurnDIPInfo SpdcoinDIPList[]=
{
	// Default Values
	{0x07, 0xff, 0xff, 0x00, NULL                     },
	{0x08, 0xff, 0xff, 0x03, NULL                     },
	{0x09, 0xff, 0xff, 0x06, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x08, 0x01, 0x01, 0x01, "Off"                    },
	{0x08, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x08, 0x01, 0x02, 0x02, "Off"                    },
	{0x08, 0x01, 0x02, 0x00, "On"                     },	
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x09, 0x01, 0x04, 0x04, "Easy"                   },
	{0x09, 0x01, 0x04, 0x00, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x09, 0x01, 0x08, 0x00, "3"                      },
	{0x09, 0x01, 0x08, 0x08, "5"                      },
};

STDDIPINFO(Spdcoin)

static struct BurnDIPInfo StratgyxDIPList[] =
{
	// Default Values
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x03, NULL                     },
	{0x14, 0xff, 0xff, 0x08, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x03, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x01, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "255"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x08, 0x08, "Upright"                },
	{0x14, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Stratgyx)

static struct BurnDIPInfo StreakngDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x40, NULL                     },
	{0x0e, 0xff, 0xff, 0x06, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0d, 0x01, 0xc0, 0x40, "10000"                  },
	{0x0d, 0x01, 0xc0, 0x81, "15000"                  },
	{0x0d, 0x01, 0xc0, 0xc0, "20000"                  },
	{0x0d, 0x01, 0xc0, 0x00, "None"                   },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0e, 0x01, 0x03, 0x00, "2 Coins 1 Play"         },
	{0x0e, 0x01, 0x03, 0x02, "1 Coin  1 Play"         },
	{0x0e, 0x01, 0x03, 0x01, "1 Coin  2 Plays"        },
	{0x0e, 0x01, 0x03, 0x03, "Freeplay"               },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0x0c, 0x0c, "1"                      },
	{0x0e, 0x01, 0x0c, 0x08, "2"                      },
	{0x0e, 0x01, 0x0c, 0x04, "3"                      },
	{0x0e, 0x01, 0x0c, 0x00, "5"                      },
};

STDDIPINFO(Streakng)

static struct BurnDIPInfo StrfbombDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x03, NULL                     },
	{0x14, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x03, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x01, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "255"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x14, 0x01, 0x06, 0x06, "A 1C/2P  B 4C/1P  C 1C/2P"},
	{0x14, 0x01, 0x06, 0x04, "A 1C/3P  B 2C/1P  C 1C/3P"},
	{0x14, 0x01, 0x06, 0x02, "A 1C/4P  B 3C/1P  C 1C/4P"},
	{0x14, 0x01, 0x06, 0x00, "A 1C/5P  B 1C/1P  C 1C/5P"},
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x08, 0x08, "Upright"                },
	{0x14, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Strfbomb)

static struct BurnDIPInfo SuperbonDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0x00, NULL                     },
	{0x0c, 0xff, 0xff, 0x02, NULL                     },
	{0x0d, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0c, 0x01, 0x03, 0x02, "3"                      },
	{0x0c, 0x01, 0x03, 0x01, "5"                      },
	{0x0c, 0x01, 0x03, 0x00, "Free Play"              },
	{0x0c, 0x01, 0x03, 0x03, "Invulnerability"        },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0x06, 0x04, "1 Coin 1 Play"          },
	{0x0d, 0x01, 0x06, 0x06, "A 1C/2P  B 2C/1P"       },
	{0x0d, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P"       },
	{0x0d, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P"       },
};

STDDIPINFO(Superbon)

static struct BurnDIPInfo SupergDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x01, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x03, 0x01, "4000"                   },
	{0x0e, 0x01, 0x03, 0x02, "5000"                   },
	{0x0e, 0x01, 0x03, 0x03, "7000"                   },
	{0x0e, 0x01, 0x03, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "3"                      },
	{0x0e, 0x01, 0x04, 0x04, "5"                      },
};

STDDIPINFO(Superg)

static struct BurnDIPInfo SwarmDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x00, NULL                     },
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0d, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	{0x0d, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x03, 0x01, "10000"                  },
	{0x0e, 0x01, 0x03, 0x02, "20000"                  },
	{0x0e, 0x01, 0x03, 0x03, "40000"                  },
	{0x0e, 0x01, 0x03, 0x00, "None"                   },	
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "3"                      },
	{0x0e, 0x01, 0x04, 0x04, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0e, 0x01, 0x08, 0x00, "Easy"                   },
	{0x0e, 0x01, 0x08, 0x08, "Hard"                   },
};

STDDIPINFO(Swarm)

static struct BurnDIPInfo TazzmangDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x04, NULL                     },
	
	// Dip 1
	
	// Dip 2

	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x01, 0x00, "Upright"                },
	{0x11, 0x01, 0x01, 0x01, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x11, 0x01, 0x06, 0x00, "A 4C/1P  B 1C/4P"       },
	{0x11, 0x01, 0x06, 0x02, "A 3C/1P  B 1C/3P"       },
	{0x11, 0x01, 0x06, 0x06, "A 2C/1P  B 1C/2P"       },
	{0x11, 0x01, 0x06, 0x04, "A 1C/1P  B 1C/1P"       },	
};

STDDIPINFO(Tazzmang)

static struct BurnDIPInfo TheendDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x03, NULL                     },
	{0x10, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x03, 0x03, "3"                      },
	{0x0f, 0x01, 0x03, 0x02, "4"                      },
	{0x0f, 0x01, 0x03, 0x01, "5"                      },
	{0x0f, 0x01, 0x03, 0x00, "256"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x06, 0x02, "3 Coins 1 Play"         },
	{0x10, 0x01, 0x06, 0x04, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x06, 0x06, "1 Coin  1 Play"         },
	{0x10, 0x01, 0x06, 0x00, "1 Coin  2 Plays"        },	
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x08, 0x08, "Upright"                },
	{0x10, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Theend)

static struct BurnDIPInfo ThepitmDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },
	
	// Dip 1
	
	// Dip 2
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0c, 0x01, 0x04, 0x00, "Off"                    },
	{0x0c, 0x01, 0x04, 0x04, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x08, 0x00, "Upright"                },
	{0x11, 0x01, 0x08, 0x08, "Cocktail"               },
};

STDDIPINFO(Thepitm)

static struct BurnDIPInfo TriplepDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x03, NULL                     },
	{0x12, 0xff, 0xff, 0xae, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x03, "3"                      },
	{0x11, 0x01, 0x03, 0x02, "4"                      },
	{0x11, 0x01, 0x03, 0x01, "5"                      },
	{0x11, 0x01, 0x03, 0x00, "256"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x12, 0x01, 0x06, 0x04, "A 1C/2P  B 1C/1P  C 1C/2C"},
	{0x12, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P  C 1C/3C"},
	{0x12, 0x01, 0x06, 0x06, "A 1C/1P  B 2C/1P  C 1C/1C"},
	{0x12, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P  C 1C/4C"},
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x08, 0x08, "Upright"                },
	{0x12, 0x01, 0x08, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Rack Test"              },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Triplep)

static struct BurnDIPInfo TurpinDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x03, NULL                     },
	{0x12, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x03, "3"                      },
	{0x11, 0x01, 0x03, 0x02, "5"                      },
	{0x11, 0x01, 0x03, 0x01, "7"                      },
	{0x11, 0x01, 0x03, 0x00, "126"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x12, 0x01, 0x06, 0x00, "4 Coins 1 Play"         },	
	{0x12, 0x01, 0x06, 0x04, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x06, 0x06, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x06, 0x02, "1 Coin  2 Plays"        },	
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x08, 0x08, "Upright"                },
	{0x12, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Turpin)

static struct BurnDIPInfo TurtlesDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x03, NULL                     },
	{0x12, 0xff, 0xff, 0x0e, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x03, "3"                      },
	{0x11, 0x01, 0x03, 0x02, "4"                      },
	{0x11, 0x01, 0x03, 0x01, "5"                      },
	{0x11, 0x01, 0x03, 0x00, "126"                    },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x12, 0x01, 0x06, 0x06, "A 1C/1P  B 2C/1P  C 1C/1P"},
	{0x12, 0x01, 0x06, 0x04, "A 1C/2P  B 1C/1P  C 1C/2P"},
	{0x12, 0x01, 0x06, 0x02, "A 1C/3P  B 3C/1P  C 1C/3P"},
	{0x12, 0x01, 0x06, 0x00, "A 1C/4P  B 4C/1P  C 1C/4P"},
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x08, 0x08, "Upright"                },
	{0x12, 0x01, 0x08, 0x00, "Cocktail"               },
};

STDDIPINFO(Turtles)

static struct BurnDIPInfo VpoolDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x20, NULL                     },
	{0x0d, 0xff, 0xff, 0xe0, NULL                     },
	{0x0e, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	
	// Dip 3	
};

STDDIPINFO(Vpool)

static struct BurnDIPInfo WarofbugDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x0a, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x20, 0x00, "Upright"                },
	{0x0f, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 3   , "Coinage"                },
	{0x10, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x10, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x10, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x00, "1"                      },
	{0x11, 0x01, 0x03, 0x01, "2"                      },
	{0x11, 0x01, 0x03, 0x02, "3"                      },
	{0x11, 0x01, 0x03, 0x03, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x11, 0x01, 0x08, 0x08, "500000"                 },
	{0x11, 0x01, 0x08, 0x00, "700000"                 },
};

STDDIPINFO(Warofbug)

static struct BurnDIPInfo ZerotimeDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x00, NULL                     },
	{0x0d, 0xff, 0xff, 0x40, NULL                     },
	{0x0e, 0xff, 0xff, 0x03, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0c, 0x01, 0x20, 0x00, "Upright"                },
	{0x0c, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x40, 0x00, "Off"                    },
	{0x0c, 0x01, 0x40, 0x40, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0d, 0x01, 0xc0, 0x40, "A 1C/1P 2C/3P  B 1C/2P" },
	{0x0d, 0x01, 0xc0, 0xc0, "A 1C/1P 2C/3P  B 1C/3P" },
	{0x0d, 0x01, 0xc0, 0x00, "A 1C/2P 2C/4P  B 1C/4P" },
	{0x0d, 0x01, 0xc0, 0x80, "A 1C/2P 2C/5P  B 1C/5P" },
	
	// Dip 3	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x0e, 0x01, 0x03, 0x03, "6000"                   },
	{0x0e, 0x01, 0x03, 0x02, "7000"                   },
	{0x0e, 0x01, 0x03, 0x01, "9000"                   },
	{0x0e, 0x01, 0x03, 0x00, "None"                   },	
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0e, 0x01, 0x04, 0x00, "3"                      },
	{0x0e, 0x01, 0x04, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x0e, 0x01, 0x08, 0x00, "Easy"                   },
	{0x0e, 0x01, 0x08, 0x08, "Hard"                   },
};

STDDIPINFO(Zerotime)

static struct BurnDIPInfo ZigzagDIPList[]=
{
	// Default Values
	{0x0e, 0xff, 0xff, 0x00, NULL                     },
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x02, NULL                     },
	
	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0f, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	{0x0f, 0x01, 0xc0, 0xc0, "Freeplay"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x10, 0x01, 0x01, 0x00, "3"                      },
	{0x10, 0x01, 0x01, 0x01, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x02, 0x02, "Upright"                },
	{0x10, 0x01, 0x02, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x0c, 0x00, "10000 60000"            },
	{0x10, 0x01, 0x0c, 0x04, "20000 60000"            },
	{0x10, 0x01, 0x0c, 0x08, "30000 60000"            },
	{0x10, 0x01, 0x0c, 0x0c, "40000 60000"            },
};

STDDIPINFO(Zigzag)

// Drivers
// Basic Galaxian Hardware
static struct BurnRomInfo GalaxianRomDesc[] = {
	{ "galmidw.u",     0x00800, 0x745e2d61, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.v",     0x00800, 0x9c999a40, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.w",     0x00800, 0xb5894925, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.y",     0x00800, 0x6b3ca10b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7l",            0x00800, 0x1b933207, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "1h.bin",        0x00800, 0x39fb43a4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "1k.bin",        0x00800, 0x7e3f56a2, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galaxian)
STD_ROM_FN(Galaxian)

static struct BurnRomInfo GalaxianaRomDesc[] = {
	{ "7f.bin",        0x01000, 0x4335b1de, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7j.bin",        0x01000, 0x4e6f66a1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7l.bin",        0x00800, 0x5341d75a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "1h.bin",        0x00800, 0x39fb43a4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "1k.bin",        0x00800, 0x7e3f56a2, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galaxiana)
STD_ROM_FN(Galaxiana)

static struct BurnRomInfo GalaxianmRomDesc[] = {
	{ "galmidw.u",     0x00800, 0x745e2d61, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.v",     0x00800, 0x9c999a40, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.w",     0x00800, 0xb5894925, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.y",     0x00800, 0x6b3ca10b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.z",     0x00800, 0xcb24f797, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "galaxian.j1",   0x00800, 0x84decf98, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "galaxian.l1",   0x00800, 0xc31ada9e, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galaxianm)
STD_ROM_FN(Galaxianm)

static struct BurnRomInfo GalaxianmoRomDesc[] = {
	{ "galaxian.u",    0x00800, 0xfac42d34, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galaxian.v",    0x00800, 0xf58283e3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galaxian.w",    0x00800, 0x4c7031c0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galaxian.y",    0x00800, 0x96a7ac94, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7l.bin",        0x00800, 0x5341d75a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "galaxian.j1",   0x00800, 0x84decf98, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "galaxian.l1",   0x00800, 0xc31ada9e, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galaxianmo)
STD_ROM_FN(Galaxianmo)

static struct BurnRomInfo GalaxiantRomDesc[] = {
	{ "gl-03.8g",      0x00800, 0xe8f3aa67, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "gl-04.8f",      0x00800, 0xf58283e3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "gl-05.8e",      0x00800, 0x4c7031c0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "gl-06.8d",      0x00800, 0x097d92a2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "gl-07.8c",      0x00800, 0x5341d75a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "gl-01.1j",      0x00800, 0x968b6016, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "gl-02.1k",      0x00800, 0xd14f7510, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galaxiant)
STD_ROM_FN(Galaxiant)

struct BurnDriver BurnDrvGalaxian = {
	"galaxian", NULL, NULL, NULL, "1979",
	"Galaxian (Namco set 1)\0", NULL, "Namco", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalaxianRomInfo, GalaxianRomName, NULL, NULL, GalaxianInputInfo, GalaxianDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalaxiana = {
	"galaxiana", "galaxian", NULL, NULL, "1979",
	"Galaxian (Namco set 2)\0", NULL, "Namco", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalaxianaRomInfo, GalaxianaRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalaxianm = {
	"galaxianm", "galaxian", NULL, NULL, "1979",
	"Galaxian (Midway set 1)\0", NULL, "Namco (Midway license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalaxianmRomInfo, GalaxianmRomName, NULL, NULL, GalaxianInputInfo, GalaxianDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalaxianmo = {
	"galaxianmo", "galaxian", NULL, NULL, "1979",
	"Galaxian (Midway set 2)\0", NULL, "Namco (Midway license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalaxianmoRomInfo, GalaxianmoRomName, NULL, NULL, GalaxianInputInfo, GalaxianDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalaxiant = {
	"galaxiant", "galaxian", NULL, NULL, "1979",
	"Galaxian (Taito)\0", NULL, "Namco (Taito license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalaxiantRomInfo, GalaxiantRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Galaxian Rip-Offs on basic Galaxian Hardware
static struct BurnRomInfo MoonalnRomDesc[] = {
	{ "galx.u",        0x00800, 0x79e4007d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "prg2.bin",      0x00800, 0x59580b30, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "prg3.bin",      0x00800, 0xb64e9d12, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "superg.y",      0x00800, 0x9463f753, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "prg5.bin",      0x00800, 0x8bb78987, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ca1.bin",       0x00800, 0x074271dd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ca2.bin",       0x00800, 0x84d90397, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Moonaln)
STD_ROM_FN(Moonaln)

static struct BurnRomInfo SupergRomDesc[] = {
	{ "7f.bin",        0x01000, 0x4335b1de, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "superg.w",      0x00800, 0xddeabdae, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "superg.y",      0x00800, 0x9463f753, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "superg.z",      0x00800, 0xe6312e35, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "galmidw.1j",    0x00800, 0x84decf98, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "galmidw.1k",    0x00800, 0xc31ada9e, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Superg)
STD_ROM_FN(Superg)

static struct BurnRomInfo GalturboRomDesc[] = {
	{ "galturbo.u",    0x00800, 0xe8f3aa67, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galx.v",        0x00800, 0xbc16064e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "superg.w",      0x00800, 0xddeabdae, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galturbo.y",    0x00800, 0xa44f450f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galturbo.z",    0x00800, 0x3247f3d4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "galturbo.1h",   0x00800, 0xa713fd1a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "galturbo.1k",   0x00800, 0x28511790, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galturbo)
STD_ROM_FN(Galturbo)

static struct BurnRomInfo GalapxRomDesc[] = {
	{ "galx.u",        0x00800, 0x79e4007d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galx.v",        0x00800, 0xbc16064e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galx.w",        0x00800, 0x72d2d3ee, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galx.y",        0x00800, 0xafe397f3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galx.z",        0x00800, 0x778c0d3c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "galx.1h",       0x00800, 0xe8810654, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "galx.1k",       0x00800, 0xcbe84a76, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galapx)
STD_ROM_FN(Galapx)

static struct BurnRomInfo Galap1RomDesc[] = {
	{ "7f.bin",        0x01000, 0x4335b1de, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galaxian.w",    0x00800, 0x4c7031c0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galx_1_4.rom",  0x00800, 0xe71e1d9e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galx_1_5.rom",  0x00800, 0x6e65a3b2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "galmidw.1j",    0x00800, 0x84decf98, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "galmidw.1k",    0x00800, 0xc31ada9e, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galap1)
STD_ROM_FN(Galap1)

static struct BurnRomInfo Galap4RomDesc[] = {
	{ "galnamco.u",    0x00800, 0xacfde501, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galnamco.v",    0x00800, 0x65cf3c77, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galnamco.w",    0x00800, 0x9eef9ae6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galnamco.y",    0x00800, 0x56a5ddd1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galnamco.z",    0x00800, 0xf4bc7262, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "galx_4c1.rom",  0x00800, 0xd5e88ab4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "galx_4c2.rom",  0x00800, 0xa57b83e4, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galap4)
STD_ROM_FN(Galap4)

static struct BurnRomInfo SwarmRomDesc[] = {
	{ "swarm1.bin",    0x00800, 0x21eba3d0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "swarm2.bin",    0x00800, 0xf3a436cd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "swarm3.bin",    0x00800, 0x2915e38b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "swarm4.bin",    0x00800, 0x8bbbf486, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "swarm5.bin",    0x00800, 0xf1b1987e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "swarma.bin",    0x00800, 0xef8657bb, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "swarmb.bin",    0x00800, 0x60c4bd31, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Swarm)
STD_ROM_FN(Swarm)

static struct BurnRomInfo ZerotimeRomDesc[] = {
	{ "zt-p01c.016",   0x00800, 0x90a2bc61, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zt-2.016",      0x00800, 0xa433067e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zt-3.016",      0x00800, 0xaaf038d4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zt-4.016",      0x00800, 0x786d690a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zt-5.016",      0x00800, 0xaf9260d7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ztc-2.016",     0x00800, 0x1b13ca05, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ztc-1.016",     0x00800, 0x5cd7df03, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Zerotime)
STD_ROM_FN(Zerotime)

static struct BurnRomInfo StarfghtRomDesc[] = {
	{ "ja.1",          0x00400, 0xc6ab558b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb.2",          0x00400, 0x34b99fed, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jc.3",          0x00400, 0x30e28016, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jd.4",          0x00400, 0xde7e7770, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "je.5",          0x00400, 0xa916c919, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jf.6",          0x00400, 0x9175882b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jg.7",          0x00400, 0x707c0f02, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jh.8",          0x00400, 0x5dd26461, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ji.9",          0x00400, 0x6651fe93, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jj.10",         0x00400, 0x12c721b9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "k1.7a",         0x00800, 0x977e37cf, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k2.9a",         0x00800, 0x15e387ce, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.7f",    0x00020, 0x24652bc4, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Starfght)
STD_ROM_FN(Starfght)

static struct BurnRomInfo GalaxbsfRomDesc[] = {
	{ "1.bn",          0x00400, 0xcc37b774, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.bn",          0x00400, 0xc6d21f03, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.bn",          0x00400, 0x30e28016, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.bn",          0x00400, 0xde7e7770, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.bn",          0x00400, 0xa916c919, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6.bn",          0x00400, 0x9175882b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7.bn",          0x00400, 0x1237b9da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8.bn",          0x00400, 0x78c53607, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "9.bn",          0x00400, 0x901894cc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "10.bn",         0x00400, 0x5876f695, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "11.bn",         0x00800, 0x39fb43a4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "12.bn",         0x00800, 0x7e3f56a2, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galaxbsf)
STD_ROM_FN(Galaxbsf)

static struct BurnRomInfo Kamakazi3RomDesc[] = {
	{ "f_r_a.bin",     0x00800, 0xe8f3aa67, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f_a.bin",       0x00800, 0xf58283e3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f_b.bin",       0x00800, 0xddeabdae, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f_r_c.bin",     0x00800, 0xc8530a88, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f_r_d.bin",     0x00800, 0xda2d77e0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "r_b.bin",       0x00800, 0x977e37cf, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "r_a.bin",       0x00800, 0xd0ba22c9, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Kamakazi3)
STD_ROM_FN(Kamakazi3)

static struct BurnRomInfo SupergxRomDesc[] = {
	{ "sg1",           0x00800, 0xb83f4578, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sg2",           0x00800, 0xd12ca054, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sg3",           0x00800, 0x53714cb1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sg4",           0x00800, 0x2f36fc69, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sg5",           0x00800, 0x1e0ed4fd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sg6",           0x00800, 0x4f3d97a8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },	
	
	{ "sgg1",          0x00800, 0xa1287bf6, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "sgg2",          0x00800, 0x528f1481, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "supergx.prm",   0x00020, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STD_ROM_PICK(Supergx)
STD_ROM_FN(Supergx)

static struct BurnRomInfo TstgalxRomDesc[] = {
	{ "test.u",        0x00800, 0x0614cd7f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.v",     0x00800, 0x9c999a40, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.w",     0x00800, 0xb5894925, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "galmidw.y",     0x00800, 0x6b3ca10b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7l",            0x00800, 0x1b933207, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "1h.bin",        0x00800, 0x39fb43a4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "1k.bin",        0x00800, 0x7e3f56a2, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Tstgalx)
STD_ROM_FN(Tstgalx)

static INT32 SupergxInit()
{
	INT32 nRet;
	
	GalTilesSharedRomSize = 0x1000;
	GalPromRomSize = 0x20;
	
	nRet = GalInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x0800, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1800, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	
	HardCodeGalaxianPROM();
	
	return nRet;
}

struct BurnDriver BurnDrvMoonaln = {
	"moonaln", "galaxian", NULL, NULL, "1979",
	"Moon Alien\0", NULL, "Nichibutsu (Karateco license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MoonalnRomInfo, MoonalnRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSuperg = {
	"superg", "galaxian", NULL, NULL, "1979",
	"Super Galaxians (galaxiaj hack)\0", NULL, "hack", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SupergRomInfo, SupergRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalturbo = {
	"galturbo", "galaxian", NULL, NULL, "1979",
	"Galaxian Turbo (superg hack)\0", NULL, "hack", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalturboRomInfo, GalturboRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalapx = {
	"galapx", "galaxian", NULL, NULL, "1979",
	"Galaxian Part X (moonaln hack)\0", NULL, "hack", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalapxRomInfo, GalapxRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalap1 = {
	"galap1", "galaxian", NULL, NULL, "1979",
	"Space Invaders Galactica (galaxiaj hack)\0", NULL, "hack", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Galap1RomInfo, Galap1RomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalap4 = {
	"galap4", "galaxian", NULL, NULL, "1979",
	"Galaxian Part 4 (hack)\0", NULL, "G.G.I", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Galap4RomInfo, Galap4RomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSwarm = {
	"swarm", "galaxian", NULL, NULL, "1979",
	"Swarm (bootleg?)\0", NULL, "Sebelectro", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SwarmRomInfo, SwarmRomName, NULL, NULL, GalaxianInputInfo, SwarmDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvZerotime = {
	"zerotime", "galaxian", NULL, NULL, "1979",
	"Zero Time\0", NULL, "Petaco S.A", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, ZerotimeRomInfo, ZerotimeRomName, NULL, NULL, GalaxianInputInfo, ZerotimeDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvStarfght = {
	"starfght", "galaxian", NULL, NULL, "1979",
	"Star Fighter\0", NULL, "Juetel", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, StarfghtRomInfo, StarfghtRomName, NULL, NULL, GalaxianInputInfo, SwarmDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalaxbsf = {
	"galaxbsf", "galaxian", NULL, NULL, "1979",
	"Galaxian (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalaxbsfRomInfo, GalaxbsfRomName, NULL, NULL, GalaxianInputInfo, GalaxianDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvKamakazi3 = {
	"kamakazi3", "galaxian", NULL, NULL, "1979",
	"Kamakazi III (superg hack)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Kamakazi3RomInfo, Kamakazi3RomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriverD BurnDrvSupergx = {
	"supergx", "galaxian", NULL, NULL, "1980",
	"Super GX\0", "Bad Dump", "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SupergxRomInfo, SupergxRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	SupergxInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTstgalx = {
	"tst_galx", "galaxian", NULL, NULL, "1979",
	"Galaxian Test ROM\0", NULL, "Test ROM", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_PROTOTYPE, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, TstgalxRomInfo, TstgalxRomName, NULL, NULL, GalaxianInputInfo, GalaxianDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Other games on basic Galaxian hardware
static struct BurnRomInfo BlkholeRomDesc[] = {
	{ "bh1",           0x00800, 0x64998819, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bh2",           0x00800, 0x26f26ce4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bh3",           0x00800, 0x3418bc45, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bh4",           0x00800, 0x735ff481, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bh5",           0x00800, 0x3f657be9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bh6",           0x00800, 0xa057ab35, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "bh7",           0x00800, 0x975ba821, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "bh8",           0x00800, 0x03d11020, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Blkhole)
STD_ROM_FN(Blkhole)

static struct BurnRomInfo OrbitronRomDesc[] = {
	{ "orbitron.3",    0x00800, 0x419f9c9b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "orbitron.4",    0x00800, 0x44ad56ac, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "orbitron.1",    0x00800, 0xda3f5168, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "orbitron.2",    0x00800, 0xa3b813fc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "orbitron.5",    0x00800, 0x20cd8bb8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "orbitron.6",    0x00800, 0x2c91b83f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "orbitron.7",    0x00800, 0x46f4cca4, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Orbitron)
STD_ROM_FN(Orbitron)

static struct BurnRomInfo LuctodayRomDesc[] = {
	{ "ltprog1.bin",   0x00800, 0x59c389b9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ltprog2.bin",   0x00800, 0xac3893b1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ltchar2.bin",   0x00800, 0x8cd73bdc, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ltchar1.bin",   0x00800, 0xb5ba9946, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "74s288.ch",     0x00020, 0x24652bc4, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Luctoday)
STD_ROM_FN(Luctoday)

static struct BurnRomInfo ChewingRomDesc[] = {
	{ "1.bin",         0x01000, 0x7470b347, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7l.bin",        0x00800, 0x78ebed36, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "2.bin",         0x00800, 0x88c605f3, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "3.bin",         0x00800, 0x77ac016a, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "74s288.ch",     0x00020, 0x24652bc4, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Chewing)
STD_ROM_FN(Chewing)

static struct BurnRomInfo CatacombRomDesc[] = {
	{ "catacomb.u",    0x00800, 0x35cc28d2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "catacomb.v",    0x00800, 0x1d1ce133, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "catacomb.w",    0x00800, 0x479bbde7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "catacomb.y",    0x00800, 0x5e3da534, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "cat-gfx1",      0x00800, 0xe871e65c, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "cat-gfx2",      0x00800, 0xb14dafaa, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Catacomb)
STD_ROM_FN(Catacomb)

static struct BurnRomInfo OmegaRomDesc[] = {
	{ "omega1.bin",    0x00800, 0xfc2a096b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "omega2.bin",    0x00800, 0xad100357, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "omega3.bin",    0x00800, 0xd7e3be79, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "omega4.bin",    0x00800, 0x42068171, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "omega5.bin",    0x00800, 0xd8a93383, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "omega6.bin",    0x00800, 0x32a42f44, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "omega1h.bin",   0x00800, 0x527fd384, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "omega1k.bin",   0x00800, 0x36de42c6, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331-1j.86", 0x00020, 0x6a0c7d87, BRF_OPT | BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Omega)
STD_ROM_FN(Omega)

static struct BurnRomInfo WarofbugRomDesc[] = {
	{ "warofbug.u",    0x00800, 0xb8dfb7e3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "warofbug.v",    0x00800, 0xfd8854e0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "warofbug.w",    0x00800, 0x4495aa14, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "warofbug.y",    0x00800, 0xc14a541f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "warofbug.z",    0x00800, 0xc167fe55, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "warofbug.1k",   0x00800, 0x8100fa85, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "warofbug.1j",   0x00800, 0xd1220ae9, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "warofbug.clr",  0x00020, 0x8688e64b, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Warofbug)
STD_ROM_FN(Warofbug)

static struct BurnRomInfo WarofbuggRomDesc[] = {
	{ "wotbg-u-1.bin", 0x00800, 0xf43ff0a8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "wotbg-v-2.bin", 0x00800, 0xeb7a028b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "wotbg-w-3.bin", 0x00800, 0x693e0e50, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "wotbg-y-4.bin", 0x00800, 0x885d4982, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "wotbg-z-5.bin", 0x00800, 0x60041ef2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "warofbug.1k",   0x00800, 0x8100fa85, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "warofbug.1j",   0x00800, 0xd1220ae9, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "warofbug.clr",  0x00020, 0x8688e64b, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Warofbugg)
STD_ROM_FN(Warofbugg)

static struct BurnRomInfo RedufoRomDesc[] = {
	{ "redufo.1",      0x00800, 0x6a3b873c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "redufo.2",      0x00800, 0x202eda3b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "redufo.3",      0x00800, 0xbf7030e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "redufo.4",      0x00800, 0x8c1c2ef9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "redufo.5",      0x00800, 0xef965b24, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "redufo.6",      0x00800, 0x58b3e39b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "redufo.7",      0x00800, 0xfd07d811, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "redufo.8",      0x00800, 0xb34c7cb4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "redufo.9",      0x00800, 0x50a2d447, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "tbp18s030n.6l", 0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Redufo)
STD_ROM_FN(Redufo)

static struct BurnRomInfo RedufobRomDesc[] = {
	{ "ru1a",          0x00800, 0x5a8e4f37, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ru2a",          0x00800, 0xc624f52d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ru3a",          0x00800, 0xe1030d1c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ru4a",          0x00800, 0x7692069e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ru5a",          0x00800, 0xcb648ff3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ru6a",          0x00800, 0xe1a9f58e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ruhja",         0x00800, 0x8a422b0d, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rukla",         0x00800, 0x1eb84cb1, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Redufob)
STD_ROM_FN(Redufob)

static struct BurnRomInfo ExodusRomDesc[] = {
	{ "exodus1.bin",   0x00800, 0x5dfe65e1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "exodus2.bin",   0x00800, 0x6559222f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "exodus3.bin",   0x00800, 0xbf7030e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "exodus4.bin",   0x00800, 0x3607909e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "exodus9.bin",   0x00800, 0x994a90c4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "exodus10.bin",  0x00800, 0xfbd11187, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "exodus11.bin",  0x00800, 0xfd07d811, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "exodus5.bin",   0x00800, 0xb34c7cb4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "exodus6.bin",   0x00800, 0x50a2d447, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Exodus)
STD_ROM_FN(Exodus)

static struct BurnRomInfo TdpgalRomDesc[] = {
	{ "1.bin",         0x00800, 0x7be819fe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.bin",         0x00800, 0x70c83a5e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.bin",         0x00800, 0x475eb5a0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.bin",         0x00800, 0x20a71943, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.bin",         0x00800, 0x72c4f2dc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6.bin",         0x00800, 0xfa4e2be4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "8.bin",         0x00800, 0xd701b1d4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "7.bin",         0x00800, 0x3113bcfd, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "bprom.bin",     0x00020, 0x2b4cf53f, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Tdpgal)
STD_ROM_FN(Tdpgal)

static struct BurnRomInfo AzurianRomDesc[] = {
	{ "pgm.1",         0x01000, 0x17a0fca7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pgm.2",         0x01000, 0x14659848, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pgm.3",         0x01000, 0x8f60fb97, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "gfx.1",         0x00800, 0xf5afb803, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "gfx.2",         0x00800, 0xae96e5d1, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Azurian)
STD_ROM_FN(Azurian)

static struct BurnRomInfo TazzmangRomDesc[] = {
	{ "tazzm1.4k",     0x01000, 0xa14480a1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tazzm2.5j",     0x01000, 0x5609f5db, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tazzm3.6f",     0x01000, 0xfe7f7002, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tazzm4.7e",     0x01000, 0xc9ca1d0a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tazzm5.7l",     0x01000, 0xf50cd8a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tazzm6.7l",     0x01000, 0x5cf2e7d2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "tazm8.1lk",     0x00800, 0x2c5b612b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "tazzm7.1jh",    0x00800, 0x3f5ff3ac, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "prom.6l",       0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Tazzmang)
STD_ROM_FN(Tazzmang)

static struct BurnRomInfo ScramblbRomDesc[] = {
	{ "scramble.1k",   0x00800, 0x9e025c4a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scramble.2k",   0x00800, 0x306f783e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scramble.3k",   0x00800, 0x0500b701, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scramble.4k",   0x00800, 0xdd380a22, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scramble.5k",   0x00800, 0xdf0b9648, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scramble.1j",   0x00800, 0xb8c07b3c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scramble.2j",   0x00800, 0x88ac07a0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scramble.3j",   0x00800, 0xc67d57ca, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "5f.k",          0x00800, 0x4708845b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h.k",          0x00800, 0x11fd2887, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "82s123.6e",     0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scramblb)
STD_ROM_FN(Scramblb)

static struct BurnRomInfo Scramb2RomDesc[] = {
	{ "r1.7f1",        0x00800, 0x4a43148c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "r1.7f2",        0x00800, 0x215a3b86, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "r2.7h1",        0x00800, 0x28779444, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "r2.7h2",        0x00800, 0x5b4b300b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "r3.7k1",        0x00800, 0xb478aa53, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "r3.7k2",        0x00800, 0xc33f072e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "r4.7l1",        0x00800, 0x88ac07a0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "r4.7l2",        0x00800, 0x321fd003, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "r6.1j",         0x00800, 0x4708845b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "r5.1l",         0x00800, 0x11fd2887, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "82s123.6e",     0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scramb2)
STD_ROM_FN(Scramb2)

UINT8 __fastcall AzurianZ80Read(UINT16 a)
{
	switch (a) {
		case 0x6000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x6800: {
			return ((GalInput[1] | GalDip[1]) & ~0x40) | ((GalFakeDip & 0x01) << 6);
		}
		
		case 0x7000: {
			return ((GalInput[2] | GalDip[2]) & ~0x04) | ((GalFakeDip & 0x02) << 1);
		}
		
		case 0x7800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

UINT8 __fastcall TazzmangZ80Read(UINT16 a)
{
	switch (a) {
		case 0x9800:
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		case 0xa000:
		case 0xa7ff: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0xa800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x7000:
		case 0xb000: {
			return GalInput[2] | GalDip[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall TazzmangZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x8800 && a <= 0x88ff) {
		INT32 Offset = a - 0x8800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall ScramblbZ80Read(UINT16 a)
{
	switch (a) {
		case 0x6000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x6800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x7000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0x7800: {
			// watchdog read
			return 0xff;
		}
		
		case 0x8102: {
			switch (ZetPc(-1)) {
				case 0x1da: return 0x80;
				case 0x1e4: return 0x00;
			}
			return 0xff;
		}
		
		case 0x8202: {
			switch (ZetPc(-1)) {
				case 0x1ca: return 0x90;
			}
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall ScramblbZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000:
		case 0x6001: {
			// start_lamp_w
			return;
		}
		
		case 0x6002: {
			// coin_lock_w
			return;
		}
		
		case 0x6004:
		case 0x6005:
		case 0x6006:
		case 0x6007: {
			GalaxianLfoFreqWrite(a - 0x6004, d);
			return;
		}
		
		case 0x6800:
		case 0x6801:
		case 0x6802:
		case 0x6803:
		case 0x6804:
		case 0x6805:
		case 0x6806:
		case 0x6807: {
			GalaxianSoundWrite(a - 0x6800, d);
			return;
		}
		
		case 0x7001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7002: {
			// coin_count_0_w
			return;
		}
		
		case 0x7003: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0x7004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x7006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x7007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Scramb2Z80Read(UINT16 a)
{
	if (a >= 0x5800 && a <= 0x5fff) return 0x25;
	
	switch (a) {
		case 0x6000:
		case 0x6001:
		case 0x6002:
		case 0x6003:
		case 0x6004:
		case 0x6005:
		case 0x6006:
		case 0x6007: {
			int Offset = a - 0x6000;
			return ((0xff - GalInput[0] - GalDip[0]) >> Offset) & 0x01;
		}
		
		case 0x6800:
		case 0x6801:
		case 0x6802:
		case 0x6803:
		case 0x6804:
		case 0x6805:
		case 0x6806:
		case 0x6807: {
			int Offset = a - 0x6800;
			return ((0xff - GalInput[1] - GalDip[1]) >> Offset) & 0x01;
		}
		
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		case 0x7800:
		case 0x7801:
		case 0x7802:
		case 0x7803:
		case 0x7804:
		case 0x7805:
		case 0x7806:
		case 0x7807: {
			INT32 Offset = a - 0x7800;
			return ((0xff - GalInput[2] - GalDip[2]) >> Offset) & 0x01;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall Scramb2Z80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000:
		case 0x6001:
		case 0x6002:
		case 0x6003:
		case 0x6004:
		case 0x6005:
		case 0x6006:
		case 0x6007:
		case 0x7000:
		case 0x7001:
		case 0x7002:
		case 0x7003:
		case 0x7004:
		case 0x7005:
		case 0x7006:
		case 0x7007: {
			// Probably PPI writes that the bootleggers didn't use
			return;
		}
		
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6802: {
			// coin_count_0_w
			return;
		}
		
		case 0x6803: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0x6804: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x6806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x6807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void OrbitronRearrangeRom()
{
	GalTempRom = (UINT8*)BurnMalloc(0x2000);
	
	memcpy(GalTempRom, GalZ80Rom1, 0x2000);
	memcpy(GalZ80Rom1 + 0x0600, GalTempRom + 0x0000, 0x200);
	memcpy(GalZ80Rom1 + 0x0400, GalTempRom + 0x0200, 0x200);
	memcpy(GalZ80Rom1 + 0x0200, GalTempRom + 0x0400, 0x200);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0600, 0x200);
	memcpy(GalZ80Rom1 + 0x0e00, GalTempRom + 0x0800, 0x200);
	memcpy(GalZ80Rom1 + 0x0c00, GalTempRom + 0x0a00, 0x200);
	memcpy(GalZ80Rom1 + 0x0a00, GalTempRom + 0x0c00, 0x200);
	memcpy(GalZ80Rom1 + 0x0800, GalTempRom + 0x0e00, 0x200);
	memcpy(GalZ80Rom1 + 0x1600, GalTempRom + 0x1000, 0x200);
	memcpy(GalZ80Rom1 + 0x1400, GalTempRom + 0x1200, 0x200);
	memcpy(GalZ80Rom1 + 0x1200, GalTempRom + 0x1400, 0x200);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x1600, 0x200);
	memcpy(GalZ80Rom1 + 0x1e00, GalTempRom + 0x1800, 0x200);
	memcpy(GalZ80Rom1 + 0x1c00, GalTempRom + 0x1a00, 0x200);
	memcpy(GalZ80Rom1 + 0x1a00, GalTempRom + 0x1c00, 0x200);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x1e00, 0x200);
	BurnFree(GalTempRom);
}

static INT32 OrbitronInit()
{
	GalPostLoadCallbackFunction = OrbitronRearrangeRom;
	
	return GalInit();
}

static void LuctodayRearrangeRom()
{
	memcpy(GalZ80Rom1 + 0x2000, GalZ80Rom1 + 0x0800, 0x800);
	memset(GalZ80Rom1 + 0x0800, 0, 0x800);
}

static INT32 LuctodayInit()
{
	GalZ80Rom1Size = 0x1800;
	GalPostLoadCallbackFunction = LuctodayRearrangeRom;
	
	return GalInit();
}

static void ChewingRearrangeRom()
{
	memcpy(GalZ80Rom1 + 0x2000, GalZ80Rom1 + 0x1000, 0x800);
	memset(GalZ80Rom1 + 0x1000, 0, 0x800);
}

static INT32 ChewingInit()
{
	GalZ80Rom1Size = 0x1000;
	GalPostLoadCallbackFunction = ChewingRearrangeRom;
	
	return GalInit();
}

static void CatacombRearrangeRom()
{
	memcpy(GalZ80Rom1 + 0x2000, GalZ80Rom1 + 0x1800, 0x800);
	memset(GalZ80Rom1 + 0x1800, 0, 0x800);
}

static INT32 CatacombInit()
{
	GalZ80Rom1Size = 0x800;
	GalPostLoadCallbackFunction = CatacombRearrangeRom;
	
	return GalInit();
}

static void TdpgalPostLoad()
{
	ZetOpen(0);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetClose();
}

static INT32 TdpgalInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = TdpgalPostLoad;
	
	nRet = GalInit();
	
	return nRet;
}	

static void AzurianPostLoad()
{
	ZetOpen(0);
	ZetSetReadHandler(AzurianZ80Read);
	ZetClose();
}

static INT32 AzurianInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = AzurianPostLoad;
	
	nRet = GalInit();
	
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	return nRet;
}

static void TazzmangPostLoad()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(TazzmangZ80Read);
	ZetSetWriteHandler(TazzmangZ80Write);
	ZetMapArea(0x0000, 0x5fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x5fff, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetMapArea(0x8800, 0x88ff, 0, GalSpriteRam);
	ZetMapArea(0x8800, 0x88ff, 2, GalSpriteRam);
	ZetMapArea(0x9000, 0x93ff, 0, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 1, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 2, GalVideoRam);
	ZetClose();
}

static INT32 TazzmangInit()
{
	GalPostLoadCallbackFunction = TazzmangPostLoad;
	
	return GalInit();
}

static void MapScramblb()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(ScramblbZ80Read);
	ZetSetWriteHandler(ScramblbZ80Write);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x4000, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 2, GalZ80Ram1);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 0, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 1, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 2, GalVideoRam);
	ZetMapArea(0x5000, 0x50ff, 0, GalSpriteRam);
	ZetMapArea(0x5000, 0x50ff, 2, GalSpriteRam);	
	ZetClose();
}

static INT32 ScramblbInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapScramblb;
	
	nRet = GalInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	return nRet;
}

static void Scramb2PostLoad()
{
	MapScramblb();
	
	ZetOpen(0);
	ZetSetReadHandler(Scramb2Z80Read);
	ZetSetWriteHandler(Scramb2Z80Write);
	ZetClose();
}

static INT32 Scramb2Init()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = Scramb2PostLoad;
	
	nRet = GalInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	return nRet;
}

struct BurnDriver BurnDrvBlkhole = {
	"blkhole", NULL, NULL, NULL, "1981",
	"Black Hole\0", NULL, "TDS", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, BlkholeRomInfo, BlkholeRomName, NULL, NULL, BlkholeInputInfo, BlkholeDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvOrbitron = {
	"orbitron", NULL, NULL, NULL, "19??",
	"Orbitron\0", NULL, "Signatron USA", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SHOOT, 0,
	NULL, OrbitronRomInfo, OrbitronRomName, NULL, NULL, OrbitronInputInfo, OrbitronDIPInfo,
	OrbitronInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvLuctoday = {
	"luctoday", NULL, NULL, NULL, "1980",
	"Lucky Today\0", "Bad colours", "Sigma", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_GALAXIAN, GBF_CASINO, 0,
	NULL, LuctodayRomInfo, LuctodayRomName, NULL, NULL, LuctodayInputInfo, LuctodayDIPInfo,
	LuctodayInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvChewing = {
	"chewing", "luctoday", NULL, NULL, "1980",
	"Chewing Gum\0", NULL, "unknown", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_CASINO, 0,
	NULL, ChewingRomInfo, ChewingRomName, NULL, NULL, LuctodayInputInfo, LuctodayDIPInfo,
	ChewingInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvCatacomb = {
	"catacomb", NULL, NULL, NULL, "1982",
	"Catacomb\0", "Bad Colours", "MTM Games", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, CatacombRomInfo, CatacombRomName, NULL, NULL, OmegaInputInfo, CatacombDIPInfo,
	CatacombInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvOmega = {
	"omega", "theend", NULL, NULL, "19??",
	"Omega\0", NULL, "bootleg?", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, OmegaRomInfo, OmegaRomName, NULL, NULL, OmegaInputInfo, OmegaDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvWarofbug = {
	"warofbug", NULL, NULL, NULL, "1981",
	"War of the Bugs or Monsterous Manouvers in a Mushroom Maze\0", NULL, "Armenia", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, WarofbugRomInfo, WarofbugRomName, NULL, NULL, WarofbugInputInfo, WarofbugDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriverD BurnDrvWarofbugg = {
	"warofbugg", "warofbug", NULL, NULL, "1981",
	"War of the Bugs or Monsterous Manouvers in a Mushroom Maze (German)\0", NULL, "Armenia", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, WarofbuggRomInfo, WarofbuggRomName, NULL, NULL, WarofbugInputInfo, WarofbugDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvRedufo = {
	"redufo", NULL, NULL, NULL, "1981",
	"Defend the Terra Attack on the Red UFO\0", NULL, "Artic", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, RedufoRomInfo, RedufoRomName, NULL, NULL, GalaxianInputInfo, RedufoDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvRedufob = {
	"redufob", "redufo", NULL, NULL, "1981",
	"Defend the Terra Attack on the Red UFO (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, RedufobRomInfo, RedufobRomName, NULL, NULL, GalaxianInputInfo, RedufobDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvExodus = {
	"exodus", "redufo", NULL, NULL, "19??",
	"Exodus (bootleg?)\0", NULL, "Subelectro", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, ExodusRomInfo, ExodusRomName, NULL, NULL, OmegaInputInfo, ExodusDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTdpgal = {
	"tdpgal", NULL, NULL, NULL, "1983",
	"Triple Draw Poker\0", NULL, "Design Labs / Thomas Automatics", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_CASINO, 0,
	NULL, TdpgalRomInfo, TdpgalRomName, NULL, NULL, TdpgalInputInfo, NULL,
	TdpgalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAzurian = {
	"azurian", NULL, NULL, NULL, "1982",
	"Azurian Attack\0", NULL, "Rait Electronics Ltd", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SHOOT, 0,
	NULL, AzurianRomInfo, AzurianRomName, NULL, NULL, AzurianInputInfo, AzurianDIPInfo,
	AzurianInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTazzmang = {
	"tazzmang", "tazmania", NULL, NULL, "1982",
	"Tazz-Mania (Galaxian hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, TazzmangRomInfo, TazzmangRomName, NULL, NULL, TazzmangInputInfo, TazzmangDIPInfo,
	TazzmangInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScramblb = {
	"scramblb", "scramble", NULL, NULL, "1981",
	"Scramble (bootleg on Galaxian hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScramblbRomInfo, ScramblbRomName, NULL, NULL, ScramblbInputInfo, ScramblbDIPInfo,
	ScramblbInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScramb2 = {
	"scramb2", "scramble", NULL, NULL, "1981",
	"Scramble (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, Scramb2RomInfo, Scramb2RomName, NULL, NULL, Scramb2InputInfo, Scramb2DIPInfo,
	Scramb2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Extra chars controlled via bank at 0x6002
static struct BurnRomInfo PiscesRomDesc[] = {
	{ "p1.bin",        0x00800, 0x40c5b0e4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p2.bin",        0x00800, 0x055f9762, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p3.bin",        0x00800, 0x3073dd04, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p4.bin",        0x00800, 0x44aaf525, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p5.bin",        0x00800, 0xfade512b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p6.bin",        0x00800, 0x5ab2822f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "g09.bin",       0x00800, 0x9503a23a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g11.bin",       0x00800, 0x0adfc3fe, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g10.bin",       0x00800, 0x3e61f849, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g12.bin",       0x00800, 0x7130e9eb, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "colour.bin",    0x00020, 0x57a45057, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Pisces)
STD_ROM_FN(Pisces)

static struct BurnRomInfo PiscesbRomDesc[] = {
	{ "pisces.a1",     0x00800, 0x856b8e1f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p2.bin",        0x00800, 0x055f9762, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pisces.b2",     0x00800, 0x5540f2e4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p4.bin",        0x00800, 0x44aaf525, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p5.bin",        0x00800, 0xfade512b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p6.bin",        0x00800, 0x5ab2822f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "g09.bin",       0x00800, 0x9503a23a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g11.bin",       0x00800, 0x0adfc3fe, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g10.bin",       0x00800, 0x3e61f849, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g12.bin",       0x00800, 0x7130e9eb, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "colour.bin",    0x00020, 0x57a45057, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Piscesb)
STD_ROM_FN(Piscesb)

static struct BurnRomInfo OmniRomDesc[] = {
	{ "omni1.7f",      0x01000, 0xa9b7acc6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "omni2.7j",      0x01000, 0x6ade29b7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "omni3.7f",      0x01000, 0x9e37bb24, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "omni5b.l1",     0x00800, 0x9503a23a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "omni6c.j22",    0x00800, 0x0adfc3fe, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "omni4a.j1",     0x00800, 0x3e61f849, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "omni7d.l2",     0x00800, 0x7130e9eb, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "colour.bin",    0x00020, 0x57a45057, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Omni)
STD_ROM_FN(Omni)

static struct BurnRomInfo UniwarsRomDesc[] = {
	{ "f07_1a.bin",    0x00800, 0xd975af10, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "h07_2a.bin",    0x00800, 0xb2ed14c3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "k07_3a.bin",    0x00800, 0x945f4160, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m07_4a.bin",    0x00800, 0xddc80bc5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "d08p_5a.bin",   0x00800, 0x62354351, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "gg6",           0x00800, 0x270a3f4d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m08p_7a.bin",   0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "n08p_8a.bin",   0x00800, 0x797d45c7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "egg10",         0x00800, 0x012941e0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "h01_2.bin",     0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "egg9",          0x00800, 0xfc8b58fd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_2.bin",     0x00800, 0xdcc2b33b, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "uniwars.clr",   0x00020, 0x25c79518, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Uniwars)
STD_ROM_FN(Uniwars)

static struct BurnRomInfo GteikokuRomDesc[] = {
	{ "f07_1a.bin",    0x00800, 0xd975af10, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "h07_2a.bin",    0x00800, 0xb2ed14c3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "k07_3a.bin",    0x00800, 0x945f4160, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m07_4a.bin",    0x00800, 0xddc80bc5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "d08p_5a.bin",   0x00800, 0x62354351, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e08p_6a.bin",   0x00800, 0xd915a389, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m08p_7a.bin",   0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "n08p_8a.bin",   0x00800, 0x797d45c7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "h01_1.bin",     0x00800, 0x8313c959, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "h01_2.bin",     0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_1.bin",     0x00800, 0xc9d4537e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_2.bin",     0x00800, 0xdcc2b33b, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Gteikoku)
STD_ROM_FN(Gteikoku)

static struct BurnRomInfo GteikokbRomDesc[] = {
	{ "1.bin",         0x00800, 0xbf00252f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.bin",         0x00800, 0xf712b7d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "k07_3a.bin",    0x00800, 0x945f4160, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.bin",         0x00800, 0x808a39a8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.bin",         0x00800, 0x36fe6e67, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6.bin",         0x00800, 0xc5ea67e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m08p_7a.bin",   0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8.bin",         0x00800, 0x28df3229, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "h01_1.bin",     0x00800, 0x8313c959, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "h01_2.bin",     0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_1.bin",     0x00800, 0xc9d4537e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_2.bin",     0x00800, 0xdcc2b33b, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Gteikokb)
STD_ROM_FN(Gteikokb)

static struct BurnRomInfo Gteikob2RomDesc[] = {
	{ "94gnog.bin",    0x00800, 0x67ec3235, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "92gnog.bin",    0x00800, 0x813c41f2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "k07_3a.bin",    0x00800, 0x945f4160, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "1gnog.bin",     0x00800, 0x49ff9658, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.bin",         0x00800, 0x36fe6e67, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e08p_6a.bin",   0x00800, 0xd915a389, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m08p_7a.bin",   0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "98gnog.bin",    0x00800, 0xe9d4ad3c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "h01_1.bin",     0x00800, 0x8313c959, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "h01_2.bin",     0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_1.bin",     0x00800, 0xc9d4537e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_2.bin",     0x00800, 0xdcc2b33b, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Gteikob2)
STD_ROM_FN(Gteikob2)

static struct BurnRomInfo SpacbattRomDesc[] = {
	{ "sb1",           0x00800, 0xd975af10, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb2",           0x00800, 0xb2ed14c3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb3",           0x00800, 0x945f4160, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb4",           0x00800, 0x8229835c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb5",           0x00800, 0xf51ef930, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb6",           0x00800, 0xd915a389, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb7",           0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb8",           0x00800, 0xe59ff1ae, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "sb12",          0x00800, 0x8313c959, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "sb14",          0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "sb11",          0x00800, 0xc9d4537e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "sb13",          0x00800, 0x92454380, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Spacbatt)
STD_ROM_FN(Spacbatt)

static struct BurnRomInfo Spacbat2RomDesc[] = {
	{ "sb1",           0x00800, 0xd975af10, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb2",           0x00800, 0xb2ed14c3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb.3",          0x00800, 0xc25ce4c1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb4",           0x00800, 0x8229835c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb5",           0x00800, 0xf51ef930, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb6",           0x00800, 0xd915a389, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb7",           0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sb8",           0x00800, 0xe59ff1ae, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "sb12",          0x00800, 0x8313c959, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "sb14",          0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "sb11",          0x00800, 0xc9d4537e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_2.bin",     0x00800, 0xdcc2b33b, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Spacbat2)
STD_ROM_FN(Spacbat2)

static struct BurnRomInfo SpacemprRomDesc[] = {
	{ "uw01",          0x00800, 0x7c64fb92, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "uw02",          0x00800, 0xb2ed14c3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "uw03",          0x00800, 0x945f4160, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "uw04",          0x00800, 0x84885060, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "uw05",          0x00800, 0xe342371d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "uw06",          0x00800, 0xd915a389, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "uw07",          0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "uw08",          0x00800, 0xa237c394, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "uw10",          0x00800, 0xaf069cba, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "uw12",          0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "uw09",          0x00800, 0xff2c20d5, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "uw11",          0x00800, 0xdcc2b33b, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331",       0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Spacempr)
STD_ROM_FN(Spacempr)

static struct BurnRomInfo SkyraidrRomDesc[] = {
	{ "f07_1a.bin",    0x00800, 0xd975af10, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "h07_2a.bin",    0x00800, 0xb2ed14c3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "k07_3a.bin",    0x00800, 0x945f4160, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sr.04",         0x00800, 0x9f61d1f8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sr.05",         0x00800, 0x4352af0a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sr.06",         0x00800, 0xd915a389, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m08p_7a.bin",   0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "n08p_8a.bin",   0x00800, 0x797d45c7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "sr.10",         0x00800, 0xaf069cba, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "h01_2.bin",     0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "sr.09",         0x00800, 0xff2c20d5, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k01_2.bin",     0x00800, 0xdcc2b33b, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "uniwars.clr",   0x00020, 0x25c79518, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Skyraidr)
STD_ROM_FN(Skyraidr)

static struct BurnRomInfo GalempRomDesc[] = {
	{ "1",             0x00800, 0xd975af10, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2",             0x00800, 0xb2ed14c3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3",             0x00800, 0x945f4160, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4",             0x00800, 0xdf7a13ea, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5",             0x00800, 0xff6128a2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6",             0x00800, 0xd915a389, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7",             0x00800, 0xc9245346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8",             0x00800, 0x797d45c7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "10",            0x00800, 0x30177b93, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "12",            0x00800, 0xc26132af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "9",             0x00800, 0x7e8dcc13, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "11",            0x00800, 0xdcc2b33b, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Galemp)
STD_ROM_FN(Galemp)

void __fastcall PiscesZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5800 && a <= 0x58ff) {
		INT32 Offset = a - 0x5800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000:
		case 0x6001: {
			// start_lamp_w
			return;
		}
		
		case 0x6002: {
			GalGfxBank[0] = d;
			return;
		}
		
		case 0x6003: {
			// coin_count_0_w
			return;
		}
		
		case 0x6004:
		case 0x6005:
		case 0x6006:
		case 0x6007: {
			GalaxianLfoFreqWrite(a - 0x6004, d);
			return;
		}
		
		case 0x6800:
		case 0x6801:
		case 0x6802:
		case 0x6803:
		case 0x6804:
		case 0x6805:
		case 0x6806:
		case 0x6807: {
			GalaxianSoundWrite(a - 0x6800, d);
			return;
		}
		
		case 0x7001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x7006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x7007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void PiscesInstallGfxBankHandler()
{
	ZetOpen(0);
	ZetSetWriteHandler(PiscesZ80Write);
	ZetClose();
}

static INT32 PiscesInit()
{
	GalPostLoadCallbackFunction = PiscesInstallGfxBankHandler;
	GalExtendTileInfoFunction = PiscesExtendTileInfo;
	GalExtendSpriteInfoFunction = PiscesExtendSpriteInfo;
	
	return GalInit();
}

struct BurnDriver BurnDrvPisces = {
	"pisces", NULL, NULL, NULL, "19??",
	"Pisces\0", NULL, "Subelectro", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, PiscesRomInfo, PiscesRomName, NULL, NULL, PicsesInputInfo, PiscesDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvPiscesb = {
	"piscesb", "pisces", NULL, NULL, "19??",
	"Pisces (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, PiscesbRomInfo, PiscesbRomName, NULL, NULL, PicsesInputInfo, PiscesbDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvOmni = {
	"omni", "pisces", NULL, NULL, "19??",
	"Omni\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, OmniRomInfo, OmniRomName, NULL, NULL, OmegaInputInfo, PiscesbDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvUniwars = {
	"uniwars", NULL, NULL, NULL, "1980",
	"UniWar S\0", NULL, "Irem", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, UniwarsRomInfo, UniwarsRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGteikoku = {
	"gteikoku", "uniwars", NULL, NULL, "1980",
	"Gingateikoku No Gyakushu\0", NULL, "Irem", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GteikokuRomInfo, GteikokuRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGteikokb = {
	"gteikokb", "uniwars", NULL, NULL, "1980",
	"Gingateikoku No Gyakushu (bootleg set 1)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GteikokbRomInfo, GteikokbRomName, NULL, NULL, OmegaInputInfo, GteikokbDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGteikob2 = {
	"gteikob2", "uniwars", NULL, NULL, "1980",
	"Gingateikoku No Gyakushu (bootleg set 2)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Gteikob2RomInfo, Gteikob2RomName, NULL, NULL, OmegaInputInfo, Gteikob2DIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSpacbatt = {
	"spacbatt", "uniwars", NULL, NULL, "1980",
	"Space Battle (bootleg set 1)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SpacbattRomInfo, SpacbattRomName, NULL, NULL, GalaxianInputInfo, SpacbattDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSpacbat2 = {
	"spacbat2", "uniwars", NULL, NULL, "1980",
	"Space Battle (bootleg set 2)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Spacbat2RomInfo, Spacbat2RomName, NULL, NULL, GalaxianInputInfo, SpacbattDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSpacempr = {
	"spacempr", "uniwars", NULL, NULL, "1980",
	"Space Empire (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SpacemprRomInfo, SpacemprRomName, NULL, NULL, GalaxianInputInfo, SpacbattDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSkyraidr = {
	"skyraidr", "uniwars", NULL, NULL, "1980",
	"Sky Raiders\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SkyraidrRomInfo, SkyraidrRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGalemp = {
	"galemp", "uniwars", NULL, NULL, "1980",
	"Galaxy Empire (bootleg?)\0", NULL, "bootleg (Taito do Brasil)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, GalempRomInfo, GalempRomName, NULL, NULL, GalaxianInputInfo, SupergDIPInfo,
	PiscesInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Extra chars contolled via different banking at 0x6002
static struct BurnRomInfo Batman2RomDesc[] = {
	{ "01.bin",        0x00800, 0x150fbca5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "02.bin",        0x00800, 0xb1624fd0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "03.bin",        0x00800, 0x93774188, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "04.bin",        0x00800, 0x8a94ec6c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "05.bin",        0x00800, 0xa3669461, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "06.bin",        0x00800, 0xfa1efbfe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "07.bin",        0x00800, 0x9b77debd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "08.bin",        0x00800, 0x6466177e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "09.bin",        0x00800, 0x1a657b1f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "11.bin",        0x00800, 0x7a2b48e5, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "10.bin",        0x00800, 0x9b570016, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "12.bin",        0x00800, 0x73956244, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Batman2)
STD_ROM_FN(Batman2)

static struct BurnRomInfo LadybuggRomDesc[] = {
	{ "lbuggx.1",      0x00800, 0xe67e241d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lbuggx.2",      0x00800, 0x3cb1fb9a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lbuggx.3",      0x00800, 0x0937009e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lbuggx.4",      0x00800, 0x3e773f62, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lbuggx.5",      0x00800, 0x2b0d42e5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lbuggx.6",      0x00800, 0x159f9433, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lbuggx.7",      0x00800, 0xf2be06d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lbuggx.8",      0x00800, 0x646fe79f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "lbuggx.a",      0x01000, 0x7efb9dc5, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "lbuggx.b",      0x01000, 0x351d4ddc, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "lbuggx.clr",    0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Ladybugg)
STD_ROM_FN(Ladybugg)

static INT32 Batman2Init()
{
	GalPostLoadCallbackFunction = PiscesInstallGfxBankHandler;
	GalExtendTileInfoFunction = Batman2ExtendTileInfo;
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;
	
	return GalInit();
}

static INT32 LadybuggInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = PiscesInstallGfxBankHandler;
	GalExtendTileInfoFunction = Batman2ExtendTileInfo;
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;
	
	nRet = GalInit();
	
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x1000);
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	memcpy(GalTempRom + 0x0800, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x0000, TempRom + 0x0800, 0x800);
	BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	memcpy(GalTempRom + 0x1800, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x1000, TempRom + 0x0800, 0x800);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	BurnFree(TempRom);
	
	return nRet;
}

struct BurnDriver BurnDrvBatman2 = {
	"batman2", "phoenix", NULL, NULL, "1981",
	"Batman Part 2\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Batman2RomInfo, Batman2RomName, NULL, NULL, Batman2InputInfo, Batman2DIPInfo,
	Batman2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvLadybugg = {
	"ladybugg", "ladybug", NULL, NULL, "1983",
	"Lady Bug (bootleg on Galaxian hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, LadybuggRomInfo, LadybuggRomName, NULL, NULL, LadybuggInputInfo, LadybuggDIPInfo,
	LadybuggInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Seperate sprite/tile roms
static struct BurnRomInfo StreakngRomDesc[] = {
	{ "sk1",           0x01000, 0xc8866ccb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sk2",           0x01000, 0x7caea29b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sk3",           0x01000, 0x7b4bfa76, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "sk4",           0x01000, 0x056fc921, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "sk5",           0x01000, 0xd27f1e0c, BRF_GRA | GAL_ROM_TILES_CHARS },
	
	{ "sk6",           0x01000, 0xa7089588, BRF_GRA | GAL_ROM_TILES_SPRITES },
	
	{ "sk.bpr",        0x00020, 0xbce79607, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Streakng)
STD_ROM_FN(Streakng)

static struct BurnRomInfo StreakngaRomDesc[] = {
	{ "st1.bin",       0x00800, 0xc827e124, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "st2.bin",       0x00800, 0xb01d4f8f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "st3.bin",       0x00800, 0xc7a9c493, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "st4.bin",       0x00800, 0x12487c75, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5st.bin",       0x00800, 0xf9f9e2be, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6st.bin",       0x00800, 0xc22fe6c2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7st.bin",       0x00800, 0x9cd7869a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8st.bin",       0x00800, 0x5e750ad3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "11.bin",        0x00800, 0xcddd8924, BRF_GRA | GAL_ROM_TILES_CHARS },
	{ "12.bin",        0x00800, 0x10cda095, BRF_GRA | GAL_ROM_TILES_CHARS },
	
	{ "9.bin",         0x00800, 0x6a2a8a0f, BRF_GRA | GAL_ROM_TILES_SPRITES },
	{ "10.bin",        0x00800, 0x3563dfbe, BRF_GRA | GAL_ROM_TILES_SPRITES },
	
	{ "sk.bpr",        0x00020, 0xbce79607, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Streaknga)
STD_ROM_FN(Streaknga)

static struct BurnRomInfo PacmanblRomDesc[] = {
	{ "1",             0x00800, 0x6718df42, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2",             0x00800, 0x33be3648, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3",             0x00800, 0xf98c0ceb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4",             0x00800, 0xa9cd0082, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5",             0x00800, 0x6d475afc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6",             0x00800, 0xcbe863d3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7",             0x00800, 0x7daef758, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "12",            0x00800, 0xb2ed320b, BRF_GRA | GAL_ROM_TILES_CHARS },
	{ "11",            0x00800, 0xab88b2c4, BRF_GRA | GAL_ROM_TILES_CHARS },
	
	{ "10",            0x00800, 0x44a45b72, BRF_GRA | GAL_ROM_TILES_SPRITES },
	{ "9",             0x00800, 0xfa84659f, BRF_GRA | GAL_ROM_TILES_SPRITES },
	
	{ "sn74s288n.6l",  0x00020, 0x24652bc4, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Pacmanbl)
STD_ROM_FN(Pacmanbl)

static struct BurnRomInfo PacmanblaRomDesc[] = {
	{ "rom1.bin",      0x01000, 0x75e4f967, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2.bin",      0x01000, 0x5b2e4293, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom3.bin",      0x01000, 0xc06e30a4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom4.bin",      0x01000, 0x592b4ba8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "rom5.bin",      0x01000, 0xf2d8c01e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom6.bin",      0x01000, 0x346a1720, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "sn74s288n.6l",  0x00020, 0x24652bc4, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Pacmanbla)
STD_ROM_FN(Pacmanbla)

static struct BurnRomInfo Phoenxp2RomDesc[] = {
	{ "1",             0x00800, 0xf6dcfd51, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2",             0x00800, 0xde951936, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3",             0x00800, 0x7a3af2da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4",             0x00800, 0xc820ad32, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5",             0x00800, 0x08e83233, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6",             0x00800, 0xf31fb9d6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7",             0x00800, 0xd3a480c1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8",             0x00800, 0xedf9779e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "10",            0x00800, 0x9b570016, BRF_GRA | GAL_ROM_TILES_CHARS },
	{ "09",            0x00800, 0x1a657b1f, BRF_GRA | GAL_ROM_TILES_CHARS },
	
	{ "12",            0x00800, 0x73956244, BRF_GRA | GAL_ROM_TILES_SPRITES },
	{ "11",            0x00800, 0x7a2b48e5, BRF_GRA | GAL_ROM_TILES_SPRITES },
	
	{ "sn74s288n.6l",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Phoenxp2)
STD_ROM_FN(Phoenxp2)

static struct BurnRomInfo AtlantisbRomDesc[] = {
	{ "1",             0x00800, 0x2b612351, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2",             0x00800, 0xb1c970e9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3",             0x00800, 0x63c3783e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4",             0x00800, 0x45f7cf34, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5",             0x00800, 0xf335b96b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6",             0x00800, 0xa50bf8d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "8",             0x00800, 0x55cd5acd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "7",             0x00800, 0x72e773b8, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "sn74s288n.6l",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Atlantisb)
STD_ROM_FN(Atlantisb)

static INT32 PacmanblInit()
{
	INT32 nRet = GalInit();
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

static void PacmanblaRearrangeRom()
{
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	memcpy(GalTempRom, GalZ80Rom1, GalZ80Rom1Size);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0000, 0x800);
	memcpy(GalZ80Rom1 + 0x2000, GalTempRom + 0x0800, 0x800);
	memcpy(GalZ80Rom1 + 0x0800, GalTempRom + 0x1000, 0x800);
	memcpy(GalZ80Rom1 + 0x2800, GalTempRom + 0x1800, 0x800);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x2000, 0x800);
	memcpy(GalZ80Rom1 + 0x3000, GalTempRom + 0x2800, 0x800);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x3000, 0x800);
	memcpy(GalZ80Rom1 + 0x3800, GalTempRom + 0x3800, 0x800);
	BurnFree(GalTempRom);
}

static INT32 PacmanblaInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = PacmanblaRearrangeRom;
	
	nRet = PacmanblInit();
	
	GalNumChars = 0x100;
	GalNumSprites = 0x40;
	CharPlaneOffsets[1] = 0x4000;
	SpritePlaneOffsets[1] = 0x4000;
	
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x2000);
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	BurnLoadRom(TempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	memcpy(GalTempRom + 0x0000, TempRom + 0x0800, 0x800);
	memcpy(GalTempRom + 0x0800, TempRom + 0x1800, 0x800);
	memcpy(GalTempRom + 0x1000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x1800, TempRom + 0x1000, 0x800);	
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom + 0x1000, GalSprites);
	BurnFree(GalTempRom);
	BurnFree(TempRom);
	
	return nRet;
}

static void AtlantisbRearrangeRom()
{
	GalTempRom = (UINT8*)BurnMalloc(0x3000);
	
	memcpy(GalTempRom, GalZ80Rom1, 0x3000);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0000, 0x1000);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x1000, 0x2000);
	BurnFree(GalTempRom);
}

static INT32 AtlantisbInit()
{
	GalZ80Rom1Size = 0x1000;
	
	GalPostLoadCallbackFunction = AtlantisbRearrangeRom;
	
	return GalInit();
}

struct BurnDriver BurnDrvStreakng = {
	"streakng", NULL, NULL, NULL, "1981",
	"Streaking (set 1)\0", "Bad Colours", "Shoei", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, StreakngRomInfo, StreakngRomName, NULL, NULL, StreakngInputInfo, StreakngDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvStreaknga = {
	"streaknga", "streakng", NULL, NULL, "1981",
	"Streaking (set 2)\0", "Bad Colours", "Shoei", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, StreakngaRomInfo, StreakngaRomName, NULL, NULL, StreakngInputInfo, StreakngDIPInfo,
	GalInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvPacmanbl = {
	"pacmanbl", "puckman", NULL, NULL, "1981",
	"Pac-Man (Galaxian hardware, set 1)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, PacmanblRomInfo, PacmanblRomName, NULL, NULL, PacmanblInputInfo, PacmanblDIPInfo,
	PacmanblInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvPacmanbla = {
	"pacmanbla", "puckman", NULL, NULL, "1981",
	"Pac-Man (Galaxian hardware, set 2)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, PacmanblaRomInfo, PacmanblaRomName, NULL, NULL, PacmanblInputInfo, PacmanblDIPInfo,
	PacmanblaInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvPhoenxp2 = {
	"phoenxp2", "phoenix", NULL, NULL, "1981",
	"Phoenix Part 2\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Phoenxp2RomInfo, Phoenxp2RomName, NULL, NULL, Phoenxp2InputInfo, Phoenxp2DIPInfo,
	PacmanblInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAtlantisb = {
	"atlantisb", "atlantis", NULL, NULL, "1981",
	"Battle of Atlantis (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, AtlantisbRomInfo, AtlantisbRomName, NULL, NULL, Phoenxp2InputInfo, AtlantisbDIPInfo,
	AtlantisbInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Seperate sprite/tile roms with IRQ0 instead of NMI
static struct BurnRomInfo DevilfsgRomDesc[] = {
	{ "dfish1.7f",     0x01000, 0x2ab19698, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "dfish2.7h",     0x01000, 0x4e77f097, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "dfish3.7k",     0x01000, 0x3f16a4c6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "dfish4.7m",     0x01000, 0x11fc7e59, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },	
	
	{ "dfish5.1h",     0x01000, 0xace6e31f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "dfish6.1k",     0x01000, 0xd7a6c4c4, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "82s123.6e",     0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Devilfsg)
STD_ROM_FN(Devilfsg)

static void DevilfsgRearrangeRom()
{
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	memcpy(GalTempRom, GalZ80Rom1, GalZ80Rom1Size);
	memcpy(GalZ80Rom1 + 0x2000, GalTempRom + 0x0000, 0x800);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0800, 0x800);
	memcpy(GalZ80Rom1 + 0x2800, GalTempRom + 0x1000, 0x800);
	memcpy(GalZ80Rom1 + 0x0800, GalTempRom + 0x1800, 0x800);
	memcpy(GalZ80Rom1 + 0x3000, GalTempRom + 0x2000, 0x800);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x2800, 0x800);
	memcpy(GalZ80Rom1 + 0x3800, GalTempRom + 0x3000, 0x800);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x3800, 0x800);
	BurnFree(GalTempRom);
}

static INT32 DevilfsgInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = DevilfsgRearrangeRom;
	
	nRet = GalInit();
	
	GalNumChars = 0x100;
	GalNumSprites = 0x40;
	CharPlaneOffsets[1] = 0x4000;
	SpritePlaneOffsets[1] = 0x4000;
	
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x2000);
	GalTempRom = (UINT8*)BurnMalloc(0x1000);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(TempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x0000, TempRom + 0x0800, 0x800);
	memcpy(GalTempRom + 0x0800, TempRom + 0x1800, 0x800);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	memcpy(GalTempRom + 0x0000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x0800, TempRom + 0x1000, 0x800);	
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	BurnFree(TempRom);
	
	GalIrqType = GAL_IRQ_TYPE_IRQ0;
	
	return nRet;
}

struct BurnDriver BurnDrvDevilfsg = {
	"devilfsg", "devilfsh", NULL, NULL, "1984",
	"Devil Fish (Galaxian hardware, bootleg?)\0", NULL, "Vision / Artic", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, DevilfsgRomInfo, DevilfsgRomName, NULL, NULL, DevilfsgInputInfo, DevilfsgDIPInfo,
	DevilfsgInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Sound hardware replaced with AY8910
static struct BurnRomInfo ZigzagRomDesc[] = {
	{ "zz_d1.7l",      0x01000, 0x8cc08d81, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zz_d2.7k",      0x01000, 0x326d8d45, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zz_d4.7f",      0x01000, 0xa94ed92a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zz_d3.7h",      0x01000, 0xce5e7a00, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },

	{ "zz_6.1h",       0x01000, 0x780c162a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "zz_5.1k",       0x01000, 0xf3cdfec5, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "zzbpr_e9.bin",  0x00020, 0xaa486dd0, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Zigzag)
STD_ROM_FN(Zigzag)

static struct BurnRomInfo Zigzag2RomDesc[] = {
	{ "z1.7l",         0x01000, 0x4c28349a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zz_d2.7k",      0x01000, 0x326d8d45, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zz_d4.7f",      0x01000, 0xa94ed92a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "zz_d3.7h",      0x01000, 0xce5e7a00, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },

	{ "zz_6.1h",       0x01000, 0x780c162a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "zz_5.1k",       0x01000, 0xf3cdfec5, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "zzbpr_e9.bin",  0x00020, 0xaa486dd0, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Zigzag2)
STD_ROM_FN(Zigzag2)

static struct BurnRomInfo Ozon1RomDesc[] = {
	{ "rom1.bin",      0x01000, 0x54899e8b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2.bin",      0x01000, 0x3c90fbfc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom3.bin",      0x01000, 0x79fe313b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "rom7.bin",      0x00800, 0x464285e8, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom8.bin",      0x00800, 0x92056dcc, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "ozon1.clr",     0x00020, 0x605ea6e9, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Ozon1)
STD_ROM_FN(Ozon1)

void __fastcall ZigzagZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x4800 && a <= 0x4fff) {
		INT32 Offset = a - 0x4800;
		
		switch (Offset & 0x300) {
			case 0x000: {
				if ((Offset & 1) != 0) {
					if ((Offset & 2) == 0) {
						AY8910Write(0, 1, ZigzagAYLatch);
					} else {
						AY8910Write(0, 0, ZigzagAYLatch);
					}
				}
				break;
			}
			
			case 0x100: {
				ZigzagAYLatch = Offset & 0xff;
				break;
			}
		}
		return;
	}
	
	if (a >= 0x5800 && a <= 0x58ff) {
		INT32 Offset = a - 0x5800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000:
		case 0x6001: {
			// start_lamp_w
			return;
		}
		
		case 0x6002: {
			// coin_lock_w
			return;
		}
		
		case 0x6003: {
			// coin_count_0_w
			return;
		}
		
		case 0x7001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7002: {
			ZetMapArea(0x2000, 0x2fff, 0, GalZ80Rom1 + 0x2000 + (0x1000 * (d & 1)));
			ZetMapArea(0x2000, 0x2fff, 2, GalZ80Rom1 + 0x2000 + (0x1000 * (d & 1)));
			ZetMapArea(0x3000, 0x3fff, 0, GalZ80Rom1 + 0x2000 + (0x1000 * (~d & 1)));
			ZetMapArea(0x3000, 0x3fff, 2, GalZ80Rom1 + 0x2000 + (0x1000 * (~d & 1)));
			return;
		}
		
		case 0x7004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x7006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x7007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			// watchdog write
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Ozon1Z80Read(UINT16 a)
{
	switch (a) {
		case 0x8100: {
			return 0xff - (GalInput[0] | GalDip[0]);
		}
		
		case 0x8101: {
			return 0xff - (GalInput[1] | GalDip[1]);
		}
		
		case 0x8102: {
			return 0xff - (GalInput[2] | GalDip[2]);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall Ozon1Z80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6801: {
			// ???
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6802: {
			// coin_counter_0_w
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall Ozon1Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0x01: {
			AY8910Write(0, 0, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static void ZigzagInstallHandler()
{
	ZetOpen(0);
	ZetSetWriteHandler(ZigzagZ80Write);
	ZetClose();
}

static INT32 ZigzagInit()
{
	INT32 nRet;
	
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_ZIGZAGAY8910;
	GalPostLoadCallbackFunction = ZigzagInstallHandler;
	
	nRet = GalInit();
	
	GalNumChars = 0x100;
	GalNumSprites = 0x40;
	CharPlaneOffsets[1] = 0x4000;
	SpritePlaneOffsets[1] = 0x4000;
	
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x2000);
	GalTempRom = (UINT8*)BurnMalloc(0x1000);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(TempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x0000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x0800, TempRom + 0x1000, 0x800);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	memcpy(GalTempRom + 0x0000, TempRom + 0x0800, 0x800);
	memcpy(GalTempRom + 0x0800, TempRom + 0x1800, 0x800);	
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	BurnFree(TempRom);
	
	GalDrawBulletsFunction = NULL;
	
	GalSoundVolumeShift = 3;
	
	return nRet;
}

static void Ozon1PostLoad()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(Ozon1Z80Read);
	ZetSetWriteHandler(Ozon1Z80Write);
	ZetSetOutHandler(Ozon1Z80PortWrite);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x4000, 0x43ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x43ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x43ff, 2, GalZ80Ram1);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 0, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 1, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 2, GalVideoRam);
	ZetMapArea(0x5000, 0x50ff, 0, GalSpriteRam);
	ZetMapArea(0x5000, 0x50ff, 2, GalSpriteRam);
	ZetMemEnd();
	ZetClose();
}

static INT32 Ozon1Init()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = Ozon1PostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_ZIGZAGAY8910;
	
	nRet = GalInit();
	
	GalDrawBulletsFunction = NULL;
	GalRenderBackgroundFunction = NULL;
	
	return nRet;
}

struct BurnDriver BurnDrvZigzag = {
	"zigzag", NULL, NULL, NULL, "1982",
	"Zig Zag (Galaxian hardware, set 1)\0", NULL, "LAX", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, ZigzagRomInfo, ZigzagRomName, NULL, NULL, ZigzagInputInfo, ZigzagDIPInfo,
	ZigzagInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvZigzag2 = {
	"zigzag2", "zigzag", NULL, NULL, "1982",
	"Zig Zag (Galaxian hardware, set 2)\0", NULL, "LAX", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, Zigzag2RomInfo, Zigzag2RomName, NULL, NULL, ZigzagInputInfo, ZigzagDIPInfo,
	ZigzagInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvOzon1 = {
	"ozon1", NULL, NULL, NULL, "1983",
	"Ozon I\0", NULL, "Proma", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Ozon1RomInfo, Ozon1RomName, NULL, NULL, Ozon1InputInfo, Ozon1DIPInfo,
	Ozon1Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Multi-game select via external switch
static struct BurnRomInfo GmgalaxRomDesc[] = {
	{ "pcb1_pm1.bin",  0x01000, 0x19338c70, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pcb1_pm2.bin",  0x01000, 0x18db074d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pcb1_pm3.bin",  0x01000, 0xabb98b1d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pcb1_pm4.bin",  0x01000, 0x2403c78e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pcb1_gx1.bin",  0x01000, 0x2faa9f53, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pcb1_gx2.bin",  0x01000, 0x121c5f16, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "pcb1_gx3.bin",  0x01000, 0x02d81a21, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "pcb2gfx1.bin",  0x01000, 0x7021bbc0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "pcb2gfx3.bin",  0x01000, 0x089c922b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "pcb2gfx2.bin",  0x01000, 0x51bf58ee, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "pcb2gfx4.bin",  0x01000, 0x908fd0dc, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "gmgalax2.clr",  0x00020, 0x499f4440, BRF_GRA | GAL_ROM_PROM },
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Gmgalax)
STD_ROM_FN(Gmgalax)

UINT8 __fastcall GmgalaxZ80Read(UINT16 a)
{
	switch (a) {
		case 0x6000: {
			return GalInput[0] | ((GmgalaxSelectedGame) ? GalDip[3] : GalDip[0]);
		}
		
		case 0x6800: {
			return GalInput[1] | ((GmgalaxSelectedGame) ? GalDip[4] : GalDip[1]);
		}
		
		case 0x7000: {
			return GalInput[2] | ((GmgalaxSelectedGame) ? GalDip[5] : GalDip[2]);
		}
		
		case 0x7800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

static void GmgalaxPostLoad()
{
	ZetOpen(0);
	ZetSetReadHandler(GmgalaxZ80Read);
	ZetClose();
}

static INT32 GmgalaxInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = GmgalaxPostLoad;
	GalZ80Rom1Size = 0x1000;	
	
	nRet = GalInit();
	
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x1000);
	GalTempRom = (UINT8*)BurnMalloc(0x4000);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x0000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x1000, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x0800, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x1800, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 2, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x2000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x3000, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 3, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x2800, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x3800, TempRom + 0x0800, 0x800);	
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	BurnFree(TempRom);
	
	GalExtendTileInfoFunction = GmgalaxExtendTileInfo;
	GalExtendSpriteInfoFunction = GmgalaxExtendSpriteInfo;
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	GameIsGmgalax = 1;
	
	return nRet;
}

struct BurnDriverD BurnDrvGmgalax = {
	"gmgalax", NULL, NULL, NULL, "1981",
	"Ghostmuncher Galaxian (bootleg)\0", NULL, "LAX", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, GmgalaxRomInfo, GmgalaxRomName, NULL, NULL, GmgalaxInputInfo, GmgalaxDIPInfo,
	GmgalaxInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Multi-game select via menu, with different graphic and sprite roms
static struct BurnRomInfo Fourin1RomDesc[] = {
	{ "rom1a",         0x01000, 0xce1af4d9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1b",         0x01000, 0x18484f9b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1c",         0x01000, 0x83248a8b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1d",         0x01000, 0x053f6da0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1e",         0x01000, 0x43c546f3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1f",         0x01000, 0x3a086b46, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1g",         0x01000, 0xac0e2050, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1h",         0x01000, 0xdc11a513, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1i",         0x01000, 0xa5fb6be4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom1j",         0x01000, 0x9054cfbe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2c",         0x01000, 0x7cd98e11, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2d",         0x01000, 0x9402f32e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2e",         0x01000, 0x468e81df, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2g",         0x01000, 0xb1ce3976, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2h",         0x01000, 0x7eab5670, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2i",         0x01000, 0x44565ac5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "rom4b",         0x01000, 0x7e6495af, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom3b",         0x01000, 0x7475f72f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom4c",         0x00800, 0x3355d46d, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom3c",         0x00800, 0xac755a25, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom4d",         0x01000, 0xbbdddb65, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom3d",         0x01000, 0x91a00204, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom4e",         0x01000, 0x0cb9e297, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom3e",         0x01000, 0xa1fe77f9, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Fourin1)
STD_ROM_FN(Fourin1)

UINT8 __fastcall Fourin1Z80Read(UINT16 a)
{
	switch (a) {
		case 0x6000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x6800: {
			return ((GalInput[1] | GalDip[1]) & ~0xc0) | (GalDip[3 + Fourin1Bank] & 0xc0);
		}
		
		case 0x7000: {
			return ((GalInput[2] | GalDip[2]) & 0x04) | (GalDip[3 + Fourin1Bank] & ~0xc4);
		}
		
		case 0x7800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall Fourin1Z80Write(UINT16 a, UINT8 d)
{
	if (a < 0x4000) return;
	
	if (a >= 0x5800 && a <= 0x58ff) {
		INT32 Offset = a - 0x5800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000:
		case 0x6001: {
			// start_lamp_w
			return;
		}
		
		case 0x6002: {
			// coin_lock_w
			return;
		}
		
		case 0x6003: {
			// coin_count_0_w
			return;
		}
		
		case 0x6004:
		case 0x6005:
		case 0x6006:
		case 0x6007: {
			GalaxianLfoFreqWrite(a - 0x6004, d);
			return;
		}
		
		case 0x6800:
		case 0x6801:
		case 0x6802:
		case 0x6803:
		case 0x6804:
		case 0x6805:
		case 0x6806:
		case 0x6807: {
			GalaxianSoundWrite(a - 0x6800, d);
			return;
		}
		
		case 0x7001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x7006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x7007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			GalPitch = d;
			return;
		}
		
		case 0x8000: {
			Fourin1Bank = d & 0x03;
			ZetMapArea(0x0000, 0x3fff, 0, GalZ80Rom1 + 0x2000 + (Fourin1Bank * 0x4000));
			ZetMapArea(0x0000, 0x3fff, 2, GalZ80Rom1 + 0x2000 + (Fourin1Bank * 0x4000));
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void Fourin1PostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(0x3000);
	memcpy(GalTempRom, GalZ80Rom1 + 0xd000, 0x3000);
	memset(GalZ80Rom1 + 0xd000, 0xff, 0x1000);
	memcpy(GalZ80Rom1 + 0xe000, GalTempRom, 0x3000);
	memset(GalZ80Rom1 + 0x11000, 0xff, 0x1000);
	BurnFree(GalTempRom);
	
	for (UINT32 i = 0; i < GalZ80Rom1Size; i++) {
		GalZ80Rom1[i] = GalZ80Rom1[i] ^ (i & 0xff);
	}

	ZetOpen(0);
	ZetSetReadHandler(Fourin1Z80Read);
	ZetSetWriteHandler(Fourin1Z80Write);
	ZetMapArea(0x0000, 0x3fff, 0, GalZ80Rom1 + 0x2000 + (Fourin1Bank * 0x4000));
	ZetMapArea(0x0000, 0x3fff, 2, GalZ80Rom1 + 0x2000 + (Fourin1Bank * 0x4000));
	ZetMapArea(0xc000, 0xdfff, 0, GalZ80Rom1);
	ZetMapArea(0xc000, 0xdfff, 2, GalZ80Rom1);
	ZetClose();
}

static INT32 Fourin1Init()
{
	INT32 nRet;
	
	Fourin1Bank = 0;
	GalPostLoadCallbackFunction = Fourin1PostLoad;
	GalZ80Rom1Size = 0x2000;
	GalTilesSharedRomSize = 0x1000;
	
	nRet = GalInit();
	
	GalNumChars = 1024;
	GalNumSprites = 256;
	CharPlaneOffsets[1] = GalNumChars * 8 * 8;
	SpritePlaneOffsets[1] = GalNumSprites * 16 * 16;
	
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x1000);
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x4000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x0000, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x6000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x2000, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 2, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x4800, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x0800, TempRom + 0x0000, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 3, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x6800, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x2800, TempRom + 0x0000, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 4, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x5000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x1000, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 5, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x7000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x3000, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 6, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x5800, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x1800, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 7, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x7800, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x3800, TempRom + 0x0800, 0x800);	
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom + 0x4000, GalSprites);
	BurnFree(GalTempRom);
	BurnFree(TempRom);
	
	GalExtendTileInfoFunction = Fourin1ExtendTileInfo;
	GalExtendSpriteInfoFunction = Fourin1ExtendSpriteInfo;
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

struct BurnDriverD BurnDrvFourin1 = {
	"4in1", NULL, NULL, NULL, "1981",
	"4 Fun in 1\0", NULL, "Armenia / Food and Fun", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, Fourin1RomInfo, Fourin1RomName, NULL, NULL, Fourin1InputInfo, Fourin1DIPInfo,
	Fourin1Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Moon Cresta on Galaxian hardware with different memory map
static struct BurnRomInfo MooncrstRomDesc[] = {
	{ "mc1",           0x00800, 0x7d954a7a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc2",           0x00800, 0x44bb7cfa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc3",           0x00800, 0x9c412104, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc4",           0x00800, 0x7e9b1ab5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc5.7r",        0x00800, 0x16c759af, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc6.8d",        0x00800, 0x69bcafdb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc7.8e",        0x00800, 0xb50dbc46, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc8",           0x00800, 0x18ca312b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mcs_b",         0x00800, 0xfb0f1f81, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_d",         0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_a",         0x00800, 0x631ebb5a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_c",         0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrst)
STD_ROM_FN(Mooncrst)

static struct BurnRomInfo MooncrstukRomDesc[] = {
	{ "mc1",           0x00800, 0x7d954a7a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc2",           0x00800, 0x44bb7cfa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc3",           0x00800, 0x9c412104, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc4",           0x00800, 0x7e9b1ab5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc5.7r",        0x00800, 0x16c759af, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc6.8d",        0x00800, 0x69bcafdb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc7.8e",        0x00800, 0xb50dbc46, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8_uk.bin",      0x00800, 0xce727ad4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mcs_b",         0x00800, 0xfb0f1f81, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_d",         0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_a",         0x00800, 0x631ebb5a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_c",         0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrstuk)
STD_ROM_FN(Mooncrstuk)

static struct BurnRomInfo MooncrstuRomDesc[] = {
	{ "smc1f",         0x00800, 0x389ca0d6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "smc2f",         0x00800, 0x410ab430, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "smc3f",         0x00800, 0xa6b4144b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "smc4f",         0x00800, 0x4cc046fe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e5",            0x00800, 0x06d378a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bepr199",       0x00800, 0x6e84a927, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e7",            0x00800, 0xb45af1e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "smc8f",         0x00800, 0xf42164c5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mcs_b",         0x00800, 0xfb0f1f81, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_d",         0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_a",         0x00800, 0x631ebb5a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_c",         0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrstu)
STD_ROM_FN(Mooncrstu)

static struct BurnRomInfo MooncrstoRomDesc[] = {
	{ "mc1.7d",        0x00800, 0x92a86aac, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc2.7e",        0x00800, 0x438c2b4b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc3.7j",        0x00800, 0x67e3d21d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc4.7p",        0x00800, 0xf4db39f6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc5.7r",        0x00800, 0x16c759af, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc6.8d",        0x00800, 0x69bcafdb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc7.8e",        0x00800, 0xb50dbc46, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mc8.8h",        0x00800, 0x7e2b1928, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mcs_b",         0x00800, 0xfb0f1f81, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_d",         0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_a",         0x00800, 0x631ebb5a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_c",         0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrsto)
STD_ROM_FN(Mooncrsto)

static struct BurnRomInfo MooncrstgRomDesc[] = {
	{ "epr194",        0x00800, 0x0e5582b1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr195",        0x00800, 0x12cb201b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr196",        0x00800, 0x18255614, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr197",        0x00800, 0x05ac1466, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr198",        0x00800, 0xc28a2e8f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr199",        0x00800, 0x5a4571de, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr200",        0x00800, 0xb7c85bf1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr201",        0x00800, 0x2caba07f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr203",        0x00800, 0xbe26b561, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_d",         0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr202",        0x00800, 0x26c7e800, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_c",         0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrstg)
STD_ROM_FN(Mooncrstg)

static void MooncrstEncryptedPostLoad()
{
	for (UINT32 Offset = 0; Offset < GalZ80Rom1Size; Offset++) {
		UINT8 Data = GalZ80Rom1[Offset];
		UINT8 Res = Data;
		if (BIT(Data, 1)) Res ^= 0x40;
		if (BIT(Data, 5)) Res ^= 0x04;
		if ((Offset & 1) == 0) Res = BITSWAP08(Res, 7, 2, 5, 4, 3, 6, 1, 0);
		GalZ80Rom1[Offset] = Res;
	}
	
	MapMooncrst();
}

static INT32 MooncrstEncryptedInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MooncrstEncryptedPostLoad;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = MooncrstExtendSpriteInfo;

	return nRet;
}

static INT32 MooncrstInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapMooncrst;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = MooncrstExtendSpriteInfo;

	return nRet;
}

struct BurnDriver BurnDrvMooncrst = {
	"mooncrst", NULL, NULL, NULL, "1980",
	"Moon Cresta (Nichibutsu)\0", NULL, "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MooncrstRomInfo, MooncrstRomName, NULL, NULL, OmegaInputInfo, MooncrstDIPInfo,
	MooncrstEncryptedInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMooncrstuk = {
	"mooncrstuk", "mooncrst", NULL, NULL, "1980",
	"Moon Cresta (Nichibutsu UK)\0", NULL, "Nichibutsu UK", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MooncrstukRomInfo, MooncrstukRomName, NULL, NULL, OmegaInputInfo, MooncrstDIPInfo,
	MooncrstEncryptedInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMooncrstu = {
	"mooncrstu", "mooncrst", NULL, NULL, "1980",
	"Moon Cresta (Nichibutsu USA, unencrypted)\0", NULL, "Nichibutsu USA", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MooncrstuRomInfo, MooncrstuRomName, NULL, NULL, OmegaInputInfo, MooncrstDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMooncrsto = {
	"mooncrsto", "mooncrst", NULL, NULL, "1980",
	"Moon Cresta (Nichibutsu, old rev)\0", NULL, "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MooncrstoRomInfo, MooncrstoRomName, NULL, NULL, OmegaInputInfo, MooncrsaDIPInfo,
	MooncrstEncryptedInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMooncrstg = {
	"mooncrstg", "mooncrst", NULL, NULL, "1980",
	"Moon Cresta (Gremlin)\0", NULL, "Gremlin", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MooncrstgRomInfo, MooncrstgRomName, NULL, NULL, OmegaInputInfo, MooncrsgDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Straight Moon Cresta Rip-Offs on basic Moon Cresta hardware
static struct BurnRomInfo MooncrsbRomDesc[] = {
	{ "bepr194",       0x00800, 0x6a23ec6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bepr195",       0x00800, 0xee262ff2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f03.bin",       0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f04.bin",       0x00800, 0x4c6a5a6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e5",            0x00800, 0x06d378a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bepr199",       0x00800, 0x6e84a927, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e7",            0x00800, 0xb45af1e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bepr201",       0x00800, 0x66da55d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr203",        0x00800, 0xbe26b561, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_d",         0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr202",        0x00800, 0x26c7e800, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_c",         0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrsb)
STD_ROM_FN(Mooncrsb)

static struct BurnRomInfo Mooncrs2RomDesc[] = {
	{ "f8.bin",        0x00800, 0xd36003e5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bepr195",       0x00800, 0xee262ff2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f03.bin",       0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f04.bin",       0x00800, 0x4c6a5a6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e5",            0x00800, 0x06d378a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bepr199",       0x00800, 0x6e84a927, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e7",            0x00800, 0xb45af1e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m7.bin",        0x00800, 0x957ee078, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "1h_1_10.bin",   0x00800, 0x528da705, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "12.chr",        0x00800, 0x5a4b17ea, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "1k_1_11.bin",   0x00800, 0x4e79ff6b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "11.chr",        0x00800, 0xe0edccbd, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrs2)
STD_ROM_FN(Mooncrs2)

static struct BurnRomInfo Mooncrs3RomDesc[] = {
	{ "b1.7f",         0x01000, 0x0b28cd8a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b2.7h",         0x01000, 0x74a6f0ca, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b3.7j",         0x01000, 0xeeb34cc9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b4.7k",         0x01000, 0x714330e5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "o.1h",          0x00800, 0x528da705, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "q.1h",          0x00800, 0x5a4b17ea, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "p.1k",          0x00800, 0x4e79ff6b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "r.1k",          0x00800, 0xe0edccbd, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrs3)
STD_ROM_FN(Mooncrs3)

static struct BurnRomInfo FantaziaRomDesc[] = {
	{ "f01.bin",       0x00800, 0xd3e23863, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f02.bin",       0x00800, 0x63fa4149, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f03.bin",       0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f04.bin",       0x00800, 0x4c6a5a6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f09.bin",       0x00800, 0x75fd5ca1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f10.bin",       0x00800, 0xe4da2dd4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f11.bin",       0x00800, 0x42869646, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f12.bin",       0x00800, 0xa48d7fb0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "1h_1_10.bin",   0x00800, 0x528da705, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_d",         0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "1k_1_11.bin",   0x00800, 0x4e79ff6b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_c",         0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "fantazia.clr",  0x00020, 0xa84ff0af, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Fantazia)
STD_ROM_FN(Fantazia)

static struct BurnRomInfo EagleRomDesc[] = {
	{ "e1",            0x00800, 0x224c9526, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e2",            0x00800, 0xcc538ebd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f03.bin",       0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f04.bin",       0x00800, 0x4c6a5a6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e5",            0x00800, 0x06d378a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e6",            0x00800, 0x0dea20d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e7",            0x00800, 0xb45af1e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e8",            0x00800, 0xc437a876, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "e10",           0x00800, 0x40ce58bf, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e12",           0x00800, 0x628fdeed, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e9",            0x00800, 0xba664099, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e11",           0x00800, 0xee4ec5fd, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Eagle)
STD_ROM_FN(Eagle)

static struct BurnRomInfo Eagle2RomDesc[] = {
	{ "e1.7f",         0x00800, 0x45aab7a3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e2",            0x00800, 0xcc538ebd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f03.bin",       0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f04.bin",       0x00800, 0x4c6a5a6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e5",            0x00800, 0x06d378a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e6.6",          0x00800, 0x9f09f8c6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e7",            0x00800, 0xb45af1e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e8",            0x00800, 0xc437a876, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "e10.2",         0x00800, 0x25b38ebd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e12",           0x00800, 0x628fdeed, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e9",            0x00800, 0xba664099, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e11",           0x00800, 0xee4ec5fd, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Eagle2)
STD_ROM_FN(Eagle2)

static struct BurnRomInfo Eagle3RomDesc[] = {
	{ "e1",            0x00800, 0x224c9526, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e2",            0x00800, 0xcc538ebd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f03.bin",       0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f04.bin",       0x00800, 0x4c6a5a6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e5",            0x00800, 0x06d378a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e6",            0x00800, 0x0dea20d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e7",            0x00800, 0xb45af1e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e8",            0x00800, 0xc437a876, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "e10a",          0x00800, 0xe3c63d4c, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e12",           0x00800, 0x628fdeed, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e9a",           0x00800, 0x59429e47, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "e11",           0x00800, 0xee4ec5fd, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Eagle3)
STD_ROM_FN(Eagle3)

static struct BurnRomInfo SpctbirdRomDesc[] = {
	{ "tssa-7f",       0x00800, 0x45aab7a3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tssa-7h",       0x00800, 0x8b328f48, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tssa-7k",       0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tssa-7m",       0x00800, 0x99c9166d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tssa-5",        0x00800, 0x797b6261, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tssa-6",        0x00800, 0x4825692c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tssa-7",        0x00800, 0xb45af1e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tssa-8",        0x00800, 0xc9b77b85, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "tssb-2",        0x00800, 0x7d23e1f2, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "tssb-4",        0x00800, 0xe4977833, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "tssb-1",        0x00800, 0x9b9267c3, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "tssb-3",        0x00800, 0x5ca5e233, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Spctbird)
STD_ROM_FN(Spctbird)

static struct BurnRomInfo SmooncrsRomDesc[] = {
	{ "927",           0x00800, 0x55c5b994, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "928a",          0x00800, 0x77ae26d3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "929",           0x00800, 0x716eaa10, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "930",           0x00800, 0xcea864f2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "931",           0x00800, 0x702c5f51, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "932a",          0x00800, 0xe6a2039f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "933",           0x00800, 0x73783cee, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "934",           0x00800, 0xc1a14aa2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr203",        0x00800, 0xbe26b561, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_d",         0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr202",        0x00800, 0x26c7e800, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mcs_c",         0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Smooncrs)
STD_ROM_FN(Smooncrs)

static struct BurnRomInfo SstarcrsRomDesc[] = {
	{ "ss1",           0x00800, 0x2ff72897, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ss2",           0x00800, 0x565e7880, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ss3",           0x00800, 0xa1939def, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ss4",           0x00800, 0xa332e012, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ss5",           0x00800, 0xb9e58453, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ss6",           0x00800, 0x7cbb5bc8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ss7",           0x00800, 0x57713b91, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ss8",           0x00800, 0xc857e898, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ss10",          0x00800, 0x2a95b8ea, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ss12",          0x00800, 0xb92c4c30, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ss9",           0x00800, 0x3661e084, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ss11",          0x00800, 0x95613048, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Sstarcrs)
STD_ROM_FN(Sstarcrs)

static struct BurnRomInfo MooncmwRomDesc[] = {
	{ "60.1x",         0x00800, 0x322859e6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "61.2x",         0x00800, 0xc249902d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "62.3x",         0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "63.4x",         0x00800, 0x4c6a5a6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "64.5x",         0x00800, 0x06d378a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "65.6x",         0x00800, 0x6e84a927, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "66.7x",         0x00800, 0xf23cd8ce, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "67.8x",         0x00800, 0x66da55d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "68.1h",         0x01000, 0x78663d86, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "69.1k",         0x01000, 0x162c50d3, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "prom-sn74s288n-71.6l", 0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncmw)
STD_ROM_FN(Mooncmw)

static struct BurnRomInfo SpcdragRomDesc[] = {
	{ "a.bin",         0x00800, 0x38cc9839, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b.bin",         0x00800, 0x419fa8d6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "c.bin",         0x00800, 0xa1939def, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "d.bin",         0x00800, 0xcbcf17c5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "em.bin",        0x00800, 0xeb81c19c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "fm.bin",        0x00800, 0x757b7672, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g.bin",         0x00800, 0x57713b91, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "h.bin",         0x00800, 0x159ad847, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "203.bin",       0x00800, 0xa2e82527, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "172.bin",       0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "202.bin",       0x00800, 0x80c3ad74, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "171.bin",       0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Spcdrag)
STD_ROM_FN(Spcdrag)

static struct BurnRomInfo SpcdragaRomDesc[] = {
	{ "1.7g",          0x00800, 0x38cc9839, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.7g",          0x00800, 0x29e00ae4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.7g",          0x00800, 0xa1939def, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.7g",          0x00800, 0x068f8830, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.10g",         0x00800, 0x32cd9adc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6.10g",         0x00800, 0x50db67c5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7.10g",         0x00800, 0x22415271, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8.10g",         0x00800, 0x159ad847, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "a2.7a",         0x00800, 0x38b042dd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "a4.7a",         0x00800, 0x5a4b17ea, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "a1.9a",         0x00800, 0x24441ab3, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "a3.9a",         0x00800, 0xe0edccbd, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "prom_6331.10f", 0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Spcdraga)
STD_ROM_FN(Spcdraga)

static struct BurnRomInfo SteraRomDesc[] = {
	{ "stera.1",       0x00800, 0xcd04fea8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "stera.2",       0x00800, 0xccd1878e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "stera.3",       0x00800, 0x29a2b0ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "stera.4",       0x00800, 0x4c6a5a6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "stera.5",       0x00800, 0x06d378a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "stera.6",       0x00800, 0x6e84a927, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "stera.7",       0x00800, 0xb45af1e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "stera.8",       0x00800, 0x37f19956, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },	
	
	{ "stera.10",      0x00800, 0x528da705, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "stera.12",      0x00800, 0x13932a15, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "stera.11",      0x00800, 0x4e79ff6b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "stera.9",       0x00800, 0x24cfd145, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "stera.6l",      0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Stera)
STD_ROM_FN(Stera)

static struct BurnRomInfo MooncrgxRomDesc[] = {
	{ "1",             0x00800, 0x84cf420b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2",             0x00800, 0x4c2a61a1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3",             0x00800, 0x1962523a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4",             0x00800, 0x75dca896, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5",             0x00800, 0x32483039, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6",             0x00800, 0x43f2ab89, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7",             0x00800, 0x1e9c168c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8",             0x00800, 0x5e09da94, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "1h_1_10.bin",   0x00800, 0x528da705, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "12.chr",        0x00800, 0x5a4b17ea, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "9.chr",         0x00800, 0x70df525c, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "11.chr",        0x00800, 0xe0edccbd, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mooncrgx)
STD_ROM_FN(Mooncrgx)

static struct BurnRomInfo MoonqsrRomDesc[] = {
	{ "mq1",           0x00800, 0x132c13ec, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mq2",           0x00800, 0xc8eb74f1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mq3",           0x00800, 0x33965a89, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mq4",           0x00800, 0xa3861d17, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mq5",           0x00800, 0x8bcf9c67, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mq6",           0x00800, 0x5750cda9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mq7",           0x00800, 0x78d7fe5b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mq8",           0x00800, 0x4919eed5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mqb",           0x00800, 0xb55ec806, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mqd",           0x00800, 0x9e7d0e13, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mqa",           0x00800, 0x66eee0db, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mqc",           0x00800, 0xa6db5b0d, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "vid_e6.bin",    0x00020, 0x0b878b54, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Moonqsr)
STD_ROM_FN(Moonqsr)

static struct BurnRomInfo Moonal2RomDesc[] = {
	{ "ali1",          0x00400, 0x0dcecab4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali2",          0x00400, 0xc6ee75a7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali3",          0x00400, 0xcd1be7e9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali4",          0x00400, 0x83b03f08, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali5",          0x00400, 0x6f3cf61d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali6",          0x00400, 0xe169d432, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali7",          0x00400, 0x41f64b73, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali8",          0x00400, 0xf72ee876, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali9",          0x00400, 0xb7fb763c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali10",         0x00400, 0xb1059179, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali11",         0x00400, 0x9e79a1c6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ali13.1h",      0x00800, 0xa1287bf6, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ali12.1k",      0x00800, 0x528f1481, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Moonal2)
STD_ROM_FN(Moonal2)

static struct BurnRomInfo Moonal2bRomDesc[] = {
	{ "ali1",          0x00400, 0x0dcecab4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali2",          0x00400, 0xc6ee75a7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "md-2",          0x00800, 0x8318b187, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali5",          0x00400, 0x6f3cf61d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali6",          0x00400, 0xe169d432, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali7",          0x00400, 0x41f64b73, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali8",          0x00400, 0xf72ee876, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali9",          0x00400, 0xb7fb763c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali10",         0x00400, 0xb1059179, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "md-6",          0x00800, 0x9cc973e0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ali13.1h",      0x00800, 0xa1287bf6, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ali12.1k",      0x00800, 0x528f1481, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bpr",        0x00020, 0xc3ac9467, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Moonal2b)
STD_ROM_FN(Moonal2b)

static struct BurnRomInfo ThepitmRomDesc[] = {
	{ "1.bin",         0x00800, 0x0f78d6ea, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.bin",         0x00800, 0xebacc6eb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.bin",         0x00800, 0x14fd0706, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.bin",         0x00800, 0x613e920f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.bin",         0x00800, 0x5a791f3f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6.bin",         0x00800, 0x0bb37f51, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7.bin",         0x00800, 0x4dfdec6f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8.bin",         0x00800, 0xa39a9189, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "9.bin",         0x00800, 0x2eb90e07, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "1h.bin",        0x00800, 0x00dce65f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "1k.bin",        0x00800, 0x3ec0056e, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "6l.bin",        0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Thepitm)
STD_ROM_FN(Thepitm)

void __fastcall MooncrgxZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5800 && a <= 0x58ff) {
		INT32 Offset = a - 0x5800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000:
		case 0x6001:
		case 0x6002: {
			GalGfxBank[a - 0x6000] = d;
			return;
		}
		
		case 0x6003: {
			// coin_count_0_w
			return;
		}
		
		case 0x6004:
		case 0x6005:
		case 0x6006:
		case 0x6007: {
			GalaxianLfoFreqWrite(a - 0x6004, d);
			return;
		}
		
		case 0x6800:
		case 0x6801:
		case 0x6802:
		case 0x6803:
		case 0x6804:
		case 0x6805:
		case 0x6806:
		case 0x6807: {
			GalaxianSoundWrite(a - 0x6800, d);
			return;
		}
		
		case 0x7001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x7006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x7007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall MoonqsrZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa004:
		case 0xa005:
		case 0xa006:
		case 0xa007: {
			GalaxianLfoFreqWrite(a - 0xa004, d);
			return;
		}
		
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb000: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Prog Write %x, %x\n"), a, d);
}

void __fastcall ThepitmZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa004:
		case 0xa005:
		case 0xa006:
		case 0xa007: {
			GalaxianLfoFreqWrite(a - 0xa004, d);
			return;
		}
		
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb004: {
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void Mooncrs2RearrangeGfx()
{
	UINT8* TempRom = (UINT8*)BurnMalloc(0x800);
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 2, 1);
	BurnLoadRom(TempRom, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	memcpy(GalTempRom + 0x0800, TempRom + 0x0000, 0x200);
	memcpy(GalTempRom + 0x0c00, TempRom + 0x0200, 0x200);
	memcpy(GalTempRom + 0x0a00, TempRom + 0x0400, 0x200);
	memcpy(GalTempRom + 0x0e00, TempRom + 0x0600, 0x200);
	BurnLoadRom(TempRom, GAL_ROM_OFFSET_TILES_SHARED + 3, 1);
	memcpy(GalTempRom + 0x1800, TempRom + 0x0000, 0x200);
	memcpy(GalTempRom + 0x1c00, TempRom + 0x0200, 0x200);
	memcpy(GalTempRom + 0x1a00, TempRom + 0x0400, 0x200);
	memcpy(GalTempRom + 0x1e00, TempRom + 0x0600, 0x200);	
	BurnFree(TempRom);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);	
	BurnFree(GalTempRom);
}

static INT32 Mooncrs2Init()
{
	GalPostLoadCallbackFunction = MapMooncrst;
	
	INT32 nRet = GalInit();
	
	Mooncrs2RearrangeGfx();
	
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = MooncrstExtendSpriteInfo;
	
	return nRet;
}

static void Mooncrs3PostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	memcpy(GalTempRom, GalZ80Rom1, GalZ80Rom1Size);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0000, 0x800);
	memcpy(GalZ80Rom1 + 0x2000, GalTempRom + 0x0800, 0x800);
	memcpy(GalZ80Rom1 + 0x0800, GalTempRom + 0x1000, 0x800);
	memcpy(GalZ80Rom1 + 0x2800, GalTempRom + 0x1800, 0x800);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x2000, 0x800);
	memcpy(GalZ80Rom1 + 0x3000, GalTempRom + 0x2800, 0x800);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x3000, 0x800);
	memcpy(GalZ80Rom1 + 0x3800, GalTempRom + 0x3800, 0x800);
	BurnFree(GalTempRom);
	
	MapMooncrst();
}

static INT32 Mooncrs3Init()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = Mooncrs3PostLoad;
	
	nRet = GalInit();
	
	Mooncrs2RearrangeGfx();
	
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = MooncrstExtendSpriteInfo;
	
	return nRet;
}

static void MooncrgxInstallHandler()
{
	ZetOpen(0);
	ZetSetWriteHandler(MooncrgxZ80Write);
	ZetClose();
}

static INT32 MooncrgxInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MooncrgxInstallHandler;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = MooncrstExtendSpriteInfo;
	
	return nRet;
}

static void MoonqsrDecrypt()
{
	for (UINT32 Offset = 0; Offset < GalZ80Rom1Size; Offset++) {
		UINT8 Data = GalZ80Rom1[Offset];
		UINT8 Res = Data;
		if (BIT(Data, 1)) Res ^= 0x40;
		if (BIT(Data, 5)) Res ^= 0x04;
		if ((Offset & 1) == 0) Res = BITSWAP08(Res, 7, 2, 5, 4, 3, 6, 1, 0);
		GalZ80Rom1Op[Offset] = Res;
	}
	
	MapMooncrst();
	
	ZetOpen(0);
	ZetSetWriteHandler(MoonqsrZ80Write);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1Op, GalZ80Rom1);
	ZetClose();
}

static INT32 MoonqsrInit()
{
	INT32 nRet;
	
	GalZ80Rom1Op = (UINT8*)BurnMalloc(0x4000);
		
	GalPostLoadCallbackFunction = MoonqsrDecrypt;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = MoonqsrExtendTileInfo;
	GalExtendSpriteInfoFunction = MoonqsrExtendSpriteInfo;
	
	return nRet;
}

static INT32 Moonal2Init()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapMooncrst;
	
	nRet = GalInit();
	
	return nRet;
}

static void ThepitmPostLoad()
{
	MapMooncrst();
	
	ZetOpen(0);
	ZetMapArea(0x0000, 0x47ff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x47ff, 2, GalZ80Rom1);
	ZetSetWriteHandler(ThepitmZ80Write);
	ZetClose();
}

static INT32 ThepitmInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = ThepitmPostLoad;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = MooncrstExtendSpriteInfo;
	
	return nRet;
}

struct BurnDriver BurnDrvMooncrsb = {
	"mooncrsb", "mooncrst", NULL, NULL, "1980",
	"Moon Cresta (bootleg set 1)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MooncrsbRomInfo, MooncrsbRomName, NULL, NULL, OmegaInputInfo, MooncrsaDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMooncrs2 = {
	"mooncrs2", "mooncrst", NULL, NULL, "1980",
	"Moon Cresta (bootleg set 2)\0", NULL, "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Mooncrs2RomInfo, Mooncrs2RomName, NULL, NULL, OmegaInputInfo, MooncrsaDIPInfo,
	Mooncrs2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMooncrs3 = {
	"mooncrs3", "mooncrst", NULL, NULL, "1980",
	"Moon Cresta (bootleg set 3)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Mooncrs3RomInfo, Mooncrs3RomName, NULL, NULL, OmegaInputInfo, MooncrstDIPInfo,
	Mooncrs3Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFantazia = {
	"fantazia", "mooncrst", NULL, NULL, "1980",
	"Fantazia (bootleg?)\0", NULL, "SubElectro", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, FantaziaRomInfo, FantaziaRomName, NULL, NULL, OmegaInputInfo, FantaziaDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvEagle = {
	"eagle", "mooncrst", NULL, NULL, "1980",
	"Eagle (set 1)\0", NULL, "Centuri", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, EagleRomInfo, EagleRomName, NULL, NULL, OmegaInputInfo, MooncrsaDIPInfo,
	Mooncrs2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvEagle2 = {
	"eagle2", "mooncrst", NULL, NULL, "1980",
	"Eagle (set 2)\0", NULL, "Centuri", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Eagle2RomInfo, Eagle2RomName, NULL, NULL, OmegaInputInfo, Eagle2DIPInfo,
	Mooncrs2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvEagle3 = {
	"eagle3", "mooncrst", NULL, NULL, "1980",
	"Eagle (set 3)\0", NULL, "Centuri", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Eagle3RomInfo, Eagle3RomName, NULL, NULL, OmegaInputInfo, MooncrsaDIPInfo,
	Mooncrs2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSpctbird = {
	"spctbird", "mooncrst", NULL, NULL, "1981?",
	"Space Thunderbird\0", NULL, "Fortrek", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SpctbirdRomInfo, SpctbirdRomName, NULL, NULL, OmegaInputInfo, Eagle2DIPInfo,
	Mooncrs2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSmooncrs = {
	"smooncrs", "mooncrst", NULL, NULL, "1981?",
	"Super Moon Cresta\0", NULL, "Gremlin", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SmooncrsRomInfo, SmooncrsRomName, NULL, NULL, SmooncrsInputInfo, SmooncrsDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSstarcrs = {
	"sstarcrs", "mooncrst", NULL, NULL, "1980?",
	"Super Star Crest\0", NULL, "Nichibutsu (Taito do Brasil license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SstarcrsRomInfo, SstarcrsRomName, NULL, NULL, OmegaInputInfo, MooncrsgDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMooncmw = {
	"mooncmw", "mooncrst", NULL, NULL, "198?",
	"Moon War (Moon Cresta bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MooncmwRomInfo, MooncmwRomName, NULL, NULL, OmegaInputInfo, MooncrsaDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSpcdrag = {
	"spcdrag", "mooncrst", NULL, NULL, "1980",
	"Space Dragon (Moon Cresta bootleg, set 1)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SpcdragRomInfo, SpcdragRomName, NULL, NULL, SmooncrsInputInfo, SmooncrsDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSpcdraga = {
	"spcdraga", "mooncrst", NULL, NULL, "1980",
	"Space Dragon (Moon Cresta bootleg, set 2)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SpcdragaRomInfo, SpcdragaRomName, NULL, NULL, SmooncrsInputInfo, SmooncrsDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvStera = {
	"stera", "mooncrst", NULL, NULL, "1980",
	"Steraranger (Moon Cresta bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SteraRomInfo, SteraRomName, NULL, NULL, SmooncrsInputInfo, SmooncrsDIPInfo,
	MooncrstInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMooncrgx = {
	"mooncrgx", "mooncrst", NULL, NULL, "1980",
	"Moon Cresta (Galaxian hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MooncrgxRomInfo, MooncrgxRomName, NULL, NULL, OmegaInputInfo, MooncrgxDIPInfo,
	MooncrgxInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMoonqsr = {
	"moonqsr", NULL, NULL, NULL, "1980",
	"Moon Quasar\0", NULL, "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MoonqsrRomInfo, MoonqsrRomName, NULL, NULL, OmegaInputInfo, MoonqsrDIPInfo,
	MoonqsrInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMoonal2 = {
	"moonal2", "galaxian", NULL, NULL, "1980",
	"Moon Alien Part 2\0", NULL, "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Moonal2RomInfo, Moonal2RomName, NULL, NULL, GalaxianInputInfo, Moonal2DIPInfo,
	Moonal2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMoonal2b = {
	"moonal2b", "galaxian", NULL, NULL, "1980",
	"Moon Alien Part 2 (older version)\0", NULL, "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Moonal2bRomInfo, Moonal2bRomName, NULL, NULL, GalaxianInputInfo, Moonal2DIPInfo,
	Moonal2Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvThepitm = {
	"thepitm", "thepit", NULL, NULL, "198?",
	"The Pit (bootleg on Moon Quasar hardware)\0", NULL, "bootleg (KZH)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, ThepitmRomInfo, ThepitmRomName, NULL, NULL, ThepitmInputInfo, ThepitmDIPInfo,
	ThepitmInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Other games on basic Moon Cresta hardware
static struct BurnRomInfo SkybaseRomDesc[] = {
	{ "skybase.9a",    0x01000, 0x845b87a5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "skybase.8a",    0x01000, 0x096785c2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "skybase.7a",    0x01000, 0xd50c715b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "skybase.6a",    0x01000, 0xf57edb27, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "skybase.5a",    0x01000, 0x50365d95, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "skybase.4a",    0x01000, 0xcbd6647f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "skybase.7t",    0x01000, 0x9b471686, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "skybase.8t",    0x01000, 0x1cf723da, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "skybase.10t",   0x01000, 0xfe02e72c, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "skybase.9t",    0x01000, 0x0871291f, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.bpr",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Skybase)
STD_ROM_FN(Skybase)

static struct BurnRomInfo BagmanmcRomDesc[] = {
	{ "b1.bin",        0x01000, 0xb74c75ee, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b2.bin",        0x01000, 0xa7d99916, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b3.bin",        0x01000, 0xc78f5360, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b4.bin",        0x01000, 0xeebd3bd1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b5.bin",        0x01000, 0x0fe24b8c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b6.bin",        0x01000, 0xf50390e7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "g1-u.bin",      0x01000, 0xb63cfae4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g2-u.bin",      0x01000, 0xa2790089, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g1-l.bin",      0x00800, 0x2ae6b5ab, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g2-l.bin",      0x00800, 0x98b37397, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "bagmanmc.clr",  0x00020, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STD_ROM_PICK(Bagmanmc)
STD_ROM_FN(Bagmanmc)

static struct BurnRomInfo DkongjrmRomDesc[] = {
	{ "a1",            0x01000, 0x299486e9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "a2",            0x01000, 0xa74a193b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b2",            0x01000, 0x7bc4f236, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "c1",            0x01000, 0x0f594c21, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "d1",            0x01000, 0xcf7d7296, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "e2",            0x01000, 0xf7528a52, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f1",            0x01000, 0x9b1d4cc5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "v_3pa.bin",     0x01000, 0x4974ffef, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "a2.gfx",        0x01000, 0x51845eaf, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "v_3na.bin",     0x01000, 0xa95c4c63, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "b2.gfx",        0x01000, 0x7b39c3d0, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "hustler.clr",   0x00020, 0xaa1f7f5e, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Dkongjrm)
STD_ROM_FN(Dkongjrm)

static struct BurnRomInfo VpoolRomDesc[] = {
	{ "vidpool1.bin",  0x01000, 0x333f4732, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vidpool2.bin",  0x01000, 0xeea6c0f1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vidpool3.bin",  0x01000, 0x309972a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vidpool4.bin",  0x01000, 0xc4f71c1d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "hustler.5f",    0x00800, 0x0bdfad0e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "hustler.5h",    0x00800, 0x8e062177, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "hustler.clr",   0x00020, 0xaa1f7f5e, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Vpool)
STD_ROM_FN(Vpool)

static struct BurnRomInfo CkonggRomDesc[] = {
	{ "g_ck1.bin",     0x01000, 0xa4323b94, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ck2.bin",       0x01000, 0x1e532996, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g_ck3.bin",     0x01000, 0x65157cde, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g_ck4.bin",     0x01000, 0x43827bc6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g_ck5.bin",     0x01000, 0xa74ed96e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g_ck7.bin",     0x00800, 0x2c4d8129, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ckvid10.bin",   0x01000, 0x7866d2cb, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ckvid7.bin",    0x01000, 0x7311a101, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "ck_cp.bin",     0x00020, 0x7e0b79cb, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Ckongg)
STD_ROM_FN(Ckongg)

static struct BurnRomInfo CkongmcRomDesc[] = {
	{ "kc1.bin",       0x01000, 0xa87fc828, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "kc2.bin",       0x01000, 0x94a13dec, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "kc3.bin",       0x01000, 0x5efc6705, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "kc4.bin",       0x01000, 0xac917d66, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "kc5.bin",       0x00800, 0x5a9ee1ed, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "kc6.bin",       0x00800, 0xf787431e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "kc7.bin",       0x00800, 0x7a185e31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "kc8carat.bin",  0x01000, 0x7866d2cb, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "kc9carat.bin",  0x01000, 0x7311a101, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "ck_cp.bin",     0x00020, 0x7e0b79cb, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Ckongmc)
STD_ROM_FN(Ckongmc)

static struct BurnRomInfo PorterRomDesc[] = {
	{ "port1.bin",     0x01000, 0xbabaf7fe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "port2.bin",     0x01000, 0x8f7eb0e3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "port3.bin",     0x01000, 0x683939b5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "port4.bin",     0x01000, 0x6a65d58d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "port5.bin",     0x00800, 0x2978a9aa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "port6.bin",     0x00800, 0x7ecdffb5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "port7.bin",     0x01000, 0x603294f9, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "port8.bin",     0x01000, 0xb66a763d, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "mmi6331.6l",    0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Porter)
STD_ROM_FN(Porter)

static struct BurnRomInfo KongRomDesc[] = {
	{ "1",             0x01000, 0xa206beb5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2",             0x01000, 0xd75597b6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3",             0x01000, 0x54e0b87b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4",             0x01000, 0x356c4ca2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5",             0x01000, 0x2d295976, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6",             0x01000, 0x77131cca, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7",             0x01000, 0x3d5ec3f1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8",             0x01000, 0x015fe5e5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "9",             0x00800, 0xfe42a052, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "11",            0x00800, 0xad2b2cdd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "10",            0x00800, 0x91fa187e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "12",            0x00800, 0xb74724df, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "prom",          0x00020, 0x00000000, BRF_OPT | BRF_NODUMP },
	
	{ "13",            0x01000, 0x7d33ca0a, BRF_OPT }, // unknown
};

STD_ROM_PICK(Kong)
STD_ROM_FN(Kong)

static struct BurnRomInfo FantastcRomDesc[] = {
	{ "f1",            0x01000, 0x8019f0b7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f2",            0x01000, 0x988a9bc6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f3",            0x01000, 0xa3c0cc0b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f4",            0x01000, 0xc1361be8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f5",            0x01000, 0x6787e93f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f6",            0x01000, 0x597029ae, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f7",            0x01000, 0x8de08d9a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "f8",            0x01000, 0x489e2fb7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "37",            0x01000, 0x3a54f749, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "38",            0x01000, 0x88b71264, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "prom-74g138",  0x00020, 0xb7cbbc1f, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Fantastc)
STD_ROM_FN(Fantastc)

UINT8 __fastcall BagmanmcZ80Read(UINT16 a)
{
	switch (a) {
		case 0xa000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0xa800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0xb000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall BagmanmcZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa004:
		case 0xa005:
		case 0xa006:
		case 0xa007: {
			GalaxianLfoFreqWrite(a - 0xa004, d);
			return;
		}
		
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall DkongjrmZ80Read(UINT16 a)
{
	if (a >= 0xa000 && a <= 0xa0ff) {
		return GalInput[0] | GalDip[0];
	}
	
	if (a >= 0xa800 && a <= 0xa8ff) {
		return GalInput[1] | GalDip[1];
	}
	
	if (a >= 0xb000 && a <= 0xb0ff) {
		return GalInput[2] | GalDip[2];
	}
	
	switch (a) {
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall DkongjrmZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa004:
		case 0xa005:
		case 0xa006:
		case 0xa007: {
			GalaxianLfoFreqWrite(a - 0xa004, d);
			return;
		}
		
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
//		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb000: {
			GalGfxBank[0] = d;
			return;
		}
		
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall CkonggZ80Read(UINT16 a)
{
	switch (a) {
		case 0xc000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0xc400: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0xc800: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0xcc00: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall CkonggZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xc400:
		case 0xc401:
		case 0xc402:
		case 0xc403:
		case 0xc404:
		case 0xc405:
		case 0xc406:
		case 0xc407: {
			GalaxianSoundWrite(a - 0xc400, d);
			return;
		}
		
		case 0xc801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xc804: {
			// link apparently cut
			return;
		}
		
		case 0xc806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xc807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xcc00: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall CkongmcZ80Read(UINT16 a)
{
	switch (a) {
		case 0xa000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0xa800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0xb000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall CkongmcZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xc806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xc807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall FantastcZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		int Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0x9900 && a <= 0xafff) {
		// ???
		return;
	}
	
	if (a >= 0xb001 && a <= 0xd7ff) {
		// ???
		return;
	}
	
	switch (a) {
		case 0x8803: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x880b: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0x880c: {
			AY8910Write(1, 0, d);
			return;
		}
		
		case 0x880e: {
			AY8910Write(1, 1, d);
			return;
		}
		
		case 0xb000: {
			GalIrqFire = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void SkybaseAlterZ80()
{
	MapMooncrst();
	
	ZetOpen(0);
	ZetMapArea(0x0000, 0x5fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x5fff, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetClose();
}

static INT32 SkybaseInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = SkybaseAlterZ80;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = SkybaseExtendTileInfo;
	GalExtendSpriteInfoFunction = SkybaseExtendSpriteInfo;
	
	return nRet;
}

static void BagmanmcPostLoad()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(BagmanmcZ80Read);
	ZetSetWriteHandler(BagmanmcZ80Write);
	ZetMapArea(0x0000, GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x6000, 0x67ff, 0, GalZ80Ram1);
	ZetMapArea(0x6000, 0x67ff, 1, GalZ80Ram1);
	ZetMapArea(0x6000, 0x67ff, 2, GalZ80Ram1);
	ZetMapArea(0x9000, 0x93ff, 0, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 1, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 2, GalVideoRam);
	ZetMapArea(0x9800, 0x98ff, 0, GalSpriteRam);
	ZetMapArea(0x9800, 0x98ff, 2, GalSpriteRam);
	ZetClose();
}

static INT32 BagmanmcInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = BagmanmcPostLoad;
	GalPromRomSize = 0x20;
	
	nRet = GalInit();
	
	GalNumChars = 512;
	GalNumSprites = 64;
	CharPlaneOffsets[1] = GalNumChars * 8 * 8;
	SpritePlaneOffsets[1] = GalNumSprites * 16 * 16;
	
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x1000);
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x0000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x2000, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x1000, TempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x2800, TempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(GalTempRom + 0x0800, GAL_ROM_OFFSET_TILES_SHARED + 2, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1800, GAL_ROM_OFFSET_TILES_SHARED + 3, 1); if (nRet) return 1;
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom + 0x2000, GalSprites);
	BurnFree(GalTempRom);
	BurnFree(TempRom);
	
	HardCodeMooncrstPROM();
		
	GalIrqType = GAL_IRQ_TYPE_IRQ0;
	GalExtendTileInfoFunction = PiscesExtendTileInfo;
	
	return nRet;
}

static void DkongjrmPostLoad()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(DkongjrmZ80Read);
	ZetSetWriteHandler(DkongjrmZ80Write);
	ZetMapArea(0x0000, 0x5fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x5fff, 2, GalZ80Rom1);
	ZetMapArea(0x6000, 0x6fff, 0, GalZ80Ram1);
	ZetMapArea(0x6000, 0x6fff, 1, GalZ80Ram1);
	ZetMapArea(0x6000, 0x6fff, 2, GalZ80Ram1);
	ZetMapArea(0x7000, 0x7fff, 0, GalZ80Rom1 + 0x6000);
	ZetMapArea(0x7000, 0x7fff, 2, GalZ80Rom1 + 0x6000);
	ZetMapArea(0x9000, 0x93ff, 0, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 1, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 2, GalVideoRam);
	ZetMapArea(0x9800, 0x98ff, 0, GalSpriteRam);
	ZetMapArea(0x9800, 0x98ff, 2, GalSpriteRam);
	ZetClose();
}

static INT32 DkongjrmInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = DkongjrmPostLoad;
	
	nRet = GalInit();
	
	GalRenderBackgroundFunction = NULL;
	GalDrawBulletsFunction = NULL;
	GalExtendTileInfoFunction = PiscesExtendTileInfo;
	GalExtendSpriteInfoFunction = DkongjrmExtendSpriteInfo;
	
	GalRenderFrameFunction = DkongjrmRenderFrame;

	return nRet;
}

static void VpoolPostLoad()
{
	MapMooncrst();
	
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	memcpy(GalTempRom, GalZ80Rom1, GalZ80Rom1Size);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0000, 0x800);
	memcpy(GalZ80Rom1 + 0x2000, GalTempRom + 0x0800, 0x800);
	memcpy(GalZ80Rom1 + 0x0800, GalTempRom + 0x1000, 0x800);
	memcpy(GalZ80Rom1 + 0x2800, GalTempRom + 0x1800, 0x800);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x2000, 0x800);
	memcpy(GalZ80Rom1 + 0x3000, GalTempRom + 0x2800, 0x800);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x3000, 0x800);
	memcpy(GalZ80Rom1 + 0x3800, GalTempRom + 0x3800, 0x800);
	BurnFree(GalTempRom);
	
	ZetOpen(0);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetClose();
}

static INT32 VpoolInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = VpoolPostLoad;
	
	nRet = GalInit();
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

static void CkonggMapZ80()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(CkonggZ80Read);
	ZetSetWriteHandler(CkonggZ80Write);
	ZetMapArea(0x0000, 0x57ff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x57ff, 2, GalZ80Rom1);
	ZetMapArea(0x6000, 0x6fff, 0, GalZ80Ram1);
	ZetMapArea(0x6000, 0x6fff, 1, GalZ80Ram1);
	ZetMapArea(0x6000, 0x6fff, 2, GalZ80Ram1);
	ZetMapArea(0x9000, 0x93ff, 0, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 1, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 2, GalVideoRam);
	ZetMapArea(0x9800, 0x98ff, 0, GalSpriteRam);
	ZetMapArea(0x9800, 0x98ff, 2, GalSpriteRam);
	ZetClose();
}

static void CkonggPostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	memcpy(GalTempRom, GalZ80Rom1, GalZ80Rom1Size);
	memcpy(GalZ80Rom1 + 0x2400, GalTempRom + 0x0000, 0x400);
	memcpy(GalZ80Rom1 + 0x1c00, GalTempRom + 0x0400, 0x400);
	memcpy(GalZ80Rom1 + 0x4800, GalTempRom + 0x0800, 0x400);
	memcpy(GalZ80Rom1 + 0x0c00, GalTempRom + 0x0c00, 0x400);
	memcpy(GalZ80Rom1 + 0x4400, GalTempRom + 0x1000, 0x400);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x1400, 0x400);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x1800, 0x400);
	memcpy(GalZ80Rom1 + 0x2800, GalTempRom + 0x1c00, 0x400);
	memcpy(GalZ80Rom1 + 0x3400, GalTempRom + 0x2000, 0x400);
	memcpy(GalZ80Rom1 + 0x4c00, GalTempRom + 0x2400, 0x400);
	memcpy(GalZ80Rom1 + 0x5000, GalTempRom + 0x2800, 0x400);
	memcpy(GalZ80Rom1 + 0x0400, GalTempRom + 0x2c00, 0x400);
	memcpy(GalZ80Rom1 + 0x2000, GalTempRom + 0x3000, 0x400);
	memcpy(GalZ80Rom1 + 0x3800, GalTempRom + 0x3400, 0x400);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x3800, 0x400);
	memcpy(GalZ80Rom1 + 0x4000, GalTempRom + 0x3c00, 0x400);
	memcpy(GalZ80Rom1 + 0x0800, GalTempRom + 0x4000, 0x400);
	memcpy(GalZ80Rom1 + 0x5400, GalTempRom + 0x4400, 0x400);
	memcpy(GalZ80Rom1 + 0x2c00, GalTempRom + 0x4800, 0x400);
	memcpy(GalZ80Rom1 + 0x1400, GalTempRom + 0x4c00, 0x400);
	memcpy(GalZ80Rom1 + 0x3000, GalTempRom + 0x5000, 0x400);
	memcpy(GalZ80Rom1 + 0x3c00, GalTempRom + 0x5400, 0x400);	
	BurnFree(GalTempRom);

	CkonggMapZ80();
}

static INT32 CkonggInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = CkonggPostLoad;
	
	nRet = GalInit();
	
	GalExtendSpriteInfoFunction = MshuttleExtendSpriteInfo;
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

static void CkongmcPostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(0x4000);
	memcpy(GalTempRom, GalZ80Rom1, 0x4000);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0000, 0x800);
	memcpy(GalZ80Rom1 + 0x2000, GalTempRom + 0x0800, 0x800);
	memcpy(GalZ80Rom1 + 0x0800, GalTempRom + 0x1000, 0x800);
	memcpy(GalZ80Rom1 + 0x2800, GalTempRom + 0x1800, 0x800);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x2000, 0x800);
	memcpy(GalZ80Rom1 + 0x3000, GalTempRom + 0x2800, 0x800);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x3000, 0x800);
	memcpy(GalZ80Rom1 + 0x3800, GalTempRom + 0x3800, 0x800);
	BurnFree(GalTempRom);
	
	CkonggMapZ80();
	
	ZetOpen(0);
	ZetSetReadHandler(CkongmcZ80Read);
	ZetSetWriteHandler(CkongmcZ80Write);
	ZetClose();
}

static INT32 CkongmcInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = CkongmcPostLoad;
	
	nRet = GalInit();
	
	GalExtendSpriteInfoFunction = MshuttleExtendSpriteInfo;
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

static void PorterPostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(0x4000);
	memcpy(GalTempRom, GalZ80Rom1, 0x4000);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0000, 0x800);
	memcpy(GalZ80Rom1 + 0x2000, GalTempRom + 0x0800, 0x800);
	memcpy(GalZ80Rom1 + 0x0800, GalTempRom + 0x1000, 0x800);
	memcpy(GalZ80Rom1 + 0x2800, GalTempRom + 0x1800, 0x800);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x2000, 0x800);
	memcpy(GalZ80Rom1 + 0x3000, GalTempRom + 0x2800, 0x800);
	memcpy(GalZ80Rom1 + 0x1800, GalTempRom + 0x3000, 0x800);
	memcpy(GalZ80Rom1 + 0x3800, GalTempRom + 0x3800, 0x800);
	BurnFree(GalTempRom);
	
	MapMooncrst();
	
	ZetOpen(0);
	ZetMapArea(0x0000, 0x5fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x5fff, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetClose();
}

static INT32 PorterInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = PorterPostLoad;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = MooncrstExtendSpriteInfo;

	return nRet;
}

static void KongAlterZ80()
{
	MapMooncrst();
	
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x7fff, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetClose();
}

static INT32 KongInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = KongAlterZ80;
	
	GalPromRomSize = 0x20;
	
	nRet = GalInit();
	
	GalRenderBackgroundFunction = NULL;
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;
	
	HardCodeMooncrstPROM();
	
	return nRet;
}

static void FantastcPostLoad()
{
	MapMooncrst();
	
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x7fff, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetSetWriteHandler(FantastcZ80Write);
	ZetClose();
	
	static const UINT16 lut_am_unscramble[32] = {
		0, 2, 4, 6,	// ok!
		7, 3, 5, 1,	// ok!
		6, 0, 2, 4,	// ok!
		1, 5, 3, 0,	// ok!
		2, 4, 6, 3,	// good, good?, guess, guess
		5, 6, 0, 2,	// good, good?, good?, guess
		4, 1, 1, 5,	// good, good, guess, good
		3, 7, 7, 7	// ok!
	};
	
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x8000);
	memcpy(pTemp, GalZ80Rom1, 0x8000);

	for (INT32 i = 0; i < 32; i++) {
		memcpy(GalZ80Rom1 + i * 0x400, pTemp + lut_am_unscramble[i] * 0x1000 + (i & 3) * 0x400, 0x400);
	}
	
	BurnFree(pTemp);
}

static INT32 FantastcInit()
{
	INT32 nRet;
	
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910;
	
	GalPostLoadCallbackFunction = FantastcPostLoad;
	GalNumSprites = 0x40;
	
	nRet = GalInit();
	
	GalRenderFrameFunction = FantastcRenderFrame;
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;
	
	HardCodeMooncrstPROM();
	
	return nRet;
}

struct BurnDriver BurnDrvSkybase = {
	"skybase", NULL, NULL, NULL, "1982",
	"Sky Base\0", NULL, "Omori Electric Co. Ltd", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, SkybaseRomInfo, SkybaseRomName, NULL, NULL, SkybaseInputInfo, SkybaseDIPInfo,
	SkybaseInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriverD BurnDrvBagmanmc = {
	"bagmanmc", "bagman", NULL, NULL, "1982",
	"Bagman (Moon Cresta hardware)\0", "Bad Colours", "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, BagmanmcRomInfo, BagmanmcRomName, NULL, NULL, BagmanmcInputInfo, BagmanmcDIPInfo,
	BagmanmcInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvDkongjrm = {
	"dkongjrm", "dkongjr", NULL, NULL, "1982",
	"Donkey Kong Jr. (Moon Cresta hardware)\0", "Bad Colours", "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, DkongjrmRomInfo, DkongjrmRomName, NULL, NULL, DkongjrmInputInfo, DkongjrmDIPInfo,
	DkongjrmInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvVpool = {
	"vpool", "hustler", NULL, NULL, "1980",
	"Video Pool (bootleg on Moon Cresta hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_SPORTSMISC, 0,
	NULL, VpoolRomInfo, VpoolRomName, NULL, NULL, GalaxianInputInfo, VpoolDIPInfo,
	VpoolInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvCkongg = {
	"ckongg", "ckong", NULL, NULL, "1981",
	"Crazy Kong (bootleg on Galaxian hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, CkonggRomInfo, CkonggRomName, NULL, NULL, CkonggInputInfo, CkonggDIPInfo,
	CkonggInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvCkongmc = {
	"ckongmc", "ckong", NULL, NULL, "1981",
	"Crazy Kong (bootleg on Moon Cresta hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, CkongmcRomInfo, CkongmcRomName, NULL, NULL, CkongmcInputInfo, CkongmcDIPInfo,
	CkongmcInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvPorter = {
	"porter", "dockman", NULL, NULL, "19??",
	"Port Man (bootleg on Moon Cresta hardware)\0", NULL, "Nova Games Ltd", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, PorterRomInfo, PorterRomName, NULL, NULL, PorterInputInfo, PorterDIPInfo,
	PorterInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvKong = {
	"kong", NULL, NULL, NULL, "198?",
	"Kong (Brazil)\0", "Bad Colours", "Taito do Brasil", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, KongRomInfo, KongRomName, NULL, NULL, KongInputInfo, KongDIPInfo,
	KongInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFantastc = {
	"fantastc", NULL, NULL, NULL, "198?",
	"Fantastic\0", "Bad Colours", "Taito do Brasil", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, FantastcRomInfo, FantastcRomName, NULL, NULL, FantastcInputInfo, FantastcDIPInfo,
	FantastcInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Custom background
static struct BurnRomInfo RockclimRomDesc[] = {
	{ "lc01.a1",       0x01000, 0x8601ae8d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lc02.a2",       0x01000, 0x2dde9d4c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lc03.a3",       0x01000, 0x82c48a67, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lc04.a4",       0x01000, 0x7cd3a04a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lc05.a5",       0x01000, 0x5e542149, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lc06.a6",       0x01000, 0xb2bdca64, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "lc08.a9",       0x00800, 0x7f18e1ef, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "lc07.a7",       0x00800, 0xf18b50ac, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "lc10.c9",       0x00800, 0xdec5781b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "lc09.c7",       0x00800, 0x06c0b5de, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "lc11.f4",       0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
	{ "lc12.e9",       0x00020, 0xf6e76547, BRF_GRA | GAL_ROM_PROM },
		
	{ "lc13.g5",       0x01000, 0x19475f2b, BRF_GRA },
	{ "lc14.g7",       0x01000, 0xcc96d1db, BRF_GRA },
};

STD_ROM_PICK(Rockclim)
STD_ROM_FN(Rockclim)

UINT8 __fastcall RockclimZ80Read(UINT16 a)
{
	switch (a) {
		case 0x8800: {
			return GalInput[3] | GalDip[3];
		}
		
		case 0xa000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0xa800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0xb000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall RockclimZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x4800: {
			RockclimScrollX = (RockclimScrollX & 0xff00) | d;
			return;
		}
		
		case 0x4801: {
			RockclimScrollX = (RockclimScrollX & 0xff) | (d << 8);
			return;
		}
		
		case 0x4802: {
			RockclimScrollY = (RockclimScrollY & 0xff00) | d;
			return;
		}
		
		case 0x4803: {
			RockclimScrollY = (RockclimScrollY & 0xff) | (d << 8);
			return;
		}
		
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa004:
		case 0xa005:
		case 0xa006:
		case 0xa007: {
			GalaxianLfoFreqWrite(a - 0xa004, d);
			return;
		}
		
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb000: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void RockclimPostLoad()
{
	MapMooncrst();
	
	GalVideoRam2 = (UINT8*)BurnMalloc(0x800);
	
	ZetOpen(0);
	ZetSetReadHandler(RockclimZ80Read);
	ZetSetWriteHandler(RockclimZ80Write);
	ZetMapArea(0x4000, 0x47ff, 0, GalVideoRam2);
	ZetMapArea(0x4000, 0x47ff, 1, GalVideoRam2);
	ZetMapArea(0x4000, 0x47ff, 2, GalVideoRam2);
	ZetMapArea(0x5000, 0x53ff, 0, GalZ80Ram1 + 0x400);
	ZetMapArea(0x5000, 0x53ff, 1, GalZ80Ram1 + 0x400);
	ZetMapArea(0x5000, 0x53ff, 2, GalZ80Ram1 + 0x400);
	ZetMapArea(0x6000, 0x7fff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0x6000, 0x7fff, 2, GalZ80Rom1 + 0x4000);
	ZetClose();
}

static INT32 RockclimInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = RockclimPostLoad;
	RockclimTiles = (UINT8*)BurnMalloc(0x100 * 8 * 8);
	
	nRet = GalInit();
	
	INT32 RockclimPlaneOffsets[4] = {4, 0, 0x8004, 0x8000};
	INT32 RockclimCharXOffsets[8] = {3, 2, 1, 0, 11, 10, 9, 8};
	INT32 RockclimCharYOffsets[8] = {0, 16, 32, 48, 64, 80, 96, 112};
	
	GalTempRom = (UINT8*)BurnMalloc(0x2000);
	nRet = BurnLoadRom(GalTempRom + 0x0000, 12, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1000, 13, 1); if (nRet) return 1;
	GfxDecode(256, 4, 8, 8, RockclimPlaneOffsets, RockclimCharXOffsets, RockclimCharYOffsets, 0x80, GalTempRom, RockclimTiles);
	BurnFree(GalTempRom);
	
	GalCalcPaletteFunction = RockclimCalcPalette;
	GalRenderBackgroundFunction = RockclimDrawBackground;
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = RockclimExtendSpriteInfo;
	
	return nRet;
}

struct BurnDriver BurnDrvRockclim = {
	"rockclim", NULL, NULL, NULL, "1981",
	"Rock Climber\0", NULL, "Taito", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, RockclimRomInfo, RockclimRomName, NULL, NULL, RockclimInputInfo, RockclimDIPInfo,
	RockclimInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

// Extra roms and sound hardware replaced with AY8910
static struct BurnRomInfo JumpbugRomDesc[] = {
	{ "jb1",           0x01000, 0x415aa1b7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb2",           0x01000, 0xb1c27510, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb3",           0x01000, 0x97c24be2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb4",           0x01000, 0x66751d12, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb5",           0x01000, 0xe2d66faf, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb6",           0x01000, 0x49e0bdfd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb7",           0x00800, 0x83d71302, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "jbl",           0x00800, 0x9a091b0a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbm",           0x00800, 0x8a0fc082, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbn",           0x00800, 0x155186e0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbi",           0x00800, 0x7749b111, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbj",           0x00800, 0x06e8d7df, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbk",           0x00800, 0xb8dbddf3, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Jumpbug)
STD_ROM_FN(Jumpbug)

static struct BurnRomInfo JumpbugbRomDesc[] = {
	{ "jb1",           0x01000, 0x415aa1b7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb2",           0x01000, 0xb1c27510, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb3b",          0x01000, 0xcb8b8a0f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb4",           0x01000, 0x66751d12, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb5b",          0x01000, 0x7553b5e2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb6b",          0x01000, 0x47be9843, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "jb7b",          0x00800, 0x460aed61, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "jbl",           0x00800, 0x9a091b0a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbm",           0x00800, 0x8a0fc082, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbn",           0x00800, 0x155186e0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbi",           0x00800, 0x7749b111, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbj",           0x00800, 0x06e8d7df, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "jbk",           0x00800, 0xb8dbddf3, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "l06_prom.bin",  0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Jumpbugb)
STD_ROM_FN(Jumpbugb)

static struct BurnRomInfo LeversRomDesc[] = {
	{ "g96059.a8",     0x01000, 0x9550627a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g96060.d8",     0x01000, 0x5ac64646, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g96061.e8",     0x01000, 0x9db8e520, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g96062.h8",     0x01000, 0x7c8e8b3a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g96063.j8",     0x01000, 0xfa61e793, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "g96064.l8",     0x01000, 0xf797f389, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "g95948.n1",     0x00800, 0xd8a0c692, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g95949.s1",     0x00800, 0x3660a552, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g95946.j1",     0x00800, 0x73b61b2d, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g95947.m1",     0x00800, 0x72ff67e2, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "g960lev.clr",   0x00020, 0x01febbbe, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Levers)
STD_ROM_FN(Levers)

static struct BurnRomInfo BongoRomDesc[] = {
	{ "bg1.bin",       0x01000, 0xde9a8ec6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bg2.bin",       0x01000, 0xa19da662, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bg3.bin",       0x01000, 0x9f6f2150, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bg4.bin",       0x01000, 0xf80372d2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bg5.bin",       0x01000, 0xfc92eade, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "bg6.bin",       0x01000, 0x561d9e5d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "b-h.bin",       0x01000, 0xfc79d103, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "b-k.bin",       0x01000, 0x94d17bf3, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "b-clr.bin",     0x00020, 0xc4761ada, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Bongo)
STD_ROM_FN(Bongo)

void __fastcall BongoZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			// watchdog write?
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall BongoZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x02: {
			return AY8910Read(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall BongoZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x01: {
			AY8910Write(0, 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static INT32 JumpbugInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapJumpbug;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_JUMPBUGAY8910;
	
	nRet = GalInit();
	
	GalRenderBackgroundFunction = JumpbugDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = JumpbugExtendTileInfo;
	GalExtendSpriteInfoFunction = JumpbugExtendSpriteInfo;
	
	GalSoundVolumeShift = 3;
	
	return nRet;
}

static void LeversPostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	memcpy(GalTempRom, GalZ80Rom1, GalZ80Rom1Size);
	memset(GalZ80Rom1, 0, GalZ80Rom1Size);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0000, 0x1000);
	memcpy(GalZ80Rom1 + 0x2000, GalTempRom + 0x1000, GalZ80Rom1Size - 0x2000);
	BurnFree(GalTempRom);
	
	MapJumpbug();
}

static INT32 LeversInit()
{
	INT32 nRet;
	
	GalZ80Rom1Size = 0x1000;
	GalTilesSharedRomSize = 0x1000;
	
	GalPostLoadCallbackFunction = LeversPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_JUMPBUGAY8910;
	
	nRet = GalInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1800, GAL_ROM_OFFSET_TILES_SHARED + 2, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x2800, GAL_ROM_OFFSET_TILES_SHARED + 3, 1); if (nRet) return 1;
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	
	GalRenderBackgroundFunction = JumpbugDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = JumpbugExtendTileInfo;
	GalExtendSpriteInfoFunction = JumpbugExtendSpriteInfo;
	
	GalSoundVolumeShift = 2;
	
	return nRet;
}

static void BongoPostLoad()
{
	MapMooncrst();
	
	ZetOpen(0);
	ZetSetWriteHandler(BongoZ80Write);
	ZetSetInHandler(BongoZ80PortRead);
	ZetSetOutHandler(BongoZ80PortWrite);
	ZetMapArea(0x0000, 0x5fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x5fff, 2, GalZ80Rom1);
	ZetMapArea(0x8400, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8400, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8400, 0x87ff, 2, GalZ80Ram1);
	ZetMapArea(0x9400, 0x97ff, 0, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 1, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 2, GalVideoRam);
	ZetClose();
}

static INT32 BongoInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = BongoPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_BONGOAY8910;
	
	nRet = GalInit();
	
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;
	
	GalSoundVolumeShift = 3;

	return nRet;
}

struct BurnDriver BurnDrvJumpbug = {
	"jumpbug", NULL, NULL, NULL, "1981",
	"Jump Bug\0", NULL, "Rock-ola", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, JumpbugRomInfo, JumpbugRomName, NULL, NULL, JumpbugInputInfo, JumpbugDIPInfo,
	JumpbugInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvJumpbugb = {
	"jumpbugb", "jumpbug", NULL, NULL, "1981",
	"Jump Bug (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, JumpbugbRomInfo, JumpbugbRomName, NULL, NULL, JumpbugInputInfo, JumpbugDIPInfo,
	JumpbugInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvLevers = {
	"levers", NULL, NULL, NULL, "1983",
	"Levers\0", NULL, "Rock-ola", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, LeversRomInfo, LeversRomName, NULL, NULL, LeversInputInfo, LeversDIPInfo,
	LeversInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvBongo = {
	"bongo", NULL, NULL, NULL, "1983",
	"Bongo\0", NULL, "Jetsoft", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, BongoRomInfo, BongoRomName, NULL, NULL, BongoInputInfo, BongoDIPInfo,
	BongoInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Second Z80 driving AY8910
static struct BurnRomInfo CheckmanRomDesc[] = {
	{ "cm1",           0x00800, 0xe8cbdd28, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cm2",           0x00800, 0xb8432d4d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cm3",           0x00800, 0x15a97f61, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cm4",           0x00800, 0x8c12ecc0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cm5",           0x00800, 0x2352cfd6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "cm13",          0x00800, 0x0b09a3e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "cm14",          0x00800, 0x47f043be, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "cm11",          0x00800, 0x8d1bcca0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "cm9",           0x00800, 0x3cd5c751, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "checkman.clr",  0x00020, 0x57a45057, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Checkman)
STD_ROM_FN(Checkman)

static struct BurnRomInfo CheckmanjRomDesc[] = {
	{ "cm_1.bin",      0x01000, 0x456a118f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cm_2.bin",      0x01000, 0x146b2c44, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cm_3.bin",      0x00800, 0x73e1c945, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "cm_4.bin",      0x01000, 0x923cffa1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "cm_6.bin",      0x00800, 0x476a7cc3, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "cm_5.bin",      0x00800, 0xb3df2b5f, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "checkman.clr",  0x00020, 0x57a45057, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Checkmanj)
STD_ROM_FN(Checkmanj)

static struct BurnRomInfo DingoRomDesc[] = {
	{ "003.e7",        0x01000, 0xd088550f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "004.h7",        0x01000, 0xa228446a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "005.j7",        0x00800, 0x14d680bb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "6.7l",          0x01000, 0x047092e0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "001.h1",        0x00800, 0x1ab1dd4d, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "002.k1",        0x00800, 0x4be375ee, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "18s030.l6",     0x00020, 0x3061d0f9, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Dingo)
STD_ROM_FN(Dingo)

static struct BurnRomInfo DingoeRomDesc[] = {
	{ "unk.2b",        0x01000, 0x0df7ac6d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "unk.2d",        0x01000, 0x0881e204, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "unk.3b",        0x01000, 0x0b6aeab5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "unk.1c",        0x00800, 0x8e354c38, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "unk.1d",        0x00800, 0x092878d6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "unk.4d",        0x00800, 0x76a00a56, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "unk.4b",        0x00800, 0x5acf57aa, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "82s123n.001",   0x00020, 0x02b11865, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Dingoe)
STD_ROM_FN(Dingoe)

void __fastcall CheckmanZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			GalSoundLatch = d;
			ZetClose();
			ZetOpen(1);
			ZetNmi();
			ZetClose();
			ZetOpen(0);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("IO Write %x, %x\n"), a, d);
}

void __fastcall CheckmanZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Prog Write %x, %x\n"), a, d);
}

UINT8 __fastcall CheckmanSoundZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x03: {
			return GalSoundLatch;
		}
		
		case 0x06: {
			return AY8910Read(0);
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Sound IO Read %x\n"), a);
	return 0xff;
}

void __fastcall CheckmanSoundZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x04: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x05: {
			AY8910Write(0, 1, d);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Sound IO Write %x, %x\n"), a, d);
}

UINT8 __fastcall CheckmanjZ80Read(UINT16 a)
{
	switch (a) {
		case 0x3800: {
			switch (ZetPc(-1)) {
				case 0x0f15:  return 0xf5;
				case 0x0f8f:  return 0x7c;
				case 0x10b3:  return 0x7c;
				case 0x10e0:  return 0x00;
				case 0x10f1:  return 0xaa;
				case 0x1402:  return 0xaa;
			}
			return 0xff;
		}
		
		case 0x6000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x6800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x7000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0x7800: {
			// watchdog read
			return 0xff;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Prog Read %x\n"), a);
	return 0xff;
}

void __fastcall CheckmanjZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5800 && a <= 0x58ff) {
		INT32 Offset = a - 0x5800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000:
		case 0x6001: {
			// start_lamp_w
			return;
		}
		
		case 0x6002: {
			// coin_lock_w
			return;
		}
		
		case 0x6003: {
			// coin_count_0_w
			return;
		}
		
		case 0x7001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x7006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x7007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			GalSoundLatch = d;
			ZetClose();
			ZetOpen(1);
			ZetNmi();
			ZetClose();
			ZetOpen(0);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Prog Write %x, %x\n"), a, d);
}

UINT8 __fastcall CheckmanjSoundZ80Read(UINT16 a)
{
	switch (a) {
		case 0xa002: {
			return AY8910Read(0);
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Sound Prog Read %x\n"), a);
	return 0xff;
}

void __fastcall CheckmanjSoundZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xa000: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0xa001: {
			AY8910Write(0, 1, d);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Sound Prog Write %x, %x\n"), a, d);
}

UINT8 __fastcall DingoZ80Read(UINT16 a)
{
	switch (a) {
		case 0x3000: {
			return 0xaa;
		}
		
		case 0x3035: {
			return 0x8c;
		}
		
		case 0x6000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x6800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x7000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0x7800: {
			// watchdog read
			return 0xff;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Prog Read %x\n"), a);
	return 0xff;
}

static void CheckmanInitSoundCPU()
{
	ZetOpen(1);
	ZetMapArea(0x0000, GalZ80Rom2Size - 1, 0, GalZ80Rom2);
	ZetMapArea(0x0000, GalZ80Rom2Size - 1, 2, GalZ80Rom2);
	ZetMapArea(0x2000, 0x23ff, 0, GalZ80Ram2);
	ZetMapArea(0x2000, 0x23ff, 1, GalZ80Ram2);
	ZetMapArea(0x2000, 0x23ff, 2, GalZ80Ram2);
	ZetMemEnd();
	ZetSetInHandler(CheckmanSoundZ80PortRead);
	ZetSetOutHandler(CheckmanSoundZ80PortWrite);
	ZetClose();
	
	nGalCyclesTotal[1] = 1620000 / 60;
}

static void CheckmanPostLoad()
{
	static const UINT8 XORTable[8][4] = {
		{ 6, 0, 6, 0 },
		{ 5, 1, 5, 1 },
		{ 4, 2, 6, 1 },
		{ 2, 4, 5, 0 },
		{ 4, 6, 1, 5 },
		{ 0, 6, 2, 5 },
		{ 0, 2, 0, 2 },
		{ 1, 4, 1, 4 }
	};
	
	for (UINT32 Offset = 0; Offset < GalZ80Rom1Size; Offset++) {
		UINT8 Data = GalZ80Rom1[Offset];
		UINT32 Line = Offset & 0x07;

		Data ^= (BIT(Data, XORTable[Line][0]) << XORTable[Line][1]) | (BIT(Data, XORTable[Line][2]) << XORTable[Line][3]);
		GalZ80Rom1[Offset] = Data;
	}
	
	MapMooncrst();
	ZetOpen(0);
	ZetSetWriteHandler(CheckmanZ80Write);
	ZetSetOutHandler(CheckmanZ80PortWrite);
	ZetClose();
	
	CheckmanInitSoundCPU();
}

static INT32 CheckmanInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = CheckmanPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_CHECKMANAY8910;
	GalTilesSharedRomSize = 0x1000;
	
	nRet = GalInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	memcpy(GalTempRom + 0x0800, GalTempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x1800, GalTempRom + 0x1000, 0x800);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);	
	BurnFree(GalTempRom);
	
	GalExtendTileInfoFunction = MooncrstExtendTileInfo;
	GalExtendSpriteInfoFunction = MooncrstExtendSpriteInfo;
	
	return nRet;
}

static void CheckmanjInitSoundCPU()
{
	ZetOpen(1);
	ZetMapArea(0x0000, GalZ80Rom2Size - 1, 0, GalZ80Rom2);
	ZetMapArea(0x0000, GalZ80Rom2Size - 1, 2, GalZ80Rom2);
	ZetMapArea(0x8000, 0x83ff, 0, GalZ80Ram2);
	ZetMapArea(0x8000, 0x83ff, 1, GalZ80Ram2);
	ZetMapArea(0x8000, 0x83ff, 2, GalZ80Ram2);
	ZetMemEnd();
	ZetSetReadHandler(CheckmanjSoundZ80Read);
	ZetSetWriteHandler(CheckmanjSoundZ80Write);
	ZetClose();
	
	nGalCyclesTotal[1] = 1620000 / 60;
}

static void CheckmanjPostLoad()
{
	ZetOpen(0);
	ZetSetReadHandler(CheckmanjZ80Read);
	ZetSetWriteHandler(CheckmanjZ80Write);
	ZetClose();
	
	CheckmanjInitSoundCPU();
}

static INT32 CheckmanjInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = CheckmanjPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_CHECKMAJAY8910;
	
	nRet = GalInit();
	
	return nRet;
}

static void DingoPostLoad()
{
	ZetOpen(0);
	ZetSetReadHandler(DingoZ80Read);
	ZetSetWriteHandler(CheckmanjZ80Write);
	ZetMapArea(0x5400, 0x57ff, 0, GalVideoRam);
	ZetMapArea(0x5400, 0x57ff, 1, GalVideoRam);
	ZetMapArea(0x5400, 0x57ff, 2, GalVideoRam);
	ZetClose();
	
	CheckmanjInitSoundCPU();
}

static INT32 DingoInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = DingoPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_CHECKMAJAY8910;
	
	nRet = GalInit();
	
	GalSoundVolumeShift = 0;
	
	return nRet;
}

static void DingoePostLoad()
{
	for (UINT32 Offset = 0; Offset < GalZ80Rom1Size; Offset++) {
		UINT8 Data = GalZ80Rom1[Offset];
		
		Data ^= BIT(Data, 2) << 4;
		Data ^= BIT(Data, 5) << 0;
		Data ^= 0x02;
		
		if (Offset & 0x02) Data = BITSWAP08(Data, 7, 6, 5, 0, 3, 2, 1, 4);
		GalZ80Rom1[Offset] = Data;
	}
	
	MapMooncrst();
	ZetOpen(0);
	ZetSetWriteHandler(CheckmanZ80Write);
	ZetSetOutHandler(CheckmanZ80PortWrite);
	ZetClose();
	
	CheckmanInitSoundCPU();
}

static INT32 DingoeInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = DingoePostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_CHECKMANAY8910;
	
	nRet = GalInit();
	
	return nRet;
}

struct BurnDriver BurnDrvCheckman = {
	"checkman", NULL, NULL, NULL, "1982",
	"Check Man\0", NULL, "Zilex-Zenitone", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, CheckmanRomInfo, CheckmanRomName, NULL, NULL, CheckmanInputInfo, CheckmanDIPInfo,
	CheckmanInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvCheckmanj = {
	"checkmanj", "checkman", NULL, NULL, "1982",
	"Check Man (Japan)\0", NULL, "Jaleco", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, CheckmanjRomInfo, CheckmanjRomName, NULL, NULL, CheckmanjInputInfo, CheckmanjDIPInfo,
	CheckmanjInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvDingo = {
	"dingo", NULL, NULL, NULL, "1983",
	"Dingo\0", NULL, "Ashby Computers and Graphics LTD. (Jaleco license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, DingoRomInfo, DingoRomName, NULL, NULL, DingoInputInfo, DingoDIPInfo,
	DingoInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriverD BurnDrvDingoe = {
	"dingoe", "dingo", NULL, NULL, "1983",
	"Dingo (encrypted)\0", "Encrypted", "Ashby Computers and Graphics LTD. (Jaleco license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, DingoeRomInfo, DingoeRomName, NULL, NULL, DingoInputInfo, DingoDIPInfo,
	DingoeInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Crazy climber sound plus AY8910, and INT instead of NMI
static struct BurnRomInfo MshuttleRomDesc[] = {
	{ "my05",          0x01000, 0x83574af1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "my04",          0x01000, 0x1cfae2c8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "my03",          0x01000, 0xc8b8a368, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "my02",          0x01000, 0xb6aeee6e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "my01",          0x01000, 0xdef82adc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "my09",          0x01000, 0x3601b380, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my11",          0x00800, 0xb659e932, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my08",          0x01000, 0x992b06cd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my10",          0x00800, 0xd860e6ce, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mscprom1.bin",  0x00020, 0xea0d1af0, BRF_GRA | GAL_ROM_PROM },
	
	{ "my07",          0x01000, 0x522a2920, BRF_SND | BRF_OPT },	// Samples
	{ "my06(__eng)",   0x01000, 0x466415f2, BRF_SND | BRF_OPT },
};

STD_ROM_PICK(Mshuttle)
STD_ROM_FN(Mshuttle)

static struct BurnRomInfo Mshuttle2RomDesc[] = {
	{ "my05",          0x01000, 0x83574af1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "my04",          0x01000, 0x1cfae2c8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "my03",          0x01000, 0xc8b8a368, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "my02(__mshuttle2)", 0x01000, 0x9804061c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "my01(__mshuttle2)", 0x01000, 0xca746a61, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "my09",          0x01000, 0x3601b380, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my11(__mshuttle2)", 0x00800, 0xd860e6ce, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my08",          0x01000, 0x992b06cd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my10",          0x00800, 0xd860e6ce, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mscprom1.bin",  0x00020, 0xea0d1af0, BRF_GRA | GAL_ROM_PROM },
	
	{ "my07",          0x01000, 0x522a2920, BRF_SND | BRF_OPT },	// Samples
	{ "my06",          0x01000, 0x6d2dd711, BRF_SND | BRF_OPT },
};

STD_ROM_PICK(Mshuttle2)
STD_ROM_FN(Mshuttle2)

static struct BurnRomInfo MshuttlejRomDesc[] = {
	{ "mcs.5",         0x01000, 0xa5a292b4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mcs.4",         0x01000, 0xacdc0f9e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mcs.3",         0x01000, 0xc1e3f5d8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mcs.2",         0x01000, 0x14577703, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mcs.1",         0x01000, 0x27d46772, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "my09",          0x01000, 0x3601b380, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my11",          0x00800, 0xb659e932, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my08",          0x01000, 0x992b06cd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my10",          0x00800, 0xd860e6ce, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mscprom1.bin",  0x00020, 0xea0d1af0, BRF_GRA | GAL_ROM_PROM },
	
	{ "my07",          0x01000, 0x522a2920, BRF_SND | BRF_OPT },	// Samples
	{ "my06",          0x01000, 0x6d2dd711, BRF_SND | BRF_OPT },
};

STD_ROM_PICK(Mshuttlej)
STD_ROM_FN(Mshuttlej)

static struct BurnRomInfo Mshuttlej2RomDesc[] = {
	{ "ali5.bin",      0x01000, 0x320fe630, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mcs.4",         0x01000, 0xacdc0f9e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mcs.3",         0x01000, 0xc1e3f5d8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali2.bin",      0x01000, 0x9ed169e1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ali1.bin",      0x01000, 0x7f8a52d9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "my09",          0x01000, 0x3601b380, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my11",          0x00800, 0xb659e932, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my08",          0x01000, 0x992b06cd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "my10",          0x00800, 0xd860e6ce, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "mscprom1.bin",  0x00020, 0xea0d1af0, BRF_GRA | GAL_ROM_PROM },
	
	{ "my07",          0x01000, 0x522a2920, BRF_SND | BRF_OPT },	// Samples
	{ "my06.4r",       0x01000, 0x4162be4d, BRF_SND | BRF_OPT },
};

STD_ROM_PICK(Mshuttlej2)
STD_ROM_FN(Mshuttlej2)

UINT8 __fastcall MshuttleZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x0c: {
			if (!MshuttleAY8910CS) return AY8910Read(0);
			return 0xff;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("IO Read %x\n"), a);
	return 0xff;
}

void __fastcall MshuttleZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x08: {
			if (!MshuttleAY8910CS) AY8910Write(0, 0, d);
			return;
		}
		
		case 0x09: {
			if (!MshuttleAY8910CS) AY8910Write(0, 1, d);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("IO Write %x, %x\n"), a, d);
}

void __fastcall MshuttleZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa001: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xa002: {
			GalFlipScreenX = d & 1;
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xa004: {
			// cclimber_sample_trigger_w
			return;
		}
		
		case 0xa007: {
			MshuttleAY8910CS = d & 1;
			return;
		}
		
		case 0xa800: {
			// cclimber_sample_rate_w
			return;
		}
		
		case 0xb000: {
			// cclimber_sample_volume_w
			return;
		}
	}
	bprintf(PRINT_NORMAL, _T("Prog Write %x, %x\n"), a, d);
}

static void MapMshuttle()
{
	MapMooncrst();
	ZetOpen(0);
	ZetMapArea(0x0000, 0x4fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x4fff, 2, GalZ80Rom1Op, GalZ80Rom1);
	ZetSetWriteHandler(MshuttleZ80Write);
	ZetSetInHandler(MshuttleZ80PortRead);
	ZetSetOutHandler(MshuttleZ80PortWrite);
	ZetClose();
}

static void MshuttleDecrypt()
{
	GalZ80Rom1Op = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	
	static const UINT8 ConvTable[8][16] = {
		{ 0x40, 0x41, 0x44, 0x15, 0x05, 0x51, 0x54, 0x55, 0x50, 0x00, 0x01, 0x04, (UINT8)-1, 0x10, 0x11, 0x14 },
		{ 0x45, 0x51, 0x55, 0x44, 0x40, 0x11, 0x05, 0x41, 0x10, 0x14, 0x54, 0x50, 0x15, 0x04, 0x00, 0x01 },
		{ 0x11, 0x14, 0x10, 0x00, 0x44, 0x05, (UINT8)-1, 0x04, 0x45, 0x15, 0x55, 0x50, (UINT8)-1, 0x01, 0x54, 0x51 },
		{ 0x14, 0x01, 0x11, 0x10, 0x50, 0x15, 0x00, 0x40, 0x04, 0x51, 0x45, 0x05, 0x55, 0x54, (UINT8)-1, 0x44 },
		{ 0x04, 0x10, (UINT8)-1, 0x40, 0x15, 0x41, 0x50, 0x50, 0x11, (UINT8)-1, 0x14, 0x00, 0x51, 0x45, 0x55, 0x01 },
		{ 0x44, 0x45, 0x00, 0x51, (UINT8)-1, (UINT8)-1, 0x15, 0x11, 0x01, 0x10, 0x04, 0x55, 0x05, 0x40, 0x50, 0x41 },
		{ 0x51, 0x00, 0x01, 0x05, 0x04, 0x55, 0x54, 0x50, 0x41, (UINT8)-1, 0x11, 0x15, 0x14, 0x10, 0x44, 0x40 },
		{ 0x05, 0x04, 0x51, 0x01, (UINT8)-1, (UINT8)-1, 0x55, (UINT8)-1, 0x00, 0x50, 0x15, 0x14, 0x44, 0x41, 0x40, 0x54 },
	};
	
	for (UINT32 Offset = 0x0000; Offset < GalZ80Rom1Size; Offset++) {
		INT32 i, j;
		UINT8 Src;

		Src = GalZ80Rom1[Offset];
		i = (Offset & 1) | (Src & 0x02) | ((Src & 0x80) >> 5);
		j = (Src & 0x01) | ((Src & 0x04) >> 1) | ((Src & 0x10) >> 2) | ((Src & 0x40) >> 3);

		GalZ80Rom1Op[Offset] = (Src & 0xaa) | ConvTable[i][j];
	}
	
	MapMshuttle();
}

static void MShuttleCommonInit()
{
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	BurnLoadRom(GalTempRom + 0x2000, GAL_ROM_OFFSET_TILES_SHARED + 2, 1);
	BurnLoadRom(GalTempRom + 0x3000, GAL_ROM_OFFSET_TILES_SHARED + 3, 1);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	
	GalDrawBulletsFunction = MshuttleDrawBullets;
	GalExtendTileInfoFunction = MshuttleExtendTileInfo;
	GalExtendSpriteInfoFunction = MshuttleExtendSpriteInfo;

	GalIrqType = GAL_IRQ_TYPE_IRQ0;
}

static INT32 MshuttleInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MshuttleDecrypt;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_MSHUTTLEAY8910;
	GalTilesSharedRomSize = 0x1000;
	
	nRet = GalInit();
	
	MShuttleCommonInit();
	
	return nRet;
}

static void MshuttlejDecrypt()
{
	GalZ80Rom1Op = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	
	static const UINT8 ConvTable[8][16] = {
		{ 0x41, 0x54, 0x51, 0x14, 0x05, 0x10, 0x01, 0x55, 0x44, 0x11, 0x00, 0x50, 0x15, 0x40, 0x04, 0x45 },
		{ 0x50, 0x11, 0x40, 0x55, 0x51, 0x14, 0x45, 0x04, 0x54, 0x15, 0x10, 0x05, 0x44, 0x01, 0x00, 0x41 },
		{ 0x44, 0x11, 0x00, 0x50, 0x41, 0x54, 0x04, 0x14, 0x15, 0x40, 0x51, 0x55, 0x05, 0x10, 0x01, 0x45 },
		{ 0x10, 0x50, 0x54, 0x55, 0x01, 0x44, 0x40, 0x04, 0x14, 0x11, 0x00, 0x41, 0x45, 0x15, 0x51, 0x05 },
		{ 0x14, 0x41, 0x01, 0x44, 0x04, 0x50, 0x51, 0x45, 0x11, 0x40, 0x54, 0x15, 0x10, 0x00, 0x55, 0x05 },
		{ 0x01, 0x05, 0x41, 0x45, 0x54, 0x50, 0x55, 0x10, 0x11, 0x15, 0x51, 0x14, 0x44, 0x40, 0x04, 0x00 },
		{ 0x05, 0x55, 0x00, 0x50, 0x11, 0x40, 0x54, 0x14, 0x45, 0x51, 0x10, 0x04, 0x44, 0x01, 0x41, 0x15 },
		{ 0x55, 0x50, 0x15, 0x10, 0x01, 0x04, 0x41, 0x44, 0x45, 0x40, 0x05, 0x00, 0x11, 0x14, 0x51, 0x54 },
	};
	
	for (UINT32 Offset = 0x0000; Offset < GalZ80Rom1Size; Offset++) {
		INT32 i, j;
		UINT8 Src;

		Src = GalZ80Rom1[Offset];
		i = (Offset & 1) | (Src & 0x02) | ((Src & 0x80) >> 5);
		j = (Src & 0x01) | ((Src & 0x04) >> 1) | ((Src & 0x10) >> 2) | ((Src & 0x40) >> 3);

		GalZ80Rom1Op[Offset] = (Src & 0xaa) | ConvTable[i][j];
	}
	
	MapMshuttle();
}

static INT32 MshuttlejInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MshuttlejDecrypt;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_MSHUTTLEAY8910;
	GalTilesSharedRomSize = 0x1000;
	
	nRet = GalInit();
	
	MShuttleCommonInit();
	
	return nRet;
}

struct BurnDriver BurnDrvMshuttle = {
	"mshuttle", NULL, NULL, NULL, "1981",
	"Moon Shuttle (US?)\0", "Incomplete Sound", "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MshuttleRomInfo, MshuttleRomName, NULL, NULL, MshuttleInputInfo, MshuttleDIPInfo,
	MshuttleInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMshuttle2 = {
	"mshuttle2", "mshuttle", NULL, NULL, "1981",
	"Moon Shuttle (US? set 2)\0", "Incomplete Sound", "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Mshuttle2RomInfo, Mshuttle2RomName, NULL, NULL, MshuttleInputInfo, MshuttleDIPInfo,
	MshuttleInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMshuttlej = {
	"mshuttlej", "mshuttle", NULL, NULL, "1981",
	"Moon Shuttle (Japan)\0", "Incomplete Sound", "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, MshuttlejRomInfo, MshuttlejRomName, NULL, NULL, MshuttleInputInfo, MshuttleDIPInfo,
	MshuttlejInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMshuttlej2 = {
	"mshuttlej2", "mshuttle", NULL, NULL, "1981",
	"Moon Shuttle (Japan set 2)\0", "Incomplete Sound", "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, Mshuttlej2RomInfo, Mshuttlej2RomName, NULL, NULL, MshuttleInputInfo, MshuttleDIPInfo,
	MshuttlejInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

// Second Z80 driving DAC
static struct BurnRomInfo KingballRomDesc[] = {
	{ "prg1.7f",       0x01000, 0x6cb49046, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "prg2.7j",       0x01000, 0xc223b416, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "prg3.7l",       0x00800, 0x453634c0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "kbe1.ic4",      0x00800, 0x5be2c80a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "kbe2.ic5",      0x00800, 0xbb59e965, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "kbe3.ic6",      0x00800, 0x1c94dd31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "kbe2.ic7",      0x00800, 0xbb59e965, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "chg1.1h",       0x00800, 0x9cd550e7, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "chg2.1k",       0x00800, 0xa206757d, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "kb2-1",         0x00020, 0x15dd5b16, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Kingball)
STD_ROM_FN(Kingball)

static struct BurnRomInfo KingballjRomDesc[] = {
	{ "prg1.7f",       0x01000, 0x6cb49046, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "prg2.7j",       0x01000, 0xc223b416, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "prg3.7l",       0x00800, 0x453634c0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "kbj1.ic4",      0x00800, 0xba16beb7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "kbj2.ic5",      0x00800, 0x56686a63, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "kbj3.ic6",      0x00800, 0xfbc570a5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "kbj2.ic7",      0x00800, 0x56686a63, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "chg1.1h",       0x00800, 0x9cd550e7, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "chg2.1k",       0x00800, 0xa206757d, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "kb2-1",         0x00020, 0x15dd5b16, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Kingballj)
STD_ROM_FN(Kingballj)

UINT8 __fastcall KingballZ80Read(UINT16 a)
{
	switch (a) {
		case 0xa000: {
			if (KingballSpeechDip) {
				return GalInput[0] | GalDip[0] | ((GalFakeDip & 0x02) ? 0x40 : 0x00);
			} else {
				return GalInput[0] | GalDip[0] | ((GalFakeDip & 0x01) ? 0x40 : 0x00);
			}
		}
		
		case 0xa800: {
			return ((GalInput[1] | GalDip[1]) & ~0x20) | (rand() & 0x20);
		}
		
		case 0xb000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall KingballZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa004:
		case 0xa005:
		case 0xa006:
		case 0xa007: {
			GalaxianLfoFreqWrite(a - 0xa004, d);
			return;
		}
		
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb000: {
			KingballSound = (KingballSound & ~0x01) | d;
			return;
		}
		
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb002: {
			KingballSound = (KingballSound & ~0x02) | (d << 1);
			GalSoundLatch = KingballSound | 0xf0;
			return;
		}
		
		case 0xb003: {
			KingballSpeechDip = d;
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall KingballSoundZ80Read(UINT16 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall KingballSoundZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall KingballSoundZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return GalSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall KingballSoundZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			DACWrite(0, d ^ 0xff);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static void KingballPostLoad()
{
	MapMooncrst();
	
	ZetOpen(0);
	ZetSetReadHandler(KingballZ80Read);
	ZetSetWriteHandler(KingballZ80Write);
	ZetClose();
	
	ZetOpen(1);
	ZetSetReadHandler(KingballSoundZ80Read);
	ZetSetWriteHandler(KingballSoundZ80Write);
	ZetSetInHandler(KingballSoundZ80PortRead);
	ZetSetOutHandler(KingballSoundZ80PortWrite);
	ZetMapArea(0x0000, 0x1fff, 0, GalZ80Rom2);
	ZetMapArea(0x0000, 0x1fff, 2, GalZ80Rom2);
	ZetMemEnd();
	ZetClose();
	
	nGalCyclesTotal[1] = (5000000 / 2) / 60;
}

static INT32 KingballInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = KingballPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KINGBALLDAC;
	GalTilesSharedRomSize = 0x1000;	
	
	nRet = GalInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	memcpy(GalTempRom + 0x0800, GalTempRom + 0x0000, 0x800);
	memcpy(GalTempRom + 0x1800, GalTempRom + 0x1000, 0x800);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);	
	BurnFree(GalTempRom);
	
	return nRet;
}

struct BurnDriver BurnDrvKingball = {
	"kingball", NULL, NULL, NULL, "1980",
	"King & Balloon (US)\0", NULL, "Namco", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, KingballRomInfo, KingballRomName, NULL, NULL, KingballInputInfo, KingballDIPInfo,
	KingballInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvKingballj = {
	"kingballj", "kingball", NULL, NULL, "1980",
	"King & Balloon (Japan)\0", NULL, "Namco", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, KingballjRomInfo, KingballjRomName, NULL, NULL, KingballInputInfo, KingballDIPInfo,
	KingballInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Frogger based hardware
static struct BurnRomInfo FroggerRomDesc[] = {
	{ "frogger.26",    0x01000, 0x597696d6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "frogger.27",    0x01000, 0xb6e6fcc3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "frsm3.7",       0x01000, 0xaca22ae0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "frogger.608",   0x00800, 0xe8ab0256, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "frogger.609",   0x00800, 0x7380a48f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "frogger.610",   0x00800, 0x31d7eb27, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "frogger.607",   0x00800, 0x05f7d883, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "frogger.606",   0x00800, 0xf524ee30, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "pr-91.6l",      0x00020, 0x413703bf, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Frogger)
STD_ROM_FN(Frogger)

static struct BurnRomInfo Froggers1RomDesc[] = {
	{ "frogger.26",    0x01000, 0x597696d6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "frogger.27",    0x01000, 0xb6e6fcc3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "frogger.34",    0x01000, 0xed866bab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr-608.ic32",  0x00800, 0xe8ab0256, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr-609.ic33",  0x00800, 0x7380a48f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr-610.ic34",  0x00800, 0x31d7eb27, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "epr-607.ic101", 0x00800, 0x05f7d883, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr-606.ic102", 0x00800, 0xf524ee30, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "pr-91.6l",      0x00020, 0x413703bf, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Froggers1)
STD_ROM_FN(Froggers1)

static struct BurnRomInfo Froggers2RomDesc[] = {
	{ "epr-1012.ic5",  0x01000, 0xefab0c79, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr-1013a.ic6", 0x01000, 0xaeca9c13, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr-1014.ic7",  0x01000, 0xdd251066, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr-1015.ic8",  0x01000, 0xbf293a02, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr-608.ic32",  0x00800, 0xe8ab0256, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr-609.ic33",  0x00800, 0x7380a48f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr-610.ic34",  0x00800, 0x31d7eb27, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "epr-607.ic101", 0x00800, 0x05f7d883, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr-606.ic102", 0x00800, 0xf524ee30, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "pr-91.6l",      0x00020, 0x413703bf, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Froggers2)
STD_ROM_FN(Froggers2)

static struct BurnRomInfo FroggermcRomDesc[] = {
	{ "epr-1031.15",   0x01000, 0x4b7c8d11, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr-1032.16",   0x01000, 0xac00b9d9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr-1033.33",   0x01000, 0xbc1d6fbc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr-1034.34",   0x01000, 0x9efe7399, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr-1082.42",   0x01000, 0x802843c2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr-1035.43",   0x00800, 0x14e74148, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "epr-1037.1h",   0x00800, 0x05f7d883, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr-1036.1k",   0x00800, 0x658745f8, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "pr-91.6l",      0x00020, 0x413703bf, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Froggermc)
STD_ROM_FN(Froggermc)

static struct BurnRomInfo FroggersRomDesc[] = {
	{ "vid_d2.bin",    0x00800, 0xc103066e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_e2.bin",    0x00800, 0xf08bc094, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_f2.bin",    0x00800, 0x637a2ff8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_h2.bin",    0x00800, 0x04c027a5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_j2.bin",    0x00800, 0xfbdfbe74, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_l2.bin",    0x00800, 0x8a4389e1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "frogger.608",   0x00800, 0xe8ab0256, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "frogger.609",   0x00800, 0x7380a48f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "frogger.610",   0x00800, 0x31d7eb27, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "frogger.607",   0x00800, 0x05f7d883, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr-1036.1k",   0x00800, 0x658745f8, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "pr-91.6l",      0x00020, 0x413703bf, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Froggers)
STD_ROM_FN(Froggers)

static struct BurnRomInfo FrogfRomDesc[] = {
	{ "6.bin",         0x01000, 0x8ff0a973, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7.bin",         0x01000, 0x3087bb4b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8.bin",         0x01000, 0xc3869d12, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "frogger.608",   0x00800, 0xe8ab0256, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "frogger.609",   0x00800, 0x7380a48f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "frogger.610",   0x00800, 0x31d7eb27, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "frogger.607",   0x00800, 0x05f7d883, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr-1036.1k",   0x00800, 0x658745f8, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "pr-91.6l",      0x00020, 0x413703bf, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Frogf)
STD_ROM_FN(Frogf)

static struct BurnRomInfo FroggRomDesc[] = {
	{ "p1.bin",        0x00800, 0x1762b266, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p2.bin",        0x00800, 0x322f3916, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p3.bin",        0x00800, 0x28bd6151, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p4.bin",        0x00800, 0x5a69ab18, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p5.bin",        0x00800, 0xb4f17745, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p6.bin",        0x00800, 0x34be71b5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p7.bin",        0x00800, 0xde3edc8c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "k.bin",         0x00800, 0x05f7d883, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "h.bin",         0x00800, 0x658745f8, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "pr-91.6l",      0x00020, 0x413703bf, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Frogg)
STD_ROM_FN(Frogg)

void __fastcall FroggermcZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa800: {
			GalSoundLatch = d;
			return;
		}
		
		case 0xb000: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb001: {
			if (!(d & 1)) {
				ZetClose();
				ZetOpen(1);
				ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
				ZetClose();
				ZetOpen(0);
			}
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall FrogfZ80Read(UINT16 a)
{
	if (a >= 0xc000) {
		UINT32 Offset = a - 0xc000;
		UINT8 Result = 0xff;
		if (Offset & 0x1000) Result &= ppi8255_r(0, (Offset >> 3) & 3);
		if (Offset & 0x2000) Result &= ppi8255_r(1, (Offset >> 3) & 3);
		return Result;
	}
	
	switch (a) {
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall FrogfZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9000 && a <= 0x90ff) {
		INT32 Offset = a - 0x9000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0xc000) {
		INT32 Offset = a - 0xc000;
		if (Offset & 0x1000) ppi8255_w(0, (Offset >> 3) & 3, d);
		if (Offset & 0x2000) ppi8255_w(1, (Offset >> 3) & 3, d);
		return;
	}
	
	switch (a) {
		case 0xa802: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xa804: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa806: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xa809: {
			// coin_count_1_w
			return;
		}
		
		case 0xa80e: {
			// coin_count_0_w
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void FroggerEncGfxPostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	BurnLoadRom(GalTempRom + 0x0800, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	for (UINT32 Offset = 0x0800; Offset < 0x1000; Offset++) GalTempRom[Offset] = BITSWAP08(GalTempRom[Offset], 7, 6, 5, 4, 3, 2, 0, 1);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	
	MapFrogger();
}

static INT32 FroggerEncGfxInit()
{
	INT32 nRet;

	GalPostLoadCallbackFunction = FroggerEncGfxPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910;

	nRet = GalInit();
	FroggerSoundInit();
	
	FroggerAdjust = 1;	
	
	GalRenderBackgroundFunction = FroggerDrawBackground;
	GalDrawBulletsFunction = NULL;
	GalExtendTileInfoFunction = FroggerExtendTileInfo;
	GalExtendSpriteInfoFunction = FroggerExtendSpriteInfo;
	
	KonamiPPIInit();
	
	return nRet;
}

static void FroggermcPostLoad()
{
	MapMooncrst();
	
	ZetOpen(0);
	ZetSetWriteHandler(FroggermcZ80Write);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetClose();
}

static INT32 FroggermcInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = FroggermcPostLoad;
//	GalSoundType = GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910;
	
	nRet = GalInit();	
	FroggerSoundInit();
	
	GalRenderBackgroundFunction = FroggerDrawBackground;
	GalDrawBulletsFunction = NULL;
	GalExtendTileInfoFunction = FroggerExtendTileInfo;
	GalExtendSpriteInfoFunction = FroggerExtendSpriteInfo;
	
	return nRet;
}

static INT32 FroggersInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapTheend;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910;
	
	nRet = GalInit();	
	FroggerSoundInit();
	
	KonamiPPIInit();
	
	GalRenderBackgroundFunction = FroggerDrawBackground;
	GalDrawBulletsFunction = NULL;
	GalExtendTileInfoFunction = FroggerExtendTileInfo;
	GalExtendSpriteInfoFunction = FroggerExtendSpriteInfo;
	
	return nRet;
}

static void FrogfPostLoad()
{
	MapFrogger();

	ZetOpen(0);
	ZetSetReadHandler(FrogfZ80Read);
	ZetSetWriteHandler(FrogfZ80Write);
	ZetMemCallback(0x8800, 0xffff, 0);
	ZetMemCallback(0x8800, 0xffff, 1);
	ZetMemCallback(0x8800, 0xffff, 2);
	ZetMapArea(0x8800, 0x8bff, 0, GalVideoRam);
	ZetMapArea(0x8800, 0x8bff, 1, GalVideoRam);
	ZetMapArea(0x8800, 0x8bff, 2, GalVideoRam);
	ZetMapArea(0x9000, 0x90ff, 0, GalSpriteRam);
	ZetMapArea(0x9000, 0x90ff, 2, GalSpriteRam);
	ZetClose();
}

static INT32 FrogfInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = FrogfPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910;

	nRet = GalInit();	
	FroggerSoundInit();
	
	KonamiPPIInit();
	
	GalRenderBackgroundFunction = FroggerDrawBackground;
	GalDrawBulletsFunction = NULL;
	GalExtendTileInfoFunction = FroggerExtendTileInfo;
	GalExtendSpriteInfoFunction = FroggerExtendSpriteInfo;
	
	return nRet;
}

static void FroggPostLoad()
{
	ZetOpen(0);
	ZetMapArea(0x4000, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 2, GalZ80Ram1);
	ZetClose();
}

static INT32 FroggInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = FroggPostLoad;

	nRet = GalInit();
	
	GalRenderBackgroundFunction = FroggerDrawBackground;
	GalExtendTileInfoFunction = FroggerExtendTileInfo;
	GalExtendSpriteInfoFunction = FroggerExtendSpriteInfo;
	
	return nRet;
}

struct BurnDriver BurnDrvFrogger = {
	"frogger", NULL, NULL, NULL, "1981",
	"Frogger\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, FroggerRomInfo, FroggerRomName, NULL, NULL, FroggerInputInfo, FroggerDIPInfo,
	FroggerEncGfxInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFroggers1 = {
	"froggers1", "frogger", NULL, NULL, "1981",
	"Frogger (Sega set 1)\0", NULL, "Konami (Sega license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, Froggers1RomInfo, Froggers1RomName, NULL, NULL, FroggerInputInfo, FroggerDIPInfo,
	FroggerEncGfxInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFroggers2 = {
	"froggers2", "frogger", NULL, NULL, "1981",
	"Frogger (Sega set 2)\0", NULL, "Konami (Sega license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, Froggers2RomInfo, Froggers2RomName, NULL, NULL, FroggerInputInfo, FroggerDIPInfo,
	FroggerEncGfxInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFroggermc = {
	"froggermc", "frogger", NULL, NULL, "1981",
	"Frogger (Moon Cresta hardware)\0", "No Sound", "Konami (Sega license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, FroggermcRomInfo, FroggermcRomName, NULL, NULL, FroggermcInputInfo, FroggermcDIPInfo,
	FroggermcInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFroggers = {
	"froggers", "frogger", NULL, NULL, "1981",
	"Frog\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, FroggersRomInfo, FroggersRomName, NULL, NULL, FroggerInputInfo, FroggerDIPInfo,
	FroggersInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFrogf = {
	"frogf", "frogger", NULL, NULL, "1981",
	"Frog (Falcon bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, FrogfRomInfo, FrogfRomName, NULL, NULL, FroggerInputInfo, FroggerDIPInfo,
	FrogfInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFrogg = {
	"frogg", "frogger", NULL, NULL, "1981",
	"Frog (Galaxian hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, FroggRomInfo, FroggRomName, NULL, NULL, FroggInputInfo, FroggDIPInfo,
	FroggInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Turtles based hardware
static struct BurnRomInfo TurtlesRomDesc[] = {
	{ "turt_vid.2c",   0x01000, 0xec5e61fb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "turt_vid.2e",   0x01000, 0xfd10821e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "turt_vid.2f",   0x01000, 0xddcfc5fa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "turt_vid.2h",   0x01000, 0x9e71696c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "turt_vid.2j",   0x01000, 0xfcd49fef, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "turt_snd.5c",   0x01000, 0xf0c30f9a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "turt_snd.5d",   0x01000, 0xaf5fc43c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "turt_vid.5h",   0x00800, 0xe5999d52, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "turt_vid.5f",   0x00800, 0xc3ffd655, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "turtles.clr",   0x00020, 0xf3ef02dd, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Turtles)
STD_ROM_FN(Turtles)

static struct BurnRomInfo TurpinRomDesc[] = {
	{ "m1",            0x01000, 0x89177473, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m2",            0x01000, 0x4c6ca5c6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m3",            0x01000, 0x62291652, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "turt_vid.2h",   0x01000, 0x9e71696c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "m5",            0x01000, 0x7d2600f2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "turt_snd.5c",   0x01000, 0xf0c30f9a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "turt_snd.5d",   0x01000, 0xaf5fc43c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "turt_vid.5h",   0x00800, 0xe5999d52, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "turt_vid.5f",   0x00800, 0xc3ffd655, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "turtles.clr",   0x00020, 0xf3ef02dd, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Turpin)
STD_ROM_FN(Turpin)

static struct BurnRomInfo SixhundredRomDesc[] = {
	{ "600_vid.2c",    0x01000, 0x8ee090ae, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "600_vid.2e",    0x01000, 0x45bfaff2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "600_vid.2f",    0x01000, 0x9f4c8ed7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "600_vid.2h",    0x01000, 0xa92ef056, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "600_vid.2j",    0x01000, 0x6dadd72d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "600_snd.5c",    0x01000, 0x1773c68e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "600_snd.5d",    0x01000, 0xa311b998, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "600_vid.5h",    0x00800, 0x006c3d56, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "600_vid.5f",    0x00800, 0x7dbc0426, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "turtles.clr",   0x00020, 0xf3ef02dd, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Sixhundred)
STD_ROM_FN(Sixhundred)

static struct BurnRomInfo TurpinsRomDesc[] = {
	{ "t1.bin",        0x01000, 0x89dd50cc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "t3.bin",        0x01000, 0x9562dc29, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "t4.bin",        0x01000, 0x62291652, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "t5.bin",        0x01000, 0x804118e8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "t2.bin",        0x01000, 0x8024f678, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "8tur.bin",      0x01000, 0xc97ed8ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "5tur.bin",      0x01000, 0xaf5fc43c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "tur.4f",        0x00800, 0xe5999d52, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "tur.5f",        0x00800, 0xc3ffd655, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "turtles.clr",   0x00020, 0xf3ef02dd, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Turpins)
STD_ROM_FN(Turpins)

static struct BurnRomInfo AmidarRomDesc[] = {
	{ "1.2c",          0x01000, 0x621b74de, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.2e",          0x01000, 0x38538b98, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.2f",          0x01000, 0x099ecb24, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.2h",          0x01000, 0xba149a93, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.2j",          0x01000, 0xeecc1abf, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "s1.5c",         0x01000, 0x8ca7b750, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "s2.5d",         0x01000, 0x9b5bdc0a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "c2.5f",         0x00800, 0x2cfe5ede, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "c2.5d",         0x00800, 0x57c4fd0d, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "amidar.clr",    0x00020, 0xf940dcc3, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Amidar)
STD_ROM_FN(Amidar)

static struct BurnRomInfo Amidar1RomDesc[] = {
	{ "amidar.2c",     0x01000, 0xc294bf27, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "amidar.2e",     0x01000, 0xe6e96826, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "amidar.2f",     0x01000, 0x3656be6f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "amidar.2h",     0x01000, 0x1be170bd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "amidar.5c",     0x01000, 0xc4b66ae4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "amidar.5d",     0x01000, 0x806785af, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "amidar.5f",     0x00800, 0x5e51e84d, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "amidar.5h",     0x00800, 0x2f7f1c30, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "amidar.clr",    0x00020, 0xf940dcc3, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Amidar1)
STD_ROM_FN(Amidar1)

static struct BurnRomInfo AmidaruRomDesc[] = {
	{ "amidarus.2c",   0x01000, 0x951e0792, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "amidarus.2e",   0x01000, 0xa1a3a136, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "amidarus.2f",   0x01000, 0xa5121bf5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "amidarus.2h",   0x01000, 0x051d1c7f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "amidarus.2j",   0x01000, 0x351f00d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "amidarus.5c",   0x01000, 0x8ca7b750, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "amidarus.5d",   0x01000, 0x9b5bdc0a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "amidarus.5f",   0x00800, 0x2cfe5ede, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "amidarus.5h",   0x00800, 0x57c4fd0d, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "amidar.clr",    0x00020, 0xf940dcc3, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Amidaru)
STD_ROM_FN(Amidaru)

static struct BurnRomInfo AmidaroRomDesc[] = {
	{ "107.2cd",       0x01000, 0xc52536be, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "108.2fg",       0x01000, 0x38538b98, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "109.2fg",       0x01000, 0x69907f0f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "110.2h",        0x01000, 0xba149a93, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "111.2j",        0x01000, 0x20d01c2e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "amidarus.5c",   0x01000, 0x8ca7b750, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "amidarus.5d",   0x01000, 0x9b5bdc0a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "amidarus.5f",   0x00800, 0x2cfe5ede, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "113.5h",        0x00800, 0xbcdce168, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "amidar.clr",    0x00020, 0xf940dcc3, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Amidaro)
STD_ROM_FN(Amidaro)

static struct BurnRomInfo AmidarbRomDesc[] = {
	{ "ami2gor.2c",    0x01000, 0x9ad2dcd2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.2f",          0x01000, 0x66282ff5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.2j",          0x01000, 0xb0860e31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.2m",          0x01000, 0x4a4086c9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "8.11d",         0x01000, 0x8ca7b750, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "9.9d",          0x01000, 0x9b5bdc0a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "5.5f",          0x00800, 0x2082ad0a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "6.5h",          0x00800, 0x3029f94f, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "n82s123n.6e",   0x00020, 0x01004d3f, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Amidarb)
STD_ROM_FN(Amidarb)

static struct BurnRomInfo AmigoRomDesc[] = {
	{ "2732.a1",       0x01000, 0x930dc856, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2732.a2",       0x01000, 0x66282ff5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2732.a3",       0x01000, 0xe9d3dc76, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2732.a4",       0x01000, 0x4a4086c9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "amidarus.5c",   0x01000, 0x8ca7b750, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "amidarus.5d",   0x01000, 0x9b5bdc0a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "2716.a6",       0x00800, 0x2082ad0a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "2716.a5",       0x00800, 0x3029f94f, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "amidar.clr",    0x00020, 0xf940dcc3, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Amigo)
STD_ROM_FN(Amigo)

static struct BurnRomInfo AmidarsRomDesc[] = {
	{ "am2d",          0x00800, 0x24b79547, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "am2e",          0x00800, 0x4c64161e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "am2f",          0x00800, 0xb3987a72, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "am2h",          0x00800, 0x29873461, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "am2j",          0x00800, 0x0fdd54d8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "am2l",          0x00800, 0x5382f7ed, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "am2m",          0x00800, 0x1d7109e9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "am2p",          0x00800, 0xc9163ac6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "amidarus.5c",   0x01000, 0x8ca7b750, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "amidarus.5d",   0x01000, 0x9b5bdc0a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "2716.a6",       0x00800, 0x2082ad0a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "2716.a5",       0x00800, 0x3029f94f, BRF_GRA | GAL_ROM_TILES_SHARED },
	
	{ "amidar.clr",    0x00020, 0xf940dcc3, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Amidars)
STD_ROM_FN(Amidars)

UINT8 __fastcall TurpinsZ80Read(UINT16 a)
{
	switch (a) {
		case 0xa000: {
			return 0xff - GalInput[0] - GalDip[0];
		}
		
		case 0xa001: {
			return 0xff - GalInput[1] - GalDip[1];
		}
		
		case 0xa002: {
			return 0xff - GalInput[2] - GalDip[2];
		}
		
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall TurpinsZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000: {
			GalBackgroundRed = d & 1;
			return;
		}
		
		case 0xa020: {
			GalBackgroundGreen = d & 1;
			return;
		}
		
		case 0xa028: {
			GalBackgroundBlue = d & 1;
			return;
		}

		
		case 0xa030: {
			// coin_count_0_w
			return;
		}
		
		case 0xa038: {
			// coin_count_1_w
			return;
		}


		case 0xa801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xa807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 TurtlesInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapTurtles;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = TurtlesDrawBackground;
	GalDrawBulletsFunction = NULL;
	
	KonamiPPIInit();
	
	return nRet;
}

static void TurpinsPostLoad()
{
	MapTurtles();
	
	ZetOpen(0);
	ZetSetReadHandler(TurpinsZ80Read);
	ZetSetWriteHandler(TurpinsZ80Write);
	ZetClose();
}

static INT32 TurpinsInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = TurpinsPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = TurtlesDrawBackground;
	GalDrawBulletsFunction = NULL;
	
	return nRet;
}

struct BurnDriver BurnDrvTurtles = {
	"turtles", NULL, NULL, NULL, "1981",
	"Turtles\0", NULL, "Konami (Stern license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, TurtlesRomInfo, TurtlesRomName, NULL, NULL, TurtlesInputInfo, TurtlesDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTurpin = {
	"turpin", "turtles", NULL, NULL, "1981",
	"Turpin\0", NULL, "Konami (Sega license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, TurpinRomInfo, TurpinRomName, NULL, NULL, TurtlesInputInfo, TurpinDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSixhundred = {
	"600", "turtles", NULL, NULL, "1981",
	"600\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, SixhundredRomInfo, SixhundredRomName, NULL, NULL, TurtlesInputInfo, TurtlesDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTurpins = {
	"turpins", "turtles", NULL, NULL, "1981",
	"Turpin (bootleg on Scramble hardware)\0", "No Sound", "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, TurpinsRomInfo, TurpinsRomName, NULL, NULL, TurtlesInputInfo, TurtlesDIPInfo,
	TurpinsInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmidar = {
	"amidar", NULL, NULL, NULL, "1981",
	"Amidar\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AmidarRomInfo, AmidarRomName, NULL, NULL, AmidarInputInfo, AmidaruDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmidar1 = {
	"amidar1", "amidar", NULL, NULL, "1981",
	"Amidar (older)\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, Amidar1RomInfo, Amidar1RomName, NULL, NULL, AmidarInputInfo, AmidarDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmidaru = {
	"amidaru", "amidar", NULL, NULL, "1982",
	"Amidar (Stern)\0", NULL, "Konami (Stern license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AmidaruRomInfo, AmidaruRomName, NULL, NULL, AmidarInputInfo, AmidaruDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmidaro = {
	"amidaro", "amidar", NULL, NULL, "1982",
	"Amidar (Olympia)\0", NULL, "Konami (Olympia license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AmidaroRomInfo, AmidaroRomName, NULL, NULL, AmidarInputInfo, AmidaroDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmidarb = {
	"amidarb", "amidar", NULL, NULL, "1982",
	"Amidar (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AmidarbRomInfo, AmidarbRomName, NULL, NULL, AmidarInputInfo, AmidaruDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmigo = {
	"amigo", "amidar", NULL, NULL, "1982",
	"Amigo\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AmigoRomInfo, AmigoRomName, NULL, NULL, AmidarInputInfo, AmidaruDIPInfo,
	TurtlesInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmidars = {
	"amidars", "amidar", NULL, NULL, "1982",
	"Amidar (Scramble hardware)\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AmidarsRomInfo, AmidarsRomName, NULL, NULL, AmidarInputInfo, AmidarsDIPInfo,
	ScrambleInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// The End / Scramble based hardware
static struct BurnRomInfo TheendRomDesc[] = {
	{ "ic13_1t.bin",   0x00800, 0x93e555ba, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic14_2t.bin",   0x00800, 0x2de7ad27, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic15_3t.bin",   0x00800, 0x035f750b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic16_4t.bin",   0x00800, 0x61286b5c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic17_5t.bin",   0x00800, 0x434a8f68, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic18_6t.bin",   0x00800, 0xdc4cc786, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ic56_1.bin",    0x00800, 0x7a141f29, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ic55_2.bin",    0x00800, 0x218497c1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "ic30_2c.bin",   0x00800, 0x68ccf7bf, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ic31_1c.bin",   0x00800, 0x4a48c999, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "6331-1j.86",    0x00020, 0x24652bc4, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Theend)
STD_ROM_FN(Theend)

static struct BurnRomInfo TheendsRomDesc[] = {
	{ "ic13",          0x00800, 0x90e5ab14, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic14",          0x00800, 0x950f0a07, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic15",          0x00800, 0x6786bcf5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic16",          0x00800, 0x380a0017, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic17",          0x00800, 0xaf067b7f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic18",          0x00800, 0xa0411b93, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ic56",          0x00800, 0x3b2c2f70, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ic55",          0x00800, 0xe0429e50, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "ic30",          0x00800, 0x527fd384, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ic31",          0x00800, 0xaf6d09b6, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "6331-1j.86",    0x00020, 0x24652bc4, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Theends)
STD_ROM_FN(Theends)

static struct BurnRomInfo ScrambleRomDesc[] = {
	{ "s1.2d",         0x00800, 0xea35ccaa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "s2.2e",         0x00800, 0xe7bba1b3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "s3.2f",         0x00800, 0x12d7fc3e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "s4.2h",         0x00800, 0xb59360eb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "s5.2j",         0x00800, 0x4919a91c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "s6.2l",         0x00800, 0x26a4547b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "s7.2m",         0x00800, 0x0bb49470, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "s8.2p",         0x00800, 0x6a5740e5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ot1.5c",        0x00800, 0xbcd297f0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot2.5d",        0x00800, 0xde7912da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot3.5e",        0x00800, 0xba2fa933, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "c2.5f",         0x00800, 0x4708845b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "c1.5h",         0x00800, 0x11fd2887, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scramble)
STD_ROM_FN(Scramble)

static struct BurnRomInfo ScramblesRomDesc[] = {
	{ "2d",            0x00800, 0xb89207a1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e",            0x00800, 0xe9b4b9eb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f",            0x00800, 0xa1f14f4c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h",            0x00800, 0x591bc0d9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j",            0x00800, 0x22f11b6b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l",            0x00800, 0x705ffe49, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2m",            0x00800, 0xea26c35c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2p",            0x00800, 0x94d8f5e3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ot1.5c",        0x00800, 0xbcd297f0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot2.5d",        0x00800, 0xde7912da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot3.5e",        0x00800, 0xba2fa933, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "5f",            0x00800, 0x5f30311a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h",            0x00800, 0x516e029e, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scrambles)
STD_ROM_FN(Scrambles)

static struct BurnRomInfo ScramblebfRomDesc[] = {
	{ "scf1.2d",       0x00800, 0xb126aa1f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scf2.2e",       0x00800, 0xce25fb77, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scns3.2f",      0x00800, 0xeec265ee, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scns4.2h",      0x00800, 0xdd380a22, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scns5.2j",      0x00800, 0x92980e72, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scns6.2l",      0x00800, 0x9fd96374, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scns7.2m",      0x00800, 0x88ac07a0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scns8.2p",      0x00800, 0x75232e09, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ot1ns.5c",      0x00800, 0xbe037cf6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot2.5d",        0x00800, 0xde7912da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot3.5e",        0x00800, 0xba2fa933, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "c2.5f",         0x00800, 0x4708845b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "c1.5h",         0x00800, 0x11fd2887, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scramblebf)
STD_ROM_FN(Scramblebf)

static struct BurnRomInfo ScramblebbRomDesc[] = {
	{ "1",             0x00800, 0x8ba174c4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2",             0x00800, 0x43cb40a4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3",             0x00800, 0xeec265ee, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4",             0x00800, 0xdd380a22, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5",             0x00800, 0x92980e72, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6",             0x00800, 0x9fd96374, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7",             0x00800, 0x88ac07a0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8",             0x00800, 0x75232e09, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ot1.5c",        0x00800, 0xbcd297f0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot2.5d",        0x00800, 0xde7912da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot3.5e",        0x00800, 0xba2fa933, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "9",             0x00800, 0x4708845b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "10",            0x00800, 0x11fd2887, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scramblebb)
STD_ROM_FN(Scramblebb)

static struct BurnRomInfo StrfbombRomDesc[] = {
	{ "1.2c",          0x00800, 0xb102aaa0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.2e",          0x00800, 0xd4155703, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.2f",          0x00800, 0xa9568c89, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.2h",          0x00800, 0x663b6c35, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.2j",          0x00800, 0x4919a91c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6.2l",          0x00800, 0x4ec66ae3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7.2m",          0x00800, 0x0feb0192, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8.2p",          0x00800, 0x280a6142, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ot1.5c",        0x00800, 0xbcd297f0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot2.5d",        0x00800, 0xde7912da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot3.5e",        0x00800, 0xba2fa933, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "9.5f",          0x00800, 0x3abeff25, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "10.5h",         0x00800, 0x79ecacbe, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Strfbomb)
STD_ROM_FN(Strfbomb)

static struct BurnRomInfo ExplorerRomDesc[] = {
	{ "10l.bin",       0x01000, 0xd5adf626, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "9l.bin",        0x01000, 0x48e32788, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "8l.bin",        0x01000, 0xc0dbdbde, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "7l.bin",        0x01000, 0x9b30d227, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "3f.bin",        0x01000, 0x9faf18cf, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "4b.bin",        0x00800, 0xe910b5c3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "c2.5f",         0x00800, 0x4708845b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "c1.5h",         0x00800, 0x11fd2887, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Explorer)
STD_ROM_FN(Explorer)

static struct BurnRomInfo AtlantisRomDesc[] = {
	{ "2c",            0x00800, 0x0e485b9a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e",            0x00800, 0xc1640513, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f",            0x00800, 0xeec265ee, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h",            0x00800, 0xa5d2e442, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j",            0x00800, 0x45f7cf34, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l",            0x00800, 0xf335b96b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ot1.5c",        0x00800, 0xbcd297f0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot2.5d",        0x00800, 0xde7912da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot3.5e",        0x00800, 0xba2fa933, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "5f",            0x00800, 0x57f9c6b9, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h",            0x00800, 0xe989f325, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Atlantis)
STD_ROM_FN(Atlantis)

static struct BurnRomInfo Atlantis2RomDesc[] = {
	{ "rom1",          0x00800, 0xad348089, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom2",          0x00800, 0xcaa705d1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom3",          0x00800, 0xe420641d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom4",          0x00800, 0x04792d90, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j",            0x00800, 0x45f7cf34, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom6",          0x00800, 0xb297bd4b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom7",          0x00800, 0xa50bf8d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rom8",          0x00800, 0xd2c5c984, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ot1.5c",        0x00800, 0xbcd297f0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot2.5d",        0x00800, 0xde7912da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ot3.5e",        0x00800, 0xba2fa933, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "rom9",          0x00800, 0x55cd5acd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom10",         0x00800, 0x72e773b8, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Atlantis2)
STD_ROM_FN(Atlantis2)

static struct BurnRomInfo CkongsRomDesc[] = {
	{ "vid_2c.bin",    0x01000, 0x49a8c234, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_2e.bin",    0x01000, 0xf1b667f1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_2f.bin",    0x01000, 0xb194b75d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_2h.bin",    0x01000, 0x2052ba8a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_2j.bin",    0x01000, 0xb377afd0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_2l.bin",    0x01000, 0xfe65e691, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "turt_snd.5c",   0x01000, 0xf0c30f9a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "snd_5d.bin",    0x01000, 0x892c9547, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "vid_5f.bin",    0x01000, 0x7866d2cb, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "vid_5h.bin",    0x01000, 0x7311a101, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "vid_6e.bin",    0x00020, 0x5039af97, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Ckongs)
STD_ROM_FN(Ckongs)

static struct BurnRomInfo MarsRomDesc[] = {
	{ "u26.3",         0x00800, 0x2f88892c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u56.4",         0x00800, 0x9e6bcbf7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u69.5",         0x00800, 0xdf496e6e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u98.6",         0x00800, 0x75f274bb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u114.7",        0x00800, 0x497fd8d0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u133.8",        0x00800, 0x3d4cd59f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "u39.9",         0x00800, 0xbb5968b9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "u51.10",        0x00800, 0x75fd7720, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "u78.11",        0x00800, 0x72a492da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "u72.1",         0x00800, 0x279789d0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "u101.2",        0x00800, 0xc5dc627f, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mars)
STD_ROM_FN(Mars)

static struct BurnRomInfo DevilfshRomDesc[] = {
	{ "u26.1",         0x00800, 0xec047d71, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u56.2",         0x00800, 0x0138ade9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u69.3",         0x00800, 0x5dd0b3fc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u98.4",         0x00800, 0xded0b745, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u114.5",        0x00800, 0x5fd40176, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u133.6",        0x00800, 0x03538336, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u143.7",        0x00800, 0x64676081, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "u163.8",        0x00800, 0xbc3d6770, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "u39.9",         0x00800, 0x09987e2e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "u51.10",        0x00800, 0x1e2b1471, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "u78.11",        0x00800, 0x45279aaa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "u72.12",        0x01000, 0x5406508e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "u101.13",       0x01000, 0x8c4018b6, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Devilfsh)
STD_ROM_FN(Devilfsh)

static struct BurnRomInfo Newsin7RomDesc[] = {
	{ "newsin.1",      0x01000, 0xe6c23fe0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "newsin.2",      0x01000, 0x3d477b5f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "newsin.3",      0x01000, 0x7dfa9af0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "newsin.4",      0x01000, 0xd1b0ba19, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "newsin.5",      0x01000, 0x06275d59, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "newsin.13",     0x00800, 0xd88489a2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "newsin.12",     0x00800, 0xb154a7af, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "newsin.11",     0x00800, 0x7ade709b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "newsin.9",      0x01000, 0x6b87adff, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "newsin.8",      0x01000, 0x0c5b895a, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "newsin.7",      0x01000, 0x6bc5d64f, BRF_GRA | GAL_ROM_TILES_SHARED },	
		
	{ "newsin.6",      0x00020, 0x5cf2cd8d, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Newsin7)
STD_ROM_FN(Newsin7)

static struct BurnRomInfo MrkougarRomDesc[] = {
	{ "2732-7.bin",    0x01000, 0xfd060ffb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2732-6.bin",    0x01000, 0x9e05d868, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2732-5.bin",    0x01000, 0xcbc7c536, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "atw-6w-2.bin",  0x01000, 0xaf42a371, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "atw-6y-3.bin",  0x01000, 0x862b8902, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "atw-6z-4.bin",  0x01000, 0xa0396cc8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "2732-1.bin",    0x01000, 0x60ef1d43, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mrkougar)
STD_ROM_FN(Mrkougar)

static struct BurnRomInfo Mrkougar2RomDesc[] = {
	{ "atw-7l-7.bin",  0x01000, 0x7b34b198, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "atw-7k-6.bin",  0x01000, 0xfbca23c7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "atw-7h-5.bin",  0x01000, 0x05b257a2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "atw-6w-2.bin",  0x01000, 0xaf42a371, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "atw-6y-3.bin",  0x01000, 0x862b8902, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "atw-6z-4.bin",  0x01000, 0xa0396cc8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "atw-1h-1.bin",  0x01000, 0x38fdfb63, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "atw-prom.bin",  0x00020, 0xc65db188, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mrkougar2)
STD_ROM_FN(Mrkougar2)

static struct BurnRomInfo MrkougbRomDesc[] = {
	{ "p01.bin",       0x00800, 0xdea0cde1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p02.bin",       0x00800, 0xc8017751, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p03.bin",       0x00800, 0xb8921984, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p04.bin",       0x00800, 0xb3c9754c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p05.bin",       0x00800, 0x8d94adbc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p06.bin",       0x00800, 0xacc921ff, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "atw-6w-2.bin",  0x01000, 0xaf42a371, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "atw-6y-3.bin",  0x01000, 0x862b8902, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "atw-6z-4.bin",  0x01000, 0xa0396cc8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "g07.bin",       0x00800, 0x0ecfd116, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g08.bin",       0x00800, 0x00bfa3c6, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "atw-prom.bin",  0x00020, 0xc65db188, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mrkougb)
STD_ROM_FN(Mrkougb)

static struct BurnRomInfo Mrkougb2RomDesc[] = {
	{ "mrk1.bin",      0x00800, 0xfc93acb9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p02.bin",       0x00800, 0xc8017751, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p03.bin",       0x00800, 0xb8921984, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p04.bin",       0x00800, 0xb3c9754c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p05.bin",       0x00800, 0x8d94adbc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p06.bin",       0x00800, 0xacc921ff, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "atw-6w-2.bin",  0x01000, 0xaf42a371, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "atw-6y-3.bin",  0x01000, 0x862b8902, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "atw-6z-4.bin",  0x01000, 0xa0396cc8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "g07.bin",       0x00800, 0x0ecfd116, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "g08.bin",       0x00800, 0x00bfa3c6, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "atw-prom.bin",  0x00020, 0xc65db188, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mrkougb2)
STD_ROM_FN(Mrkougb2)

static struct BurnRomInfo HotshockRomDesc[] = {
	{ "0d.l10",        0x01000, 0x3e8aeaeb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "1d.l9",         0x01000, 0x0eab3246, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2d.l8",         0x01000, 0xe646bde3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3d.l7",         0x01000, 0x5bde9312, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "6d.b3",         0x01000, 0xc5e02651, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "7d.b4",         0x01000, 0x49dc113d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "4d.h3",         0x01000, 0x751c850e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5d.h5",         0x01000, 0xfc74282e, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "18s030.1k",     0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Hotshock)
STD_ROM_FN(Hotshock)

static struct BurnRomInfo HotshockbRomDesc[] = {
	{ "hotshock.l10",  0x01000, 0x401078f7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "hotshock.l9",   0x01000, 0xaf76c237, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "hotshock.l8",   0x01000, 0x30486031, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "hotshock.l7",   0x01000, 0x5bde9312, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "hotshock.b3",   0x01000, 0x0092f0e2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "hotshock.b4",   0x01000, 0xc2135a44, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "hotshock.h4",   0x01000, 0x60bdaea9, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "hotshock.h5",   0x01000, 0x4ef17453, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Hotshockb)
STD_ROM_FN(Hotshockb)

static struct BurnRomInfo ConquerRomDesc[] = {
	{ "conquer3.l10",  0x01000, 0xa33a824f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "conquer2.l9",   0x01000, 0x3ffa8285, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "conquer1.l8",   0x01000, 0x9ded2dff, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "conquer6.b3",   0x01000, 0xd363b2ea, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "conquer7.b4",   0x01000, 0xe6a63d71, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "conquer4.h3",   0x01000, 0xac533893, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "conquer5.h5",   0x01000, 0xd884fd49, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Conquer)
STD_ROM_FN(Conquer)

static struct BurnRomInfo CavelonRomDesc[] = {
	{ "2.bin",         0x02000, 0xa3b353ac, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "1.bin",         0x02000, 0x3f62efd6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.bin",         0x02000, 0x39d74e4e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "1c_snd.bin",    0x00800, 0xf58dcf55, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "h.bin",         0x01000, 0xd44fcd6f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k.bin",         0x01000, 0x59bc7f9e, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "cavelon.clr",   0x00020, 0xd133356b, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Cavelon)
STD_ROM_FN(Cavelon)

static struct BurnRomInfo MimonscrRomDesc[] = {
	{ "mm1",           0x01000, 0x0399a0c4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm2",           0x01000, 0x2c5e971e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm3",           0x01000, 0x24ce1ce3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm4",           0x01000, 0xc83fb639, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm5",           0x01000, 0xa9f12dfc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm6",           0x01000, 0xe492a40c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm7",           0x01000, 0x5339928d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm8",           0x01000, 0xeee7a12e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mmsound1",      0x01000, 0x2d14c527, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "mmsnd2a",       0x01000, 0x35ed0f96, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "mmgfx1",        0x02000, 0x4af47337, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mmgfx2",        0x02000, 0xdef47da8, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "c01s.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mimonscr)
STD_ROM_FN(Mimonscr)

UINT8 __fastcall ExplorerZ80Read(UINT16 a)
{
	switch (a) {
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		case 0x8000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x8001: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x8002: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0x8003: {
			return GalInput[3] | GalDip[3];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall ExplorerZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0x5100 && a <= 0x51ff) {
		INT32 Offset = a - 0x5100;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6802: {
			// coin_count_0_w
			return;
		}
		
		case 0x6803: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0x6804: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x6806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x6807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7000: {
			// watchdog write
			return;
		}
		
		case 0x8000: {
			GalSoundLatch = d;
			return;
		}
		
		case 0x9000: {
			ZetClose();
			ZetOpen(1);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			ZetOpen(0);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall CkongsZ80Read(UINT16 a)
{
	switch (a) {
		case 0x7000:
		case 0x7001:
		case 0x7002:
		case 0x7003: {
			return ppi8255_r(0, a - 0x7000);
		}
		
		case 0x7800:
		case 0x7801:
		case 0x7802:
		case 0x7803: {
			return ppi8255_r(1, a - 0x7800);
		}
		
		case 0xb000: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall CkongsZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x7000:
		case 0x7001:
		case 0x7002:
		case 0x7003: {
			ppi8255_w(0, a - 0x7000, d);
			return;
		}
		
		case 0x7800:
		case 0x7801:
		case 0x7802:
		case 0x7803: {
			ppi8255_w(1, a - 0x7800, d);
			return;
		}
		
		case 0xa801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xa807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb000: {
			// watchdog write
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall MarsZ80Read(UINT16 a)
{
	if (a >= 0x8100 && a <= 0x810f) {
		INT32 Offset = a - 0x8100;
		return ppi8255_r(0, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01));
	}
	
	if (a >= 0x8200 && a <= 0x820f) {
		INT32 Offset = a - 0x8200;
		return ppi8255_r(1, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01));
	}
	
	switch (a) {
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall MarsZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0x8100 && a <= 0x810f) {
		INT32 Offset = a - 0x8100;
		ppi8255_w(0, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01), d);
		return;
	}
	
	if (a >= 0x8200 && a <= 0x820f) {
		INT32 Offset = a - 0x8200;
		ppi8255_w(1, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01), d);
		return;
	}
	
	switch (a) {
		case 0x6800: {
			// coin_count_1_w
			return;
		}
		
		case 0x6801: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x6802: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6808: {
			// coin_count_0_w
			return;
		}
		
		case 0x6809: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x680b: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Newsin7Z80Read(UINT16 a)
{
	if (a >= 0x8200 && a <= 0x820f) {
		INT32 Offset = a - 0x8200;
		return ppi8255_r(1, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01));
	}
	
	if (a >= 0xc100 && a <= 0xc10f) {
		INT32 Offset = a - 0xc100;
		return ppi8255_r(0, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01));
	}
	
	switch (a) {
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall Newsin7Z80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0x8200 && a <= 0x820f) {
		INT32 Offset = a - 0x8200;
		ppi8255_w(1, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01), d);
		return;
	}
	
	if (a >= 0xc100 && a <= 0xc10f) {
		INT32 Offset = a - 0xc100;
		ppi8255_w(0, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01), d);
		return;
	}
	
	switch (a) {
		case 0x6800: {
			// coin_count_1_w
			return;
		}
		
		case 0x6801: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x6802: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6808: {
			// coin_count_0_w
			return;
		}
		
		case 0x6809: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x680b: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall MrkougarZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0x8100 && a <= 0x810f) {
		INT32 Offset = a - 0x8100;
		ppi8255_w(0, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01), d);
		return;
	}
	
	if (a >= 0x8200 && a <= 0x820f) {
		INT32 Offset = a - 0x8200;
		ppi8255_w(1, ((Offset >> 2) & 0x02) | ((Offset >> 1) & 0x01), d);
		return;
	}
	
	switch (a) {
		case 0x6800: {
			// coin_count_1_w
			return;
		}
		
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6808: {
			// coin_count_0_w
			return;
		}
		
		case 0x6809: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x680b: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall HotshockZ80Read(UINT16 a)
{
	switch (a) {
		case 0x8000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x8001: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x8002: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0x8003: {
			return GalInput[3] | GalDip[3];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall HotshockZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000: {
			// coin_count_2_w
			return;
		}
		
		case 0x6002: {
			// coin_count_1_w
			return;
		}
		
		case 0x6004: {
			GalFlipScreenX = d & 1;
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x6005: {
			// coin_count_0_w
			return;
		}
		
		case 0x6006: {
			GalGfxBank[0] = d & 1;
			return;
		}
		
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7000: {
			// watchdog write
			return;
		}
		
		case 0x8000: {
			GalSoundLatch = d;
			return;
		}
		
		case 0x9000: {
			ZetClose();
			ZetOpen(1);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			ZetOpen(0);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall HotshockSoundZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x20: {
			return AY8910Read(0);
		}
		
		case 0x40: {
			return AY8910Read(1);
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Sound Port Read %x\n"), a);
	
	return 0;
}

void __fastcall HotshockSoundZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x10: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x20: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0x40: {
			AY8910Write(1, 1, d);
			return;
		}
		
		case 0x80: {
			AY8910Write(1, 0, d);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Sound Port Write %x, %x\n"), a, d);
}

static void CavelonDoBankSwitch()
{
	CavelonBankSwitch = !CavelonBankSwitch;
	ZetMapArea(0x0000, 0x1fff, 0, GalZ80Rom1 + ((CavelonBankSwitch) ? 0x4000 : 0x0000));
	ZetMapArea(0x0000, 0x1fff, 2, GalZ80Rom1 + ((CavelonBankSwitch) ? 0x4000 : 0x0000));
}

UINT8 __fastcall CavelonZ80Read(UINT16 a)
{
	if (a >= 0x8000) {
		CavelonDoBankSwitch();
		UINT32 Offset = a - 0x8000;
		UINT8 Result = 0xff;
		if (Offset & 0x0100) Result &= ppi8255_r(0, Offset & 3);
		if (Offset & 0x0200) Result &= ppi8255_r(1, Offset & 3);
		return Result;
	}
	
	switch (a) {
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall CavelonZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0x8000) {
		CavelonDoBankSwitch();
		INT32 Offset = a - 0x8000;
		if (Offset & 0x0100) ppi8255_w(0, Offset & 3, d);
		if (Offset & 0x0200) ppi8255_w(1, Offset & 3, d);
		return;
	}
	
	switch (a) {
		case 0x2000:
		case 0x3800:
		case 0x3801: {
			// write in rom area
			return;
		}
		
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6802: {
			// coin_count_0_w
			return;
		}
		
		case 0x6803: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0x6804: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x6806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x6807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall MimonscrZ80Read(UINT16 a)
{
	switch (a) {
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		case 0x8100:
		case 0x8101:
		case 0x8102:
		case 0x8103: {
			return ppi8255_r(0, a - 0x8100);
		}
		
		case 0x8200:
		case 0x8201:
		case 0x8202:
		case 0x8203: {
			return ppi8255_r(1, a - 0x8200);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall MimonscrZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6800: {
			GalGfxBank[0] = d & 1;
			return;
		}
		
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6802: {
			GalGfxBank[1] = d & 1;
			return;
		}
		
		case 0x6803: {
			// ???
			return;
		}
		
		case 0x6804: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0x6805: {
			// ???
			return;
		}
		
		case 0x6806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x6807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x8100:
		case 0x8101:
		case 0x8102:
		case 0x8103: {
			ppi8255_w(0, a - 0x8100, d);
			return;
		}
		
		case 0x8200:
		case 0x8201:
		case 0x8202:
		case 0x8203: {
			ppi8255_w(1, a - 0x8200, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 TheendInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapTheend;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = GalaxianDrawBackground;
	GalDrawBulletsFunction = TheendDrawBullets;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
	
	return nRet;
}

static UINT8 ScramblePPIReadIN2()
{
	UINT8 Val = (ScrambleProtectionResult >> 7) & 1;
	
	if (Val) {
		return 0x5f - GalInput[2] - GalDip[2];
	} else {
		return 0xff - GalInput[2] - GalDip[2];
	}
}

static UINT8 ScrambleProtectionRead()
{
	return ScrambleProtectionResult;
}

static void ScrambleProtectionWrite(UINT8 d)
{
	ScrambleProtectionState = (ScrambleProtectionState << 4) | (d & 0x0f);
	
	switch (ScrambleProtectionState & 0xfff) {
		case 0xf09: ScrambleProtectionResult = 0xff; return;
		case 0xa49: ScrambleProtectionResult = 0xbf; return;
		case 0x319: ScrambleProtectionResult = 0x4f; return;
		case 0x5c9: ScrambleProtectionResult = 0x6f; return;
		
		// scrambles
		case 0x246: ScrambleProtectionResult ^= 0x80; return;
		case 0xb5f: ScrambleProtectionResult = 0x6f; return;
	}
}

static INT32 ScrambleInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapTheend;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	PPI0PortReadC = ScramblePPIReadIN2;
	PPI1PortReadC = ScrambleProtectionRead;
	PPI1PortWriteC = ScrambleProtectionWrite;
	
	return nRet;
}

static void ExplorerPostLoad()
{
	MapTheend();
	
	ZetOpen(0);
	ZetSetReadHandler(ExplorerZ80Read);
	ZetSetWriteHandler(ExplorerZ80Write);
	ZetMapArea(0x5100, 0x51ff, 0, GalSpriteRam);
	ZetMapArea(0x5100, 0x51ff, 2, GalSpriteRam);
	ZetClose();
}

static INT32 ExplorerInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = ExplorerPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	return nRet;
}

static INT32 AtlantisInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapTheend;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	return nRet;
}

static void CkongsPostLoad()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(CkongsZ80Read);
	ZetSetWriteHandler(CkongsZ80Write);
	ZetMapArea(0x0000, 0x5fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x5fff, 2, GalZ80Rom1);
	ZetMapArea(0x6000, 0x6fff, 0, GalZ80Ram1);
	ZetMapArea(0x6000, 0x6fff, 1, GalZ80Ram1);
	ZetMapArea(0x6000, 0x6fff, 2, GalZ80Ram1);
	ZetMapArea(0x9000, 0x93ff, 0, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 1, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 2, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 0, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 1, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 2, GalVideoRam);
	ZetMapArea(0x9800, 0x98ff, 0, GalSpriteRam);
	ZetMapArea(0x9800, 0x98ff, 2, GalSpriteRam);
	ZetClose();
}

static INT32 CkongsInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = CkongsPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	KonamiPPIInit();
	
	GalExtendSpriteInfoFunction = MshuttleExtendSpriteInfo;
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

static void MarsDecrypt()
{
	for (UINT32 i = 0; i < GalZ80Rom1Size; i += 16) {
		UINT8 SwapBuffer[16];
		
		for (UINT32 j = 0; j < 16; j++) {
			INT32 NewOffset = BITSWAP08(j, 7, 6, 5, 4, 2, 0, 3, 1);
			SwapBuffer[j] = GalZ80Rom1[i + NewOffset];
		}
		memcpy(GalZ80Rom1 + i, SwapBuffer, 16);
	}
}

static void MarsPostLoad()
{
	MarsDecrypt();
	
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(MarsZ80Read);
	ZetSetWriteHandler(MarsZ80Write);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x4000, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 2, GalZ80Ram1);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetMapArea(0x5000, 0x50ff, 0, GalSpriteRam);
	ZetMapArea(0x5000, 0x50ff, 2, GalSpriteRam);
	ZetClose();
}

static INT32 MarsInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MarsPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 0;
	
	KonamiPPIInit();
	
	return nRet;
}

static INT32 DevilfshInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MarsPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalNumChars = 0x100;
	GalNumSprites = 0x40;
	CharPlaneOffsets[1] = 0x8000;
	SpritePlaneOffsets[1] = 0x8000;
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom + 0x800, GalSprites);
	BurnFree(GalTempRom);
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 0;
	
	KonamiPPIInit();
	
	return nRet;
}

static void Newsin7PostLoad()
{
	MarsDecrypt();
	
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(Newsin7Z80Read);
	ZetSetWriteHandler(Newsin7Z80Write);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x4000, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 2, GalZ80Ram1);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetMapArea(0x5000, 0x50ff, 0, GalSpriteRam);
	ZetMapArea(0x5000, 0x50ff, 2, GalSpriteRam);
	ZetMapArea(0xa000, 0xafff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0xa000, 0xafff, 2, GalZ80Rom1 + 0x4000);
	ZetClose();
}

static INT32 Newsin7Init()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = Newsin7PostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalNumChars = 0x100;
	GalNumSprites = 0x40;
	
	INT32 Newsin7PlaneOffsets[3] = {0x10000, 0, 0x8000};
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x2000, GAL_ROM_OFFSET_TILES_SHARED + 2, 1); if (nRet) return 1;
	GfxDecode(GalNumChars, 3, 8, 8, Newsin7PlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 3, 16, 16, Newsin7PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom + 0x800, GalSprites);
	BurnFree(GalTempRom);
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalColourDepth = 3;
	GalSpriteClipStart = 0;
	GalSpriteClipEnd = 246;
	
	GalSoundVolumeShift = 0;
	
	KonamiPPIInit();
	
	return nRet;
}

static void MapMrkougar()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(MarsZ80Read);
	ZetSetWriteHandler(MrkougarZ80Write);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x4000, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 2, GalZ80Ram1);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 0, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 1, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 2, GalVideoRam);
	ZetMapArea(0x5000, 0x50ff, 0, GalSpriteRam);
	ZetMapArea(0x5000, 0x50ff, 2, GalSpriteRam);
	ZetClose();
}

static void MrkougarPostLoad()
{
	MarsDecrypt();
	
	MapMrkougar();
}

static INT32 MrkougarInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MrkougarPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	INT32 MrkougarPlaneOffsets[2] = {0, 4};
	INT32 MrkougarCharXOffsets[8] = {64, 65, 66, 67, 0, 1, 2, 3};
	INT32 MrkougarCharYOffsets[8] = {0, 8, 16, 24, 32, 40, 48, 56};
	INT32 MrkougarSpriteXOffsets[16] = {64, 65, 66, 67, 0, 1, 2, 3, 192, 193, 194, 195, 128, 129, 130, 131};
	INT32 MrkougarSpriteYOffsets[16] = {0, 8, 16, 24, 32, 40, 48, 56, 256, 264, 272, 280, 288, 296, 304, 312};
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	GfxDecode(GalNumChars, 2, 8, 8, MrkougarPlaneOffsets, MrkougarCharXOffsets, MrkougarCharYOffsets, 0x80, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, MrkougarPlaneOffsets, MrkougarSpriteXOffsets, MrkougarSpriteYOffsets, 0x200, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	return nRet;
}

static INT32 MrkougbInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapMrkougar;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	return nRet;
}

static void HotshockPostLoad()
{
	MapMrkougar();
	
	ZetOpen(0);
	ZetSetReadHandler(HotshockZ80Read);
	ZetSetWriteHandler(HotshockZ80Write);
	ZetClose();
	
	GalZ80Rom1[0x2ef9] = 0xc9;
}

static INT32 HotshockInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = HotshockPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	ZetOpen(1);
	ZetSetInHandler(HotshockSoundZ80PortRead);
	ZetSetOutHandler(HotshockSoundZ80PortWrite);
	ZetClose();
	
	GalExtendTileInfoFunction = PiscesExtendTileInfo;
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	GalSoundVolumeShift = 0;
	
	return nRet;
}

static void ConquerPostLoad()
{
	MapMrkougar();
	
	ZetOpen(0);
	ZetSetReadHandler(HotshockZ80Read);
	ZetSetWriteHandler(HotshockZ80Write);
	ZetClose();
}

static INT32 ConquerInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = ConquerPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	ZetOpen(1);
	ZetSetInHandler(HotshockSoundZ80PortRead);
	ZetSetOutHandler(HotshockSoundZ80PortWrite);
	ZetClose();
	
	GalExtendTileInfoFunction = PiscesExtendTileInfo;
	
	GalSoundVolumeShift = 0;
	
	return nRet;
}

static void CavelonPostLoad()
{
	MapTheend();
	
	ZetOpen(0);
	ZetSetReadHandler(CavelonZ80Read);
	ZetSetWriteHandler(CavelonZ80Write);
	ZetClose();
}

static INT32 CavelonInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = CavelonPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;	
	GalExtendSpriteInfoFunction = MshuttleExtendSpriteInfo;
	
	GalSoundVolumeShift = 2;
	
	KonamiPPIInit();
	
	return nRet;
}

static void MimonscrPostLoad()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(MimonscrZ80Read);
	ZetSetWriteHandler(MimonscrZ80Write);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x4000, 0x43ff, 0, GalVideoRam);
	ZetMapArea(0x4000, 0x43ff, 1, GalVideoRam);
	ZetMapArea(0x4000, 0x43ff, 2, GalVideoRam);
	ZetMapArea(0x4400, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4400, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4400, 0x47ff, 2, GalZ80Ram1);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetMapArea(0x5000, 0x50ff, 0, GalSpriteRam);
	ZetMapArea(0x5000, 0x50ff, 2, GalSpriteRam);
	ZetMapArea(0xc000, 0xffff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0xc000, 0xffff, 2, GalZ80Rom1 + 0x4000);
	ZetClose();
}

static INT32 MimonscrInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MimonscrPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;	
	
	nRet = GalInit();
	KonamiSoundInit();

	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = MimonkeyExtendTileInfo;
	GalExtendSpriteInfoFunction = MimonkeyExtendSpriteInfo;

	KonamiPPIInit();
	
	return nRet;
}

struct BurnDriver BurnDrvTheend = {
	"theend", NULL, NULL, NULL, "1980",
	"The End\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, TheendRomInfo, TheendRomName, NULL, NULL, TheendInputInfo, TheendDIPInfo,
	TheendInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTheends = {
	"theends", "theend", NULL, NULL, "1980",
	"The End (Stern)\0", NULL, "Konami (Stern license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, TheendsRomInfo, TheendsRomName, NULL, NULL, TheendInputInfo, TheendDIPInfo,
	TheendInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScramble = {
	"scramble", NULL, NULL, NULL, "1981",
	"Scramble\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScrambleRomInfo, ScrambleRomName, NULL, NULL, ScrambleInputInfo, ScrambleDIPInfo,
	ScrambleInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScrambles = {
	"scrambles", "scramble", NULL, NULL, "1981",
	"Scramble (Stern)\0", NULL, "Konami (Stern license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScramblesRomInfo, ScramblesRomName, NULL, NULL, ScrambleInputInfo, ScrambleDIPInfo,
	ScrambleInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScramblebf = {
	"scramblebf", "scramble", NULL, NULL, "1981",
	"Scramble (Karateko, French bootleg)\0", NULL, "Karateko (bootleg)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScramblebfRomInfo, ScramblebfRomName, NULL, NULL, ScrambleInputInfo, ScrambleDIPInfo,
	ScrambleInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScramblebb = {
	"scramblebb", "scramble", NULL, NULL, "1981",
	"Scramble (bootleg?)\0", NULL, "bootleg?", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScramblebbRomInfo, ScramblebbRomName, NULL, NULL, ScrambleInputInfo, ScrambleDIPInfo,
	ScrambleInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvStrfbomb = {
	"strfbomb", "scramble", NULL, NULL, "1981",
	"Strafe Bomb\0", NULL, "Omni", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, StrfbombRomInfo, StrfbombRomName, NULL, NULL, ScrambleInputInfo, StrfbombDIPInfo,
	ScrambleInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvExplorer = {
	"explorer", "scramble", NULL, NULL, "1981",
	"Explorer\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ExplorerRomInfo, ExplorerRomName, NULL, NULL, ExplorerInputInfo, ExplorerDIPInfo,
	ExplorerInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAtlantis = {
	"atlantis", NULL, NULL, NULL, "1981",
	"Battle of Atlantis (set 1)\0", NULL, "Comsoft", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, AtlantisRomInfo, AtlantisRomName, NULL, NULL, AtlantisInputInfo, AtlantisDIPInfo,
	AtlantisInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAtlantis2 = {
	"atlantis2", "atlantis", NULL, NULL, "1981",
	"Battle of Atlantis (set 2)\0", NULL, "Comsoft", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, Atlantis2RomInfo, Atlantis2RomName, NULL, NULL, AtlantisInputInfo, AtlantisDIPInfo,
	AtlantisInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvCkongs = {
	"ckongs", "ckong", NULL, NULL, "1981",
	"Crazy Kong (Scramble hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, CkongsRomInfo, CkongsRomName, NULL, NULL, CkongsInputInfo, CkongsDIPInfo,
	CkongsInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMars = {
	"mars", NULL, NULL, NULL, "1981",
	"Mars\0", NULL, "Artic", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, MarsRomInfo, MarsRomName, NULL, NULL, MarsInputInfo, MarsDIPInfo,
	MarsInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvDevilfsh = {
	"devilfsh", NULL, NULL, NULL, "1982",
	"Devil Fish\0", NULL, "Artic", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, DevilfshRomInfo, DevilfshRomName, NULL, NULL, DevilfshInputInfo, DevilfshDIPInfo,
	DevilfshInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNewsin7 = {
	"newsin7", NULL, NULL, NULL, "1983",
	"New Sinbad 7\0", NULL, "ATW USA, Inc.", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, Newsin7RomInfo, Newsin7RomName, NULL, NULL, DevilfshInputInfo, Newsin7DIPInfo,
	Newsin7Init, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMrkougar = {
	"mrkougar", NULL, NULL, NULL, "1984",
	"Mr. Kougar\0", NULL, "ATW", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, MrkougarRomInfo, MrkougarRomName, NULL, NULL, MrkougarInputInfo, MrkougarDIPInfo,
	MrkougarInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMrkougar2 = {
	"mrkougar2", "mrkougar", NULL, NULL, "1983",
	"Mr. Kougar (earlier)\0", NULL, "ATW", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, Mrkougar2RomInfo, Mrkougar2RomName, NULL, NULL, MrkougarInputInfo, MrkougarDIPInfo,
	MrkougarInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMrkougb = {
	"mrkougb", "mrkougar", NULL, NULL, "1983",
	"Mr. Kougar (bootleg set 1)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, MrkougbRomInfo, MrkougbRomName, NULL, NULL, MrkougarInputInfo, MrkougarDIPInfo,
	MrkougbInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMrkougb2 = {
	"mrkougb2", "mrkougar", NULL, NULL, "1983",
	"Mr. Kougar (bootleg set 2)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, Mrkougb2RomInfo, Mrkougb2RomName, NULL, NULL, MrkougarInputInfo, MrkougarDIPInfo,
	MrkougbInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHotshock = {
	"hotshock", NULL, NULL, NULL, "1982",
	"Hot Shocker\0", NULL, "E.G. Felaco (Domino license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, HotshockRomInfo, HotshockRomName, NULL, NULL, HotshockInputInfo, HotshockDIPInfo,
	HotshockInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHotshockb = {
	"hotshockb", "hotshock", NULL, NULL, "1982",
	"Hot Shocker (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, HotshockbRomInfo, HotshockbRomName, NULL, NULL, HotshockInputInfo, HotshockDIPInfo,
	HotshockInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriverD BurnDrvConquer = {
	"conquer", NULL, NULL, NULL, "1982",
	"Conquer\0", "Bad dump", "unknown", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, ConquerRomInfo, ConquerRomName, NULL, NULL, HotshockInputInfo, HotshockDIPInfo,
	ConquerInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvCavelon = {
	"cavelon", NULL, NULL, NULL, "1983",
	"Cavelon\0", NULL, "Jetsoft", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, CavelonRomInfo, CavelonRomName, NULL, NULL, AtlantisInputInfo, CavelonDIPInfo,
	CavelonInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMimonscr = {
	"mimonscr", "mimonkey", NULL, NULL, "198?",
	"Mighty Monkey (bootleg on Scramble hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, MimonscrRomInfo, MimonscrRomName, NULL, NULL, AtlantisInputInfo, MimonkeyDIPInfo,
	MimonscrInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Scramble based hardware with one CPU and one AY8910
static struct BurnRomInfo TriplepRomDesc[] = {
	{ "triplep.2g",    0x01000, 0xc583a93d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "triplep.2h",    0x01000, 0xc03ddc49, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "triplep.2k",    0x01000, 0xe83ca6b5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "triplep.2l",    0x01000, 0x982cc3b9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "triplep.5f",    0x00800, 0xd51cbd6f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "triplep.5h",    0x00800, 0xf21c0059, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "tripprom.6e",   0x00020, 0x624f75df, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Triplep)
STD_ROM_FN(Triplep)

static struct BurnRomInfo KnockoutRomDesc[] = {
	{ "knockout.2h",   0x01000, 0xeaaa848e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "knockout.2k",   0x01000, 0xbc26d2c0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "knockout.2l",   0x01000, 0x02025c10, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "knockout.2m",   0x01000, 0xe9abc42b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "triplep.5f",    0x00800, 0xd51cbd6f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "triplep.5h",    0x00800, 0xf21c0059, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "tripprom.6e",   0x00020, 0x624f75df, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Knockout)
STD_ROM_FN(Knockout)

static struct BurnRomInfo MarinerRomDesc[] = {
	{ "tp1.2h",        0x01000, 0xdac1dfd0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tm2.2k",        0x01000, 0xefe7ca28, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tm3.2l",        0x01000, 0x027881a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tm4.2m",        0x01000, 0xa0fde7dc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tm5.2p",        0x01000, 0xd7ebcb8e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "tm8.5f",        0x01000, 0x70ae611f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "tm9.5h",        0x01000, 0x8e4e999e, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "t4.6e",         0x00020, 0xca42b6dd, BRF_GRA | GAL_ROM_PROM },	
	{ "t6.6p",         0x00100, 0xad208ccc, BRF_GRA | GAL_ROM_PROM },	// Background colour
	{ "t5.7p",         0x00020, 0x1bd88cff, BRF_GRA | GAL_ROM_PROM },	// Char banking and star placement
};

STD_ROM_PICK(Mariner)
STD_ROM_FN(Mariner)

static struct BurnRomInfo EighthundredfathRomDesc[] = {
	{ "tu1.2h",        0x01000, 0x5dd3d42f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tm2.2k",        0x01000, 0xefe7ca28, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tm3.2l",        0x01000, 0x027881a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tm4.2m",        0x01000, 0xa0fde7dc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "tu5.2p",        0x01000, 0xf864a8a6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "tm8.5f",        0x01000, 0x70ae611f, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "tm9.5h",        0x01000, 0x8e4e999e, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "t4.6e",         0x00020, 0xca42b6dd, BRF_GRA | GAL_ROM_PROM },	
	{ "t6.6p",         0x00100, 0xad208ccc, BRF_GRA | GAL_ROM_PROM },	// Background colour
	{ "t5.7p",         0x00020, 0x1bd88cff, BRF_GRA | GAL_ROM_PROM },	// Char banking and star placement
};

STD_ROM_PICK(Eighthundredfath)
STD_ROM_FN(Eighthundredfath)

UINT8 __fastcall TriplepZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			// ???
			return 0;
		}
		
		case 0x01: {
			return AY8910Read(0);
		}
		
		case 0x02: {
			if (ZetPc(-1) == 0x015a) return 0xff;
			if (ZetPc(-1) == 0x0886) return 0x05;
			return 0;
		}
		
		case 0x03: {
			if (ZetPc(-1) == 0x015d) return 0x04;
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall TriplepZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0x01: {
			AY8910Write(0, 0, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall MarinerZ80Read(UINT16 a)
{
	if (a >= 0x8000) {
		if (a == 0xb401) return 0x07;
		if (a == 0x9008) return 0x03;
		
		UINT32 Offset = a - 0x8000;
		UINT8 Result = 0xff;
		if (Offset & 0x0100) Result &= ppi8255_r(0, Offset & 3);
		if (Offset & 0x0200) Result &= ppi8255_r(1, Offset & 3);
		return Result;
	}
	
	switch (a) {
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

static void TriplepPostLoad()
{
	MapTheend();
	
	ZetOpen(0);
	ZetSetInHandler(TriplepZ80PortRead);
	ZetSetOutHandler(TriplepZ80PortWrite);
	ZetClose();
}

static INT32 TriplepInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = TriplepPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_ZIGZAGAY8910;
	
	nRet = GalInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	PPI1PortWriteA = NULL;
	PPI1PortWriteB = NULL;
	
	return nRet;
}

static void MarinerPostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(0x1000);
	memcpy(GalTempRom, GalZ80Rom1 + 0x4000, 0x1000);
	memcpy(GalZ80Rom1 + 0x4800, GalTempRom + 0x0000, 0x800);
	memcpy(GalZ80Rom1 + 0x4000, GalTempRom + 0x0800, 0x800);
	BurnFree(GalTempRom);
	
	MapTheend();
	
	ZetOpen(0);
	ZetSetReadHandler(MarinerZ80Read);
	ZetSetInHandler(TriplepZ80PortRead);
	ZetSetOutHandler(TriplepZ80PortWrite);
	ZetMapArea(0x5800, 0x67ff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0x5800, 0x67ff, 2, GalZ80Rom1 + 0x4000);
	ZetClose();
}

static INT32 MarinerInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MarinerPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_ZIGZAGAY8910;
	
	nRet = GalInit();
	
	GalCalcPaletteFunction = MarinerCalcPalette;
	GalRenderBackgroundFunction = MarinerDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = MarinerExtendTileInfo;
	
	KonamiPPIInit();
	PPI1PortWriteA = NULL;
	PPI1PortWriteB = NULL;
	
	return nRet;
}

struct BurnDriver BurnDrvTriplep = {
	"triplep", NULL, NULL, NULL, "1982",
	"Triple Punch\0", NULL, "KKI", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, TriplepRomInfo, TriplepRomName, NULL, NULL, AmidarInputInfo, TriplepDIPInfo,
	TriplepInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvKnockout = {
	"knockout", "triplep", NULL, NULL, "1982",
	"Knock Out!!\0", NULL, "KKK", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, KnockoutRomInfo, KnockoutRomName, NULL, NULL, AmidarInputInfo, TriplepDIPInfo,
	TriplepInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMariner = {
	"mariner", NULL, NULL, NULL, "1981",
	"Mariner\0", NULL, "Armenip", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, MarinerRomInfo, MarinerRomName, NULL, NULL, ScrambleInputInfo, ScrambleDIPInfo,
	MarinerInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvEighthundredfath = {
	"800fath", "mariner", NULL, NULL, "1981",
	"800 Fathoms\0", NULL, "Amenip (US Billiards Inc. license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE |BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, EighthundredfathRomInfo, EighthundredfathRomName, NULL, NULL, ScrambleInputInfo, ScrambleDIPInfo,
	MarinerInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Scorpion hardware - based on Scramble but with a third AY8910 and a speech chip
static struct BurnRomInfo ScorpionRomDesc[] = {
	{ "1.2d",          0x01000, 0xba1219b4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2.2f",          0x01000, 0xc3909ab6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "3.2g",          0x01000, 0x43261352, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "4.2h",          0x01000, 0xaba2276a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.2k",          0x01000, 0x952f78f2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "32_a4.7c",      0x01000, 0x361b8a36, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "32_a5.7d",      0x01000, 0xaddecdd4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "32_f5.5f",      0x01000, 0x1e5da9d6, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "32_h5.5h",      0x01000, 0xa57adb0a, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "prom.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
	
	{ "32_a3.6e",      0x01000, 0x279ae6f9, BRF_OPT | BRF_SND }, // Speech samples?
	{ "32_a2.6d",      0x01000, 0x90352dd4, BRF_OPT | BRF_SND },
	{ "32_a1.6c",      0x01000, 0x3bf2452d, BRF_OPT | BRF_SND },
};

STD_ROM_PICK(Scorpion)
STD_ROM_FN(Scorpion)

static struct BurnRomInfo ScorpionaRomDesc[] = {
	{ "scor_d2.bin",   0x01000, 0xc5b9daeb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scor_e2.bin",   0x01000, 0x82308d05, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scor_g2.bin",   0x01000, 0x756b09cd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scor_h2.bin",   0x01000, 0xa0457b93, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scor_l2.bin",   0x00800, 0x6623da33, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scor_k2.bin",   0x00800, 0x42ec34d8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },	
		
	{ "32_a4.7c",      0x01000, 0x361b8a36, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "32_a5.7d",      0x01000, 0xaddecdd4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "scor_f5.bin",   0x01000, 0x60180a38, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "32_h5.5h",      0x01000, 0xa57adb0a, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "prom.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
	
	{ "scor_a3.bin",   0x01000, 0x04abf178, BRF_OPT | BRF_SND }, // Speech samples?
	{ "scor_a2.bin",   0x01000, 0x452d6354, BRF_OPT | BRF_SND },
	{ "32_a1.6c",      0x01000, 0x3bf2452d, BRF_OPT | BRF_SND },
};

STD_ROM_PICK(Scorpiona)
STD_ROM_FN(Scorpiona)

static struct BurnRomInfo ScorpionbRomDesc[] = {
	{ "ic109.2d",      0x01000, 0xf54688c9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic110.2e",      0x01000, 0x91aaaa12, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic111.2g",      0x01000, 0x4c3720da, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic112.2h",      0x01000, 0x53e2a983, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ic113.2k",      0x01000, 0xe4ad299a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "32_a4.7c",      0x01000, 0x361b8a36, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ic13.7d",       0x01000, 0xaddecdd4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "ic72.5f",       0x01000, 0x1e5da9d6, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ic73.5h",       0x01000, 0xa57adb0a, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "prom.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
	
	{ "ic25.6e",       0x01000, 0x04abf178, BRF_OPT | BRF_SND }, // Speech samples?
	{ "ic24.6d",       0x01000, 0x90352dd4, BRF_OPT | BRF_SND },
	{ "ic23.6c",       0x01000, 0x3bf2452d, BRF_OPT | BRF_SND },
};

STD_ROM_PICK(Scorpionb)
STD_ROM_FN(Scorpionb)

static struct BurnRomInfo ScorpionmcRomDesc[] = {
	{ "p1.bin",        0x00800, 0x58818d88, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p2.bin",        0x00800, 0x8bec5f9f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p3.bin",        0x00800, 0x24b7fdff, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p4.bin",        0x00800, 0x9082e2f0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p5.bin",        0x00800, 0x20387fc0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p6.bin",        0x00800, 0xf66c48e1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p7.bin",        0x00800, 0x931e34c7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p8.bin",        0x00800, 0xab5ab61d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p9.bin",        0x01000, 0xb551b974, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "p10.bin",       0x00800, 0xa7bd8d20, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "h.bin",         0x01000, 0x1e5da9d6, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "k.bin",         0x01000, 0xa57adb0a, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "mmi6331.bpr",   0x00020, 0x6a0c7d87, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scorpionmc)
STD_ROM_FN(Scorpionmc)

static struct BurnRomInfo AracnisRomDesc[] = {
	{ "00sc.bin",      0x01000, 0xc7e0d6b9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "01sc.bin",      0x01000, 0x03eb27dc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "02sc.bin",      0x01000, 0xf3d49d4f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "03sc.bin",      0x01000, 0x0e741984, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "05sc.bin",      0x01000, 0xf27ee3e4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "06sc.bin",      0x00800, 0xfdfc2c82, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "08sc.1h",       0x01000, 0x1e5da9d6, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "07sc.1k",       0x01000, 0xa57adb0a, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "mmi6331-1.6l",  0x00020, 0x24652bc4, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Aracnis)
STD_ROM_FN(Aracnis)

UINT8 __fastcall ScorpionmcZ80Read(UINT16 a)
{
	switch (a) {
		case 0xa000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0xa800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0xb001: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0xb002: {
			return GalInput[3] | GalDip[3];
		}
		
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall ScorpionmcZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa004:
		case 0xa005:
		case 0xa006:
		case 0xa007: {
			GalaxianLfoFreqWrite(a - 0xa004, d);
			return;
		}
		
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall ScorpionSoundZ80Read(UINT16 a)
{
	switch (a) {
		case 0x3000: {
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

UINT8 __fastcall ScorpionSoundZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	UINT8 Result = 0xff;
	if (a & 0x08) Result &= AY8910Read(2);
	if (a & 0x20) Result &= AY8910Read(0);
	if (a & 0x80) Result &= AY8910Read(1);
	return Result;
}

void __fastcall ScorpionSoundZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	if (a & 0x04) AY8910Write(2, 0, d);
	if (a & 0x08) AY8910Write(2, 1, d);
	if (a & 0x10) AY8910Write(0, 0, d);
	if (a & 0x20) AY8910Write(0, 1, d);
	if (a & 0x40) AY8910Write(1, 0, d);
	if (a & 0x80) AY8910Write(1, 1, d);
}

static UINT8 ScorpionProtectionRead()
{
	UINT16 ParityBits;
	UINT8 Parity = 0;
	
	for (ParityBits = ScrambleProtectionState & 0xce29; ParityBits != 0; ParityBits >>= 1) {
		if (ParityBits & 1) Parity++;
	}
	
	return Parity;
}

static void ScorpionProtectionWrite(UINT8 d)
{
	if (!(d & 0x20)) ScrambleProtectionState = 0x0000;
	if (!(d & 0x10)) ScrambleProtectionState = (ScrambleProtectionState << 1) | (~ScorpionProtectionRead() & 1);
}

static void ScorpionPostLoad()
{	
	GalTempRom = (UINT8*)BurnMalloc(0x1000);
	memcpy(GalTempRom, GalZ80Rom1 + 0x4000, 0x1000);
	memcpy(GalZ80Rom1 + 0x4800, GalTempRom + 0x0000, 0x800);
	memcpy(GalZ80Rom1 + 0x4000, GalTempRom + 0x0800, 0x800);
	BurnFree(GalTempRom);
	
	MapTheend();
	
	ZetOpen(0);
	ZetMapArea(0x5800, 0x67ff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0x5800, 0x67ff, 2, GalZ80Rom1 + 0x4000);
	ZetClose();
}

static INT32 ScorpionInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = ScorpionPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	ZetOpen(1);
	ZetSetReadHandler(ScorpionSoundZ80Read);
	ZetSetInHandler(ScorpionSoundZ80PortRead);
	ZetSetOutHandler(ScorpionSoundZ80PortWrite);
	ZetClose();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = Batman2ExtendTileInfo;
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;
	
	KonamiPPIInit();
	PPI1PortReadC = ScorpionProtectionRead;
	PPI1PortWriteC = ScorpionProtectionWrite;
	
	return nRet;
}

static void ScorpionmcPostLoad()
{
	MapMooncrst();
	
	ZetOpen(0);
	ZetSetReadHandler(ScorpionmcZ80Read);
	ZetSetWriteHandler(ScorpionmcZ80Write);
	ZetMapArea(0x4000, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 2, GalZ80Ram1);
	ZetMapArea(0x5000, 0x67ff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0x5000, 0x67ff, 2, GalZ80Rom1 + 0x4000);
	ZetMapArea(0x9400, 0x97ff, 0, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 1, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 2, GalVideoRam);
	ZetClose();
}

static INT32 ScorpionmcInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = ScorpionmcPostLoad;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = Batman2ExtendTileInfo;
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;

	return nRet;
}

struct BurnDriver BurnDrvScorpion = {
	"scorpion", NULL, NULL, NULL, "1982",
	"Scorpion (set 1)\0", "Incomplete Sound", "Zaccaria", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScorpionRomInfo, ScorpionRomName, NULL, NULL, AtlantisInputInfo, ScorpionDIPInfo,
	ScorpionInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScorpiona = {
	"scorpiona", "scorpion", NULL, NULL, "1982",
	"Scorpion (set 2)\0", "Incomplete Sound", "Zaccaria", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScorpionaRomInfo, ScorpionaRomName, NULL, NULL, AtlantisInputInfo, ScorpionDIPInfo,
	ScorpionInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScorpionb = {
	"scorpionb", "scorpion", NULL, NULL, "1982",
	"Scorpion (set 3)\0", "Incomplete Sound", "Zaccaria", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScorpionbRomInfo, ScorpionbRomName, NULL, NULL, AtlantisInputInfo, ScorpionDIPInfo,
	ScorpionInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScorpionmc = {
	"scorpionmc", "scorpion", NULL, NULL, "19??",
	"Scorpion (Moon Cresta hardware)\0", NULL, "Dorneer", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScorpionmcRomInfo, ScorpionmcRomName, NULL, NULL, ScorpionmcInputInfo, ScorpionmcDIPInfo,
	ScorpionmcInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAracnis = {
	"aracnis", "scorpion", NULL, NULL, "19??",
	"Aracnis (bootleg of Scorpion on Moon Cresta hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, AracnisRomInfo, AracnisRomName, NULL, NULL, AracnisInputInfo, AracnisDIPInfo,
	ScorpionmcInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// A.D. 2083 hardware - based on Scramble with TMS5100A for speech
static struct BurnRomInfo Ad2083RomDesc[] = {
	{ "ad0.10o",       0x02000, 0x4d34325a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ad1.9o",        0x02000, 0x0f37134b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ad2.8o",        0x02000, 0xbcfa655f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ad3.7o",        0x02000, 0x60655225, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
		
	{ "ad1s.3d",       0x02000, 0x80f39b0f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ad2s.4d",       0x01000, 0x5177fe2b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },

	{ "ad4.5k",        0x02000, 0x388cdd21, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ad5.3k",        0x02000, 0xf53f3449, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "prom-am27s19dc.1m", 0x00020, 0x2759aebd, BRF_GRA | GAL_ROM_PROM },
	
	{ "ad1v.9a",       0x01000, 0x4cb93fff, BRF_OPT | BRF_SND }, // TMS5110A data
	{ "ad2v.10a",      0x01000, 0x4b530ea7, BRF_OPT | BRF_SND }, // TMS5110A data
	{ "prom-sn74s188.8a", 0x00020, 0xc58a4f6a, BRF_OPT | BRF_SND }, // TMS5110A data
};

STD_ROM_PICK(Ad2083)
STD_ROM_FN(Ad2083)

UINT8 __fastcall Ad2083Z80Read(UINT16 a)
{
	switch (a) {
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		case 0x8000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x8001: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x8002: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0x8003: {
			return GalInput[3] | GalDip[3];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall Ad2083Z80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6004: {
			GalFlipScreenX = d & 1;
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x6800: {
			// coin_counter_2_w
			return;
		}
		
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6802: {
			// coin_counter_0_w
			return;
		}
		
		case 0x6803: {
			GalBackgroundBlue = d & 1;
			return;
		}
		
		case 0x6805: {
			// coin_counter_1_w
			return;
		}
		
		case 0x6806: {
			GalBackgroundRed = d & 1;
			return;
		}
		
		case 0x6807: {
			GalBackgroundGreen = d & 1;
			return;
		}
		
		case 0x8000: {
			GalSoundLatch = d;
			return;
		}
		
		case 0x9000: {
			ZetClose();
			ZetOpen(1);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			ZetOpen(0);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void Ad2083PostLoad()
{
	UINT8 c;
	
	for (UINT32 i = 0; i < GalZ80Rom1Size; i++) {
		c = GalZ80Rom1[i] ^ 0x35;
		c = BITSWAP08(c, 6, 2, 5, 1, 7, 3, 4, 0);
		GalZ80Rom1[i] = c;
	}
	
	MapTheend();
	
	ZetOpen(0);
	ZetSetReadHandler(Ad2083Z80Read);
	ZetSetWriteHandler(Ad2083Z80Write);
	ZetMapArea(0xa000, 0xdfff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0xa000, 0xdfff, 2, GalZ80Rom1 + 0x4000);
	ZetMapArea(0xe800, 0xebff, 0, GalZ80Ram1 + 0x800);
	ZetMapArea(0xe800, 0xebff, 1, GalZ80Ram1 + 0x800);
	ZetMapArea(0xe800, 0xebff, 2, GalZ80Ram1 + 0x800);
	ZetClose();
}

static INT32 Ad2083Init()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = Ad2083PostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_AD2083AY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	ZetOpen(1);
	ZetSetInHandler(HotshockSoundZ80PortRead);
	ZetSetOutHandler(HotshockSoundZ80PortWrite);
	ZetClose();
	
	GalRenderBackgroundFunction = TurtlesDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = Ad2083ExtendTileInfo;
	GalExtendSpriteInfoFunction = Ad2083ExtendSpriteInfo;
	
	GalSoundVolumeShift = 0;
	
	return nRet;
}

struct BurnDriver BurnDrvAd2083 = {
	"ad2083", NULL, NULL, NULL, "1983",
	"A.D. 2083\0", "Incomplete Sound", "Midcoin", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, Ad2083RomInfo, Ad2083RomName, NULL, NULL, Ad2083InputInfo, Ad2083DIPInfo,
	Ad2083Init, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// SF-X hardware - based on Scramble with extra Z80 and 8255 driving a DAC based sample player
static struct BurnRomInfo SfxRomDesc[] = {
	{ "sfx_b-0.1j",    0x02000, 0xe5bc6952, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "1.1c",          0x01000, 0x1b3c48e7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "22.1d",         0x01000, 0xed44950d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "23.1e",         0x01000, 0xf44a3ca0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "27.1a",         0x01000, 0xed86839f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "24.1g",         0x01000, 0xe6d7dc74, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "5.1h",          0x01000, 0xd1e8d390, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "5.5j",          0x01000, 0x59028fb6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "6.6j",          0x01000, 0x5427670f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "1.1j",          0x01000, 0x2f172c58, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "2.2j",          0x01000, 0xa6ad2f6b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "3.3j",          0x01000, 0xfa1274fa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "4.4j",          0x01000, 0x1cd33f3a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "10.3h",         0x01000, 0xb833a15b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "11.4h",         0x01000, 0xcbd76ec2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
		
	{ "28.5a",         0x01000, 0xd73a8252, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "29.5c",         0x01000, 0x1401ccf2, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "6331.9g",       0x00020, 0xca1d9ccd, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Sfx)
STD_ROM_FN(Sfx)

static struct BurnRomInfo SkelagonRomDesc[] = {
	{ "31.bin",        0x01000, 0xae6f8647, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "32.bin",        0x01000, 0xa28c5838, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "33.bin",        0x01000, 0x32f7e99c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "37.bin",        0x01000, 0x47f68a31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "24.bin",        0x01000, 0xe6d7dc74, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "35.bin",        0x01000, 0x5b2a0158, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "36.bin",        0x01000, 0xf53ead29, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },	
	
	{ "5.5j",          0x01000, 0x59028fb6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "6.6j",          0x01000, 0x5427670f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "1.1j",          0x01000, 0x2f172c58, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "2.2j",          0x01000, 0xa6ad2f6b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "3.3j",          0x01000, 0xfa1274fa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "4.4j",          0x01000, 0x1cd33f3a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "10.bin",        0x01000, 0x2c719de2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "8.bin",         0x01000, 0x350379dd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
		
	{ "38.bin",        0x01000, 0x2fffa8b1, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "39.bin",        0x01000, 0xa854b5de, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "6331.9g",       0x00020, 0xca1d9ccd, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Skelagon)
STD_ROM_FN(Skelagon)

static struct BurnRomInfo MonsterzRomDesc[] = {
	{ "b-1e.a1",       0x01000, 0x97886542, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b-2e.c1",       0x01000, 0x184ffcb4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b-3e.d1",       0x01000, 0xb7b10ac7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b-4e.e1",       0x01000, 0xfb02c736, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b-5e.g1",       0x01000, 0xb2788ab9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b-6e.h1",       0x01000, 0x77d7aa8d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },	
	
	{ "a-1e.k1",       0x01000, 0xb88ba44e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "a-2.k2",        0x01000, 0x8913c94e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "a-3e.k3",       0x01000, 0xa8fa5095, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "a-4.k4",        0x01000, 0x93f81317, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	
	{ "a-5e.k5",       0x01000, 0xb5bcdb4e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "a-6.k6",        0x01000, 0x24832b2e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "a-7e.k7",       0x01000, 0x20ebea81, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "a-8.k8",        0x01000, 0xb833a15b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
	{ "a-9.k9",        0x01000, 0xcbd76ec2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG3 },
		
	{ "b-7e.a5",       0x01000, 0xddd4158d, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "b-8e.c5",       0x01000, 0xb1331b4c, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "prom.g9",       0x00020, 0xb7ea00d7, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Monsterz)
STD_ROM_FN(Monsterz)

void __fastcall SfxZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0x8000) {
		INT32 Offset = a - 0x8000;
		if (Offset & 0x0100) ppi8255_w(0, Offset & 3, d);
		if (Offset & 0x0200) ppi8255_w(1, Offset & 3, d);
		return;
	}
	
	switch (a) {
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6802: {
			// coin_count_0_w
			return;
		}
		
		case 0x6800: {
			GalBackgroundRed = d & 1;
			return;
		}
		
		case 0x6803: {
			GalBackgroundBlue = d & 1;
			return;
		}
		
		case 0x6804: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x6805: {
			GalBackgroundGreen = d & 1;
			return;
		}
		
		case 0x6806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x6807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall SfxSampleZ80Read(UINT16 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall SfxSampleZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall SfxSampleZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	UINT8 Result = 0xff;
	if (a & 0x04) Result &= ppi8255_r(2, a & 3);
	return Result;
}

void __fastcall SfxSampleZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	if (a & 0x04) ppi8255_w(2, a & 3, d);
	if (a & 0x10) DACSignedWrite(0, d);
}

static UINT8 SfxSoundLatch2Read()
{
	return GalSoundLatch2;
}

static void SfxPostLoad()
{
	MapTheend();
	
	ZetOpen(0);
	ZetSetWriteHandler(SfxZ80Write);
	ZetMapArea(0x7000, 0x7fff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0x7000, 0x7fff, 2, GalZ80Rom1 + 0x4000);
	ZetMapArea(0xc000, 0xefff, 0, GalZ80Rom1 + 0x5000);
	ZetMapArea(0xc000, 0xefff, 2, GalZ80Rom1 + 0x5000);
	ZetClose();
	
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	memcpy(GalTempRom, GalZ80Rom1, GalZ80Rom1Size);
	memcpy(GalZ80Rom1 + 0x0000, GalTempRom + 0x0000, 0x1000);
	memcpy(GalZ80Rom1 + 0x7000, GalTempRom + 0x1000, 0x1000);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x2000, 0x6000);
	BurnFree(GalTempRom);
	
	ZetOpen(2);
	ZetSetReadHandler(SfxSampleZ80Read);
	ZetSetWriteHandler(SfxSampleZ80Write);
	ZetSetInHandler(SfxSampleZ80PortRead);
	ZetSetOutHandler(SfxSampleZ80PortWrite);
	ZetMapArea(0x0000, GalZ80Rom3Size - 1, 0, GalZ80Rom3);
	ZetMapArea(0x0000, GalZ80Rom3Size - 1, 2, GalZ80Rom3);
	ZetMapArea(0x8000, 0x83ff, 0, GalZ80Ram3);
	ZetMapArea(0x8000, 0x83ff, 1, GalZ80Ram3);
	ZetMapArea(0x8000, 0x83ff, 2, GalZ80Ram3);
	ZetClose();
	
	nGalCyclesTotal[2] = (14318000 / 8) / 60;
}

static INT32 SfxInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = SfxPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = UpperExtendTileInfo;
	
	ppi8255_init(3);
	PPI0PortReadA = KonamiPPIReadIN0;
	PPI0PortReadB = KonamiPPIReadIN1;
	PPI0PortReadC = KonamiPPIReadIN2;
	PPI1PortReadC = KonamiPPIReadIN3;
	PPI1PortWriteA = KonamiSoundLatchWrite;
	PPI1PortWriteB = KonamiSoundControlWrite;
	PPI2PortReadA = SfxSoundLatch2Read;
	
	SfxTilemap = 1;
	GalOrientationFlipX = 1;
	
	GalSoundVolumeShift = 3;

	return nRet;
}

static void SkelagonPostLoad()
{
	MapTheend();
	
	ZetOpen(0);
	ZetSetWriteHandler(SfxZ80Write);
	ZetMapArea(0x7000, 0x7fff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0x7000, 0x7fff, 2, GalZ80Rom1 + 0x4000);
	ZetMapArea(0xc000, 0xefff, 0, GalZ80Rom1 + 0x5000);
	ZetMapArea(0xc000, 0xefff, 2, GalZ80Rom1 + 0x5000);
	ZetClose();
	
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom1Size);
	memcpy(GalTempRom, GalZ80Rom1, GalZ80Rom1Size);
	memset(GalZ80Rom1, 0xff, 0x1000);
	memcpy(GalZ80Rom1 + 0x1000, GalTempRom + 0x0000, 0x7000);
	BurnFree(GalTempRom);
	
	ZetOpen(2);
	ZetSetReadHandler(SfxSampleZ80Read);
	ZetSetWriteHandler(SfxSampleZ80Write);
	ZetSetInHandler(SfxSampleZ80PortRead);
	ZetSetOutHandler(SfxSampleZ80PortWrite);
	ZetMapArea(0x0000, GalZ80Rom3Size - 1, 0, GalZ80Rom3);
	ZetMapArea(0x0000, GalZ80Rom3Size - 1, 2, GalZ80Rom3);
	ZetMapArea(0x8000, 0x83ff, 0, GalZ80Ram3);
	ZetMapArea(0x8000, 0x83ff, 1, GalZ80Ram3);
	ZetMapArea(0x8000, 0x83ff, 2, GalZ80Ram3);
	ZetClose();
	
	nGalCyclesTotal[2] = (14318000 / 8) / 60;
}

static INT32 SkelagonInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = SkelagonPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC;
	GalZ80Rom1Size = 0x1000;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = UpperExtendTileInfo;
	
	ppi8255_init(3);
	PPI0PortReadA = KonamiPPIReadIN0;
	PPI0PortReadB = KonamiPPIReadIN1;
	PPI0PortReadC = KonamiPPIReadIN2;
	PPI1PortReadC = KonamiPPIReadIN3;
	PPI1PortWriteA = KonamiSoundLatchWrite;
	PPI1PortWriteB = KonamiSoundControlWrite;
	PPI2PortReadA = SfxSoundLatch2Read;
	
	SfxTilemap = 1;
	GalOrientationFlipX = 1;

	return nRet;
}

static void MonsterzPostLoad()
{
	MapTheend();
	
	ZetOpen(0);
	ZetSetWriteHandler(SfxZ80Write);
	ZetMapArea(0xc000, 0xdfff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0xc000, 0xdfff, 2, GalZ80Rom1 + 0x4000);
	ZetClose();
	
	ZetOpen(2);
	ZetSetReadHandler(SfxSampleZ80Read);
	ZetSetWriteHandler(SfxSampleZ80Write);
	ZetSetInHandler(SfxSampleZ80PortRead);
	ZetSetOutHandler(SfxSampleZ80PortWrite);
	ZetMapArea(0x0000, GalZ80Rom3Size - 1, 0, GalZ80Rom3);
	ZetMapArea(0x0000, GalZ80Rom3Size - 1, 2, GalZ80Rom3);
	ZetMapArea(0x8000, 0x83ff, 0, GalZ80Ram3);
	ZetMapArea(0x8000, 0x83ff, 1, GalZ80Ram3);
	ZetMapArea(0x8000, 0x83ff, 2, GalZ80Ram3);
	ZetClose();
	
	GalTempRom = (UINT8*)BurnMalloc(GalZ80Rom3Size);
	memcpy(GalTempRom, GalZ80Rom3, GalZ80Rom3Size);
	memcpy(GalZ80Rom3 + 0x0000, GalTempRom + 0x0000, 0x3000);
	memset(GalZ80Rom3 + 0x3000, 0xff, 0x1000);
	memcpy(GalZ80Rom3 + 0x4000, GalTempRom + 0x3000, 0x2000);
	BurnFree(GalTempRom);
	
	nGalCyclesTotal[2] = (14318000 / 8) / 60;
}

static INT32 MonsterzInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MonsterzPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC;
	GalZ80Rom3Size = 0x1000;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = UpperExtendTileInfo;
	
	ppi8255_init(3);
	PPI0PortReadA = KonamiPPIReadIN0;
	PPI0PortReadB = KonamiPPIReadIN1;
	PPI0PortReadC = KonamiPPIReadIN2;
	PPI1PortReadC = KonamiPPIReadIN3;
	PPI1PortWriteA = KonamiSoundLatchWrite;
	PPI1PortWriteB = KonamiSoundControlWrite;
	PPI2PortReadA = SfxSoundLatch2Read;
	
	SfxTilemap = 1;
	GalOrientationFlipX = 1;

	return nRet;
}

struct BurnDriver BurnDrvSfx = {
	"sfx", NULL, NULL, NULL, "1983",
	"SF-X\0", "Incomplete Sound", "Nichibutsu", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, SfxRomInfo, SfxRomName, NULL, NULL, SfxInputInfo, SfxDIPInfo,
	SfxInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriverD BurnDrvSkelagon = {
	"skelagon", "sfx", NULL, NULL, "1983",
	"Skelagon\0", "Bad Dump", "Nichibutsu USA", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, SkelagonRomInfo, SkelagonRomName, NULL, NULL, SfxInputInfo, SfxDIPInfo,
	SkelagonInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriverD BurnDrvMonsterz = {
	"monsterz", NULL, NULL, NULL, "19??",
	"Monster Zero\0", "Protected", "Nihon", "Galaxian",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, MonsterzRomInfo, MonsterzRomName, NULL, NULL, SfxInputInfo, SfxDIPInfo,
	MonsterzInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

// Super Cobra based hardware
static struct BurnRomInfo ScobraRomDesc[] = {
	{ "epr1265.2c",    0x01000, 0xa0744b3f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e",            0x01000, 0x8e7245cd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr1267.2f",    0x01000, 0x47a4e6fb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h",            0x01000, 0x7244f21c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr1269.2j",    0x01000, 0xe1f8a801, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l",            0x01000, 0xd52affde, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "5c",            0x00800, 0xd4346959, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "5d",            0x00800, 0xcc025d95, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "5e",            0x00800, 0x1628c53f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
		
	{ "epr1274.5h",    0x00800, 0x64d113b4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr1273.5f",    0x00800, 0xa96316d3, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.6e",     0x00020, 0x9b87f90d, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scobra)
STD_ROM_FN(Scobra)

static struct BurnRomInfo ScobrasRomDesc[] = {
	{ "scobra2c.bin",  0x01000, 0xe15ade38, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2e.bin",  0x01000, 0xa270e44d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2f.bin",  0x01000, 0xbdd70346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2h.bin",  0x01000, 0xdca5ec31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2j.bin",  0x01000, 0x0d8f6b6e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2l.bin",  0x01000, 0x6f80f3a9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr1275.5c",    0x00800, 0xdeeb0dd3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr1276.5d",    0x00800, 0x872c1a74, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr1277.5e",    0x00800, 0xccd7a110, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
		
	{ "epr1274.5h",    0x00800, 0x64d113b4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr1273.5f",    0x00800, 0xa96316d3, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.6e",     0x00020, 0x9b87f90d, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scobras)
STD_ROM_FN(Scobras)

static struct BurnRomInfo ScobraseRomDesc[] = {
	{ "epr1265.2c",    0x01000, 0xa0744b3f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr1266.2e",    0x01000, 0x65306279, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr1267.2f",    0x01000, 0x47a4e6fb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr1268.2h",    0x01000, 0x53eecaf2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr1269.2j",    0x01000, 0xe1f8a801, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "epr1270.2l",    0x01000, 0xf7709710, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr1275.5c",    0x00800, 0xdeeb0dd3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr1276.5d",    0x00800, 0x872c1a74, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr1277.5e",    0x00800, 0xccd7a110, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
		
	{ "epr1274.5h",    0x00800, 0x64d113b4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr1273.5f",    0x00800, 0xa96316d3, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "pr1278.6e",     0x00020, 0xfd35c561, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scobrase)
STD_ROM_FN(Scobrase)

static struct BurnRomInfo ScobrabRomDesc[] = {
	{ "vid_2c.bin",    0x00800, 0xaeddf391, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "vid_2e.bin",    0x00800, 0x72b57eb7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2e.bin",  0x01000, 0xa270e44d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2f.bin",  0x01000, 0xbdd70346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2h.bin",  0x01000, 0xdca5ec31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2j.bin",  0x01000, 0x0d8f6b6e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2l.bin",  0x01000, 0x6f80f3a9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr1275.5c",    0x00800, 0xdeeb0dd3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr1276.5d",    0x00800, 0x872c1a74, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr1277.5e",    0x00800, 0xccd7a110, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
		
	{ "epr1274.5h",    0x00800, 0x64d113b4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr1273.5f",    0x00800, 0xa96316d3, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.6e",     0x00020, 0x9b87f90d, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Scobrab)
STD_ROM_FN(Scobrab)

static struct BurnRomInfo SuprheliRomDesc[] = {
	{ "1.2c",          0x01000, 0xb25141d8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2e.bin",  0x01000, 0xa270e44d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2f.bin",  0x01000, 0xbdd70346, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2h.bin",  0x01000, 0xdca5ec31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "scobra2j.bin",  0x01000, 0x0d8f6b6e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "6.2l",          0x01000, 0x10a474d9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "epr1275.5c",    0x00800, 0xdeeb0dd3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "epr1276.5d",    0x00800, 0x872c1a74, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "9.9d",          0x00800, 0x2b69b8f3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
		
	{ "epr1274.5h",    0x00800, 0x64d113b4, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "epr1273.5f",    0x00800, 0xa96316d3, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "k.6e",          0x00020, 0xfd35c561, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Suprheli)
STD_ROM_FN(Suprheli)

static struct BurnRomInfo LosttombRomDesc[] = {
	{ "2c",            0x01000, 0xd6176d2c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e",            0x01000, 0xa5f55f4a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f",            0x01000, 0x0169fa3c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h-easy",       0x01000, 0x054481b6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j",            0x01000, 0x249ee040, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l",            0x01000, 0xc7d2e608, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2m",            0x01000, 0xbc4bc5b1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "5c",            0x00800, 0xb899be2a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "5d",            0x00800, 0x6907af31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "5f",            0x00800, 0x61f137e7, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h",            0x00800, 0x5581de5f, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "ltprom",        0x00020, 0x1108b816, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Losttomb)
STD_ROM_FN(Losttomb)

static struct BurnRomInfo LosttombhRomDesc[] = {
	{ "2c",            0x01000, 0xd6176d2c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e",            0x01000, 0xa5f55f4a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f",            0x01000, 0x0169fa3c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "lthard",        0x01000, 0xe32cbf0e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j",            0x01000, 0x249ee040, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l",            0x01000, 0xc7d2e608, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2m",            0x01000, 0xbc4bc5b1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "5c",            0x00800, 0xb899be2a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "5d",            0x00800, 0x6907af31, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "5f",            0x00800, 0x61f137e7, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h",            0x00800, 0x5581de5f, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "ltprom",        0x00020, 0x1108b816, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Losttombh)
STD_ROM_FN(Losttombh)

static struct BurnRomInfo ArmorcarRomDesc[] = {
	{ "cpu.2c",        0x01000, 0x0d7bfdfb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cpu.2e",        0x01000, 0x76463213, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cpu.2f",        0x01000, 0x2cc6d5f0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cpu.2h",        0x01000, 0x61278dbb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "cpu.2j",        0x01000, 0xfb158d8c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "sound.5c",      0x00800, 0x54ee7753, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "sound.5d",      0x00800, 0x5218fec0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "cpu.5f",        0x00800, 0x8a3da4d1, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "cpu.5h",        0x00800, 0x85bdb113, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.6e",     0x00020, 0x9b87f90d, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Armorcar)
STD_ROM_FN(Armorcar)

static struct BurnRomInfo Armorcar2RomDesc[] = {
	{ "2c",            0x01000, 0xe393bd2f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e",            0x01000, 0xb7d443af, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2g",            0x01000, 0xe67380a4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h",            0x01000, 0x72af7b37, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j",            0x01000, 0xe6b0dd7f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "sound.5c",      0x00800, 0x54ee7753, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "sound.5d",      0x00800, 0x5218fec0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "cpu.5f",        0x00800, 0x8a3da4d1, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "cpu.5h",        0x00800, 0x85bdb113, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.6e",     0x00020, 0x9b87f90d, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Armorcar2)
STD_ROM_FN(Armorcar2)

static struct BurnRomInfo TazmaniaRomDesc[] = {
	{ "2c.cpu",        0x01000, 0x932c5a06, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e.cpu",        0x01000, 0xef17ce65, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f.cpu",        0x01000, 0x43c7c39d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h.cpu",        0x01000, 0xbe829694, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j.cpu",        0x01000, 0x6e197271, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2k.cpu",        0x01000, 0xa1eb453b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "rom0.snd",      0x00800, 0xb8d741f1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "5f.cpu",        0x00800, 0x2c5b612b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h.cpu",        0x00800, 0x3f5ff3ac, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "colr6f.cpu",    0x00020, 0xfce333c7, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Tazmania)
STD_ROM_FN(Tazmania)

static struct BurnRomInfo Tazmani2RomDesc[] = {
	{ "2ck.cpu",       0x01000, 0xbf0492bf, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2ek.cpu",       0x01000, 0x6636c4d0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2fk.cpu",       0x01000, 0xce59a57b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2hk.cpu",       0x01000, 0x8bda3380, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2jk.cpu",       0x01000, 0xa4095e35, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2kk.cpu",       0x01000, 0xf308ca36, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "rom0.snd",      0x00800, 0xb8d741f1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "5f.cpu",        0x00800, 0x2c5b612b, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h.cpu",        0x00800, 0x3f5ff3ac, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "colr6f.cpu",    0x00020, 0xfce333c7, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Tazmani2)
STD_ROM_FN(Tazmani2)

static struct BurnRomInfo AnteaterRomDesc[] = {
	{ "ra1-2c",        0x01000, 0x58bc9393, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ra1-2e",        0x01000, 0x574fc6f6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ra1-2f",        0x01000, 0x2f7c1fe5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ra1-2h",        0x01000, 0xae8a5da3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ra4-5c",        0x00800, 0x87300b4f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ra4-5d",        0x00800, 0xaf4e5ffe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "ra6-5f",        0x00800, 0x4c3f8a08, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ra6-5h",        0x00800, 0xb30c7c9f, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "colr6f.cpu",    0x00020, 0xfce333c7, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Anteater)
STD_ROM_FN(Anteater)

static struct BurnRomInfo AnteatergRomDesc[] = {
	{ "prg_2.bin",     0x02000, 0x2ba793a8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "prg_1.bin",     0x02000, 0x7a798af5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ra4-5c",        0x00800, 0x87300b4f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ra4-5d",        0x00800, 0xaf4e5ffe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "gfx_1.bin",     0x00800, 0x1e2824b1, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "gfx_2.bin",     0x00800, 0x784319b3, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "colr6f.cpu",    0x00020, 0xfce333c7, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Anteaterg)
STD_ROM_FN(Anteaterg)

static struct BurnRomInfo AnteaterukRomDesc[] = {
	{ "ant1.bin",      0x02000, 0x69debc90, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ant2.bin",      0x02000, 0xab352805, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "ra4-5c",        0x00800, 0x87300b4f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ra4-5d",        0x00800, 0xaf4e5ffe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "gfx_1.bin",     0x00800, 0x1e2824b1, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "gfx_2.bin",     0x00800, 0x784319b3, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "colr6f.cpu",    0x00020, 0xfce333c7, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Anteateruk)
STD_ROM_FN(Anteateruk)

static struct BurnRomInfo SpdcoinRomDesc[] = {
	{ "spdcoin.2c",    0x01000, 0x65cf1e49, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "spdcoin.2e",    0x01000, 0x1ee59232, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "spdcoin.5c",    0x00800, 0xb4cf64b7, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "spdcoin.5d",    0x00800, 0x92304df0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "spdcoin.5f",    0x00800, 0xdd5f1dbc, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "spdcoin.5h",    0x00800, 0xab1fe81b, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "spdcoin.clr",   0x00020, 0x1a2ccc56, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Spdcoin)
STD_ROM_FN(Spdcoin)

static struct BurnRomInfo SuperbonRomDesc[] = {
	{ "2d.cpu",        0x01000, 0x60c0ba18, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e.cpu",        0x01000, 0xddcf44bf, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f.cpu",        0x01000, 0xbb66c2d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h.cpu",        0x01000, 0x74f4f04d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j.cpu",        0x01000, 0x78effb08, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l.cpu",        0x01000, 0xe9dcecbd, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2m.cpu",        0x01000, 0x3ed0337e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "5c",            0x00800, 0xb899be2a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "5d.snd",        0x00800, 0x80640a04, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "5f.cpu",        0x00800, 0x5b9d4686, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h.cpu",        0x00800, 0x58c29927, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "superbon.clr",  0x00020, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STD_ROM_PICK(Superbon)
STD_ROM_FN(Superbon)

static struct BurnRomInfo CalipsoRomDesc[] = {
	{ "calipso.2c",    0x01000, 0x0fcb703c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "calipso.2e",    0x01000, 0xc6622f14, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "calipso.2f",    0x01000, 0x7bacbaba, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "calipso.2h",    0x01000, 0xa3a8111b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "calipso.2j",    0x01000, 0xfcbd7b9e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "calipso.2l",    0x01000, 0xf7630cab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "calipso.5c",    0x00800, 0x9cbc65ab, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "calipso.5d",    0x00800, 0xa225ee3b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "calipso.5f",    0x02000, 0xfd4252e9, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "calipso.5h",    0x02000, 0x1663a73a, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "calipso.clr",   0x00020, 0x01165832, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Calipso)
STD_ROM_FN(Calipso)

static struct BurnRomInfo MoonwarRomDesc[] = {
	{ "mw2.2c",        0x01000, 0x7c11b4d9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mw2.2e",        0x01000, 0x1b6362be, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mw2.2f",        0x01000, 0x4fd8ba4b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mw2.2h",        0x01000, 0x56879f0d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mw2.5c",        0x00800, 0xc26231eb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "mw2.5d",        0x00800, 0xbb48a646, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "mw2.5f",        0x00800, 0xc5fa1aa0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mw2.5h",        0x00800, 0xa6ccc652, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "mw2.clr",       0x00020, 0x99614c6c, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Moonwar)
STD_ROM_FN(Moonwar)

static struct BurnRomInfo MoonwaraRomDesc[] = {
	{ "2c",            0x01000, 0xbc20b734, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e",            0x01000, 0xdb6ffec2, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f",            0x01000, 0x378931b8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h",            0x01000, 0x031dbc2c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mw2.5c",        0x00800, 0xc26231eb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "mw2.5d",        0x00800, 0xbb48a646, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "mw2.5f",        0x00800, 0xc5fa1aa0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mw2.5h",        0x00800, 0xa6ccc652, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "moonwara.clr",  0x00020, 0xf58d4f58, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Moonwara)
STD_ROM_FN(Moonwara)

static struct BurnRomInfo StratgyxRomDesc[] = {
	{ "2c_1.bin",      0x01000, 0xeec01237, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e_2.bin",      0x01000, 0x926cb2d5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f_3.bin",      0x01000, 0x849e2504, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h_4.bin",      0x01000, 0x8a64069b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j_5.bin",      0x01000, 0x78b9b898, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l_6.bin",      0x01000, 0x20bae414, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "s1.bin",        0x01000, 0x713a5db8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "s2.bin",        0x01000, 0x46079411, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
				
	{ "5f_c2.bin",     0x00800, 0x7121b679, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h_c1.bin",     0x00800, 0xd105ad91, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "strategy.6e",   0x00020, 0x51a629e1, BRF_GRA | GAL_ROM_PROM },
	{ "strategy.10k",  0x00020, 0xd95c0318, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Stratgyx)
STD_ROM_FN(Stratgyx)

static struct BurnRomInfo StratgysRomDesc[] = {
	{ "2c.cpu",        0x01000, 0xf2aaaf2b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2e.cpu",        0x01000, 0x5873fdc8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f.cpu",        0x01000, 0x532d604f, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h.cpu",        0x01000, 0x82b1d95e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j.cpu",        0x01000, 0x66e84cde, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l.cpu",        0x01000, 0x62b032d0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "s1.bin",        0x01000, 0x713a5db8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "s2.bin",        0x01000, 0x46079411, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
				
	{ "5f.cpu",        0x00800, 0xf4aa5ddd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h.cpu",        0x00800, 0x548e4635, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "strategy.6e",   0x00020, 0x51a629e1, BRF_GRA | GAL_ROM_PROM },
	{ "strategy.10k",  0x00020, 0xd95c0318, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Stratgys)
STD_ROM_FN(Stratgys)

static struct BurnRomInfo StrongxRomDesc[] = {
	{ "strongx.2d",    0x01000, 0xc2f7268c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "strongx.2e",    0x01000, 0x91059422, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2f_3.bin",      0x01000, 0x849e2504, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2h_4.bin",      0x01000, 0x8a64069b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2j_5.bin",      0x01000, 0x78b9b898, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "2l_6.bin",      0x01000, 0x20bae414, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "s1.bin",        0x01000, 0x713a5db8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "s2.bin",        0x01000, 0x46079411, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
				
	{ "5f_c2.bin",     0x00800, 0x7121b679, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h_c1.bin",     0x00800, 0xd105ad91, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "strategy.6e",   0x00020, 0x51a629e1, BRF_GRA | GAL_ROM_PROM },
	{ "strategy.10k",  0x00020, 0xd95c0318, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Strongx)
STD_ROM_FN(Strongx)

static struct BurnRomInfo DarkplntRomDesc[] = {
	{ "drkplt2c.dat",  0x01000, 0x5a0ca559, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "drkplt2e.dat",  0x01000, 0x52e2117d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "drkplt2g.dat",  0x01000, 0x4093219c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "drkplt2j.dat",  0x01000, 0xb974c78d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "drkplt2k.dat",  0x01000, 0x71a37385, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "drkplt2l.dat",  0x01000, 0x5ad25154, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "drkplt2m.dat",  0x01000, 0x8d2f0122, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "drkplt2p.dat",  0x01000, 0x2d66253b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "5c.snd",        0x01000, 0x672b9454, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
				
	{ "drkplt5f.dat",  0x00800, 0x2af0ee66, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "drkplt5h.dat",  0x00800, 0x66ef3225, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "6e.cpu",        0x00020, 0x86b6e124, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Darkplnt)
STD_ROM_FN(Darkplnt)

static struct BurnRomInfo RescueRomDesc[] = {
	{ "rb15acpu.bin",  0x01000, 0xd7e654ba, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rb15bcpu.bin",  0x01000, 0xa93ea158, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rb15ccpu.bin",  0x01000, 0x058cd3d0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rb15dcpu.bin",  0x01000, 0xd6505742, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rb15ecpu.bin",  0x01000, 0x604df3a4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "rb15csnd.bin",  0x00800, 0x8b24bf17, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "rb15dsnd.bin",  0x00800, 0xd96e4fb3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "rb15fcpu.bin",  0x00800, 0x4489d20c, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rb15hcpu.bin",  0x00800, 0x5512c547, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "rescue.clr",    0x00020, 0x40c6bcbd, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Rescue)
STD_ROM_FN(Rescue)

static struct BurnRomInfo AponowRomDesc[] = {
	{ "aponow01.rom",  0x01000, 0x33011579, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "aponow02.rom",  0x01000, 0xd477573e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "aponow03.rom",  0x01000, 0x46c41898, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rb15dcpu.bin",  0x01000, 0xd6505742, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "rb15ecpu.bin",  0x01000, 0x604df3a4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "rb15csnd.bin",  0x00800, 0x8b24bf17, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "rb15dsnd.bin",  0x00800, 0xd96e4fb3, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "rb15fcpu.bin",  0x00800, 0x4489d20c, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rb15hcpu.bin",  0x00800, 0x5512c547, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "rescue.clr",    0x00020, 0x40c6bcbd, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Aponow)
STD_ROM_FN(Aponow)

static struct BurnRomInfo MinefldRomDesc[] = {
	{ "ma22c",         0x01000, 0x1367a035, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ma22e",         0x01000, 0x68946d21, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ma22f",         0x01000, 0x7663aee5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ma22h",         0x01000, 0x9787475d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ma22j",         0x01000, 0x2ceceb54, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "ma22l",         0x01000, 0x85138fc9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },	
	
	{ "ma15c",         0x00800, 0x8bef736b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "ma15d",         0x00800, 0xf67b3f97, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "ma15f",         0x00800, 0x9f703006, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "ma15h",         0x00800, 0xed0dccb1, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "minefld.clr",   0x00020, 0x1877368e, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Minefld)
STD_ROM_FN(Minefld)

static struct BurnRomInfo HustlerRomDesc[] = {
	{ "hustler.1",     0x01000, 0x94479a3e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "hustler.2",     0x01000, 0x3cc67bcc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "hustler.3",     0x01000, 0x9422226a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "hustler.6",     0x00800, 0x7a946544, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "hustler.7",     0x00800, 0x3db57351, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "hustler.5f",    0x00800, 0x0bdfad0e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "hustler.5h",    0x00800, 0x8e062177, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "hustler.clr",   0x00020, 0xaa1f7f5e, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Hustler)
STD_ROM_FN(Hustler)

static struct BurnRomInfo BilliardRomDesc[] = {
	{ "a",             0x01000, 0xb7eb50c0, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "b",             0x01000, 0x988fe1c5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "c",             0x01000, 0x7b8de793, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "hustler.6",     0x00800, 0x7a946544, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "hustler.7",     0x00800, 0x3db57351, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "hustler.5f",    0x00800, 0x0bdfad0e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "hustler.5h",    0x00800, 0x8e062177, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "hustler.clr",   0x00020, 0xaa1f7f5e, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Billiard)
STD_ROM_FN(Billiard)

static struct BurnRomInfo HustlerbRomDesc[] = {
	{ "hustler.2c",    0x01000, 0x3a1ac6a9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "hustler.2f",    0x01000, 0xdc6752ec, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "hustler.2j",    0x01000, 0x27c1e0f8, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "hustler.11d",   0x00800, 0xb559bfde, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "hustler.10d",   0x00800, 0x6ef96cfb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "hustler.5f",    0x00800, 0x0bdfad0e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "hustler.5h",    0x00800, 0x8e062177, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "hustler.clr",   0x00020, 0xaa1f7f5e, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Hustlerb)
STD_ROM_FN(Hustlerb)

static struct BurnRomInfo MimonkeyRomDesc[] = {
	{ "mm1.2e",        0x01000, 0x9019f1b1, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm2.2e",        0x01000, 0x043e97d6, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm3.2f",        0x01000, 0x1052726a, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm4.2h",        0x01000, 0x7b3f35ff, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm5.2j",        0x01000, 0xb4e5c32d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm6.2l",        0x01000, 0x409036c4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm7.2m",        0x01000, 0x119c08fa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "mm8.2p",        0x01000, 0xf7989f04, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mm13.11d",      0x01000, 0x2d14c527, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "mm14.10d",      0x01000, 0x35ed0f96, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "mm12.5h",       0x01000, 0xf73a8412, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mm10.5h",       0x01000, 0x3828c9db, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mm11.5f",       0x01000, 0x9e0e9289, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mm9.5f",        0x01000, 0x92085b0c, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.6e",     0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mimonkey)
STD_ROM_FN(Mimonkey)

static struct BurnRomInfo MimonscoRomDesc[] = {
	{ "fra_1a",        0x01000, 0x8e7a7379, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "fra_1b",        0x01000, 0xab08cbfe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "fra_2a",        0x01000, 0x2d4da24d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "fra_2b",        0x01000, 0x8d88fc7c, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "fra_3a",        0x01000, 0xb4e5c32d, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "fra_3b",        0x01000, 0x409036c4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "fra_4a",        0x01000, 0x119c08fa, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "fra_4b",        0x01000, 0xd700fd03, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "mmsound1",      0x01000, 0x2d14c527, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
	{ "mmsnd2a",       0x01000, 0x35ed0f96, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG2 },
			
	{ "mmgfx1",        0x02000, 0x4af47337, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "mmgfx2",        0x02000, 0xdef47da8, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.6e",     0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
};

STD_ROM_PICK(Mimonsco)
STD_ROM_FN(Mimonsco)

UINT8 __fastcall Tazmani2Z80Read(UINT16 a)
{
	if (a >= 0xa000 && a <= 0xa00f) {
		return ppi8255_r(0, (a - 0xa000) >> 2);
	}
	
	if (a >= 0xa800 && a <= 0xa80f) {
		return ppi8255_r(1, (a - 0xa800) >> 2);
	}
	
	switch (a) {
		case 0x9800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall Tazmani2Z80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x8800 && a <= 0x88ff) {
		INT32 Offset = a - 0x8800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0xa000 && a <= 0xa00f) {
		ppi8255_w(0, (a - 0xa000) >> 2, d);
		return;
	}
	
	if (a >= 0xa800 && a <= 0xa80f) {
		ppi8255_w(1, (a - 0xa800) >> 2, d);
		return;
	}
	
	switch (a) {
		case 0xb000: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb002: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0xb004: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb006: {
			// coin_count_0_w
			return;
		}
		
		case 0xb00c: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb00e: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall AnteatergZ80Read(UINT16 a)
{
	switch (a) {
		case 0xf521: {
			// watchdog read
			return 0xff;
		}
		
		case 0xf612: {
			return ppi8255_r(0, 0);
		}
		
		case 0xf631: {
			return ppi8255_r(0, 1);
		}
		
		case 0xf710: {
			return ppi8255_r(0, 2);
		}		
		
		case 0xf753: {
			return ppi8255_r(0, 3);
		}		
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall AnteatergZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x2000 && a <= 0x20ff) {
		INT32 Offset = a - 0x2000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x2423: {
			ppi8255_w(1, 3, d);
			return;
		}
		
		case 0x2450: {
			ppi8255_w(1, 0, d);
			return;
		}
		
		case 0x2511: {
			ppi8255_w(1, 1, d);
			return;
		}
		
		case 0x2621: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x2624: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x2647: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x2653: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0x2702: {
			// coin_count_0_w
			return;
		}
		
		case 0x2736: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xf612: {
			ppi8255_w(0, 0, d);
			return;
		}
		
		case 0xf631: {
			ppi8255_w(0, 1, d);
			return;
		}
		
		case 0xf710: {
			ppi8255_w(0, 2, d);
			return;
		}
		
		case 0xf753: {
			ppi8255_w(0, 3, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall AnteatgbZ80Read(UINT16 a)
{
	switch (a) {
		case 0x145b: {
			// watchdog read
			return 0xff;
		}
		
		case 0xf300:
		case 0xf301:
		case 0xf302:
		case 0xf303: {
			return ppi8255_r(0, a - 0xf300);
		}
		
		case 0xfe00:
		case 0xfe01:
		case 0xfe02:
		case 0xfe03: {
			return ppi8255_r(1, a - 0xfe00);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall AnteatgbZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x1200 && a <= 0x12ff) {
		INT32 Offset = a - 0x1200;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x1171: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x1172: {
			// coin_count_0_w
			return;
		}
		
		case 0x1173: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0x1174: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x1176: {
			GalFlipScreenX = d & 1;
			return;
		}
				
		case 0x1177: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xf300:
		case 0xf301:
		case 0xf302:
		case 0xf303: {
			ppi8255_w(0, a - 0xf300, d);
			return;
		}
		
		case 0xfe00:
		case 0xfe01:
		case 0xfe02:
		case 0xfe03: {
			ppi8255_w(1, a - 0xfe00, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall StratgyxZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x8800 && a <= 0x88ff) {
		INT32 Offset = a - 0x8800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0xa000 && a <= 0xa00f) {
		ppi8255_w(0, (a - 0xa000) >> 2, d);
		return;
	}
	
	if (a >= 0xa800 && a <= 0xa80f) {
		ppi8255_w(1, (a - 0xa800) >> 2, d);
		return;
	}
	
	switch (a) {
		case 0xb000: {
			GalBackgroundGreen = d & 1;
			return;
		}
		
		case 0xb002: {
			GalBackgroundBlue = d & 1;
			return;
		}
		
		case 0xb004: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb006: {
			// coin_count_0_w
			return;
		}
		
		case 0xb00a: {
			GalBackgroundRed = d & 1;
			return;
		}
		
		case 0xb00c: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb00e: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall DarkplntZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x8800 && a <= 0x88ff) {
		INT32 Offset = a - 0x8800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0xa000 && a <= 0xa00f) {
		ppi8255_w(0, (a - 0xa000) >> 2, d);
		return;
	}
	
	if (a >= 0xa800 && a <= 0xa80f) {
		ppi8255_w(1, (a - 0xa800) >> 2, d);
		return;
	}
	
	switch (a) {
		case 0xb000: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb002: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0xb004: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb006: {
			// coin_count_0_w
			return;
		}
		
		case 0xb00a: {
			DarkplntBulletColour = d & 1;
			return;
		}
		
		case 0xb00c: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb00e: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall HustlerZ80Read(UINT16 a)
{
	if (a >= 0xd000 && a <= 0xd01f) {
		return ppi8255_r(0, (a - 0xd000) >> 3);
	}
	
	if (a >= 0xe000 && a <= 0xe01f) {
		return ppi8255_r(1, (a - 0xe000) >> 3);
	}
	
	switch (a) {
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall HustlerZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9000 && a <= 0x90ff) {
		INT32 Offset = a - 0x9000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0xd000 && a <= 0xd01f) {
		ppi8255_w(0, (a - 0xd000) >> 3, d);
		return;
	}
	
	if (a >= 0xe000 && a <= 0xe01f) {
		ppi8255_w(1, (a - 0xe000) >> 3, d);
		return;
	}
	
	switch (a) {
		case 0xa802: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xa804: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa806: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xa80e: {
			// coin_count_0_w
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall HustlerbZ80Read(UINT16 a)
{
	switch (a) {
		case 0xb000: {
			// watchdog read
			return 0xff;
		}
		
		case 0xc100:
		case 0xc101:
		case 0xc102:
		case 0xc103: {
			return ppi8255_r(0, a - 0xc100);
		}
		
		case 0xc200:
		case 0xc201:
		case 0xc202:
		case 0xc203: {
			return ppi8255_r(1, a - 0xc200);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall HustlerbZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9000 && a <= 0x90ff) {
		INT32 Offset = a - 0x9000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa802: {
			// coin_count_0_w
			return;
		}
		
		case 0xa806: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xa807: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xc100:
		case 0xc101:
		case 0xc102:
		case 0xc103: {
			ppi8255_w(0, a - 0xc100, d);
			return;
		}
		
		case 0xc200:
		case 0xc201:
		case 0xc202:
		case 0xc203: {
			ppi8255_w(1, a - 0xc200, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall HustlerbSoundZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x80: {
			return AY8910Read(0);
		}
	}
	
	return 0xff;
}

void __fastcall HustlerbSoundZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x40: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x80: {
			AY8910Write(0, 1, d);
			return;
		}
	}
}

void __fastcall MimonkeyZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9000 && a <= 0x90ff) {
		INT32 Offset = a - 0x9000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x9800:
		case 0x9801:
		case 0x9802:
		case 0x9803: {
			ppi8255_w(0, a - 0x9800, d);
			return;
		}
		
		case 0xa000:
		case 0xa001:
		case 0xa002:
		case 0xa003: {
			ppi8255_w(1, a - 0xa000, d);
			return;
		}
		
		case 0xa800: {
			GalGfxBank[0] = d & 1;
			return;
		}
		
		case 0xa801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa802: {
			GalGfxBank[1] = d & 1;
			return;
		}
		
		case 0xa803: {
			// ???
			return;
		}
		
		case 0xa804: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0xa805: {
			// ???
			return;
		}
		
		case 0xa806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xa807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 ScobraInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapScobra;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	return nRet;
}

static INT32 LosttombInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapScobra;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	UINT8 *TempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	BurnLoadRom(TempRom + 0x0800, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	for (UINT32 Offset = 0; Offset < GalTilesSharedRomSize; Offset++) {
		UINT32 SrcOffset = Offset & 0xa7f;
		SrcOffset |= ((BIT(Offset, 1) & BIT(Offset, 8)) | ((1 ^ BIT(Offset, 1)) & (BIT(Offset, 10)))) << 7;
		SrcOffset |= (BIT(Offset, 7) ^ (BIT(Offset, 1) & (BIT(Offset, 7) ^ BIT(Offset, 10)))) << 8;
		SrcOffset |= ((BIT(Offset, 1) & BIT(Offset, 7)) | ((1 ^ BIT(Offset, 1)) & (BIT(Offset, 8)))) << 10;
		GalTempRom[Offset] = TempRom[SrcOffset];
	}
	BurnFree(TempRom);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	return nRet;
}

static void MapTazmani2()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(Tazmani2Z80Read);
	ZetSetWriteHandler(Tazmani2Z80Write);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x8000) ? 0x7fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x8000) ? 0x7fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetMapArea(0x8800, 0x88ff, 0, GalSpriteRam);
	ZetMapArea(0x8800, 0x88ff, 2, GalSpriteRam);
	ZetMapArea(0x9000, 0x93ff, 0, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 1, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 2, GalVideoRam);	
	ZetMemEnd();
	ZetClose();
}

static INT32 Tazmani2Init()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapTazmani2;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	return nRet;
}

static INT32 AnteaterInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapScobra;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	UINT8 *TempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	BurnLoadRom(TempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1);
	BurnLoadRom(TempRom + 0x0800, GAL_ROM_OFFSET_TILES_SHARED + 1, 1);
	for (UINT32 Offset = 0; Offset < GalTilesSharedRomSize; Offset++) {
		UINT32 SrcOffset = Offset & 0x9bf;
		SrcOffset |= (BIT(Offset, 4) ^ BIT(Offset, 9) ^ (BIT(Offset, 2) & BIT(Offset, 10))) << 6;
		SrcOffset |= (BIT(Offset, 2) ^ BIT(Offset, 10)) << 9;
		SrcOffset |= (BIT(Offset, 0) ^ BIT(Offset, 6) ^ 1) << 10;
		GalTempRom[Offset] = TempRom[SrcOffset];
	}
	BurnFree(TempRom);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	
	GalRenderBackgroundFunction = AnteaterDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
	
	return nRet;
}

static void AnteatergPostLoad()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(AnteatergZ80Read);
	ZetSetWriteHandler(AnteatergZ80Write);
	ZetMapArea(0x0000, 0x03ff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x03ff, 2, GalZ80Rom1);
	ZetMapArea(0x0400, 0x0bff, 0, GalZ80Ram1);
	ZetMapArea(0x0400, 0x0bff, 1, GalZ80Ram1);
	ZetMapArea(0x0400, 0x0bff, 2, GalZ80Ram1);
	ZetMapArea(0x0c00, 0x0fff, 0, GalVideoRam);
	ZetMapArea(0x0c00, 0x0fff, 1, GalVideoRam);
	ZetMapArea(0x0c00, 0x0fff, 2, GalVideoRam);
	ZetMapArea(0x2000, 0x20ff, 0, GalSpriteRam);
	ZetMapArea(0x2000, 0x20ff, 2, GalSpriteRam);
	ZetMapArea(0x4600, 0x4fff, 0, GalZ80Rom1 + 0x0400);
	ZetMapArea(0x4600, 0x4fff, 2, GalZ80Rom1 + 0x0400);
	ZetMapArea(0x6400, 0x7aff, 0, GalZ80Rom1 + 0x0e00);
	ZetMapArea(0x6400, 0x7aff, 2, GalZ80Rom1 + 0x0e00);
	ZetMapArea(0x7c00, 0x7fff, 0, GalVideoRam);
	ZetMapArea(0x7c00, 0x7fff, 1, GalVideoRam);
	ZetMapArea(0x7c00, 0x7fff, 2, GalVideoRam);
	ZetMapArea(0x8300, 0x98ff, 0, GalZ80Rom1 + 0x2500);
	ZetMapArea(0x8300, 0x98ff, 2, GalZ80Rom1 + 0x2500);
	ZetMapArea(0xa300, 0xa7ff, 0, GalZ80Rom1 + 0x3b00);
	ZetMapArea(0xa300, 0xa7ff, 2, GalZ80Rom1 + 0x3b00);
	ZetMemEnd();
	ZetClose();
}

static INT32 AnteatergInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = AnteatergPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = AnteaterDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
	
	return nRet;
}

static void AnteaterukPostLoad()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(AnteatgbZ80Read);
	ZetSetWriteHandler(AnteatgbZ80Write);
	ZetMapArea(0x0000, 0x03ff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x03ff, 2, GalZ80Rom1);
	ZetMapArea(0x0400, 0x0bff, 0, GalZ80Ram1);
	ZetMapArea(0x0400, 0x0bff, 1, GalZ80Ram1);
	ZetMapArea(0x0400, 0x0bff, 2, GalZ80Ram1);
	ZetMapArea(0x0c00, 0x0fff, 0, GalVideoRam);
	ZetMapArea(0x0c00, 0x0fff, 1, GalVideoRam);
	ZetMapArea(0x0c00, 0x0fff, 2, GalVideoRam);
	ZetMapArea(0x1200, 0x12ff, 0, GalSpriteRam);
	ZetMapArea(0x1200, 0x12ff, 2, GalSpriteRam);
	ZetMapArea(0x4600, 0x4fff, 0, GalZ80Rom1 + 0x0400);
	ZetMapArea(0x4600, 0x4fff, 2, GalZ80Rom1 + 0x0400);
	ZetMapArea(0x6400, 0x7aff, 0, GalZ80Rom1 + 0x0e00);
	ZetMapArea(0x6400, 0x7aff, 2, GalZ80Rom1 + 0x0e00);
	ZetMapArea(0x8300, 0x98ff, 0, GalZ80Rom1 + 0x2500);
	ZetMapArea(0x8300, 0x98ff, 2, GalZ80Rom1 + 0x2500);
	ZetMapArea(0xa300, 0xa7ff, 0, GalZ80Rom1 + 0x3b00);
	ZetMapArea(0xa300, 0xa7ff, 2, GalZ80Rom1 + 0x3b00);
	ZetMemEnd();
	ZetClose();
}

static INT32 AnteaterukInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = AnteaterukPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
	
	return nRet;
}

static void SuperbonPostLoad()
{
	MapScobra();
	
	for (UINT32 Offset = 0; Offset < 0x1000; Offset++) {
		switch (Offset & 0x280) {
			case 0x000: GalZ80Rom1[Offset] ^= 0x92; break;
			case 0x080: GalZ80Rom1[Offset] ^= 0x82; break;
			case 0x200: GalZ80Rom1[Offset] ^= 0x12; break;
			case 0x280: GalZ80Rom1[Offset] ^= 0x10; break;
		}
	}
}

static INT32 SuperbonInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = SuperbonPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;	
	GalPromRomSize = 0x20;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
		
	HardCodeGalaxianPROM();
	
	return nRet;
}

static INT32 CalipsoInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapScobra;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;	
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendSpriteInfoFunction = CalipsoExtendSpriteInfo;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
	
	return nRet;
}

static UINT8 MoonwarPPIReadIN0()
{
	UINT8 Dial1 = (GalAnalogPort0 >> 8) & 0xff;
	UINT8 Dial2 = (GalAnalogPort1 >> 8) & 0xff;
	UINT8 Input0 = 0xff - GalInput[0] - GalDip[0];
	UINT8 Sign;
	UINT8 Delta;
	
	if (Dial1 >= 0xfc || (Dial1 >= 0x01 && Dial1 <= 0x04)) Dial1 = 0;
	if (Dial1 >= 0xf8) Dial1 = 0xfd;
	if (Dial1 >= 0x01 && Dial1 <= 0x07) Dial1 = 0x02;
	
	if (Dial2 >= 0xfc || (Dial2 >= 0x01 && Dial2 <= 0x04)) Dial2 = 0;
	if (Dial2 >= 0xf8) Dial2 = 0xfd;
	if (Dial2 >= 0x01 && Dial2 <= 0x07) Dial2 = 0x02;
	
	Delta = (MoonwarPortSelect ? Dial1 : Dial2);
	Sign = (Delta & 0x80) >> 3;
	Delta &= 0x0f;
	
	return (Input0 & 0xe0) | Delta | Sign;
}

static void MoonwarPortSelectWrite(UINT8 d)
{
	MoonwarPortSelect = d & 0x10;
}

static INT32 MoonwarInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapScobra;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;	
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = MoonwarDrawBullets;
	
	KonamiPPIInit();
	
	PPI0PortReadA = MoonwarPPIReadIN0;
	PPI0PortWriteC = MoonwarPortSelectWrite;
	
	return nRet;
}

static void StratgyxPostLoad()
{
	MapTazmani2();
	
	ZetOpen(0);
	ZetSetWriteHandler(StratgyxZ80Write);
	ZetClose();
}

static INT32 StratgyxInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = StratgyxPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalCalcPaletteFunction = StratgyxCalcPalette;
	GalRenderBackgroundFunction = StratgyxDrawBackground;
	
	KonamiPPIInit();
	
	return nRet;
}

static void DarkplntPostLoad()
{
	MapTazmani2();
	
	ZetOpen(0);
	ZetSetWriteHandler(DarkplntZ80Write);
	ZetClose();
}

static INT16 DarkplntDial = 0;
static UINT32 DarkplntDialFrameNumberRead = 0;

static UINT8 DarkplntPPIReadIN1()
{
	UINT32 CurrentFrame = GetCurrentFrame();
	
	if (DarkplntDialFrameNumberRead != CurrentFrame) {
		if (GalInputPort3[0]) DarkplntDial -= 0x01;
		if (GalInputPort3[1]) DarkplntDial += 0x01;
		if (DarkplntDial > 0xfc) DarkplntDial = 0x00;
		if (DarkplntDial < 0) DarkplntDial = 0xfc;
		DarkplntDialFrameNumberRead = CurrentFrame;
	}
	
	static const UINT8 remap[] = {0x03, 0x02, 0x00, 0x01, 0x21, 0x20, 0x22, 0x23, 0x33, 0x32, 0x30, 0x31, 0x11, 0x10, 0x12, 0x13, 0x17, 0x16, 0x14, 0x15, 0x35, 0x34, 0x36, 0x37, 0x3f, 0x3e, 0x3c, 0x3d, 0x1d, 0x1c, 0x1e, 0x1f, 0x1b, 0x1a, 0x18, 0x19, 0x39, 0x38, 0x3a, 0x3b, 0x2b, 0x2a, 0x28, 0x29, 0x09, 0x08, 0x0a, 0x0b, 0x0f, 0x0e, 0x0c, 0x0d, 0x2d, 0x2c, 0x2e, 0x2f, 0x27, 0x26, 0x24, 0x25, 0x05, 0x04, 0x06, 0x07 };
		
	return 0xff - GalInput[1] - GalDip[1] - remap[DarkplntDial >> 2];
}

static INT32 DarkplntInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = DarkplntPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalCalcPaletteFunction = DarkplntCalcPalette;
	GalDrawBulletsFunction = DarkplntDrawBullets;
	
	GalSoundVolumeShift = 2;
	
	KonamiPPIInit();	
	PPI0PortReadB = DarkplntPPIReadIN1;
	
	return nRet;
}

static INT32 RescueInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapScobra;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	UINT8* TempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x0800, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	memcpy(TempRom, GalTempRom, GalTilesSharedRomSize);
	for (UINT32 Offset = 0; Offset < GalTilesSharedRomSize; Offset++) {
		UINT32 j = Offset & 0xa7f;
		j |= (BIT(Offset, 3) ^ BIT(Offset, 10)) << 7;
		j |= (BIT(Offset, 1) ^ BIT(Offset, 7)) << 8;
		j |= (BIT(Offset, 0) ^ BIT(Offset, 8)) << 10;
		GalTempRom[Offset] = TempRom[j];
	}
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(TempRom);
	BurnFree(GalTempRom);
	
	GalCalcPaletteFunction = RescueCalcPalette;
	GalRenderBackgroundFunction = RescueDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
	
	return nRet;
}

static INT32 MinefldInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapScobra;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;
	
	nRet = GalInit();
	KonamiSoundInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	UINT8* TempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	nRet = BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x0800, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	memcpy(TempRom, GalTempRom, GalTilesSharedRomSize);
	for (UINT32 Offset = 0; Offset < GalTilesSharedRomSize; Offset++) {
		UINT32 j = Offset & 0xd5f;
		j |= (BIT(Offset, 3) ^ BIT(Offset, 7)) << 5;
		j |= (BIT(Offset, 2) ^ BIT(Offset, 9) ^ (BIT(Offset, 0) & BIT(Offset, 5)) ^ (BIT(Offset, 3) & BIT(Offset, 7) & (BIT(Offset, 0) ^ BIT(Offset, 5)))) << 7;
		j |= (BIT(Offset, 0) ^ BIT(Offset, 5) ^ (BIT(Offset, 3) & BIT(Offset, 7))) << 9;
		GalTempRom[Offset] = TempRom[j];
	}
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(TempRom);
	BurnFree(GalTempRom);
	
	GalCalcPaletteFunction = MinefldCalcPalette;
	GalRenderBackgroundFunction = MinefldDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 2;
	
	KonamiPPIInit();
	
	return nRet;
}

static void HustlerPostLoad()
{
	MapScobra();
	
	ZetOpen(0);
	ZetSetReadHandler(HustlerZ80Read);
	ZetSetWriteHandler(HustlerZ80Write);
	ZetClose();
	
	for (UINT32 Offset = 0; Offset < GalZ80Rom1Size; Offset++) {
		UINT8 XorMask;
		INT32 Bits[8];

		for (UINT32 i = 0; i < 8; i++) Bits[i] = (Offset >> i) & 1;

		XorMask = 0xff;
		if (Bits[0] ^ Bits[1]) XorMask ^= 0x01;
		if (Bits[3] ^ Bits[6]) XorMask ^= 0x02;
		if (Bits[4] ^ Bits[5]) XorMask ^= 0x04;
		if (Bits[0] ^ Bits[2]) XorMask ^= 0x08;
		if (Bits[2] ^ Bits[3]) XorMask ^= 0x10;
		if (Bits[1] ^ Bits[5]) XorMask ^= 0x20;
		if (Bits[0] ^ Bits[7]) XorMask ^= 0x40;
		if (Bits[4] ^ Bits[6]) XorMask ^= 0x80;

		GalZ80Rom1[Offset] ^= XorMask;
	}
}

static INT32 HustlerInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = HustlerPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910;
	
	nRet = GalInit();
	FroggerSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

static void BilliardPostLoad()
{
	MapScobra();
	
	ZetOpen(0);
	ZetSetReadHandler(HustlerZ80Read);
	ZetSetWriteHandler(HustlerZ80Write);
	ZetClose();
	
	for (UINT32 Offset = 0; Offset < GalZ80Rom1Size; Offset++) {
		UINT8 XorMask;
		INT32 Bits[8];

		for (UINT32 i = 0; i < 8; i++) Bits[i] = (Offset >> i) & 1;

		XorMask = 0x55;
		if (Bits[2] ^ ( Bits[3] &  Bits[6])) XorMask ^= 0x01;
		if (Bits[4] ^ ( Bits[5] &  Bits[7])) XorMask ^= 0x02;
		if (Bits[0] ^ ( Bits[7] & !Bits[3])) XorMask ^= 0x04;
		if (Bits[3] ^ (!Bits[0] &  Bits[2])) XorMask ^= 0x08;
		if (Bits[5] ^ (!Bits[4] &  Bits[1])) XorMask ^= 0x10;
		if (Bits[6] ^ (!Bits[2] & !Bits[5])) XorMask ^= 0x20;
		if (Bits[1] ^ (!Bits[6] & !Bits[4])) XorMask ^= 0x40;
		if (Bits[7] ^ (!Bits[1] &  Bits[0])) XorMask ^= 0x80;

		GalZ80Rom1[Offset] ^= XorMask;
		GalZ80Rom1[Offset] = BITSWAP08(GalZ80Rom1[Offset], 6, 1, 2, 5, 4, 3, 0, 7);
	}
}

static INT32 BilliardInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = BilliardPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910;
	
	nRet = GalInit();
	FroggerSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

static void HustlerbPostLoad()
{
	MapScobra();
	
	ZetOpen(0);
	ZetSetReadHandler(HustlerbZ80Read);
	ZetSetWriteHandler(HustlerbZ80Write);
	ZetClose();
	
	FroggerSoundNoEncryptionInit();
	ZetOpen(1);
	ZetSetInHandler(HustlerbSoundZ80PortRead);
	ZetSetOutHandler(HustlerbSoundZ80PortWrite);
	ZetMapArea(0x8000, 0x83ff, 0, GalZ80Ram2);
	ZetMapArea(0x8000, 0x83ff, 1, GalZ80Ram2);
	ZetMapArea(0x8000, 0x83ff, 2, GalZ80Ram2);
	ZetClose();
}

static INT32 HustlerbInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = HustlerbPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910;
	
	nRet = GalInit();
		
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	KonamiPPIInit();
	
	GalSpriteClipStart = 7;
	GalSpriteClipEnd = 246;
	
	return nRet;
}

static void MapMimonkey()
{
	MapScobra();
	
	ZetOpen(0);
	ZetSetWriteHandler(MimonkeyZ80Write);
	ZetMemCallback(0x4000, 0x7fff, 0);
	ZetMemCallback(0x4000, 0x7fff, 2);
	ZetMapArea(0xc000, 0xffff, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0xc000, 0xffff, 2, GalZ80Rom1 + 0x4000);
	ZetClose();
}

static void MimonkeyPostLoad()
{
	static const UINT8 XorTable[16][16] = {
		{ 0x03, 0x03, 0x05, 0x07, 0x85, 0x00, 0x85, 0x85, 0x80, 0x80, 0x06, 0x03, 0x03, 0x00, 0x00, 0x81 },
		{ 0x83, 0x87, 0x03, 0x87, 0x06, 0x00, 0x06, 0x04, 0x02, 0x00, 0x84, 0x84, 0x04, 0x00, 0x01, 0x83 },
		{ 0x82, 0x82, 0x84, 0x02, 0x04, 0x00, 0x00, 0x03, 0x82, 0x00, 0x06, 0x80, 0x03, 0x00, 0x81, 0x07 },
		{ 0x06, 0x06, 0x82, 0x81, 0x85, 0x00, 0x04, 0x07, 0x81, 0x05, 0x04, 0x00, 0x03, 0x00, 0x82, 0x84 },
		{ 0x07, 0x07, 0x80, 0x07, 0x07, 0x00, 0x85, 0x86, 0x00, 0x07, 0x06, 0x04, 0x85, 0x00, 0x86, 0x85 },
		{ 0x81, 0x83, 0x02, 0x02, 0x87, 0x00, 0x86, 0x03, 0x04, 0x06, 0x80, 0x05, 0x87, 0x00, 0x81, 0x81 },
		{ 0x01, 0x01, 0x00, 0x07, 0x07, 0x00, 0x01, 0x01, 0x07, 0x07, 0x06, 0x00, 0x06, 0x00, 0x07, 0x07 },
		{ 0x80, 0x87, 0x81, 0x87, 0x83, 0x00, 0x84, 0x01, 0x01, 0x86, 0x86, 0x80, 0x86, 0x00, 0x86, 0x86 },
		{ 0x03, 0x03, 0x05, 0x07, 0x85, 0x00, 0x85, 0x85, 0x80, 0x80, 0x06, 0x03, 0x03, 0x00, 0x00, 0x81 },
		{ 0x83, 0x87, 0x03, 0x87, 0x06, 0x00, 0x06, 0x04, 0x02, 0x00, 0x84, 0x84, 0x04, 0x00, 0x01, 0x83 },
		{ 0x82, 0x82, 0x84, 0x02, 0x04, 0x00, 0x00, 0x03, 0x82, 0x00, 0x06, 0x80, 0x03, 0x00, 0x81, 0x07 },
		{ 0x06, 0x06, 0x82, 0x81, 0x85, 0x00, 0x04, 0x07, 0x81, 0x05, 0x04, 0x00, 0x03, 0x00, 0x82, 0x84 },
		{ 0x07, 0x07, 0x80, 0x07, 0x07, 0x00, 0x85, 0x86, 0x00, 0x07, 0x06, 0x04, 0x85, 0x00, 0x86, 0x85 },
		{ 0x81, 0x83, 0x02, 0x02, 0x87, 0x00, 0x86, 0x03, 0x04, 0x06, 0x80, 0x05, 0x87, 0x00, 0x81, 0x81 },
		{ 0x01, 0x01, 0x00, 0x07, 0x07, 0x00, 0x01, 0x01, 0x07, 0x07, 0x06, 0x00, 0x06, 0x00, 0x07, 0x07 },
		{ 0x80, 0x87, 0x81, 0x87, 0x83, 0x00, 0x84, 0x01, 0x01, 0x86, 0x86, 0x80, 0x86, 0x00, 0x86, 0x86 }
	};

	INT32 Ctr = 0, Line, Col;

	for (INT32 Offset = 0; Offset < 0x4000; Offset++) {
		Line = (Ctr & 0x07) | ((Ctr & 0x200) >> 6);
		Col = ((GalZ80Rom1[Offset] & 0x80) >> 4) | (GalZ80Rom1[Offset] & 0x07);
		GalZ80Rom1[Offset] = GalZ80Rom1[Offset] ^ XorTable[Line][Col];
		Ctr++;
	}
	
	MapMimonkey();
}

static INT32 MimonkeyInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MimonkeyPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;	
	
	nRet = GalInit();
	KonamiSoundInit();

	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = MimonkeyExtendTileInfo;
	GalExtendSpriteInfoFunction = MimonkeyExtendSpriteInfo;

	KonamiPPIInit();
	
	return nRet;
}

static INT32 MimonscoInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = MapMimonkey;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910;	
	
	nRet = GalInit();
	KonamiSoundInit();

	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = MimonkeyExtendTileInfo;
	GalExtendSpriteInfoFunction = MimonkeyExtendSpriteInfo;

	KonamiPPIInit();
	
	return nRet;
}

struct BurnDriver BurnDrvScobra = {
	"scobra", NULL, NULL, NULL, "1981",
	"Super Cobra\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScobraRomInfo, ScobraRomName, NULL, NULL, SfxInputInfo, ScobraDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScobras = {
	"scobras", "scobra", NULL, NULL, "1981",
	"Super Cobra (Stern)\0", NULL, "Konami (Stern license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScobrasRomInfo, ScobrasRomName, NULL, NULL, SfxInputInfo, ScobrasDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScobrase = {
	"scobrase", "scobra", NULL, NULL, "1981",
	"Super Cobra (Sega)\0", NULL, "Konami (Sega license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScobraseRomInfo, ScobraseRomName, NULL, NULL, SfxInputInfo, ScobrasDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvScobrab = {
	"scobrab", "scobra", NULL, NULL, "1981",
	"Super Cobra (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, ScobrabRomInfo, ScobrabRomName, NULL, NULL, SfxInputInfo, ScobrasDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSuprheli = {
	"suprheli", "scobra", NULL, NULL, "1981",
	"Super Heli (Super Cobra bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, SuprheliRomInfo, SuprheliRomName, NULL, NULL, SfxInputInfo, ScobrasDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvLosttomb = {
	"losttomb", NULL, NULL, NULL, "1982",
	"Lost Tomb (Easy)\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, LosttombRomInfo, LosttombRomName, NULL, NULL, LosttombInputInfo, LosttombDIPInfo,
	LosttombInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvLosttombh = {
	"losttombh", "losttomb", NULL, NULL, "1982",
	"Lost Tomb (Hard)\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, LosttombhRomInfo, LosttombhRomName, NULL, NULL, LosttombInputInfo, LosttombDIPInfo,
	LosttombInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvArmorcar = {
	"armorcar", NULL, NULL, NULL, "1981",
	"Armored Car (set 1)\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, ArmorcarRomInfo, ArmorcarRomName, NULL, NULL, AtlantisInputInfo, ArmorcarDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvArmorcar2 = {
	"armorcar2", "armorcar", NULL, NULL, "1981",
	"Armored Car (set 2)\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, Armorcar2RomInfo, Armorcar2RomName, NULL, NULL, AtlantisInputInfo, ArmorcarDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTazmania = {
	"tazmania", NULL, NULL, NULL, "1982",
	"Tazz-Mania (set 1)\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, TazmaniaRomInfo, TazmaniaRomName, NULL, NULL, TazmaniaInputInfo, ArmorcarDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTazmani2 = {
	"tazmani2", "tazmania", NULL, NULL, "1982",
	"Tazz-Mania (set 2)\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, Tazmani2RomInfo, Tazmani2RomName, NULL, NULL, TazmaniaInputInfo, ArmorcarDIPInfo,
	Tazmani2Init, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAnteater = {
	"anteater", NULL, NULL, NULL, "1982",
	"Anteater\0", NULL, "Stern (Tago license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AnteaterRomInfo, AnteaterRomName, NULL, NULL, AnteaterInputInfo, AnteaterDIPInfo,
	AnteaterInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAnteaterg = {
	"anteaterg", "anteater", NULL, NULL, "1983",
	"Ameisenbaer (German)\0", NULL, "TV-Tuning (F.E.G. license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AnteatergRomInfo, AnteatergRomName, NULL, NULL, AnteatergInputInfo, AnteatergDIPInfo,
	AnteatergInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAnteateruk = {
	"anteateruk", "anteater", NULL, NULL, "1983",
	"The Anteater (UK)\0", NULL, "Free Enterprise Games", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, AnteaterukRomInfo, AnteaterukRomName, NULL, NULL, AnteaterukInputInfo, AnteaterukDIPInfo,
	AnteaterukInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSpdcoin = {
	"spdcoin", NULL, NULL, NULL, "1984",
	"Speed Coin (prototype)\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_PROTOTYPE, 2, HARDWARE_GALAXIAN, GBF_MISC, 0,
	NULL, SpdcoinRomInfo, SpdcoinRomName, NULL, NULL, SpdcoinInputInfo, SpdcoinDIPInfo,
	ScobraInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvSuperbon = {
	"superbon", NULL, NULL, NULL, "1985",
	"Agent Super Bond (scobra hardware)\0", "Bad Colours", "Signaton USA", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, SuperbonRomInfo, SuperbonRomName, NULL, NULL, SuperbonInputInfo, SuperbonDIPInfo,
	SuperbonInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvCalipso = {
	"calipso", NULL, NULL, NULL, "1982",
	"Calipso\0", NULL, "Stern (Tago license)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_MAZE, 0,
	NULL, CalipsoRomInfo, CalipsoRomName, NULL, NULL, CalipsoInputInfo, CalipsoDIPInfo,
	CalipsoInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMoonwar = {
	"moonwar", NULL, NULL, NULL, "1981",
	"Moonwar\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SHOOT, 0,
	NULL, MoonwarRomInfo, MoonwarRomName, NULL, NULL, MoonwarInputInfo, MoonwarDIPInfo,
	MoonwarInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMoonwara = {
	"moonwara", "moonwar", NULL, NULL, "1981",
	"Moonwar (older)\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SHOOT, 0,
	NULL, MoonwaraRomInfo, MoonwaraRomName, NULL, NULL, MoonwarInputInfo, MoonwaraDIPInfo,
	MoonwarInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvStratgyx = {
	"stratgyx", NULL, NULL, NULL, "1981",
	"Strategy X\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, StratgyxRomInfo, StratgyxRomName, NULL, NULL, StratgyxInputInfo, StratgyxDIPInfo,
	StratgyxInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriver BurnDrvStratgys = {
	"stratgys", "stratgyx", NULL, NULL, "1981",
	"Strategy X (Stern)\0", NULL, "Konami (Stern License)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, StratgysRomInfo, StratgysRomName, NULL, NULL, StratgyxInputInfo, StratgyxDIPInfo,
	StratgyxInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriver BurnDrvStrongx = {
	"strongx", "stratgyx", NULL, NULL, "1982",
	"Strong X\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_VERSHOOT, 0,
	NULL, StrongxRomInfo, StrongxRomName, NULL, NULL, StratgyxInputInfo, StratgyxDIPInfo,
	StratgyxInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriver BurnDrvDarkplnt = {
	"darkplnt", NULL, NULL, NULL, "1982",
	"Dark Planet\0", "Dial doesn't work very well", "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SHOOT, 0,
	NULL, DarkplntRomInfo, DarkplntRomName, NULL, NULL, DarkplntInputInfo, DarkplntDIPInfo,
	DarkplntInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriver BurnDrvRescue = {
	"rescue", NULL, NULL, NULL, "1982",
	"Rescue\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, RescueRomInfo, RescueRomName, NULL, NULL, RescueInputInfo, RescueDIPInfo,
	RescueInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAponow = {
	"aponow", "rescue", NULL, NULL, "1982",
	"Apocaljpse Now\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, AponowRomInfo, AponowRomName, NULL, NULL, RescueInputInfo, RescueDIPInfo,
	RescueInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMinefld = {
	"minefld", NULL, NULL, NULL, "1983",
	"Minefield\0", NULL, "Stern", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, MinefldRomInfo, MinefldRomName, NULL, NULL, RescueInputInfo, MinefldDIPInfo,
	MinefldInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHustler = {
	"hustler", NULL, NULL, NULL, "1981",
	"Video Hustler\0", NULL, "Konami", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SPORTSMISC, 0,
	NULL, HustlerRomInfo, HustlerRomName, NULL, NULL, HustlerInputInfo, HustlerDIPInfo,
	HustlerInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvBilliard = {
	"billiard", "hustler", NULL, NULL, "1981",
	"The Billiards\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SPORTSMISC, 0,
	NULL, BilliardRomInfo, BilliardRomName, NULL, NULL, HustlerInputInfo, HustlerDIPInfo,
	BilliardInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHustlerb = {
	"hustlerb", "hustler", NULL, NULL, "1981",
	"Video Hustler (bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SPORTSMISC, 0,
	NULL, HustlerbRomInfo, HustlerbRomName, NULL, NULL, HustlerInputInfo, HustlerDIPInfo,
	HustlerbInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMimonkey = {
	"mimonkey", NULL, NULL, NULL, "198?",
	"Mighty Monkey\0", NULL, "Universal Video Games", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, MimonkeyRomInfo, MimonkeyRomName, NULL, NULL, AtlantisInputInfo, MimonkeyDIPInfo,
	MimonkeyInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvMimonsco = {
	"mimonsco", "mimonkey", NULL, NULL, "198?",
	"Mighty Monkey (bootleg on Super Cobra hardware)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, MimonscoRomInfo, MimonscoRomName, NULL, NULL, AtlantisInputInfo, MimonscoDIPInfo,
	MimonscoInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// Dambusters based hardware - background can take priority over all graphics except high colour chars
static struct BurnRomInfo DambustrRomDesc[] = {
	{ "db11a.pr11",    0x01000, 0x427bd3fb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db9a.pr9",      0x01000, 0x57164563, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db10a.pr10",    0x01000, 0x075b9c5e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db12a.pr12",    0x01000, 0xed01a68b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db8a.pr8",      0x01000, 0xfd041ff4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db6a.pr6",      0x01000, 0x448db54b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db7a.pr7",      0x01000, 0x675b1f5e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db5a.pr5",      0x01000, 0x75659ecc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "db3a.pr3",      0x01000, 0x9e9a9710, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db1a.pr1",      0x01000, 0x4cb964cd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db4a.pr3",      0x01000, 0xd9d2df33, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db2a.pr2",      0x01000, 0x0a0a6af5, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "mi-7603-5.ic4", 0x00020, 0xf131f92f, BRF_GRA | GAL_ROM_PROM },
	
	{ "mi-7603-5.ic3", 0x00020, 0xe2a54c47, BRF_OPT }, // timing?
};

STD_ROM_PICK(Dambustr)
STD_ROM_FN(Dambustr)

static struct BurnRomInfo DambustraRomDesc[] = {
	{ "db11.pr11",     0x01000, 0x427bd3fb, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db9a.pr9",      0x01000, 0x57164563, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db10a.pr10",    0x01000, 0x075b9c5e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db12a.pr12",    0x01000, 0xed01a68b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db8a.pr8",      0x01000, 0xfd041ff4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db6.pr6",       0x01000, 0x56d301a9, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db7a.pr7",      0x01000, 0x675b1f5e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db5a.pr5",      0x01000, 0x75659ecc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "db3a.rom",      0x01000, 0x2347e26e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db1a.pr1",      0x01000, 0x4cb964cd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db4a.pr3",      0x01000, 0xd9d2df33, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db2a.pr2",      0x01000, 0x0a0a6af5, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "mi-7603-5.ic4", 0x00020, 0xf131f92f, BRF_GRA | GAL_ROM_PROM },
	
	{ "mi-7603-5.ic3", 0x00020, 0xe2a54c47, BRF_OPT }, // timing?
};

STD_ROM_PICK(Dambustra)
STD_ROM_FN(Dambustra)

static struct BurnRomInfo DambustrukRomDesc[] = {
	{ "db11.bin",      0x01000, 0x9e6b34fe, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db9.pr9",       0x01000, 0x57164563, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db10p.bin",     0x01000, 0xc129c57b, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db12.bin",      0x01000, 0xea4c65f5, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db8.pr8",       0x01000, 0xfd041ff4, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db6p.bin",      0x01000, 0x35dcee01, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db7.pr7",       0x01000, 0x675b1f5e, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	{ "db5.pr5",       0x01000, 0x75659ecc, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },
	
	{ "db3.pr3",       0x01000, 0x9e9a9710, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db1ap.pr1",     0x01000, 0x4cb964cd, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db4.pr3",       0x01000, 0xd9d2df33, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "db2.pr2",       0x01000, 0x0a0a6af5, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "mi-7603-5.ic4", 0x00020, 0xf131f92f, BRF_GRA | GAL_ROM_PROM },
	
	{ "mi-7603-5.ic3", 0x00020, 0xe2a54c47, BRF_OPT }, // timing?
};

STD_ROM_PICK(Dambustruk)
STD_ROM_FN(Dambustruk)

UINT8 __fastcall DambustrZ80Read(UINT16 a)
{
	if (a >= 0xd900 && a <= 0xdbff) {
		// ???
		return 0xff;
	}
	
	switch (a) {
		case 0xe000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0xe800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0xf000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0xf800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall DambustrZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0xd800 && a <= 0xd8ff) {
		INT32 Offset = a - 0xd800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0xd900 && a <= 0xdbff) {
		// ???
		return;
	}
	
	switch (a) {
		case 0x8000: {
			DambustrBgColour1 = (BIT(d, 2) << 2) | (BIT(d, 1) << 1) | BIT(d, 0);
			DambustrBgColour2 = (BIT(d, 6) << 2) | (BIT(d, 5) << 1) | BIT(d, 4);
			DambustrBgPriority = BIT(d, 3);
			GalGfxBank[0] = BIT(d, 7);
			return;
		}
		
		case 0x8001: {
			DambustrBgSplitLine = d;
			return;
		}
				
		case 0xe002:
		case 0xe003: {
			// coin_count_0_w
			return;
		}
		
		case 0xe004:
		case 0xe005:
		case 0xe006:
		case 0xe007: {
			GalaxianLfoFreqWrite(a - 0xe004, d);
			return;
		}
		
		case 0xe800:
		case 0xe801:
		case 0xe802:
		case 0xe803:
		case 0xe804:
		case 0xe805:
		case 0xe806:
		case 0xe807: {
			GalaxianSoundWrite(a - 0xe800, d);
			return;
		}
		
		case 0xf001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xf004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xf006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xf007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xf800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall DambustrZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall DambustrZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static void DambustrPostLoad()
{
	UINT8 Temp;
	
	GalTempRom = (UINT8*)BurnMalloc(0x4000);
	memcpy(GalTempRom, GalZ80Rom1, 0x4000);
	for (INT32 i = 0; i < 0x4000; i++) {
		GalZ80Rom1[i] = GalTempRom[BITSWAP16(i, 15, 14, 13, 12, 4, 10, 9, 8, 7, 6, 5, 3, 11, 2, 1, 0)];
	}
	BurnFree(GalTempRom);
	
	for (INT32 i = 0; i < 0x1000; i++) {
		Temp = GalZ80Rom1[0x5000 + i];
		GalZ80Rom1[0x5000 + i] = GalZ80Rom1[0x6000 + i];
		GalZ80Rom1[0x6000 + i] = GalZ80Rom1[0x1000 + i];
		GalZ80Rom1[0x1000 + i] = Temp;
	}
	
	for (INT32 i = 0; i < 0x1000; i++) {
		GalZ80Rom1[0x1000 + i] = BITSWAP08(GalZ80Rom1[0x1000 + i], 7, 6, 5, 1, 3, 2, 4, 0);
		GalZ80Rom1[0x4000 + i] = BITSWAP08(GalZ80Rom1[0x4000 + i], 7, 6, 5, 1, 3, 2, 4, 0);
		GalZ80Rom1[0x5000 + i] = BITSWAP08(GalZ80Rom1[0x5000 + i], 7, 6, 5, 1, 3, 2, 4, 0);
	}
	
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(DambustrZ80Read);
	ZetSetWriteHandler(DambustrZ80Write);
	ZetSetInHandler(DambustrZ80PortRead);
	ZetSetOutHandler(DambustrZ80PortWrite);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x8000) ? 0x7fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x8000) ? 0x7fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0xc000, 0xc7ff, 0, GalZ80Ram1);
	ZetMapArea(0xc000, 0xc7ff, 1, GalZ80Ram1);
	ZetMapArea(0xc000, 0xc7ff, 2, GalZ80Ram1);
	ZetMapArea(0xd000, 0xd3ff, 0, GalVideoRam);
	ZetMapArea(0xd000, 0xd3ff, 1, GalVideoRam);
	ZetMapArea(0xd000, 0xd3ff, 2, GalVideoRam);
	ZetMapArea(0xd400, 0xd7ff, 0, GalVideoRam);
	ZetMapArea(0xd400, 0xd7ff, 1, GalVideoRam);
	ZetMapArea(0xd400, 0xd7ff, 2, GalVideoRam);
	ZetMapArea(0xd800, 0xd8ff, 0, GalSpriteRam);
	ZetMapArea(0xd800, 0xd8ff, 2, GalSpriteRam);
	ZetMemEnd();
	ZetClose();
}

static INT32 DambustrInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = DambustrPostLoad;
	
	nRet = GalInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	UINT8 Temp[16];
	nRet = BurnLoadRom(GalTempRom + 0x0000, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x1000, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x2000, GAL_ROM_OFFSET_TILES_SHARED + 2, 1); if (nRet) return 1;
	nRet = BurnLoadRom(GalTempRom + 0x3000, GAL_ROM_OFFSET_TILES_SHARED + 3, 1); if (nRet) return 1;
	for (UINT32 i = 0; i < GalTilesSharedRomSize; i += 16) {
		UINT32 j;
		
		for (j = 0; j < 16; j++) {
			Temp[j] = GalTempRom[i + j];
		}
		
		for (j = 0; j < 8; j++) {
			GalTempRom[i + j + 0] = Temp[(j * 2) + 0];
			GalTempRom[i + j + 8] = Temp[(j * 2) + 1];
		}
	}
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);

	GalCalcPaletteFunction = DambustrCalcPalette;
	GalRenderBackgroundFunction = DambustrDrawBackground;
	GalDrawBulletsFunction = DambustrDrawBullets;
	GalExtendTileInfoFunction = DambustrExtendTileInfo;
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;
	GalRenderFrameFunction = DambustrRenderFrame;
	
	GalSpriteClipStart = 0;
	GalSpriteClipEnd = 255;
	
	GalVideoRam2 = (UINT8*)BurnMalloc(0x400);

	return nRet;
}

struct BurnDriver BurnDrvDambustr = {
	"dambustr", NULL, NULL, NULL, "1981",
	"Dambusters (US, set 1)\0", NULL, "South West Research", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, DambustrRomInfo, DambustrRomName, NULL, NULL, DambustrInputInfo, DambustrDIPInfo,
	DambustrInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvDambustra = {
	"dambustra", "dambustr", NULL, NULL, "1981",
	"Dambusters (US, set 2)\0", NULL, "South West Research", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, DambustraRomInfo, DambustraRomName, NULL, NULL, DambustrInputInfo, DambustrDIPInfo,
	DambustrInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvDambustruk = {
	"dambustruk", "dambustr", NULL, NULL, "1981",
	"Dambusters (UK)\0", NULL, "South West Research", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_GALAXIAN, GBF_HORSHOOT, 0,
	NULL, DambustrukRomInfo, DambustrukRomName, NULL, NULL, DambustrInputInfo, DambustrDIPInfo,
	DambustrInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

// S2650 Based Games
static struct BurnRomInfo HunchbkgRomDesc[] = {
	{ "gal_hb_kl",     0x00800, 0x3977650e, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "gal_hb_hj",     0x00800, 0xdb489c3d, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "gal_hb_cp",     0x00020, 0xcbff6762, BRF_GRA | GAL_ROM_PROM },
	
	{ "gal_hb_1",      0x00800, 0x46590e9b, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "gal_hb_2",      0x00800, 0x4e6e671c, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "gal_hb_3",      0x00800, 0xd29dc242, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "gal_hb_4",      0x00800, 0xd409d292, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "gal_hb_5",      0x00800, 0x29d3a8c4, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "gal_hb_6",      0x00800, 0xb016fd15, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "gal_hb_7",      0x00800, 0xd2731d27, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "gal_hb_8",      0x00800, 0xe4b1a666, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
};

STD_ROM_PICK(Hunchbkg)
STD_ROM_FN(Hunchbkg)

static struct BurnRomInfo DrivfrcgRomDesc[] = {
	{ "dfgj2.bin",     0x01000, 0x8e19f1e7, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "dfgj1.bin",     0x01000, 0x86b60ca8, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "dfgl2.bin",     0x01000, 0xea5e9959, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "dfgl1.bin",     0x01000, 0xb7ed195c, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "top.clr",       0x00020, 0x3110ddae, BRF_GRA | GAL_ROM_PROM },
	{ "bot.clr",       0x00020, 0x0f0782af, BRF_GRA | GAL_ROM_PROM },
				
	{ "dfgp1.bin",     0x01000, 0x52d5e77d, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "dfgp2.bin",     0x01000, 0x9cf4dbce, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "dfgp3.bin",     0x01000, 0x79763f62, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "dfgp4.bin",     0x01000, 0xdd95338b, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
};

STD_ROM_PICK(Drivfrcg)
STD_ROM_FN(Drivfrcg)

static struct BurnRomInfo DrivfrcbRomDesc[] = {
	{ "df1.bin",       0x02000, 0x8adc3de0, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "df2.bin",       0x02000, 0x6d95ec35, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "top.clr",       0x00020, 0x3110ddae, BRF_GRA | GAL_ROM_PROM },
	{ "bot.clr",       0x00020, 0x0f0782af, BRF_GRA | GAL_ROM_PROM },
				
	{ "dfp.bin",       0x04000, 0xb5b2981d, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
};

STD_ROM_PICK(Drivfrcb)
STD_ROM_FN(Drivfrcb)

static struct BurnRomInfo RacknrolRomDesc[] = {
	{ "horz_g.bin",    0x04000, 0x97069ad5, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "horz_r.bin",    0x04000, 0xff64e84b, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.bin",    0x00020, 0x737802bf, BRF_GRA | GAL_ROM_PROM },
	
	{ "horz_p.bin",    0x04000, 0x32ca5b43, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	
	{ "82s147.bin",    0x00200, 0xaace7fa5, BRF_OPT }, // unknown
};

STD_ROM_PICK(Racknrol)
STD_ROM_FN(Racknrol)

static struct BurnRomInfo HexpoolRomDesc[] = {
	{ "vert_g.bin",    0x04000, 0x7e257e80, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "vert_r.bin",    0x04000, 0xc5f0851e, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.bin",    0x00020, 0x737802bf, BRF_GRA | GAL_ROM_PROM },
	
	{ "vert_p.bin",    0x04000, 0xbdb078fc, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	
	{ "82s147.bin",    0x00200, 0xaace7fa5, BRF_OPT }, // unknown
};

STD_ROM_PICK(Hexpool)
STD_ROM_FN(Hexpool)

static struct BurnRomInfo HexpoolaRomDesc[] = {
	{ "rom.1m",        0x04000, 0x7e257e80, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "rom.1l",        0x04000, 0xc5f0851e, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "82s123.11r",    0x00020, 0xdeb2fcf4, BRF_GRA | GAL_ROM_PROM },
	
	{ "rom.4l",        0x04000, 0x2ca8018d, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	
	{ "82s147.5pr",    0x00200, 0xcf496b1e, BRF_OPT }, // unknown
	
	{ "82s153.6pr.bin",0x000eb, 0xbc07939a, BRF_OPT }, // PLD
};

STD_ROM_PICK(Hexpoola)
STD_ROM_FN(Hexpoola)

static struct BurnRomInfo HunchbksRomDesc[] = {
	{ "11d_snd.bin",   0x00800, 0x88226086, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },

	{ "5f_hb09.bin",   0x00800, 0xdb489c3d, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "5h_hb10.bin",   0x00800, 0x3977650e, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "6e_prom.bin",   0x00020, 0x01004d3f, BRF_GRA | GAL_ROM_PROM },
	
	{ "2c_hb01.bin",   0x00800, 0x8bebd834, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "2e_hb02.bin",   0x00800, 0x07de4229, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "2f_hb03.bin",   0x00800, 0xb75a0dfc, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "2h_hb04.bin",   0x00800, 0xf3206264, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "2j_hb05.bin",   0x00800, 0x1bb78728, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "2l_hb06.bin",   0x00800, 0xf25ed680, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "2m_hb07.bin",   0x00800, 0xc72e0e17, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "2p_hb08.bin",   0x00800, 0x412087b0, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
};

STD_ROM_PICK(Hunchbks)
STD_ROM_FN(Hunchbks)

static struct BurnRomInfo HncholmsRomDesc[] = {
	{ "hncholym.5c",   0x00800, 0xe7758775, BRF_ESS | BRF_PRG | GAL_ROM_Z80_PROG1 },

	{ "hncholym.5f",   0x01000, 0x75ad3542, BRF_GRA | GAL_ROM_TILES_SHARED },
	{ "hncholym.5h",   0x01000, 0x6fec9dd3, BRF_GRA | GAL_ROM_TILES_SHARED },
		
	{ "prom.6e",       0x00020, 0x4e3caeab, BRF_GRA | GAL_ROM_PROM },
	
	{ "hncholym.2d",   0x00800, 0xfb453f9c, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "hncholym.2e",   0x00800, 0xb1429420, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "hncholym.2f",   0x00800, 0xafc98e28, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "hncholym.2h",   0x00800, 0x6785bf17, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "hncholym.2j",   0x00800, 0x0e1e4133, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "hncholym.2l",   0x00800, 0x6e982609, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "hncholym.2m",   0x00800, 0xb9141914, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	{ "hncholym.2p",   0x00800, 0xca37b55b, BRF_ESS | BRF_PRG | GAL_ROM_S2650_PROG1 },
	
	{ "82s147.1a",     0x00200, 0xd461a48b, BRF_OPT },
};

STD_ROM_PICK(Hncholms)
STD_ROM_FN(Hncholms)

UINT8 HunchbkgS2650Read(UINT16 Address)
{
	switch (Address) {
		case 0x1500:
		case 0x3500:
		case 0x5500:
		case 0x7500: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x1580:
		case 0x3580:
		case 0x5580:
		case 0x7580: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x1600:
		case 0x3600:
		case 0x5600:
		case 0x7600: {
		 	return GalInput[2] | GalDip[2];
		}
		
		case 0x1680:
		case 0x3680:
		case 0x5680:
		case 0x7680: {
			// ???
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Read %04x\n"), Address);
		}
	}

	return 0;
}

void HunchbkgS2650Write(UINT16 Address, UINT8 Data)
{
	if ((Address >= 0x1480 && Address <= 0x14ff) || (Address >= 0x3480 && Address <= 0x34ff)  || (Address >= 0x5480 && Address <= 0x54ff)  || (Address >= 0x7480 && Address <= 0x74ff)) {
		INT32 Offset = Address & 0x7f;
		
		GalSpriteRam[Offset] = Data;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = Data;
			}
		}
		
		return;
	}
	
	switch (Address) {
		case 0x1503:
		case 0x3503:
		case 0x5503:
		case 0x7503: {
			// coin_count_0_w
			return;
		}
		
		case 0x1580:
		case 0x1581:
		case 0x1582:{
			GalaxianSoundWrite(Address - 0x1580, Data);
			return;
		}
		
		case 0x3580:
		case 0x3581:
		case 0x3582:{
			GalaxianSoundWrite(Address - 0x3580, Data);
			return;
		}
		
		case 0x5580:
		case 0x5581:
		case 0x5582:{
			GalaxianSoundWrite(Address - 0x5580, Data);
			return;
		}
		
		case 0x7580:
		case 0x7581:
		case 0x7582:{
			GalaxianSoundWrite(Address - 0x7580, Data);
			return;
		}
		
		case 0x1583:
		case 0x3583:
		case 0x5583:
		case 0x7583: {
			// nop
			return;
		}
		
		case 0x1584:
		case 0x1585:
		case 0x1586:
		case 0x1587: {
			GalaxianLfoFreqWrite(Address - 0x1584, Data);
			return;
		}
		
		case 0x3584:
		case 0x3585:
		case 0x3586:
		case 0x3587: {
			GalaxianLfoFreqWrite(Address - 0x3584, Data);
			return;
		}
		
		case 0x5584:
		case 0x5585:
		case 0x5586:
		case 0x5587: {
			GalaxianLfoFreqWrite(Address - 0x5584, Data);
			return;
		}
		
		case 0x7584:
		case 0x7585:
		case 0x7586:
		case 0x7587: {
			GalaxianLfoFreqWrite(Address - 0x7584, Data);
			return;
		}
		
		case 0x1600:
		case 0x3600:
		case 0x5600:
		case 0x7600: {
			// nop
			return;
		}
		
		case 0x1601:
		case 0x3601:
		case 0x5601:
		case 0x7601: {
			// nop
			return;
		}
		
		case 0x1604:
		case 0x3604:
		case 0x5604:
		case 0x7604: {
			// nop
			return;
		}
		
		case 0x1606:
		case 0x3606:
		case 0x5606:
		case 0x7606: {
			GalFlipScreenX = Data & 1;
			return;
		}
		
		case 0x1607:
		case 0x3607:
		case 0x5607:
		case 0x7607: {
			GalFlipScreenY = Data & 1;
			return;
		}
		
		case 0x1680:
		case 0x3680:
		case 0x5680:
		case 0x7680: {
			GalPitch = Data;
			return;
		}
				
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Write %04x, %02x\n"), Address, Data);
		}
	}
}

UINT8 HunchbkgS2650PortRead(UINT16 Port)
{
	switch (Port) {
		case S2650_DATA_PORT: {
			// nop
			return 0;
		}
		
		case S2650_SENSE_PORT: {
			return GalVBlank ? 0x80 : 0x00;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Read %04x\n"), Port);
		}
	}

	return 0;
}

void HunchbkgS2650PortWrite(UINT16 Port, UINT8 Data)
{
	switch (Port) {
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Write %04x, %02x\n"), Port, Data);
		}
	}
}

UINT8 DrivfrcgS2650Read(UINT16 Address)
{
	switch (Address) {
		case 0x1500:
		case 0x3500:
		case 0x5500:
		case 0x7500: {
		 	return GalInput[0] | GalDip[0];
		}
		
		case 0x1580:
		case 0x3580:
		case 0x5580:
		case 0x7580: {
		 	return GalInput[1] | GalDip[1];
		}
		
		case 0x1600:
		case 0x3600:
		case 0x5600:
		case 0x7600: {
		 	return GalInput[2] | GalDip[2];
		}
		
		case 0x1700:
		case 0x3700:
		case 0x5700:
		case 0x7700: {
		 	return GalInput[3] | GalDip[3];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Read %04x\n"), Address);
		}
	}

	return 0;
}

void DrivfrcgS2650Write(UINT16 Address, UINT8 Data)
{
	if ((Address >= 0x1480 && Address <= 0x14ff) || (Address >= 0x3480 && Address <= 0x34ff)  || (Address >= 0x5480 && Address <= 0x54ff)  || (Address >= 0x7480 && Address <= 0x74ff)) {
		INT32 Offset = Address & 0x7f;
		
		GalSpriteRam[Offset] = Data;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = Data;
			}
		}
		
		return;
	}
	
	switch (Address) {
		case 0x1503:
		case 0x3503:
		case 0x5503:
		case 0x7503: {
			// coin_count_0_w
			return;
		}
		
		case 0x1580:
		case 0x1581:
		case 0x1582:{
			GalaxianSoundWrite(Address - 0x1580, Data);
			return;
		}
		
		case 0x3580:
		case 0x3581:
		case 0x3582:{
			GalaxianSoundWrite(Address - 0x3580, Data);
			return;
		}
		
		case 0x5580:
		case 0x5581:
		case 0x5582:{
			GalaxianSoundWrite(Address - 0x5580, Data);
			return;
		}
		
		case 0x7580:
		case 0x7581:
		case 0x7582:{
			GalaxianSoundWrite(Address - 0x7580, Data);
			return;
		}
		
		case 0x1583:
		case 0x3583:
		case 0x5583:
		case 0x7583: {
			// nop
			return;
		}
		
		case 0x1585:
		case 0x3585:
		case 0x5585:
		case 0x7585: {
			// nop
			return;
		}
		
		case 0x1586:
		case 0x1587: {
			GalaxianLfoFreqWrite(Address - 0x1586, Data);
			return;
		}
		
		case 0x3586:
		case 0x3587: {
			GalaxianLfoFreqWrite(Address - 0x3586, Data);
			return;
		}
		
		case 0x5586:
		case 0x5587: {
			GalaxianLfoFreqWrite(Address - 0x5586, Data);
			return;
		}
		
		case 0x7586:
		case 0x7587: {
			GalaxianLfoFreqWrite(Address - 0x7586, Data);
			return;
		}
		
		case 0x1600:
		case 0x3600:
		case 0x5600:
		case 0x7600: {
			GalPitch = Data;
			return;
		}
		
		case 0x1700:
		case 0x1701: {
			// nop
			return;
		}
		
		case 0x1704:
		case 0x1705:
		case 0x1706:
		case 0x1707:
		case 0x3704:
		case 0x3705:
		case 0x3706:
		case 0x3707:
		case 0x5704:
		case 0x5705:
		case 0x5706:
		case 0x5707:
		case 0x7704:
		case 0x7705:
		case 0x7706:
		case 0x7707: {
			INT32 Offset = Address & 0x03;
			GalVol = (GalVol & ~(1 << Offset)) | ((Data & 1) << Offset);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Write %04x, %02x\n"), Address, Data);
		}
	}
}

UINT8 DrivfrcgS2650PortRead(UINT16 Port)
{
	switch (Port) {
		case 0x00: {
			switch (s2650GetPc()) {
				case 0x02e:
				case 0x297: {
					return 0x01;
				}
			}
			bprintf(PRINT_NORMAL, _T("%x\n"), s2650GetPc());			
			return 0x00;
		}
		
		case S2650_SENSE_PORT: {
			return GalVBlank ? 0x80 : 0x00;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Read %04x\n"), Port);
		}
	}

	return 0;
}

void DrivfrcgS2650PortWrite(UINT16 Port, UINT8 Data)
{
	switch (Port) {
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Write %04x, %02x\n"), Port, Data);
		}
	}
}

UINT8 RacknrolS2650Read(UINT16 Address)
{
	switch (Address) {
		case 0x1500:
		case 0x3500:
		case 0x5500:
		case 0x7500: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x1580:
		case 0x3580:
		case 0x5580:
		case 0x7580: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x1600:
		case 0x3600:
		case 0x5600:
		case 0x7600: {
		 	return GalInput[2] | GalDip[2];
		}
		
		case 0x1680:
		case 0x3680:
		case 0x5680:
		case 0x7680: {
			// ???
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Read %04x\n"), Address);
		}
	}

	return 0;
}

void RacknrolS2650Write(UINT16 Address, UINT8 Data)
{
	if ((Address >= 0x1400 && Address <= 0x14ff) || (Address >= 0x3400 && Address <= 0x34ff)  || (Address >= 0x5400 && Address <= 0x54ff)  || (Address >= 0x7400 && Address <= 0x74ff)) {
		INT32 Offset = Address & 0xff;
		
		GalSpriteRam[Offset] = Data;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = Data;
			}
		}
		
		return;
	}
	
	switch (Address) {
		case 0x1601:
		case 0x3601:
		case 0x5601:
		case 0x7601: {
			// nop
			return;
		}
		
		case 0x1606:
		case 0x3606:
		case 0x5606:
		case 0x7606: {
			GalFlipScreenX = Data & 1;
			return;
		}
		
		case 0x1607:
		case 0x3607:
		case 0x5607:
		case 0x7607: {
			GalFlipScreenY = Data & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Write %04x, %02x\n"), Address, Data);
		}
	}
}

UINT8 RacknrolS2650PortRead(UINT16 Port)
{
	switch (Port) {
		case S2650_SENSE_PORT: {
			return GalVBlank ? 0x80 : 0x00;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Read %04x\n"), Port);
		}
	}

	return 0;
}

void RacknrolS2650PortWrite(UINT16 Port, UINT8 Data)
{
	if (Port >= 0x0020 && Port <= 0x003f) {
		GalGfxBank[Port - 0x0020] = Data;
		return;
	}
	
	switch (Port) {
		case 0x001d: {
			SN76496Write(0, Data);
			return;
		}
		
		case 0x001e: {
			SN76496Write(1, Data);
			return;
		}
		
		case 0x001f: {
			SN76496Write(2, Data);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Write %04x, %02x\n"), Port, Data);
		}
	}
}

UINT8 HexpoolaS2650PortRead(UINT16 Port)
{
	switch (Port) {
		case 0x0000: {
			// nop
			return 0;
		}
		
		case S2650_DATA_PORT: {
			switch (s2650GetPc()) {
				case 0x22: return 0x00;
				case 0x31: return 0x01;
			}
			return 0;
		}
		
		case S2650_SENSE_PORT: {
			return GalVBlank ? 0x80 : 0x00;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Read %04x\n"), Port);
		}
	}

	return 0;
}

void HexpoolaS2650PortWrite(UINT16 Port, UINT8 Data)
{
	if (Port >= 0x0020 && Port <= 0x003f) {
		GalGfxBank[Port - 0x0020] = Data;
		return;
	}
	
	switch (Port) {
		case S2650_DATA_PORT: {
			SN76496Write(0, Data);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Write %04x, %02x\n"), Port, Data);
		}
	}
}

UINT8 HunchbksS2650Read(UINT16 Address)
{
	switch (Address) {
		case 0x1210:
		case 0x1211:
		case 0x1212:
		case 0x1213:
		case 0x3210:
		case 0x3211:
		case 0x3212:
		case 0x3213:
		case 0x5210:
		case 0x5211:
		case 0x5212:
		case 0x5213:
		case 0x7210:
		case 0x7211:
		case 0x7212:
		case 0x7213: {
			return ppi8255_r(0, Address & 0x03);
		}
		
		case 0x1500:
		case 0x1501:
		case 0x1502:
		case 0x1503:
		case 0x3500:
		case 0x3501:
		case 0x3502:
		case 0x3503:
		case 0x5500:
		case 0x5501:
		case 0x5502:
		case 0x5503:
		case 0x7500:
		case 0x7501:
		case 0x7502:
		case 0x7503: {
			return ppi8255_r(0, Address & 0x03);
		}
		
		case 0x1680:
		case 0x3680:
		case 0x5680:
		case 0x7680: {
			// watchdog read
			return 0;
		}
		
		case 0x1780:
		case 0x3780:
		case 0x5780:
		case 0x7780: {
			// watchdog read
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Read %04x\n"), Address);
		}
	}

	return 0;
}

void HunchbksS2650Write(UINT16 Address, UINT8 Data)
{
	if ((Address >= 0x1400 && Address <= 0x14ff) || (Address >= 0x3400 && Address <= 0x34ff)  || (Address >= 0x5400 && Address <= 0x54ff)  || (Address >= 0x7400 && Address <= 0x74ff)) {
		INT32 Offset = Address & 0xff;
		
		GalSpriteRam[Offset] = Data;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = Data;
			}
		}
		
		return;
	}
	
	switch (Address) {
		case 0x1210:
		case 0x1211:
		case 0x1212:
		case 0x1213:
		case 0x3210:
		case 0x3211:
		case 0x3212:
		case 0x3213:
		case 0x5210:
		case 0x5211:
		case 0x5212:
		case 0x5213:
		case 0x7210:
		case 0x7211:
		case 0x7212:
		case 0x7213: {
			ppi8255_w(1, Address & 0x03, Data);
			return;
		}
		
		case 0x1500:
		case 0x1501:
		case 0x1502:
		case 0x1503:
		case 0x3500:
		case 0x3501:
		case 0x3502:
		case 0x3503:
		case 0x5500:
		case 0x5501:
		case 0x5502:
		case 0x5503:
		case 0x7500:
		case 0x7501:
		case 0x7502:
		case 0x7503: {
			ppi8255_w(0, Address & 0x03, Data);
			return;
		}
		
		case 0x1600:
		case 0x3600:
		case 0x5600:
		case 0x7600: {
			// nop
			return;
		}
		
		case 0x1601:
		case 0x3601:
		case 0x5601:
		case 0x7601: {
			// nop
			return;
		}
		
		case 0x1606:
		case 0x3606:
		case 0x5606:
		case 0x7606: {
			GalFlipScreenX = Data & 1;
			return;
		}
		
		case 0x1607:
		case 0x3607:
		case 0x5607:
		case 0x7607: {
			GalFlipScreenY = Data & 1;
			return;
		}
		
		case 0x1700:
		case 0x1701: {
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Write %04x, %02x\n"), Address, Data);
		}
	}
}

UINT8 HunchbksS2650PortRead(UINT16 Port)
{
	switch (Port) {
		case S2650_DATA_PORT: {
			// nop
			return 0;
		}
		
		case S2650_SENSE_PORT: {
			return GalVBlank ? 0x80 : 0x00;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Read %04x\n"), Port);
		}
	}

	return 0;
}

void HunchbksS2650PortWrite(UINT16 Port, UINT8 Data)
{
	switch (Port) {
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Write %04x, %02x\n"), Port, Data);
		}
	}
}

UINT8 HncholmsS2650PortRead(UINT16 Port)
{
	switch (Port) {
		case 0x0000: {
			switch (s2650GetPc()) {
				case 0x02b: return 1;
				case 0xa27: return 1;
			}
			return 0;
		}
		
		case S2650_DATA_PORT: {
			// nop
			return 0;
		}
		
		case S2650_SENSE_PORT: {
			return GalVBlank ? 0x80 : 0x00;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("S2650 #1 Port Read %04x\n"), Port);
		}
	}

	return 0;
}

static INT32 GalS2650ReturnVector(INT32)
{
	return 0x03;
}

static void S2650PostLoad()
{
	s2650Init(1);
	s2650Open(0);
	s2650MapMemory(GalS2650Rom1 + 0x0000, 0x0000, 0x0fff, S2650_ROM);
	s2650MapMemory(GalSpriteRam, 0x1480, 0x14ff, S2650_READ);
	s2650MapMemory(GalVideoRam, 0x1800, 0x1bff, S2650_RAM);
	s2650MapMemory(GalZ80Ram1, 0x1c00, 0x1fff, S2650_RAM);
	s2650MapMemory(GalS2650Rom1 + 0x1000, 0x2000, 0x2fff, S2650_ROM);
	s2650MapMemory(GalSpriteRam, 0x3480, 0x34ff, S2650_READ);
	s2650MapMemory(GalVideoRam, 0x3800, 0x3bff, S2650_RAM);
	s2650MapMemory(GalZ80Ram1, 0x3c00, 0x3fff, S2650_RAM);
	s2650MapMemory(GalS2650Rom1 + 0x2000, 0x4000, 0x4fff, S2650_ROM);
	s2650MapMemory(GalSpriteRam, 0x5480, 0x54ff, S2650_READ);
	s2650MapMemory(GalVideoRam, 0x5800, 0x5bff, S2650_RAM);
	s2650MapMemory(GalZ80Ram1, 0x5c00, 0x5fff, S2650_RAM);
	s2650MapMemory(GalS2650Rom1 + 0x3000, 0x6000, 0x6fff, S2650_ROM);
	s2650MapMemory(GalSpriteRam, 0x7480, 0x74ff, S2650_READ);
	s2650MapMemory(GalVideoRam, 0x7800, 0x7bff, S2650_RAM);
	s2650MapMemory(GalZ80Ram1, 0x7c00, 0x7fff, S2650_RAM);
	s2650SetReadHandler(HunchbkgS2650Read);
	s2650SetWriteHandler(HunchbkgS2650Write);
	s2650SetInHandler(HunchbkgS2650PortRead);
	s2650SetOutHandler(HunchbkgS2650PortWrite);
	s2650SetIrqCallback(GalS2650ReturnVector);
	s2650Close();
}

static INT32 HunchbkgInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = S2650PostLoad;
	
	nRet = GalInit();
	
	return nRet;
}

static void DrivfrcgPostLoad()
{
	GalTempRom = (UINT8*)BurnMalloc(GalS2650Rom1Size);
	memcpy(GalTempRom, GalS2650Rom1, GalS2650Rom1Size);
	memcpy(GalS2650Rom1 + 0x1800, GalTempRom + 0x0000, 0x400);
	memcpy(GalS2650Rom1 + 0x1c00, GalTempRom + 0x0400, 0x400);
	memcpy(GalS2650Rom1 + 0x0000, GalTempRom + 0x0800, 0x400);
	memcpy(GalS2650Rom1 + 0x0400, GalTempRom + 0x0c00, 0x400);
	memcpy(GalS2650Rom1 + 0x0800, GalTempRom + 0x1000, 0x400);
	memcpy(GalS2650Rom1 + 0x0c00, GalTempRom + 0x1400, 0x400);
	memcpy(GalS2650Rom1 + 0x1000, GalTempRom + 0x1800, 0x400);
	memcpy(GalS2650Rom1 + 0x1400, GalTempRom + 0x1c00, 0x400);
	memcpy(GalS2650Rom1 + 0x3800, GalTempRom + 0x2000, 0x400);
	memcpy(GalS2650Rom1 + 0x3c00, GalTempRom + 0x2400, 0x400);
	memcpy(GalS2650Rom1 + 0x2000, GalTempRom + 0x2800, 0x400);
	memcpy(GalS2650Rom1 + 0x2400, GalTempRom + 0x2c00, 0x400);
	memcpy(GalS2650Rom1 + 0x2800, GalTempRom + 0x3000, 0x400);
	memcpy(GalS2650Rom1 + 0x2c00, GalTempRom + 0x3400, 0x400);
	memcpy(GalS2650Rom1 + 0x3000, GalTempRom + 0x3800, 0x400);
	memcpy(GalS2650Rom1 + 0x3400, GalTempRom + 0x3c00, 0x400);
	BurnFree(GalTempRom);
	
	S2650PostLoad();
	s2650Open(0);
	s2650SetReadHandler(DrivfrcgS2650Read);
	s2650SetWriteHandler(DrivfrcgS2650Write);
	s2650SetInHandler(DrivfrcgS2650PortRead);
	s2650SetOutHandler(DrivfrcgS2650PortWrite);
	s2650Close();
}

static INT32 DrivfrcgInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = DrivfrcgPostLoad;
	
	nRet = GalInit();
	
	GalCalcPaletteFunction = RockclimCalcPalette;
	GalExtendTileInfoFunction = Ad2083ExtendTileInfo;
	GalExtendSpriteInfoFunction = MshuttleExtendSpriteInfo;
	
	return nRet;
}

static INT32 DrivfrcbInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = DrivfrcgPostLoad;
	
	nRet = GalInit();
	
	GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x2000);
	nRet = BurnLoadRom(TempRom, GAL_ROM_OFFSET_TILES_SHARED + 0, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x1000, TempRom + 0x0000, 0x1000);
	memcpy(GalTempRom + 0x0000, TempRom + 0x1000, 0x1000);
	nRet = BurnLoadRom(TempRom, GAL_ROM_OFFSET_TILES_SHARED + 1, 1); if (nRet) return 1;
	memcpy(GalTempRom + 0x3000, TempRom + 0x0000, 0x1000);
	memcpy(GalTempRom + 0x2000, TempRom + 0x1000, 0x1000);
	GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
	GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);
	BurnFree(GalTempRom);
	BurnFree(TempRom);
	
	GalCalcPaletteFunction = RockclimCalcPalette;
	GalExtendTileInfoFunction = Ad2083ExtendTileInfo;
	GalExtendSpriteInfoFunction = MshuttleExtendSpriteInfo;
	
	return nRet;
}

static void RacknrolPostLoad()
{
	s2650Init(1);
	s2650Open(0);
	s2650MapMemory(GalS2650Rom1 + 0x0000, 0x0000, 0x0fff, S2650_ROM);
	s2650MapMemory(GalSpriteRam, 0x1400, 0x14ff, S2650_READ);
	s2650MapMemory(GalVideoRam, 0x1800, 0x1bff, S2650_RAM);
	s2650MapMemory(GalZ80Ram1, 0x1c00, 0x1fff, S2650_RAM);
	s2650MapMemory(GalS2650Rom1 + 0x1000, 0x2000, 0x2fff, S2650_ROM);
	s2650MapMemory(GalSpriteRam, 0x3400, 0x34ff, S2650_READ);
	s2650MapMemory(GalVideoRam, 0x3800, 0x3bff, S2650_RAM);
	s2650MapMemory(GalZ80Ram1, 0x3c00, 0x3fff, S2650_RAM);
	s2650MapMemory(GalS2650Rom1 + 0x2000, 0x4000, 0x4fff, S2650_ROM);
	s2650MapMemory(GalSpriteRam, 0x5400, 0x54ff, S2650_READ);
	s2650MapMemory(GalVideoRam, 0x5800, 0x5bff, S2650_RAM);
	s2650MapMemory(GalZ80Ram1, 0x5c00, 0x5fff, S2650_RAM);
	s2650MapMemory(GalS2650Rom1 + 0x3000, 0x6000, 0x6fff, S2650_ROM);
	s2650MapMemory(GalSpriteRam, 0x7400, 0x74ff, S2650_READ);
	s2650MapMemory(GalVideoRam, 0x7800, 0x7bff, S2650_RAM);
	s2650MapMemory(GalZ80Ram1, 0x7c00, 0x7fff, S2650_RAM);
	s2650SetReadHandler(RacknrolS2650Read);
	s2650SetWriteHandler(RacknrolS2650Write);
	s2650SetInHandler(RacknrolS2650PortRead);
	s2650SetOutHandler(RacknrolS2650PortWrite);
	s2650SetIrqCallback(GalS2650ReturnVector);
	s2650Close();
}

static INT32 RacknrolInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = RacknrolPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_RACKNROLSN76496;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = RacknrolExtendTileInfo;
	
	return nRet;
}

static void HexpoolaPostLoad()
{
	RacknrolPostLoad();
	
	s2650Open(0);
	s2650SetInHandler(HexpoolaS2650PortRead);
	s2650SetOutHandler(HexpoolaS2650PortWrite);
	s2650Close();
}

static INT32 HexpoolaInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = HexpoolaPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_HEXPOOLASN76496;
	
	nRet = GalInit();
	
	GalExtendTileInfoFunction = RacknrolExtendTileInfo;
	
	return nRet;
}

static void HunchbksPostLoad()
{
	RacknrolPostLoad();
	s2650Open(0);
	s2650SetReadHandler(HunchbksS2650Read);
	s2650SetWriteHandler(HunchbksS2650Write);
	s2650SetInHandler(HunchbksS2650PortRead);
	s2650SetOutHandler(HunchbksS2650PortWrite);
	s2650Close();
	
	ZetInit(1);
}

static INT32 HunchbksInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = HunchbksPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910;
	
	nRet = GalInit();
	HunchbksSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
	PPI1PortWriteB = HunchbksSoundControlWrite;
	
	return nRet;
}

static void HncholmsPostLoad()
{
	RacknrolPostLoad();
	s2650Open(0);
	s2650SetReadHandler(HunchbksS2650Read);
	s2650SetWriteHandler(HunchbksS2650Write);
	s2650SetInHandler(HncholmsS2650PortRead);
	s2650SetOutHandler(HunchbksS2650PortWrite);
	s2650Close();
	
	ZetInit(1);
}

static INT32 HncholmsInit()
{
	INT32 nRet;
	
	GalPostLoadCallbackFunction = HncholmsPostLoad;
	GalSoundType = GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910;
	
	nRet = GalInit();
	HunchbksSoundInit();
	
	GalRenderBackgroundFunction = ScrambleDrawBackground;
	GalDrawBulletsFunction = ScrambleDrawBullets;
	GalExtendTileInfoFunction = Batman2ExtendTileInfo;
	GalExtendSpriteInfoFunction = UpperExtendSpriteInfo;
	
	nGalCyclesTotal[0] = (18432000 / 6 / 2 / 2) / 60;
	
	GalSoundVolumeShift = 3;
	
	KonamiPPIInit();
	PPI1PortWriteB = HunchbksSoundControlWrite;
	
	return nRet;
}

struct BurnDriver BurnDrvHunchbkg = {
	"hunchbkg", "hunchbak", NULL, NULL, "1981",
	"Hunchback (Galaxian hardware)\0", NULL, "Century Electronics", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, HunchbkgRomInfo, HunchbkgRomName, NULL, NULL, HunchbkgInputInfo, HunchbkgDIPInfo,
	HunchbkgInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvDrivfrcg = {
	"drivfrcg", "drivfrcp", NULL, NULL, "1984",
	"Driving Force (Galaxian conversion)\0", NULL, "Shinkai Inc. (Magic Eletronics USA licence)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_RACING, 0,
	NULL, DrivfrcgRomInfo, DrivfrcgRomName, NULL, NULL, DrivfrcgInputInfo, DrivfrcgDIPInfo,
	DrivfrcgInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvDrivfrcb = {
	"drivfrcb", "drivfrcp", NULL, NULL, "1985",
	"Driving Force (Galaxian conversion bootleg)\0", NULL, "bootleg", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_RACING, 0,
	NULL, DrivfrcbRomInfo, DrivfrcbRomName, NULL, NULL, DrivfrcgInputInfo, DrivfrcgDIPInfo,
	DrivfrcbInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvRacknrol = {
	"racknrol", NULL, NULL, NULL, "1986",
	"Rack + Roll\0", NULL, "Status (Shinkai License)", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SPORTSMISC, 0,
	NULL, RacknrolRomInfo, RacknrolRomName, NULL, NULL, RacknrolInputInfo, RacknrolDIPInfo,
	RacknrolInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 256, 224, 4, 3
};

struct BurnDriver BurnDrvHexpool = {
	"hexpool", "racknrol", NULL, NULL, "1986",
	"Hex Pool (Shinkai)\0", NULL, "Shinkai", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SPORTSMISC, 0,
	NULL, HexpoolRomInfo, HexpoolRomName, NULL, NULL, RacknrolInputInfo, RacknrolDIPInfo,
	RacknrolInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHexpoola = {
	"hexpoola", "racknrol", NULL, NULL, "1985",
	"Hex Pool (Senko)\0", NULL, "Senko", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SPORTSMISC, 0,
	NULL, HexpoolaRomInfo, HexpoolaRomName, NULL, NULL, RacknrolInputInfo, RacknrolDIPInfo,
	HexpoolaInit, GalExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHunchbks = {
	"hunchbks", "hunchbak", NULL, NULL, "1983",
	"Hunchback (Scramble hardware)\0", NULL, "Century Electronics", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_PLATFORM, 0,
	NULL, HunchbksRomInfo, HunchbksRomName, NULL, NULL, HunchbksInputInfo, HunchbksDIPInfo,
	HunchbksInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHncholms = {
	"hncholms", "huncholy", NULL, NULL, "1984",
	"Hunchback Olympic (Scramble hardware)\0", "Imperfect Sound", "Century Electronics", "Galaxian",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_HISCORE_SUPPORTED, 2, HARDWARE_GALAXIAN, GBF_SPORTSMISC, 0,
	NULL, HncholmsRomInfo, HncholmsRomName, NULL, NULL, HunchbksInputInfo, HunchbksDIPInfo,
	HncholmsInit, KonamiExit, GalFrame, NULL, GalScan,
	NULL, 392, 224, 256, 3, 4
};
