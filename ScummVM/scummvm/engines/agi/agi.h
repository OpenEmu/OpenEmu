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

#ifndef AGI_H
#define AGI_H

#include "common/scummsys.h"
#include "common/error.h"
#include "common/util.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/rendermode.h"
#include "common/stack.h"
#include "common/system.h"

#include "engines/engine.h"

#include "gui/debugger.h"

// AGI resources
#include "agi/console.h"
#include "agi/view.h"
#include "agi/picture.h"
#include "agi/logic.h"
#include "agi/sound.h"

#include "gui/predictivedialog.h"

namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the AGI engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Early Sierra adventure games
 * - many fan made games
 * - Mickey's Space Adventure (Pre-AGI)
 * - Winnie the Pooh in the Hundred Acre Wood (Pre-AGI)
 * - Troll's Tale (Pre-AGI)
 */
namespace Agi {

typedef signed int Err;

//
// Version and other definitions
//

#define	TITLE		"AGI engine"

#define DIR_		"dir"
#define LOGDIR		"logdir"
#define PICDIR		"picdir"
#define VIEWDIR		"viewdir"
#define	SNDDIR		"snddir"
#define OBJECTS		"object"
#define WORDS		"words.tok"

#define	MAX_DIRS	256
#define	MAX_VARS	256
#define	MAX_FLAGS	(256 >> 3)
#define MAX_VIEWTABLE	255	// KQ3 uses o255!
#define MAX_WORDS	20
#define	MAX_STRINGS	24		// MAX_STRINGS + 1 used for get.num
#define MAX_STRINGLEN	40
#define MAX_CONTROLLERS 39

#define	_EMPTY		0xfffff
#define	EGO_OWNED	0xff
#define	EGO_OWNED_V1	0xf9

#define	CRYPT_KEY_SIERRA	"Avis Durgan"
#define CRYPT_KEY_AGDS		"Alex Simkin"

#define	MSG_BOX_COLOR	0x0f	// White
#define MSG_BOX_TEXT	0x00	// Black
#define MSG_BOX_LINE	0x04	// Red
#define BUTTON_BORDER	0x00	// Black
#define STATUS_FG	0x00		// Black
#define	STATUS_BG	0x0f		// White

#define ADD_PIC 1
#define ADD_VIEW 2

#define CMD_BSIZE 12

enum AgiGameID {
	GID_AGIDEMO,
	GID_BC,
	GID_DDP,
	GID_GOLDRUSH,
	GID_KQ1,
	GID_KQ2,
	GID_KQ3,
	GID_KQ4,
	GID_LSL1,
	GID_MH1,
	GID_MH2,
	GID_MIXEDUP,
	GID_PQ1,
	GID_SQ1,
	GID_SQ2,
	GID_XMASCARD,
	GID_FANMADE,
	GID_GETOUTTASQ,	// Fanmade
	GID_MICKEY,			// PreAGI
	GID_WINNIE,			// PreAGI
	GID_TROLL				// PreAGI
};

enum AgiGameType {
	GType_PreAGI = 0,
	GType_V1 = 1,
	GType_V2 = 2,
	GType_V3 = 3
};

 enum BooterDisks {
	 BooterDisk1 = 0,
	 BooterDisk2 = 1
 };

//
// GF_OLDAMIGAV20 means that the interpreter is an old Amiga AGI interpreter that
// uses value 20 for the computer type (v20 i.e. vComputer) rather than the usual value 5.
//
// GF_CLIPCOORDS means that views' coordinates must be clipped at least in commands
// position and position.v.
//
enum AgiGameFeatures {
	GF_AGIMOUSE =    (1 << 0),
	GF_AGDS =        (1 << 1),
	GF_AGI256 =      (1 << 2),
	GF_AGI256_2 =    (1 << 3),
	GF_AGIPAL =      (1 << 4),
	GF_MACGOLDRUSH = (1 << 5),
	GF_FANMADE =     (1 << 6),
	GF_MENUS =		 (1 << 7),
	GF_ESCPAUSE =	 (1 << 8),
	GF_OLDAMIGAV20 = (1 << 9),
	GF_CLIPCOORDS  = (1 << 10),
	GF_2GSOLDSOUND = (1 << 11)
};

struct AGIGameDescription;

enum {
	NO_GAMEDIR = 0,
	GAMEDIR
};

enum AGIErrors {
	errOK = 0,
	errDoNothing,
	errBadCLISwitch,
	errInvalidAGIFile,
	errBadFileOpen,
	errNotEnoughMemory,
	errBadResource,
	errUnknownAGIVersion,
	errNoLoopsInView,
	errViewDataError,
	errNoGameList,
	errIOError,

