/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


// - Remove scummconsole.c
// - Delete files
// - Fatlib conversion?

// - libcartreset
// - Alternative controls - tap for left click, double for right
// - Inherit the Earth?
// - Stereo audio?
// - Delete saves?
// - Software scaler?
// - 100% scale

// - Arrow keys cause key events when keyboard enabled - Done
// - Mouse cursor display - Done
// - Disable scaler on options menu - Done
// - Fix scale icons on top screen - Done
// - Fseek optimisation? - No need
// - Fix agi hack to be cleaner - done
// - Fix not typing looong words - Done
// - Show keyboard by default in AGI games
// - Fix mouse moving when cursor on keyboard screen - Done
// - Fix 'fit' thingy always appearing - Done
// - check cine backbuffer code - Done
// - Add long filename support - Done
// - New icons
// - Add key config for gob engine: Start:F1, Shift-numbers: F keys - Done
// - Fix [ds] appearing in game menu

// - Find out what's going wrong when you turn the console off
// - enable console when asserting

// - Alternative controls?


// - Fix 512x256 backbuffer to 320x240 - Done
// - Fix keyboard appearing on wrong screen - Done
// - Volume amplify option
// - Make save/restore game screen use scaler buffer


// 1.0.0!
// - Fix text on tabs on config screen
// - Remove ini file debug msg
// - Memory size for ite
// - Try discworld?


#define FORBIDDEN_SYMBOL_ALLOW_ALL



#include <nds.h>
#include <nds/registers_alt.h>
#include <nds/arm9/exceptions.h>
#include <nds/arm9/console.h>

//#include <ARM9/console.h> //basic print funcionality

#include <stdlib.h>
#include <string.h>

#include "NDS/scummvm_ipc.h"
#include "dsmain.h"
#include "osystem_ds.h"
#include "icons_raw.h"
#include "fat/gba_nds_fat.h"
#include "fat/disc_io.h"
#include "keyboard_raw.h"
#include "keyboard_pal_raw.h"
#define V16(a, b) ((a << 12) | b)
#include "touchkeyboard.h"
//#include "compact_flash.h"
#include "dsoptions.h"
#ifdef USE_DEBUGGER
#include "user_debugger.h"
#endif
#include "blitters.h"
#include "keys.h"
#ifdef USE_PROFILER
#include "profiler/cyg-profile.h"
#endif
#include "engine.h"

#include "backends/plugins/ds/ds-provider.h"
#include "backends/fs/ds/ds-fs.h"
#include "base/version.h"
#include "common/util.h"

extern "C" void OurIntrMain(void);
extern "C" u32 getExceptionAddress(u32 opcodeAddress, u32 thumbState);

extern const char __itcm_start[];
static const char *registerNames[] =
	{	"r0","r1","r2","r3","r4","r5","r6","r7",
		"r8 ","r9 ","r10","r11","r12","sp ","lr ","pc" };

#ifdef WRAP_MALLOC

extern "C" void *__real_malloc(size_t size);

static int s_total_malloc = 0;

void *operator new (size_t size) {
	register unsigned int reg asm("lr");
	volatile unsigned int poo = reg;

	void *res = __real_malloc(size);
	s_total_malloc += size;

	if (!res) {
//		*((u8 *) NULL) = 0;
		consolePrintf("Failed alloc (new) %d (%d)\n", size, s_total_malloc);
		return NULL;
	}

	return res;
}


extern "C" void *__wrap_malloc(size_t size) {
/*	u32 addr;

	asm("mov %0, lr"
		: "=r" (addr)
		:
		: );*/

	register unsigned int reg asm("lr");
	volatile unsigned int poo = reg;


	if (size == 0) {
		static int zeroSize = 0;
		consolePrintf("0 size malloc (%d)", zeroSize++);
	}

	void *res = __real_malloc(size);
	if (res) {
		if (size > 50 * 1024) {
			consolePrintf("Allocated %d (%x)\n", size, poo);
		}
		s_total_malloc += size;
		return res;
	} else {

//		*((u8 *) NULL) = 0;
		consolePrintf("Failed alloc %d (%d)\n", size, s_total_malloc);
		return NULL;
	}
}


#endif

namespace DS {

// From console.c in NDSLib

//location of cursor
extern u8 row;
extern u8 col;

// Mouse mode
enum MouseMode {
	MOUSE_LEFT, MOUSE_RIGHT, MOUSE_HOVER, MOUSE_NUM_MODES
};

// Defines
#define FRAME_TIME 17
#define SCUMM_GAME_HEIGHT 142
#define SCUMM_GAME_WIDTH 227

static int frameCount;
static int currentTimeMillis;

// Timer Callback
static int callbackInterval;
static int callbackTimer;
static OSystem_DS::TimerProc callback;

// Scaled
static bool scaledMode;
static int scX;
static int scY;

static int subScX;
static int subScY;
static int subScTargetX;
static int subScTargetY;
static int subScreenWidth = SCUMM_GAME_WIDTH;
static int subScreenHeight = SCUMM_GAME_HEIGHT;
static int subScreenScale = 256;



// Sound
static int bufferSize;
static s16 *soundBuffer;
static int bufferFrame;
static int bufferRate;
static int bufferSamples;
static bool soundHiPart;
static int soundFrequency;

// Events
static int lastEventFrame;
static bool indyFightState;
static bool indyFightRight;

static OSystem_DS::SoundProc soundCallback;
static int lastCallbackFrame;
static bool bufferFirstHalf;
static bool bufferSecondHalf;

// Saved buffers
static bool highBuffer;
static bool displayModeIs8Bit = false;

// Game id
static u8 gameID;

static bool snapToBorder = false;
static bool consoleEnable = false;
static bool gameScreenSwap = false;
bool isCpuScalerEnabled();
//#define HEAVY_LOGGING

static MouseMode mouseMode = MOUSE_LEFT;

static int storedMouseX = 0;
static int storedMouseY = 0;

// Sprites
static SpriteEntry sprites[128];
static SpriteEntry spritesMain[128];
static int tweak;

// Shake
static int s_shakePos = 0;

// Keyboard
static bool keyboardEnable = false;
static bool leftHandedMode = false;
static bool keyboardIcon = false;

// Touch
static int touchScX, touchScY, touchX, touchY;
static int mouseHotspotX, mouseHotspotY;
static bool cursorEnable = false;
static bool mouseCursorVisible = true;
static bool leftButtonDown = false;
static bool rightButtonDown = false;
static bool touchPadStyle = false;
static int touchPadSensitivity = 8;
static bool tapScreenClicks = true;

static int tapCount = 0;
static int tapTimeout = 0;
static int tapComplete = 0;

// Dragging
static int dragStartX, dragStartY;
static bool dragging = false;
static int dragScX, dragScY;

// Interface styles
static char gameName[32];

// 8-bit surface size
static int gameWidth = 320;
static int gameHeight = 200;

// Scale
static bool twoHundredPercentFixedScale = false;
static bool cpuScalerEnable = false;

		// 100    256
		// 150	  192
		// 200	  128

