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


#ifndef SCUMM_ACTOR_H
#define SCUMM_ACTOR_H

#include "common/scummsys.h"
#include "scumm/saveload.h"
#include "scumm/scumm.h"


namespace Scumm {

enum {
	V12_X_MULTIPLIER = 8,
	V12_Y_MULTIPLIER = 2,

	V12_X_SHIFT = 3,
	V12_Y_SHIFT = 1
};

enum MoveFlags {
	MF_NEW_LEG = 1,
	MF_IN_LEG = 2,
	MF_TURN = 4,
	MF_LAST_LEG = 8,
	MF_FROZEN = 0x80
};

struct CostumeData {
	byte active[16];
	uint16 animCounter;
	byte soundCounter;
	byte soundPos;
	uint16 stopped;
	uint16 curpos[16];
	uint16 start[16];
	uint16 end[16];
	uint16 frame[16];

	/* HE specific */
	uint16 heJumpOffsetTable[16];
	uint16 heJumpCountTable[16];
	uint32 heCondMaskTable[16];

	void reset() {
		stopped = 0;
		for (int i = 0; i < 16; i++) {
			active[i] = 0;
			curpos[i] = start[i] = end[i] = frame[i] = 0xFFFF;
		}
	}
};

struct AdjustBoxResult {	/* Result type of AdjustBox functions */
	int16 x, y;
	byte box;
};

enum {
	kOldInvalidBox = 255,	// For small header games
	kNewInavlidBox = 0
};

class Actor : public Serializable {
public:
	static byte kInvalidBox;

protected:
	ScummEngine *_vm;

	/** The position of the actor inside the virtual screen. */
	Common::Point _pos;

public:
	int _top, _bottom;
	uint _width;
	byte _number;
	uint16 _costume;
	byte _room;

public:
	byte _talkColor;
	int _talkFrequency;
	byte _talkPan;
	byte _talkVolume;
	uint16 _boxscale;
	byte _scalex, _scaley;
	byte _charset;
	byte _moving;
	bool _ignoreBoxes;
	byte _forceClip;

	byte _initFrame;
	byte _walkFrame;
	byte _standFrame;
	byte _talkStartFrame;
	byte _talkStopFrame;

	bool _needRedraw, _needBgReset, _visible;
	byte _shadowMode;
	bool _flip;
	byte _frame;
	byte _walkbox;
	int16 _talkPosX, _talkPosY;
	uint16 _talkScript, _walkScript;
	bool _ignoreTurns;
	bool _drawToBackBuf;
	int32 _layer;
	uint16 _sound[32];
	CostumeData _cost;

	/* HE specific */
	int _heOffsX, _heOffsY;
	bool _heSkipLimbs;
	uint32 _heCondMask;
	uint32 _hePaletteNum;
	uint32 _heXmapNum;

protected:
	struct ActorWalkData {
		Common::Point dest;           // Final destination point
		byte destbox;                 // Final destination box
		int16 destdir;                // Final destination, direction to face at

		Common::Point cur;            // Last position
		byte curbox;                  // Last box

		Common::Point next;           // Next position on our way to the destination, i.e. our intermediate destination

		Common::Point point3;
		int32 deltaXFactor, deltaYFactor;
		uint16 xfrac, yfrac;
	};


	uint16 _palette[256];
	int _elevation;
	uint16 _facing;
	uint16 _targetFacing;
	uint _speedx, _speedy;
	byte _animProgress, _animSpeed;
	bool _costumeNeedsInit;
	ActorWalkData _walkdata;
	int16 _animVariable[27];

public:

	Actor(ScummEngine *scumm, int id);
	virtual ~Actor() {}

//protected:
	virtual void hideActor();
	void showActor();

	virtual void initActor(int mode);

	void putActor() {
		putActor(_pos.x, _pos.y, _room);
	}

	void putActor(int room) {
		putActor(_pos.x, _pos.y, room);
	}

	void putActor(int x, int y) {
		putActor(x, y, _room);
	}

	void putActor(int x, int y, int room);
	void setActorWalkSpeed(uint newSpeedX, uint newSpeedY);
protected:
	int calcMovementFactor(const Common::Point& next);
	int actorWalkStep();
	int remapDirection(int dir, bool is_walking);
	virtual void setupActorScale();

	void setBox(int box);
	int updateActorDirection(bool is_walking);

public:
	void adjustActorPos();
	virtual AdjustBoxResult adjustXYToBeInBox(int dstX, int dstY);

	virtual void setDirection(int direction);
	void faceToObject(int obj);
	void turnToDirection(int newdir);
	virtual void walkActor();
	void drawActorCostume(bool hitTestMode = false);
	virtual void prepareDrawActorCostume(BaseCostumeRenderer *bcr);
	virtual void animateCostume();
	virtual void setActorCostume(int c);

	void animateLimb(int limb, int f);