	errUnk = 127
};

enum kDebugLevels {
	kDebugLevelMain =      1 << 0,
	kDebugLevelResources = 1 << 1,
	kDebugLevelSprites =   1 << 2,
	kDebugLevelInventory = 1 << 3,
	kDebugLevelInput =     1 << 4,
	kDebugLevelMenu =      1 << 5,
	kDebugLevelScripts =   1 << 6,
	kDebugLevelSound =     1 << 7,
	kDebugLevelText =      1 << 8,
	kDebugLevelSavegame =  1 << 9
};

/**
 * AGI resources.
 */
enum {
	rLOGIC = 1,
	rSOUND,
	rVIEW,
	rPICTURE
};

enum {
	RES_LOADED = 1,
	RES_COMPRESSED = 0x40
};

enum {
	lCOMMAND_MODE = 1,
	lTEST_MODE
};

struct gameIdList {
	gameIdList *next;
	uint32 version;
	uint32 crc;
	char *gName;
	char *switches;
};

struct Mouse {
	int button;
	int x;
	int y;
};

// Used by AGI Mouse protocol 1.0 for v27 (i.e. button pressed -variable).
enum AgiMouseButton {
	kAgiMouseButtonUp,    // Mouse button is up (not pressed)
	kAgiMouseButtonLeft,  // Left mouse button
	kAgiMouseButtonRight, // Right mouse button
	kAgiMouseButtonMiddle // Middle mouse button
};

enum GameId {
	GID_AGI = 1
};

#define WIN_TO_PIC_X(x) ((x) / 2)
#define WIN_TO_PIC_Y(y) ((y) < 8 ? 999 : (y) >= (8 + _HEIGHT) ? 999 : (y) - 8)

/**
 * AGI variables.
 */
enum {
	vCurRoom = 0,		// 0
	vPrevRoom,
	vBorderTouchEgo,
	vScore,
	vBorderCode,
	vBorderTouchObj,	// 5
	vEgoDir,
	vMaxScore,
	vFreePages,
	vWordNotFound,
	vTimeDelay,		// 10
	vSeconds,
	vMinutes,
	vHours,
	vDays,
	vJoystickSensitivity,	// 15
	vEgoViewResource,
	vAgiErrCode,
	vAgiErrCodeInfo,
	vKey,
	vComputer,		// 20
	vWindowReset,
	vSoundgen,
	vVolume,
	vMaxInputChars,
	vSelItem,		// 25
	vMonitor
};

/**
 * Different monitor types.
 * Used with AGI variable 26 i.e. vMonitor.
 */
enum AgiMonitorType {
	kAgiMonitorCga = 0,
	// kAgiMonitorTandy = 1, // Not sure about this
	kAgiMonitorHercules = 2,
	kAgiMonitorEga = 3
	// kAgiMonitorVga = 4 // Not sure about this
};

/**
 * Different computer types.
 * Used with AGI variable 20 i.e. vComputer.
 *
 * At least these Amiga AGI versions use value 5:
 * 2.082 (King's Quest I v1.0U 1986)
 * 2.090 (King's Quest III v1.01 1986-11-08)
 * x.yyy (Black Cauldron v2.00 1987-06-14)
 * x.yyy (Larry I v1.05 1987-06-26)
 * 2.107 (King's Quest II v2.0J. Date is probably 1987-01-29)
 * 2.202 (Space Quest II v2.0F)
 * 2.310 (Police Quest I v2.0B 1989-02-22)
 * 2.316 (Gold Rush! v2.05 1989-03-09)
 * 2.333 (King's Quest III v2.15 1989-11-15)
 *
 * At least these Amiga AGI versions use value 20:
 * 2.082 (Space Quest I v1.2 1986)
 * x.yyy (Manhunter NY 1.06 3/18/89)
 * 2.333 (Manhunter SF 3.06 8/17/89)
 *
 */
enum AgiComputerType {
	kAgiComputerPC = 0,
	kAgiComputerAtariST = 4,
	kAgiComputerAmiga = 5, // Newer Amiga AGI interpreters' value (Commonly used)
	kAgiComputerApple2GS = 7,
	kAgiComputerAmigaOld = 20 // Older Amiga AGI interpreters' value (Seldom used)
};

enum AgiSoundType {
	kAgiSoundPC = 1,
	kAgiSoundTandy = 3, // Tandy (This value is also used by the Amiga AGI and Apple IIGS AGI)
	kAgiSound2GSOld = 8 // Apple IIGS's Gold Rush! (Version 1.0M 1989-02-28 (CE), AGI 3.003) uses value 8
};

/**
 * AGI flags
 */
enum {
	fEgoWater = 0,	// 0
	fEgoInvisible,
	fEnteredCli,
	fEgoTouchedP2,
	fSaidAcceptedInput,
	fNewRoomExec,	// 5
	fRestartGame,
	fScriptBlocked,
	fJoySensitivity,
	fSoundOn,
	fDebuggerOn,		// 10
	fLogicZeroFirsttime,
	fRestoreJustRan,
	fStatusSelectsItems,
	fMenusWork,
	fOutputMode,		// 15
	fAutoRestart
};

enum AgiSlowliness {
	kPauseRoom = 1500,
	kPausePicture = 500
};

struct AgiController {
	uint16 keycode;
	uint8 controller;
};

struct AgiObject {
	int location;
	char *name;
};

struct AgiWord {
	int id;
	char *word;
};

struct AgiDir {
	uint8 volume;
	uint32 offset;
	uint32 len;
	uint32 clen;

