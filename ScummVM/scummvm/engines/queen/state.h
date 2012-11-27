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

#ifndef QUEEN_STATE_H
#define QUEEN_STATE_H

#include "common/util.h"
#include "queen/defs.h"

namespace Queen {


enum StateTalk {
	STATE_TALK_TALK,
	STATE_TALK_MUTE
};

enum StateGrab {
	STATE_GRAB_NONE,
	STATE_GRAB_DOWN,
	STATE_GRAB_UP,
	STATE_GRAB_MID
};

enum StateOn {
	STATE_ON_ON,
	STATE_ON_OFF
};

enum StateUse {
	STATE_USE,
	STATE_USE_ON
};


/*!
	Each object/item in game has a state field.
	(refer to ObjectData and ItemData).

	<table>
		<tr>
			<td>Name</td>
			<td>Bits</td>
			<td>Description</td>
		</tr>
		<tr>
			<td>USE</td>
			<td>10</td>
			<td>Use</td>
		</tr>
		<tr>
			<td>TALK</td>
			<td>9</td>
			<td>Talk</td>
		</tr>
		<tr>
			<td>ON</td>
			<td>8</td>
			<td>On/Off</td>
		</tr>
		<tr>
			<td>DEF</td>
			<td>7,6,5,4</td>
			<td>Default verb command</td>
		</tr>
		<tr>
			<td>DIR</td>
			<td>3,2</td>
			<td>Direction to face for the object</td>
		</tr>
		<tr>
			<td>GRAB</td>
			<td>1,0</td>
			<td>Grab Direction</td>
		</tr>
	</table>
*/
struct State {

	static Direction findDirection(uint16 state);
	static StateTalk findTalk(uint16 state);
	static StateGrab findGrab(uint16 state);
	static StateOn   findOn(uint16 state);
	static Verb      findDefaultVerb(uint16 state);
	static StateUse  findUse(uint16 state);

	static void alterOn(uint16 *objState, StateOn state);
	static void alterDefaultVerb(uint16 *objState, Verb v);
};


} // End of namespace Queen

#endif
