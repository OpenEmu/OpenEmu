#include "sys16.h"

/*====================================================
Input defs
====================================================*/

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo System16bInputList[] = {
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

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(System16b)

static struct BurnInputInfo System16bfire1InputList[] = {
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

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(System16bfire1)

static struct BurnInputInfo System16bfire3InputList[] = {
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

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(System16bfire3)

static struct BurnInputInfo System16bfire4InputList[] = {
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
	{"P1 Fire 4"         , BIT_DIGITAL  , System16InputPort1 + 3, "p1 fire 4" },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort2 + 5, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort2 + 4, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort2 + 7, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort2 + 6, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort2 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , System16InputPort2 + 2, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL  , System16InputPort2 + 3, "p2 fire 4" },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(System16bfire4)

static struct BurnInputInfo System16bDip3InputList[] = {
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

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
	{"Dip 3"             , BIT_DIPSWITCH, System16Dip + 2        , "dip"      },
};

STDINPUTINFO(System16bDip3)

static struct BurnInputInfo AtomicpInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort1 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort1 + 6, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort0 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , System16InputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(Atomicp)

static struct BurnInputInfo BulletInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },
	{"Coin 3"            , BIT_DIGITAL  , System16InputPort0 + 7, "p3 coin"   },
	{"Start 3"           , BIT_DIGITAL  , System16InputPort0 + 6, "p3 start"  },

	{"P1 Up 1"           , BIT_DIGITAL  , System16InputPort1 + 1, "p1 up"     },
	{"P1 Down 1"         , BIT_DIGITAL  , System16InputPort1 + 0, "p1 down"   },
	{"P1 Left 1"         , BIT_DIGITAL  , System16InputPort1 + 3, "p1 left"   },
	{"P1 Right 1"        , BIT_DIGITAL  , System16InputPort1 + 2, "p1 right"  },
	{"P1 Up 2"           , BIT_DIGITAL  , System16InputPort1 + 5, "p1 up 2"   },
	{"P1 Down 2"         , BIT_DIGITAL  , System16InputPort1 + 4, "p1 down 2" },
	{"P1 Left 2"         , BIT_DIGITAL  , System16InputPort1 + 7, "p1 left 2" },
	{"P1 Right 2"        , BIT_DIGITAL  , System16InputPort1 + 6, "p1 right 2"},	

	{"P2 Up 1"           , BIT_DIGITAL  , System16InputPort2 + 1, "p2 up"     },
	{"P2 Down 1"         , BIT_DIGITAL  , System16InputPort2 + 0, "p2 down"   },
	{"P2 Left 1"         , BIT_DIGITAL  , System16InputPort2 + 3, "p2 left"   },
	{"P2 Right 1"        , BIT_DIGITAL  , System16InputPort2 + 2, "p2 right"  },
	{"P2 Up 2"           , BIT_DIGITAL  , System16InputPort2 + 5, "p2 up 2"   },
	{"P2 Down 2"         , BIT_DIGITAL  , System16InputPort2 + 4, "p2 down 2" },
	{"P2 Left 2"         , BIT_DIGITAL  , System16InputPort2 + 7, "p2 left 2" },
	{"P2 Right 2"        , BIT_DIGITAL  , System16InputPort2 + 6, "p2 right 2"},	
	
	{"P3 Up 1"           , BIT_DIGITAL  , System16InputPort3 + 1, "p3 up"     },
	{"P3 Down 1"         , BIT_DIGITAL  , System16InputPort3 + 0, "p3 down"   },
	{"P3 Left 1"         , BIT_DIGITAL  , System16InputPort3 + 3, "p3 left"   },
	{"P3 Right 1"        , BIT_DIGITAL  , System16InputPort3 + 2, "p3 right"  },
	{"P3 Up 2"           , BIT_DIGITAL  , System16InputPort3 + 5, "p3 up 2"   },
	{"P3 Down 2"         , BIT_DIGITAL  , System16InputPort3 + 4, "p3 down 2" },
	{"P3 Left 2"         , BIT_DIGITAL  , System16InputPort3 + 7, "p3 left 2" },
	{"P3 Right 2"        , BIT_DIGITAL  , System16InputPort3 + 6, "p3 right 2"},

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(Bullet)

static struct BurnInputInfo DunkshotInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , System16InputPort0 + 5, "p2 start"  },
	{"Start 3"           , BIT_DIGITAL   , System16InputPort0 + 6, "p3 start"  },
	{"Start 4"           , BIT_DIGITAL   , System16InputPort0 + 7, "p4 start"  },

	{"P1 Up"             , BIT_DIGITAL   , System16InputPort2 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , System16InputPort2 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , System16InputPort2 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , System16InputPort2 + 1, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , System16InputPort1 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , System16InputPort1 + 1, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL   , System16InputPort2 + 6, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , System16InputPort2 + 7, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , System16InputPort2 + 4, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , System16InputPort2 + 5, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , System16InputPort1 + 2, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , System16InputPort1 + 3, "p2 fire 2" },
	
	{"P3 Up"             , BIT_DIGITAL   , System16InputPort3 + 2, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL   , System16InputPort3 + 3, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL   , System16InputPort3 + 0, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL   , System16InputPort3 + 1, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL   , System16InputPort1 + 4, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL   , System16InputPort1 + 5, "p3 fire 2" },

	{"P4 Up"             , BIT_DIGITAL   , System16InputPort3 + 6, "p4 up"     },
	{"P4 Down"           , BIT_DIGITAL   , System16InputPort3 + 7, "p4 down"   },
	{"P4 Left"           , BIT_DIGITAL   , System16InputPort3 + 4, "p4 left"   },
	{"P4 Right"          , BIT_DIGITAL   , System16InputPort3 + 5, "p4 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL   , System16InputPort1 + 6, "p4 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL   , System16InputPort1 + 7, "p4 fire 2" },

	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"      },
};

STDINPUTINFO(Dunkshot)

static struct BurnInputInfo ExctleagInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"      },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 4, "p1 start"     },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"      },
	{"Start 2"           , BIT_DIGITAL   , System16InputPort0 + 5, "p2 start"     },

	{"P1 Up"             , BIT_DIGITAL   , System16InputPort4 + 2, "p1 up"        },
	{"P1 Down"           , BIT_DIGITAL   , System16InputPort4 + 3, "p1 down"      },
	{"P1 Left"           , BIT_DIGITAL   , System16InputPort4 + 0, "p1 left"      },
	{"P1 Right"          , BIT_DIGITAL   , System16InputPort4 + 1, "p1 right"     },
	{"P1 Bant0"          , BIT_DIGITAL   , System16InputPort1 + 0, "p1 fire 1"    },
	{"P1 Bant1"          , BIT_DIGITAL   , System16InputPort1 + 1, "p1 fire 2"    },
	{"P1 Bant2"          , BIT_DIGITAL   , System16InputPort1 + 2, "p1 fire 3"    },
	{"P1 Swing0"         , BIT_DIGITAL   , System16InputPort1 + 3, "p1 fire 4"    },
	{"P1 Swing1"         , BIT_DIGITAL   , System16InputPort1 + 4, "p1 fire 5"    },
	{"P1 Low"            , BIT_DIGITAL   , System16InputPort1 + 5, "p1 fire 6"    },
	{"P1 Mid"            , BIT_DIGITAL   , System16InputPort1 + 6, "p1 fire 7"    },
	{"P1 Hi"             , BIT_DIGITAL   , System16InputPort1 + 7, "p1 fire 8"    },
	{"P1 Change"         , BIT_DIGITAL   , System16InputPort3 + 1, "p1 fire 9"    },
	{"P1 Select"         , BIT_DIGITAL   , System16InputPort3 + 2, "p1 fire 10"   },
	{"P1 Chase"          , BIT_DIGITAL   , System16InputPort3 + 0, "p1 fire 11"   },
	
	{"P2 Up"             , BIT_DIGITAL   , System16InputPort4 + 6, "p2 up"        },
	{"P2 Down"           , BIT_DIGITAL   , System16InputPort4 + 7, "p2 down"      },
	{"P2 Left"           , BIT_DIGITAL   , System16InputPort4 + 4, "p2 left"      },
	{"P2 Right"          , BIT_DIGITAL   , System16InputPort4 + 5, "p2 right"     },
	{"P2 Bant0"          , BIT_DIGITAL   , System16InputPort2 + 0, "p2 fire 1"    },
	{"P2 Bant1"          , BIT_DIGITAL   , System16InputPort2 + 1, "p2 fire 2"    },
	{"P2 Bant2"          , BIT_DIGITAL   , System16InputPort2 + 2, "p2 fire 3"    },
	{"P2 Swing0"         , BIT_DIGITAL   , System16InputPort2 + 3, "p2 fire 4"    },
	{"P2 Swing1"         , BIT_DIGITAL   , System16InputPort2 + 4, "p2 fire 5"    },
	{"P2 Low"            , BIT_DIGITAL   , System16InputPort2 + 5, "p2 fire 6"    },
	{"P2 Mid"            , BIT_DIGITAL   , System16InputPort2 + 6, "p2 fire 7"    },
	{"P2 Hi"             , BIT_DIGITAL   , System16InputPort2 + 7, "p2 fire 8"    },
	{"P2 Change"         , BIT_DIGITAL   , System16InputPort3 + 5, "p2 fire 9"    },
	{"P2 Select"         , BIT_DIGITAL   , System16InputPort3 + 6, "p2 fire 10"   },
	{"P2 Chase"          , BIT_DIGITAL   , System16InputPort3 + 4, "p2 fire 11"   },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"      },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"         },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"        },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"          },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"          },
};

STDINPUTINFO(Exctleag)

static struct BurnInputInfo FpointblInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort1 + 5, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort1 + 4, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort1 + 7, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort1 + 6, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 2, "p1 fire 1" },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort2 + 5, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort2 + 4, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort2 + 7, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort2 + 6, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 2, "p2 fire 1" },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(Fpointbl)

static struct BurnInputInfo HwchampInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , System16InputPort0 + 5, "p2 start"  },

	A("Left/Right"       , BIT_ANALOG_REL, &System16AnalogPort0,   "p1 x-axis"  ),
	A("Left"             , BIT_ANALOG_REL, &System16AnalogPort1,   "p1 fire 1"  ),
	A("Right"            , BIT_ANALOG_REL, &System16AnalogPort2,   "p1 fire 2"  ),
	
	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Hwchamp)

static struct BurnInputInfo PassshtInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort0 + 4, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , System16InputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , System16InputPort0 + 5, "p2 start"  },
	{"Start 3"           , BIT_DIGITAL  , System16InputPort0 + 6, "p3 start"  },
	{"Start 4"           , BIT_DIGITAL  , System16InputPort0 + 7, "p4 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort1 + 1, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort1 + 0, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort1 + 3, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort1 + 2, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort1 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort1 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , System16InputPort1 + 6, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL  , System16InputPort1 + 7, "p1 fire 4" },

	{"P2 Up"             , BIT_DIGITAL  , System16InputPort2 + 1, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , System16InputPort2 + 0, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , System16InputPort2 + 3, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , System16InputPort2 + 2, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , System16InputPort2 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , System16InputPort2 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , System16InputPort2 + 6, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL  , System16InputPort2 + 7, "p2 fire 4" },
	
	{"P3 Up"             , BIT_DIGITAL  , System16InputPort3 + 1, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL  , System16InputPort3 + 0, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL  , System16InputPort3 + 3, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL  , System16InputPort3 + 2, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL  , System16InputPort3 + 4, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL  , System16InputPort3 + 5, "p3 fire 2" },
	{"P3 Fire 3"         , BIT_DIGITAL  , System16InputPort3 + 6, "p3 fire 3" },
	{"P3 Fire 4"         , BIT_DIGITAL  , System16InputPort3 + 7, "p3 fire 4" },

	{"P4 Up"             , BIT_DIGITAL  , System16InputPort4 + 1, "p4 up"     },
	{"P4 Down"           , BIT_DIGITAL  , System16InputPort4 + 0, "p4 down"   },
	{"P4 Left"           , BIT_DIGITAL  , System16InputPort4 + 3, "p4 left"   },
	{"P4 Right"          , BIT_DIGITAL  , System16InputPort4 + 2, "p4 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL  , System16InputPort4 + 4, "p4 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL  , System16InputPort4 + 5, "p4 fire 2" },
	{"P4 Fire 3"         , BIT_DIGITAL  , System16InputPort4 + 6, "p4 fire 3" },
	{"P4 Fire 4"         , BIT_DIGITAL  , System16InputPort4 + 7, "p4 fire 4" },

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(Passsht)

static struct BurnInputInfo RyukyuInputList[] = {
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

	{"Service"           , BIT_DIGITAL  , System16InputPort0 + 3 , "service"  },
	{"Diagnostics"       , BIT_DIGITAL  , System16InputPort0 + 2 , "diag"     },
	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(Ryukyu)

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

static struct BurnInputInfo SjryukoInputList[] = {
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

STDINPUTINFO(Sjryuko)

static struct BurnInputInfo SnapperInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , System16InputPort1 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , System16InputPort1 + 1, "p1 start"  },

	{"P1 Up"             , BIT_DIGITAL  , System16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , System16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , System16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , System16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , System16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , System16InputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , System16InputPort0 + 6, "p1 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &System16Reset         , "reset"    },
	{"Dip 1"             , BIT_DIPSWITCH, System16Dip + 0        , "dip"      },
	{"Dip 2"             , BIT_DIPSWITCH, System16Dip + 1        , "dip"      },
};

STDINPUTINFO(Snapper)

#undef A

/*====================================================
Dip defs
====================================================*/

#define SYSTEM16B_COINAGE(dipval)								\
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

static struct BurnDIPInfo AliensynDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfd, NULL                                 },
	{0x12, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x11, 0x01, 0x02, 0x02, "Off"                                },
	{0x11, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x11, 0x01, 0x0c, 0x08, "2"                                  },
	{0x11, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x11, 0x01, 0x0c, 0x04, "4"                                  },
	{0x11, 0x01, 0x0c, 0x00, "127"                                },
	
	{0   , 0xfe, 0   , 4   , "Timer"                              },
	{0x11, 0x01, 0x30, 0x00, "120"                                },
	{0x11, 0x01, 0x30, 0x10, "130"                                },
	{0x11, 0x01, 0x30, 0x20, "140"                                },
	{0x11, 0x01, 0x30, 0x30, "150"                                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x11, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x11, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x11, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x11, 0x01, 0xc0, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x12)
};

STDDIPINFO(Aliensyn)

static struct BurnDIPInfo AliensynjDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfd, NULL                                 },
	{0x12, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x11, 0x01, 0x02, 0x02, "Off"                                },
	{0x11, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x11, 0x01, 0x0c, 0x08, "2"                                  },
	{0x11, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x11, 0x01, 0x0c, 0x04, "4"                                  },
	{0x11, 0x01, 0x0c, 0x00, "127"                                },
	
	{0   , 0xfe, 0   , 4   , "Timer"                              },
	{0x11, 0x01, 0x30, 0x00, "150"                                },
	{0x11, 0x01, 0x30, 0x10, "160"                                },
	{0x11, 0x01, 0x30, 0x20, "170"                                },
	{0x11, 0x01, 0x30, 0x30, "180"                                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x11, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x11, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x11, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x11, 0x01, 0xc0, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x12)
};

STDDIPINFO(Aliensynj)

static struct BurnDIPInfo AltbeastDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfd, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Credits needed"                     },
	{0x15, 0x01, 0x01, 0x01, "1 to start, 1 to continue"          },
	{0x15, 0x01, 0x01, 0x00, "2 to start, 1 to continue"          },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x02, 0x02, "Off"                                },
	{0x15, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x15, 0x01, 0x0c, 0x08, "2"                                  },
	{0x15, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x15, 0x01, 0x0c, 0x04, "4"                                  },
	{0x15, 0x01, 0x0c, 0x00, "240"                                },
	
	{0   , 0xfe, 0   , 4   , "Energy Meter"                       },
	{0x15, 0x01, 0x30, 0x20, "2"                                  },
	{0x15, 0x01, 0x30, 0x30, "3"                                  },
	{0x15, 0x01, 0x30, 0x10, "4"                                  },
	{0x15, 0x01, 0x30, 0x00, "5"                                  },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x15, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x15, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x15, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x15, 0x01, 0xc0, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Altbeast)

static struct BurnDIPInfo AtomicpDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                                 },
	{0x12, 0xff, 0xff, 0x2f, NULL                                 },
	
	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                             },
	{0x11, 0x01, 0x38, 0x00, "4 Coins 1 Credit"                   },
	{0x11, 0x01, 0x38, 0x20, "3 Coins 1 Credit"                   },
	{0x11, 0x01, 0x38, 0x18, "2 Coins 1 Credit"                   },
	{0x11, 0x01, 0x38, 0x38, "1 Coin  1 Credit"                   },
	{0x11, 0x01, 0x38, 0x10, "1 Coin  2 Credits"                  },
	{0x11, 0x01, 0x38, 0x28, "1 Coin  3 Credits"                  },
	{0x11, 0x01, 0x38, 0x08, "1 Coin  4 Credits"                  },
	{0x11, 0x01, 0x38, 0x30, "1 Coin  5 Credits"                  },

	{0   , 0xfe, 0   , 8   , "Coin B"                             },
	{0x11, 0x01, 0x07, 0x00, "4 Coins 1 Credit"                   },
	{0x11, 0x01, 0x07, 0x04, "3 Coins 1 Credit"                   },
	{0x11, 0x01, 0x07, 0x03, "2 Coins 1 Credit"                   },
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credit"                   },
	{0x11, 0x01, 0x07, 0x02, "1 Coin  2 Credits"                  },
	{0x11, 0x01, 0x07, 0x05, "1 Coin  3 Credits"                  },
	{0x11, 0x01, 0x07, 0x01, "1 Coin  4 Credits"                  },
	{0x11, 0x01, 0x07, 0x06, "1 Coin  5 Credits"                  },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x11, 0x01, 0xc0, 0xc0, "1"                                  },
	{0x11, 0x01, 0xc0, 0x80, "2"                                  },
	{0x11, 0x01, 0xc0, 0x40, "3"                                  },
	{0x11, 0x01, 0xc0, 0x00, "5"                                  },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x12, 0x01, 0x03, 0x01, "Easy"                               },
	{0x12, 0x01, 0x03, 0x03, "Normal"                             },
	{0x12, 0x01, 0x03, 0x02, "Hard"                               },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 2   , "Level Select"                       },
	{0x12, 0x01, 0x04, 0x04, "Off"                                },
	{0x12, 0x01, 0x04, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"                        },
	{0x12, 0x01, 0x08, 0x08, "Off"                                },
	{0x12, 0x01, 0x08, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x12, 0x01, 0x10, 0x10, "Off"                                },
	{0x12, 0x01, 0x10, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                     },
	{0x12, 0x01, 0x20, 0x00, "Off"                                },
	{0x12, 0x01, 0x20, 0x20, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Game Mode"                          },
	{0x12, 0x01, 0x40, 0x40, "Normal Tetris"                      },
	{0x12, 0x01, 0x40, 0x00, "Atomic Point"                       },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x12, 0x01, 0x80, 0x00, "Off"                                },
	{0x12, 0x01, 0x80, 0x80, "On"                                 },
};

STDDIPINFO(Atomicp)

static struct BurnDIPInfo AurailDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfd, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x15, 0x01, 0x01, 0x01, "Upright"                            },
	{0x15, 0x01, 0x01, 0x00, "Cocktail"                           },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x02, 0x02, "Off"                                },
	{0x15, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x15, 0x01, 0x0c, 0x00, "2"                                  },
	{0x15, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x15, 0x01, 0x0c, 0x08, "4"                                  },
	{0x15, 0x01, 0x0c, 0x04, "5"                                  },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"                         },
	{0x15, 0x01, 0x10, 0x10, "80k, 200k, 500k, 1000k"             },
	{0x15, 0x01, 0x10, 0x00, "100k, 300k, 700k, 1000k"            },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"                         },
	{0x15, 0x01, 0x20, 0x20, "Normal"                             },
	{0x15, 0x01, 0x20, 0x00, "Hard"                               },
	
	{0   , 0xfe, 0   , 2   , "Controller Select"                  },
	{0x15, 0x01, 0x40, 0x40, "1 Player Side"                      },
	{0x15, 0x01, 0x40, 0x00, "2 Players Side"                     },
	
	{0   , 0xfe, 0   , 2   , "Special Function Mode"              },
	{0x15, 0x01, 0x80, 0x80, "Off"                                },
	{0x15, 0x01, 0x80, 0x00, "On"                                 },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Aurail)

static struct BurnDIPInfo BayrouteDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfd, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                     },
	{0x15, 0x01, 0x01, 0x00, "Off"                                },
	{0x15, 0x01, 0x01, 0x01, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x02, 0x02, "Off"                                },
	{0x15, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x15, 0x01, 0x0c, 0x04, "1"                                  },
	{0x15, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x15, 0x01, 0x0c, 0x08, "5"                                  },
	{0x15, 0x01, 0x0c, 0x00, "Unlimited"                          },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                         },
	{0x15, 0x01, 0x30, 0x30, "100000"                             },
	{0x15, 0x01, 0x30, 0x20, "150000"                             },
	{0x15, 0x01, 0x30, 0x10, "200000"                             },
	{0x15, 0x01, 0x30, 0x00, "None"                               },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x15, 0x01, 0xc0, 0xc0, "Easy"                               },
	{0x15, 0x01, 0xc0, 0x80, "Normal"                             },
	{0x15, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x15, 0x01, 0xc0, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Bayroute)

static struct BurnDIPInfo Blox16bDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfe, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },
};

STDDIPINFO(Blox16b)

static struct BurnDIPInfo BulletDIPList[]=
{
	// Default Values
	{0x21, 0xff, 0xff, 0xff, NULL                                 },
	{0x22, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	
	// Dip 2
	SYSTEM16B_COINAGE(0x22)
};

STDDIPINFO(Bullet)

static struct BurnDIPInfo CottonDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x13, 0x01, 0x01, 0x01, "Off"                                },
	{0x13, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x13, 0x01, 0x06, 0x04, "2"                                  },
	{0x13, 0x01, 0x06, 0x06, "3"                                  },
	{0x13, 0x01, 0x06, 0x02, "4"                                  },
	{0x13, 0x01, 0x06, 0x00, "5"                                  },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x13, 0x01, 0x18, 0x10, "Easy"                               },
	{0x13, 0x01, 0x18, 0x18, "Normal"                             },
	{0x13, 0x01, 0x18, 0x08, "Hard"                               },
	{0x13, 0x01, 0x18, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x14)
};

STDDIPINFO(Cotton)

static struct BurnDIPInfo DduxDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x13, 0x01, 0x01, 0x01, "Off"                                },
	{0x13, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x13, 0x01, 0x06, 0x04, "Easy"                               },
	{0x13, 0x01, 0x06, 0x06, "Normal"                             },
	{0x13, 0x01, 0x06, 0x02, "Hard"                               },
	{0x13, 0x01, 0x06, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x13, 0x01, 0x18, 0x10, "2"                                  },
	{0x13, 0x01, 0x18, 0x18, "3"                                  },
	{0x13, 0x01, 0x18, 0x08, "4"                                  },
	{0x13, 0x01, 0x18, 0x00, "5"                                  },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                         },
	{0x13, 0x01, 0x60, 0x40, "150000"                             },
	{0x13, 0x01, 0x60, 0x60, "200000"                             },
	{0x13, 0x01, 0x60, 0x20, "300000"                             },
	{0x13, 0x01, 0x60, 0x00, "400000"                             },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x14)
};

STDDIPINFO(Ddux)

static struct BurnDIPInfo DunkshotDIPList[]=
{
	// Default Values
	{0x21, 0xff, 0xff, 0xfd, NULL                                 },
	{0x22, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x21, 0x01, 0x02, 0x02, "Off"                                },
	{0x21, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "VS Time"                            },
	{0x21, 0x01, 0x0c, 0x08, "2P 1:30 | 3P 2:00 | 4P 2:30"        },
	{0x21, 0x01, 0x0c, 0x0c, "2P 2:00 | 3P 2:30 | 4P 3:00"        },
	{0x21, 0x01, 0x0c, 0x04, "2P 2:30 | 3P 3:00 | 4P 3:30"        },
	{0x21, 0x01, 0x0c, 0x00, "2P 3:00 | 3P 3:30 | 4P 4:00"        },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x21, 0x01, 0x30, 0x20, "Easy"                               },
	{0x21, 0x01, 0x30, 0x30, "Normal"                             },
	{0x21, 0x01, 0x30, 0x10, "Hard"                               },
	{0x21, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 2   , "CPU starts with +6 pts"             },
	{0x21, 0x01, 0x40, 0x40, "Off"                                },
	{0x21, 0x01, 0x40, 0x00, "On"                                 },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x22)
};

STDDIPINFO(Dunkshot)

static struct BurnDIPInfo EswatDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfd, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "2 Credits to Start"                 },
	{0x15, 0x01, 0x01, 0x01, "Off"                                },
	{0x15, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x02, 0x02, "Off"                                },
	{0x15, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"                        },
	{0x15, 0x01, 0x04, 0x04, "Off"                                },
	{0x15, 0x01, 0x04, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Time"                               },
	{0x15, 0x01, 0x08, 0x08, "Normal"                             },
	{0x15, 0x01, 0x08, 0x00, "Hard"                               },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x15, 0x01, 0x30, 0x20, "Easy"                               },
	{0x15, 0x01, 0x30, 0x30, "Normal"                             },
	{0x15, 0x01, 0x30, 0x10, "Hard"                               },
	{0x15, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x15, 0x01, 0xc0, 0x00, "1"                                  },
	{0x15, 0x01, 0xc0, 0x40, "2"                                  },
	{0x15, 0x01, 0xc0, 0xc0, "3"                                  },
	{0x15, 0x01, 0xc0, 0x80, "4"                                  },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Eswat)

static struct BurnDIPInfo ExctleagDIPList[]=
{
	// Default Values
	{0x25, 0xff, 0xff, 0xfe, NULL                                 },
	{0x26, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x25, 0x01, 0x01, 0x01, "Off"                                },
	{0x25, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 8   , "Starting Points"                    },
	{0x25, 0x01, 0x0e, 0x06, "2000"                               },
	{0x25, 0x01, 0x0e, 0x0a, "3000"                               },
	{0x25, 0x01, 0x0e, 0x0c, "4000"                               },
	{0x25, 0x01, 0x0e, 0x0e, "5000"                               },	
	{0x25, 0x01, 0x0e, 0x08, "6000"                               },	
	{0x25, 0x01, 0x0e, 0x04, "7000"                               },
	{0x25, 0x01, 0x0e, 0x02, "8000"                               },
	{0x25, 0x01, 0x0e, 0x00, "9000"                               },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x25, 0x01, 0x30, 0x20, "Easy"                               },
	{0x25, 0x01, 0x30, 0x30, "Normal"                             },
	{0x25, 0x01, 0x30, 0x10, "Hard"                               },
	{0x25, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Point Table"                        },
	{0x25, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x25, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x25, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x25, 0x01, 0xc0, 0x00, "Hardest"                            },
	
	// Dip 1
	SYSTEM16B_COINAGE(0x26)
};

STDDIPINFO(Exctleag)

static struct BurnDIPInfo Fantzn2xDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x13, 0x01, 0x0c, 0x08, "2"                                  },
	{0x13, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x13, 0x01, 0x0c, 0x04, "4"                                  },
	{0x13, 0x01, 0x0c, 0x00, "240"                                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x13, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x13, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x13, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x13, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Fantzn2x)

static struct BurnDIPInfo FpointDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfd, NULL                                 },
	{0x12, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x11, 0x01, 0x02, 0x02, "Off"                                },
	{0x11, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x11, 0x01, 0x30, 0x20, "Easy"                               },
	{0x11, 0x01, 0x30, 0x30, "Normal"                             },
	{0x11, 0x01, 0x30, 0x10, "Hard"                               },
	{0x11, 0x01, 0x30, 0x00, "Hardest"                            },

	{0   , 0xfe, 0   , 2   , "Clear Round Allowed"                },
	{0x11, 0x01, 0x40, 0x00, "1"                                  },
	{0x11, 0x01, 0x40, 0x40, "2"                                  },
	
	{0   , 0xfe, 0   , 2   , "Cell Move"                          },
	{0x11, 0x01, 0x80, 0x00, "Off"                                },
	{0x11, 0x01, 0x80, 0x80, "On"                                 },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x12)
};

STDDIPINFO(Fpoint)

static struct BurnDIPInfo GoldnaxeDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfd, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Credits needed"                     },
	{0x15, 0x01, 0x01, 0x01, "1 to start, 1 to continue"          },
	{0x15, 0x01, 0x01, 0x00, "2 to start, 1 to continue"          },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x02, 0x02, "Off"                                },
	{0x15, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x15, 0x01, 0x0c, 0x08, "1"                                  },
	{0x15, 0x01, 0x0c, 0x0c, "2"                                  },
	{0x15, 0x01, 0x0c, 0x04, "3"                                  },
	{0x15, 0x01, 0x0c, 0x00, "5"                                  },
	
	{0   , 0xfe, 0   , 4   , "Energy Meter"                       },
	{0x15, 0x01, 0x30, 0x20, "2"                                  },
	{0x15, 0x01, 0x30, 0x30, "3"                                  },
	{0x15, 0x01, 0x30, 0x10, "4"                                  },
	{0x15, 0x01, 0x30, 0x00, "5"                                  },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Goldnaxe)

static struct BurnDIPInfo HwchampDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0xf9, NULL                                 },
	{0x0b, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0a, 0x01, 0x02, 0x02, "Off"                                },
	{0x0a, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Start Level Select"                 },
	{0x0a, 0x01, 0x04, 0x04, "Off"                                },
	{0x0a, 0x01, 0x04, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                     },
	{0x0a, 0x01, 0x08, 0x08, "Off"                                },
	{0x0a, 0x01, 0x08, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0a, 0x01, 0x30, 0x20, "Easy"                               },
	{0x0a, 0x01, 0x30, 0x30, "Normal"                             },
	{0x0a, 0x01, 0x30, 0x10, "Hard"                               },
	{0x0a, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Time Adjust"                        },
	{0x0a, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x0a, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x0a, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x0a, 0x01, 0xc0, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x0b)
};

STDDIPINFO(Hwchamp)

static struct BurnDIPInfo MvpDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfd, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Credits Needed"                     },
	{0x15, 0x01, 0x01, 0x01, "1 to start, 1 to continue"          },
	{0x15, 0x01, 0x01, 0x00, "2 to start, 1 to continue"          },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x02, 0x02, "Off"                                },
	{0x15, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Innings"                            },
	{0x15, 0x01, 0x04, 0x04, "1 Credit 1 Inning Only"             },
	{0x15, 0x01, 0x04, 0x0c, "+2 Credits 3 Innings"               },
	
	{0   , 0xfe, 0   , 8   , "Time Limits"                        },
	{0x15, 0x01, 0x38, 0x18, "Easy"                               },
	{0x15, 0x01, 0x38, 0x28, "Easy 2"                             },
	{0x15, 0x01, 0x38, 0x08, "Easy 3"                             },
	{0x15, 0x01, 0x38, 0x38, "Normal"                             },
	{0x15, 0x01, 0x38, 0x30, "Hard"                               },
	{0x15, 0x01, 0x38, 0x10, "Hard 2"                             },
	{0x15, 0x01, 0x38, 0x20, "Hard 3"                             },
	{0x15, 0x01, 0x38, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x15, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x15, 0x01, 0xc0, 0x40, "Easy 2"                             },
	{0x15, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x15, 0x01, 0xc0, 0x30, "Hard"                               },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Mvp)

static struct BurnDIPInfo PassshtDIPList[]=
{
	// Default Values
	{0x17, 0xff, 0xff, 0xf0, NULL                                 },
	{0x18, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x17, 0x01, 0x01, 0x01, "Off"                                },
	{0x17, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 8   , "Initial Point"                      },
	{0x17, 0x01, 0x0e, 0x06, "2000"                               },
	{0x17, 0x01, 0x0e, 0x0a, "3000"                               },
	{0x17, 0x01, 0x0e, 0x0c, "4000"                               },
	{0x17, 0x01, 0x0e, 0x0e, "5000"                               },
	{0x17, 0x01, 0x0e, 0x08, "6000"                               },
	{0x17, 0x01, 0x0e, 0x04, "7000"                               },
	{0x17, 0x01, 0x0e, 0x02, "8000"                               },
	{0x17, 0x01, 0x0e, 0x00, "9000"                               },
	
	{0   , 0xfe, 0   , 4   , "Point Table"                        },
	{0x17, 0x01, 0x30, 0x20, "Easy"                               },
	{0x17, 0x01, 0x30, 0x30, "Normal"                             },
	{0x17, 0x01, 0x30, 0x10, "Hard"                               },
	{0x17, 0x01, 0x30, 0x00, "Hardest"                            },	
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x17, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x17, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x17, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x17, 0x01, 0xc0, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x18)
};

STDDIPINFO(Passsht)


static struct BurnDIPInfo PassshtaDIPList[]=
{
	// Default Values
	{0x29, 0xff, 0xff, 0xf0, NULL                                 },
	{0x2a, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x29, 0x01, 0x01, 0x01, "Off"                                },
	{0x29, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 8   , "Initial Point"                      },
	{0x29, 0x01, 0x0e, 0x06, "2000"                               },
	{0x29, 0x01, 0x0e, 0x0a, "3000"                               },
	{0x29, 0x01, 0x0e, 0x0c, "4000"                               },
	{0x29, 0x01, 0x0e, 0x0e, "5000"                               },
	{0x29, 0x01, 0x0e, 0x08, "6000"                               },
	{0x29, 0x01, 0x0e, 0x04, "7000"                               },
	{0x29, 0x01, 0x0e, 0x02, "8000"                               },
	{0x29, 0x01, 0x0e, 0x00, "9000"                               },
	
