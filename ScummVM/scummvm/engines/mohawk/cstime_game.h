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

#ifndef MOHAWK_CSTIME_GAME_H
#define MOHAWK_CSTIME_GAME_H

#include "mohawk/cstime.h"
#include "mohawk/sound.h" // CueList

namespace Mohawk {

class Feature;
class CSTimeCase;
class CSTimeScene;

class Region {
public:
	void loadFrom(Common::SeekableReadStream *stream);
	bool containsPoint(Common::Point &pos) const;

	Common::Array<Common::Rect> _rects;
};

struct CSTimeAmbient {
	uint16 delay;
	Feature *feature;
	uint32 nextTime;
};

class CSTimeChar {
public:
	CSTimeChar(MohawkEngine_CSTime *vm, CSTimeScene *scene, uint id);
	~CSTimeChar();

	void idle();
	void setupAmbientAnims(bool onetime);
	void idleAmbients();
	void stopAmbients(bool restpos);
	void setupRestPos();
	void removeChr();
	uint16 getChrBaseId();
	uint getScriptCount();
	void playNIS(uint16 id);
	bool NISIsDone();
	void removeNIS();
	void startFlapping(uint16 id);
	void interruptFlapping();

	uint16 _unknown1, _unknown2, _unknown3;
	Common::Array<CSTimeAmbient> _ambients;
	bool _enabled;
	uint16 _flappingState;

protected:
	MohawkEngine_CSTime *_vm;
	CSTimeScene *_scene;
	uint _id;

	Feature *_NIS;
	Feature *_restFeature;
	Feature *_talkFeature;
	Feature *_talkFeature1, *_talkFeature2, *_talkFeature3;

	uint16 _playingWaveId;
	CueList _cueList;
	uint _nextCue;
	uint32 _lastTime1, _lastTime2, _lastTime3;

	bool _resting;
	byte _waveStatus;
	byte _surfingState;

	void installAmbientAnim(uint id, uint32 flags);
	uint16 getChrTypeScriptBase();
	void playFlapWave(uint16 id);
	void updateWaveStatus();
	void setupTalk();
	void idleTalk();
	void stopFlapping();
};

struct CSTimeHotspot {
	uint16 stringId;
	uint16 state;
	uint16 invObjId;
	uint16 cursor;
	Common::Array<CSTimeEvent> events;
	Region region;
};

struct CSTimeLocation {
	uint16 sceneId, hotspotId;
};

struct CSTimeInventoryHotspot {
	uint16 sceneId, hotspotId, stringId;
	Common::Array<CSTimeEvent> events;
};

#define TIME_CUFFS_ID 0
struct CSTimeInventoryObject {
	uint16 id, stringId, hotspotId, featureId, canTake;
	Feature *feature;
	bool featureDisabled;
	Common::Array<CSTimeLocation> locations;
	Common::Array<CSTimeInventoryHotspot> hotspots;
	Common::Array<CSTimeEvent> events;
};

struct CSTimeQaR {
	bool finished;
	uint16 id;
	uint16 unknown1;
	uint16 questionStringId;
	uint16 responseStringId;
	uint16 unknown2;
	uint16 nextQaRsId;
	Common::Array<CSTimeEvent> events;
};

class CSTimeConversation {
public:
	CSTimeConversation(MohawkEngine_CSTime *vm, uint id);
	~CSTimeConversation();

	void start();
	void finishProcessingQaR();
	void end(bool useLastClicked, bool runEvents = true);
	void display();
	void selectItemsToDisplay();

	void mouseDown(Common::Point &pos);
	void mouseMove(Common::Point &pos);
	void mouseUp(Common::Point &pos);

	void incrementTalkCount() { _talkCount++; }
	uint16 getNameId() { return _nameId; }
	uint16 getSourceChar() { return _sourceChar; }
	void setSourceChar(uint16 source) { _sourceChar = source; }
	void setAsked(uint qar, uint entry);
	bool getAsked(uint qar, uint entry) { return _asked[qar][entry]; }

	void setState(uint state) { _state = state; }
	uint getState() { return _state; }

protected:
	MohawkEngine_CSTime *_vm;
	uint _id;