	// 0 = not in mem, can be freed
	// 1 = in mem, can be released
	// 2 = not in mem, cant be released
	// 3 = in mem, cant be released
	// 0x40 = was compressed
	uint8 flags;
};

struct AgiBlock {
	int active;
	int x1, y1;
	int x2, y2;
	uint8 *buffer;		// used for window background
};

/** AGI text color (Background and foreground color). */
struct AgiTextColor {
	/** Creates an AGI text color. Uses black text on white background by default. */
	AgiTextColor(int fgColor = 0x00, int bgColor = 0x0F) : fg(fgColor), bg(bgColor) {}

	/** Get an AGI text color with swapped foreground and background color. */
	AgiTextColor swap() const { return AgiTextColor(bg, fg); }

	int fg; ///< Foreground color (Used for text).
	int bg; ///< Background color (Used for text's background).
};

/**
 * AGI button style (Amiga or PC).
 *
 * Supports positive and negative button types (Used with Amiga-style only):
 * Positive buttons do what the dialog was opened for.
 * Negative buttons cancel what the dialog was opened for.
 * Restart-dialog example: Restart-button is positive, Cancel-button negative.
 * Paused-dialog example: Continue-button is positive.
 */
struct AgiButtonStyle {
// Public constants etc
public:
	static const int
		// Amiga colors (Indexes into the Amiga-ish palette)
		amigaBlack  = 0x00, ///< Accurate,                   is          #000000 (24-bit RGB)
		amigaWhite  = 0x0F, ///< Practically accurate,       is close to #FFFFFF (24-bit RGB)
		amigaGreen  = 0x02, ///< Quite accurate,             should be   #008A00 (24-bit RGB)
		amigaOrange = 0x0C, ///< Inaccurate, too much blue,  should be   #FF7500 (24-bit RGB)
		amigaPurple = 0x0D, ///< Inaccurate, too much green, should be   #FF00FF (24-bit RGB)
		amigaRed    = 0x04, ///< Quite accurate,             should be   #BD0000 (24-bit RGB)
		amigaCyan   = 0x0B, ///< Inaccurate, too much red,   should be   #00FFDE (24-bit RGB)
		// PC colors (Indexes into the EGA-palette)
		pcBlack     = 0x00,
		pcWhite     = 0x0F;

// Public methods
public:
	/**
	 * Get the color of the button with the given state and type using current style.
	 *
	 * @param hasFocus True if button has focus, false otherwise.
	 * @param pressed True if button is being pressed, false otherwise.
	 * @param positive True if button is positive, false if button is negative. Only matters for Amiga-style buttons.
	 */
	AgiTextColor getColor(bool hasFocus, bool pressed, bool positive = true) const;