	{0   , 0xfe, 0   , 4   , "Point Table"                        },
	{0x29, 0x01, 0x30, 0x20, "Easy"                               },
	{0x29, 0x01, 0x30, 0x30, "Normal"                             },
	{0x29, 0x01, 0x30, 0x10, "Hard"                               },
	{0x29, 0x01, 0x30, 0x00, "Hardest"                            },	
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x29, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x29, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x29, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x29, 0x01, 0xc0, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x2a)
};

STDDIPINFO(Passshta)

static struct BurnDIPInfo RiotcityDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfd, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "2 Credits to Start"                 },
	{0x13, 0x01, 0x01, 0x01, "Off"                                },
	{0x13, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x13, 0x01, 0x02, 0x02, "Off"                                },
	{0x13, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x13, 0x01, 0x0c, 0x00, "1"                                  },
	{0x13, 0x01, 0x0c, 0x0c, "2"                                  },
	{0x13, 0x01, 0x0c, 0x08, "3"                                  },
	{0x13, 0x01, 0x0c, 0x04, "4"                                  },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                         },
	{0x13, 0x01, 0x30, 0x20, "Easy"                               },
	{0x13, 0x01, 0x30, 0x30, "Normal"                             },
	{0x13, 0x01, 0x30, 0x10, "Hard"                               },
	{0x13, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"                         },
	{0x13, 0x01, 0x40, 0x40, "Normal"                             },
	{0x13, 0x01, 0x40, 0x00, "Hard"                               },
	
	{0   , 0xfe, 0   , 2   , "Attack button to start"             },
	{0x13, 0x01, 0x80, 0x80, "Off"                                },
	{0x13, 0x01, 0x80, 0x00, "On"                                 },
		
	// Dip 2
	SYSTEM16B_COINAGE(0x14)
};

STDDIPINFO(Riotcity)

static struct BurnDIPInfo RyukyuDIPList[]=
{
	// Default Values
	{0x0d, 0xff, 0xff, 0xf8, NULL                                 },
	{0x0e, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0d, 0x01, 0x01, 0x01, "Off"                                },
	{0x0d, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Cancel per credit"                  },
	{0x0d, 0x01, 0x02, 0x00, "2"                                  },
	{0x0d, 0x01, 0x02, 0x02, "3"                                  },
	
	{0   , 0xfe, 0   , 2   , "Timer Speed"                        },
	{0x0d, 0x01, 0x04, 0x04, "20 seconds"                         },
	{0x0d, 0x01, 0x04, 0x00, "30 seconds"                         },
	
	{0   , 0xfe, 0   , 2   , "PCM Voice"                          },
	{0x0d, 0x01, 0x08, 0x00, "Off"                                },
	{0x0d, 0x01, 0x08, 0x08, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Omikuji Difficulty"                 },
	{0x0d, 0x01, 0x30, 0x20, "Easy"                               },
	{0x0d, 0x01, 0x30, 0x30, "Normal"                             },
	{0x0d, 0x01, 0x30, 0x10, "Hard"                               },
	{0x0d, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0d, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x0d, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x0d, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x0d, 0x01, 0xc0, 0x00, "Hardest"                            },
		
	// Dip 2
	SYSTEM16B_COINAGE(0x0e)
};

STDDIPINFO(Ryukyu)

static struct BurnDIPInfo SdibDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfd, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                     },
	{0x15, 0x01, 0x01, 0x01, "No"                                 },
	{0x15, 0x01, 0x01, 0x00, "Yes"                                },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x02, 0x02, "Off"                                },
	{0x15, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x15, 0x01, 0x0c, 0x08, "2"                                  },
	{0x15, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x15, 0x01, 0x0c, 0x04, "4"                                  },
	{0x15, 0x01, 0x0c, 0x00, "Free"                               },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x15, 0x01, 0x30, 0x20, "Easy"                               },
	{0x15, 0x01, 0x30, 0x30, "Normal"                             },
	{0x15, 0x01, 0x30, 0x10, "Hard"                               },
	{0x15, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                         },
	{0x15, 0x01, 0xc0, 0x80, "Every 50000"                        },
	{0x15, 0x01, 0xc0, 0xc0, "50000"                              },
	{0x15, 0x01, 0xc0, 0x40, "100000"                             },
	{0x15, 0x01, 0xc0, 0x00, "None"                               },
	
	// Dip 1
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Sdib)

static struct BurnDIPInfo ShinobiDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfc, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x15, 0x01, 0x01, 0x00, "Upright"                            },
	{0x15, 0x01, 0x01, 0x01, "Cocktail"                           },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x02, 0x02, "Off"                                },
	{0x15, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x15, 0x01, 0x0c, 0x08, "2"                                  },
	{0x15, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x15, 0x01, 0x0c, 0x04, "5"                                  },
	{0x15, 0x01, 0x0c, 0x00, "240"                                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x15, 0x01, 0x30, 0x20, "Easy"                               },
	{0x15, 0x01, 0x30, 0x30, "Normal"                             },
	{0x15, 0x01, 0x30, 0x10, "Hard"                               },
	{0x15, 0x01, 0x30, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 2   , "Enemy Bullet Speed"                 },
	{0x15, 0x01, 0x40, 0x40, "Slow"                               },
	{0x15, 0x01, 0x40, 0x00, "Fast"                               },
	
	{0   , 0xfe, 0   , 2   , "Language"                           },
	{0x15, 0x01, 0x80, 0x80, "Japanese"                           },
	{0x15, 0x01, 0x80, 0x00, "English"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Shinobi)

static struct BurnDIPInfo SonicbomDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0x7f, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Difficulty"                         },
	{0x13, 0x01, 0x07, 0x06, "Easy"                               },
	{0x13, 0x01, 0x07, 0x07, "Normal"                             },
	{0x13, 0x01, 0x07, 0x05, "Hard 1"                             },
	{0x13, 0x01, 0x07, 0x04, "Hard 2"                             },
	{0x13, 0x01, 0x07, 0x03, "Hard 3"                             },
	{0x13, 0x01, 0x07, 0x02, "Hard 4"                             },
	{0x13, 0x01, 0x07, 0x01, "Hard 5"                             },
	{0x13, 0x01, 0x07, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x13, 0x01, 0x18, 0x10, "2"                                  },
	{0x13, 0x01, 0x18, 0x18, "3"                                  },
	{0x13, 0x01, 0x18, 0x08, "4"                                  },
	{0x13, 0x01, 0x18, 0x00, "5"                                  },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"                         },
	{0x13, 0x01, 0x60, 0x40, "30000"                              },
	{0x13, 0x01, 0x60, 0x60, "40000"                              },
	{0x13, 0x01, 0x60, 0x20, "50000"                              },
	{0x13, 0x01, 0x60, 0x00, "60000"                              },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x13, 0x01, 0x80, 0x00, "Upright"                            },
	{0x13, 0x01, 0x80, 0x80, "Cocktail"                           },
		
	// Dip 2
	SYSTEM16B_COINAGE(0x14)
};

STDDIPINFO(Sonicbom)

static struct BurnDIPInfo SjryukoDIPList[]=
{
	// Default Values
	{0x1c, 0xff, 0xff, 0xff, NULL                                 },
	{0x1d, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	
	
	// Dip 2
	SYSTEM16B_COINAGE(0x1d)
};

STDDIPINFO(Sjryuko)

static struct BurnDIPInfo SnapperDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0xff, NULL                                 },
	{0x0b, 0xff, 0xff, 0xff, NULL                                 },
	
	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                             },
	{0x0a, 0x01, 0x07, 0x07, "1 Coin  1 Credit"                   },
	{0x0a, 0x01, 0x07, 0x06, "1 Coin  2 Credits"                  },
	{0x0a, 0x01, 0x07, 0x05, "1 Coin  3 Credits"                  },
	{0x0a, 0x01, 0x07, 0x04, "1 Coin  4 Credits"                  },
	{0x0a, 0x01, 0x07, 0x03, "1 Coin  5 Credits"                  },
	{0x0a, 0x01, 0x07, 0x02, "2 Coins 1 Credit"                   },
	{0x0a, 0x01, 0x07, 0x01, "3 Coins 1 Credit"                   },
	{0x0a, 0x01, 0x07, 0x00, "4 Coins 1 Credit"                   },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x0a, 0x01, 0x18, 0x10, "3"                                  },
	{0x0a, 0x01, 0x18, 0x18, "4"                                  },	
	{0x0a, 0x01, 0x18, 0x08, "5"                                  },
	{0x0a, 0x01, 0x18, 0x00, "6"                                  },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0b, 0x01, 0x10, 0x00, "Off"                                },
	{0x0b, 0x01, 0x10, 0x10, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x0b, 0x01, 0x80, 0x80, "Off"                                },
	{0x0b, 0x01, 0x80, 0x00, "On"                                 },
};

STDDIPINFO(Snapper)

static struct BurnDIPInfo TetrisDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfd, NULL                                 },
	{0x12, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x11, 0x01, 0x02, 0x02, "Off"                                },
	{0x11, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x11, 0x01, 0x30, 0x20, "Easy"                               },
	{0x11, 0x01, 0x30, 0x30, "Normal"                             },
	{0x11, 0x01, 0x30, 0x10, "Hard"                               },
	{0x11, 0x01, 0x30, 0x00, "Hardest"                            },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x12)
};

STDDIPINFO(Tetris)

static struct BurnDIPInfo TimescanDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xf5, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },
	{0x15, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x13, 0x01, 0x01, 0x00, "Cocktail"                           },
	{0x13, 0x01, 0x01, 0x01, "Upright"                            },
	
	{0   , 0xfe, 0   , 16  , "Bonus"                              },
	{0x13, 0x01, 0x1e, 0x16, "Replay 1000000/2000000"             },
	{0x13, 0x01, 0x1e, 0x14, "Replay 1200000/2500000"             },
	{0x13, 0x01, 0x1e, 0x12, "Replay 1500000/3000000"             },
	{0x13, 0x01, 0x1e, 0x10, "Replay 2000000/4000000"             },
	{0x13, 0x01, 0x1e, 0x1c, "Replay 1000000"                     },
	{0x13, 0x01, 0x1e, 0x1e, "Replay 1200000"                     },
	{0x13, 0x01, 0x1e, 0x1a, "Replay 1500000"                     },
	{0x13, 0x01, 0x1e, 0x18, "Replay 1800000"                     },
	{0x13, 0x01, 0x1e, 0x0e, "Extra Ball 100000"                  },
	{0x13, 0x01, 0x1e, 0x0c, "Extra Ball 200000"                  },
	{0x13, 0x01, 0x1e, 0x0a, "Extra Ball 300000"                  },
	{0x13, 0x01, 0x1e, 0x08, "Extra Ball 400000"                  },
	{0x13, 0x01, 0x1e, 0x06, "Extra Ball 500000"                  },
	{0x13, 0x01, 0x1e, 0x04, "Extra Ball 600000"                  },
	{0x13, 0x01, 0x1e, 0x02, "Extra Ball 700000"                  },
	{0x13, 0x01, 0x1e, 0x00, "None"                               },
	
	{0   , 0xfe, 0   , 2   , "Match"                              },
	{0x13, 0x01, 0x20, 0x00, "Off"                                },
	{0x13, 0x01, 0x20, 0x20, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Pin Rebound"                        },
	{0x13, 0x01, 0x40, 0x40, "Well"                               },
	{0x13, 0x01, 0x40, 0x00, "A Little"                           },
	
	{0   , 0xfe, 0   , 2   , "Lives"                              },
	{0x13, 0x01, 0x80, 0x80, "3"                                  },
	{0x13, 0x01, 0x80, 0x00, "5"                                  },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x14)
	
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

static struct BurnDIPInfo ToryumonDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfe, NULL                                 },
	{0x12, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x11, 0x01, 0x01, 0x01, "Off"                                },
	{0x11, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "VS-Mode Battle"                     },
	{0x11, 0x01, 0x10, 0x10, "1"                                  },
	{0x11, 0x01, 0x10, 0x00, "3"                                  },
	
	{0   , 0xfe, 0   , 8   , "Difficulty"                         },
	{0x11, 0x01, 0xe0, 0xc0, "Easy"                               },
	{0x11, 0x01, 0xe0, 0xe0, "Normal"                             },
	{0x11, 0x01, 0xe0, 0xa0, "Hard"                               },
	{0x11, 0x01, 0xe0, 0x80, "Hard+1"                             },
	{0x11, 0x01, 0xe0, 0x60, "Hard+2"                             },
	{0x11, 0x01, 0xe0, 0x40, "Hard+3"                             },
	{0x11, 0x01, 0xe0, 0x20, "Hard+4"                             },
	{0x11, 0x01, 0xe0, 0x00, "Hard+5"                             },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x12)
};

STDDIPINFO(Toryumon)

static struct BurnDIPInfo TturfDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0x2c, NULL                                 },
	{0x16, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Continues"                          },
	{0x15, 0x01, 0x03, 0x00, "None"                               },
	{0x15, 0x01, 0x03, 0x01, "3"                                  },
	{0x15, 0x01, 0x03, 0x02, "Unlimited"                          },
	{0x15, 0x01, 0x03, 0x03, "Unlimited"                          },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x15, 0x01, 0x0c, 0x08, "Easy"                               },
	{0x15, 0x01, 0x0c, 0x0c, "Normal"                             },
	{0x15, 0x01, 0x0c, 0x04, "Hard"                               },
	{0x15, 0x01, 0x0c, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Starting Energy"                    },
	{0x15, 0x01, 0x30, 0x00, "3"                                  },
	{0x15, 0x01, 0x30, 0x10, "4"                                  },
	{0x15, 0x01, 0x30, 0x20, "6"                                  },
	{0x15, 0x01, 0x30, 0x30, "8"                                  },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x15, 0x01, 0x40, 0x40, "Off"                                },
	{0x15, 0x01, 0x40, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Bonus Energy"                       },
	{0x15, 0x01, 0x80, 0x80, "1"                                  },
	{0x15, 0x01, 0x80, 0x00, "2"                                  },
	
	// Dip 2
	SYSTEM16B_COINAGE(0x16)
};

STDDIPINFO(Tturf)

static struct BurnDIPInfo Wb3DIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfd, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x13, 0x01, 0x02, 0x02, "Off"                                },
	{0x13, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x13, 0x01, 0x0c, 0x00, "2"                                  },
	{0x13, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x13, 0x01, 0x0c, 0x08, "4"                                  },
	{0x13, 0x01, 0x0c, 0x04, "5"                                  },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"                         },
	{0x13, 0x01, 0x10, 0x10, "5k, 10k, 18k, 30k"                  },
	{0x13, 0x01, 0x10, 0x00, "5k, 15k, 30k"                       },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"                         },
	{0x13, 0x01, 0x20, 0x20, "Normal"                             },
	{0x13, 0x01, 0x20, 0x00, "Hard"                               },
	
	{0   , 0xfe, 0   , 2   , "Invincible Mode"                    },
	{0x13, 0x01, 0x40, 0x40, "No"                                 },
	{0x13, 0x01, 0x40, 0x00, "Yes"                                },
		
	// Dip 2
	SYSTEM16B_COINAGE(0x14)
};

STDDIPINFO(Wb3)

static struct BurnDIPInfo WrestwarDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfd, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x13, 0x01, 0x02, 0x02, "Off"                                },
	{0x13, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Round Time"                         },
	{0x13, 0x01, 0x0c, 0x00, "100"                                },
	{0x13, 0x01, 0x0c, 0x0c, "110"                                },
	{0x13, 0x01, 0x0c, 0x08, "120"                                },
	{0x13, 0x01, 0x0c, 0x04, "130"                                },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"                     },
	{0x13, 0x01, 0x20, 0x00, "No"                                 },
	{0x13, 0x01, 0x20, 0x20, "Yes"                                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x13, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x13, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x13, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x13, 0x01, 0xc0, 0x00, "Hardest"                            },
		
	// Dip 2
	SYSTEM16B_COINAGE(0x14)
};

STDDIPINFO(Wrestwar)

#undef SYSTEM16B_COINAGE

/*====================================================
Rom defs
====================================================*/

static struct BurnRomInfo AceattacRomDesc[] = {
	{ "epr-11491.a4",   0x10000, 0xf3c19c36, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11489.a1",   0x10000, 0xbbe623c5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11492.a5",   0x10000, 0xd8bd3139, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11490.a2",   0x10000, 0x38cb3a41, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11493.b9",   0x10000, 0x654485d9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11494.b10",  0x10000, 0xb67971ab, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11495.b11",  0x10000, 0xb687ab61, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11501.b1",   0x10000, 0x09179ead, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11505.b5",   0x10000, 0xb67f1ecf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11502.b2",   0x10000, 0xa3ee36b8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11506.b6",   0x10000, 0xb0104def, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11503.b3",   0x10000, 0x344c0692, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11507.b7",   0x10000, 0xa2af710a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11504.b4",   0x10000, 0x7cae7920, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11508.b8",   0x10000, 0x5cbb833c, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-11496.a7",   0x08000, 0x82cb40a9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11497.a8",   0x08000, 0xb04f62cc, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11498.a9",   0x08000, 0x97baf52b, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11499.a10",  0x08000, 0xea332866, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11500.a11",  0x08000, 0x2ddf1c31, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0059.key",   0x02000, 0x00000000, SYS16_ROM_KEY | BRF_ESS | BRF_PRG | BRF_NODUMP },
};


STD_ROM_PICK(Aceattac)
STD_ROM_FN(Aceattac)

static struct BurnRomInfo AliensynRomDesc[] = {
	{ "epr-11083.a4",   0x08000, 0xcb2ad9b3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11080.a1",   0x08000, 0xfe7378d9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11084.a5",   0x08000, 0x2e1ec7b1, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11081.a2",   0x08000, 0x1308ee63, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11085.a6",   0x08000, 0xcff78f39, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11082.a3",   0x08000, 0x9cdc2a14, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10702.b9",   0x10000, 0x393bc813, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10703.b10",  0x10000, 0x6b6dd9f5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10704.b11",  0x10000, 0x911e7ebc, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10709.b1",   0x10000, 0xaddf0a90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10713.b5",   0x10000, 0xececde3a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10710.b2",   0x10000, 0x992369eb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10714.b6",   0x10000, 0x91bf42fb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10711.b3",   0x10000, 0x29166ef6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10715.b7",   0x10000, 0xa7c57384, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10712.b4",   0x10000, 0x876ad019, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10716.b8",   0x10000, 0x40ba1d48, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10723.a7",   0x08000, 0x99953526, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10724.a8",   0x08000, 0xf971a817, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-10725.a9",   0x08000, 0x6a50e08f, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-10726.a10",  0x08000, 0xd50b7736, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Aliensyn)
STD_ROM_FN(Aliensyn)

static struct BurnRomInfo Aliensyn3RomDesc[] = {
	{ "epr-10816.a4",   0x08000, 0x17bf5304, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10814.a1",   0x08000, 0x4cd134df, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10817.a5",   0x08000, 0xc8b791b0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10815.a2",   0x08000, 0xbdcf4a30, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10822a.a6",  0x08000, 0x1d0790aa, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10819a.a3",  0x08000, 0x1e7586b7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10702.b9",   0x10000, 0x393bc813, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10703.b10",  0x10000, 0x6b6dd9f5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10704.b11",  0x10000, 0x911e7ebc, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10709.b1",   0x10000, 0xaddf0a90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10713.b5",   0x10000, 0xececde3a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10710.b2",   0x10000, 0x992369eb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10714.b6",   0x10000, 0x91bf42fb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10711.b3",   0x10000, 0x29166ef6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10715.b7",   0x10000, 0xa7c57384, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10712.b4",   0x10000, 0x876ad019, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10716.b8",   0x10000, 0x40ba1d48, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10723.a7",   0x08000, 0x99953526, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10724.a8",   0x08000, 0xf971a817, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-10725.a9",   0x08000, 0x6a50e08f, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-10726.a10",  0x08000, 0xd50b7736, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0033.key",   0x02000, 0x49e882e5, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Aliensyn3)
STD_ROM_FN(Aliensyn3)

static struct BurnRomInfo AliensynjRomDesc[] = {
	{ "epr-10720a.a4",  0x08000, 0x1b920893, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10717a.a1",  0x08000, 0x972ae358, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10721a.a5",  0x08000, 0xf4d2d1c3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10718a.a2",  0x08000, 0xc79bf61b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10722a.a6",  0x08000, 0x1d0790aa, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10719a.a3",  0x08000, 0x1e7586b7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10702.b9",   0x10000, 0x393bc813, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10703.b10",  0x10000, 0x6b6dd9f5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10704.b11",  0x10000, 0x911e7ebc, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10709.b1",   0x10000, 0xaddf0a90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10713.b5",   0x10000, 0xececde3a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10710.b2",   0x10000, 0x992369eb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10714.b6",   0x10000, 0x91bf42fb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10711.b3",   0x10000, 0x29166ef6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10715.b7",   0x10000, 0xa7c57384, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10712.b4",   0x10000, 0x876ad019, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10716.b8",   0x10000, 0x40ba1d48, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10723.a7",   0x08000, 0x99953526, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10724.a8",   0x08000, 0xf971a817, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-10725.a9",   0x08000, 0x6a50e08f, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-10726.a10",  0x08000, 0xd50b7736, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0033.key",   0x02000, 0x49e882e5, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Aliensynj)
STD_ROM_FN(Aliensynj)

static struct BurnRomInfo AltbeastRomDesc[] = {
	{ "epr-11907.a7",   0x20000, 0x29e0c3ad, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11906.a5",   0x20000, 0x4c9e9cd8, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "opr-11674.a14",  0x20000, 0xa57a66d5, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11675.a15",  0x20000, 0x2ef2f144, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11676.a16",  0x20000, 0x0c04acac, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11677.b1",   0x20000, 0xa01425cd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11681.b5",   0x20000, 0xd9e03363, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11678.b2",   0x20000, 0x17a9fc53, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11682.b6",   0x20000, 0xe3f77c5e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11679.b3",   0x20000, 0x14dcc245, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11683.b7",   0x20000, 0xf9a60f06, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11680.b4",   0x20000, 0xf43dcdec, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11684.b8",   0x20000, 0xb20c0edb, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11671.a10",  0x08000, 0x2b71343b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11672.a11",  0x20000, 0xbbd7f460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "opr-11673.a12",  0x20000, 0x400c4a36, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0078.mcu",   0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Altbeast)
STD_ROM_FN(Altbeast)

static struct BurnRomInfo AltbeastjRomDesc[] = {
	{ "epr-11885.a7",   0x20000, 0x5bb715aa, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11884.a5",   0x20000, 0xe1707090, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11722.a14",  0x10000, 0xadaa8db5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11736.b14",  0x10000, 0xe9ad5e89, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11723.a15",  0x10000, 0x131a3f9a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11737.b15",  0x10000, 0x2e420023, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11724.a16",  0x10000, 0x6f2ed50a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11738.b16",  0x10000, 0xde3d6d02, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11725.b1",   0x10000, 0xf8b3684e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11729.b5",   0x10000, 0xae3c2793, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11726.b2",   0x10000, 0x3cce5419, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11730.b6",   0x10000, 0x3af62b55, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11727.b3",   0x10000, 0xb0390078, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11731.b7",   0x10000, 0x2a87744a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11728.b4",   0x10000, 0xf3a43fd8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11732.b8",   0x10000, 0x2fb3e355, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11717.a1",   0x10000, 0x676be0cb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11733.b10",  0x10000, 0x802cac94, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11718.a2",   0x10000, 0x882864c2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11734.b11",  0x10000, 0x76c704d2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11719.a3",   0x10000, 0x339987f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11735.b12",  0x10000, 0x4fe406aa, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11671.a10",  0x08000, 0x2b71343b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11672.a11",  0x20000, 0xbbd7f460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "opr-11673.a12",  0x20000, 0x400c4a36, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0077.mcu",   0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Altbeastj)
STD_ROM_FN(Altbeastj)

static struct BurnRomInfo Altbeast2RomDesc[] = {
	{ "epr-11705.a7",   0x20000, 0x57dc5c7a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11704.a5",   0x20000, 0x33bbcf07, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "opr-11674.a14",  0x20000, 0xa57a66d5, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11675.a15",  0x20000, 0x2ef2f144, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11676.a16",  0x20000, 0x0c04acac, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11677.b1",   0x20000, 0xa01425cd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11681.b5",   0x20000, 0xd9e03363, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11678.b2",   0x20000, 0x17a9fc53, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11682.b6",   0x20000, 0xe3f77c5e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11679.b3",   0x20000, 0x14dcc245, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11683.b7",   0x20000, 0xf9a60f06, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11680.b4",   0x20000, 0xf43dcdec, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11684.b8",   0x20000, 0xb20c0edb, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-11686.a10",  0x08000, 0x828a45b3, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11672.a11",  0x20000, 0xbbd7f460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "opr-11673.a12",  0x20000, 0x400c4a36, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0066.key",   0x02000, 0xed85a054, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Altbeast2)
STD_ROM_FN(Altbeast2)

static struct BurnRomInfo Altbeastj3RomDesc[] = {
	{ "epr-11721.a7",   0x20000, 0x1c5d11de, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11720.a5",   0x20000, 0x735350cf, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11722.a14",  0x10000, 0xadaa8db5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11736.b14",  0x10000, 0xe9ad5e89, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11723.a15",  0x10000, 0x131a3f9a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11737.b15",  0x10000, 0x2e420023, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11724.a16",  0x10000, 0x6f2ed50a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11738.b16",  0x10000, 0xde3d6d02, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11725.b1",   0x10000, 0xf8b3684e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11729.b5",   0x10000, 0xae3c2793, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11726.b2",   0x10000, 0x3cce5419, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11730.b6",   0x10000, 0x3af62b55, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11727.b3",   0x10000, 0xb0390078, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11731.b7",   0x10000, 0x2a87744a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11728.b4",   0x10000, 0xf3a43fd8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11732.b8",   0x10000, 0x2fb3e355, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11717.a1",   0x10000, 0x676be0cb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11733.b10",  0x10000, 0x802cac94, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11718.a2",   0x10000, 0x882864c2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11734.b11",  0x10000, 0x76c704d2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11719.a3",   0x10000, 0x339987f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11735.b12",  0x10000, 0x4fe406aa, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11671.a10",  0x08000, 0x2b71343b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11672.a11",  0x20000, 0xbbd7f460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "opr-11673.a12",  0x20000, 0x400c4a36, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0068.key",   0x02000, 0xc1ed4310, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Altbeastj3)
STD_ROM_FN(Altbeastj3)

static struct BurnRomInfo Altbeast4RomDesc[] = {
	{ "epr-11740.a7",   0x20000, 0xce227542, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11739.a5",   0x20000, 0xe466eb65, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11722.a14",  0x10000, 0xadaa8db5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11736.b14",  0x10000, 0xe9ad5e89, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11723.a15",  0x10000, 0x131a3f9a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11737.b15",  0x10000, 0x2e420023, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11724.a16",  0x10000, 0x6f2ed50a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11738.b16",  0x10000, 0xde3d6d02, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11725.b1",   0x10000, 0xf8b3684e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11729.b5",   0x10000, 0xae3c2793, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11726.b2",   0x10000, 0x3cce5419, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11730.b6",   0x10000, 0x3af62b55, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11727.b3",   0x10000, 0xb0390078, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11731.b7",   0x10000, 0x2a87744a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11728.b4",   0x10000, 0xf3a43fd8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11732.b8",   0x10000, 0x2fb3e355, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11717.a1",   0x10000, 0x676be0cb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11733.b10",  0x10000, 0x802cac94, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11718.a2",   0x10000, 0x882864c2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11734.b11",  0x10000, 0x76c704d2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11719.a3",   0x10000, 0x339987f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11735.b12",  0x10000, 0x4fe406aa, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11686.a10",  0x08000, 0x828a45b3, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11672.a11",  0x20000, 0xbbd7f460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "opr-11673.a12",  0x20000, 0x400c4a36, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0066.key",   0x02000, 0xed85a054, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Altbeast4)
STD_ROM_FN(Altbeast4)

static struct BurnRomInfo Altbeast5RomDesc[] = {
	{ "epr-11742.a7",   0x20000, 0x61839534, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11741.a5",   0x20000, 0x9b2159cb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11722.a14",  0x10000, 0xadaa8db5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11736.b14",  0x10000, 0xe9ad5e89, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11723.a15",  0x10000, 0x131a3f9a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11737.b15",  0x10000, 0x2e420023, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11724.a16",  0x10000, 0x6f2ed50a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11738.b16",  0x10000, 0xde3d6d02, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11725.b1",   0x10000, 0xf8b3684e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11729.b5",   0x10000, 0xae3c2793, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11726.b2",   0x10000, 0x3cce5419, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11730.b6",   0x10000, 0x3af62b55, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11727.b3",   0x10000, 0xb0390078, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11731.b7",   0x10000, 0x2a87744a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11728.b4",   0x10000, 0xf3a43fd8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11732.b8",   0x10000, 0x2fb3e355, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11717.a1",   0x10000, 0x676be0cb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11733.b10",  0x10000, 0x802cac94, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11718.a2",   0x10000, 0x882864c2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11734.b11",  0x10000, 0x76c704d2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11719.a3",   0x10000, 0x339987f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11735.b12",  0x10000, 0x4fe406aa, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11671.a10",  0x08000, 0x2b71343b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11672.a11",  0x20000, 0xbbd7f460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "opr-11673.a12",  0x20000, 0x400c4a36, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0069.key",   0x02000, 0x959e256a, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Altbeast5)
STD_ROM_FN(Altbeast5)

static struct BurnRomInfo Altbeast6RomDesc[] = {
	{ "epr-11883.a7",   0x20000, 0xc5b3e8f7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11882.a5",   0x20000, 0x9c01170b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11722.a14",  0x10000, 0xadaa8db5, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11736.b14",  0x10000, 0xe9ad5e89, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11723.a15",  0x10000, 0x131a3f9a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11737.b15",  0x10000, 0x2e420023, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11724.a16",  0x10000, 0x6f2ed50a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11738.b16",  0x10000, 0xde3d6d02, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11725.b1",   0x10000, 0xf8b3684e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11729.b5",   0x10000, 0xae3c2793, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11726.b2",   0x10000, 0x3cce5419, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11730.b6",   0x10000, 0x3af62b55, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11727.b3",   0x10000, 0xb0390078, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11731.b7",   0x10000, 0x2a87744a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11728.b4",   0x10000, 0xf3a43fd8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11732.b8",   0x10000, 0x2fb3e355, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11717.a1",   0x10000, 0x676be0cb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11733.b10",  0x10000, 0x802cac94, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11718.a2",   0x10000, 0x882864c2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11734.b11",  0x10000, 0x76c704d2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11719.a3",   0x10000, 0x339987f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11735.b12",  0x10000, 0x4fe406aa, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11671.a10",  0x08000, 0x2b71343b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11672.a11",  0x20000, 0xbbd7f460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "opr-11673.a12",  0x20000, 0x400c4a36, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0076.mcu",   0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Altbeast6)
STD_ROM_FN(Altbeast6)

static struct BurnRomInfo AtomicpRomDesc[] = {
	{ "ap-t2.bin",      0x10000, 0x97421047, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "ap-t1.bin",      0x10000, 0x5c65fe56, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "ap-t4.bin",      0x08000, 0x332e58f4, SYS16_ROM_TILES | BRF_GRA },
	{ "ap-t3.bin",      0x08000, 0xdddc122c, SYS16_ROM_TILES | BRF_GRA },
	{ "ap-t5.bin",      0x08000, 0xef5ecd6b, SYS16_ROM_TILES | BRF_GRA },
};


STD_ROM_PICK(Atomicp)
STD_ROM_FN(Atomicp)

static struct BurnRomInfo AurailRomDesc[] = {
	{ "epr-13577.a7",   0x20000, 0x6701b686, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13576.a5",   0x20000, 0x1e428d94, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13447.a8",   0x20000, 0x70a52167, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13445.a6",   0x20000, 0x28dfc3dd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "mpr-13450.a14",  0x20000, 0x0fc4a7a8, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13465.b14",  0x20000, 0xe08135e0, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13451.a15",  0x20000, 0x1c49852f, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13466.b15",  0x20000, 0xe14c6684, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13452.a16",  0x20000, 0x047bde5e, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13467.b16",  0x20000, 0x6309fec4, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-13453.b1",   0x20000, 0x5fa0a9f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13457.b5",   0x20000, 0x0d1b54da, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13454.b2",   0x20000, 0x5f6b33b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13458.b6",   0x20000, 0xbad340c3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13455.b3",   0x20000, 0x4e80520b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13459.b7",   0x20000, 0x7e9165ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13456.b4",   0x20000, 0x5733c428, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13460.b8",   0x20000, 0x66b8f9b3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13440.a1",   0x20000, 0x4f370b2b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13461.b10",  0x20000, 0xf76014bf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13441.a2",   0x20000, 0x37cf9cb4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13462.b11",  0x20000, 0x1061e7da, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13442.a3",   0x20000, 0x049698ef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13463.b12",  0x20000, 0x7dbcfbf1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13443.a4",   0x20000, 0x77a8989e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13464.b13",  0x20000, 0x551df422, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-13448.a10",  0x08000, 0xb5183fb9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-13449.a11",  0x20000, 0xd3d9aaf9, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Aurail)
STD_ROM_FN(Aurail)

static struct BurnRomInfo Aurail1RomDesc[] = {
	{ "epr-13469.a7",   0x20000, 0xc628b69d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13468.a5",   0x20000, 0xce092218, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13447.a8",   0x20000, 0x70a52167, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13445.a6",   0x20000, 0x28dfc3dd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "mpr-13450.a14",  0x20000, 0x0fc4a7a8, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13465.b14",  0x20000, 0xe08135e0, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13451.a15",  0x20000, 0x1c49852f, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13466.b15",  0x20000, 0xe14c6684, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13452.a16",  0x20000, 0x047bde5e, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13467.b16",  0x20000, 0x6309fec4, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-13453.b1",   0x20000, 0x5fa0a9f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13457.b5",   0x20000, 0x0d1b54da, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13454.b2",   0x20000, 0x5f6b33b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13458.b6",   0x20000, 0xbad340c3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13455.b3",   0x20000, 0x4e80520b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13459.b7",   0x20000, 0x7e9165ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13456.b4",   0x20000, 0x5733c428, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13460.b8",   0x20000, 0x66b8f9b3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13440.a1",   0x20000, 0x4f370b2b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13461.b10",  0x20000, 0xf76014bf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13441.a2",   0x20000, 0x37cf9cb4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13462.b11",  0x20000, 0x1061e7da, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13442.a3",   0x20000, 0x049698ef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13463.b12",  0x20000, 0x7dbcfbf1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13443.a4",   0x20000, 0x77a8989e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13464.b13",  0x20000, 0x551df422, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-13448.a10",  0x08000, 0xb5183fb9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-13449.a11",  0x20000, 0xd3d9aaf9, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0168.key",   0x02000, 0xbc2d1792, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Aurail1)
STD_ROM_FN(Aurail1)

static struct BurnRomInfo AurailjRomDesc[] = {
	{ "epr-13446.a7",   0x20000, 0xd1f57b2a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13444.a5",   0x20000, 0x7a2b045f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13447.a8",   0x20000, 0x70a52167, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13445.a6",   0x20000, 0x28dfc3dd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-13450.a14",  0x20000, 0x0fc4a7a8, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13465.b14",  0x20000, 0xe08135e0, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13451.a15",  0x20000, 0x1c49852f, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13466.b15",  0x20000, 0xe14c6684, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13452.a16",  0x20000, 0x047bde5e, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-13467.b16",  0x20000, 0x6309fec4, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-13453.b1",   0x20000, 0x5fa0a9f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13457.b5",   0x20000, 0x0d1b54da, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13454.b2",   0x20000, 0x5f6b33b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13458.b6",   0x20000, 0xbad340c3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13455.b3",   0x20000, 0x4e80520b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13459.b7",   0x20000, 0x7e9165ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13456.b4",   0x20000, 0x5733c428, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13460.b8",   0x20000, 0x66b8f9b3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13440.a1",   0x20000, 0x4f370b2b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13461.b10",  0x20000, 0xf76014bf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13441.a2",   0x20000, 0x37cf9cb4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13462.b11",  0x20000, 0x1061e7da, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13442.a3",   0x20000, 0x049698ef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13463.b12",  0x20000, 0x7dbcfbf1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13443.a4",   0x20000, 0x77a8989e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-13464.b13",  0x20000, 0x551df422, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-13448.a10",  0x08000, 0xb5183fb9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-13449.a11",  0x20000, 0xd3d9aaf9, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0167.key",   0x02000, 0xbc2d1792, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Aurailj)
STD_ROM_FN(Aurailj)

static struct BurnRomInfo BayrouteRomDesc[] = {
	{ "epr-12517.a7",   0x20000, 0x436728a9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12516.a5",   0x20000, 0x4ff0353f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12458.a8",   0x20000, 0xe7c7476a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12456.a6",   0x20000, 0x25dc2eaf, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12462.a14",  0x10000, 0xa19943b5, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12463.a15",  0x10000, 0x62f8200d, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12464.a16",  0x10000, 0xc8c59703, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12465.b1",   0x20000, 0x11d61b45, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12467.b5",   0x20000, 0xc3b4e4c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12466.b2",   0x20000, 0xa57f236f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12468.b6",   0x20000, 0xd89c77de, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12459.a10",  0x08000, 0x3e1d29d0, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12460.a11",  0x20000, 0x0bae570d, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "mpr-12461.a12",  0x20000, 0xb03b8b46, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0116.key",   0x02000, 0x8778ee49, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Bayroute)
STD_ROM_FN(Bayroute)

static struct BurnRomInfo BayroutejRomDesc[] = {
	{ "epr-12457.a7",   0x20000, 0xbc726255, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12455.a5",   0x20000, 0xb6a722eb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12458.a8",   0x20000, 0xe7c7476a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12456.a6",   0x20000, 0x25dc2eaf, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12462.a14",  0x10000, 0xa19943b5, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12463.a15",  0x10000, 0x62f8200d, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12464.a16",  0x10000, 0xc8c59703, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12465.b1",   0x20000, 0x11d61b45, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12467.b5",   0x20000, 0xc3b4e4c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12466.b2",   0x20000, 0xa57f236f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12468.b6",   0x20000, 0xd89c77de, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12459.a10",  0x08000, 0x3e1d29d0, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12460.a11",  0x20000, 0x0bae570d, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "mpr-12461.a12",  0x20000, 0xb03b8b46, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0115.key",   0x02000, 0x75a55614, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Bayroutej)
STD_ROM_FN(Bayroutej)

static struct BurnRomInfo Bayroute1RomDesc[] = {
	{ "br.a4",         0x10000, 0x91c6424b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "br.a1",         0x10000, 0x76954bf3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "br.a5",         0x10000, 0x9d6fd183, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "br.a2",         0x10000, 0x5ca1e3d2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "br.a6",         0x10000, 0xed97ad4c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "br.a3",         0x10000, 0x0d362905, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-12462.a14", 0x10000, 0xa19943b5, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12463.a15", 0x10000, 0x62f8200d, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12464.a16", 0x10000, 0xc8c59703, SYS16_ROM_TILES | BRF_GRA },
	
