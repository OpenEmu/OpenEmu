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

#include "mohawk/view.h"
#include "mohawk/resource.h"
#include "mohawk/graphics.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/palette.h"

namespace Mohawk {

Module::Module() {
}

Module::~Module() {
}

Feature::Feature(View *view) : _view(view) {
}

Feature::~Feature() {
}

void Feature::setNodeDefaults(Feature *prev, Feature *next) {
	_prev = prev;
	_next = next;

	_moveProc = NULL;
	_drawProc = NULL;
	_doneProc = NULL;
	_frameProc = NULL;

	_data.bounds = Common::Rect();
	_data.clipRect = Common::Rect();
	_data.useClipRect = 0;

	_region = 0;
	_id = 0; // This is dealt with elsewhere.
	_scrbId = 0;
	_storedScrbId = 0;

	_data.scrbIndex = 0;
	_data.compoundSHAPIndex = 0;
	_data.bitmapIds[0] = 0;

	_data.unknown192 = 0;
	_data.currFrame = 0;

	_data.syncChannel = 0;
	_data.enabled = 1;

	_data.paused = 0; // new
	_data.hidden = 0; // new

	_flags = 0;

	_dirty = 1;
	_needsReset = 1;
	_justReset = 0; // old
	_notifyDone = 0;
	_done = 0; // new

	_nextTime = 0;
	_delayTime = 0;
}

void Feature::resetFeatureScript(uint16 enabled, uint16 scrbId) {
	if (!scrbId)
		scrbId = _scrbId;
	if (scrbId != _scrbId || _needsReset) {
		if (_needsReset)
			_data.bounds = Common::Rect();
		_scrbId = scrbId;
		_view->getnthScriptSetGroup(_data.scrbIndex, _data.compoundSHAPIndex, scrbId);
	}
	if (_data.scrbIndex == 0xFFFF) {
		_data.enabled = 0;
		_data.bitmapIds[0] = 0;
		_data.scrbIndex = 0;
		_data.compoundSHAPIndex = 0;
		resetFrame();
		return;
	}

	resetScript();
	resetFrame();
	_nextTime = 0; // New feature code uses _view->_lastIdleTime, but should be equivalent.
	_data.enabled = enabled;
	_dirty = 1;

	finishResetFeatureScript();

	_needsReset = 0;

	if (_region) {
		// TODO: mark _region as dirty
	} else {
		// TODO: mark _data.bounds as dirty
	}
}

void Feature::resetFeature(bool notifyDone, Module::FeatureProc doneProc, uint16 scrbId) {
	resetFeatureScript(1, scrbId);
	_doneProc = doneProc;
	_notifyDone = notifyDone;
}

void Feature::hide(bool clip) {
	// FIXME: stuff

	if (!_data.hidden && clip) {
		if (_region) {
			// TODO: mark _region as dirty
		} else {
			// TODO: mark _data.bounds as dirty
		}
	}

	_data.hidden++;
	_data.paused++;
}

void Feature::show() {
	if (_data.hidden == 1) {
		if (_region) {
			// TODO: mark _region as dirty
		} else {
			// TODO: mark _data.bounds as dirty
		}
	}

	_data.hidden--;
	_data.paused--;
}

void Feature::moveAndUpdate(Common::Point newPos) {
	if (newPos == _data.currentPos)
		return;

	_nextTime = 0;
	_dirty = 1;
	// TODO: mark _data.bounds as dirty

	if (_data.bitmapIds[0])
		_data.bounds.moveTo(newPos);

	int xDiff = _data.currentPos.x - newPos.x;
	int yDiff = _data.currentPos.y - newPos.y;

	for (uint i = 0; i < FEATURE_BITMAP_ITEMS; i++) {
		uint16 bitmapId = _data.bitmapIds[i];
		if (!bitmapId) // || bitmapId > compoundSHAP.size()
			break;
		_data.bitmapPos[i].x -= xDiff;
		_data.bitmapPos[i].y -= yDiff;
	}

	_data.currentPos = newPos;
}

void Feature::defaultDraw() {
	if (_data.useClipRect) {
		// TODO: set clip rect
	}
	uint16 compoundSHAPId = _view->getCompoundSHAPId(_data.compoundSHAPIndex);
	for (uint i = 0; i < FEATURE_BITMAP_ITEMS; i++) {
		uint16 bitmapId = _data.bitmapIds[i];
		if (!bitmapId) // || bitmapId > compoundSHAP.size()
			break;
		_view->getGfx()->copyAnimSubImageToScreen(compoundSHAPId, bitmapId - 1, _data.bitmapPos[i].x, _data.bitmapPos[i].y);
	}
	if (_data.useClipRect) {
		// TODO: restore clip rgn
	}
}

OldFeature::OldFeature(View *view) : Feature(view) {
}

OldFeature::~OldFeature() {
}

void OldFeature::resetFrame() {
	_data.currFrame = 0;
	_data.currOffset = 1;
}

void OldFeature::resetFeatureScript(uint16 enabled, uint16 scrbId) {
	if ((_flags & kFeatureOldAlternateScripts) && (_justReset || !_needsReset)) {
		if (_storedScrbId)
			return;
		if (_flags & kFeatureOldRandom) {
			_storedScrbId = -(int16)_scrbId;
			_flags &= ~kFeatureOldRandom;
		} else {
			_storedScrbId = _scrbId;
		}
	}

	Feature::resetFeatureScript(enabled, scrbId);
}

void OldFeature::resetScript() {
	Common::SeekableReadStream *ourSCRB = _view->getSCRB(_data.scrbIndex, _scrbId);
	_data.endFrame = ourSCRB->readUint16BE() - 1;
	delete ourSCRB;
}

void OldFeature::finishResetFeatureScript() {
	_justReset = 1;

	if (_flags & kFeatureOldAdjustByPos) {
		Common::SeekableReadStream *ourSCRB = _view->getSCRB(_data.scrbIndex, _scrbId);
		ourSCRB->seek(4);
		_data.nextPos.x = ourSCRB->readUint16BE();
		_data.nextPos.y = ourSCRB->readUint16BE();
		delete ourSCRB;
	}
}

NewFeature::NewFeature(View *view) : Feature(view) {
}

NewFeature::~NewFeature() {
}

void NewFeature::resetFrame() {
	_data.currOffset = 26;
}

void NewFeature::resetFeatureScript(uint16 enabled, uint16 scrbId) {
	// TODO: _frameProc(this, -3);
	// TODO: set unknown184 to 0x01010101

	Feature::resetFeatureScript(enabled, scrbId);
}

void NewFeature::resetScript() {
	// FIXME: registrations, etc
	Common::SeekableReadStream *ourSCRB = _view->getSCRB(_data.scrbIndex, _scrbId);
	ourSCRB->seek(16);
	Common::Point scriptBase, scriptSize;
	scriptBase.x = ourSCRB->readUint16BE();
	scriptBase.y = ourSCRB->readUint16BE();
	scriptSize.x = ourSCRB->readUint16BE();
	scriptSize.y = ourSCRB->readUint16BE();
	ourSCRB->seek(26);
	Common::Point one, two;
	while (true) {
		if (ourSCRB->pos() == ourSCRB->size())
			error("resetScript (getNewXYAndReg) ran out of script");
		byte opcode = ourSCRB->readByte();
		byte size = ourSCRB->readByte();
		if (opcode != 0x10) {
			ourSCRB->skip(size - 2);
		} else if (size) {
			assert(size >= 1);
			ourSCRB->skip(2);
			int16 x = ourSCRB->readUint16BE();
			int16 y = ourSCRB->readUint16BE();
			one.x = -x;
			one.y = -y;
			two.x = scriptBase.x + x;
			two.y = scriptBase.y + y;
			break;
		}
	}
	delete ourSCRB;

	if ((_needsReset || false /* TODO: param */) && (_unknown168 == 0x7FFFFFFF || false /* TODO: param */)) {
		_data.currentPos = two;
		_data.nextPos = one;
		_unknown168 = 0;
		if (_needsReset || false /* TODO: param */) {
			_data.bounds = Common::Rect(scriptBase.x, scriptBase.y, scriptSize.x, scriptSize.y);
		}
	} else {
		if (false /* FIXME: 0 shapes? */) {
			_data.nextPos.x = one.x + two.x - _data.currentPos.x;
			_data.nextPos.y = one.y + two.y - _data.currentPos.y;
		} else if (_unknown168 != 0x7FFFFFFF) {
			_data.nextPos = one;
		}
	}
	// _needsReset = 0; (handled by caller)
}

void NewFeature::finishResetFeatureScript() {
	_done = 0;
}

View::View(MohawkEngine *vm) : _vm(vm) {
	_currentModule = NULL;

	_backgroundId = 0xffff;

	for (uint i = 0; i < 14; i++) { // used to be 8
		_compoundSHAPGroups[i] = 0;
	}
	_numSCRBGroups = 0;
}

View::~View() {
}

void View::idleView() {
	assert(_currentModule);

	_lastIdleTime = getTime();

	for (Feature *node = _rootNode; node; node = node->_next) {
		if (node->_moveProc)
			(_currentModule->*(node->_moveProc))(node);
	}

	// TODO: find a way this works for all clients
	//if (/* TODO: _sortView */ true && !_inDialog) {
	//	sortView();
	//}
	sortView();

	for (Feature *node = _rootNode; node; node = node->_next) {
		if (node->_dirty) {
			// TODO: clipping
			_needsUpdate = true;
		}
		if (node->_drawProc)
			(_currentModule->*(node->_drawProc))(node);
		node->_dirty = 0;
	}

	if (_needsUpdate) {
		finishDraw();
		_vm->_system->updateScreen();
		_needsUpdate = false;
		if (_backgroundId != 0xffff)
			_gfx->copyAnimImageToScreen(_backgroundId);
	}
}

void View::setModule(Module *module) {
	if (_currentModule) {
		module->shutdown();
		delete module;
	}

	_currentModule = NULL;

	if (module) {
		_currentModule = module;
		module->init();
	}
}

Common::Array<uint16> View::getSHPL(uint16 id) {
	Common::SeekableReadStream *stream;

	if (_vm->hasResource(ID_TCNT, id)) {
		stream = _vm->getResource(ID_TCNT, id);
	} else {
		stream = _vm->getResource(ID_SHPL, id);
		stream->seek(4);
		setColors(stream);
		stream->seek(0);
	}

	uint16 base = stream->readUint16BE();
	uint16 count = stream->readUint16BE();
	delete stream;

	Common::Array<uint16> items;
	for (uint i = 0; i < count; i++)
		items.push_back(base + i);

	return items;
}

void View::installBG(uint16 id) {
	// getShapes
	Common::Array<uint16> shapes = getSHPL(id);
	if (_vm->hasResource(ID_TPAL, id)) {
		Common::SeekableReadStream *stream = _vm->getResource(ID_TPAL, id);
		setColors(stream);
		delete stream;
	}

	if (shapes.size() != 1) {
		// TODO
		warning("background with id 0x%04x has the wrong number of shapes (%d)", id, shapes.size());
		_backgroundId = id;
		_gfx->copyAnimImageToScreen(_backgroundId);
	} else {
		// DrawViewBackground
		_backgroundId = shapes[0];
		_gfx->copyAnimImageToScreen(_backgroundId);
	}
}

void View::setColors(Common::SeekableReadStream *tpalStream) {
	uint16 colorStart = tpalStream->readUint16BE();
	uint16 colorCount = tpalStream->readUint16BE();
	byte *palette = new byte[colorCount * 3];

	for (uint16 i = 0; i < colorCount; i++) {
		palette[i * 3 + 0] = tpalStream->readByte();
		palette[i * 3 + 1] = tpalStream->readByte();
		palette[i * 3 + 2] = tpalStream->readByte();
		tpalStream->readByte();
	}

	// TODO: copy into temporary buffer
	_vm->_system->getPaletteManager()->setPalette(palette, colorStart, colorCount);
	delete[] palette;

	// original does pdLightenUp here..
}

void View::copyFadeColors(uint start, uint count) {
	// TODO
}

uint16 View::getCompoundSHAPId(uint16 shapIndex) {
	return _compoundSHAPGroups[shapIndex];
}

void View::installGroupOfSCRBs(bool main, uint base, uint size, uint count) {
	if (main) {
		// TODO: _dropSpots.clear();
		_numSCRBGroups = 0;
		_SCRBEntries.clear();
	}

	if (_numSCRBGroups >= 14) // used to be 8
		error("installGroupOfSCRBs called when we already had 14 groups");

	for (uint i = 0; i < size; i++)
		_SCRBEntries.push_back(base + i);

	// TODO: think about this
	if (count == 0)
		count = size;
	else if (count > size) {
		for (uint i = 0; i < count - size; i++)
			_SCRBEntries.push_back(0);
	} else
		error("installGroupOfSCRBs got count %d, size %d", count, size);

	_SCRBGroupBases[_numSCRBGroups] = base;
	_SCRBGroupSizes[_numSCRBGroups] = count;
	_numSCRBGroups++;
}

void View::freeScripts() {
	freeFeatureShapes();

	for (uint i = 0; i < 14; i++) { // used to be 8
		_SCRBGroupBases[i] = 0;
		_SCRBGroupSizes[i] = 0;
	}
	_SCRBEntries.clear();
	_numSCRBGroups = 0;
}

void View::installFeatureShapes(bool regs, uint groupId, uint16 resourceBase) {
	if (groupId >= 14) // used to be 8
		error("installFeatureShapes called for invalid group %d", groupId);

	if (_compoundSHAPGroups[groupId])
		error("installFeatureShapes called for existing group %d", groupId);

	_compoundSHAPGroups[groupId] = resourceBase;

	if (regs) {
		// TODO
	}
}

void View::freeFeatureShapes() {
	for (uint i = 0; i < 14; i++) { // used to be 8
		_compoundSHAPGroups[i] = 0;
		// TODO: wipe regs data
	}
}

uint16 View::getGroupFromBaseId(uint16 baseId) {
	for (uint i = 0; i < 14; i++) {
		if (_compoundSHAPGroups[i] == baseId)
			return i;
	}

	// TODO: error?
	return 0xffff;
}

void View::getnthScriptSetGroup(uint16 &scrbIndex, uint16 &shapIndex, uint16 scrbId) {
	scrbIndex = 0;
	for (uint i = 0; i < _numSCRBGroups; i++) {
		if (_SCRBGroupBases[i] <= scrbId && _SCRBGroupBases[i] + _SCRBGroupSizes[i] > scrbId) {
			shapIndex = i;
			scrbIndex += scrbId - _SCRBGroupBases[i];
			return;
		}
		scrbIndex += _SCRBGroupSizes[i];
	}
	scrbIndex = 0xffff;
}

Common::SeekableReadStream *View::getSCRB(uint16 index, uint16 id) {
	// If we don't have an entry, load the load provided id.
	// (The 0xffff check is a default parameter hack.)
	if (!_SCRBEntries[index] && id != 0xffff)
		_SCRBEntries[index] = id;

	// FIXME
	if (_vm->hasResource(ID_SCRB, _SCRBEntries[index]))
		return _vm->getResource(ID_SCRB, _SCRBEntries[index]);
	return _vm->getResource(ID_TSCR, _SCRBEntries[index]);
}

Feature *View::getFeaturePtr(uint16 id) {
	for (Feature *node = _cursorNode; node; node = node->_prev) {
		if (node->_id == id)
			return node;
	}

	return NULL;
}

uint16 View::getNewFeatureId() {
	uint16 nextId = 0;
	Feature *node;
	for (node = _rootNode; node; node = node->_next) {
		// The original doesn't check for 0xffff but I don't want to fudge with signed integers.
		if (node->_id != 0xffff && node->_id > nextId)
			nextId = node->_id;
	}
	return nextId + 1;
}

void View::removeFeature(Feature *feature, bool free) {
	// TODO: or bounds into dirty feature bounds

	feature->_prev->_next = feature->_next;
	feature->_next->_prev = feature->_prev;
	feature->_next = NULL;
	feature->_prev = NULL;

	if (free)
		delete feature;
}

void View::insertUnderCursor(Feature *feature) {
	feature->_next = _cursorNode;
	feature->_prev = _cursorNode->_prev;
	feature->_prev->_next = feature;
	feature->_next->_prev = feature;
}

Feature *View::pointOnFeature(bool topdown, uint32 flags, Common::Point pos) {
	flags &= 0x7fffff;
	Feature *curr = _rootNode->_next;
	if (topdown)
		curr = _cursorNode->_prev;
	while (curr) {
		if ((curr->_flags & 0x7fffff) == flags)
			if (curr->_data.bounds.contains(pos))
				return curr;
		if (topdown)
			curr = curr->_prev;
		else
			curr = curr->_next;
	}
	return NULL;
}

void View::sortView() {
	Feature *base = _rootNode;
	Feature *next = base->_next;
	Feature *otherRoot = NULL;
	Feature *otherBase = NULL;
	Feature *objectRoot = NULL;
	Feature *objectBase = NULL;
	Feature *staticRoot = NULL;
	Feature *staticBase = NULL;

	// Remove all features.
	base->_next = NULL;

	// Iterate through all the previous features, placing them in the appropriate list.
	while (next) {
		Feature *curr = next;
		next = next->_next;

		if (curr->_flags & kFeatureSortBackground) {
			// These are behind everything else (e.g. stars, drop spot highlights),
			// so we insert this node directly after the current base.
			base->_next = curr;
			curr->_prev = base;
			curr->_next = NULL;
			base = base->_next;
		} else if (curr->_flags & kFeatureSortStatic) {
			// Insert this node into the list of static objects.
			if (staticBase) {
				staticBase->_next = curr;
				curr->_prev = staticBase;
				curr->_next = NULL;
				staticBase = curr;
			} else {
				staticBase = curr;
				staticRoot = curr;
				curr->_prev = NULL;
				curr->_next = NULL;
			}
		} else if (curr->_flags & kFeatureObjectMask) { // This is == 1 or == 2 in old code.
			// Insert this node into the list of objects.
			if (objectRoot) {
				objectBase->_next = curr;
				curr->_prev = objectBase;
				curr->_next = NULL;
				objectBase = curr;
			} else {
				objectBase = curr;
				objectRoot = curr;
				curr->_prev = NULL;
				curr->_next = NULL;
			}
		} else {
			if (!(curr->_flags & kFeatureOldSortForeground))
				curr->_flags |= kFeatureSortStatic;

			// Insert this node into the list of other features.
			if (otherRoot) {
				otherBase->_next = curr;
				curr->_prev = otherBase;
				curr->_next = NULL;
				otherBase = curr;
			} else {
				otherBase = curr;
				otherRoot = curr;
				curr->_prev = NULL;
				curr->_next = NULL;
			}
		}
	}

	// Add the static features after the background ones.
	Feature *curr = staticRoot;
	while (curr) {
		Feature *prev = curr;
		curr = curr->_next;
		base->_next = prev;
		prev->_prev = base;
		base = base->_next;
		base->_next = NULL;
	}

	// Add the other features on top..
	_rootNode = mergeLists(_rootNode, sortOneList(otherRoot));
	// Then finally, add the objects.
	_rootNode = mergeLists(_rootNode, sortOneList(objectRoot));
}

Feature *View::sortOneList(Feature *root) {
	if (!root)
		return NULL;

	// Save the next feature and then clear the list.
	Feature *curr = root->_next;
	root->_next = NULL;
	root->_prev = NULL;

	// Iterate over all the features.
	while (curr) {
		Feature *prev = curr;
		curr = curr->_next;
		Common::Rect &prevRect = prev->_data.bounds;

		// Check against all features currently in the list.
		Feature *check = root;
		while (check) {
			Common::Rect &checkRect = check->_data.bounds;

			if ((prev->_flags & kFeatureOldSortForeground) || (prevRect.bottom >= checkRect.bottom && (prevRect.bottom != checkRect.bottom || prevRect.left >= checkRect.left))) {
				// If we're meant to be in front of everything else, or we're in front of the check object..
				if (!check->_next) {
					// This is the end of the list: add ourselves there.
					check->_next = prev;
					prev->_prev = check;
					prev->_next = NULL;
					break;
				}
			} else {
				// We're meant to be behind this object. Insert ourselves here.
				prev->_prev = check->_prev;
				prev->_next = check;
				check->_prev = prev;
				if (prev->_prev)
					prev->_prev->_next = prev;
				else
					root = prev;
				break;
			}

			check = check->_next;
		}
	}

	return root;
}

Feature *View::mergeLists(Feature *root, Feature *mergeRoot) {
	Feature *base = root;
	// Skip anything marked as being behind everything else.
	while (base->_next && (base->_next->_flags & kFeatureSortBackground))
		base = base->_next;

	// Iterate over all the objects in the root to be merged.
	Feature *curr = mergeRoot;
	while (curr) {
		Feature *prev = curr;
		curr = curr->_next;
		Common::Rect &prevRect = prev->_data.bounds;

		// Check against all objects currently in the list.
		Feature *check = base;
		if (prev->_flags & kFeatureOldSortForeground) {
			// This object is meant to be in front of everything else,
			// put it at the end of the list.
			while (check && check->_next)
				check = check->_next;
			check->_next = prev;
			prev->_prev = check;
			prev->_next = NULL;
			continue;
		}

		while (check) {
			if (check->_flags & kFeatureOldSortForeground) {
				// The other object is meant to be in front of everything else,
				// put ourselves before it.
				prev->_prev = check->_prev;
				prev->_next = check;
				check->_prev = prev;
				// The original doesn't bother with this 'if'.
				if (prev->_prev)
					prev->_prev->_next = prev;
				else
					root = prev;
				break;
			}

			if (!check->_next) {
				// We're at the end of the list, so we have to go here.
				check->_next = prev;
				prev->_prev = check;
				prev->_next = NULL;
				base = prev;
				break;
			}

			Common::Rect &checkRect = check->_data.bounds;

			if (prevRect.bottom < checkRect.bottom || (prevRect.bottom == checkRect.bottom && prevRect.left < checkRect.left)) {
				if (prevRect.bottom < checkRect.top || (
					(!(check->_flags & kFeatureSortCheckLeft) || prevRect.left >= checkRect.left) &&
					(!(check->_flags & kFeatureSortCheckTop) || prevRect.top >= checkRect.top) &&
					(!(check->_flags & kFeatureSortCheckRight) || prevRect.right <= checkRect.right))) {
					// Insert ourselves before this one.
					prev->_prev = check->_prev;
					prev->_next = check;
					check->_prev = prev;
					if (prev->_prev)
						prev->_prev->_next = prev;
					else
						root = prev;
					base = prev->_next;
					break;
				}
			}

			check = check->_next;
		}
	}

	return root;
}

} // End of namespace Mohawk
