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

#ifndef DREAMWEB_H
#define DREAMWEB_H

#include "common/error.h"
#include "common/file.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "engines/engine.h"

#include "dreamweb/console.h"

#include "dreamweb/structs.h"

#define SCUMMVM_HEADER MKTAG('S', 'C', 'V', 'M')
#define SCUMMVM_BLOCK_MAGIC_SIZE 0x1234
#define SAVEGAME_VERSION 1

namespace DreamWeb {

const unsigned int kNumReelRoutines = 57;
const unsigned int kUnderTextSizeX = 180;
const unsigned int kUnderTextSizeY = 10;
const unsigned int kUnderTimedTextSizeY = 24;
const unsigned int kUnderTextSizeX_f = 228; // foreign version
const unsigned int kUnderTextSizeY_f = 13; // foreign version
const unsigned int kUnderTimedTextSizeY_f = 30;
const unsigned int kUnderTextBufSize = kUnderTextSizeX_f * kUnderTextSizeY_f;
const unsigned int kUnderTimedTextBufSize = 256 * kUnderTimedTextSizeY_f;
const unsigned int kLengthOfVars = 68;
const unsigned int kNumChanges = 250;
const unsigned int kLenOfMapStore = 22*8*20*8;
const unsigned int kMapWidth = 66;
const unsigned int kMapHeight = 60;
const unsigned int kLengthOfMap = kMapWidth * kMapHeight;
const unsigned int kNumExObjects = 114;
const unsigned int kScreenwidth = 320;
const unsigned int kDiaryx = (68+24);
const unsigned int kDiaryy = (48+12);
const unsigned int kInventx = 80;
const unsigned int kInventy = 58;
const unsigned int kMenux = (80+40);
const unsigned int kMenuy = 60;
const unsigned int kOpsx = 60;
const unsigned int kOpsy = 52;
const unsigned int kSymbolx = 64;
const unsigned int kSymboly = 56;
const unsigned int kLengthofvars = 68;
const unsigned int kFrameBlocksize = 2080;
const unsigned int kGraphicsFileFrameSize = 347; // ceil(2080 / sizeof(Frame))
const unsigned int kNumexobjects = 114;
const unsigned int kNumExTexts = kNumexobjects + 2;
const uint16 kExtextlen = 18000;
const uint16 kExframeslen = 30000;
const unsigned int kLengthofextra = kFrameBlocksize + kExframeslen + sizeof(DynObject)*kNumexobjects + sizeof(uint16)*kNumExTexts + kExtextlen;
const unsigned int kItempicsize = 44;
const unsigned int kNumSetTexts = 130;
const unsigned int kNumBlockTexts = 98;
const unsigned int kNumRoomTexts = 38;
const unsigned int kNumFreeTexts = 82;
const unsigned int kNumPersonTexts = 1026;

// Keyboard buffer. data.word(kBufferin) and data.word(kBufferout) are indexes
// into this, making it a ring buffer
extern uint8 g_keyBuffer[16];

// Engine Debug Flags
enum {
	kDebugAnimation = (1 << 0),
	kDebugSaveLoad = (1 << 1)
};

struct DreamWebGameDescription;
class DreamWebSound;

class DreamWebEngine : public Engine {
private:
	DreamWebConsole			*_console;
	DreamWebSound *_sound;
	bool					_vSyncInterrupt;

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

public:
	DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc);
	virtual ~DreamWebEngine();

	void setVSyncInterrupt(bool flag);
	void waitForVSync();

	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);

	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();

	uint8 randomNumber() { return _rnd.getRandomNumber(255); }

	void mouseCall(uint16 *x, uint16 *y, uint16 *state); //fill mouse pos and button state
	void processEvents();
	void blit(const uint8 *src, int pitch, int x, int y, int w, int h);
	void cls();
	bool isCD();
	bool hasSpeech();

	void getPalette(uint8 *data, uint start, uint count);
	void setPalette(const uint8 *data, uint start, uint count);

	Common::String getSavegameFilename(int slot) const;

	void setShakePos(int pos) { _system->setShakePos(pos); }
	void printUnderMonitor();

	void quit();

	bool loadSpeech(const Common::String &filename);

	void enableSavingOrLoading(bool enable = true) { _enableSavingOrLoading = enable; }

	Common::Language getLanguage() const;
	uint8 modifyChar(uint8 c) const;
	Common::String modifyFileName(const char *);

	const Common::String& getDatafilePrefix() { return _datafilePrefix; }
	const Common::String& getSpeechDirName() { return _speechDirName; }

