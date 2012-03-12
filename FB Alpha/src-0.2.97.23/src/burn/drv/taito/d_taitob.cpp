// FB Alpha Taito B System driver module
// Based on MAME driver by Jarek Burczynski and various others

// to do:
//	fix rambo alt sets inputs
//	master of weapon title screen is incorrect

#include "tiles_generic.h"
#include "taito.h"
#include "taito_ic.h"
#include "burn_ym2610.h"
#include "msm6295.h"
#include "burn_ym2203.h"
#include "burn_gun.h"
#include "eeprom.h"

static UINT8  *DrvPxlRAM	= NULL;
static UINT16 *DrvPxlScroll	= NULL;
static UINT8  *DrvFramebuffer	= NULL;

static INT32 eeprom_latch = 0;
static INT32 coin_control = 0;

static UINT8 color_config[4];
static INT32 irq_config[2];
static INT32 sound_config = 0;
static INT32 cpu_speed[2];
static UINT8 nTaitoInputConfig[5] = { 0, 0, 0, 0, 0 };

static struct BurnInputInfo CommonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TC0220IOCInputPort0 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TC0220IOCInputPort0 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TC0220IOCInputPort0 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TC0220IOCInputPort0 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort2 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,			"reset"		},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	TC0220IOCDip + 1,		"dip"		},
};

STDINPUTINFO(Common)

static struct BurnInputInfo PbobbleInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TaitoInputPort3 + 0,		"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TaitoInputPort3 + 1,		"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TaitoInputPort3 + 2,		"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TaitoInputPort3 + 3,		"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TaitoInputPort3 + 4,		"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TaitoInputPort3 + 5,		"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TaitoInputPort3 + 6,		"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TaitoInputPort3 + 7,		"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p2 fire 3"	},

	{"P3 Coin",		BIT_DIGITAL,	TC0220IOCInputPort0 + 6,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 6,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	TaitoInputPort5 + 0,		"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	TaitoInputPort5 + 1,		"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	TaitoInputPort5 + 2,		"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	TaitoInputPort5 + 3,		"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	TaitoInputPort4 + 0,		"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	TaitoInputPort4 + 1,		"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	TaitoInputPort4 + 2,		"p3 fire 3"	},

	{"P4 Coin",		BIT_DIGITAL,	TC0220IOCInputPort0 + 7,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 7,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	TaitoInputPort5 + 4,		"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	TaitoInputPort5 + 5,		"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	TaitoInputPort5 + 6,		"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	TaitoInputPort5 + 7,		"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	TaitoInputPort4 + 4,		"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	TaitoInputPort4 + 5,		"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	TaitoInputPort4 + 6,		"p4 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,			"reset"		},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"service"	},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"service"	},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
};

STDINPUTINFO(Pbobble)

static struct BurnInputInfo QzshowbyInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	TaitoInputPort3 + 0,		"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TaitoInputPort3 + 1,		"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	TaitoInputPort3 + 3,		"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	TaitoInputPort3 + 2,		"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	TaitoInputPort3 + 4,		"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TaitoInputPort3 + 5,		"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	TaitoInputPort3 + 7,		"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	TaitoInputPort3 + 6,		"p2 fire 4"	},

	{"P3 Coin",		BIT_DIGITAL,	TC0220IOCInputPort0 + 6,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 6,	"p3 start"	},
	{"P3 Button 1",		BIT_DIGITAL,	TaitoInputPort5 + 8,		"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	TaitoInputPort5 + 9,		"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	TaitoInputPort5 + 11,		"p3 fire 3"	},
	{"P3 Button 4",		BIT_DIGITAL,	TaitoInputPort5 + 10,		"p3 fire 4"	},

	{"P4 Coin",		BIT_DIGITAL,	TC0220IOCInputPort0 + 7,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 7,	"p4 start"	},
	{"P4 Button 1",		BIT_DIGITAL,	TaitoInputPort5 + 12,		"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	TaitoInputPort5 + 13,		"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	TaitoInputPort5 + 15,		"p4 fire 3"	},
	{"P4 Button 4",		BIT_DIGITAL,	TaitoInputPort5 + 14,		"p4 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,			"reset"		},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"service"	},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"service"	},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
};

STDINPUTINFO(Qzshowby)

static struct BurnInputInfo SpacedxoInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 2,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 7,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p2 fire 3"	},

	{"P3 Coin",		BIT_DIGITAL,	TaitoInputPort5 + 0,		"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	TaitoInputPort3 + 0,		"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	TaitoInputPort3 + 3,		"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	TaitoInputPort3 + 4,		"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	TaitoInputPort3 + 1,		"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	TaitoInputPort3 + 2,		"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	TaitoInputPort3 + 5,		"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	TaitoInputPort3 + 6,		"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	TaitoInputPort3 + 7,		"p3 fire 3"	},

	{"P4 Coin",		BIT_DIGITAL,	TaitoInputPort5 + 2,		"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	TaitoInputPort4 + 0,		"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	TaitoInputPort4 + 3,		"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	TaitoInputPort4 + 4,		"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	TaitoInputPort4 + 1,		"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	TaitoInputPort4 + 2,		"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	TaitoInputPort4 + 5,		"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	TaitoInputPort4 + 6,		"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	TaitoInputPort4 + 7,		"p4 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,			"reset"		},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"service"	},
	{"Service",		BIT_DIGITAL,	TaitoInputPort5 + 1,		"service"	},
	{"Service",		BIT_DIGITAL,	TaitoInputPort5 + 3,		"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	TC0220IOCDip + 1,		"dip"		},
};

STDINPUTINFO(Spacedxo)

static struct BurnInputInfo SelfeenaInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 2,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 7,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,	"reset"},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	TC0220IOCDip + 1,		"dip"		},
};

STDINPUTINFO(Selfeena)

static struct BurnInputInfo SbmInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TC0220IOCInputPort0 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TC0220IOCInputPort0 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TC0220IOCInputPort0 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TC0220IOCInputPort0 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	TC0220IOCInputPort2 + 7,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort2 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TC0220IOCInputPort0 + 6,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TC0220IOCInputPort0 + 7,	"p2 right"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,			"reset"		},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	TC0220IOCDip + 1,		"dip"		},
};

STDINPUTINFO(Sbm)

static struct BurnInputInfo SilentdInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 2,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 7,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p2 fire 3"	},

	{"P3 Coin",		BIT_DIGITAL,	TaitoInputPort5 + 0,		"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	TaitoInputPort3 + 0,		"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	TaitoInputPort3 + 3,		"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	TaitoInputPort3 + 4,		"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	TaitoInputPort3 + 1,		"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	TaitoInputPort3 + 2,		"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	TaitoInputPort3 + 5,		"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	TaitoInputPort3 + 6,		"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	TaitoInputPort3 + 7,		"p3 fire 3"	},

	{"P4 Coin",		BIT_DIGITAL,	TaitoInputPort5 + 2,		"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	TaitoInputPort4 + 0,		"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	TaitoInputPort4 + 3,		"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	TaitoInputPort4 + 4,		"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	TaitoInputPort4 + 1,		"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	TaitoInputPort4 + 2,		"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	TaitoInputPort4 + 5,		"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	TaitoInputPort4 + 6,		"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	TaitoInputPort4 + 7,		"p4 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,			"reset"		},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	TC0220IOCDip + 1,		"dip"		},
};

STDINPUTINFO(Silentd)

static struct BurnInputInfo ViofightInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TC0220IOCInputPort0 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TC0220IOCInputPort0 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TC0220IOCInputPort0 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TC0220IOCInputPort0 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort2 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,	"reset"},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	TC0220IOCDip + 1,		"dip"		},
};

STDINPUTINFO(Viofight)

static struct BurnInputInfo HiticeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 2,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 7,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	TC0220IOCInputPort0 + 5,	"p2 fire 3"	},

	{"P3 Start",		BIT_DIGITAL,	TaitoInputPort3 + 7,		"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	TaitoInputPort3 + 0,		"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	TaitoInputPort3 + 1,		"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	TaitoInputPort3 + 2,		"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	TaitoInputPort3 + 3,		"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	TaitoInputPort3 + 4,		"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	TaitoInputPort3 + 5,		"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	TaitoInputPort3 + 6,		"p3 fire 3"	},

	{"P4 Start",		BIT_DIGITAL,	TaitoInputPort4 + 7,		"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	TaitoInputPort4 + 0,		"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	TaitoInputPort4 + 1,		"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	TaitoInputPort4 + 2,		"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	TaitoInputPort4 + 3,		"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	TaitoInputPort4 + 4,		"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	TaitoInputPort4 + 5,		"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	TaitoInputPort4 + 6,		"p4 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&TaitoReset,			"reset"		},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	TC0220IOCDip + 1,		"dip"		},
};

STDINPUTINFO(Hitice)

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo Rambo3uInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 1,	"p1 fire 2"	},

	A("P1 Trackball X",     BIT_ANALOG_REL, &TaitoAnalogPort0,		"p1 x-axis"	),
	A("P1 Trackball Y",     BIT_ANALOG_REL, &TaitoAnalogPort1,		"p1 y-axis"	),

	{"P2 Coin",		BIT_DIGITAL,	TC0220IOCInputPort1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	TC0220IOCInputPort1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	TC0220IOCInputPort2 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	TC0220IOCInputPort2 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	TC0220IOCInputPort2 + 6,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	TC0220IOCInputPort2 + 7,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	TC0220IOCInputPort0 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	TC0220IOCInputPort0 + 4,	"p2 fire 2"	},

	A("P2 Trackball X",	BIT_ANALOG_REL, &TaitoAnalogPort2,		"p2 x-axis"	),
	A("P2 Trackball Y",	BIT_ANALOG_REL, &TaitoAnalogPort3,		"p2 y-axis"	),

	{"Reset",		BIT_DIGITAL,	&TaitoReset,	"reset"},
	{"Service",		BIT_DIGITAL,	TC0220IOCInputPort1 + 1,	"service"	},
	{"Tilt",		BIT_DIGITAL,	TC0220IOCInputPort1 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	TC0220IOCDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	TC0220IOCDip + 1,		"dip"		},
};

STDINPUTINFO(Rambo3u)

#undef A

static struct BurnDIPInfo NastarDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin A"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "100k only"		},
	{0x14, 0x01, 0x0c, 0x08, "150k only"		},
	{0x14, 0x01, 0x0c, 0x04, "200k only"		},
	{0x14, 0x01, 0x0c, 0x00, "250k only"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x20, "1"			},
	{0x14, 0x01, 0x30, 0x10, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Nastar)

