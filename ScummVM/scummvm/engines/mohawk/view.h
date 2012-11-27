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

#ifndef MOHAWK_VIEW_H
#define MOHAWK_VIEW_H

#include "mohawk/mohawk.h"
#include "common/rect.h"

namespace Mohawk {

class GraphicsManager;

class Feature;
class View;

enum {
	kFeatureObjectMask = 0xff, // both (sort of)
	kFeatureOldSortForeground = 0x1000, // old
	kFeatureOldDropSpot = 0x2000, // old
	kFeatureOldNoClip = 0x4000, // old
	kFeatureNewSortForeground = 0x4000, // new
	kFeatureSortBackground = 0x8000, // both
	kFeatureOldReset = 0x10000, // old
	kFeatureOldDisable = 0x20000, // old
	kFeatureOldAlternateScripts = 0x40000, // old
	kFeatureOldDisableOnReset = 0x80000, // old
	kFeatureDisableOnEnd = 0x100000, // both
	kFeatureNewDisable = 0x200000, // new
	kFeatureNewDisableOnReset = 0x400000, // new
	kFeatureOldAdjustByPos = 0x800000, // old
	kFeatureNewNoLoop = 0x800000, // new
	kFeatureOldDisabled = 0x1000000, // old
	kFeatureOldRandom = 0x2000000, // old
	kFeatureNewClip = 0x2000000, // new
	kFeatureSortStatic = 0x4000000, // both
	kFeatureInternalRegion = 0x8000000, // both
	kFeatureSortCheckRight = 0x10000000, // both
	kFeatureSortCheckTop = 0x20000000, // both
	kFeatureSortCheckLeft = 0x40000000, // both
	kFeatureNewInternalTiming = 0x80000000 // new
};

class Module {
public:
	Module();
	virtual ~Module();

	virtual void init() = 0;
	virtual void shutdown() = 0;
	virtual void update() = 0;

	typedef void (Module::*HotspotProc)(uint);
	typedef void (Module::*FeatureProc)(Feature *);
	typedef bool (Module::*BooleanProc)(Feature *);
	typedef void (Module::*PickupProc)(Feature *, Common::Point, uint32, Common::Rect *);
};

// This is memcpy()ed around, beware.
#define FEATURE_BITMAP_ITEMS 48 // this is 24 in old
struct FeatureData {
	uint16 bitmapIds[FEATURE_BITMAP_ITEMS];
	Common::Point bitmapPos[FEATURE_BITMAP_ITEMS];

	uint16 unknown192; // old?

	uint16 scrbIndex;
	uint16 compoundSHAPIndex;
	uint16 endFrame; // old?
	uint16 currFrame; // old?
	uint32 currOffset;

	Common::Rect bounds;

	Common::Point currentPos;
	Common::Point nextPos;

	uint16 unknown220; // old?

	uint16 syncChannel;
	uint16 enabled;
	byte paused; // new
	byte hidden; // new

	uint16 useClipRect;
	Common::Rect clipRect;
};

class Feature {
public:
	Feature(View *view);
	virtual ~Feature();

	virtual void resetFrame() = 0;

	virtual void setNodeDefaults(Feature *prev, Feature *next);
	virtual void resetFeatureScript(uint16 enabled, uint16 scrbId);
	virtual void resetFeature(bool notifyDone, Module::FeatureProc doneProc, uint16 scrbId);

	void hide(bool clip);
	void show();

	void moveAndUpdate(Common::Point newPos);

	void defaultDraw();

	Feature *_next, *_prev;

	Module::FeatureProc _drawProc;
	Module::FeatureProc _moveProc;
	Module::FeatureProc _doneProc;
	Module::FeatureProc _frameProc;
	Module::BooleanProc _timeProc;

	uint16 _region; // TODO
	uint16 _id;
	uint16 _scrbId;
	uint16 _storedScrbId; // old
	uint32 _flags;
	uint32 _nextTime;
	uint32 _delayTime;
	uint16 _dirty; // byte in old
	byte _needsReset;
	byte _justReset; // old
	byte _notifyDone; // old
	byte _done; // new

	FeatureData _data;

protected:
	View *_view;

	virtual void resetScript() = 0;
	virtual void finishResetFeatureScript() = 0;
};

class OldFeature : public Feature {
public:
	OldFeature(View *view);
	~OldFeature();

	void resetFrame();
	void resetFeatureScript(uint16 enabled, uint16 scrbId);

protected:
	void resetScript();
	void finishResetFeatureScript();
};

class NewFeature : public Feature {
public:
	NewFeature(View *view);
	~NewFeature();

	void resetFrame();
	void resetFeatureScript(uint16 enabled, uint16 scrbId);

	uint32 _unknown168;

	// Drag/drop variables.
	Module::PickupProc _pickupProc;
	Module::FeatureProc _dropProc;
	Module::FeatureProc _dragMoveProc;
	Module::FeatureProc _oldMoveProc;
	uint32 _dragFlags;
	uint32 _oldFlags;
	Common::Point _oldPos;
	Common::Point _posDiff;
	Common::Point _currDragPos;

protected:
	void resetScript();
	void finishResetFeatureScript();
};

#define NUM_SYNC_CHANNELS 17
struct SyncChannel {
	uint16 masterId;
	byte state;
	bool alternate;
};

class View {
public:
	View(MohawkEngine *vm);
	virtual ~View();

	virtual void idleView();

	void setModule(Module *module);
	Module *getCurrentModule() { return _currentModule; }
	GraphicsManager *getGfx() { return _gfx; }

	Common::Array<uint16> getSHPL(uint16 id);
	void installBG(uint16 id);
	void setColors(Common::SeekableReadStream *tpalStream);
	void copyFadeColors(uint start, uint count);

	uint16 getCompoundSHAPId(uint16 shapIndex);

	void installGroupOfSCRBs(bool main, uint base, uint size, uint count = 0);
	virtual void freeScripts();
	void installFeatureShapes(bool regs, uint groupId, uint16 resourceBase);
	void freeFeatureShapes();

	uint16 getGroupFromBaseId(uint16 baseId);
	void getnthScriptSetGroup(uint16 &scrbIndex, uint16 &shapIndex, uint16 scrbId);
	Common::SeekableReadStream *getSCRB(uint16 index, uint16 id = 0xffff);

	Feature *getFeaturePtr(uint16 id);
	uint16 getNewFeatureId();
	void removeFeature(Feature *feature, bool free);
	void insertUnderCursor(Feature *feature);
	Feature *pointOnFeature(bool topdown, uint32 flags, Common::Point pos);
	void sortView();

	uint32 _lastIdleTime;
	SyncChannel _syncChannels[NUM_SYNC_CHANNELS];

	virtual uint32 getTime() = 0;

	bool _needsUpdate;

protected:
	MohawkEngine *_vm;
	GraphicsManager *_gfx;
	void setGraphicsManager(GraphicsManager *gfx) { _gfx = gfx; } // TODO
	Module *_currentModule;

	uint16 _backgroundId;

	Feature *_rootNode, *_cursorNode;

	uint16 _numSCRBGroups;
	uint16 _SCRBGroupBases[14];
	uint16 _SCRBGroupSizes[14];
	Common::Array<uint16> _SCRBEntries;
	//uint16 _numCompoundSHAPGroups;
	uint16 _compoundSHAPGroups[14];

	Feature *sortOneList(Feature *root);
	Feature *mergeLists(Feature *root, Feature *mergeRoot);

	virtual void finishDraw() { }
};

} // End of namespace Mohawk

#endif
