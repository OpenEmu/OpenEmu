#include "tiles_generic.h"
#include "z80_intf.h"
#include "dac.h"
#include "8255ppi.h"
#include "bitswap.h"
#include "s2650_intf.h"
#include "sn76496.h"
#include "flt_rc.h"

#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

// ROM types
#define GAL_ROM_Z80_PROG1				1
#define GAL_ROM_Z80_PROG2				2
#define GAL_ROM_Z80_PROG3				3
#define GAL_ROM_TILES_SHARED				4
#define GAL_ROM_TILES_CHARS				5
#define GAL_ROM_TILES_SPRITES				6
#define GAL_ROM_PROM					7
#define GAL_ROM_S2650_PROG1				8

#define GAL_ROM_OFFSET_Z80_PROG2			GalZ80Rom1Num
#define GAL_ROM_OFFSET_Z80_PROG3			GalZ80Rom1Num + GalZ80Rom2Num
#define GAL_ROM_OFFSET_TILES_SHARED			GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num
#define GAL_ROM_OFFSET_TILES_CHARS			GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num + GalTilesSharedRomNum
#define GAL_ROM_OFFSET_TILES_SPRITES			GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num + GalTilesSharedRomNum + GalTilesCharRomNum
#define GAL_ROM_OFFSET_PROM				GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num + GalTilesSharedRomNum + GalTilesCharRomNum + GalTilesSpriteRomNum
#define GAL_ROM_OFFSET_S2650_PROG1			GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num + GalTilesSharedRomNum + GalTilesCharRomNum + GalTilesSpriteRomNum + GalPromRomNum

// IRQ types
#define GAL_IRQ_TYPE_NMI				1
#define GAL_IRQ_TYPE_IRQ0				2

// Sound hardware types
#define GAL_SOUND_HARDWARE_TYPE_GALAXIAN		1
#define GAL_SOUND_HARDWARE_TYPE_ZIGZAGAY8910		2
#define GAL_SOUND_HARDWARE_TYPE_JUMPBUGAY8910		3
#define GAL_SOUND_HARDWARE_TYPE_CHECKMANAY8910		4
#define GAL_SOUND_HARDWARE_TYPE_CHECKMAJAY8910		5
#define GAL_SOUND_HARDWARE_TYPE_MSHUTTLEAY8910		6
#define GAL_SOUND_HARDWARE_TYPE_KINGBALLDAC		7
#define GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910		8
#define GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910		9
#define GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910		10
#define GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910		11
#define GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC		12
#define GAL_SOUND_HARDWARE_TYPE_BONGOAY8910		13
#define GAL_SOUND_HARDWARE_TYPE_AD2083AY8910		14
#define GAL_SOUND_HARDWARE_TYPE_RACKNROLSN76496		15
#define GAL_SOUND_HARDWARE_TYPE_HEXPOOLASN76496		16
#define GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910		17
#define GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910		18

// Palette Numbers
#define GAL_PALETTE_NUM_COLOURS_PROM			64
#define GAL_PALETTE_NUM_COLOURS_STARS			64
#define GAL_PALETTE_NUM_COLOURS_BULLETS			8
#define GAL_PALETTE_NUM_COLOURS_BACKGROUND		256
#define GAL_PALETTE_STARS_OFFSET			GAL_PALETTE_NUM_COLOURS_PROM
#define GAL_PALETTE_BULLETS_OFFSET			GAL_PALETTE_NUM_COLOURS_PROM + GAL_PALETTE_NUM_COLOURS_STARS
#define GAL_PALETTE_BACKGROUND_OFFSET			GAL_PALETTE_NUM_COLOURS_PROM + GAL_PALETTE_NUM_COLOURS_STARS + GAL_PALETTE_NUM_COLOURS_BULLETS

// gal_gfx.cpp
extern UINT8 GalFlipScreenX;
extern UINT8 GalFlipScreenY;
extern UINT8 *GalGfxBank;
extern UINT8 GalPaletteBank;
extern UINT8 GalSpriteClipStart;
extern UINT8 GalSpriteClipEnd;
extern UINT8 FroggerAdjust;
extern UINT8 GalBackgroundRed;
extern UINT8 GalBackgroundGreen;
extern UINT8 GalBackgroundBlue;
extern UINT8 GalBackgroundEnable;
extern UINT8 SfxTilemap;
extern UINT8 GalOrientationFlipX;
extern UINT8 GalColourDepth;
extern UINT8 DarkplntBulletColour;
extern UINT8 DambustrBgColour1;
extern UINT8 DambustrBgColour2;
extern UINT8 DambustrBgPriority;
extern UINT8 DambustrBgSplitLine;
extern UINT8 *RockclimTiles;
extern UINT16 RockclimScrollX;
extern UINT16 RockclimScrollY;
extern INT32 CharPlaneOffsets[2];
extern INT32 CharXOffsets[8];
extern INT32 CharYOffsets[8];
extern INT32 SpritePlaneOffsets[2];
extern INT32 SpriteXOffsets[16];
extern INT32 SpriteYOffsets[16];

