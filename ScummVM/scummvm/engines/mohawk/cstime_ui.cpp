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

#include "mohawk/cstime_game.h"
#include "mohawk/cstime_ui.h"
#include "mohawk/cstime_view.h"
#include "mohawk/resource.h"
#include "common/algorithm.h" // find
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/fontman.h"

namespace Mohawk {

// read a null-terminated string from a stream
static Common::String readString(Common::SeekableReadStream *stream) {
	Common::String ret;
	while (!stream->eos()) {
		byte in = stream->readByte();
		if (!in)
			break;
		ret += in;
	}
	return ret;
}

CSTimeInterface::CSTimeInterface(MohawkEngine_CSTime *vm) : _vm(vm) {
	_sceneRect = Common::Rect(0, 0, 640, 340);
	_uiRect = Common::Rect(0, 340, 640, 480);

	_bookRect = Common::Rect(_uiRect.right - 95, _uiRect.top + 32, _uiRect.right - 25, _uiRect.top + 122);
	_noteRect = Common::Rect(_uiRect.left + 27, _uiRect.top + 31, _uiRect.left + 103, _uiRect.top + 131);
	_dialogTextRect = Common::Rect(0 + 125, 340 + 40, 640 - 125, 480 - 20);

	_cursorActive = false;
	_cursorShapes[0] = 0xFFFF;
	_cursorShapes[1] = 0xFFFF;
	_cursorShapes[2] = 0xFFFF;
	_cursorNextTime = 0;

	_help = new CSTimeHelp(_vm);
	_inventoryDisplay = new CSTimeInventoryDisplay(_vm, _dialogTextRect);
	_book = new CSTimeBook(_vm);
	_note = new CSTimeCarmenNote(_vm);
	_options = new CSTimeOptions(_vm);

	// The demo uses hardcoded system fonts
	if (!(_vm->getFeatures() & GF_DEMO)) {
		if (!_normalFont.loadFromFON("EvP14.fon"))
			error("failed to load normal font");
		if (!_dialogFont.loadFromFON("Int1212.fon"))
			error("failed to load dialog font");
		if (!_rolloverFont.loadFromFON("Int1818.fon"))
			error("failed to load rollover font");
	}

	_uiFeature = NULL;
	_dialogTextFeature = NULL;
	_rolloverTextFeature = NULL;
	_bubbleTextFeature = NULL;

	_mouseWasInScene = false;
	_state = kCSTimeInterfaceStateNormal;

	_dialogLines.resize(5);
	_dialogLineColors.resize(5);
}

CSTimeInterface::~CSTimeInterface() {
	delete _help;
	delete _inventoryDisplay;
	delete _book;
	delete _note;
	delete _options;
}

const Graphics::Font &CSTimeInterface::getNormalFont() const {
	// HACK: Use a ScummVM GUI font in place of a system one for the demo
	if (_vm->getFeatures() & GF_DEMO)
		return *FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);

	return _normalFont;
}

const Graphics::Font &CSTimeInterface::getDialogFont() const {
	// HACK: Use a ScummVM GUI font in place of a system one for the demo
	if (_vm->getFeatures() & GF_DEMO)
		return *FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);

	return _dialogFont;
}

const Graphics::Font &CSTimeInterface::getRolloverFont() const {
	// HACK: Use a ScummVM GUI font in place of a system one for the demo
	if (_vm->getFeatures() & GF_DEMO)
		return *FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);

	return _rolloverFont;
}

void CSTimeInterface::cursorInstall() {
	_vm->getView()->loadBitmapCursors(200);
}

void CSTimeInterface::cursorIdle() {
	if (!_cursorActive || _cursorShapes[1] == 0xFFFF)
		return;

	if (_vm->_system->getMillis() <= _cursorNextTime + 250)
		return;

	cursorSetShape(_cursorShapes[1], false);
	_cursorShapes[1] = _cursorShapes[2];
	_cursorShapes[2] = 0xFFFF;
}

void CSTimeInterface::cursorActivate(bool state) {
	_cursorActive = state;
}

void CSTimeInterface::cursorChangeShape(uint16 id) {
	if (_cursorShapes[1] == 0xFFFF)
		_cursorShapes[1] = id;
	else
		_cursorShapes[2] = id;
}

uint16 CSTimeInterface::cursorGetShape() {
	if (_cursorShapes[2] != 0xFFFF)
		return _cursorShapes[2];
	else if (_cursorShapes[1] != 0xFFFF)
		return _cursorShapes[1];
	else
		return _cursorShapes[0];
}

void CSTimeInterface::cursorSetShape(uint16 id, bool reset) {
	if (_cursorShapes[0] != id) {
		_cursorShapes[0] = id;
		_vm->getView()->setBitmapCursor(id);
		_cursorNextTime = _vm->_system->getMillis();
	}
}

void CSTimeInterface::cursorSetWaitCursor() {
	uint16 shape = cursorGetShape();
	switch (shape) {
	case 8:
		cursorChangeShape(9);
		break;
	case 9:
		break;
	case 11:
		cursorChangeShape(12);
		break;
	case 13:
		cursorChangeShape(15);
		break;
	default:
		cursorChangeShape(3);
		break;
	}
}

void CSTimeInterface::openResFile() {
	_vm->loadResourceFile("data/iface");
}

