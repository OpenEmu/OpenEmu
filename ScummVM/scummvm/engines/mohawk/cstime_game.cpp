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
#include "mohawk/sound.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

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

// read a rect from a stream
Common::Rect readRect(Common::SeekableReadStream *stream) {
	Common::Rect rect;

	rect.left = stream->readSint16BE();
	rect.top = stream->readSint16BE();
	rect.right = stream->readSint16BE();
	rect.bottom = stream->readSint16BE();

	return rect;
}

void Region::loadFrom(Common::SeekableReadStream *stream) {
	uint16 rectCount = stream->readUint16BE();
	if (!rectCount) {
		// TODO: why this?
		stream->skip(2);
		rectCount = stream->readUint16BE();
	}
	for (uint i = 0; i < rectCount; i++) {
		Common::Rect rect = readRect(stream);
		if (!rect.isValidRect())
			continue;
		_rects.push_back(rect);
	}
}

bool Region::containsPoint(Common::Point &pos) const {
	for (uint i = 0; i < _rects.size(); i++)
		if (_rects[i].contains(pos))
			return true;

	return false;
}

CSTimeChar::CSTimeChar(MohawkEngine_CSTime *vm, CSTimeScene *scene, uint id) : _vm(vm), _scene(scene), _id(id) {
	_resting = true;
	_flappingState = 0xffff;
	_surfingState = 0;

	_NIS = NULL;
	_restFeature = NULL;
	_talkFeature = NULL;

	_talkFeature1 = NULL;
	_talkFeature2 = NULL;
	_talkFeature3 = NULL;
	_lastTime1 = 0;
	_lastTime2 = 0;
	_lastTime3 = 0;

	_playingWaveId = 0;
}

CSTimeChar::~CSTimeChar() {
}

void CSTimeChar::idle() {
	if (!_unknown2)
		return;

	if (_flappingState == 1) {
		idleTalk();
		return;
	}

	if (!_NIS)
		idleAmbients();
}

void CSTimeChar::setupAmbientAnims(bool onetime) {
	CSTimeConversation *conv = _vm->getCase()->getCurrConversation();
	if (_unknown1 == 0xffff || !_unknown2 || !_ambients.size() || !_resting || !_enabled ||
		(conv->getState() != (uint)~0 && conv->getSourceChar() == _id)) {
		setupRestPos();
		_resting = true;
		return;
	}

	removeChr();
	for (uint i = 0; i < _ambients.size(); i++) {
		// FIXME: check ambient condition
		uint32 flags = kFeatureSortStatic;
		if (_ambients[i].delay != 0xffff) {
			flags |= kFeatureNewNoLoop;
			if (onetime)
				flags |= kFeatureNewDisableOnReset;
		}
		installAmbientAnim(i, flags);
	}
}

void CSTimeChar::idleAmbients() {
	if (_flappingState != 0xffff)
		return;

	for (uint i = 0; i < _ambients.size(); i++) {
		if (!_ambients[i].feature)
			continue;
		uint16 delay = _ambients[i].delay;
		if (delay == 0xffff)
			continue;
		uint32 now = _vm->_system->getMillis();
		if (now < _ambients[i].nextTime)
			continue;
		_ambients[i].feature->resetFeatureScript(1, 0);
		_ambients[i].nextTime = now + delay;
	}
}

void CSTimeChar::stopAmbients(bool restpos) {
	for (uint i = 0; i < _ambients.size(); i++) {
		if (!_ambients[i].feature)
			continue;
		_vm->getView()->removeFeature(_ambients[i].feature, true);
		_ambients[i].feature = NULL;
	}

	if (restpos)
		setupRestPos();
}

void CSTimeChar::setupRestPos() {
	if (_unknown1 == 0xffff || !_unknown1 || !_unknown2)
		return;

	if (!_restFeature) {
		uint id = _enabled ? 0 : 13;
		uint32 flags = kFeatureSortStatic | kFeatureNewNoLoop | kFeatureNewDisableOnReset;
		Feature *feature = _vm->getView()->installViewFeature(getChrBaseId() + id, flags, NULL);
		// FIXME: fix priorities
		_restFeature = feature;
	} else {
		_restFeature->resetFeatureScript(1, 0);
	}

	// FIXME: fix more priorities
}

void CSTimeChar::removeChr() {
	if (_unknown1 == 0xffff || !_unknown1)
		return;

	if (_talkFeature) {
		_vm->getView()->removeFeature(_talkFeature, true);
		_vm->getView()->removeFeature(_talkFeature3, true);
		if (_talkFeature1)
			_vm->getView()->removeFeature(_talkFeature1, true);
		if (_talkFeature2) // original checks unknown1 > 1, but this is silly when e.g. _enabled is false
			_vm->getView()->removeFeature(_talkFeature2, true);
	}

	if (_restFeature)
		_vm->getView()->removeFeature(_restFeature, true);

	_talkFeature1 = NULL;
	_talkFeature2 = NULL;
	_talkFeature3 = NULL;

	_talkFeature = NULL;
	_restFeature = NULL;
}

uint16 CSTimeChar::getChrBaseId() {
	return _scene->getSceneId() + (_id + 1) * 200;
}

uint CSTimeChar::getScriptCount() {
	static uint bases[4] = { 0, 10, 13, 21 };
	assert(_unknown1 < 4);
	return bases[_unknown1] + _ambients.size() + _unknown3;
}