		// (256 << 8) / scale



#ifdef USE_PROFILER
static int hBlankCount = 0;
#endif

static u8 *scalerBackBuffer = NULL;

#define NUM_SUPPORTED_GAMES 21

static const gameListType gameList[NUM_SUPPORTED_GAMES] = {
	// Unknown game - use normal SCUMM controls
	{"unknown", 	CONT_SCUMM_ORIGINAL},

	// SCUMM games
	{"maniac",		CONT_SCUMM_ORIGINAL},
	{"zak",			CONT_SCUMM_ORIGINAL},
	{"loom",		CONT_SCUMM_ORIGINAL},
	{"indy3",		CONT_SCUMM_ORIGINAL},
	{"atlantis",		CONT_SCUMM_ORIGINAL},
	{"monkey",		CONT_SCUMM_ORIGINAL},
	{"monkey2",		CONT_SCUMM_ORIGINAL},
	{"tentacle",		CONT_SCUMM_ORIGINAL},
	{"samnmax",		CONT_SCUMM_SAMNMAX},

	// Non-SCUMM games
	{"sky",			CONT_SKY},
	{"simon1",		CONT_SIMON},
	{"simon2",		CONT_SIMON},
	{"gob",			CONT_GOBLINS},
	{"queen",		CONT_SCUMM_ORIGINAL},
	{"cine",		CONT_FUTURE_WARS},
	{"agi",			CONT_AGI},
	{"elvira2",		CONT_SIMON},
	{"elvira1",		CONT_SIMON},
	{"waxworks",		CONT_SIMON},
	{"parallaction",	CONT_NIPPON},
};

static const gameListType *s_currentGame = NULL;

// Stylus
static bool penDown = FALSE;
static bool penHeld = FALSE;
static bool penReleased = FALSE;
static bool penDownLastFrame = FALSE;
static s32 penX = 0, penY = 0;
static s32 penDownX = 0, penDownY = 0;
static int keysDownSaved = 0;
static int keysReleasedSaved = 0;
static int keysChangedSaved = 0;

static bool penDownSaved = FALSE;
static bool penReleasedSaved = FALSE;
static int penDownFrames = 0;
static int touchXOffset = 0;
static int touchYOffset = 0;

static int triggeredIcon = 0;
static int triggeredIconTimeout = 0;

static u16 savedPalEntry255 = RGB15(31, 31, 31);


extern "C" int scummvm_main(int argc, char *argv[]);
Common::EventType getKeyEvent(int key);
int getKeysChanged();

void updateStatus();
void triggerIcon(int imageNum);
void setIcon(int num, int x, int y, int imageNum, int flags, bool enable);
void setIconMain(int num, int x, int y, int imageNum, int flags, bool enable);
void uploadSpriteGfx();

static TransferSound soundControl;

static bool isScrollingWithDPad() {
	return (getKeysHeld() & (KEY_L | KEY_R)) != 0;
}

bool isCpuScalerEnabled() {
	return cpuScalerEnable || !displayModeIs8Bit;
}


void setCpuScalerEnable(bool enable) {
	cpuScalerEnable = enable;
}

void setTrackPadStyleEnable(bool enable) {
	touchPadStyle = enable;
}

void setTapScreenClicksEnable(bool enable) {
	tapScreenClicks = enable;
}

void setGameScreenSwap(bool enable) {
	gameScreenSwap = enable;
}

void setSensitivity(int sensitivity) {
	touchPadSensitivity = sensitivity;
}

void setGamma(int gamma) {
	OSystem_DS::instance()->setGammaValue(gamma);
}

void setTopScreenZoom(int percentage) {
		// 100    256
		// 150	  192
		// 200	  128

		// (256 << 8) / scale

	s32 scale = (percentage << 8) / 100;
	subScreenScale = (256 * 256) / scale;

//	consolePrintf("Scale is %d %%\n", percentage);
}

//	return (ConfMan.hasKey("cpu_scaler", "ds") && ConfMan.getBool("cpu_scaler", "ds"));

controlType getControlType() {
	return s_currentGame->control;
}


//plays an 8 bit mono sample at 11025Hz
void playSound(const void *data, u32 length, bool loop, bool adpcm, int rate) {

	if (!IPC->soundData) {
		soundControl.count = 0;
	}

	soundControl.data[soundControl.count].data = data;
	soundControl.data[soundControl.count].len = length | (loop? 0x80000000: 0x00000000);
	soundControl.data[soundControl.count].rate = rate;		// 367 samples per frame
	soundControl.data[soundControl.count].pan = 64;
	soundControl.data[soundControl.count].vol = 127;
	soundControl.data[soundControl.count].format = adpcm? 2: 0;

	soundControl.count++;

	DC_FlushAll();
	IPC->soundData = &soundControl;
}

void stopSound(int channel) {
	playSound(NULL, 0, false, false, -channel);
}

void updateOAM() {
	DC_FlushAll();

	if (gameScreenSwap) {
		dmaCopy(sprites, OAM, 128 * sizeof(SpriteEntry));
		dmaCopy(spritesMain, OAM_SUB, 128 * sizeof(SpriteEntry));
	} else {
		dmaCopy(sprites, OAM_SUB, 128 * sizeof(SpriteEntry));
		dmaCopy(spritesMain, OAM, 128 * sizeof(SpriteEntry));
	}
}

void setGameSize(int width, int height) {
	gameWidth = width;
	gameHeight = height;
}

int getGameWidth() {
	return gameWidth;
}

int getGameHeight() {
	return gameHeight;
}

void initSprites() {
	for (int i = 0; i < 128; i++) {
		sprites[i].attribute[0] = ATTR0_DISABLED;
		sprites[i].attribute[1] = 0;
		sprites[i].attribute[2] = 0;
		sprites[i].filler = 0;
	}

	for (int i = 0; i < 128; i++) {
		spritesMain[i].attribute[0] = ATTR0_DISABLED;
		spritesMain[i].attribute[1] = 0;
		spritesMain[i].attribute[2] = 0;
		spritesMain[i].filler = 0;
	}

	updateOAM();
}


void saveGameBackBuffer() {

	// Sometimes the only copy of the game screen is in video memory.
	// So, I lock the video memory here, as if I'm going to modify it.  This
	// forces OSystem_DS to create a system memory copy if one doesn't exist.
	// This will be automatially restored by OSystem_DS::updateScreen().

	OSystem_DS::instance()->lockScreen();
	OSystem_DS::instance()->unlockScreen();
}



void startSound(int freq, int buffer) {
	bufferRate = freq * 2;
	bufferFrame = 0;
	bufferSamples = 4096;

	bufferFirstHalf = false;
	bufferSecondHalf = true;

	int bytes = (2 * (bufferSamples)) + 100;

	soundBuffer = (s16 *) malloc(bytes * 2);
	if (!soundBuffer)
		consolePrintf("Sound buffer alloc failed\n");


	soundHiPart = true;

	for (int r = 0; r < bytes; r++) {
		soundBuffer[r] = 0;
	}

	soundFrequency = freq;


	swiWaitForVBlank();
	swiWaitForVBlank();
	playSound(soundBuffer, (bufferSamples * 2), true, false, freq * 2);
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
}

int getSoundFrequency() {
	return soundFrequency;
}

void exitGame() {
	s_currentGame = NULL;
}

void initGame() {
	// This is a good time to check for left handed mode since the mode change is done as the game starts.
	// There's probably a better way, but hey.
	#ifdef HEAVY_LOGGING
	consolePrintf("initing game...");
	#endif

//	static bool firstTime = true;

	setOptions();

	//strcpy(gameName, ConfMan.getActiveDomain().c_str());
	if (s_currentGame == NULL) {

		strcpy(gameName, ConfMan.get("gameid").c_str());
	//	consolePrintf("\n\n\n\nCurrent game: '%s' %d\n", gameName, gameName[0]);

		s_currentGame = &gameList[0];		// Default game

		for (int r = 0; r < NUM_SUPPORTED_GAMES; r++) {
			if (!stricmp(gameName, gameList[r].gameId)) {
				s_currentGame = &gameList[r];
	//			consolePrintf("Game list num: %d\n", r);
			}
		}
	}

/*	if (firstTime) {
		firstTime = false;


	}
*/
	#ifdef HEAVY_LOGGING
	consolePrintf("done\n");
	#endif

}

void setLeftHanded(bool enable) {
	leftHandedMode = enable;
}

void setSnapToBorder(bool enable) {
	snapToBorder = enable;
}

void setTouchXOffset(int x) {
	touchXOffset = x;
}

void setTouchYOffset(int y) {
	touchYOffset = y;
}

void set200PercentFixedScale(bool on) {
	twoHundredPercentFixedScale = on;
}

void setUnscaledMode(bool enable) {
	scaledMode = !enable;
}

void displayMode8Bit() {

#ifdef HEAVY_LOGGING
	consolePrintf("displayMode8Bit...");
#endif
	u16 buffer[32 * 32];

	initGame();

	setKeyboardEnable(false);

	if (!displayModeIs8Bit) {
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r];
		}
	} else {
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK(2))[r];
		}
	}

	displayModeIs8Bit = true;

	if (isCpuScalerEnabled()) {
		videoSetMode(MODE_5_2D | (consoleEnable? DISPLAY_BG0_ACTIVE: 0) | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
		videoSetModeSub(MODE_3_2D /*| DISPLAY_BG0_ACTIVE*/ | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

		vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
		vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

		vramSetBankC(VRAM_C_SUB_BG_0x06200000);
		vramSetBankD(VRAM_D_SUB_SPRITE);

		vramSetBankH(VRAM_H_LCD);

		BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(8);

		BG3_XDX = 256;
		BG3_XDY = 0;
		BG3_YDX = 0;
		BG3_YDY = (int) ((200.0f / 192.0f) * 256);

	} else {
		videoSetMode(MODE_5_2D | (consoleEnable? DISPLAY_BG0_ACTIVE: 0) | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
		videoSetModeSub(MODE_3_2D /*| DISPLAY_BG0_ACTIVE*/ | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

		vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
		vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

		vramSetBankC(VRAM_C_SUB_BG_0x06200000);
		vramSetBankD(VRAM_D_SUB_SPRITE);

		vramSetBankH(VRAM_H_LCD);

		BG3_CR = BG_BMP8_512x256 | BG_BMP_BASE(8);

		BG3_XDX = (int) (((float) (gameWidth) / 256.0f) * 256);
		BG3_XDY = 0;
		BG3_YDX = 0;
		BG3_YDY = (int) ((200.0f / 192.0f) * 256);
	}

	SUB_BG3_CR = BG_BMP8_512x256;

	SUB_BG3_XDX = (int) (subScreenWidth / 256.0f * 256);
	SUB_BG3_XDY = 0;
	SUB_BG3_YDX = 0;
	SUB_BG3_YDY = (int) (subScreenHeight / 192.0f * 256);



	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true);

	// Set this again because consoleinit resets it
	videoSetMode(MODE_5_2D | (consoleEnable? DISPLAY_BG0_ACTIVE: 0) | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);

	// Move the cursor to the bottom of the screen using ANSI escape code
	consolePrintf("\033[23;0f");


	for (int r = 0; r < 32 * 32; r++) {
		((u16 *) SCREEN_BASE_BLOCK(2))[r] = buffer[r];
//		dmaCopyHalfWords(3, (u16 *) SCREEN_BASE_BLOCK(0), buffer, 32 * 32 * 2);
	}

	// ConsoleInit destroys the hardware palette :-(
	if (OSystem_DS::instance()) {
		OSystem_DS::instance()->restoreHardwarePalette();
	}

//	BG_PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255

	// Do text stuff
	// console chars at 1C000 (7), map at 1D000 (74)

//	BG0_CR = BG_MAP_BASE(2) | BG_TILE_BASE(0);
//	BG0_Y0 = 0;

	// Restore palette entry used by text in the front-end
//	PALETTE_SUB[255] = savedPalEntry255;


	#ifdef HEAVY_LOGGING
	consolePrintf("done\n");
	#endif

	if (gameScreenSwap) {
		POWER_CR |= POWER_SWAP_LCDS;
	} else {
		POWER_CR &= ~POWER_SWAP_LCDS;
	}

	uploadSpriteGfx();

	keyboardEnable = false;

}

void setGameID(int id) {
	gameID = id;
}

void dummyHandler() {
	REG_IF = IRQ_VBLANK;
}

void checkSleepMode() {
	if (IPC->performArm9SleepMode) {

		consolePrintf("ARM9 Entering sleep mode\n");

		int intSave = REG_IE;
		irqSet(IRQ_VBLANK, dummyHandler);
//		int irqHandlerSave = (int) IRQ_HANDLER;
		REG_IE = IRQ_VBLANK;
		//IRQ_HANDLER = dummyHandler;

		int powerSave = POWER_CR;
		POWER_CR &= ~POWER_ALL;

		while (IPC->performArm9SleepMode) {
			swiWaitForVBlank();
		}

		POWER_CR = powerSave;
//		IRQ_HANDLER = (void (*)()) irqHandlerSave;
		irqSet(IRQ_VBLANK, VBlankHandler);
		REG_IE = intSave;

		consolePrintf("ARM9 Waking from sleep mode\n");
	}
}

void setShowCursor(bool enable) {
	if ((s_currentGame) && (s_currentGame->control == CONT_SCUMM_SAMNMAX)) {
		if (cursorEnable) {
			sprites[1].attribute[0] = ATTR0_BMP | 150;
		} else {
			sprites[1].attribute[0] = ATTR0_DISABLED;
		}

	}

	cursorEnable = enable;
}

void setMouseCursorVisible(bool enable) {
	mouseCursorVisible = enable;
}

void setCursorIcon(const u8 *icon, uint w, uint h, byte keycolor, int hotspotX, int hotspotY) {

	int off;

	mouseHotspotX = hotspotX;
	mouseHotspotY = hotspotY;

	//consolePrintf("Set cursor icon %d, %d\n", w, h);

	off = 128*64;


	memset(SPRITE_GFX + off, 0, 32 * 32 * 2);
	memset(SPRITE_GFX_SUB + off, 0, 32 * 32 * 2);


	for (uint y=0; y<h; y++) {
		for (uint x=0; x<w; x++) {
			int color = icon[y*w+x];

			//consolePrintf("%d:%d ", color, OSystem_DS::instance()->getDSPaletteEntry(color));

			if (color == keycolor) {
				SPRITE_GFX[off+(y)*32+x] = 0x0000; // black background
				SPRITE_GFX_SUB[off+(y)*32+x] = 0x0000; // black background
			} else {
				SPRITE_GFX[off+(y)*32+x] = OSystem_DS::instance()->getDSCursorPaletteEntry(color) | 0x8000;
				SPRITE_GFX_SUB[off+(y)*32+x] = OSystem_DS::instance()->getDSCursorPaletteEntry(color) | 0x8000;
			}
		}
	}

	if (s_currentGame->control != CONT_SCUMM_SAMNMAX)
		return;

	uint16 border = RGB15(24,24,24) | 0x8000;


	off = 176*64;
	memset(SPRITE_GFX_SUB+off, 0, 64*64*2);
	memset(SPRITE_GFX+off, 0, 64*64*2);

	int pos = 190 - (w+2);



	// make border
	for (uint i=0; i<w+2; i++) {
		SPRITE_GFX[off+i] = border;
		SPRITE_GFX[off+(31)*64+i] = border;

		SPRITE_GFX_SUB[off+i] = border;
		SPRITE_GFX_SUB[off+(31)*64+i] = border;
	}
	for (uint i=1; i<31; i++) {
		SPRITE_GFX[off+(i*64)] = border;
		SPRITE_GFX[off+(i*64)+(w+1)] = border;

		SPRITE_GFX_SUB[off+(i*64)] = border;
		SPRITE_GFX_SUB[off+(i*64)+(w+1)] = border;
	}

	int offset = (32 - h) >> 1;

	for (uint y=0; y<h; y++) {
		for (uint x=0; x<w; x++) {
			int color = icon[y*w+x];

			if (color == keycolor) {
				SPRITE_GFX[off+(y+1+offset)*64+(x+1)] = 0x8000; // black background
				SPRITE_GFX_SUB[off+(y+1+offset)*64+(x+1)] = 0x8000; // black background
			} else {
				SPRITE_GFX[off+(y+1+offset)*64+(x+1)] = BG_PALETTE[color] | 0x8000;
				SPRITE_GFX_SUB[off+(y+1+offset)*64+(x+1)] = BG_PALETTE[color] | 0x8000;
			}
		}
	}


	if (cursorEnable) {
		sprites[1].attribute[0] = ATTR0_BMP | 150;
		sprites[1].attribute[1] = ATTR1_SIZE_64 | pos;
		sprites[1].attribute[2] = ATTR2_ALPHA(1) | 176;
	} else {
		sprites[1].attribute[0] = ATTR0_DISABLED | 150;
		sprites[1].attribute[1] = ATTR1_SIZE_64 | pos;
		sprites[1].attribute[2] = ATTR2_ALPHA(1) | 176;
	}

}




void displayMode16Bit() {
	#ifdef HEAVY_LOGGING
	consolePrintf("displayMode16Bit...");
	#endif

	u16 buffer[32 * 32 * 2];

	releaseAllKeys();

	setKeyboardEnable(false);

	if (!displayModeIs8Bit) {
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r];
		}
	} else {
		saveGameBackBuffer();
		for (int r = 0; r < 32 * 32; r++) {
			buffer[r] = ((u16 *) SCREEN_BASE_BLOCK(2))[r];
		}
	}


	videoSetMode(MODE_5_2D | /*DISPLAY_BG0_ACTIVE |*/ DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE |/* DISPLAY_BG1_ACTIVE |*/ DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	vramSetBankC(VRAM_C_MAIN_BG);
	vramSetBankD(VRAM_D_MAIN_BG);
	vramSetBankH(VRAM_H_SUB_BG);

	BG3_CR = BG_BMP16_512x256;
	highBuffer = false;


	memset(BG_GFX, 0, 512 * 256 * 2);

	savedPalEntry255 = BG_PALETTE_SUB[255];
	BG_PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255

	// Do text stuff
	SUB_BG0_CR = BG_MAP_BASE(4) | BG_TILE_BASE(0);
	SUB_BG0_Y0 = 0;

	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, false, true);
//	consoleInitDefault((u16 *)SCREEN_BASE_BLOCK_SUB(4), (u16 *)CHAR_BASE_BLOCK_SUB(0), 16);

	for (int r = 0; r < 32 * 32; r++) {
		((u16 *) SCREEN_BASE_BLOCK_SUB(4))[r] = buffer[r];
	}

	consoleSetWindow(NULL, 0, 0, 32, 24);
//	consolePrintSet(0, 23);
//	consolePrintf("Hello world!\n\n");
//	consolePrintf("\n");

	// Show keyboard
	SUB_BG1_CR = BG_TILE_BASE(1) | BG_MAP_BASE(12);
	//drawKeyboard(1, 12);

	POWER_CR &= ~POWER_SWAP_LCDS;

	displayModeIs8Bit = false;

	// ConsoleInit destroys the hardware palette :-(
	OSystem_DS::instance()->restoreHardwarePalette();

	BG3_XDX = isCpuScalerEnabled() ? 256 : (int) (1.25f * 256);
	BG3_XDY = 0;
	BG3_YDX = 0;
	BG3_YDY = (int) ((200.0f / 192.0f) * 256);

	#ifdef HEAVY_LOGGING
	consolePrintf("done\n");
	#endif

	BG_PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255

}