	{ "br_obj0o.b1",   0x10000, 0x098a5e82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "br_obj0e.b5",   0x10000, 0x85238af9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "br_obj1o.b2",   0x10000, 0xcc641da1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "br_obj1e.b6",   0x10000, 0xd3123315, SYS16_ROM_SPRITES | BRF_GRA },
	{ "br_obj2o.b3",   0x10000, 0x84efac1f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "br_obj2e.b7",   0x10000, 0xb73b12cb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "br_obj3o.b4",   0x10000, 0xa2e238ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "br.b8",         0x10000, 0xd8de78ff, SYS16_ROM_SPRITES | BRF_GRA },

	{ "sound.a7",      0x08000, 0x00000000, SYS16_ROM_Z80PROG | BRF_NODUMP | BRF_PRG },
	
	{ "sound.a8",      0x10000, 0x00000000, SYS16_ROM_UPD7759DATA | BRF_NODUMP | BRF_SND },
	{ "sound.a9",      0x10000, 0x00000000, SYS16_ROM_UPD7759DATA | BRF_NODUMP | BRF_SND },
	{ "sound.a10",     0x10000, 0x00000000, SYS16_ROM_UPD7759DATA | BRF_NODUMP | BRF_SND },
	{ "sound.a11",     0x10000, 0x00000000, SYS16_ROM_UPD7759DATA | BRF_NODUMP | BRF_SND },
};


STD_ROM_PICK(Bayroute1)
STD_ROM_FN(Bayroute1)

static struct BurnRomInfo Blox16bRomDesc[] = {
	{ "bs16b.p00",      0x040000, 0xfd1978b9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "bs16b.scr",      0x040000, 0x1de4e95b, SYS16_ROM_TILES | BRF_GRA },
	
	{ "bs16b.obj",      0x020000, 0x05076220, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "bs16b.snd",      0x018000, 0x930c7e7b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Blox16b)
STD_ROM_FN(Blox16b)

static struct BurnRomInfo BulletRomDesc[] = {
	{ "epr-11010.a4",   0x08000, 0xdd9001de, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11007.a1",   0x08000, 0xd9e08110, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11011.a5",   0x08000, 0x7f446b9f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11008.a2",   0x08000, 0x34824d3b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11012.a6",   0x08000, 0x3992f159, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11009.a3",   0x08000, 0xdf199999, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10994.b9",   0x10000, 0x3035468a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10995.b10",  0x10000, 0x6b97aff1, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10996.b11",  0x10000, 0x501bddd6, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10999.b1",   0x10000, 0x119f0008, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11003.b5",   0x10000, 0x2f429089, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11000.b2",   0x10000, 0xf5482bbe, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11004.b6",   0x10000, 0x8c886df0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11001.b3",   0x10000, 0x65ea71e0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11005.b7",   0x10000, 0xea2f9d50, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11002.b4",   0x10000, 0x9e25042b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11006.b8",   0x10000, 0x6b7384f2, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10997.a7",   0x08000, 0x5dd9cab5, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10998.a8",   0x08000, 0xf971a817, SYS16_ROM_UPD7759DATA | BRF_SND },
		
	{ "317-0041.key",   0x02000, 0x4cd4861a, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Bullet)
STD_ROM_FN(Bullet)

static struct BurnRomInfo CottonRomDesc[] = {
	{ "epr-13921a.a7",  0x20000, 0xf047a037, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13919a.a5",  0x20000, 0x651108b1, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13922a.a8",  0x20000, 0x1ca248c5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13920a.a6",  0x20000, 0xfa3610f9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-13862.a14",  0x20000, 0xa47354b6, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13877.b14",  0x20000, 0xd38424b5, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13863.a15",  0x20000, 0x8c990026, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13878.b15",  0x20000, 0x21c15b8a, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13864.a16",  0x20000, 0xd2b175bf, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13879.b16",  0x20000, 0xb9d62531, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-13865.b1",   0x20000, 0x7024f404, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13869.b5",   0x20000, 0xab4b3468, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13866.b2",   0x20000, 0x6169bba4, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13870.b6",   0x20000, 0x69b41ac3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13867.b3",   0x20000, 0xb014f02d, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13871.b7",   0x20000, 0x0801cf02, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13868.b4",   0x20000, 0xe62a7cd6, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13872.b8",   0x20000, 0xf066f315, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13852.a1",   0x20000, 0x943aba8b, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13873.b10",  0x20000, 0x1bd145f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13853.a2",   0x20000, 0x7ea93200, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13874.b11",  0x20000, 0x4fd59bff, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13891.a3",   0x20000, 0xc6b3c414, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13894.b12",  0x20000, 0xe3d0bee2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13855.a4",   0x20000, 0x856f3ee2, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13876.b13",  0x20000, 0x1c5ffad8, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-13892.a10",  0x08000, 0xfdfbe6ad, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-13893.a11",  0x20000, 0x384233df, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0181a.key",  0x02000, 0x5c419b36, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Cotton)
STD_ROM_FN(Cotton)

static struct BurnRomInfo CottonuRomDesc[] = {
	{ "cotton.a7",     0x20000, 0xe7ef7d10, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "cotton.a5",     0x20000, 0xabe4f83e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "cotton.a8",     0x20000, 0xfc0f4401, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "cotton.a6",     0x20000, 0xf50f1ea2, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-13862.a14",  0x20000, 0xa47354b6, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13877.b14",  0x20000, 0xd38424b5, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13863.a15",  0x20000, 0x8c990026, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13878.b15",  0x20000, 0x21c15b8a, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13864.a16",  0x20000, 0xd2b175bf, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13879.b16",  0x20000, 0xb9d62531, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-13865.b1",   0x20000, 0x7024f404, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13869.b5",   0x20000, 0xab4b3468, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13866.b2",   0x20000, 0x6169bba4, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13870.b6",   0x20000, 0x69b41ac3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13867.b3",   0x20000, 0xb014f02d, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13871.b7",   0x20000, 0x0801cf02, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13868.b4",   0x20000, 0xe62a7cd6, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13872.b8",   0x20000, 0xf066f315, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13852.a1",   0x20000, 0x943aba8b, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13873.b10",  0x20000, 0x1bd145f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13853.a2",   0x20000, 0x7ea93200, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13874.b11",  0x20000, 0x4fd59bff, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13891.a3",   0x20000, 0xc6b3c414, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13894.b12",  0x20000, 0xe3d0bee2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13855.a4",   0x20000, 0x856f3ee2, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13876.b13",  0x20000, 0x1c5ffad8, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-13892.a10",  0x08000, 0xfdfbe6ad, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-13893.a11",  0x20000, 0x384233df, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0180.key",   0x02000, 0xa236b915, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Cottonu)
STD_ROM_FN(Cottonu)

static struct BurnRomInfo CottonjRomDesc[] = {
	{ "epr-13858a.a7",  0x20000, 0x276f42fe, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13856a.a5",  0x20000, 0x14e6b5e7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13859a.a8",  0x20000, 0x4703ef9d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13857a.a6",  0x20000, 0xde37e527, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-13862.a14",  0x20000, 0xa47354b6, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13877.b14",  0x20000, 0xd38424b5, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13863.a15",  0x20000, 0x8c990026, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13878.b15",  0x20000, 0x21c15b8a, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13864.a16",  0x20000, 0xd2b175bf, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13879.b16",  0x20000, 0xb9d62531, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-13865.b1",   0x20000, 0x7024f404, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13869.b5",   0x20000, 0xab4b3468, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13866.b2",   0x20000, 0x6169bba4, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13870.b6",   0x20000, 0x69b41ac3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13867.b3",   0x20000, 0xb014f02d, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13871.b7",   0x20000, 0x0801cf02, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13868.b4",   0x20000, 0xe62a7cd6, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13872.b8",   0x20000, 0xf066f315, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13852.a1",   0x20000, 0x943aba8b, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13873.b10",  0x20000, 0x1bd145f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13853.a2",   0x20000, 0x7ea93200, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13874.b11",  0x20000, 0x4fd59bff, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13854.a3",   0x20000, 0x1c942190, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13875.b12",  0x20000, 0x6a66868d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13855.a4",   0x20000, 0x856f3ee2, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "opr-13876.b13",  0x20000, 0x1c5ffad8, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-13860.a10",  0x08000, 0x6a57b027, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-13061.a11",  0x20000, 0x4d21153f, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0179a.key",  0x02000, 0x488096d3, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Cottonj)
STD_ROM_FN(Cottonj)

static struct BurnRomInfo DduxRomDesc[] = {
	{ "epr-11191.a7",   0x20000, 0x500e400a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11190.a5",   0x20000, 0x2a698308, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11915.a8",   0x20000, 0xd8ed3132, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11913.a6",   0x20000, 0x30c6cb92, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-11917.a14",  0x10000, 0x6f772190, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11918.a15",  0x10000, 0xc731db95, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11919.a16",  0x10000, 0x64d5a491, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-11920.b1",   0x20000, 0xe5d1e3cd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11922.b5",   0x20000, 0x70b0c4dd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11921.b2",   0x20000, 0x61d2358c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11923.b6",   0x20000, 0xc9ffe47d, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11916.a10",  0x08000, 0x7ab541cf, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0096.key",   0x02000, 0x6fd7d26e, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Ddux)
STD_ROM_FN(Ddux)

static struct BurnRomInfo Ddux1RomDesc[] = {
	{ "epr-12189.a7",   0x20000, 0x558e9b5d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12188.a5",   0x20000, 0x802a240f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11915.a8",   0x20000, 0xd8ed3132, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11913.a6",   0x20000, 0x30c6cb92, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-11917.a14",  0x10000, 0x6f772190, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11918.a15",  0x10000, 0xc731db95, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11919.a16",  0x10000, 0x64d5a491, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-11920.b1",   0x20000, 0xe5d1e3cd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11922.b5",   0x20000, 0x70b0c4dd, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11921.b2",   0x20000, 0x61d2358c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11923.b6",   0x20000, 0xc9ffe47d, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11916.a10",  0x08000, 0x7ab541cf, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0095.bin",   0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Ddux1)
STD_ROM_FN(Ddux1)

static struct BurnRomInfo DduxblRomDesc[] = {
	{ "dduxb03.bin",    0x20000, 0xe7526012, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "dduxb05.bin",    0x20000, 0x459d1237, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "dduxb02.bin",    0x20000, 0xd8ed3132, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "dduxb04.bin",    0x20000, 0x30c6cb92, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "dduxb14.bin",    0x10000, 0x664bd135, SYS16_ROM_TILES | BRF_GRA },
	{ "dduxb15.bin",    0x10000, 0xce0d2b30, SYS16_ROM_TILES | BRF_GRA },
	{ "dduxb16.bin",    0x10000, 0x6de95434, SYS16_ROM_TILES | BRF_GRA },
	
	{ "dduxb10.bin",    0x10000, 0x0be3aee5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "dduxb06.bin",    0x10000, 0xb0079e99, SYS16_ROM_SPRITES | BRF_GRA },
	{ "dduxb11.bin",    0x10000, 0xcfb2af18, SYS16_ROM_SPRITES | BRF_GRA },
	{ "dduxb07.bin",    0x10000, 0x0217369c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "dduxb12.bin",    0x10000, 0x28ce9b15, SYS16_ROM_SPRITES | BRF_GRA },
	{ "dduxb08.bin",    0x10000, 0x8844f336, SYS16_ROM_SPRITES | BRF_GRA },
	{ "dduxb13.bin",    0x10000, 0xefe57759, SYS16_ROM_SPRITES | BRF_GRA },
	{ "dduxb09.bin",    0x10000, 0x6b64f665, SYS16_ROM_SPRITES | BRF_GRA },

	{ "dduxb01.bin",    0x08000, 0x0dbef0d7, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Dduxbl)
STD_ROM_FN(Dduxbl)

static struct BurnRomInfo DunkshotRomDesc[] = {
	{ "epr-10470.a4",   0x08000, 0x8c60761f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10467.a1",   0x08000, 0x29774114, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10471.a5",   0x08000, 0x22777314, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10468.a2",   0x08000, 0xe2d5f97a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10472.a6",   0x08000, 0x206027a6, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10469.a3",   0x08000, 0xaa442b81, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr-10485.b9",   0x08000, 0xf16dda29, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10486.b10",  0x08000, 0x311d973c, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10487.b11",  0x08000, 0xa8fb179f, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10477.b1",   0x08000, 0xf9d3b2cb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10481.b5",   0x08000, 0xfeb04bc9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10478.b2",   0x08000, 0x5b5c5c92, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "epr-10482.b6",   0x08000, 0x5bc07618, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10479.b3",   0x08000, 0xe84190a0, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "epr-10483.b7",   0x08000, 0x7cab4f9e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10480.b4",   0x08000, 0x5dffd9dd, SYS16_ROM_SPRITES | BRF_GRA },	
	{ "epr-10484.b8",   0x08000, 0xbcb5fcc9, SYS16_ROM_SPRITES | BRF_GRA },	
	
	{ "epr-10473.a7",   0x08000, 0x7f1f5a27, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10474.a8",   0x08000, 0x419a656e, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-10475.a9",   0x08000, 0x17d55e85, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-10476.a10",  0x08000, 0xa6be0956, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0022.key",   0x02000, 0x4eedc66d, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Dunkshot)
STD_ROM_FN(Dunkshot)

static struct BurnRomInfo EswatRomDesc[] = {
	{ "epr-12659.bin",  0x40000, 0xc5ab2db9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12658.bin",  0x40000, 0xaf40bd71, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12624.b11",  0x40000, 0x375a5ec4, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12625.b12",  0x40000, 0x3b8c757e, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12626.b13",  0x40000, 0x3efca25c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12618.b1",   0x40000, 0x0d1530bf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12621.b4",   0x40000, 0x18ff0799, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12619.b2",   0x40000, 0x32069246, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12622.b5",   0x40000, 0xa3dfe436, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12620.b3",   0x40000, 0xf6b096e0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12623.b6",   0x40000, 0x6773fef6, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12617.a13",  0x08000, 0x7efecf23, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12616.a11",  0x40000, 0x254347c2, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0130.key",   0x02000, 0xba7b717b, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Eswat)
STD_ROM_FN(Eswat)

static struct BurnRomInfo EswatjRomDesc[] = {
	{ "epr-12615.a2",   0x40000, 0x388c2ea7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12614.a1",   0x40000, 0xd5f0fb47, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12624.b11",  0x40000, 0x375a5ec4, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12625.b12",  0x40000, 0x3b8c757e, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12626.b13",  0x40000, 0x3efca25c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12618.b1",   0x40000, 0x0d1530bf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12621.b4",   0x40000, 0x18ff0799, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12619.b2",   0x40000, 0x32069246, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12622.b5",   0x40000, 0xa3dfe436, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12620.b3",   0x40000, 0xf6b096e0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12623.b6",   0x40000, 0x6773fef6, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12617.a13",  0x08000, 0x7efecf23, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12616.a11",  0x40000, 0x254347c2, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0128.key",   0x02000, 0x95f96277, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Eswatj)
STD_ROM_FN(Eswatj)

static struct BurnRomInfo EswatuRomDesc[] = {
	{ "epr-12657.a2",   0x40000, 0x43ca72aa, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12656.a1",   0x40000, 0x5f018967, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12624.b11",  0x40000, 0x375a5ec4, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12625.b12",  0x40000, 0x3b8c757e, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12626.b13",  0x40000, 0x3efca25c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12618.b1",   0x40000, 0x0d1530bf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12621.b4",   0x40000, 0x18ff0799, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12619.b2",   0x40000, 0x32069246, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12622.b5",   0x40000, 0xa3dfe436, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12620.b3",   0x40000, 0xf6b096e0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12623.b6",   0x40000, 0x6773fef6, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12617.a13",  0x08000, 0x7efecf23, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12616.a11",  0x40000, 0x254347c2, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0129.key",   0x02000, 0x128302c7, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Eswatu)
STD_ROM_FN(Eswatu)

static struct BurnRomInfo EswatblRomDesc[] = {
	{ "eswat_c.rom",   0x10000, 0x1028cc81, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "eswat_f.rom",   0x10000, 0xf7b2d388, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "eswat_b.rom",   0x10000, 0x87c6b1b5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "eswat_e.rom",   0x10000, 0x937ddf9a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "eswat_a.rom",   0x08000, 0x2af4fc62, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "eswat_d.rom",   0x08000, 0xb4751e19, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr12624.b11",  0x40000, 0x375a5ec4, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr12625.b12",  0x40000, 0x3b8c757e, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr12626.b13",  0x40000, 0x3efca25c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "ic9",           0x40000, 0x0d1530bf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "ic12",          0x40000, 0x18ff0799, SYS16_ROM_SPRITES | BRF_GRA },
	{ "ic10",          0x40000, 0x32069246, SYS16_ROM_SPRITES | BRF_GRA },
	{ "ic13",          0x40000, 0xa3dfe436, SYS16_ROM_SPRITES | BRF_GRA },
	{ "ic11",          0x40000, 0xf6b096e0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "ic14",          0x40000, 0x6773fef6, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr12617.a13",  0x08000, 0x7efecf23, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr12616.a11",  0x40000, 0x254347c2, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Eswatbl)
STD_ROM_FN(Eswatbl)

static struct BurnRomInfo ExctleagRomDesc[] = {
	{ "epr-11939.a04",   0x10000, 0x117dd98f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11936.a01",   0x10000, 0x0863de60, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11940.a05",   0x10000, 0xdec83274, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11937.a02",   0x10000, 0x4ebda367, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11941.a06",   0x10000, 0x4df2d451, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11938.a03",   0x10000, 0x07c08d47, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11942.b09",   0x10000, 0xeb70e827, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11943.b10",   0x10000, 0xd97c8982, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11944.b11",   0x10000, 0xa75cae80, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11950.b01",   0x10000, 0xaf497849, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11954.b05",   0x10000, 0x5fa2106c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11951.b02",   0x10000, 0xc04fa974, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11955.b06",   0x10000, 0x86a0c368, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11952.b03",   0x10000, 0xe64a9761, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11956.b07",   0x10000, 0xaff5c2fa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11953.b04",   0x10000, 0x4cae3999, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11957.b08",   0x10000, 0x218f835b, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11945.a07",   0x08000, 0xc2a83012, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11140.a08",   0x08000, 0xb297371b, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11141.a09",   0x08000, 0x19756aa6, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11142.a10",   0x08000, 0x25d26c66, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11143.a11",   0x08000, 0x848b7b77, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0079.key",    0x02000, 0xeffefa1c, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Exctleag)
STD_ROM_FN(Exctleag)

static struct BurnRomInfo Fantzn2xRomDesc[] = {
	{ "fz2.a7",         0x020000, 0x94c05f0b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "fz2.a5",         0x020000, 0xf3526895, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "fz2.a8",         0x020000, 0xb2ebb209, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "fz2.a6",         0x020000, 0x6833f546, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "fz2.a14",        0x020000, 0x1c0a4537, SYS16_ROM_TILES | BRF_GRA },
	{ "fz2.a15",        0x020000, 0x2b933344, SYS16_ROM_TILES | BRF_GRA },
	{ "fz2.a16",        0x020000, 0xe63281a1, SYS16_ROM_TILES | BRF_GRA },
	
	{ "fz2.b1",         0x020000, 0x46bba615, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b5",         0x020000, 0xbebeee5d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b2",         0x020000, 0x6681a7b6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b6",         0x020000, 0x42d3241f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b3",         0x020000, 0x5863926f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b7",         0x020000, 0xcd830510, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b4",         0x020000, 0xb98fa5b6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b8",         0x020000, 0xe8248f68, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.a1",         0x020000, 0x9d2f41f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b10",        0x020000, 0x7686ea33, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.a2",         0x020000, 0x3b4050b7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "fz2.b11",        0x020000, 0xda8a95dc, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "fz2.a10",        0x008000, 0x92c92924, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "fz2.a11",        0x020000, 0x8c641bb9, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Fantzn2x)
STD_ROM_FN(Fantzn2x)

static struct BurnRomInfo Fantzn2xpRomDesc[] = {
	{ "cpu1b.bin",      0x020000, 0xd23ef944, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "cpu1a.bin",      0x020000, 0x407490e4, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "cpu1d.bin",      0x020000, 0xc8c7716b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "cpu1c.bin",      0x020000, 0x242e7b6e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "tilea.bin",      0x020000, 0x47e0e5ce, SYS16_ROM_TILES | BRF_GRA },
	{ "tileb.bin",      0x020000, 0x59e181b7, SYS16_ROM_TILES | BRF_GRA },
	{ "tilec.bin",      0x020000, 0x375d354c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "obja.bin",       0x020000, 0x9af87a4d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "objb.bin",       0x020000, 0x2fdbca68, SYS16_ROM_SPRITES | BRF_GRA },
	{ "objc.bin",       0x020000, 0x2587487a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "objd.bin",       0x020000, 0x8de4e7aa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "obje.bin",       0x020000, 0xdfada4ff, SYS16_ROM_SPRITES | BRF_GRA },
	{ "objf.bin",       0x020000, 0x65e5d23d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "objg.bin",       0x020000, 0xdc9fbb75, SYS16_ROM_SPRITES | BRF_GRA },
	{ "objh.bin",       0x020000, 0x97bb7c19, SYS16_ROM_SPRITES | BRF_GRA },
	{ "obji.bin",       0x020000, 0xc7790fee, SYS16_ROM_SPRITES | BRF_GRA },
	{ "objj.bin",       0x020000, 0x4535eb0e, SYS16_ROM_SPRITES | BRF_GRA },
		
	{ "cpu2a.bin",      0x008000, 0x92c92924, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "cpu2b.bin",      0x020000, 0x2c8ad475, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Fantzn2xp)
STD_ROM_FN(Fantzn2xp)

static struct BurnRomInfo Fantzn2xps2RomDesc[] = {
	{ "fz2_s16c.p00",   0x040000, 0xb7d16c1d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "fz2_s16c.p01",   0x040000, 0x2c47487c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "fz2_s16c.scr",   0x080000, 0xed3b1ac0, SYS16_ROM_TILES | BRF_GRA },
	
	{ "fz2_s16c.spr",   0x200000, 0x14d06fee, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "fz2_s16c.snd",   0x030000, 0x0ed30ec1, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Fantzn2xps2)
STD_ROM_FN(Fantzn2xps2)

static struct BurnRomInfo FantzntaRomDesc[] = {
	{ "fz1_s16b_ta.p00",0x040000, 0xbad0537a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "fz1_s16b.scr",   0x040000, 0x10ecd8b9, SYS16_ROM_TILES | BRF_GRA },
	
	{ "fz1_s16b_ta.obj",0x200000, 0x51fd438f, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "fz1_s16b.snd",   0x020000, 0xa00701fb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Fantznta)
STD_ROM_FN(Fantznta)

static struct BurnRomInfo FpointRomDesc[] = {
	{ "epr-12599b.a4",  0x10000, 0x26e3f354, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12598b.a1",  0x10000, 0xc0f2c97d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "opr-12593.a14",  0x10000, 0xcc0582d8, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12594.a15",  0x10000, 0x8bfc4815, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12595.a16",  0x10000, 0x5b18d60b, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-12596.b1",   0x10000, 0x4a4041f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12597.b5",   0x10000, 0x6961e676, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12592.a10",  0x08000, 0x9a8c11bb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0127a.key",  0x02000, 0x5adb0042, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Fpoint)
STD_ROM_FN(Fpoint)

static struct BurnRomInfo Fpoint1RomDesc[] = {
	{ "epr-12591b.a7",  0x10000, 0x248b3e1b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12590b.a5",  0x10000, 0x75256e3d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "opr-12593.a14",  0x10000, 0xcc0582d8, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12594.a15",  0x10000, 0x8bfc4815, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-12595.a16",  0x10000, 0x5b18d60b, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-12596.b1",   0x10000, 0x4a4041f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12597.b5",   0x10000, 0x6961e676, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12592.a10",  0x08000, 0x9a8c11bb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0127a.key",  0x02000, 0x5adb0042, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Fpoint1)
STD_ROM_FN(Fpoint1)

static struct BurnRomInfo FpointblRomDesc[] = {
	{ "flpoint.003",    0x10000, 0x4d6df514, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "flpoint.002",    0x10000, 0x4dff2ee8, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "flpoint.006",    0x10000, 0xc539727d, SYS16_ROM_TILES | BRF_GRA },
	{ "flpoint.005",    0x10000, 0x82c0b8b0, SYS16_ROM_TILES | BRF_GRA },
	{ "flpoint.004",    0x10000, 0x522426ae, SYS16_ROM_TILES | BRF_GRA },
	