	/**
	 * Get the color of a button with the given base color and state ignoring current style.
	 * Swaps foreground and background color when the button has focus or is being pressed.
	 *
	 * @param hasFocus True if button has focus, false otherwise.
	 * @param pressed True if button is being pressed, false otherwise.
	 * @param baseFgColor Foreground color of the button when it has no focus and is not being pressed.
	 * @param baseBgColor Background color of the button when it has no focus and is not being pressed.
	 */
	AgiTextColor getColor(bool hasFocus, bool pressed, int baseFgColor, int baseBgColor) const;

	/**
	 * Get the color of a button with the given base color and state ignoring current style.
	 * Swaps foreground and background color when the button has focus or is being pressed.
	 *
	 * @param hasFocus True if button has focus, false otherwise.
	 * @param pressed True if button is being pressed, false otherwise.
	 * @param baseColor Color of the button when it has no focus and is not being pressed.
	 */
	AgiTextColor getColor(bool hasFocus, bool pressed, const AgiTextColor &baseColor) const;

	/**
	 * How many pixels to offset the shown text diagonally down and to the right.
	 * Currently only used for pressed PC-style buttons.
	 */
	int getTextOffset(bool hasFocus, bool pressed) const;

	/**
	 * Show border around the button?
	 * Currently border is only used for in focus or pressed Amiga-style buttons
	 * when in inauthentic Amiga-style mode.
	 */
	bool getBorder(bool hasFocus, bool pressed) const;

	/**
	 * Set Amiga-button style.
	 *
	 * @param amigaStyle Set Amiga-button style if true, otherwise set PC-button style.
	 * @param olderAgi If true then use older AGI style in Amiga-mode, otherwise use newer.
	 * @param authenticAmiga If true then don't use a border around buttons in Amiga-mode, otherwise use.
	 */
	void setAmigaStyle(bool amigaStyle = true, bool olderAgi = false, bool authenticAmiga = false);

	/**
	 * Set PC-button style.
	 * @param pcStyle Set PC-button style if true, otherwise set default Amiga-button style.
	 */
	void setPcStyle(bool pcStyle = true);

// Public constructors
public:
	/**
	 * Create a button style based on the given rendering mode.
	 * @param renderMode If Common::kRenderAmiga then creates default Amiga-button style, otherwise PC-style.
	 */
	AgiButtonStyle(Common::RenderMode renderMode = Common::kRenderDefault);

// Private member variables
private:
	bool _amigaStyle;     ///< Use Amiga-style buttons if true, otherwise use PC-style buttons.
	bool _olderAgi;       ///< Use older AGI style in Amiga-style mode.
	bool _authenticAmiga; ///< Don't use border around buttons in Amiga-style mode.
};

struct ScriptPos {
	int script;
	int curIP;
};

enum {
	EGO_VIEW_TABLE	= 0,
	HORIZON			= 36,
	_WIDTH			= 160,
	_HEIGHT			= 168
};

enum InputMode {
	INPUT_NORMAL	= 0x01,
	INPUT_GETSTRING	= 0x02,
	INPUT_MENU		= 0x03,
	INPUT_NONE		= 0x04
};

enum State {
	STATE_INIT		= 0x00,
	STATE_LOADED	= 0x01,
	STATE_RUNNING	= 0x02
};

enum {
	SBUF16_OFFSET = 0,
	SBUF256_OFFSET = ((_WIDTH) * (_HEIGHT)),
	FROM_SBUF16_TO_SBUF256_OFFSET = ((SBUF256_OFFSET) - (SBUF16_OFFSET)),
	FROM_SBUF256_TO_SBUF16_OFFSET = ((SBUF16_OFFSET) - (SBUF256_OFFSET))
};

/**
 * AGI game structure.
 * This structure contains all global data of an AGI game executed
 * by the interpreter.
 */
struct AgiGame {
	AgiEngine *_vm;

