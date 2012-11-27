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
*
*/

#ifdef ENABLE_KEYMAPPER

#ifndef KEYMAPPER_DEFAULTS_H
#define KEYMAPPER_DEFAULTS_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/hash-str.h"

namespace Common {

class KeymapperDefaultBindings : HashMap<String, String> {
public:
	/**
	 * This sets a default hwInput for a given Keymap Action
	 * @param keymapId String representing Keymap id (Keymap.name)
	 * @param actionId String representing Action id (Action.id)
	 * @param hwInputId String representing the HardwareInput id (HardwareInput.id)
	 */
	void setDefaultBinding(String keymapId, String actionId, String hwInputId) { setVal(keymapId + "_" + actionId, hwInputId); }
	/**
	 * This retrieves the assigned default hwKey for a given Keymap Action
	 * @param keymapId String representing Keymap id (Keymap.name)
	 * @param actionId String representing Action id (Action.id)
	 * @return String representing the HardwareInput id (HardwareInput.id)
	 */
	String getDefaultBinding(String keymapId, String actionId) { return getVal(keymapId + "_" + actionId); }
};

} //namespace Common

#endif // #ifndef KEYMAPPER_DEFAULTS_H
#endif // #ifdef ENABLE_KEYMAPPER