	{ "12596.bin",      0x10000, 0x4a4041f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12597.bin",      0x10000, 0x6961e676, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "flpoint.001",    0x08000, 0xc5b8e0fe, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Fpointbl)
STD_ROM_FN(Fpointbl)

static struct BurnRomInfo FpointbjRomDesc[] = {
	{ "boot2.003",      0x10000, 0x6c00d1b0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "boot2.002",      0x10000, 0xc1fcd704, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "flpoint.006",    0x10000, 0xc539727d, SYS16_ROM_TILES | BRF_GRA },
	{ "flpoint.005",    0x10000, 0x82c0b8b0, SYS16_ROM_TILES | BRF_GRA },
	{ "flpoint.004",    0x10000, 0x522426ae, SYS16_ROM_TILES | BRF_GRA },
	
	{ "12596.bin",      0x10000, 0x4a4041f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12597.bin",      0x10000, 0x6961e676, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "flpoint.001",    0x08000, 0xc5b8e0fe, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "82s129.1",       0x00100, 0xa7c22d96, BRF_OPT },
	{ "82s123.2",       0x00020, 0x58bcf8bd, BRF_OPT },
	{ "fpointbj_gal16v8_1.bin", 0x00117, 0xba7f292c, BRF_OPT },
	{ "fpointbj_gal16v8_3.bin", 0x00117, 0xce1ab1e1, BRF_OPT },
	{ "fpointbj_gal20v8.bin", 0x00400, 0x00000000, BRF_OPT | BRF_NODUMP },
};


STD_ROM_PICK(Fpointbj)
STD_ROM_FN(Fpointbj)

static struct BurnRomInfo GoldnaxeRomDesc[] = {
	{ "epr-12545.ic2",  0x40000, 0xa97c4e4d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12544.ic1",  0x40000, 0x5e38f668, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-12385.ic19", 0x20000, 0xb8a4e7e0, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12386.ic20", 0x20000, 0x25d7d779, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12387.ic21", 0x20000, 0xc7fcadf3, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12378.ic9",  0x40000, 0x119e5a82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12379.ic12", 0x40000, 0x1a0e8c57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12380.ic10", 0x40000, 0xbb2c0853, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12381.ic13", 0x40000, 0x81ba6ecc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12382.ic11", 0x40000, 0x81601c6f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12383.ic14", 0x40000, 0x5dbacf7a, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12390.ic8",  0x08000, 0x399fc5f5, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12384.ic6",  0x20000, 0x6218d8e7, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0123a.bin",  0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Goldnaxe)
STD_ROM_FN(Goldnaxe)

static struct BurnRomInfo Goldnaxe1RomDesc[] = {
	{ "epr-12389.ic2",  0x40000, 0x35d5fa77, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12388.ic1",  0x40000, 0x72952a93, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-12385.ic19", 0x20000, 0xb8a4e7e0, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12386.ic20", 0x20000, 0x25d7d779, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12387.ic21", 0x20000, 0xc7fcadf3, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12378.ic9",  0x40000, 0x119e5a82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12379.ic12", 0x40000, 0x1a0e8c57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12380.ic10", 0x40000, 0xbb2c0853, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12381.ic13", 0x40000, 0x81ba6ecc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12382.ic11", 0x40000, 0x81601c6f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12383.ic14", 0x40000, 0x5dbacf7a, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12390.ic8",  0x08000, 0x399fc5f5, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12384.ic6",  0x20000, 0x6218d8e7, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0110.key",   0x02000, 0xcd517dc6, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Goldnaxe1)
STD_ROM_FN(Goldnaxe1)

static struct BurnRomInfo Goldnaxe2RomDesc[] = {
	{ "epr-12523.a7",   0x20000, 0x8e6128d7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12522.a5",   0x20000, 0xb6c35160, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12521.a8",   0x20000, 0x5001d713, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12519.a6",   0x20000, 0x4438ca8e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-12385.a14",  0x20000, 0xb8a4e7e0, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12386.a15",  0x20000, 0x25d7d779, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12387.a16",  0x20000, 0xc7fcadf3, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12378.b1",   0x40000, 0x119e5a82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12379.b5",   0x40000, 0x1a0e8c57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12380.b2",   0x40000, 0xbb2c0853, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12381.b6",   0x40000, 0x81ba6ecc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12382.b3",   0x40000, 0x81601c6f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12383.b7",   0x40000, 0x5dbacf7a, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12390.a10",  0x08000, 0x399fc5f5, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12384.a11",  0x20000, 0x6218d8e7, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0112.bin",   0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Goldnaxe2)
STD_ROM_FN(Goldnaxe2)

static struct BurnRomInfo Goldnaxe3RomDesc[] = {
	{ "epr-12525.a7",   0x20000, 0x48332c76, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12524.a5",   0x20000, 0x8e58f342, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12521.a8",   0x20000, 0x5001d713, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12519.a6",   0x20000, 0x4438ca8e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-12385.a14",  0x20000, 0xb8a4e7e0, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12386.a15",  0x20000, 0x25d7d779, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12387.a16",  0x20000, 0xc7fcadf3, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12378.b1",   0x40000, 0x119e5a82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12379.b5",   0x40000, 0x1a0e8c57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12380.b2",   0x40000, 0xbb2c0853, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12381.b6",   0x40000, 0x81ba6ecc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12382.b3",   0x40000, 0x81601c6f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12383.b7",   0x40000, 0x5dbacf7a, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12390.a10",  0x08000, 0x399fc5f5, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12384.a11",  0x20000, 0x6218d8e7, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0120.key",   0x02000, 0x946e9fa6, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Goldnaxe3)
STD_ROM_FN(Goldnaxe3)

static struct BurnRomInfo GoldnaxejRomDesc[] = {
	{ "epr-12540.a7",   0x20000, 0x0c7ccc6d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12539.a5",   0x20000, 0x1f24f7d0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12521.a8",   0x20000, 0x5001d713, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12519.a6",   0x20000, 0x4438ca8e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-12385.a14",  0x20000, 0xb8a4e7e0, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12386.a15",  0x20000, 0x25d7d779, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12387.a16",  0x20000, 0xc7fcadf3, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12378.b1",   0x40000, 0x119e5a82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12379.b5",   0x40000, 0x1a0e8c57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12380.b2",   0x40000, 0xbb2c0853, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12381.b6",   0x40000, 0x81ba6ecc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12382.b3",   0x40000, 0x81601c6f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12383.b7",   0x40000, 0x5dbacf7a, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12390.a10",  0x08000, 0x399fc5f5, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12384.a11",  0x20000, 0x6218d8e7, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0121.key",   0x02000, 0x72afed01, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Goldnaxej)
STD_ROM_FN(Goldnaxej)

static struct BurnRomInfo GoldnaxeuRomDesc[] = {
	{ "epr-12543.ic2",  0x40000, 0xb0df9ca4, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12542.ic1",  0x40000, 0xb7994d3c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-12385.ic19", 0x20000, 0xb8a4e7e0, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12386.ic20", 0x20000, 0x25d7d779, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12387.ic21", 0x20000, 0xc7fcadf3, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12378.ic9",  0x40000, 0x119e5a82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12379.ic12", 0x40000, 0x1a0e8c57, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12380.ic10", 0x40000, 0xbb2c0853, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12381.ic13", 0x40000, 0x81ba6ecc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12382.ic11", 0x40000, 0x81601c6f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12383.ic14", 0x40000, 0x5dbacf7a, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12390.ic8",  0x08000, 0x399fc5f5, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12384.ic6",  0x20000, 0x6218d8e7, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0122.key",   0x02000, 0xf123c2fb, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Goldnaxeu)
STD_ROM_FN(Goldnaxeu)

static struct BurnRomInfo HwchampRomDesc[] = {
	{ "epr-11239.a7",   0x20000, 0xe5abfed7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11238.a5",   0x20000, 0x25180124, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-11241.a14",  0x20000, 0xfc586a86, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11166.b14",  0x20000, 0xaeaaa9d8, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11242.a15",  0x20000, 0x7715a742, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11167.b15",  0x20000, 0x63a82afa, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11243.a16",  0x20000, 0xf30cd5fd, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-11168.b16",  0x20000, 0x5b8494a8, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-11158.b1",   0x20000, 0xfc098a13, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11162.b5",   0x20000, 0x5db934a8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11159.b2",   0x20000, 0x1f27ee74, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11163.b6",   0x20000, 0x8a6a5cf1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11160.b3",   0x20000, 0xc0b2ba82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11164.b7",   0x20000, 0xd6c7917b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11161.b4",   0x20000, 0x35c9e44b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-11165.b8",   0x20000, 0x57e8f9d2, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-11240.a10",  0x08000, 0x96a12d9d, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-11244.a11",  0x20000, 0x4191c03d, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "mpr-11245.a12",  0x20000, 0xa4d53f7b, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "pls153.bin",     0x000eb, 0x39b47212, BRF_OPT }, // PLD
};


STD_ROM_PICK(Hwchamp)
STD_ROM_FN(Hwchamp)

static struct BurnRomInfo HwchampjRomDesc[] = {
	{ "epr-11152.a7",   0x20000, 0x8ab0ce62, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11153.a5",   0x20000, 0x84a743de, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11202.a14",  0x20000, 0x7c94ede3, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11213.b14",  0x20000, 0xaeaaa9d8, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11203.a15",  0x20000, 0x327754f7, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11214.b15",  0x20000, 0x63a82afa, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11204.a16",  0x20000, 0xdfc4cd33, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11215.b16",  0x20000, 0x5b8494a8, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11205.b1",   0x20000, 0xfc098a13, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11209.b5",   0x20000, 0x5db934a8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11206.b2",   0x20000, 0x1f27ee74, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11210.b6",   0x20000, 0x8a6a5cf1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11207.b3",   0x20000, 0xc0b2ba82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11211.b7",   0x20000, 0xd6c7917b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11208.b4",   0x20000, 0x35c9e44b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11212.b8",   0x20000, 0x57e8f9d2, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-11154.a10",  0x08000, 0x65791275, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11200.a11",  0x20000, 0x5c41a68a, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11201.a12",  0x20000, 0x9a993120, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0046.key",   0x02000, 0x488b3f8b, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
	
	{ "pls153.bin",     0x000eb, 0x39b47212, BRF_OPT }, // PLD
};


STD_ROM_PICK(Hwchampj)
STD_ROM_FN(Hwchampj)

static struct BurnRomInfo MvpRomDesc[] = {
	{ "13000.rom",      0x40000, 0x2e0e21ec, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "12999.rom",      0x40000, 0xfd213d28, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "13011.rom",      0x40000, 0x1cb871fc, SYS16_ROM_TILES | BRF_GRA },
	{ "13012.rom",      0x40000, 0xb75e6821, SYS16_ROM_TILES | BRF_GRA },
	{ "13013.rom",      0x40000, 0xf1944a3c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "13003.rom",      0x40000, 0x21424151, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13006.rom",      0x40000, 0x2e9afd2f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13004.rom",      0x40000, 0x0aa09dd3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13007.rom",      0x40000, 0x55c8605b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13005.rom",      0x40000, 0xc899c810, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13008.rom",      0x40000, 0xb3d46dfc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13009.rom",      0x40000, 0x126d2e37, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13010.rom",      0x40000, 0xdf37c567, SYS16_ROM_SPRITES | BRF_GRA },

	{ "13002.rom",      0x08000, 0x1b6e1515, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "13001.rom",      0x40000, 0xe8cace8c, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0143.key",   0x02000, 0xfba2e8da, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Mvp)
STD_ROM_FN(Mvp)

static struct BurnRomInfo MvpjRomDesc[] = {
	{ "epr-12967.a7",   0x20000, 0xe53ac137, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12965.a5",   0x20000, 0x4266cb9e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12968.a8",   0x20000, 0x91c772ac, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12966.a6",   0x20000, 0x39365a79, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-12971.a14",  0x20000, 0x245dcd1f, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12983.b14",  0x20000, 0xf3570fc9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12972.a15",  0x20000, 0xff7c4278, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12984.b15",  0x20000, 0xd37d1876, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12973.a16",  0x20000, 0x8dc9b9ea, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12985.b16",  0x20000, 0xe3f33a8a, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12974.b1",   0x20000, 0xe1da5597, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12977.b5",   0x20000, 0xb9eb9762, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12975.b2",   0x20000, 0x364d51d1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12978.b6",   0x20000, 0x014b5442, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12976.b3",   0x20000, 0x43b549c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12979.b7",   0x20000, 0x20f603f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12962.a1",   0x20000, 0x9b678da3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12980.b10",  0x20000, 0x883b792a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12963.a2",   0x20000, 0x8870f95a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12981.b11",  0x20000, 0x48636cb0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12964.a3",   0x20000, 0xf9148c5d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12982.b12",  0x20000, 0xc4453292, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12969.a10",  0x08000, 0xec621893, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12970.a11",  0x20000, 0x8f7d7657, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0142.key",   0x02000, 0x90468045, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Mvpj)
STD_ROM_FN(Mvpj)

static struct BurnRomInfo PassshtRomDesc[] = {
	{ "epr-11871.a4",   0x10000, 0x0f9ccea5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11870.a1",   0x10000, 0xdf43ebcf, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11854.b9",   0x10000, 0xd31c0b6c, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11855.b10",  0x10000, 0xb78762b4, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11856.b11",  0x10000, 0xea49f666, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-11862.b1",   0x10000, 0xb6e94727, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11865.b5",   0x10000, 0x17e8d5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11863.b2",   0x10000, 0x3e670098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11866.b6",   0x10000, 0x50eb71cc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11864.b3",   0x10000, 0x05733ca8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11867.b7",   0x10000, 0x81e49697, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11857.a7",   0x08000, 0x789edc06, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11858.a8",   0x08000, 0x08ab0018, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11859.a9",   0x08000, 0x8673e01b, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11860.a10",  0x08000, 0x10263746, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11861.a11",  0x08000, 0x38b54a71, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0080.key",   0x02000, 0x222d016f, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Passsht)
STD_ROM_FN(Passsht)

static struct BurnRomInfo PassshtaRomDesc[] = {
	{ "8.4a",           0x10000, 0xb84dc139, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "5.1a",           0x10000, 0xeffe29df, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11854.b9",   0x10000, 0xd31c0b6c, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11855.b10",  0x10000, 0xb78762b4, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11856.b11",  0x10000, 0xea49f666, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-11862.b1",   0x10000, 0xb6e94727, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11865.b5",   0x10000, 0x17e8d5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11863.b2",   0x10000, 0x3e670098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11866.b6",   0x10000, 0x50eb71cc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11864.b3",   0x10000, 0x05733ca8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11867.b7",   0x10000, 0x81e49697, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11857.a7",   0x08000, 0x789edc06, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11858.a8",   0x08000, 0x08ab0018, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11859.a9",   0x08000, 0x8673e01b, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11860.a10",  0x08000, 0x10263746, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11861.a11",  0x08000, 0x38b54a71, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0074.key",  0x02000, 0x71bd232d, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Passshta)
STD_ROM_FN(Passshta)

static struct BurnRomInfo PassshtjRomDesc[] = {
	{ "epr-11853.a4",   0x10000, 0xfab337e7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11852.a1",   0x10000, 0x892a81fc, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-11854.b9",   0x10000, 0xd31c0b6c, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11855.b10",  0x10000, 0xb78762b4, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11856.b11",  0x10000, 0xea49f666, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-11862.b1",   0x10000, 0xb6e94727, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11865.b5",   0x10000, 0x17e8d5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11863.b2",   0x10000, 0x3e670098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11866.b6",   0x10000, 0x50eb71cc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11864.b3",   0x10000, 0x05733ca8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11867.b7",   0x10000, 0x81e49697, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11857.a7",   0x08000, 0x789edc06, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11858.a8",   0x08000, 0x08ab0018, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11859.a9",   0x08000, 0x8673e01b, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11860.a10",  0x08000, 0x10263746, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11861.a11",  0x08000, 0x38b54a71, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0070.key",   0x02000, 0x5d0308aa, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Passshtj)
STD_ROM_FN(Passshtj)

static struct BurnRomInfo PassshtbRomDesc[] = {
	{ "pass3_2p.bin",   0x10000, 0x26bb9299, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "pass4_2p.bin",   0x10000, 0x06ac6d5d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "opr11854.b9",    0x10000, 0xd31c0b6c, SYS16_ROM_TILES | BRF_GRA },
	{ "opr11855.b10",   0x10000, 0xb78762b4, SYS16_ROM_TILES | BRF_GRA },
	{ "opr11856.b11",   0x10000, 0xea49f666, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr11862.b1",    0x10000, 0xb6e94727, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr11865.b5",    0x10000, 0x17e8d5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr11863.b2",    0x10000, 0x3e670098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr11866.b6",    0x10000, 0x50eb71cc, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr11864.b3",    0x10000, 0x05733ca8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr11867.b7",    0x10000, 0x81e49697, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr11857.a7",    0x08000, 0x789edc06, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr11858.a8",    0x08000, 0x08ab0018, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr11859.a9",    0x08000, 0x8673e01b, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr11860.a10",   0x08000, 0x10263746, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr11861.a11",   0x08000, 0x38b54a71, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Passshtb)
STD_ROM_FN(Passshtb)

static struct BurnRomInfo RiotcityRomDesc[] = {
	{ "epr-14612.bin",  0x20000, 0xa1b331ec, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-14610.bin",  0x20000, 0xcd4f2c50, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-14613.bin",  0x20000, 0x0659df4c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-14611.bin",  0x20000, 0xd9e6f80b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-14616.bin",  0x20000, 0x46d30368, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-14625.bin",  0x20000, 0xabfb80fe, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-14617.bin",  0x20000, 0x884e40f9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-14626.bin",  0x20000, 0x4ef55846, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-14618.bin",  0x20000, 0x00eb260e, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-14627.bin",  0x20000, 0x961e5f82, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-14619.bin",  0x40000, 0x6f2b5ef7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-14622.bin",  0x40000, 0x7ca7e40d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-14620.bin",  0x40000, 0x66183333, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-14623.bin",  0x40000, 0x98630049, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-14621.bin",  0x40000, 0xc0f2820e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-14624.bin",  0x40000, 0xd1a68448, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-14614.bin",  0x10000, 0xc65cc69a, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-14615.bin",  0x20000, 0x46653db1, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Riotcity)
STD_ROM_FN(Riotcity)

static struct BurnRomInfo RyukyuRomDesc[] = {
	{ "epr-13348.a7",   0x10000, 0x5f0e0c86, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-13347.a5",   0x10000, 0x398031fa, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "opr-13351.a14",  0x20000, 0xa68a4e6d, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13352.a15",  0x20000, 0x5e5531e4, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-13353.a16",  0x20000, 0x6d23dfd8, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-13354.b1",   0x20000, 0xf07aad99, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13356.b5",   0x20000, 0x5498290b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13355.b2",   0x20000, 0x67890019, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-13357.b6",   0x20000, 0xf9e7cf03, SYS16_ROM_SPRITES | BRF_GRA },
		
	{ "epr-13349.a10",  0x08000, 0xb83183f8, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "opr-13350.a11",  0x20000, 0x3c59a658, SYS16_ROM_UPD7759DATA | BRF_SND },
		
	{ "317-5023.key",   0x02000, 0x43704331, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Ryukyu)
STD_ROM_FN(Ryukyu)

static struct BurnRomInfo SdibRomDesc[] = {
	{ "epr-10986a.a4",  0x08000, 0x3e136215, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10984a.a1",  0x08000, 0x44bf3cf5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10987a.a5",  0x08000, 0xcfd79404, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10985a.a2",  0x08000, 0x1c21a03f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10829.a6",   0x08000, 0xa431ab08, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10826.a3",   0x08000, 0x2ed8e4b7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10772.b9",   0x10000, 0x182b6301, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10773.b10",  0x10000, 0x8f7129a2, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10774.b11",  0x10000, 0x4409411f, SYS16_ROM_TILES | BRF_GRA },
	
	{ "10760.b1",       0x10000, 0x70de327b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10763.b5",       0x10000, 0x99ec5cb5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10761.b2",       0x10000, 0x4e80f80d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10764.b6",       0x10000, 0x602da5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10762.b3",       0x10000, 0x464b5f78, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10765.b7",       0x10000, 0x0a73a057, SYS16_ROM_SPRITES | BRF_GRA },

	{ "10775.a7",       0x08000, 0x4cbd55a8, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0028.key",   0x02000, 0x9a5307b2, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Sdib)
STD_ROM_FN(Sdib)

static struct BurnRomInfo SdiblRomDesc[] = {
	{ "a4.rom",         0x08000, 0xf2c41dd6, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "a1.rom",         0x08000, 0xa9f816ef, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "a5.rom",         0x08000, 0x7952e27e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "a2.rom",         0x08000, 0x369af326, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "a6.rom",         0x08000, 0x8ee2c287, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "a3.rom",         0x08000, 0x193e4231, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10772.b9",   0x10000, 0x182b6301, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10773.b10",  0x10000, 0x8f7129a2, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10774.b11",  0x10000, 0x4409411f, SYS16_ROM_TILES | BRF_GRA },
	
	{ "b1.rom",         0x10000, 0x30e2c50a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b5.rom",         0x10000, 0x794e3e8b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b2.rom",         0x10000, 0x6a8b3fd0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10764.b6",       0x10000, 0x602da5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "b3.rom",         0x10000, 0xb9de3aeb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10765.b7",       0x10000, 0x0a73a057, SYS16_ROM_SPRITES | BRF_GRA },

	{ "a7.rom",         0x08000, 0x793f9f7f, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Sdibl)
STD_ROM_FN(Sdibl)

static struct BurnRomInfo DefenseRomDesc[] = {
	{ "epr-10917a.a4",  0x08000, 0xd91ac47c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10915.a1",   0x08000, 0x7344c510, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10918a.a5",  0x08000, 0xe41befcd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10916a.a2",  0x08000, 0x7f58ba12, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10829.a6",   0x08000, 0xa431ab08, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10826.a3",   0x08000, 0x2ed8e4b7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "10919.b9",       0x10000, 0x23b88f82, SYS16_ROM_TILES | BRF_GRA },
	{ "10920.b10",      0x10000, 0x22b1fb4c, SYS16_ROM_TILES | BRF_GRA },
	{ "10921.b11",      0x10000, 0x7788f55d, SYS16_ROM_TILES | BRF_GRA },
	
	{ "10760.b1",       0x10000, 0x70de327b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10763.b5",       0x10000, 0x99ec5cb5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10761.b2",       0x10000, 0x4e80f80d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10764.b6",       0x10000, 0x602da5d5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10762.b3",       0x10000, 0x464b5f78, SYS16_ROM_SPRITES | BRF_GRA },
	{ "10765.b7",       0x10000, 0x0a73a057, SYS16_ROM_SPRITES | BRF_GRA },

	{ "10775.a7",       0x08000, 0x4cbd55a8, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0028.key",   0x02000, 0x9a5307b2, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Defense)
STD_ROM_FN(Defense)

static struct BurnRomInfo Shinobi2RomDesc[] = {
	{ "epr-11282.a4",   0x10000, 0x5f2e5524, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11280.a1",   0x10000, 0xbdfe5c38, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11283.a5",   0x10000, 0x9d46e707, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11281.a2",   0x10000, 0x7961d07e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11284.b9",   0x10000, 0x5f62e163, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11285.b10",  0x10000, 0x75f8fbc9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11286.b11",  0x10000, 0x06508bb9, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11290.b1",   0x10000, 0x611f413a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11294.b5",   0x10000, 0x5eb00fc1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11291.b2",   0x10000, 0x3c0797c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11295.b6",   0x10000, 0x25307ef8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11292.b3",   0x10000, 0xc29ac34e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11296.b7",   0x10000, 0x04a437f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11293.b4",   0x10000, 0x41f41063, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11297.b8",   0x10000, 0xb6e1fd72, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11287.a7",   0x08000, 0xe8cccd42, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11288.a8",   0x08000, 0xc8df8460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11289.a9",   0x08000, 0xe5a4cf30, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0049.key",   0x02000, 0x8fac824f, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Shinobi2)
STD_ROM_FN(Shinobi2)

static struct BurnRomInfo Shinobi3RomDesc[] = {
	{ "epr-11299.a4",   0x10000, 0xb930399d, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11298.a1",   0x10000, 0x343f4c46, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11283.a5",   0x10000, 0x9d46e707, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11281.a2",   0x10000, 0x7961d07e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11284.b9",   0x10000, 0x5f62e163, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11285.b10",  0x10000, 0x75f8fbc9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11286.b11",  0x10000, 0x06508bb9, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11290.b1",   0x10000, 0x611f413a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11294.b5",   0x10000, 0x5eb00fc1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11291.b2",   0x10000, 0x3c0797c0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11295.b6",   0x10000, 0x25307ef8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11292.b3",   0x10000, 0xc29ac34e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11296.b7",   0x10000, 0x04a437f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11293.b4",   0x10000, 0x41f41063, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11297.b8",   0x10000, 0xb6e1fd72, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11372.a7",   0x08000, 0x0824269a, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11288.a8",   0x08000, 0xc8df8460, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11289.a9",   0x08000, 0xe5a4cf30, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0054.key",   0x02000, 0x39fd4535, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Shinobi3)
STD_ROM_FN(Shinobi3)

static struct BurnRomInfo Shinobi4RomDesc[] = {
	{ "epr-11360.a7",   0x20000, 0xb1f67ab9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11359.a5",   0x20000, 0x0f0306e1, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11363.a14",  0x20000, 0x40914168, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11364.a15",  0x20000, 0xe63649a4, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11365.a16",  0x20000, 0x1ef55d20, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11366.b1",   0x20000, 0x319ede73, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11368.b5",   0x20000, 0x0377d7ce, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11367.b2",   0x20000, 0x1d06c5c7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11369.b6",   0x20000, 0xd751d2a2, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11377.a10",  0x08000, 0x0fb6af34, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11362.a11",  0x20000, 0x256af749, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0054.key",   0x02000, 0x39fd4535, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Shinobi4)
STD_ROM_FN(Shinobi4)

static struct BurnRomInfo Shinobi5RomDesc[] = {
	{ "epr-11360.a7",   0x20000, 0xb1f67ab9, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11359.a5",   0x20000, 0x0f0306e1, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11363.a14",  0x20000, 0x40914168, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11364.a15",  0x20000, 0xe63649a4, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11365.a16",  0x20000, 0x1ef55d20, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11366.b1",   0x20000, 0x319ede73, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11368.b5",   0x20000, 0x0377d7ce, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11367.b2",   0x20000, 0x1d06c5c7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11369.b6",   0x20000, 0xd751d2a2, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11361.a10",  0x08000, 0x1f47ebcb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11362.a11",  0x20000, 0x256af749, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Shinobi5)
STD_ROM_FN(Shinobi5)

static struct BurnRomInfo SjryukoRomDesc[] = {
	{ "epr-12256.a4",     0x08000, 0x5987ee1b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12253.a1",     0x08000, 0x26a822df, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12257.a5",     0x08000, 0x3a2acc3f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12254.a2",     0x08000, 0x7e908217, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12224-95.b9",  0x08000, 0xeac17ba1, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12225-94.b10", 0x08000, 0x2310fc98, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12226-93.b11", 0x08000, 0x210e6999, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12232-10.b1",  0x10000, 0x0adec62b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12236-11.b5",  0x10000, 0x286b9af8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12233-17.b2",  0x10000, 0x3e45969c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12237-18.b6",  0x10000, 0xe5058e96, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12234-23.b3",  0x10000, 0x8c8d54ef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12238-24.b7",  0x10000, 0x7ada3304, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12235-29.b4",  0x10000, 0xfa45d511, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12239-30.b8",  0x10000, 0x91f70c8b, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12244.a7",     0x08000, 0xcb2a47e5, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12245.a8",     0x08000, 0x66164134, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-12246.a9",     0x08000, 0xf1242582, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-12247.a10",    0x08000, 0xef8a64c6, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-12248.a11",    0x08000, 0xd1eabdab, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-5021.key",     0x02000, 0x4a3422e4, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Sjryuko)
STD_ROM_FN(Sjryuko)

static struct BurnRomInfo SnapperRomDesc[] = {
	{ "snap2.r01",      0x10000, 0x9a9e4ed3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "snap1.r02",      0x10000, 0xcd468d6a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "snap4.r03",      0x08000, 0x0f848e1e, SYS16_ROM_TILES | BRF_GRA },
	{ "snap3.r04",      0x08000, 0xc7f8cf0e, SYS16_ROM_TILES | BRF_GRA },
	{ "snap5.r05",      0x08000, 0x378e08eb, SYS16_ROM_TILES | BRF_GRA },
};


STD_ROM_PICK(Snapper)
STD_ROM_FN(Snapper)

static struct BurnRomInfo SonicbomRomDesc[] = {
	{ "epr-11342.a4",   0x10000, 0x454693f1, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11340.a1",   0x10000, 0x03ba3fed, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11343.a5",   0x10000, 0xedfeb7d4, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11341.a2",   0x10000, 0x0338f771, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "opr-11344.b9",   0x10000, 0x59a9f940, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11345.b10",  0x10000, 0xb44c068b, SYS16_ROM_TILES | BRF_GRA },
	{ "opr-11346.b11",  0x10000, 0xe5ada66c, SYS16_ROM_TILES | BRF_GRA },
	
	{ "opr-11350.b1",   0x10000, 0x525ba1df, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11354.b5",   0x10000, 0x793fa3ac, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11351.b2",   0x10000, 0x63b1f1ca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11355.b6",   0x10000, 0xfe0fa332, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11352.b3",   0x10000, 0x047fa4b0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11356.b7",   0x10000, 0xaea3c39d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11353.b4",   0x10000, 0x4e0791f8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-11357.b8",   0x10000, 0xa7c5ea41, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11347.a7",   0x08000, 0xb41f0ced, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11348.a8",   0x08000, 0x89924588, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11349.a9",   0x08000, 0x8e4b6204, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0053.key",   0x02000, 0x91c80c88, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Sonicbom)
STD_ROM_FN(Sonicbom)

static struct BurnRomInfo SuprleagRomDesc[] = {
	{ "epr-11133.a04",   0x10000, 0xeed72f37, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11130.a01",   0x10000, 0xe2451676, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11134.a05",   0x10000, 0xccd857f5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11131.a02",   0x10000, 0x9b78c2cc, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11135.a06",   0x10000, 0x3735e0e1, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-11132.a03",   0x10000, 0xff199325, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-11136.b09",   0x10000, 0xc3860ce4, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11137.b10",   0x10000, 0x92d96187, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-11138.b11",   0x10000, 0xc01dc773, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-11144.b01",   0x10000, 0xb31de51c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11148.b05",   0x10000, 0x126e1309, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11145.b02",   0x10000, 0x4223d2c3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11149.b06",   0x10000, 0x694d3765, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11146.b03",   0x10000, 0xbf0359b6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11150.b07",   0x10000, 0x9fc0aded, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11147.b04",   0x10000, 0x3e592772, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-11151.b08",   0x10000, 0x9de95169, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-11139.a07",   0x08000, 0x9cbd99da, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-11140.a08",   0x08000, 0xb297371b, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11141.a09",   0x08000, 0x19756aa6, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11142.a10",   0x08000, 0x25d26c66, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-11143.a11",   0x08000, 0x848b7b77, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0045.key",    0x02000, 0x0594cc2e, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Suprleag)
STD_ROM_FN(Suprleag)

static struct BurnRomInfo Tetris1RomDesc[] = {
	{ "epr-12164.a4",  0x08000, 0xb329cd6f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12163.a1",  0x08000, 0xd372d3f3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr-12165.b9",  0x10000, 0x62640221, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12166.b10", 0x10000, 0x9abd183b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12167.b11", 0x10000, 0x2495fd4e, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12169.b1",  0x08000, 0xdacc6165, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12170.b5",  0x08000, 0x87354e42, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12168.a7",  0x08000, 0xbd9ba01b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0091.key",  0x02000, 0xa7937661, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Tetris1)
STD_ROM_FN(Tetris1)

static struct BurnRomInfo Tetris2RomDesc[] = {
	{ "epr-12193.a7",  0x20000, 0x44466ed4, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12192.a5",  0x20000, 0xa1c8af00, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr-12165.b9",  0x10000, 0x62640221, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12166.b10", 0x10000, 0x9abd183b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12167.b11", 0x10000, 0x2495fd4e, SYS16_ROM_TILES | BRF_GRA },
	
	{ "obj0-o.rom",    0x10000, 0x2fb38880, SYS16_ROM_SPRITES | BRF_GRA },
	{ "obj0-e.rom",    0x10000, 0xd6a02cba, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12168.a7",  0x08000, 0xbd9ba01b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0092.key",  0x02000, 0xd10e1ad9, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Tetris2)
STD_ROM_FN(Tetris2)

static struct BurnRomInfo TetrisblRomDesc[] = {
	{ "rom2.bin",      0x10000, 0x4d165c38, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "rom1.bin",      0x10000, 0x1e912131, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr12165.b9",   0x10000, 0x62640221, SYS16_ROM_TILES | BRF_GRA },
	{ "epr12166.b10",  0x10000, 0x9abd183b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr12167.b11",  0x10000, 0x2495fd4e, SYS16_ROM_TILES | BRF_GRA },
	
	{ "obj0-o.rom",    0x10000, 0x2fb38880, SYS16_ROM_SPRITES | BRF_GRA },
	{ "obj0-e.rom",    0x10000, 0xd6a02cba, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr12168.a7",   0x08000, 0xbd9ba01b, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Tetrisbl)
STD_ROM_FN(Tetrisbl)

static struct BurnRomInfo TimescanRomDesc[] = {
	{ "epr-10853.a4",   0x08000, 0x24d7c5fb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10850.a1",   0x08000, 0xf1575732, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10854.a5",   0x08000, 0x82d0b237, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10851.a2",   0x08000, 0xf5ce271b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10855.a6",   0x08000, 0x63e95a53, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10852.a3",   0x08000, 0x7cd1382b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-10543.b9",   0x08000, 0x07dccc37, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10544.b10",  0x08000, 0x84fb9a3a, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10545.b11",  0x08000, 0xc8694bc0, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-10548.b1",   0x08000, 0xaa150735, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10552.b5",   0x08000, 0x6fcbb9f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10549.b2",   0x08000, 0x2f59f067, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10553.b6",   0x08000, 0x8a220a9f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10550.b3",   0x08000, 0xf05069ff, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10554.b7",   0x08000, 0xdc64f809, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10551.b4",   0x08000, 0x435d811f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10555.b8",   0x08000, 0x2143c471, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-10562.a7",   0x08000, 0x3f5028bf, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10563.a8",   0x08000, 0x9db7eddf, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Timescan)
STD_ROM_FN(Timescan)

static struct BurnRomInfo ToryumonRomDesc[] = {
	{ "epr-17689.a2",   0x20000, 0x4f0dee19, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-17688.a1",   0x20000, 0x717d81c7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "epr-17700.b11",  0x40000, 0x8f288b37, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-17701.b12",  0x40000, 0x6dfb025b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-17702.b13",  0x40000, 0xae0b7eab, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-17692.b1",   0x20000, 0x543c4327, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-17695.b4",   0x20000, 0xee60f244, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-17693.b2",   0x20000, 0x4a350b3e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-17696.b5",   0x20000, 0x6edb54f1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-17694.b3",   0x20000, 0xb296d71d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-17697.b6",   0x20000, 0x6ccb7b28, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-17698.b7",   0x20000, 0xcd4dfb82, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-17699.b8",   0x20000, 0x2694ecce, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-17691.a13",  0x08000, 0x14205388, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-17690.a11",  0x40000, 0x4f9ba4e4, SYS16_ROM_UPD7759DATA | BRF_SND },
};


STD_ROM_PICK(Toryumon)
STD_ROM_FN(Toryumon)

static struct BurnRomInfo TturfRomDesc[] = {
	{ "12327.a7",       0x20000, 0x0376c593, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "12326.a5",       0x20000, 0xf998862b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "12268.a14",      0x10000, 0xe0dac07f, SYS16_ROM_TILES | BRF_GRA },
	{ "12269.a15",      0x10000, 0x457a8790, SYS16_ROM_TILES | BRF_GRA },
	{ "12270.a16",      0x10000, 0x69fc025b, SYS16_ROM_TILES | BRF_GRA },
	
	{ "12279.b1",       0x10000, 0x7a169fb1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12283.b5",       0x10000, 0xae0fa085, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12278.b2",       0x10000, 0x961d06b7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12282.b6",       0x10000, 0xe8671ee1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12277.b3",       0x10000, 0xf16b6ba2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12281.b7",       0x10000, 0x1ef1077f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12276.b4",       0x10000, 0x838bd71f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12280.b8",       0x10000, 0x639a57cb, SYS16_ROM_SPRITES | BRF_GRA },

	{ "12328.a10",      0x08000, 0x00000000, SYS16_ROM_Z80PROG | BRF_NODUMP | BRF_PRG },
	
	{ "12329.a11",      0x10000, 0xed9a686d, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "12330.a12",      0x10000, 0xfb762bca, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0104.bin",   0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Tturf)
STD_ROM_FN(Tturf)

static struct BurnRomInfo TturfuRomDesc[] = {
	{ "epr-12266.bin",  0x10000, 0xf549def8, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12264.bin",  0x10000, 0xf7cdb289, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12267.bin",  0x10000, 0x3c3ce191, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12265.bin",  0x10000, 0x8cdadd9a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },

	{ "12268.a14",      0x10000, 0xe0dac07f, SYS16_ROM_TILES | BRF_GRA },
	{ "12269.a15",      0x10000, 0x457a8790, SYS16_ROM_TILES | BRF_GRA },
	{ "12270.a16",      0x10000, 0x69fc025b, SYS16_ROM_TILES | BRF_GRA },
	
	{ "12276.b4",       0x10000, 0x838bd71f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12280.b8",       0x10000, 0x639a57cb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12277.b3",       0x10000, 0xf16b6ba2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12281.b7",       0x10000, 0x1ef1077f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12278.b2",       0x10000, 0x961d06b7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12282.b6",       0x10000, 0xe8671ee1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12279.b1",       0x10000, 0x7a169fb1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12283.b5",       0x10000, 0xae0fa085, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-12271.a7",   0x08000, 0x99671e52, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12272.a8",   0x08000, 0x7cf7e69f, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-12273.a9",   0x08000, 0x28f0bb8b, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-12274.a10",  0x08000, 0x8207f0c4, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "epr-12275.a11",  0x08000, 0x182f3c3d, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0099.bin",   0x01000, 0xf676e3e4, BRF_OPT },
};


STD_ROM_PICK(Tturfu)
STD_ROM_FN(Tturfu)

static struct BurnRomInfo Wb3RomDesc[] = {
	{ "epr-12259.a7",   0x20000, 0x54927c7e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12258.a5",   0x20000, 0x01f5898c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-12124.a14",  0x10000, 0xdacefb6f, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12125.a15",  0x10000, 0x9fc36df7, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12126.a16",  0x10000, 0xa693fd94, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12090.b1",   0x10000, 0xaeeecfca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12094.b5",   0x10000, 0x615e4927, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12091.b2",   0x10000, 0x8409a243, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12095.b6",   0x10000, 0xe774ec2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12092.b3",   0x10000, 0x5c2f0d90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12096.b7",   0x10000, 0x0cd59d6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12093.b4",   0x10000, 0x4891e7bb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12097.b8",   0x10000, 0xe645902c, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12127.a10",  0x08000, 0x0bb901bb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0098.bin",   0x01000, 0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(Wb3)
STD_ROM_FN(Wb3)

static struct BurnRomInfo Wb32RomDesc[] = {
	{ "epr-12100.a6",   0x10000, 0xf5ca4abc, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12098.a1",   0x10000, 0xd998e5e5, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12101.a5",   0x10000, 0x6146492b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12099.a2",   0x10000, 0x3e243b45, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr-12124.b9",   0x10000, 0xdacefb6f, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12125.b10",  0x10000, 0x9fc36df7, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12126.b11",  0x10000, 0xa693fd94, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12090.b1",   0x10000, 0xaeeecfca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12094.b5",   0x10000, 0x615e4927, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12091.b2",   0x10000, 0x8409a243, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12095.b6",   0x10000, 0xe774ec2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12092.b3",   0x10000, 0x5c2f0d90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12096.b7",   0x10000, 0x0cd59d6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12093.b4",   0x10000, 0x4891e7bb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12097.b8",   0x10000, 0xe645902c, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12127.a7",   0x08000, 0x0bb901bb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0085.key",   0x02000, 0x8150f38d, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Wb32)
STD_ROM_FN(Wb32)

static struct BurnRomInfo Wb33RomDesc[] = {
	{ "epr-12137.a7",   0x20000, 0x6f81238e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12136.a5",   0x20000, 0x4cf05003, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr-12124.a14",  0x10000, 0xdacefb6f, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12125.a15",  0x10000, 0x9fc36df7, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12126.a16",  0x10000, 0xa693fd94, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12090.b1",   0x10000, 0xaeeecfca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12094.b5",   0x10000, 0x615e4927, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12091.b2",   0x10000, 0x8409a243, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12095.b6",   0x10000, 0xe774ec2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12092.b3",   0x10000, 0x5c2f0d90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12096.b7",   0x10000, 0x0cd59d6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12093.b4",   0x10000, 0x4891e7bb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12097.b8",   0x10000, 0xe645902c, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12127.a10",  0x08000, 0x0bb901bb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0089.key",   0x02000, 0x597d30d3, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Wb33)
STD_ROM_FN(Wb33)

static struct BurnRomInfo Wb34RomDesc[] = {
	{ "epr-12131.a7",   0x20000, 0xb95ecf88, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12128.a5",   0x20000, 0xb711372b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr-12124.a14",  0x10000, 0xdacefb6f, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12125.a15",  0x10000, 0x9fc36df7, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-12126.a16",  0x10000, 0xa693fd94, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-12090.b1",   0x10000, 0xaeeecfca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12094.b5",   0x10000, 0x615e4927, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12091.b2",   0x10000, 0x8409a243, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12095.b6",   0x10000, 0xe774ec2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12092.b3",   0x10000, 0x5c2f0d90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12096.b7",   0x10000, 0x0cd59d6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12093.b4",   0x10000, 0x4891e7bb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-12097.b8",   0x10000, 0xe645902c, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12127.a10",  0x08000, 0x0bb901bb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "317-0087.key",   0x02000, 0x162cb531, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Wb34)
STD_ROM_FN(Wb34)

static struct BurnRomInfo Wb3bblRomDesc[] = {
	{ "wb3_03",         0x10000, 0x0019ab3b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "wb3_05",         0x10000, 0x196e17ee, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "wb3_02",         0x10000, 0xc87350cb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "wb3_04",         0x10000, 0x565d5035, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "wb3_14",         0x10000, 0xd3f20bca, SYS16_ROM_TILES | BRF_GRA },
	{ "wb3_15",         0x10000, 0x96ff9d52, SYS16_ROM_TILES | BRF_GRA },
	{ "wb3_16",         0x10000, 0xafaf0d31, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr12090.b1",    0x10000, 0xaeeecfca, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr12094.b5",    0x10000, 0x615e4927, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr12091.b2",    0x10000, 0x8409a243, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr12095.b6",    0x10000, 0xe774ec2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr12092.b3",    0x10000, 0x5c2f0d90, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr12096.b7",    0x10000, 0x0cd59d6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr12093.b4",    0x10000, 0x4891e7bb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr12097.b8",    0x10000, 0xe645902c, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr12127.a10",   0x08000, 0x0bb901bb, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Wb3bbl)
STD_ROM_FN(Wb3bbl)

static struct BurnRomInfo WrestwarRomDesc[] = {
	{ "epr-12372.a7",   0x20000, 0xeeaba126, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12371.a5",   0x20000, 0x6714600a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12146.a8",   0x20000, 0xb77ba665, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12144.a6",   0x20000, 0xddf075cb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12150.a14",  0x20000, 0x6a821ab9, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12151.a15",  0x20000, 0x2b1a0751, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12152.a16",  0x20000, 0xf6e190fe, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12153.b1",   0x20000, 0xffa7d368, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12157.b5",   0x20000, 0x8d7794c1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12154.b2",   0x20000, 0x0ed343f2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12158.b6",   0x20000, 0x99458d58, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12155.b3",   0x20000, 0x3087104d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12159.b7",   0x20000, 0xabcf9bed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12156.b4",   0x20000, 0x41b6068b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12160.b8",   0x20000, 0x97eac164, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12141.a1",   0x20000, 0x260311c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12161.b10",  0x20000, 0x35a4b1b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12142.a2",   0x10000, 0x12e38a5c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12162.b11",  0x10000, 0xfa06fd24, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12147.a10",  0x08000, 0xc3609607, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12148.a11",  0x20000, 0xfb9a7f29, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "mpr-12149.a12",  0x20000, 0xd6617b19, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0103.bin",   0x01000, 0xaa0710f5, BRF_OPT }
};


STD_ROM_PICK(Wrestwar)
STD_ROM_FN(Wrestwar)

static struct BurnRomInfo Wrestwar1RomDesc[] = {
	{ "epr-12145.a7",   0x20000, 0x2af51e2e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12143.a5",   0x20000, 0x4131e345, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12146.a8",   0x20000, 0xb77ba665, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12144.a6",   0x20000, 0xddf075cb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12150.a14",  0x20000, 0x6a821ab9, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12151.a15",  0x20000, 0x2b1a0751, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12152.a16",  0x20000, 0xf6e190fe, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12153.b1",   0x20000, 0xffa7d368, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12157.b5",   0x20000, 0x8d7794c1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12154.b2",   0x20000, 0x0ed343f2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12158.b6",   0x20000, 0x99458d58, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12155.b3",   0x20000, 0x3087104d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12159.b7",   0x20000, 0xabcf9bed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12156.b4",   0x20000, 0x41b6068b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12160.b8",   0x20000, 0x97eac164, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12141.a1",   0x20000, 0x260311c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12161.b10",  0x20000, 0x35a4b1b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12142.a2",   0x10000, 0x12e38a5c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12162.b11",  0x10000, 0xfa06fd24, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12147.a10",  0x08000, 0xc3609607, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12148.a11",  0x20000, 0xfb9a7f29, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "mpr-12149.a12",  0x20000, 0xd6617b19, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0090.key",   0x02000, 0xb7c24c4a, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Wrestwar1)
STD_ROM_FN(Wrestwar1)

static struct BurnRomInfo Wrestwar2RomDesc[] = {
	{ "epr-12370.a7",   0x20000, 0xcb5dbb76, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12369.a5",   0x20000, 0x6f47dd2f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12146.a8",   0x20000, 0xb77ba665, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-12144.a6",   0x20000, 0xddf075cb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12150.a14",  0x20000, 0x6a821ab9, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12151.a15",  0x20000, 0x2b1a0751, SYS16_ROM_TILES | BRF_GRA },
	{ "mpr-12152.a16",  0x20000, 0xf6e190fe, SYS16_ROM_TILES | BRF_GRA },
	
	{ "mpr-12153.b1",   0x20000, 0xffa7d368, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12157.b5",   0x20000, 0x8d7794c1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12154.b2",   0x20000, 0x0ed343f2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12158.b6",   0x20000, 0x99458d58, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12155.b3",   0x20000, 0x3087104d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-12159.b7",   0x20000, 0xabcf9bed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12156.b4",   0x20000, 0x41b6068b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12160.b8",   0x20000, 0x97eac164, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12141.a1",   0x20000, 0x260311c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12161.b10",  0x20000, 0x35a4b1b1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12142.a2",   0x10000, 0x12e38a5c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "opr-12162.b11",  0x10000, 0xfa06fd24, SYS16_ROM_SPRITES | BRF_GRA },

	{ "epr-12147.a10",  0x08000, 0xc3609607, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "mpr-12148.a11",  0x20000, 0xfb9a7f29, SYS16_ROM_UPD7759DATA | BRF_SND },
	{ "mpr-12149.a12",  0x20000, 0xd6617b19, SYS16_ROM_UPD7759DATA | BRF_SND },
	
	{ "317-0102.key",   0x02000, 0x28ba1bf0, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Wrestwar2)
STD_ROM_FN(Wrestwar2)

/*====================================================
Bootleg Z80 Handling
====================================================*/

UINT8 __fastcall BootlegZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x01: {
			return BurnYM2151ReadStatus();
		}
		
		case 0x40:
		case 0xc0: {
			return System16SoundLatch;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read Port -> %02X\n"), a);
#endif

	return 0;
}

void __fastcall BootlegZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	d &= 0xff;
	
	switch (a) {
		case 0x00: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0x01: {
			BurnYM2151WriteRegister(d);
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Write Port -> %02X, %02X\n"), a, d);
#endif
}

UINT8 __fastcall BootlegZ80Read(UINT16 a)
{
	switch (a) {
		case 0xe000:
		case 0xe800: {
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return System16SoundLatch;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read -> %04X\n"), a);
#endif

	return 0;
}

#if 0 && defined FBA_DEBUG
void __fastcall BootlegZ80Write(UINT16 a, UINT8 d)
{

	bprintf(PRINT_NORMAL, _T("Z80 Write -> %04X, %02X\n"), a, d);
}
#endif

void BootlegMapZ80()
{
	ZetMapArea(0x0000, 0x7fff, 0, System16Z80Rom);
	ZetMapArea(0x0000, 0x7fff, 2, System16Z80Rom);

	ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram);
	ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram);
	ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram);
	ZetMemEnd();
	
	ZetSetReadHandler(BootlegZ80Read);
#if 0 && defined FBA_DEBUG
	ZetSetWriteHandler(BootlegZ80Write);
#endif
	ZetSetInHandler(BootlegZ80PortRead);
	ZetSetOutHandler(BootlegZ80PortWrite);
}

/*====================================================
Memory Handlers
====================================================*/

UINT8 __fastcall System16BReadByte(UINT32 a)
{
	switch (a) {
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
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

void __fastcall System16BWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileByteWrite((a - 0x400000) ^ 1, d);
		return;
	}

	switch (a) {
		case 0xc40001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
		
		case 0xfe0007: {
			System16SoundLatch = d;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

void __fastcall System16BWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileWordWrite(a - 0x400000, d);
		return;
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

void __fastcall AliensynWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xc00007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

UINT8 __fastcall AtomicpReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			return 0xff - System16Input[1];
		}
		
		case 0xc41005: {
			return System16Dip[0];
		}
		
		case 0xc41007: {
			return System16Dip[1];
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

void __fastcall AtomicpWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileByteWrite((a - 0x400000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0x123407: {
			// ???
			return;
		}
		
		case 0x3f0001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f0003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x080000: {
			BurnYM2413Write(0, d);
			return;
		}
		
		case 0x080002: {
			BurnYM2413Write(1, d);
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

void __fastcall AurailWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xfc0001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0xfc0003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

UINT8 __fastcall BayrouteReadByte(UINT32 a)
{
	switch (a) {
		case 0x901001: {
			return 0xff - System16Input[0];
		}
		
		case 0x901003: {
			return 0xff - System16Input[1];
		}
		
		case 0x901007: {
			return 0xff - System16Input[2];
		}
		
		case 0x902001: {
			return System16Dip[0];
		}

		case 0x902003: {
			return System16Dip[1];
		}
	}

	return 0xff;
}

void __fastcall BayrouteWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x700000 && a <= 0x70ffff) {
		System16BTileByteWrite((a - 0x700000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0x900001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
		
		case 0xff0007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

void __fastcall BayrouteWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x700000 && a <= 0x70ffff) {
		System16BTileWordWrite(a - 0x700000, d);
		return;
	}
}

UINT8 __fastcall BulletReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			return 0xff - System16Input[1];
		}
		
		case 0xc41005: {
			return 0xff - System16Input[3];
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
	}

	return 0xff;
}

void __fastcall BulletWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xc00007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

UINT8 __fastcall CottonReadByte(UINT32 a)
{
	switch (a) {
		case 0x601001: {
			return 0xff - System16Input[0];
		}
		
		case 0x601003: {
			return 0xff - System16Input[1];
		}
		
		case 0x601005: {
			return 0xff;
		}
		
		case 0x601007: {
			return 0xff - System16Input[2];
		}
		
		case 0x602001: {
			return System16Dip[0];
		}

		case 0x602003: {
			return System16Dip[1];
		}
		
		case 0x7038f7: {
			return (System16Rom[SekGetPC(0) + 1] << 8) | System16Rom[SekGetPC(0) + 0];
		}
	}
	
	return 0xff;
}

void __fastcall CottonWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileByteWrite((a - 0x400000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0x100001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x100003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	
		case 0x600001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
		
		case 0xff0007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

void __fastcall CottonWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileWordWrite(a - 0x400000, d);
		return;
	}
}

void __fastcall DduxWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x3f0001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f0003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

void __fastcall DduxblGfxWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xc46021: {
			BootlegFgPage[1] = d & 0xf;
			BootlegBgPage[1] = (d >> 4) & 0xf;
			return;
		}
		
		case 0xc46023: {
			BootlegFgPage[0] = d & 0xf;
			BootlegBgPage[0] = (d >> 4) & 0xf;
			return;
		}
		
		case 0xc46025: {
			BootlegFgPage[3] = d & 0xf;
			BootlegBgPage[3] = (d >> 4) & 0xf;
			return;
		}
		
		case 0xc46027: {
			BootlegFgPage[2] = d & 0xf;
			BootlegBgPage[2] = (d >> 4) & 0xf;
			return;
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

void __fastcall DduxblGfxWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xc46000: {
			System16ScrollY[1] = d;
			return;
		}
		
		case 0xc46008: {
			System16ScrollX[1] = (d ^ 0xffff) & 0x1ff;
			return;
		}
		
		case 0xc46010: {
			System16ScrollY[0] = d;
			return;
		}
		
		case 0xc46018: {
			System16ScrollX[0] = (d ^ 0xffff) & 0x1ff;
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

void __fastcall DduxblWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xc40001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
		
		case 0xc40007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

static INT16 DunkshotTrack1X = 0;
static INT16 DunkshotTrack1Y = 0;
static INT16 DunkshotTrack2X = 0;
static INT16 DunkshotTrack2Y = 0;
static INT16 DunkshotTrack3X = 0;
static INT16 DunkshotTrack3Y = 0;
static INT16 DunkshotTrack4X = 0;
static INT16 DunkshotTrack4Y = 0;

void DunkshotMakeAnalogInputs()
{
	if (System16InputPort2[0]) DunkshotTrack1X += 0x40;
	if (System16InputPort2[1]) DunkshotTrack1X -= 0x40;
	if (DunkshotTrack1X >= 0x1000) DunkshotTrack1X = 0;
	if (DunkshotTrack1X < 0) DunkshotTrack1X = 0xfc0;
	
	if (System16InputPort2[2]) DunkshotTrack1Y -= 0x40;
	if (System16InputPort2[3]) DunkshotTrack1Y += 0x40;
	if (DunkshotTrack1Y >= 0x1000) DunkshotTrack1Y = 0;
	if (DunkshotTrack1Y < 0) DunkshotTrack1Y = 0xfc0;
	
	if (System16InputPort2[4]) DunkshotTrack2X += 0x40;
	if (System16InputPort2[5]) DunkshotTrack2X -= 0x40;
	if (DunkshotTrack2X >= 0x1000) DunkshotTrack2X = 0;
	if (DunkshotTrack2X < 0) DunkshotTrack2X = 0xfc0;
	
	if (System16InputPort2[6]) DunkshotTrack2Y -= 0x40;
	if (System16InputPort2[7]) DunkshotTrack2Y += 0x40;
	if (DunkshotTrack2Y >= 0x1000) DunkshotTrack2Y = 0;
	if (DunkshotTrack2Y < 0) DunkshotTrack2Y = 0xfc0;
	
	if (System16InputPort3[0]) DunkshotTrack3X += 0x40;
	if (System16InputPort3[1]) DunkshotTrack3X -= 0x40;
	if (DunkshotTrack3X >= 0x1000) DunkshotTrack3X = 0;
	if (DunkshotTrack3X < 0) DunkshotTrack3X = 0xfc0;
	
	if (System16InputPort3[2]) DunkshotTrack3Y -= 0x40;
	if (System16InputPort3[3]) DunkshotTrack3Y += 0x40;
	if (DunkshotTrack3Y >= 0x1000) DunkshotTrack3Y = 0;
	if (DunkshotTrack3Y < 0) DunkshotTrack3Y = 0xfc0;

	if (System16InputPort3[4]) DunkshotTrack4X += 0x40;
	if (System16InputPort3[5]) DunkshotTrack4X -= 0x40;
	if (DunkshotTrack4X >= 0x1000) DunkshotTrack4X = 0;
	if (DunkshotTrack4X < 0) DunkshotTrack4X = 0xfc0;
	
	if (System16InputPort3[6]) DunkshotTrack4Y -= 0x40;
	if (System16InputPort3[7]) DunkshotTrack4Y += 0x40;
	if (DunkshotTrack4Y >= 0x1000) DunkshotTrack4Y = 0;
	if (DunkshotTrack4Y < 0) DunkshotTrack4Y = 0xfc0;
}

UINT8 __fastcall DunkshotReadByte(UINT32 a)
{
	switch (a) {
		case 0xc43001: {
			return DunkshotTrack1X & 0xff;
		}

		case 0xc43003: {
			return DunkshotTrack1X >> 8;
		}
		
		case 0xc43005: {
			return DunkshotTrack1Y & 0xff;
		}

		case 0xc43007: {
			return DunkshotTrack1Y >> 8;
		}
		
		case 0xc43009: {
			return DunkshotTrack2X & 0xff;
		}

		case 0xc4300b: {
			return DunkshotTrack2X >> 8;
		}
		
		case 0xc4300d: {
			return DunkshotTrack2Y & 0xff;
		}

		case 0xc4300f: {
			return DunkshotTrack2Y >> 8;
		}
		
		case 0xc43011: {
			return DunkshotTrack3X & 0xff;
		}

		case 0xc43013: {
			return DunkshotTrack3X >> 8;
		}
		
		case 0xc43015: {
			return DunkshotTrack3Y & 0xff;
		}

		case 0xc43017: {
			return DunkshotTrack3Y >> 8;
		}
		
		case 0xc43019: {
			return DunkshotTrack4X & 0xff;
		}

		case 0xc4301b: {
			return DunkshotTrack4X >> 8;
		}
		
		case 0xc4301d: {
			return DunkshotTrack4Y & 0xff;
		}

		case 0xc4301f: {
			return DunkshotTrack4Y >> 8;
		}
	}

	return 0xff;
}

UINT16 __fastcall EswatMultiply0ReadWord(UINT32 a)
{
	return System16MultiplyChipRead(0, (a - 0x3e0000) >> 1);
}

void __fastcall EswatMultiply0WriteWord(UINT32 a, UINT16 d)
{
	System16MultiplyChipWrite(0, (a - 0x3e0000) >> 1, d);
}

UINT16 __fastcall EswatCompare0ReadWord(UINT32 a)
{
	return System16CompareTimerChipRead(0, (a - 0x3e1000) >> 1);
}

void __fastcall EswatCompare0WriteWord(UINT32 a, UINT16 d)
{
	System16CompareTimerChipWrite(0, (a - 0x3e1000) >> 1, d);
}

void __fastcall EswatSoundWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x123407: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

void __fastcall EswatWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x3e2001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3e2003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

void __fastcall EswatblSoundWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xc42007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

void __fastcall EswatblGfxWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x418031: {
			System16TileBanks[1] = d & 7;
			return;
		}
	}
}

void __fastcall EswatblGfxWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x418000: {
			System16ScrollY[0] = d;
			return;
		}
	
		case 0x418008: {
			System16ScrollX[0] = (d ^ 0xffff);
			return;
		}
		
		case 0x418010: {
			System16ScrollY[1] = d;
			return;
		}
		
		case 0x418018: {
			System16ScrollX[1] = (d ^ 0xffff);
			return;
		}
		
		case 0x418020: {
			BootlegFgPage[3] = (d >> 12) & 0xf;
			BootlegFgPage[2] = (d >> 8) & 0xf;
			BootlegFgPage[1] = (d >> 4) & 0xf;
			BootlegFgPage[0] = (d >> 0) & 0xf;
			return;
		}
		
		case 0x418028: {
			BootlegBgPage[3] = (d >> 12) & 0xf;
			BootlegBgPage[2] = (d >> 8) & 0xf;
			BootlegBgPage[1] = (d >> 4) & 0xf;
			BootlegBgPage[0] = (d >> 0) & 0xf;
			return;
		}
	}
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

static INT16 ExctleagTrack1X = 0;
static INT16 ExctleagTrack1Y = 0;
static INT16 ExctleagTrack2X = 0;
static INT16 ExctleagTrack2Y = 0;

void ExctleagMakeAnalogInputs()
{
	if (System16InputPort4[0]) ExctleagTrack1X += 0x1;
	if (System16InputPort4[1]) ExctleagTrack1X -= 0x1;
	if (ExctleagTrack1X >= 0x100) ExctleagTrack1X = 0;
	if (ExctleagTrack1X < 0) ExctleagTrack1X = 0xff;
	
	if (System16InputPort4[2]) ExctleagTrack1Y -= 0x1;
	if (System16InputPort4[3]) ExctleagTrack1Y += 0x1;
	if (ExctleagTrack1Y >= 0x100) ExctleagTrack1Y = 0;
	if (ExctleagTrack1Y < 0) ExctleagTrack1Y = 0xff;
	
	if (System16InputPort4[4]) ExctleagTrack2X += 0x4;
	if (System16InputPort4[5]) ExctleagTrack2X -= 0x4;
	if (ExctleagTrack2X >= 0x100) ExctleagTrack2X = 0;
	if (ExctleagTrack2X < 0) ExctleagTrack2X = 0xfc;
	
	if (System16InputPort4[6]) ExctleagTrack2Y -= 0x4;
	if (System16InputPort4[7]) ExctleagTrack2Y += 0x4;
	if (ExctleagTrack2Y >= 0x100) ExctleagTrack2Y = 0;
	if (ExctleagTrack2Y < 0) ExctleagTrack2Y = 0xfc;
}

UINT8 __fastcall ExctleagReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			return 0xff - System16Input[1];
		}
		
		case 0xc41005: {
			return 0xff - System16Input[3];
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
		
		case 0xc43001:
		case 0xc43003: {
			return ExctleagTrack1X;
		}
		
		case 0xc43005:
		case 0xc43007: {
			return ExctleagTrack1Y;
		}
		
		case 0xc43009:
		case 0xc4300b: {
			return ExctleagTrack2X;
		}
		
		case 0xc4300d:
		case 0xc4300f: {
			return ExctleagTrack2Y;
		}
	}

	return 0xff;
}

void __fastcall Fantzn2xWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x3f0001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f0003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

UINT8 __fastcall FpointblReadByte(UINT32 a)
{
	switch (a) {
		case 0x601001: {
			return 0xff - System16Input[0];
		}
		
		case 0x601003: {
			return 0xff - System16Input[1];
		}
		
		case 0x601005: {
			return 0xff - System16Input[2];
		}
		
		case 0x600001: {
			return System16Dip[0];
		}

		case 0x600003: {
			return System16Dip[1];
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

void __fastcall FpointblWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x600007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;

		}
		
		case 0x843001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

void __fastcall FpointblGfxWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xc46000: {
			System16ScrollY[0] = d;
			return;
		}
	
		case 0xc46008: {
			System16ScrollX[0] = (d ^ 0xffff);
			return;
		}
		
		case 0xc46010: {
			System16ScrollY[1] = d + 2;
			return;
		}
		
		case 0xc46018: {
			System16ScrollX[1] = (d ^ 0xffff);
			return;
		}
		
		case 0xc46022: {
			BootlegFgPage[3] = (d >> 12) & 0xf;
			BootlegFgPage[2] = (d >> 8) & 0xf;
			BootlegFgPage[1] = (d >> 4) & 0xf;
			BootlegFgPage[0] = (d >> 0) & 0xf;
			return;
		}
		
		case 0xc46026: {
			BootlegBgPage[0] = (d >> 12) & 0xf;
			BootlegBgPage[1] = (d >> 8) & 0xf;
			BootlegBgPage[2] = (d >> 4) & 0xf;
			BootlegBgPage[3] = (d >> 0) & 0xf;			
			return;
		}
	}
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X, 0x%04X\n"), a, d, d ^ 0xffff);
#endif
}

void __fastcall GoldnaxeTileWriteByte(UINT32 a, UINT8 d)
{
	System16BTileByteWrite((a - 0x100000) ^ 1, d);
}

void __fastcall GoldnaxeTileWriteWord(UINT32 a, UINT16 d)
{
	System16BTileWordWrite(a - 0x100000, d);
}

UINT16 __fastcall GoldnaxeMultiply0ReadWord(UINT32 a)
{
	return System16MultiplyChipRead(0, (a - 0x1f0000) >> 1);
}

void __fastcall GoldnaxeMultiply0WriteWord(UINT32 a, UINT16 d)
{
	System16MultiplyChipWrite(0, (a - 0x1f0000) >> 1, d);
}

UINT16 __fastcall GoldnaxeCompare0ReadWord(UINT32 a)
{
	return System16CompareTimerChipRead(0, (a - 0x1f1000) >> 1);
}

void __fastcall GoldnaxeCompare0WriteWord(UINT32 a, UINT16 d)
{
	System16CompareTimerChipWrite(0, (a - 0x1f1000) >> 1, d);
}

UINT16 __fastcall GoldnaxeCompare1ReadWord(UINT32 a)
{
	return System16CompareTimerChipRead(1, (a - 0x1e0000) >> 1);
}

void __fastcall GoldnaxeCompare1WriteWord(UINT32 a, UINT16 d)
{
	System16CompareTimerChipWrite(1, (a - 0x1e0000) >> 1, d);
}

void __fastcall Goldnaxe1WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x1f2001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x1f2003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

void __fastcall Goldnaxe3WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x1f0001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x1f0003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

static UINT8 HwchampInputVal;

UINT16 __fastcall HwchampCtrlReadWord(UINT32 a)
{
	UINT16 result;
	
	switch (a) {
		case 0xc43020: 
		case 0xc43022: 
		case 0xc43024: {
			result = (HwchampInputVal & 0x80) >> 7;
			HwchampInputVal <<= 1;
			return result;
		}
	}

	return 0xffff;
}

void __fastcall HwchampCtrlWriteWord(UINT32 a, UINT16 /*d*/)
{
	UINT8 temp = 0;
	
	switch (a) {
		case 0xc43020: {
			temp = 0x80 + (System16AnalogPort0 >> 4);
			if (temp < 0x01) temp = 0x01;
			if (temp > 0xfe) temp = 0xfe;
			HwchampInputVal = temp;
			return;
		}
		
		case 0xc43022: {
			temp = 0x26;
			if (System16AnalogPort2 > 1) temp = 0xfe;
			HwchampInputVal = temp;
			return;
		}
		
		case 0xc43024: {
			temp = 0x26;
			if (System16AnalogPort1 > 1) temp = 0xfe;
			HwchampInputVal = temp;
			return;
		}
	}
}

void __fastcall HwchampWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x3f0001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f0003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

void __fastcall MvpWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x3f2001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f2003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

void __fastcall MvpjWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x3f0001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f0003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

void __fastcall PassshtWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xc00007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

UINT8 __fastcall PassshtaReadByte(UINT32 a)
{
	switch (a) {
		case 0xc43001: {
			return 0xff - System16Input[1];
		}
		
		case 0xc43003: {
			return 0xff - System16Input[2];
		}
		
		case 0xc43005: {
			return 0xff - System16Input[3];
		}
		
		case 0xc43007: {
			return 0xff - System16Input[4];
		}
	}

	return 0xff;
}

void __fastcall PassshtbGfxWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xc46000: {
			System16ScrollY[0] = d;
			return;
		}
	
		case 0xc46002: {
			System16ScrollX[0] = (d ^ 0xffff) & 0x1ff;
			return;
		}
		
		case 0xc46004: {
			System16ScrollY[1] = d;
			return;
		}
		
		case 0xc46006: {
			System16ScrollX[1] = (d ^ 0xffff) & 0x1ff;
			return;
		}
		
/*		case 0xc46022: {
			BootlegFgPage[3] = (d >> 12) & 0xf;
			BootlegFgPage[2] = (d >> 8) & 0xf;
			BootlegFgPage[1] = (d >> 4) & 0xf;
			BootlegFgPage[0] = (d >> 0) & 0xf;
			return;
		}
		
		case 0xc46026: {
			BootlegBgPage[0] = (d >> 12) & 0xf;
			BootlegBgPage[1] = (d >> 8) & 0xf;
			BootlegBgPage[2] = (d >> 4) & 0xf;
			BootlegBgPage[3] = (d >> 0) & 0xf;			
			return;
		}*/
	}
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X, 0x%04X\n"), a, d, (d ^ 0xffff) & 0x1ff);
#endif
}

UINT8 __fastcall RiotcityReadByte(UINT32 a)
{
	switch (a) {
		case 0xf81001: {
			return 0xff - System16Input[0];
		}
		
		case 0xf81003: {
			return 0xff - System16Input[1];
		}
		
		case 0xf81007: {
			return 0xff - System16Input[2];
		}
		
		case 0xf82001: {
			return System16Dip[0];
		}

		case 0xf82003: {
			return System16Dip[1];
		}
	}

	return 0xff;
}

void __fastcall RiotcityWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0xfa0000 && a <= 0xfaffff) {
		System16BTileByteWrite((a - 0xfa0000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0xf00007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
		
		case 0xf20001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0xf20003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}

		case 0xf80001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
	}
}

void __fastcall RiotcityWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0xfa0000 && a <= 0xfaffff) {
		System16BTileWordWrite(a - 0xfa0000, d);
		return;
	}
}

UINT16 __fastcall RyukyuReadWord(UINT32 a)
{
	switch (a) {
		case 0x601000: {
			return 0xff - System16Input[0];
		}
		
		case 0x601002: {
			return 0xff - System16Input[1];
		}
		
		case 0x602000: {
			return System16Dip[0];
		}
		
		case 0x602002: {
			return System16Dip[1];
		}
	}
	
	return 0xffff;
}

void __fastcall RyukyuWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileWordWrite(a - 0x400000, d);
		return;
	}
	
	switch (a) {
		case 0x100000: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x100002: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x600000: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
	}
}

void __fastcall RyukyuWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xff0007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

static INT16 SdiTrack1X = 0;
static INT16 SdiTrack1Y = 0;
static INT16 SdiTrack2X = 0;
static INT16 SdiTrack2Y = 0;

void SdibMakeAnalogInputs()
{
	SdiTrack1X -= (System16AnalogPort0 >> 8) & 0xff;
	SdiTrack1Y += (System16AnalogPort1 >> 8) & 0xff;
	
	SdiTrack2X -= (System16AnalogPort2 >> 8) & 0xff;
	SdiTrack2Y += (System16AnalogPort3 >> 8) & 0xff;
}

UINT8 __fastcall SdibReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41005: {
			return 0xff - System16Input[1];
		}
		
		case 0xc42003: {
			return System16Dip[1];
		}
		
		case 0xc42005: {
			return System16Dip[0];
		}
		
		case 0xc43001: {
			return SdiTrack1X;
		}
		
		case 0xc43005: {
			return SdiTrack1Y;
		}
		
		case 0xc43009: {
			return SdiTrack2X;
		}
		
		case 0xc4300d: {
			return SdiTrack2Y;
		}
	}
	
	return 0xff;
}

void __fastcall SdibSoundWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x123407: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

static UINT8 MahjongInputNum;

UINT16 __fastcall SjryukoReadWord(UINT32 a)
{
	SEK_DEF_READ_WORD(0, a);
	
	return 0xffff;
}

UINT8 __fastcall SjryukoReadByte(UINT32 a)
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

void __fastcall SjryukoWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xc40003: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			if (d & 4) MahjongInputNum = (MahjongInputNum + 1) % 6;
			return;
		}
		
		case 0xc00007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

UINT16 __fastcall SonicbomReadWord(UINT32 a)
{
	switch (a) {
		case 0xc42000: {
			return (0xff << 8) | System16Dip[0];
		}
		
		case 0xc42002: {
			return (0xff << 8) | System16Dip[1];
		}
	}
	
	return 0xffff;
}

void __fastcall SonicbomWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x123407: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

void __fastcall TetrisblGfxWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x418000: {
			System16ScrollY[0] = d;
			return;
		}
	
		case 0x418008: {
			System16ScrollX[0] = (d ^ 0xffff) & 0x3ff;
			return;
		}
		
		case 0x418010: {
			System16ScrollY[1] = d;
			return;
		}
		
		case 0x418018: {
			System16ScrollX[1] = ((d ^ 0xffff) & 0x3ff) + 2;
			return;
		}
		
		case 0x418020: {
			BootlegFgPage[3] = (d >> 12) & 0xf;
			BootlegFgPage[2] = (d >> 8) & 0xf;
			BootlegFgPage[1] = (d >> 4) & 0xf;
			BootlegFgPage[0] = (d >> 0) & 0xf;
			return;
		}
		
		case 0x418028: {
			BootlegBgPage[0] = (d >> 12) & 0xf;
			BootlegBgPage[1] = (d >> 8) & 0xf;
			BootlegBgPage[2] = (d >> 4) & 0xf;
			BootlegBgPage[3] = (d >> 0) & 0xf;
			return;
		}
	}
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X, 0x%04X\n"), a, d, (d ^ 0xffff) & 0x3ff);
#endif
}

void __fastcall TetrisblSndWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xc42007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

UINT8 __fastcall ToryumonReadByte(UINT32 a)
{
	switch (a) {
		case 0xe41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xe41003: {
			return 0xff - System16Input[1];
		}
		
		case 0xe41005: {
			return 0;
		}
		
		case 0xe41007: {
			return 0xff - System16Input[2];
		}
		
		case 0xe42001: {
			return System16Dip[0];
		}

		case 0xe42003: {
			return System16Dip[1];
		}
	}

	return 0xff;
}

void __fastcall ToryumonWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x3e2001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3e2003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0xe40001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
		
		case 0xfe0007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

UINT8 __fastcall TturfReadByte(UINT32 a)
{
	switch (a) {
		case 0x602001: {
			return System16Dip[0];
		}

		case 0x602003: {
			return System16Dip[1];
		}
	}

	return 0xff;
}

void __fastcall TturfWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileByteWrite((a - 0x400000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0x600001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
	}
}

void __fastcall Wb3WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xdf0007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
	}
}

void __fastcall Wb3bblGfxWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xc46000: {
			System16ScrollY[0] = d;
			return;
		}
	
		case 0xc46002: {
			System16ScrollX[0] = (d ^ 0xffff) & 0x3ff;
			return;
		}
		
		case 0xc46004: {
			System16ScrollY[1] = d;
			return;
		}
		
		case 0xc46006: {
			System16ScrollX[1] = (d ^ 0xffff) & 0x3ff;
			return;
		}
		
/*		case 0xc46022: {
			BootlegFgPage[3] = (d >> 12) & 0xf;
			BootlegFgPage[2] = (d >> 8) & 0xf;
			BootlegFgPage[1] = (d >> 4) & 0xf;
			BootlegFgPage[0] = (d >> 0) & 0xf;
			return;
		}
		
		case 0xc46026: {
			BootlegBgPage[0] = (d >> 12) & 0xf;
			BootlegBgPage[1] = (d >> 8) & 0xf;
			BootlegBgPage[2] = (d >> 4) & 0xf;
			BootlegBgPage[3] = (d >> 0) & 0xf;			
			return;
		}*/
	}
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X, 0x%04X\n"), a, d, d ^ 0xffff);
#endif
}

void __fastcall WrestwarTileWriteByte(UINT32 a, UINT8 d)
{
	System16BTileByteWrite((a - 0x100000) ^ 1, d);
}

void __fastcall WrestwarTileWriteWord(UINT32 a, UINT16 d)
{
	System16BTileWordWrite(a - 0x100000, d);
}

void __fastcall WrestwarWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x400001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x400003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
	}
}

/*====================================================
Driver Inits
====================================================*/

static INT32 Fantzn2xPlaneOffsets[3] = { 1, 2, 3 };
static INT32 Fantzn2xXOffsets[8]     = { 0, 4, 8, 12, 16, 20, 24, 28 };
static INT32 Fantzn2xYOffsets[8]     = { 0, 32, 64, 96, 128, 160, 192, 224 };

static INT32 AliensynInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xc00006, 0xc00007, SM_WRITE);
		SekSetWriteByteHandler(1, AliensynWriteByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 Aliensyn3Init()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xc00006, 0xc00007, SM_WRITE);
		SekSetWriteByteHandler(1, AliensynWriteByte);
		SekClose();
	}
	
	return nRet;
}

void Altbeast_Sim8751()
{
	// System Inputs
	*((UINT16*)(System16Ram + 0x30c2)) = BURN_ENDIAN_SWAP_INT16((UINT16)(System16Input[0] << 8));
	
	// Tile Banking
	System16TileBanks[1] = ((System16Ram[0x3094 + 1] << 8) | System16Ram[0x3094 + 0]) & 7;
	
	// Sound command
	UINT16 temp = (System16Ram[0x30c4 + 1] << 8) | System16Ram[0x30c4 + 0];
	if ((temp & 0xff00) != 0x0000) {
		System16SoundLatch = temp >> 8;
		ZetOpen(0);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		ZetClose();
		*((UINT16*)(System16Ram + 0x30c4)) = BURN_ENDIAN_SWAP_INT16((UINT16)(temp & 0xff));
	}
}

void Altbeastj_Sim8751()
{
	// System Inputs
	*((UINT16*)(System16Ram + 0x30d0)) = BURN_ENDIAN_SWAP_INT16((UINT16)(System16Input[0] << 8));
	
	// Tile Banking
	System16TileBanks[1] = ((System16Ram[0x3094 + 1] << 8) | System16Ram[0x3094 + 0]) & 7;
	
	// Sound command
	UINT16 temp = (System16Ram[0x30d4 + 1] << 8) | System16Ram[0x30d4 + 0];
	if ((temp & 0xff00) != 0x0000) {
		System16SoundLatch = temp >> 8;
		ZetOpen(0);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		ZetClose();
		*((UINT16*)(System16Ram + 0x30d4)) = BURN_ENDIAN_SWAP_INT16((UINT16)(temp & 0xff));
	}
}

void Altbeast6_Sim8751()
{
	// System Inputs
	*((UINT16*)(System16Ram + 0x3096)) = BURN_ENDIAN_SWAP_INT16((UINT16)(System16Input[0] << 8));
	
	// Tile Banking
	System16TileBanks[1] = ((System16Ram[0x3094 + 1] << 8) | System16Ram[0x3094 + 0]) & 7;
	
	// Sound command
	UINT16 temp = (System16Ram[0x3098 + 1] << 8) | System16Ram[0x3098 + 0];
	if ((temp & 0xff00) != 0x0000) {
		System16SoundLatch = temp >> 8;
		ZetOpen(0);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		ZetClose();
		*((UINT16*)(System16Ram + 0x3098)) = BURN_ENDIAN_SWAP_INT16((UINT16)(temp & 0xff));
	}
}

static INT32 AltbeastInit()
{
	Simulate8751 = Altbeast_Sim8751;

	return System16Init();
}

static INT32 AltbeastjInit()
{
	Simulate8751 = Altbeastj_Sim8751;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1a0000 - 0xe0000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0xe0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0xe0000);
			memset(System16Sprites, 0, 0x1a0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x40000, 0x20000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x60000, 0x20000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x80000, 0x20000);
			memcpy(System16Sprites + 0x140000, pTemp + 0xa0000, 0x20000);
			memcpy(System16Sprites + 0x180000, pTemp + 0xc0000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Altbeast6Init()
{
	Simulate8751 = Altbeast6_Sim8751;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1a0000 - 0xe0000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0xe0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0xe0000);
			memset(System16Sprites, 0, 0x1a0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x40000, 0x20000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x60000, 0x20000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x80000, 0x20000);
			memcpy(System16Sprites + 0x140000, pTemp + 0xa0000, 0x20000);
			memcpy(System16Sprites + 0x180000, pTemp + 0xc0000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Altbeastj3Init()
{
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1a0000 - 0xe0000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0xe0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0xe0000);
			memset(System16Sprites, 0, 0x1a0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x40000, 0x20000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x60000, 0x20000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x80000, 0x20000);
			memcpy(System16Sprites + 0x140000, pTemp + 0xa0000, 0x20000);
			memcpy(System16Sprites + 0x180000, pTemp + 0xc0000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Altbeast4Init()
{
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1a0000 - 0xe0000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0xe0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0xe0000);
			memset(System16Sprites, 0, 0x1a0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x40000, 0x20000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x60000, 0x20000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x80000, 0x20000);
			memcpy(System16Sprites + 0x140000, pTemp + 0xa0000, 0x20000);
			memcpy(System16Sprites + 0x180000, pTemp + 0xc0000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

void AtomicpMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x01ffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x01ffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x840000, 0x840fff, SM_RAM);
	SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0, System16BWriteWord);
	SekSetReadByteHandler(0, AtomicpReadByte);
	SekSetWriteByteHandler(0, AtomicpWriteByte);
	SekClose();
}

static INT32 AtomicpInit()
{
	System16Map68KDo = AtomicpMap68K;
	
	INT32 nRet = System16Init();
	
	System16IgnoreVideoEnable = 1;
	System16YM2413IRQInterval = 166;
	
	return nRet;
}

static INT32 AurailInit()
{
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xfc0000, 0xfc0003, SM_WRITE);
		SekSetWriteByteHandler(1, AurailWriteByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 Aurail1Init()
{
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xfc0000, 0xfc0003, SM_WRITE);
		SekSetWriteByteHandler(1, AurailWriteByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 AurailjInit()
{
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xfc0000, 0xfc0003, SM_WRITE);
		SekSetWriteByteHandler(1, AurailWriteByte);
		SekClose();
	}
	
	return nRet;
}

void BayrouteMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0bffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0bffff, SM_FETCH);
	SekMapMemory(System16Ram           , 0x500000, 0x503fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x600000, 0x6007ff, SM_RAM);
	SekMapMemory(System16TileRam       , 0x700000, 0x70ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x710000, 0x710fff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x800000, 0x800fff, SM_RAM);
	SekSetReadByteHandler(0, BayrouteReadByte);
	SekSetWriteByteHandler(0, BayrouteWriteByte);
	SekSetWriteWordHandler(0, BayrouteWriteWord);
	SekClose();
}

static INT32 BayrouteInit()
{
	System16Map68KDo = BayrouteMap68K;
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	return System16Init();
}

static INT32 Bayroute1Init()
{
	System16Map68KDo = BayrouteMap68K;
	System16CustomLoadRomDo = CustomLoadRom20000;
	
	return System16Init();
}

void Blox16bMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x440000, 0x4407ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x840000, 0x840fff, SM_RAM);
	SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0, System16BWriteWord);
	SekSetReadByteHandler(0, System16BReadByte);
	SekSetWriteByteHandler(0, System16BWriteByte);
	SekMapHandler(1, 0x3f0000, 0x3f0003, SM_WRITE);
	SekSetWriteByteHandler(1, Fantzn2xWriteByte);
	SekClose();
}

static INT32 Blox16bLoadRom()
{
	if (BurnLoadRom(System16Rom + 0x00000, 0, 1)) return 1;
	
	System16TempGfx = (UINT8*)BurnMalloc(System16TileRomSize);
	BurnLoadRom(System16TempGfx, 1, 1);
	GfxDecode(0x2000, 3, 8, 8, Fantzn2xPlaneOffsets, Fantzn2xXOffsets, Fantzn2xYOffsets, 0x100, System16TempGfx, System16Tiles);
	System16NumTiles = 0x2000;
	BurnFree(System16TempGfx);
	
	BurnLoadRom(System16Sprites, 2, 1);
	BurnByteswap(System16Sprites, System16SpriteRomSize);
	
	if (BurnLoadRom(System16Z80Rom, 3, 1)) return 1;
	memcpy(System16UPD7759Data, System16Z80Rom + 0x10000, 0x08000);
		
	return 0;
}

static INT32 Blox16bInit()
{
	System16Map68KDo = Blox16bMap68K;
	System16CustomLoadRomDo = Blox16bLoadRom;
	System16UPD7759DataSize = 0x08000;
	
	return System16Init();
}

static INT32 BulletInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, BulletReadByte);
		SekMapHandler(1, 0xc00006, 0xc00007, SM_WRITE);
		SekSetWriteByteHandler(1, BulletWriteByte);
		SekClose();
	}
	return nRet;
}

void CottonMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0fffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0fffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x300000, 0x3007ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x500000, 0x500fff, SM_RAM);
	SekMapMemory(System16Ram           , 0x200000, 0x203fff, SM_RAM);
	SekSetReadByteHandler(0, CottonReadByte);
	SekSetWriteByteHandler(0, CottonWriteByte);
	SekSetWriteWordHandler(0, CottonWriteWord);
	SekClose();
}

static INT32 CottonInit()
{
	System16Map68KDo = CottonMap68K;
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	return System16Init();
}

void Ddux_Sim8751()
{
	// Sound command
	UINT16 temp = (System16Ram[0x0bd0 + 1] << 8) | System16Ram[0x0bd0 + 0];
	if ((temp & 0xff00) != 0x0000) {
		System16SoundLatch = temp >> 8;
		ZetOpen(0);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		ZetClose();
		*((UINT16*)(System16Ram + 0x0bd0)) = BURN_ENDIAN_SWAP_INT16((UINT16)(temp & 0xff));
	}
}

static INT32 DduxInit()
{
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x3f0000, 0x3f0003, SM_WRITE);
		SekSetWriteByteHandler(1, DduxWriteByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 Ddux1Init()
{
	Simulate8751 = Ddux_Sim8751;
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x3f0000, 0x3f0003, SM_WRITE);
		SekSetWriteByteHandler(1, DduxWriteByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 DduxblInit()
{
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	System16SpriteXOffset = 112;
	
	System16MapZ80Do = BootlegMapZ80;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetWriteByteHandler(0, DduxblWriteByte);
		SekMapHandler(1, 0x3f0000, 0x3f0003, SM_WRITE);
		SekSetWriteByteHandler(1, DduxWriteByte);
		SekMapHandler(2, 0xc46000, 0xc46027, SM_WRITE);
		SekSetWriteByteHandler(2, DduxblGfxWriteByte);
		SekSetWriteWordHandler(2, DduxblGfxWriteWord);
		SekClose();
		
		bSystem16BootlegRender = true;
	}
	
	return nRet;
}

static INT32 DunkshotInit()
{
	System16MakeAnalogInputsDo = DunkshotMakeAnalogInputs;
	
	System16BTileAlt = true;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x80000 - 0x40000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xc43000, 0xc4301f, SM_READ);
		SekSetReadByteHandler(1, DunkshotReadByte);
		SekClose();
		
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x80000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x80000);
			memset(System16Sprites, 0, 0x80000);
			memcpy(System16Sprites + 0x00000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x10000, pTemp + 0x00000, 0x10000);
			memcpy(System16Sprites + 0x20000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x30000, pTemp + 0x10000, 0x10000);
			memcpy(System16Sprites + 0x40000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x50000, pTemp + 0x20000, 0x10000);
			memcpy(System16Sprites + 0x60000, pTemp + 0x30000, 0x10000);
			memcpy(System16Sprites + 0x70000, pTemp + 0x30000, 0x10000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 DunkshotExit()
{
	DunkshotTrack1X = 0;
	DunkshotTrack1Y = 0;
	DunkshotTrack2X = 0;
	DunkshotTrack2Y = 0;
	DunkshotTrack3X = 0;
	DunkshotTrack3Y = 0;
	DunkshotTrack4X = 0;
	DunkshotTrack4Y = 0;

	return System16Exit();
}

static INT32 DunkshotScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin =  0x029660;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(DunkshotTrack1X);
		SCAN_VAR(DunkshotTrack1Y);
		SCAN_VAR(DunkshotTrack2X);
		SCAN_VAR(DunkshotTrack2Y);
		SCAN_VAR(DunkshotTrack3X);
		SCAN_VAR(DunkshotTrack3Y);
		SCAN_VAR(DunkshotTrack4X);
		SCAN_VAR(DunkshotTrack4Y);
	}
	
	return System16Scan(nAction, pnMin);;
}

void EswatMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0bffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0bffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x440000, 0x4407ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x840000, 0x840fff, SM_RAM);
	SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
	SekSetReadByteHandler(0, System16BReadByte);
	SekSetWriteByteHandler(0, System16BWriteByte);
	SekSetWriteWordHandler(0, System16BWriteWord);
	SekClose();
}

static INT32 EswatInit()
{
	System16Map68KDo = EswatMap68K;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1c0000 - 0x180000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x123406, 0x123407, SM_WRITE);
		SekSetWriteByteHandler(1, EswatSoundWriteByte);
		
		SekMapHandler(2, 0x3e0000, 0x3e0fff, SM_RAM);
		SekSetReadWordHandler(2, EswatMultiply0ReadWord);
		SekSetWriteWordHandler(2, EswatMultiply0WriteWord);
		
		SekMapHandler(3, 0x3e1000, 0x3e1fff, SM_RAM);
		SekSetReadWordHandler(3, EswatCompare0ReadWord);
		SekSetWriteWordHandler(3, EswatCompare0WriteWord);
		
		SekMapHandler(4, 0x3e2000, 0x3e2003, SM_WRITE);
		SekSetWriteByteHandler(4, EswatWriteByte);
		SekClose();
		
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x1c0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x1c0000);
			memset(System16Sprites, 0, 0x1c0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0x40000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x040000, 0x40000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x080000, 0x40000);
			memcpy(System16Sprites + 0x140000, pTemp + 0x0c0000, 0x40000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x100000, 0x40000);
			memcpy(System16Sprites + 0x180000, pTemp + 0x140000, 0x40000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 EswatblInit()
{
	System16Map68KDo = EswatMap68K;
	
	System16SpriteXOffset = 124;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1c0000 - 0x180000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x418000, 0x418031, SM_WRITE);
		SekSetWriteWordHandler(1, EswatblGfxWriteWord);
		SekSetWriteByteHandler(1, EswatblGfxWriteByte);
		
		SekMapHandler(2, 0xc42006, 0xc42007, SM_WRITE);
		SekSetWriteByteHandler(2, EswatblSoundWriteByte);
		
		SekMapHandler(3, 0x3e2000, 0x3e2003, SM_WRITE);
		SekSetWriteByteHandler(3, EswatWriteByte);

		SekClose();
		
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x1c0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x1c0000);
			memset(System16Sprites, 0, 0x1c0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0x40000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x040000, 0x40000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x080000, 0x40000);
			memcpy(System16Sprites + 0x140000, pTemp + 0x0c0000, 0x40000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x100000, 0x40000);
			memcpy(System16Sprites + 0x180000, pTemp + 0x140000, 0x40000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
		
		bSystem16BootlegRender = true;
	}
	
	return nRet;
}

static INT32 ExctleagInit()
{
	System16MakeAnalogInputsDo = ExctleagMakeAnalogInputs;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, ExctleagReadByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 ExctleagExit()
{
	ExctleagTrack1X = 0;
	ExctleagTrack1Y = 0;
	ExctleagTrack2X = 0;
	ExctleagTrack2Y = 0;

	return System16Exit();
}

static INT32 ExctleagScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin =  0x029660;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(ExctleagTrack1X);
		SCAN_VAR(ExctleagTrack1Y);
		SCAN_VAR(ExctleagTrack2X);
		SCAN_VAR(ExctleagTrack2Y);
	}
	
	return System16Scan(nAction, pnMin);;
}

void Fantzn2xMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0fffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0fffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x440000, 0x4407ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x840000, 0x840fff, SM_RAM);
	SekMapMemory(System16Ram           , 0x200000, 0x23ffff, SM_RAM);
	SekSetWriteWordHandler(0, System16BWriteWord);
	SekSetReadByteHandler(0, System16BReadByte);
	SekSetWriteByteHandler(0, System16BWriteByte);
	SekMapHandler(1, 0x3f0000, 0x3f0003, SM_WRITE);
	SekSetWriteByteHandler(1, Fantzn2xWriteByte);
	SekClose();
}

static INT32 Fantzn2xLoadRom()
{
	INT32 nRet = System16LoadRoms(1);
	
	memcpy(System16Rom + 0x80000, System16Rom + 0x40000, 0x40000);
	memset(System16Rom + 0x40000, 0, 0x40000);
	memcpy(System16Code + 0x80000, System16Code + 0x40000, 0x40000);
	memset(System16Code + 0x40000, 0, 0x40000);
	
	return nRet;
}

static INT32 Fantzn2xps2LoadRom()
{
	if (BurnLoadRom(System16Rom + 0x00000, 0, 1)) return 1;
	if (BurnLoadRom(System16Rom + 0x80000, 1, 1)) return 1;
	
	memcpy(System16Code, System16Rom, 0x100000);
	
	System16TempGfx = (UINT8*)BurnMalloc(System16TileRomSize);
	BurnLoadRom(System16TempGfx, 2, 1);
	GfxDecode(0x4000, 3, 8, 8, Fantzn2xPlaneOffsets, Fantzn2xXOffsets, Fantzn2xYOffsets, 0x100, System16TempGfx, System16Tiles);
	System16NumTiles = 0x4000;
	BurnFree(System16TempGfx);
	
	BurnLoadRom(System16Sprites, 3, 1);
	
	if (BurnLoadRom(System16Z80Rom, 4, 1)) return 1;
	memcpy(System16UPD7759Data, System16Z80Rom + 0x10000, 0x20000);
		
	return 0;
}

static INT32 Fantzn2xInit()
{
	System16Map68KDo = Fantzn2xMap68K;
	System16CustomLoadRomDo = Fantzn2xLoadRom;
	
	return System16Init();
}

static INT32 Fantzn2xps2Init()
{
	System16Map68KDo = Fantzn2xMap68K;
	System16CustomLoadRomDo = Fantzn2xps2LoadRom;
	System16UPD7759DataSize = 0x20000;
	
	return System16Init();
}

void FantzntaMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x440000, 0x4407ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x840000, 0x840fff, SM_RAM);
	SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0, System16BWriteWord);
	SekSetReadByteHandler(0, System16BReadByte);
	SekSetWriteByteHandler(0, System16BWriteByte);
	SekMapHandler(1, 0x3f0000, 0x3f0003, SM_WRITE);
	SekSetWriteByteHandler(1, Fantzn2xWriteByte);
	SekClose();
}

static INT32 FantzntaLoadRom()
{
	if (BurnLoadRom(System16Rom + 0x00000, 0, 1)) return 1;
	
	System16TempGfx = (UINT8*)BurnMalloc(System16TileRomSize);
	BurnLoadRom(System16TempGfx, 1, 1);
	GfxDecode(0x2000, 3, 8, 8, Fantzn2xPlaneOffsets, Fantzn2xXOffsets, Fantzn2xYOffsets, 0x100, System16TempGfx, System16Tiles);
	System16NumTiles = 0x2000;
	BurnFree(System16TempGfx);
	
	BurnLoadRom(System16Sprites, 2, 1);
	BurnByteswap(System16Sprites, System16SpriteRomSize);
	
	if (BurnLoadRom(System16Z80Rom, 3, 1)) return 1;
	memcpy(System16UPD7759Data, System16Z80Rom + 0x10000, 0x10000);
		
	return 0;
}

static INT32 FantzntaInit()
{
	System16Map68KDo = FantzntaMap68K;
	System16CustomLoadRomDo = FantzntaLoadRom;
	System16UPD7759DataSize = 0x10000;
	
	return System16Init();
}

static INT32 FpointblInit()
{
	System16MapZ80Do = BootlegMapZ80;
	
	INT32 nRet = System16Init();
	
	System16SpriteXOffset = 109;
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, FpointblReadByte);
		SekSetWriteByteHandler(0, FpointblWriteByte);
		
		SekMapHandler(1, 0xc46000, 0xc46031, SM_WRITE);
		SekSetWriteWordHandler(1, FpointblGfxWriteWord);
		SekClose();
		
		bSystem16BootlegRender = true;
	}
	
	return nRet;
}

void GoldnaxeMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0bffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0bffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0x100000, 0x10ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x110000, 0x110fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x200000, 0x2007ff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x300000, 0x3007ff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x400000, 0x4007ff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x500000, 0x5007ff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x700000, 0x7007ff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x800000, 0x8007ff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x900000, 0x9007ff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x600000, 0x6007ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x140000, 0x140fff, SM_RAM);
	SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);	
	SekSetReadByteHandler(0, System16BReadByte);
	SekSetWriteByteHandler(0, System16BWriteByte);
	
	SekMapHandler(1, 0x100000, 0x10ffff, SM_WRITE);
	SekSetWriteByteHandler(1, GoldnaxeTileWriteByte);
	SekSetWriteWordHandler(1, GoldnaxeTileWriteWord);
	SekClose();
}

void Goldnaxe_Sim8751()
{
	// Protection MCU
	UINT16 temp1 = (System16Ram[0x2cd8 + 1] << 8) | System16Ram[0x2cd8 + 0];
	UINT16 temp2 = (System16Ram[0x2cda + 1] << 8) | System16Ram[0x2cda + 0];
	UINT16 temp3 = (System16Ram[0x2cdc + 1] << 8) | System16Ram[0x2cdc + 0];
	UINT16 temp4 = (System16Ram[0x2cde + 1] << 8) | System16Ram[0x2cde + 0];
	if (temp1 == 0 && temp2 == 0 && temp3 == 0 && temp4 == 0) {
		*((UINT16*)(System16Ram + 0x2cd8)) = BURN_ENDIAN_SWAP_INT16(0x048c);
		*((UINT16*)(System16Ram + 0x2cda)) = BURN_ENDIAN_SWAP_INT16(0x159d);
		*((UINT16*)(System16Ram + 0x2cdc)) = BURN_ENDIAN_SWAP_INT16(0x26ae);
		*((UINT16*)(System16Ram + 0x2cde)) = BURN_ENDIAN_SWAP_INT16(0x37bf);
	}
	
	// Sound command
	UINT16 temp = (System16Ram[0x2cfc + 1] << 8) | System16Ram[0x2cfc + 0];
	if ((temp & 0xff00) != 0x0000) {
		System16SoundLatch = temp >> 8;
		ZetOpen(0);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		ZetClose();
		*((UINT16*)(System16Ram + 0x2cfc)) = BURN_ENDIAN_SWAP_INT16((UINT16)(temp & 0xff));
	}
	
	// Inputs
	*((UINT16*)(System16Ram + 0x2cd0)) = BURN_ENDIAN_SWAP_INT16((UINT16)(~((System16Input[1] << 8) | System16Input[2])));
	*((UINT16*)(System16Ram + 0x2c96)) = BURN_ENDIAN_SWAP_INT16((UINT16)(~System16Input[0] << 8));
}

static INT32 GoldnaxeInit()
{
	Simulate8751 = Goldnaxe_Sim8751;
	System16Map68KDo = GoldnaxeMap68K;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1c0000 - 0x180000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(2, 0x1e0000, 0x1e0fff, SM_RAM);
		SekSetReadWordHandler(2, GoldnaxeCompare1ReadWord);
		SekSetWriteWordHandler(2, GoldnaxeCompare1WriteWord);
		
		SekMapHandler(3, 0x1f0000, 0x1f0fff, SM_RAM);
		SekSetReadWordHandler(3, GoldnaxeMultiply0ReadWord);
		SekSetWriteWordHandler(3, GoldnaxeMultiply0WriteWord);
		
		SekMapHandler(4, 0x1f1000, 0x1f1fff, SM_RAM);
		SekSetReadWordHandler(4, GoldnaxeCompare0ReadWord);
		SekSetWriteWordHandler(4, GoldnaxeCompare0WriteWord);
		
		SekMapHandler(5, 0x1f2000, 0x1f2003, SM_WRITE);
		SekSetWriteByteHandler(5, Goldnaxe1WriteByte);
		SekClose();
		
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x1c0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x1c0000);
			memset(System16Sprites, 0, 0x1c0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0x40000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x040000, 0x40000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x080000, 0x40000);
			memcpy(System16Sprites + 0x140000, pTemp + 0x0c0000, 0x40000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x100000, 0x40000);
			memcpy(System16Sprites + 0x180000, pTemp + 0x140000, 0x40000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Goldnaxe1Init()
{
	System16Map68KDo = GoldnaxeMap68K;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1c0000 - 0x180000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(2, 0x1e0000, 0x1e0fff, SM_RAM);
		SekSetReadWordHandler(2, GoldnaxeCompare1ReadWord);
		SekSetWriteWordHandler(2, GoldnaxeCompare1WriteWord);
		
		SekMapHandler(3, 0x1f0000, 0x1f0fff, SM_RAM);
		SekSetReadWordHandler(3, GoldnaxeMultiply0ReadWord);
		SekSetWriteWordHandler(3, GoldnaxeMultiply0WriteWord);
		
		SekMapHandler(4, 0x1f1000, 0x1f1fff, SM_RAM);
		SekSetReadWordHandler(4, GoldnaxeCompare0ReadWord);
		SekSetWriteWordHandler(4, GoldnaxeCompare0WriteWord);
		
		SekMapHandler(5, 0x1f2000, 0x1f2003, SM_WRITE);
		SekSetWriteByteHandler(5, Goldnaxe1WriteByte);
		SekClose();
		
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x1c0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x1c0000);
			memset(System16Sprites, 0, 0x1c0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0x40000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x040000, 0x40000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x080000, 0x40000);
			memcpy(System16Sprites + 0x140000, pTemp + 0x0c0000, 0x40000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x100000, 0x40000);
			memcpy(System16Sprites + 0x180000, pTemp + 0x140000, 0x40000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Goldnaxe2Init()
{
	Simulate8751 = Goldnaxe_Sim8751;
	System16Map68KDo = GoldnaxeMap68K;
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1c0000 - 0x180000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(2, 0x1f0000, 0x1f0003, SM_WRITE);
		SekSetWriteByteHandler(2, Goldnaxe3WriteByte);
		SekClose();
	
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x1c0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x1c0000);
			memset(System16Sprites, 0, 0x1c0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0x40000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x040000, 0x40000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x080000, 0x40000);
			memcpy(System16Sprites + 0x140000, pTemp + 0x0c0000, 0x40000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x100000, 0x40000);
			memcpy(System16Sprites + 0x180000, pTemp + 0x140000, 0x40000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 Goldnaxe3Init()
{
	System16Map68KDo = GoldnaxeMap68K;
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1c0000 - 0x180000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(2, 0x1f0000, 0x1f0003, SM_WRITE);
		SekSetWriteByteHandler(2, Goldnaxe3WriteByte);
		SekClose();
		
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x1c0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x1c0000);
			memset(System16Sprites, 0, 0x1c0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0x40000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x040000, 0x40000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x080000, 0x40000);
			memcpy(System16Sprites + 0x140000, pTemp + 0x0c0000, 0x40000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x100000, 0x40000);
			memcpy(System16Sprites + 0x180000, pTemp + 0x140000, 0x40000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

static INT32 HwchampInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x3f0000, 0x3f0003, SM_WRITE);
		SekSetWriteByteHandler(1, HwchampWriteByte);
		
		SekMapHandler(2, 0xc43020, 0xc43025, SM_RAM);
		SekSetReadWordHandler(2, HwchampCtrlReadWord);
		SekSetWriteWordHandler(2, HwchampCtrlWriteWord);
		SekClose();
	}
	
	return nRet;
}

static INT32 HwchampExit()
{
	HwchampInputVal = 0;
	
	return System16Exit();
}

static INT32 HwchampScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin =  0x029660;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(HwchampInputVal);
	}
	
	return System16Scan(nAction, pnMin);;
}

static INT32 MvpInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x3f2000, 0x3f2003, SM_WRITE);
		SekSetWriteByteHandler(1, MvpWriteByte);
		SekClose();
	
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x200000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x200000);
			memset(System16Sprites, 0, 0x200000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0x40000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x040000, 0x40000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x080000, 0x40000);
			memcpy(System16Sprites + 0x140000, pTemp + 0x0c0000, 0x40000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x100000, 0x40000);
			memcpy(System16Sprites + 0x180000, pTemp + 0x140000, 0x40000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x180000, 0x40000);
			memcpy(System16Sprites + 0x1c0000, pTemp + 0x1c0000, 0x40000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}

	return nRet;
}

static INT32 MvpjInit()
{
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x200000 - 0x180000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x3f0000, 0x3f0003, SM_WRITE);
		SekSetWriteByteHandler(1, MvpjWriteByte);
		SekClose();
	
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x200000);
		if (pTemp) {
			memset(pTemp, 0, 0x200000);
			memcpy(pTemp, System16Sprites, 0x200000);
			memset(System16Sprites, 0, 0x200000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0xc0000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x0c0000, 0xc0000);
		} else {
			nRet = 1;
		}		
		BurnFree(pTemp);
	}

	return nRet;
}

static INT32 PassshtInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xc00006, 0xc00007, SM_WRITE);
		SekSetWriteByteHandler(1, PassshtWriteByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 PassshtaInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xc00006, 0xc00007, SM_WRITE);
		SekSetWriteByteHandler(1, PassshtWriteByte);
		SekMapHandler(2, 0xc43000, 0xc43007, SM_READ);
		SekSetReadByteHandler(2, PassshtaReadByte);
		SekClose();
	}
	
	return nRet;
}

static INT32 PassshtbInit()
{
	INT32 nRet = System16Init();
	
//	System16SpriteXOffset = 114;
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xc46000, 0xc46031, SM_WRITE);
		SekSetWriteWordHandler(1, PassshtbGfxWriteWord);
		
//		SekMapHandler(2, 0x842000, 0x842001, SM_READ);
//		SekSetReadByteHandler(2, PassshtbReadByte);
		
//		SekMapHandler(2, 0xc42006, 0xc42007, SM_WRITE);
//		SekSetWriteByteHandler(2, TetrisblSndWriteByte);
		SekClose();
		
		bSystem16BootlegRender = true;
	}
	
	return nRet;
}

void RiotcityMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0bffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0bffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0xfa0000, 0xfaffff, SM_READ);
	SekMapMemory(System16TextRam       , 0xfb0000, 0xfb0fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0xf40000, 0xf407ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0xf60000, 0xf60fff, SM_RAM);
	SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
	SekSetReadByteHandler(0, RiotcityReadByte);
	SekSetWriteByteHandler(0, RiotcityWriteByte);
	SekSetWriteWordHandler(0, RiotcityWriteWord);
	SekClose();
}

static INT32 RiotcityInit()
{
	System16Map68KDo = RiotcityMap68K;
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x1c0000 - 0x180000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x1c0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x1c0000);
			memset(System16Sprites, 0, 0x1c0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x000000, 0x40000);
			memcpy(System16Sprites + 0x100000, pTemp + 0x040000, 0x40000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x080000, 0x40000);
			memcpy(System16Sprites + 0x140000, pTemp + 0x0c0000, 0x40000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x100000, 0x40000);
			memcpy(System16Sprites + 0x180000, pTemp + 0x140000, 0x40000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}
	
	return nRet;
}

void RyukyuMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0bffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0bffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x300000, 0x3007ff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x300800, 0x300fff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x500000, 0x500fff, SM_RAM);
	SekMapMemory(System16Ram           , 0x200000, 0x203fff, SM_RAM);
	SekSetReadWordHandler(0, RyukyuReadWord);
	SekSetWriteWordHandler(0, RyukyuWriteWord);
	SekSetReadByteHandler(0, System16BReadByte);
	SekSetWriteByteHandler(0, RyukyuWriteByte);
	SekClose();
}

static INT32 RyukyuInit()
{
	System16Map68KDo = RyukyuMap68K;
	
	return System16Init();
}

static INT32 SdibInit()
{
	System16MakeAnalogInputsDo = SdibMakeAnalogInputs;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, SdibReadByte);
		SekMapHandler(1, 0x123406, 0x123407, SM_WRITE);
		SekSetWriteByteHandler(1, SdibSoundWriteByte);		
		SekClose();
	}
	
	return nRet;
}

static INT32 SdiblInit()
{
	System16MakeAnalogInputsDo = SdibMakeAnalogInputs;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, SdibReadByte);
		SekMapHandler(1, 0x123406, 0x123407, SM_WRITE);
		SekSetWriteByteHandler(1, SdibSoundWriteByte);		
		SekClose();
	}
	
	return nRet;
}

static INT32 SdibExit()
{
	SdiTrack1X = 0;
	SdiTrack1Y = 0;
	SdiTrack2X = 0;
	SdiTrack2Y = 0;

	return System16Exit();
}

static INT32 SdibScan(INT32 nAction,INT32 *pnMin)
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

static INT32 SjryukoInit()
{
	System16BTileAlt = true;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetWriteByteHandler(0, SjryukoWriteByte);
		SekSetReadByteHandler(0, SjryukoReadByte);
		SekSetReadWordHandler(0, SjryukoReadWord);
		SekClose();
	}
	
	return nRet;
}

static INT32 SjryukoExit()
{
	MahjongInputNum = 0;
	
	return System16Exit();
}

static INT32 SjryukoScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin =  0x029660;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(MahjongInputNum);
	}
	
	return System16Scan(nAction, pnMin);;
}

static INT32 SnapperInit()
{
	System16Map68KDo = AtomicpMap68K;
	
	INT32 nRet = System16Init();
	
	System16IgnoreVideoEnable = 1;
	System16YM2413IRQInterval = 41;
	
	return nRet;
}

static INT32 SonicbomInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x123406, 0x123407, SM_WRITE);
		SekSetWriteByteHandler(1, SonicbomWriteByte);
		SekMapHandler(2, 0xc42000, 0xc42003, SM_READ);
		SekSetReadWordHandler(2, SonicbomReadWord);
		SekClose();
	}

	return nRet;
}

static INT32 TetrisblInit()
{
	INT32 nRet = System16Init();
	
	System16SpriteXOffset = 114;
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0x418000, 0x418031, SM_WRITE);
		SekSetWriteWordHandler(1, TetrisblGfxWriteWord);
		
