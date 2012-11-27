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

#ifndef AGOS_AGOS_H
#define AGOS_AGOS_H

#include "engines/engine.h"

#include "common/archive.h"
#include "common/array.h"
#include "common/error.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/stack.h"
#include "common/util.h"

#include "agos/sound.h"
#include "agos/vga.h"

/**
 * This is the namespace of the AGOS engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Elvira: Mistress of the Dark
 * - Elvira 2: The Jaws of Cerberus
 * - The Feeble Files
 * - Simon the Sorcerer
 * - Simon the Sorcerer 2
 * - Simon the Sorcerer Puzzle Pack
 */

namespace Common {
class File;
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace AGOS {

uint fileReadItemID(Common::SeekableReadStream *in);

#define CHECK_BOUNDS(x, y) assert((uint)(x) < ARRAYSIZE(y))

#ifdef ENABLE_AGOS2
class MoviePlayer;
#endif

class MidiPlayer;

struct Child;
struct SubObject;

struct Item;
struct WindowBlock;
struct Subroutine;
struct SubroutineLine;
struct TimeEvent;

struct TextLocation {
	int16 x, y, width;
	TextLocation() { memset(this, 0, sizeof(*this)); }
};

struct HitArea {
	uint16 x, y;
	uint16 width, height;
	uint16 flags;
	uint16 id;
	uint16 data;
	WindowBlock *window;
	Item *itemPtr;
	uint16 verb;
	uint16 priority;

	// Personal Nightmare specific
	uint16 msg1, msg2;
	HitArea() { memset(this, 0, sizeof(*this)); }
};

struct VgaPointersEntry {
	byte *vgaFile1;
	byte *vgaFile1End;
	byte *vgaFile2;
	byte *vgaFile2End;
	byte *sfxFile;
	byte *sfxFileEnd;
	VgaPointersEntry() { memset(this, 0, sizeof(*this)); }
};

struct VgaSprite {
	uint16 id;
	int16 image;
	uint16 palette;
	int16 x, y;
	uint16 flags;
	uint16 priority;
	uint16 windowNum;
	uint16 zoneNum;
	VgaSprite() { memset(this, 0, sizeof(*this)); }
};

struct VgaSleepStruct {
	uint16 ident;
	const byte *codePtr;
	uint16 id;
	uint16 zoneNum;
	VgaSleepStruct() { memset(this, 0, sizeof(*this)); }
};

struct VgaTimerEntry {
	int16 delay;
	const byte *codePtr;
	uint16 id;
	uint16 zoneNum;
	uint8 type;
	VgaTimerEntry() { memset(this, 0, sizeof(*this)); }
};

struct AnimTable {
	const byte *srcPtr;
	int16 x;
	int16 y;
	uint16 width;
	uint16 height;
	uint16 windowNum;
	uint16 id;
	uint16 zoneNum;
	AnimTable() { memset(this, 0, sizeof(*this)); }
};

enum SIMONGameType {
	GType_PN = 0,
	GType_ELVIRA1 = 1,
	GType_ELVIRA2 = 2,
	GType_WW = 3,
	GType_SIMON1 = 4,
	GType_SIMON2 = 5,
	GType_FF = 6,
	GType_PP = 7
};

enum EventType {
	ANIMATE_INT   = 1 << 1,
	ANIMATE_EVENT = 1 << 2,
	SCROLL_EVENT  = 1 << 3,
	PLAYER_DAMAGE_EVENT = 1 << 4,
	MONSTER_DAMAGE_EVENT = 1 << 5
};

struct AGOSGameDescription;

struct GameSpecificSettings;

class Debugger;

// This is to help devices with small memory (PDA, smartphones, ...)
// to save a bit of memory used by opcode names in the AGOS engine.

#ifndef REDUCE_MEMORY_USAGE
#	define _OPCODE(ver, x)	{ &ver::x, #x }
#else
#	define _OPCODE(ver, x)	{ &ver::x, "" }
#endif

class ArchiveMan : public Common::SearchSet {
public:
	ArchiveMan();

	void enableFallback(bool val) { _fallBack = val; }

#ifdef ENABLE_AGOS2
	void registerArchive(const Common::String &filename, int priority);
#endif

	virtual bool hasFile(const Common::String &name) const;
	virtual int listMatchingMembers(Common::ArchiveMemberList &list, const Common::String &pattern) const;
	virtual int listMembers(Common::ArchiveMemberList &list) const;

	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &filename) const;

private:
	bool _fallBack;
};

class AGOSEngine : public Engine {
protected:
	friend class Debugger;

	// Engine APIs
	Common::Error init();
	virtual Common::Error go();
	virtual Common::Error run() {
		Common::Error err;
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}
	virtual GUI::Debugger *getDebugger();
	virtual bool hasFeature(EngineFeature f) const;
	virtual void syncSoundSettings();
	virtual void pauseEngineIntern(bool pause);

	virtual void setupOpcodes();
	uint16 _numOpcodes, _opcode;

	typedef void (AGOSEngine::*VgaOpcodeProc) ();

	void setupVgaOpcodes();
	VgaOpcodeProc _vga_opcode_table[100];
	uint8 _numVideoOpcodes;

	virtual void setupVideoOpcodes(VgaOpcodeProc *op);

	const AGOSGameDescription * const _gameDescription;

public:
	virtual void setupGame();

	int getGameId() const;
	int getGameType() const;
	uint32 getFeatures() const;
	const char *getExtra() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	const char *getFileName(int type) const;

protected:
	void playSting(uint16 a);

	const byte *_vcPtr;								/* video code ptr */
	uint16 _vcGetOutOfCode;


	uint32 *_gameOffsetsPtr;

	uint8 _numMusic, _numSFX;
	uint16 _numSpeech;
	uint16 _numZone;

	uint8 _numBitArray1, _numBitArray2, _numBitArray3, _numItemStore;
	uint16 _numVars;

	uint8 _vgaBaseDelay, _vgaPeriod;

	uint16 _musicIndexBase;
	uint16 _soundIndexBase;
	uint16 _tableIndexBase;
	uint16 _textIndexBase;

	uint32 _itemMemSize;
	uint32 _tableMemSize;
	uint32 _vgaMemSize;

	const GameSpecificSettings *gss;

	Common::KeyState _keyPressed;

	Common::File *_gameFile;

	byte *_strippedTxtMem;
	byte *_textMem;
	uint32 _textSize;
	uint32 _stringTabNum, _stringTabPos, _stringTabSize;
	byte **_stringTabPtr;

	Item **_itemArrayPtr;
	uint32 _itemArraySize;
	uint32 _itemArrayInited;

	Common::Array<byte *> _itemHeap;

	byte *_iconFilePtr;

	const byte *_codePtr;

	byte **_localStringtable;
	uint16 _stringIdLocalMin, _stringIdLocalMax;

	RoomState *_roomStates;
	uint16 _numRoomStates;

	byte *_menuBase;
	byte *_roomsList;
	byte *_roomsListPtr;

	byte *_xtblList;
	byte *_xtablesHeapPtrOrg;
	uint32 _xtablesHeapCurPosOrg;
	Subroutine *_xsubroutineListOrg;

	byte *_tblList;
	byte *_tablesHeapPtr, *_tablesHeapPtrOrg, *_tablesheapPtrNew;
	uint32 _tablesHeapSize, _tablesHeapCurPos, _tablesHeapCurPosOrg;
	uint32 _tablesHeapCurPosNew;
	Subroutine *_subroutineListOrg;

	Subroutine *_subroutineList;

	uint8 _recursionDepth;

	uint32 _lastVgaTick;

	uint16 _marks;
	bool _scanFlag;

	bool _scriptVar2;
	bool _runScriptReturn1;
	bool _runScriptCondition[40];
	int16 _runScriptReturn[40];
	bool _skipVgaWait;
	bool _noParentNotify;
	bool _beardLoaded;
	bool _litBoxFlag;
	bool _mortalFlag;
	uint16 _displayFlag;
	bool _syncFlag2;
	bool _inCallBack;
	bool _cepeFlag;
	bool _fastMode;
	bool _backFlag;

