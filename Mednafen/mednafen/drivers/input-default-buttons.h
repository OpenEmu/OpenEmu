// This file should only be included *ONCE* from drivers/input.cpp!!!


#define GPZ()   {MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ()}
static const ButtConfig NESGamePadConfig[4][8]={
        /* Gamepad 1 */
        {
         MK(KP3), MK(KP2), MK(TAB), MK(RETURN), MK(w),MK(s),
                MK(a), MK(d)
        },

        /* Gamepad 2 */
        GPZ(),

        /* Gamepad 3 */
        GPZ(),

        /* Gamepad 4 */
        GPZ()
};
#undef GPZ

static const ButtConfig GBPadConfig[8] =
{
         MK(KP3), MK(KP2), MK(TAB), MK(RETURN), MK(d),MK(a),
                MK(w), MK(s)
};

static const ButtConfig GBAPadConfig[10] =
{
         MK(KP3), MK(KP2), MK(TAB), MK(RETURN), MK(d),MK(a),
                MK(w), MK(s), MK(KP6), MK(KP5)
};

#define GPZ()   {MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ()}
static const ButtConfig PCFXPadConfig[2][15]=
{
        /* Gamepad 1 */
        {
         MK(KP3), MK(KP2), MK(KP1), MK(KP4), MK(KP5), MK(KP6), MK(TAB), MK(RETURN),
         MK(w), MK(d), MK(s), MK(a),
	 MK(KP8), MK(KP9),
        },

        /* Gamepad 2 */
	GPZ(),
};
#undef GPZ

#define GPZ()   {MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ()}
static const ButtConfig PCEPadConfig[5][13]={
        /* Gamepad 1 */
        {
         MK(KP3), MK(KP2), MK(TAB), MK(RETURN), MK(w), MK(d), MK(s), MK(a),

         // Extra 4 buttons on 6-button pad
         MK(KP1), MK(KP4), MK(KP5), MK(KP6),

         // ..and special 2/6 mode select
         MK(m),
        },

        /* Gamepad 2 */
        GPZ(),

        /* Gamepad 3 */
        GPZ(),

        /* Gamepad 4 */
        GPZ(),

        /* Gamepad 5 */
        GPZ()
};
#undef GPZ

static const ButtConfig LynxPadConfig[9] =
{
        // A, B, Option 2, Option 1, Left, Right, Up, Down, Pause
         MK(KP3), MK(KP2), MK(KP1), MK(KP7), MK(a),MK(d),
                MK(w), MK(s), MK(RETURN)
};

static const ButtConfig NGPPadConfig[7 + 2] =
{
        // Up, down, left, right, a(inner), b(outer), option
        MK(w), MK(s), MK(a), MK(d), MK(KP2), MK(KP3), MK(RETURN), MK(KP5), MK(KP6)
};

static const ButtConfig WSwanPadConfig[12] =
{
        // Up, right, down, left,
        // up-y, right-y, down-y, left-y,
	//  start, a(outer), b(inner), option
        MK(w), MK(d), MK(s), MK(a), 
	MK(UP), MK(RIGHT), MK(DOWN), MK(LEFT),
	MK(RETURN), MK(KP3), MK(KP2), MK(RETURN)
};


static const ButtConfig powerpadsc[2][12]={
                              {
                                MK(o),MK(p),MK(LEFTBRACKET),MK(RIGHTBRACKET),
                                MK(k),MK(l),MK(SEMICOLON),MK(QUOTE),
                                MK(m),MK(COMMA),MK(PERIOD),MK(SLASH)
                              },
                              {
                                MK(o),MK(p),MK(LEFTBRACKET),MK(RIGHTBRACKET),
                                MK(k),MK(l),MK(SEMICOLON),MK(QUOTE),
                                MK(m),MK(COMMA),MK(PERIOD),MK(SLASH)
                              }
                             };