private:
	void keyPressed(uint16 ascii);
	void setSpeed(uint speed);

	const DreamWebGameDescription	*_gameDescription;
	Common::RandomSource			_rnd;
	Common::String _datafilePrefix;
	Common::String _speechDirName;

	uint _speed;
	bool _turbo;
	uint _oldMouseState;
	bool _enableSavingOrLoading;

protected:
	GameVars _vars; // saved variables

	// from backdrop.cpp
	uint8 *_backdropBlocks;
	BackdropMapFlag _backdropFlags[96];
	uint8 _mapData[kLengthOfMap + 32];

	// from keypad.cpp
	uint8 _pressList[6];
	uint8 _pressed;
	uint16 _pressPointer;
	uint8 _graphicPress;
	uint8 _pressCount;
	uint8 _lightCount;
	uint8 _folderPage;
	uint8 _diaryPage;
	uint8 _menuCount;
	uint8 _symbolTopX;
	uint8 _symbolTopNum;
	int8 _symbolTopDir;
	uint8 _symbolBotX;
	uint8 _symbolBotNum;
	int8 _symbolBotDir;

	// from monitor.cpp
	char _inputLine[64];
	char _operand1[64];
	char _currentFile[14];

	// from newplace.cpp
	uint8 _roomsCanGo[16];

	// from object.cpp
	uint16 _openChangeSize;
	ObjectRef _oldSubject;

	// from pathfind.cpp
	Common::Point _lineData[200];		// Output of Bresenham

	// from saveload.cpp
	char _saveNames[17*21];
	char _saveNamesOld[17*21];

	// from vgagrafx.cpp
	uint8 _workspace[(0x1000 + 2) * 16];
	uint8 _mapStore[kLenOfMapStore + 32];

	// from people.cpp
	ReelRoutine _reelRoutines[kNumReelRoutines+1];
	ReelRoutine *_personData;

	// from Buffers
	uint8 _textUnder[kUnderTextBufSize];
	ObjectRef _openInvList[16];
	ObjectRef _ryanInvList[30];
	uint8 _pointerBack[32*32];
	MapFlag _mapFlags[11*10];
	uint8 _startPal[3*256];
	uint8 _endPal[3*256];
	uint8 _mainPal[3*256];
	Common::List<Sprite> _spriteTable;
	Common::List<ObjPos> _setList;
	Common::List<ObjPos> _freeList;
	Common::List<ObjPos> _exList;
	Common::List<People> _peopleList;
	uint8 _zoomSpace[46*40];
	// _printedList (unused?)
	Change _listOfChanges[kNumChanges]; // Note: this array is saved
	uint8 _underTimedText[kUnderTimedTextBufSize];
	Common::List<Rain> _rainList;

	// textfiles
	TextFile _textFile1;
	TextFile _textFile2;
	TextFile _textFile3;
	TextFile _travelText;
	TextFile _puzzleText;
	TextFile _commandText;

	// local graphics files
	GraphicsFile _keypadGraphics;
	GraphicsFile _menuGraphics;
	GraphicsFile _menuGraphics2;
	GraphicsFile _folderGraphics;
	GraphicsFile _folderGraphics2;
	GraphicsFile _folderGraphics3;
	GraphicsFile _folderCharset;
	GraphicsFile _symbolGraphics;
	GraphicsFile _diaryGraphics;
	GraphicsFile _diaryCharset;
	GraphicsFile _monitorGraphics;
	GraphicsFile _monitorCharset;
	GraphicsFile _newplaceGraphics;
	GraphicsFile _newplaceGraphics2;
	GraphicsFile _newplaceGraphics3;
	GraphicsFile _cityGraphics;
	GraphicsFile _saveGraphics;

	// global graphics files
	GraphicsFile _icons1;
	GraphicsFile _icons2;
	GraphicsFile _charset1;
	GraphicsFile _mainSprites;
	const GraphicsFile *_currentCharset;

	// room files
	GraphicsFile _setFrames;
	GraphicsFile _freeFrames;
	GraphicsFile _reel1;
	GraphicsFile _reel2;
	GraphicsFile _reel3;
	TextFile _setDesc;
	TextFile _blockDesc;
	TextFile _roomDesc;
	TextFile _freeDesc;
	TextFile _personText;
	uint16 _personFramesLE[12];
	RoomPaths _pathData[36];
	Reel *_reelList;
	SetObject _setDat[128];
	DynObject _freeDat[80];

	// Extras segment (NB: this is saved)
	GraphicsFile _exFrames;
	DynObject _exData[kNumExObjects];
	TextFile _exText;