void CSTimeChar::playNIS(uint16 id) {
	if (_NIS)
		removeNIS();
	stopAmbients(false);
	removeChr();
	uint32 flags = kFeatureSortStatic | kFeatureNewNoLoop;
	_NIS = _vm->getView()->installViewFeature(getChrTypeScriptBase() + id + _ambients.size(), flags, NULL);
	// FIXME: fix priorities
}

bool CSTimeChar::NISIsDone() {
	return (_NIS->_data.paused || !_NIS->_data.enabled);
}

void CSTimeChar::removeNIS() {
	if (!_NIS)
		return;
	_vm->getView()->removeFeature(_NIS, true);
	_NIS = NULL;
}

void CSTimeChar::startFlapping(uint16 id) {
	if (!_unknown2)
		return;

	_scene->_activeChar = this;
	if (_restFeature) {
		_vm->getView()->removeFeature(_restFeature, true);
		_restFeature = NULL;
	}
	stopAmbients(false);
	setupTalk();
	_flappingState = 1;
	playFlapWave(id);
}

void CSTimeChar::interruptFlapping() {
	if (_playingWaveId)
		_vm->_sound->stopSound(_playingWaveId);
	// TODO: kill any other (preload) sound
	_waveStatus = 'q';
}

void CSTimeChar::installAmbientAnim(uint id, uint32 flags) {
	Feature *feature = _vm->getView()->installViewFeature(getChrTypeScriptBase() + id, flags, NULL);
	// FIXME: fix priorities

	_ambients[id].feature = feature;
	_ambients[id].nextTime = _vm->_system->getMillis() + _ambients[id].delay;
}

uint16 CSTimeChar::getChrTypeScriptBase() {
	static uint bases[4] = { 0, 10, 13, 21 };
	assert(_unknown1 < 4);
	return bases[_unknown1] + getChrBaseId();
}

void CSTimeChar::playFlapWave(uint16 id) {
	_playingWaveId = id;
	_vm->_sound->playSound(id, Audio::Mixer::kMaxChannelVolume, false, &_cueList);
	_nextCue = 0;
	_waveStatus = 'b';
}

void CSTimeChar::updateWaveStatus() {
	// This is a callback in the original, handling audio events.
	assert(_playingWaveId);

	// FIXME: state 's' for .. something? stopped?
	if (!_vm->_sound->isPlaying(_playingWaveId)) {
		_waveStatus = 'q';
		return;
	}

	uint samples = _vm->_sound->getNumSamplesPlayed(_playingWaveId);
	for (uint i = _nextCue; i < _cueList.pointCount; i++) {
		if (_cueList.points[i].sampleFrame > samples)
			return;
		if (_cueList.points[i].name.empty())
			warning("cue %d reached but was empty", i);
		else
			_waveStatus = _cueList.points[i].name[0];
		_nextCue = i + 1;
	}
}

void CSTimeChar::setupTalk() {
	if (_unknown1 == 0xffff || !_unknown1 || !_unknown2 || _talkFeature)
		return;

	uint32 flags = kFeatureSortStatic | kFeatureNewNoLoop | kFeatureNewDisableOnReset;
	_talkFeature = _vm->getView()->installViewFeature(getChrBaseId() + (_enabled ? 1 : 14), flags, NULL);

	_talkFeature3 = _vm->getView()->installViewFeature(getChrBaseId() + (_enabled ? 4 : 15), flags, NULL);
	if (_enabled) {
		_talkFeature1 = _vm->getView()->installViewFeature(getChrBaseId() + 2, flags, NULL);
		if (_unknown1 > 1) {
			_talkFeature2 = _vm->getView()->installViewFeature(getChrBaseId() + 10, flags, NULL);
		}
	}
	// FIXME: fix priorities
}

void CSTimeChar::idleTalk() {
	updateWaveStatus();

	if (_waveStatus == 'q') {
		if (_surfingState) {
			// FIXME
			_surfingState = 0;
		} else {
			// FIXME
			_playingWaveId = 0;
		}
		stopFlapping();
		return;
	}

	if (_waveStatus == 's' && _surfingState) {
		// FIXME
		_waveStatus = 'q';
		return;
	}

	CSTimeView *view = _vm->getView();

	if (_enabled && view->getTime() > _lastTime1) {
		_lastTime1 = view->getTime() + 2000 + _vm->_rnd->getRandomNumberRng(0, 1000);
		if (_talkFeature1)
			_talkFeature1->resetFeatureScript(1, getChrBaseId() + 2 + _vm->_rnd->getRandomNumberRng(0, 1));
	}

	if (_enabled && view->getTime() > _lastTime2) {
		_lastTime2 = view->getTime() + 3000 + _vm->_rnd->getRandomNumberRng(0, 1000);
		if (_talkFeature2)
			_talkFeature2->resetFeatureScript(1, getChrBaseId() + 10 + _vm->_rnd->getRandomNumberRng(0, 2));
	}

	if (_waveStatus == 'c') {
		if (_talkFeature3)
			_talkFeature3->resetFeatureScript(1, getChrBaseId() + (_enabled ? 4 : 15));
	} else if (view->getTime() > _lastTime3) {
		_lastTime3 = view->getTime() + 100;
		if (_talkFeature3)
			_talkFeature3->resetFeatureScript(1, getChrBaseId() + (_enabled ? 4 : 15) + _vm->_rnd->getRandomNumberRng(1, 5));
	}

	// FIXME: more animations
}