		SekMapHandler(2, 0xc42006, 0xc42007, SM_WRITE);
		SekSetWriteByteHandler(2, TetrisblSndWriteByte);
		SekClose();
		
		bSystem16BootlegRender = true;
	}
	
	return nRet;
}

static INT32 TimescanInit()
{
	System16BTileAlt = true;

	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0x80000 - 0x40000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0x80000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x40000);
			memset(System16Sprites, 0, 0x80000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x020000, pTemp + 0x10000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x060000, pTemp + 0x30000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}

	return nRet;
}

static INT32 ToryumonInit()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekSetReadByteHandler(0, ToryumonReadByte);
		SekSetWriteByteHandler(0, ToryumonWriteByte);
		SekMapMemory(System16Ram, 0xff0000, 0xff3fff, SM_RAM);
		SekMapMemory(System16Ram, 0xff4000, 0xff7fff, SM_RAM);
		SekMapMemory(System16Ram, 0xff8000, 0xffbfff, SM_RAM);
		SekClose();
	}

	return nRet;
}

void TturfMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0bffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0bffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x300000, 0x3007ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x500000, 0x500fff, SM_RAM);
	SekMapMemory(System16Ram           , 0x200000, 0x203fff, SM_RAM);
	SekSetReadByteHandler(0, TturfReadByte);
	SekSetWriteByteHandler(0, TturfWriteByte);
	SekSetWriteWordHandler(0, System16BWriteWord);
	SekClose();
}

void Tturf_Sim8751()
{
	// Inputs
	*((UINT16*)(System16Ram + 0x01e6)) = BURN_ENDIAN_SWAP_INT16((UINT16)(~System16Input[0] << 8));
	*((UINT16*)(System16Ram + 0x01e8)) = BURN_ENDIAN_SWAP_INT16((UINT16)(~System16Input[1] << 8));
	*((UINT16*)(System16Ram + 0x01ea)) = BURN_ENDIAN_SWAP_INT16((UINT16)(~System16Input[2] << 8));
	
	// Sound command
	UINT16 temp = (System16Ram[0x01d0 + 1] << 8) | System16Ram[0x01d0 + 0];
	if ((temp & 0xff00) != 0x0000) {
		System16SoundLatch = temp & 0xff;
		ZetOpen(0);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		ZetClose();
		*((UINT16*)(System16Ram + 0x01d0)) = BURN_ENDIAN_SWAP_INT16((UINT16)(temp & 0xff));
	}
}

static INT32 TturfInit()
{
	Simulate8751 = Tturf_Sim8751;
	System16Map68KDo = TturfMap68K;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0xe0000 - 0x80000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0xe0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x80000);
			memset(System16Sprites, 0, 0xe0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x40000, 0x20000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x60000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}