	bool actorHitTest(int x, int y);

	const byte *getActorName();
	void startWalkActor(int x, int y, int dir);
	void stopActorMoving();
protected:
	void startWalkAnim(int cmd, int angle);
public:
	void runActorTalkScript(int f);
	virtual void startAnimActor(int frame);

	void remapActorPalette(int r_fact, int g_fact, int b_fact, int threshold);
	void remapActorPaletteColor(int slot, int color);

	void animateActor(int anim);

	bool isInCurrentRoom() const {
		return _room == _vm->_currentRoom;
	}

	Common::Point getPos() const {
		Common::Point p(_pos);
		if (_vm->_game.version <= 2) {
			p.x *= V12_X_MULTIPLIER;
			p.y *= V12_Y_MULTIPLIER;
		}
		return p;
	}

	const Common::Point& getRealPos() const {
		return _pos;
	}

	int getRoom() const {
		return _room;
	}

	int getFacing() const {
		return _facing;
	}

	void setFacing(int newFacing) {
		_facing = newFacing;
	}

	int getAnimVar(byte var) const;
	void setAnimVar(byte var, int value);

	void setAnimSpeed(byte newAnimSpeed) {
		_animSpeed = newAnimSpeed;
		_animProgress = 0;
	}

	int getAnimSpeed() const {
		return _animSpeed;
	}

	int getAnimProgress() const {
		return _animProgress;
	}

	int getElevation() const {
		return _elevation;
	}

	void setElevation(int newElevation) {
		if (_elevation != newElevation) {
			_elevation = newElevation;
			_needRedraw = true;
		}
	}

	void setPalette(int idx, int val) {
		_palette[idx] = val;
		_needRedraw = true;
	}

	void setScale(int sx, int sy) {
		if (sx != -1)
			_scalex = sx;
		if (sy != -1)
			_scaley = sy;
		_needRedraw = true;
	}

	void classChanged(int cls, bool value);

	// Used by the save/load system:
	virtual void saveLoadWithSerializer(Serializer *ser);

protected:
	bool isInClass(int cls);

	virtual bool isPlayer();

	bool findPathTowards(byte box, byte box2, byte box3, Common::Point &foundPath);
};

class Actor_v3 : public Actor {
public:
	Actor_v3(ScummEngine *scumm, int id) : Actor(scumm, id) {}

	virtual void walkActor();

protected:
	virtual void setupActorScale();
	void findPathTowardsOld(byte box, byte box2, byte box3, Common::Point &p2, Common::Point &p3);
};

class Actor_v2 : public Actor_v3 {
public:
	Actor_v2(ScummEngine *scumm, int id) : Actor_v3(scumm, id) {}

	virtual void initActor(int mode);
	virtual void walkActor();
	virtual AdjustBoxResult adjustXYToBeInBox(int dstX, int dstY);

protected:
	virtual bool isPlayer();
	virtual void prepareDrawActorCostume(BaseCostumeRenderer *bcr);
	virtual bool checkWalkboxesHaveDirectPath(Common::Point &foundPath);
};

enum ActorV0MiscFlags {
	kActorMiscFlagStrong    = 0x01, // Kid is strong (Hunk-O-Matic used)
	kActorMiscFlagGTFriend  = 0x02, // Kid is green tentacle's friend (recording contract)
	kActorMiscFlagWatchedTV = 0x04, // Kid knows publisher's address (watched TV)
	kActorMiscFlagEdsEnemy  = 0x08, // Kid is not Weird Ed's friend
	kActorMiscFlag_10       = 0x10, // ???
	kActorMiscFlag_20       = 0x20, // ???
	kActorMiscFlagFreeze    = 0x40, // Stop moving
	kActorMiscFlagHide      = 0x80  // Kid is invisible (dead or in radiation suit)
};

class Actor_v0 : public Actor_v2 {
public:
	byte _costCommandNew;
	byte _costCommand;
	byte _miscflags;
	byte _speaking;

	int8 _animFrameRepeat;
	int8 _limbFrameRepeatNew[8];
	int8 _limbFrameRepeat[8];

	bool _limb_flipped[8];

public:
	Actor_v0(ScummEngine *scumm, int id) : Actor_v2(scumm, id) {}

	virtual void initActor(int mode);
	virtual void animateActor(int anim);
	virtual void animateCostume();

	void limbFrameCheck(int limb);

	void speakCheck();
	virtual void setDirection(int direction);
	void startAnimActor(int f);

	// Used by the save/load system:
	virtual void saveLoadWithSerializer(Serializer *ser);

protected:
	bool intersectLineSegments(const Common::Point &line1Start, const Common::Point &line1End,
		const Common::Point &line2Start, const Common::Point &line2End, Common::Point &result);
	virtual bool checkWalkboxesHaveDirectPath(Common::Point &foundPath);
};


} // End of namespace Scumm

#endif