void CSTimeInterface::install() {
	uint16 resourceId = 100; // TODO
	_vm->getView()->installGroup(resourceId, 16, 0, true, 100);

	// TODO: store/reset these

	_dialogTextFeature = _vm->getView()->installViewFeature(0, 0, NULL);
	_dialogTextFeature->_data.bounds = _dialogTextRect;
	_dialogTextFeature->_data.bitmapIds[0] = 0;
	// We don't set a port.
	_dialogTextFeature->_moveProc = (Module::FeatureProc)&CSTimeModule::dialogTextMoveProc;
	_dialogTextFeature->_drawProc = (Module::FeatureProc)&CSTimeModule::dialogTextDrawProc;
	_dialogTextFeature->_timeProc = NULL;
	_dialogTextFeature->_flags = kFeatureOldSortForeground; // FIXME: not in original

	_rolloverTextFeature = _vm->getView()->installViewFeature(0, 0, NULL);
	_rolloverTextFeature->_data.bounds = Common::Rect(0 + 100, 340 + 5, 640 - 100, 480 - 25);
	_rolloverTextFeature->_data.bitmapIds[0] = 0;
	_rolloverTextFeature->_moveProc = (Module::FeatureProc)&CSTimeModule::rolloverTextMoveProc;
	_rolloverTextFeature->_drawProc = (Module::FeatureProc)&CSTimeModule::rolloverTextDrawProc;
	_rolloverTextFeature->_timeProc = NULL;
	_rolloverTextFeature->_flags = kFeatureOldSortForeground; // FIXME: not in original
}

void CSTimeInterface::draw() {
	// TODO
	if (!_uiFeature) {
		uint32 flags = kFeatureSortStatic | kFeatureNewNoLoop;
		assert(flags == 0x4800000);
		uint16 resourceId = 100; // TODO
		_uiFeature = _vm->getView()->installViewFeature(resourceId, flags, NULL);
		// TODO: special-case for case 20
	} else {
		_uiFeature->resetFeatureScript(1, 0);
		// TODO: special-case for case 20
	}

	// TODO: special-case for cases 19 and 20

	_note->drawSmallNote();
	_book->drawSmallBook();
	_inventoryDisplay->draw();
}

void CSTimeInterface::idle() {
	// TODO: inv sound handling

	_vm->getCase()->getCurrScene()->idle();
	_inventoryDisplay->idle();
	cursorIdle();

	_vm->getView()->idleView();
}

void CSTimeInterface::mouseDown(Common::Point pos) {
	_vm->resetTimeout();

	if (_options->getState()) {
		// TODO: _options->mouseDown(pos);
		return;
	}

	if (!cursorGetState())
		return;
	if (_vm->getCase()->getCurrScene()->eventIsActive())
		return;

	switch (cursorGetShape()) {
	case 1:
		cursorChangeShape(4);
		break;
	case 2:
		cursorChangeShape(5);
		break;
	case 13:
		cursorChangeShape(14);
		break;
	}

	if (_book->getState() == 2) {
		// TODO: _book->mouseDown(pos);
		return;
	}

	// TODO: if in sailing puzzle, sailing puzzle mouse down, return

	if (_note->getState() > 0) {
		return;
	}

	if (_sceneRect.contains(pos)) {
		_vm->getCase()->getCurrScene()->mouseDown(pos);
		return;
	}

	// TODO: case 20 ui craziness is handled seperately..

	CSTimeConversation *conv = _vm->getCase()->getCurrConversation();
	if (_bookRect.contains(pos) || (_noteRect.contains(pos) && _note->havePiece(0xffff))) {
		if (conv->getState() != (uint)~0)
			conv->end(false);
		if (_help->getState() != (uint)~0)
			_help->end();
		return;
	}

	if (_help->getState() != (uint)~0) {
		_help->mouseDown(pos);
		return;
	}

	if (conv->getState() != (uint)~0) {
		conv->mouseDown(pos);
		return;
	}

	// TODO: handle symbols

	if (_inventoryDisplay->_invRect.contains(pos)) {
		// TODO: special handling for case 6

		_inventoryDisplay->mouseDown(pos);
	}
}

void CSTimeInterface::mouseMove(Common::Point pos) {
	if (_options->getState()) {
		// TODO: _options->mouseMove(pos);
		return;
	}

	if (!cursorGetState())
		return;

	if (_mouseWasInScene && _uiRect.contains(pos)) {
		clearTextLine();
		_mouseWasInScene = false;
	}

	if (_book->getState() == 2) {
		// TODO: _book->mouseMove(pos);
		return;
	}

	if (_note->getState() == 2)
		return;

	// TODO: case 20 ui craziness is handled seperately..

	if (_sceneRect.contains(pos) && !_vm->getCase()->getCurrScene()->eventIsActive()) {
		_vm->getCase()->getCurrScene()->mouseMove(pos);
		_mouseWasInScene = true;
		return;
	}

	if (cursorGetShape() == 13) {
		cursorSetShape(1);
		return;
	} else if (cursorGetShape() == 14) {
		cursorSetShape(4);
		return;
	}

	bool mouseIsDown = _vm->getEventManager()->getButtonState() & 1;

	if (_book->getState() == 1 && !_bookRect.contains(pos)) {
		if (_state != kCSTimeInterfaceStateDragging) {
			clearTextLine();
			cursorSetShape(mouseIsDown ? 4 : 1);
			_book->setState(0);
		}
		return;
	}

	// TODO: case 20 ui craziness again

	if (_note->getState() == 1 && !_noteRect.contains(pos)) {
		if (_state != kCSTimeInterfaceStateDragging) {
			clearTextLine();
			cursorSetShape(mouseIsDown ? 4 : 1);
			_note->setState(0);
		}
		return;
	}

	// TODO: handle symbols

	if (_bookRect.contains(pos)) {
		if (_state != kCSTimeInterfaceStateDragging) {
			displayTextLine("Open Chronopedia");
			cursorSetShape(mouseIsDown ? 5 : 2);
			_book->setState(1);
		}
		return;
	}

	if (_noteRect.contains(pos)) {
		if (_state != kCSTimeInterfaceStateDragging && _note->havePiece(0xffff) && !_note->getState()) {
			displayTextLine("Look at Note");
			cursorSetShape(mouseIsDown ? 5 : 2);
			_note->setState(1);
		}
		return;
	}

	if (_vm->getCase()->getCurrConversation()->getState() != (uint)~0) {
		_vm->getCase()->getCurrConversation()->mouseMove(pos);
		return;
	}

	if (_help->getState() != (uint)~0) {
		_help->mouseMove(pos);
		return;
	}

	if (_state == kCSTimeInterfaceStateDragging) {
		// FIXME: dragging
		return;
	}

	// FIXME: if case is 20, we're done, return

	if (_inventoryDisplay->_invRect.contains(pos)) {
		_inventoryDisplay->mouseMove(pos);
	}
}

