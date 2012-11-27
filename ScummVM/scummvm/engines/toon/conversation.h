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

#ifndef TOON_CONVERSATION_H
#define TOON_CONVERSATION_H

#include "engines/engine.h"
#include "common/stream.h"

namespace Toon {

class Conversation {
public:
	int32 _enable;    // 00

	struct ConvState {
		int32 _data2; // 04
		int16 _data3; // 08
		void *_data4; // 10
	} state[10];

	void save(Common::WriteStream *stream, int16 *conversationDataBase);
	void load(Common::ReadStream *stream, int16 *conversationDataBase);
};

} // End of namespace Toon

#endif
