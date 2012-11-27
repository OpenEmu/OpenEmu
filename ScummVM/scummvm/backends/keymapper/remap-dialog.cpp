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

#include "backends/keymapper/remap-dialog.h"

#ifdef ENABLE_KEYMAPPER

#include "common/system.h"
#include "gui/gui-manager.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollbar.h"
#include "gui/ThemeEval.h"
#include "common/translation.h"

namespace Common {

enum {
	kRemapCmd = 'REMP',
	kClearCmd = 'CLER',
	kCloseCmd = 'CLOS'
};

RemapDialog::RemapDialog()
	: Dialog("KeyMapper"), _keymapTable(0), _topAction(0), _remapTimeout(0), _topKeymapIsGui(false) {

	_keymapper = g_system->getEventManager()->getKeymapper();
	assert(_keymapper);

	_kmPopUpDesc = new GUI::StaticTextWidget(this, "KeyMapper.PopupDesc", _("Keymap:"));
	_kmPopUp = new GUI::PopUpWidget(this, "KeyMapper.Popup");

	_scrollBar = new GUI::ScrollBarWidget(this, 0, 0, 0, 0);

	new GUI::ButtonWidget(this, "KeyMapper.Close", _("Close"), 0, kCloseCmd);
}

RemapDialog::~RemapDialog() {
	free(_keymapTable);
}

void RemapDialog::open() {
	const Stack<Keymapper::MapRecord> &activeKeymaps = _keymapper->getActiveStack();

	if (activeKeymaps.size() > 0) {
		if (activeKeymaps.top().keymap->getName() == Common::kGuiKeymapName)
			_topKeymapIsGui = true;
		// Add the entry for the "effective" special view. See RemapDialog::loadKeymap()
		_kmPopUp->appendEntry(activeKeymaps.top().keymap->getName() + _(" (Effective)"));
	}

	Keymapper::Domain *_globalKeymaps = &_keymapper->getGlobalDomain();
	Keymapper::Domain *_gameKeymaps = 0;

	int keymapCount = 0;

	if (_globalKeymaps->empty())
		_globalKeymaps = 0;
	else
		keymapCount += _globalKeymaps->size();

	if (ConfMan.getActiveDomain() != 0) {
		_gameKeymaps = &_keymapper->getGameDomain();

		if (_gameKeymaps->empty())
			_gameKeymaps = 0;
		else
			keymapCount += _gameKeymaps->size();
	}

	if (activeKeymaps.size() > 1) {
		keymapCount += activeKeymaps.size() - 1;
	}

	debug(3, "RemapDialog::open keymaps: %d", keymapCount);

	_keymapTable = (Keymap **)malloc(sizeof(Keymap *) * keymapCount);

	Keymapper::Domain::iterator it;
	uint32 idx = 0;

	if (activeKeymaps.size() > 1) {
		int topIndex = activeKeymaps.size() - 1;
		bool active = activeKeymaps[topIndex].transparent;
		for (int i = topIndex - 1; i >= 0; --i) {
			Keymapper::MapRecord mr = activeKeymaps[i];
			// Add an entry for each keymap in the stack after the top keymap. Mark it Active if it is
			// reachable or Blocked if an opaque keymap is on top of it thus blocking access to it.
			_kmPopUp->appendEntry(mr.keymap->getName() + (active ? _(" (Active)") : _(" (Blocked)")), idx);
			_keymapTable[idx++] = mr.keymap;
			active &= mr.transparent;
		}
	}

	_kmPopUp->appendEntry("");

	// Now add entries for all known keymaps. Note that there will be duplicates with the stack entries.

	if (_globalKeymaps) {
		for (it = _globalKeymaps->begin(); it != _globalKeymaps->end(); ++it) {
			// "global" means its keybindings apply to all games; saved in a global conf domain
			_kmPopUp->appendEntry(it->_value->getName() + _(" (Global)"), idx);
			_keymapTable[idx++] = it->_value;
		}
	}

	if (_gameKeymaps) {
		for (it = _gameKeymaps->begin(); it != _gameKeymaps->end(); ++it) {
			// "game" means its keybindings are saved per-target
			_kmPopUp->appendEntry(it->_value->getName() + _(" (Game)"), idx);
			_keymapTable[idx++] = it->_value;
		}
	}

	_changes = false;

	Dialog::open();

	_kmPopUp->setSelected(0);
	loadKeymap();
}

void RemapDialog::close() {
	_kmPopUp->clearEntries();

	free(_keymapTable);
	_keymapTable = 0;

	if (_changes)
		ConfMan.flushToDisk();

	Dialog::close();
}

void RemapDialog::reflowLayout() {
	Dialog::reflowLayout();

	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);
	int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);

	int16 areaX, areaY;
	uint16 areaW, areaH;
	g_gui.xmlEval()->getWidgetData((const String&)String("KeyMapper.KeymapArea"), areaX, areaY, areaW, areaH);

	int spacing = g_gui.xmlEval()->getVar("Globals.KeyMapper.Spacing");
	int keyButtonWidth = g_gui.xmlEval()->getVar("Globals.KeyMapper.ButtonWidth");
	int clearButtonWidth = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int clearButtonHeight = g_gui.xmlEval()->getVar("Globals.Line.Height");

	int colWidth = areaW - scrollbarWidth;
	int labelWidth =  colWidth - (keyButtonWidth + spacing + clearButtonWidth + spacing);

	_rowCount = (areaH + spacing) / (buttonHeight + spacing);
	debug(7, "rowCount = %d" , _rowCount);
	if (colWidth <= 0  || _rowCount <= 0)
		error("Remap dialog too small to display any keymaps");

	_scrollBar->resize(areaX + areaW - scrollbarWidth, areaY, scrollbarWidth, areaH);
	_scrollBar->_entriesPerPage = _rowCount;
	_scrollBar->_numEntries = 1;
	_scrollBar->recalc();

	uint textYOff = (buttonHeight - kLineHeight) / 2;
	uint clearButtonYOff = (buttonHeight - clearButtonHeight) / 2;
	uint oldSize = _keymapWidgets.size();
	uint newSize = _rowCount;

	_keymapWidgets.reserve(newSize);

	for (uint i = 0; i < newSize; i++) {
		ActionWidgets widg;

		if (i >= _keymapWidgets.size()) {
			widg.actionText =
				new GUI::StaticTextWidget(this, 0, 0, 0, 0, "", Graphics::kTextAlignLeft);
			widg.keyButton =
				new GUI::ButtonWidget(this, 0, 0, 0, 0, "", 0, kRemapCmd + i);
			widg.clearButton = addClearButton(this, "", kClearCmd + i, 0, 0, clearButtonWidth, clearButtonHeight);
			_keymapWidgets.push_back(widg);
		} else {
			widg = _keymapWidgets[i];
		}

		uint x = areaX;
		uint y = areaY + (i) * (buttonHeight + spacing);

		widg.keyButton->resize(x, y, keyButtonWidth, buttonHeight);
		widg.clearButton->resize(x + keyButtonWidth + spacing, y + clearButtonYOff, clearButtonWidth, clearButtonHeight);
		widg.actionText->resize(x + keyButtonWidth + spacing + clearButtonWidth + spacing, y + textYOff, labelWidth, kLineHeight);

	}
	while (oldSize > newSize) {
		ActionWidgets widg = _keymapWidgets.remove_at(--oldSize);

		removeWidget(widg.actionText);
		delete widg.actionText;

		removeWidget(widg.keyButton);
		delete widg.keyButton;

		removeWidget(widg.clearButton);
		delete widg.clearButton;
	}
}

void RemapDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	debug(3, "RemapDialog::handleCommand %u %u", cmd, data);

	if (cmd >= kRemapCmd && cmd < kRemapCmd + _keymapWidgets.size()) {
		startRemapping(cmd - kRemapCmd);
	} else if (cmd >= kClearCmd && cmd < kClearCmd + _keymapWidgets.size()) {
		clearMapping(cmd - kClearCmd);
	} else if (cmd == GUI::kPopUpItemSelectedCmd) {
		loadKeymap();
	} else if (cmd == GUI::kSetPositionCmd) {
		refreshKeymap();
	} else if (cmd == kCloseCmd) {
		close();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void RemapDialog::clearMapping(uint i) {
	if (_topAction + i >= _currentActions.size())
		return;

	debug(3, "clear the mapping %u", i);
	Action *activeRemapAction = _currentActions[_topAction + i].action;
	activeRemapAction->mapInput(0);
	activeRemapAction->getParent()->saveMappings();
	_changes = true;

	// force refresh
	stopRemapping(true);
	refreshKeymap();
}

void RemapDialog::startRemapping(uint i) {
	if (_topAction + i >= _currentActions.size())
		return;

	_remapTimeout = g_system->getMillis() + kRemapTimeoutDelay;
	Action *activeRemapAction = _currentActions[_topAction + i].action;
	_keymapWidgets[i].keyButton->setLabel("...");
	_keymapWidgets[i].keyButton->draw();
	_keymapper->startRemappingMode(activeRemapAction);

}

void RemapDialog::stopRemapping(bool force) {
	_topAction = -1;

	refreshKeymap();

	if (force)
		_keymapper->stopRemappingMode();
}

void RemapDialog::handleKeyDown(Common::KeyState state) {
	if (_keymapper->isRemapping())
		return;

	GUI::Dialog::handleKeyDown(state);
}

void RemapDialog::handleKeyUp(Common::KeyState state) {
	if (_keymapper->isRemapping())
		return;

	GUI::Dialog::handleKeyUp(state);
}

void RemapDialog::handleOtherEvent(Event ev) {
	if (ev.type == EVENT_GUI_REMAP_COMPLETE_ACTION) {
		// _keymapper is telling us that something changed
		_changes = true;
		stopRemapping();
	} else {
		GUI::Dialog::handleOtherEvent(ev);
	}
}

void RemapDialog::handleMouseDown(int x, int y, int button, int clickCount) {
	if (_keymapper->isRemapping())
		stopRemapping();
	else
		Dialog::handleMouseDown(x, y, button, clickCount);
}

void RemapDialog::handleTickle() {
	if (_keymapper->isRemapping() && g_system->getMillis() > _remapTimeout)
		stopRemapping(true);
	Dialog::handleTickle();
}

void RemapDialog::loadKeymap() {
	_currentActions.clear();
	const Stack<Keymapper::MapRecord> &activeKeymaps = _keymapper->getActiveStack();

	debug(3, "RemapDialog::loadKeymap active keymaps: %u", activeKeymaps.size());

	if (!activeKeymaps.empty() && _kmPopUp->getSelected() == 0) {
		// This is the "effective" view which shows all effective actions:
		// - all of the topmost keymap action
		// - all mapped actions that are reachable

		List<const HardwareInput *> freeInputs(_keymapper->getHardwareInputs());

		int topIndex = activeKeymaps.size() - 1;

		// This is a WORKAROUND for changing the popup list selected item and changing it back
		// to the top entry. Upon changing it back, the top keymap is always "gui".
		if (!_topKeymapIsGui && activeKeymaps[topIndex].keymap->getName() == kGuiKeymapName)
			--topIndex;

		// add most active keymap's keys
		Keymapper::MapRecord top = activeKeymaps[topIndex];
		List<Action *>::iterator actIt;
		debug(3, "RemapDialog::loadKeymap top keymap: %s", top.keymap->getName().c_str());
		for (actIt = top.keymap->getActions().begin(); actIt != top.keymap->getActions().end(); ++actIt) {
			Action *act = *actIt;
			ActionInfo info = {act, false, act->description};

			_currentActions.push_back(info);

			if (act->getMappedInput())
				freeInputs.remove(act->getMappedInput());
		}

		// loop through remaining finding mappings for unmapped keys
		if (top.transparent && topIndex >= 0) {
			for (int i = topIndex - 1; i >= 0; --i) {
				Keymapper::MapRecord mr = activeKeymaps[i];
				debug(3, "RemapDialog::loadKeymap keymap: %s", mr.keymap->getName().c_str());
				List<const HardwareInput *>::iterator inputIt = freeInputs.begin();
				const HardwareInput *input = *inputIt;
				while (inputIt != freeInputs.end()) {

					Action *act = 0;
					if (input->type == kHardwareInputTypeKeyboard)
						act = mr.keymap->getMappedAction(input->key);
					else if (input->type == kHardwareInputTypeGeneric)
						act = mr.keymap->getMappedAction(input->inputCode);

					if (act) {
						ActionInfo info = {act, true, act->description + " (" + mr.keymap->getName() + ")"};
						_currentActions.push_back(info);
						freeInputs.erase(inputIt);
					} else {
						++inputIt;
					}
				}

				if (mr.transparent == false || freeInputs.empty())
					break;
			}
		}

	} else if (_kmPopUp->getSelected() != -1) {
		// This is the regular view of a keymap that isn't the topmost one.
		// It shows all of that keymap's actions

		Keymap *km = _keymapTable[_kmPopUp->getSelectedTag()];

		List<Action *>::iterator it;

		for (it = km->getActions().begin(); it != km->getActions().end(); ++it) {
			ActionInfo info = {*it, false, (*it)->description};

			_currentActions.push_back(info);
		}
	}

	// refresh scroll bar
	_scrollBar->_currentPos = 0;
	_scrollBar->_numEntries = _currentActions.size();
	_scrollBar->recalc();

	// force refresh
	_topAction = -1;
	refreshKeymap();
}

void RemapDialog::refreshKeymap() {
	int newTopAction = _scrollBar->_currentPos;

	if (newTopAction == _topAction)
		return;

	_topAction = newTopAction;

	//_container->draw();
	_scrollBar->draw();

	uint actionI = _topAction;

	for (uint widgetI = 0; widgetI < _keymapWidgets.size(); widgetI++) {
		ActionWidgets& widg = _keymapWidgets[widgetI];

		if (actionI < _currentActions.size()) {
			debug(8, "RemapDialog::refreshKeymap actionI=%u", actionI);
			ActionInfo&    info = _currentActions[actionI];

			widg.actionText->setLabel(info.description);
			widg.actionText->setEnabled(!info.inherited);

			const HardwareInput *mappedInput = info.action->getMappedInput();

			if (mappedInput)
				widg.keyButton->setLabel(mappedInput->description);
			else
				widg.keyButton->setLabel("-");

			widg.actionText->setVisible(true);
			widg.keyButton->setVisible(true);
			widg.clearButton->setVisible(true);

			actionI++;
		} else {
			widg.actionText->setVisible(false);
			widg.keyButton->setVisible(false);
			widg.clearButton->setVisible(false);
		}
		//widg.actionText->draw();
		//widg.keyButton->draw();
	}
	// need to redraw entire Dialog so that invisible
	// widgets disappear
	draw();
}


} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