	uint16 _debugMode;
	Common::Language _language;
	bool _copyProtection;
	bool _pause;
	bool _dumpScripts;
	bool _dumpOpcodes;
	bool _dumpVgaScripts;
	bool _dumpVgaOpcodes;
	bool _dumpImages;
	bool _speech;
	bool _subtitles;
	bool _vgaVar9;
	int16 _chanceModifier;
	bool _restoreWindow6;
	int16 _scrollX, _scrollXMax;
	int16 _scrollY, _scrollYMax;
	int16 _scrollCount, _scrollFlag;
	uint16 _scrollWidth, _scrollHeight;
	const byte *_scrollImage;
	byte _boxStarHeight;

	SubroutineLine *_classLine;
	int16 _classMask, _classMode1, _classMode2;
	Item *_findNextPtr;
	Subroutine *_currentTable;
	SubroutineLine *_currentLine;

	uint8 _agosMenu;
	byte _textMenu[10];
	uint16 _currentRoom, _superRoomNumber;
	uint8 _wallOn;

	uint16 _hyperLink, _newLines;
	uint16 _oracleMaxScrollY, _noOracleScroll;
	uint16 _interactY;

	int16 _scriptVerb, _scriptNoun1, _scriptNoun2;
	int16 _scriptAdj1, _scriptAdj2;

	uint16 _curWindow;
	WindowBlock *_inputWindow, *_textWindow;

	Item *_subjectItem, *_objectItem;
	Item *_currentPlayer;

	Item *_hitAreaObjectItem;
	HitArea *_lastHitArea;
	HitArea *_lastNameOn;
	HitArea *_lastHitArea3;
	Item *_hitAreaSubjectItem;
	HitArea *_currentBox, *_currentVerbBox, *_lastVerbOn;
	uint16	_currentBoxNum;
	uint16 _needHitAreaRecalc;
	uint16 _verbHitArea;
	uint16 _defaultVerb;
	bool _iOverflow;
	bool _nameLocked;

	bool _dragAccept;
	bool _dragEnd;
	bool _dragFlag;
	bool _dragMode;
	uint8 _dragCount;
	HitArea *_lastClickRem;

	uint16 _windowNum;

	int16 _printCharCurPos, _printCharMaxPos, _printCharPixelCount;
	uint16 _numLettersToPrint;

	uint8 _numTextBoxes;

	uint32 getTime() const;

	uint32 _lastMinute; // Used in processSpecialKeys()
	uint32 _lastTime;
	uint32 _clockStopped, _gameStoppedClock;
	uint32 _timeStore;

	TimeEvent *_firstTimeStruct, *_pendingDeleteTimeEvent;

	bool _initMouse;
	Common::Point _mouse;
	Common::Point _mouseOld;

	byte *_mouseData;
	bool _animatePointer;
	byte _maxCursorWidth, _maxCursorHeight;
	byte _mouseAnim, _mouseAnimMax, _mouseCursor;
	byte _currentMouseAnim, _currentMouseCursor;
	byte _oldMouseAnimMax, _oldMouseCursor;
	uint16 _mouseHideCount;
	bool _mouseToggle;

	bool _leftButtonDown, _rightButtonDown;
	byte _leftButton, _leftButtonCount, _leftButtonOld;
	byte _oneClick;
	bool _clickOnly;
	bool _leftClick, _rightClick;
	bool _noRightClick;

	Item *_dummyItem1;
	Item *_dummyItem2;
	Item *_dummyItem3;

	volatile uint16 _videoLockOut;
	uint16 _scrollUpHitArea;
	uint16 _scrollDownHitArea;

	bool _fastFadeOutFlag;
	byte _paletteFlag;
	bool _bottomPalette;
	uint16 _fastFadeCount;
	volatile uint16 _fastFadeInFlag;

	uint16 _screenWidth, _screenHeight;

	uint16 _noOverWrite;
	bool _rejectBlock;

	bool _exitCutscene, _picture8600;

	uint16 _soundFileId;
	int16 _lastMusicPlayed;
	int16 _nextMusicToPlay;
	bool _showPreposition;
	bool _showMessageFlag;

	bool _newDirtyClip;
	bool _wiped;
	uint16 _copyScnFlag, _vgaSpriteChanged;

	byte *_block, *_blockEnd;
	byte *_vgaMemPtr, *_vgaMemEnd, *_vgaMemBase;
	byte *_vgaFrozenBase, *_vgaRealBase;
	byte *_zoneBuffers;

	byte *_curVgaFile1;
	byte *_curVgaFile2;

	uint16 _syncCount;

	uint16 _frameCount;

	uint16 _zoneNumber;
	uint16 _vgaWaitFor, _lastVgaWaitFor;
	uint16 _vgaCurSpriteId, _vgaCurZoneNum;

	int16 _baseY;
	float _scale;
	Common::Rect _feebleRect;
	int16 _scaleX, _scaleY, _scaleWidth, _scaleHeight;

	VgaTimerEntry *_nextVgaTimerToProcess;

	uint8 _opcode177Var1, _opcode177Var2;
	uint8 _opcode178Var1, _opcode178Var2;

	Item *_objectArray[50];
	Item *_itemStore[50];

	uint16 _shortText[40];
	uint16 _shortTextX[40];
	uint16 _shortTextY[40];
	uint16 _longText[40];
	uint16 _longSound[40];

	uint16 _bitArray[128];
	uint16 _bitArrayTwo[16];
	uint16 _bitArrayThree[16];
	int16 *_variableArray;
	int16 *_variableArray2;
	int16 *_variableArrayPtr;

	WindowBlock *_dummyWindow;
	WindowBlock *_windowArray[80];

	byte _fcsData1[8];
	bool _fcsData2[8];

	TextLocation _textLocation1, _textLocation2, _textLocation3, _textLocation4;

	byte _awaitTwoByteToken;
	byte *_byteTokens;
	byte *_byteTokenStrings;
	byte *_twoByteTokens;
	byte *_twoByteTokenStrings;
	byte *_secondTwoByteTokenStrings;
	byte *_thirdTwoByteTokenStrings;
	byte _textBuffer[180];
	int _textCount;

	int _freeStringSlot;

	byte _stringReturnBuffer[2][180];

	HitArea _hitAreas[250];
	HitArea *_hitAreaList;

	AnimTable _screenAnim1[90];
	VgaPointersEntry _vgaBufferPointers[450];
	VgaSprite _vgaSprites[200];
	VgaSleepStruct _onStopTable[60];
	VgaSleepStruct _waitEndTable[60];
	VgaSleepStruct _waitSyncTable[60];

	const uint16 *_pathFindArray[100];

	uint8 _pathValues[400];
	uint16 _PVCount;
	uint16 _GPVCount;

	uint8 _pathValues1[400];
	uint16 _PVCount1;
	uint16 _GPVCount1;

	uint8 _currentPalette[768];
	uint8 _displayPalette[768];

	byte *_planarBuf;
	byte _videoBuf1[32000];
	uint16 _videoWindows[128];

	uint8 _window3Flag;
	uint8 _window4Flag;
	uint8 _window6Flag;

	uint16 _moveXMin, _moveYMin;
	uint16 _moveXMax, _moveYMax;

	VgaTimerEntry _vgaTimerList[205];

	WindowBlock *_windowList;

	byte _lettersToPrintBuf[80];

	MidiPlayer *_midi;
	bool _midiEnabled;

	int _vgaTickCounter;

	Audio::SoundHandle _modHandle;

	Sound *_sound;

	bool _effectsPaused;
	bool _ambientPaused;
	bool _musicPaused;

	Debugger *_debugger;

	uint8 _saveGameNameLen;
	uint16 _saveLoadRowCurPos;
	uint16 _numSaveGameRows;
	bool _saveDialogFlag;
	bool _saveOrLoad;
	bool _saveLoadEdit;

	byte _saveLoadType, _saveLoadSlot;
	char _saveLoadName[108];

	Graphics::Surface *_backGroundBuf;
	Graphics::Surface *_backBuf;
	Graphics::Surface *_scaleBuf;
	Graphics::Surface *_window4BackScn;
	Graphics::Surface *_window6BackScn;

	Common::RandomSource _rnd;

	const byte *_vc10BasePtrOld;
	byte _hebrewCharWidths[32];

public:
	AGOSEngine(OSystem *system, const AGOSGameDescription *gd);
	virtual ~AGOSEngine();

