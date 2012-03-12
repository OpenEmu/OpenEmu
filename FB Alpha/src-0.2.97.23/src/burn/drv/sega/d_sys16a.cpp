#include "sys16.h"

/*====================================================
Input defs
====================================================*/

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo System16aInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort1 + 5, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort1 + 4, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort1 + 7, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort1 + 6, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 1, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort1 + 2, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort2 + 5, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort2 + 4, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort2 + 7, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort2 + 6, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 1, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort2 + 2, "p2 fire 2" },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
};

STDINPUTINFO(System16a)

static struct BurnInputInfo System16afire1InputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort1 + 5, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort1 + 4, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort1 + 7, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort1 + 6, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 1, "p1 fire 1" },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort2 + 5, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort2 + 4, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort2 + 7, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort2 + 6, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 1, "p2 fire 1" },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
};

STDINPUTINFO(System16afire1)

static struct BurnInputInfo System16afire3InputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort1 + 5, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort1 + 4, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort1 + 7, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort1 + 6, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort1 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , System16InputPort1 + 2, "p1 fire 3" },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort2 + 5, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort2 + 4, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort2 + 7, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort2 + 6, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort2 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , System16InputPort2 + 2, "p2 fire 3" },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
};

STDINPUTINFO(System16afire3)

static struct BurnInputInfo System16aDip3InputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort1 + 5, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort1 + 4, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort1 + 7, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort1 + 6, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 1, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort1 + 2, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort2 + 5, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort2 + 4, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort2 + 7, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort2 + 6, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 1, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort2 + 2, "p2 fire 2" },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, System16Dip + 2        , "dip"       },
};

STDINPUTINFO(System16aDip3)

static struct BurnInputInfo AceattacaInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort3 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort3 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort3 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort3 + 1, "p1 right"  },
	{"P1 Dial Left"      , BIT_DIGITAL  , System16InputPort4 + 0, "p1 fire 14"},
	{"P1 Dial Right"     , BIT_DIGITAL  , System16InputPort4 + 1, "p1 fire 15"},
	{"P1 Block"          , BIT_DIGITAL  , System16InputPort0 + 6, "p1 fire 1" },
	{"P1 Select"         , BIT_DIGITAL  , System16InputPort1 + 4, "p1 fire 2" },
	{"P1 Attack Dir0"    , BIT_DIGITAL  , System16InputPort1 + 0, "p1 fire 3" },
	{"P1 Attack Dir1"    , BIT_DIGITAL  , System16InputPort1 + 1, "p1 fire 4" },
	{"P1 Attack Dir2"    , BIT_DIGITAL  , System16InputPort1 + 2, "p1 fire 5" },
	{"P1 Attack Dir3"    , BIT_DIGITAL  , System16InputPort1 + 3, "p1 fire 6" },
	{"P1 Attack Dir4"    , BIT_DIGITAL  , System16InputPort1 + 4, "p1 fire 7" },
	{"P1 Attack Dir5"    , BIT_DIGITAL  , System16InputPort1 + 5, "p1 fire 8" },
	{"P1 Attack Dir6"    , BIT_DIGITAL  , System16InputPort1 + 6, "p1 fire 9" },
	{"P1 Attack Dir7"    , BIT_DIGITAL  , System16InputPort1 + 7, "p1 fire 10"},
	{"P1 Attack Pow0"    , BIT_DIGITAL  , System16InputPort2 + 4, "p1 fire 11"},
	{"P1 Attack Pow1"    , BIT_DIGITAL  , System16InputPort2 + 5, "p1 fire 12"},
	{"P1 Attack Pow2"    , BIT_DIGITAL  , System16InputPort2 + 6, "p1 fire 13"},

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort3 + 6, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort3 + 7, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort3 + 4, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort3 + 5, "p2 right"  },
	{"P2 Dial Left"      , BIT_DIGITAL  , System16InputPort4 + 2, "p2 fire 14"},
	{"P2 Dial Right"     , BIT_DIGITAL  , System16InputPort4 + 3, "p2 fire 15"},
	{"P2 Block"          , BIT_DIGITAL  , System16InputPort0 + 7, "p2 fire 1" },
	{"P2 Select"         , BIT_DIGITAL  , System16InputPort5 + 4, "p2 fire 2" },
	{"P2 Attack Dir0"    , BIT_DIGITAL  , System16InputPort5 + 0, "p2 fire 3" },
	{"P2 Attack Dir1"    , BIT_DIGITAL  , System16InputPort5 + 1, "p2 fire 4" },
	{"P2 Attack Dir2"    , BIT_DIGITAL  , System16InputPort5 + 2, "p2 fire 5" },
	{"P2 Attack Dir3"    , BIT_DIGITAL  , System16InputPort5 + 3, "p2 fire 6" },
	{"P2 Attack Dir4"    , BIT_DIGITAL  , System16InputPort5 + 4, "p2 fire 7" },
	{"P2 Attack Dir5"    , BIT_DIGITAL  , System16InputPort5 + 5, "p2 fire 8" },
	{"P2 Attack Dir6"    , BIT_DIGITAL  , System16InputPort5 + 6, "p2 fire 9" },
	{"P2 Attack Dir7"    , BIT_DIGITAL  , System16InputPort5 + 7, "p2 fire 10"},
	{"P2 Attack Pow0"    , BIT_DIGITAL  , System16InputPort6 + 4, "p2 fire 11"},
	{"P2 Attack Pow1"    , BIT_DIGITAL  , System16InputPort6 + 5, "p2 fire 12"},
	{"P2 Attack Pow2"    , BIT_DIGITAL  , System16InputPort6 + 6, "p2 fire 13"},

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Aceattaca)

static struct BurnInputInfo MjleagueInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort3 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort3 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort3 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort3 + 1, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 7, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort1 + 0, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , System16InputPort1 + 1, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL  , System16InputPort1 + 2, "p1 fire 4" },
	{"P1 Fire 5"         , BIT_DIGITAL  , System16InputPort1 + 3, "p1 fire 5" },
	A("P1 Bat Swing"     , BIT_ANALOG_REL, &System16AnalogPort0,  "p1 z-axis" ),

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort3 + 6, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort3 + 7, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort3 + 4, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort3 + 5, "p1 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 7, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort2 + 0, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , System16InputPort2 + 1, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL  , System16InputPort2 + 2, "p2 fire 4" },
	{"P2 Fire 5"         , BIT_DIGITAL  , System16InputPort2 + 3, "p2 fire 5" },
	A("P2 Bat Swing"     , BIT_ANALOG_REL, &System16AnalogPort1,  "p2 z-axis" ),

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Mjleague)

static struct BurnInputInfo Passsht16aInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"    },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"   },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"    },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"   },
	{"Start 3"           , BIT_DIGITAL  , System16InputPort0 + 6, "p3 start"   },
	{"Start 4"           , BIT_DIGITAL  , System16InputPort0 + 7, "p4 start"   },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort1 + 1, "p1 up"      },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort1 + 0, "p1 down"    },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort1 + 3, "p1 left"    },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort1 + 2, "p1 right"   },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 4, "p1 fire 1"  },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort1 + 5, "p1 fire 2"  },
	{"P1 Fire 3"         , BIT_DIGITAL  , System16InputPort1 + 6, "p1 fire 3"  },
	{"P1 Fire 4"         , BIT_DIGITAL  , System16InputPort1 + 7, "p1 fire 4"  },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort2 + 1, "p2 up"      },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort2 + 0, "p2 down"    },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort2 + 3, "p2 left"    },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort2 + 2, "p2 right"   },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 4, "p2 fire 1"  },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort2 + 5, "p2 fire 2"  },
	{"P2 Fire 3"         , BIT_DIGITAL  , System16InputPort2 + 6, "p2 fire 3"  },
	{"P2 Fire 4"         , BIT_DIGITAL  , System16InputPort2 + 7, "p2 fire 4"  },
	
	{"P3 Up"             , BIT_DIGITAL  , System16InputPort3 + 1, "p3 up"      },
	{"P3 Down"           , BIT_DIGITAL  , System16InputPort3 + 0, "p3 down"    },
	{"P3 Left"           , BIT_DIGITAL  , System16InputPort3 + 3, "p3 left"    },
	{"P3 Right"          , BIT_DIGITAL  , System16InputPort3 + 2, "p3 right"   },
	{"P3 Fire 1"         , BIT_DIGITAL  , System16InputPort3 + 4, "p3 fire 1"  },
	{"P3 Fire 2"         , BIT_DIGITAL  , System16InputPort3 + 5, "p3 fire 2"  },
	{"P3 Fire 3"         , BIT_DIGITAL  , System16InputPort3 + 6, "p3 fire 3"  },
	{"P3 Fire 4"         , BIT_DIGITAL  , System16InputPort3 + 7, "p3 fire 4"  },

	{"P4 Up"             , BIT_DIGITAL  , System16InputPort4 + 1, "p4 up"      },
	{"P4 Down"           , BIT_DIGITAL  , System16InputPort4 + 0, "p4 down"    },
	{"P4 Left"           , BIT_DIGITAL  , System16InputPort4 + 3, "p4 left"    },
	{"P4 Right"          , BIT_DIGITAL  , System16InputPort4 + 2, "p4 right"   },
	{"P4 Fire 1"         , BIT_DIGITAL  , System16InputPort4 + 4, "p4 fire 1"  },
	{"P4 Fire 2"         , BIT_DIGITAL  , System16InputPort4 + 5, "p4 fire 2"  },
	{"P4 Fire 3"         , BIT_DIGITAL  , System16InputPort4 + 6, "p4 fire 3"  },
	{"P4 Fire 4"         , BIT_DIGITAL  , System16InputPort4 + 7, "p4 fire 4"  },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Passsht16a)

static struct BurnInputInfo QuartetInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 6, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort1 + 6, "p2 coin"   },
	{"Coin 3"            , BIT_DIGITAL  , System16InputPort2 + 6, "p3 coin"   },
	{"Coin 4"            , BIT_DIGITAL  , System16InputPort3 + 6, "p4 coin"   },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort0 + 1, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort0 + 0, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort0 + 3, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort0 + 2, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort0 + 5, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort1 + 1, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort1 + 0, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort1 + 3, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort1 + 2, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort1 + 5, "p2 fire 2" },
	
	{"P3 Up"             , BIT_DIGITAL  , System16InputPort2 + 1, "p1 up"     },
	{"P3 Down"           , BIT_DIGITAL  , System16InputPort2 + 0, "p1 down"   },
	{"P3 Left"           , BIT_DIGITAL  , System16InputPort2 + 3, "p1 left"   },
	{"P3 Right"          , BIT_DIGITAL  , System16InputPort2 + 2, "p1 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 4, "p1 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL  , System16InputPort2 + 5, "p1 fire 2" },

	{"P4 Up"             , BIT_DIGITAL  , System16InputPort3 + 1, "p2 up"     },
	{"P4 Down"           , BIT_DIGITAL  , System16InputPort3 + 0, "p2 down"   },
	{"P4 Left"           , BIT_DIGITAL  , System16InputPort3 + 3, "p2 left"   },
	{"P4 Right"          , BIT_DIGITAL  , System16InputPort3 + 2, "p2 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL  , System16InputPort3 + 4, "p2 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL  , System16InputPort3 + 5, "p2 fire 2" },

	{"Service 1"         , BIT_DIGITAL  , System16InputPort0 + 7 , "service"   },
	{"Service 2"         , BIT_DIGITAL  , System16InputPort1 + 7 , "service2"  },
	{"Service 3"         , BIT_DIGITAL  , System16InputPort2 + 7 , "service3"  },
	{"Service 4"         , BIT_DIGITAL  , System16InputPort3 + 7 , "service4"  },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Quartet)

static struct BurnInputInfo SdiInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"      },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 4, "p1 start"     },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"      },
	{"Start 2"           , BIT_DIGITAL   , System16InputPort0 + 5, "p2 start"     },

	{"P1 Up"             , BIT_DIGITAL   , System16InputPort1 + 1, "p1 up"        },
	{"P1 Down"           , BIT_DIGITAL   , System16InputPort1 + 0, "p1 down"      },
	{"P1 Left"           , BIT_DIGITAL   , System16InputPort1 + 3, "p1 left"      },
	{"P1 Right"          , BIT_DIGITAL   , System16InputPort1 + 2, "p1 right"     },
	A("Target Left/Right", BIT_ANALOG_REL, &System16AnalogPort0,   "mouse x-axis" ),
	A("Target Up/Down"   , BIT_ANALOG_REL, &System16AnalogPort1,   "mouse y-axis" ),
	{"P1 Fire 1"         , BIT_DIGITAL   , System16InputPort0 + 6, "mouse button 1"},

	{"P2 Up"             , BIT_DIGITAL   , System16InputPort1 + 5, "p2 up"        },
	{"P2 Down"           , BIT_DIGITAL   , System16InputPort1 + 4, "p2 down"      },
	{"P2 Left"           , BIT_DIGITAL   , System16InputPort1 + 7, "p2 left"      },
	{"P2 Right"          , BIT_DIGITAL   , System16InputPort1 + 6, "p2 right"     },
	A("Target Left/Right", BIT_ANALOG_REL, &System16AnalogPort2,   "p2 x-axis"    ),
	A("Target Up/Down"   , BIT_ANALOG_REL, &System16AnalogPort3,   "p2 y-axis"    ),
	{"P2 Fire 1"         , BIT_DIGITAL   , System16InputPort0 + 7, "p2 fire 1"    },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"      },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"         },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"        },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"          },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"          },
};

