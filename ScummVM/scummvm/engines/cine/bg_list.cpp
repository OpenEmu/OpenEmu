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


#include "common/endian.h"
#include "common/stream.h"

#include "cine/cine.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/various.h"
#include "cine/bg_list.h"

namespace Cine {

uint32 var8;

/**
 * Add masked sprite to the background
 * @param objIdx Sprite description
 */
void addToBGList(int16 objIdx) {
	createBgIncrustListElement(objIdx, 0);

	renderer->incrustSprite(g_cine->_bgIncrustList.back());
}

/**
 * Add filled sprite to the background
 * @param objIdx Sprite description
 */
void addSpriteFilledToBGList(int16 objIdx) {
	createBgIncrustListElement(objIdx, 1);

	renderer->incrustMask(g_cine->_bgIncrustList.back());
}

/**
 * Add new element to incrust list
 * @param objIdx Element description
 * @param param Type of element
 */
void createBgIncrustListElement(int16 objIdx, int16 param) {
	BGIncrust tmp;

	tmp.unkPtr = 0;
	tmp.objIdx = objIdx;
	tmp.param = param;
	tmp.x = g_cine->_objectTable[objIdx].x;
	tmp.y = g_cine->_objectTable[objIdx].y;
	tmp.frame = g_cine->_objectTable[objIdx].frame;
	tmp.part = g_cine->_objectTable[objIdx].part;

	g_cine->_bgIncrustList.push_back(tmp);
}

/**
 * Reset var8 (probably something related to bgIncrustList)
 */
void resetBgIncrustList() {
	var8 = 0;
}

/**
 * Restore incrust list from savefile
 * @param fHandle Savefile open for reading
 */
void loadBgIncrustFromSave(Common::SeekableReadStream &fHandle) {
	BGIncrust tmp;
	int size = fHandle.readSint16BE();

	for (int i = 0; i < size; i++) {
		fHandle.readUint32BE();
		fHandle.readUint32BE();

		tmp.unkPtr = 0;
		tmp.objIdx = fHandle.readUint16BE();
		tmp.param = fHandle.readUint16BE();
		tmp.x = fHandle.readUint16BE();
		tmp.y = fHandle.readUint16BE();
		tmp.frame = fHandle.readUint16BE();
		tmp.part = fHandle.readUint16BE();

		g_cine->_bgIncrustList.push_back(tmp);

		if (tmp.param == 0) {
			renderer->incrustSprite(tmp);
		} else {
			renderer->incrustMask(tmp);
		}
	}
}

} // End of namespace Cine