void CSTimeChar::stopFlapping() {
	_flappingState = 0;
	removeChr();
	// FIXME: stupid hardcoded hack for case 5
	setupAmbientAnims(true);
}

CSTimeConversation::CSTimeConversation(MohawkEngine_CSTime *vm, uint id) : _vm(vm), _id(id) {
	clear();

	Common::SeekableReadStream *convStream = _vm->getResource(ID_CONV, id * 10 + 500);

	_nameId = convStream->readUint16BE();
	_greeting = convStream->readUint16BE();
	_greeting2 = convStream->readUint16BE();

	uint16 qarIds[8];
	for (uint i = 0; i < 8; i++)
		qarIds[i] = convStream->readUint16BE();

	delete convStream;

	for (uint i = 0; i < 8; i++) {
		// FIXME: are they always in order?
		if (qarIds[i] == 0xffff)
			continue;
		_qars.push_back(CSTimeQaR());
		CSTimeQaR &qar = _qars.back();
		loadQaR(qar, qarIds[i]);
	}
}

void CSTimeConversation::start() {
	uint16 greeting = _greeting;

	if (_talkCount > 1)
		greeting = _greeting2;

	_state = 2;

	if (greeting == 0xffff) {
		finishProcessingQaR();
		return;
	}

	CSTimeEvent event;
	event.type = kCSTimeEventCharStartFlapping;
	event.param1 = _sourceChar;
	event.param2 = greeting;
	_vm->addEvent(event);
}

void CSTimeConversation::finishProcessingQaR() {
	if (_state == 2) {
		_vm->getInterface()->getInventoryDisplay()->hide();
		_vm->getInterface()->clearTextLine();
		selectItemsToDisplay();
		display();
		return;
	}

	if (_nextToProcess == 0xffff)
		return;

	uint qarIndex = _itemsToDisplay[_nextToProcess];
	CSTimeQaR &qar = _qars[qarIndex];

	if (!qar.nextQaRsId) {
		end(true);
		_nextToProcess = 0xffff;
		return;
	}

	if (qar.responseStringId != 0xffff) {
		_vm->addEventList(qar.events);
	}

	if (qar.nextQaRsId == 0xffff) {
		_qars.remove_at(qarIndex);
		_vm->getInterface()->clearDialogLine(_nextToProcess);
		_nextToProcess = 0xffff;
		return;
	}

	loadQaR(qar, qar.nextQaRsId);
	if (qar.unknown2)
		qar.finished = true;
	_vm->getInterface()->displayDialogLine(qar.questionStringId, _nextToProcess, qar.finished ? 13 : 32);

	_nextToProcess = 0xffff;
}

void CSTimeConversation::end(bool useLastClicked, bool runEvents) {
	if (runEvents) {
		uint entry = _currEntry;
		if (!useLastClicked)
			entry = _itemsToDisplay.size() - 1;
		CSTimeQaR &qar = _qars[_itemsToDisplay[entry]];
		_vm->addEventList(qar.events);
		if (_sourceChar != 0xffff)
			_vm->getCase()->getCurrScene()->getChar(_sourceChar)->setupAmbientAnims(true);
	}

	CSTimeInterface *iface = _vm->getInterface();
	CSTimeInventoryDisplay *invDisplay = iface->getInventoryDisplay();
	if (invDisplay->getState() == 4) {
		invDisplay->hide();
		invDisplay->setState(0);
	}

	setState((uint)~0);
	_currHover = 0xffff;

	iface->clearTextLine();
	iface->clearDialogArea();
	invDisplay->show();

	// TODO: stupid case 20 stuff
}

void CSTimeConversation::display() {
	_vm->getInterface()->clearDialogArea();

	for (uint i = 0; i < _itemsToDisplay.size(); i++) {
		// FIXME: some rect stuff?

		CSTimeQaR &qar = _qars[_itemsToDisplay[i]];
		_vm->getInterface()->displayDialogLine(qar.questionStringId, i, qar.finished ? 13 : 32);
	}

	_state = 1;
}

void CSTimeConversation::selectItemsToDisplay() {
	_itemsToDisplay.clear();

	for (uint i = 0; i < _qars.size(); i++) {
		if (_qars[i].unknown1 != 0xffff && !_vm->getCase()->checkConvCondition(_qars[i].unknown1))
			continue;
		if (_itemsToDisplay.size() == 5)
			error("Too many conversation paths");
		_itemsToDisplay.push_back(i);
	}
}

void CSTimeConversation::mouseDown(Common::Point &pos) {
	if (_vm->getInterface()->getInventoryDisplay()->getState() == 4)
		return;

	// TODO: case 20 rect check

	for (uint i = 0; i < _itemsToDisplay.size(); i++) {
		Common::Rect thisRect = _vm->getInterface()->_dialogTextRect;
		thisRect.top += 1 + i*15;
		thisRect.bottom = thisRect.top + 15;
		if (!thisRect.contains(pos))
			continue;

		_currEntry = i;
		highlightLine(i);
		_vm->getInterface()->cursorSetShape(5, true);
		break;
	}
}

void CSTimeConversation::mouseMove(Common::Point &pos) {
	// TODO: case 20 rect check

	bool mouseIsDown = _vm->getEventManager()->getButtonState() & 1;

	for (uint i = 0; i < _itemsToDisplay.size(); i++) {
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
			_vm->getInterface()->cursorSetShape(1, true);
			if (_vm->getInterface()->getInventoryDisplay()->getState() != 4)
				unhighlightLine(_currHover);
		}
		_currHover = 0xffff;
	}
}