void CSTimeInterface::mouseUp(Common::Point pos) {
	if (_options->getState()) {
		// TODO: options->mouseUp(pos);
		return;
	}

	if (!cursorGetState())
		return;

	if (_state == kCSTimeInterfaceStateDragging) {
		stopDragging();
		return;
	}

	if (_state == kCSTimeInterfaceStateDragStart)
		_state = kCSTimeInterfaceStateNormal;

	switch (cursorGetShape()) {
	case 4:
		cursorChangeShape(1);
		break;
	case 5:
		cursorChangeShape(2);
		break;
	case 14:
		cursorChangeShape(13);
		break;
	}

	if (_vm->getCase()->getCurrScene()->eventIsActive()) {
		if (_vm->getCurrentEventType() == kCSTimeEventWaitForClick)
			_vm->mouseClicked();
		return;
	}

	if (_book->getState() == 2) {
		// TODO: _book->mouseUp(pos);
		return;
	}

	if (_note->getState() == 2) {
		_note->closeNote();
		mouseMove(pos);
		return;
	}

	// TODO: if in sailing puzzle, sailing puzzle mouse up, return

	// TODO: case 20 ui craziness is handled seperately..

	if (_sceneRect.contains(pos)) {
		_vm->getCase()->getCurrScene()->mouseUp(pos);
		return;
	}

	if (_vm->getCase()->getCurrConversation()->getState() != (uint)~0) {
		_vm->getCase()->getCurrConversation()->mouseUp(pos);
		return;
	}

	if (_help->getState() != (uint)~0) {
		_help->mouseUp(pos);
		return;
	}

	// TODO: handle symbols

	if (_bookRect.contains(pos)) {
		// TODO: handle flashing cuffs
		// TODO: _book->open();
		return;
	}

	if (_noteRect.contains(pos)) {
		// TODO: special-casing for case 19
		if (_note->havePiece(0xffff))
			_note->drawBigNote();
	}

	if (_inventoryDisplay->_invRect.contains(pos)) {
		// TODO: special-casing for case 6
		_inventoryDisplay->mouseUp(pos);
	}
}

void CSTimeInterface::cursorOverHotspot() {
	if (!cursorGetState())
		return;
	if (_state == kCSTimeInterfaceStateDragStart || _state == kCSTimeInterfaceStateDragging)
		return;
	if (cursorGetShape() == 3 || cursorGetShape() == 9)
		return;

	bool mouseIsDown = _vm->getEventManager()->getButtonState() & 1;
	if (mouseIsDown)
		cursorSetShape(5);
	else if (cursorGetShape() == 1)
		cursorChangeShape(2);
}

void CSTimeInterface::setCursorForCurrentPoint() {
	uint16 shape = 1;

	Common::Point mousePos = _vm->getEventManager()->getMousePos();
	if (_bookRect.contains(mousePos)) {
		shape = 2;
	} else {
		uint convState = _vm->getCase()->getCurrConversation()->getState();
		uint helpState = _help->getState();
		// TODO: symbols too
		if (convState == (uint)~0 || convState == 0 || helpState == (uint)~0 || helpState == 0) {
			// FIXME: set cursor to 2 if inventory display occupied
		} else if (convState == 1 || helpState == 1) {
			// FIXME: set cursor to 2 if over conversation rect
		}
	}

	cursorSetShape(shape);
}

void CSTimeInterface::clearTextLine() {
	_rolloverText.clear();
}

void CSTimeInterface::displayTextLine(Common::String text) {
	_rolloverText = text;
}

void CSTimeInterface::clearDialogArea() {
	_dialogLines.clear();
	_dialogLines.resize(5);
	// TODO: dirty feature
}

void CSTimeInterface::clearDialogLine(uint line) {
	_dialogLines[line].clear();
	// TODO: dirty feature
}

void CSTimeInterface::displayDialogLine(uint16 id, uint line, byte color) {
	Common::SeekableReadStream *stream = _vm->getResource(ID_STRI, id);
	Common::String text = readString(stream);
	delete stream;

	_dialogLines[line] = text;
	_dialogLineColors[line] = color;
	// TODO: dirty feature
}

void CSTimeInterface::drawTextIdToBubble(uint16 id) {
	Common::SeekableReadStream *stream = _vm->getResource(ID_STRI, id);
	Common::String text = readString(stream);
	delete stream;

	drawTextToBubble(&text);
}

