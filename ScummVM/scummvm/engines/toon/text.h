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

#ifndef TOON_TEXT_H
#define TOON_TEXT_H

#include "toon/toon.h"

namespace Toon {

class TextResource {
public:
	TextResource(ToonEngine *vm);
	~TextResource(void);

	bool loadTextResource(const Common::String &fileName);
	char *getText(int32 id);
	int32 getId(int32 offset);
	int32 getNext(int32 offset);

protected:
	int32 _numTexts;
	uint8 *_textData;
	ToonEngine *_vm;
};

} // End of namespace Toon

#endif