void CSTimeConversation::mouseUp(Common::Point &pos) {
	if (_vm->getInterface()->getInventoryDisplay()->getState() == 4)
		return;

	if (_currEntry == 0xffff)
		return;

	// TODO: case 20 rect check

	CSTimeQaR &qar = _qars[_itemsToDisplay[_currEntry]];
	Common::Rect thisRect = _vm->getInterface()->_dialogTextRect;
	thisRect.top += 1 + _currEntry*15;
	thisRect.bottom = thisRect.top + 15;
	if (!thisRect.contains(pos))
		return;

	if (qar.responseStringId != 0xffff) {
		CSTimeEvent newEvent;
		newEvent.type = kCSTimeEventCharStartFlapping;
		newEvent.param1 = _sourceChar;
		newEvent.param2 = qar.responseStringId;
		_vm->addEvent(newEvent);
		_nextToProcess = _currEntry;
		return;
	}

	if (!qar.nextQaRsId) {
		_vm->getInterface()->cursorChangeShape(1);
		end(true);
		return;
	}

	_vm->addEventList(qar.events);
	_nextToProcess = _currEntry;
}

void CSTimeConversation::setAsked(uint qar, uint entry) {
	assert(qar < 8 && entry < 5);
	_asked[qar][entry] = true;
}

void CSTimeConversation::clear() {
	_state = (uint)~0;
	_talkCount = 0;
	_sourceChar = 0xffff;
	_currHover = 0xffff;
	_currEntry = 0xffff;
	_nextToProcess = 0xffff;
	for (uint i = 0; i < 8; i++)
		for (uint j = 0; j < 5; j++)
			_asked[i][j] = false;
}

void CSTimeConversation::loadQaR(CSTimeQaR &qar, uint16 id) {
	Common::SeekableReadStream *qarsStream = _vm->getResource(ID_QARS, id);
	qar.finished = false;
	qar.unknown1 = qarsStream->readUint16BE();
	qar.questionStringId = qarsStream->readUint16BE();
	qar.responseStringId = qarsStream->readUint16BE();
	qar.unknown2 = qarsStream->readUint16BE();
	qar.nextQaRsId = qarsStream->readUint16BE();
	uint16 numEvents = qarsStream->readUint16BE();
	for (uint j = 0; j < numEvents; j++) {
		CSTimeEvent event;
		event.type = qarsStream->readUint16BE();
		event.param1 = qarsStream->readUint16BE();
		event.param2 = qarsStream->readUint16BE();
		qar.events.push_back(event);
	}
}

void CSTimeConversation::highlightLine(uint line) {
	CSTimeQaR &qar = _qars[_itemsToDisplay[line]];
	_vm->getInterface()->displayDialogLine(qar.questionStringId, line, 244);
}

void CSTimeConversation::unhighlightLine(uint line) {
	CSTimeQaR &qar = _qars[_itemsToDisplay[line]];
	_vm->getInterface()->displayDialogLine(qar.questionStringId, line, qar.finished ? 13 : 32);
}

CSTimeCase::CSTimeCase(MohawkEngine_CSTime *vm, uint id) : _vm(vm), _id(id) {
	_vm->loadResourceFile(Common::String::format("Cases/C%dText", id));
	// We load this early, so we can use the text for debugging.
	loadRolloverText();

	_vm->loadResourceFile(Common::String::format("Cases/C%dInfo", id));
	Common::SeekableReadStream *caseInfoStream = _vm->getResource(ID_CINF, 1);
	uint16 numScenes = caseInfoStream->readUint16BE();
	uint16 numInvObjs = caseInfoStream->readUint16BE();
	uint16 numConversations = caseInfoStream->readUint16BE();
	for (uint i = 0; i < 3; i++)
		_noteFeatureId[i] = caseInfoStream->readUint16BE();
	delete caseInfoStream;

	debug("Loading %d inventory objects...", numInvObjs);
	for (uint i = 0; i < numInvObjs; i++)
		_inventoryObjs.push_back(loadInventoryObject(i));

	_vm->loadResourceFile(Common::String::format("Cases/C%dArt", id));
	_vm->loadResourceFile(Common::String::format("Cases/C%dDlog", id));

	debug("Loading %d scenes...", numScenes);
	for (uint i = 0; i < numScenes; i++)
		_scenes.push_back(new CSTimeScene(_vm, this, i + 1));

	debug("Loading %d conversations...", numConversations);
	for (uint i = 0; i < numConversations; i++)
		_conversations.push_back(new CSTimeConversation(_vm, i));

	assert(!_conversations.empty());
	_currConv = _conversations[0];

	_currScene = 0xffff;
}

CSTimeCase::~CSTimeCase() {
}

void CSTimeCase::loadRolloverText() {
	Common::SeekableReadStream *stringStream = _vm->getResource(ID_STRL, 9100);
	while (stringStream->pos() < stringStream->size())
		_rolloverText.push_back(readString(stringStream));
	for (uint i = 0; i < _rolloverText.size(); i++)
		debug("string %d: '%s'", i, _rolloverText[i].c_str());
	delete stringStream;
}