void CSTimeInterface::drawTextToBubble(Common::String *text) {
	if (_bubbleTextFeature)
		error("Attempt to display two text objects");
	if (!text)
		text = &_bubbleText;
	if (text->empty())
		return;

	_currentBubbleText = *text;

	uint bubbleId = _vm->getCase()->getCurrScene()->getBubbleType();
	Common::Rect bounds;
	switch (bubbleId) {
	case 0:
		bounds = Common::Rect(15, 7, 625, 80);
		break;
	case 1:
		bounds = Common::Rect(160, 260, 625, 333);
		break;
	case 2:
		bounds = Common::Rect(356, 3, 639, 90);
		break;
	case 3:
		bounds = Common::Rect(10, 7, 380, 80);
		break;
	case 4:
		bounds = Common::Rect(15, 270, 625, 328);
		break;
	case 5:
		bounds = Common::Rect(15, 7, 550, 70);
		break;
	case 6:
		bounds = Common::Rect(0, 0, 313, 76);
		break;
	case 7:
		bounds = Common::Rect(200, 25, 502, 470);
		break;
	default:
		error("unknown bubble type %d in drawTextToBubble", bubbleId);
	}

	_bubbleTextFeature = _vm->getView()->installViewFeature(0, 0, NULL);
	_bubbleTextFeature->_data.bounds = bounds;
	_bubbleTextFeature->_data.bitmapIds[0] = 0;
	_bubbleTextFeature->_moveProc = (Module::FeatureProc)&CSTimeModule::bubbleTextMoveProc;
	_bubbleTextFeature->_drawProc = (Module::FeatureProc)&CSTimeModule::bubbleTextDrawProc;
	_bubbleTextFeature->_timeProc = NULL;
	_bubbleTextFeature->_flags = kFeatureOldSortForeground; // FIXME: not in original
}

void CSTimeInterface::closeBubble() {
	if (_bubbleTextFeature)
		_vm->getView()->removeFeature(_bubbleTextFeature, true);
	_bubbleTextFeature = NULL;
}

void CSTimeInterface::startDragging(uint16 id) {
	CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[id];

	cursorSetShape(11);
	_draggedItem = id;

	if (_draggedItem == TIME_CUFFS_ID) {
		if (_inventoryDisplay->getCuffsShape() == 11) {
			_inventoryDisplay->setCuffsFlashing();
			_vm->getView()->idleView();
		}
	}

	uint32 dragFlags = (grabbedFromInventory() ? 0x800 : 0x600);
	_vm->getView()->dragFeature((NewFeature *)invObj->feature, _grabPoint, 4, dragFlags, NULL);

	if (_vm->getCase()->getId() == 1 && id == 2) {
		// Hardcoded behavior for the torch in the first case.
		if (_vm->getCase()->getCurrScene()->getId() == 4) {
			// This is the dark tomb.
			// FIXME: apply torch hack
			_vm->_caseVariable[2]++;
		} else {
			// FIXME: unapply torch hack
		}
	}

	_state = kCSTimeInterfaceStateDragging;

	if (grabbedFromInventory())
		return;

	// Hide the associated scene feature, if there is one.
	if (invObj->featureId != 0xffff) {
		CSTimeEvent event;
		event.type = kCSTimeEventDisableFeature;
		event.param2 = invObj->featureId;
		_vm->addEvent(event);
	}

	_vm->addEventList(invObj->events);
}

void CSTimeInterface::stopDragging() {
	CSTimeScene *scene = _vm->getCase()->getCurrScene();
	CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[_draggedItem];

	Common::Point mousePos = _vm->_system->getEventManager()->getMousePos();
	_state = kCSTimeInterfaceStateNormal;

	if (_sceneRect.contains(mousePos))
		scene->setCursorForCurrentPoint();
	else
		setCursorForCurrentPoint();

	// Find the inventory object hotspot which is topmost for this drop, if any.
	uint16 foundInvObjHotspot = 0xffff;
	const Common::Array<CSTimeHotspot> &hotspots = scene->getHotspots();
	for (uint i = 0; i < hotspots.size(); i++) {
		if (hotspots[i].state != 1)
			continue;
		if (!hotspots[i].region.containsPoint(mousePos))
			continue;
		for (uint j = 0; j < invObj->hotspots.size(); j++) {
			if (invObj->hotspots[j].sceneId != scene->getId())
				continue;
			if (invObj->hotspots[j].hotspotId != i)
				continue;
			if (foundInvObjHotspot != 0xffff && invObj->hotspots[foundInvObjHotspot].hotspotId < invObj->hotspots[j].hotspotId)
				continue;
			foundInvObjHotspot = j;
		}
	}

	// Work out if we're going to consume (nom-nom) the object after the drop.
	bool consumeObj = false;
	bool runConsumeEvents = false;
	if (foundInvObjHotspot != 0xffff) {
		CSTimeInventoryHotspot &hotspot = invObj->hotspots[foundInvObjHotspot];

		clearTextLine();
		for (uint i = 0; i < invObj->locations.size(); i++) {
			if (invObj->locations[i].sceneId != hotspot.sceneId)
				continue;
			if (invObj->locations[i].hotspotId != hotspot.hotspotId)
				continue;
			consumeObj = true;
			break;
		}

		if (_draggedItem == TIME_CUFFS_ID && !_inventoryDisplay->getCuffsState()) {
			consumeObj = false;
			// Nuh-uh, they're not activated yet.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, _vm->getCase()->getCurrScene()->getHelperId(), 9902));
		} else {
			// FIXME: ding();
			runConsumeEvents = true;
		}
	}

	// Deal with the actual drop.
	if (grabbedFromInventory()) {
		if (!consumeObj) {
			_vm->getView()->dragFeature((NewFeature *)invObj->feature, mousePos, 2, 0x800, NULL);
			// TODO: playSound(151);
		} else if (_draggedItem != TIME_CUFFS_ID) {
			_vm->getView()->dragFeature((NewFeature *)invObj->feature, mousePos, 2, 0x600, NULL);
			_vm->_haveInvItem[_draggedItem] = 0;
			invObj->feature = NULL;
			invObj->featureDisabled = true;
			_inventoryDisplay->removeItem(_draggedItem);
		} else if (!_inventoryDisplay->getCuffsState()) {
			// Inactive cuffs.
			// TODO: We never actually get here? Which would explain why it makes no sense.
			_vm->getView()->dragFeature((NewFeature *)invObj->feature, mousePos, 2, 0x800, NULL);
			invObj->feature = NULL;
		} else {
			// Active cuffs.
			_vm->getView()->dragFeature((NewFeature *)invObj->feature, mousePos, 2, 0x600, NULL);
			_vm->_haveInvItem[_draggedItem] = 0;
			invObj->feature = NULL;
			invObj->featureDisabled = true;
		}

		if (runConsumeEvents) {
			_vm->addEventList(invObj->hotspots[foundInvObjHotspot].events);
		}

		_inventoryDisplay->draw();
	} else {
		uint32 dragFlags = 0x600;
		_vm->getView()->dragFeature((NewFeature *)invObj->feature, mousePos, 2, dragFlags, NULL);

		if (_inventoryDisplay->_invRect.contains(mousePos)) {
			// Dropped into the inventory.
			invObj->feature = NULL;
			if (invObj->canTake) {
				dropItemInInventory(_draggedItem);
				if (invObj->hotspotId)
					_vm->addEvent(CSTimeEvent(kCSTimeEventDisableHotspot, 0xffff, invObj->hotspotId));
			} else {
				if (invObj->featureId)
					_vm->addEvent(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, invObj->featureId));
			}

			for (uint i = 0; i < invObj->hotspots.size(); i++) {
				if (invObj->hotspots[i].sceneId != scene->getId())
					continue;
				if (invObj->hotspots[i].hotspotId != 0xffff)
					continue;
				_vm->addEventList(invObj->hotspots[i].events);
			}
		} else {
			// Dropped into the scene.

			CSTimeEvent event;
			event.param1 = 0xffff;
			if (consumeObj) {
				invObj->feature = NULL;
				invObj->featureDisabled = true;
				event.type = kCSTimeEventDisableHotspot;
				event.param2 = invObj->hotspotId;
			} else {
				invObj->feature = NULL;
				event.type = kCSTimeEventAddFeature;
				event.param2 = invObj->featureId;
			}
			_vm->addEvent(event);

			if (runConsumeEvents) {
				_vm->addEventList(invObj->hotspots[foundInvObjHotspot].events);
			} else {
				for (uint i = 0; i < invObj->hotspots.size(); i++) {
					if (invObj->hotspots[i].sceneId != scene->getId())
						continue;
					if (invObj->hotspots[i].hotspotId != 0xfffe)
						continue;
					_vm->addEventList(invObj->hotspots[i].events);
				}
			}
		}
	}

	if (_vm->getCase()->getId() == 1 && _vm->getCase()->getCurrScene()->getId() == 4) {
		// Hardcoded behavior for torches in the dark tomb, in the first case.
		if (_draggedItem == 1 && foundInvObjHotspot == 0xffff) {
			// Trying to drag an unlit torch around?
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 0, 16352));
		} else if (_draggedItem == 2 && _vm->_caseVariable[2] == 1) {
			// This the first time we tried dragging the lit torch around.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 0, 16354));
		}
	}

	// TODO: Is this necessary?
	_draggedItem = 0xffff;
}

