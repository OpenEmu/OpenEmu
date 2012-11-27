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

#ifndef DRASCULA_H
#define DRASCULA_H

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/hash-str.h"
#include "common/keyboard.h"
#include "common/ptr.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/util.h"

#include "audio/mixer.h"

#include "engines/engine.h"

/**
 * This is the namespace of the Drascula engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Drascula: The Vampire Strikes Back
 */
namespace Drascula {

#define DRASCULA_DAT_VER 4
#define DATAALIGNMENT 4

enum DrasculaGameFeatures {
	GF_PACKED = (1 << 0)
};

enum Languages {
	kEnglish = 0,
	kSpanish = 1,
	kGerman = 2,
	kFrench = 3,
	kItalian = 4
};

enum Verbs {
	kVerbDefault = -1,
	kVerbNone = 0,
	kVerbLook = 1,
	kVerbPick = 2,
	kVerbOpen = 3,
	kVerbClose = 4,
	kVerbTalk = 5,
	kVerbMove = 6
};

// Items up to chapter 3
enum InventoryItems {
	kItemMoney = 7,
	kItemLeaves = 8,
	kItemCross = 9,
	kItemSpike = 10,
	kItemEarplugs = 11,
	kItemBook = 12,
	kItemBubbleGum = 13,
	kItemSickle = 14,
	kItemTissues = 15,
	kItemCigarettes = 16,
	kItemCandle = 17,
	kItemTwoCoins = 18,
	kItemOneCoin = 19,
	kItemReefer = 20,
	kItemKey = 21,
	kItemHandbag = 22,
	kItemEarWithEarPlug = 23,
	kItemPhone = 28
};

// Items from chapter 4 onwards
enum InventoryItems2 {
	kItemKey2 = 7,
	kItemCross2 = 9,
	kItemRope2 = 19,
	kItemReefer2 = 20,
	kItemOneCoin2 = 22,
	kItemPhone2 = 28
};

enum Colors {
	kColorBrown = 1,
	kColorDarkBlue = 2,
	kColorLightGreen = 3,
	kColorDarkGreen = 4,
	kColorYellow = 5,
	kColorOrange = 6,
	kColorRed = 7,
	kColorMaroon = 8,
	kColorPurple = 9,
	kColorWhite = 10,
	kColorPink = 11
};

enum SSNFrames {
	kFrameInit = 0,
	kFrameCmpRle = 1,
	kFrameCmpOff = 2,
	kFrameEndAnim = 3,
	kFrameSetPal = 4,
	kFrameMouseKey = 5,		// unused
	kFrameEmptyFrame = 6
};

enum IgorTalkerTypes {
	kIgorDch = 0,
	kIgorFront = 1,
	kIgorDoor = 2,
	kIgorSeated = 3,
	kIgorWig = 4
};

enum VonBraunTalkerTypes {
	kVonBraunNormal = 0,
	kVonBraunDoor = 1
};

enum AnimFrameTypes {
	kFrameBlind = 0,
	kFrameSnore = 1,
	kFrameBat = 2,
	kFrameVonBraun = 3,
	kFramePianist = 4,
	kFrameDrunk = 5,
	kFrameCandles = 6,
	kFramePendulum = 7
};

enum DialogOptionStatus {
	kDialogOptionUnselected = 1,
	kDialogOptionSelected = 2,
	kDialogOptionClicked = 3
};

enum TalkSequenceCommands {
	kPause = 0,
	kSetFlag = 1,
	kClearFlag = 2,
	kPickObject = 3,
	kAddObject = 4,
	kBreakOut = 5,
	kConverse = 6,
	kPlaceVB = 7,
	kUpdateRoom = 8,
	kUpdateScreen = 9,
	kTrackProtagonist = 10,
	kPlaySound = 11,
	kFinishSound = 12,
	kTalkerGeneral = 13,
	kTalkerDrunk = 14,
	kTalkerPianist = 15,
	kTalkerBJ = 16,
	kTalkerVBNormal = 17,
	kTalkerVBDoor = 18,
	kTalkerIgorSeated = 19,
	kTalkerWerewolf = 20,
	kTalkerMus = 21,
	kTalkerDrascula = 22,
	kTalkerBartender0 = 23,
	kTalkerBartender1 = 24
};

enum CharacterDirections {
	kDirectionUp = 0,
	kDirectionDown = 1,
	kDirectionLeft = 2,
	kDirectionRight = 3
};

enum MouseCursors {
	kCursorCrosshair = 0,
	kCursorCurrentItem = 1
};

enum DoorActions {
	kCloseDoor = 0,
	kOpenDoor = 1
};

struct TalkSequenceCommand {
	int chapter;
	int sequence;
	int commandType;
	int action;
};

#define TEXTD_START 68

struct DrasculaGameDescription;

struct RoomTalkAction {
	int room;
	int chapter;
	int action;
	int objectID;
	int speechID;
};

struct RoomUpdate {
	int roomNum;
	int flag;
	int flagValue;
	int sourceX;
	int sourceY;
	int destX;
	int destY;
	int width;
	int height;
	int type;	// 0 - background, 1 - rect
};

struct ItemLocation {
	int x;
	int y;
};

struct CharInfo {
	byte inChar;
	uint16 mappedChar;
	byte charType;	// 0 - letters, 1 - signs, 2 - accented
};

class ArchiveMan : public Common::SearchSet {
public:
	ArchiveMan();

