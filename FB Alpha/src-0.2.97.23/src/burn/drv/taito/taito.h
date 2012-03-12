extern UINT8 TaitoInputPort0[8];
extern UINT8 TaitoInputPort1[8];
extern UINT8 TaitoInputPort2[8];
extern UINT8 TaitoInputPort3[8];
extern UINT8 TaitoInputPort4[8];
extern UINT8 TaitoInputPort5[8];
extern UINT8 TaitoDip[2];
extern UINT8 TaitoInput[6];
extern UINT8 TaitoReset;
extern UINT8 TaitoCoinLockout[4];

extern INT32 TaitoAnalogPort0;
extern INT32 TaitoAnalogPort1;
extern INT32 TaitoAnalogPort2;
extern INT32 TaitoAnalogPort3;

extern UINT8 *TaitoMem;
extern UINT8 *TaitoMemEnd;
extern UINT8 *TaitoRamStart;
extern UINT8 *TaitoRamEnd;
extern UINT8 *Taito68KRam1;
extern UINT8 *Taito68KRam2;
extern UINT8 *Taito68KRam3;
extern UINT8 *TaitoSharedRam;
extern UINT8 *TaitoZ80Ram1;
extern UINT8 *TaitoZ80Ram2;
extern UINT8 *TaitoPaletteRam;
extern UINT8 *TaitoSpriteRam;
extern UINT8 *TaitoSpriteRam2;
extern UINT8 *TaitoSpriteRamBuffered;
extern UINT8 *TaitoSpriteRamDelayed;
extern UINT8 *TaitoSpriteExtension;
extern UINT8 *TaitoVideoRam;
extern UINT32  *TaitoPalette;
extern UINT8 *TaitoPriorityMap;

extern UINT8 TaitoZ80Bank;
extern UINT8 TaitoSoundLatch;
extern UINT16 TaitoCpuACtrl;
extern INT32 TaitoRoadPalBank;

extern INT32 TaitoXOffset;
extern INT32 TaitoYOffset;
extern INT32 TaitoIrqLine;
extern INT32 TaitoFrameInterleave;
extern INT32 TaitoNumEEPROM;
extern INT32 TaitoFlipScreenX;

extern INT32 TaitoNum68Ks;
extern INT32 TaitoNumZ80s;
extern INT32 TaitoNumYM2610;
extern INT32 TaitoNumYM2151;
extern INT32 TaitoNumYM2203;
extern INT32 TaitoNumMSM5205;
extern INT32 TaitoNumMSM6295;
extern INT32 TaitoNumES5505;

extern INT32 nTaitoCyclesDone[4], nTaitoCyclesTotal[4];
extern INT32 nTaitoCyclesSegment;

typedef void (*TaitoRender)();
extern TaitoRender TaitoDrawFunction;
typedef void (*TaitoMakeInputs)();
extern TaitoMakeInputs TaitoMakeInputsFunction;
typedef INT32 (*TaitoResetFunc)();
extern TaitoResetFunc TaitoResetFunction;

extern INT32 TaitoDoReset();

#define TAITO_68KROM1			1
#define TAITO_68KROM1_BYTESWAP		2
#define TAITO_68KROM1_BYTESWAP_JUMPING	3
#define TAITO_68KROM1_BYTESWAP32	4
#define TAITO_68KROM2			5
#define TAITO_68KROM2_BYTESWAP		6
#define TAITO_68KROM3			7
#define TAITO_68KROM3_BYTESWAP		8
#define TAITO_Z80ROM1			9
#define TAITO_Z80ROM2			10
#define TAITO_CHARS			11
#define TAITO_CHARS_BYTESWAP		12
#define TAITO_CHARSB			13
#define TAITO_CHARSB_BYTESWAP		14
#define TAITO_SPRITESA			15
#define TAITO_SPRITESA_BYTESWAP		16
#define TAITO_SPRITESA_BYTESWAP32	17
#define TAITO_SPRITESA_TOPSPEED		18
#define TAITO_SPRITESB			19
#define TAITO_SPRITESB_BYTESWAP		20
#define TAITO_SPRITESB_BYTESWAP32	21
#define TAITO_ROAD			22
#define TAITO_SPRITEMAP			23
#define TAITO_YM2610A			24
#define TAITO_YM2610B			25
#define TAITO_MSM5205			26
#define TAITO_MSM5205_BYTESWAP		27
#define TAITO_CHARS_PIVOT		28
#define TAITO_MSM6295			29
#define TAITO_ES5505			30
#define TAITO_ES5505_BYTESWAP		31
#define TAITO_DEFAULT_EEPROM		32

extern UINT8 *Taito68KRom1;
extern UINT8 *Taito68KRom2;
extern UINT8 *Taito68KRom3;
extern UINT8 *TaitoZ80Rom1;
extern UINT8 *TaitoZ80Rom2;
extern UINT8 *TaitoChars;
extern UINT8 *TaitoCharsB;
extern UINT8 *TaitoCharsPivot;
extern UINT8 *TaitoSpritesA;
extern UINT8 *TaitoSpritesB;
extern UINT8 *TaitoSpriteMapRom;
extern UINT8 *TaitoYM2610ARom;
extern UINT8 *TaitoYM2610BRom;
extern UINT8 *TaitoMSM5205Rom;
extern UINT8 *TaitoMSM6295Rom;
extern UINT8 *TaitoES5505Rom;
extern UINT8 *TaitoDefaultEEProm;