STDINPUTINFO(Sdi)

static struct BurnInputInfo Sjryuko1InputList[] = {
	{"Coin 1"             , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Coin 2"             , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	
	{"Mahjong A"          , BIT_DIGITAL  , System16InputPort1 + 0, "mah a"     },
	{"Mahjong B"          , BIT_DIGITAL  , System16InputPort1 + 1, "mah b"     },
	{"Mahjong C"          , BIT_DIGITAL  , System16InputPort1 + 2, "mah c"     },
	{"Mahjong D"          , BIT_DIGITAL  , System16InputPort1 + 3, "mah d"     },
	{"Mahjong E"          , BIT_DIGITAL  , System16InputPort2 + 0, "mah e"     },
	{"Mahjong F"          , BIT_DIGITAL  , System16InputPort2 + 1, "mah f"     },
	{"Mahjong G"          , BIT_DIGITAL  , System16InputPort2 + 2, "mah g"     },
	{"Mahjong H"          , BIT_DIGITAL  , System16InputPort2 + 3, "mah h"     },
	{"Mahjong I"          , BIT_DIGITAL  , System16InputPort3 + 0, "mah i"     },
	{"Mahjong J"          , BIT_DIGITAL  , System16InputPort3 + 1, "mah j"     },
	{"Mahjong K"          , BIT_DIGITAL  , System16InputPort3 + 2, "mah k"     },
	{"Mahjong L"          , BIT_DIGITAL  , System16InputPort3 + 3, "mah l"     },
	{"Mahjong M"          , BIT_DIGITAL  , System16InputPort4 + 0, "mah m"     },
	{"Mahjong N"          , BIT_DIGITAL  , System16InputPort4 + 1, "mah n"     },
	{"Mahjong Kan"        , BIT_DIGITAL  , System16InputPort6 + 0, "mah kan"   },
	{"Mahjong Pon"        , BIT_DIGITAL  , System16InputPort4 + 3, "mah pon"   },
	{"Mahjong Chi"        , BIT_DIGITAL  , System16InputPort4 + 2, "mah chi"   },
	{"Mahjong Reach"      , BIT_DIGITAL  , System16InputPort6 + 1, "mah reach" },
	{"Mahjong Ron"        , BIT_DIGITAL  , System16InputPort6 + 2, "mah ron"   },
	{"Mahjong Bet"        , BIT_DIGITAL  , System16InputPort5 + 1, "mah bet"   },
	{"Mahjong Last Chance", BIT_DIGITAL  , System16InputPort1 + 4, "mah lc"    },
	{"Mahjong Score"      , BIT_DIGITAL  , System16InputPort5 + 0, "mah score" },
	{"Mahjong Flip Flop"  , BIT_DIGITAL  , System16InputPort4 + 4, "mah ff"    },
		
	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Sjryuko1)

#undef A

/*====================================================
Dip defs
====================================================*/

#define SYSTEM16A_COINAGE(dipval)								\
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
	
static struct BurnDIPInfo AceattacaDIPList[]=
{
	// Default Values
	{0x2d, 0xff, 0xff, 0xff, NULL                                 },
	{0x2e, 0xff, 0xff, 0xfe, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x2d)
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x2e, 0x01, 0x01, 0x01, "Off"                                },
	{0x2e, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 8   , "Starting Points"                    },
	{0x2e, 0x01, 0x0e, 0x06, "2000"                               },
	{0x2e, 0x01, 0x0e, 0x0a, "3000"                               },
	{0x2e, 0x01, 0x0e, 0x0c, "4000"                               },
	{0x2e, 0x01, 0x0e, 0x0e, "5000"                               },	
	{0x2e, 0x01, 0x0e, 0x08, "6000"                               },	
	{0x2e, 0x01, 0x0e, 0x04, "7000"                               },
	{0x2e, 0x01, 0x0e, 0x02, "8000"                               },
	{0x2e, 0x01, 0x0e, 0x00, "9000"                               },
};

STDDIPINFO(Aceattaca)

static struct BurnDIPInfo AfighterDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xfc, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x13)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x14, 0x01, 0x01, 0x00, "Upright"                            },
	{0x14, 0x01, 0x01, 0x01, "Cocktail"                           },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x14, 0x01, 0x02, 0x02, "Off"                                },
	{0x14, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x14, 0x01, 0x0c, 0x08, "2"                                  },
	{0x14, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x14, 0x01, 0x0c, 0x04, "4"                                  },
	{0x14, 0x01, 0x0c, 0x00, "Infinite"                           },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                         },
	{0x14, 0x01, 0x30, 0x30, "10000 - 20000"                      },
	{0x14, 0x01, 0x30, 0x20, "20000 - 40000"                      },
	{0x14, 0x01, 0x30, 0x10, "30000 - 60000"                      },
	{0x14, 0x01, 0x30, 0x00, "40000 - 80000"                      },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"                         },
	{0x14, 0x01, 0x40, 0x40, "Normal"                             },
	{0x14, 0x01, 0x40, 0x00, "Hard"                               },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                     },
	{0x14, 0x01, 0x80, 0x00, "No"                                 },
	{0x14, 0x01, 0x80, 0x80, "Yes"                                },
};

STDDIPINFO(Afighter)

static struct BurnDIPInfo AlexkiddDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xfc, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x13)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Continues"                          },
	{0x14, 0x01, 0x01, 0x01, "Only before level 5"                },
	{0x14, 0x01, 0x01, 0x00, "Unlimited"                          },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x14, 0x01, 0x02, 0x02, "Off"                                },
	{0x14, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x14, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x14, 0x01, 0x0c, 0x08, "4"                                  },
	{0x14, 0x01, 0x0c, 0x04, "5"                                  },
	{0x14, 0x01, 0x0c, 0x00, "240"                                },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                         },
	{0x14, 0x01, 0x30, 0x20, "10000"                              },
	{0x14, 0x01, 0x30, 0x30, "20000"                              },
	{0x14, 0x01, 0x30, 0x10, "40000"                              },
	{0x14, 0x01, 0x30, 0x00, "None"                               },
	
	{0   , 0xfe, 0   , 4   , "Timer Adjust"                       },
	{0x14, 0x01, 0xc0, 0x80, "70"                                 },
	{0x14, 0x01, 0xc0, 0xc0, "60"                                 },
	{0x14, 0x01, 0xc0, 0x40, "50"                                 },
	{0x14, 0x01, 0xc0, 0x00, "40"                                 },
};

STDDIPINFO(Alexkidd)

static struct BurnDIPInfo AliensynDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                                 },
	{0x12, 0xff, 0xff, 0xfd, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x11)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x12, 0x01, 0x02, 0x02, "Off"                                },
	{0x12, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x12, 0x01, 0x0c, 0x08, "2"                                  },
	{0x12, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x12, 0x01, 0x0c, 0x04, "4"                                  },
	{0x12, 0x01, 0x0c, 0x00, "127"                                },
	
	{0   , 0xfe, 0   , 4   , "Timer"                              },
	{0x12, 0x01, 0x30, 0x00, "150"                                },
	{0x12, 0x01, 0x30, 0x10, "160"                                },
	{0x12, 0x01, 0x30, 0x20, "170"                                },
	{0x12, 0x01, 0x30, 0x30, "180"                                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x12, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x12, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x12, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x12, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Aliensyn)

static struct BurnDIPInfo BodyslamDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xfc, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x13)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x14, 0x01, 0x02, 0x02, "Off"                                },
	{0x14, 0x01, 0x02, 0x00, "On"                                 },
};

STDDIPINFO(Bodyslam)

static struct BurnDIPInfo FantzoneDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xfc, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x13)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x14, 0x01, 0x01, 0x00, "Upright"                            },
	{0x14, 0x01, 0x01, 0x01, "Cocktail"                           },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x14, 0x01, 0x02, 0x02, "Off"                                },
	{0x14, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x14, 0x01, 0x0c, 0x08, "2"                                  },
	{0x14, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x14, 0x01, 0x0c, 0x04, "4"                                  },
	{0x14, 0x01, 0x0c, 0x00, "240"                                },
	
	{0   , 0xfe, 0   , 4   , "Extra Ship Cost"                    },
	{0x14, 0x01, 0x30, 0x30, "5000"                               },
	{0x14, 0x01, 0x30, 0x20, "10000"                              },
	{0x14, 0x01, 0x30, 0x10, "15000"                              },
	{0x14, 0x01, 0x30, 0x00, "20000"                              },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x14, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x14, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x14, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x14, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Fantzone)

static struct BurnDIPInfo MjleagueDIPList[]=
{
	// Default Values
	{0x1b, 0xff, 0xff, 0xff, NULL                                 },
	{0x1c, 0xff, 0xff, 0xf0, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x1b)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x1c, 0x01, 0x01, 0x00, "Upright"                            },
	{0x1c, 0x01, 0x01, 0x01, "Cocktail"                           },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x1c, 0x01, 0x02, 0x02, "Off"                                },
	{0x1c, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Starting Points"                    },
	{0x1c, 0x01, 0x0c, 0x0c, "2000"                               },
	{0x1c, 0x01, 0x0c, 0x08, "3000"                               },
	{0x1c, 0x01, 0x0c, 0x04, "5000"                               },
	{0x1c, 0x01, 0x0c, 0x00, "10000"                              },
	
	{0   , 0xfe, 0   , 2   , "Team Select"                        },
	{0x1c, 0x01, 0x10, 0x02, "Off"                                },
	{0x1c, 0x01, 0x10, 0x10, "On"                                 },
};

STDDIPINFO(Mjleague)

static struct BurnDIPInfo Passsht16aDIPList[]=
{
	// Default Values
	{0x29, 0xff, 0xff, 0xff, NULL                                 },
	{0x2a, 0xff, 0xff, 0xf0, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x29)
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x2a, 0x01, 0x01, 0x01, "Off"                                },
	{0x2a, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 8   , "Initial Point"                      },
	{0x2a, 0x01, 0x0e, 0x06, "2000"                               },
	{0x2a, 0x01, 0x0e, 0x0a, "3000"                               },
	{0x2a, 0x01, 0x0e, 0x0c, "4000"                               },
	{0x2a, 0x01, 0x0e, 0x0e, "5000"                               },
	{0x2a, 0x01, 0x0e, 0x08, "6000"                               },
	{0x2a, 0x01, 0x0e, 0x04, "7000"                               },
	{0x2a, 0x01, 0x0e, 0x02, "8000"                               },
	{0x2a, 0x01, 0x0e, 0x00, "9000"                               },
	
	{0   , 0xfe, 0   , 4   , "Point Table"                        },
	{0x2a, 0x01, 0x30, 0x20, "Easy"                               },
	{0x2a, 0x01, 0x30, 0x30, "Normal"                             },
	{0x2a, 0x01, 0x30, 0x10, "Hard"                               },
	{0x2a, 0x01, 0x30, 0x00, "Hardest"                            },	
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x2a, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x2a, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x2a, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x2a, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Passsht16a)

static struct BurnDIPInfo QuartetDIPList[]=
{
	// Default Values
	{0x21, 0xff, 0xff, 0xff, NULL                                 },
	{0x22, 0xff, 0xff, 0xfe, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x21)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x22, 0x01, 0x01, 0x01, "Off"                                },
	{0x22, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Credit Power"                       },
	{0x22, 0x01, 0x06, 0x04, "500"                                },
	{0x22, 0x01, 0x06, 0x06, "1000"                               },
	{0x22, 0x01, 0x06, 0x02, "2000"                               },
	{0x22, 0x01, 0x06, 0x00, "9000"                               },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x22, 0x01, 0x18, 0x10, "Easy"                               },
	{0x22, 0x01, 0x18, 0x18, "Normal"                             },
	{0x22, 0x01, 0x18, 0x08, "Hard"                               },
	{0x22, 0x01, 0x18, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 2   , "Coin During Game"                   },
	{0x22, 0x01, 0x20, 0x20, "Power"                              },
	{0x22, 0x01, 0x20, 0x00, "Credit"                             },
	
	{0   , 0xfe, 0   , 2   , "Free Play"                          },
	{0x22, 0x01, 0x40, 0x40, "Off"                                },
	{0x22, 0x01, 0x40, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x22, 0x01, 0x80, 0x80, "Off"                                },
	{0x22, 0x01, 0x80, 0x00, "On"                                 },
};

STDDIPINFO(Quartet)

static struct BurnDIPInfo Quart2DIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xfe, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x13)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x14, 0x01, 0x01, 0x01, "Off"                                },
	{0x14, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Credit Power"                       },
	{0x14, 0x01, 0x06, 0x04, "500"                                },
	{0x14, 0x01, 0x06, 0x06, "1000"                               },
	{0x14, 0x01, 0x06, 0x02, "2000"                               },
	{0x14, 0x01, 0x06, 0x00, "9000"                               },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x14, 0x01, 0x18, 0x10, "Easy"                               },
	{0x14, 0x01, 0x18, 0x18, "Normal"                             },
	{0x14, 0x01, 0x18, 0x08, "Hard"                               },
	{0x14, 0x01, 0x18, 0x00, "Hardest"                            },
};