	uint _state;

	uint16 _nameId;
	uint16 _greeting, _greeting2;

	uint _talkCount;
	uint16 _sourceChar;

	uint16 _currEntry, _currHover;
	uint16 _nextToProcess;

	bool _asked[8][5];
	Common::Array<CSTimeQaR> _qars;
	Common::Array<uint> _itemsToDisplay;

	void clear();
	void loadQaR(CSTimeQaR &qar, uint16 id);

	void highlightLine(uint line);
	void unhighlightLine(uint line);
};

class CSTimeScene {
public:
	CSTimeScene(MohawkEngine_CSTime *vm, CSTimeCase *case_, uint id);
	~CSTimeScene();

	void installGroup();
	void buildScene();
	void leave();
	uint16 getSceneId();
	void mouseDown(Common::Point &pos);
	void mouseMove(Common::Point &pos);
	void mouseUp(Common::Point &pos);
	void idle();
	void setupAmbientAnims();
	void idleAmbientAnims();
	bool eventIsActive();

	void setCursorForCurrentPoint();
	void drawHotspots(); // debugging

	uint16 getBubbleType() { return _bubbleType; }
	const Common::Array<CSTimeEvent> &getEvents(bool second);
	const Common::Array<CSTimeHotspot> &getHotspots() { return _hotspots; }
	CSTimeHotspot &getHotspot(uint id) { return _hotspots[id]; }
	uint16 getInvObjForCurrentHotspot() { return _hotspots[_currHotspot].invObjId; }
	CSTimeChar *getChar(uint id) { return _chars[id]; }
	uint16 getHelperId() { return _helperId; }
	uint getId() { return _id; }

	uint _visitCount;
	CSTimeChar *_activeChar;

	Common::Array<Feature *> _objectFeatures;

protected:
	MohawkEngine_CSTime *_vm;
	CSTimeCase *_case;
	uint _id;

	uint16 _currHotspot;
	uint16 _hoverHotspot;

	void load();
	void cursorOverHotspot(uint id);
	void mouseDownOnHotspot(uint id);
	void mouseUpOnHotspot(uint id);
	bool hotspotContainsEvent(uint id, uint16 eventType);

	uint16 _unknown1, _unknown2, _helperId;
	uint16 _bubbleType;
	uint16 _numObjects;
	Common::Array<CSTimeEvent> _events, _events2;
	Common::Array<CSTimeChar *> _chars;
	Common::Array<CSTimeHotspot> _hotspots;
};

class CSTimeCase {
public:
	CSTimeCase(MohawkEngine_CSTime *vm, uint id);
	virtual ~CSTimeCase();

	uint getId() { return _id; }
	Common::String &getRolloverText(uint id) { return _rolloverText[id]; }
	CSTimeScene *getCurrScene();
	void setCurrScene(uint16 id) { _currScene = id; }
	void setConversation(uint id) { _currConv = _conversations[id]; }
	CSTimeConversation *getCurrConversation() { return _currConv; }
	uint16 getNoteFeatureId(uint16 id) { return _noteFeatureId[id]; }

	// Hard-coded per-case logic, implemented in subclasses.
	virtual bool checkConvCondition(uint16 conditionId) = 0;
	virtual bool checkAmbientCondition(uint16 charId, uint16 ambientId) = 0;
	virtual bool checkObjectCondition(uint16 objectId) = 0;
	virtual void selectHelpStrings() = 0;
	virtual void handleConditionalEvent(const CSTimeEvent &event) = 0;

	Common::Array<CSTimeInventoryObject *> _inventoryObjs;

protected:
	MohawkEngine_CSTime *_vm;
	uint _id;

	uint16 _currScene;
	uint16 _noteFeatureId[3];
	Common::Array<Common::String> _rolloverText;
	Common::Array<CSTimeScene *> _scenes;
	Common::Array<CSTimeConversation *> _conversations;
	CSTimeConversation *_currConv;

	void loadRolloverText();
	CSTimeInventoryObject *loadInventoryObject(uint id);
};

} // End of namespace Mohawk

#endif