	return nRet;
}

static INT32 TturfuInit()
{
	Simulate8751 = Tturf_Sim8751;
	System16Map68KDo = TturfMap68K;
	
	return System16Init();
}

void Wb3_Sim8751()
{
	// Sound command
	UINT16 temp = (System16Ram[0x0008 + 1] << 8) | System16Ram[0x0008 + 0];
	if ((temp & 0xff00) != 0x0000) {
		System16SoundLatch = temp >> 8;
		ZetOpen(0);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		ZetClose();
		*((UINT16*)(System16Ram + 0x0008)) = BURN_ENDIAN_SWAP_INT16((UINT16)(temp & 0xff));
	}
}

static INT32 Wb3Init()
{
	Simulate8751 = Wb3_Sim8751;
	
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0xe0000 - 0x80000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		UINT8 *pTemp = (UINT8*)BurnMalloc(0xe0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x80000);
			memset(System16Sprites, 0, 0xe0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x40000, 0x20000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x60000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}

	return nRet;
}

static INT32 Wb32Init()
{
	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xdf0006, 0xdf0007, SM_WRITE);
		SekSetWriteByteHandler(1, Wb3WriteByte);
		SekClose();
	}

	return nRet;
}

static INT32 Wb33Init()
{
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0xe0000 - 0x80000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xdf0006, 0xdf0007, SM_WRITE);
		SekSetWriteByteHandler(1, Wb3WriteByte);
		SekClose();
	
