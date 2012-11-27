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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_COLL_TEMPL_H
#define WINTERMUTE_COLL_TEMPL_H

#include "common/array.h"
#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

// Basically Common::Array with peristence-support.
template<typename TYPE>
class BaseArray : public Common::Array<TYPE> {
public:
// TODO: Might want to make sure that destructors are called when replacing/deleting/getting destructed
	bool persist(BasePersistenceManager *persistMgr) {
		int j;
		if (persistMgr->getIsSaving()) {
			j = Common::Array<TYPE>::size();
			persistMgr->transfer("ArraySize", &j);
			typename Common::Array<TYPE>::const_iterator it = Common::Array<TYPE>::begin();
			for (; it != Common::Array<TYPE>::end(); ++it) {
				TYPE obj = *it;
				persistMgr->transfer("", &obj);
			}
		} else {
			Common::Array<TYPE>::clear();
			persistMgr->transfer("ArraySize", &j);
			for (int i = 0; i < j; i++) {
				TYPE obj;
				persistMgr->transfer("", &obj);
				add(obj);
			}
		}
		return true;
	}
	int add(TYPE newElement) {
		Common::Array<TYPE>::push_back(newElement);
		return Common::Array<TYPE>::size() - 1;
	}
	void remove_at(uint32 idx) {
		Common::Array<TYPE>::remove_at(idx);
	}
	void remove_at(uint32 idx, uint32 num) {
		while (num) {
			if (idx >= Common::Array<TYPE>::size()) {
				break;
			}
			Common::Array<TYPE>::remove_at(idx);
		}
	}
	template<typename T2>
	void copy(const BaseArray<T2> &src) {
		Common::Array<TYPE>::insert_at(0, src);
	}
};

} // end of namespace Wintermute

#endif
