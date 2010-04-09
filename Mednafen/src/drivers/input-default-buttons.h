// This file should only be included *ONCE* from drivers/input.cpp!!!


#define GPZ()   {MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ()}
static const ButtConfig NESGamePadConfig[4][8]={
        /* Gamepad 1 */
        {
         MK(KP3), MK(KP2), MK(TAB), MK(RETURN), MK(w),MK(z),
                MK(a), MK(s)
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
         MK(KP3), MK(KP2), MK(TAB), MK(RETURN), MK(s),MK(a),
                MK(w), MK(z)
};

static const ButtConfig GBAPadConfig[10] =
{
         MK(KP3), MK(KP2), MK(TAB), MK(RETURN), MK(s),MK(a),
                MK(w), MK(z), MK(KP6), MK(KP5)
};

static const ButtConfig PCFXPadConfig[2][12]=
{
        /* Gamepad 1 */
        {
         MK(KP3), MK(KP2), MK(KP1), MK(KP4), MK(KP5), MK(KP6), MK(TAB), MK(RETURN),
         MK(w), MK(s), MK(z), MK(a)
        },

        /* Gamepad 2 */
        {MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ()}
};

#define GPZ()   {MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ()}
static const ButtConfig PCEPadConfig[5][13]={
        /* Gamepad 1 */
        {
         MK(KP3), MK(KP2), MK(TAB), MK(RETURN), MK(w), MK(s), MK(z), MK(a),

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
         MK(KP3), MK(KP2), MK(KP1), MK(KP7), MK(a),MK(s),
                MK(w), MK(z), MK(RETURN)
};

static const ButtConfig NGPPadConfig[7 + 2] =
{
        // Up, down, left, right, a(inner), b(outer), option
        MK(w), MK(z), MK(a), MK(s), MK(KP2), MK(KP3), MK(RETURN), MK(KP5), MK(KP6)
};

static const ButtConfig WSwanPadConfig[12] =
{
        // Up, right, down, left,
        // up-y, right-y, down-y, left-y,
	//  start, a(outer), b(inner), option
        MK(w), MK(s), MK(z), MK(a), 
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

static const ButtConfig QuizKingButtons[6]=
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

static ButtConfig PCEMouseConfig[2] =
{
 { BUTTC_MOUSE, 0, 0, 0 },
 { BUTTC_MOUSE, 0, 2, 0 },
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
	 MK(w), MK(z), MK(a), MK(s), MK(KP2), MK(KP3), MK(RETURN)
        },

        /* Gamepad 2 */
        {MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ(), MKZ()}
};

static const ButtConfig GGPadConfig[12]=
{
         MK(w), MK(z), MK(a), MK(s), MK(KP2), MK(KP3), MK(RETURN)
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
 { "nes.input.fcexp.quizking", QuizKingButtons, sizeof(QuizKingButtons) / sizeof(ButtConfig) },

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

 { "pcfx.input.port1.gamepad", PCFXPadConfig[0], sizeof(PCFXPadConfig[0]) / sizeof(ButtConfig)  },
 { "pcfx.input.port2.gamepad", PCFXPadConfig[1], sizeof(PCFXPadConfig[1]) / sizeof(ButtConfig)  },

 { "pcfx.input.port1.mouse", PCFXMouseConfig, sizeof(PCFXMouseConfig) / sizeof(ButtConfig) },
 { "pcfx.input.port2.mouse", PCFXMouseConfig, sizeof(PCFXMouseConfig) / sizeof(ButtConfig) },

 { "sms.input.port1.gamepad", SMSPadConfig[0], sizeof(SMSPadConfig[0]) / sizeof(ButtConfig) },
 { "sms.input.port1.gamepad", SMSPadConfig[0], sizeof(SMSPadConfig[0]) / sizeof(ButtConfig) },
 { "gg.input.builtin.gamepad", GGPadConfig, sizeof(GGPadConfig) / sizeof(ButtConfig) },
};