void CSTimeInterface::setGrabPoint() {
	_grabPoint = _vm->_system->getEventManager()->getMousePos();
}

bool CSTimeInterface::grabbedFromInventory() {
	return (_inventoryDisplay->_invRect.contains(_grabPoint));
}

void CSTimeInterface::dropItemInInventory(uint16 id) {
	if (_vm->_haveInvItem[id])
		return;

	_vm->_haveInvItem[id] = 1;
	_vm->getCase()->_inventoryObjs[id]->feature = NULL;

	_inventoryDisplay->insertItemInDisplay(id);

	// TODO: deal with symbols

	if (_vm->getCase()->getCurrConversation()->getState() == (uint)~0 || _vm->getCase()->getCurrConversation()->getState() == 0) {
		// FIXME: additional check here
		// FIXME: play sound 151?
		_inventoryDisplay->draw();
		return;
	}

	// FIXME: ding();
	clearDialogArea();
	_inventoryDisplay->show();
	_inventoryDisplay->draw();
	_inventoryDisplay->setState(4);
}

CSTimeHelp::CSTimeHelp(MohawkEngine_CSTime *vm) : _vm(vm) {
	_state = (uint)~0;
	_currEntry = 0xffff;
	_currHover = 0xffff;
	_nextToProcess = 0xffff;
}

CSTimeHelp::~CSTimeHelp() {
}

void CSTimeHelp::addQaR(uint16 text, uint16 speech) {
	CSTimeHelpQaR qar;
	qar.text = text;
	qar.speech = speech;
	_qars.push_back(qar);
}

void CSTimeHelp::start() {
	if (_vm->getInterface()->getInventoryDisplay()->getState() == 4)
		return;

	_state = 2;

	uint16 speech = 5900 + _vm->_rnd->getRandomNumberRng(0, 2);
	_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, _vm->getCase()->getCurrScene()->getHelperId(), speech));

	if (noHelperChanges())
		return;

	// Play a NIS, making sure the Good Guide is disabled.
	_vm->addEvent(CSTimeEvent(kCSTimeEventCharSetState, _vm->getCase()->getCurrScene()->getHelperId(), 0));
	_vm->addEvent(CSTimeEvent(kCSTimeEventCharPlayNIS, _vm->getCase()->getCurrScene()->getHelperId(), 0));
	_vm->addEvent(CSTimeEvent(kCSTimeEventCharSetState, _vm->getCase()->getCurrScene()->getHelperId(), 0));
}

void CSTimeHelp::end(bool runEvents) {
	_state = (uint)~0;
	_currHover = 0xffff;

	_vm->getInterface()->clearDialogArea();
	_vm->getInterface()->getInventoryDisplay()->show();

	if (noHelperChanges())
		return;

	_vm->addEvent(CSTimeEvent(kCSTimeEventCharSetState, _vm->getCase()->getCurrScene()->getHelperId(), 1));
	_vm->addEvent(CSTimeEvent(kCSTimeEventCharSomeNIS55, _vm->getCase()->getCurrScene()->getHelperId(), 1));
}

