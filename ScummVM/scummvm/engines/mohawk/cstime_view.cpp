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

#include "mohawk/cstime_game.h" // debugging..
#include "mohawk/cstime_ui.h"
#include "mohawk/cstime_view.h"
#include "mohawk/resource.h"
#include "mohawk/cursors.h"

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Mohawk {

CSTimeView::CSTimeView(MohawkEngine_CSTime *vm) : View(vm) {
	_timeVm = vm;
	_gfx = vm->_gfx;
	_bitmapCursorId = 0;
}

uint32 CSTimeView::getTime() {
	return _vm->_system->getMillis();
}

void CSTimeView::setupView() {
	_rootNode = new NewFeature(this);
	_cursorNode = new NewFeature(this);
	_rootNode->setNodeDefaults(NULL, _cursorNode);
	_rootNode->_id = 1; // TODO: 10 in new?
	_rootNode->_data.enabled = 0;
	_rootNode->_flags = kFeatureSortBackground;
	_rootNode->_moveProc = NULL;
	_rootNode->_drawProc = NULL;
	_rootNode->_timeProc = NULL;
	_cursorNode->setNodeDefaults(_rootNode, NULL);
	_cursorNode->_id = 0xffff; // TODO: 1 in new?
	_cursorNode->_data.enabled = 0;
	_cursorNode->_flags = kFeatureOldSortForeground; // TODO: 0x4000 in new..
	_cursorNode->_moveProc = (Module::FeatureProc)&CSTimeModule::cursorMoveProc;
	_cursorNode->_drawProc = (Module::FeatureProc)&CSTimeModule::cursorDrawProc;
	_cursorNode->_timeProc = NULL;
}

Feature *CSTimeView::installViewFeature(uint16 scrbId, uint32 flags, Common::Point *pos) {
	Feature *node = _rootNode;
	// FIXME: find the right node to insert under
	while (node) {
		if (node->_next && (node->_next->_id == 0xffff || ((flags & 0x8000) && !(node->_next->_flags & 0x8000))))
			break;
		node = node->_next;
	}
	if (!node)
		error("failed to install view feature");

	NewFeature *feature = new NewFeature(this);
	feature->setNodeDefaults(node, node->_next);

	feature->_moveProc = (Module::FeatureProc)&CSTimeModule::defaultMoveProc;
	feature->_drawProc = (Module::FeatureProc)&CSTimeModule::defaultDrawProc;
	feature->_timeProc = (Module::BooleanProc)&CSTimeModule::defaultTimeProc;
	feature->_pickupProc = (Module::PickupProc)&CSTimeModule::defaultPickupProc;
	feature->_dropProc = (Module::FeatureProc)&CSTimeModule::defaultDropProc;
	feature->_dragMoveProc = (Module::FeatureProc)&CSTimeModule::defaultDragMoveProc;
	feature->_oldMoveProc = NULL;
	feature->_dragFlags = 0x8000;

	feature->_id = getNewFeatureId();
	node->_next = feature;
	feature->_next->_prev = feature;
	if (pos) {
		feature->_data.currentPos = *pos;
		feature->_unknown168 = 1;
	} else {
		feature->_data.currentPos = Common::Point();
		feature->_unknown168 = 0x7FFFFFFF;
	}
	feature->_data.nextPos = Common::Point();
	feature->_scrbId = scrbId;
	feature->_flags = flags;
	feature->_delayTime = 100;
	return feature;
}

void CSTimeView::installGroup(uint16 resourceId, uint size, uint count, bool regs, uint16 baseId) {
	// TODO: make sure this is in sync!
	assert(_numSCRBGroups < 14);
	installFeatureShapes(regs, _numSCRBGroups, resourceId);
	if (baseId == 0xffff)
		baseId = resourceId;
	_SCRBGroupResources[_numSCRBGroups] = resourceId; // TODO: Meh. This needs some rethinking.
	installGroupOfSCRBs(false, baseId, size, count);
}

void CSTimeView::removeGroup(uint16 resourceId) {
	// FIXME: deal with zero resourceId
	if (resourceId == 0)
		error("removeGroup got zero resourceId");

	uint16 groupId = getGroupFromBaseId(resourceId);
	if (groupId == 0xffff)
		return;

	removeObjectsUsingBaseId(resourceId);
	freeShapesUsingResourceId(resourceId);
	freeScriptsUsingResourceId(resourceId);
	// adjustShapeGroups(groupId); - TODO: unnecessary?
}

void CSTimeView::removeObjectsUsingBaseId(uint16 baseId) {
	uint16 groupId = getGroupFromBaseId(baseId);

	Feature *node = _rootNode->_next;
	while (node->_next) {
		Feature *curr = node;
		node = node->_next;
		if (curr->_data.compoundSHAPIndex == groupId) {
			removeFeature(curr, true);
		}
	}
}

void CSTimeView::freeShapesUsingResourceId(uint16 resourceId) {
	// TODO: Meh. This needs some rethinking.
	for (int i = _numSCRBGroups - 1; i >= 0; i--) {
		if (_SCRBGroupResources[i] != resourceId)
			continue;
		for (uint j = i; j < 13; j++)
			_compoundSHAPGroups[j] = _compoundSHAPGroups[j + 1];
		_compoundSHAPGroups[13] = 0;
		// TODO: deal with REGS
	}
}

void CSTimeView::freeScriptsUsingResourceId(uint16 resourceId) {
	// TODO: Meh. This needs some rethinking.
	for (int i = _numSCRBGroups - 1; i >= 0; i--) {
		if (_SCRBGroupResources[i] == resourceId)
			groupFreeScript(i);
	}
}

void CSTimeView::groupFreeScript(uint index) {
	uint count = _SCRBGroupSizes[index];
	_numSCRBGroups--;
	for (uint i = index; i < _numSCRBGroups; i++) {
		_SCRBGroupBases[i] = _SCRBGroupBases[i + 1];
		_SCRBGroupSizes[i] = _SCRBGroupSizes[i + 1];
		_SCRBGroupResources[i] = _SCRBGroupResources[i + 1]; // TODO: Meh. This needs some rethinking.
	}
	uint base = 0;
	for (uint i = 0; i < index; i++)
		base += _SCRBGroupSizes[i];
	for (uint i = 0; i < count; i++)
		_SCRBEntries.remove_at(base);
	// TODO: kill any actual scripts
	groupAdjustView(index, count);
}

void CSTimeView::groupAdjustView(uint index, uint count) {
	for (Feature *node = _rootNode->_next; node->_next; node = node->_next) {
		if (node->_data.compoundSHAPIndex < index)
			continue;
		node->_data.compoundSHAPIndex--;
		node->_data.scrbIndex -= count;
	}
}

void CSTimeView::loadBitmapCursors(uint16 baseId) {
	// TODO
}

void CSTimeView::setBitmapCursor(uint16 id) {
	if (_bitmapCursorId == id)
		return;

	if (!id) {
		_vm->_cursor->showCursor();
	} else {
		_vm->_cursor->hideCursor();
	}

	_bitmapCursorId = id;
}

void CSTimeView::dragFeature(NewFeature *feature, Common::Point pos, uint mode, uint32 flags, Common::Rect *rect) {
	feature->_data.hidden = 0;

	if (mode == 2) {
		if (feature->_dragFlags & 0x800000) {
			feature->_dragFlags = flags | 0x8000;
			if (!(flags & 1))
				(_currentModule->*(feature->_dropProc))(feature);
		}
		return;
	}

	if (feature->_dragFlags & 0x800000)
		(_currentModule->*(feature->_dropProc))(feature);
	else
		(_currentModule->*(feature->_pickupProc))(feature, pos, flags, rect);
}

void CSTimeView::finishDraw() {
	// TODO: This is a kinda stupid hack, here just for debugging.
	((MohawkEngine_CSTime *)_vm)->getCase()->getCurrScene()->drawHotspots();
}

CSTimeModule::CSTimeModule(MohawkEngine_CSTime *vm) : _vm(vm) {
}

void CSTimeModule::defaultMoveProc(Feature *feature) {
	if (feature->_data.paused > 0)
		return;

	if (!feature->_data.enabled)
		return;

	if (feature->_timeProc && !(this->*(feature->_timeProc))(feature))
		return;

	if (feature->_needsReset) {
		feature->resetFeatureScript(1, feature->_scrbId);
		if ((feature->_flags & kFeatureNewDisable) || (feature->_flags & kFeatureNewDisableOnReset)) {
			feature->_data.enabled = 0;
		}
		feature->_dirty = 1;
		if (feature->_flags & kFeatureInternalRegion) {
			// TODO: create region [+140] (if not already done)
		}
	} else {
		if (!(feature->_flags & kFeatureNewClip)) {
			if (feature->_data.useClipRect) {
				// TODO: or clip with _unknown228
			} else if (feature->_region) {
				// TODO: or clip with region
			} else {
				// TODO: or clip with bounds
			}
		}
		feature->_dirty = 1;
		if (feature->_flags & kFeatureNewInternalTiming) {
			feature->_nextTime += feature->_delayTime;
		} else {
			feature->_nextTime = _vm->getView()->_lastIdleTime + feature->_delayTime;
		}
		if (feature->_done) {
			if (feature->_flags & kFeatureNewNoLoop) {
				// FIXME: sync channel reset
				uint16 unknown184 = 1, unknown186 = 1; // FIXME: XXX
				if (feature->_flags & kFeatureDisableOnEnd || (unknown184 != 0 && unknown186 != 0)) { // FIXME: XXX
					feature->_data.enabled = 0;
					if (feature->_doneProc) {
						(this->*(feature->_doneProc))(feature); // TODO: with -2
					}
				}
				return;
			}

			feature->_data.currOffset = 26;
			feature->_done = 0;
		}
		if (feature->_flags & kFeatureNewDisable)
			feature->_data.enabled = 0;
	}

	int xOffset = feature->_data.currentPos.x + feature->_data.nextPos.x;
	int yOffset = feature->_data.currentPos.y + feature->_data.nextPos.y;

	Common::SeekableReadStream *ourSCRB = _vm->getView()->getSCRB(feature->_data.scrbIndex);
	ourSCRB->seek(feature->_data.currOffset);

	bool setBitmap = false;
	uint bitmapId = 0;
	bool done = false;
	while (!done) {
		byte opcode = ourSCRB->readByte();
		byte size = ourSCRB->readByte();
		switch (opcode) {
		case 1:
			ourSCRB->skip(size - 2);
			opcode = ourSCRB->readByte();
			size = ourSCRB->readByte();
			if (opcode != 0) {
				ourSCRB->seek(-2, SEEK_CUR);
				done = true;
				break;
			}
		case 0:
			// TODO: set ptr +176 to 1
			feature->_done = 1;
			if (feature->_doneProc) {
				(this->*(feature->_doneProc))(feature); // TODO: with -1
			}
			done = true;
			break;

		case 3:
			{
			int32 pos = ourSCRB->pos();
			ourSCRB->seek(2);
			uint16 base = ourSCRB->readUint16BE();
			ourSCRB->seek(pos);
			base += ourSCRB->readUint16BE();
			if (base) {
				// FIXME: sound?
			}
			ourSCRB->skip(size - 4);
			}
			warning("saw feature opcode 0x3 (size %d)", size);
			break;

		case 4:
			// FIXME
			if (false /* TODO: !+72 */) {
				ourSCRB->skip(size - 2);
			} else {
				uint16 time = ourSCRB->readUint16BE();
				// FIXME: not right
				feature->_delayTime = time;
				ourSCRB->skip(size - 4);
			}
			warning("saw feature opcode 0x4 (size %d)", size);
			break;

		case 9:
			// FIXME
			ourSCRB->skip(size - 2);
			warning("ignoring feature opcode 0x9 (size %d)", size);
			break;

		case 0xf:
			// FIXME
			ourSCRB->skip(size - 2);
			warning("ignoring feature opcode 0xf (size %d)", size);
			break;

		case 0x10:
			while (bitmapId < 48) {
				if (!size)
					break;
				size--;
				feature->_data.bitmapIds[bitmapId] = ourSCRB->readUint16BE() & 0xFFF;
				feature->_data.bitmapPos[bitmapId].x = ourSCRB->readUint16BE() + xOffset;
				feature->_data.bitmapPos[bitmapId].y = ourSCRB->readUint16BE() + yOffset;
				bitmapId++;
			}
			feature->_data.bitmapIds[bitmapId] = 0;
			setBitmap = true;
			break;

		default:
			warning("unknown new feature opcode %d", opcode);
			ourSCRB->skip(size - 2);
			break;
		}
	}

	feature->_data.currOffset = ourSCRB->pos();
	if (!setBitmap) {
		// TODO: set fail flag
		return;
	}
	if (feature->_frameProc) {
		(this->*(feature->_frameProc))(feature);
	}
	// TODO: set palette if needed

	// TODO: adjust for regs if needed
	Common::Array<int16> regsX, regsY;
	Common::SeekableReadStream *regsStream;
	uint16 compoundSHAPIndex = _vm->getView()->getCompoundSHAPId(feature->_data.compoundSHAPIndex);
	regsStream = _vm->getResource(ID_REGS, compoundSHAPIndex);
	while (regsStream->pos() != regsStream->size())
		regsX.push_back(regsStream->readSint16BE());
	delete regsStream;
	regsStream = _vm->getResource(ID_REGS, compoundSHAPIndex + 1);
	while (regsStream->pos() != regsStream->size())
		regsY.push_back(regsStream->readSint16BE());
	delete regsStream;
	for (uint i = 0; i < 48; i++) {
		uint16 thisBitmapId = feature->_data.bitmapIds[i];
		if (!thisBitmapId)
			break;
		feature->_data.bitmapPos[i].x -= regsX[thisBitmapId];
		feature->_data.bitmapPos[i].y -= regsY[thisBitmapId];
	}

	// TODO: set bounds
	// TODO: unset fail flag
}

void CSTimeModule::defaultDrawProc(Feature *feature) {
	if (feature->_data.hidden > 0)
		return;

	feature->defaultDraw();
}

bool CSTimeModule::defaultTimeProc(Feature *feature) {
	return (feature->_nextTime <= _vm->getView()->getTime());
}

void CSTimeModule::defaultPickupProc(NewFeature *feature, Common::Point pos, uint32 flags, Common::Rect *rect) {
	_vm->getView()->removeFeature(feature, false);

	feature->_dragFlags |= flags | 0x800000;
	feature->_oldFlags = feature->_flags;
	feature->_data.useClipRect = 0;
	// TODO: these flags are weird/different
	feature->_flags = (feature->_flags & ~kFeatureSortBackground) | kFeatureOldSortForeground | kFeatureSortStatic | 0x2000;
	_vm->getView()->insertUnderCursor(feature);

	feature->_nextTime = 0;
	// FIXME: preserve old delayTime (see also script op 4)
	feature->_delayTime = 50;

	feature->_oldPos = feature->_data.currentPos;

	feature->_posDiff.x = pos.x - feature->_data.currentPos.x;
	feature->_posDiff.y = pos.y - feature->_data.currentPos.y;
	debug("defaultPickupProc: diff is %d, %d", feature->_posDiff.x, feature->_posDiff.y);

	feature->_oldMoveProc = feature->_moveProc;
	feature->_moveProc = feature->_dragMoveProc;

	// FIXME: deal with rect
	if (rect)
		error("defaultPickupProc doesn't handle rect yet");
}

void CSTimeModule::defaultDropProc(NewFeature *feature) {
	// FIXME: invalidation

	feature->_flags = feature->_oldFlags;
	// FIXME: restore old delayTime
	feature->_dragFlags &= ~0x800000;

	if (feature->_dragFlags & 0x800)
		feature->moveAndUpdate(feature->_oldPos);
	if (feature->_dragFlags & 0x200)
		feature->hide(true);
	feature->_moveProc = feature->_oldMoveProc;
}

void CSTimeModule::defaultDragMoveProc(NewFeature *feature) {
	// FIXME

	if (feature->_dragFlags & 0x8000)
		feature->_currDragPos = _vm->getEventManager()->getMousePos();

	Common::Point pos = feature->_currDragPos;
	pos.x -= feature->_posDiff.x;
	pos.y -= feature->_posDiff.y;

	if (feature->_dragFlags & 0x80) {
		// FIXME: handle 0x80 case
		error("encountered 0x80 case in defaultDragMoveProc");
	}

	feature->moveAndUpdate(pos);

	(this->*(feature->_oldMoveProc))(feature);
}

void CSTimeModule::cursorMoveProc(Feature *feature) {
	uint16 cursor = _vm->getView()->getBitmapCursor();
	if (!cursor)
		return;

	Common::Point pos = _vm->getEventManager()->getMousePos();

	// FIXME: shouldn't be hardcoded
	uint16 compoundSHAPIndex = 200;
	// FIXME: stupid REGS stuff..
	Common::SeekableReadStream *regsStream = _vm->getResource(ID_REGS, compoundSHAPIndex);
	regsStream->seek(cursor * 2);
	feature->_data.bounds.left = pos.x - regsStream->readSint16BE();
	delete regsStream;
	regsStream = _vm->getResource(ID_REGS, compoundSHAPIndex + 1);
	regsStream->seek(cursor * 2);
	feature->_data.bounds.top = pos.y - regsStream->readSint16BE();
	delete regsStream;
}

void CSTimeModule::cursorDrawProc(Feature *feature) {
	uint16 cursor = _vm->getView()->getBitmapCursor();
	if (!cursor)
		return;
	// FIXME: shouldn't be hardcoded
	uint16 compoundSHAPIndex = 200;
	_vm->getView()->getGfx()->copyAnimSubImageToScreen(compoundSHAPIndex, cursor - 1, feature->_data.bounds.left, feature->_data.bounds.top);
}

void CSTimeModule::rolloverTextMoveProc(Feature *feature) {
	// Should OR the whole bounds into the dirty region, if the text changed.
}

void CSTimeModule::rolloverTextDrawProc(Feature *feature) {
	// TODO: if timeBook->getState() is 2, return
	const Common::String &text = _vm->getInterface()->getRolloverText();
	if (!text.empty()) {
		Common::Rect &rect = feature->_data.bounds;
		Graphics::Surface *screen = g_system->lockScreen();
		_vm->getInterface()->getRolloverFont().drawString(screen, text, rect.left, rect.top, rect.width(), 32, Graphics::kTextAlignCenter);
		g_system->unlockScreen();
	}
	// TODO: some special case about dragging in case 1, scene 4 (torch?)
	// TODO: unset text changed flag
}

void CSTimeModule::dialogTextMoveProc(Feature *feature) {
	// FIXME
}

void CSTimeModule::dialogTextDrawProc(Feature *feature) {
	const Common::Array<Common::String> &lines = _vm->getInterface()->getDialogLines();
	const Common::Array<byte> &colors = _vm->getInterface()->getDialogLineColors();
	const Common::Rect &bounds = feature->_data.bounds;
	const Graphics::Font &font = _vm->getInterface()->getDialogFont();

	Graphics::Surface *screen = _vm->_system->lockScreen();
	for (uint i = 0; i < lines.size(); i++)
		font.drawString(screen, lines[i], bounds.left, bounds.top + 1 + i*15, bounds.width(), colors[i], Graphics::kTextAlignCenter);
	_vm->_system->unlockScreen();
	// FIXME
}

void CSTimeModule::bubbleTextMoveProc(Feature *feature) {
	// FIXME
}

void CSTimeModule::bubbleTextDrawProc(Feature *feature) {
	Common::Rect bounds = feature->_data.bounds;
	bounds.grow(-5);
	const Graphics::Font &font = _vm->getInterface()->getDialogFont();
	uint height = font.getFontHeight();

	Common::Array<Common::String> lines;
	font.wordWrapText(_vm->getInterface()->getCurrBubbleText(), bounds.width(), lines);

	Graphics::Surface *screen = _vm->_system->lockScreen();
	for (int x = -2; x < 2; x++)
		for (int y = -1; y < 3; y++)
			for (uint i = 0; i < lines.size(); i++)
				font.drawString(screen, lines[i], bounds.left + x, bounds.top + y + i*height, bounds.width(), 241, Graphics::kTextAlignCenter);
	for (uint i = 0; i < lines.size(); i++)
		font.drawString(screen, lines[i], bounds.left, bounds.top + i*height, bounds.width(), 32, Graphics::kTextAlignCenter);
	_vm->_system->unlockScreen();
}

} // End of namespace Mohawk