	ArchiveMan _archives;

	byte *_curSfxFile;
	uint32 _curSfxFileSize;
	uint16 _sampleEnd, _sampleWait;

protected:
	virtual uint16 to16Wrapper(uint value);
	virtual uint16 readUint16Wrapper(const void *src);
	virtual uint32 readUint32Wrapper(const void *src);

#ifdef ENABLE_AGOS2
	void loadArchives();
#endif

	int allocGamePcVars(Common::SeekableReadStream *in);
	void createPlayer();
	void allocateStringTable(int num);
	void setupStringTable(byte *mem, int num);
	void setupLocalStringTable(byte *mem, int num);
	void readGamePcText(Common::SeekableReadStream *in);
	virtual void readItemChildren(Common::SeekableReadStream *in, Item *item, uint tmp);
	void readItemFromGamePc(Common::SeekableReadStream *in, Item *item);
	virtual void loadGamePcFile();
	void readGamePcFile(Common::SeekableReadStream *in);
	void decompressData(const char *srcName, byte *dst, uint32 offset, uint32 srcSize, uint32 dstSize);
	void decompressPN(Common::Stack<uint32> &dataList, uint8 *&dataOut, int &dataOutSize);
	void loadOffsets(const char *filename, int number, uint32 &file, uint32 &offset, uint32 &compressedSize, uint32 &size);
	void loadSound(uint16 sound, int16 pan, int16 vol, uint16 type);
	void loadSound(uint16 sound, uint16 freq, uint16 flags);
	void loadVoice(uint speechId);

	void loadSoundFile(const char *filename);

	int getUserFlag(Item *item, int a);
	int getUserFlag1(Item *item, int a);
	int getUserItem(Item *item, int n);
	void setUserFlag(Item *item, int a, int b);
	void setUserItem(Item *item, int n, int m);

	void paletteFadeOut(byte *palPtr, uint num, uint size);

	void *allocateItem(uint size);
	void *allocateTable(uint size);
	void alignTableMem();

	Child *findChildOfType(Item *i, uint child);
	Child *allocateChildBlock(Item *i, uint type, uint size);

	void allocItemHeap();
	void allocTablesHeap();

	Subroutine *createSubroutine(uint16 a);
	void readSubroutine(Common::SeekableReadStream *in, Subroutine *sub);
	SubroutineLine *createSubroutineLine(Subroutine *sub, int a);
	void readSubroutineLine(Common::SeekableReadStream *in, SubroutineLine *newTable, Subroutine *sub);
	byte *readSingleOpcode(Common::SeekableReadStream *in, byte *ptr);
	void readSubroutineBlock(Common::SeekableReadStream *in);

	Subroutine *getSubroutineByID(uint subroutineId);

	/* used in debugger */
	void dumpAllSubroutines();
	void dumpAllVgaImageFiles();
	void dumpAllVgaScriptFiles();
	void dumpSubroutines();
	void dumpSubroutine(Subroutine *sub);
	void dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub);
	const byte *dumpOpcode(const byte *p);

	int startSubroutine(Subroutine *sub);
	int startSubroutineEx(Subroutine *sub);

	bool checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub);

	int runScript();
	virtual void executeOpcode(int opcode) = 0;

	byte getByte();
	int getNextWord();

	uint getNextVarContents();
	uint getVarWrapper();
	uint getVarOrWord();
	uint getVarOrByte();
	uint readVariable(uint16 variable);
	void writeNextVarContents(uint16 contents);
	void writeVariable(uint16 variable, uint16 contents);

	Item *derefItem(uint item);
	Item *getNextItemPtr();
	uint getNextItemID();
	uint getItem1ID() {return 1;}
	uint itemPtrToID(Item *id);
	Item *me();
	Item *actor();

	void uncompressText(byte *ptr);
	byte *uncompressToken(byte a, byte *ptr);

	void showMessageFormat(const char *s, ...) GCC_PRINTF(2, 3);
	const byte *getStringPtrByID(uint16 stringId, bool upperCase = false);
	const byte *getLocalStringByID(uint16 stringId);
	uint getNextStringID();

	void addTimeEvent(uint16 timeout, uint16 subroutineId);
	void delTimeEvent(TimeEvent *te);

	Item *findInByClass(Item *i, int16 m);
	Item *nextInByClass(Item *i, int16 m);
	Item *findMaster(int16 a, int16 n);
	Item *nextMaster(Item *item, int16 a, int16 n);
	int wordMatch(Item *item, int16 a, int16 n);

	bool isRoom(Item *item);
	bool isObject(Item *item);
	bool isPlayer(Item *item);

	void itemChildrenChanged(Item *item);
	void unlinkItem(Item *item);
	void linkItem(Item *item, Item *parent);

	void setItemParent(Item *item, Item *parent);
	void setItemState(Item *item, int value);

	void stopAnimate(uint16 a);
	void stopAnimateSimon2(uint16 a, uint16 b);

	void enableBox(uint hitarea);
	void disableBox(uint hitarea);
	void moveBox(uint hitarea, int x, int y);
	bool isBoxDead(uint hitarea);
	void undefineBox(uint hitarea);
	void defineBox(int id, int x, int y, int width, int height, int flags, int verb, Item *itemPtr);
	void defineBox(uint16 id, uint16 x, uint16 y, uint16 width, uint16 height, uint16 msg1, uint16 msg2, uint16 flags);

	HitArea *findEmptyHitArea();

	virtual void resetVerbs();
	virtual void setVerb(HitArea * ha);
	virtual void hitarea_leave(HitArea * ha, bool state = false);
	void leaveHitAreaById(uint hitarea_id);

	void sendSync(uint a);
	void waitForSync(uint a);

	uint getOffsetOfChild2Param(SubObject *child, uint prop);
	void scriptMouseOff();
	void freezeBottom();
	void unfreezeBottom();

	TextLocation *getTextLocation(uint a);

	uint setVerbText(HitArea *ha);
	void waitForInput();
	void setup_cond_c_helper();

	uint16 getBackExit(int n);
	uint16 getDoorState(Item *item, uint16 d);
	uint16 getExitOf(Item *item, uint16 d);
	void changeDoorState(SubRoom *r, uint16 d, uint16 n);
	void setDoorState(Item *i, uint16 d, uint16 n);

	// Elvira 1 specific
	Item *getDoorOf(Item *item, uint16 d);
	Item *getExitOf_e1(Item *item, uint16 d);
	virtual void moveDirn(Item *i, uint x);

	virtual int canPlace(Item *x, Item *y);
	int contains(Item *a, Item *b);
	int sizeContents(Item *x);
	virtual int sizeOfRec(Item *o, int d);
	int sizeRec(Item *x, int d);
	int weighUp(Item *x);
	int weightRec(Item *x, int d);
	virtual int weightOf(Item *x);
	void xPlace(Item *x, Item *y);

	void restoreMenu();
	void drawMenuStrip(uint windowNum, uint menuNum);
	void lightMenuStrip(int a);
	void unlightMenuStrip();
	void lightMenuBox(uint hitarea);

	uint menuFor_e2(Item *item);
	uint menuFor_ww(Item *item, uint id);
	void clearMenuStrip();
	void doMenuStrip(uint menuNum);

	void mouseOff();
	void mouseOn();

	bool loadRoomItems(uint16 item);

	virtual bool loadTablesIntoMem(uint16 subrId);
	bool loadXTablesIntoMem(uint16 subrId);
	void loadTextIntoMem(uint16 stringId);

	uint loadTextFile(const char *filename, byte *dst);
	Common::SeekableReadStream *openTablesFile(const char *filename);
	void closeTablesFile(Common::SeekableReadStream *in);

	uint loadTextFile_simon1(const char *filename, byte *dst);
	Common::SeekableReadStream *openTablesFile_simon1(const char *filename);

	uint loadTextFile_gme(const char *filename, byte *dst);
	Common::SeekableReadStream *openTablesFile_gme(const char *filename);

	void invokeTimeEvent(TimeEvent *te);
	bool kickoffTimeEvents();
	void killAllTimers();

	void endCutscene();
	virtual void runSubroutine101();

	virtual void inventoryUp(WindowBlock *window);
	virtual void inventoryDown(WindowBlock *window);

	WindowBlock *openWindow(uint x, uint y, uint w, uint h, uint flags, uint fillColor, uint textColor);
	uint getWindowNum(WindowBlock *window);
	void clearWindow(WindowBlock *window);
	void changeWindow(uint a);
	void closeWindow(uint a);
	void setTextColor(uint color);
	virtual void windowPutChar(WindowBlock *window, byte c, byte b = 0);
	void waitWindow(WindowBlock *window);

	HitArea *findBox(uint hitarea_id);
	virtual void boxController(uint x, uint y, uint mode);
	void handleVerbClicked(uint verb);
	virtual void clearName();
	void displayName(HitArea * ha);
	void resetNameWindow();
	void displayBoxStars();
	void invertBox(HitArea * ha, byte a, byte b, byte c, byte d);

	virtual void initMouse();
	virtual void handleMouseMoved();
	virtual void drawMousePointer();

	void drawArrow(uint16 x, uint16 y, int8 dir);
	virtual void addArrows(WindowBlock *window, uint8 num);
	virtual void removeArrows(WindowBlock *window, uint num);

	virtual void drawIcon(WindowBlock *window, uint icon, uint x, uint y);
	virtual bool hasIcon(Item *item);
	virtual uint itemGetIconNumber(Item *item);
	virtual uint setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr);

	virtual void drawIconArray(uint i, Item *itemPtr, int line, int classMask);
	void removeIconArray(uint num);

	void loadIconData();
	void loadIconFile();
	void loadMenuFile();

	virtual bool processSpecialKeys();
	void hitarea_stuff_helper();

	void permitInput();

	uint getFeebleFontSize(byte chr);
	void justifyStart();
	void justifyOutPut(byte chr);

	void loadZone(uint16 zoneNum, bool useError = true);

	void animate(uint16 windowNum, uint16 zoneNum, uint16 vgaSpriteId, int16 x, int16 y, uint16 palette, bool vgaScript = false);
	void setImage(uint16 vgaSpriteId, bool vgaScript = false);
	void setWindowImage(uint16 mode, uint16 vgaSpriteId, bool specialCase = false);
	virtual void setWindowImageEx(uint16 mode, uint16 vgaSpriteId);
	void drawEdging();

	void skipSpeech();

	const char *getPixelLength(const char *string, uint16 maxWidth, uint16 &pixels);
	bool printNameOf(Item *item, uint x, uint y);
	bool printTextOf(uint a, uint x, uint y);
	void printVerbOf(uint hitarea_id);
	void showActionString(const byte *string);

	virtual void printScreenText(uint vgaSpriteId, uint color, const char *stringPtr, int16 x, int16 y, int16 width);

	void renderStringAmiga(uint vgaSpriteId, uint color, uint width, uint height, const char *txt);
	void renderString(uint vgaSpriteId, uint color, uint width, uint height, const char *txt);

	void writeChar(WindowBlock *window, int x, int y, int offs, int val);

	byte *allocBlock(uint32 size);
	void checkNoOverWrite();
	void checkRunningAnims();
	void checkAnims(uint a);
	void checkZonePtrs();
	void setZoneBuffers();

	void runVgaScript();