void CSTimeHelp::cleanupAfterFlapping() {
	if (_state == 2) {
		// Startup.
		_vm->getInterface()->getInventoryDisplay()->hide();
		selectStrings();
		display();
		_state = 1;
		return;
	}

	if (_nextToProcess == 0xffff)
		return;

	unhighlightLine(_nextToProcess);
	_nextToProcess = 0xffff;

	// TODO: case 18 hard-coding
}

void CSTimeHelp::mouseDown(Common::Point &pos) {
	for (uint i = 0; i < _qars.size(); i++) {
		Common::Rect thisRect = _vm->getInterface()->_dialogTextRect;
		thisRect.top += 1 + i*15;
		thisRect.bottom = thisRect.top + 15;
		if (!thisRect.contains(pos))
			continue;

		_currEntry = i;
		highlightLine(i);
		_vm->getInterface()->cursorSetShape(5);
	}
}

void CSTimeHelp::mouseMove(Common::Point &pos) {
	bool mouseIsDown = _vm->getEventManager()->getButtonState() & 1;

	for (uint i = 0; i < _qars.size(); i++) {
		Common::Rect thisRect = _vm->getInterface()->_dialogTextRect;
		thisRect.top += 1 + i*15;
		thisRect.bottom = thisRect.top + 15;
		if (!thisRect.contains(pos))
			continue;

		if (mouseIsDown) {
			if (i != _currEntry)
				break;
			highlightLine(i);
		}

		_vm->getInterface()->cursorOverHotspot();
		_currHover = i;
		return;
	}

	if (_currHover != 0xffff) {
		if (_vm->getInterface()->cursorGetShape() != 3) {
			unhighlightLine(_currHover);
			_vm->getInterface()->cursorSetShape(1);
		}
		_currHover = 0xffff;
	}
}

void CSTimeHelp::mouseUp(Common::Point &pos) {
	if (_currEntry == 0xffff || _qars[_currEntry].speech == 0) {
		_vm->getInterface()->cursorSetShape(1);
		end();
		return;
	}

	Common::Rect thisRect = _vm->getInterface()->_dialogTextRect;
	thisRect.top += 1 + _currEntry*15;
	thisRect.bottom = thisRect.top + 15;
	if (!thisRect.contains(pos))
		return;

	_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, _vm->getCase()->getCurrScene()->getHelperId(), 5900 + _qars[_currEntry].speech));
	_nextToProcess = _currEntry;
	_askedAlready.push_back(_qars[_currEntry].text);
}

void CSTimeHelp::display() {
	_vm->getInterface()->clearDialogArea();

	for (uint i = 0; i < _qars.size(); i++) {
		uint16 text = _qars[i].text;
		bool askedAlready = Common::find(_askedAlready.begin(), _askedAlready.end(), text) != _askedAlready.end();

		_vm->getInterface()->displayDialogLine(5900 + text, i, askedAlready ? 13 : 32);
	}
}

void CSTimeHelp::highlightLine(uint line) {
	uint16 text = _qars[line].text;
	_vm->getInterface()->displayDialogLine(5900 + text, line, 244);
}

void CSTimeHelp::unhighlightLine(uint line) {
	uint16 text = _qars[line].text;
	bool askedAlready = Common::find(_askedAlready.begin(), _askedAlready.end(), text) != _askedAlready.end();
	_vm->getInterface()->displayDialogLine(5900 + text, line, askedAlready ? 13 : 32);
}


void CSTimeHelp::selectStrings() {
	_qars.clear();
	_vm->getCase()->selectHelpStrings();
}

bool CSTimeHelp::noHelperChanges() {
	// These are hardcoded.
	if (_vm->getCase()->getId() == 4 && _vm->getCase()->getCurrScene()->getId() == 5)
		return true;
	if (_vm->getCase()->getId() == 5)
		return true;
	if (_vm->getCase()->getId() == 14 && _vm->getCase()->getCurrScene()->getId() == 4)
		return true;
	if (_vm->getCase()->getId() == 17 && _vm->getCase()->getCurrScene()->getId() == 2)
		return true;

	return false;
}

CSTimeInventoryDisplay::CSTimeInventoryDisplay(MohawkEngine_CSTime *vm, Common::Rect baseRect) : _vm(vm) {
	_state = 0;
	_cuffsState = false;
	_cuffsShape = 10;

	_invRect = baseRect;

	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++) {
		_itemRect[i].left = baseRect.left + 15 + i * 92;
		_itemRect[i].top = baseRect.top + 5;
		_itemRect[i].right = _itemRect[i].left + 90;
		_itemRect[i].bottom = _itemRect[i].top + 70;
	}
}

CSTimeInventoryDisplay::~CSTimeInventoryDisplay() {
}

void CSTimeInventoryDisplay::install() {
	uint count = _vm->getCase()->_inventoryObjs.size() - 1;

	// FIXME: some cases have hard-coded counts

	_vm->getView()->installGroup(9000, count, 0, true, 9000);
}