typedef void (*GalRenderBackground)();
extern GalRenderBackground GalRenderBackgroundFunction;
typedef void (*GalCalcPalette)();
extern GalCalcPalette GalCalcPaletteFunction;
typedef void (*GalDrawBullet)(INT32, INT32, INT32);
extern GalDrawBullet GalDrawBulletsFunction;
typedef void (*GalExtendTileInfo)(UINT16*, INT32*, INT32, INT32);
extern GalExtendTileInfo GalExtendTileInfoFunction;
typedef void (*GalExtendSpriteInfo)(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16*, UINT8*);
extern GalExtendSpriteInfo GalExtendSpriteInfoFunction;
typedef void (*GalRenderFrame)();
extern GalRenderFrame GalRenderFrameFunction;

void UpperExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void UpperExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void PiscesExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void PiscesExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void Batman2ExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void GmgalaxExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void GmgalaxExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void MooncrstExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void MooncrstExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void MoonqsrExtendTileInfo(UINT16 *Code, INT32*, INT32 Attr, INT32);
void MoonqsrExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void SkybaseExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void SkybaseExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void RockclimExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void JumpbugExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void JumpbugExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void FroggerExtendTileInfo(UINT16*, INT32 *Colour, INT32, INT32);
void FroggerExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16*, UINT8 *Colour);
void CalipsoExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8 *xFlip, UINT8 *yFlip, UINT16 *Code, UINT8*);
void MshuttleExtendTileInfo(UINT16 *Code, INT32*, INT32 Attr, INT32);
void MshuttleExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void Fourin1ExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void Fourin1ExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void DkongjrmExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8 *xFlip, UINT8*, UINT16 *Code, UINT8*);
void MarinerExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32 x);
void MimonkeyExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32);
void MimonkeyExtendSpriteInfo(const UINT8*, INT32*, INT32*, UINT8*, UINT8*, UINT16 *Code, UINT8*);
void DambustrExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32 x);
void Ad2083ExtendTileInfo(UINT16 *Code, INT32 *Colour, INT32 Attr, INT32);
void Ad2083ExtendSpriteInfo(const UINT8 *Base, INT32*, INT32*, UINT8 *xFlip, UINT8*, UINT16 *Code, UINT8*);
void RacknrolExtendTileInfo(UINT16 *Code, INT32*, INT32, INT32 x);
void HardCodeGalaxianPROM();
void HardCodeMooncrstPROM();
void GalaxianCalcPalette();
void RockclimCalcPalette();
void MarinerCalcPalette();
void StratgyxCalcPalette();
void RescueCalcPalette();
void MinefldCalcPalette();
void DarkplntCalcPalette();
void DambustrCalcPalette();
void GalaxianDrawBackground();
void RockclimDrawBackground();
void JumpbugDrawBackground();
void FroggerDrawBackground();
void TurtlesDrawBackground();
void ScrambleDrawBackground();
void AnteaterDrawBackground();
void MarinerDrawBackground();
void StratgyxDrawBackground();
void RescueDrawBackground();
void MinefldDrawBackground();
void DambustrDrawBackground();
void GalaxianDrawBullets(INT32 Offs, INT32 x, INT32 y);
void TheendDrawBullets(INT32 Offs, INT32 x, INT32 y);
void ScrambleDrawBullets(INT32, INT32 x, INT32 y);
void MoonwarDrawBullets(INT32, INT32 x, INT32 y);
void MshuttleDrawBullets(INT32, INT32 x, INT32 y);
void DarkplntDrawBullets(INT32, INT32 x, INT32 y);
void DambustrDrawBullets(INT32 Offs, INT32 x, INT32 y);
void GalDraw();
void DkongjrmRenderFrame();
void DambustrRenderFrame();
void FantastcRenderFrame();

// gal_run.cpp
extern UINT8 GalInputPort0[8];
extern UINT8 GalInputPort1[8];
extern UINT8 GalInputPort2[8];
extern UINT8 GalInputPort3[8];
extern UINT8 GalDip[7];
extern UINT8 GalInput[4];
extern UINT8 GalReset;
extern UINT8 GalFakeDip;
extern INT32           GalAnalogPort0;
extern INT32           GalAnalogPort1;

