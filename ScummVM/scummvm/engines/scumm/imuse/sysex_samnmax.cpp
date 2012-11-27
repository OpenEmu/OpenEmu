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


#include "common/endian.h"
#include "common/util.h"

/*
 * SysEx command handlers must have full access to the
 * internal IMuse implementation classes. Before including
 * the relevant header file, two things must happen:
 *   1. A function declaration must be made.
 *   2. The following #define must be established:
 *      #define SYSEX_CALLBACK_FUNCTION functionName
 */
#define SYSEX_CALLBACK_FUNCTION sysexHandler_SamNMax
#include "scumm/imuse/imuse_internal.h"

namespace Scumm {

extern void sysexHandler_Scumm(Player *, const byte *, uint16);

void sysexHandler_SamNMax(Player *player, const byte *msg, uint16 len) {
	byte a;

	IMuseInternal *se = player->_se;
	const byte *p = msg;

	switch (*p++) {
	case 0:
		// Trigger Event
		// Triggers are set by doCommand(ImSetTrigger).
		// When a SysEx marker is encountered whose sound
		// ID and marker ID match what was set by ImSetTrigger,
		// something magical is supposed to happen....
		for (a = 0; a < ARRAYSIZE(se->_snm_triggers); ++a) {
			if (se->_snm_triggers[a].sound == player->_id &&
			        se->_snm_triggers[a].id == *p) {
				se->_snm_triggers[a].sound = se->_snm_triggers[a].id = 0;
				se->doCommand(8, se->_snm_triggers[a].command);
				break;
			}
		}
		break;

	case 1:
		// maybe_jump.
		if (player->_scanning)
			break;
		player->maybe_jump(p[0], p[1] - 1, (READ_BE_UINT16(p + 2) - 1) * 4 + p[4], ((p[5] * TICKS_PER_BEAT) >> 2) + p[6]);
		break;

	default:
		sysexHandler_Scumm(player, msg, len);
	}
}

} // End of namespace Scumm