static struct BurnDIPInfo Rastsag2DIPList[]=
{
	{0x13, 0xff, 0xff, 0x3f, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin A"		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "100k only"		},
	{0x14, 0x01, 0x0c, 0x08, "150k only"		},
	{0x14, 0x01, 0x0c, 0x04, "200k only"		},
	{0x14, 0x01, 0x0c, 0x00, "250k only"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x20, "1"			},
	{0x14, 0x01, 0x30, 0x10, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Rastsag2)

static struct BurnDIPInfo NastarwDIPList[]=
{
	{0x13, 0xff, 0xff, 0x3e, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x00, "Upright"		},
	{0x13, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "100k only"		},
	{0x14, 0x01, 0x0c, 0x08, "150k only"		},
	{0x14, 0x01, 0x0c, 0x04, "200k only"		},
	{0x14, 0x01, 0x0c, 0x00, "250k only"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x20, "1"			},
	{0x14, 0x01, 0x30, 0x10, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x00, "5"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Nastarw)

static struct BurnDIPInfo AshuraDIPList[]=
{
	{0x13, 0xff, 0xff, 0xfe, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x00, "Upright"		},
	{0x13, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x08, "every 100k"		},
	{0x14, 0x01, 0x0c, 0x0c, "every 150k"		},
	{0x14, 0x01, 0x0c, 0x04, "every 200k"		},
	{0x14, 0x01, 0x0c, 0x00, "every 250k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x00, "1"			},
	{0x14, 0x01, 0x30, 0x10, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x20, "4"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Ashura)

static struct BurnDIPInfo AshurajDIPList[]=
{
	{0x13, 0xff, 0xff, 0xfe, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x00, "Upright"		},
	{0x13, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x08, "every 100k"		},
	{0x14, 0x01, 0x0c, 0x0c, "every 150k"		},
	{0x14, 0x01, 0x0c, 0x04, "every 200k"		},
	{0x14, 0x01, 0x0c, 0x00, "every 250k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x00, "1"			},
	{0x14, 0x01, 0x30, 0x10, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x20, "4"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Ashuraj)

static struct BurnDIPInfo AshurauDIPList[]=
{
	{0x13, 0xff, 0xff, 0xfe, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x00, "Upright"		},
	{0x13, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Continue Price"	},
	{0x13, 0x01, 0xc0, 0x00, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xc0, 0xc0, "Same as Start"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x08, "every 100k"		},
	{0x14, 0x01, 0x0c, 0x0c, "every 150k"		},
	{0x14, 0x01, 0x0c, 0x04, "every 200k"		},
	{0x14, 0x01, 0x0c, 0x00, "every 250k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x00, "1"			},
	{0x14, 0x01, 0x30, 0x10, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x20, "4"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Ashurau)

static struct BurnDIPInfo CrimecDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Hi Score"		},
	{0x13, 0x01, 0x01, 0x01, "Scribble"		},
	{0x13, 0x01, 0x01, 0x00, "3 Characters"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x08, "every 80k"		},
	{0x14, 0x01, 0x0c, 0x0c, "80k only"		},
	{0x14, 0x01, 0x0c, 0x04, "160k only"		},
	{0x14, 0x01, 0x0c, 0x00, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x10, "1"			},
	{0x14, 0x01, 0x30, 0x20, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"	},
	{0x14, 0x01, 0xc0, 0x00, "Off"			},
	{0x14, 0x01, 0xc0, 0x40, "5 Times"		},
	{0x14, 0x01, 0xc0, 0x80, "8 Times"		},
	{0x14, 0x01, 0xc0, 0xc0, "On"			},
};

STDDIPINFO(Crimec)

static struct BurnDIPInfo CrimecjDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Hi Score"		},
	{0x13, 0x01, 0x01, 0x01, "Scribble"		},
	{0x13, 0x01, 0x01, 0x00, "3 Characters"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x08, "every 80k"		},
	{0x14, 0x01, 0x0c, 0x0c, "80k only"		},
	{0x14, 0x01, 0x0c, 0x04, "160k only"		},
	{0x14, 0x01, 0x0c, 0x00, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x10, "1"			},
	{0x14, 0x01, 0x30, 0x20, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"	},
	{0x14, 0x01, 0xc0, 0x00, "Off"			},
	{0x14, 0x01, 0xc0, 0x40, "5 Times"		},
	{0x14, 0x01, 0xc0, 0x80, "8 Times"		},
	{0x14, 0x01, 0xc0, 0xc0, "On"			},
};

STDDIPINFO(Crimecj)

static struct BurnDIPInfo CrimecuDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Hi Score"		},
	{0x13, 0x01, 0x01, 0x01, "Scribble"		},
	{0x13, 0x01, 0x01, 0x00, "3 Characters"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Price to Continue"	},
	{0x13, 0x01, 0xc0, 0x00, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xc0, 0xc0, "Same as Start"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x08, "every 80k"		},
	{0x14, 0x01, 0x0c, 0x0c, "80k only"		},
	{0x14, 0x01, 0x0c, 0x04, "160k only"		},
	{0x14, 0x01, 0x0c, 0x00, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x10, "1"			},
	{0x14, 0x01, 0x30, 0x20, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"	},
	{0x14, 0x01, 0xc0, 0x00, "Off"			},
	{0x14, 0x01, 0xc0, 0x40, "5 Times"		},
	{0x14, 0x01, 0xc0, 0x80, "8 Times"		},
	{0x14, 0x01, 0xc0, 0xc0, "On"			},
};

STDDIPINFO(Crimecu)

static struct BurnDIPInfo TetristDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	},
};

STDDIPINFO(Tetrist)

static struct BurnDIPInfo Rambo3DIPList[]=
{
	{0x13, 0xff, 0xff, 0xfe, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x10, 0x00, "Off"			},
	{0x14, 0x01, 0x10, 0x10, "On"			},
};

STDDIPINFO(Rambo3)

static struct BurnDIPInfo MasterwDIPList[]=
{
	{0x13, 0xff, 0xff, 0xfe, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x00, "Upright"		},
	{0x13, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x08, "500k, 1000k and 1500k"},
	{0x14, 0x01, 0x0c, 0x0c, "500k and 1000k"	},
	{0x14, 0x01, 0x0c, 0x04, "500k only"		},
	{0x14, 0x01, 0x0c, 0x00, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x20, "4"			},
	{0x14, 0x01, 0x30, 0x10, "5"			},
	{0x14, 0x01, 0x30, 0x00, "6"			},

	{0   , 0xfe, 0   ,    2, "Ship Type"		},
	{0x14, 0x01, 0x80, 0x80, "Space Ship"		},
	{0x14, 0x01, 0x80, 0x00, "Hover Cycle"		},
};

STDDIPINFO(Masterw)

static struct BurnDIPInfo PbobbleDIPList[]=
{
	{0x29, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x29, 0x01, 0x80, 0x80, "Off"			},
	{0x29, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Pbobble)

static struct BurnDIPInfo QzshowbyDIPList[]=
{
	{0x1d, 0xff, 0xff, 0x80, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode "	},
	{0x1d, 0x01, 0x80, 0x80, "Off"			},
	{0x1d, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Qzshowby)

static struct BurnDIPInfo SpacedxoDIPList[]=
{
	{0x29, 0xff, 0xff, 0xfe, NULL			},
	{0x2a, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x29, 0x01, 0x01, 0x00, "Upright"		},
	{0x29, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x29, 0x01, 0x02, 0x02, "Off"			},
	{0x29, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x29, 0x01, 0x04, 0x04, "Off"			},
	{0x29, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x29, 0x01, 0x08, 0x00, "Off"			},
	{0x29, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x29, 0x01, 0x30, 0x00, "3 Coins 1 Credits"	},
	{0x29, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x29, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x29, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x29, 0x01, 0xc0, 0x00, "3 Coins 1 Credits"	},
	{0x29, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x29, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x29, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x2a, 0x01, 0x03, 0x02, "Easy"			},
	{0x2a, 0x01, 0x03, 0x03, "Medium"		},
	{0x2a, 0x01, 0x03, 0x01, "Hard"			},
	{0x2a, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Match Point"		},
	{0x2a, 0x01, 0x0c, 0x08, "4"			},
	{0x2a, 0x01, 0x0c, 0x0c, "3"			},
	{0x2a, 0x01, 0x0c, 0x04, "5"			},
	{0x2a, 0x01, 0x0c, 0x00, "2"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x2a, 0x01, 0x30, 0x30, "3"			},
	{0x2a, 0x01, 0x30, 0x20, "4"			},
	{0x2a, 0x01, 0x30, 0x10, "5"			},
	{0x2a, 0x01, 0x30, 0x00, "6"			},

	{0   , 0xfe, 0   ,    2, "Bonus Life"		},
	{0x2a, 0x01, 0x40, 0x40, "1500 Points"		},
	{0x2a, 0x01, 0x40, 0x00, "1000 Points"		},

	{0   , 0xfe, 0   ,    2, "Game Type"		},
	{0x2a, 0x01, 0x80, 0x80, "Double Company"	},
	{0x2a, 0x01, 0x80, 0x00, "Single Company"	},
};

STDDIPINFO(Spacedxo)

static struct BurnDIPInfo SelfeenaDIPList[]=
{
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x15, 0x01, 0x02, 0x02, "Off"			},
	{0x15, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Service Mode"		},
	{0x15, 0x01, 0x04, 0x04, "Off"			},
	{0x15, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x08, 0x00, "Off"			},
	{0x15, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin A"		},
	{0x15, 0x01, 0x30, 0x00, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x15, 0x01, 0xc0, 0x00, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x03, 0x02, "Easy"			},
	{0x16, 0x01, 0x03, 0x03, "Medium"		},
	{0x16, 0x01, 0x03, 0x01, "Hard"			},
	{0x16, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x16, 0x01, 0x0c, 0x0c, "100k only"		},
	{0x16, 0x01, 0x0c, 0x08, "200k only"		},
	{0x16, 0x01, 0x0c, 0x04, "300k only"		},
	{0x16, 0x01, 0x0c, 0x00, "400k only"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x16, 0x01, 0x30, 0x00, "1"			},
	{0x16, 0x01, 0x30, 0x10, "2"			},
	{0x16, 0x01, 0x30, 0x30, "3"			},
	{0x16, 0x01, 0x30, 0x20, "4"			},
};

STDDIPINFO(Selfeena)

static struct BurnDIPInfo RyujinDIPList[]=
{
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x02, 0x02, "Off"			},
	{0x15, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x15, 0x01, 0x04, 0x04, "Off"			},
	{0x15, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x08, 0x00, "Off"			},
	{0x15, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin A"		},
	{0x15, 0x01, 0x30, 0x00, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x15, 0x01, 0xc0, 0x00, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x03, 0x02, "Easy"			},
	{0x16, 0x01, 0x03, 0x03, "Medium"		},
	{0x16, 0x01, 0x03, 0x01, "Hard"			},
	{0x16, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x16, 0x01, 0x0c, 0x00, "1"			},
	{0x16, 0x01, 0x0c, 0x04, "2"			},
	{0x16, 0x01, 0x0c, 0x0c, "3"			},
	{0x16, 0x01, 0x0c, 0x08, "4"			},
};

STDDIPINFO(Ryujin)

static struct BurnDIPInfo SbmDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xfc, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x03, "Medium"		},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},
};

STDDIPINFO(Sbm)

static struct BurnDIPInfo SilentdDIPList[]=
{
	{0x27, 0xff, 0xff, 0xff, NULL			},
	{0x28, 0xff, 0xff, 0xbf, NULL			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x27, 0x01, 0x02, 0x02, "Off"			},
	{0x27, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Service Mode"		},
	{0x27, 0x01, 0x04, 0x04, "Off"			},
	{0x27, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x27, 0x01, 0x08, 0x00, "Off"			},
	{0x27, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Coinage"		},
	{0x27, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x27, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x27, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x27, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x28, 0x01, 0x03, 0x02, "Easy"			},
	{0x28, 0x01, 0x03, 0x03, "Medium"		},
	{0x28, 0x01, 0x03, 0x01, "Hard"			},
	{0x28, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Invulnerability"	},
	{0x28, 0x01, 0x04, 0x04, "Off"			},
	{0x28, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Power-Up Bonus"	},
	{0x28, 0x01, 0x08, 0x08, "Off"			},
	{0x28, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Regain Power"		},
	{0x28, 0x01, 0x10, 0x10, "Off"			},
	{0x28, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Credits"		},
	{0x28, 0x01, 0x20, 0x20, "Combined"		},
	{0x28, 0x01, 0x20, 0x00, "Separate"		},

	{0   , 0xfe, 0   ,    2, "Cabinet Style"	},
	{0x28, 0x01, 0xc0, 0xc0, "3 Players"		},
	{0x28, 0x01, 0xc0, 0x80, "2 Players"		},
	{0x28, 0x01, 0xc0, 0x40, "4 Players/1 Machine"	},
	{0x28, 0x01, 0xc0, 0x00, "4 Players/2 Machines"	},
};

STDDIPINFO(Silentd)

static struct BurnDIPInfo SilentdjDIPList[]=
{
	{0x27, 0xff, 0xff, 0xff, NULL			},
	{0x28, 0xff, 0xff, 0xbf, NULL			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x27, 0x01, 0x02, 0x02, "Off"			},
	{0x27, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Service Mode"		},
	{0x27, 0x01, 0x04, 0x04, "Off"			},
	{0x27, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x27, 0x01, 0x08, 0x00, "Off"			},
	{0x27, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin A"		},
	{0x27, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x27, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x27, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x27, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x27, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x27, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x27, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x27, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x28, 0x01, 0x03, 0x02, "Easy"			},
	{0x28, 0x01, 0x03, 0x03, "Medium"		},
	{0x28, 0x01, 0x03, 0x01, "Hard"			},
	{0x28, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Invulnerability"	},
	{0x28, 0x01, 0x04, 0x04, "Off"			},
	{0x28, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Power-Up Bonus"	},
	{0x28, 0x01, 0x08, 0x08, "Off"			},
	{0x28, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Regain Power"		},
	{0x28, 0x01, 0x10, 0x10, "Off"			},
	{0x28, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Credits"		},
	{0x28, 0x01, 0x20, 0x20, "Combined"		},
	{0x28, 0x01, 0x20, 0x00, "Separate"		},

	{0   , 0xfe, 0   ,    2, "Cabinet Style"	},
	{0x28, 0x01, 0xc0, 0xc0, "3 Players"		},
	{0x28, 0x01, 0xc0, 0x80, "2 Players"		},
	{0x28, 0x01, 0xc0, 0x40, "4 Players/1 Machine"	},
	{0x28, 0x01, 0xc0, 0x00, "4 Players/2 Machines"	},
};

STDDIPINFO(Silentdj)

static struct BurnDIPInfo ViofightDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x02, "Off"			},
	{0x14, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x08, 0x00, "Off"			},
	{0x14, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin A"		},
	{0x14, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xc0, 0x40, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x03, 0x02, "Easy"			},
	{0x15, 0x01, 0x03, 0x03, "Medium"		},
	{0x15, 0x01, 0x03, 0x01, "Hard"			},
	{0x15, 0x01, 0x03, 0x00, "Hardest"		},
};

STDDIPINFO(Viofight)

static struct BurnDIPInfo HiticeDIPList[]=
{
	{0x25, 0xff, 0xff, 0xff, NULL			},
	{0x26, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet Style"	},
	{0x25, 0x01, 0x01, 0x01, "4 Players"		},
	{0x25, 0x01, 0x01, 0x00, "2 Players"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x25, 0x01, 0x04, 0x04, "Off"			},
	{0x25, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x25, 0x01, 0x08, 0x00, "Off"			},
	{0x25, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x25, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x25, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x25, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x25, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x25, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x25, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x25, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x25, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x26, 0x01, 0x03, 0x02, "Easy"			},
	{0x26, 0x01, 0x03, 0x03, "Medium"		},
	{0x26, 0x01, 0x03, 0x01, "Hard"			},
	{0x26, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Timer count"		},
	{0x26, 0x01, 0x0c, 0x0c, "1 sec = 58/60"	},
	{0x26, 0x01, 0x0c, 0x04, "1 sec = 56/60"	},
	{0x26, 0x01, 0x0c, 0x08, "1 sec = 62/60"	},
	{0x26, 0x01, 0x0c, 0x00, "1 sec = 45/60"	},

	{0   , 0xfe, 0   ,    2, "Maximum credits"	},
	{0x26, 0x01, 0x80, 0x00, "99"			},
	{0x26, 0x01, 0x80, 0x80, "9"			},
};

STDDIPINFO(Hitice)

static struct BurnDIPInfo Rambo3uDIPList[]=
{
	{0x17, 0xff, 0xff, 0xff, NULL			},
	{0x18, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x17, 0x01, 0x02, 0x02, "Off"			},
	{0x17, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x17, 0x01, 0x04, 0x04, "Off"			},
	{0x17, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x17, 0x01, 0x08, 0x00, "Off"			},
	{0x17, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Coinage"		},
	{0x17, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},
	{0x17, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x17, 0x01, 0x30, 0x00, "4 Coins 3 Credits"	},
	{0x17, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Price to Continue"	},
	{0x17, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x17, 0x01, 0xc0, 0x80, "1 Coin  1 Credits"	},
	{0x17, 0x01, 0xc0, 0xc0, "Same as Start"	},
	{0x17, 0x01, 0xc0, 0x00, "Same as Start or 1C/1C (if Coinage 4C/3C)" },

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x18, 0x01, 0x03, 0x02, "Easy"			},
	{0x18, 0x01, 0x03, 0x03, "Medium"		},
	{0x18, 0x01, 0x03, 0x01, "Hard"			},
	{0x18, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    1, "Control"		},
	{0x18, 0x01, 0x08, 0x08, "8 way Joystick"	},
//	{0x18, 0x01, 0x08, 0x00, "Trackball"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x18, 0x01, 0x10, 0x00, "Off"			},
	{0x18, 0x01, 0x10, 0x10, "On"			},
};

STDDIPINFO(Rambo3u)

static const eeprom_interface taitob_eeprom_intf =
{
	6,		// address bits
	16,		// data bits
	"0110",		// read command
	"0101",		// write command
	"0111",		// erase command
	"0100000000",	// lock command
	"0100110000",	// unlock command
	0,
	0
};

static void bankswitch(UINT32, UINT32 data)
{
	TaitoZ80Bank = data & 0x03;

	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + TaitoZ80Bank * 0x4000);
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + TaitoZ80Bank * 0x4000);
}

void __fastcall taitob_sound_write_ym2610(UINT16 a, UINT8 d)
{
	switch (a)
	{
		case 0xe000:
		case 0xe001:
		case 0xe002:
		case 0xe003:
			BurnYM2610Write(a & 3, d);
		return;

		case 0xe200:
			TC0140SYTSlavePortWrite(d);
		return;

		case 0xe201:
			TC0140SYTSlaveCommWrite(d);
		return;

		case 0xf200:
			bankswitch(0, d);
		return;
	}
}

UINT8 __fastcall taitob_sound_read_ym2610(UINT16 a)
{
	switch (a)
	{
		case 0xe000:
		case 0xe001:
		case 0xe002:
		case 0xe003:
			return BurnYM2610Read(a & 3); 

		case 0xe201:
			return TC0140SYTSlaveCommRead();
	}

	return 0;
}

void __fastcall taitob_sound_write_ym2203(UINT16 a, UINT8 d)
{
	switch (a)
	{
		case 0x9000:
		case 0x9001:
			BurnYM2203Write(0, a & 1, d);
		return;

		case 0xb000:
		case 0xb001:
			MSM6295Command(0, d);
		return;

		case 0xa000:
			TC0140SYTSlavePortWrite(d);
		return;

		case 0xa001:
			TC0140SYTSlaveCommWrite(d);
		return;
	}
}

UINT8 __fastcall taitob_sound_read_ym2203(UINT16 a)
{
	switch (a)
	{
		case 0x9000:
		case 0x9001:
			return BurnYM2203Read(0, a & 1);

		case 0xb000:
		case 0xb001:
			return MSM6295ReadStatus(0);

		case 0xa001:
			return TC0140SYTSlaveCommRead();
	}

	return 0;
}

static void DrvMakeInputs()
{
	memset (TC0220IOCInput, 0xff, 3);
	memset (TaitoInput + 3, 0xff, 3);

	for (INT32 i = 0; i < 8; i++) {
		TC0220IOCInput[0] ^= (TC0220IOCInputPort0[i] & 1) << i;
		TC0220IOCInput[1] ^= (TC0220IOCInputPort1[i] & 1) << i;
		TC0220IOCInput[2] ^= (TC0220IOCInputPort2[i] & 1) << i;
		TaitoInput[3] ^= (TaitoInputPort3[i] & 1) << i;
		TaitoInput[4] ^= (TaitoInputPort4[i] & 1) << i;
		TaitoInput[5] ^= (TaitoInputPort5[i] & 1) << i;
	}

	// normal coin state is active low, but some games have
	// active high buttons or coin inputs
	TC0220IOCInput[0] ^= nTaitoInputConfig[0];
	TC0220IOCInput[1] ^= nTaitoInputConfig[1];
	TC0220IOCInput[2] ^= nTaitoInputConfig[2];
	TaitoInput[3] ^= nTaitoInputConfig[3];

	// set up coin lockout
	switch (nTaitoInputConfig[4]) {
		case 0: // viofight, sbm, common
			if (TaitoCoinLockout[0]) TC0220IOCInput[2] |= (1 << 2);
			if (TaitoCoinLockout[1]) TC0220IOCInput[2] |= (1 << 3);
		break;

		case 1: // rambo3a, hitice, selfeena
			if (TaitoCoinLockout[0]) TC0220IOCInput[1] |= (1 << 4);
			if (TaitoCoinLockout[1]) TC0220IOCInput[1] |= (1 << 5);
		break;

		case 2: // silentd, spacedxo
			if (TaitoCoinLockout[0]) TC0220IOCInput[1] |= (1 << 4);
			if (TaitoCoinLockout[1]) TC0220IOCInput[1] |= (1 << 5);
			if (TaitoCoinLockout[2]) TaitoInput[5] |= (1 << 0);
			if (TaitoCoinLockout[3]) TaitoInput[5] |= (1 << 2);
		break;

		case 3: // qzshowby, pbobble
			if (TaitoCoinLockout[0]) TC0220IOCInput[0] |= (1 << 4);
			if (TaitoCoinLockout[1]) TC0220IOCInput[0] |= (1 << 5);
			if (TaitoCoinLockout[2]) TC0220IOCInput[0] |= (1 << 6);
			if (TaitoCoinLockout[3]) TC0220IOCInput[0] |= (1 << 7);
		break;
	}

	// for rambo3a's trackball
	if (nBurnGunNumPlayers) BurnGunMakeInputs(0, (INT16)TaitoAnalogPort0, (INT16)TaitoAnalogPort1);
	if (nBurnGunNumPlayers) BurnGunMakeInputs(1, (INT16)TaitoAnalogPort2, (INT16)TaitoAnalogPort3);
}

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / cpu_speed[1];
}

static double DrvGetTime()
{
	return (double)ZetTotalCycles() / (cpu_speed[1] * 1.0);
}

static INT32 DrvDoReset(INT32 reset_ram)
{
	if (reset_ram) {
		memset (TaitoRamStart, 0, TaitoRamEnd - TaitoRamStart);
	}

	if (DrvFramebuffer) {
		memset (DrvFramebuffer, 0, 1024 * 512);
	}

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	if (sound_config == 0) {
		BurnYM2610Reset();
	} else {
		ZetOpen(0);
		BurnYM2203Reset();
		ZetClose();
		MSM6295Reset(0);
	}

	TaitoICReset();

	EEPROMReset();

	coin_control = 0;
	eeprom_latch = 0;
	TaitoZ80Bank = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = TaitoMem;

	Taito68KRom1			= Next; Next += ((Taito68KRom1Size - 1) | 0x7ffff) + 1;
	TaitoZ80Rom1			= Next; Next += TaitoZ80Rom1Size;

	TaitoChars			= Next; Next += (TaitoCharRomSize * 8) / 4;
	TaitoSpritesA			= Next; Next += (TaitoCharRomSize * 8) / 4;

	TaitoMSM6295Rom			= Next; Next += TaitoMSM6295RomSize;
	TaitoYM2610ARom			= Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom			= Next; Next += TaitoYM2610BRomSize;

	if (!(TaitoMSM6295RomSize | TaitoYM2610ARomSize | TaitoYM2610ARomSize)) {
						Next += 0x040000; // games without samples...
	}

	TaitoRamStart			= Next;

	Taito68KRam1			= Next; Next += 0x010000;
	TaitoPaletteRam			= Next; Next += 0x002000;
	TaitoSpriteRam			= Next; Next += 0x002000;

	// hit the ice
	DrvPxlRAM			= Next; Next += 0x080000;
	DrvPxlScroll			= (UINT16*)Next; Next += 2 * sizeof(UINT16);

	TaitoZ80Ram1			= Next; Next += 0x002000;

	TaitoRamEnd			= Next;

	TaitoPalette			= (UINT32*)Next; Next += 0x1000 * sizeof(UINT32);

	TaitoMemEnd			= Next;

	return 0;
}

static void DrvGfxDecode(INT32 len, INT32 *tilemask0, INT32 *tilemask1)
{
	if (len == 0) return; // tetrist

	INT32 Planes[4] = { 0, 8, (len * 8) / 2 + 0, (len * 8) / 2 + 8 };
	INT32 XOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x080, 0x081, 0x082, 0x083, 0x084, 0x085, 0x086, 0x087 };
	INT32 YOffs[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070, 0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170 };

	UINT8 *tmp = (UINT8*)BurnMalloc(len);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, TaitoChars, len);

	GfxDecode(((len * 8) / 4) / ( 8 *  8), 4,  8,  8, Planes, XOffs, YOffs, 0x080, tmp, TaitoChars);
	GfxDecode(((len * 8) / 4) / (16 * 16), 4, 16, 16, Planes, XOffs, YOffs, 0x200, tmp, TaitoSpritesA);

	*tilemask0 = (((len * 8) / 4) / ( 8 *  8)) - 1;
	*tilemask1 = (((len * 8) / 4) / (16 * 16)) - 1;

	BurnFree (tmp);
}

static void common_ym2610_init()
{
	sound_config = 0;

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1);
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1);
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1);
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1);
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1);
	ZetSetWriteHandler(taitob_sound_write_ym2610);
	ZetSetReadHandler(taitob_sound_read_ym2610);
	ZetMemEnd();
	ZetClose();

	TC0140SYTInit();

	INT32 len0 = TaitoYM2610ARomSize;
	INT32 len1 = TaitoYM2610BRomSize;

	BurnYM2610Init(8000000, TaitoYM2610ARom, &len0, TaitoYM2610BRom, &len1, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(cpu_speed[1]);
	BurnYM2610SetSoundMixMode(1);
}

static void common_ym2203_init()
{
	sound_config = 1;

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1);
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1);
	ZetMapArea(0x8000, 0x8fff, 0, TaitoZ80Ram1);
	ZetMapArea(0x8000, 0x8fff, 1, TaitoZ80Ram1);
	ZetMapArea(0x8000, 0x8fff, 2, TaitoZ80Ram1);
	ZetSetWriteHandler(taitob_sound_write_ym2203);
	ZetSetReadHandler(taitob_sound_read_ym2203);
	ZetMemEnd();
	ZetClose();

	TC0140SYTInit();

	BurnYM2203Init(1, 3000000, DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetPorts(0, NULL, NULL, &bankswitch, NULL);
	BurnYM2203SetVolumeShift(2);
	BurnTimerAttachZet(cpu_speed[1]);

	MSM6295ROM = TaitoMSM6295Rom;

	MSM6295Init(0, 1056000 / 132, 40, 1);
}

static INT32 CommonInit(void (*pInitCallback)(), INT32 sound_type, INT32 color_select, INT32 input_type, INT32 irq0, INT32 irq1)
{
	static const UINT8 color_types[3][4] = {
		{ 0xc0, 0x80, 0x00, 0x40 },
		{ 0x00, 0x40, 0xc0, 0x80 },
		{ 0x30, 0x20, 0x00, 0x10 }
	};

	TaitoLoadRoms(false);

	TaitoMem = NULL;
	MemIndex();
	INT32 nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(true)) return 1;

	INT32 tilemaskChars = 0, tilemaskSprites = 0;
	DrvGfxDecode(TaitoCharRomSize, &tilemaskChars, &tilemaskSprites);

	memcpy (color_config, color_types[color_select], 4);

	irq_config[0] = irq0;
	irq_config[1] = irq1;

	cpu_speed[0] = 12000000;
	cpu_speed[1] =  4000000;

	nTaitoInputConfig[4] = input_type;

	TC0220IOCInit();
	TaitoMakeInputsFunction = DrvMakeInputs;

	TC0180VCUInit(TaitoChars, tilemaskChars, TaitoSpritesA, tilemaskSprites, 0, 16);

	EEPROMInit(&taitob_eeprom_intf);

	if (pInitCallback) {
		pInitCallback();
	}

	if (sound_type) {
		common_ym2203_init();
	} else {
		common_ym2610_init();
	}

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	EEPROMExit();

	SekExit();
	ZetExit();
	
	if (sound_config == 0) {
		BurnYM2610SetSoundMixMode(0);
		BurnYM2610Exit();
	} else {
		BurnYM2203Exit();
		MSM6295Exit(0);
		MSM6295ROM = NULL;
	}

	BurnFree (DrvFramebuffer);

	memset (nTaitoInputConfig, 0, 5);

	TaitoExit();

	return 0;
}

static void DrvPaletteUpdate()
{
	UINT16 *p = (UINT16*)TaitoPaletteRam;

	for (INT32 i = 0; i < 0x2000 / 2; i++)
	{
		INT32 r = (p[i] >> 12) & 0x0f;
		INT32 g = (p[i] >>  8) & 0x0f;
		INT32 b = (p[i] >>  4) & 0x0f;

		r |= r << 4;
		g |= g << 4;
		b |= b << 4;

		TaitoPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void draw_hitice_framebuffer()
{
	if (DrvFramebuffer == NULL) return;

	INT32 scrollx = -((2 * DrvPxlScroll[0] +  0) & 0x3ff);
	INT32 scrolly = -((1 * DrvPxlScroll[1] + 16) & 0x1ff);

	for (INT32 sy = 0; sy < nScreenHeight; sy++)
	{
		UINT16 *dst = pTransDraw + sy * nScreenWidth;
		UINT8  *src = DrvFramebuffer + ((sy + scrolly) & 0x1ff) * 1024;

		for (INT32 sx = 0; sx < nScreenWidth; sx++) {
			INT32 pxl = src[(sx + scrollx) & 0x3ff];

			if (pxl) {
				dst[sx] = pxl | 0x800;
			}
		}
	}
}

static INT32 DrvDraw()
{
	DrvPaletteUpdate();

	INT32 ctrl = TC0180VCUReadControl();

	if (~ctrl & 0x20) {
		BurnTransferClear();
		BurnTransferCopy(TaitoPalette);
		return 0;
	}

	if (~nBurnLayer & 1) BurnTransferClear();

	if (nBurnLayer & 1) TC0180VCUDrawLayer(color_config[0], 1, -1);

	if (nSpriteEnable & 1) TC0180VCUFramebufferDraw(1, color_config[3] << 4);

	if (nBurnLayer & 2) TC0180VCUDrawLayer(color_config[1], 0,  0);

	draw_hitice_framebuffer();

	if (nSpriteEnable & 2) TC0180VCUFramebufferDraw(0, color_config[3] << 4);

	if (nBurnLayer & 4) TC0180VCUDrawCharLayer(color_config[2]);

	BurnTransferCopy(TaitoPalette);

	TC0180VCUBufferSprites();

	return 0;
}

static INT32 DrvFrame()
{
	if (TaitoReset) {
		DrvDoReset(1);
	}

	SekNewFrame();
	ZetNewFrame();

	TaitoWatchdog++;
	if (TaitoWatchdog > 180) {
		DrvDoReset(0);
//		bprintf (0, _T("watchdog triggered!\n"));
	}

	TaitoMakeInputsFunction();

	SekOpen(0);
	ZetOpen(0);

	INT32 SekSpeed = (INT32)((INT64)cpu_speed[0] * nBurnCPUSpeedAdjust / 0x100);
	INT32 ZetSpeed = (INT32)((INT64)cpu_speed[1] * nBurnCPUSpeedAdjust / 0x100);

	INT32 nInterleave = 200;	// high so that ym2203 sounds are good, 200 is perfect for irq #0
	INT32 nCyclesTotal[2] = { SekSpeed / 60, ZetSpeed / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };
	INT32 nNext[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++) {
		nNext[0] += nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += SekRun(nNext[0] - nCyclesDone[0]);
		if (i == 4)                     SekSetIRQLine(irq_config[0], SEK_IRQSTATUS_AUTO); // Start of frame + 5000 cycles
		if (i == (nInterleave / 1) - 1) SekSetIRQLine(irq_config[1], SEK_IRQSTATUS_AUTO); // End of frame
		nNext[1] += nCyclesTotal[1] / nInterleave;
		BurnTimerUpdate(nNext[1]);
		nCyclesDone[1] += nNext[1];
	}

	BurnTimerEndFrame(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		if (sound_config == 0) {
			BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
		} else {
			BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
			MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		}
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
		*pnMin = 0x029708;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = TaitoRamStart;
		ba.nLen	  = TaitoRamEnd - TaitoRamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		TaitoICScan(nAction);

		if (sound_config == 0) {
			BurnYM2610Scan(nAction, pnMin);
		} else {
			BurnYM2203Scan(nAction, pnMin);
			MSM6295Scan(0, nAction);
		}

		SCAN_VAR(TaitoZ80Bank);
		SCAN_VAR(TaitoWatchdog);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		bankswitch(0, TaitoZ80Bank);
		ZetClose();
	}

	return 0;
}


//----------------------------------------------------------------------------------------------------------
// Rastan Saga 2 / Ashura Blaster

UINT8 __fastcall rastsag2_read_byte(UINT32 a)
{
	TC0180VCUHalfWordRead_Map(0x400000)
	TC0220IOCHalfWordRead_Map(0xa00000)

	switch (a)
	{
		case 0x800002:
			return TC0140SYTCommRead();
	}

	return 0;
}

void __fastcall rastsag2_write_byte(UINT32 a, UINT8 d)
{
	TC0180VCUHalfWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0xa00000)

	switch (a)
	{
		case 0x800000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x800002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall rastsag2_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x400000)
}

static void NastarInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(TaitoPaletteRam,		0x200000, 0x201fff, SM_RAM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0x600000, 0x607fff, SM_RAM);
	SekSetWriteByteHandler(0,		rastsag2_write_byte);
	SekSetWriteWordHandler(0,		rastsag2_write_word);
	SekSetReadByteHandler(0,		rastsag2_read_byte);
//	SekSetReadWordHandler(0,		rastsag2_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------
// Crime City

UINT8 __fastcall crimec_read_byte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x200000)
	TC0180VCUHalfWordRead_Map(0x400000)

	switch (a)
	{
		case 0x600002:
			return TC0140SYTCommRead();
	}

	return 0;
}

void __fastcall crimec_write_byte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x200000)
	TC0180VCUHalfWordWrite_Map(0x400000)

	switch (a)
	{
		case 0x600000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x600002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall crimec_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x400000)
}

static void CrimecInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0x800000, 0x801fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0xa00000, 0xa0ffff, SM_RAM);
	SekSetWriteByteHandler(0,		crimec_write_byte);
	SekSetWriteWordHandler(0,		crimec_write_word);
	SekSetReadByteHandler(0,		crimec_read_byte);
//	SekSetReadWordHandler(0,		crimec_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------
// Tetris (ym2610)

UINT8 __fastcall tetrist_read_byte(UINT32 a)
{
	TC0180VCUHalfWordRead_Map(0x400000)
	TC0220IOCHalfWordRead_Map(0x600000)

	switch (a)
	{
		case 0x200002: // not used?
			return TC0140SYTCommRead();

		case 0x600010: { // tracky1_lo_r 
			INT32 ret = (( TaitoAnalogPort1 >> 4) & 0xffff);
			if (ret == 0xffff) return 0;
			return (ret+1);
		}

		case 0x600014: {// trackx1_lo_r
			INT32 ret = ((~TaitoAnalogPort0 >> 4) & 0xffff);
			if (ret == 0xffff) return 0;
			return (ret+1);
		}

		case 0x600018: {// tracky2_lo_r
			INT32 ret = (( TaitoAnalogPort3 >> 4) & 0xffff);
			if (ret == 0xffff) return 0;
			return (ret+1);
		}

		case 0x60001c: {// trackx2_lo_r
			INT32 ret = ((~TaitoAnalogPort2 >> 4) & 0xffff);
			if (ret == 0xffff) return 0;
			return (ret+1);
		}
	}

	return 0;
}

UINT16 __fastcall tetrist_read_word(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x600000)

	switch (a)
	{
		case 0x600012: {// tracky1_hi_r
			INT32 ret = (( TaitoAnalogPort1 >> 4) & 0xffff);
			if (ret == 0xffff) return 0;
			return (ret+1);
		}

		case 0x600016: {// trackx1_hi_r
			INT32 ret = ((~TaitoAnalogPort0 >> 4) & 0xffff);
			if (ret == 0xffff) return 0;
			return (ret+1);
		}

		case 0x60001a: {// tracky2_hi_r
			INT32 ret = (( TaitoAnalogPort3 >> 4) & 0xffff);
			if (ret == 0xffff) return 0;
			return (ret+1);
		}

		case 0x60001e: {// trackx2_hi_r
			INT32 ret = ((~TaitoAnalogPort2 >> 4) & 0xffff);
			if (ret == 0xffff) return 0;
			return (ret+1);
		}
	}

	return 0;
}

void __fastcall tetrist_write_byte(UINT32 a, UINT8 d)
{
	TC0180VCUHalfWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x600000)

	switch (a)
	{
		case 0x200000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x200002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall tetrist_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x600000)
}

static void TetristInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0x800000, 0x807fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0xa00000, 0xa01fff, SM_RAM);
	SekSetWriteByteHandler(0,		tetrist_write_byte);
	SekSetWriteWordHandler(0,		tetrist_write_word);
	SekSetReadByteHandler(0,		tetrist_read_byte);
	SekSetReadWordHandler(0,		tetrist_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------
// Puzzle Bobble / Space Invaders Dx (v2.1)

UINT8 __fastcall pbobble_read_byte(UINT32 a)
{
	TC0180VCUHalfWordRead_Map(0x400000)

	switch (a)
	{
		case 0x500000:
			return TC0220IOCDip[0];

		case 0x500002:
			return ((TC0220IOCInput[0] & 0xfe) | (EEPROMRead() & 1));

		case 0x500004:
			return TC0220IOCInput[1];

		case 0x500006:
			return TC0220IOCInput[2];

		case 0x500008:
			return TC0220IOCRead(0x08 / 2);

		case 0x50000e:
			return TaitoInput[3];

		case 0x500024:
			return TaitoInput[4];

		case 0x500026:
			return eeprom_latch;

		case 0x50002e:
			return TaitoInput[5];

		case 0x700002:
			return TC0140SYTCommRead();
	}

	return 0;
}

void __fastcall pbobble_write_byte(UINT32 a, UINT8 d)
{
	TC0180VCUHalfWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x500000)

	switch (a)
	{
		case 0x500026:
			eeprom_latch = d;
			EEPROMWrite((d & 0x08), (d & 0x10), (d & 0x04));
		return;

		case 0x500028:
			coin_control = d;
			TaitoCoinLockout[2] = ~d & 0x01;
			TaitoCoinLockout[3] = ~d & 0x02;
			// coin counter d & 0x04, d & 0x08
		return;

		case 0x600000:
		case 0x600002: // gain
		return;

		case 0x700000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x700002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall pbobble_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x500000)
}

static void PbobbleInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0x800000, 0x801fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0x900000, 0x90ffff, SM_RAM);
	SekSetWriteByteHandler(0,		pbobble_write_byte);
	SekSetWriteWordHandler(0,		pbobble_write_word);
	SekSetReadByteHandler(0,		pbobble_read_byte);
//	SekSetReadWordHandler(0,		pbobble_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------
// Sel Feena / Ryu Jin

UINT8 __fastcall selfeena_read_byte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x400000)
	TC0220IOCHalfWordRead_Map(0x410000)

	if (a >= 0x218000 && a <= 0x21801f) {
		return TC0180VCUReadRegs(a);
	}

	switch (a)
	{
		case 0x500002: // not used?
			return TC0140SYTCommRead();
	}

	return 0;
}

void __fastcall selfeena_write_byte(UINT32 a, UINT8 d)
{
	TC0180VCUHalfWordWrite_Map(0x200000)
	TC0220IOCHalfWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x410000)

	switch (a)
	{
		case 0x500000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x500002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall selfeena_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x200000)
}

static void SelfeenaInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1,		0x100000, 0x103fff, SM_RAM);
	SekMapMemory(TC0180VCURAM,		0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x210000, 0x2137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x213800, 0x213fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0x300000, 0x301fff, SM_RAM);
	SekSetWriteByteHandler(0,		selfeena_write_byte);
	SekSetWriteWordHandler(0,		selfeena_write_word);
	SekSetReadByteHandler(0,		selfeena_read_byte);
//	SekSetReadWordHandler(0,		selfeena_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------
// Sonic Blast Man

UINT8 __fastcall sbm_read_byte(UINT32 a)
{
	if ((a & 0xffffff0) == 0x300000) a ^= 2;
	TC0220IOCHalfWordRead_Map(0x300000)
	TC0180VCUHalfWordRead_Map(0x900000)

	switch (a)
	{
		case 0x320002:
			return TC0140SYTCommRead();
	}

	return 0;
}

void __fastcall sbm_write_byte(UINT32 a, UINT8 d)
{
	if ((a & 0xffffff0) == 0x300000) a ^= 2;
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0180VCUHalfWordWrite_Map(0x900000)

	switch (a)
	{
		case 0x320000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x320002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall sbm_write_word(UINT32 a, UINT16 d)
{
	if ((a & 0xffffff0) == 0x0300000) a ^= 2;
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0180VCUWordWrite_Map(0x900000)
}

static void SbmInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1,		0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0x200000, 0x201fff, SM_RAM);
	SekMapMemory(TC0180VCURAM,		0x900000, 0x90ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x910000, 0x9137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x913800, 0x913fff, SM_RAM);
	SekSetWriteByteHandler(0,		sbm_write_byte);
	SekSetWriteWordHandler(0,		sbm_write_word);
	SekSetReadByteHandler(0,		sbm_read_byte);
//	SekSetReadWordHandler(0,		sbm_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------
// Silent Dragon

UINT8 __fastcall silentd_read_byte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x200000)
	TC0180VCUHalfWordRead_Map(0x500000)

	switch (a)
	{
		case 0x100003: // not used?
			return TC0140SYTCommRead();

		case 0x210001:
			return TaitoInput[3];

		case 0x220001:
			return TaitoInput[4];

		case 0x230001:
			return TaitoInput[5];
	}

	return 0;
}

void __fastcall silentd_write_byte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x200000)
	TC0180VCUHalfWordWrite_Map(0x500000)

	switch (a)
	{
		case 0x100000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x100002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall silentd_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x500000)
}

