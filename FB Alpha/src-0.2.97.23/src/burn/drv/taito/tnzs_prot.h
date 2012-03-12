enum
{
	MCU_NONE,
	MCU_EXTRMATN,
	MCU_ARKANOID,
	MCU_DRTOPPEL,
	MCU_CHUKATAI,
	MCU_PLUMPOP,
	MCU_TNZS,
	MCU_NONE_INSECTX,
	MCU_NONE_KAGEKI,
	MCU_NONE_JPOPNICS
};

INT32 tnzs_mcu_type();
void tnzs_mcu_reset();
void tnzs_mcu_init(INT32 type);
void tnzs_mcu_interrupt();
void tnzs_mcu_scan();

UINT8 tnzs_mcu_read(INT32 offset);
void tnzs_mcu_write(INT32 offset, INT32 data);

extern UINT8 *tnzs_mcu_inputs;

#define TNZS_DIFFICULTY_DIPSETTING(dip)	\
	{0   , 0xfe, 0   ,    4, "Difficulty"	},	\
	{dip,  0x01, 0x03, 0x02, "Easy"		},	\
	{dip,  0x01, 0x03, 0x03, "Medium"	},	\
	{dip,  0x01, 0x03, 0x01, "Hard"		},	\
	{dip,  0x01, 0x03, 0x00, "Hardest"	},	\

#define TNZS_LIVES_DIPSETTING(dip)				\
	{0   , 0xfe, 0   ,    0, "Lives"		},	\
	{dip, 0x01, 0x30, 0x20, "2"			},	\
	{dip, 0x01, 0x30, 0x30, "3"			},	\
	{dip, 0x01, 0x30, 0x10, "4"			},	\
	{dip, 0x01, 0x30, 0x00, "5"			},	\

#define TNZS_CABINET_FLIP_SERVICE_DIPSETTING(dip)		\
	{0   , 0xfe, 0   ,    2, "Cabinet"		},	\
	{dip, 0x01, 0x01, 0x00, "Upright"		},	\
	{dip, 0x01, 0x01, 0x01, "Cocktail"		},	\
								\
	{0   , 0xfe, 0   ,    2, "Flip Screen"		},	\
	{dip, 0x01, 0x02, 0x02, "Off"			},	\
	{dip, 0x01, 0x02, 0x00, "On"			},	\
								\
	{0   , 0xfe, 0   ,    2, "Service Mode"		},	\
	{dip, 0x01, 0x04, 0x04, "Off"			},	\
	{dip, 0x01, 0x04, 0x00, "On"			},	\

#define TNZS_DEMOSOUNDS_DIPSETTING(dip)				\
	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},	\
	{dip, 0x01, 0x08, 0x00, "Off"			},	\
	{dip, 0x01, 0x08, 0x08, "On"			},	\

#define TNZS_ALLOWCONTINUE_DIPSETTING(dip, val)			\
	{0   , 0xfe, 0   ,    2, "Allow Continue"	},	\
	{dip, 0x01, val, 0x00, "No"			},	\
	{dip, 0x01, val, val,  "Yes"			},	\

// Type 1 is old-style
#define TNZS_COINA_TYPE1_DIPSETTING(dip)			\
	{0   , 0xfe, 0   ,    4, "Coin A"		},	\
	{dip, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},	\
	{dip, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},	\
	{dip, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},	\
	{dip, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},	\

#define TNZS_COINB_TYPE1_DIPSETTING(dip)			\
	{0   , 0xfe, 0   ,    4, "Coin B"		},	\
	{dip, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},	\
	{dip, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},	\
	{dip, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},	\
	{dip, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},	\

// Type 2 is new-style
#define TNZS_COINA_TYPE2_DIPSETTING(dip)			\
	{0   , 0xfe, 0   ,    4, "Coin A"		},	\
	{dip, 0x01, 0x30, 0x00, "4 Coins 1 Credits"	},	\
	{dip, 0x01, 0x30, 0x10, "3 Coins 1 Credits"	},	\
	{dip, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},	\
	{dip, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},	\

#define TNZS_COINB_TYPE2_DIPSETTING(dip)			\
	{0   , 0xfe, 0   ,    4, "Coin B"		},	\
	{dip, 0x01, 0xc0, 0xc0, "1 Coin  2 Credits"	},	\
	{dip, 0x01, 0xc0, 0x80, "1 Coin  3 Credits"	},	\
	{dip, 0x01, 0xc0, 0x40, "1 Coin  4 Credits"	},	\
	{dip, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	},	\


#define TNZS_COINAGE_WORLD(dip)	\
	TNZS_COINA_TYPE2_DIPSETTING(dip) \
	TNZS_COINB_TYPE2_DIPSETTING(dip)

#define TNZS_COINAGE_JAPAN_OLD(dip) \
	TNZS_COINA_TYPE1_DIPSETTING(dip) \
	TNZS_COINB_TYPE1_DIPSETTING(dip)
