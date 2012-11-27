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

#include "backends/keymapper/keymap.h"

#ifdef ENABLE_KEYMAPPER

#include "common/system.h"

#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymapper-defaults.h"

#define KEYMAP_KEY_PREFIX "keymap_"

namespace Common {

Keymap::Keymap(const Keymap& km) : _actions(km._actions), _keymap(), _nonkeymap(), _configDomain(0) {
	List<Action *>::iterator it;

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		const HardwareInput *hwInput = (*it)->getMappedInput();

		if (hwInput) {
			if (hwInput->type == kHardwareInputTypeKeyboard)
				_keymap[hwInput->key] = *it;
			else if (hwInput->type == kHardwareInputTypeGeneric)
				_nonkeymap[hwInput->inputCode] = *it;
		}
	}
}

Keymap::~Keymap() {
	List<Action *>::iterator it;

	for (it = _actions.begin(); it != _actions.end(); ++it)
		delete *it;
}

void Keymap::addAction(Action *action) {
	if (findAction(action->id))
		error("Action with id %s already in KeyMap", action->id);

	_actions.push_back(action);
}

void Keymap::registerMapping(Action *action, const HardwareInput *hwInput) {
	if (hwInput->type == kHardwareInputTypeKeyboard) {
		HashMap<KeyState, Action *>::iterator it = _keymap.find(hwInput->key);
		// if input is already mapped to a different action then unmap it from there
		if (it != _keymap.end() && action != it->_value)
			it->_value->mapInput(0);
		// now map it
		_keymap[hwInput->key] = action;
	} else if (hwInput->type == kHardwareInputTypeGeneric) {
		HashMap<HardwareInputCode, Action *>::iterator it = _nonkeymap.find(hwInput->inputCode);
		// if input is already mapped to a different action then unmap it from there
		if (it != _nonkeymap.end() && action != it->_value)
			it->_value->mapInput(0);
		// now map it
		_nonkeymap[hwInput->inputCode] = action;
	}
}

void Keymap::unregisterMapping(Action *action) {
	const HardwareInput *hwInput = action->getMappedInput();

	if (hwInput) {
		if (hwInput->type == kHardwareInputTypeKeyboard)
			_keymap.erase(hwInput->key);
		else if (hwInput->type == kHardwareInputTypeGeneric)
			_nonkeymap.erase(hwInput->inputCode);
	}
}

Action *Keymap::getAction(const char *id) {
	return findAction(id);
}

Action *Keymap::findAction(const char *id) {
	List<Action *>::iterator it;

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		if (strncmp((*it)->id, id, ACTION_ID_SIZE) == 0)
			return *it;
	}
	return 0;
}

const Action *Keymap::findAction(const char *id) const {
	List<Action *>::const_iterator it;

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		if (strncmp((*it)->id, id, ACTION_ID_SIZE) == 0)
			return *it;
	}

	return 0;
}

Action *Keymap::getMappedAction(const KeyState& ks) const {
	HashMap<KeyState, Action *>::iterator it;

	it = _keymap.find(ks);

	if (it == _keymap.end())
		return 0;
	else
		return it->_value;
}

Action *Keymap::getMappedAction(const HardwareInputCode code) const {
	HashMap<HardwareInputCode, Action *>::iterator it;

	it = _nonkeymap.find(code);

	if (it == _nonkeymap.end())
		return 0;
	else
		return it->_value;
}

void Keymap::setConfigDomain(ConfigManager::Domain *dom) {
	_configDomain = dom;
}

void Keymap::loadMappings(const HardwareInputSet *hwKeys) {
	if (!_configDomain)
		return;

	if (_actions.empty())
		return;

	Common::KeymapperDefaultBindings *defaults = g_system->getKeymapperDefaultBindings();

	HashMap<String, const HardwareInput *> mappedInputs;
	List<Action*>::iterator it;
	String prefix = KEYMAP_KEY_PREFIX + _name + "_";

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		Action* ua = *it;
		String actionId(ua->id);
		String confKey = prefix + actionId;

		String hwInputId = _configDomain->getVal(confKey);

		bool defaulted = false;
		// fall back to the platform-specific defaults
		if (hwInputId.empty() && defaults) {
			hwInputId = defaults->getDefaultBinding(_name, actionId);
			if (!hwInputId.empty())
				defaulted = true;
		}
		// there's no mapping
		if (hwInputId.empty())
			continue;

		const HardwareInput *hwInput = hwKeys->findHardwareInput(hwInputId.c_str());

		if (!hwInput) {
			warning("HardwareInput with ID '%s' not known", hwInputId.c_str());
			continue;
		}

		if (defaulted) {
			if (mappedInputs.contains(hwInputId)) {
				debug(1, "Action [%s] not falling back to hardcoded default value [%s] because the hardware input is in use", confKey.c_str(), hwInputId.c_str());
				continue;
			}
			warning("Action [%s] fell back to hardcoded default value [%s]", confKey.c_str(), hwInputId.c_str());
		}

		mappedInputs.setVal(hwInputId, hwInput);
		// map the key
		ua->mapInput(hwInput);
	}
}

void Keymap::saveMappings() {
	if (!_configDomain)
		return;

	List<Action *>::const_iterator it;
	String prefix = KEYMAP_KEY_PREFIX + _name + "_";

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		uint actIdLen = strlen((*it)->id);

		actIdLen = (actIdLen > ACTION_ID_SIZE) ? ACTION_ID_SIZE : actIdLen;

		String actId((*it)->id, (*it)->id + actIdLen);
		String hwId = "";

		if ((*it)->getMappedInput()) {
			hwId = (*it)->getMappedInput()->id;
		}
		_configDomain->setVal(prefix + actId, hwId);
	}
}

bool Keymap::isComplete(const HardwareInputSet *hwInputs) {
	List<Action *>::iterator it;
	bool allMapped = true;
	uint numberMapped = 0;

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		if ((*it)->getMappedInput()) {
			++numberMapped;
		} else {
			allMapped = false;
		}
	}

	return allMapped || (numberMapped == hwInputs->size());
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