static void SilentdInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(TaitoPaletteRam,		0x300000, 0x301fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0x400000, 0x403fff, SM_RAM);
	SekMapMemory(TC0180VCURAM,		0x500000, 0x50ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x510000, 0x5137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x513800, 0x513fff, SM_RAM);
	SekSetWriteByteHandler(0,		silentd_write_byte);
	SekSetWriteWordHandler(0,		silentd_write_word);
	SekSetReadByteHandler(0,		silentd_read_byte);
//	SekSetReadWordHandler(0,		silentd_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------
// Violence Fight

UINT8 __fastcall viofight_read_byte(UINT32 a)
{
	TC0180VCUHalfWordRead_Map(0x400000)
	TC0220IOCHalfWordRead_Map(0x800000)

	switch (a)
	{
		case 0x200002:
			return TC0140SYTCommRead();
	}

	return 0;
}

void __fastcall viofight_write_byte(UINT32 a, UINT8 d)
{
	TC0180VCUHalfWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x800000)

	switch (a)
	{
		case 0x200000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x200002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall viofight_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x800000)
}

static void ViofightInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0x600000, 0x601fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0xa00000, 0xa03fff, SM_RAM);
	SekSetWriteByteHandler(0,		viofight_write_byte);
	SekSetWriteWordHandler(0,		viofight_write_word);
	SekSetReadByteHandler(0,		viofight_read_byte);