void CSTimeInventoryDisplay::draw() {
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++) {
		if (_displayedItems[i] == 0xffff)
			continue;
		CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[_displayedItems[i]];

		if (invObj->featureDisabled)
			continue;

		if (invObj->feature) {
			invObj->feature->resetFeatureScript(1, 0);
			continue;
		}

		// TODO: 0x2000 is set! help?
		uint32 flags = kFeatureSortStatic | kFeatureNewNoLoop | 0x2000;
		if (i == TIME_CUFFS_ID) {
			// Time Cuffs are handled differently.
			// TODO: Can we not use _cuffsShape here?
			uint16 id = 100 + 10;
			if (_cuffsState) {
				id = 100 + 12;
				flags &= ~kFeatureNewNoLoop;
			}
			invObj->feature = _vm->getView()->installViewFeature(id, flags, NULL);
		} else {
			Common::Point pos((_itemRect[i].left + _itemRect[i].right) / 2, (_itemRect[i].top + _itemRect[i].bottom) / 2);
			uint16 id = 9000 + (invObj->id - 1);
			invObj->feature = _vm->getView()->installViewFeature(id, flags, &pos);
		}
	}
}

void CSTimeInventoryDisplay::show() {
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++) {
		if (_displayedItems[i] == 0xffff)
			continue;
		CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[_displayedItems[i]];
		if (!invObj->feature)
			continue;
		invObj->feature->show();
	}
}

void CSTimeInventoryDisplay::hide() {
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++) {
		if (_displayedItems[i] == 0xffff)
			continue;
		CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[_displayedItems[i]];
		if (!invObj->feature)
			continue;
		invObj->feature->hide(true);
	}
}

void CSTimeInventoryDisplay::idle() {
	if (_vm->getInterface()->getCarmenNote()->getState() ||
		_vm->getCase()->getCurrConversation()->getState() != 0xffff ||
		_vm->getInterface()->getHelp()->getState() != 0xffff) {
		if (_state == 4) {
			// FIXME: check timeout!
			hide();
			_vm->getCase()->getCurrConversation()->display();
			_state = 0;
		}
		return;
	}

	if (!_state)
		return;

	// FIXME: deal with actual inventory stuff
}

void CSTimeInventoryDisplay::clearDisplay() {
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++)
		_displayedItems[i] = 0xffff;

	// We always start out with the Time Cuffs.
	_vm->_haveInvItem[TIME_CUFFS_ID] = 1;
	insertItemInDisplay(TIME_CUFFS_ID);

	_cuffsState = false;
}

void CSTimeInventoryDisplay::insertItemInDisplay(uint16 id) {
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++)
		if (_displayedItems[i] == id)
			return;

	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++)
		if (_displayedItems[i] == 0xffff) {
			_displayedItems[i] = id;
			return;
		}

	error("couldn't insert item into display");
}

void CSTimeInventoryDisplay::removeItem(uint16 id) {
	CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[id];
	if (invObj->feature) {
		_vm->getView()->removeFeature(invObj->feature, true);
		invObj->feature = NULL;
	}
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++)
		if (_displayedItems[i] == id)
			_displayedItems[i] = 0xffff;
}

void CSTimeInventoryDisplay::mouseDown(Common::Point &pos) {
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++) {
		if (_displayedItems[i] == 0xffff)
			continue;
		if (!_itemRect[i].contains(pos))
			continue;
		_draggedItem = i;
		_vm->getInterface()->cursorSetShape(8);
		_vm->getInterface()->setGrabPoint();
		_vm->getInterface()->setState(kCSTimeInterfaceStateDragStart);
	}
}

void CSTimeInventoryDisplay::mouseMove(Common::Point &pos) {
	bool mouseIsDown = _vm->getEventManager()->getButtonState() & 1;
	if (mouseIsDown && _vm->getInterface()->cursorGetShape() == 8) {
		Common::Point grabPoint = _vm->getInterface()->getGrabPoint();
		if (mouseIsDown && (abs(grabPoint.x - pos.x) > 2 || abs(grabPoint.y - pos.y) > 2)) {
			if (_vm->getInterface()->grabbedFromInventory()) {
				_vm->getInterface()->startDragging(getLastDisplayedClicked());
			} else {
				// TODO: CSTimeScene::mouseMove does quite a lot more, why not here too?
				_vm->getInterface()->startDragging(_vm->getCase()->getCurrScene()->getInvObjForCurrentHotspot());
			}
		}
	}

	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++) {
		if (_displayedItems[i] == 0xffff)
			continue;
		if (!_itemRect[i].contains(pos))
			continue;
		CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[_displayedItems[i]];
		Common::String text = "Pick up ";
		// TODO: special-case for case 11, scene 3, inv obj 1 (just use "Read " instead)
		text += _vm->getCase()->getRolloverText(invObj->stringId);
		_vm->getInterface()->displayTextLine(text);
		_vm->getInterface()->cursorOverHotspot();
		// FIXME: there's some trickery here to store the id for the below
		return;
	}

	if (false /* FIXME: if we get here and the stored id mentioned above is set.. */) {
		_vm->getInterface()->clearTextLine();
		if (_vm->getInterface()->getState() != kCSTimeInterfaceStateDragging) {
			if (_vm->getInterface()->cursorGetShape() != 3 && _vm->getInterface()->cursorGetShape() != 9)
				_vm->getInterface()->cursorSetShape(1);
		}
		// FIXME: reset that stored id
	}
}

void CSTimeInventoryDisplay::mouseUp(Common::Point &pos) {
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++) {
		if (_displayedItems[i] == 0xffff)
			continue;
		if (!_itemRect[i].contains(pos))
			continue;
		// TODO: instead, stupid hack for case 11, scene 3, inv obj 1 (kCSTimeEventUnknown39, 0xffff, 0xffff)
		// TODO: instead, stupid hack for case 18, scene not 3, inv obj 4 (kCSTimeEventCondition, 1, 29)
		CSTimeEvent event;
		event.param1 = _vm->getCase()->getCurrScene()->getHelperId();
		if (event.param1 == 0xffff)
			event.type = kCSTimeEventSpeech;
		else
			event.type = kCSTimeEventCharStartFlapping;
		if (i == TIME_CUFFS_ID) {
			if (_cuffsState)
				event.param2 = 9903;
			else
				event.param2 = 9902;
		} else {
			event.param2 = 9905 + _displayedItems[i];
		}
		_vm->addEvent(event);
	}
}