void displayMode16BitFlipBuffer() {
	#ifdef HEAVY_LOGGING
	consolePrintf("Flip %s...", displayModeIs8Bit ? "8bpp" : "16bpp");
	#endif
	if (!displayModeIs8Bit) {
		u16 *back = get16BitBackBuffer();

//		highBuffer = !highBuffer;
//		BG3_CR = BG_BMP16_512x256 |	BG_BMP_RAM(highBuffer? 1: 0);

		if (isCpuScalerEnabled()) {
			Rescale_320x256x1555_To_256x256x1555(BG_GFX, back, 512, 512);
		} else {
			for (int r = 0; r < 512 * 256; r++) {
				*(BG_GFX + r) = *(back + r);
			}
		}
	} else if (isCpuScalerEnabled()) {
		//#define SCALER_PROFILE

		#ifdef SCALER_PROFILE
		TIMER1_CR = TIMER_ENABLE | TIMER_DIV_1024;
		u16 t0 = TIMER1_DATA;
		#endif
		const u8 *back = (const u8*)get8BitBackBuffer();
		u16 *base = BG_GFX + 0x10000;
		Rescale_320x256xPAL8_To_256x256x1555(
			base,
			back,
			256,
			get8BitBackBufferStride(),
			BG_PALETTE,
			getGameHeight() );

		#ifdef SCALER_PROFILE
		// 10 pixels : 1ms
		u16 t1 = TIMER1_DATA;
		TIMER1_CR &= ~TIMER_ENABLE;
		u32 dt = t1 - t0;
		u32 dt_us = (dt * 10240) / 334;
		u32 dt_10ms = dt_us / 100;
		int i;
		for(i=0; i<dt_10ms; ++i)
			base[i] = ((i/10)&1) ? 0xFFFF : 0x801F;
		for(; i<256; ++i)
			base[i] = 0x8000;
		#endif
	}
	#ifdef HEAVY_LOGGING
	consolePrintf("done\n");
	#endif
}

void setShakePos(int shakePos) {
	s_shakePos = shakePos;
}


u16 *get16BitBackBuffer() {
	return BG_GFX + 0x20000;
}

s32 get8BitBackBufferStride() {
	// When the CPU scaler is enabled, the back buffer is in system RAM and is
	// 320 pixels wide. When the CPU scaler is disabled, the back buffer is in
	// video memory and therefore must have a 512 pixel stride.

	if (isCpuScalerEnabled()){
		return 320;
	} else {
		return 512;
	}
}

u16 *getScalerBuffer() {
	return (u16 *) scalerBackBuffer;
}

u16 *get8BitBackBuffer() {
	if (isCpuScalerEnabled())
		return (u16 *) scalerBackBuffer;
	else
		return BG_GFX + 0x10000;		// 16bit qty!
}

// The sound system in ScummVM seems to always return stereo interleaved samples.
// Here, I'm treating an 11Khz stereo stream as a 22Khz mono stream, which works sorta ok, but is
// a horrible bodge.  Any advice on how to change the engine to output mono would be greatly
// appreciated.
void doSoundCallback() {
	#ifdef HEAVY_LOGGING
	consolePrintf("doSoundCallback...");
	#endif

	if (OSystem_DS::instance())
	if (OSystem_DS::instance()->getMixerImpl()) {
		lastCallbackFrame = frameCount;

		for (int r = IPC->playingSection; r < IPC->playingSection + 4; r++) {
			int chunk = r & 3;

			if (IPC->fillNeeded[chunk]) {
				IPC->fillNeeded[chunk] = false;
				DC_FlushAll();
				OSystem_DS::instance()->getMixerImpl()->mixCallback((byte *) (soundBuffer + ((bufferSamples >> 2) * chunk)), bufferSamples >> 1);
				IPC->fillNeeded[chunk] = false;
				DC_FlushAll();
			}

		}

	}
	#ifdef HEAVY_LOGGING
	consolePrintf("done\n");
	#endif
}

void doTimerCallback() {
	if (callback) {
		if (callbackTimer <= 0) {
			callbackTimer += callbackInterval;
			callback(callbackInterval);
		}
	}
}

void soundUpdate() {
	if ((bufferFrame == 0)) {
//		playSound(soundBuffer, (bufferSamples * 2), true);
	}
//	consolePrintf("%x\n", IPC->test);


	if (bufferFrame == 0) {
//		bufferFirstHalf = true;
	}
	if (bufferFrame == bufferSize >> 1) {
	//bufferSecondHalf = true;
	}

	bufferFrame++;
	if (bufferFrame == bufferSize) {
		bufferFrame = 0;
	}
}

void memoryReport() {
	int r = 0;
	int *p;
	do {
		p = (int *) malloc(r * 8192);
		free(p);
		r++;
	} while ((p) && (r < 512));

	int t = -1;
	void *block[1024];
	do {
		t++;
		block[t] = (int *) malloc(4096);
	} while ((t < 1024) && (block[t]));

	for (int q = 0; q < t; q++) {
		free(block[q]);
	}

	consolePrintf("Free: %dK, Largest: %dK\n", t * 4, r * 8);
}


void addIndyFightingKeys() {
	OSystem_DS *system = OSystem_DS::instance();
	Common::Event event;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.flags = 0;

//	consolePrintf("Fight keys\n");

	if ((getKeysDown() & KEY_L)) {
		indyFightRight = false;
	}

	if ((getKeysDown() & KEY_R)) {
		indyFightRight = true;
	}

//	consolePrintf("ifr:%d\n", indyFightRight);

	if ((getKeysChanged() & KEY_UP)) {
		event.type = getKeyEvent(KEY_UP);
		event.kbd.keycode = Common::KEYCODE_8;
		event.kbd.ascii = '8';
		system->addEvent(event);
	}
	if ((getKeysChanged() & KEY_LEFT)) {
		event.type = getKeyEvent(KEY_LEFT);
		event.kbd.keycode = Common::KEYCODE_4;
		event.kbd.ascii = '4';
		system->addEvent(event);
	}
	if ((getKeysChanged() & KEY_RIGHT)) {
		event.type = getKeyEvent(KEY_RIGHT);
		event.kbd.keycode = Common::KEYCODE_6;
		event.kbd.ascii = '6';
		system->addEvent(event);
	}
	if ((getKeysChanged() & KEY_DOWN)) {
		event.type = getKeyEvent(KEY_DOWN);
		event.kbd.keycode = Common::KEYCODE_2;
		event.kbd.ascii = '2';
		system->addEvent(event);
	}

	if (indyFightRight) {

		if ((getKeysChanged() & KEY_X)) {
			event.type = getKeyEvent(KEY_X);
			event.kbd.keycode = Common::KEYCODE_9;
			event.kbd.ascii = '9';
			system->addEvent(event);
		}
		if ((getKeysChanged() & KEY_A)) {
			event.type = getKeyEvent(KEY_A);
			event.kbd.keycode = Common::KEYCODE_6;
			event.kbd.ascii = '6';
			system->addEvent(event);
		}
		if ((getKeysChanged() & KEY_B)) {
			event.type = getKeyEvent(KEY_B);
			event.kbd.keycode = Common::KEYCODE_3;
			event.kbd.ascii = '3';
			system->addEvent(event);
		}

	} else {

		if ((getKeysChanged() & KEY_X)) {
			event.type = getKeyEvent(KEY_X);
			event.kbd.keycode = Common::KEYCODE_7;
			event.kbd.ascii = '7';
			system->addEvent(event);
		}
		if ((getKeysChanged() & KEY_A)) {
			event.type = getKeyEvent(KEY_A);
			event.kbd.keycode = Common::KEYCODE_4;
			event.kbd.ascii = '4';
			system->addEvent(event);
		}
		if ((getKeysChanged() & KEY_B)) {
			event.type = getKeyEvent(KEY_B);
			event.kbd.keycode = Common::KEYCODE_1;
			event.kbd.ascii = '1';
			system->addEvent(event);
		}

	}


	if ((getKeysChanged() & KEY_Y)) {
		event.type = getKeyEvent(KEY_Y);
		event.kbd.keycode = Common::KEYCODE_5;
		event.kbd.ascii = '5';
		system->addEvent(event);
	}
}