CSTimeInventoryObject *CSTimeCase::loadInventoryObject(uint id) {
	CSTimeInventoryObject *invObj = new CSTimeInventoryObject;
	invObj->feature = NULL;
	invObj->id = id;

	Common::SeekableReadStream *invObjStream = _vm->getResource(ID_INVO, id + 1);
	uint16 numHotspots = invObjStream->readUint16BE();
	invObj->stringId = invObjStream->readUint16BE();
	invObj->hotspotId = invObjStream->readUint16BE();
	invObj->featureId = invObjStream->readUint16BE();
	invObj->canTake = invObjStream->readUint16BE();
	invObj->featureDisabled = false;
	debug(" invobj '%s', hotspot id %d, feature id %d, can take %d", _rolloverText[invObj->stringId].c_str(), invObj->hotspotId, invObj->featureId, invObj->canTake);
	uint16 numConsumableLocations = invObjStream->readUint16BE();
	debug(" Loading %d consumable locations...", numConsumableLocations);
	for (uint i = 0; i < numConsumableLocations; i++) {
		CSTimeLocation location;
		location.sceneId = invObjStream->readUint16BE();
		location.hotspotId = invObjStream->readUint16BE();
		invObj->locations.push_back(location);
	}
	uint16 numEvents = invObjStream->readUint16BE();
	debug(" Loading %d events...", numEvents);
	for (uint i = 0; i < numEvents; i++) {
		CSTimeEvent event;
		event.type = invObjStream->readUint16BE();
		event.param1 = invObjStream->readUint16BE();
		event.param2 = invObjStream->readUint16BE();
		invObj->events.push_back(event);
	}
	debug(" Loading %d hotspots...", numHotspots);
	for (uint i = 0; i < numHotspots; i++) {
		CSTimeInventoryHotspot hotspot;
		hotspot.sceneId = invObjStream->readUint16BE();
		hotspot.hotspotId = invObjStream->readUint16BE();
		hotspot.stringId = invObjStream->readUint16BE();
		uint16 numHotspotEvents = invObjStream->readUint16BE();
		debug("  scene %d, hotspot %d, string id %d, with %d hotspot events", hotspot.sceneId, hotspot.hotspotId, hotspot.stringId, numHotspotEvents);
		for (uint j = 0; j < numHotspotEvents; j++) {
			CSTimeEvent event;
			event.type = invObjStream->readUint16BE();
			event.param1 = invObjStream->readUint16BE();
			event.param2 = invObjStream->readUint16BE();
			hotspot.events.push_back(event);
		}
		invObj->hotspots.push_back(hotspot);
	}
	delete invObjStream;

	return invObj;
}

CSTimeScene *CSTimeCase::getCurrScene() {
	return _scenes[_currScene - 1];
}

CSTimeScene::CSTimeScene(MohawkEngine_CSTime *vm, CSTimeCase *case_, uint id) : _vm(vm), _case(case_), _id(id) {
	_visitCount = 0;
	_activeChar = NULL;
	_currHotspot = 0xffff;
	_hoverHotspot = 0xffff;
	load();
}

void CSTimeScene::load() {
	Common::SeekableReadStream *sceneStream = _vm->getResource(ID_SCEN, _id + 1000);
	_unknown1 = sceneStream->readUint16BE();
	_unknown2 = sceneStream->readUint16BE();
	_helperId = sceneStream->readUint16BE();
	_bubbleType = sceneStream->readUint16BE();
	uint16 numHotspots = sceneStream->readUint16BE();
	_numObjects = sceneStream->readUint16BE();
	debug("Scene: unknowns %d, %d, %d, bubble type %d, %d objects", _unknown1, _unknown2, _helperId, _bubbleType, _numObjects);

	uint16 numChars = sceneStream->readUint16BE();
	uint16 numEvents = sceneStream->readUint16BE();
	debug(" Loading %d events...", numEvents);
	for (uint i = 0; i < numEvents; i++) {
		CSTimeEvent event;
		event.type = sceneStream->readUint16BE();
		event.param1 = sceneStream->readUint16BE();
		event.param2 = sceneStream->readUint16BE();
		_events.push_back(event);
	}
	uint16 numEvents2 = sceneStream->readUint16BE();
	debug(" Loading %d events2...", numEvents2);
	for (uint i = 0; i < numEvents2; i++) {
		CSTimeEvent event;
		event.type = sceneStream->readUint16BE();
		event.param1 = sceneStream->readUint16BE();
		event.param2 = sceneStream->readUint16BE();
		_events2.push_back(event);
	}
	debug(" Loading %d chars...", numChars);
	for (uint i = 0; i < numChars; i++) {
		CSTimeChar *chr = new CSTimeChar(_vm, this, i);
		if (!_activeChar)
			_activeChar = chr;
		chr->_enabled = true;
		chr->_unknown1 = sceneStream->readUint16BE();
		chr->_unknown2 = sceneStream->readUint16BE();
		uint16 numAmbients = sceneStream->readUint16BE();
		chr->_unknown3 = sceneStream->readUint16BE();
		debug("  unknowns %d, %d, %d, with %d ambients", chr->_unknown1, chr->_unknown2, chr->_unknown3, numAmbients);
		for (uint j = 0; j < numAmbients; j++) {
			CSTimeAmbient ambient;
			ambient.delay = sceneStream->readUint16BE();
			ambient.feature = NULL;
			chr->_ambients.push_back(ambient);
		}
		_chars.push_back(chr);
	}
	debug(" Loading %d hotspots...", numHotspots);
	for (uint i = 0; i < numHotspots; i++) {
		CSTimeHotspot hotspot;
		hotspot.stringId = sceneStream->readUint16BE();
		hotspot.invObjId = sceneStream->readUint16BE();
		hotspot.cursor = sceneStream->readUint16BE();
		hotspot.state = sceneStream->readUint16BE();
		uint16 numHotspotEvents = sceneStream->readUint16BE();
		debug("  hotspot '%s', inv obj %d, cursor %d, state %d, with %d hotspot events", _case->getRolloverText(hotspot.stringId).c_str(), hotspot.invObjId, hotspot.cursor, hotspot.state, numHotspotEvents);
		for (uint j = 0; j < numHotspotEvents; j++) {
			CSTimeEvent event;
			event.type = sceneStream->readUint16BE();
			event.param1 = sceneStream->readUint16BE();
			event.param2 = sceneStream->readUint16BE();
			hotspot.events.push_back(event);
		}
		_hotspots.push_back(hotspot);
	}
	delete sceneStream;

	Common::SeekableReadStream *hotspotStream = _vm->getResource(ID_HOTS, _id + 1100);
	for (uint i = 0; i < _hotspots.size(); i++) {
		_hotspots[i].region.loadFrom(hotspotStream);
	}
	delete hotspotStream;
}