//	SekSetReadWordHandler(0,		viofight_read_word);
	SekClose();

	memcpy (Taito68KRom1 + 0x40000, Taito68KRom1 + 0x20000, 0x40000);

	cpu_speed[1] = 6000000; // 6mhz
}

//----------------------------------------------------------------------------------------------------------
// Hit the Ice

UINT8 __fastcall hitice_read_byte(UINT32 a)
{
	TC0180VCUHalfWordRead_Map(0x400000)
	TC0220IOCHalfWordRead_Map(0x600000)

	switch (a)
	{
		case 0x610000:
			return TC0220IOCInput[4];

		case 0x610001:
			return TC0220IOCInput[3];

		case 0x700002:
			return TC0140SYTCommRead();
	}

	return 0;
}

static void hiticeFramebufferUpdate(UINT32 offset)
{
	offset &= 0x7fffe;
	DrvFramebuffer[offset + 0] = DrvPxlRAM[offset];
	DrvFramebuffer[offset + 1] = DrvPxlRAM[offset];
}

void __fastcall hitice_write_byte(UINT32 a, UINT8 d)
{
	TC0180VCUHalfWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x600000)

	if (a >= 0xb00000 && a <= 0xb7ffff) {
		DrvPxlRAM[(a & 0x7ffff)^1] = d;
		hiticeFramebufferUpdate(a);
		return;
	}

	switch (a)
	{
		case 0x700000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x700002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall hitice_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x600000)

	if (a >= 0xb00000 && a <= 0xb7ffff) {
		*((UINT16*)(DrvPxlRAM + (a & 0x7fffe))) = d;
		hiticeFramebufferUpdate(a);
		return;
	}

	switch (a)
	{
		case 0xbffff2:
			DrvPxlScroll[0] = d;
		return;

		case 0xbffff4:
			DrvPxlScroll[1] = d;
		return;
	}
}

static void HiticeInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0x800000, 0x803fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0xa00000, 0xa01fff, SM_RAM);
	SekMapMemory(DrvPxlRAM,			0xb00000, 0xb7ffff, SM_ROM);
	SekSetWriteByteHandler(0,		hitice_write_byte);
	SekSetWriteWordHandler(0,		hitice_write_word);
	SekSetReadByteHandler(0,		hitice_read_byte);
//	SekSetReadWordHandler(0,		hitice_read_word);
	SekClose();

	DrvFramebuffer	= (UINT8*)BurnMalloc(1024 * 512);
}

//----------------------------------------------------------------------------------------------------------
// Tetris (ym2203)

