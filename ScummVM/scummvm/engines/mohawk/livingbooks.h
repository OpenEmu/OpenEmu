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

#ifndef MOHAWK_LIVINGBOOKS_H
#define MOHAWK_LIVINGBOOKS_H

#include "mohawk/mohawk.h"
#include "mohawk/console.h"
#include "mohawk/livingbooks_graphics.h"
#include "mohawk/sound.h"

#include "common/config-file.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/random.h"

#include "livingbooks_code.h"

namespace Common {
	class SeekableSubReadStreamEndian;
	class MemoryReadStreamEndian;
}

namespace Mohawk {

#define LBKEY_MOD_CTRL 1
#define LBKEY_MOD_ALT 2
#define LBKEY_MOD_SHIFT 4

struct LBKey {
	byte code;
	byte modifiers;
	byte char_;
	byte repeats;
};

enum NodeState {
	kLBNodeDone = 0,
	kLBNodeRunning = 1,
	kLBNodeWaiting = 2
};

enum LBMode {
	kLBIntroMode = 1,
	kLBControlMode = 2,
	kLBCreditsMode = 3,
	kLBPreviewMode = 4,
	kLBReadMode = 5,
	kLBPlayMode = 6
};

enum {
	kLBPhaseInit = 0x0,
	kLBPhaseIntro = 0x1,
	kLBPhaseMain = 0x2,
	kLBPhaseNone = 0x7fff,
	kLBPhaseLoad = 0xfffe,
	kLBPhaseCreate = 0xffff
};

// automatic modes used in _timingMode
enum {
	kLBAutoNone = 0,
	kLBAutoIntro = 1,
	kLBAutoUserIdle = 2,
	kLBAutoMain = 3,
	kLBAutoSync = 4,
	kLBAutoInit = 5,
	kLBAutoCreate = 6,
	kLBAutoLoad = 7
};

// control modes used in _controlMode
enum {
	kLBControlNone = 0,
	kLBControlHideMouse = 1,
	kLBControlPauseItems = 2
};

enum {
	kLBStaticTextItem = 0x1,
	kLBPictureItem = 0x2,
	kLBEditTextItem = 0x14,
	kLBLiveTextItem = 0x15,
	kLBAnimationItem = 0x40,
	kLBSoundItem = 0x41,
	kLBGroupItem = 0x42,
	kLBMovieItem = 0x43,
	kLBPaletteAItem = 0x44, // unused?
	kLBPaletteItem = 0x45,
	kLBProxyItem = 0x46,
	kLBMiniGameItem = 666, // EVIL!!!!
	kLBXDataFileItem = 0x3e9,
	kLBDiscDectectorItem = 0xfa1
};

enum {
	// no 0x1?
	kLBAnimOpNotify = 0x2,
	kLBAnimOpSetTempo = 0x3,
	// no 0x4?
	kLBAnimOpMoveTo = 0x5,
	kLBAnimOpWait = 0x6,
	kLBAnimOpSetCel = 0x7,
	kLBAnimOpSleepUntil = 0x8,
	kLBAnimOpDrawMode = 0x9,
	kLBAnimOpPlaySound = 0xa,
	kLBAnimOpWaitForSound = 0xb,
	kLBAnimOpReleaseSound = 0xc,
	kLBAnimOpResetSound = 0xd,
	kLBAnimOpSetTempoDiv = 0xe,
	kLBAnimOpDelay = 0xf
};

enum {
	kLBEventPhaseInit = 0,
	kLBEventPhaseIntro = 1,
	kLBEventMouseDown = 2,
	kLBEventStarted = 3,
	kLBEventDone = 4,
	kLBEventMouseUp = 5,
	kLBEventPhaseMain = 6,
	kLBEventNotified = 7,
	kLBEventDragStart = 8,
	kLBEventDragMove = 9,
	kLBEventDragEnd = 0xa,
	kLBEventRolloverBegin = 0xb,
	kLBEventRolloverMove = 0xc,
	kLBEventRolloverEnd = 0xd,
	kLBEventMouseUpIn = 0xe,
	kLBEventMouseUpOut = 0xf,
	kLBEventMouseTrackIn = 0x10,
	kLBEventMouseTrackMove = 0x11,
	kLBEventMouseTrackMoveIn = 0x12,
	kLBEventMouseTrackMoveOut = 0x13,
	kLBEventMouseTrackOut = 0x14,
	kLBEventFocusBegin = 0x15,
	kLBEventFocusEnd = 0x16,
	kLBEventInit = 0x17,
	kLBEventLoad = 0x1a,
	kLBEventListLoad = 0x1b,
	kLBEventPhaseCreate = 0xff
};

enum {
	kLBGroupData = 0x64,
	kLBLiveTextData = 0x65,
	kLBMsgListScript = 0x66,
	kLBNotifyScript = 0x67,
	kLBSetPlayInfo = 0x68,
	kLBSetRandomLoc = 0x69,  // unused?
	kLBSetDrag = 0x6a,       // unused?
	kLBSetDrawMode = 0x6b,
	kLBSetFont = 0x6c,       // unused?
	kLBSetOneShot = 0x6d,    // unused?
	kLBSetPlayPhase = 0x6e,
	// from here, 2.x+
	kLBSetKeyNotify = 0x6f,
	kLBCommand = 0x70,
	kLBPaletteAData = 0x71,  // unused?
	kLBPaletteXData = 0x72,
	kLBDisable = 0x73,       // unused?
	kLBEnable = 0x74,        // unused?
	kLBSetNotVisible = 0x75,
	kLBSetVisible = 0x76,    // unused?
	kLBGlobalDisable = 0x77,
	kLBGlobalEnable = 0x78,  // unused?
	kLBGlobalSetNotVisible = 0x79,
	kLBGlobalSetVisible = 0x7a, // unused?
	kLBSetAmbient = 0x7b,
	kLBSetDragParams = 0x7c,
	kLBSetKeyEvent = 0x7d,
	kLBSetRolloverData = 0x7e,
	kLBSetParent = 0x7f,
	kLBSetHitTest = 0x80,
	// from here, rugrats
	kLBUnknown194 = 0x194
};

enum {
	kLBOpNone = 0x0,
	kLBOpXShow = 0x1,
	kLBOpTogglePlay = 0x2,
	kLBOpSetNotVisible = 0x3,
	kLBOpSetVisible = 0x4,
	kLBOpDestroy = 0x5,
	kLBOpRewind = 0x6,
	kLBOpStop = 0x7,
	kLBOpDisable = 0x8,
	kLBOpEnable = 0x9,
	// (no 0xa)
	kLBOpGlobalSetNotVisible = 0xb,
	kLBOpGlobalSetVisible = 0xc,
	kLBOpGlobalDisable = 0xd,
	kLBOpGlobalEnable = 0xe,
	kLBOpSeekToEnd = 0xf,
	// (no 0x10)
	kLBOpMute = 0x11,
	kLBOpUnmute = 0x12,
	kLBOpLoad = 0x13,
	kLBOpPreload = 0x14,
	kLBOpUnload = 0x15,
	kLBOpSeekToNext = 0x16,
	kLBOpSeekToPrev = 0x17,
	kLBOpDragBegin = 0x18,
	kLBOpDragEnd = 0x19,
	kLBOpScriptDisable = 0x1a,
	kLBOpScriptEnable = 0x1b,
	kLBOpUnknown1C = 0x1c,
	kLBOpSendExpression = 0x1d,
	kLBOpJumpUnlessExpression = 0xfffb,
	kLBOpBreakExpression = 0xfffc,
	kLBOpJumpToExpression = 0xfffd,
	kLBOpRunSubentries = 0xfffe,
	kLBOpRunData = 0xffff
};

enum {
	kLBNotifyGUIAction = 1,
	kLBNotifyGoToControls = 2,
	kLBNotifyChangePage = 3,
	kLBNotifyGotoQuit = 4,
	kLBNotifyIntroDone = 5,
	kLBNotifyChangeMode = 6,
	kLBNotifyCursorChange = 7,
	kLBNotifyPrintPage = 0xc,
	kLBNotifyQuit = 0xd
};

enum {
	kTargetTypeExpression = 0x3f3f,
	kTargetTypeCode = 0xfffe,
	kTargetTypeName = 0xffff
};

class MohawkEngine_LivingBooks;
class LBPage;
class LBGraphics;
class LBAnimation;

struct LBScriptEntry {
	LBScriptEntry();
	~LBScriptEntry();