void CSTimeScene::installGroup() {
	uint16 resourceId = getSceneId();
	_vm->getView()->installGroup(resourceId, _numObjects, 0, true, resourceId);

	for (uint i = 0; i < _chars.size(); i++) {
		uint count = _chars[i]->getScriptCount();
		if (!count)
			continue;
		_vm->getView()->installGroup(resourceId, count, 0, true, _chars[i]->getChrBaseId());
	}
}

void CSTimeScene::buildScene() {
	uint16 resourceId = getSceneId();

	_vm->getView()->installBG(resourceId);

	for (uint i = 0; i < _numObjects; i++) {
		if (!_case->checkObjectCondition(i)) {
			_objectFeatures.push_back(NULL);
			continue;
		}

		// FIXME: reset object if it already exists
		// FIXME: deal with NULL
		uint32 flags = kFeatureSortStatic | kFeatureNewNoLoop | kFeatureNewDisableOnReset;
		assert(flags == 0x4C00000);
		Feature *feature = _vm->getView()->installViewFeature(resourceId + i, flags, NULL);
		_objectFeatures.push_back(feature);
	}
}

void CSTimeScene::leave() {
	for (uint i = 0; i < _objectFeatures.size(); i++) {
		if (_objectFeatures[i] == NULL)
			continue;
		_vm->getView()->removeFeature(_objectFeatures[i], true);
		_objectFeatures[i] = NULL;
	}

	for (uint i = 0; i < _chars.size(); i++) {
		_chars[i]->stopAmbients(false);
		_chars[i]->removeChr();
		_chars[i]->removeNIS();
	}

	_vm->getView()->removeGroup(getSceneId());
}

uint16 CSTimeScene::getSceneId() {
	// FIXME: there are a lot of special cases
	uint16 resourceId = 10000 + 2000 * (_id - 1);
	return resourceId;
}

void CSTimeScene::mouseDown(Common::Point &pos) {
	CSTimeConversation *conv = _vm->getCase()->getCurrConversation();
	bool convActive = (conv->getState() != (uint)~0);
	bool helpActive = (_vm->getInterface()->getHelp()->getState() != (uint)~0);
	if (convActive || helpActive) {
		bool foundPoint = false;
		for (uint i = 0; i < _hotspots.size(); i++) {
			CSTimeHotspot &hotspot = _hotspots[i];
			if (!hotspot.region.containsPoint(pos))
				continue;
			foundPoint = true;

			if (!convActive) {
				// In help mode, we ignore clicks on any help hotspot.
				if (!hotspotContainsEvent(i, kCSTimeEventStartHelp))
					break;
				_currHotspot = 0xffff;
				return;
			}

			// In conversation mode, we ignore clicks which would restart the current conversation.
			for (uint j = 0; j < hotspot.events.size(); j++) {
				if (hotspot.events[j].type != kCSTimeEventStartConversation)
					continue;
				// FIXME: check that the conversation *is* the current one
				_currHotspot = 0xffff;
				return;
			}

			break;
		}

		if (foundPoint) {
			// We found a valid hotspot and we didn't ignore it, stop the conversation/help.
			if (convActive) {
				conv->end(false);
			} else {
				_vm->getInterface()->getHelp()->end();
			}
		}
	} else {
		// FIXME: check symbols
	}

	// FIXME: return if sailing puzzle

	_currHotspot = 0xffff;
	for (uint i = 0; i < _hotspots.size(); i++) {
		CSTimeHotspot &hotspot = _hotspots[i];
		if (!hotspot.region.containsPoint(pos))
			continue;
		if (hotspot.state != 1)
			continue;
		mouseDownOnHotspot(i);
		break;
	}

	if (_currHotspot == 0xffff)
		_vm->getInterface()->cursorSetShape(4, false);
}