STDDIPINFO(Quart2)

static struct BurnDIPInfo SdiDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                                 },
	{0x16, 0xff, 0xff, 0xfd, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x15)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Allow Continue"                     },
	{0x16, 0x01, 0x01, 0x01, "No"                                 },
	{0x16, 0x01, 0x01, 0x00, "Yes"                                },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x16, 0x01, 0x02, 0x02, "Off"                                },
	{0x16, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x16, 0x01, 0x0c, 0x08, "2"                                  },
	{0x16, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x16, 0x01, 0x0c, 0x04, "4"                                  },
	{0x16, 0x01, 0x0c, 0x00, "Free"                               },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x16, 0x01, 0x30, 0x20, "Easy"                               },
	{0x16, 0x01, 0x30, 0x30, "Normal"                             },
	{0x16, 0x01, 0x30, 0x10, "Hard"                               },
	{0x16, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                         },
	{0x16, 0x01, 0xc0, 0x80, "Every 50000"                        },
	{0x16, 0x01, 0xc0, 0xc0, "50000"                              },
	{0x16, 0x01, 0xc0, 0x40, "100000"                             },
	{0x16, 0x01, 0xc0, 0x00, "None"                               },
};

STDDIPINFO(Sdi)

static struct BurnDIPInfo ShinobiDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                                 },
	{0x16, 0xff, 0xff, 0xfc, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x15)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x16, 0x01, 0x01, 0x00, "Upright"                            },
	{0x16, 0x01, 0x01, 0x01, "Cocktail"                           },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x16, 0x01, 0x02, 0x02, "Off"                                },
	{0x16, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x16, 0x01, 0x0c, 0x08, "2"                                  },
	{0x16, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x16, 0x01, 0x0c, 0x04, "5"                                  },
	{0x16, 0x01, 0x0c, 0x00, "240"                                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x16, 0x01, 0x30, 0x20, "Easy"                               },
	{0x16, 0x01, 0x30, 0x30, "Normal"                             },
	{0x16, 0x01, 0x30, 0x10, "Hard"                               },
	{0x16, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 2   , "Enemy Bullet Speed"                 },
	{0x16, 0x01, 0x40, 0x40, "Slow"                               },
	{0x16, 0x01, 0x40, 0x00, "Fast"                               },
	
	{0   , 0xfe, 0   , 2   , "Language"                           },
	{0x16, 0x01, 0x80, 0x80, "Japanese"                           },
	{0x16, 0x01, 0x80, 0x00, "English"                            },
};

STDDIPINFO(Shinobi)

static struct BurnDIPInfo Sjryuko1DIPList[]=
{
	// Default Values
	{0x1c, 0xff, 0xff, 0xff, NULL                                 },
	{0x1d, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x1c)
	
	// Dip 2
};

STDDIPINFO(Sjryuko1)

static struct BurnDIPInfo TetrisDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                                 },
	{0x12, 0xff, 0xff, 0xfd, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x11)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x12, 0x01, 0x02, 0x02, "Off"                                },
	{0x12, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x12, 0x01, 0x30, 0x20, "Easy"                               },
	{0x12, 0x01, 0x30, 0x30, "Normal"                             },
	{0x12, 0x01, 0x30, 0x10, "Hard"                               },
	{0x12, 0x01, 0x30, 0x00, "Hardest"                            },
};

STDDIPINFO(Tetris)

static struct BurnDIPInfo TimescanDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xf5, NULL                                 },
	{0x15, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x13)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x14, 0x01, 0x01, 0x00, "Cocktail"                           },
	{0x14, 0x01, 0x01, 0x01, "Upright"                            },
	
	{0   , 0xfe, 0   , 16  , "Bonus"                              },
	{0x14, 0x01, 0x1e, 0x16, "Replay 1000000/2000000"             },
	{0x14, 0x01, 0x1e, 0x14, "Replay 1200000/2500000"             },
	{0x14, 0x01, 0x1e, 0x12, "Replay 1500000/3000000"             },
	{0x14, 0x01, 0x1e, 0x10, "Replay 2000000/4000000"             },
	{0x14, 0x01, 0x1e, 0x1c, "Replay 1000000"                     },
	{0x14, 0x01, 0x1e, 0x1e, "Replay 1200000"                     },
	{0x14, 0x01, 0x1e, 0x1a, "Replay 1500000"                     },
	{0x14, 0x01, 0x1e, 0x18, "Replay 1800000"                     },
	{0x14, 0x01, 0x1e, 0x0e, "Extra Ball 100000"                  },
	{0x14, 0x01, 0x1e, 0x0c, "Extra Ball 200000"                  },
	{0x14, 0x01, 0x1e, 0x0a, "Extra Ball 300000"                  },
	{0x14, 0x01, 0x1e, 0x08, "Extra Ball 400000"                  },
	{0x14, 0x01, 0x1e, 0x06, "Extra Ball 500000"                  },
	{0x14, 0x01, 0x1e, 0x04, "Extra Ball 600000"                  },
	{0x14, 0x01, 0x1e, 0x02, "Extra Ball 700000"                  },
	{0x14, 0x01, 0x1e, 0x00, "None"                               },
	
	{0   , 0xfe, 0   , 2   , "Match"                              },
	{0x14, 0x01, 0x20, 0x00, "Off"                                },
	{0x14, 0x01, 0x20, 0x20, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Pin Rebound"                        },
	{0x14, 0x01, 0x40, 0x40, "Well"                               },
	{0x14, 0x01, 0x40, 0x00, "A Little"                           },
	
	{0   , 0xfe, 0   , 2   , "Lives"                              },
	{0x14, 0x01, 0x80, 0x80, "3"                                  },
	{0x14, 0x01, 0x80, 0x00, "5"                                  },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Flip Screen"                        },
	{0x15, 0x01, 0x01, 0x01, "Off"                                },
	{0x15, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Out Lane Pin"                       },
	{0x15, 0x01, 0x02, 0x02, "Near"                               },
	{0x15, 0x01, 0x02, 0x00, "Far"                                },
	
	{0   , 0xfe, 0   , 4   , "Special"                            },
	{0x15, 0x01, 0x0c, 0x08, "7 Credits"                          },
	{0x15, 0x01, 0x0c, 0x0c, "3 Credits"                          },
	{0x15, 0x01, 0x0c, 0x04, "1 Credit"                           },
	{0x15, 0x01, 0x0c, 0x00, "2000000 Points"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                     },
	{0x15, 0x01, 0x10, 0x00, "No"                                 },
	{0x15, 0x01, 0x10, 0x10, "Yes"                                },
};

STDDIPINFO(Timescan)

static struct BurnDIPInfo Wb31DIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xfc, NULL                                 },

	// Dip 1
	SYSTEM16A_COINAGE(0x13)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x14, 0x01, 0x02, 0x02, "Off"                                },
	{0x14, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x14, 0x01, 0x0c, 0x00, "2"                                  },
	{0x14, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x14, 0x01, 0x0c, 0x08, "4"                                  },
	{0x14, 0x01, 0x0c, 0x04, "5"                                  },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"                         },
	{0x14, 0x01, 0x10, 0x10, "5k, 10k, 18k, 30k"                  },
	{0x14, 0x01, 0x10, 0x00, "5k, 15k, 30k"                       },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"                         },
	{0x14, 0x01, 0x20, 0x20, "Normal"                             },
	{0x14, 0x01, 0x20, 0x00, "Hard"                               },
	
	{0   , 0xfe, 0   , 2   , "Invincible Mode"                    },
	{0x14, 0x01, 0x40, 0x40, "No"                                 },
	{0x14, 0x01, 0x40, 0x00, "Yes"                                },
};

STDDIPINFO(Wb31)

#undef SYSTEM16A_COINAGE

/*====================================================
Rom defs
====================================================*/