public:
	bool getBitFlag(uint bit);
	void setBitFlag(uint bit, bool value);

	// Video Script Opcodes, Common
	void vc1_fadeOut();
	void vc2_call();
	virtual void vc3_loadSprite();
	void vc4_fadeIn();
	void vc5_ifEqual();
	void vc6_ifObjectHere();
	void vc7_ifObjectNotHere();
	void vc8_ifObjectIsAt();
	void vc9_ifObjectStateIs();
	void vc10_draw();
	void vc12_delay();
	void vc13_addToSpriteX();
	void vc14_addToSpriteY();
	void vc15_sync();
	void vc16_waitSync();
	void vc18_jump();
	void vc19_loop();
	void vc20_setRepeat();
	void vc21_endRepeat();
	virtual void vc22_setPalette();
	void vc23_setPriority();
	void vc24_setSpriteXY();
	void vc25_halt_sprite();
	void vc26_setSubWindow();
	void vc27_resetSprite();
	void vc28_playSFX();
	void vc29_stopAllSounds();
	void vc30_setFrameRate();
	void vc31_setWindow();
	void vc33_setMouseOn();
	void vc34_setMouseOff();
	void vc35_clearWindow();
	virtual void vc36_setWindowImage();
	void vc38_ifVarNotZero();
	void vc39_setVar();
	void vc40_scrollRight();
	void vc41_scrollLeft();
	void vc42_delayIfNotEQ();

	// Video Script Opcodes, Personal Nightmare
	void vc11_onStop();
	void vc36_pause();
	void vc39_volume();
	void vc44_enableBox();
	void vc45_disableBox();
	void vc46_maxBox();
	void vc48_specialEffect();
	void vc50_setBox();
	void vc55_scanFlag();

	// Video Script Opcodes, Elvira 1
	void vc17_waitEnd();
	void vc32_saveScreen();
	void vc37_pokePalette();

	// Video Script Opcodes, Elvira 2
	void vc43_ifBitSet();
	void vc44_ifBitClear();
	void vc45_setWindowPalette();
	void vc46_setPaletteSlot1();
	void vc47_setPaletteSlot2();
	void vc48_setPaletteSlot3();
	void vc49_setBit();
	void vc50_clearBit();
	void vc51_enableBox();
	void vc52_playSound();
	void vc53_dissolveIn();
	void vc54_dissolveOut();
	void vc55_moveBox();
	void vc56_fullScreen();
	void vc57_blackPalette();
	void vc58_checkCodeWheel();
	void vc58_changePriority();
	void vc59_ifEGA();

	// Video Script Opcodes, Waxworks
	void vc60_stopAnimation();
	void vc61();
	void vc62_fastFadeOut();
	virtual void vc63_fastFadeIn();

	// Video Script Opcodes, Simon 1
	void vc11_clearPathFinder();
	void vc17_setPathfinderItem();
	void vc32_copyVar();
	void vc37_addToSpriteY();
	void vc45_setSpriteX();
	void vc46_setSpriteY();
	void vc47_addToVar();
	virtual void vc48_setPathFinder();
	void vc59_ifSpeech();
	void vc61_setMaskImage();

	// Video Script Opcodes, Simon 2
	void vc56_delayLong();
	void vc59_stopAnimations();
	void vc64_ifSpeech();
	void vc65_slowFadeIn();
	void vc66_ifEqual();
	void vc67_ifLE();
	void vc68_ifGE();
	void vc69_playSeq();
	void vc70_joinSeq();
	void vc71_ifSeqWaiting();
	void vc72_segue();
	void vc73_setMark();
	void vc74_clearMark();

	// Video Script Opcodes, Feeble Files
	void vc75_setScale();
	void vc76_setScaleXOffs();
	void vc77_setScaleYOffs();
	void vc78_computeXY();
	void vc79_computePosNum();
	void vc80_setOverlayImage();
	void vc81_setRandom();
	void vc82_getPathValue();
	void vc83_playSoundLoop();
	void vc84_stopSoundLoop();

	void setScriptCondition(bool cond);
	bool getScriptCondition();
	void setScriptReturn(int ret);
	int getScriptReturn();

	// Opcodes, common
	void o_invalid();
	void o_at();
	void o_notAt();
	void o_carried();
	void o_notCarried();
	void o_isAt();
	void o_zero();
	void o_notZero();
	void o_eq();
	void o_notEq();
	void o_gt();
	void o_lt();
	void o_eqf();
	void o_notEqf();
	void o_ltf();
	void o_gtf();
	void o_chance();
	void o_isRoom();
	void o_isObject();
	void o_state();
	void o_oflag();
	void o_destroy();
	void o_place();
	void o_copyff();
	void o_clear();
	void o_let();
	void o_add();
	void o_sub();
	void o_addf();
	void o_subf();
	void o_mul();
	void o_div();
	void o_mulf();
	void o_divf();
	void o_mod();
	void o_modf();
	void o_random();
	void o_goto();
	void o_oset();
	void o_oclear();
	void o_putBy();
	void o_inc();
	void o_dec();
	void o_setState();
	void o_print();
	void o_message();
	void o_msg();
	void o_end();
	void o_done();
	void o_process();
	void o_when();
	void o_if1();
	void o_if2();
	void o_isCalled();
	void o_is();
	void o_debug();
	void o_comment();
	void o_haltAnimation();
	void o_restartAnimation();
	void o_getParent();
	void o_getNext();
	void o_getChildren();
	void o_picture();
	void o_loadZone();
	void o_killAnimate();
	void o_defWindow();
	void o_window();
	void o_cls();
	void o_closeWindow();
	void o_addBox();
	void o_delBox();
	void o_enableBox();
	void o_disableBox();
	void o_moveBox();
	void o_doIcons();
	void o_isClass();
	void o_setClass();
	void o_unsetClass();
	void o_waitSync();
	void o_sync();
	void o_defObj();
	void o_here();
	void o_doClassIcons();
	void o_playTune();
	void o_setAdjNoun();
	void o_saveUserGame();
	void o_loadUserGame();
	void o_copysf();
	void o_restoreIcons();
	void o_freezeZones();
	void o_placeNoIcons();
	void o_clearTimers();
	void o_setDollar();
	void o_isBox();

	int16 moreText(Item *i);
	void lobjFunc(Item *i, const char *f);
	uint confirmYesOrNo(uint16 x, uint16 y);
	uint continueOrQuit();
	void printScroll();
	virtual void printStats();
	void synchChain(Item *i);