void CSTimeScene::mouseMove(Common::Point &pos) {
	// TODO: if we're in sailing puzzle, return

	bool mouseIsDown = _vm->getEventManager()->getButtonState() & 1;

	if (_vm->getInterface()->getState() == kCSTimeInterfaceStateDragStart) {
		Common::Point grabPoint = _vm->getInterface()->getGrabPoint();
		if (mouseIsDown && (abs(grabPoint.x - pos.x) > 2 || abs(grabPoint.y - pos.y) > 2)) {
			if (_vm->getInterface()->grabbedFromInventory()) {
				_vm->getInterface()->startDragging(_vm->getInterface()->getInventoryDisplay()->getLastDisplayedClicked());
			} else {
				CSTimeHotspot &hotspot = _hotspots[_currHotspot];
				if (_vm->_haveInvItem[hotspot.invObjId]) {
					_vm->getInterface()->setState(kCSTimeInterfaceStateNormal);
				} else {
					// FIXME: special-casing for cases 9, 13, 15, 18 19

					CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[hotspot.invObjId];
					if (invObj->feature)
						error("Inventory object already had feature when dragging from scene");
					uint16 id = 9000 + (invObj->id - 1);
					// FIXME: 0x2000 is set! help?
					uint32 flags = kFeatureNewNoLoop | 0x2000;
					invObj->feature = _vm->getView()->installViewFeature(id, flags, &grabPoint);
					invObj->featureDisabled = false;
					_vm->getInterface()->startDragging(hotspot.invObjId);
				}
			}
		}
	}

	for (uint i = 0; i < _hotspots.size(); i++) {
		CSTimeHotspot &hotspot = _hotspots[i];
		if (hotspot.state != 1)
			continue;
		if (!hotspot.region.containsPoint(pos))
			continue;

		if (_vm->getInterface()->getState() == kCSTimeInterfaceStateDragging) {
			// Only show a hotspot here if the dragged object can be dropped onto it.
			CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[_vm->getInterface()->getDraggedNum()];
			bool showText = false;
			for (uint j = 0; j < invObj->hotspots.size(); j++) {
				if (invObj->hotspots[j].sceneId != _id)
					continue;
				if (invObj->hotspots[j].hotspotId != i)
					continue;
				showText = true;
			}
			if (!showText)
				continue;
		} else {
			// If we're not dragging but the mouse is down, ignore all hotspots
			// except the one which was clicked on.
			if (mouseIsDown && (i != _currHotspot))
				continue;
		}

		if (i != _hoverHotspot)
			_vm->getInterface()->clearTextLine();
		cursorOverHotspot(i);
		_hoverHotspot = i;
		return;
	}

	if (_vm->getInterface()->getState() != kCSTimeInterfaceStateDragging) {
		switch (_vm->getInterface()->cursorGetShape()) {
			case 2:
			case 13:
				_vm->getInterface()->cursorSetShape(1);
				break;
			case 5:
			case 14:
				_vm->getInterface()->cursorSetShape(4);
				break;
			case 11:
				_vm->getInterface()->cursorSetShape(10);
				break;
		}
	}

	if (_hoverHotspot == 0xffff)
		return;

	CSTimeConversation *conv = _case->getCurrConversation();
	CSTimeHelp *help = _vm->getInterface()->getHelp();
	if (conv->getState() != (uint)~0 && conv->getState() != 0) {
		Common::String text = "Talking to " + _case->getRolloverText(conv->getNameId());
		_vm->getInterface()->displayTextLine(text);
	} else if (help->getState() != (uint)~0 && help->getState() != 0) {
		Common::String text = "Talking to " + _case->getRolloverText(0);
		_vm->getInterface()->displayTextLine(text);
	} else if (_vm->getInterface()->getState() == kCSTimeInterfaceStateDragging) {
		// TODO: check symbols before this
		CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[_vm->getInterface()->getDraggedNum()];
		Common::String text = "Holding " + _case->getRolloverText(invObj->stringId);
		_vm->getInterface()->displayTextLine(text);
	} else {
		_vm->getInterface()->clearTextLine();
	}

	_hoverHotspot = 0xffff;
}

void CSTimeScene::mouseUp(Common::Point &pos) {
	// TODO: if sailing puzzle is active, return

	if (_currHotspot == 0xffff) {
		if (_vm->getInterface()->cursorGetShape() == 4)
			_vm->getInterface()->cursorChangeShape(1);
		return;
	}

	if (_vm->getInterface()->getState() == kCSTimeInterfaceStateDragStart)
		_vm->getInterface()->setState(kCSTimeInterfaceStateNormal);

	CSTimeHotspot &hotspot = _hotspots[_currHotspot];
	if (hotspot.region.containsPoint(pos) && hotspot.state == 1) {
		mouseUpOnHotspot(_currHotspot);
	} else {
		if (_vm->getInterface()->cursorGetShape() == 4 || _vm->getInterface()->cursorGetShape() == 14)
			_vm->getInterface()->cursorChangeShape(1);
	}
}

void CSTimeScene::idle() {
	// TODO: if sailing puzzle is active, idle it instead

	for (uint i = 0; i < _chars.size(); i++)
		_chars[i]->idle();
}

void CSTimeScene::setupAmbientAnims() {
	for (uint i = 0; i < _chars.size(); i++)
		_chars[i]->setupAmbientAnims(false);
}

void CSTimeScene::idleAmbientAnims() {
	if (_vm->NISIsRunning())
		return;

	for (uint i = 0; i < _chars.size(); i++)
		_chars[i]->idleAmbients();
}