extern UINT32 Taito68KRom1Num;
extern UINT32 Taito68KRom2Num;
extern UINT32 Taito68KRom3Num;
extern UINT32 TaitoZ80Rom1Num;
extern UINT32 TaitoZ80Rom2Num;
extern UINT32 TaitoCharRomNum;
extern UINT32 TaitoCharBRomNum;
extern UINT32 TaitoCharPivotRomNum;
extern UINT32 TaitoSpriteARomNum;
extern UINT32 TaitoSpriteBRomNum;
extern UINT32 TaitoRoadRomNum;
extern UINT32 TaitoSpriteMapRomNum;
extern UINT32 TaitoYM2610ARomNum;
extern UINT32 TaitoYM2610BRomNum;
extern UINT32 TaitoMSM5205RomNum;
extern UINT32 TaitoMSM6295RomNum;
extern UINT32 TaitoES5505RomNum;
extern UINT32 TaitoDefaultEEPromNum;

extern UINT32 Taito68KRom1Size;
extern UINT32 Taito68KRom2Size;
extern UINT32 Taito68KRom3Size;
extern UINT32 TaitoZ80Rom1Size;
extern UINT32 TaitoZ80Rom2Size;
extern UINT32 TaitoCharRomSize;
extern UINT32 TaitoCharRomBSize;
extern UINT32 TaitoCharPivotRomSize;
extern UINT32 TaitoSpriteARomSize;
extern UINT32 TaitoSpriteBRomSize;
extern UINT32 TaitoRoadRomSize;
extern UINT32 TaitoSpriteMapRomSize;
extern UINT32 TaitoYM2610ARomSize;
extern UINT32 TaitoYM2610BRomSize;
extern UINT32 TaitoMSM5205RomSize;
extern UINT32 TaitoMSM6295RomSize;
extern UINT32 TaitoES5505RomSize;
extern UINT32 TaitoDefaultEEPromSize;

extern UINT32 TaitoCharModulo;
extern UINT32 TaitoCharNumPlanes;
extern UINT32 TaitoCharWidth;
extern UINT32 TaitoCharHeight;
extern UINT32 TaitoNumChar;
extern INT32 *TaitoCharPlaneOffsets;
extern INT32 *TaitoCharXOffsets;
extern INT32 *TaitoCharYOffsets;

extern UINT32 TaitoCharBModulo;
extern UINT32 TaitoCharBNumPlanes;
extern UINT32 TaitoCharBWidth;
extern UINT32 TaitoCharBHeight;
extern UINT32 TaitoNumCharB;
extern INT32 *TaitoCharBPlaneOffsets;
extern INT32 *TaitoCharBXOffsets;
extern INT32 *TaitoCharBYOffsets;

extern UINT32 TaitoCharPivotModulo;
extern UINT32 TaitoCharPivotNumPlanes;
extern UINT32 TaitoCharPivotWidth;
extern UINT32 TaitoCharPivotHeight;
extern UINT32 TaitoNumCharPivot;
extern INT32 *TaitoCharPivotPlaneOffsets;
extern INT32 *TaitoCharPivotXOffsets;
extern INT32 *TaitoCharPivotYOffsets;

extern UINT32 TaitoSpriteAModulo;
extern UINT32 TaitoSpriteANumPlanes;
extern UINT32 TaitoSpriteAWidth;
extern UINT32 TaitoSpriteAHeight;
extern UINT32 TaitoNumSpriteA;
extern INT32 *TaitoSpriteAPlaneOffsets;
extern INT32 *TaitoSpriteAXOffsets;
extern INT32 *TaitoSpriteAYOffsets;
extern INT32 TaitoSpriteAInvertRom;

extern UINT32 TaitoSpriteBModulo;
extern UINT32 TaitoSpriteBNumPlanes;
extern UINT32 TaitoSpriteBWidth;
extern UINT32 TaitoSpriteBHeight;
extern UINT32 TaitoNumSpriteB;
extern INT32 *TaitoSpriteBPlaneOffsets;
extern INT32 *TaitoSpriteBXOffsets;
extern INT32 *TaitoSpriteBYOffsets;

extern INT32 TaitoLoadRoms(INT32 bLoad);
extern INT32 TaitoExit();

// Taito F2 Sprites
struct TaitoF2SpriteEntry {
	INT32 Code;
	INT32 x;
	INT32 y;
	INT32 Colour;
	INT32 xFlip;
	INT32 yFlip;
	INT32 xZoom;
	INT32 yZoom;
	INT32 Priority;
};
extern struct TaitoF2SpriteEntry *TaitoF2SpriteList;

extern INT32 TaitoF2SpriteType;
extern INT32 TaitoF2SpritesFlipScreen;
extern INT32 TaitoF2PrepareSprites;
extern INT32 TaitoF2SpritesDisabled, TaitoF2SpritesActiveArea, TaitoF2SpritesMasterScrollX, TaitoF2SpritesMasterScrollY;
extern INT32 TaitoF2SpriteBlendMode;
extern UINT16 TaitoF2SpriteBank[8];
extern UINT16 TaitoF2SpriteBankBuffered[8];
extern UINT8 TaitoF2TilePriority[5];
extern UINT8 TaitoF2SpritePriority[4];

typedef void (*TaitoF2SpriteBufferUpdate)();
void TaitoF2NoBuffer();
void TaitoF2PartialBufferDelayed();
void TaitoF2PartialBufferDelayedQzchikyu();
void TaitoF2PartialBufferDelayedThundfox();
void TaitoF2FullBufferDelayed();

void TaitoF2MakeSpriteList();
void TaitoF2RenderSpriteList(INT32 TaitoF2SpritePriorityLevel);
void TaitoF2HandleSpriteBuffering();