extern UINT8 *GalMem;
extern UINT8 *GalMemEnd;
extern UINT8 *GalRamStart;
extern UINT8 *GalRamEnd;
extern UINT8 *GalZ80Rom1;
extern UINT8 *GalZ80Rom1Op;
extern UINT8 *GalZ80Rom2;
extern UINT8 *GalZ80Rom3;
extern UINT8 *GalS2650Rom1;
extern UINT8 *GalZ80Ram1;
extern UINT8 *GalZ80Ram2;
extern UINT8 *GalZ80Ram3;
extern UINT8 *GalVideoRam;
extern UINT8 *GalVideoRam2;
extern UINT8 *GalSpriteRam;
extern UINT8 *GalScrollVals;
extern UINT8 *GalProm;
extern UINT8 *GalChars;
extern UINT8 *GalSprites;
extern UINT8 *GalTempRom;
extern UINT32 *GalPalette;
extern UINT32 GalZ80Rom1Size;
extern UINT32 GalZ80Rom1Num;
extern UINT32 GalZ80Rom2Size;
extern UINT32 GalZ80Rom2Num;
extern UINT32 GalZ80Rom3Size;
extern UINT32 GalZ80Rom3Num;
extern UINT32 GalS2650Rom1Size;
extern UINT32 GalS2650Rom1Num;
extern UINT32 GalTilesSharedRomSize;
extern UINT32 GalTilesSharedRomNum;
extern UINT32 GalTilesCharRomSize;
extern UINT32 GalTilesCharRomNum;
extern UINT32 GalNumChars;
extern UINT32 GalTilesSpriteRomSize;
extern UINT32 GalTilesSpriteRomNum;
extern UINT32 GalNumSprites;
extern UINT32 GalPromRomSize;
extern UINT32 GalPromRomNum;

typedef void (*GalPostLoadCallback)();
extern GalPostLoadCallback GalPostLoadCallbackFunction;

extern UINT8 GalIrqType;
extern UINT8 GalIrqFire;
extern INT32 nGalCyclesDone[3], nGalCyclesTotal[3];

extern UINT8 ZigzagAYLatch;
extern UINT8 GalSoundLatch;
extern UINT8 GalSoundLatch2;
extern UINT8 KingballSound;
extern UINT8 KonamiSoundControl;
extern UINT8 SfxSampleControl;
extern UINT8 KingballSpeechDip;
extern UINT16 ScrambleProtectionState;
extern UINT8 ScrambleProtectionResult;
extern UINT8 MoonwarPortSelect;
extern UINT8 MshuttleAY8910CS;
extern UINT8 GmgalaxSelectedGame;
extern UINT8 Fourin1Bank;
extern UINT8 GameIsGmgalax;
extern UINT8 CavelonBankSwitch;
extern UINT8 GalVBlank;

UINT8 KonamiPPIReadIN0();
UINT8 KonamiPPIReadIN1();
UINT8 KonamiPPIReadIN2();
UINT8 KonamiPPIReadIN3();
INT32 GalInit();
void MapMooncrst();
void MapJumpbug();
void MapFrogger();
void KonamiPPIInit();
void MapTheend();
void MapTurtles();
void MapScobra();
INT32 GalExit();
INT32 KonamiExit();
INT32 GalFrame();
INT32 GalScan(INT32 nAction, INT32 *pnMin);

// gal_sound.cpp
extern INT16* pFMBuffer;
extern INT16* pAY8910Buffer[9];
extern UINT8 GalSoundType;
extern UINT8 HunchbksSoundIrqFire;
extern UINT8 GalLastPort2;
extern UINT8 GalShootEnable;
extern UINT8 GalNoiseEnable;
extern INT32 GalNoiseVolume;
extern double GalShootWavePos;
extern double GalNoiseWavePos;
extern INT32 GalPitch;
extern INT32 GalVol;
extern INT32 GalLfoVolume[3];
extern double GalLfoFreq;
extern double GalLfoFreqFrameVar;
extern INT32 GalLfoBit[4];

void GalaxianSoundSetRoute(double nVolume, INT32 nRouteDir);

void GalSoundReset();
void GalSoundInit();
void GalSoundExit();
void GalSoundScan(INT32 nAction, INT32 *pnMin);
UINT8 CheckmajPortARead(UINT32);
UINT8 BongoDipSwitchRead(UINT32);
UINT8 KonamiSoundLatchRead(UINT32);
UINT8 KonamiSoundTimerRead(UINT32);
UINT8 FroggerSoundTimerRead(UINT32);
UINT8 HunchbksSoundTimerRead(UINT32);
void KonamiSoundLatchWrite(UINT8 d);
void KonamiSoundControlWrite(UINT8 d);
void HunchbksSoundControlWrite(UINT8 d);
void SfxSoundLatch2Write(UINT32, UINT32 d);
void SfxSampleControlWrite(UINT32, UINT32 d);
void FroggerSoundInit();
void FroggerSoundNoEncryptionInit();
void KonamiSoundInit();
void HunchbksSoundInit();
INT32 KingballSyncDAC();
INT32 SfxSyncDAC();
void GalRenderSoundSamples(INT16 *pSoundBuf, INT32 nLength);
void GalaxianSoundWrite(UINT32 Offset, UINT8 d);
void GalaxianLfoFreqWrite(UINT32 Offset, UINT8 d);
void GalaxianSoundUpdateTimers();

// gal_stars.cpp
extern INT32 GalStarsEnable;
extern INT32 GalStarsScrollPos;
extern INT32 GalStarsBlinkState;
extern INT32 GalBlinkTimerStartFrame;

void GalInitStars();
void GalaxianRenderStarLayer();
void JumpbugRenderStarLayer();
void ScrambleRenderStarLayer();
void MarinerRenderStarLayer();
void RescueRenderStarLayer();
