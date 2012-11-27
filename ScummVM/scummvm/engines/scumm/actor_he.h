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


#ifndef SCUMM_ACTOR_HE_H
#define SCUMM_ACTOR_HE_H

#include "scumm/actor.h"

namespace Scumm {

struct AuxBlock {
	bool visible;
	Common::Rect r;

	void reset() {
		visible = false;
		r.left = r.top = 0;
		r.right = r.bottom = -1;
	}
};

struct AuxEntry {
	int actorNum;
	int subIndex;
};

class ActorHE : public Actor {
public:
	ActorHE(ScummEngine *scumm, int id) : Actor(scumm, id) {}

	virtual void initActor(int mode);

	virtual void hideActor();

	void drawActorToBackBuf(int x, int y);

	void setHEFlag(int bit, int set);

	void setUserCondition(int slot, int set);
	bool isUserConditionSet(int slot) const;

	void setTalkCondition(int slot);
	bool isTalkConditionSet(int slot) const;

public:
	/** This rect is used to clip actor drawing. */
	Common::Rect _clipOverride;

	bool _heNoTalkAnimation;
	bool _heTalking;
	byte _heFlags;

	AuxBlock _auxBlock;

	struct {
		int16 posX;
		int16 posY;
		int16 color;
		byte sentence[128];
	} _heTalkQueue[16];


	virtual void prepareDrawActorCostume(BaseCostumeRenderer *bcr);
	virtual void setActorCostume(int c);
};

} // End of namespace Scumm

#endif