protected:
	bool drawImage_clip(VC10_state *state);

	void drawImage_init(int16 image, uint16 palette, int16 x, int16 y, uint16 flags);

	virtual void drawImage(VC10_state *state);
	void drawBackGroundImage(VC10_state *state);
	void drawVertImage(VC10_state *state);
	void drawVertImageCompressed(VC10_state *state);
	void drawVertImageUncompressed(VC10_state *state);

	void setMoveRect(uint16 x, uint16 y, uint16 width, uint16 height);

	void horizontalScroll(VC10_state *state);
	void verticalScroll(VC10_state *state);

	int vcReadVarOrWord();
	uint vcReadNextWord();
	uint vcReadNextByte();
	uint vcReadVar(uint var);
	void vcWriteVar(uint var, int16 value);
	void vcSkipNextInstruction();

	int getScale(int16 y, int16 x);
	void checkScrollX(int16 x, int16 xpos);
	void checkScrollY(int16 y, int16 ypos);
	void centerScroll();

	virtual void clearVideoWindow(uint16 windowNum, uint16 color);
	void clearVideoBackGround(uint16 windowNum, uint16 color);

	void setPaletteSlot(uint16 srcOffs, uint8 dstOffs);
	void checkOnStopTable();
	void checkWaitEndTable();

	virtual bool ifObjectHere(uint16 val);
	virtual bool ifObjectAt(uint16 a, uint16 b);
	virtual bool ifObjectState(uint16 a, int16 b);

	bool isVgaQueueEmpty();
	void haltAnimation();
	void restartAnimation();
	void addVgaEvent(uint16 num, uint8 type, const byte *codePtr, uint16 curSprite, uint16 curZoneNum);
	void deleteVgaEvent(VgaTimerEntry * vte);
	void processVgaEvents();
	void animateEvent(const byte *codePtr, uint16 curZoneNum, uint16 curSprite);
	void scrollEvent();
	void drawStuff(const byte *src, uint offs);
	void playerDamageEvent(VgaTimerEntry * vte, uint dx);
	void monsterDamageEvent(VgaTimerEntry * vte, uint dx);

	VgaSprite *findCurSprite();

	bool isSpriteLoaded(uint16 id, uint16 zoneNum);

	void resetWindow(WindowBlock *window);
	void freeBox(uint index);

	void sendWindow(uint a);

	virtual void colorWindow(WindowBlock *window);
	void colorBlock(WindowBlock *window, uint16 x, uint16 y, uint16 w, uint16 h);

	void restoreWindow(WindowBlock *window);
	void restoreBlock(uint16 x, uint16 y, uint16 w, uint16 h);

	byte *getBackBuf();
	byte *getBackGround();
	byte *getScaleBuf();

	byte *convertImage(VC10_state *state, bool compressed);

	bool decrunchFile(byte *src, byte *dst, uint32 size);
	void loadVGABeardFile(uint16 id);
	void loadVGAVideoFile(uint16 id, uint8 type, bool useError = true);
	bool loadVGASoundFile(uint16 id, uint8 type);

	void openGameFile();
	void readGameFile(void *dst, uint32 offs, uint32 size);

	virtual void timerProc();

	virtual void animateSprites();

	void dirtyClips();
	void dirtyClipCheck(int16 x, int16 y, int16 w, int16 h);
	void dirtyBackGround();
	void restoreBackGround();
	void saveBackGround(VgaSprite *vsp);

	void clearSurfaces();
	void displayScreen();

	void dumpVideoScript(const byte *src, bool singeOpcode);
	virtual void dumpVgaFile(const byte *vga);
	void dumpVgaScript(const byte *ptr, uint16 res, uint16 id);
	void dumpVgaScriptAlways(const byte *ptr, uint16 res, uint16 id);

	void dumpVgaBitmaps(uint16 zoneNum);

	void dumpSingleBitmap(int file, int image, const byte *offs, int w, int h, byte base);
	void dumpBitmap(const char *filename, const byte *offs, uint16 w, uint16 h, int flags, const byte *palette, byte base);
	void palLoad(byte *pal, const byte *vga1, int a, int b);

	void fillBackFromBackGround(uint16 height, uint16 width);
	void fillBackFromFront();
	void fillBackGroundFromBack();
	void fillBackGroundFromFront();

	virtual void doOutput(const byte *src, uint len);
	void clsCheck(WindowBlock *window);

	virtual void quickLoadOrSave();

	byte *vc10_uncompressFlip(const byte *src, uint16 w, uint16 h);
	byte *vc10_flip(const byte *src, uint16 w, uint16 h);

	Item *getNextItemPtrStrange();

	virtual bool loadGame(const char *filename, bool restartMode = false);
	virtual bool saveGame(uint slot, const char *caption);

	void openTextWindow();
	void tidyIconArray(uint i);

	virtual void windowNewLine(WindowBlock *window);
	void windowScroll(WindowBlock *window);
	virtual void windowDrawChar(WindowBlock *window, uint x, uint y, byte chr);

	void loadMusic(uint16 track);
	void playModule(uint16 music);
	virtual void playMusic(uint16 music, uint16 track);
	void stopMusic();

	void delay(uint delay);
	void pause();

	void waitForMark(uint i);
	void scrollScreen();

	void decodeColumn(byte *dst, const byte *src, uint16 height, uint16 pitch);
	void decodeRow(byte *dst, const byte *src, uint16 width, uint16 pitch);
	void hitarea_stuff_helper_2();
	void fastFadeIn();
	void slowFadeIn();
	void fullFade();

	virtual void vcStopAnimation(uint16 zone, uint16 sprite);

	virtual bool confirmOverWrite(WindowBlock *window);
	int16 matchSaveGame(const char *name, uint16 max);
	void disableFileBoxes();
	virtual void userGame(bool load);
	void userGameBackSpace(WindowBlock *window, int x, byte b = 0);
	void fileError(WindowBlock *window, bool saveError);

	int countSaveGames();

	virtual char *genSaveName(int slot);
};

class AGOSEngine_PN : public AGOSEngine {

	virtual Common::Error go();
	void demoSeq();
	void introSeq();
	void setupBoxes();
	int readfromline();
public:
	AGOSEngine_PN(OSystem *system, const AGOSGameDescription *gd);
	~AGOSEngine_PN();

	virtual void setupGame();
	virtual void setupOpcodes();
	virtual void setupVideoOpcodes(VgaOpcodeProc *op);

	virtual void executeOpcode(int opcode);

	int actCallD(int n);