static struct BurnRomInfo AceattacaRomDesc[] = {
	{ "epr-11573.43",  0x10000, 0xca116367, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11571.26",  0x10000, 0x5631d1d6, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11574.42",  0x10000, 0x8d3ed7bd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11572.25",  0x10000, 0x35c27c25, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11575.95",  0x10000, 0xbab9fbf3, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11576.94",  0x10000, 0x067ed682, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11577.93",  0x10000, 0xf67cf331, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11501.10",  0x10000, 0x09179ead, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11505.11",  0x10000, 0xb67f1ecf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11502.17",  0x10000, 0x7464bae4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11506.18",  0x10000, 0xb0104def, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11503.23",  0x10000, 0x344c0692, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11507.24",  0x10000, 0xa2af710a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11504.29",  0x10000, 0x42b4a5f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11508.30",  0x10000, 0x5cbb833c, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-11578.12",  0x08000, 0x3d58e39a, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11579.1",   0x08000, 0x1a994135, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-11580.2",   0x08000, 0x961646ed, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-11581.4",   0x08000, 0xd271a6e5, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-11582.5",   0x08000, 0xbcbe3d8a, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0060.key",  0x02000, 0xf4ee940f, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Aceattaca)
STD_ROM_FN(Aceattaca)

static struct BurnRomInfo AfighterRomDesc[] = {
	{ "epr-10353",     0x08000, 0x5a757dc9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10350",     0x08000, 0xf2cd6b3f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10352",     0x08000, 0xf8abb143, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10349",     0x08000, 0x4b434c37, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10351",     0x08000, 0xede21d8d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10348",     0x08000, 0xe51e3012, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10281.95",  0x10000, 0x30e92cda, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10282.94",  0x10000, 0xb67b8910, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10283.93",  0x10000, 0xe7dbfd2d, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10285",     0x08000, 0x98aa3d04, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10289",     0x08000, 0xc59d1b98, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10286",     0x08000, 0x8da050cf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10290",     0x08000, 0x39354223, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10287",     0x08000, 0x7989b74a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10291",     0x08000, 0x6e4b245c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10288",     0x08000, 0xd3ce551a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10292",     0x08000, 0xcef289a3, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10284.12",  0x08000, 0x8ff09116, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0018.key",  0x02000, 0xfee04be8, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Afighter)
STD_ROM_FN(Afighter)

static struct BurnRomInfo AlexkiddRomDesc[] = {
	{ "epr-10447.43",  0x10000, 0x29e87f71, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10445.26",  0x10000, 0x25ce5b6f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10448.42",  0x10000, 0x05baedb5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10446.25",  0x10000, 0xcd61d23c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10431.95",  0x08000, 0xa7962c39, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10432.94",  0x08000, 0xdb8cd24e, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10433.93",  0x08000, 0xe163c8c2, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10437.10",  0x08000, 0x522f7618, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10441.11",  0x08000, 0x74e3a35c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10438.17",  0x08000, 0x738a6362, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10442.18",  0x08000, 0x86cb9c14, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10439.23",  0x08000, 0xb391aca7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10443.24",  0x08000, 0x95d32635, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10440.29",  0x08000, 0x23939508, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10444.30",  0x08000, 0x82115823, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10434.12",  0x08000, 0x77141cce, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10435.1",   0x08000, 0xad89f6e3, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10436.2",   0x08000, 0x96c76613, SYS16_ROM_7751DATA | BRF_SND },
};


STD_ROM_PICK(Alexkidd)
STD_ROM_FN(Alexkidd)

static struct BurnRomInfo Alexkidd1RomDesc[] = {
	{ "epr-10429.43",  0x10000, 0xbdf49eca, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10427.26",  0x10000, 0xf6e3dd29, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10430.42",  0x10000, 0x89e3439f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10428.25",  0x10000, 0xdbed3210, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10431.95",  0x08000, 0xa7962c39, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10432.94",  0x08000, 0xdb8cd24e, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10433.93",  0x08000, 0xe163c8c2, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10437.10",  0x08000, 0x522f7618, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10441.11",  0x08000, 0x74e3a35c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10438.17",  0x08000, 0x738a6362, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10442.18",  0x08000, 0x86cb9c14, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10439.23",  0x08000, 0xb391aca7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10443.24",  0x08000, 0x95d32635, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10440.29",  0x08000, 0x23939508, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10444.30",  0x08000, 0x82115823, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10434.12",  0x08000, 0x77141cce, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10435.1",   0x08000, 0xad89f6e3, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10436.2",   0x08000, 0x96c76613, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0021.key",  0x02000, 0x85be8eac, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Alexkidd1)
STD_ROM_FN(Alexkidd1)

static struct BurnRomInfo AliensynjoRomDesc[] = {
	{ "epr-10699.43",  0x08000, 0x3fd38d17, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10696.26",  0x08000, 0xd734f19f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10700.42",  0x08000, 0x3b04b252, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10697.25",  0x08000, 0xf2bc123d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10701.41",  0x08000, 0x92171751, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10698.24",  0x08000, 0xc1e4fdc0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10739.95",  0x10000, 0xa29ec207, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10740.94",  0x10000, 0x47f93015, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10741.93",  0x10000, 0x4970739c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10709.10",  0x10000, 0xaddf0a90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10713.11",  0x10000, 0xececde3a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10710.17",  0x10000, 0x992369eb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10714.18",  0x10000, 0x91bf42fb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10711.23",  0x10000, 0x29166ef6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10715.24",  0x10000, 0xa7c57384, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10712.29",  0x10000, 0x876ad019, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10716.30",  0x10000, 0x40ba1d48, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10705.12",  0x08000, 0x777b749e, SYS16_ROM_Z80PROG | BRF_ESS |BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10706.1",   0x08000, 0xaa114acc, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10707.2",   0x08000, 0x800c1d82, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10708.4",   0x08000, 0x5921ef52, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0033.key",  0x02000, 0x49e882e5, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Aliensynjo)
STD_ROM_FN(Aliensynjo)

static struct BurnRomInfo Aliensyn2RomDesc[] = {
	{ "epr-10808",     0x08000, 0xe669929f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10806",     0x08000, 0x9f7f8fdd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10809",     0x08000, 0x9a424919, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10807",     0x08000, 0x3d2c3530, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10701",     0x08000, 0x92171751, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10698",     0x08000, 0xc1e4fdc0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "10739",         0x10000, 0xa29ec207, SYS16_ROM_TILES | BRF_GRA },
	{ "10740",         0x10000, 0x47f93015, SYS16_ROM_TILES | BRF_GRA },
	{ "10741",         0x10000, 0x4970739c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "10709.b1",      0x10000, 0xaddf0a90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10713.b5",      0x10000, 0xececde3a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10710.b2",      0x10000, 0x992369eb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10714.b6",      0x10000, 0x91bf42fb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10711.b3",      0x10000, 0x29166ef6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10715.b7",      0x10000, 0xa7c57384, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10712.b4",      0x10000, 0x876ad019, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10716.b8",      0x10000, 0x40ba1d48, SYS16_ROM_SPRITES | BRF_GRA },

	{ "10705",         0x08000, 0x777b749e, SYS16_ROM_Z80PROG | BRF_ESS |BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "10706",         0x08000, 0xaa114acc, SYS16_ROM_7751DATA | BRF_SND },
	{ "10707",         0x08000, 0x800c1d82, SYS16_ROM_7751DATA | BRF_SND },
	{ "10708",         0x08000, 0x5921ef52, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0033.key",  0x02000, 0x49e882e5, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Aliensyn2)
STD_ROM_FN(Aliensyn2)

static struct BurnRomInfo Aliensyn5RomDesc[] = {
	{ "epr-10804.43",  0x08000, 0x23f78b83, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10802.26",  0x08000, 0x996768bd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10805.42",  0x08000, 0x53d7fe50, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10803.25",  0x08000, 0x0536dd33, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10732.41",  0x08000, 0xc5712bfc, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10729.24",  0x08000, 0x3e520e30, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10739.95",  0x10000, 0xa29ec207, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10740.94",  0x10000, 0x47f93015, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10741.93",  0x10000, 0x4970739c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10709.10",  0x10000, 0xaddf0a90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10713.11",  0x10000, 0xececde3a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10710.17",  0x10000, 0x992369eb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10714.18",  0x10000, 0x91bf42fb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10711.23",  0x10000, 0x29166ef6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10715.24",  0x10000, 0xa7c57384, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10712.29",  0x10000, 0x876ad019, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10716.30",  0x10000, 0x40ba1d48, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10705.12",  0x08000, 0x777b749e, SYS16_ROM_Z80PROG | BRF_ESS |BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10706.1",   0x08000, 0xaa114acc, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10707.2",   0x08000, 0x800c1d82, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10708.4",   0x08000, 0x5921ef52, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0037.key",  0x02000, 0x49e882e5, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Aliensyn5)
STD_ROM_FN(Aliensyn5)

static struct BurnRomInfo BodyslamRomDesc[] = {
	{ "epr-10066.b9",  0x08000, 0x6cd53290, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10063.b6",  0x08000, 0xdd849a16, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10067.b10", 0x08000, 0xdb22a5ce, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10064.b7",  0x08000, 0x53d6b7e0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10068.b11", 0x08000, 0x15ccc665, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10065.b8",  0x08000, 0x0e5fa314, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10321.c9",  0x08000, 0xcd3e7cba, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10322.c10", 0x08000, 0xb53d3217, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10323.c11", 0x08000, 0x915a3e61, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10012.c5",  0x08000, 0x990824e8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10016.b2",  0x08000, 0xaf5dc72f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10013.c6",  0x08000, 0x9a0919c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10017.b3",  0x08000, 0x62aafd95, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10027.c7",  0x08000, 0x3f1c57c7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10028.b4",  0x08000, 0x80d4946d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10015.c8",  0x08000, 0x582d3b6a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10019.b5",  0x08000, 0xe020c38b, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10026.b1",  0x08000, 0x123b69b8, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10029.c1",  0x08000, 0x7e4aca83, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10030.c2",  0x08000, 0xdcc1df0b, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10031.c3",  0x08000, 0xea3c4472, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-10032.c4",  0x08000, 0x0aabebce, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0015.bin",  0x01000, 0x833869e2, BRF_PRG | BRF_OPT },
};


STD_ROM_PICK(Bodyslam)
STD_ROM_FN(Bodyslam)

static struct BurnRomInfo DumpmtmtRomDesc[] = {
	{ "epr-7704a.b9",  0x08000, 0x96de6c7b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7701a.b6",  0x08000, 0x786d1009, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7705a.b10", 0x08000, 0xfc584391, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7702a.b7",  0x08000, 0x2241a8fd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7706a.b11", 0x08000, 0x6bbcc9d0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7703a.b8",  0x08000, 0xfcb0cd40, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7707a.c9",  0x08000, 0x45318738, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7708a.c10", 0x08000, 0x411be9a4, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7709a.c11", 0x08000, 0x74ceb5a8, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7715.c5",   0x08000, 0xbf47e040, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7719.b2",   0x08000, 0xfa5c5d6c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10013.c6",  0x08000, 0x9a0919c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10017.b3",  0x08000, 0x62aafd95, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7717.c7",   0x08000, 0xfa64c86d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7721.b4",   0x08000, 0x62a9143e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10015.c8",  0x08000, 0x582d3b6a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10019.b5",  0x08000, 0xe020c38b, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7710a.b1",  0x08000, 0xa19b8ba8, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7711.c1",   0x08000, 0xefa9aabd, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7712.c2",   0x08000, 0x7bcd85cf, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7713.c3",   0x08000, 0x33f292e7, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7714.c4",   0x08000, 0x8fd48c47, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0011a.mcu", 0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Dumpmtmt)
STD_ROM_FN(Dumpmtmt)

static struct BurnRomInfo FantzoneRomDesc[] = {
	{ "epr-7385a.43",  0x08000, 0x4091af42, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7382a.26",  0x08000, 0x77d67bfd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7386a.42",  0x08000, 0xb0a67cd0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7383a.25",  0x08000, 0x5f79b2a9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7387.41",   0x08000, 0x0acd335d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7384.24",   0x08000, 0xfd909341, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7388.95",   0x08000, 0x8eb02f6b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7389.94",   0x08000, 0x2f4f71b8, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7390.93",   0x08000, 0xd90609c6, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7392.10",   0x08000, 0x5bb7c8b6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7396.11",   0x08000, 0x74ae4b57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7393.17",   0x08000, 0x14fc7e82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7397.18",   0x08000, 0xe05a1e25, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7394.23",   0x08000, 0x531ca13f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7398.24",   0x08000, 0x68807b49, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7535a.12",  0x08000, 0xbc1374fa, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Fantzone)
STD_ROM_FN(Fantzone)

static struct BurnRomInfo Fantzone1RomDesc[] = {
	{ "epr-7385.43",   0x08000, 0x5cb64450, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7382.26",   0x08000, 0x3fda7416, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7386.42",   0x08000, 0x15810ace, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7383.25",   0x08000, 0xa001e10a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7387.41",   0x08000, 0x0acd335d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7384.24",   0x08000, 0xfd909341, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7388.95",   0x08000, 0x8eb02f6b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7389.94",   0x08000, 0x2f4f71b8, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7390.93",   0x08000, 0xd90609c6, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7392.10",   0x08000, 0x5bb7c8b6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7396.11",   0x08000, 0x74ae4b57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7393.17",   0x08000, 0x14fc7e82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7397.18",   0x08000, 0xe05a1e25, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7394.23",   0x08000, 0x531ca13f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7398.24",   0x08000, 0x68807b49, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7535.12",   0x08000, 0x0cb2126a, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Fantzone1)
STD_ROM_FN(Fantzone1)

static struct BurnRomInfo FantzonepRomDesc[] = {
	{ "epr-7385.43",   0x08000, 0x5cb64450, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7382.26",   0x08000, 0x3fda7416, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7386.42",   0x08000, 0x15810ace, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7383.25",   0x08000, 0xa001e10a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7387.41",   0x08000, 0x0acd335d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7384.24",   0x08000, 0xfd909341, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7388.95",   0x08000, 0x8eb02f6b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7389.94",   0x08000, 0x2f4f71b8, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7390.93",   0x08000, 0xd90609c6, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7392.10",   0x08000, 0x5bb7c8b6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7396.11",   0x08000, 0x74ae4b57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7393.17",   0x08000, 0x14fc7e82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7397.18",   0x08000, 0xe05a1e25, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7394.23",   0x08000, 0x531ca13f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7398.24",   0x08000, 0x68807b49, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7391.12",   0x08000, 0xc03e155e, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Fantzonep)
STD_ROM_FN(Fantzonep)

static struct BurnRomInfo MjleagueRomDesc[] = {
	{ "epr-7404.9b",   0x08000, 0xec1655b5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7401.6b",   0x08000, 0x2befa5e0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7405.10b",  0x08000, 0x7a4f4e38, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7402.7b",   0x08000, 0xb7bef762, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7406a.11b", 0x08000, 0xbb743639, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7403a.8b",  0x08000, 0x0a39a4d0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7051.9a",   0x08000, 0x10ca255a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7052.10a",  0x08000, 0x2550db0e, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7053.11a",  0x08000, 0x5bfea038, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7055.5a",   0x08000, 0x1fb860bd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7059.2b",   0x08000, 0x3d14091d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7056.6a",   0x08000, 0xb35dd968, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7060.3b",   0x08000, 0x61bb3757, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7057.7a",   0x08000, 0x3e5a2b6f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7061.4b",   0x08000, 0xc808dad5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7058.8a",   0x08000, 0xb543675f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7062.5b",   0x08000, 0x9168eb47, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7054c.1b",  0x08000, 0x4443b744, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7063.1a",   0x08000, 0x45d8908a, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7065.2a",   0x08000, 0x8c8f8cff, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7064.3a",   0x08000, 0x159f6636, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7066.4a",   0x08000, 0xf5cfa91f, SYS16_ROM_7751DATA | BRF_SND },
};


STD_ROM_PICK(Mjleague)
STD_ROM_FN(Mjleague)

static struct BurnRomInfo Passsht16aRomDesc[] = {
	{ "epr-11833.43",  0x10000, 0x5eb1405c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11832.26",  0x10000, 0x718a3fe4, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11834.95",  0x10000, 0xdf4e18ab, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11835.94",  0x10000, 0x6a07acc0, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11836.93",  0x10000, 0x93c74928, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11842.10",  0x10000, 0xb6e94727, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11845.11",  0x10000, 0x17e8d5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11843.17",  0x10000, 0x3e670098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11846.18",  0x10000, 0x50eb71cc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11844.23",  0x10000, 0x05733ca8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11847.24",  0x10000, 0x81e49697, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-11837.12",  0x08000, 0x74d11552, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11838.1",   0x08000, 0xa465cd69, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-11839.2",   0x08000, 0x99de6197, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-11840.4",   0x08000, 0x9854e8b3, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-11841.5",   0x08000, 0x1e89877e, SYS16_ROM_7751DATA | BRF_SND },
		
	{ "317-0071.key",  0x02000, 0xc69949ec, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Passsht16a)
STD_ROM_FN(Passsht16a)

static struct BurnRomInfo QuartetRomDesc[] = {
	{ "epr-7458a.9b",  0x08000, 0x42e7b23e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7455a.6b",  0x08000, 0x01631ab2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7459a.10b", 0x08000, 0x6b540637, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7456a.7b",  0x08000, 0x31ca583e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7460.11b",  0x08000, 0xa444ea13, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7457.8b",   0x08000, 0x3b282c23, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7461.9c",   0x08000, 0xf6af07f2, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7462.10c",  0x08000, 0x7914af28, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7463.11c",  0x08000, 0x827c5603, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7465.5c",   0x08000, 0x8a1ab7d7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7469.2b",   0x08000, 0xcb65ae4f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7466.6c",   0x08000, 0xb2d3f4f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7470.3b",   0x08000, 0x16fc67b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7467.7c",   0x08000, 0x0af68de2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7471.4b",   0x08000, 0x13fad5ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7468.8c",   0x08000, 0xddfd40c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7472.5b",   0x08000, 0x8e2762ec, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7464.1b",   0x08000, 0x9f291306, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7473.1c",   0x08000, 0x06ec75fa, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7475.2c",   0x08000, 0x7abd1206, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7474.3c",   0x08000, 0xdbf853b8, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7476.4c",   0x08000, 0x5eba655a, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "315-5194.mcu",  0x01000, 0x00000000, BRF_NODUMP },
	
	{ "pal16r6a.22g",  0x00104, 0x00000000, BRF_NODUMP }, // PLD
	{ "pal16r6a.23g",  0x00104, 0x00000000, BRF_NODUMP }, // PLD
	{ "pls153.8j",     0x000eb, 0x0fe1eefd, BRF_OPT }, // PLD
};


STD_ROM_PICK(Quartet)
STD_ROM_FN(Quartet)

static struct BurnRomInfo QuartetaRomDesc[] = {
	{ "epr-7458.9b",   0x08000, 0x0096499f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7455.6b",   0x08000, 0xda934390, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7459.10b",  0x08000, 0xd130cf61, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7456.7b",   0x08000, 0x7847149f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7460.11b",  0x08000, 0xa444ea13, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7457.8b",   0x08000, 0x3b282c23, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7461.9c",   0x08000, 0xf6af07f2, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7462.10c",  0x08000, 0x7914af28, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7463.11c",  0x08000, 0x827c5603, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7465.5c",   0x08000, 0x8a1ab7d7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7469.2b",   0x08000, 0xcb65ae4f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7466.6c",   0x08000, 0xb2d3f4f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7470.3b",   0x08000, 0x16fc67b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7467.7c",   0x08000, 0x0af68de2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7471.4b",   0x08000, 0x13fad5ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7468.8c",   0x08000, 0xddfd40c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7472.5b",   0x08000, 0x8e2762ec, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7464.1b",   0x08000, 0x9f291306, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7473.1c",   0x08000, 0x06ec75fa, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7475.2c",   0x08000, 0x7abd1206, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7474.3c",   0x08000, 0xdbf853b8, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7476.4c",   0x08000, 0x5eba655a, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "315-5194.mcu",  0x01000, 0x00000000, BRF_NODUMP },
	
	{ "pal16r6a.22g",  0x00104, 0x00000000, BRF_NODUMP }, // PLD
	{ "pal16r6a.23g",  0x00104, 0x00000000, BRF_NODUMP }, // PLD
	{ "pls153.8j",     0x000eb, 0x0fe1eefd, BRF_OPT }, // PLD
};


STD_ROM_PICK(Quarteta)
STD_ROM_FN(Quarteta)

static struct BurnRomInfo Quartet2RomDesc[] = {
	{ "epr-7695.b9",   0x08000, 0x67177cd8, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7692.b6",   0x08000, 0x50f50b08, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7696.b10",  0x08000, 0x4273c3b7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7693.b7",   0x08000, 0x0aa337bb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7697.b11",  0x08000, 0x3a6a375d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7694.b8",   0x08000, 0xd87b2ca2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7698.c9",   0x08000, 0x547a6058, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7699.c10",  0x08000, 0x77ec901d, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7700.c11",  0x08000, 0x7e348cce, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7465.5c",   0x08000, 0x8a1ab7d7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7469.2b",   0x08000, 0xcb65ae4f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7466.6c",   0x08000, 0xb2d3f4f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7470.3b",   0x08000, 0x16fc67b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7467.7c",   0x08000, 0x0af68de2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7471.4b",   0x08000, 0x13fad5ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7468.8c",   0x08000, 0xddfd40c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7472.5b",   0x08000, 0x8e2762ec, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7464.1b",   0x08000, 0x9f291306, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7473.1c",   0x08000, 0x06ec75fa, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7475.2c",   0x08000, 0x7abd1206, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7474.3c",   0x08000, 0xdbf853b8, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7476.4c",   0x08000, 0x5eba655a, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0010.bin",  0x01000, 0x8c2033ea, BRF_PRG | BRF_OPT },
};


STD_ROM_PICK(Quartet2)
STD_ROM_FN(Quartet2)

static struct BurnRomInfo Quartet2aRomDesc[] = {
	{ "epr-7728.b9",   0x08000, 0x56a8c88e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7725.b6",   0x08000, 0xee15fcc9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7729.b10",  0x08000, 0xbc242123, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7726.b7",   0x08000, 0x9d1c48e7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7697.b11",  0x08000, 0x3a6a375d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7694.b8",   0x08000, 0xd87b2ca2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-7698.c9",   0x08000, 0x547a6058, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7699.c10",  0x08000, 0x77ec901d, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7700.c11",  0x08000, 0x7e348cce, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7465.5c",   0x08000, 0x8a1ab7d7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7469.2b",   0x08000, 0xcb65ae4f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7466.6c",   0x08000, 0xb2d3f4f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7470.3b",   0x08000, 0x16fc67b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7467.7c",   0x08000, 0x0af68de2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7471.4b",   0x08000, 0x13fad5ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7468.8c",   0x08000, 0xddfd40c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7472.5b",   0x08000, 0x8e2762ec, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-7464.1b",   0x08000, 0x9f291306, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7473.1c",   0x08000, 0x06ec75fa, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7475.2c",   0x08000, 0x7abd1206, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7474.3c",   0x08000, 0xdbf853b8, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-7476.4c",   0x08000, 0x5eba655a, SYS16_ROM_7751DATA | BRF_SND },
};


STD_ROM_PICK(Quartet2a)
STD_ROM_FN(Quartet2a)

static struct BurnRomInfo SdiRomDesc[] = {
	{ "epr-10970.43",  0x08000, 0xb8fa4a2c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10968.26",  0x08000, 0xa3f97793, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10971.42",  0x08000, 0xc44a0328, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10969.25",  0x08000, 0x455d15bd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10755.41",  0x08000, 0x405e3969, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10752.24",  0x08000, 0x77453740, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10756.95",  0x10000, 0x44d8a506, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10757.94",  0x10000, 0x497e1740, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10758.93",  0x10000, 0x61d61486, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10760.10",  0x10000, 0x30e2c50a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10763.11",  0x10000, 0x794e3e8b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10761.17",  0x10000, 0x6a8b3fd0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10764.18",  0x10000, 0x602da5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10762.23",  0x10000, 0xb9de3aeb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10765.24",  0x10000, 0x0a73a057, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10759.12",  0x08000, 0xd7f9649f, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0027.key",  0x02000, 0x9a5307b2, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Sdi)
STD_ROM_FN(Sdi)

static struct BurnRomInfo ShinobiRomDesc[] = {
	{ "epr-12010.43",  0x10000, 0x7df7f4a2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12008.26",  0x10000, 0xf5ae64cd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12011.42",  0x10000, 0x9d46e707, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12009.25",  0x10000, 0x7961d07e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11264.95",  0x10000, 0x46627e7d, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11265.94",  0x10000, 0x87d0f321, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11266.93",  0x10000, 0xefb4af87, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11290.10",  0x10000, 0x611f413a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11294.11",  0x10000, 0x5eb00fc1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11291.17",  0x10000, 0x3c0797c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11295.18",  0x10000, 0x25307ef8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11292.23",  0x10000, 0xc29ac34e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11296.24",  0x10000, 0x04a437f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11293.29",  0x10000, 0x41f41063, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11297.30",  0x10000, 0xb6e1fd72, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11267.12",  0x08000, 0xdd50b745, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11268.1",   0x08000, 0x6d7966da, SYS16_ROM_7751DATA | BRF_SND },
};


STD_ROM_PICK(Shinobi)
STD_ROM_FN(Shinobi)

static struct BurnRomInfo Shinobi1RomDesc[] = {
	{ "epr-11262.42",  0x10000, 0xd4b8df12, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11260.27",  0x10000, 0x2835c95d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11263.43",  0x10000, 0xa2a620bd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11261.25",  0x10000, 0xa3ceda52, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11264.95",  0x10000, 0x46627e7d, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11265.94",  0x10000, 0x87d0f321, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11266.93",  0x10000, 0xefb4af87, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11290.10",  0x10000, 0x611f413a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11294.11",  0x10000, 0x5eb00fc1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11291.17",  0x10000, 0x3c0797c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11295.18",  0x10000, 0x25307ef8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11292.23",  0x10000, 0xc29ac34e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11296.24",  0x10000, 0x04a437f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11293.29",  0x10000, 0x41f41063, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11297.30",  0x10000, 0xb6e1fd72, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11267.12",  0x08000, 0xdd50b745, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11268.1",   0x08000, 0x6d7966da, SYS16_ROM_7751DATA | BRF_SND },

	{ "317-0050.key",  0x02000, 0x82c39ced, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Shinobi1)
STD_ROM_FN(Shinobi1)

static struct BurnRomInfo ShinoblsRomDesc[] = {
	{ "b3",            0x10000, 0x38e59646, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "b1",            0x10000, 0x8529d192, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "b4",            0x10000, 0xa2a620bd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "b2",            0x10000, 0xa3ceda52, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "b5",            0x10000, 0x46627e7d, SYS16_ROM_TILES | BRF_GRA },
	{ "b6",            0x10000, 0x87d0f321, SYS16_ROM_TILES | BRF_GRA },
	{ "b7",            0x10000, 0xefb4af87, SYS16_ROM_TILES | BRF_GRA },
	
	{ "b10",           0x10000, 0x611f413a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b14",           0x10000, 0x5eb00fc1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b11",           0x10000, 0x3c0797c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b15",           0x10000, 0x25307ef8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b12",           0x10000, 0xc29ac34e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b16",           0x10000, 0x04a437f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b13",           0x10000, 0x7e98bd36, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b17",           0x10000, 0x0315cf42, SYS16_ROM_SPRITES | BRF_GRA },

	{ "b8",            0x08000, 0xdd50b745, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "b9",            0x08000, 0x6d7966da, SYS16_ROM_7751DATA | BRF_SND },
};


STD_ROM_PICK(Shinobls)
STD_ROM_FN(Shinobls)

static struct BurnRomInfo ShinoblbRomDesc[] = {
	{ "4.3k",          0x10000, 0xc178a39c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "2.3n",          0x10000, 0x5ad8ebf2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "5.2k",          0x10000, 0xa2a620bd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "3.2n",          0x10000, 0xa3ceda52, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "8.3b",          0x10000, 0x46627e7d, SYS16_ROM_TILES | BRF_GRA },
	{ "7.4b",          0x10000, 0x87d0f321, SYS16_ROM_TILES | BRF_GRA },
	{ "6.5b",          0x10000, 0xefb4af87, SYS16_ROM_TILES | BRF_GRA },
	
	{ "9.6r",          0x10000, 0x611f413a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13.8r",         0x10000, 0x5eb00fc1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10.6q",         0x10000, 0x3c0797c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "14.8q",         0x10000, 0x25307ef8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "11.6p",         0x10000, 0xc29ac34e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "15.8p",         0x10000, 0x04a437f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12.6n",         0x10000, 0x41f41063, SYS16_ROM_SPRITES | BRF_GRA },
	{ "16.8n",         0x10000, 0xb6e1fd72, SYS16_ROM_SPRITES | BRF_GRA },

	{ "1.5s",          0x08000, 0xdd50b745, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "b9",            0x08000, 0x6d7966da, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "17.6u",         0x08000, 0xb7a6890c, BRF_SND | BRF_OPT },
};


STD_ROM_PICK(Shinoblb)
STD_ROM_FN(Shinoblb)

static struct BurnRomInfo Sjryuko1RomDesc[] = {
	{ "epr-12251.43",  0x08000, 0x1af3cd0b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12249.26",  0x08000, 0x743d467d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12252.42",  0x08000, 0x7ae309d6, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12250.25",  0x08000, 0x52c40f19, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "12224-95.b9",   0x08000, 0xeac17ba1, SYS16_ROM_TILES | BRF_GRA },
	{ "12225-94.b10",  0x08000, 0x2310fc98, SYS16_ROM_TILES | BRF_GRA },
	{ "12226-93.b11",  0x08000, 0x210e6999, SYS16_ROM_TILES | BRF_GRA },
	
	{ "12232-10.b1",   0x10000, 0x0adec62b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12236-11.b5",   0x10000, 0x286b9af8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12233-17.b2",   0x10000, 0x3e45969c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12237-18.b6",   0x10000, 0xe5058e96, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12234-23.b3",   0x10000, 0x8c8d54ef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12238-24.b7",   0x10000, 0x7ada3304, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12235-29.b4",   0x10000, 0xfa45d511, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12239-30.b8",   0x10000, 0x91f70c8b, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12227.12",  0x08000, 0x5b12409d, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12228.1",   0x08000, 0x6b2e6aef, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-12229.2",   0x08000, 0xb7aa015c, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-12230.4",   0x08000, 0xd0f61fd4, SYS16_ROM_7751DATA | BRF_SND },
	{ "epr-12231.5",   0x08000, 0x780bdc57, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-5021.key",  0x02000, 0x4a3422e4, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Sjryuko1)
STD_ROM_FN(Sjryuko1)

static struct BurnRomInfo TetrisRomDesc[] = {
	{ "epr-12201.rom", 0x08000, 0x338e9b51, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12200.rom", 0x08000, 0xfb058779, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12202.rom", 0x10000, 0x2f7da741, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12203.rom", 0x10000, 0xa6e58ec5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12204.rom", 0x10000, 0x0ae98e23, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12169.b1",  0x08000, 0xdacc6165, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12170.b5",  0x08000, 0x87354e42, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12205.rom", 0x08000, 0x6695dc99, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0093.key",  0x02000, 0xe0064442, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Tetris)
STD_ROM_FN(Tetris)

static struct BurnRomInfo Tetris3RomDesc[] = {
	{ "epr-12201a.43", 0x08000, 0x9250e5cf, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12200a.26", 0x08000, 0x85d4b0ff, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12202.rom", 0x10000, 0x2f7da741, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12203.rom", 0x10000, 0xa6e58ec5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12204.rom", 0x10000, 0x0ae98e23, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12169.b1",  0x08000, 0xdacc6165, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12170.b5",  0x08000, 0x87354e42, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12205.rom", 0x08000, 0x6695dc99, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0093a.key", 0x02000, 0x7ca4a8ee, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Tetris3)
STD_ROM_FN(Tetris3)

static struct BurnRomInfo Timescan1RomDesc[] = {
	{ "epr-10540a.43", 0x08000, 0x76848b0b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10537a.26", 0x08000, 0x4ddc434a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10541.42",  0x08000, 0xcc6d945e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10538.25",  0x08000, 0x68379473, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10542.41",  0x08000, 0x10217dfa, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10539.24",  0x08000, 0x10943b2e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10543.95",  0x08000, 0x07dccc37, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10544.94",  0x08000, 0x84fb9a3a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10545.93",  0x08000, 0xc8694bc0, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10548.10",  0x08000, 0xaa150735, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10552.11",  0x08000, 0x6fcbb9f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10549.17",  0x08000, 0x2f59f067, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10553.18",  0x08000, 0x8a220a9f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10550.23",  0x08000, 0xf05069ff, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10554.24",  0x08000, 0xdc64f809, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10551.29",  0x08000, 0x435d811f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10555.30",  0x08000, 0x2143c471, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10546.12",  0x08000, 0x1ebee5cc, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "7751.bin",      0x00400, 0x6a9534fc, SYS16_ROM_7751PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10547.1",   0x08000, 0xd24ffc4b, SYS16_ROM_7751DATA | BRF_SND },
	
	{ "317-0024.key",  0x02000, 0xee42ec18, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Timescan1)
STD_ROM_FN(Timescan1)

static struct BurnRomInfo Wb31RomDesc[] = {
	{ "epr-12084.bin", 0x10000, 0xb6deb654, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12082.bin", 0x10000, 0x38dc5b15, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12085.bin", 0x10000, 0x0962098b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12083.bin", 0x10000, 0x3d631a8e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12086.bin", 0x10000, 0x45b949df, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12087.bin", 0x10000, 0x6f0396b7, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12088.bin", 0x10000, 0xba8c0749, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12090.b1",  0x10000, 0xaeeecfca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12094.b5",  0x10000, 0x615e4927, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12091.b2",  0x10000, 0x8409a243, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12095.b6",  0x10000, 0xe774ec2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12092.b3",  0x10000, 0x5c2f0d90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12096.b7",  0x10000, 0x0cd59d6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12093.b4",  0x10000, 0x4891e7bb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12097.b8",  0x10000, 0xe645902c, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12089.bin", 0x08000, 0x8321eb0b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0084.key",  0x02000, 0x2c58dafa, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Wb31)
STD_ROM_FN(Wb31)

/*====================================================
Memory Handlers
====================================================*/

static bool Mjleague = false;

void System16APPI0WritePortA(UINT8 data)
{
	System16SoundLatch = data & 0xff;
}

void System16APPI0WritePortB(UINT8 data)
{
	System16VideoControl = data;
	System16VideoEnable = data & 0x10;
	if (Mjleague) System16ScreenFlip = data & 0x80;
}

void System16APPI0WritePortC(UINT8 data)
{
	if (!(data & 0x80)) {
		ZetOpen(0);
		ZetNmi();
		ZetClose();
	}
	System16ColScroll = ~data & 0x04;
	System16RowScroll = ~data & 0x02;
}

UINT16 __fastcall System16AReadWord(UINT32 a)
{
	switch (a) {
		case 0xc40000:
		case 0xc40002:
		case 0xc40004:
		case 0xc40006:
		case 0xc41000:
		case 0xc41002:
		case 0xc41004:
		case 0xc41006:
		case 0xc42000:
		case 0xc42002:
		case 0xc42004:
		case 0xc42006: {
			SEK_DEF_READ_WORD(0, a);
		}
		
		case 0xc60000: {
			return 0;
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Word -> 0x%06X\n"), a);
#endif

	return 0xffff;
}

UINT8 __fastcall System16AReadByte(UINT32 a)
{
	switch (a) {
		case 0xc40001:
		case 0xc40003: 
		case 0xc40005:
		case 0xc40007: {
			return ppi8255_r(0, (a - 0xc40000) >> 1);
		}
		
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			return 0xff - System16Input[1];
		}
		
		case 0xc41005: {
			return System16Dip[2];
		}
		
		case 0xc41007: {
			return 0xff - System16Input[2];
		}
		
		case 0xc42001: {
			return System16Dip[0];
		}

		case 0xc42003: {
			return System16Dip[1];
		}
		
		case 0xc60000: {
			return 0;
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

void __fastcall System16AWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16ATileWordWrite(a - 0x400000, d);
		return;
	}
	
	switch (a) {
		case 0xc40000:
		case 0xc40002:
		case 0xc40004:
		case 0xc40006: {
			SEK_DEF_WRITE_WORD(0, a, d);
			return;
		}
		
		case 0xc60000: {
			return;
		}
	}		

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

void __fastcall System16AWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16ATileByteWrite((a - 0x400000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0xc40001:
		case 0xc40003: 
		case 0xc40005:
		case 0xc40007: {
			ppi8255_w(0, (a - 0xc40000) >> 1, d & 0xff);
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

static INT16 AceattacaTrack1X = 0;
static INT16 AceattacaTrack1Y = 0;
static INT16 AceattacaTrack2X = 0;
static INT16 AceattacaTrack2Y = 0;
static INT8 AceattacaDial1 = 0;
static INT8 AceattacaDial2 = 0;

void AceattacaMakeAnalogInputs()
{
	if (System16InputPort3[0]) AceattacaTrack1X += 0x40;
	if (System16InputPort3[1]) AceattacaTrack1X -= 0x40;
	if (AceattacaTrack1X >= 0x100) AceattacaTrack1X = 0;
	if (AceattacaTrack1X < 0) AceattacaTrack1X = 0xfd;
	
	if (System16InputPort3[2]) AceattacaTrack1Y -= 0x40;
	if (System16InputPort3[3]) AceattacaTrack1Y += 0x40;
	if (AceattacaTrack1Y >= 0x100) AceattacaTrack1Y = 0;
	if (AceattacaTrack1Y < 0) AceattacaTrack1Y = 0xfd;
	
	if (System16InputPort4[0]) AceattacaDial1 += 0x01;
	if (System16InputPort4[1]) AceattacaDial1 -= 0x01;
	if (AceattacaDial1 >= 0x10) AceattacaDial1 = 0;
	if (AceattacaDial1 < 0) AceattacaDial1 = 0x0f;
	
	if (System16InputPort3[4]) AceattacaTrack2X += 0x40;
	if (System16InputPort3[5]) AceattacaTrack2X -= 0x40;
	if (AceattacaTrack2X >= 0x100) AceattacaTrack2X = 0;
	if (AceattacaTrack2X < 0) AceattacaTrack2X = 0xfd;
	
	if (System16InputPort3[6]) AceattacaTrack2Y -= 0x40;
	if (System16InputPort3[7]) AceattacaTrack2Y += 0x40;
	if (AceattacaTrack2Y >= 0x100) AceattacaTrack2Y = 0;
	if (AceattacaTrack2Y < 0) AceattacaTrack2Y = 0xfd;
	
	if (System16InputPort4[2]) AceattacaDial2 += 0x01;
	if (System16InputPort4[3]) AceattacaDial2 -= 0x01;
	if (AceattacaDial2 >= 0x10) AceattacaDial2 = 0;
	if (AceattacaDial2 < 0) AceattacaDial2 = 0x0f;
}

UINT8 __fastcall AceattacaReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			if (System16VideoControl == 0x10) return 0xff - System16Input[1];
			if (System16VideoControl == 0x14) return AceattacaTrack1X;
			if (System16VideoControl == 0x18) return AceattacaTrack1Y;
			if (System16VideoControl == 0x1c) return System16Input[2];
			return 0xff;
		}
		
		case 0xc41005: {
			return AceattacaDial1 | (AceattacaDial2 << 4);
		}
		
		case 0xc41007: {
			if (System16VideoControl == 0x10) return 0xff - System16Input[5];
			if (System16VideoControl == 0x14) return AceattacaTrack2X;
			if (System16VideoControl == 0x18) return AceattacaTrack2Y;
			if (System16VideoControl == 0x1c) return System16Input[6];
			return 0xff;
		}
		
		case 0xc42001: {
			return System16Dip[0];
		}

		case 0xc42003: {
			return System16Dip[1];
		}
		
		case 0xc60000: {
			return 0;
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

static INT16 MjleagueTrack1X = 0;
static INT16 MjleagueTrack1Y = 0;
static INT16 MjleagueTrack2X = 0;
static INT16 MjleagueTrack2Y = 0;
static INT16 MjleagueBat1 = 0;
static INT16 MjleagueBat2 = 0;

void MjleagueMakeAnalogInputs()
{
	if (System16InputPort3[0]) MjleagueTrack1X -= 0x04;
	if (System16InputPort3[1]) MjleagueTrack1X += 0x04;
	if (MjleagueTrack1X >= 0x100) MjleagueTrack1X = 0;
	if (MjleagueTrack1X < 0) MjleagueTrack1X = 0xfc;
	
	if (System16InputPort3[2]) MjleagueTrack1Y -= 0x04;
	if (System16InputPort3[3]) MjleagueTrack1Y += 0x04;
	if (MjleagueTrack1Y >= 0x100) MjleagueTrack1Y = 0;
	if (MjleagueTrack1Y < 0) MjleagueTrack1Y = 0xfc;
	
	if (System16InputPort3[4]) MjleagueTrack2X -= 0x04;
	if (System16InputPort3[5]) MjleagueTrack2X += 0x04;
	if (MjleagueTrack2X >= 0x100) MjleagueTrack2X = 0;
	if (MjleagueTrack2X < 0) MjleagueTrack2X = 0xfc;
	
	if (System16InputPort3[6]) MjleagueTrack2Y -= 0x04;
	if (System16InputPort3[7]) MjleagueTrack2Y += 0x04;
	if (MjleagueTrack2Y >= 0x100) MjleagueTrack2Y = 0;
	if (MjleagueTrack2Y < 0) MjleagueTrack2Y = 0xfc;
	
	MjleagueBat1 = 0x80 + (System16AnalogPort0 >> 4);
	MjleagueBat2 = 0x80 + (System16AnalogPort1 >> 4);
}

UINT8 __fastcall MjleagueReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			UINT8 buttons = 0x3f - System16Input[0];
			UINT8 analog1 = (System16VideoControl & 4) ? MjleagueTrack1Y : MjleagueTrack1X;
			UINT8 analog2 = (System16VideoControl & 4) ? MjleagueTrack2Y : MjleagueTrack2X;
			
			buttons |= (analog1 & 0x80) >> 1;
			buttons |= (analog2 & 0x80);
			
			return buttons;
		}
		
		case 0xc41003: {
			UINT8 analog = (System16VideoControl & 4) ? MjleagueTrack1Y : MjleagueTrack1X;
			return ((0xff - System16Input[1]) & 0x80) | (analog & 0x7f);
		}
		
		case 0xc41005: {
			if (System16VideoControl & 4) {
				return (MjleagueBat1 >> 4) | (MjleagueBat2 & 0xf0);
			} else {
				static UINT8 last_buttons1 = 0;
				static UINT8 last_buttons2 = 0;
				UINT8 buttons1 = 0xff - System16Input[1];
				UINT8 buttons2 = 0xff - System16Input[2];

				if (!(buttons1 & 0x01))
					last_buttons1 = 0;
				else if (!(buttons1 & 0x02))
					last_buttons1 = 1;
				else if (!(buttons1 & 0x04))
					last_buttons1 = 2;
				else if (!(buttons1 & 0x08))
					last_buttons1 = 3;

				if (!(buttons2 & 0x01))
					last_buttons2 = 0;
				else if (!(buttons2 & 0x02))
					last_buttons2 = 1;
				else if (!(buttons2 & 0x04))
					last_buttons2 = 2;
				else if (!(buttons2 & 0x08))
					last_buttons2 = 3;

				return last_buttons1 | (last_buttons2 << 4);
			}
		}
		
		case 0xc41007: {
			UINT8 analog = (System16VideoControl & 4) ? MjleagueTrack2Y : MjleagueTrack2X;
			return ((0xff - System16Input[2]) & 0x80) | (analog & 0x7f);
		}
		
		case 0xc42001: {
			return System16Dip[0];
		}

		case 0xc42003: {
			return System16Dip[1];
		}
	}
	
	return 0xff;
}

UINT8 __fastcall Passsht16aReadByte(UINT32 a)
{
	static INT32 PortNum = 0;
	
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			switch ((PortNum++) & 3) {
				case 0: return 0xff - System16Input[1];
				case 1: return 0xff - System16Input[2];
				case 2: return 0xff - System16Input[3];
				case 3: return 0xff - System16Input[4];
			}
		}
		
		case 0xc42001: {
			return System16Dip[0];
		}

		case 0xc42003: {
			return System16Dip[1];
		}
		
		case 0xc60000: {
			return 0;
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

UINT8 __fastcall QuartetReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			return 0xff - System16Input[1];
		}
		
		case 0xc41005: {
			return 0xff - System16Input[2];
		}
		
		case 0xc41007: {
			return 0xff - System16Input[3];
		}
		
		case 0xc42001: {
			return System16Dip[0];
		}

		case 0xc42003: {
			return System16Dip[1];
		}
		
		case 0xc60000: {
			return 0;
		}
	}
		
	return 0xff;
}

static INT16 SdiTrack1X = 0;
static INT16 SdiTrack1Y = 0;
static INT16 SdiTrack2X = 0;
static INT16 SdiTrack2Y = 0;

void SdiMakeAnalogInputs()
{
	SdiTrack1X += (System16AnalogPort0 >> 8) & 0xff;
	SdiTrack1Y -= (System16AnalogPort1 >> 8) & 0xff;
	
	SdiTrack2X += (System16AnalogPort2 >> 8) & 0xff;
	SdiTrack2Y -= (System16AnalogPort3 >> 8) & 0xff;
}

UINT8 __fastcall SdiReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			return (System16VideoControl & 4) ? SdiTrack1Y : SdiTrack1X;
		}
		
		case 0xc41005: {
			return 0xff - System16Input[1];
		}
		
		case 0xc41007: {
			return (System16VideoControl & 4) ? SdiTrack2Y : SdiTrack2X;
		}
		
		case 0xc42001: {
			return System16Dip[0];
		}

		case 0xc42003: {
			return System16Dip[1];
		}
		
		case 0xc60000: {
			return 0xff;
		}
	}

	return 0xff;
}

static UINT8 MahjongInputNum;

UINT16 __fastcall Sjryuko1ReadWord(UINT32 a)
{
	SEK_DEF_READ_WORD(0, a);
	
	return 0xffff;
}

UINT8 __fastcall Sjryuko1ReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			if (System16Input[MahjongInputNum + 1] != 0xff) return 0xff & ~(1 << MahjongInputNum);
			return 0xff;
		}
		
		case 0xc41005: {
			return 0xff - System16Input[MahjongInputNum + 1];
		}
		
		case 0xc42001: {
			return System16Dip[0];
		}

		case 0xc42003: {
			return System16Dip[1];
		}
	}
	
	return 0xff;
}

void __fastcall Sjryuko1WriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16ATileByteWrite((a - 0x400000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0xc40001: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetNmi();
			ZetClose();
			return;
		}
		
		case 0xc40003: {
			if ((System16VideoControl ^ d) & 0xc) {
				if (((System16VideoControl ^ d) & 4) && (d & 4)) MahjongInputNum = (MahjongInputNum + 1) % 6;
			}
			System16VideoControl = d;
			System16VideoEnable = d & 0x10;
			return;
		}
	}
}

/*====================================================
Driver Inits
====================================================*/

static INT32 AceattacaInit()
{
	System16MakeAnalogInputsDo = AceattacaMakeAnalogInputs;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, AceattacaReadByte);
		SekClose();
		
		// Handle different sprite rom layout
		UINT8 *pTemp = (UINT8*)BurnMalloc(System16SpriteRomSize);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, System16SpriteRomSize);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x40000, 0x10000);
			memcpy(System16Sprites + 0x30000, pTemp + 0x60000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x50000, 0x10000);
			memcpy(System16Sprites + 0x70000, pTemp + 0x70000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 AceattacaExit()
{
	AceattacaTrack1X = 0;
	AceattacaTrack1Y = 0;
	AceattacaTrack2X = 0;
	AceattacaTrack2Y = 0;
	AceattacaDial1 = 0;
	AceattacaDial2 = 0;

	return System16Exit();
}

static INT32 AceattacaScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin =  0x029660;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(AceattacaTrack1X);
		SCAN_VAR(AceattacaTrack1Y);
		SCAN_VAR(AceattacaTrack2X);
		SCAN_VAR(AceattacaTrack2Y);
		SCAN_VAR(AceattacaDial1);
		SCAN_VAR(AceattacaDial2);
	}
	
	return System16Scan(nAction, pnMin);;
}

static INT32 AliensynjoInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		// Handle different sprite rom layout
		UINT8 *pTemp = (UINT8*)BurnMalloc(System16SpriteRomSize);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, System16SpriteRomSize);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x40000, 0x10000);
			memcpy(System16Sprites + 0x30000, pTemp + 0x60000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x50000, 0x10000);
			memcpy(System16Sprites + 0x70000, pTemp + 0x70000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Aliensyn5Init()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		// Handle different sprite rom layout
		UINT8 *pTemp = (UINT8*)BurnMalloc(System16SpriteRomSize);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, System16SpriteRomSize);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x40000, 0x10000);
			memcpy(System16Sprites + 0x30000, pTemp + 0x60000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x50000, 0x10000);
			memcpy(System16Sprites + 0x70000, pTemp + 0x70000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

void Bodyslam_Sim8751()
{
	UINT8 flag = ((System16Ram[0x200 + 1] << 8) | System16Ram[0x200 + 0]) >> 8;
	UINT8 tick = ((System16Ram[0x200 + 1] << 8) | System16Ram[0x200 + 0]) & 0xff;
	UINT8 sec  = ((System16Ram[0x202 + 1] << 8) | System16Ram[0x202 + 0]) >> 8;
	UINT8 min  = ((System16Ram[0x202 + 1] << 8) | System16Ram[0x202 + 0]) & 0xff;

	/* out of time? set the flag */
	if (tick == 0 && sec == 0 && min == 0)
		flag = 1;
	else
	{
		if (tick != 0)
			tick--;
		else
		{
			/* the game counts 64 ticks per second */
			tick = 0x40;

			/* seconds are counted in BCD */
			if (sec != 0)
				sec = (sec & 0xf) ? sec - 1 : (sec - 0x10) + 9;
			else
			{
				sec = 0x59;

				/* minutes are counted normally */
				if (min != 0)
					min--;
				else
				{
					flag = 1;
					tick = sec = min = 0;
				}
			}
		}
	}
	
	*((UINT16*)(System16Ram + 0x200)) = BURN_ENDIAN_SWAP_INT16((flag << 8) + tick);
	*((UINT16*)(System16Ram + 0x202)) = BURN_ENDIAN_SWAP_INT16((sec << 8) + min);
}

static INT32 BodyslamInit()
{
	Simulate8751 = Bodyslam_Sim8751;

	return System16Init();
}

static void SegaDecode2(const UINT8 xor_table[128],const int swap_table[128])
{
	INT32 A;
	static const UINT8 swaptable[24][4] =
	{
		{ 6,4,2,0 }, { 4,6,2,0 }, { 2,4,6,0 }, { 0,4,2,6 },
		{ 6,2,4,0 }, { 6,0,2,4 }, { 6,4,0,2 }, { 2,6,4,0 },
		{ 4,2,6,0 }, { 4,6,0,2 }, { 6,0,4,2 }, { 0,6,4,2 },
		{ 4,0,6,2 }, { 0,4,6,2 }, { 6,2,0,4 }, { 2,6,0,4 },
		{ 0,6,2,4 }, { 2,0,6,4 }, { 0,2,6,4 }, { 4,2,0,6 },
		{ 2,4,0,6 }, { 4,0,2,6 }, { 2,0,4,6 }, { 0,2,4,6 },
	};

	UINT8 *rom = System16Z80Rom;
	UINT8 *decrypted = System16Z80Code;

	for (A = 0x0000;A < 0x8000;A++)
	{
		INT32 row;
		UINT8 src;
		const UINT8 *tbl;


		src = rom[A];

		/* pick the translation table from bits 0, 3, 6, 9, 12 and 14 of the address */
		row = (A & 1) + (((A >> 3) & 1) << 1) + (((A >> 6) & 1) << 2)
				+ (((A >> 9) & 1) << 3) + (((A >> 12) & 1) << 4) + (((A >> 14) & 1) << 5);

		/* decode the opcodes */
		tbl = swaptable[swap_table[2*row]];
		decrypted[A] = BITSWAP08(src,7,tbl[0],5,tbl[1],3,tbl[2],1,tbl[3]) ^ xor_table[2*row];

		/* decode the data */
		tbl = swaptable[swap_table[2*row+1]];
		rom[A] = BITSWAP08(src,7,tbl[0],5,tbl[1],3,tbl[2],1,tbl[3]) ^ xor_table[2*row+1];
	}
}

static INT32 FantzonepDecryptZ80()
{
	System16Z80Code = (UINT8*)BurnMalloc(0x8000);
	
	static const UINT8 xor_table[128] =
	{
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,0x41,0x45,0x00,0x50,0x54,0x11,0x45,0x40,
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,0x41,0x45,0x00,0x50,0x54,0x11,0x45,0x40,
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,

		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,0x41,0x45,0x00,0x50,0x54,0x11,0x45,0x40,
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,0x41,0x45,0x00,0x50,0x54,0x11,0x45,0x40,
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,
	};

	static const INT32 swap_table[128] =
	{
		0,0,0,0,
		1,1,1,1,1,
		2,2,2,2,2,
		3,3,3,3,
		4,4,4,4,4,
		5,5,5,5,5,
		6,6,6,6,6,
		7,7,7,7,7,
		8,8,8,8,
		9,9,9,9,9,
		10,10,10,10,10,
		11,11,11,11,11,
		12,12,12,12,12,
		13,13,

		8,8,8,8,
		9,9,9,9,9,
		10,10,10,10,10,
		11,11,11,11,
		12,12,12,12,12,
		13,13,13,13,13,
		14,14,14,14,14,
		15,15,15,15,15,
		16,16,16,16,
		17,17,17,17,17,
		18,18,18,18,18,
		19,19,19,19,19,
		20,20,20,20,20,
		21,21,
	};

	SegaDecode2(xor_table, swap_table);
	
	return 0;
}

static void FantzonepMapZ80()
{
	ZetMapArea(0x0000, 0x7fff, 0, System16Z80Rom);
	ZetMapArea(0x0000, 0x7fff, 2, System16Z80Code, System16Z80Rom);
	
	ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram);
	ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram);
	ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram);
	ZetMemEnd();
	
	ZetSetInHandler(System16PPIZ80PortRead);
	ZetSetOutHandler(System16Z80PortWrite);
}

static INT32 FantzonepInit()
{
	System16CustomLoadRomDo = FantzonepDecryptZ80;
	System16MapZ80Do = FantzonepMapZ80;
	
	return System16Init();
}

static INT32 FantzonepExit()
{
	BurnFree(System16Z80Code);
	
	return System16Exit();
}

static INT32 MjleagueInit()
{
	System16MakeAnalogInputsDo = MjleagueMakeAnalogInputs;
	
	Mjleague = true;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, MjleagueReadByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 MjleagueExit()
{
	MjleagueTrack1X = 0;
	MjleagueTrack1Y = 0;
	MjleagueTrack2X = 0;
	MjleagueTrack2Y = 0;
	MjleagueBat1 = 0;
	MjleagueBat2 = 0;
	
	Mjleague = false;
	
	return System16Exit();
}

static INT32 MjleagueScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin =  0x029660;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(MjleagueTrack1X);
		SCAN_VAR(MjleagueTrack1Y);
		SCAN_VAR(MjleagueTrack2X);
		SCAN_VAR(MjleagueTrack2Y);
		SCAN_VAR(MjleagueBat1);
		SCAN_VAR(MjleagueBat2);
	}
	
	return System16Scan(nAction, pnMin);;
}

void Quartet_Sim8751()
{
	// X-Scroll Values
	*((UINT16*)(System16TextRam + 0xff8)) = BURN_ENDIAN_SWAP_INT16(((System16Ram[0x0d14 + 1] << 8) | System16Ram[0x0d14 + 0]));
	*((UINT16*)(System16TextRam + 0xffa)) = BURN_ENDIAN_SWAP_INT16(((System16Ram[0x0d18 + 1] << 8) | System16Ram[0x0d18 + 0]));
	
	// Page Values
	*((UINT16*)(System16TextRam + 0xe9e)) = BURN_ENDIAN_SWAP_INT16(((System16Ram[0x0d1c + 1] << 8) | System16Ram[0x0d1c + 0]));
	*((UINT16*)(System16TextRam + 0xe9c)) = BURN_ENDIAN_SWAP_INT16(((System16Ram[0x0d1e + 1] << 8) | System16Ram[0x0d1e + 0]));
}

static INT32 Passsht16aInit()
{
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x70000 - 0x60000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, Passsht16aReadByte);
		SekClose();

		// Handle different sprite rom layout
		UINT8 *pTemp = (UINT8*)BurnMalloc(System16SpriteRomSize);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, System16SpriteRomSize);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x40000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x50000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 QuartetInit()
{
	Simulate8751 = Quartet_Sim8751;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, QuartetReadByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 SdiInit()
{
	System16MakeAnalogInputsDo = SdiMakeAnalogInputs;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x70000 - 0x60000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, SdiReadByte);
		SekClose();
		
		// Handle different sprite rom layout
		UINT8 *pTemp = (UINT8*)BurnMalloc(System16SpriteRomSize);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, System16SpriteRomSize);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x40000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x50000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 SdiExit()
{
	SdiTrack1X = 0;
	SdiTrack1Y = 0;
	SdiTrack2X = 0;
	SdiTrack2Y = 0;

	return System16Exit();
}

static INT32 SdiScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin =  0x029660;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(SdiTrack1X);
		SCAN_VAR(SdiTrack1Y);
		SCAN_VAR(SdiTrack2X);
		SCAN_VAR(SdiTrack2Y);
	}
	
	return System16Scan(nAction, pnMin);;
}

static INT32 ShinobiInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		// Handle different sprite rom layout
		UINT8 *pTemp = (UINT8*)BurnMalloc(System16SpriteRomSize);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, System16SpriteRomSize);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x40000, 0x10000);
			memcpy(System16Sprites + 0x30000, pTemp + 0x60000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x50000, 0x10000);
			memcpy(System16Sprites + 0x70000, pTemp + 0x70000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Sjryuko1Init()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetWriteByteHandler(0, Sjryuko1WriteByte);
		SekSetReadByteHandler(0, Sjryuko1ReadByte);
		SekSetReadWordHandler(0, Sjryuko1ReadWord);
		SekClose();
		
		// Handle different sprite rom layout
		UINT8 *pTemp = (UINT8*)BurnMalloc(System16SpriteRomSize);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, System16SpriteRomSize);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x40000, 0x10000);
			memcpy(System16Sprites + 0x30000, pTemp + 0x60000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x50000, 0x10000);
			memcpy(System16Sprites + 0x70000, pTemp + 0x70000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Sjryuko1Exit()
{
	MahjongInputNum = 0;
	
	return System16Exit();
}

static INT32 Sjryuko1Scan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin =  0x029660;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(MahjongInputNum);
	}
	
	return System16Scan(nAction, pnMin);;
}

static INT32 Wb31Init()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		// Handle different sprite rom layout
		UINT8 *pTemp = (UINT8*)BurnMalloc(System16SpriteRomSize);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, System16SpriteRomSize);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x40000, 0x10000);
			memcpy(System16Sprites + 0x30000, pTemp + 0x60000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x50000, 0x10000);
			memcpy(System16Sprites + 0x70000, pTemp + 0x70000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

/*====================================================
Driver defs
====================================================*/

struct BurnDriver BurnDrvAceattaca = {
	"aceattaca", "aceattac", NULL, NULL, "1986",
	"Ace Attacker (Japan, System 16A, FD1094 317-0060)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1094_ENC, GBF_SPORTSMISC, 0,
	NULL, AceattacaRomInfo, AceattacaRomName, NULL, NULL, AceattacaInputInfo, AceattacaDIPInfo,
	AceattacaInit, AceattacaExit, System16AFrame, NULL, AceattacaScan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvAFighter = {
	"afighter", NULL, NULL, NULL, "1986",
	"Action Fighter, FD1089A 317-0018\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1089A_ENC, GBF_RACING, 0,
	NULL, AfighterRomInfo, AfighterRomName, NULL, NULL, System16aInputInfo, AfighterDIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvAlexkidd = {
	"alexkidd", NULL, NULL, NULL, "1986",
	"Alex Kidd: The Lost Stars (set 2, unprotected)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16A, GBF_PLATFORM, 0,
	NULL, AlexkiddRomInfo, AlexkiddRomName, NULL, NULL, System16aInputInfo, AlexkiddDIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAlexkidd1 = {
	"alexkidd1", "alexkidd", NULL, NULL, "1986",
	"Alex Kidd: The Lost Stars (set 1, FD1089A 317-unknown)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1089A_ENC, GBF_PLATFORM, 0,
	NULL, Alexkidd1RomInfo, Alexkidd1RomName, NULL, NULL, System16aInputInfo, AlexkiddDIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAliensynjo = {
	"aliensynjo", "aliensyn", NULL, NULL, "1987",
	"Alien Syndrome (set 1, Japan, old, System 16A, FD1089A 317-0033)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1089A_ENC, GBF_MAZE, 0,
	NULL, AliensynjoRomInfo, AliensynjoRomName, NULL, NULL, System16afire1InputInfo, AliensynDIPInfo,
	AliensynjoInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAliensyn2 = {
	"aliensyn2", "aliensyn", NULL, NULL, "1987",
	"Alien Syndrome (set 2, System 16A, FD1089A 317-0033)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1089A_ENC, GBF_MAZE, 0,
	NULL, Aliensyn2RomInfo, Aliensyn2RomName, NULL, NULL, System16afire1InputInfo, AliensynDIPInfo,
	AliensynjoInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAliensyn5 = {
	"aliensyn5", "aliensyn", NULL, NULL, "1987",
	"Alien Syndrome (set 5, System 16A, FD1089B 317-0037)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1089B_ENC, GBF_MAZE, 0,
	NULL, Aliensyn5RomInfo, Aliensyn5RomName, NULL, NULL, System16afire1InputInfo, AliensynDIPInfo,
	Aliensyn5Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvBodyslam = {
	"bodyslam", NULL, NULL, NULL, "1986",
	"Body Slam (8751 317-0015)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16A, GBF_VSFIGHT, 0,
	NULL, BodyslamRomInfo, BodyslamRomName, NULL, NULL, System16aInputInfo, BodyslamDIPInfo,
	BodyslamInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDumpmtmt = {
	"dumpmtmt", "bodyslam", NULL, NULL, "1986",
	"Dump Matsumoto (Japan, 8751 317-unknown)\0", NULL, "Sega", "System 16A",
	 L"Dump Matsumoto (Japan, 8751 317-unknown)\0\u30C0\u30F3\u30D7\u677E\u672C\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A, GBF_VSFIGHT, 0,
	NULL, DumpmtmtRomInfo, DumpmtmtRomName, NULL, NULL, System16aInputInfo, BodyslamDIPInfo,
	BodyslamInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFantzone = {
	"fantzone", NULL, NULL, NULL, "1986",
	"Fantasy Zone (Rev A, unprotected)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16A, GBF_HORSHOOT, 0,
	NULL, FantzoneRomInfo, FantzoneRomName, NULL, NULL, System16aInputInfo, FantzoneDIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFantzone1 = {
	"fantzone1", "fantzone", NULL, NULL, "1986",
	"Fantasy Zone (unprotected)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A, GBF_HORSHOOT, 0,
	NULL, Fantzone1RomInfo, Fantzone1RomName, NULL, NULL, System16aInputInfo, FantzoneDIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFantzonep = {
	"fantzonep", "fantzone", NULL, NULL, "1986",
	"Fantasy Zone (317-5000)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A, GBF_HORSHOOT, 0,
	NULL, FantzonepRomInfo, FantzonepRomName, NULL, NULL, System16aInputInfo, FantzoneDIPInfo,
	FantzonepInit, FantzonepExit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMjleague = {
	"mjleague", NULL, NULL, NULL, "1985",
	"Major League\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16A, GBF_SPORTSMISC, 0,
	NULL, MjleagueRomInfo, MjleagueRomName, NULL, NULL, MjleagueInputInfo, MjleagueDIPInfo,
	MjleagueInit, MjleagueExit, System16AFrame, NULL, MjleagueScan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvPasssht16a = {
	"passsht16a", "passsht", NULL, NULL, "1988",
	"Passing Shot (Japan, 4 Players, System 16A, FD1094 317-0071)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 4, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1094_ENC, GBF_SPORTSMISC, 0,
	NULL, Passsht16aRomInfo, Passsht16aRomName, NULL, NULL, Passsht16aInputInfo, Passsht16aDIPInfo,
	Passsht16aInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvQuartet = {
	"quartet", NULL, NULL, NULL, "1986",
	"Quartet (Rev A, 8751 317-unknown)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_SEGA_SYSTEM16A, GBF_PLATFORM, 0,
	NULL, QuartetRomInfo, QuartetRomName, NULL, NULL, QuartetInputInfo, QuartetDIPInfo,
	QuartetInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQuarteta = {
	"quarteta", "quartet", NULL, NULL, "1986",
	"Quartet (8751 315-5194)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_SEGA_SYSTEM16A, GBF_PLATFORM, 0,
	NULL, QuartetaRomInfo, QuartetaRomName, NULL, NULL, QuartetInputInfo, QuartetDIPInfo,
	QuartetInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQuartet2 = {
	"quartet2", "quartet", NULL, NULL, "1986",
	"Quartet 2 (8751 317-0010)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A, GBF_PLATFORM, 0,
	NULL, Quartet2RomInfo, Quartet2RomName, NULL, NULL, System16aInputInfo, Quart2DIPInfo,
	QuartetInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQuartet2a = {
	"quartet2a", "quartet", NULL, NULL, "1986",
	"Quartet 2 (unprotected)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A, GBF_PLATFORM, 0,
	NULL, Quartet2aRomInfo, Quartet2aRomName, NULL, NULL, System16aInputInfo, Quart2DIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSdi = {
	"sdi", NULL, NULL, NULL, "1987",
	"SDI - Strategic Defense Initiative (Japan, old, System 16A, FD1089B 317-0027)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1089B_ENC, GBF_SHOOT, 0,
	NULL, SdiRomInfo, SdiRomName, NULL, NULL, SdiInputInfo, SdiDIPInfo,
	SdiInit, SdiExit, System16AFrame, NULL, SdiScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinobi = {
	"shinobi", NULL, NULL, NULL, "1987",
	"Shinobi (set 6, System 16A, unprotected)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16A, GBF_PLATFORM, 0,
	NULL, ShinobiRomInfo, ShinobiRomName, NULL, NULL, System16afire3InputInfo, ShinobiDIPInfo,
	ShinobiInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinobi1 = {
	"shinobi1", "shinobi", NULL, NULL, "1987",
	"Shinobi (set 1, System 16A, FD1094 317-0050)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, Shinobi1RomInfo, Shinobi1RomName, NULL, NULL, System16afire3InputInfo, ShinobiDIPInfo,
	ShinobiInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinobls = {
	"shinobls", "shinobi", NULL, NULL, "1987",
	"Shinobi (Star bootleg, System 16A)\0", NULL, "bootleg", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16A, GBF_PLATFORM, 0,
	NULL, ShinoblsRomInfo, ShinoblsRomName, NULL, NULL, System16afire3InputInfo, ShinobiDIPInfo,
	ShinobiInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinoblb = {
	"shinoblb", "shinobi", NULL, NULL, "1987",
	"Shinobi (beta bootleg, System 16A)\0", NULL, "bootleg", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16A, GBF_PLATFORM, 0,
	NULL, ShinoblbRomInfo, ShinoblbRomName, NULL, NULL, System16afire3InputInfo, ShinobiDIPInfo,
	ShinobiInit, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSjryuko1 = {
	"sjryuko1", "sjryuko", NULL, NULL, "1987",
	"Sukeban Jansi Ryuko (set 1, System 16A, FD1089B 317-5021)\0", NULL, "White Board", "System 16A",
	L"Sukeban Jansi Ryuko (set 1, System 16A, FD1089B 317-5021)\0\u30B9\u30B1\u30D0\u30F3\u96C0\u58EB \u7ADC\u5B50\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1089B_ENC, GBF_MAHJONG, 0,
	NULL, Sjryuko1RomInfo, Sjryuko1RomName, NULL, NULL, Sjryuko1InputInfo, Sjryuko1DIPInfo,
	Sjryuko1Init, Sjryuko1Exit, System16AFrame, NULL, Sjryuko1Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTetris = {
	"tetris", NULL, NULL, NULL, "1988",
	"Tetris (set 4, Japan, System 16A, FD1094 317-0093)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1094_ENC, GBF_PUZZLE, 0,
	NULL, TetrisRomInfo, TetrisRomName, NULL, NULL, System16afire1InputInfo, TetrisDIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTetris3 = {
	"tetris3", "tetris", NULL, NULL, "1988",
	"Tetris (set 3, Japan, System 16A, FD1094 317-0093a)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1094_ENC, GBF_PUZZLE, 0,
	NULL, Tetris3RomInfo, Tetris3RomName, NULL, NULL, System16afire1InputInfo, TetrisDIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTimescan1 = {
	"timescan1", "timescan", NULL, NULL, "1987",
	"Time Scanner (set 1, System 16A, FD1089B 317-0024)\0", NULL, "Sega", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1089B_ENC, GBF_PINBALL, 0,
	NULL, Timescan1RomInfo, Timescan1RomName, NULL, NULL, System16aDip3InputInfo, TimescanDIPInfo,
	System16Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvWb31 = {
	"wb31", "wb3", NULL, NULL, "1988",
	"Wonder Boy III - Monster Lair (set 1, System 16A, FD1094 317-0084)\0", NULL, "Sega / Westone", "System 16A",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16A | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, Wb31RomInfo, Wb31RomName, NULL, NULL, System16aInputInfo, Wb31DIPInfo,
	Wb31Init, System16Exit, System16AFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};