	uint16 state;

	uint16 type;
	uint16 event;
	uint16 opcode;
	uint16 param;

	uint16 argc;
	uint16 *argvParam;
	uint16 *argvTarget;

	uint16 targetingType;
	Common::Array<Common::String> targets;

	// kLBNotifyChangeMode
	uint16 newUnknown;
	uint16 newMode;
	uint16 newPage;
	uint16 newSubpage;
	Common::String newCursor;

	// kLBEventNotified
	uint16 matchFrom;
	uint16 matchNotify;

	// kLBOpSendExpression
	uint32 offset;
	// kLBOpJumpUnlessExpression
	uint16 target;

	uint16 dataType;
	uint16 dataLen;
	byte *data;

	Common::Array<Common::String> conditions;
	Common::Array<LBScriptEntry *> subentries;
};

struct LBAnimScriptEntry {
	byte opcode;
	byte size;
	byte *data;
};

class LBAnimationNode {
public:
	LBAnimationNode(MohawkEngine_LivingBooks *vm, LBAnimation *parent, uint16 scriptResourceId);
	~LBAnimationNode();

	void draw(const Common::Rect &_bounds);
	void reset();
	NodeState update(bool seeking = false);
	bool transparentAt(int x, int y);

protected:
	MohawkEngine_LivingBooks *_vm;
	LBAnimation *_parent;