	void opn_opcode00();
	void opn_add();
	void opn_sub();
	void opn_mul();
	void opn_div();
	void opn_opcode05();
	void opn_opcode06();
	void opn_opcode07();
	void opn_opcode08();
	void opn_opcode09();
	void opn_opcode10();
	void opn_opcode11();
	void opn_opcode12();
	void opn_opcode13();
	void opn_opcode14();
	void opn_opcode15();
	void opn_opcode16();
	void opn_lt();
	void opn_gt();
	void opn_eq();
	void opn_neq();
	void opn_opcode21();
	void opn_opcode22();
	void opn_opcode23();
	void opn_opcode24();
	void opn_opcode25();
	void opn_opcode26();
	void opn_opcode27();
	void opn_opcode28();
	void opn_opcode29();
	void opn_opcode30();
	void opn_opcode31();
	void opn_opcode32();
	void opn_opcode33();
	void opn_opcode34();
	void opn_opcode35();
	void opn_opcode36();
	void opn_opcode37();
	void opn_opcode38();
	void opn_opcode39();
	void opn_opcode40();
	void opn_opcode41();
	void opn_opcode42();
	void opn_opcode43();
	void opn_opcode44();
	void opn_opcode45();
	void opn_opcode46();
	void opn_opcode47();
	void opn_opcode48();
	void opn_opcode49();
	void opn_opcode50();
	void opn_opcode51();
	void opn_opcode52();
	void opn_opcode53();
	void opn_opcode54();
	void opn_opcode55();
	void opn_opcode56();
	void opn_opcode57();
	void opn_opcode62();
	void opn_opcode63();

protected:
	struct StackFrame {
		StackFrame *nextframe;
		int16 flag[6];
		int16 param[8];
		int16 classnum;
		uint8 *linpos;
		uint8 *lbase;
		int16 ll;
		int16 linenum;
		int16 process;
		int tagOfParentDoline;	///< tag of the doline "instance" to which this StackFrame belongs
		StackFrame() { memset(this, 0, sizeof(*this)); }
	};


	StackFrame *_stackbase;

	int _tagOfActiveDoline;	///< tag of the active doline "instance"
	int _dolineReturnVal;

	byte *_dataBase, *_textBase;
	uint32 _dataBaseSize, _textBaseSize;

	HitArea _invHitAreas[45];

	char _buffer[80];
	char _inputline[61];
	char _saveFile[20];
	char _sb[80];
	uint8 _wordcp[7];

	const char *_mouseString, *_mouseString1;
	char _objectName1[15], _objectName2[15];
	char _inMessage[20];
	char _placeMessage[15];
	bool _inputReady;
	bool _inputting;
	uint16 _intputCounter, _inputMax;
	uint16 _mousePrintFG;
	HitArea *_dragStore;
	uint8 _hitCalled;

	uint32 _quickptr[16];
	uint16 _quickshort[12];

	bool _noScanFlag;
	char _keyboardBuffer[61];

	uint16 _objects;
	int16 _objectCountS;

	int16 _bp;
	int16 _xofs;
	int16 _havinit;
	uint16 _seed;

	char *_curwrdptr;
	char *_inpp;
	int _fnst;
	int _procnum;
	int _linct;
	int _linembr;
	uint8 *_linebase;
	uint8 *_workptr;

	uint16 getptr(uint32 pos);
	uint32 getlong(uint32 pos);

	virtual void loadGamePcFile();

	int bitextract(uint32 ptr, int offs);
	int doaction();
	int doline(int needsave);
	int setposition(int process, int line);
	int varval();

	char *getMessage(char *msg, uint16 num);
	void getResponse(uint16 charNum, uint16 objNum, uint16 &msgNum1, uint16 &msgNum2);
	void getObjectName(char *v, uint16 x);

	void processor();
	void setbitf(uint32 ptr, int offs, int val);
	void setqptrs();
	void writeval(uint8 *ptr, int val);

	void addstack(int type);
	void dumpstack();
	void popstack(int type);
	void funccpy(int *store);
	void funcentry(int *storestore, int procn);

	int findentry();
	int findset();
	int gvwrd(uint8 *wptr, int mask);
	int wrdmatch(uint8 *word1, int mask1, uint8 *word2, int mask2);

	bool testContainer(uint16 a);
	bool testObvious(uint16 a);
	bool testSeen(uint16 a);

	bool ifObjectInInv(uint16 a);
	int inventoryOn(int val);
	int inventoryOff();
	void mouseHit();
	void execMouseHit(HitArea *ha);
	void hitBox1(HitArea *ha);
	void hitBox2(HitArea *ha);
	void hitBox3(HitArea *ha);
	void hitBox4(HitArea *ha);
	void hitBox5(HitArea *ha);
	void hitBox6(HitArea *ha);
	void hitBox7(HitArea *ha);
	void hitBox8(HitArea *ha);
	void hitBox9(HitArea *ha);
	void hitBox11(HitArea *ha);

	void drawIconHitBar();
	void iconPage();
	void printIcon(HitArea *ha, uint8 i, uint8 r);
	virtual void windowPutChar(WindowBlock *window, byte c, byte b = 0);

	bool badload(int8 errorNum);
	int loadFile(char *name);
	int saveFile(char *name);
	void getFilename();
	void sysftodb();
	void dbtosysf();

	uint32 ftext(uint32 base, int n);
	char *unctok(char *c, int n);
	void uncomstr(char *c, uint32 x);
	void patok(int n);
	void pcf(uint8 ch);
	void pcl(const char *s);
	void pmesd(int n);
	void plocd(int n, int m);
	void pobjd(int n, int m);
	void ptext(uint32 tptr);

	virtual void clearVideoWindow(uint16 windowNum, uint16 color);
	virtual void setWindowImageEx(uint16 mode, uint16 vga_res);

	virtual bool ifObjectHere(uint16 val);
	virtual bool ifObjectAt(uint16 a, uint16 b);
	virtual bool ifObjectState(uint16 a, int16 b);

	virtual void boxController(uint x, uint y, uint mode);
	virtual void timerProc();

	void addChar(uint8 chr);
	void clearCursor(WindowBlock *window);
	void clearInputLine();
	void handleKeyboard();
	virtual void handleMouseMoved();
	void interact(char *buffer, uint8 size);

	virtual bool processSpecialKeys();
protected:
	typedef void (AGOSEngine_PN::*OpcodeProcPN) ();
	struct OpcodeEntryPN {
		OpcodeProcPN proc;
		const char *desc;
	};

	const OpcodeEntryPN *_opcodesPN;
};

class AGOSEngine_Elvira1 : public AGOSEngine {
public:
	AGOSEngine_Elvira1(OSystem *system, const AGOSGameDescription *gd);
	//~AGOSEngine_Elvira1();

	virtual void setupGame();
	virtual void setupOpcodes();
	virtual void setupVideoOpcodes(VgaOpcodeProc *op);

	virtual void executeOpcode(int opcode);

	void oe1_present();
	void oe1_notPresent();
	void oe1_worn();
	void oe1_notWorn();
	void oe1_notCarried();
	void oe1_isNotAt();
	void oe1_sibling();
	void oe1_notSibling();
	void oe1_isIn();
	void oe1_isNotIn();
	void oe1_isPlayer();
	void oe1_canPut();
	void oe1_create();
	void oe1_copyof();
	void oe1_copyfo();
	void oe1_whatO();
	void oe1_weigh();
	void oe1_setFF();
	void oe1_moveDirn();
	void oe1_score();
	void oe1_look();
	void oe1_doClass();
	void oe1_pObj();
	void oe1_pName();
	void oe1_pcName();
	void oe1_isCalled();
	void oe1_cFlag();
	void oe1_rescan();
	void oe1_setUserItem();
	void oe1_getUserItem();
	void oe1_whereTo();
	void oe1_doorExit();
	void oe1_loadGame();
	void oe1_clearUserItem();
	void oe1_findMaster();
	void oe1_nextMaster();
	void oe1_animate();
	void oe1_stopAnimate();
	void oe1_menu();
	void oe1_addBox();
	void oe1_enableInput();
	void oe1_setTime();
	void oe1_ifTime();
	void oe1_playTune();
	void oe1_bitClear();
	void oe1_bitSet();
	void oe1_bitTest();
	void oe1_zoneDisk();
	void oe1_printStats();
	void oe1_stopTune();
	void oe1_printPlayerDamage();
	void oe1_printMonsterDamage();
	void oe1_pauseGame();
	void oe1_printPlayerHit();
	void oe1_printMonsterHit();

protected:
	typedef void (AGOSEngine_Elvira1::*OpcodeProcElvira1) ();
	struct OpcodeEntryElvira1 {
		OpcodeProcElvira1 proc;
		const char *desc;
	};