static const ButtConfig fkbmap[0x48]=
{
 MK(F1),MK(F2),MK(F3),MK(F4),MK(F5),MK(F6),MK(F7),MK(F8),
 MK(1),MK(2),MK(3),MK(4),MK(5),MK(6),MK(7),MK(8),MK(9),MK(0),MK(MINUS),MK(EQUALS),MK(BACKSLASH),MK(BACKSPACE),
 MK(ESCAPE),MK(q),MK(w),MK(e),MK(r),MK(t),MK(y),MK(u),MK(i),MK(o),MK(p),MK(BACKQUOTE),MK(LEFTBRACKET),MK(RETURN),
 MK(LCTRL),MK(a),MK(s),MK(d),MK(f),MK(g),MK(h),MK(j),MK(k),MK(l),MK(SEMICOLON),MK(QUOTE),MK(RIGHTBRACKET),MK(INSERT),
 MK(LSHIFT),MK(z),MK(x),MK(c),MK(v),MK(b),MK(n),MK(m),MK(COMMA),MK(PERIOD),MK(SLASH),MK(RALT),MK(RSHIFT),MK(LALT),MK(SPACE),
 MK(DELETE),MK(END),MK(PAGEDOWN),MK(UP),MK(LEFT),MK(RIGHT),MK(DOWN)
};

static const ButtConfig HyperShotButtons[4]=
{
 MK(q),MK(w),MK(e),MK(r)
};

static const ButtConfig MahjongButtons[21]=
{
 MK(q),MK(w),MK(e),MK(r),MK(t),
 MK(a),MK(s),MK(d),MK(f),MK(g),MK(h),MK(j),MK(k),MK(l),
 MK(z),MK(x),MK(c),MK(v),MK(b),MK(n),MK(m)
};

static const ButtConfig PartyTapButtons[6]=
{
 MK(q),MK(w),MK(e),MK(r),MK(t),MK(y)
};

static const ButtConfig FTrainerButtons[12]=
{
                               MK(o),MK(p),MK(LEFTBRACKET),
                               MK(RIGHTBRACKET),MK(k),MK(l),MK(SEMICOLON),
                                MK(QUOTE),
                               MK(m),MK(COMMA),MK(PERIOD),MK(SLASH)
};

static ButtConfig OekaKidsConfig[1] =
{
 { BUTTC_MOUSE, 0, 0, 0 },
};

static ButtConfig ArkanoidConfig[1] =
{
 { BUTTC_MOUSE, 0, 0, 0 },
};

static ButtConfig NESZapperConfig[2] =
{
 { BUTTC_MOUSE, 0, 0, 0 },
 { BUTTC_MOUSE, 0, 2, 0 },
};

static ButtConfig PCEMouseConfig[4] =
{
 { BUTTC_MOUSE, 0, 2, 0 },
 { BUTTC_MOUSE, 0, 0, 0 },
 MK(TAB),
 MK(RETURN)
};

static ButtConfig PCFXMouseConfig[2] =
{
 { BUTTC_MOUSE, 0, 0, 0 },
 { BUTTC_MOUSE, 0, 2, 0 },
};

static const ButtConfig SMSPadConfig[2][12]=
{
        /* Gamepad 1 */
        {
	 MK(w), MK(s), MK(a), MK(d), MK(KP2), MK(KP3), MK(RETURN)
        },

        /* Gamepad 2 */
        {MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ()}
};

static const ButtConfig GGPadConfig[12]=
{
         MK(w), MK(s), MK(a), MK(d), MK(KP2), MK(KP3), MK(RETURN)
};