UINT8 __fastcall tetrista_read_byte(UINT32 a)
{
	TC0180VCUHalfWordRead_Map(0x400000)

	switch (a)
	{
		case 0x600000:
		case 0x800000:
			return TC0220IOCPortRegRead();

		case 0x600002:
		case 0x800002:
			return TC0220IOCPortRead();

		case 0xa00002:
			return TC0140SYTCommRead();
	}

	return 0;
}

void __fastcall tetrista_write_byte(UINT32 a, UINT8 d)
{
	TC0180VCUHalfWordWrite_Map(0x400000)

	switch (a)
	{
		case 0x600000:
		case 0x800000:
			TaitoWatchdog = 0;
			TC0220IOCHalfWordPortRegWrite(d);
		return;

		case 0x600002:
		case 0x800002:
			TC0220IOCHalfWordPortWrite(d);
		return;

		case 0xa00000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0xa00002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;
	}
}

void __fastcall tetrista_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x400000)
}

static void TetristaInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(TaitoPaletteRam,		0x200000, 0x201fff, SM_RAM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0x800000, 0x803fff, SM_RAM);
	SekSetWriteByteHandler(0,		tetrista_write_byte);
	SekSetWriteWordHandler(0,		tetrista_write_word);
	SekSetReadByteHandler(0,		tetrista_read_byte);
//	SekSetReadWordHandler(0,		tetrista_read_word);
	SekClose();
}

static void MasterwInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Taito68KRam1,		0x200000, 0x203fff, SM_RAM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0x600000, 0x601fff, SM_RAM);
	SekSetWriteByteHandler(0,		tetrista_write_byte);
	SekSetWriteWordHandler(0,		tetrista_write_word);
	SekSetReadByteHandler(0,		tetrista_read_byte);
//	SekSetReadWordHandler(0,		tetrista_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------
// Quiz Sekai Wa Show by shobai


UINT8 __fastcall qzshowby_read_byte(UINT32 a)
{
	if (a != 0x200002) {
		TC0220IOCHalfWordRead_Map(0x200000)
	}

	TC0180VCUHalfWordRead_Map(0x400000)

	switch (a)
	{
		case 0x200002:
			return ((TC0220IOCInput[0] & 0xfe) | (EEPROMRead() & 1));

		case 0x200024:
			return TC0220IOCInput[4];

		case 0x200028:
			return coin_control;

		case 0x20002e:
			return TC0220IOCInput[5];

		case 0x600002:
			return TC0140SYTCommRead();
	}

	return 0;
}

void __fastcall qzshowby_write_byte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x200000)
	TC0180VCUHalfWordWrite_Map(0x400000)

	switch (a)
	{
		case 0x200026:
			EEPROMWrite((d & 0x08), (d & 0x10), (d & 0x04));
		return;

		case 0x200028:
			coin_control = d;
			TaitoCoinLockout[2] = ~d & 0x01;
			TaitoCoinLockout[3] = ~d & 0x02;
			// coin counter d & 0x04, d & 0x08
		return;

		case 0x600000:
			TC0140SYTPortWrite(d & 0xff);
		return;

		case 0x600002:
			ZetClose();
			TC0140SYTCommWrite(d & 0xff);
			ZetOpen(0);
		return;

		case 0x700000:
		case 0x700002:
			// gain
		return;
	}
}

void __fastcall qzshowby_write_word(UINT32 a, UINT16 d)
{
	TC0180VCUWordWrite_Map(0x400000)
	TC0220IOCHalfWordWrite_Map(0x200000)
}

static void QzshowbyInitCallback()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1,		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(TC0180VCURAM,		0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam,		0x410000, 0x4137ff, SM_RAM);
	SekMapMemory(TC0180VCUScrollRAM,	0x413800, 0x413fff, SM_RAM);
	SekMapMemory(TaitoPaletteRam,		0x800000, 0x801fff, SM_RAM);
	SekMapMemory(Taito68KRam1,		0x900000, 0x90ffff, SM_RAM);
	SekSetWriteByteHandler(0,		qzshowby_write_byte);
	SekSetWriteWordHandler(0,		qzshowby_write_word);
	SekSetReadByteHandler(0,		qzshowby_read_byte);
//	SekSetReadWordHandler(0,		qzshowby_read_word);
	SekClose();
}

//----------------------------------------------------------------------------------------------------------


// Master of Weapon (World)

static struct BurnRomInfo masterwRomDesc[] = {
	{ "b72_06.33",			0x020000, 0xae848eff, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b72_12.24",			0x020000, 0x7176ce70, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b72_04.34",			0x020000, 0x141e964c, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b72_03.25",			0x020000, 0xf4523496, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b72_07.30",			0x010000, 0x2b1a946f, TAITO_Z80ROM1 },		//  4 Z80 code

	{ "b72-02.rom",			0x080000, 0xc519f65a, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b72-01.5",			0x080000, 0xa24ac26e, TAITO_CHARS },		//  6
};

STD_ROM_PICK(masterw)
STD_ROM_FN(masterw)

static INT32 MasterwInit()
{
	return CommonInit(MasterwInitCallback, 1, 2, 0, 4, 5);
}

struct BurnDriver BurnDrvMasterw = {
	"masterw", NULL, NULL, NULL, "1989",
	"Master of Weapon (World)\0", "Imperfect graphics", "Taito Corporation Japan", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TAITO_TAITOB, GBF_VERSHOOT, 0,
	NULL, masterwRomInfo, masterwRomName, NULL, NULL, CommonInputInfo, MasterwDIPInfo,
	MasterwInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	224, 320, 3, 4
};


// Master of Weapon (US)

static struct BurnRomInfo masterwuRomDesc[] = {
	{ "b72_06.33",			0x020000, 0xae848eff, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b72_11.24",			0x020000, 0x0671fee6, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b72_04.34",			0x020000, 0x141e964c, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b72_03.25",			0x020000, 0xf4523496, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b72_07.30",			0x010000, 0x2b1a946f, TAITO_Z80ROM1 },		//  4 Z80 code

	{ "b72-02.rom",			0x080000, 0xc519f65a, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b72-01.5",			0x080000, 0xa24ac26e, TAITO_CHARS },		//  6
};

STD_ROM_PICK(masterwu)
STD_ROM_FN(masterwu)

struct BurnDriver BurnDrvMasterwu = {
	"masterwu", "masterw", NULL, NULL, "1989",
	"Master of Weapon (US)\0", "Imperfect graphics", "Taito America Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TAITO_TAITOB, GBF_VERSHOOT, 0,
	NULL, masterwuRomInfo, masterwuRomName, NULL, NULL, CommonInputInfo, MasterwDIPInfo,
	MasterwInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	224, 320, 3, 4
};


// Master of Weapon (Japan)

static struct BurnRomInfo masterwjRomDesc[] = {
	{ "b72_06.33",			0x020000, 0xae848eff, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b72_05.24",			0x020000, 0x9f78af5c, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b72_04.34",			0x020000, 0x141e964c, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b72_03.25",			0x020000, 0xf4523496, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b72_07.30",			0x010000, 0x2b1a946f, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "b72-02.6",			0x080000, 0x843444eb, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b72-01.5",			0x080000, 0xa24ac26e, TAITO_CHARS },		//  6
};

STD_ROM_PICK(masterwj)
STD_ROM_FN(masterwj)

struct BurnDriver BurnDrvMasterwj = {
	"masterwj", "masterw", NULL, NULL, "1989",
	"Master of Weapon (Japan)\0", "Imperfect graphics", "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TAITO_TAITOB, GBF_VERSHOOT, 0,
	NULL, masterwjRomInfo, masterwjRomName, NULL, NULL, CommonInputInfo, MasterwDIPInfo,
	MasterwInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	224, 320, 3, 4
};


// Nastar (World)

static struct BurnRomInfo nastarRomDesc[] = {
	{ "b81-08.50",			0x020000, 0xd6da9169, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b81-13.31",			0x020000, 0x60d176fb, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b81-10.49",			0x020000, 0x53f34344, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b81-09.30",			0x020000, 0x630d34af, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b81-11.37",			0x010000, 0x3704bf09, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "b81-03.14",			0x080000, 0x551b75e6, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b81-04.15",			0x080000, 0xcf734e12, TAITO_CHARS },		//  6

	{ "b81-02.2",			0x080000, 0x20ec3b86, TAITO_YM2610A },		//  7 YM2610 A Samples

	{ "b81-01.1",			0x080000, 0xb33f796b, TAITO_YM2610B },		//  8 YM2610 B Samples

	{ "ampal16l8-b81-05.21",	0x000104, 0x922fd368, BRF_OPT },		//  9 PLDs
	{ "ampal16l8-b81-06a.22",	0x000104, 0xbb1cec84, BRF_OPT },	        // 10
};

STD_ROM_PICK(nastar)
STD_ROM_FN(nastar)

static INT32 NastarInit()
{
	return CommonInit(NastarInitCallback, 0, 0, 0, 2, 4);
}

struct BurnDriver BurnDrvNastar = {
	"nastar", NULL, NULL, NULL, "1988",
	"Nastar (World)\0", NULL, "Taito Corporation Japan", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_SCRFIGHT, 0,
	NULL, nastarRomInfo, nastarRomName, NULL, NULL, CommonInputInfo, NastarDIPInfo,
	NastarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Rastan Saga 2 (Japan)

static struct BurnRomInfo rastsag2RomDesc[] = {
	{ "b81-08.50",			0x020000, 0xd6da9169, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b81-07.31",			0x020000, 0x8edf17d7, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b81-10.49",			0x020000, 0x53f34344, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b81-09.30",			0x020000, 0x630d34af, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b81-11.37",			0x010000, 0x3704bf09, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "b81-03.14",			0x080000, 0x551b75e6, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b81-04.15",			0x080000, 0xcf734e12, TAITO_CHARS },		//  6

	{ "b81-02.2",			0x080000, 0x20ec3b86, TAITO_YM2610A },		//  7 YM2610 A Samples

	{ "b81-01.1",			0x080000, 0xb33f796b, TAITO_YM2610B },		//  8 YM2610 B Samples
};

STD_ROM_PICK(rastsag2)
STD_ROM_FN(rastsag2)

struct BurnDriver BurnDrvRastsag2 = {
	"rastsag2", "nastar", NULL, NULL, "1988",
	"Rastan Saga 2 (Japan)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_SCRFIGHT, 0,
	NULL, rastsag2RomInfo, rastsag2RomName, NULL, NULL, CommonInputInfo, Rastsag2DIPInfo,
	NastarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Nastar Warrior (US)

static struct BurnRomInfo nastarwRomDesc[] = {
	{ "b81-08.50",			0x020000, 0xd6da9169, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b81-12.31",			0x020000, 0xf9d82741, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b81-10.49",			0x020000, 0x53f34344, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b81-09.30",			0x020000, 0x630d34af, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b81-11.37",			0x010000, 0x3704bf09, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "b81-03.14",			0x080000, 0x551b75e6, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b81-04.15",			0x080000, 0xcf734e12, TAITO_CHARS },		//  6

	{ "b81-02.2",			0x080000, 0x20ec3b86, TAITO_YM2610A },		//  7 YM2610 A Samples

	{ "b81-01.1",			0x080000, 0xb33f796b, TAITO_YM2610B },		//  8 YM2610 B Samples
};

STD_ROM_PICK(nastarw)
STD_ROM_FN(nastarw)

struct BurnDriver BurnDrvNastarw = {
	"nastarw", "nastar", NULL, NULL, "1988",
	"Nastar Warrior (US)\0", NULL, "Taito America Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_SCRFIGHT, 0,
	NULL, nastarwRomInfo, nastarwRomName, NULL, NULL, CommonInputInfo, NastarwDIPInfo,
	NastarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Rambo III (Europe set 1)

static struct BurnRomInfo rambo3RomDesc[] = {
	{ "ramb3-11.bin",		0x020000, 0x1cc42247, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "ramb3-14.bin",		0x020000, 0x7d917c21, TAITO_68KROM1_BYTESWAP }, //  1
	{ "ramb3-07.bin",		0x020000, 0xc973ff6f, TAITO_68KROM1_BYTESWAP }, //  2
	{ "ramb3-06.bin",		0x020000, 0xa83d3fd5, TAITO_68KROM1_BYTESWAP }, //  3

	{ "ramb3-10.bin",		0x010000, 0xb18bc020, TAITO_Z80ROM1 },          //  4 Z80 Code

	{ "ramb3-03.bin",		0x080000, 0xf5808c41, TAITO_CHARS },            //  5 Graphics Tiles
	{ "ramb3-04.bin",		0x080000, 0xc57831ce, TAITO_CHARS },            //  6
	{ "ramb3-01.bin",		0x080000, 0xc55fcf54, TAITO_CHARS },            //  7
	{ "ramb3-02.bin",		0x080000, 0x9dd014c6, TAITO_CHARS },            //  8

	{ "ramb3-05.bin",		0x080000, 0x0179dc40, TAITO_YM2610A },          //  9 YM2610 A Samples
};

STD_ROM_PICK(rambo3)
STD_ROM_FN(rambo3)

static INT32 Rambo3Init()
{
	nTaitoInputConfig[1] = 0x30;
	BurnGunInit(2, false);

	return CommonInit(TetristInitCallback, 0, 2, 0, 1, 6);
}

struct BurnDriver BurnDrvRambo3 = {
	"rambo3", NULL, NULL, NULL, "1989",
	"Rambo III (Europe)\0", NULL, "Taito Europe Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_MISC, 0,
	NULL, rambo3RomInfo, rambo3RomName, NULL, NULL, Rambo3uInputInfo, Rambo3uDIPInfo,
	Rambo3Init, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Rambo III (US)

static struct BurnRomInfo rambo3uRomDesc[] = {
	{ "ramb3-11.bin",		0x020000, 0x1cc42247, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "ramb3-13.bin",		0x020000, 0x0a964cb7, TAITO_68KROM1_BYTESWAP }, //  1
	{ "ramb3-07.bin",		0x020000, 0xc973ff6f, TAITO_68KROM1_BYTESWAP }, //  2
	{ "ramb3-06.bin",		0x020000, 0xa83d3fd5, TAITO_68KROM1_BYTESWAP }, //  3

	{ "ramb3-10.bin",		0x010000, 0xb18bc020, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "ramb3-03.bin",		0x080000, 0xf5808c41, TAITO_CHARS },		//  5 Graphics Tiles
	{ "ramb3-04.bin",		0x080000, 0xc57831ce, TAITO_CHARS },		//  6
	{ "ramb3-01.bin",		0x080000, 0xc55fcf54, TAITO_CHARS },		//  7
	{ "ramb3-02.bin",		0x080000, 0x9dd014c6, TAITO_CHARS },		//  8