	void loadScript(uint16 resourceId);
	uint _currentEntry;
	Common::Array<LBAnimScriptEntry> _scriptEntries;

	uint _currentCel;
	int16 _xPos, _yPos;
	uint32 _delay;
};

class LBAnimationItem;

class LBAnimation {
public:
	LBAnimation(MohawkEngine_LivingBooks *vm, LBAnimationItem *parent, uint16 resourceId);
	~LBAnimation();

	void draw();
	bool update();

	void start();
	void seek(uint16 pos);
	void seekToTime(uint32 time);
	void stop();

	void playSound(uint16 resourceId);
	bool soundPlaying(uint16 resourceId, const Common::String &cue);

	bool transparentAt(int x, int y);

	void setTempo(uint16 tempo);

	uint getNumResources() { return _shapeResources.size(); }
	uint16 getResource(uint num) { return _shapeResources[num]; }
	Common::Point getOffset(uint num) { return _shapeOffsets[num]; }

	uint32 getCurrentFrame() { return _currentFrame; }

	uint16 getParentId();

protected:
	MohawkEngine_LivingBooks *_vm;
	LBAnimationItem *_parent;

	Common::Rect _bounds, _clip;
	Common::Array<LBAnimationNode *> _nodes;

	uint16 _tempo;

	uint16 _currentSound;
	CueList _cueList;

	uint32 _lastTime, _currentFrame;
	bool _running;

	void loadShape(uint16 resourceId);
	Common::Array<uint16> _shapeResources;
	Common::Array<Common::Point> _shapeOffsets;
};

class LBItem {
	friend class LBCode;

public:
	LBItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect);
	virtual ~LBItem();