static const ButtConfig TsushinKBConfig[0x80]=
{
 // 0
 MK(KP0),
 MK(KP1),
 MK(KP2),
 MK(KP3),
 MK(KP4),
 MK(KP5),
 MK(KP6),

// 1
 MK(KP8),
 MK(KP9),
 MK(KP_MULTIPLY),		// Keypad Multiply
 MK(KP_PLUS),			// Keypad Plus
 MK(KP_EQUALS),			// Keypad Equals
 MK(UNKNOWN), // KP_COMMA	// Keypad Comma
 MK(KP_PERIOD),			// Keypad Period

// 2
 MK(BACKQUOTE),		// @
 MK(a),
 MK(b),
 MK(c),
 MK(d),
 MK(e),
 MK(f),

// 3
 MK(h),
 MK(i),
 MK(j),
 MK(k),
 MK(l),
 MK(m),
 MK(n),

// 4
 MK(p),
 MK(q),
 MK(r),
 MK(s),
 MK(t),
 MK(u),
 MK(v),

// 5
 MK(x),
 MK(y),
 MK(z),
 MK(LEFTBRACKET),	// Left bracket
 MK(PLUS),		// Yen
 MK(RIGHTBRACKET),	// Right bracket
 MK(EQUALS),		// Caret

// 6
 MK(0),
 MK(1),
 MK(2),
 MK(3),
 MK(4),
 MK(5),
 MK(6),

// 7
 MK(8),
 MK(9),
 MK(QUOTE),		// Colon
 MK(SEMICOLON),		// Semicolon
 MK(COMMA),		// Comma
 MK(PERIOD),		// Period
 MK(SLASH),		// Slash

// 8
 MK(HOME),		// HOME CLEAR
 MK(UP),
 MK(RIGHT),
 //MK(UNKNOWN),
 MK(UNKNOWN),		// GRPH
 MK(LSUPER),		// カナ
 //MK(UNKNOWN),

// 9
 MK(BREAK),		// STOP
 MK(F1),
 MK(F2),
 MK(F3),
 MK(F4),
 MK(F5),
 MK(SPACE),

 // A
 MK(TAB),
 MK(DOWN),
 MK(LEFT),
 MK(END),		// HELP
 MK(PRINT),		// COPY
 MK(KP_MINUS),
 MK(KP_DIVIDE),

// B
 MK(PAGEDOWN),		// ROLL DOWN
 MK(PAGEUP),		// ROLL UP
 //MK(UNKNOWN),
 //MK(UNKNOWN),
 MK(o),
 MK(UNKNOWN),	// TODO: Underscore
 MK(g),

// C
 MK(F6),
 MK(F7),
 MK(F8),
 MK(F9),
 MK(F10),
 MK(BACKSPACE),
 MK(INSERT),

// D
 MK(RALT),		// 変換
 MK(LALT),		// 決定
 MK(RSUPER),		// PC
 MK(RCTRL),		// 変換
 MK(LCTRL),		// CTRL
 MK(KP7),
 MK(w),

// E
 MK(RETURN),
 MK(KP_ENTER),
 MK(LSHIFT),
 MK(RSHIFT),
 MK(CAPSLOCK),
 MK(DELETE),
 MK(ESCAPE),

// F
 //MK(UNKNOWN),
 //MK(UNKNOWN),
 //MK(UNKNOWN),
 //MK(UNKNOWN),
 //MK(UNKNOWN),
 MK(MINUS),		// Minus
 MK(7),			// 7
};


static const ButtConfig MMPlayInputConfig[] =
{
	MK(p),
	MK(LEFT),
	MK(RIGHT),
	MK(DOWN),
	MK(UP),
};

static const ButtConfig CDPlayInputConfig[] =
{
        MK(SPACE),
	MK(RETURN),
        MK(RIGHT),
	MK(LEFT),
        MK(UP),
	MK(DOWN),
	MK(PAGEUP),
	MK(PAGEDOWN),
};


static const ButtConfig MDPad3Config[2][8] =
{
 { MK(w), MK(s), MK(d), MK(a), MK(KP1), MK(KP2), MK(KP3), MK(RETURN) },
 { MKZ(), MKZ(), MKZ(), MKZ(), MKZ(),   MKZ(),   MKZ(),   MKZ() },

};

typedef struct
{
 const char *base_name;
 const ButtConfig *bc;
 int num;
} DefaultSettingsMeow;