	void enableFallback(bool val) { _fallBack = val; }

	void registerArchive(const Common::String &filename, int priority);

	Common::SeekableReadStream *open(const Common::String &filename);

private:
	bool _fallBack;
};

class TextResourceParser {
	Common::DisposablePtr<Common::SeekableReadStream> _stream;
	int _maxLen;

	void getLine(char *buf);

public:
	TextResourceParser(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose);

	void parseInt(int &result);
	void parseString(char *result);
};


#define NUM_SAVES		10
#define NUM_FLAGS		50
#define DIF_MASK		55
#define OBJWIDTH		40
#define OBJHEIGHT		25

#define DIF_MASK_HARE	72
#define DIF_MASK_ABC	22
#define CHAR_WIDTH		8
#define CHAR_HEIGHT		6

#define TALK_HEIGHT		25
#define TALK_WIDTH		23
#define STEP_X			8
#define STEP_Y			3
#define CHARACTER_HEIGHT	70
#define CHARACTER_WIDTH		43
#define FEET_HEIGHT		12

#define CHAR_WIDTH_OPC	6
#define CHAR_HEIGHT_OPC	5
#define NO_DOOR			99

#define COMPLETE_PAL	256
#define HALF_PAL		128

#define KEYBUFSIZE		16

static const int interf_x[] = { 1, 65, 129, 193, 1, 65, 129 };
static const int interf_y[] = { 51, 51, 51, 51, 83, 83, 83 };

struct RoomHandlers;

class Console;

class DrasculaEngine : public Engine {
protected:
	// Engine APIs
	virtual Common::Error run();

public:
	DrasculaEngine(OSystem *syst, const DrasculaGameDescription *gameDesc);
	virtual ~DrasculaEngine();
	virtual bool hasFeature(EngineFeature f) const;

	Common::RandomSource *_rnd;
	const DrasculaGameDescription *_gameDescription;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	void updateEvents();

	void loadArchives();

	Audio::SoundHandle _soundHandle;

	void allocMemory();
	void freeMemory();
	void endChapter();

	void loadPic(int roomNum, byte *targetSurface, int colorCount = 1) {
		char rm[20];
		sprintf(rm, "%i.alg", roomNum);
		loadPic(rm, targetSurface, colorCount);
	}

	void loadPic(const char *NamePcc, byte *targetSurface, int colorCount = 1);

	typedef signed char DacPalette256[256][3];

	void setRGB(byte *pal, int plt);
	void assignPalette(DacPalette256 pal);
	void setDefaultPalette(DacPalette256 pal);
	void setPalette(byte *PalBuf);
	void copyBackground(int xorg, int yorg, int xdes, int ydes, int width,
				int height, byte *src, byte *dest);