	State state;		/**< state of the interpreter */

	// TODO: Check whether adjMouseX and adjMouseY must be saved and loaded when using savegames.
	//       If they must be then loading and saving is partially broken at the moment.
	int adjMouseX;	/**< last given adj.ego.move.to.x.y-command's 1st parameter */
	int adjMouseY;	/**< last given adj.ego.move.to.x.y-command's 2nd parameter */

	char name[8];	/**< lead in id (e.g. `GR' for goldrush) */
	char id[8];		/**< game id */
	uint32 crc;		/**< game CRC */

	// game flags and variables
	uint8 flags[MAX_FLAGS]; /**< 256 1-bit flags */
	uint8 vars[MAX_VARS];   /**< 256 variables */

	// internal variables
	int horizon;			/**< horizon y coordinate */
	int lineStatus;		/**< line number to put status on */
	int lineUserInput;	/**< line to put user input on */
	int lineMinPrint;		/**< num lines to print on */
	int cursorPos;			/**< column where the input cursor is */
	byte inputBuffer[40]; /**< buffer for user input */
	byte echoBuffer[40];	/**< buffer for echo.line */
	int keypress;

	InputMode inputMode;			/**< keyboard input mode */
	bool inputEnabled;		/**< keyboard input enabled */
	int lognum;				/**< current logic number */
	Common::Array<ScriptPos> execStack;

	// internal flags
	int playerControl;		/**< player is in control */
	int statusLine;		/**< status line on/off */
	int clockEnabled;		/**< clock is on/off */
	int exitAllLogics;	/**< break cycle after new.room */
	int pictureShown;		/**< show.pic has been issued */
	int hasPrompt;			/**< input prompt has been printed */
#define ID_AGDS		0x00000001
#define ID_AMIGA	0x00000002
	int gameFlags;			/**< agi options flags */

	uint8 priTable[_HEIGHT];/**< priority table */

	// windows
	uint32 msgBoxTicks;	/**< timed message box tick counter */
	AgiBlock block;
	AgiBlock window;
	int hasWindow;

	// graphics & text
	int gfxMode;
	char cursorChar;
	unsigned int colorFg;
	unsigned int colorBg;

	uint8 *sbufOrig;		/**< Pointer to the 160x336 AGI screen buffer that contains vertically two 160x168 screens (16 color and 256 color). */
	uint8 *sbuf16c;			/**< 160x168 16 color (+control line & priority information) AGI screen buffer. Points at sbufOrig + SBUF16_OFFSET. */
	uint8 *sbuf256c;		/**< 160x168 256 color AGI screen buffer (For AGI256 and AGI256-2 support). Points at sbufOrig + SBUF256_OFFSET. */
	uint8 *sbuf;			/**< Currently chosen AGI screen buffer (sbuf256c if AGI256 or AGI256-2 is used, otherwise sbuf16c). */

	// player command line
	AgiWord egoWords[MAX_WORDS];
	int numEgoWords;

	unsigned int numObjects;

	bool controllerOccured[MAX_DIRS];  /**< keyboard keypress events */
	AgiController controllers[MAX_CONTROLLERS];

	char strings[MAX_STRINGS + 1][MAX_STRINGLEN]; /**< strings */

	// directory entries for resources
	AgiDir dirLogic[MAX_DIRS];
	AgiDir dirPic[MAX_DIRS];
	AgiDir dirView[MAX_DIRS];
	AgiDir dirSound[MAX_DIRS];

