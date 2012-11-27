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

// Event management module

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/animation.h"
#include "saga/console.h"
#include "saga/scene.h"
#include "saga/interface.h"
#include "saga/palanim.h"
#include "saga/render.h"
#include "saga/sndres.h"
#include "saga/resource.h"
#include "saga/music.h"
#include "saga/actor.h"

#include "saga/events.h"

namespace Saga {

Events::Events(SagaEngine *vm) : _vm(vm) {
	debug(8, "Initializing event subsystem...");
}

Events::~Events() {
	debug(8, "Shutting down event subsystem...");
}

// Function to process event list once per frame.
// First advances event times, then processes each event with the appropriate
// handler depending on the type of event.
void Events::handleEvents(long msec) {
	long delta_time;
	int result;

	// Advance event times
	processEventTime(msec);

	// Process each event in list
	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {
		Event *event_p = &eventi->front();

		// Call the appropriate event handler for the specific event type
		switch (event_p->type) {

		case kEvTOneshot:
			result = handleOneShot(event_p);
			break;

		case kEvTContinuous:
			result = handleContinuous(event_p);
			break;

		case kEvTInterval:
			result = handleInterval(event_p);
			break;

		case kEvTImmediate:
			result = handleImmediate(event_p);
			break;

		default:
			result = kEvStInvalidCode;
			warning("Invalid event code encountered");
			break;
		}

		// Process the event appropriately based on result code from
		// handler
		if ((result == kEvStDelete) || (result == kEvStInvalidCode)) {
			// If there is no event chain, delete the base event.
			if (eventi->size() < 2) {
				eventi = _eventList.reverse_erase(eventi);
			} else {
				// If there is an event chain present, move the next event
				// in the chain up, adjust it by the previous delta time,
				// and reprocess the event
				delta_time = event_p->time;
				eventi->pop_front();
				event_p = &eventi->front();
				event_p->time += delta_time;
				--eventi;
			}
		} else if (result == kEvStBreak) {
			break;
		}
	}
}

int Events::handleContinuous(Event *event) {
	double event_pc = 0.0; // Event completion percentage
	int event_done = 0;

	BGInfo bgInfo;
	Rect rect;
	if (event->duration != 0) {
		event_pc = ((double)event->duration - event->time) / event->duration;
	} else {
		event_pc = 1.0;
	}

	if (event_pc >= 1.0) {
		// Cap percentage to 100
		event_pc = 1.0;
		event_done = 1;
	}

	if (event_pc < 0.0) {
		// Event not signaled, skip it
		return kEvStContinue;
	} else if (!(event->code & kEvFSignaled)) {
		// Signal event
		event->code |= kEvFSignaled;
		event_pc = 0.0;
	}

	switch (event->code & EVENT_MASK) {
	case kPalEvent:
		switch (event->op) {
		case kEventBlackToPal:
			_vm->_gfx->blackToPal((PalEntry *)event->data, event_pc);
			break;
		case kEventPalToBlack:
			_vm->_gfx->palToBlack((PalEntry *)event->data, event_pc);
			break;
#ifdef ENABLE_IHNM
		case kEventPalFade:
			_vm->_gfx->palFade((PalEntry *)event->data, event->param, event->param2, event->param3, event->param4, event_pc);
			break;
#endif
		default:
			break;
		}
		break;
	case kTransitionEvent:
		switch (event->op) {
		case kEventDissolve:
			_vm->_scene->getBGInfo(bgInfo);
			rect.left = rect.top = 0;
			rect.right = bgInfo.bounds.width();
			rect.bottom = bgInfo.bounds.height();
			_vm->_render->getBackGroundSurface()->transitionDissolve(bgInfo.buffer, rect, 0, event_pc);
			_vm->_render->setFullRefresh(true);
			break;
		case kEventDissolveBGMask:
			// we dissolve it centered.
			// set flag of Dissolve to 1. It is a hack to simulate zero masking.
			int w, h;
			byte *maskBuffer;

			_vm->_scene->getBGMaskInfo(w, h, maskBuffer);
			rect.left = (_vm->getDisplayInfo().width - w) / 2;
			rect.top = (_vm->getDisplayInfo().height - h) / 2;
			rect.setWidth(w);
			rect.setHeight(h);

			_vm->_render->getBackGroundSurface()->transitionDissolve( maskBuffer, rect, 1, event_pc);
			_vm->_render->setFullRefresh(true);
			break;
		default:
			break;
		}
		break;
	default:
		break;

	}

	if (event_done) {
		return kEvStDelete;
	}

	return kEvStContinue;
}

int Events::handleImmediate(Event *event) {
	double event_pc = 0.0; // Event completion percentage
	bool event_done = false;

	// Duration might be 0 so dont do division then
	if (event->duration != 0) {
		event_pc = ((double)event->duration - event->time) / event->duration;
	} else {
		// Just make sure that event_pc is 1.0 so event_done is true
		event_pc = 1.0;
	}

	if (event_pc >= 1.0) {
		// Cap percentage to 100
		event_pc = 1.0;
		event_done = true;
	}

	if (event_pc < 0.0) {
		// Event not signaled, skip it
		return kEvStBreak;
	} else if (!(event->code & kEvFSignaled)) {
		// Signal event
		event->code |= kEvFSignaled;
		event_pc = 0.0;
	}

	switch (event->code & EVENT_MASK) {
	case kPalEvent:
		switch (event->op) {
		case kEventBlackToPal:
			_vm->_gfx->blackToPal((PalEntry *)event->data, event_pc);
			break;
		case kEventPalToBlack:
			_vm->_gfx->palToBlack((PalEntry *)event->data, event_pc);
			break;
#ifdef ENABLE_IHNM
		case kEventPalFade:
			_vm->_gfx->palFade((PalEntry *)event->data, event->param, event->param2, event->param3, event->param4, event_pc);
			break;
#endif
		default:
			break;
		}
		break;
	case kScriptEvent:
	case kBgEvent:
	case kInterfaceEvent:
	case kSceneEvent:
	case kAnimEvent:
	case kCutawayEvent:
	case kActorEvent:
		handleOneShot(event);
		event_done = true;
		break;
	default:
		warning("Unhandled Immediate event type (%d)", event->code & EVENT_MASK);
		break;

	}

	if (event_done) {
		return kEvStDelete;
	}

	return kEvStBreak;
}

int Events::handleOneShot(Event *event) {
	Rect rect;


	if (event->time > 0) {
		return kEvStContinue;
	}

	// Event has been signaled

	switch (event->code & EVENT_MASK) {
	case kTextEvent:
		switch (event->op) {
		case kEventDisplay:
			((TextListEntry *)event->data)->display = true;
			break;
		case kEventRemove: {
			TextListEntry entry = *((TextListEntry *)event->data);
			_vm->_scene->_textList.remove(entry);
			} break;
		default:
			break;
		}

		break;
	case kSoundEvent:
		_vm->_sound->stopSound();
		if (event->op == kEventPlay)
			_vm->_sndRes->playSound(event->param, event->param2, event->param3 != 0);
		break;
	case kVoiceEvent:
		_vm->_sndRes->playVoice(event->param);
		break;
	case kMusicEvent:
		if (event->op == kEventPlay)
			_vm->_music->play(event->param, (MusicFlags)event->param2);
		break;
	case kBgEvent:
		{
			Surface *backGroundSurface = _vm->_render->getBackGroundSurface();
			BGInfo bgInfo;

			if (!(_vm->_scene->getFlags() & kSceneFlagISO)) {
				_vm->_scene->getBGInfo(bgInfo);

				backGroundSurface->blit(bgInfo.bounds, bgInfo.buffer);

				// If it is inset scene then draw black border
				if (bgInfo.bounds.width() < _vm->getDisplayInfo().width || bgInfo.bounds.height() < _vm->_scene->getHeight()) {
					Common::Rect rect1(2, bgInfo.bounds.height() + 4);
					Common::Rect rect2(bgInfo.bounds.width() + 4, 2);
					Common::Rect rect3(2, bgInfo.bounds.height() + 4);
					Common::Rect rect4(bgInfo.bounds.width() + 4, 2);
					rect1.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.top - 2);
					rect2.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.top - 2);
					rect3.moveTo(bgInfo.bounds.right, bgInfo.bounds.top - 2);
					rect4.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.bottom);

					backGroundSurface->drawRect(rect1, kITEColorBlack);
					backGroundSurface->drawRect(rect2, kITEColorBlack);
					backGroundSurface->drawRect(rect3, kITEColorBlack);
					backGroundSurface->drawRect(rect4, kITEColorBlack);
				}

				if (event->param == kEvPSetPalette) {
					PalEntry *palPointer;

#ifdef ENABLE_IHNM
					if (_vm->getGameId() == GID_IHNM) {
						PalEntry portraitBgColor = _vm->_interface->_portraitBgColor;
						byte portraitColor = (_vm->getLanguage() == Common::ES_ESP) ? 253 : 254;

						// Set the portrait bg color, in case a saved state is restored from the
						// launcher. In this case, sfSetPortraitBgColor is not called, thus the
						// portrait color will always be 0 (black).
						if (portraitBgColor.red == 0 && portraitBgColor.green == 0 && portraitBgColor.blue == 0)
							portraitBgColor.green = 255;

						if (_vm->_spiritualBarometer > 255)
							_vm->_gfx->setPaletteColor(portraitColor, 0xff, 0xff, 0xff);
						else
							_vm->_gfx->setPaletteColor(portraitColor,
								_vm->_spiritualBarometer * portraitBgColor.red / 256,
								_vm->_spiritualBarometer * portraitBgColor.green / 256,
								_vm->_spiritualBarometer * portraitBgColor.blue / 256);
					}
#endif

					_vm->_scene->getBGPal(palPointer);
					_vm->_gfx->setPalette(palPointer);
				}
			}
			_vm->_render->clearFlag(RF_DISABLE_ACTORS);
		}
		break;
	case kPsychicProfileBgEvent:
		{
		ResourceContext *context = _vm->_resource->getContext(GAME_RESOURCEFILE);

		ByteArray resourceData;

		_vm->_resource->loadResource(context, _vm->getResourceDescription()->psychicProfileResourceId, resourceData);

		ByteArray image;
		int width;
		int height;

		_vm->decodeBGImage(resourceData, image, &width, &height);

		const PalEntry *palette = (const PalEntry *)_vm->getImagePal(resourceData);

		const Rect profileRect(width, height);

		_vm->_render->getBackGroundSurface()->blit(profileRect, image.getBuffer());
		_vm->_render->addDirtyRect(profileRect);
		_vm->_frameCount++;

		_vm->_gfx->setPalette(palette);

		// Draw the scene. It won't be drawn by Render::drawScene(), as a placard is up
		_vm->_scene->draw();
		}
		break;
	case kAnimEvent:
		switch (event->op) {
		case kEventPlay:
			_vm->_anim->play(event->param, event->time, true);
			break;
		case kEventStop:
			_vm->_anim->stop(event->param);
			break;
		case kEventFrame:
			_vm->_anim->play(event->param, event->time, false);
			break;
		case kEventSetFlag:
			_vm->_anim->setFlag(event->param, event->param2);
			break;
		case kEventClearFlag:
			_vm->_anim->clearFlag(event->param, event->param2);
			break;
		case kEventResumeAll:
			_vm->_anim->resumeAll();
			break;
		default:
			break;
		}
		break;
	case kSceneEvent:
		switch (event->op) {
		case kEventDraw:
			{
				BGInfo bgInfo;
				_vm->_scene->getBGInfo(bgInfo);
				_vm->_render->getBackGroundSurface()->blit(bgInfo.bounds, bgInfo.buffer);
				_vm->_render->addDirtyRect(bgInfo.bounds);
				_vm->_scene->draw();
			}
			break;
		case kEventEnd:
			_vm->_scene->nextScene();
			return kEvStBreak;
		default:
			break;
		}
		break;
	case kPalAnimEvent:
		switch (event->op) {
		case kEventCycleStart:
			_vm->_palanim->cycleStart();
			break;
		case kEventCycleStep:
			_vm->_palanim->cycleStep(event->time);
			break;
		default:
			break;
		}
		break;
	case kInterfaceEvent:
		switch (event->op) {
		case kEventActivate:
			_vm->_interface->activate();
			break;
		case kEventDeactivate:
			_vm->_interface->deactivate();
			break;
		case kEventSetStatus:
			_vm->_interface->setStatusText((const char*)event->data);
			_vm->_interface->drawStatusBar();
			break;
		case kEventClearStatus:
			_vm->_interface->setStatusText("");
			_vm->_interface->drawStatusBar();
			break;
		case kEventSetFadeMode:
			_vm->_interface->setFadeMode(event->param);
			break;
		case kEventRestoreMode:
			_vm->_interface->restoreMode();
			break;
		case kEventSetMode:
			_vm->_interface->setMode(event->param);
			break;
		default:
			break;
		}
		break;
	case kScriptEvent:
		switch (event->op) {
		case kEventExecBlocking:
		case kEventExecNonBlocking: {
			debug(6, "Exec module number %ld script entry number %ld", event->param, event->param2);

			ScriptThread &sthread = _vm->_script->createThread(event->param, event->param2);
			sthread._threadVars[kThreadVarAction] = event->param3;
			sthread._threadVars[kThreadVarObject] = event->param4;
			sthread._threadVars[kThreadVarWithObject] = event->param5;
			sthread._threadVars[kThreadVarActor] = event->param6;

			if (event->op == kEventExecBlocking)
				_vm->_script->completeThread();

			break;
			}
		case kEventThreadWake:
			_vm->_script->wakeUpThreads(event->param);
			break;
		}
		break;
	case kCursorEvent:
		switch (event->op) {
		case kEventShow:
			_vm->_gfx->showCursor(true);
			break;
		case kEventHide:
			_vm->_gfx->showCursor(false);
			break;
		case kEventSetNormalCursor:
			// in ITE and IHNM demo there is just one cursor
			// ITE never makes this call
			if (!_vm->isIHNMDemo())
				_vm->_gfx->setCursor(kCursorNormal);
			break;
		case kEventSetBusyCursor:
			// in ITE and IHNM demo there is just one cursor
			// ITE never makes this call
			if (!_vm->isIHNMDemo())
				_vm->_gfx->setCursor(kCursorBusy);
			break;
		default:
			break;
		}
		break;
	case kGraphicsEvent:
		switch (event->op) {
		case kEventFillRect:
			rect.top = event->param2;
			rect.bottom = event->param3;
			rect.left = event->param4;
			rect.right = event->param5;
			_vm->_gfx->drawRect(rect, event->param);
			break;
		case kEventSetFlag:
			_vm->_render->setFlag(event->param);
			break;
		case kEventClearFlag:
			_vm->_render->clearFlag(event->param);
			break;
		default:
			break;
		}