	void copyBackground() {
		copyBackground(0, 0, 0, 0, 320, 200, bgSurface, screenSurface);
	}

	void copyRect(int xorg, int yorg, int xdes, int ydes, int width,
				int height, byte *src, byte *dest);
	void updateScreen() {
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
	}
	void updateScreen(int xorg, int yorg, int xdes, int ydes, int width, int height, byte *buffer);
	int checkWrapX(int x) {
		if (x < 0) x += 320;
		if (x > 319) x -= 320;
		return x;
	}
	int checkWrapY(int y) {
		if (y < 0) y += 200;
		if (y > 199) y -= 200;
		return y;
	}

	DacPalette256 gamePalette;
	DacPalette256 defaultPalette;
	DacPalette256 brightPalette;
	DacPalette256 darkPalette;

	byte *crosshairCursor;
	byte *mouseCursor;

	// Graphics buffers/pointers
	byte *bgSurface;
	byte *backSurface;
	byte *cursorSurface;
	byte *drawSurface3;
	byte *drawSurface2;
	byte *tableSurface;
	byte *extraSurface;	// not sure about this one, was "dir_hare_dch"
	byte *screenSurface;
	byte *frontSurface;

	byte cPal[768];

	ArchiveMan _archives;

	int actorFrames[8];

	int previousMusic, roomMusic;
	int roomNumber;
	char roomDisk[20];
	char currentData[20];
	int numRoomObjs;
	char menuBackground[20];

	char objName[30][20];
	char iconName[44][13];

	int objectNum[40], visible[40], isDoor[40];
	int roomObjX[40], roomObjY[40], trackObj[40];
	int inventoryObjects[43];
	char _targetSurface[40][20];
	int _destX[40], _destY[40], trackCharacter_alkeva[40], roomExits[40];
	int x1[40], y1[40], x2[40], y2[40];
	int takeObject, pickedObject;
	bool _subtitlesDisabled;
	bool _menuBar, _menuScreen, _hasName;
	char textName[20];
	int curExcuseLook;
	int curExcuseAction;

	int flags[NUM_FLAGS];

	int frame_y;
	int curX, curY, characterMoved, curDirection, trackProtagonist, num_frame;
	int hare_se_ve;		// TODO: what is this for?
	int roomX, roomY, checkFlags;
	int doBreak;
	int stepX, stepY;
	int curHeight, curWidth, feetHeight;
	int talkHeight, talkWidth;
	int floorX1, floorY1, floorX2, floorY2;
	int lowerLimit, upperLimit;
	int trackFinal, walkToObject;
	int objExit;
	int timeDiff, startTime;
	int hasAnswer;
	int savedTime;
	int breakOut;
	int vonBraunX, trackVonBraun, vonBraunHasMoved;
	float newHeight, newWidth;
	int factor_red[202];
	int color_solo;
	int blinking;
	int igorX, igorY, trackIgor;
	int drasculaX, drasculaY, trackDrascula;
	int bjX, bjY, trackBJ;
	int framesWithoutAction;
	int term_int;
	int currentChapter;
	int loadedDifferentChapter;
	char saveName[13];
	int _color;
	int musicStopped;
	char select[23];
	int selectionMade;
	int mouseX;
	int mouseY;
	int leftMouseButton;
	int rightMouseButton;

	Common::KeyState _keyBuffer[KEYBUFSIZE];
	int _keyBufferHead;
	int _keyBufferTail;

	bool loadDrasculaDat();

	bool runCurrentChapter();
	void black();
	void pickObject(int);
	void walkUp();
	void walkDown();
	void moveVonBraun();
	void placeVonBraun(int pointX);
	void hipo_sin_nadie(int counter);
	void toggleDoor(int nflag, int doorNum, int action);
	void showMap();