void setKeyboardEnable(bool en) {
	if (en == keyboardEnable) return;
	keyboardEnable = en;
	u16 *backupBank = (u16 *) 0x6040000;

	if (keyboardEnable) {


		DS::drawKeyboard(1, 15, backupBank);


		SUB_BG1_CR = BG_TILE_BASE(1) | BG_MAP_BASE(15);

		if (displayModeIs8Bit) {
			SUB_DISPLAY_CR |= DISPLAY_BG1_ACTIVE;	// Turn on keyboard layer
			SUB_DISPLAY_CR &= ~DISPLAY_BG3_ACTIVE;	// Turn off game layer
		} else {
			SUB_DISPLAY_CR |= DISPLAY_BG1_ACTIVE;	// Turn on keyboard layer
			SUB_DISPLAY_CR &= ~DISPLAY_BG0_ACTIVE;	// Turn off console layer
		}

		// Ensure the keyboard is on the lower screen
		POWER_CR |= POWER_SWAP_LCDS;


	} else {

		DS::releaseAllKeys();
		// Restore the palette that the keyboard has used
		for (int r = 0; r < 256; r++) {
			BG_PALETTE_SUB[r] = BG_PALETTE[r];
		}


		//restoreVRAM(1, 12, backupBank);

		if (displayModeIs8Bit) {
			// Copy the sub screen VRAM from the top screen - they should always be
			// the same.
			u16 *buffer = get8BitBackBuffer();
			s32 stride = get8BitBackBufferStride();

			for (int y = 0; y < gameHeight; y++) {
				for (int x = 0; x < gameWidth; x++) {
					BG_GFX_SUB[y * 256 + x] = buffer[(y * (stride / 2)) + x];
				}
			}
/*
			for (int r = 0; r < (512 * 256) >> 1; r++)
				BG_GFX_SUB[r] = buffer[r];
*/
			SUB_DISPLAY_CR &= ~DISPLAY_BG1_ACTIVE;	// Turn off keyboard layer
			SUB_DISPLAY_CR |= DISPLAY_BG3_ACTIVE;	// Turn on game layer
		} else {
			SUB_DISPLAY_CR &= ~DISPLAY_BG1_ACTIVE;	// Turn off keyboard layer
			SUB_DISPLAY_CR |= DISPLAY_BG0_ACTIVE;	// Turn on console layer
		}

		// Restore the screens so they're the right way round
		if (gameScreenSwap) {
			POWER_CR |= POWER_SWAP_LCDS;
		} else {
			POWER_CR &= ~POWER_SWAP_LCDS;
		}
	}
}

bool getKeyboardEnable() {
	return keyboardEnable;
}

bool getIsDisplayMode8Bit() {
	return displayModeIs8Bit;
}

void doScreenTapMode(OSystem_DS *system) {
	Common::Event event;
	static bool left = false, right = false;

	if (left) {
		event.type = Common::EVENT_LBUTTONUP;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		left = false;
	}

	if (right) {
		event.type = Common::EVENT_RBUTTONUP;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		right = false;
	}


	if (tapComplete == 1) {
		event.type = Common::EVENT_LBUTTONDOWN;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		tapComplete = 0;
		left = true;
	} else if (tapComplete == 2) {
		event.type = Common::EVENT_RBUTTONDOWN;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		tapComplete = 0;
		right = true;
	}

	if (!isScrollingWithDPad()) {

		if (getKeysDown() & KEY_LEFT) {
			event.type = Common::EVENT_LBUTTONDOWN;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}

		if (getKeysReleased() & KEY_LEFT) {
			event.type = Common::EVENT_LBUTTONUP;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}


		if (getKeysDown() & KEY_RIGHT) {
			event.type = Common::EVENT_RBUTTONDOWN;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}

		if (getKeysReleased() & KEY_RIGHT) {
			event.type = Common::EVENT_RBUTTONUP;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}
	}

	event.type = Common::EVENT_MOUSEMOVE;
	event.mouse = Common::Point(getPenX(), getPenY());
	system->addEvent(event);
}

void doButtonSelectMode(OSystem_DS *system) {
	Common::Event event;


	if (!isScrollingWithDPad()) {
		event.type = Common::EVENT_MOUSEMOVE;
		event.mouse = Common::Point(getPenX(), getPenY());
		system->addEvent(event);
		//consolePrintf("x=%d   y=%d  \n", getPenX(), getPenY());
	}

	if (getPenReleased() && (leftButtonDown || rightButtonDown)) {
		if (leftButtonDown) {
			event.type = Common::EVENT_LBUTTONUP;
			leftButtonDown = false;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		} else if (rightButtonDown) {
			event.type = Common::EVENT_RBUTTONUP;
			rightButtonDown = false;
			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}
	}


	if ((mouseMode != MOUSE_HOVER) || (!displayModeIs8Bit)) {
		if (getPenDown() && !isScrollingWithDPad()) {
			if (mouseMode == MOUSE_LEFT) {
				event.type = Common::EVENT_LBUTTONDOWN;
				leftButtonDown = true;
			} else {
				event.type = Common::EVENT_RBUTTONDOWN;
				rightButtonDown = true;
			}

			event.mouse = Common::Point(getPenX(), getPenY());
			system->addEvent(event);
		}

	} else {
		// In hover mode, D-pad left and right click the mouse when the pen is on the screen

		if (getPenHeld()) {
			if (getKeysDown() & KEY_LEFT) {
				event.type = Common::EVENT_LBUTTONDOWN;
				event.mouse = Common::Point(getPenX(), getPenY());
				system->addEvent(event);
			}
			if (getKeysReleased() & KEY_LEFT) {
				event.type = Common::EVENT_LBUTTONUP;
				event.mouse = Common::Point(getPenX(), getPenY());
				system->addEvent(event);
			}


			if (getKeysDown() & KEY_RIGHT) {
				event.type = Common::EVENT_RBUTTONDOWN;
				event.mouse = Common::Point(getPenX(), getPenY());
				system->addEvent(event);
			}
			if (getKeysReleased() & KEY_RIGHT) {
				event.type = Common::EVENT_RBUTTONUP;
				event.mouse = Common::Point(getPenX(), getPenY());
				system->addEvent(event);
			}
		}
	}

	if (!isScrollingWithDPad() && !getIndyFightState() && !getKeyboardEnable()) {

		if (!getPenHeld() || (mouseMode != MOUSE_HOVER)) {
			if (getKeysDown() & KEY_LEFT) {
				mouseMode = MOUSE_LEFT;
			}

			if (rightButtonDown) {
				event.mouse = Common::Point(getPenX(), getPenY());
				event.type = Common::EVENT_RBUTTONUP;
				system->addEvent(event);
				rightButtonDown = false;
			}


			if (getKeysDown() & KEY_RIGHT) {
				if ((s_currentGame->control != CONT_SCUMM_SAMNMAX) && (s_currentGame->control != CONT_FUTURE_WARS) && (s_currentGame->control != CONT_GOBLINS)) {
					mouseMode = MOUSE_RIGHT;
				} else {
					// If we're playing sam and max, click and release the right mouse
					// button to change verb

					if (s_currentGame->control == CONT_FUTURE_WARS) {
						event.mouse = Common::Point(320 - 128, 200 - 128);
						event.type = Common::EVENT_MOUSEMOVE;
						system->addEvent(event);
					} else {
						event.mouse = Common::Point(getPenX(), getPenY());
					}

					rightButtonDown = true;


					event.type = Common::EVENT_RBUTTONDOWN;
					system->addEvent(event);

					//event.type = Common::EVENT_RBUTTONUP;
					//system->addEvent(event);
				}
			}



			if (getKeysDown() & KEY_UP) {
				mouseMode = MOUSE_HOVER;
			}
		}
	}
}