bool CSTimeScene::eventIsActive() {
	return _vm->NISIsRunning() /* TODO || _vm->soundIsPlaying()*/ || _vm->getCurrentEventType() == kCSTimeEventWaitForClick
		|| _activeChar->_flappingState != 0xffff || _vm->getInterface()->getState() == 4;
}

void CSTimeScene::cursorOverHotspot(uint id) {
	CSTimeHotspot &hotspot = _hotspots[id];

	if (!_vm->getInterface()->cursorGetState())
		return;

	if (_vm->getInterface()->getState() == kCSTimeInterfaceStateDragging) {
		uint16 stringId = 0xffff;

		uint16 draggedId = _vm->getInterface()->getDraggedNum();
		CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[draggedId];
		for (uint j = 0; j < invObj->hotspots.size(); j++) {
			if (invObj->hotspots[j].sceneId != _id)
				continue;
			if (invObj->hotspots[j].hotspotId != id)
				continue;
			stringId = invObj->hotspots[j].stringId;
			break;
		}

		if (hotspot.stringId != 0xFFFF) {
			Common::String textLine;
			if (false) {
				// FIXME: special-case time cuffs
			} else {
				bool isChar = (hotspot.cursor == 1) && (draggedId != TIME_CUFFS_ID);
				textLine = (isChar ? "Give " : "Use ");
				textLine += _case->getRolloverText(invObj->stringId);
				textLine += (isChar ? " to " : " on ");
				textLine += _case->getRolloverText(stringId);
			}
			_vm->getInterface()->displayTextLine(textLine);
		}
	} else {
		if (hotspot.stringId != 0xFFFF)
			_vm->getInterface()->displayTextLine(_case->getRolloverText(hotspot.stringId));
	}

	if (_vm->getEventManager()->getButtonState() & 1) {
		if (_vm->getInterface()->getState() != kCSTimeInterfaceStateDragStart && _vm->getInterface()->getState() != kCSTimeInterfaceStateDragging) {
			CSTimeHotspot &currHotspot = _hotspots[_currHotspot];
			if (currHotspot.cursor == 2)
				_vm->getInterface()->cursorSetShape(14);
			else
				_vm->getInterface()->cursorSetShape(5);
		}
	} else {
		if (hotspot.cursor == 2)
			_vm->getInterface()->cursorSetShape(13);
		else if (_vm->getInterface()->cursorGetShape() != 8 && _vm->getInterface()->cursorGetShape() != 11)
			_vm->getInterface()->cursorSetShape(2);
	}
}

void CSTimeScene::mouseDownOnHotspot(uint id) {
	CSTimeHotspot &hotspot = _hotspots[id];

	_currHotspot = id;

	if (hotspot.invObjId == 0xffff || _vm->_haveInvItem[hotspot.invObjId]) {
		if (hotspot.cursor == 2)
			_vm->getInterface()->cursorChangeShape(14);
		else
			_vm->getInterface()->cursorChangeShape(5);
		return;
	}

	_vm->getInterface()->cursorSetShape(8, true);
	_vm->getInterface()->setGrabPoint();
	_vm->getInterface()->setState(kCSTimeInterfaceStateDragStart);

	CSTimeInventoryObject *invObj = _vm->getCase()->_inventoryObjs[hotspot.invObjId];
	_vm->getInterface()->displayTextLine("Pick up " + _case->getRolloverText(invObj->stringId));
}

void CSTimeScene::mouseUpOnHotspot(uint id) {
	CSTimeHotspot &hotspot = _hotspots[id];

	_vm->addEventList(hotspot.events);
	if (_vm->getInterface()->cursorGetShape() == 8 || (!hotspot.events.empty() && _vm->getInterface()->cursorGetShape() == 11))
		return;
	if (hotspot.cursor == 2)
		_vm->getInterface()->cursorChangeShape(13);
	else
		_vm->getInterface()->cursorChangeShape(2);
}

bool CSTimeScene::hotspotContainsEvent(uint id, uint16 eventType) {
	CSTimeHotspot &hotspot = _hotspots[id];

	for (uint i = 0; i < hotspot.events.size(); i++)
		if (hotspot.events[i].type == eventType)
			return true;

	return false;
}

void CSTimeScene::setCursorForCurrentPoint() {
	Common::Point mousePos = _vm->getEventManager()->getMousePos();

	for (uint i = 0; i < _hotspots.size(); i++) {
		if (!_hotspots[i].region.containsPoint(mousePos))
			continue;
		if (_hotspots[i].state != 1)
			continue;
		if (_hotspots[i].cursor == 2) {
			_vm->getInterface()->cursorSetShape(13);
		} else switch (_vm->getInterface()->cursorGetShape()) {
		case 8:
			break;
		case 12:
			_vm->getInterface()->cursorSetShape(11);
			break;
		default:
			_vm->getInterface()->cursorSetShape(2);
			break;
		}
		return;
	}

	_vm->getInterface()->cursorSetShape(1);
}

void CSTimeScene::drawHotspots() {
	for (uint i = 0; i < _hotspots.size(); i++) {
		for (uint j = 0; j < _hotspots[i].region._rects.size(); j++) {
			_vm->_gfx->drawRect(_hotspots[i].region._rects[j], 10 + 5*i);
		}
	}
}

const Common::Array<CSTimeEvent> &CSTimeScene::getEvents(bool second) {
	if (second)
		return _events2;
	else
		return _events;
}

} // End of namespace Mohawk