	void enterRoom(int);
	void clearRoom();
	void gotoObject(int, int);
	void moveCursor();
	void checkObjects();
	void selectVerbFromBar();
	bool verify1();
	bool verify2();
	Common::KeyCode getScan();
	void addKeyToBuffer(Common::KeyState& key);
	void flushKeyBuffer();
	void selectVerb(int);
	void updateVolume(Audio::Mixer::SoundType soundType, int prevVolume);
	void volumeControls();
	bool saveLoadScreen();
	void loadSaveNames();
	void saveSaveNames();
	void print_abc(const char *, int, int);
	void delay(int ms);
	bool confirmExit();
	void screenSaver();
	void chooseObject(int object);
	void addObject(int);
	int removeObject(int osj);
	void playFLI(const char *filefli, int vel);
	void fadeFromBlack(int fadeSpeed);
	void fadeToBlack(int fadeSpeed);
	signed char adjustToVGA(signed char value);
	void color_abc(int cl);
	bool textFitsCentered(char *text, int x);
	void centerText(const char *,int,int);
	void playSound(int soundNum);
	bool animate(const char *animation, int FPS);
	void pause(int);
	void placeIgor();
	void placeBJ();
	void placeDrascula();

	void talkInit(const char *filename);
	bool isTalkFinished();
	void talk_igor(int, int);
	void talk_drascula(int index, int talkerType = 0);
	void talk_solo(const char *, const char *);
	void talk_bartender(int, int talkerType = 0);
	void talk_pen(const char *, const char *, int);
	void talk_bj_bed(int);
	void talk_htel(int);
	void talk_bj(int);
	void talk_trunk(int);
	void talk(int);
	void talk(const char *, const char *);
	void talk_sync(const char *, const char *, const char *);
	void talk_drunk(int);
	void talk_pianist(int);
	void talk_werewolf(int);
	void talk_mus(int);
	void talk_drascula_big(int);
	void talk_vonBraun(int, int);
	void talk_blind(int);
	void talk_hacker(int);
	void talk_generic(const char* said, const char* filename, int* faces, int faceCount, int* coords, byte* surface);

	void hiccup(int);
	void finishSound();
	void stopSound();
	void playMusic(int p);
	void stopMusic();
	void updateMusic();
	int musicStatus();
	void updateRoom();
	bool loadGame(const char *);
	void updateDoor(int);
	void setPaletteBase(int darkness);
	void updateVisible();
	void startWalking();
	void updateRefresh();
	void updateRefresh_pre();
	void moveCharacters();
	void showMenu();
	void clearMenu();
	void removeObject();
	bool exitRoom(int);
	bool pickupObject();
	bool checkAction(int);
	void setCursor(int cursor);
	void showCursor();
	void hideCursor();
	bool isCursorVisible();
	void enterName();
	bool soundIsActive();
	void waitFrameSSN();
	void mixVideo(byte *OldScreen, byte *NewScreen, uint16 oldPitch);
	void decodeRLE(byte *BufferRLE, byte *MiVideoRLE, uint16 pitch = 320);
	void decodeOffset(byte *BufferOFF, byte *MiVideoOFF, int length);
	int playFrameSSN(Common::SeekableReadStream *stream);

	int FrameSSN;
	int globalSpeed;
	uint32 LastFrame;

	int flag_tv;

	void showFrame(Common::SeekableReadStream *stream, bool firstFrame = false);
	int getTime();
	void reduce_hare_chico(int, int, int, int, int, int, int, byte *, byte *);
	void quadrant_1();
	void quadrant_2();
	void quadrant_3();
	void quadrant_4();
	void saveGame(const char *gameName);
	void increaseFrameNum();
	int whichObject();
	bool checkMenuFlags();
	void setupRoomsTable();
	void freeRoomsTable();
	bool roomParse(int, int);
	void cleanupString(char *string);
	void playTalkSequence(int sequence);
	void doTalkSequenceCommand(TalkSequenceCommand cmd);
	void converse(int);
	int print_abc_opc(const char *, int, int);
	void response(int);
	void activatePendulum();

	void MusicFadeout();
	void playFile(const char *fname);

	void grr();
	void updateAnim(int y, int destX, int destY, int width, int height, int count, byte* src, int delayVal = 3, bool copyRectangle = false);