	{ "ramb3-05.bin",		0x080000, 0x0179dc40, TAITO_YM2610A },		//  9 YM2610 A Samples
};

STD_ROM_PICK(rambo3u)
STD_ROM_FN(rambo3u)

struct BurnDriver BurnDrvRambo3u = {
	"rambo3u", "rambo3", NULL, NULL, "1989",
	"Rambo III (US)\0", NULL, "Taito Europe Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_MISC, 0,
	NULL, rambo3uRomInfo, rambo3uRomName, NULL, NULL, Rambo3uInputInfo, Rambo3uDIPInfo,
	Rambo3Init, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Rambo III (Europe, Proto?)

static struct BurnRomInfo rambo3pRomDesc[] = {
	{ "r3-0e.rom",			0x010000, 0x3efa4177, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "r3-0o.rom",			0x010000, 0x55c38d92, TAITO_68KROM1_BYTESWAP }, //  1
	{ "r3-1e.rom",			0x020000, 0x40e363c7, TAITO_68KROM1_BYTESWAP }, //  2
	{ "r3-1o.rom",			0x020000, 0x7f1fe6ab, TAITO_68KROM1_BYTESWAP }, //  3

	{ "r3-00.rom",			0x010000, 0xdf7a6ed6, TAITO_Z80ROM1 },		//  4 Z80 Code

//	This order is different than that of MAME! This allows use of standard gfx decode.
	{ "r3-ch1ll.rom",		0x020000, 0xc86ea5fc, TAITO_CHARS_BYTESWAP },   //  5 Graphics Tiles
	{ "r3-ch1lh.rom",		0x020000, 0x75568cf0, TAITO_CHARS_BYTESWAP },   //  6
	{ "r3-ch1hl.rom",		0x020000, 0x7525eb92, TAITO_CHARS_BYTESWAP },   //  7
	{ "r3-ch1hh.rom",		0x020000, 0xe39cff37, TAITO_CHARS_BYTESWAP },   //  8
	{ "r3-ch3ll.rom",		0x020000, 0xabe54b1e, TAITO_CHARS_BYTESWAP },   //  9
	{ "r3-ch3lh.rom",		0x020000, 0x5a155c04, TAITO_CHARS_BYTESWAP },   // 10
	{ "r3-ch3hl.rom",		0x020000, 0x80e5647e, TAITO_CHARS_BYTESWAP },   // 11
	{ "r3-ch3hh.rom",		0x020000, 0xabe58fdb, TAITO_CHARS_BYTESWAP },   // 12
	{ "r3-ch0ll.rom",		0x020000, 0xb416f1bf, TAITO_CHARS_BYTESWAP },   // 13
	{ "r3-ch0lh.rom",		0x020000, 0x76a330a2, TAITO_CHARS_BYTESWAP },   // 14
	{ "r3-ch0hl.rom",		0x020000, 0xa4cad36d, TAITO_CHARS_BYTESWAP },   // 15
	{ "r3-ch0hh.rom",		0x020000, 0x4dc69751, TAITO_CHARS_BYTESWAP },   // 16
	{ "r3-ch2ll.rom",		0x020000, 0xd0ce3051, TAITO_CHARS_BYTESWAP },   // 17
	{ "r3-ch2lh.rom",		0x020000, 0xdf3bc48f, TAITO_CHARS_BYTESWAP },   // 18
	{ "r3-ch2hl.rom",		0x020000, 0x837d8677, TAITO_CHARS_BYTESWAP },   // 19
	{ "r3-ch2hh.rom",		0x020000, 0xbf37dfac, TAITO_CHARS_BYTESWAP },   // 20

	{ "r3-a1.rom",			0x020000, 0x4396fa19, TAITO_YM2610A },		// 21 YM2610 A Samples
	{ "r3-a2.rom",			0x020000, 0x41fe53a8, TAITO_YM2610A },		// 22
	{ "r3-a3.rom",			0x020000, 0xe89249ba, TAITO_YM2610A },		// 23
	{ "r3-a4.rom",			0x020000, 0x9cf4c21b, TAITO_YM2610A },		// 24
};

STD_ROM_PICK(rambo3p)
STD_ROM_FN(rambo3p)

static INT32 Rambo3pInit()
{
	INT32 nRet = CommonInit(TetristInitCallback, 0, 0, 0, 1, 6);

	if (nRet == 0) {
		memmove (Taito68KRom1 + 0x40000, Taito68KRom1 + 0x20000, 0x40000);
	}

	return nRet;
}

struct BurnDriver BurnDrvRambo3p = {
	"rambo3p", "rambo3", NULL, NULL, "1989",
	"Rambo III (Europe, Proti?)\0", NULL, "Taito Europe Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_SHOOT, 0,
	NULL, rambo3pRomInfo, rambo3pRomName, NULL, NULL, CommonInputInfo, Rambo3DIPInfo,
	Rambo3pInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Crime City (World)

static struct BurnRomInfo crimecRomDesc[] = {
	{ "b99-07.40",			0x020000, 0x26e886e6, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b99-05.29",			0x020000, 0xff7f9a9d, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b99-06.39",			0x020000, 0x1f26aa92, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b99-14.28",			0x020000, 0x71c8b4d7, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b99-08.45",			0x010000, 0x26135451, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "b99-02.18",			0x080000, 0x2a5d4a26, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b99-01.19",			0x080000, 0xa19e373a, TAITO_CHARS },		//  6

	{ "b99-03.37",			0x080000, 0xdda10df7, TAITO_YM2610A },		//  7 YM2610 A Samples
};

STD_ROM_PICK(crimec)
STD_ROM_FN(crimec)

static INT32 CrimecInit()
{
	return CommonInit(CrimecInitCallback, 0, 1, 0, 3, 5);
}

struct BurnDriver BurnDrvCrimec = {
	"crimec", NULL, NULL, NULL, "1989",
	"Crime City (World)\0", NULL, "Taito Corporation Japan", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_PLATFORM, 0,
	NULL, crimecRomInfo, crimecRomName, NULL, NULL, CommonInputInfo, CrimecDIPInfo,
	CrimecInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Crime City (US)

static struct BurnRomInfo crimecuRomDesc[] = {
	{ "b99-07.40",			0x020000, 0x26e886e6, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b99-05.29",			0x020000, 0xff7f9a9d, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b99-06.39",			0x020000, 0x1f26aa92, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b99-13.28",			0x020000, 0x06cf8441, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b99-08.45",			0x010000, 0x26135451, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "b99-02.18",			0x080000, 0x2a5d4a26, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b99-01.19",			0x080000, 0xa19e373a, TAITO_CHARS },		//  6

	{ "b99-03.37",			0x080000, 0xdda10df7, TAITO_YM2610A },		//  7 YM2610 A Samples
};

STD_ROM_PICK(crimecu)
STD_ROM_FN(crimecu)

struct BurnDriver BurnDrvCrimecu = {
	"crimecu", "crimec", NULL, NULL, "1989",
	"Crime City (US)\0", NULL, "Taito America Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_PLATFORM, 0,
	NULL, crimecuRomInfo, crimecuRomName, NULL, NULL, CommonInputInfo, CrimecuDIPInfo,
	CrimecInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Crime City (Japan)

static struct BurnRomInfo crimecjRomDesc[] = {
	{ "b99-07.40",			0x020000, 0x26e886e6, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "b99-05.29",			0x020000, 0xff7f9a9d, TAITO_68KROM1_BYTESWAP }, //  1
	{ "b99-06.39",			0x020000, 0x1f26aa92, TAITO_68KROM1_BYTESWAP }, //  2
	{ "b99-15.28",			0x020000, 0xe8c1e56d, TAITO_68KROM1_BYTESWAP }, //  3

	{ "b99-08.45",			0x010000, 0x26135451, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "b99-02.18",			0x080000, 0x2a5d4a26, TAITO_CHARS },		//  5 Graphics Tiles
	{ "b99-01.19",			0x080000, 0xa19e373a, TAITO_CHARS },		//  6

	{ "b99-03.37",			0x080000, 0xdda10df7, TAITO_YM2610A },		//  7 YM2610 A Samples
};

STD_ROM_PICK(crimecj)
STD_ROM_FN(crimecj)

struct BurnDriver BurnDrvCrimecj = {
	"crimecj", "crimec", NULL, NULL, "1989",
	"Crime City (Japan)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_PLATFORM, 0,
	NULL, crimecjRomInfo, crimecjRomName, NULL, NULL, CommonInputInfo, CrimecjDIPInfo,
	CrimecInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Tetris (Japan, B-System, YM2610)

static struct BurnRomInfo tetristRomDesc[] = {
	{ "c12-03.bin",			0x020000, 0x38f1ed41, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c12-02.bin",			0x020000, 0xed9530bc, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c12-05.bin",			0x020000, 0x128e9927, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c12-04.bin",			0x020000, 0x5da7a319, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c12-06.bin",			0x010000, 0xf2814b38, TAITO_Z80ROM1 },		//  4 Z80 Code
};

STD_ROM_PICK(tetrist)
STD_ROM_FN(tetrist)

static INT32 TetristInit()
{
	return CommonInit(TetristInitCallback, 0, 0, 0, 2, 4);
}

struct BurnDriver BurnDrvTetrist = {
	"tetrist", "tetris", NULL, NULL, "1989",
	"Tetris (Japan, B-System, YM2610)\0", NULL, "Sega", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_PUZZLE, 0,
	NULL, tetristRomInfo, tetristRomName, NULL, NULL, CommonInputInfo, TetristDIPInfo,
	TetristInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Tetris (Japan, B-System, YM2203)

static struct BurnRomInfo tetristaRomDesc[] = {
	{ "c35-04.bin",			0x020000, 0xfa6e42ff, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c35-03.bin",			0x020000, 0xaebd8539, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c35-02.bin",			0x020000, 0x128e9927, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c35-01.bin",			0x020000, 0x5da7a319, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c35-05.bin",			0x010000, 0x785c63fb, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "b72-02.rom",			0x080000, 0x843444eb, BRF_OPT },		//  5 Graphics Tiles (not used)
	{ "b72-01.rom",			0x080000, 0xa24ac26e, BRF_OPT },		//  6
};

STD_ROM_PICK(tetrista)
STD_ROM_FN(tetrista)

static INT32 TetristaInit()
{
	return CommonInit(TetristaInitCallback, 1, 2, 0, 4, 5);
}

struct BurnDriver BurnDrvTetrista = {
	"tetrista", "tetris", NULL, NULL, "1989",
	"Tetris (Japan, B-System, YM2203)\0", NULL, "Sega", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_PUZZLE, 0,
	NULL, tetristaRomInfo, tetristaRomName, NULL, NULL, CommonInputInfo, TetristDIPInfo,
	TetristaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Violence Fight (World)

static struct BurnRomInfo viofightRomDesc[] = {
	{ "c16-11.42",			0x010000, 0x23dbd388, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c16-14.23",			0x010000, 0xdc934f6a, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c16-07.41",			0x020000, 0x64d1d059, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c16-06.22",			0x020000, 0x043761d8, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c16-12.32",			0x010000, 0x6fb028c7, TAITO_Z80ROM1 },          //  4 Z80 Code

	{ "c16-01.1",			0x080000, 0x7059ce83, TAITO_CHARS },            //  5 Graphics Tiles
	{ "c16-02.2",			0x080000, 0xb458e905, TAITO_CHARS },            //  6
	{ "c16-03.3",			0x080000, 0x515a9431, TAITO_CHARS },            //  7
	{ "c16-04.4",			0x080000, 0xebf285e2, TAITO_CHARS },            //  8

	{ "c16-05.47",			0x080000, 0xa49d064a, TAITO_MSM6295 },          //  9 MSM6295 Samples

	{ "pal16l8b-c16-08.ic34",	0x000104, 0x9456d278, BRF_OPT },		// 10 PLDs
	{ "pal16l8b-c16-09.ic35",	0x000104, 0x0965baab, BRF_OPT },		// 11
};

STD_ROM_PICK(viofight)
STD_ROM_FN(viofight)

static INT32 ViofightInit()
{
	return CommonInit(ViofightInitCallback, 1, 2, 0, 1, 4);
}

struct BurnDriver BurnDrvViofight = {
	"viofight", NULL, NULL, NULL, "1989",
	"Violence Fight (World)\0", NULL, "Taito Corporation Japan", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_VSFIGHT, 0,
	NULL, viofightRomInfo, viofightRomName, NULL, NULL, ViofightInputInfo, ViofightDIPInfo,
	ViofightInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Violence Fight (US)

static struct BurnRomInfo viofightuRomDesc[] = {
	{ "c16-11.42",			0x010000, 0x23dbd388, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c16-13.23",			0x010000, 0xab947ffc, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c16-07.41",			0x020000, 0x64d1d059, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c16-06.22",			0x020000, 0x043761d8, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c16-12.32",			0x010000, 0x6fb028c7, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "c16-01.1",			0x080000, 0x7059ce83, TAITO_CHARS },		//  5 Graphics Tiles
	{ "c16-02.2",			0x080000, 0xb458e905, TAITO_CHARS },		//  6
	{ "c16-03.3",			0x080000, 0x515a9431, TAITO_CHARS },		//  7
	{ "c16-04.4",			0x080000, 0xebf285e2, TAITO_CHARS }, 		//  8

	{ "c16-05.47",			0x080000, 0xa49d064a, TAITO_MSM6295 },		//  9 MSM6295 Samples

	{ "pal16l8b-c16-08.ic34",	0x000104, 0x9456d278, BRF_OPT },		// 10 PLDs
	{ "pal16l8b-c16-09.ic35",	0x000104, 0x0965baab, BRF_OPT },		// 11
};

STD_ROM_PICK(viofightu)
STD_ROM_FN(viofightu)

struct BurnDriver BurnDrvViofightu = {
	"viofightu", "viofight", NULL, NULL, "1989",
	"Violence Fight (US)\0", NULL, "Taito America Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_VSFIGHT, 0,
	NULL, viofightuRomInfo, viofightuRomName, NULL, NULL, ViofightInputInfo, ViofightDIPInfo,
	ViofightInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Violence Fight (Japan)

static struct BurnRomInfo viofightjRomDesc[] = {
	{ "c16-11.42",			0x010000, 0x23dbd388, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c16-10.23",			0x010000, 0x329d2e46, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c16-07.41",			0x020000, 0x64d1d059, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c16-06.22",			0x020000, 0x043761d8, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c16-12.32",			0x010000, 0x6fb028c7, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "c16-01.1",			0x080000, 0x7059ce83, TAITO_CHARS },		//  5 Graphics Tiles
	{ "c16-02.2",			0x080000, 0xb458e905, TAITO_CHARS },		//  6
	{ "c16-03.3",			0x080000, 0x515a9431, TAITO_CHARS },		//  7
	{ "c16-04.4",			0x080000, 0xebf285e2, TAITO_CHARS },		//  8

