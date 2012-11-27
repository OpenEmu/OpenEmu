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

#if !defined(SCUMM_IMUSE_DIGI_TABLES_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_TABLES_H

#include "common/scummsys.h"

namespace Scumm {

struct imuseRoomMap {
	int8 roomId;
	byte stateIndex1;
	byte offset;
	byte stateIndex2;
	byte attribPos;
	byte stateIndex3;
};

struct imuseDigTable {
	byte transitionType;
	int16 soundId;
	char name[20];
	byte attribPos;
	byte hookId;
	char filename[13];
};

struct imuseComiTable {
	byte transitionType;
	int16 soundId;
	char name[20];
	byte attribPos;
	byte hookId;
	int16 fadeOutDelay;
	char filename[13];
};


struct imuseFtNames {
	char name[20];
};

struct imuseFtStateTable {
	char audioName[9];
	byte transitionType;
	byte volume;
	char name[21];
};

struct imuseFtSeqTable {
	char audioName[9];
	byte transitionType;
	byte volume;
};

extern const imuseRoomMap _digStateMusicMap[];
extern const imuseDigTable _digStateMusicTable[];
extern const imuseDigTable _digSeqMusicTable[];
extern const imuseComiTable _comiStateMusicTable[];
extern const imuseComiTable _comiSeqMusicTable[];
extern const imuseFtStateTable _ftStateMusicTable[];
extern const imuseFtSeqTable _ftSeqMusicTable[];
extern const imuseFtNames _ftSeqNames[];

} // End of namespace Scumm

#endif