public:
	DreamWebEngine(/*DreamWeb::DreamWebEngine *en*/);

	bool _quitRequested;
	bool _subtitles;
	bool _foreignRelease;

	bool _wonGame;
	bool _hasSpeech;

	// sound related
	uint8 _roomsSample;
	bool _speechLoaded;

	// misc variables
	uint8 _speechCount;
	uint16 _charShift;
	uint8 _kerning;
	bool _brightPalette;
	bool _copyProtection;
	uint8 _roomLoaded;
	uint8 _didZoom;
	uint16 _lineSpacing;
	uint16 _textAddressX;
	uint16 _textAddressY;
	uint8 _textLen;
	uint16 _lastXPos;
	uint8 _itemFrame;
	uint8 _withObject;
	uint8 _withType;
	uint16 _lookCounter;
	uint8 _command;
	uint8 _commandType;
	uint8 _objectType;
	uint8 _getBack;
	uint8 _invOpen;
	uint8 _mainMode;
	uint8 _pickUp;
	uint8 _lastInvPos;
	uint8 _examAgain;
	uint8 _newTextLine;
	uint8 _openedOb;
	uint8 _openedType;
	uint16 _mapAdX;
	uint16 _mapAdY;
	uint16 _mapOffsetX;
	uint16 _mapOffsetY;
	uint16 _mapXStart;
	uint16 _mapYStart;
	uint8 _mapXSize;
	uint8 _mapYSize;
	uint8 _haveDoneObs;
	uint8 _manIsOffScreen;
	uint8 _facing;
	uint8 _leaveDirection;
	uint8 _turnToFace;
	uint8 _turnDirection;
	uint16 _mainTimer;
	uint8 _introCount;
	uint8 _currentKey;
	uint8 _timerCount;
	uint8 _mapX;
	uint8 _mapY;
	uint8 _ryanX;
	uint8 _ryanY;
	uint8 _lastFlag;
	uint8 _destPos;
	uint8 _realLocation;
	uint8 _roomNum;
	uint8 _nowInNewRoom;
	uint8 _resetManXY;
	uint8 _newLocation;
	uint8 _autoLocation;
	uint16 _mouseX;
	uint16 _mouseY;
	uint16 _mouseButton;
	uint16 _oldButton;
	uint16 _oldX;
	uint16 _oldY;
	uint16 _oldPointerX;
	uint16 _oldPointerY;
	uint16 _delHereX;
	uint16 _delHereY;
	uint8 _pointerXS;
	uint8 _pointerYS;
	uint8 _delXS;
	uint8 _delYS;
	uint8 _pointerFrame;
	uint8 _pointerPower;
	uint8 _pointerMode;
	uint8 _pointerSpeed;
	uint8 _pointerCount;
	uint8 _inMapArea;
	uint8 _talkMode;
	uint8 _talkPos;
	uint8 _character;
	uint8 _watchDump;
	uint8 _logoNum;
	uint8 _oldLogoNum;
	uint8 _walkAndExam;
	uint8 _walkExamType;
	uint8 _walkExamNum;
	uint16 _cursLocX;
	uint16 _cursLocY;
	uint16 _curPos;
	uint16 _monAdX;
	uint16 _monAdY;
	uint16 _timeCount;
	uint8 _needToDumpTimed;
	TimedTemp _previousTimedTemp;
	TimedTemp _timedTemp;
	uint8 _loadingOrSave;
	uint8 _saveLoadPage;
	uint8 _currentSlot;
	uint8 _cursorPos;
	uint8 _colourPos;
	uint8 _fadeDirection;
	uint8 _numToFade;
	uint8 _fadeCount;
	uint8 _addToGreen;
	uint8 _addToRed;
	uint8 _addToBlue;
	uint16 _lastSoundReel;
	uint8 _lastHardKey;
	uint16 _bufferIn;
	uint16 _bufferOut;
	uint8 _blinkFrame;
	uint8 _blinkCount;
	uint8 _reAssesChanges;
	uint8 _pointersPath;
	uint8 _mansPath;
	uint8 _pointerFirstPath;
	uint8 _finalDest;
	uint8 _destination;
	uint16 _lineStartX;
	uint16 _lineStartY;
	uint16 _lineEndX;
	uint16 _lineEndY;
	uint8 _linePointer;
	uint8 _lineDirection;
	uint8 _lineLength;

	// from backdrop.cpp
	void doBlocks();
	uint8 getXAd(const uint8 *setData, uint8 *result);
	uint8 getYAd(const uint8 *setData, uint8 *result);
	uint8 getMapAd(const uint8 *setData, uint16 *x, uint16 *y);
	void calcFrFrame(const Frame &frame, uint8* width, uint8* height, uint16 x, uint16 y, ObjPos *objPos);
	void makeBackOb(SetObject *objData, uint16 x, uint16 y);
	void showAllObs();
	void getDimension(uint8 *mapXstart, uint8 *mapYstart, uint8 *mapXsize, uint8 *mapYsize);
	void calcMapAd();
	void showAllFree();
	void drawFlags();
	void showAllEx();

	// from keypad.cpp
	void getUnderMenu();
	void putUnderMenu();
	void singleKey(uint8 key, uint16 x, uint16 y);
	void loadKeypad();
	void showKeypad();
	bool isItRight(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3);
	void addToPressList();
	void buttonOne();
	void buttonTwo();
	void buttonThree();
	void buttonFour();
	void buttonFive();
	void buttonSix();
	void buttonSeven();
	void buttonEight();
	void buttonNine();
	void buttonNought();
	void buttonEnter();
	void buttonPress(uint8 buttonId);
	void showOuterPad();
	void dumpKeypad();
	void dumpSymbol();
	void quitSymbol();
	void enterCode(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3);

	// from monitor.cpp
	void input();
	byte makeCaps(byte c);
	void delChar();
	void monMessage(uint8 index);
	void netError();
	void monitorLogo();
	void randomAccess(uint16 count);
	void printOuterMon();
	void showCurrentFile();
	void accessLightOn();
	void accessLightOff();
	void turnOnPower();
	void powerLightOn();
	void powerLightOff();
	void printLogo();
	void scrollMonitor();
	const char *monPrint(const char *string);
	void lockLightOn();
	void lockLightOff();
	void loadPersonal();
	void loadNews();
	void loadCart();
	void showKeys();
	const char *parser();
	const char *searchForString(const char *topic, const char *text);
	const char *getKeyAndLogo(const char *foundString);
	void read();
	void dirFile(const char *dirName);
	void dirCom();
	void useMon();
	bool execCommand();

	// from newplace.cpp
	void getUnderCentre();
	void putUnderCentre();
	void showArrows();
	uint8 getLocation(uint8 index);
	void setLocation(uint8 index);
	void clearLocation(uint8 index);
	void resetLocation(uint8 index);
	void readCityPic();
	void readDestIcon();
	void showCity();
	void locationPic();
	void selectLocation();
	void newPlace();
	void nextDest();
	void lastDest();
	void destSelect();
	void lookAtPlace();

	// from object.cpp
	void obIcons();
	void fillRyan();
	void findAllRyan();
	void obToInv(uint8 index, uint8 flag, uint16 x, uint16 y);
	void obPicture();
	void removeObFromInv();
	void deleteExObject(uint8 index);
	void deleteExFrame(uint8 frameNum);
	void deleteExText(uint8 textNum);
	void purgeALocation(uint8 index);
	const uint8 *getObTextStart();
	void wornError();
	void makeWorn(DynObject *object);
	void dropObject();
	ObjectRef findOpenPos();
	byte getOpenedSlotSize();
	byte getOpenedSlotCount();
	void openOb();
	void findAllOpen();
	void fillOpen();
	ObjectRef findInvPos();
	void reExFromInv();
	void swapWithInv();
	void transferText(uint8 from, uint8 to);
	void pickupConts(uint8 from, uint8 containerEx);
	byte transferToEx(uint8 from);
	void swapWithOpen();
	void outOfOpen();
	void inToInv();
	void outOfInv();
	bool checkObjectSize();
	void useOpened();
	void setPickup();
	void selectOpenOb();
	void examineOb(bool examineAgain = true);
	void selectOb();
	void inventory();
	void identifyOb();

	// from pathfind.cpp
	void turnPathOn(uint8 param);
	void turnPathOff(uint8 param);
	void turnAnyPathOn(uint8 param, uint8 room);
	void turnAnyPathOff(uint8 param, uint8 room);
	RoomPaths *getRoomsPaths();
	void faceRightWay();
	void setWalk();
	void autoSetWalk();
	void checkDest(const RoomPaths *roomsPaths);
	void findXYFromPath();
	bool checkIfPathIsOn(uint8 index);
	void bresenhams();
	void workoutFrames();
	byte findFirstPath(byte x, byte y);
	byte findPathOfPoint(byte x, byte y);

	// from people.cpp
	void setupInitialReelRoutines();
	void updatePeople();
	void madmanText();
	void madman(ReelRoutine &routine);
	void madMode();
	void addToPeopleList(ReelRoutine *routine);
	bool checkSpeed(ReelRoutine &routine);
	void sparkyDrip(ReelRoutine &routine);
	void genericPerson(ReelRoutine &routine);
	void gamer(ReelRoutine &routine);
	void eden(ReelRoutine &routine);
	void sparky(ReelRoutine &routine);
	void rockstar(ReelRoutine &routine);
	void madmansTelly(ReelRoutine &routine);
	void smokeBloke(ReelRoutine &routine);
	void manAsleep(ReelRoutine &routine);
	void drunk(ReelRoutine &routine);
	void introMagic1(ReelRoutine &routine);
	void introMagic2(ReelRoutine &routine);
	void introMagic3(ReelRoutine &routine);
	void introMusic(ReelRoutine &routine);
	void candles(ReelRoutine &routine);
	void candles1(ReelRoutine &routine);
	void candles2(ReelRoutine &routine);
	void smallCandle(ReelRoutine &routine);
	void gates(ReelRoutine &routine);
	void security(ReelRoutine &routine);
	void edenInBath(ReelRoutine &routine);
	void louis(ReelRoutine &routine);
	void handClap(ReelRoutine &routine);
	void carParkDrip(ReelRoutine &routine);
	void foghornSound(ReelRoutine &routine);
	void train(ReelRoutine &routine);
	void attendant(ReelRoutine &routine);
	void keeper(ReelRoutine &routine);
	void interviewer(ReelRoutine &routine);
	void drinker(ReelRoutine &routine);
	void alleyBarkSound(ReelRoutine &routine);
	void louisChair(ReelRoutine &routine);
	void bossMan(ReelRoutine &routine);
	void priest(ReelRoutine &routine);
	void monkAndRyan(ReelRoutine &routine);
	void copper(ReelRoutine &routine);
	void introMonks1(ReelRoutine &routine);
	void introMonks2(ReelRoutine &routine);
	void soldier1(ReelRoutine &routine);
	void receptionist(ReelRoutine &routine);
	void bartender(ReelRoutine &routine);
	void heavy(ReelRoutine &routine);
	void helicopter(ReelRoutine &routine);
	void mugger(ReelRoutine &routine);
	void businessMan(ReelRoutine &routine);
	void endGameSeq(ReelRoutine &routine);
	void poolGuard(ReelRoutine &routine);

	// from print.cpp
	uint8 getNextWord(const GraphicsFile &charSet, const uint8 *string, uint8 *totalWidth, uint8 *charCount);
	void printChar(const GraphicsFile &charSet, uint16 *x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height);
	void printChar(const GraphicsFile &charSet, uint16 x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height);
	void printBoth(const GraphicsFile &charSet, uint16 *x, uint16 y, uint8 c, uint8 nextChar);
	uint8 printDirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered);
	uint8 printDirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered);
	uint8 getNumber(const GraphicsFile &charSet, const uint8 *string, uint16 maxWidth, bool centered, uint16 *offset);
	uint8 kernChars(uint8 firstChar, uint8 secondChar, uint8 width);
	uint8 printSlow(const uint8 *string, uint16 x, uint16 y, uint8 maxWidth, bool centered);
	uint16 waitFrames();
	void printCurs();
	void delCurs();
	void rollEndCreditsGameWon();
	void rollEndCreditsGameLost();

	// from saveload.cpp
	void loadGame();
	void doLoad(int slot);
	void saveGame();
	void namesToOld();
	void oldToNames();
	void saveLoad();
	void doSaveLoad();
	void showMainOps();
	void showDiscOps();
	void discOps();
	void actualSave();
	void actualLoad();
	void loadPosition(unsigned int slot);
	void savePosition(unsigned int slot, const char *descbuf);
	uint scanForNames();
	void loadOld();
	void showDecisions();
	void loadSaveBox();
	void showNames();
	void checkInput();
	void selectSaveLoadPage();
	void selectSlot();
	void showSlots();
	void showOpBox();
	void showSaveOps();
	void showLoadOps();

	// from sprite.cpp
	void printSprites();
	void printASprite(const Sprite *sprite);
	void clearSprites();
	Sprite *makeSprite(uint8 x, uint8 y, bool _mainManCallback, const GraphicsFile *frameData);
	void initMan();
	void walking(Sprite *sprite);
	void aboutTurn(Sprite *sprite);
	void backObject(Sprite *sprite);
	void constant(Sprite *sprite, SetObject *objData);
	void randomSprite(Sprite *sprite, SetObject *objData);
	void doorway(Sprite *sprite, SetObject *objData);
	void wideDoor(Sprite *sprite, SetObject *objData);
	void doDoor(Sprite *sprite, SetObject *objData, Common::Rect check);
	void steady(Sprite *sprite, SetObject *objData);
	void lockedDoorway(Sprite *sprite, SetObject *objData);
	void liftSprite(Sprite *sprite, SetObject *objData);

	Reel *getReelStart(uint16 reelPointer);
	const GraphicsFile *findSource(uint16 &frame);
	void showReelFrame(Reel *reel);
	void showGameReel(ReelRoutine *routine);
	const Frame *getReelFrameAX(uint16 frame);
	void moveMap(uint8 param);
	void checkOne(uint8 x, uint8 y, uint8 *flag, uint8 *flagEx, uint8 *type, uint8 *flagX, uint8 *flagY);

	uint8 getBlockOfPixel(uint8 x, uint8 y);
	void splitIntoLines(uint8 x, uint8 y);
	void initRain();

	void intro1Text();
	void intro2Text(uint16 nextReelPointer);
	void intro3Text(uint16 nextReelPointer);

	void monks2text();
	void textForEnd();
	void textForMonkHelper(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount);
	void textForMonk();
	void priestText(ReelRoutine &routine);
	void soundOnReels(uint16 reelPointer);
	void clearBeforeLoad();
	void clearReels();
	void getRidOfReels();
	void liftNoise(uint8 index);
	void checkForExit(Sprite *sprite);
	void mainMan(Sprite *sprite);
	void spriteUpdate();
	void showRain();
	void reconstruct();
	void reelsOnScreen();

	// from stubs.cpp
	void setupInitialVars();
	void crosshair();
	void delTextLine();
	void showBlink();
	void dumpBlink();
	void dumpPointer();
	void showPointer();
	void delPointer();
	void showRyanPage();
	void switchRyanOn();
	void switchRyanOff();
	void middlePanel();
	void showDiary();
	void readMouse();
	uint16 readMouseState();
	void hangOn(uint16 frameCount);
	void lockMon();
	uint8 *textUnder();
	void readKey();
	void findOrMake(uint8 index, uint8 value, uint8 type);
	DynObject *getFreeAd(uint8 index);
	DynObject *getExAd(uint8 index);
	DynObject *getEitherAd();
	void *getAnyAdDir(uint8 index, uint8 flag);
	void showWatch();
	void showTime();
	void showExit();
	void showMan();
	void panelIcons1();
	SetObject *getSetAd(uint8 index);
	void *getAnyAd(uint8 *slotSize, uint8 *slotCount);
	const uint8 *getTextInFile1(uint16 index);
	uint8 findNextColon(const uint8 **string);
	void allocateBuffers();
	void loadTextFile(TextFile &file, const char *suffix);
	void loadGraphicsFile(GraphicsFile &file, const char *suffix);
	void loadGraphicsSegment(GraphicsFile &file, Common::File &inFile, unsigned int len);
	void loadTextSegment(TextFile &file, Common::File &inFile, unsigned int len);
	void loadTravelText();
	void loadTempText(const char *suffix);
	void sortOutMap();
	void loadRoomData(const Room &room, bool skipDat);
	void useTempCharset(GraphicsFile *charset);
	void useCharset1();
	void printMessage(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered);
	void printMessage2(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered, uint8 count);
	bool isItDescribed(const ObjPos *objPos);
	void zoomIcon();
	void roomName();
	void showIcon();
	void eraseOldObs();
	void commandOnlyCond(uint8 command, uint8 commandType);
	void commandOnly(uint8 command);
	void commandWithOb(uint8 command, uint8 type, uint8 index);
	void blank();
	void setTopLeft();
	void setTopRight();
	void setBotLeft();
	void setBotRight();
	void examIcon();
	void animPointer();
	void getFlagUnderP(uint8 *flag, uint8 *flagEx);
	void workToScreenM();
	void quitKey();
	void restoreReels();
	void loadFolder();
	void folderHints();
	void folderExit();
	void showFolder();
	void showLeftPage();
	void showRightPage();
	void underTextLine();
	void hangOnP(uint16 count);
	void getUnderZoom();
	void putUnderZoom();
	void examineInventory();
	void openInv();
	void getBack1();
	void getBackFromOb();
	void getBackFromOps();
	void getBackToOps();
	void DOSReturn();
	bool isItWorn(const DynObject *object);
	bool compare(uint8 index, uint8 flag, const char id[4]);
	void hangOnW(uint16 frameCount);
	void getRidOfTempText();
	void getRidOfAll();
	void placeSetObject(uint8 index);
	void removeSetObject(uint8 index);
	bool isSetObOnMap(uint8 index);
	void dumpZoom();
	void diaryKeyP();
	void diaryKeyN();
	void findRoomInLoc();
	void loadMenu();
	void showMenu();
	void dumpMenu();
	void dealWithSpecial(uint8 firstParam, uint8 secondParam);
	void plotReel(uint16 &reelPointer);
	void setupTimedTemp(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount);
	void getUnderTimed();
	void putUnderTimed();
	void dumpTextLine();
	void useTimedText();
	void dumpTimedText();
	void getTime();
	void doChange(uint8 index, uint8 value, uint8 type);
	bool isRyanHolding(const char *id);
	void clearBuffers();
	void clearChanges();
	void drawFloor();
	uint16 findSetObject(const char *id);
	void hangOnCurs(uint16 frameCount);
	const uint8 *findObName(uint8 type, uint8 index);
	void copyName(uint8 type, uint8 index, uint8 *dst);
	uint16 findExObject(const char *id);
	void makeMainScreen();
	void showWatchReel();
	void watchReel();
	void examineObText();
	void blockNameText();
	void personNameText();
	void walkToText();
	void entryTexts();
	void setAllChanges();
	void restoreAll();
	void redrawMainScrn();
	void checkCoords(const RectWithCallback *rectWithCallbacks);
	void newGame();
	void deleteTaken();
	void autoAppear();
	void loadRoom();
	void startLoading(const Room &room);
	void startup();
	void atmospheres();
	bool objectMatches(void *object, const char *id);
	void checkFolderCoords();
	void nextFolder();
	void lastFolder();
	void lookAtCard();
	void obsThatDoThings();
	void describeOb();
	void putBackObStuff();
	void reExFromOpen();
	void showDiaryPage();
	void showDiaryKeys();
	void dumpDiaryKeys();
	void useMenu();
	void incRyanPage();
	void edensFlatReminders();
	void dropError();
	void cantDrop();
	void entryAnims();
	bool finishedWalking();
	void emergencyPurge();
	void purgeAnItem();
	uint8 nextSymbol(uint8 symbol);
	void enterSymbol();
	void showSymbol();
	void updateSymbolTop();
	void updateSymbolBot();
	void pickupOb(uint8 command, uint8 pos);
	void errorMessage1();
	void errorMessage2();
	void errorMessage3();
	void decide();
	void allPointer();
	void dumpWatch();
	void watchCount();
	void signOn();
	void searchForFiles(const char *filesString);
	void triggerMessage(uint16 index);
	void processTrigger();
	void dreamwebFinalize();
	void dreamweb();
	void screenUpdate();
	void startup1();
	void readOneBlock();
	bool checkIfPerson(uint8 x, uint8 y);
	bool checkIfFree(uint8 x, uint8 y);
	bool checkIfEx(uint8 x, uint8 y);
	void walkAndExamine();
	void obName(uint8 command, uint8 commandType);
	bool checkIfSet(uint8 x, uint8 y);
	void readSetData();
	void look();
	void autoLook();
	void doLook();
	void mainScreen();
	void zoomOnOff();
	void initialInv();
	void walkIntoRoom();
	void afterNewRoom();
	void madmanRun();
	void showGun();

	// from talk.cpp
	void talk();
	void convIcons();
	uint16 getPersFrame(uint8 index);
	const uint8 *getPersonText(uint8 index, uint8 talkPos);
	void startTalk();
	void moreTalk();
	void doSomeTalk();
	bool hangOnPQ();
	void redes();

	// from titles.cpp
	void endGame();
	void monkSpeaking();
	void gettingShot();
	void bibleQuote();
	void hangOne(uint16 delay);
	void intro();
	void runIntroSeq();
	void runEndSeq();
	void loadIntroRoom();
	void set16ColPalette();
	void realCredits();
	uint8 getExPos();

	// from use.cpp
	void useRoutine();
	void useObject();
	void placeFreeObject(uint8 index);
	void removeFreeObject(uint8 index);
	void setupTimedUse(uint16 offset, uint16 countToTimed, uint16 timeCount, byte x, byte y);
	void withWhat();
	uint16 checkInside(uint16 command, uint16 type);
	void showPuzText(uint16 command, uint16 count);
	void useText(const uint8 *string);
	void showFirstUse();
	void showSecondUse();
	void viewFolder();
	void edensCDPlayer();
	void hotelBell();
	void playGuitar();
	void useElevator1();
	void useElevator2();
	void useElevator3();
	void useElevator4();
	void useElevator5();
	void useHatch();
	void wheelSound();
	void callHotelLift();
	void useShield();
	void useCoveredBox();
	void useRailing();
	void useChurchHole();
	void sitDownInBar();
	void useBalcony();
	void useWindow();
	void trapDoor();
	void useDryer();
	void callEdensDLift();
	void callEdensLift();
	void openYourNeighbour();
	void openRyan();
	void openPoolBoss();
	void openEden();
	void openSarters();
	void openLouis();
	void useWall();
	void useChurchGate();
	void useLadder();
	void useLadderB();
	bool defaultUseHandler(const char *id);
	void slabDoorA();
	void slabDoorB();
	void slabDoorC();
	void slabDoorE();
	void slabDoorD();
	void slabDoorF();
	void useGun();
	void useFullCart();
	void useClearBox();
	void openTVDoor();
	void usePlate();
	void usePlinth();
	void useElvDoor();
	void useWinch();
	void useCart();
	void useHole();
	void openHotelDoor();
	void openHotelDoor2();
	void grafittiDoor();
	void useCardReader1();
	void useCardReader2();
	void useCardReader3();
	void usePoolReader();
	void useLighter();
	void useWire();
	void openTomb();
	void hotelControl();
	void useCooker();
	void useDiary();
	void useControl();
	void useSlab();
	void usePipe();
	void useOpenBox();
	void runTap();
	void useAxe();
	void useHandle();
	void useAltar();
	void notHeldError();
	void useCashCard();
	void useButtonA();
	void wearWatch();
	void wearShades();
	void useTrainer();
	void useStereo();
	void chewy();
	void delEverything();
	void afterIntroRoom();

	// from vgafades.cpp
	void clearStartPal();
	void clearEndPal();
	void palToStartPal();
	void endPalToStart();
	void startPalToEnd();
	void palToEndPal();
	void fadeDOS();
	void doFade();
	void fadeCalculation();
	void fadeUpYellows();
	void fadeUpMonFirst();
	void fadeUpMon();
	void fadeDownMon();
	void initialMonCols();
	void fadeScreenUp();
	void fadeScreenUps();
	void fadeScreenUpHalf();
	void fadeScreenDown();
	void fadeScreenDowns();
	void fadeScreenDownHalf();
	void clearPalette();
	void greyscaleSum();
	void allPalette();
	void dumpCurrent();

	// from vgagrafx.cpp
	inline uint8 *workspace() { return _workspace; }
	void clearWork();

	void panelToMap();
	void mapToPanel();
	void dumpMap();
	void transferFrame(uint8 from, uint8 to, uint8 offset);
	void zoom();
	void multiGet(uint8 *dst, uint16 x, uint16 y, uint8 width, uint8 height);
	void multiPut(const uint8 *src, uint16 x, uint16 y, uint8 width, uint8 height);
	void multiDump(uint16 x, uint16 y, uint8 width, uint8 height);
	void workToScreen();
	void frameOutV(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, int16 x, int16 y);
	void frameOutNm(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameOutBh(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameOutFx(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void doShake();
	void setMode();
	void showPCX(const Common::String &suffix);
	void showFrameInternal(const uint8 *pSrc, uint16 x, uint16 y, uint8 effectsFlag, uint8 width, uint8 height);
	void showFrame(const GraphicsFile &frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height);
	void showFrame(const GraphicsFile &frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag);
	bool pixelCheckSet(const ObjPos *pos, uint8 x, uint8 y);
	void loadPalFromIFF();
	void createPanel();
	void createPanel2();
	void showPanel();
};

} // End of namespace DreamWeb

#endif