		UINT8 *pTemp = (UINT8*)BurnMalloc(0xe0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x80000);
			memset(System16Sprites, 0, 0xe0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x40000, 0x20000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x60000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
	}

	return nRet;
}

static INT32 Wb3bblInit()
{
	// Start off with some sprite rom and let the load routine add on the rest
	System16SpriteRomSize = 0xe0000 - 0x80000;

	INT32 nRet = System16Init();
	
	if (!nRet) {
		SekOpen(0);
		SekMapHandler(1, 0xc44000, 0xc46031, SM_WRITE);
		SekSetWriteWordHandler(1, Wb3bblGfxWriteWord);
		SekClose();
		
		UINT8 *pTemp = (UINT8*)BurnMalloc(0xe0000);
		if (pTemp) {
			memcpy(pTemp, System16Sprites, 0x80000);
			memset(System16Sprites, 0, 0xe0000);
			memcpy(System16Sprites + 0x000000, pTemp + 0x00000, 0x20000);
			memcpy(System16Sprites + 0x040000, pTemp + 0x20000, 0x20000);
			memcpy(System16Sprites + 0x080000, pTemp + 0x40000, 0x20000);
			memcpy(System16Sprites + 0x0c0000, pTemp + 0x60000, 0x20000);
		} else {
			nRet = 1;
		}
		BurnFree(pTemp);
		
		bSystem16BootlegRender = true;
	}

	return nRet;
}

void WrestwarMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom           , 0x000000, 0x0bffff, SM_READ);
	SekMapMemory(System16Code          , 0x000000, 0x0bffff, SM_FETCH);
	SekMapMemory(System16TileRam       , 0x100000, 0x10ffff, SM_READ);
	SekMapMemory(System16TextRam       , 0x110000, 0x110fff, SM_RAM);
	SekMapMemory(System16SpriteRam     , 0x200000, 0x2007ff, SM_RAM);
	SekMapMemory(System16PaletteRam    , 0x300000, 0x300fff, SM_RAM);
	SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
	SekSetReadByteHandler(0, System16BReadByte);
	SekSetWriteByteHandler(0, System16BWriteByte);
	SekMapHandler(1, 0x400000, 0x400003, SM_WRITE);
	SekSetWriteByteHandler(1, WrestwarWriteByte);
	SekMapHandler(2, 0x100000, 0x10ffff, SM_WRITE);
	SekSetWriteByteHandler(2, WrestwarTileWriteByte);
	SekSetWriteWordHandler(2, WrestwarTileWriteWord);
	SekClose();
}

void Wrestwar_Sim8751()
{
	// System Inputs
	*((UINT16*)(System16Ram + 0x2082)) = BURN_ENDIAN_SWAP_INT16((UINT16)~System16Input[0]);
	
	// Sound command
	UINT16 temp = (System16Ram[0x208e + 1] << 8) | System16Ram[0x208e + 0];
	if ((temp & 0xff00) != 0x0000) {
		System16SoundLatch = temp & 0xff;
		ZetOpen(0);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		ZetClose();
		*((UINT16*)(System16Ram + 0x208e)) = BURN_ENDIAN_SWAP_INT16((UINT16)(temp & 0xff));
	}
}

static INT32 WrestwarInit()
{
	Simulate8751 = Wrestwar_Sim8751;
	System16Map68KDo = WrestwarMap68K;
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	return System16Init();
}

static INT32 Wrestwar1Init()
{
	System16Map68KDo = WrestwarMap68K;
	System16CustomLoadRomDo = CustomLoadRom40000;
	
	return System16Init();
}

/*====================================================
Driver defs
====================================================*/

struct BurnDriver BurnDrvAceattac = {
	"aceattac", NULL, NULL, NULL, "1988",
	"Ace Attacker (FD1094 317-0059)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358 | HARDWARE_SEGA_FD1094_ENC, GBF_SPORTSMISC, 0,
	NULL, AceattacRomInfo, AceattacRomName, NULL, NULL, System16bDip3InputInfo, NULL,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvAliensyn = {
	"aliensyn", NULL, NULL, NULL, "1987",
	"Alien Syndrome (set 4, System 16B, unprotected)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358, GBF_MAZE, 0,
	NULL, AliensynRomInfo, AliensynRomName, NULL, NULL, System16bfire1InputInfo, AliensynDIPInfo,
	AliensynInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAliensyn3 = {
	"aliensyn3", "aliensyn", NULL, NULL, "1987",
	"Alien Syndrome (set 3, System 16B, FD1089A 317-0033)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358 | HARDWARE_SEGA_FD1089A_ENC, GBF_MAZE, 0,
	NULL, Aliensyn3RomInfo, Aliensyn3RomName, NULL, NULL, System16bfire1InputInfo, AliensynDIPInfo,
	Aliensyn3Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAliensynj = {
	"aliensynj", "aliensyn", NULL, NULL, "1987",
	"Alien Syndrome (set 6, Japan, new, System 16B, FD1089A 317-0033)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358 | HARDWARE_SEGA_FD1089A_ENC, GBF_MAZE, 0,
	NULL, AliensynjRomInfo, AliensynjRomName, NULL, NULL, System16bfire1InputInfo, AliensynjDIPInfo,
	Aliensyn3Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAltbeast = {
	"altbeast", NULL, NULL, NULL, "1988",
	"Altered Beast (set 8, 8751 317-0078)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, AltbeastRomInfo, AltbeastRomName, NULL, NULL, System16bfire3InputInfo, AltbeastDIPInfo,
	AltbeastInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAltbeastj = {
	"altbeastj", "altbeast", NULL, NULL, "1988",
	"Juuouki (set 7, Japan, 8751 317-0077)\0", NULL, "Sega", "System 16B",
	L"Juuoki (set 7, Japan, 8751 317-0077)\0\u7363\u738B\u8A18\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, AltbeastjRomInfo, AltbeastjRomName, NULL, NULL, System16bfire3InputInfo, AltbeastDIPInfo,
	AltbeastjInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAltbeast2 = {
	"altbeast2", "altbeast", NULL, NULL, "1988",
	"Altered Beast (set 2, MC-8123B 317-0066)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_MC8123_ENC, GBF_SCRFIGHT, 0,
	NULL, Altbeast2RomInfo, Altbeast2RomName, NULL, NULL, System16bfire3InputInfo, AltbeastDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAltbeastj3 = {
	"altbeastj3", "altbeast", NULL, NULL, "1988",
	"Juuouki (set 3, Japan, FD1094 317-0068)\0", NULL, "Sega", "System 16B",
	L"Juuoki (set 3, Japan, FD1094 317-0068)\0\u7363\u738B\u8A18\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_SCRFIGHT, 0,
	NULL, Altbeastj3RomInfo, Altbeastj3RomName, NULL, NULL, System16bfire3InputInfo, AltbeastDIPInfo,
	Altbeastj3Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAltbeast4 = {
	"altbeast4", "altbeast", NULL, NULL, "1988",
	"Altered Beast (set 4, MC-8123B 317-0066)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_MC8123_ENC, GBF_SCRFIGHT, 0,
	NULL, Altbeast4RomInfo, Altbeast4RomName, NULL, NULL, System16bfire3InputInfo, AltbeastDIPInfo,
	Altbeast4Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAltbeast5 = {
	"altbeast5", "altbeast", NULL, NULL, "1988",
	"Altered Beast (set 5, FD1094 317-0069)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_SCRFIGHT, 0,
	NULL, Altbeast5RomInfo, Altbeast5RomName, NULL, NULL, System16bfire3InputInfo, AltbeastDIPInfo,
	Altbeast4Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAltbeast6 = {
	"altbeast6", "altbeast", NULL, NULL, "1988",
	"Altered Beast (set 6, 8751 317-0076)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, Altbeast6RomInfo, Altbeast6RomName, NULL, NULL, System16bfire3InputInfo, AltbeastDIPInfo,
	Altbeast6Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAtomicp = {
	"atomicp", NULL, NULL, NULL, "1990",
	"Atomic Point (Korea)\0", NULL, "Philco", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_YM2413, GBF_MISC, 0,
	NULL, AtomicpRomInfo, AtomicpRomName, NULL, NULL, AtomicpInputInfo, AtomicpDIPInfo,
	AtomicpInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAurail = {
	"aurail", NULL, NULL, NULL, "1990",
	"Aurail (set 3, US, unprotected)\0", NULL, "Sega / Westone", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_VERSHOOT, 0,
	NULL, AurailRomInfo, AurailRomName, NULL, NULL, System16bfire3InputInfo, AurailDIPInfo,
	AurailInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAurail1 = {
	"aurail1", "aurail", NULL, NULL, "1990",
	"Aurail (set 2, World, FD1089B 317-0168)\0", NULL, "Sega / Westone", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1089B_ENC, GBF_VERSHOOT, 0,
	NULL, Aurail1RomInfo, Aurail1RomName, NULL, NULL, System16bfire3InputInfo, AurailDIPInfo,
	Aurail1Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvAurailj = {
	"aurailj", "aurail", NULL, NULL, "1990",
	"Aurail (set 1, Japan, FD1089A 317-0167)\0", NULL, "Sega / Westone", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1089A_ENC, GBF_VERSHOOT, 0,
	NULL, AurailjRomInfo, AurailjRomName, NULL, NULL, System16bfire3InputInfo, AurailDIPInfo,
	AurailjInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvBayroute = {
	"bayroute", NULL, NULL, NULL, "1989",
	"Bay Route (set 3, World, FD1094 317-0116)\0", NULL, "Sunsoft / Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, BayrouteRomInfo, BayrouteRomName, NULL, NULL, System16bfire3InputInfo, BayrouteDIPInfo,
	BayrouteInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvBayroutej = {
	"bayroutej", "bayroute", NULL, NULL, "1989",
	"Bay Route (set 2, Japan, FD1094 317-0115)\0", NULL, "Sunsoft / Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, BayroutejRomInfo, BayroutejRomName, NULL, NULL, System16bfire3InputInfo, BayrouteDIPInfo,
	BayrouteInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvBayroute1 = {
	"bayroute1", "bayroute", NULL, NULL, "1989",
	"Bay Route (set 1, US, unprotected)\0", "No Sound, Missing Z80 Program ROM", "Sunsoft / Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358, GBF_PLATFORM, 0,
	NULL, Bayroute1RomInfo, Bayroute1RomName, NULL, NULL, System16bfire3InputInfo, BayrouteDIPInfo,
	Bayroute1Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvBlox16b = {
	"blox16b", "bloxeed", NULL, NULL, "2008",
	"Bloxeed (System 16B, PS2 data file)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5704_PS2, GBF_PUZZLE, 0,
	NULL, Blox16bRomInfo, Blox16bRomName, NULL, NULL, System16bfire3InputInfo, Blox16bDIPInfo,
	Blox16bInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvBullet = {
	"bullet", NULL, NULL, NULL, "1987",
	"Bullet (FD1094 317-0041)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_5358, GBF_VERSHOOT, 0,
	NULL, BulletRomInfo, BulletRomName, NULL, NULL, BulletInputInfo, BulletDIPInfo,
	BulletInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvCotton = {
	"cotton", NULL, NULL, NULL, "1991",
	"Cotton (set 3, World, FD1094 317-0181a)\0", NULL, "Sega / Success", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_HORSHOOT, 0,
	NULL, CottonRomInfo, CottonRomName, NULL, NULL, System16bInputInfo, CottonDIPInfo,
	CottonInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvCottonu = {
	"cottonu", "cotton", NULL, NULL, "1991",
	"Cotton (set 2, US, FD1094 317-0180)\0", NULL, "Sega / Success", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_HORSHOOT, 0,
	NULL, CottonuRomInfo, CottonuRomName, NULL, NULL, System16bInputInfo, CottonDIPInfo,
	CottonInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvCottonj = {
	"cottonj", "cotton", NULL, NULL, "1991",
	"Cotton (set 1, Japan, FD1094 317-0179a))\0", NULL, "Sega / Success", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_HORSHOOT, 0,
	NULL, CottonjRomInfo, CottonjRomName, NULL, NULL, System16bInputInfo, CottonDIPInfo,
	CottonInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDdux = {
	"ddux", NULL, NULL, NULL, "1989",
	"Dynamite Dux (set 2, FD1094 317-0096)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_SCRFIGHT, 0,
	NULL, DduxRomInfo, DduxRomName, NULL, NULL, System16bInputInfo, DduxDIPInfo,
	DduxInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDdux1 = {
	"ddux1", "ddux", NULL, NULL, "1989",
	"Dynamite Dux (set 1, 8751 317-0095)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, Ddux1RomInfo, Ddux1RomName, NULL, NULL, System16bInputInfo, DduxDIPInfo,
	Ddux1Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDduxbl = {
	"dduxbl", "ddux", NULL, NULL, "1989",
	"Dynamite Dux (bootleg)\0", NULL, "bootleg", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_INVERT_TILES | HARDWARE_SEGA_5358, GBF_SCRFIGHT, 0,
	NULL, DduxblRomInfo, DduxblRomName, NULL, NULL, System16bInputInfo, DduxDIPInfo,
	DduxblInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDunkshot = {
	"dunkshot", NULL, NULL, NULL, "1986",
	"Dunk Shot (FD1089 317-0022)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1089A_ENC | HARDWARE_SEGA_5358, GBF_SPORTSMISC, 0,
	NULL, DunkshotRomInfo, DunkshotRomName, NULL, NULL, DunkshotInputInfo, DunkshotDIPInfo,
	DunkshotInit, DunkshotExit, System16BFrame, NULL, DunkshotScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEswat = {
	"eswat", NULL, NULL, NULL, "1989",
	"E-Swat - Cyber Police (set 3, World, FD1094 317-0130)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, EswatRomInfo, EswatRomName, NULL, NULL, System16bfire3InputInfo, EswatDIPInfo,
	EswatInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEswatj = {
	"eswatj", "eswat", NULL, NULL, "1989",
	"E-Swat - Cyber Police (set 1, Japan, FD1094 317-0128)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, EswatjRomInfo, EswatjRomName, NULL, NULL, System16bfire3InputInfo, EswatDIPInfo,
	EswatInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEswatu = {
	"eswatu", "eswat", NULL, NULL, "1989",
	"E-Swat - Cyber Police (set 2, US, FD1094 317-0129)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, EswatuRomInfo, EswatuRomName, NULL, NULL, System16bfire3InputInfo, EswatDIPInfo,
	EswatInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEswatbl = {
	"eswatbl", "eswat", NULL, NULL, "1989",
	"E-Swat - Cyber Police (bootleg)\0", NULL, "bootleg", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797, GBF_PLATFORM, 0,
	NULL, EswatblRomInfo, EswatblRomName, NULL, NULL, System16bfire3InputInfo, EswatDIPInfo,
	EswatblInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvExctleag = {
	"exctleag", NULL, NULL, NULL, "1989",
	"Excite League (FD1094 317-0079)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_5358, GBF_SPORTSMISC, 0,
	NULL, ExctleagRomInfo, ExctleagRomName, NULL, NULL, ExctleagInputInfo, ExctleagDIPInfo,
	ExctleagInit, ExctleagExit, System16BFrame, NULL, ExctleagScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFantzn2x = {
	"fantzn2x", NULL, NULL, NULL, "2008",
	"Fantasy Zone II - The Tears of Opa-Opa (System 16C)\0", NULL, "Sega / M2", "System 16C",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5704_PS2, GBF_HORSHOOT, 0,
	NULL, Fantzn2xRomInfo, Fantzn2xRomName, NULL, NULL, System16bInputInfo, Fantzn2xDIPInfo,
	Fantzn2xInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFantzn2xp = {
	"fantzn2xp", "fantzn2x", NULL, NULL, "2008",
	"Fantasy Zone II - The Tears of Opa-Opa (System 16C, prototype)\0", NULL, "Sega / M2", "System 16C",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5704_PS2, GBF_HORSHOOT, 0,
	NULL, Fantzn2xpRomInfo, Fantzn2xpRomName, NULL, NULL, System16bInputInfo, Fantzn2xDIPInfo,
	Fantzn2xInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFantzn2xps2 = {
	"fantzn2xps2", "fantzn2x", NULL, NULL, "2008",
	"Fantasy Zone II - The Tears of Opa-Opa (System 16C, PS2 data file)\0", NULL, "Sega / M2", "System 16C",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5704_PS2, GBF_HORSHOOT, 0,
	NULL, Fantzn2xps2RomInfo, Fantzn2xps2RomName, NULL, NULL, System16bInputInfo, Fantzn2xDIPInfo,
	Fantzn2xps2Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFantznta = {
	"fantznta", "fantzn2x", NULL, NULL, "2008",
	"Fantasy Zone Time Attack (System 16B, PS2 data file)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5704_PS2, GBF_HORSHOOT, 0,
	NULL, FantzntaRomInfo, FantzntaRomName, NULL, NULL, System16bInputInfo, Fantzn2xDIPInfo,
	FantzntaInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFpoint = {
	"fpoint", NULL, NULL, NULL, "1989",
	"Flash Point (set 2, Japan, FD1094 317-0127A)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_5358, GBF_PUZZLE, 0,
	NULL, FpointRomInfo, FpointRomName, NULL, NULL, System16bfire1InputInfo, FpointDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFpoint1 = {
	"fpoint1", "fpoint", NULL, NULL, "1989",
	"Flash Point (set 1, Japan, FD1094 317-0127A)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_PUZZLE, 0,
	NULL, Fpoint1RomInfo, Fpoint1RomName, NULL, NULL, System16bfire1InputInfo, FpointDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFpointbl = {
	"fpointbl", "fpoint", NULL, NULL, "1989",
	"Flash Point (World, bootleg)\0", NULL, "bootleg", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_INVERT_TILES, GBF_PUZZLE, 0,
	NULL, FpointblRomInfo, FpointblRomName, NULL, NULL, FpointblInputInfo, FpointDIPInfo,
	FpointblInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFpointbj = {
	"fpointbj", "fpoint", NULL, NULL, "1989",
	"Flash Point (Japan, bootleg)\0", NULL, "bootleg", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_INVERT_TILES, GBF_PUZZLE, 0,
	NULL, FpointbjRomInfo, FpointbjRomName, NULL, NULL, FpointblInputInfo, FpointDIPInfo,
	FpointblInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGoldnaxe = {
	"goldnaxe", NULL, NULL, NULL, "1989",
	"Golden Axe (set 6, US, 8751 317-123A)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797, GBF_SCRFIGHT, 0,
	NULL, GoldnaxeRomInfo, GoldnaxeRomName, NULL, NULL, System16bfire3InputInfo, GoldnaxeDIPInfo,
	GoldnaxeInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGoldnaxe1 = {
	"goldnaxe1", "goldnaxe", NULL, NULL, "1989",
	"Golden Axe (set 1, World, FD1094 317-0110)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797 | HARDWARE_SEGA_FD1094_ENC, GBF_SCRFIGHT, 0,
	NULL, Goldnaxe1RomInfo, Goldnaxe1RomName, NULL, NULL, System16bfire3InputInfo, GoldnaxeDIPInfo,
	Goldnaxe1Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGoldnaxe2 = {
	"goldnaxe2", "goldnaxe", NULL, NULL, "1989",
	"Golden Axe (set 2, US, 8751 317-0112)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, Goldnaxe2RomInfo, Goldnaxe2RomName, NULL, NULL, System16bfire3InputInfo, GoldnaxeDIPInfo,
	Goldnaxe2Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGoldnaxe3 = {
	"goldnaxe3", "goldnaxe", NULL, NULL, "1989",
	"Golden Axe (set 3, World, FD1094 317-0120)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_SCRFIGHT, 0,
	NULL, Goldnaxe3RomInfo, Goldnaxe3RomName, NULL, NULL, System16bfire3InputInfo, GoldnaxeDIPInfo,
	Goldnaxe3Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGoldnaxej = {
	"goldnaxej", "goldnaxe", NULL, NULL, "1989",
	"Golden Axe (set 4, Japan, FD1094 317-0121)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_SCRFIGHT, 0,
	NULL, GoldnaxejRomInfo, GoldnaxejRomName, NULL, NULL, System16bfire3InputInfo, GoldnaxeDIPInfo,
	Goldnaxe3Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGoldnaxeu = {
	"goldnaxeu", "goldnaxe", NULL, NULL, "1989",
	"Golden Axe (set 5, US, FD1094 317-0122)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797 | HARDWARE_SEGA_FD1094_ENC, GBF_SCRFIGHT, 0,
	NULL, GoldnaxeuRomInfo, GoldnaxeuRomName, NULL, NULL, System16bfire3InputInfo, GoldnaxeDIPInfo,
	Goldnaxe1Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvHwchamp = {
	"hwchamp", NULL, NULL, NULL, "1987",
	"Heavyweight Champ\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_VSFIGHT, 0,
	NULL, HwchampRomInfo, HwchampRomName, NULL, NULL, HwchampInputInfo, HwchampDIPInfo,
	HwchampInit, HwchampExit, System16BFrame, NULL, HwchampScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvHwchampj = {
	"hwchampj", "hwchamp", NULL, NULL, "1987",
	"Heavyweight Champ (Japan, FD1094 317-0046)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_VSFIGHT, 0,
	NULL, HwchampjRomInfo, HwchampjRomName, NULL, NULL, HwchampInputInfo, HwchampDIPInfo,
	HwchampInit, HwchampExit, System16BFrame, NULL, HwchampScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMvp = {
	"mvp", NULL, NULL, NULL, "1989",
	"MVP (set 2, US, FD1094 317-0143)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797 | HARDWARE_SEGA_FD1094_ENC, GBF_SPORTSMISC, 0,
	NULL, MvpRomInfo, MvpRomName, NULL, NULL, System16bfire3InputInfo, MvpDIPInfo,
	MvpInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMvpj = {
	"mvpj", "mvp", NULL, NULL, "1989",
	"MVP (set 1, Japan, FD1094 317-0142)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_SPORTSMISC, 0,
	NULL, MvpjRomInfo, MvpjRomName, NULL, NULL, System16bfire3InputInfo, MvpDIPInfo,
	MvpjInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvPasssht = {
	"passsht", NULL, NULL, NULL, "1988",
	"Passing Shot (World, 2 Players, FD1094 317-0080)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_5358, GBF_SPORTSMISC, 0,
	NULL, PassshtRomInfo, PassshtRomName, NULL, NULL, System16bfire4InputInfo, PassshtDIPInfo,
	PassshtInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvPassshta = {
	"passshta", "passsht", NULL, NULL, "1988",
	"Passing Shot (World, 4 Players, FD1094 317-0074)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 4, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_5358, GBF_SPORTSMISC, 0,
	NULL, PassshtaRomInfo, PassshtaRomName, NULL, NULL, PassshtInputInfo, PassshtaDIPInfo,
	PassshtaInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvPassshtj = {
	"passshtj", "passsht", NULL, NULL, "1988",
	"Passing Shot (Japan, 4 Players, FD1094 317-0070)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 4, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_5358, GBF_SPORTSMISC, 0,
	NULL, PassshtjRomInfo, PassshtjRomName, NULL, NULL, PassshtInputInfo, PassshtaDIPInfo,
	PassshtaInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriverD BurnDrvPassshtb = {
	"passshtb", "passsht", NULL, NULL, "1988",
	"Passing Shot (bootleg, 2 Players)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358, GBF_SPORTSMISC, 0,
	NULL, PassshtbRomInfo, PassshtbRomName, NULL, NULL, PassshtInputInfo, PassshtDIPInfo,
	PassshtbInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvRiotcity = {
	"riotcity", NULL, NULL, NULL, "1991",
	"Riot City (Japan)\0", NULL, "Sega / Westone", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, RiotcityRomInfo, RiotcityRomName, NULL, NULL, System16bInputInfo, RiotcityDIPInfo,
	RiotcityInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvRyukyu = {
	"ryukyu", NULL, NULL, NULL, "1990",
	"RyuKyu (Japan, FD1094 317-5023)\0", NULL, "Success / Sega", "System 16B",
	L"RyuKyu \u7409\u7403 (Japan, FD1094 317-5023)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_PUZZLE, 0,
	NULL, RyukyuRomInfo, RyukyuRomName, NULL, NULL, RyukyuInputInfo, RyukyuDIPInfo,
	RyukyuInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSdib = {
	"sdib", "sdi", NULL, NULL, "1987",
	"SDI - Strategic Defense Initiative (System 16B, FD1089A 317-0028)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1089A_ENC | HARDWARE_SEGA_5358, GBF_SHOOT, 0,
	NULL, SdibRomInfo, SdibRomName, NULL, NULL, SdiInputInfo, SdibDIPInfo,
	SdibInit, SdibExit, System16BFrame, NULL, SdibScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSdibl = {
	"sdibl", "sdi", NULL, NULL, "1987",
	"SDI - Strategic Defense Initiative (bootleg)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358, GBF_SHOOT, 0,
	NULL, SdiblRomInfo, SdiblRomName, NULL, NULL, SdiInputInfo, SdibDIPInfo,
	SdiblInit, SdibExit, System16BFrame, NULL, SdibScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDefense = {
	"defense", "sdi", NULL, NULL, "1987",
	"Defense (System 16B, FD1089A 317-0028)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1089A_ENC | HARDWARE_SEGA_5358, GBF_SHOOT, 0,
	NULL, DefenseRomInfo, DefenseRomName, NULL, NULL, SdiInputInfo, SdibDIPInfo,
	SdibInit, SdibExit, System16BFrame, NULL, SdibScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinobi2 = {
	"shinobi2", "shinobi", NULL, NULL, "1987",
	"Shinobi (set 2, System 16B, FD1094 317-0049)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, Shinobi2RomInfo, Shinobi2RomName, NULL, NULL, System16bfire3InputInfo, ShinobiDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinobi3 = {
	"shinobi3", "shinobi", NULL, NULL, "1987",
	"Shinobi (set 3, System 16B, MC-8123B 317-0054)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358 | HARDWARE_SEGA_MC8123_ENC, GBF_PLATFORM, 0,
	NULL, Shinobi3RomInfo, Shinobi3RomName, NULL, NULL, System16bfire3InputInfo, ShinobiDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinobi4 = {
	"shinobi4", "shinobi", NULL, NULL, "1987",
	"Shinobi (set 4, System 16B, MC-8123B 317-0054)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_MC8123_ENC, GBF_PLATFORM, 0,
	NULL, Shinobi4RomInfo, Shinobi4RomName, NULL, NULL, System16bfire3InputInfo, ShinobiDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinobi5 = {
	"shinobi5", "shinobi", NULL, NULL, "1987",
	"Shinobi (set 5, System 16B, unprotected)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_PLATFORM, 0,
	NULL, Shinobi5RomInfo, Shinobi5RomName, NULL, NULL, System16bfire3InputInfo, ShinobiDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSjryuko = {
	"sjryuko", NULL, NULL, NULL, "1987",
	"Sukeban Jansi Ryuko (set 2, System 16B, FD1089B 317-5021)\0", NULL, "White Board", "System 16B",
	L"Sukeban Jansi Ryuko (set 2, System 16B, FD1089B 317-5021)\0\u30B9\u30B1\u30D0\u30F3\u96C0\u58EB \u7ADC\u5B50\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1089B_ENC | HARDWARE_SEGA_5358, GBF_MAHJONG, 0,
	NULL, SjryukoRomInfo, SjryukoRomName, NULL, NULL, SjryukoInputInfo, SjryukoDIPInfo,
	SjryukoInit, SjryukoExit, System16BFrame, NULL, SjryukoScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSnapper = {
	"snapper", NULL, NULL, NULL, "1990",
	"Snapper (Korea)\0", NULL, "Philko", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_YM2413, GBF_MISC, 0,
	NULL, SnapperRomInfo, SnapperRomName, NULL, NULL, SnapperInputInfo, SnapperDIPInfo,
	SnapperInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSonicbom = {
	"sonicbom", NULL, NULL, NULL, "1987",
	"Sonic Boom (FD1094 317-0053)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358 | HARDWARE_SEGA_FD1094_ENC, GBF_VERSHOOT, 0,
	NULL, SonicbomRomInfo, SonicbomRomName, NULL, NULL, System16bInputInfo, SonicbomDIPInfo,
	SonicbomInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvSuprleag = {
	"suprleag", NULL, NULL, NULL, "1987",
	"Super League (FD1094 317-0045)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_FD1094_ENC | HARDWARE_SEGA_5358, GBF_SPORTSMISC, 0,
	NULL, SuprleagRomInfo, SuprleagRomName, NULL, NULL, ExctleagInputInfo, ExctleagDIPInfo,
	ExctleagInit, ExctleagExit, System16BFrame, NULL, ExctleagScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTetris1 = {
	"tetris1", "tetris", NULL, NULL, "1988",
	"Tetris (set 1, Japan, System 16B, FD1094 317-0091)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358 | HARDWARE_SEGA_FD1094_ENC, GBF_PUZZLE, 0,
	NULL, Tetris1RomInfo, Tetris1RomName, NULL, NULL, System16bfire1InputInfo, TetrisDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTetris2 = {
	"tetris2", "tetris", NULL, NULL, "1988",
	"Tetris (set 2, Japan, System 16B, FD1094 317-0092)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_PUZZLE, 0,
	NULL, Tetris2RomInfo, Tetris2RomName, NULL, NULL, System16bfire1InputInfo, TetrisDIPInfo,
	System16Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTetrisbl = {
	"tetrisbl", "tetris", NULL, NULL, "1988",
	"Tetris (bootleg)\0", NULL, "bootleg", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16B, GBF_PUZZLE, 0,
	NULL, TetrisblRomInfo, TetrisblRomName, NULL, NULL, System16bfire1InputInfo, TetrisDIPInfo,
	TetrisblInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTimescan = {
	"timescan", NULL, NULL, NULL, "1987",
	"Time Scanner (set 2, System 16B)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358, GBF_PINBALL, 0,
	NULL, TimescanRomInfo, TimescanRomName, NULL, NULL, System16bDip3InputInfo, TimescanDIPInfo,
	TimescanInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvToryumon = {
	"toryumon", NULL, NULL, NULL, "1994",
	"Toryumon\0", NULL, "Sega / Westone", "System 16B",
	L"Toryumon\0\u767B\u9F8D\u9580\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5797, GBF_PUZZLE, 0,
	NULL, ToryumonRomInfo, ToryumonRomName, NULL, NULL, System16bfire1InputInfo, ToryumonDIPInfo,
	ToryumonInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTturf = {
	"tturf", NULL, NULL, NULL, "1989",
	"Tough Turf (set 2, Japan, 8751 317-0104)\0", "No Sound, Missing Z80 Program", "Sega / Sunsoft", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, TturfRomInfo, TturfRomName, NULL, NULL, System16bfire3InputInfo, TturfDIPInfo,
	TturfInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTturfu = {
	"tturfu", "tturf", NULL, NULL, "1989",
	"Tough Turf (set 1, US, 8751 317-0099)\0", NULL, "Sega / Sunsoft", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358, GBF_SCRFIGHT, 0,
	NULL, TturfuRomInfo, TturfuRomName, NULL, NULL, System16bfire3InputInfo, TturfDIPInfo,
	TturfuInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvWb3 = {
	"wb3", NULL, NULL, NULL, "1988",
	"Wonder Boy III - Monster Lair (set 5, World, System 16B, 8751 317-0098)\0", NULL, "Sega / Westone", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_PLATFORM, 0,
	NULL, Wb3RomInfo, Wb3RomName, NULL, NULL, System16bInputInfo, Wb3DIPInfo,
	Wb3Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvWb32 = {
	"wb32", "wb3", NULL, NULL, "1988",
	"Wonder Boy III - Monster Lair (set 2, Japan, System 16B, FD1094 317-0085)\0", NULL, "Sega / Westone", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5358 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, Wb32RomInfo, Wb32RomName, NULL, NULL, System16bInputInfo, Wb3DIPInfo,
	Wb32Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvWb33 = {
	"wb33", "wb3", NULL, NULL, "1988",
	"Wonder Boy III - Monster Lair (set 3, World, System 16B, FD1094 317-0089)\0", NULL, "Sega / Westone", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, Wb33RomInfo, Wb33RomName, NULL, NULL, System16bInputInfo, Wb3DIPInfo,
	Wb33Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvWb34 = {
	"wb34", "wb3", NULL, NULL, "1988",
	"Wonder Boy III - Monster Lair (set 4, Japan, System 16B, FD1094 317-0087)\0", NULL, "Sega / Westone", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_PLATFORM, 0,
	NULL, Wb34RomInfo, Wb34RomName, NULL, NULL, System16bInputInfo, Wb3DIPInfo,
	Wb33Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriverD BurnDrvWb3bbl = {
	"wb3bbl", "wb3", NULL, NULL, "1988",
	"Wonder Boy III - Monster Lair (bootleg)\0", NULL, "bootleg", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_INVERT_TILES, GBF_PLATFORM, 0,
	NULL, Wb3bblRomInfo, Wb3bblRomName, NULL, NULL, System16bInputInfo, Wb3DIPInfo,
	Wb3bblInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvWrestwar = {
	"wrestwar", NULL, NULL, NULL, "1988",
	"Wrestle War (set 3, World, 8751 317-0103)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521, GBF_VSFIGHT, 0,
	NULL, WrestwarRomInfo, WrestwarRomName, NULL, NULL, System16bInputInfo, WrestwarDIPInfo,
	WrestwarInit, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvWrestwar1 = {
	"wrestwar1", "wrestwar", NULL, NULL, "1988",
	"Wrestle War (set 1, Japan, FD1094 317-0090)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_VSFIGHT, 0,
	NULL, Wrestwar1RomInfo, Wrestwar1RomName, NULL, NULL, System16bInputInfo, WrestwarDIPInfo,
	Wrestwar1Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

struct BurnDriver BurnDrvWrestwar2 = {
	"wrestwar2", "wrestwar", NULL, NULL, "1988",
	"Wrestle War (set 2, World, FD1094 317-0102)\0", NULL, "Sega", "System 16B",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_5521 | HARDWARE_SEGA_FD1094_ENC, GBF_VSFIGHT, 0,
	NULL, Wrestwar2RomInfo, Wrestwar2RomName, NULL, NULL, System16bInputInfo, WrestwarDIPInfo,
	Wrestwar1Init, System16Exit, System16BFrame, NULL, System16Scan,
	NULL, 0x1800, 224, 320, 3, 4
};

// ISG Selection Master Type 2006 hardware

static UINT16 IsgsmCartAddrLatch;
static UINT32 IsgsmCartAddr;
static INT32 IsgsmType = 0;
static UINT32 IsgsmAddr;
static UINT8  IsgsmMode;
static UINT16 IsgsmAddrLatch;
static UINT32 IsgsmSecurity;
static UINT16 IsgsmSecurityLatch;
static UINT8 IsgsmRleControlPosition = 8;
static UINT8 IsgsmRleControlByte;
static INT32 IsgsmRleLatched;
static UINT8 IsgsmRleByte;
static UINT8 IsgsmReadXor;
static UINT32 nCartSize;
static INT32 GameRomMapped = 0;

typedef UINT32 (*isgsm_security_callback)(UINT32 input);
isgsm_security_callback IsgsmSecurityCallback;

static INT32 IsgsmTilePlaneOffsets[3] = { 0x200000, 0x100000, 0 };
static INT32 IsgsmTileXOffsets[8]     = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 IsgsmTileYOffsets[8]     = { 0, 8, 16, 24, 32, 40, 48, 56 };

static struct BurnDIPInfo ShinfzDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfc, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },
	{0x15, 0xff, 0xff, 0x00, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x13, 0x01, 0x01, 0x00, "Upright"                            },
	{0x13, 0x01, 0x01, 0x01, "Cocktail"                           },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x13, 0x01, 0x0c, 0x08, "2"                                  },
	{0x13, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x13, 0x01, 0x0c, 0x04, "4"                                  },
	{0x13, 0x01, 0x0c, 0x00, "240"                                },
	
	{0   , 0xfe, 0   , 4   , "Extra Ship Cost"                    },
	{0x13, 0x01, 0x30, 0x30, "5000"                               },
	{0x13, 0x01, 0x30, 0x20, "10000"                              },
	{0x13, 0x01, 0x30, 0x10, "15000"                              },
	{0x13, 0x01, 0x30, 0x00, "20000"                              },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"                         },
	{0x13, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x13, 0x01, 0xc0, 0xc0, "Normal"                             },
	{0x13, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x13, 0x01, 0xc0, 0x00, "Hardest"                            },
		
	// Dip 2
	
	// Dip 3
	{0   , 0xfe, 0   , 3   , "Game Select"                        },
	{0x15, 0x01, 0x03, 0x00, "Shinobi Ninja Game"                 },
	{0x15, 0x01, 0x03, 0x01, "FZ-2006 Game I"                     },
	{0x15, 0x01, 0x03, 0x02, "FZ-2006 Game II"                    },
};

STDDIPINFO(Shinfz)

static struct BurnDIPInfo TetrbxDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                                 },
	{0x14, 0xff, 0xff, 0xff, NULL                                 },
	{0x15, 0xff, 0xff, 0x00, NULL                                 },

	// Dip 1
			
	// Dip 2
	
	// Dip 3
	{0   , 0xfe, 0   , 3   , "Game Select"                        },
	{0x15, 0x01, 0x03, 0x00, "Tetris"                             },
	{0x15, 0x01, 0x03, 0x01, "Tetris II (Blox)"                   },
	{0x15, 0x01, 0x03, 0x02, "Tetris Turbo"                       },
};

STDDIPINFO(Tetrbx)

static struct BurnRomInfo emptyRomDesc[] = {
	{ "",                    0,          0, 0 },
};

static struct BurnRomInfo IsgsmRomDesc[] = {
	{ "ism2006v00.u1",  0x020000, 0x2292585c, BRF_ESS | BRF_PRG | BRF_BIOS },
};

STD_ROM_PICK(Isgsm)
STD_ROM_FN(Isgsm)

static struct BurnRomInfo ShinfzRomDesc[] = {
	{ "shin06.u13",     0x200000, 0x39d773e9, BRF_ESS | BRF_PRG },
};

STDROMPICKEXT(Shinfz, Shinfz, Isgsm)
STD_ROM_FN(Shinfz)

static struct BurnRomInfo TetrbxRomDesc[] = {
	{ "tetr06.u13",     0x080000, 0x884dd693, BRF_ESS | BRF_PRG },
};

STDROMPICKEXT(Tetrbx, Tetrbx, Isgsm)
STD_ROM_FN(Tetrbx)

static UINT32 ShinfzSecurity(UINT32 input)
{
	return BITSWAP32(input, 19, 20, 25, 26, 15, 0, 16, 2, 8, 9, 13, 14, 31, 21, 7, 18, 11, 30, 22, 17, 3, 4, 12, 28, 29, 5, 27, 10, 23, 24, 1, 6);
}

static UINT32 TetrbxSecurity(UINT32 input)
{
	return input;
}

UINT8 __fastcall IsgsmReadByte(UINT32 a)
{
	switch (a) {
		case 0xc41001: {
			return 0xff - System16Input[0];
		}
		
		case 0xc41003: {
			return 0xff - System16Input[1];
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
		
		case 0xe80001: {
			UINT32 Address;
			UINT8 Data;
			
			IsgsmCartAddr++;
			Address = (IsgsmCartAddr & (nCartSize - 1));
			Data = System16Rom[(0x100000 + Address) ^ 1] ^ IsgsmReadXor;
			
			return Data;
		}
		
		case 0xe80003: {
			return System16Dip[2];
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0xff;
}

void __fastcall IsgsmWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileByteWrite((a - 0x400000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0x3f0001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f0003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f2001: {
			if (System16TileBanks[0] != (d & 0x07)) {
				System16TileBanks[0] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0x3f2003: {
			if (System16TileBanks[1] != (d & 0x07)) {
				System16TileBanks[1] = d & 0x07;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
			return;
		}
		
		case 0xc40001: {
			System16VideoEnable = d & 0x20;
			System16ScreenFlip = d & 0x40;
			return;
		}
		
		case 0xc43001: {
			// ????
			return;
		}
		
		case 0xe00001: {
			UINT8 *pDest = 0;
			int AddressMask = 0;
			
			switch (IsgsmType & 0x0f) {
				case 0x00: {
					pDest = System16Sprites;
					AddressMask = 0x1fffff;
					break;
				}

				case 0x01: {
					pDest = System16TempGfx;
					AddressMask = 0xfffff;
					break;
				}

				case 0x02: {
					pDest = System16Z80Rom;
					AddressMask = 0x3ffff;
					break;
				}

				case 0x03: {
					pDest = System16Rom + 0x300000;
					AddressMask = 0xfffff;
					break;
				}
			}

			if ((IsgsmType & 0x10) == 0x00) {
				switch (IsgsmType & 0xe0) {
					case 0x00: d = BITSWAP08(d, 0, 7, 6, 5, 4, 3, 2, 1); break;
					case 0x20: d = BITSWAP08(d, 7, 6, 5, 4, 3, 2, 1, 0); break;
					case 0x40: d = BITSWAP08(d, 6, 5, 4, 3, 2, 1, 0, 7); break;
					case 0x60: d = BITSWAP08(d, 5, 4, 3, 2, 1, 0, 7, 6); break;
					case 0x80: d = BITSWAP08(d, 4, 3, 2, 1, 0, 7, 6, 5); break;
					case 0xa0: d = BITSWAP08(d, 3, 2, 1, 0, 7, 6, 5, 4); break;
					case 0xc0: d = BITSWAP08(d, 2, 1, 0, 7, 6, 5, 4, 3); break;
					case 0xe0: d = BITSWAP08(d, 1, 0, 7, 6, 5, 4, 3, 2); break;
				}
			}

			if (pDest) {
				INT32 BytesToWrite;
				BytesToWrite = 1;

				if (IsgsmMode & 0x04) {
					if (!IsgsmRleLatched)	{
						if (IsgsmRleControlPosition == 8) {
							IsgsmRleControlByte = d;
							IsgsmRleControlPosition = 0;
							BytesToWrite = 0;
						} else {
							if (((IsgsmRleControlByte << IsgsmRleControlPosition) & 0x80) == 0) {
								IsgsmRleByte = d;
								IsgsmRleLatched = 1;
							} else {
								BytesToWrite = 1;
							}

							IsgsmRleControlPosition++;
						}
					} else {
						IsgsmRleLatched = 0;
						BytesToWrite = d + 2;
						d = IsgsmRleByte;
					}
				}

				for (INT32 i = 0; i < BytesToWrite; i++) {
					UINT8 Byte = 0;

					if (IsgsmMode & 0x08) {
						IsgsmAddr++;
						IsgsmAddr &= 0xfffffff;
					} else {
						IsgsmAddr--;
						IsgsmAddr &= 0xfffffff;
					}
					
					switch (IsgsmMode & 0x03) {
						case 0x00: Byte = d; break;
						case 0x01: Byte = pDest[IsgsmAddr & AddressMask] ^ d; break;
						case 0x02: Byte = pDest[IsgsmAddr & AddressMask] | d; break;
						case 0x03: Byte = pDest[IsgsmAddr & AddressMask] & d; break;
					}

					if ((IsgsmType & 0x10) == 0x10) {
						switch (IsgsmType & 0xe0) {
							case 0x00: Byte = BITSWAP08(Byte, 0, 7, 6, 5, 4, 3, 2, 1); break;
							case 0x20: Byte = BITSWAP08(Byte, 7, 6, 5, 4, 3, 2, 1, 0); break;
							case 0x40: Byte = BITSWAP08(Byte, 6, 5, 4, 3, 2, 1, 0, 7); break;
							case 0x60: Byte = BITSWAP08(Byte, 5, 4, 3, 2, 1, 0, 7, 6); break;
							case 0x80: Byte = BITSWAP08(Byte, 4, 3, 2, 1, 0, 7, 6, 5); break;
							case 0xa0: Byte = BITSWAP08(Byte, 3, 2, 1, 0, 7, 6, 5, 4); break;
							case 0xc0: Byte = BITSWAP08(Byte, 2, 1, 0, 7, 6, 5, 4, 3); break;
							case 0xe0: Byte = BITSWAP08(Byte, 1, 0, 7, 6, 5, 4, 3, 2); break;
						}
					}
					
					if ((IsgsmType & 0x0f) == 0x01) {
						if (IsgsmAddr < System16TileRomSize) {
							pDest[IsgsmAddr] = Byte;
							GfxDecodeSingle((IsgsmAddr & 0x1ffff) / 8, 3, 8, 8, IsgsmTilePlaneOffsets, IsgsmTileXOffsets, IsgsmTileYOffsets, 0x40, System16TempGfx, System16Tiles);
						}
					} else {
						pDest[IsgsmAddr & AddressMask] = Byte;
					}
				}
			}
			
			return;
		}
		
		case 0xe00003: {
			IsgsmType = d;
			return;
		}
		
		case 0xfe0007: {
			System16SoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
		
		case 0xfe0009: {
			if (d == 0) {
				ZetOpen(0);
				ZetReset();
				ZetClose();
				System16Z80Enable = true;
			}
			
			if (d == 1) {
				ZetOpen(0);
				ZetReset();
				ZetClose();
				System16Z80Enable = false;
			}
			
			return;
		}
		
		case 0xfe000b: {
			SekMapMemory(System16Rom + 0x300000, 0x000000, 0x0fffff, SM_ROM);
			GameRomMapped = 1;
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

UINT16 __fastcall IsgsmReadWord(UINT32 a)
{
	switch (a) {
		case 0xe80008: {
			return (IsgsmSecurity >> 16) & 0xffff;
		}
		
		case 0xe8000a: {
			return IsgsmSecurity & 0xffff;
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Word -> 0x%06X\n"), a);
#endif

	return 0xffff;
}

void __fastcall IsgsmWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x400000 && a <= 0x40ffff) {
		System16BTileWordWrite(a - 0x400000, d);
		return;
	}
	
	switch (a) {
		case 0xe00004: {
			IsgsmAddrLatch = d;
			return;
		}
		
		case 0xe00006: {
			IsgsmMode = (IsgsmAddrLatch & 0xf000) >> 12;
			IsgsmAddr = d | ((IsgsmAddrLatch & 0x0fff) << 16);

			IsgsmRleControlPosition = 8;
			IsgsmRleControlByte = 0;
			IsgsmRleLatched = 0;
			return;
		}
		
		case 0xe80004: {
			IsgsmCartAddrLatch = d;
			return;
		}
		
		case 0xe80006: {
			IsgsmCartAddr = d | IsgsmCartAddrLatch << 16;
			return;
		}
		
		case 0xe80008: {
			IsgsmSecurityLatch = d;
			return;
		}
		
		case 0xe8000a: {
			IsgsmSecurity = d | IsgsmSecurityLatch << 16;
			if (IsgsmSecurityCallback) IsgsmSecurity = IsgsmSecurityCallback(IsgsmSecurity);
			return;
		}
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

static void IsgsmMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom            , 0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(System16ExtraRam       , 0x200000, 0x23ffff, SM_RAM);
	SekMapMemory(System16TileRam        , 0x400000, 0x40ffff, SM_READ);
	SekMapMemory(System16TextRam        , 0x410000, 0x410fff, SM_RAM);
	SekMapMemory(System16SpriteRam      , 0x440000, 0x4407ff, SM_RAM);
	SekMapMemory(System16PaletteRam     , 0x840000, 0x840fff, SM_RAM);
	SekMapMemory(System16Rom + 0x100000 , 0xee0000, 0xefffff, SM_ROM);
	SekMapMemory(System16Ram            , 0xffc000, 0xffffff, SM_RAM);
	SekSetReadWordHandler(0, IsgsmReadWord);
	SekSetWriteWordHandler(0, IsgsmWriteWord);
	SekSetReadByteHandler(0, IsgsmReadByte);
	SekSetWriteByteHandler(0, IsgsmWriteByte);
	SekClose();
}

static INT32 IsgsmInit()
{
	System16RomSize        = 0x400000;
	System16TileRomSize    = 0x60000;
	System16SpriteRomSize  = 0x200000;
	System16UPD7759DataSize = 0x30000;
	
	System16Map68KDo = IsgsmMap68K;
	
	INT32 nRet = System16Init();
	
	if (!nRet) {
		memset(System16Rom, 0, 0x400000);
		
		// Load and Decrypt BIOS
		UINT16 *pTemp = (UINT16*)BurnMalloc(0x20000);
		memset(pTemp, 0, 0x20000);
		UINT16 *Rom = (UINT16*)System16Rom;
		
		nRet = BurnLoadRom(System16Rom, 0x80, 1); if (nRet) return 1;
		
		for (UINT32 i = 0; i < 0x10000; i++) {
			pTemp[i ^ 0x4127] = BITSWAP16(Rom[i], 6, 14, 4, 2, 12, 10, 8, 0, 1, 9, 11, 13, 3, 5, 7, 15);
		}
		
		memcpy(Rom, pTemp, 0x20000);
		BurnFree(pTemp);
		
		// Load program ROM
		nRet = BurnLoadRom(System16Rom + 0x100000, 0, 1); if (nRet) return 1;
		
		System16TempGfx = (UINT8*)BurnMalloc(System16TileRomSize);
		memset(System16TempGfx, 0, System16TileRomSize);
		memset(System16Tiles, 0, System16NumTiles * 8 * 8);
		memset(System16Sprites, 0, System16TileRomSize);
		
		System16UPD7759Data = (UINT8*)(System16Z80Rom + 0x10000);
	}
	
	System16ClockSpeed = 16000000;

	return nRet;
}

static INT32 ShinfzInit()
{
	INT32 nRet = IsgsmInit();
	
	if (!nRet) {
		nCartSize = 0x200000;
		UINT16 *pTemp = (UINT16*)BurnMalloc(0x200000);
		memset(pTemp, 0, 0x200000);
		UINT16 *Rom = (UINT16*)(System16Rom + 0x100000);
		
		for (UINT32 i = 0; i < 0x100000; i++) {
			pTemp[i ^ 0x68956] = BITSWAP16(Rom[i], 8, 4, 12, 3, 6, 7, 1, 0, 15, 11, 5, 14, 10, 2, 9, 13);
		}
		
		memcpy(Rom, pTemp, 0x200000);
		BurnFree(pTemp);
		
		IsgsmReadXor = 0x66;
		IsgsmSecurityCallback = ShinfzSecurity;
	}

	return nRet;
}

static INT32 TetrbxInit()
{
	INT32 nRet = IsgsmInit();
	
	if (!nRet) {
		nCartSize = 0x80000;
		UINT16 *pTemp = (UINT16*)BurnMalloc(nCartSize);
		memset(pTemp, 0, nCartSize);
		UINT16 *Rom = (UINT16*)(System16Rom + 0x100000);
		
		for (UINT32 i = 0; i < nCartSize >> 1; i++) {
			pTemp[i ^ 0x2a6e6] = BITSWAP16(Rom[i], 4, 0, 12, 5, 7, 3, 1, 14, 10, 11, 9, 6, 15, 2, 13, 8);
		}
		
		memcpy(Rom, pTemp, nCartSize);
		BurnFree(pTemp);
		
		IsgsmReadXor = 0x73;
		IsgsmSecurityCallback = TetrbxSecurity;
	}

	return nRet;
}

static INT32 IsgsmExit()
{
	INT32 nRet = System16Exit();
	
	BurnFree(System16TempGfx);
	
	IsgsmCartAddrLatch = 0;
	IsgsmCartAddr = 0;
	IsgsmType = 0;
	IsgsmAddr = 0;
	IsgsmMode = 0;
	IsgsmAddrLatch = 0;
	IsgsmSecurity = 0;
	IsgsmSecurityLatch = 0;
	IsgsmRleControlPosition = 0;
	IsgsmRleControlByte = 0;
	IsgsmRleLatched = 0;
	IsgsmRleByte = 0;
	IsgsmReadXor = 0;
	nCartSize = 0;
	IsgsmSecurityCallback = NULL;
	GameRomMapped = 0;
	
	return nRet;
}

static INT32 IsgsmScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {
		*pnMin =  0x029719;
	}
	
	struct BurnArea ba;
	
	if (nAction & ACB_DRIVER_DATA) {
		memset(&ba, 0, sizeof(ba));
		ba.Data		= System16Sprites;
		ba.nLen		= System16SpriteRomSize - 1;
		ba.nAddress = 0;
		ba.szName	= "SpriteROM";
		BurnAcb(&ba);
		
		memset(&ba, 0, sizeof(ba));
		ba.Data		= System16TempGfx;
		ba.nLen		= System16TileRomSize - 1;
		ba.nAddress = 0;
		ba.szName	= "TileROM";
		BurnAcb(&ba);
		
		memset(&ba, 0, sizeof(ba));
		ba.Data		= System16Z80Rom;
		ba.nLen		= 0x3ffff;
		ba.nAddress = 0;
		ba.szName	= "Z80ROM";
		BurnAcb(&ba);
		
		memset(&ba, 0, sizeof(ba));
		ba.Data		= System16Rom + 0x300000;
		ba.nLen		= 0xfffff;
		ba.nAddress = 0;
		ba.szName	= "GameROM";
		BurnAcb(&ba);
		
		SCAN_VAR(IsgsmCartAddrLatch);
		SCAN_VAR(IsgsmCartAddr);
		SCAN_VAR(IsgsmType);
		SCAN_VAR(IsgsmAddr);
		SCAN_VAR(IsgsmMode);
		SCAN_VAR(IsgsmAddrLatch);
		SCAN_VAR(IsgsmSecurity);
		SCAN_VAR(IsgsmSecurityLatch);
		SCAN_VAR(IsgsmRleControlPosition);
		SCAN_VAR(IsgsmRleControlByte);
		SCAN_VAR(IsgsmRleLatched);
		SCAN_VAR(IsgsmRleByte);
		SCAN_VAR(GameRomMapped);
		
		if (nAction & ACB_WRITE) {
			if (GameRomMapped) {
				SekOpen(0);
				SekMapMemory(System16Rom + 0x300000, 0x000000, 0x0fffff, SM_ROM);
				SekClose();
			}
			
			for (UINT32 i = 0; i < System16TileRomSize; i++) {
				GfxDecodeSingle((i & 0x1ffff) / 8, 3, 8, 8, IsgsmTilePlaneOffsets, IsgsmTileXOffsets, IsgsmTileYOffsets, 0x40, System16TempGfx, System16Tiles);
			}
		}
	}

	return System16Scan(nAction, pnMin);
}

struct BurnDriver BurnDrvIsgsm = {
	"isgsm", NULL, NULL, NULL, "2006",
	"ISG Selection Master Type 2006 System BIOS\0", "BIOS only", "ISG", "ISG Selection Master Type 2006",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_BOARDROM, 0, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_ISGSM | HARDWARE_SEGA_5521, GBF_BIOS, 0,
	NULL, IsgsmRomInfo, IsgsmRomName, NULL, NULL, System16bDip3InputInfo, NULL,
	IsgsmInit, IsgsmExit, System16BFrame, NULL, IsgsmScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvShinfz = {
	"shinfz", NULL, "isgsm", NULL, "2006",
	"Shinobi / FZ-2006 (Korean System 16 bootleg) (ISG Selection Master Type 2006)\0", NULL, "ISG", "ISG Selection Master Type 2006",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_ISGSM | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, ShinfzRomInfo, ShinfzRomName, NULL, NULL, System16bDip3InputInfo, ShinfzDIPInfo,
	ShinfzInit, IsgsmExit, System16BFrame, NULL, IsgsmScan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvTetrbx = {
	"tetrbx", NULL, "isgsm", NULL, "2006",
	"Tetris / Bloxeed (Korean System 16 bootleg) (ISG Selection Master Type 2006)\0", NULL, "ISG", "ISG Selection Master Type 2006",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_SYSTEM16B | HARDWARE_SEGA_ISGSM | HARDWARE_SEGA_5521, GBF_SCRFIGHT, 0,
	NULL, TetrbxRomInfo, TetrbxRomName, NULL, NULL, System16bDip3InputInfo, TetrbxDIPInfo,
	TetrbxInit, IsgsmExit, System16BFrame, NULL, IsgsmScan,
	NULL, 0x1800, 320, 224, 4, 3
};
