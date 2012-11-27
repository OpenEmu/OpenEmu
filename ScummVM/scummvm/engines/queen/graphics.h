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

#ifndef QUEEN_GRAPHICS_H
#define QUEEN_GRAPHICS_H

#include "common/util.h"
#include "queen/structs.h"

namespace Queen {

struct BobSlot {
	bool active;
	//! current position
	int16 x, y;
	//! bounding box
	Box box;
	bool xflip;
	//! shrinking percentage
	uint16 scale;
	//! associated BobFrame
	uint16 frameNum;
	//! 'direction' for the next frame (-1, 1)
	int frameDir;

	//! animation stuff
	bool animating;
	struct {
		int16 speed, speedBak;

		//! string based animation
		struct {
			const AnimFrame *buffer;
			const AnimFrame *curPos;
		} string;

		//! normal moving animation
		struct {
			bool rebound;
			uint16 firstFrame, lastFrame;
		} normal;

	} anim;

	bool moving;
	//! moving speed
	int16 speed;
	//! move along x axis instead of y
	bool xmajor;
	//! moving direction
	int8 xdir, ydir;
	//! destination point
	int16 endx, endy;
	uint16 dx, dy;
	uint16 total;

	void curPos(int16 xx, int16 yy);
	void move(int16 dstx, int16 dsty, int16 spd);
	void moveOneStep();
	void animOneStep();

	void animString(const AnimFrame *animBuf);
	void animNormal(uint16 firstFrame, uint16 lastFrame, uint16 speed, bool rebound, bool xflip);

	void scaleWalkSpeed(uint16 ms);

	void clear(const Box *defaultBox);
};

class QueenEngine;

class Graphics {
public:

	Graphics(QueenEngine *vm);
	~Graphics();

	//! unpacks control frames (ie. arrows)
	void unpackControlBank();

	//! setup dialog arrows
	void setupArrows();

	//! setup mouse cursor
	void setupMouseCursor();

	//! draw a bob
	void drawBob(const BobSlot *bs, const BobFrame *bf, const Box *box, int16 x, int16 y);

	//! draw an inventory item
	void drawInventoryItem(uint32 frameNum, uint16 x, uint16 y);

	//! draw a bob directly on the backdrop bitmap
	void pasteBob(uint16 objNum, uint16 image);

	//! resize a bobframe
	void shrinkFrame(const BobFrame *bf, uint16 percentage);

	//! animate/move bobs and sort them
	void sortBobs();

	//! draw all the sorted bobs
	void drawBobs();

	//! clear all setup bobs
	void clearBobs();

	//! stop all animating/movings bobs
	void stopBobs();

	//! returns a reference to the specified bob
	BobSlot *bob(int index);

	void clearBob(int index) { bob(index)->clear(&_defaultBox); }

	//! display a text 'near' the specified bob
	void setBobText(const BobSlot *bob, const char *text, int textX, int textY, int color, int flags);

	//! handles parallax scrolling for the specified room
	void handleParallax(uint16 roomNum);

	void setupNewRoom(const char *room, uint16 roomNum, int16 *furniture, uint16 furnitureCount);

	void setBobCutawayAnim(uint16 bobNum, bool xflip, const AnimFrame *af, uint8 frameCount);
	void fillAnimBuffer(const char *anim, AnimFrame *af);
	uint16 countAnimFrames(const char *anim);
	void setupObjectAnim(const GraphicData *gd, uint16 firstImage, uint16 bobNum, bool visible);
	uint16 setupPersonAnim(const ActorData *ad, const char *anim, uint16 curImage);
	void resetPersonAnim(uint16 bobNum);
	void erasePersonAnim(uint16 bobNum);
	void eraseAllAnims();

	uint16 refreshObject(uint16 obj);

	void setupRoomFurniture(int16 *furniture, uint16 furnitureCount);
	void setupRoomObjects();

	uint16 setupPerson(uint16 noun, uint16 curImage);
	uint16 allocPerson(uint16 noun, uint16 curImage);

	uint16 personFrames(uint16 bobNum) const { return _personFrames[bobNum]; }
	void clearPersonFrames() { memset(_personFrames, 0, sizeof(_personFrames)); }
	uint16 numFrames() const { return _numFrames; }
	uint16 numStaticFurniture() const { return _numFurnitureStatic; }
	uint16 numAnimatedFurniture() const { return _numFurnitureAnimated; }
	uint16 numFurnitureFrames() const { return _numFurnitureStatic + _numFurnitureAnimatedLen; }

	void putCameraOnBob(int bobNum) { _cameraBob = bobNum; }

	void update(uint16 room);

	enum {
		ARROW_BOB_UP        =  62,
		ARROW_BOB_DOWN      =  63,
		MAX_BOBS_NUMBER     =  64,
		MAX_STRING_LENGTH   = 255,
		MAX_STRING_SIZE     = (MAX_STRING_LENGTH + 1),
		BOB_SHRINK_BUF_SIZE = 60000
	};


private:

	BobSlot _bobs[MAX_BOBS_NUMBER];

	//! bobs to display
	BobSlot *_sortedBobs[MAX_BOBS_NUMBER];

	//! number of bobs to display
	uint16 _sortedBobsCount;

	//! used to scale a BobFrame
	BobFrame _shrinkBuffer;

	//! in-game objects/persons animations
	AnimFrame _newAnim[17][30];

	//! cutaway objects/persons animations
	AnimFrame _cutAnim[21][30];

	uint16 _personFrames[4];

	//! number of animated furniture in current room
	uint16 _numFurnitureAnimated;

	//! number of static furniture in current room
	uint16 _numFurnitureStatic;

	//! total number of frames for the animated furniture
	uint16 _numFurnitureAnimatedLen;

	//! current number of frames unpacked
	uint16 _numFrames;

	//! bob number followed by camera
	int _cameraBob;

	QueenEngine *_vm;

	const Box _defaultBox;
	const Box _gameScreenBox;
	const Box _fullScreenBox;
};

class BamScene {
public:

	BamScene(QueenEngine *vm);

	void playSfx();
	void prepareAnimation();
	void updateCarAnimation();
	void updateFightAnimation();

	void saveState(byte *&ptr);
	void loadState(uint32 ver, byte *&ptr);

	enum {
		BOB_OBJ1 = 5,
		BOB_OBJ2 = 6,
		BOB_FX   = 7
	};

	enum {
		F_STOP     = 0,
		F_PLAY     = 1,
		F_REQ_STOP = 2
	};

	uint16 _flag, _index;

private:

	struct BamDataObj {
		int16 x, y;
		int16 frame;
	};

	struct BamDataBlock {
		BamDataObj obj1; // truck / Frank
		BamDataObj obj2; // Rico  / robot
		BamDataObj fx;
		int16 sfx;
	};

	BobSlot *_obj1;
	BobSlot *_obj2;
	BobSlot *_objfx;
	bool _screenShaked;
	const BamDataBlock *_fightData;
	uint16 _lastSoundIndex;

	QueenEngine *_vm;

	static const BamDataBlock _carData[];
	static const BamDataBlock _fight1Data[];
	static const BamDataBlock _fight2Data[];
	static const BamDataBlock _fight3Data[];
	static const BamDataBlock _fight4Data[];
};

} // End of namespace Queen

#endif
