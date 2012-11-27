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


#include "queen/state.h"

namespace Queen {

Direction State::findDirection(uint16 state) {
	static const Direction sd[] = {
		DIR_BACK,
		DIR_RIGHT,
		DIR_LEFT,
		DIR_FRONT
	};
	return sd[(state >> 2) & 3];
}

StateTalk State::findTalk(uint16 state) {
	return (state & (1 << 9)) ? STATE_TALK_TALK : STATE_TALK_MUTE;
}

StateGrab State::findGrab(uint16 state) {
	static const StateGrab sg[] = {
		STATE_GRAB_NONE,
		STATE_GRAB_DOWN,
		STATE_GRAB_UP,
		STATE_GRAB_MID
	};
	return sg[state & 3];
}

StateOn State::findOn(uint16 state) {
	return (state & (1 << 8)) ? STATE_ON_ON : STATE_ON_OFF;
}

Verb State::findDefaultVerb(uint16 state) {
	static const Verb sdv[] = {
		VERB_NONE,
		VERB_OPEN,
		VERB_NONE,
		VERB_CLOSE,

		VERB_NONE,
		VERB_NONE,
		VERB_LOOK_AT,
		VERB_MOVE,

		VERB_GIVE,
		VERB_TALK_TO,
		VERB_NONE,
		VERB_NONE,

		VERB_USE,
		VERB_NONE,
		VERB_PICK_UP,
		VERB_NONE
	};
	return sdv[(state >> 4) & 0xF];
}

StateUse State::findUse(uint16 state) {
	return (state & (1 << 10)) ? STATE_USE : STATE_USE_ON;
}

void State::alterOn(uint16 *objState, StateOn state) {
	switch (state) {
	case STATE_ON_ON:
		*objState |= (1 << 8);
		break;
	case STATE_ON_OFF:
		*objState &= ~(1 << 8);
		break;
	}
}

void State::alterDefaultVerb(uint16 *objState, Verb v) {
	uint16 val;
	switch (v) {
	case VERB_OPEN:
		val = 1;
		break;
	case VERB_CLOSE:
		val = 3;
		break;
	case VERB_MOVE:
		val = 7;
		break;
	case VERB_GIVE:
		val = 8;
		break;
	case VERB_USE:
		val = 12;
		break;
	case VERB_PICK_UP:
		val = 14;
		break;
	case VERB_TALK_TO:
		val = 9;
		break;
	case VERB_LOOK_AT:
		val = 6;
		break;
	default:
		val = 0;
		break;
	}
	*objState = (*objState & ~0xF0) | (val << 4);
}

} // End of namespace Queen