void addEventsToQueue() {
	#ifdef HEAVY_LOGGING
	consolePrintf("addEventsToQueue\n");
	#endif
	OSystem_DS *system = OSystem_DS::instance();
	Common::Event event;

#ifdef USE_PROFILER
/*
	if (keysDown() & KEY_R) {
		cygprofile_begin();
		cygprofile_enable();
	}
	if (keysDown() & KEY_L) {
		cygprofile_disable();
		cygprofile_end();
	}
*/
#endif



	if (system->isEventQueueEmpty()) {

/*
		if (getKeysDown() & KEY_L) {
			tweak--;
			consolePrintf("Tweak: %d\n", tweak);
			IPC->tweakChanged = true;
		}


		if (getKeysDown() & KEY_R) {
			tweak++;
			consolePrintf("Tweak: %d\n", tweak);
			IPC->tweakChanged = true;
		}
	*/
		if ((keysHeld() & KEY_L) && (keysHeld() & KEY_R)) {
			memoryReport();
		}

		if (displayModeIs8Bit) {

			if (!indyFightState) {

				if (!isScrollingWithDPad() && (getKeysDown() & KEY_B)) {
					if (s_currentGame->control == CONT_AGI) {
						event.kbd.keycode = Common::KEYCODE_RETURN;
						event.kbd.ascii = 13;
						event.kbd.flags = 0;
					} else {
						event.kbd.keycode = Common::KEYCODE_ESCAPE;
						event.kbd.ascii = 27;
						event.kbd.flags = 0;
					}

					event.type = Common::EVENT_KEYDOWN;
					system->addEvent(event);

					event.type = Common::EVENT_KEYUP;
					system->addEvent(event);
				}

			}



			if ((!getIndyFightState()) && (getKeysDown() & KEY_Y)) {
				consoleEnable = !consoleEnable;
				consolePrintf("Console enable: %d\n", consoleEnable);
				if (displayModeIs8Bit) {
					displayMode8Bit();
				} else {
					displayMode16Bit();
				}
			}

			if ((getKeyboardEnable())) {
				event.kbd.flags = 0;

				bool down = getKeysDown() & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN);
				bool release = getKeysReleased() & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN);
				bool shoulders = getKeysHeld() & (KEY_L | KEY_R);

				if ( (down && (!shoulders)) || release) {

					if (getKeysChanged() & KEY_LEFT) {
						event.kbd.keycode = Common::KEYCODE_LEFT;
						event.kbd.ascii = 0;
						event.type = getKeyEvent(KEY_LEFT);
						system->addEvent(event);
					}

					if (getKeysChanged() & KEY_RIGHT) {
						event.kbd.keycode = Common::KEYCODE_RIGHT;
						event.kbd.ascii = 0;
						event.type = getKeyEvent(KEY_RIGHT);
						system->addEvent(event);
					}

					if (getKeysChanged() & KEY_UP) {
						event.kbd.keycode = Common::KEYCODE_UP;
						event.kbd.ascii = 0;
						event.type = getKeyEvent(KEY_UP);
						system->addEvent(event);
					}

					if (getKeysChanged() & KEY_DOWN) {
						event.kbd.keycode = Common::KEYCODE_DOWN;
						event.kbd.ascii = 0;
						event.type = getKeyEvent(KEY_DOWN);
						system->addEvent(event);
					}
				}

			}

			if (!isScrollingWithDPad() && !getIndyFightState() && !getKeyboardEnable()) {

				if ((getKeysDown() & KEY_A) && (!indyFightState)) {
					gameScreenSwap = !gameScreenSwap;

					if (gameScreenSwap) {
						POWER_CR |= POWER_SWAP_LCDS;
					} else {
						POWER_CR &= ~POWER_SWAP_LCDS;
					}

				}
			}


			static int selectTimeDown = -1;
			static const int SELECT_HOLD_TIME = 1000;

			if (getKeysDown() & KEY_SELECT) {
				selectTimeDown = getMillis();
			}

			if (selectTimeDown != -1) {
				if (getKeysHeld() & KEY_SELECT) {
					if (getMillis() - selectTimeDown >= SELECT_HOLD_TIME) {
						// Hold select down for one second - show GMM
						g_engine->openMainMenuDialog();
					}
				}

				if (getKeysReleased() & KEY_SELECT) {
					if (getMillis() - selectTimeDown < SELECT_HOLD_TIME) {
						// Just pressed select - show DS options screen
						showOptionsDialog();
					}
				}
			}
		}

		if (!getIndyFightState() && !isScrollingWithDPad() && (getKeysDown() & KEY_X)) {
			setKeyboardEnable(!keyboardEnable);
		}

		updateStatus();


		if ((tapScreenClicks) && (getIsDisplayMode8Bit())) {
			if ((!keyboardEnable) || (!isInsideKeyboard(penDownX, penDownY))) {
				doScreenTapMode(system);
			}
		} else {
			if (!keyboardEnable) {
				doButtonSelectMode(system);
			} else if ((!keyboardEnable) || (!isInsideKeyboard(penDownX, penDownY))) {
				doScreenTapMode(system);
			}
		}


		if (!keyboardEnable) {
			if ((!isScrollingWithDPad() || (indyFightState)) && (displayModeIs8Bit)) {
				// Controls specific to the control method

				if (s_currentGame->control == CONT_SKY) {
					// Extra controls for Beneath a Steel Sky
					if ((getKeysDown() & KEY_DOWN)) {
						penY = 0;
						penX = 160;		// Show inventory by moving mouse onto top line
					}
				}

				if (s_currentGame->control == CONT_AGI) {
					// Extra controls for Leisure Suit Larry and KQ4
					if ((getKeysHeld() & KEY_UP) && (getKeysHeld() & KEY_START)
						/*&& (!strcmp(gameName, "LLLLL"))*/) {
						consolePrintf("Cheat key!\n");
						event.type = Common::EVENT_KEYDOWN;
						event.kbd.keycode = (Common::KeyCode)'X';		// Skip age test in LSL
						event.kbd.ascii = 'X';
						event.kbd.flags = Common::KBD_ALT;
						system->addEvent(event);

						event.type = Common::EVENT_KEYUP;
						system->addEvent(event);
					}
				}

				if (s_currentGame->control == CONT_SIMON) {
					// Extra controls for Simon the Sorcerer
					if ((getKeysDown() & KEY_DOWN)) {
						event.type = Common::EVENT_KEYDOWN;
						event.kbd.keycode = Common::KEYCODE_F10;		// F10 or # - show hotspots
						event.kbd.ascii = Common::ASCII_F10;
						event.kbd.flags = 0;
						system->addEvent(event);
//						consolePrintf("F10\n");

						event.type = Common::EVENT_KEYUP;
						system->addEvent(event);
					}
				}

				if (s_currentGame->control == CONT_SCUMM_ORIGINAL) {
					// Extra controls for Scumm v1-5 games
					if ((getKeysDown() & KEY_DOWN)) {
						event.type = Common::EVENT_KEYDOWN;
						event.kbd.keycode = Common::KEYCODE_PERIOD;		// Full stop - skips current dialogue line
						event.kbd.ascii = '.';
						event.kbd.flags = 0;
						system->addEvent(event);

						event.type = Common::EVENT_KEYUP;
						system->addEvent(event);
					}

					if (indyFightState) {
						addIndyFightingKeys();
					}

				}

			}
		}

		if (!displayModeIs8Bit) {
			// Front end controls

			if (leftHandedSwap(getKeysChanged()) & KEY_UP) {
				event.type = getKeyEvent(leftHandedSwap(KEY_UP));
				event.kbd.keycode = Common::KEYCODE_UP;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);
			}

			if (leftHandedSwap(getKeysChanged()) & KEY_DOWN) {
				event.type = getKeyEvent(leftHandedSwap(KEY_DOWN));
				event.kbd.keycode = Common::KEYCODE_DOWN;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);
			}

			if (leftHandedSwap(getKeysDown()) & KEY_A) {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_RETURN;
				event.kbd.ascii = 0;
				event.kbd.flags = 0;
				system->addEvent(event);

				event.type = Common::EVENT_KEYUP;
				system->addEvent(event);
			}

		}


		if ((getKeysChanged() & KEY_START)) {
			event.kbd.flags = 0;
			event.type = getKeyEvent(KEY_START);
			if (s_currentGame->control == CONT_FUTURE_WARS) {
				event.kbd.keycode = Common::KEYCODE_F10;
				event.kbd.ascii = Common::ASCII_F10;
			} else if (s_currentGame->control == CONT_GOBLINS) {
				event.kbd.keycode = Common::KEYCODE_F1;
				event.kbd.ascii = Common::ASCII_F1;
//				consolePrintf("!!!!!F1!!!!!");
			} else if (s_currentGame->control == CONT_AGI) {
				event.kbd.keycode = Common::KEYCODE_ESCAPE;
				event.kbd.ascii = 27;
			} else {
				event.kbd.keycode = Common::KEYCODE_F5;		// F5
				event.kbd.ascii = Common::ASCII_F5;
//				consolePrintf("!!!!!F5!!!!!");
			}
			system->addEvent(event);
		}


		if (keyboardEnable) {
			DS::addKeyboardEvents();
		}

		consumeKeys();
		consumePenEvents();
	}
}



void triggerIcon(int imageNum) {
	triggeredIcon = imageNum;
	triggeredIconTimeout = 120;
}


void setIcon(int num, int x, int y, int imageNum, int flags, bool enable) {
	sprites[num].attribute[0] = ATTR0_BMP | (enable? (y & 0xFF): 192) | (!enable? ATTR0_DISABLED: 0);
	sprites[num].attribute[1] = ATTR1_SIZE_32 | (x & 0x1FF) | flags;
	sprites[num].attribute[2] = ATTR2_ALPHA(1)| (imageNum * 16);
}

void setIconMain(int num, int x, int y, int imageNum, int flags, bool enable) {
	spritesMain[num].attribute[0] = ATTR0_BMP | (y & 0xFF) | (!enable? ATTR0_DISABLED: 0);
	spritesMain[num].attribute[1] = ATTR1_SIZE_32 | (x & 0x1FF) | flags;
	spritesMain[num].attribute[2] = ATTR2_ALPHA(1)| (imageNum * 16);
}

void updateStatus() {
	int offs;

	if (displayModeIs8Bit) {
		if (!tapScreenClicks) {
			switch (mouseMode) {
			case MOUSE_LEFT:
				offs = 1;
				break;
			case MOUSE_RIGHT:
				offs = 2;
				break;
			case MOUSE_HOVER:
				offs = 0;
				break;
			default:
				// Nothing!
				offs = 0;
				break;
			}

			setIcon(0, 208, 150, offs, 0, true);
		}

		if (indyFightState) {
			setIcon(1, (190 - 32), 150, 3, (indyFightRight? 0: ATTR1_FLIP_X), true);
//			consolePrintf("%d\n", indyFightRight);
		} else {
//			setIcon(1, 0, 0, 0, 0, false);
		}

		if (triggeredIconTimeout > 0) {
			triggeredIconTimeout--;
			setIcon(4, 16, 150, triggeredIcon, 0, true);
		} else {
			setIcon(4, 0, 0, 0, 0, false);
		}

	} else {
		setIcon(0, 0, 0, 0, 0, false);
		setIcon(1, 0, 0, 0, 0, false);
		setIcon(2, 0, 0, 0, 0, false);
		setIcon(3, 0, 0, 0, 0, false);
		setIcon(4, 0, 0, 0, 0, false);
	}

	if ((keyboardIcon) && (!keyboardEnable) && (!displayModeIs8Bit)) {
//		spritesMain[0].attribute[0] = ATTR0_BMP | 160;
//		spritesMain[0].attribute[1] = ATTR1_SIZE_32 | 0;
//		spritesMain[0].attribute[2] = ATTR2_ALPHA(1) | 64;
		setIconMain(0, 0, 160, 4, 0, true);
	} else {
//		spritesMain[0].attribute[0] = ATTR0_DISABLED;
//		spritesMain[0].attribute[1] = 0;
//		spritesMain[0].attribute[2] = 0;
//		spritesMain[0].filler = 0;
		setIconMain(0, 0, 0, 0, 0, false);
	}

}

void soundBufferEmptyHandler() {
	REG_IF = IRQ_TIMER2;

	if (soundHiPart) {
//		bufferSecondHalf = true;
	} else {
//		bufferFirstHalf = true;
	}

// TIMER0
	if ((callback) && (callbackTimer > 0)) {
		callbackTimer--;
	}
	currentTimeMillis++;
// TIMER0 end

	soundHiPart = !soundHiPart;
}

void setMainScreenScroll(int x, int y) {
/*	if (gameScreenSwap) {
		SUB_BG3_CX = x + (((frameCount & 1) == 0)? 64: 0);
		SUB_BG3_CY = y;
	} else */{
		BG3_CX = x + (((frameCount & 1) == 0)? 64: 0);
		BG3_CY = y;

		if ((!gameScreenSwap) || (touchPadStyle)) {
			touchX = x >> 8;
			touchY = y >> 8;
		}
	}
}

void setMainScreenScale(int x, int y) {
/*	if (gameScreenSwap) {
		SUB_BG3_XDX = x;
		SUB_BG3_XDY = 0;
		SUB_BG3_YDX = 0;
		SUB_BG3_YDY = y;
	} else*/ {
		if (isCpuScalerEnabled() && (x==320)) {
			BG3_XDX = 256;
			BG3_XDY = 0;
			BG3_YDX = 0;
			BG3_YDY = y;
		} else {
			BG3_XDX = x;
			BG3_XDY = 0;
			BG3_YDX = 0;
			BG3_YDY = y;
		}

		if ((!gameScreenSwap) || (touchPadStyle)) {
			touchScX = x;
			touchScY = y;
		}
	}
}

void setZoomedScreenScroll(int x, int y, bool shake) {
/*	if (gameScreenSwap) {
		BG3_CX = x + ((shake && ((frameCount & 1) == 0))? 64: 0);
		BG3_CY = y;

		touchX = x >> 8;
		touchY = y >> 8;
	} else */{

		if ((gameScreenSwap) && (!touchPadStyle)) {
			touchX = x >> 8;
			touchY = y >> 8;
		}


		SUB_BG3_CX = x + ((shake && (frameCount & 1) == 0)? 64: 0);
		SUB_BG3_CY = y;
	}
}

void setZoomedScreenScale(int x, int y) {
/*	if (gameScreenSwap) {
		BG3_XDX = x;
		BG3_XDY = 0;
		BG3_YDX = 0;
		BG3_YDY = y;

	} else */{

		if ((gameScreenSwap) && (!touchPadStyle)) {
			touchScX = x;
			touchScY = y;
		}

		SUB_BG3_XDX = x;
		SUB_BG3_XDY = 0;
		SUB_BG3_YDX = 0;
		SUB_BG3_YDY = y;
	}
}

#ifdef USE_PROFILER
void VBlankHandler(void) __attribute__ ((no_instrument_function));
#endif