	{ "c16-05.47",			0x080000, 0xa49d064a, TAITO_MSM6295 },          //  9 MSM6295 Samples

	{ "pal16l8b-c16-08.ic34",	0x000104, 0x9456d278, BRF_OPT },		// 10 PLDs
	{ "pal16l8b-c16-09.ic35",	0x000104, 0x0965baab, BRF_OPT },		// 11
};

STD_ROM_PICK(viofightj)
STD_ROM_FN(viofightj)

struct BurnDriver BurnDrvViofightj = {
	"viofightj", "viofight", NULL, NULL, "1989",
	"Violence Fight (Japan)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_VSFIGHT, 0,
	NULL, viofightjRomInfo, viofightjRomName, NULL, NULL, ViofightInputInfo, ViofightDIPInfo,
	ViofightInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Ashura Blaster (World)

static struct BurnRomInfo ashuraRomDesc[] = {
	{ "c43-15.50",			0x020000, 0x5d05d6c6, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c43-13.31",			0x020000, 0x75b7d877, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c43-14.49",			0x020000, 0xede7f37d, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c43-12.30",			0x020000, 0xb08a4ba0, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c43-16",			0x010000, 0xcb26fce1, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "c43-02",			0x080000, 0x105722ae, TAITO_CHARS },		//  5 Graphics Tiles
	{ "c43-03",			0x080000, 0x426606ba, TAITO_CHARS },		//  6

	{ "c43-01",			0x080000, 0xdb953f37, TAITO_YM2610A },		//  7 YM2610 A Samples
};

STD_ROM_PICK(ashura)
STD_ROM_FN(ashura)

struct BurnDriver BurnDrvAshura = {
	"ashura", NULL, NULL, NULL, "1990",
	"Ashura Blaster (World)\0", NULL, "Taito Corporation Japan", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TAITO_TAITOB, GBF_VERSHOOT, 0,
	NULL, ashuraRomInfo, ashuraRomName, NULL, NULL, CommonInputInfo, AshuraDIPInfo,
	NastarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	224, 320, 3, 4
};


// Ashura Blaster (Japan)

static struct BurnRomInfo ashurajRomDesc[] = {
	{ "c43-07-1.50",		0x020000, 0xd5ceb20f, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c43-05-1.31",		0x020000, 0xa6f3bb37, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c43-06-1.49",		0x020000, 0x0f331802, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c43-04-1.30",		0x020000, 0xe06a2414, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c43-16",			0x010000, 0xcb26fce1, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "c43-02",			0x080000, 0x105722ae, TAITO_CHARS },		//  5 Graphics Tiles
	{ "c43-03",			0x080000, 0x426606ba, TAITO_CHARS },		//  6

	{ "c43-01",			0x080000, 0xdb953f37, TAITO_YM2610A },		//  7 YM2610 A Samples
};

STD_ROM_PICK(ashuraj)
STD_ROM_FN(ashuraj)

struct BurnDriver BurnDrvAshuraj = {
	"ashuraj", "ashura", NULL, NULL, "1990",
	"Ashura Blaster (Japan)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TAITO_TAITOB, GBF_VERSHOOT, 0,
	NULL, ashurajRomInfo, ashurajRomName, NULL, NULL, CommonInputInfo, AshurajDIPInfo,
	NastarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	224, 320, 3, 4
};


// Ashura Blaster (US)

static struct BurnRomInfo ashurauRomDesc[] = {
	{ "c43-11.50",			0x020000, 0xd5aefc9b, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c43-09.31",			0x020000, 0xe91d0ab1, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c43-10.49",			0x020000, 0xc218e7ea, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c43-08.30",			0x020000, 0x5ef4f19f, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c43-16",			0x010000, 0xcb26fce1, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "c43-02",			0x080000, 0x105722ae, TAITO_CHARS },		//  5 Graphics Tiles
	{ "c43-03",			0x080000, 0x426606ba, TAITO_CHARS },		//  6

	{ "c43-01",			0x080000, 0xdb953f37, TAITO_YM2610A },		//  7 YM2610 A Samples
};

STD_ROM_PICK(ashurau)
STD_ROM_FN(ashurau)

struct BurnDriver BurnDrvAshurau = {
	"ashurau", "ashura", NULL, NULL, "1990",
	"Ashura Blaster (US)\0", NULL, "Taito America Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TAITO_TAITOB, GBF_VERSHOOT, 0,
	NULL, ashurauRomInfo, ashurauRomName, NULL, NULL, CommonInputInfo, AshurauDIPInfo,
	NastarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	224, 320, 3, 4
};


// Hit the Ice (US)

static struct BurnRomInfo hiticeRomDesc[] = {
	{ "c59-10.42",			0x020000, 0xe4ffad15, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c59-12.64",			0x020000, 0xa080d7af, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c59-09.41",			0x010000, 0xe243e3b0, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c59-11.63",			0x010000, 0x4d4dfa52, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c59-08.50",			0x010000, 0xd3cbc10b, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "c59-03.12",			0x080000, 0x9e513048, TAITO_CHARS },		//  5 Graphics Tiles
	{ "c59-02.13",			0x080000, 0xaffb5e07, TAITO_CHARS },		//  6

	{ "c59-01.30",			0x020000, 0x46ae291d, TAITO_MSM6295 },		//  7 MSM6295 Samples

	{ "pal20l8b-c59-04.25",		0x000144, 0x2ebcf07c, BRF_OPT }, 		//  8 PLDs
	{ "pal16l8b-c59-05.26",		0x000104, 0x37b67c5c, BRF_OPT }, 		//  9
	{ "pal20l8b-c59-06.53",		0x000144, 0xbf176875, BRF_OPT }, 		// 10
	{ "pal16r4b-c59-07.61",		0x000104, 0xcf64bd95, BRF_OPT }, 		// 11
};

STD_ROM_PICK(hitice)
STD_ROM_FN(hitice)

static INT32 HiticeInit()
{
	return CommonInit(HiticeInitCallback, 1, 0, 1, 6, 4);
}

struct BurnDriver BurnDrvHitice = {
	"hitice", NULL, NULL, NULL, "1990",
	"Hit the Ice (US)\0", "Imperfect graphics", "Williams", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_SPORTSMISC, 0,
	NULL, hiticeRomInfo, hiticeRomName, NULL, NULL, HiticeInputInfo, HiticeDIPInfo,
	HiticeInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Hit the Ice (Japan)

static struct BurnRomInfo hiticejRomDesc[] = {
	{ "c59-23.42",			0x020000, 0x01958fcc, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c59-25.64",			0x020000, 0x71984c76, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c59-22.41",			0x010000, 0xc2c86140, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c59-24.63",			0x010000, 0x26c8f409, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c59-08.50",			0x010000, 0xd3cbc10b, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "c59-03.12",			0x080000, 0x9e513048, TAITO_CHARS },		//  5 Graphics Tiles
	{ "c59-02.13",			0x080000, 0xaffb5e07, TAITO_CHARS },		//  6

	{ "c59-01.30",			0x020000, 0x46ae291d, TAITO_MSM6295 },		//  7 MSM6295 Samples

	{ "pal20l8b-c59-04.25",		0x000144, 0x2ebcf07c, BRF_OPT }, 		//  8 PLDs
	{ "pal16l8b-c59-05.26",		0x000104, 0x37b67c5c, BRF_OPT }, 		//  9
	{ "pal20l8b-c59-06.53",		0x000144, 0xbf176875, BRF_OPT }, 		// 10
	{ "pal16r4b-c59-07.61",		0x000104, 0xcf64bd95, BRF_OPT }, 		// 11
};

STD_ROM_PICK(hiticej)
STD_ROM_FN(hiticej)

struct BurnDriver BurnDrvHiticej = {
	"hiticej", "hitice", NULL, NULL, "1990",
	"Hit the Ice (Japan)\0", "Imperfect graphics", "Midway/Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_SPORTSMISC, 0,
	NULL, hiticejRomInfo, hiticejRomName, NULL, NULL, HiticeInputInfo, HiticeDIPInfo,
	HiticeInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Sel Feena

static struct BurnRomInfo selfeenaRomDesc[] = {
	{ "se-02.27",			0x020000, 0x08f0c8e3, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "se-01.26",			0x020000, 0xa06ca64b, TAITO_68KROM1_BYTESWAP }, //  1

	{ "se-03.39",			0x010000, 0x675998be, TAITO_Z80ROM1 },		//  2 Z80 Code

	{ "se-04.2",			0x080000, 0x920ad100, TAITO_CHARS },		//  3 Graphics Tiles
	{ "se-05.1",			0x080000, 0xd297c995, TAITO_CHARS },		//  4

	{ "se-06.11",			0x080000, 0x80d5e772, TAITO_YM2610A },		//  5 YM2610 A Samples
};

STD_ROM_PICK(selfeena)
STD_ROM_FN(selfeena)

static INT32 SelfeenaInit()
{
	return CommonInit(SelfeenaInitCallback, 0, 2, 1, 4, 6);
}

struct BurnDriver BurnDrvSelfeena = {
	"selfeena", NULL, NULL, NULL, "1991",
	"Sel Feena\0", NULL, "East Technology", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_MAZE, 0,
	NULL, selfeenaRomInfo, selfeenaRomName, NULL, NULL, SelfeenaInputInfo, SelfeenaDIPInfo,
	SelfeenaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Silent Dragon (World)

static struct BurnRomInfo silentdRomDesc[] = {
	{ "east-12-1.ic32",		0x020000, 0x5883d362, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "east-15-1.ic10",		0x020000, 0x8c0a72ae, TAITO_68KROM1_BYTESWAP }, //  1
	{ "east-11.ic31",		0x020000, 0x35da4428, TAITO_68KROM1_BYTESWAP }, //  2
	{ "east-09.ic9",		0x020000, 0x2f05b14a, TAITO_68KROM1_BYTESWAP }, //  3

	{ "east-13.ic15",		0x010000, 0x651861ab, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "east-04.ic28",		0x100000, 0x53237217, TAITO_CHARS },		//  5 Graphics Tiles
	{ "east-06.ic29",		0x100000, 0xe6e6dfa7, TAITO_CHARS },		//  6
	{ "east-03.ic39",		0x100000, 0x1b9b2846, TAITO_CHARS },		//  7
	{ "east-05.ic40",		0x100000, 0xe02472c5, TAITO_CHARS },		//  8

	{ "east-01.ic1",		0x080000, 0xb41fff1a, TAITO_YM2610A },		//  9 YM2610 A Samples

	{ "east-02.ic3",		0x080000, 0xe0de5c39, TAITO_YM2610B },		// 10 YM2610 B Samples
};

STD_ROM_PICK(silentd)
STD_ROM_FN(silentd)

static INT32 SilentdInit()
{
	INT32 nRet = CommonInit(SilentdInitCallback, 0, 2, 2, 4, 6);

	if (nRet == 0) {
		cpu_speed[0] = 16000000; // 16mhz
	}

	return nRet;
}

struct BurnDriver BurnDrvSilentd = {
	"silentd", NULL, NULL, NULL, "1992",
	"Silent Dragon (World)\0", NULL, "Taito Corporation Japan", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_SCRFIGHT, 0,
	NULL, silentdRomInfo, silentdRomName, NULL, NULL, SilentdInputInfo, SilentdDIPInfo,
	SilentdInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Silent Dragon (Japan)

static struct BurnRomInfo silentdjRomDesc[] = {
	{ "east-12-1.ic32",		0x020000, 0x5883d362, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "east-10-1.ic10",		0x020000, 0x584306fc, TAITO_68KROM1_BYTESWAP }, //  1
	{ "east-11.ic31",		0x020000, 0x35da4428, TAITO_68KROM1_BYTESWAP }, //  2
	{ "east-09.ic9",		0x020000, 0x2f05b14a, TAITO_68KROM1_BYTESWAP }, //  3

	{ "east-13.ic15",		0x010000, 0x651861ab, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "east-04.ic28",		0x100000, 0x53237217, TAITO_CHARS },		//  5 Graphics Tiles
	{ "east-06.ic29",		0x100000, 0xe6e6dfa7, TAITO_CHARS },		//  6
	{ "east-03.ic39",		0x100000, 0x1b9b2846, TAITO_CHARS },		//  7
	{ "east-05.ic40",		0x100000, 0xe02472c5, TAITO_CHARS },		//  8

	{ "east-01.ic1",		0x080000, 0xb41fff1a, TAITO_YM2610A },		//  9 YM2610 A Samples

	{ "east-02.ic3",		0x080000, 0xe0de5c39, TAITO_YM2610B },		// 10 YM2610 B Samples
};

STD_ROM_PICK(silentdj)
STD_ROM_FN(silentdj)

struct BurnDriver BurnDrvSilentdj = {
	"silentdj", "silentd", NULL, NULL, "1992",
	"Silent Dragon (Japan)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_SCRFIGHT, 0,
	NULL, silentdjRomInfo, silentdjRomName, NULL, NULL, SilentdInputInfo, SilentdjDIPInfo,
	SilentdInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Silent Dragon (US)

static struct BurnRomInfo silentduRomDesc[] = {
	{ "east-12-1.ic32",		0x020000, 0x5883d362, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "east-14-1.ic10",		0x020000, 0x3267bcd5, TAITO_68KROM1_BYTESWAP }, //  1
	{ "east-11.ic31",		0x020000, 0x35da4428, TAITO_68KROM1_BYTESWAP }, //  2
	{ "east-09.ic9",		0x020000, 0x2f05b14a, TAITO_68KROM1_BYTESWAP }, //  3

	{ "east-13.ic15",		0x010000, 0x651861ab, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "east-04.ic28",		0x100000, 0x53237217, TAITO_CHARS },		//  5 Graphics Tiles
	{ "east-06.ic29",		0x100000, 0xe6e6dfa7, TAITO_CHARS },		//  6
	{ "east-03.ic39",		0x100000, 0x1b9b2846, TAITO_CHARS },		//  7
	{ "east-05.ic40",		0x100000, 0xe02472c5, TAITO_CHARS },		//  8

	{ "east-01.ic1",		0x080000, 0xb41fff1a, TAITO_YM2610A },		//  9 YM2610 A Samples