	void readFrom(Common::SeekableSubReadStreamEndian *stream);
	void readData(uint16 type, uint16 size, byte *data);
	virtual void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream);

	virtual void destroySelf(); // 0x2
	virtual void setEnabled(bool enabled); // 0x3
	virtual void setGlobalEnabled(bool enabled);
	virtual bool contains(Common::Point point); // 0x7
	virtual void update(); // 0x8
	virtual void draw() { } // 0x9
	virtual void handleKeyChar(Common::Point pos) { } // 0xA
	virtual void handleMouseDown(Common::Point pos); // 0xB
	virtual void handleMouseMove(Common::Point pos); // 0xC
	virtual void handleMouseUp(Common::Point pos); // 0xD
	virtual bool togglePlaying(bool playing, bool restart = false); // 0xF
	virtual void done(bool onlyNotify); // 0x10
	virtual void init(); // 0x11
	virtual void seek(uint16 pos) { } // 0x13
	virtual void seekToTime(uint32 time) { }
	virtual void setFocused(bool focused) { } // 0x14
	virtual void setVisible(bool visible); // 0x17
	virtual void setGlobalVisible(bool enabled);
	virtual void startPhase(uint phase); // 0x18
	virtual void stop(); // 0x19
	virtual void notify(uint16 data, uint16 from); // 0x1A
	virtual void load();
	virtual void unload();
	virtual void moveBy(const Common::Point &pos);
	virtual void moveTo(const Common::Point &pos);

	LBItem *clone(uint16 newId, const Common::String &newName);

	uint16 getId() { return _itemId; }
	const Common::String &getName() { return _desc; }
	const Common::Rect &getRect() { return _rect; }
	uint16 getSoundPriority() { return _soundMode; }
	bool isLoaded() { return _loaded; }
	bool isAmbient() { return _isAmbient; }

	Common::List<LBItem *>::iterator _iterator;

	// TODO: make private
	Common::HashMap<Common::String, LBValue, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;

protected:
	MohawkEngine_LivingBooks *_vm;
	LBPage *_page;

	void setNextTime(uint16 min, uint16 max);
	void setNextTime(uint16 min, uint16 max, uint32 start);

	Common::Rect _rect;
	Common::String _desc;
	uint16 _resourceId;
	uint16 _itemId;

	bool _loaded, _visible, _globalVisible, _playing, _enabled, _globalEnabled;

	uint32 _nextTime, _startTime;
	uint16 _loops;

	uint16 _phase, _timingMode, _delayMin, _delayMax;
	uint16 _loopMode, _periodMin, _periodMax;
	uint16 _controlMode, _soundMode;
	Common::Point _relocPoint;

	bool _isAmbient;
	bool _doHitTest;

	virtual LBItem *createClone();

	Common::Array<LBScriptEntry *> _scriptEntries;
	void runScript(uint event, uint16 data = 0, uint16 from = 0);
	int runScriptEntry(LBScriptEntry *entry);

	void runCommand(const Common::String &command);
	bool checkCondition(const Common::String &condition);

	LBScriptEntry *parseScriptEntry(uint16 type, uint16 &size, Common::MemoryReadStreamEndian *stream, bool isSubentry = false);
};

class LBSoundItem : public LBItem {
public:
	LBSoundItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBSoundItem();

	void update();
	bool togglePlaying(bool playing, bool restart);
	void stop();

protected:
	LBItem *createClone();

	bool _running;
};

struct GroupEntry {
	uint entryId;
	uint entryType;
};

class LBGroupItem : public LBItem {
public:
	LBGroupItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream);

	void destroySelf();
	void setEnabled(bool enabled);
	void setGlobalEnabled(bool enabled);
	bool contains(Common::Point point);
	bool togglePlaying(bool playing, bool restart);
	// 0x12
	void seek(uint16 pos);
	void setVisible(bool visible);
	void setGlobalVisible(bool visible);
	void startPhase(uint phase);
	void stop();
	void load();
	void unload();
	void moveBy(const Common::Point &pos);
	void moveTo(const Common::Point &pos);

protected:
	LBItem *createClone();

	bool _starting;

	Common::Array<GroupEntry> _groupEntries;
};

class LBPaletteItem : public LBItem {
public:
	LBPaletteItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBPaletteItem();

