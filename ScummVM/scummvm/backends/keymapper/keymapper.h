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

#ifndef COMMON_KEYMAPPER_H
#define COMMON_KEYMAPPER_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

#include "common/events.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "common/stack.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymap.h"

namespace Common {

const char *const kGuiKeymapName = "gui";
const char *const kGlobalKeymapName = "global";

class Keymapper : public Common::DefaultEventMapper {
public:

	struct MapRecord {
		Keymap* keymap;
		bool transparent;
		bool global;
	};

	/* Nested class that represents a set of keymaps */
	class Domain : public HashMap<String, Keymap*,
				IgnoreCase_Hash, IgnoreCase_EqualTo>  {
	public:
		Domain() : _configDomain(0) {}
		~Domain() {
			deleteAllKeyMaps();
		}

		void setConfigDomain(ConfigManager::Domain *confDom) {
			_configDomain = confDom;
		}
		ConfigManager::Domain *getConfigDomain() {
			return _configDomain;
		}

		void addKeymap(Keymap *map);

		void deleteAllKeyMaps();

		Keymap *getKeymap(const String& name);

	private:
		ConfigManager::Domain *_configDomain;
	};

	Keymapper(EventManager *eventMan);
	~Keymapper();

	// EventMapper interface
	virtual List<Event> mapEvent(const Event &ev, EventSource *source);

	/**
	 * Registers a HardwareInputSet with the Keymapper
	 * @note should only be called once (during backend initialisation)
	 */
	void registerHardwareInputSet(HardwareInputSet *inputs);

	/**
	 * Get a list of all registered HardwareInputs
	 */
	const List<const HardwareInput *> &getHardwareInputs() const {
		assert(_hardwareInputs);
		return _hardwareInputs->getHardwareInputs();
	}

	/**
	 * Add a keymap to the global domain.
	 * If a saved key setup exists for it in the ini file it will be used.
	 * Else, the key setup will be automatically mapped.
	 */
	void addGlobalKeymap(Keymap *keymap);

	/**
	 * Add a keymap to the game domain.
	 * @see addGlobalKeyMap
	 * @note initGame() should be called before any game keymaps are added.
	 */
	void addGameKeymap(Keymap *keymap);

	/**
	 * Should be called at end of game to tell Keymapper to deactivate and free
	 * any game keymaps that are loaded.
	 */
	void cleanupGameKeymaps();

	/**
	 * Obtain a keymap of the given name from the keymapper.
	 * Game keymaps have priority over global keymaps
	 * @param name		name of the keymap to return
	 * @param global	set to true if returned keymap is global, false if game
	 */
	Keymap *getKeymap(const String& name, bool *global = 0);

	/**
	 * Push a new keymap to the top of the active stack, activating
	 * it for use.
	 * @param name			name of the keymap to push
	 * @param transparent	if true keymapper will iterate down the
	 *						stack if it cannot find a key in the new map
	 * @return				true if succesful
	 */
	bool pushKeymap(const String& name, bool transparent = false);

	/**
	 * Pop the top keymap off the active stack.
	 * @param name	(optional) name of keymap expected to be popped
	 * 				if provided, will not pop unless name is the same
	 * 				as the top keymap
	 */
	void popKeymap(const char *name = 0);

	/**
	 * @brief Map a key press event.
	 * If the active keymap contains a Action mapped to the given key, then
	 * the Action's events are pushed into the EventManager's event queue.
	 * @param key		key that was pressed
	 * @param keyDown	true for key down, false for key up
	 * @return			mapped events
	 */
	List<Event> mapKey(const KeyState& key, bool keyDown);
	List<Event> mapNonKey(const HardwareInputCode code);

	/**
	 * @brief Map a key down event.
	 * @see mapKey
	 */
	List<Event> mapKeyDown(const KeyState& key);

	/**
	 * @brief Map a key up event.
	 * @see mapKey
	 */
	List<Event> mapKeyUp(const KeyState& key);

	/**
	 * Enable/disable the keymapper
	 */
	void setEnabled(bool enabled) { _enabled = enabled; }

	/**
	 * @brief Activate remapping mode
	 * While this mode is active, any mappable event will be bound to the action
	 * provided.
	 * @param actionToRemap Action that is the target of the remap
	 */
	void startRemappingMode(Action *actionToRemap);

	/**
	 * @brief Force-stop the remapping mode
	 */
	void stopRemappingMode() { _remapping = false; }

	/**
	 * Query whether the keymapper is currently in the remapping mode
	 */
	bool isRemapping() const { return _remapping; }

	/**
	 * Return a HardwareInput pointer for the given key state
	 */
	const HardwareInput *findHardwareInput(const KeyState& key);

	/**
	 * Return a HardwareInput pointer for the given input code
	 */
	const HardwareInput *findHardwareInput(const HardwareInputCode code);

	Domain& getGlobalDomain() { return _globalDomain; }
	Domain& getGameDomain() { return _gameDomain; }
	const Stack<MapRecord>& getActiveStack() const { return _activeMaps; }

private:

	enum IncomingEventType {
		kIncomingKeyDown,
		kIncomingKeyUp,
		kIncomingNonKey
	};

	void initKeymap(Domain &domain, Keymap *keymap);

	Domain _globalDomain;
	Domain _gameDomain;

	HardwareInputSet *_hardwareInputs;

	void pushKeymap(Keymap *newMap, bool transparent, bool global);

	Action *getAction(const KeyState& key);
	List<Event> executeAction(const Action *act, IncomingEventType incomingType = kIncomingNonKey);
	EventType convertDownToUp(EventType eventType);
	List<Event> remap(const Event &ev);

	EventManager *_eventMan;

	bool _enabled;
	bool _remapping;

	Action *_actionToRemap;
	Stack<MapRecord> _activeMaps;
	HashMap<KeyState, Action *> _keysDown;

};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef COMMON_KEYMAPPER_H