void VBlankHandler(void) {
//	BG_PALETTE[0] = RGB15(31, 31, 31);
//	if (*((int *) (0x023FFF00)) != 0xBEEFCAFE) {
	//	consolePrintf("Guard band overwritten!");
//  }

	//consolePrintf("X:%d Y:%d\n", getPenX(), getPenY());
/*
	if ((callback) && (callbackTimer > 0)) {
		callbackTimer--;
	}
	currentTimeMillis++;
*/
/*	static int firstTime = 1;

	// This is to ensure that the ARM7 vblank handler runs before this one.
	// Fixes the problem with the MMD when the screens swap over on load.
	if (firstTime > 0) {
		REG_IF = IRQ_VBLANK;
		firstTime--;
		return;
	}
*/

	IPC->tweak = tweak;
	soundUpdate();


	if ((!gameScreenSwap) && !isScrollingWithDPad()) {
		if (s_currentGame) {
			if (s_currentGame->control != CONT_SCUMM_SAMNMAX) {
				if (getPenHeld() && (getPenY() < SCUMM_GAME_HEIGHT)) {
					setTopScreenTarget(getPenX(), getPenY());
				}
			} else {
				if (getPenHeld()) {
					setTopScreenTarget(getPenX(), getPenY());
				}
			}
		}
	}


	penUpdate();
	keysUpdate();


	frameCount++;

	if ((cursorEnable) && (mouseCursorVisible)) {
		storedMouseX = penX;
		storedMouseY = penY;

		if (gameScreenSwap && touchPadStyle) {
			setIcon(3, storedMouseX - mouseHotspotX, storedMouseY - mouseHotspotY, 8, 0, true);
			setIconMain(3, 0, 0, 0, 0, false);
		} else {
			setIconMain(3, storedMouseX - mouseHotspotX, storedMouseY - mouseHotspotY, 8, 0, true);
			setIcon(3, 0, 0, 0, 0, false);
		}
	} else {
		setIconMain(3, 0, 0, 0, 0, false);
		setIcon(3, 0, 0, 0, 0, false);
	}


	if (callback) {
		callbackTimer -= FRAME_TIME;
	}

	if (isScrollingWithDPad()) {

		if ((!dragging) && (getPenHeld()) && (penDownFrames > 5)) {
			dragging = true;
			dragStartX = penX;
			dragStartY = penY;

			if (gameScreenSwap) {
				dragScX = subScTargetX;
				dragScY = subScTargetY;
			} else {
				dragScX = scX;
				dragScY = scY;
			}


		}

		if ((dragging) && (!getPenHeld())) {
			dragging = false;
		}

		if (dragging) {

			if (gameScreenSwap) {
				subScTargetX = dragScX + ((dragStartX - penX) << 8);
				subScTargetY = dragScY + ((dragStartY - penY) << 8);
			} else {
				scX = dragScX + ((dragStartX - penX));
				scY = dragScY + ((dragStartY - penY));
			}

//			consolePrintf("X:%d Y:%d\n", dragStartX - penX, dragStartY - penY);
		}
	}


/*	if ((frameCount & 1) == 0) {
		SUB_BG3_CX = subScX;
	} else {
		SUB_BG3_CX = subScX + 64;
	}

	SUB_BG3_CY = subScY + (s_shakePos << 8);*/

	/*SUB_BG3_XDX = (int) (subScreenWidth / 256.0f * 256);
	SUB_BG3_XDY = 0;
	SUB_BG3_YDX = 0;
	SUB_BG3_YDY = (int) (subScreenHeight / 192.0f * 256);*/

	static int ratio = (320 << 8) / SCUMM_GAME_WIDTH;

	bool zooming = false;

	if (isScrollingWithDPad()) {
		if ((getKeysHeld() & KEY_A) && (subScreenScale < ratio)) {
			subScreenScale += 1;
			zooming = true;
		}

		if ((getKeysHeld() & KEY_B) && (subScreenScale > 128)) {
			subScreenScale -=1;
			zooming = true;
		}
	}


	int xCenter = subScTargetX + ((subScreenWidth >> 1) << 8);
	int yCenter = subScTargetY + ((subScreenHeight >> 1) << 8);


	if (twoHundredPercentFixedScale) {
		subScreenWidth = 256 >> 1;
		subScreenHeight = 192 >> 1;
	} else {
//		subScreenWidth = (((SCUMM_GAME_HEIGHT * 256) / 192) * subScreenScale) >> 8;
//		subScreenHeight = SCUMM_GAME_HEIGHT * subScreenScale >> 8;


		subScreenWidth = (256 * subScreenScale) >> 8;
		subScreenHeight = (192 * subScreenScale) >> 8;

		if ( ((subScreenWidth) > 256 - 8) && ((subScreenWidth) < 256 + 8) ) {
			subScreenWidth = 256;
			subScreenHeight = 192;
			if (zooming) {
				subScX = subScTargetX;
				subScY = subScTargetY;
			 	triggerIcon(5);
			}
		} else if ( ((subScreenWidth) > 128 - 8) && ((subScreenWidth) < 128 + 8) ) {
			subScreenWidth = 128;
			subScreenHeight = 96;
			if (zooming) {
				subScX = subScTargetX;
				subScY = subScTargetY;
				triggerIcon(6);
			}
		} else if (subScreenWidth > 256) {
			subScreenWidth = 320;
			subScreenHeight = 200;
			if (zooming) {
				subScX = subScTargetX;
				subScY = subScTargetY;
				triggerIcon(7);
			}
		} else {
			//triggerIcon(-1);
		}
	}


	subScTargetX = xCenter - ((subScreenWidth >> 1) << 8);
	subScTargetY = yCenter - ((subScreenHeight >> 1) << 8);




	if (subScTargetX < 0) subScTargetX = 0;
	if (subScTargetX > (gameWidth - subScreenWidth) << 8) subScTargetX = (gameWidth - subScreenWidth) << 8;

	if (subScTargetY < 0) subScTargetY = 0;
	if (subScTargetY > (gameHeight - subScreenHeight) << 8) subScTargetY = (gameHeight - subScreenHeight) << 8;



	subScX += (subScTargetX - subScX) >> 2;
	subScY += (subScTargetY - subScY) >> 2;

	if (displayModeIs8Bit) {

		if (isScrollingWithDPad()) {

			int offsX = 0, offsY = 0;


			if ((getKeysHeld() & KEY_LEFT)) {
				offsX -= 2;
			}

			if ((getKeysHeld() & KEY_RIGHT)) {
				offsX += 2;
			}

			if ((getKeysHeld() & KEY_UP)) {
				offsY -= 2;
			}

			if ((getKeysHeld() & KEY_DOWN)) {
				offsY += 2;
			}

			if (((gameScreenSwap) && (getKeysHeld() & KEY_L)) || ((!gameScreenSwap) && (getKeysHeld() & KEY_R))) {
				subScTargetX += offsX << 8;
				subScTargetY += offsY << 8;
			} else {
				scX += offsX;
				scY += offsY;
			}
		}

		if (!scaledMode) {

			if (scX + 256 > gameWidth - 1) {
				scX = gameWidth - 1 - 256;
			}

			if (scX < 0) {
				scX = 0;
			}

			if (scY + 192 > gameHeight - 1) {
				scY = gameHeight - 1 - 192;
			}

			if (scY < 0) {
				scY = 0;
			}

			setZoomedScreenScroll(subScX, subScY, (subScreenWidth != 256) && (subScreenWidth != 128));
			setZoomedScreenScale(subScreenWidth, ((subScreenHeight * (256 << 8)) / 192) >> 8);


			setMainScreenScroll(scX << 8, (scY << 8) + (s_shakePos << 8));
			setMainScreenScale(256, 256);		// 1:1 scale

		} else {

			if (scY > gameHeight - 192 - 1) {
				scY = gameHeight - 192 - 1;
			}

			if (scY < 0) {
				scY = 0;
			}

			setZoomedScreenScroll(subScX, subScY, (subScreenWidth != 256) && (subScreenWidth != 128));
			setZoomedScreenScale(subScreenWidth, ((subScreenHeight * (256 << 8)) / 192) >> 8);

			setMainScreenScroll(64, (scY << 8) + (s_shakePos << 8));
			setMainScreenScale(320, 256);		// 1:1 scale

		}
	} else {
		setZoomedScreenScroll(0, 0, true);
		setZoomedScreenScale(320, 256);

		setMainScreenScroll(0, 0);
		setMainScreenScale(320, 256);		// 1:1 scale
	}

	// Enable on screen keyboard when pen taps icon
	if ((keyboardIcon) && (penX < 32) && (penY > 160) && (penHeld)) {
		setKeyboardEnable(true);
	}

	if (keyboardEnable) {
		if (DS::getKeyboardClosed()) {
			setKeyboardEnable(false);
		}
	}

	updateOAM();

	//PALETTE[0] = RGB15(0, 0, 0);
	//REG_IF = IRQ_VBLANK;
}

int getMillis() {
	return currentTimeMillis;
//	return frameCount * FRAME_TIME;
}

void setTimerCallback(OSystem_DS::TimerProc proc, int interval) {
//	consolePrintf("Set timer proc %x, int %d\n", proc, interval);
	callback = proc;
	callbackInterval = interval;
	callbackTimer = interval;
}

void timerTickHandler() {
//	REG_IF = IRQ_TIMER0;
	if ((callback) && (callbackTimer > 0)) {
		callbackTimer--;
	}
	currentTimeMillis++;
}





void setTalkPos(int x, int y) {
//	if (gameID != Scumm::GID_SAMNMAX) {
//		setTopScreenTarget(x, 0);
//	} else {
		setTopScreenTarget(x, y);
//	}
}

void setTopScreenTarget(int x, int y) {
	subScTargetX = (x - (subScreenWidth >> 1));
	subScTargetY = (y - (subScreenHeight >> 1));

	if (subScTargetX < 0) subScTargetX = 0;
	if (subScTargetX > gameWidth - subScreenWidth) subScTargetX = gameWidth - subScreenWidth;

	if (subScTargetY < 0) subScTargetY = 0;
	if (subScTargetY > gameHeight - subScreenHeight) subScTargetY = gameHeight - subScreenHeight;

	subScTargetX <<=8;
	subScTargetY <<=8;
}

#ifdef USE_PROFILER
void hBlankHanlder() __attribute__ ((no_instrument_function));

void hBlankHandler() {
	hBlankCount++;
}
#endif

void uploadSpriteGfx() {
	vramSetBankD(VRAM_D_SUB_SPRITE);
	vramSetBankE(VRAM_E_MAIN_SPRITE);

	// Convert texture from 24bit 888 to 16bit 1555, remembering to set top bit!
	const u8 *srcTex = (const u8 *) ::icons_raw;
	for (int r = 32 * 256 ; r >= 0; r--) {
		SPRITE_GFX_SUB[r] = 0x8000 | (srcTex[r * 3] >> 3) | ((srcTex[r * 3 + 1] >> 3) << 5) | ((srcTex[r * 3 + 2] >> 3) << 10);
		SPRITE_GFX[r] = 0x8000 | (srcTex[r * 3] >> 3) | ((srcTex[r * 3 + 1] >> 3) << 5) | ((srcTex[r * 3 + 2] >> 3) << 10);
	}

}