	// resources
	AgiPicture pictures[MAX_DIRS];	/**< AGI picture resources */
	AgiLogic logics[MAX_DIRS];		/**< AGI logic resources */
	AgiView views[MAX_DIRS];		/**< AGI view resources */
	AgiSound *sounds[MAX_DIRS];		/**< Pointers to AGI sound resources */

	AgiLogic *_curLogic;

	// words
	Common::Array<AgiWord *> words[26];

	// view table
	VtEntry viewTable[MAX_VIEWTABLE];

	int32 ver;						/**< detected game version */

	int simpleSave;					/**< select simple savegames */

	Common::Rect mouseFence;		/**< rectangle set by fence.mouse command */

	// IF condition handling
	int testResult;


	int max_logics;
	int logic_list[256];
};

class AgiLoader {
public:

	AgiLoader() {}
	virtual ~AgiLoader() {}

	virtual int init() = 0;
	virtual int deinit() = 0;
	virtual int detectGame() = 0;
	virtual int loadResource(int, int) = 0;
	virtual int unloadResource(int, int) = 0;
	virtual int loadObjects(const char *) = 0;
	virtual int loadWords(const char *) = 0;
};

class AgiLoader_v1 : public AgiLoader {
private:
	AgiEngine *_vm;
	Common::String _filenameDisk0;
	Common::String _filenameDisk1;

	int loadDir_DDP(AgiDir *agid, int offset, int max);
	int loadDir_BC(AgiDir *agid, int offset, int max);
	uint8 *loadVolRes(AgiDir *agid);

public:
	AgiLoader_v1(AgiEngine *vm);

	virtual int init();
	virtual int deinit();
	virtual int detectGame();
	virtual int loadResource(int, int);
	virtual int unloadResource(int, int);
	virtual int loadObjects(const char *);
	virtual int loadWords(const char *);
};

class AgiLoader_v2 : public AgiLoader {
private:
	AgiEngine *_vm;

	int loadDir(AgiDir *agid, const char *fname);
	uint8 *loadVolRes(AgiDir *agid);

public:

	AgiLoader_v2(AgiEngine *vm) {
		_vm = vm;
	}

	virtual int init();
	virtual int deinit();
	virtual int detectGame();
	virtual int loadResource(int, int);
	virtual int unloadResource(int, int);
	virtual int loadObjects(const char *);
	virtual int loadWords(const char *);
};

class AgiLoader_v3 : public AgiLoader {
private:
	AgiEngine *_vm;

	int loadDir(AgiDir *agid, Common::File *fp, uint32 offs, uint32 len);
	uint8 *loadVolRes(AgiDir *agid);

public:

	AgiLoader_v3(AgiEngine *vm) {
		_vm = vm;
	}

	virtual int init();
	virtual int deinit();
	virtual int detectGame();
	virtual int loadResource(int, int);
	virtual int unloadResource(int, int);
	virtual int loadObjects(const char *);
	virtual int loadWords(const char *);
};


class GfxMgr;
class SpritesMgr;
class Menu;

// Image stack support
struct ImageStackElement {
	uint8 type;
	uint8 pad;
	int16 parm1;
	int16 parm2;
	int16 parm3;
	int16 parm4;
	int16 parm5;
	int16 parm6;
	int16 parm7;
};

struct StringData {
	int x;
	int y;
	int len;
	int str;
};

#define TICK_SECONDS 20

#define KEY_QUEUE_SIZE 16

class AgiBase : public ::Engine {
protected:
	// Engine API
	Common::Error init();
	virtual Common::Error go() = 0;
	virtual Common::Error run() {
		Common::Error err;
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}
	virtual bool hasFeature(EngineFeature f) const;

	virtual void initialize() = 0;

	void initRenderMode();

public:
	GfxMgr *_gfx;

	AgiButtonStyle _defaultButtonStyle;
	AgiButtonStyle _buttonStyle;
	Common::RenderMode _renderMode;
	volatile uint32 _clockCount;
	AgiDebug _debug;
	AgiGame _game;
	Common::RandomSource *_rnd;

	SoundMgr *_sound;

	Mouse _mouse;

	bool _noSaveLoadAllowed;