	void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream);

	bool togglePlaying(bool playing, bool restart);
	void update();

protected:
	LBItem *createClone();

	uint16 _fadeInPeriod, _fadeInStep, _drawStart, _drawCount;
	uint32 _fadeInStart, _fadeInCurrent;
	byte *_palette;
};

struct LiveTextWord {
	Common::Rect bounds;
	uint16 soundId;

	uint16 itemType;
	uint16 itemId;
};

struct LiveTextPhrase {
	uint16 wordStart, wordCount;
	uint16 highlightStart, highlightEnd;
	uint16 startId, endId;
};

class LBLiveTextItem : public LBItem {
public:
	LBLiveTextItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream);

	bool contains(Common::Point point);
	void update();
	void draw();
	void handleMouseDown(Common::Point pos);
	bool togglePlaying(bool playing, bool restart);
	void stop();
	void notify(uint16 data, uint16 from);

protected:
	LBItem *createClone();

	void paletteUpdate(uint16 word, bool on);
	void drawWord(uint word, uint yPos);

	uint16 _currentPhrase, _currentWord;

	byte _backgroundColor[4];
	byte _foregroundColor[4];
	byte _highlightColor[4];
	uint16 _paletteIndex;

	Common::Array<LiveTextWord> _words;
	Common::Array<LiveTextPhrase> _phrases;
};

class LBPictureItem : public LBItem {
public:
	LBPictureItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream);

	bool contains(Common::Point point);
	void draw();
	void init();

protected:
	LBItem *createClone();
};

class LBAnimationItem : public LBItem {
public:
	LBAnimationItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBAnimationItem();

	void setEnabled(bool enabled);
	bool contains(Common::Point point);
	void update();
	void draw();
	bool togglePlaying(bool playing, bool restart);
	void done(bool onlyNotify);
	void init();
	void seek(uint16 pos);
	void seekToTime(uint32 time);
	void startPhase(uint phase);
	void stop();

protected:
	LBItem *createClone();

	LBAnimation *_anim;
	bool _running;
};

class LBMovieItem : public LBItem {
public:
	LBMovieItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBMovieItem();

	void update();
	bool togglePlaying(bool playing, bool restart);

protected:
	LBItem *createClone();
};

class LBMiniGameItem : public LBItem {
public:
	LBMiniGameItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBMiniGameItem();

	bool togglePlaying(bool playing, bool restart);

protected:
	LBItem *createClone();
};

class LBProxyItem : public LBItem {
public:
	LBProxyItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBProxyItem();

	void load();
	void unload();

protected:
	LBItem *createClone();

	class LBPage *_page;
};

struct NotifyEvent {
	NotifyEvent(uint t, uint p) : type(t), param(p), newUnknown(0), newMode(0), newPage(0), newSubpage(0) { }
	uint type;
	uint param;

	// kLBNotifyChangeMode
	uint16 newUnknown;
	uint16 newMode;
	uint16 newPage;
	uint16 newSubpage;
	Common::String newCursor;
};

enum DelayedEventType {
	kLBDelayedEventDestroy = 0,
	kLBDelayedEventSetNotVisible = 1,
	kLBDelayedEventDone = 2
};

struct DelayedEvent {
	DelayedEvent(LBItem *i, DelayedEventType t) : item(i), type(t) { }
	LBItem *item;
	DelayedEventType type;
};

class LBPage {
public:
	LBPage(MohawkEngine_LivingBooks *vm);
	~LBPage();

	void open(Archive *mhk, uint16 baseId);
	uint16 getResourceVersion();

	void addClonedItem(LBItem *item);
	void itemDestroyed(LBItem *item);

	LBCode *_code;

protected:
	MohawkEngine_LivingBooks *_vm;

	Archive *_mhk;
	Common::Array<LBItem *> _items;

	uint16 _baseId;
	bool _cascade;

	void loadBITL(uint16 resourceId);
};