	const OpcodeEntryElvira1 *_opcodesElvira1;

	virtual void drawIcon(WindowBlock *window, uint icon, uint x, uint y);

	virtual char *genSaveName(int slot);
};

class AGOSEngine_Elvira2 : public AGOSEngine_Elvira1 {
public:
	AGOSEngine_Elvira2(OSystem *system, const AGOSGameDescription *gd);
	//~AGOSEngine_Elvira2();

	virtual void setupGame();
	virtual void setupOpcodes();
	virtual void setupVideoOpcodes(VgaOpcodeProc *op);

	virtual void executeOpcode(int opcode);

	void oe2_moveDirn();
	void oe2_doClass();
	void oe2_pObj();
	void oe2_isCalled();
	void oe2_menu();
	void oe2_drawItem();
	void oe2_doTable();
	void oe2_pauseGame();
	void oe2_setDoorOpen();
	void oe2_setDoorClosed();
	void oe2_setDoorLocked();
	void oe2_ifDoorOpen();
	void oe2_ifDoorClosed();
	void oe2_ifDoorLocked();
	void oe2_storeItem();
	void oe2_getItem();
	void oe2_bSet();
	void oe2_bClear();
	void oe2_bZero();
	void oe2_bNotZero();
	void oe2_getOValue();
	void oe2_setOValue();
	void oe2_ink();
	void oe2_printStats();
	void oe2_setSuperRoom();
	void oe2_getSuperRoom();
	void oe2_setExitOpen();
	void oe2_setExitClosed();
	void oe2_setExitLocked();
	void oe2_ifExitOpen();
	void oe2_ifExitClosed();
	void oe2_ifExitLocked();
	void oe2_playEffect();
	void oe2_getDollar2();
	void oe2_setSRExit();
	void oe2_printPlayerDamage();
	void oe2_printMonsterDamage();
	void oe2_isAdjNoun();
	void oe2_b2Set();
	void oe2_b2Clear();
	void oe2_b2Zero();
	void oe2_b2NotZero();

	virtual void printStats();
protected:
	typedef void (AGOSEngine_Elvira2::*OpcodeProcElvira2) ();
	struct OpcodeEntryElvira2 {
		OpcodeProcElvira2 proc;
		const char *desc;
	};

	const OpcodeEntryElvira2 *_opcodesElvira2;

	virtual void readItemChildren(Common::SeekableReadStream *in, Item *item, uint tmp);

	virtual bool loadGame(const char *filename, bool restartMode = false);
	virtual bool saveGame(uint slot, const char *caption);

	virtual void addArrows(WindowBlock *window, uint8 num);
	virtual void removeArrows(WindowBlock *window, uint num);

	virtual void drawIcon(WindowBlock *window, uint icon, uint x, uint y);
	virtual bool hasIcon(Item *item);
	virtual uint itemGetIconNumber(Item *item);
	virtual uint setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr);

	virtual void moveDirn(Item *i, uint x);
	virtual int canPlace(Item *x, Item *y);
	virtual int sizeOfRec(Item *o, int d);
	virtual int weightOf(Item *x);

	int changeExitStates(SubSuperRoom *sr, int n, int d, uint16 s);
	uint16 getExitState(Item *item, uint16 x, uint16 d);
	void setExitState(Item *i, uint16 n, uint16 d, uint16 s);
	void setSRExit(Item *i, int n, int d, uint16 s);

	virtual void listSaveGames(char *dst);
	virtual bool confirmOverWrite(WindowBlock *window);
	virtual void userGame(bool load);
	virtual int userGameGetKey(bool *b, char *buf, uint maxChar);

	virtual char *genSaveName(int slot);
};

class AGOSEngine_Waxworks : public AGOSEngine_Elvira2 {
public:
	AGOSEngine_Waxworks(OSystem *system, const AGOSGameDescription *gd);
	//~AGOSEngine_Waxworks();

	virtual void setupGame();
	virtual void setupOpcodes();
	virtual void setupVideoOpcodes(VgaOpcodeProc *op);

	virtual void executeOpcode(int opcode);

	void boxTextMessage(const char *x);
	void boxTextMsg(const char *x);
	void printBox();
	uint16 getBoxSize();
	uint16 checkFit(char *Ptr, int width, int lines);

	void oww_goto();
	void oww_addTextBox();
	void oww_setShortText();
	void oww_setLongText();
	void oww_printLongText();
	void oww_whereTo();
	void oww_textMenu();
	void oww_pauseGame();
	void oww_boxMessage();
	void oww_boxMsg();
	void oww_boxLongText();
	void oww_printBox();
	void oww_boxPObj();
	void oww_lockZones();
	void oww_unlockZones();

protected:
	typedef void (AGOSEngine_Waxworks::*OpcodeProcWaxworks) ();
	struct OpcodeEntryWaxworks {
		OpcodeProcWaxworks proc;
		const char *desc;
	};

	const OpcodeEntryWaxworks *_opcodesWaxworks;

	bool _boxCR;
	char _boxBuffer[310];
	char *_boxBufferPtr;
	int _boxLineCount;
	int _lineCounts[6];
	char *_linePtrs[6];

	virtual void drawIcon(WindowBlock *window, uint icon, uint x, uint y);

	virtual void boxController(uint x, uint y, uint mode);

	virtual void addArrows(WindowBlock *window, uint8 num);
	virtual void removeArrows(WindowBlock *window, uint num);

	virtual uint setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr);

	virtual bool loadTablesIntoMem(uint16 subrId);

	virtual void moveDirn(Item *i, uint x);

	virtual bool confirmOverWrite(WindowBlock *window);

	virtual char *genSaveName(int slot);
};

class AGOSEngine_Simon1 : public AGOSEngine_Waxworks {
public:
	AGOSEngine_Simon1(OSystem *system, const AGOSGameDescription *gd);
	//~AGOSEngine_Simon1();

	virtual void setupGame();
	virtual void setupOpcodes();
	virtual void setupVideoOpcodes(VgaOpcodeProc *op);

	virtual void executeOpcode(int opcode);

	virtual void vc22_setPalette();

	// Opcodes, Simon 1
	void os1_animate();
	void os1_pauseGame();
	void os1_screenTextBox();
	void os1_screenTextMsg();
	void os1_playEffect();
	void os1_screenTextPObj();
	void os1_getPathPosn();
	void os1_scnTxtLongText();
	void os1_mouseOn();
	void os1_mouseOff();
	void os1_loadBeard();
	void os1_unloadBeard();
	void os1_unloadZone();
	void os1_loadStrings();
	void os1_unfreezeZones();
	void os1_specialFade();

protected:
	typedef void (AGOSEngine_Simon1::*OpcodeProcSimon1) ();
	struct OpcodeEntrySimon1 {
		OpcodeProcSimon1 proc;
		const char *desc;
	};

	const OpcodeEntrySimon1 *_opcodesSimon1;

	virtual void drawImage(VC10_state *state);
	void drawMaskedImage(VC10_state *state);
	void draw32ColorImage(VC10_state *state);

	virtual void dumpVgaFile(const byte *vga);

	virtual void clearName();

	virtual void drawIcon(WindowBlock *window, uint icon, uint x, uint y);

	virtual void initMouse();
	virtual void handleMouseMoved();

	virtual void addArrows(WindowBlock *window, uint8 num);
	virtual void removeArrows(WindowBlock *window, uint num);

	virtual uint setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr);

	virtual void playSpeech(uint16 speechId, uint16 vgaSpriteId);

	virtual void listSaveGames(char *dst);
	virtual void userGame(bool load);
	virtual int userGameGetKey(bool *b, char *buf, uint maxChar);

	virtual void playMusic(uint16 music, uint16 track);

	virtual void vcStopAnimation(uint16 zone, uint16 sprite);

	virtual char *genSaveName(int slot);
};

class AGOSEngine_Simon2 : public AGOSEngine_Simon1 {
public:
	AGOSEngine_Simon2(OSystem *system, const AGOSGameDescription *gd);
	//~AGOSEngine_Simon2();

	virtual void setupGame();
	virtual void setupOpcodes();
	virtual void setupVideoOpcodes(VgaOpcodeProc *op);

	virtual void executeOpcode(int opcode);