	virtual void pollTimer() = 0;
	virtual int getKeypress() = 0;
	virtual bool isKeypress() = 0;
	virtual void clearKeyQueue() = 0;

	AgiBase(OSystem *syst, const AGIGameDescription *gameDesc);
	~AgiBase();

	virtual void clearImageStack() = 0;
	virtual void recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) = 0;
	virtual void replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) = 0;
	virtual void releaseImageStack() = 0;

	int _soundemu;

	int getflag(int);
	void setflag(int, int);
	void flipflag(int);

	const AGIGameDescription *_gameDescription;

	uint32 _gameFeatures;
	uint16 _gameVersion;

	uint32 getGameID() const;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	uint16 getGameType() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	const char *getGameMD5() const;
	void initFeatures();
	void setFeature(uint32 feature);
	void initVersion();
	void setVersion(uint16 version);

	const char *getDiskName(uint16 id);

	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
};

typedef void (*AgiCommand)(AgiGame *state, uint8 *p);

class AgiEngine : public AgiBase {
protected:
	// Engine APIs
	virtual Common::Error go();

	void initialize();

	uint32 _lastSaveTime;

public:
	AgiEngine(OSystem *syst, const AGIGameDescription *gameDesc);
	virtual ~AgiEngine();

	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);

private:
	uint32 _lastTick;

	int _keyQueue[KEY_QUEUE_SIZE];
	int _keyQueueStart;
	int _keyQueueEnd;

	bool _allowSynthetic;

	int checkPriority(VtEntry *v);
	int checkCollision(VtEntry *v);
	int checkPosition(VtEntry *v);

	void parseFeatures();

	int _firstSlot;

public:
	AgiObject *_objects;	// objects in the game

	StringData _stringdata;

	Common::String getSavegameFilename(int num) const;
	void getSavegameDescription(int num, char *buf, bool showEmpty = true);
	int selectSlot();
	int saveGame(const Common::String &fileName, const Common::String &saveName);
	int loadGame(const Common::String &fileName, bool checkId = true);
	int saveGameDialog();
	int saveGameSimple();
	int loadGameDialog();
	int loadGameSimple();
	int doSave(int slot, const Common::String &desc);
	int doLoad(int slot, bool showMessages);
	int scummVMSaveLoadDialog(bool isSave);

	uint8 *_intobj;
	InputMode _oldMode;
	bool _restartGame;

	Menu* _menu;
	bool _menuSelected;

	char _lastSentence[40];

	SpritesMgr *_sprites;
	PictureMgr *_picture;
	AgiLoader *_loader;	// loader

	Common::Stack<ImageStackElement> _imageStack;

	void clearImageStack();
	void recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7);
	void replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7);
	void releaseImageStack();

	void pause(uint32 msec);

	Console *_console;
	GUI::Debugger *getDebugger() { return _console; }

	int agiInit();
	int agiDeinit();
	int agiDetectGame();
	int agiLoadResource(int, int);
	int agiUnloadResource(int, int);
	void agiUnloadResources();

	virtual void pollTimer();
	virtual int getKeypress();
	virtual bool isKeypress();
	virtual void clearKeyQueue();

	void initPriTable();

	void newInputMode(InputMode mode);
	void oldInputMode();

	int getvar(int);
	void setvar(int, int);
	void decrypt(uint8 *mem, int len);
	void releaseSprites();
	int mainCycle();
	int viewPictures();
	int runGame();
	void inventory();
	void updateTimer();
	int getAppDir(char *appDir, unsigned int size);

	int setupV2Game(int ver);
	int setupV3Game(int ver);

	void newRoom(int n);
	void resetControllers();
	void interpretCycle();
	int playGame();

	void printItem(int n, int fg, int bg);
	int findItem();
	int showItems();
	void selectItems(int n);

	void allowSynthetic(bool);
	void processEvents();
	void checkQuickLoad();

	// Objects
public:
	int showObjects();
	int loadObjects(const char *fname);
	int loadObjects(Common::File &fp);
	void unloadObjects();
	const char *objectName(unsigned int);
	int objectGetLocation(unsigned int);
	void objectSetLocation(unsigned int, int);