void initHardware() {
	// Guard band
//((int *) (0x023FFF00)) = 0xBEEFCAFE;


	penInit();

	powerOn(POWER_ALL);
/*	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG); */
	vramSetBankD(VRAM_D_SUB_SPRITE);
	vramSetBankE(VRAM_E_MAIN_SPRITE);

	currentTimeMillis = 0;


/*
	// Set up a millisecond counter
	TIMER0_CR = 0;
	TIMER0_DATA = 0xFFFF;
	TIMER0_CR = TIMER_ENABLE | TIMER_CASCADE;
*/


	for (int r = 0; r < 255; r++) {
		BG_PALETTE[r] = 0;
	}

	BG_PALETTE[255] = RGB15(0,31,0);


	for (int r = 0; r < 255; r++) {
		BG_PALETTE_SUB[r] = 0;
	}

	BG_PALETTE_SUB[255] = RGB15(0,31,0);

	// Allocate save buffer for game screen
//	savedBuffer = new u8[320 * 200];
	displayMode16Bit();

	memset(BG_GFX, 0, 512 * 256 * 2);
	scaledMode = true;
	scX = 0;
	scY = 0;
	subScX = 0;
	subScY = 0;
	subScTargetX = 0;
	subScTargetY = 0;

	//lcdSwap();
	POWER_CR &= ~POWER_SWAP_LCDS;

	frameCount = 0;
	callback = NULL;

//	vramSetBankH(VRAM_H_SUB_BG);


//	// Do text stuff
	//BG0_CR = BG_MAP_BASE(0) | BG_TILE_BASE(1);
//	BG0_Y0 = 48;

	BG_PALETTE[255] = RGB15(31,31,31);//by default font will be rendered with color 255

	//consoleInit() is a lot more flexible but this gets you up and running quick
//	consoleInitDefault((u16 *)SCREEN_BASE_BLOCK(0), (u16 *)CHAR_BASE_BLOCK(1), 16);
	//consolePrintSet(0, 6);

	//irqs are nice
	irqInit();
	irqInitHandler(OurIntrMain);
	irqSet(IRQ_VBLANK, VBlankHandler);
	irqSet(IRQ_TIMER0, timerTickHandler);
	irqSet(IRQ_TIMER2, soundBufferEmptyHandler);

	irqEnable(IRQ_VBLANK);
	irqEnable(IRQ_TIMER0);
//	irqEnable(IRQ_TIMER2);

#ifdef USE_PROFILER
	irqSet(IRQ_HBLANK, hBlankHandler);
	irqEnable(IRQ_HBLANK);
#endif


	// Set up a millisecond timer
	#ifdef HEAVY_LOGGING
	consolePrintf("Setting up timer...");
	#endif
	TIMER0_CR = 0;
	TIMER0_DATA = (u32) TIMER_FREQ(1000);
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;
	REG_IME = 1;
	#ifdef HEAVY_LOGGING
	consolePrintf("done\n");
	#endif

	BG_PALETTE[255] = RGB15(0,0,31);

	initSprites();

//	videoSetModeSub(MODE_3_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP); //sub bg 0 will be used to print text

	// If the software scaler's back buffer has not been allocated, do it now
	scalerBackBuffer = (u8 *) malloc(320 * 256);


	WAIT_CR &= ~(0x0080);
//	REG_WRAM_CNT = 0;

	uploadSpriteGfx();

	// This is a bodge to get around the fact that the cursor is turned on before it's image is set
	// during startup in Sam & Max.  This bodge moves the cursor offscreen so it is not seen.
	sprites[1].attribute[1] = ATTR1_SIZE_64 | 192;

}




void setKeyboardIcon(bool enable) {
	keyboardIcon = enable;
}

bool getKeyboardIcon() {
	return keyboardIcon;
}


////////////////////
// Pen stuff
////////////////////


void penInit() {
	penDown = false;
	penHeld = false;
	penReleased = false;
	penDownLastFrame = false;
	penDownSaved = false;
	penReleasedSaved = false;
	penDownFrames = 0;
	consumeKeys();
}

void penUpdate() {

//	if (getKeysHeld() & KEY_L) consolePrintf("%d, %d   penX=%d, penY=%d tz=%d\n", IPC->touchXpx, IPC->touchYpx, penX, penY, IPC->touchZ1);

	bool penDownThisFrame = (IPC->touchZ1 > 0) && (IPC->touchXpx > 0) && (IPC->touchYpx > 0);
	static bool moved = false;

	if (( (tapScreenClicks) || getKeyboardEnable() ) && (getIsDisplayMode8Bit())) {


		if ((tapTimeout >= 0)) {
			tapTimeout++;

			if (((tapTimeout > 15) || (tapCount == 2)) && (tapCount > 0)) {
				tapComplete = tapCount;
				tapCount = 0;
//				consolePrintf("Taps: %d\n", tapComplete);
			}
		}



		if ((penHeld) && (!penDownThisFrame)) {
			if ((touchPadStyle) || (getKeyboardEnable() && (!isInsideKeyboard(penDownX, penDownY))) || (moved) || (tapCount == 1)) {
				if ((penDownFrames > 0) && (penDownFrames < 6) && ((tapTimeout == -1) || (tapTimeout > 2))) {
					tapCount++;
					tapTimeout = 0;
//					consolePrintf("Tap! %d\n", penDownFrames);
					moved = false;
				}
			}
		}
	}


	if ( ((keyboardEnable) || (touchPadStyle)) && (getIsDisplayMode8Bit()) ) {
		// Relative positioning mode


		if ((penDownFrames > 0) ) {

			if ((penHeld)) {

				if (penDownThisFrame) {
					if (penDownFrames >= 2) {

						if ((!keyboardEnable) || (!isInsideKeyboard(IPC->touchXpx, IPC->touchYpx))) {
							int diffX = IPC->touchXpx - penDownX;
							int diffY = IPC->touchYpx - penDownY;

							int speed = ABS(diffX) + ABS(diffY);

							if ((ABS(diffX) < 35) && (ABS(diffY) < 35)) {

								if (speed >= 8)	{
									diffX *= ((speed >> 3) * touchPadSensitivity) >> 3;
									diffY *= ((speed >> 3) * touchPadSensitivity) >> 3;
								}

								penX += diffX;
								penY += diffY;

								if (penX > 255) {
									scX -= 255 - penX;
									penX = 255;
								}

								if (penX < 0) {
									scX -= -penX;
									penX = 0;
								}

								if (penY > 191) {
									scY += penY - 191;
									penY = 191;
								}

								if (penY < 0) {
									scY -= -penY;
									penY = 0;
								}
							}

	//						consolePrintf("x: %d y: %d\n", IPC->touchYpx - penDownY, IPC->touchYpx - penDownY);
						}
						penDownX = IPC->touchXpx;
						penDownY = IPC->touchYpx;

					}
				}

			} else {
				penDown = true;
				penHeld = true;
				penDownSaved = true;

				// First frame, so save pen positions
				if (penDownThisFrame) {
					penDownX = IPC->touchXpx;
					penDownY = IPC->touchYpx;
				}
			}
		} else {
			if (penHeld) {
				penReleased = true;
				penReleasedSaved = true;
			} else {
				penReleased = false;
			}

			penDown = false;
			penHeld = false;
		}


	} else {	// Absolute positioning mode
		if ((penDownFrames > 1)) {			// Is this right?  Dunno, but it works for me.

			if ((penHeld)) {
				penHeld = true;
				penDown = false;
			} else {
				if (penDownFrames == 2) {
					penDownX = IPC->touchXpx;
					penDownY = IPC->touchYpx;
				}
				penDown = true;
				penHeld = true;
				penDownSaved = true;
			}

			if ((IPC->touchZ1 > 0) && (IPC->touchXpx > 0) && (IPC->touchYpx > 0)) {
				penX = IPC->touchXpx + touchXOffset;
				penY = IPC->touchYpx + touchYOffset;
				moved = true;
			}


		} else {
			if (penHeld) {
				penReleased = true;
				penReleasedSaved = true;
			} else {
				penReleased = false;
			}

			penDown = false;
			penHeld = false;
		}
	}



	if ((IPC->touchZ1 > 0) || ((penDownFrames == 2)) ) {
		penDownLastFrame = true;
		penDownFrames++;
	} else {
		penDownLastFrame = false;
		penDownFrames = 0;
	}
}

int leftHandedSwap(int keys) {
	// Start and select are unchanged
	if (leftHandedMode) {
		int result = keys & (~(KEY_R | KEY_L | KEY_Y | KEY_A | KEY_B | KEY_X | KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN));

		if (keys & KEY_L) result |= KEY_R;
		if (keys & KEY_R) result |= KEY_L;

		if (keys & KEY_LEFT) result |= KEY_Y;
		if (keys & KEY_RIGHT) result |= KEY_A;
		if (keys & KEY_DOWN) result |= KEY_B;
		if (keys & KEY_UP) result |= KEY_X;

		if (keys & KEY_Y) result |= KEY_LEFT;
		if (keys & KEY_A) result |= KEY_RIGHT;
		if (keys & KEY_B) result |= KEY_DOWN;
		if (keys & KEY_X) result |= KEY_UP;

		return result;
	} else {
		return keys;
	}
}

void keysUpdate() {
	scanKeys();
	keysDownSaved |= leftHandedSwap(keysDown());
	keysReleasedSaved |= leftHandedSwap(keysUp());
	keysChangedSaved = keysDownSaved | keysReleasedSaved;
}

int getKeysDown() {
	return keysDownSaved;
}

int getKeysHeld() {
	return leftHandedSwap(keysHeld());
}

int getKeysReleased() {
	return keysReleasedSaved;
}

int getKeysChanged() {
	return keysChangedSaved;
}

Common::EventType getKeyEvent(int key) {
	if (getKeysDown() & key) {
		return Common::EVENT_KEYDOWN;
	} else if (getKeysReleased() & key) {
		return Common::EVENT_KEYUP;
	} else {
		return (Common::EventType) 0;
	}
}

void consumeKeys() {
	keysDownSaved = 0;
	keysReleasedSaved = 0;
	keysChangedSaved = 0;
}

bool getPenDown() {
	return penDownSaved;
}

bool getPenHeld() {
	return penHeld;
}

bool getPenReleased() {
	return penReleasedSaved;
}

void consumePenEvents() {
	penDownSaved = false;
	penReleasedSaved = false;
}

int getPenX() {
	int x = ((penX * touchScX) >> 8) + touchX;
	x = x < 0? 0: (x > gameWidth - 1? gameWidth - 1: x);

	if (snapToBorder) {
		if (x < 8) x = 0;
		if (x > gameWidth - 8) x = gameWidth - 1;
	}

	return x;
}

int getPenY() {
	int y = ((penY * touchScY) >> 8) + touchY;
	y = y < 0? 0: (y > gameHeight - 1? gameHeight - 1: y);

	if (snapToBorder) {
		if (y < 8) y = 0;
		if (y > gameHeight - 8) y = gameHeight - 1;
	}

	return y;
}

GLvector getPenPos() {
	GLvector v;

	v.x = (penX * inttof32(1)) / SCREEN_WIDTH;
	v.y = (penY * inttof32(1)) / SCREEN_HEIGHT;

	return v;
}

void setIndyFightState(bool st) {
	indyFightState = st;
	indyFightRight = true;
}

bool getIndyFightState() {
	return indyFightState;
}

///////////////////
// Fast Ram
///////////////////

#define FAST_RAM_SIZE (22500)
#define ITCM_DATA	__attribute__((section(".itcm")))

u8 *fastRamPointer;
u8 fastRamData[FAST_RAM_SIZE] ITCM_DATA;

void *fastRamAlloc(int size) {
	void *result = (void *) fastRamPointer;
	fastRamPointer += size;
	if(fastRamPointer > fastRamData + FAST_RAM_SIZE) {
		consolePrintf("FastRam (ITCM) allocation failed!\n");
		return malloc(size);
	}
	return result;
}

void fastRamReset() {
	fastRamPointer = &fastRamData[0];
}


/////////////////
// GBAMP
/////////////////

bool GBAMPAvail = false;

bool initGBAMP(int mode) {
	if (FAT_InitFiles()) {
		if (mode == 2)	{
			disc_IsInserted();
		}
		GBAMPAvail = true;
//		consolePrintf("Found flash card reader!\n");
		return true;
	} else {
		GBAMPAvail = false;
//		consolePrintf("Flash card reader not found!\n");
		return false;
	}
}

bool isGBAMPAvailable() {
	return GBAMPAvail;
}


#ifdef USE_DEBUGGER
void initDebugger() {
	set_verbosity(VERBOSE_INFO | VERBOSE_ERROR);
	wireless_init(0);
	wireless_connect();

	// This is where the address of the computer running the Java
	// stub goes.
	debugger_connect_tcp(192, 168, 0, 1);
	debugger_init();

	// Update function - should really call every frame
	user_debugger_update();
}


// Ensure the function is processed with C linkage
extern "C" void debug_print_stub(char *string);

void debug_print_stub(char *string) {
	consolePrintf(string);
}
#endif


void powerOff() {
	while (keysHeld() != 0) {		// Wait for all keys to be released.
		swiWaitForVBlank();			// Allow you to read error before the power
	}								// is turned off.

	for (int r = 0; r < 60; r++) {
		swiWaitForVBlank();
	}

	if (ConfMan.hasKey("disablepoweroff", "ds") && ConfMan.getBool("disablepoweroff", "ds")) {
		while (true);
	} else {

		IPC->reset = true;				// Send message to ARM7 to turn power off
		while (true) {
			// Stop the program from continuing beyond this point
		}
	}
}