class MohawkEngine_LivingBooks : public MohawkEngine {
protected:
	Common::Error run();

public:
	MohawkEngine_LivingBooks(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_LivingBooks();

	Common::RandomSource *_rnd;

	LBGraphics *_gfx;
	bool _needsRedraw, _needsUpdate;

	void addNotifyEvent(NotifyEvent event);

	Common::SeekableSubReadStreamEndian *wrapStreamEndian(uint32 tag, uint16 id);
	Common::String readString(Common::ReadStream *stream);
	Common::Rect readRect(Common::ReadStreamEndian *stream);
	GUI::Debugger *getDebugger() { return _console; }

	void addArchive(Archive *archive);
	void removeArchive(Archive *archive);
	void addItem(LBItem *item);
	void removeItems(const Common::Array<LBItem *> &items);

	LBItem *getItemById(uint16 id);
	LBItem *getItemByName(Common::String name);

	void setFocus(LBItem *focus);
	void setEnableForAll(bool enable, LBItem *except = 0);
	void notifyAll(uint16 data, uint16 from);
	void queueDelayedEvent(DelayedEvent event);

	bool playSound(LBItem *source, uint16 resourceId);
	void lockSound(LBItem *owner, bool lock);

	bool isBigEndian() const { return getGameType() != GType_LIVINGBOOKSV1 || getPlatform() == Common::kPlatformMacintosh; }
	bool isPreMohawk() const;

	LBMode getCurMode() { return _curMode; }

	bool tryLoadPageStart(LBMode mode, uint page);
	bool loadPage(LBMode mode, uint page, uint subpage);
	void prevPage();
	void nextPage();

	// TODO: make private
	Common::HashMap<Common::String, LBValue, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;

	// helper functions, also used by LBProxyItem
	Common::String getFileNameFromConfig(const Common::String &section, const Common::String &key, Common::String &leftover);
	Archive *createArchive() const;

private:
	LivingBooksConsole *_console;
	Common::ConfigFile _bookInfoFile;

	Common::String getBookInfoFileName() const;
	void loadBookInfo(const Common::String &filename);

	Common::String stringForMode(LBMode mode);

	bool _readOnly, _introDone;
	LBMode _curMode;
	uint16 _curPage, _curSubPage;
	uint16 _phase;
	LBPage *_page;
	Common::Array<LBItem *> _items;
	Common::List<LBItem *> _orderedItems;
	Common::Queue<DelayedEvent> _eventQueue;
	LBItem *_focus;
	void destroyPage();
	void updatePage();

	uint16 _lastSoundOwner, _lastSoundId;
	uint16 _lastSoundPriority;
	uint16 _soundLockOwner;
	uint16 _maxSoundPriority;

	void loadSHP(uint16 resourceId);

	bool tryDefaultPage();

	void handleUIMenuClick(uint controlId);
	void handleUIPoetryMenuClick(uint controlId);
	void handleUIQuitClick(uint controlId);
	void handleUIOptionsClick(uint controlId);

	Common::Queue<NotifyEvent> _notifyEvents;
	void handleNotify(NotifyEvent &event);

	uint16 _screenWidth;
	uint16 _screenHeight;
	uint16 _numLanguages;
	uint16 _numPages;
	Common::String _title;
	Common::String _copyright;
	bool _poetryMode;

	uint16 _curLanguage;
	uint16 _curSelectedPage;
	bool _alreadyShowedIntro;

	// String Manipulation Functions
	Common::String removeQuotesFromString(const Common::String &string, Common::String &leftover);
	Common::String convertMacFileName(const Common::String &string);
	Common::String convertWinFileName(const Common::String &string);

	// Configuration File Functions
	Common::String getStringFromConfig(const Common::String &section, const Common::String &key);
	Common::String getStringFromConfig(const Common::String &section, const Common::String &key, Common::String &leftover);
	int getIntFromConfig(const Common::String &section, const Common::String &key);
};

} // End of namespace Mohawk

#endif