private:
	int decodeObjects(uint8 *mem, uint32 flen);
	int readObjects(Common::File &fp, int flen);
	int allocObjects(int);

	// Logic
public:
	int decodeLogic(int);
	void unloadLogic(int);
	int runLogic(int);
	void debugConsole(int, int, const char *);
	int testIfCode(int);
	void executeAgiCommand(uint8, uint8 *);

public:
	// Some submethods of testIfCode
	void skipInstruction(byte op);
	void skipInstructionsUntil(byte v);
	uint8 testObjRight(uint8, uint8, uint8, uint8, uint8);
	uint8 testObjCenter(uint8, uint8, uint8, uint8, uint8);
	uint8 testObjInBox(uint8, uint8, uint8, uint8, uint8);
	uint8 testPosn(uint8, uint8, uint8, uint8, uint8);
	uint8 testSaid(uint8, uint8 *);
	uint8 testController(uint8);
	uint8 testKeypressed();
	uint8 testCompareStrings(uint8, uint8);

	// View
private:

	void lSetCel(VtEntry *v, int n);
	void lSetLoop(VtEntry *v, int n);
	void updateView(VtEntry *v);

public:

	void setCel(VtEntry *, int);
	void clipViewCoordinates(VtEntry *v);
	void setLoop(VtEntry *, int);
	void setView(VtEntry *, int);
	void startUpdate(VtEntry *);
	void stopUpdate(VtEntry *);
	void updateViewtable();
	void unloadView(int);
	int decodeView(int);
	void addToPic(int, int, int, int, int, int, int);
	void drawObj(int);
	bool isEgoView(const VtEntry *v);

	// Words
public:
	int showWords();
	int loadWords(const char *);
	int loadWords_v1(Common::File &f);
	void unloadWords();
	int findWord(const char *word, int *flen);
	void dictionaryWords(char *);

	// Motion
private:
	int checkStep(int delta, int step);
	int checkBlock(int x, int y);
	void changePos(VtEntry *v);
	void motionWander(VtEntry *v);
	void motionFollowEgo(VtEntry *v);
	void motionMoveObj(VtEntry *v);
	void checkMotion(VtEntry *v);

public:
	void checkAllMotions();
	void moveObj(VtEntry *);
	void inDestination(VtEntry *);
	void fixPosition(int);
	void updatePosition();
	int getDirection(int x0, int y0, int x, int y, int s);

	bool _egoHoldKey;

	// Keyboard
	void initWords();
	void cleanInput();
	int doPollKeyboard();
	void cleanKeyboard();
	void handleKeys(int);
	void handleGetstring(int);
	int handleController(int);
	void getString(int, int, int, int);
	uint16 agiGetKeypress();
	int waitKey();
	int waitAnyKey();

	// Text
public:
	int messageBox(const char *);
	int selectionBox(const char *, const char **);
	void closeWindow();
	void drawWindow(int, int, int, int);
	void printText(const char *, int, int, int, int, int, int, bool checkerboard = false);
	void printTextConsole(const char *, int, int, int, int, int);
	int print(const char *, int, int, int);
	char *wordWrapString(const char *, int *);
	char *agiSprintf(const char *);
	void writeStatus();
	void writePrompt();
	void clearPrompt(bool useBlackBg = false);
	void clearLines(int, int, int);
	void flushLines(int, int);

private:
	void printStatus(const char *message, ...) GCC_PRINTF(2, 3);
	void printText2(int l, const char *msg, int foff, int xoff, int yoff, int len, int fg, int bg, bool checkerboard = false);
	void blitTextbox(const char *p, int y, int x, int len);
	void eraseTextbox();
	bool matchWord();

public:
	char _predictiveResult[40];

private:
	AgiCommand _agiCommands[183];
	AgiCommand _agiCondCommands[256];

	void setupOpcodes();

public:
	int _timerHack;			// Workaround for timer loop in MH1 logic 153
};

} // End of namespace Agi

#endif /* AGI_H */
