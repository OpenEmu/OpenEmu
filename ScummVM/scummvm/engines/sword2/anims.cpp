/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

// ---------------------------------------------------------------------------
// A more intelligent version of the old ANIMS.C
// All this stuff by James
// DON'T TOUCH!
// ---------------------------------------------------------------------------


#include "common/file.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/screen.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/sound.h"
#include "sword2/animation.h"

namespace Sword2 {

int Router::doAnimate(byte *ob_logic, byte *ob_graph, int32 animRes, bool reverse) {
	AnimHeader anim_head;
	byte *anim_file;

	ObjectLogic obLogic(ob_logic);
	ObjectGraphic obGraph(ob_graph);

	if (obLogic.getLooping() == 0) {
		// This is the start of the anim - set up the first frame

		// For testing all anims!
		// A script loop can send every resource number to the anim
		// function & it will only run the valid ones. See
		// 'testing_routines' object in George's Player Character
		// section of linc

		if (_vm->_logic->readVar(SYSTEM_TESTING_ANIMS)) {
			if (!_vm->_resman->checkValid(animRes)) {
				// Not a valid resource number. Switch off
				// the sprite. Don't animate - just continue
				// script next cycle.
				setSpriteStatus(ob_graph, NO_SPRITE);
				return IR_STOP;
			}

			// if it's not an animation file
			if (_vm->_resman->fetchType(animRes) != ANIMATION_FILE) {
				// switch off the sprite
				// don't animate - just continue
				// script next cycle
				setSpriteStatus(ob_graph, NO_SPRITE);
				return IR_STOP;
			}

			// switch on the sprite
			setSpriteStatus(ob_graph, SORT_SPRITE);
		}

		assert(animRes);

		// open anim file
		anim_file = _vm->_resman->openResource(animRes);

		assert(_vm->_resman->fetchType(animRes) == ANIMATION_FILE);

		// point to anim header
		anim_head.read(_vm->fetchAnimHeader(anim_file));

		// now running an anim, looping back to this call again
		obLogic.setLooping(1);
		obGraph.setAnimResource(animRes);

		if (reverse)
			obGraph.setAnimPc(anim_head.noAnimFrames - 1);
		else
			obGraph.setAnimPc(0);
	} else if (_vm->_logic->getSync() != -1) {
		// We've received a sync - return to script immediately
		debug(5, "**sync stopped %d**", _vm->_logic->readVar(ID));

		// If sync received, anim finishes right now (remaining on
		// last frame). Quit animation, but continue script.
		obLogic.setLooping(0);
		return IR_CONT;
	} else {
		// Not first frame, and no sync received - set up the next
		// frame of the anim.

		// open anim file and point to anim header
		anim_file = _vm->_resman->openResource(obGraph.getAnimResource());
		anim_head.read(_vm->fetchAnimHeader(anim_file));

		if (reverse)
			obGraph.setAnimPc(obGraph.getAnimPc() - 1);
		else
			obGraph.setAnimPc(obGraph.getAnimPc() + 1);
	}

	// check for end of anim

	if (reverse) {
		if (obGraph.getAnimPc() == 0)
			obLogic.setLooping(0);
	} else {
		if (obGraph.getAnimPc() == anim_head.noAnimFrames - 1)
			obLogic.setLooping(0);
	}

	// close the anim file
	_vm->_resman->closeResource(obGraph.getAnimResource());

	// check if we want the script to loop back & call this function again
	return obLogic.getLooping() ? IR_REPEAT : IR_STOP;
}

int Router::megaTableAnimate(byte *ob_logic, byte *ob_graph, byte *ob_mega, byte *animTable, bool reverse) {
	int32 animRes = 0;

	// If this is the start of the anim, read the anim table to get the
	// appropriate anim resource

	ObjectLogic obLogic(ob_logic);

	if (obLogic.getLooping() == 0) {
		ObjectMega obMega(ob_mega);

		// Appropriate anim resource is in 'table[direction]'
		animRes = READ_LE_UINT32(animTable + 4 * obMega.getCurDir());
	}

	return doAnimate(ob_logic, ob_graph, animRes, reverse);
}

void Router::setSpriteStatus(byte *ob_graph, uint32 type) {
	ObjectGraphic obGraph(ob_graph);

	// Remove the previous status, but don't affect the shading upper-word
	obGraph.setType((obGraph.getType() & 0xffff0000) | type);
}

void Router::setSpriteShading(byte *ob_graph, uint32 type) {
	ObjectGraphic obGraph(ob_graph);

	// Remove the previous shading, but don't affect the status lower-word.
	// Note that mega frames may still be shaded automatically, even when
	// not sent 'RDSPR_SHADOW'.
	obGraph.setType((obGraph.getType() & 0x0000ffff) | type);
}

} // End of namespace Sword2