	bool room(int rN, int fl);
	bool room_0(int);
	bool room_1(int);
	bool room_2(int);
	bool room_3(int);
	bool room_4(int);
	bool room_5(int);
	bool room_6(int);
	bool room_7(int);
	bool room_8(int);
	bool room_9(int);
	bool room_12(int);
	bool room_13(int);
	bool room_14(int);
	bool room_15(int);
	bool room_16(int);
	bool room_17(int);
	bool room_18(int);
	bool room_21(int);
	bool room_22(int);
	bool room_23(int);
	bool room_24(int);
	bool room_26(int);
	bool room_27(int);
	bool room_29(int);
	bool room_30(int);
	bool room_31(int);
	bool room_34(int);
	bool room_35(int);
	bool room_49(int);
	bool room_53(int);
	bool room_54(int);
	bool room_55(int);
	bool room_56(int);
	bool room_58(int);
	bool room_59(int);
	bool room_60(int);
	bool room_62(int);
	bool room_102(int);

	void animation_1_1();
	void animation_2_1();
	void animation_3_1();
	void animation_4_1();
	//
	void animation_2_2();
	void animation_4_2();
	void animation_5_2();
	void animation_6_2();
	void animation_7_2();
	void animation_11_2();
	void animation_12_2();
	void animation_13_2();
	void animation_14_2();
	void asco();
	void animation_16_2();
	void animation_20_2();
	void animation_23_2();
	void animation_23_joined();
	void animation_23_joined2();
	void animation_24_2();
	void animation_25_2();
	void animation_26_2();
	void animation_27_2();
	void animation_29_2();
	void animation_31_2();
	void animation_32_2();
	void animation_33_2();
	void animation_34_2();
	void animation_35_2();
	void animation_36_2();
	//
	void animation_2_3();
	void animation_3_3();
	void animation_4_3();
	void animation_5_3();
	void animation_6_3();
	void animation_ray();
	//
	void animation_1_4();
	void animation_5_4();
	void animation_6_4();
	void animation_7_4();
	void animation_8_4();
	//
	void animation_1_5();
	void animation_5_5();
	void animation_11_5();
	void animation_12_5();
	void animation_13_5();
	void animation_14_5();
	//
	void animation_1_6();
	void animation_5_6();
	void animation_6_6();
	void animation_9_6();
	void animation_19_6();

	void update_1_pre();
	void update_2();
	void update_3();
	void update_4();
	void update_6_pre();
	void update_9_pre();
	void update_14_pre();
	void update_13();
	void update_16_pre();
	void update_18_pre();
	void update_23_pre();
	void update_26_pre();
	void update_26();
	void update_35_pre();
	void update_58();
	void update_58_pre();
	void update_59_pre();
	void update_60_pre();
	void update_60();
	void update_62();
	void update_62_pre();
	void update_102();

private:
	int _lang;

	Console *_console;

	CharInfo *_charMap;
	int _charMapSize;

	int _itemLocationsSize;
	int _polXSize;
	int _verbBarXSize;
	int _x1dMenuSize;
	int _frameXSize;
	int _candleXSize;
	int _pianistXSize;
	int _drunkXSize;
	int _roomPreUpdatesSize;
	int _roomUpdatesSize;
	int _roomActionsSize;
	int _talkSequencesSize;
	int _numLangs;

	char **_text;
	char **_textd;
	char **_textb;
	char **_textbj;
	char **_texte;
	char **_texti;
	char **_textl;
	char **_textp;
	char **_textt;
	char **_textvb;
	char **_textsys;
	char **_texthis;
	char **_textverbs;
	char **_textmisc;
	char **_textd1;
	ItemLocation *_itemLocations;
	int *_polX, *_polY;
	int *_verbBarX;
	int *_x1d_menu, *_y1d_menu;
	int *_frameX;
	int *_candleX, *_candleY;
	int *_pianistX, *_drunkX;
	RoomUpdate *_roomPreUpdates, *_roomUpdates;
	RoomTalkAction *_roomActions;
	TalkSequenceCommand *_talkSequences;
	char _saveNames[10][23];

	char **loadTexts(Common::File &in);
	void freeTexts(char **ptr);

protected:
	RoomHandlers	*_roomHandlers;
};


} // End of namespace Drascula

#endif /* DRASCULA_H */