void CSTimeInventoryDisplay::activateCuffs(bool active) {
	_cuffsState = active;
	if (!_cuffsState)
		return;

	CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[TIME_CUFFS_ID];
	_cuffsShape = 11;
	if (invObj->feature)
		_vm->getView()->removeFeature(invObj->feature, true);
	uint32 flags = kFeatureSortStatic | kFeatureNewNoLoop;
	invObj->feature = _vm->getView()->installViewFeature(100 + _cuffsShape, flags, NULL);
	invObj->featureDisabled = false;
}

void CSTimeInventoryDisplay::setCuffsFlashing() {
	CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[TIME_CUFFS_ID];
	_cuffsShape = 12;
	if (invObj->feature)
		_vm->getView()->removeFeature(invObj->feature, true);
	uint32 flags = kFeatureSortStatic | 0x2000;
	invObj->feature = _vm->getView()->installViewFeature(100 + _cuffsShape, flags, NULL);
	invObj->featureDisabled = false;
}

bool CSTimeInventoryDisplay::isItemDisplayed(uint16 id) {
	for (uint i = 0; i < MAX_DISPLAYED_ITEMS; i++) {
		if (_displayedItems[i] == id)
			return true;
	}

	return false;
}

CSTimeBook::CSTimeBook(MohawkEngine_CSTime *vm) : _vm(vm) {
	_state = 0;
	_smallBookFeature = NULL;
}

CSTimeBook::~CSTimeBook() {
}

void CSTimeBook::drawSmallBook() {
	if (!_smallBookFeature) {
		_smallBookFeature = _vm->getView()->installViewFeature(101, kFeatureSortStatic | kFeatureNewNoLoop, NULL);
	} else {
		_smallBookFeature->resetFeature(false, NULL, 0);
	}
}

CSTimeCarmenNote::CSTimeCarmenNote(MohawkEngine_CSTime *vm) : _vm(vm) {
	_state = 0;
	_feature = NULL;
	clearPieces();
}

CSTimeCarmenNote::~CSTimeCarmenNote() {
}

void CSTimeCarmenNote::clearPieces() {
	for (uint i = 0; i < NUM_NOTE_PIECES; i++)
		_pieces[i] = 0xffff;
}

bool CSTimeCarmenNote::havePiece(uint16 piece) {
	for (uint i = 0; i < NUM_NOTE_PIECES; i++) {
		if (piece == 0xffff) {
			if (_pieces[i] != 0xffff)
				return true;
		} else if (_pieces[i] == piece)
			return true;
	}

	return false;
}

void CSTimeCarmenNote::addPiece(uint16 piece, uint16 speech) {
	uint i;
	for (i = 0; i < NUM_NOTE_PIECES; i++) {
		if (_pieces[i] == 0xffff) {
			_pieces[i] = piece;
			break;
		}
	}
	if (i == NUM_NOTE_PIECES)
		error("addPiece couldn't add piece to carmen note");

	// Get the Good Guide to say something.
	if (i == 2)
		speech = 9900; // Found the third piece.
	if (speech != 0xffff)
		_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, _vm->getCase()->getCurrScene()->getHelperId(), speech));

	// Remove the note feature, if any.
	uint16 noteFeatureId = _vm->getCase()->getNoteFeatureId(piece);
	if (noteFeatureId != 0xffff)
		_vm->addEvent(CSTimeEvent(kCSTimeEventDisableFeature, 0xffff, noteFeatureId));

	_vm->addEvent(CSTimeEvent(kCSTimeEventShowBigNote, 0xffff, 0xffff));

	if (i != 2)
		return;

	// TODO: special-casing for case 5

	_vm->addEvent(CSTimeEvent(kCSTimeEventCharPlayNIS, _vm->getCase()->getCurrScene()->getHelperId(), 3));

	// TODO: special-casing for case 5

	_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, _vm->getCase()->getCurrScene()->getHelperId(), 9901));
	_vm->addEvent(CSTimeEvent(kCSTimeEventActivateCuffs, 0xffff, 0xffff));
}

void CSTimeCarmenNote::drawSmallNote() {
	if (!havePiece(0xffff))
		return;

	uint16 id = 100;
	if (_pieces[2] != 0xffff)
		id += 5;
	else if (_pieces[1] != 0xffff)
		id += 4;
	else
		id += 2;

	if (_feature)
		_vm->getView()->removeFeature(_feature, true);
	_feature = _vm->getView()->installViewFeature(id, kFeatureSortStatic | kFeatureNewNoLoop, NULL);
}

void CSTimeCarmenNote::drawBigNote() {
	if (_vm->getCase()->getCurrConversation()->getState() != (uint)~0) {
		_vm->getCase()->getCurrConversation()->end(false);
	} else if (_vm->getInterface()->getHelp()->getState() != (uint)~0) {
		_vm->getInterface()->getHelp()->end();
	}
	// TODO: kill symbols too

	uint16 id = 100;
	if (_pieces[2] != 0xffff)
		id += 9;
	else if (_pieces[1] != 0xffff)
		id += 8;
	else
		id += 6;

	if (_feature)
		_vm->getView()->removeFeature(_feature, true);
	_feature = _vm->getView()->installViewFeature(id, kFeatureSortStatic | kFeatureNewNoLoop, NULL);
	// FIXME: attach note drawing proc
	_state = 2;
}

void CSTimeCarmenNote::closeNote() {
	_state = 0;
	drawSmallNote();
}

CSTimeOptions::CSTimeOptions(MohawkEngine_CSTime *vm) : _vm(vm) {
	_state = 0;
}

CSTimeOptions::~CSTimeOptions() {
}

} // End of namespace Mohawk