	{ "east-02.ic3",		0x080000, 0xe0de5c39, TAITO_YM2610B },		// 10 YM2610 B Samples
};

STD_ROM_PICK(silentdu)
STD_ROM_FN(silentdu)

struct BurnDriver BurnDrvSilentdu = {
	"silentdu", "silentd", NULL, NULL, "1992",
	"Silent Dragon (US)\0", NULL, "Taito America Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_SCRFIGHT, 0,
	NULL, silentduRomInfo, silentduRomName, NULL, NULL, SilentdInputInfo, SilentdjDIPInfo,
	SilentdInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Ryu Jin (Japan)

static struct BurnRomInfo ryujinRomDesc[] = {
	{ "ruj02.27",			0x020000, 0x0d223aee, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "ruj01.26",			0x020000, 0xc6bcdd1e, TAITO_68KROM1_BYTESWAP }, //  1
	{ "ruj04.29",			0x020000, 0x0c153cab, TAITO_68KROM1_BYTESWAP }, //  2
	{ "ruj03.28",			0x020000, 0x7695f89c, TAITO_68KROM1_BYTESWAP }, //  3

	{ "ruj05.39",			0x010000, 0x95270b16, TAITO_Z80ROM1 },		//  4 Z80 Code

	{ "ryujin07.2",			0x100000, 0x34f50980, TAITO_CHARS },		//  5 Graphics Tiles
	{ "ryujin06.1",			0x100000, 0x1b85ff34, TAITO_CHARS },		//  6

	{ "ryujin08.11",		0x080000, 0x480d040d, TAITO_YM2610A },		//  7 YM2610 A Samples
};

STD_ROM_PICK(ryujin)
STD_ROM_FN(ryujin)

struct BurnDriver BurnDrvRyujin = {
	"ryujin", NULL, NULL, NULL, "1993",
	"Ryu Jin (Japan)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TAITO_TAITOB, GBF_VERSHOOT, 0,
	NULL, ryujinRomInfo, ryujinRomName, NULL, NULL, SelfeenaInputInfo, RyujinDIPInfo,
	SelfeenaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	224, 320, 3, 4
};


// Quiz Sekai wa SHOW by shobai (Japan)

static struct BurnRomInfo qzshowbyRomDesc[] = {
	{ "d72-13.bin",			0x080000, 0xa867759f, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "d72-12.bin",			0x080000, 0x522c09a7, TAITO_68KROM1_BYTESWAP }, //  1

	{ "d72-11.bin",			0x020000, 0x2ca046e2, TAITO_Z80ROM1 },		//  2 Z80 Code

	{ "d72-03.bin",			0x200000, 0x1de257d0, TAITO_CHARS },		//  3 Graphics Tiles
	{ "d72-02.bin",			0x200000, 0xbf0da640, TAITO_CHARS },		//  4

	{ "d72-01.bin",			0x200000, 0xb82b8830, TAITO_YM2610A },		//  5 YM2610 A Samples
	
	{ "pal16l8-d72-05.bin",		  0x0104, 0xc3d4cb7e, BRF_OPT },		//  6 PLDs
	{ "pal16l8-d72-06.bin",		  0x0104, 0x27580efc, BRF_OPT }, 		//  7
	{ "palce20v8-d72-07.bin",	  0x0157, 0x6359e64c, BRF_OPT }, 		//  8
	{ "palce20v8-d72-08.bin",	  0x0157, 0x746a6474, BRF_OPT }, 		//  9
	{ "palce20v8-d72-09.bin",         0x0157, 0x9f680800, BRF_OPT }, 		// 10
	{ "palce16v8-d72-10.bin",         0x0117, 0xa5181ba2, BRF_OPT }, 		// 11
};

STD_ROM_PICK(qzshowby)
STD_ROM_FN(qzshowby)

static INT32 QzshowbyInit()
{
	return CommonInit(QzshowbyInitCallback, 0, 1, 3, 5, 3);
}

struct BurnDriver BurnDrvQzshowby = {
	"qzshowby", NULL, NULL, NULL, "1993",
	"Quiz Sekai wa SHOW by shobai (Japan)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_QUIZ, 0,
	NULL, qzshowbyRomInfo, qzshowbyRomName, NULL, NULL, QzshowbyInputInfo, QzshowbyDIPInfo,
	QzshowbyInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Puzzle Bobble (Japan, B-System)

static struct BurnRomInfo pbobbleRomDesc[] = {
	{ "pb-1c18.bin",		0x040000, 0x5de14f49, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "pb-ic2.bin",			0x040000, 0x2abe07d1, TAITO_68KROM1_BYTESWAP }, //  1

	{ "pb-ic27.bin",		0x020000, 0x26efa4c4, TAITO_Z80ROM1 },		//  2 Z80 Code

	{ "pb-ic14.bin",		0x080000, 0x55f90ea4, TAITO_CHARS },		//  3 Graphics Tiles
	{ "pb-ic9.bin",			0x080000, 0x3253aac9, TAITO_CHARS },		//  4

	{ "pb-ic15.bin",		0x100000, 0x0840cbc4, TAITO_YM2610A },		//  5 YM2610 A Samples
};

STD_ROM_PICK(pbobble)
STD_ROM_FN(pbobble)

static INT32 PbobbleInit()
{
	return CommonInit(PbobbleInitCallback, 0, 1, 3, 5, 3);
}

struct BurnDriver BurnDrvPbobble = {
	"pbobble", NULL, NULL, NULL, "1994",
	"Puzzle Bobble (Japan, B-System)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_MISC, 0,
	NULL, pbobbleRomInfo, pbobbleRomName, NULL, NULL, PbobbleInputInfo, PbobbleDIPInfo,
	PbobbleInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Space Invaders DX (US, v2.1)

static struct BurnRomInfo spacedxRomDesc[] = {
	{ "d89-06",			0x040000, 0x7122751e, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "d89-xx.ic2",			0x040000, 0x56b0be6c, TAITO_68KROM1_BYTESWAP }, //  1

	{ "d89-07.27",			0x010000, 0xbd743401, TAITO_Z80ROM1 },		//  2 Z80 Code

	{ "d89-02.14",			0x080000, 0xc36544b9, TAITO_CHARS },		//  3 Graphics Tiles
	{ "d89-01.9",			0x080000, 0xfffa0660, TAITO_CHARS },		//  4

	{ "d89-03.15",			0x080000, 0x218f31a4, TAITO_YM2610A },		//  5 YM2610 A Samples

	{ "pal16l8-d72-05.ic37",	  0x0104, 0xc3d4cb7e, BRF_OPT },		//  6 PLDs
	{ "pal16l8-d72-06.ic50",	  0x0104, 0xe96b7f37, BRF_OPT }, 		//  7
	{ "palce20v8-d72-07.ic28",	  0x0157, 0x6359e64c, BRF_OPT }, 		//  8
	{ "palce20v8-d72-09.ic47",	  0x0157, 0xde1760fd, BRF_OPT }, 		//  9
	{ "palce16v8-d72-10.ic12",        0x0117, 0xa5181ba2, BRF_OPT }, 		// 10
	{ "pal20l8b-d89-04.ic40",	  0x0144, 0x00000000, BRF_OPT | BRF_NODUMP },	// 12
};

STD_ROM_PICK(spacedx)
STD_ROM_FN(spacedx)

struct BurnDriver BurnDrvSpacedx = {
	"spacedx", NULL, NULL, NULL, "1994",
	"Space Invaders DX (US, v2.1)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_MISC, 0,
	NULL, spacedxRomInfo, spacedxRomName, NULL, NULL, PbobbleInputInfo, PbobbleDIPInfo,
	PbobbleInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Space Invaders DX (Japan, v2.1)

static struct BurnRomInfo spacedxjRomDesc[] = {
	{ "d89-06",			0x040000, 0x7122751e, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "d89-05",			0x040000, 0xbe1638af, TAITO_68KROM1_BYTESWAP }, //  1

	{ "d89-07.27",			0x010000, 0xbd743401, TAITO_Z80ROM1 },		//  2 Z80 Code

	{ "d89-02.14",			0x080000, 0xc36544b9, TAITO_CHARS },		//  3 Graphics Tiles
	{ "d89-01.9",			0x080000, 0xfffa0660, TAITO_CHARS },		//  4

	{ "d89-03.15",			0x080000, 0x218f31a4, TAITO_YM2610A },		//  5 YM2610 A Samples

	{ "pal16l8-d72-05.ic37",	  0x0104, 0xc3d4cb7e, BRF_OPT },		//  6 PLDs
	{ "pal16l8-d72-06.ic50",	  0x0104, 0xe96b7f37, BRF_OPT }, 		//  7
	{ "palce20v8-d72-07.ic28",	  0x0157, 0x6359e64c, BRF_OPT }, 		//  8
	{ "palce20v8-d72-09.ic47",	  0x0157, 0xde1760fd, BRF_OPT }, 		//  9
	{ "palce16v8-d72-10.ic12",        0x0117, 0xa5181ba2, BRF_OPT }, 		// 10
	{ "pal20l8b-d89-04.ic40",	  0x0144, 0x00000000, BRF_OPT | BRF_NODUMP },	// 12
};

STD_ROM_PICK(spacedxj)
STD_ROM_FN(spacedxj)

struct BurnDriver BurnDrvSpacedxj = {
	"spacedxj", "spacedx", NULL, NULL, "1994",
	"Space Invaders DX (Japan, v2.1)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_MISC, 0,
	NULL, spacedxjRomInfo, spacedxjRomName, NULL, NULL, PbobbleInputInfo, PbobbleDIPInfo,
	PbobbleInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Space Invaders DX (Japan, v2.0)

static struct BurnRomInfo spacedxoRomDesc[] = {
	{ "d89-08.bin",			0x020000, 0x0c2fe7f9, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "d89-09.bin",			0x020000, 0x7f0a0ba4, TAITO_68KROM1_BYTESWAP }, //  1

	{ "d89-07.27",			0x010000, 0xbd743401, TAITO_Z80ROM1 },		//  2 Z80 Code

	{ "d89-12.bin",			0x080000, 0x53df86f1, TAITO_CHARS },		//  3 Graphics Tiles
	{ "d89-13.bin",			0x080000, 0xc44c1352, TAITO_CHARS },		//  4

	{ "d89-03.15",			0x080000, 0x218f31a4, TAITO_YM2610A },		//  5 YM2610 A Samples
};

STD_ROM_PICK(spacedxo)
STD_ROM_FN(spacedxo)

static INT32 SpacedxoInit()
{
	return CommonInit(SilentdInitCallback, 0, 2, 3, 4, 6);
}

struct BurnDriver BurnDrvSpacedxo = {
	"spacedxo", "spacedx", NULL, NULL, "1994",
	"Space Invaders DX (Japan, v2.0)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOB, GBF_MISC, 0,
	NULL, spacedxoRomInfo, spacedxoRomName, NULL, NULL, SpacedxoInputInfo, SpacedxoDIPInfo,
	SpacedxoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};


// Sonic Blast Man (Japan)

static struct BurnRomInfo sbmRomDesc[] = {
	{ "c69-20-1.10",		0x020000, 0xb40e4910, TAITO_68KROM1_BYTESWAP }, //  0 68k Code
	{ "c69-22-1.12",		0x020000, 0xecbcf830, TAITO_68KROM1_BYTESWAP }, //  1
	{ "c69-19-1.9",			0x020000, 0x5719c158, TAITO_68KROM1_BYTESWAP }, //  2
	{ "c69-21-1.11",		0x020000, 0x73562394, TAITO_68KROM1_BYTESWAP }, //  3

	{ "c69-23.28",			0x010000, 0xb2fce13e, TAITO_Z80ROM1 },		//  4 Z80 Code

	// Some roms are repeated to compensate for the way the taito rom loading system in fba works.
	{ "c69-01.ic5",			0x100000, 0x521fabe3, TAITO_CHARS },		//  5 Graphics Tiles
	{ "c69-13.ic2",			0x020000, 0xd1550884, TAITO_CHARS_BYTESWAP },	//  6
	{ "c69-12.ic1",			0x020000, 0xeb56582c, TAITO_CHARS_BYTESWAP },	//  7
	{ "c69-13.ic2",			0x020000, 0xd1550884, TAITO_CHARS_BYTESWAP },	//  8
	{ "c69-12.ic1",			0x020000, 0xeb56582c, TAITO_CHARS_BYTESWAP },	//  9
	{ "c69-13.ic2",			0x020000, 0xd1550884, TAITO_CHARS_BYTESWAP },	// 10
	{ "c69-12.ic1",			0x020000, 0xeb56582c, TAITO_CHARS_BYTESWAP },	// 11
	{ "c69-13.ic2",			0x020000, 0xd1550884, TAITO_CHARS_BYTESWAP },	// 12
	{ "c69-12.ic1",			0x020000, 0xeb56582c, TAITO_CHARS_BYTESWAP },	// 13
	{ "c69-02.ic6",			0x100000, 0xf0e20d35, TAITO_CHARS },		// 14
	{ "c69-15.ic4",			0x020000, 0x9761d316, TAITO_CHARS_BYTESWAP },	// 15
	{ "c69-14.ic3",			0x020000, 0x0ed0272a, TAITO_CHARS_BYTESWAP },	// 16
	{ "c69-15.ic4",			0x020000, 0x9761d316, TAITO_CHARS_BYTESWAP },	// 17
	{ "c69-14.ic3",			0x020000, 0x0ed0272a, TAITO_CHARS_BYTESWAP },	// 18
	{ "c69-15.ic4",			0x020000, 0x9761d316, TAITO_CHARS_BYTESWAP },	// 19
	{ "c69-14.ic3",			0x020000, 0x0ed0272a, TAITO_CHARS_BYTESWAP },	// 20
	{ "c69-15.ic4",			0x020000, 0x9761d316, TAITO_CHARS_BYTESWAP },	// 21
	{ "c69-14.ic3",			0x020000, 0x0ed0272a, TAITO_CHARS_BYTESWAP },	// 22

	{ "c69-03.36",			0x080000, 0x63e6b6e7, TAITO_YM2610A },		// 23 ymsnd
};

STD_ROM_PICK(sbm)
STD_ROM_FN(sbm)

static INT32 SbmInit()
{
	nTaitoInputConfig[2] = 0x60;

	return CommonInit(SbmInitCallback, 0, 0, 0, 5, 4);
}

struct BurnDriver BurnDrvSbm = {
	"sbm", NULL, NULL, NULL, "1990",
	"Sonic Blast Man (Japan)\0", NULL, "Taito Corporation", "Taito B System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOB, GBF_MISC, 0,
	NULL, sbmRomInfo, sbmRomName, NULL, NULL, SbmInputInfo, SbmDIPInfo,
	SbmInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x1000,
	320, 224, 4, 3
};