/////////////////
// Main
/////////////////



void dsExceptionHandler() {
	consolePrintf("Blue screen of death");
	setExceptionHandler(NULL);

	u32	currentMode = getCPSR() & 0x1f;
	u32 thumbState = ((*(u32 *)0x027FFD90) & 0x20);

	u32 codeAddress, exceptionAddress = 0;

	int offset = 8;

	if (currentMode == 0x17) {
		consolePrintf("\x1b[10Cdata abort!\n\n");
		codeAddress = exceptionRegisters[15] - offset;
		if (	(codeAddress > 0x02000000 && codeAddress < 0x02400000) ||
				(codeAddress > (u32)__itcm_start && codeAddress < (u32)(__itcm_start + 32768)) )
			exceptionAddress = getExceptionAddress( codeAddress, thumbState);
		else
			exceptionAddress = codeAddress;

	} else {
		if (thumbState)
			offset = 2;
		else
			offset = 4;
		consolePrintf("\x1b[5Cundefined instruction!\n\n");
		codeAddress = exceptionRegisters[15] - offset;
		exceptionAddress = codeAddress;
	}

	consolePrintf("  pc: %08X addr: %08X\n\n",codeAddress,exceptionAddress);


	int i;
	for (i = 0; i < 8; i++) {
		consolePrintf("  %s: %08X   %s: %08X\n",
					registerNames[i], exceptionRegisters[i],
					registerNames[i+8],exceptionRegisters[i+8]);
	}

	while(1)
		;	// endles loop

	u32 *stack = (u32 *)exceptionRegisters[13];


	for (i = 0; i < 10; i++) {
		consolePrintf("%08X %08X %08X\n", stack[i*3], stack[i*3+1], stack[(i*3)+2] );
	}

	memoryReport();

	while(1);
}




int main(void) {

	soundCallback = NULL;

	initHardware();

	setExceptionHandler(dsExceptionHandler);

#ifdef USE_DEBUGGER
	for (int r = 0; r < 150; r++) {
		swiWaitForVBlank();
	}
	if (!(keysHeld() & KEY_Y)) {
		initDebugger();
	}
#endif


	// Let arm9 read cartridge
	*((u16 *) (0x04000204)) &= ~0x0080;

	lastCallbackFrame = 0;
	tweak = 0;

	indyFightState = false;
	indyFightRight = true;


	// CPU speed = 67108864
	// 8 frames = 2946   368.5 bytes per fr

//	playSound(stretch, 47619, false);
//	playSound(twang, 11010, true);   // 18640

//	bufferSize = 10;


	/*bufferRate = 44100;
	bufferFrame = 0;
	bufferSamples = 8192;

	bufferFirstHalf = false;
	bufferSecondHalf = true;

	int bytes = (2 * (bufferSamples)) + 100;

	soundBuffer = (s16 *) malloc(bytes * 2);


	soundHiPart = true;

	for (int r = 0; r < bytes; r++) {
		soundBuffer[r] = 0;
	}


	swiWaitForVBlank();
	swiWaitForVBlank();
	playSound(soundBuffer, (bufferSamples * 2), true);
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
*/


	lastEventFrame = 0;
	mouseMode = MOUSE_LEFT;


/*
	TIMER1_CR = 0;
	TIMER1_DATA = TIMER_FREQ(bufferRate);
	TIMER1_CR = TIMER_ENABLE | TIMER_DIV_1;

	TIMER2_CR = 0;
	TIMER2_DATA = 0xFFFF - (bufferSamples / 2);
	TIMER2_CR = TIMER_ENABLE | TIMER_IRQ_REQ | TIMER_CASCADE;
	*/
	// 2945 - 2947



//	for (int r = 2946; r < 3000; r++) {
//		soundBuffer[r] = 30000;
//	}


	//2372
	consolePrintf("-------------------------------\n");
	consolePrintf("ScummVM DS\n");
	consolePrintf("Ported by Neil Millstone\n");
	consolePrintf("Version %s ", gScummVMVersion);
#if defined(DS_BUILD_A)
	consolePrintf("build A\n");
	consolePrintf("Lucasarts SCUMM games (SCUMM)\n");
#elif defined(DS_BUILD_B)
	consolePrintf("build B\n");
	consolePrintf("BASS, QUEEN\n");
#elif defined(DS_BUILD_C)
	consolePrintf("build C\n");
	consolePrintf("Simon/Elvira/Waxworks (AGOS)\n");
#elif defined(DS_BUILD_D)
	consolePrintf("build D\n");
	consolePrintf("AGI, CINE, GOB\n");
#elif defined(DS_BUILD_E)
	consolePrintf("build E\n");
	consolePrintf("Inherit the Earth (SAGA)\n");
#elif defined(DS_BUILD_F)
	consolePrintf("build F\n");
	consolePrintf("The Legend of Kyrandia (KYRA)\n");
#elif defined(DS_BUILD_G)
	consolePrintf("build G\n");
	consolePrintf("Lure of the Tempress (LURE)\n");
#elif defined(DS_BUILD_H)
	consolePrintf("build H\n");
	consolePrintf("Nippon Safes (PARALLATION)\n");
#elif defined(DS_BUILD_I)
	consolePrintf("build I\n");
	consolePrintf("Activision Games (MADE)\n");
#elif defined(DS_BUILD_K)
	consolePrintf("build K\n");
	consolePrintf("Cruise for a Corpse (Cruise)\n");
#endif
	consolePrintf("-------------------------------\n");
	consolePrintf("L/R + D-pad/pen:    Scroll view\n");
	consolePrintf("D-pad left:   Left mouse button\n");
	consolePrintf("D-pad right: Right mouse button\n");
	consolePrintf("D-pad up:           Hover mouse\n");
	consolePrintf("B button:        Skip cutscenes\n");
	consolePrintf("Select:         DS Options menu\n");
	consolePrintf("Start:   Game menu (some games)\n");
	consolePrintf("Y (in game):     Toggle console\n");
	consolePrintf("X:              Toggle keyboard\n");
	consolePrintf("A:                 Swap screens\n");
	consolePrintf("L+R (on start):      Clear SRAM\n");


#if defined(DS_BUILD_A)
	consolePrintf("For a complete key list see the\n");
	consolePrintf("help screen.\n\n");
#else
	consolePrintf("\n");
#endif


#ifdef USE_BUILT_IN_DRIVER_SELECTION
	// Do M3 detection selectioon
	int extraData = DSSaveFileManager::getExtraData();
	bool present = DSSaveFileManager::isExtraDataPresent();

	for (int r = 0; r < 30; r++) {
		swiWaitForVBlank();
	}

	int mode = extraData & 0x03;

	if (mode == 0) {
		if ((keysHeld() & KEY_L) && !(keysHeld() & KEY_R)) {
			mode = 1;
		} else if (!(keysHeld() & KEY_L) && (keysHeld() & KEY_R)) {
			mode = 2;
		}
	} else {
		if ((keysHeld() & KEY_L) && !(keysHeld() & KEY_R)) {
			mode = 0;
		}
	}


	if (mode == 0) {
		consolePrintf("On startup hold L if you have\n");
		consolePrintf("an M3 SD or R for an SC SD\n");
	} else if (mode == 1) {
		consolePrintf("Using M3 SD Mode.\n");
		consolePrintf("Hold L on startup to disable.\n");
	} else if (mode == 2) {
		consolePrintf("Using SC SD Mode.\n");
		consolePrintf("Hold L on startup to disable.\n");
	}

	disc_setEnable(mode);
	DSSaveFileManager::setExtraData(mode);
#else

	int mode = 0;

#endif


/*
	if ((present) && (extraData & 0x00000001)) {

		if (keysHeld() & KEY_L) {
			extraData &= ~0x00000001;
			consolePrintf("M3 SD Detection: OFF\n");
			DSSaveFileManager::setExtraData(extraData);
		} else {
			consolePrintf("M3 SD Detection: ON\n");
			consolePrintf("Hold L on startup to disable.\n");
		}

	} else if (keysHeld() & KEY_L) {
		consolePrintf("M3 SD Detection: ON\n");
		extraData |= 0x00000001;
		DSSaveFileManager::setExtraData(extraData);
	} else {
		consolePrintf("M3 SD Detection: OFF\n");
		consolePrintf("Hold L on startup to enable.\n");
	}

	disc_setM3SDEnable(extraData & 0x00000001);
*/
	// Create a file system node to force search for a zip file in GBA rom space

	DSFileSystemNode *node = new DSFileSystemNode();
	if (!node->getZip() || (!node->getZip()->isReady())) {
		// If not found, init CF/SD driver
		initGBAMP(mode);

		if (!initGBAMP(mode)) {
			consolePrintf("\nNo file system was found.\n");
			consolePrintf("View the readme file\n");
			consolePrintf("for more information.\n");

			while (1);
		}
	}
	delete node;



	updateStatus();


//	OSystem_DS::instance();

	g_system = new OSystem_DS();
	assert(g_system);

	IPC->adpcm.semaphore = false;

//	printf("'%s'", Common::ConfigManager::kTransientDomain.c_str());
	//printf("'%s'", Common::ConfigManager::kApplicationDomain.c_str());

#if defined(DS_BUILD_A)
	const char *argv[] = {"/scummvmds"};
#elif defined(DS_BUILD_B)
	const char *argv[] = {"/scummvmds", "--config=scummvmb.ini"};
#elif defined(DS_BUILD_C)
	const char *argv[] = {"/scummvmds", "--config=scummvmc.ini"};
#elif defined(DS_BUILD_D)
	const char *argv[] = {"/scummvmds", "--config=scummvmd.ini"};
#elif defined(DS_BUILD_E)
	const char *argv[] = {"/scummvmds", "--config=scummvme.ini"};
#elif defined(DS_BUILD_F)
	const char *argv[] = {"/scummvmds", "--config=scummvmf.ini"};
#elif defined(DS_BUILD_G)
	const char *argv[] = {"/scummvmds", "--config=scummvmg.ini"};
#elif defined(DS_BUILD_H)
	const char *argv[] = {"/scummvmds", "--config=scummvmh.ini"};
#elif defined(DS_BUILD_I)
	const char *argv[] = {"/scummvmds", "--config=scummvmi.ini"};
#elif defined(DS_BUILD_J)
	const char *argv[] = {"/scummvmds", "--config=scummvmj.ini"};
#elif defined(DS_BUILD_K)
	const char *argv[] = {"/scummvmds", "--config=scummvmk.ini"};
#else
	// Use the default config file if no build was specified. This currently
	// only happens with builds made using the regular ScummVM build system (as
	// opposed to the nds specific build system).
	const char *argv[] = {"/scummvmds"};
#endif

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new DSPluginProvider());
#endif

	while (1) {
		scummvm_main(ARRAYSIZE(argv), (char **) &argv);
		powerOff();
	}


	return 0;
}

}	// End of namespace DS


int main() {
#ifndef DISABLE_TEXT_CONSOLE
	consoleDebugInit(DebugDevice_NOCASH);
	nocashMessage("startup\n");
#endif
	DS::main();
}


#ifdef USE_PROFILER
int cygprofile_getHBlanks() __attribute__ ((no_instrument_function));


int cygprofile_getHBlanks() {
	return DS::hBlankCount;
}


extern "C" void consolePrintf(char * format, ...) __attribute__ ((no_instrument_function));
#endif


extern "C" void consolePrintf(const char * format, ...) {
	va_list args;
	va_start(args, format);
	viprintf(format, args);
	va_end(args);
}