static DefaultSettingsMeow defset[] =
{
 { "nes.input.port1.gamepad", NESGamePadConfig[0], sizeof(NESGamePadConfig[0]) / sizeof(ButtConfig) },
 { "nes.input.port2.gamepad", NESGamePadConfig[1], sizeof(NESGamePadConfig[1]) / sizeof(ButtConfig) },
 { "nes.input.port3.gamepad", NESGamePadConfig[2], sizeof(NESGamePadConfig[2]) / sizeof(ButtConfig) },
 { "nes.input.port4.gamepad", NESGamePadConfig[3], sizeof(NESGamePadConfig[3]) / sizeof(ButtConfig) },


 { "nes.input.port1.powerpada", powerpadsc[0], sizeof(powerpadsc[0]) / sizeof(ButtConfig) },
 { "nes.input.port2.powerpada", powerpadsc[1], sizeof(powerpadsc[1]) / sizeof(ButtConfig) },
 { "nes.input.port3.powerpadb", powerpadsc[0], sizeof(powerpadsc[0]) / sizeof(ButtConfig) },
 { "nes.input.port4.powerpadb", powerpadsc[1], sizeof(powerpadsc[1]) / sizeof(ButtConfig) },

 { "nes.input.port1.zapper", NESZapperConfig, sizeof(NESZapperConfig) / sizeof(ButtConfig) },
 { "nes.input.port2.zapper", NESZapperConfig, sizeof(NESZapperConfig) / sizeof(ButtConfig) },

 { "nes.input.fcexp.fkb", fkbmap, sizeof(fkbmap) / sizeof(ButtConfig) },
 { "nes.input.fcexp.mahjong", MahjongButtons, sizeof(MahjongButtons) / sizeof(ButtConfig) },
 { "nes.input.fcexp.ftrainera", FTrainerButtons, sizeof(FTrainerButtons) / sizeof(ButtConfig) },
 { "nes.input.fcexp.ftrainerb", FTrainerButtons, sizeof(FTrainerButtons) / sizeof(ButtConfig) },

 { "nes.input.fcexp.hypershot", HyperShotButtons, sizeof(HyperShotButtons) / sizeof(ButtConfig) },
 { "nes.input.fcexp.partytap", PartyTapButtons, sizeof(PartyTapButtons) / sizeof(ButtConfig) },

 { "nes.input.fcexp.oekakids", OekaKidsConfig, sizeof(OekaKidsConfig) / sizeof(ButtConfig) },

 { "nes.input.port1.arkanoid", ArkanoidConfig, sizeof(ArkanoidConfig) / sizeof(ArkanoidConfig) },
 { "nes.input.port2.arkanoid", ArkanoidConfig, sizeof(ArkanoidConfig) / sizeof(ArkanoidConfig) },
 { "nes.input.port3.arkanoid", ArkanoidConfig, sizeof(ArkanoidConfig) / sizeof(ArkanoidConfig) },
 { "nes.input.port4.arkanoid", ArkanoidConfig, sizeof(ArkanoidConfig) / sizeof(ArkanoidConfig) },
 { "nes.input.fcexp.arkanoid", ArkanoidConfig, sizeof(ArkanoidConfig) / sizeof(ArkanoidConfig) },
 { "lynx.input.builtin.gamepad", LynxPadConfig, sizeof(LynxPadConfig) / sizeof(ButtConfig) },
 { "gb.input.builtin.gamepad", GBPadConfig, sizeof(GBPadConfig) / sizeof(ButtConfig) },
 { "gba.input.builtin.gamepad", GBAPadConfig, sizeof(GBAPadConfig) / sizeof(ButtConfig) },
 { "ngp.input.builtin.gamepad", NGPPadConfig, sizeof(NGPPadConfig) / sizeof(ButtConfig) },
 { "wswan.input.builtin.gamepad", WSwanPadConfig, sizeof(WSwanPadConfig) / sizeof(ButtConfig) },

 { "pce.input.port1.gamepad", PCEPadConfig[0], sizeof(PCEPadConfig[0]) / sizeof(ButtConfig) },
 { "pce.input.port2.gamepad", PCEPadConfig[1], sizeof(PCEPadConfig[1]) / sizeof(ButtConfig)  },
 { "pce.input.port3.gamepad", PCEPadConfig[2], sizeof(PCEPadConfig[2]) / sizeof(ButtConfig)  },
 { "pce.input.port4.gamepad", PCEPadConfig[3], sizeof(PCEPadConfig[3]) / sizeof(ButtConfig)  },
 { "pce.input.port5.gamepad", PCEPadConfig[4], sizeof(PCEPadConfig[4]) / sizeof(ButtConfig)  },

 { "pce.input.port1.mouse", PCEMouseConfig, sizeof(PCEMouseConfig) / sizeof(ButtConfig) },
 { "pce.input.port2.mouse", PCEMouseConfig, sizeof(PCEMouseConfig) / sizeof(ButtConfig) },
 { "pce.input.port3.mouse", PCEMouseConfig, sizeof(PCEMouseConfig) / sizeof(ButtConfig) },
 { "pce.input.port4.mouse", PCEMouseConfig, sizeof(PCEMouseConfig) / sizeof(ButtConfig) },
 { "pce.input.port5.mouse", PCEMouseConfig, sizeof(PCEMouseConfig) / sizeof(ButtConfig) },

 { "pce.input.port1.tsushinkb", TsushinKBConfig, sizeof(TsushinKBConfig) / sizeof(ButtConfig) },

 { "pcfx.input.port1.gamepad", PCFXPadConfig[0], sizeof(PCFXPadConfig[0]) / sizeof(ButtConfig)  },
 { "pcfx.input.port2.gamepad", PCFXPadConfig[1], sizeof(PCFXPadConfig[1]) / sizeof(ButtConfig)  },
 { "pcfx.input.port3.gamepad", PCFXPadConfig[1], sizeof(PCFXPadConfig[1]) / sizeof(ButtConfig)  },
 { "pcfx.input.port4.gamepad", PCFXPadConfig[1], sizeof(PCFXPadConfig[1]) / sizeof(ButtConfig)  },
 { "pcfx.input.port5.gamepad", PCFXPadConfig[1], sizeof(PCFXPadConfig[1]) / sizeof(ButtConfig)  },
 { "pcfx.input.port6.gamepad", PCFXPadConfig[1], sizeof(PCFXPadConfig[1]) / sizeof(ButtConfig)  },
 { "pcfx.input.port7.gamepad", PCFXPadConfig[1], sizeof(PCFXPadConfig[1]) / sizeof(ButtConfig)  },
 { "pcfx.input.port8.gamepad", PCFXPadConfig[1], sizeof(PCFXPadConfig[1]) / sizeof(ButtConfig)  },


 { "pcfx.input.port1.mouse", PCFXMouseConfig, sizeof(PCFXMouseConfig) / sizeof(ButtConfig) },
 { "pcfx.input.port2.mouse", PCFXMouseConfig, sizeof(PCFXMouseConfig) / sizeof(ButtConfig) },

 { "sms.input.port1.gamepad", SMSPadConfig[0], sizeof(SMSPadConfig[0]) / sizeof(ButtConfig) },
 { "sms.input.port1.gamepad", SMSPadConfig[0], sizeof(SMSPadConfig[0]) / sizeof(ButtConfig) },
 { "gg.input.builtin.gamepad", GGPadConfig, sizeof(GGPadConfig) / sizeof(ButtConfig) },


 { "md.input.port1.gamepad", MDPad3Config[0], sizeof(MDPad3Config[0]) / sizeof(ButtConfig) },
 { "md.input.port2.gamepad", MDPad3Config[1], sizeof(MDPad3Config[1]) / sizeof(ButtConfig) }, 
 { "md.input.port3.gamepad", MDPad3Config[1], sizeof(MDPad3Config[1]) / sizeof(ButtConfig) },
 { "md.input.port4.gamepad", MDPad3Config[1], sizeof(MDPad3Config[1]) / sizeof(ButtConfig) },
 { "md.input.port5.gamepad", MDPad3Config[1], sizeof(MDPad3Config[1]) / sizeof(ButtConfig) },
 { "md.input.port6.gamepad", MDPad3Config[1], sizeof(MDPad3Config[1]) / sizeof(ButtConfig) },
 { "md.input.port7.gamepad", MDPad3Config[1], sizeof(MDPad3Config[1]) / sizeof(ButtConfig) },
 { "md.input.port8.gamepad", MDPad3Config[1], sizeof(MDPad3Config[1]) / sizeof(ButtConfig) },

 { "mmplay.input.builtin.controller", MMPlayInputConfig, sizeof(MMPlayInputConfig) / sizeof(ButtConfig) },
 { "cdplay.input.builtin.controller", CDPlayInputConfig, sizeof(CDPlayInputConfig) / sizeof(ButtConfig) },

};

