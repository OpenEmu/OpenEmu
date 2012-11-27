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

#include "mohawk/cstime.h"
#include "mohawk/cstime_cases.h"
#include "mohawk/cstime_game.h"
#include "mohawk/cstime_ui.h"
#include "mohawk/cstime_view.h"
#include "mohawk/resource.h"
#include "mohawk/cursors.h"
#include "mohawk/video.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/textconsole.h"
#include "common/system.h"

namespace Mohawk {

MohawkEngine_CSTime::MohawkEngine_CSTime(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	_rnd = new Common::RandomSource("cstime");

	// If the user just copied the CD contents, the fonts are in a subdirectory.
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	// They're in setup/data32 for 'Where in Time is Carmen Sandiego?'.
	SearchMan.addSubDirectoryMatching(gameDataDir, "setup/data32");
	// They're in 95instal for 'Carmen Sandiego's Great Chase Through Time'.
	SearchMan.addSubDirectoryMatching(gameDataDir, "95instal");

	_state = kCSTStateStartup;

	reset();
}

MohawkEngine_CSTime::~MohawkEngine_CSTime() {
	delete _interface;
	delete _view;
	delete _console;
	delete _gfx;
	delete _rnd;
}

Common::Error MohawkEngine_CSTime::run() {
	MohawkEngine::run();

	_console = new CSTimeConsole(this);
	_gfx = new CSTimeGraphics(this);
	_cursor = new DefaultCursorManager(this, ID_CURS);

	_interface = new CSTimeInterface(this);

	_view = new CSTimeView(this);
	_view->setupView();
	_view->setModule(new CSTimeModule(this));

	while (!shouldQuit()) {
		switch (_state) {
		case kCSTStateStartup:
			// We just jump straight to the case for now.
			_state = kCSTStateNewCase;
			break;

		case kCSTStateNewCase:
			initCase();
			_state = kCSTStateNewScene;
			break;

		case kCSTStateNewScene:
			nextScene();
			_state = kCSTStateNormal;
			break;

		case kCSTStateNormal:
			update();
			break;
		}
	}

	return Common::kNoError;
}

void MohawkEngine_CSTime::update() {
	Common::Event event;
	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_interface->mouseMove(event.mouse);
			_needsUpdate = true;
			break;

		case Common::EVENT_LBUTTONUP:
			_interface->mouseUp(event.mouse);
			break;

		case Common::EVENT_LBUTTONDOWN:
			_interface->mouseDown(event.mouse);
			break;

		case Common::EVENT_RBUTTONDOWN:
			// (All of this case is only run if the relevant option is enabled.)

			// FIXME: Don't do these if the options are open.
			if (_case->getCurrScene()->_activeChar->_flappingState != 0xffff)
				_case->getCurrScene()->_activeChar->interruptFlapping();
			if (getCurrentEventType() == kCSTimeEventWaitForClick)
				mouseClicked();

			// TODO: This is always run, even if not in-game.
			//pauseCurrentNIS();
			//stopSound();
			// FIXME: There's some more stuff here.
			break;

		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_d:
				if (event.kbd.flags & Common::KBD_CTRL) {
					_console->attach();
					_console->onFrame();
				}
				break;

			case Common::KEYCODE_SPACE:
				pauseGame();
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

	_needsUpdate = true;

	if (_video->updateMovies())
		_needsUpdate = true;

	if (_needsUpdate) {
		_view->_needsUpdate = true;
		_needsUpdate = false;
	}

	eventIdle();
	_interface->idle();

	// Cut down on CPU usage
	_system->delayMillis(10);
}

void MohawkEngine_CSTime::initCase() {
	_interface->openResFile();
	_interface->install();
	_interface->cursorInstall();
	// TODO: _interface->paletteOff(true);
	_interface->cursorActivate(true);
	_interface->cursorSetShape(1, false);
	for (uint i = 0; i < 19; i++)
		_haveInvItem[i] = 0;
	_interface->getInventoryDisplay()->clearDisplay();
	_interface->getCarmenNote()->clearPieces();
	// TODO: fixup book rect for case 20
	for (uint i = 0; i < 20; i++)
		_caseVariable[i] = 0;
	_case = new CSTimeCase1(this); // TODO
	_interface->getInventoryDisplay()->install();
	_nextSceneId = 1;
}

void MohawkEngine_CSTime::nextScene() {
	_case->setCurrScene(_nextSceneId);
	CSTimeScene *scene = _case->getCurrScene();
	// TODO: scene->setState(1);
	scene->_visitCount++;
	scene->installGroup();

	_interface->draw();
	scene->buildScene();
	scene->setupAmbientAnims();
	_interface->cursorSetShape(1, false);

	addEvent(CSTimeEvent(kCSTimeEventWait, 0xffff, 500));
	scene->idleAmbientAnims();
	// TODO: startEnvironmentSound();
	if (scene->_visitCount == 1) {
		addEventList(scene->getEvents(false));
	} else {
		addEventList(scene->getEvents(true));
	}
	_view->idleView();
	// TODO: maybe startMusic();
}

void MohawkEngine_CSTime::loadResourceFile(Common::String name) {
	MohawkArchive *archive = new MohawkArchive();
	if (!archive->openFile(name + ".mhk"))
		error("failed to open %s.mhk", name.c_str());
	_mhk.push_back(archive);
}

void MohawkEngine_CSTime::addEvent(const CSTimeEvent &event) {
	_events.push_back(event);
}

void MohawkEngine_CSTime::addEventList(const Common::Array<CSTimeEvent> &list) {
	for (uint i = 0; i < list.size(); i++)
		addEvent(list[i]);
}

void MohawkEngine_CSTime::insertEventAtFront(const CSTimeEvent &event) {
	if (_events.empty())
		_events.push_front(event);
	else
		_events.insert(++_events.begin(), event);
}

uint16 MohawkEngine_CSTime::getCurrentEventType() {
	if (_events.empty())
		return 0xffff;
	else
		return _events.front().type;
}

void MohawkEngine_CSTime::eventIdle() {
	bool done = false;
	while (_events.size() && !done && true /* TODO: !_options->getState() */) {
		_lastTimeout = 0xffffffff;

		bool advanceQueue = true;
		bool processEvent = true;
		CSTimeEvent &event = _events.front();

		switch (event.type) {
		case kCSTimeEventCharPlayNIS:
			if (_NISRunning) {
				CSTimeChar *chr = _case->getCurrScene()->getChar(event.param1);
				if (chr->NISIsDone()) {
					chr->removeNIS();
					_NISRunning = false;
					chr->setupAmbientAnims(true);
					_events.pop_front();
					processEvent = false;
				} else {
					done = true;
				}
			} else {
				advanceQueue = false;
			}
			break;

		case kCSTimeEventNewScene:
			if (_processingEvent) {
				processEvent = false;
				_events.pop_front();
				_processingEvent = false;
				// FIXME: check skip global, if set:
				// stopMusic(), stopEnvironmentSound(), set scene to _nextSceneId,
				// set scene state to 1, set state to idle loop
			} else {
				triggerEvent(event);
				done = true;
				_processingEvent = true;
			}
			break;

		case kCSTimeEventCharStartFlapping:
			assert(_case->getCurrScene()->_activeChar);
			switch (_case->getCurrScene()->_activeChar->_flappingState) {
			case 0xffff:
				// FIXME: check skip global, if set, processEvent = false and pop event
				advanceQueue = false;
				break;
			case 0:
				_case->getCurrScene()->_activeChar->_flappingState = 0xffff;
				_events.pop_front();
				processEvent = false;
				break;
			default:
				done = true;
				break;
			}
			break;

		case kCSTimeEventCharSomeNIS55:
			// This is like kCSTimeEventCharPlayNIS, only using a different flag variable.
			if (_processingNIS55) {
				CSTimeChar *chr = _case->getCurrScene()->getChar(event.param1);
				if (chr->NISIsDone()) {
					chr->removeNIS();
					_processingNIS55 = false;
					chr->setupAmbientAnims(true);
					_events.pop_front();
					processEvent = false;
				} else {
					done = true;
				}
			} else {
				advanceQueue = false;
			}
			break;

		default:
			break;
		}

		if (!done && processEvent) {
			_interface->cursorSetWaitCursor();
			triggerEvent(event);
			if (advanceQueue)
				_events.pop_front();
		}

		if (!_events.size()) {
			Common::Point pos = _system->getEventManager()->getMousePos();
			if (_interface->_sceneRect.contains(pos))
				_case->getCurrScene()->setCursorForCurrentPoint();
			else
				_interface->setCursorForCurrentPoint();
			_interface->mouseMove(pos);
			resetTimeout();
		}
	}
}

void MohawkEngine_CSTime::resetTimeout() {
	_lastTimeout = _system->getMillis();
}

void MohawkEngine_CSTime::mouseClicked() {
	// TODO
}

bool MohawkEngine_CSTime::NISIsRunning() {
	if (_NISRunning || (!_events.empty() && _events.front().type == kCSTimeEventUnused63))
		return true;

	return false;
}

void MohawkEngine_CSTime::reset() {
	_NISRunning = false;
	_processingNIS55 = false;
	_lastTimeout = 0xffffffff;
	_processingEvent = false;
}

void MohawkEngine_CSTime::triggerEvent(CSTimeEvent &event) {
	debug("triggerEvent: type %d, param1 %d, param2 %d", event.type, event.param1, event.param2);

	switch (event.type) {
	case kCSTimeEventNothing:
	case kCSTimeEventUnused8:
	case kCSTimeEventUnused21:
	case kCSTimeEventUnused63:
		break;

	case kCSTimeEventCondition:
		_case->handleConditionalEvent(event);
		break;

	case kCSTimeEventCharPlayNIS:
		_case->getCurrScene()->getChar(event.param1)->playNIS(event.param2);
		_NISRunning = true;
		break;

	case kCSTimeEventStartConversation:
		_case->setConversation(event.param2);
		_case->getCurrConversation()->setSourceChar(event.param1);
		_case->getCurrConversation()->incrementTalkCount();
		_case->getCurrConversation()->start();
		break;

	case kCSTimeEventNewScene:
		if (_case->getCurrConversation()->getState() != (uint)~0)
			_case->getCurrConversation()->end(false);
		_interface->clearTextLine();
		// TODO: _interface->fadeDown();
		_interface->cursorSetShape(1);
		// TODO: swap cursor
		// TODO: unloadPreloadedSounds?
		if (_interface->getInventoryDisplay()->getState() == 4) {
			_interface->getInventoryDisplay()->hide();
			_interface->getInventoryDisplay()->setState(0);
		}
		// TODO: stupid case 20 handling
		_case->getCurrScene()->leave();
		// TODO: set dim palette(true)
		_view->_needsUpdate = true;
		_view->idleView();
		// TODO: set dim palette(false)
		_nextSceneId = event.param2;
		_state = kCSTStateNewScene;
		break;

	case kCSTimeEventCharStartFlapping:
		{
		CSTimeChar *chr = _case->getCurrScene()->getChar(event.param1);
		if (!chr->_unknown2) {
			_case->getCurrScene()->_activeChar->_flappingState = 0;
			break;
		}

		chr->startFlapping(event.param2);
		if (event.param2)
			_interface->drawTextIdToBubble(event.param2);

		CSTimeEvent newEvent;
		newEvent.param1 = 0xffff;
		newEvent.param2 = 0xffff;

		newEvent.type = kCSTimeEventUnknown70;
		insertEventAtFront(newEvent);

		newEvent.type = kCSTimeEventUpdateBubble;
		insertEventAtFront(newEvent);

		newEvent.type = kCSTimeEventUnknown69;
		insertEventAtFront(newEvent);
		}
		break;

	case kCSTimeEventDropItemInInventory:
		_interface->dropItemInInventory(event.param2);
		break;

	case kCSTimeEventRemoveItemFromInventory:
		if (!_interface->getInventoryDisplay()->isItemDisplayed(event.param2))
			break;
		_haveInvItem[event.param2] = 0;
		_interface->getInventoryDisplay()->removeItem(event.param2);
		break;

	case kCSTimeEventAddNotePiece:
		_interface->clearTextLine();
		_interface->getCarmenNote()->addPiece(event.param2, event.param1);
		break;

	case kCSTimeEventDisableHotspot:
		_case->getCurrScene()->getHotspot(event.param2).state = 0;
		break;

	case kCSTimeEventDisableFeature:
		if (!_case->getCurrScene()->_objectFeatures[event.param2])
			break;
		_view->removeFeature(_case->getCurrScene()->_objectFeatures[event.param2], true);
		_case->getCurrScene()->_objectFeatures[event.param2] = NULL;
		break;

	case kCSTimeEventAddFeature:
		// TODO: merge this with buildScene somehow?
		if (_case->getCurrScene()->_objectFeatures[event.param2]) {
			_case->getCurrScene()->_objectFeatures[event.param2]->resetFeatureScript(1, 0);
			break;
		}
		{
		uint32 flags = kFeatureSortStatic | kFeatureNewNoLoop | kFeatureNewDisableOnReset;
		_case->getCurrScene()->_objectFeatures[event.param2] = _view->installViewFeature(_case->getCurrScene()->getSceneId() + event.param2, flags, NULL);
		// FIXME: a mess of priority stuff
		}
		break;

	case kCSTimeEventEnableHotspot:
		_case->getCurrScene()->getHotspot(event.param2).state = 1;
		break;

	case kCSTimeEventSetAsked:
		uint qar, entry;
		qar = event.param2 / 5;
		entry = event.param2 % 5;
		if (qar > 7)
			error("SetAsked event out of range");
		_case->getCurrConversation()->setAsked(qar, entry);
		break;

	case kCSTimeEventStartHelp:
		_interface->getHelp()->start();
		break;

	case kCSTimeEventShowBigNote:
		_interface->getCarmenNote()->drawBigNote();
		break;

	case kCSTimeEventActivateCuffs:
		_interface->getInventoryDisplay()->activateCuffs(true);
		break;

	case kCSTimeEventHelperSetupRestPos:
		_case->getCurrScene()->getChar(_case->getCurrScene()->getHelperId())->setupRestPos();
		break;

	case kCSTimeEventUnknown25:
		_case->getCurrScene()->getChar(event.param1)->_unknown2 = 1;
		break;

	case kCSTimeEventUnknown26:
		_case->getCurrScene()->getChar(event.param1)->_unknown2 = 0;
		break;

	case kCSTimeEventWait:
		warning("ignoring wait");
		// FIXME
		break;

	case kCSTimeEventCharSetState:
		_case->getCurrScene()->getChar(event.param1)->_enabled = event.param2;
		break;

	case kCSTimeEventCharSetupRestPos:
		_case->getCurrScene()->getChar(event.param1)->setupRestPos();
		break;

	case kCSTimeEventStopEnvironmentSound:
		warning("ignoring stop environment sound");
		// FIXME
		break;

	case kCSTimeEventSetMusic:
		warning("ignoring set music");
		// FIXME
		break;

	case kCSTimeEventSetInsertBefore:
		warning("ignoring insert before");
		// FIXME
		break;

	case kCSTimeEventCharSomeNIS55:
		_processingNIS55 = true;
		_case->getCurrScene()->getChar(event.param1)->playNIS(event.param2);
		break;

	case kCSTimeEventUpdateBubble:
		switch (event.param2) {
		case 0:
			// FIXME
			warning("ignoring bubble update (queue events)");
			break;
		case 1:
			// FIXME
			warning("ignoring bubble update (install)");
			break;
		default:
			_interface->closeBubble();
			break;
		}
		break;

	case kCSTimeEventInitScene:
		_interface->displayTextLine("");

		// FIXME: install palette

		// FIXME: swapCursor(true)
		break;

	case kCSTimeEventUnknown69:
		// TODO: if persistent text, insert a kCSTimeEventWaitForClick in front of the queue
		break;

	case kCSTimeEventUnknown70:
		if (_case->getCurrConversation()->getState() != (uint)~0 && _case->getCurrConversation()->getState()) {
			_case->getCurrConversation()->finishProcessingQaR();
		} else if (_interface->getHelp()->getState() != (uint)~0 && _interface->getHelp()->getState()) {
			_interface->getHelp()->cleanupAfterFlapping();
		}
		break;

	default:
		error("unknown scene event type %d", event.type);
	}
}

} // End of namespace Mohawk