#ifdef ENABLE_IHNM
	case kCutawayEvent:
		switch (event->op) {
		case kEventClear:
			_vm->_anim->clearCutaway();
			break;
		case kEventShowCutawayBg:
			_vm->_anim->showCutawayBg(event->param);
			break;
		default:
			break;
		}
#endif
	case kActorEvent:
		switch (event->op) {
		case kEventMove:
			// TODO (check Actor::direct)
			break;
		default:
			break;
		}
	default:
		break;
	}

	return kEvStDelete;
}

int Events::handleInterval(Event *event) {
	return kEvStDelete;
}

EventColumns *Events::chain(EventColumns *eventColumns, const Event &event) {

	if (eventColumns == NULL) {
		EventColumns tmp;

		_eventList.push_back(tmp);
		eventColumns = &_eventList.back();
	}

	eventColumns->push_back(event);
	initializeEvent(eventColumns->back());

	return eventColumns;
}

void Events::initializeEvent(Event &event) {
	switch (event.type) {
	case kEvTOneshot:
		break;
	case kEvTContinuous:
	case kEvTImmediate:
		event.time += event.duration;
		break;
	case kEvTInterval:
		break;
	}
}

void Events::clearList(bool playQueuedMusic) {
	// Walk down event list
	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {

		// Only remove events not marked kEvFNoDestory (engine events)
		if (!(eventi->front().code & kEvFNoDestory)) {
			// Handle queued music change events before deleting them
			// This can happen in IHNM by music events set by sfQueueMusic()
			// Fixes bug #2057987 - "IHNM: Music stops in Ellen's chapter"
			if (playQueuedMusic && ((eventi->front().code & EVENT_MASK) == kMusicEvent)) {
				_vm->_music->stop();
				if (eventi->front().op == kEventPlay)
					_vm->_music->play(eventi->front().param, (MusicFlags)eventi->front().param2);
			}

			eventi = _eventList.reverse_erase(eventi);
		}
	}
}

// Removes all events from the list (even kEvFNoDestory)
void Events::freeList() {
	_eventList.clear();
}

// Walks down the event list, updating event times by 'msec'.
void Events::processEventTime(long msec) {
	uint16 event_count = 0;

	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {
		eventi->front().time -= msec;
		event_count++;

		if (eventi->front().type == kEvTImmediate)
			break;

		if (event_count > EVENT_WARNINGCOUNT) {
			warning("Event list exceeds %u", EVENT_WARNINGCOUNT);
		}
	}
}

} // End of namespace Saga