	void os2_printLongText();
	void os2_rescan();
	void os2_animate();
	void os2_stopAnimate();
	void os2_playTune();
	void os2_screenTextPObj();
	void os2_mouseOn();
	void os2_mouseOff();
	void os2_isShortText();
	void os2_clearMarks();
	void os2_waitMark();

protected:
	typedef void (AGOSEngine_Simon2::*OpcodeProcSimon2) ();
	struct OpcodeEntrySimon2 {
		OpcodeProcSimon2 proc;
		const char *desc;
	};

	const OpcodeEntrySimon2 *_opcodesSimon2;

	virtual void clearName();

	virtual void drawIcon(WindowBlock *window, uint icon, uint x, uint y);

	virtual void addArrows(WindowBlock *window, uint8 num);
	virtual uint setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr);

	virtual void clearVideoWindow(uint16 windowNum, uint16 color);

	virtual void playSpeech(uint16 speechId, uint16 vgaSpriteId);

	virtual char *genSaveName(int slot);
};

#ifdef ENABLE_AGOS2
class AGOSEngine_Feeble : public AGOSEngine_Simon2 {
public:
	AGOSEngine_Feeble(OSystem *system, const AGOSGameDescription *gd);
	~AGOSEngine_Feeble();

	virtual void setupGame();
	virtual void setupOpcodes();
	virtual void setupVideoOpcodes(VgaOpcodeProc *op);

	virtual void executeOpcode(int opcode);

	virtual void vc36_setWindowImage();
	virtual void vc48_setPathFinder();

	void off_chance();
	void off_jumpOut();
	void off_addTextBox();
	void off_printLongText();
	void off_addBox();
	void off_oracleTextDown();
	void off_oracleTextUp();
	void off_ifTime();
	void off_setTime();
	void off_saveUserGame();
	void off_loadUserGame();
	void off_listSaveGames();
	void off_checkCD();
	void off_screenTextBox();
	void off_isAdjNoun();
	void off_hyperLinkOn();
	void off_hyperLinkOff();
	void off_checkPaths();
	void off_screenTextPObj();
	void off_mouseOn();
	void off_mouseOff();
	void off_loadVideo();
	void off_playVideo();
	void off_centerScroll();
	void off_resetPVCount();
	void off_setPathValues();
	void off_stopClock();
	void off_restartClock();
	void off_setColor();
	void off_b3Set();
	void off_b3Clear();
	void off_b3Zero();
	void off_b3NotZero();

protected:
	friend class MoviePlayer;
	friend class MoviePlayerDXA;
	friend class MoviePlayerSMK;

	typedef void (AGOSEngine_Feeble::*OpcodeProcFeeble) ();
	struct OpcodeEntryFeeble {
		OpcodeProcFeeble proc;
		const char *desc;
	};

	const OpcodeEntryFeeble *_opcodesFeeble;

	MoviePlayer *_moviePlayer;

	uint8 _interactiveVideo;
	uint16 _vgaCurSpritePriority;

	virtual uint16 to16Wrapper(uint value);
	virtual uint16 readUint16Wrapper(const void *src);
	virtual uint32 readUint32Wrapper(const void *src);

	void playVideo(const char *filename, bool lastSceneUsed = false);
	void stopInteractiveVideo();

	virtual void drawImage(VC10_state *state);
	void scaleClip(int16 h, int16 w, int16 y, int16 x, int16 scrollY);

	void drawMousePart(int image, byte x, byte y);
	virtual void initMouse();
	virtual void drawMousePointer();

	virtual void animateSprites();
	void animateSpritesByY();

	void oracleLogo();
	void swapCharacterLogo();
	virtual void timerProc();

	virtual void addArrows(WindowBlock *window, uint8 num);
	virtual uint setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr);

	virtual void resetVerbs();
	virtual void setVerb(HitArea * ha);
	virtual void hitarea_leave(HitArea * ha, bool state = false);
	void invertBox(HitArea *ha, bool state);

	virtual void windowNewLine(WindowBlock *window);
	virtual void windowDrawChar(WindowBlock *window, uint x, uint y, byte chr);

	virtual void clearName();

	virtual void drawIconArray(uint i, Item *itemPtr, int line, int classMask);

	virtual void colorWindow(WindowBlock *window);

	virtual void dumpVgaFile(const byte *vga);

	virtual void doOutput(const byte *src, uint len);

	virtual void printScreenText(uint vgaSpriteId, uint color, const char *stringPtr, int16 x, int16 y, int16 width);

	void printInteractText(uint16 num, const char *string);
	void sendInteractText(uint16 num, const char *fmt, ...) GCC_PRINTF(3, 4);

	void checkLinkBox();
	void hyperLinkOn(uint16 x);
	void hyperLinkOff();
	void linksUp();
	void linksDown();

	virtual void runSubroutine101();

	void checkUp(WindowBlock *window);
	void checkDown(WindowBlock *window);
	virtual void inventoryUp(WindowBlock *window);
	virtual void inventoryDown(WindowBlock *window);

	void oracleTextUp();
	void oracleTextDown();
	void scrollOracle();
	void scrollOracleUp();
	void scrollOracleDown();

	void listSaveGamesFeeble();
	void saveUserGame(int slot);
	void windowBackSpace(WindowBlock *window);

	virtual char *genSaveName(int slot);
	virtual void quickLoadOrSave();
};

class AGOSEngine_FeebleDemo : public AGOSEngine_Feeble {
public:
	AGOSEngine_FeebleDemo(OSystem *system, const AGOSGameDescription *gd);

protected:
	bool _filmMenuUsed;

	virtual Common::Error go();

	virtual void initMouse();
	virtual void drawMousePointer();

	void exitMenu();
	void filmMenu();
	void handleText();
	void handleWobble();
	void mainMenu();
	void startInteractiveVideo(const char *filename);
	void waitForSpace();
};

class AGOSEngine_PuzzlePack : public AGOSEngine_Feeble {
public:
	AGOSEngine_PuzzlePack(OSystem *system, const AGOSGameDescription *gd);
	//~AGOSEngine_PuzzlePack();

	virtual void setupGame();
	virtual void setupOpcodes();

	virtual void executeOpcode(int opcode);

	virtual void vc3_loadSprite();
	virtual void vc63_fastFadeIn();

	void opp_iconifyWindow();
	void opp_restoreOopsPosition();
	void opp_loadMouseImage();
	void opp_message();
	void opp_setShortText();
	void opp_loadHiScores();
	void opp_checkHiScores();
	void opp_sync();
	void opp_saveUserGame();
	void opp_loadUserGame();
	void opp_playTune();
	void opp_saveOopsPosition();
	void opp_resetGameTime();
	void opp_resetPVCount();
	void opp_setPathValues();
	void opp_pauseClock();

protected:
	typedef void (AGOSEngine_PuzzlePack::*OpcodeProcPuzzlePack) ();
	struct OpcodeEntryPuzzlePack {
		OpcodeProcPuzzlePack proc;
		const char *desc;
	};

	const OpcodeEntryPuzzlePack *_opcodesPuzzlePack;

	bool _oopsValid;
	uint32 _gameTime;

	virtual void initMouse();
	virtual void handleMouseMoved();
	virtual void drawMousePointer();

	virtual void resetVerbs();

	void loadMouseImage();

	void startOverlayAnims();
	void startAnOverlayAnim();

	void printInfoText(const char *itemText);

	virtual char *genSaveName(int slot);
};


class AGOSEngine_DIMP : public AGOSEngine_PuzzlePack {
public:
	AGOSEngine_DIMP(OSystem *system, const AGOSGameDescription *gd);
	//~AGOSEngine_DIMP();

	virtual void setupOpcodes();

	virtual void executeOpcode(int opcode);

protected:
	typedef void (AGOSEngine_DIMP::*OpcodeProcDIMP) ();
	struct OpcodeEntryDIMP {
		OpcodeProcDIMP proc;
		const char *desc;
	};

	const OpcodeEntryDIMP *_opcodesDIMP;

	int16 _iconToggleCount, _voiceCount;
	uint32 _lastTickCount;
	uint32 _startSecondCount, _tSecondCount;

	void odp_saveUserGame();
	void odp_loadUserGame();

	void dimpIdle();
	virtual void timerProc();

};
#endif

} // End of namespace AGOS

#endif
