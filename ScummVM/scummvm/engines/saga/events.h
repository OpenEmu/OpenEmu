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

// Event management module header file

#ifndef SAGA_EVENT_H
#define SAGA_EVENT_H

#include "common/list.h"

namespace Saga {

enum EventTypes {
	kEvTOneshot,    // Event takes no time
	kEvTContinuous, // Event takes time; next event starts immediately
	kEvTInterval,   // Not yet implemented
	kEvTImmediate   // Event takes time; next event starts when event is done
};

enum EventFlags {
	kEvFSignaled = 0x8000,
	kEvFNoDestory = 0x4000
};

enum EventCodes {
	kBgEvent = 1,
	kAnimEvent,
	kMusicEvent,
	kVoiceEvent,
	kSoundEvent,
	kSceneEvent,
	kTextEvent,
	kPalEvent,
	kPalAnimEvent,
	kTransitionEvent,
	kInterfaceEvent,
	kActorEvent,
	kScriptEvent,
	kCursorEvent,
	kGraphicsEvent,
	kCutawayEvent,
	kPsychicProfileBgEvent
};

enum EventOps {
	// INSTANTANEOUS events
	// BG events
	kEventDisplay = 1,
	// ANIM events
	kEventPlay = 1,			// used in music and sound events too
	kEventStop = 2,			// used in music and sound events too
	kEventFrame = 3,
	kEventSetFlag = 4,		// used in graphics events too
	kEventClearFlag = 5,	// used in graphics events too
	kEventResumeAll = 6,
	// MUSIC and SOUND events
	// Reused: kEventPlay, kEventStop
	// SCENE events
	kEventDraw = 1,
	kEventEnd = 2,
	// TEXT events
	kEventRemove = 3,
	// Reused: kEventHide
	// PALANIM events
	kEventCycleStart = 1,
	kEventCycleStep = 2,
	// INTERFACE events
	kEventActivate = 1,
	kEventDeactivate = 2,
	kEventSetStatus = 3,
	kEventClearStatus = 4,
	kEventSetFadeMode = 5,
	kEventRestoreMode = 6,
	kEventSetMode = 7,
	// ACTOR events
	kEventMove = 1,
	// SCRIPT events
	kEventExecBlocking = 1,
	kEventExecNonBlocking = 2,
	kEventThreadWake = 3,
	// CURSOR events
	kEventShow = 1,
	kEventHide = 2,			// used in text events too
	kEventSetNormalCursor = 3,
	kEventSetBusyCursor = 4,
	// GRAPHICS events
	kEventFillRect = 1,
	// Reused: kEventSetFlag, kEventClearFlag
	// CONTINUOUS events
	//
	// PALETTE events
	kEventPalToBlack = 1,
	kEventBlackToPal = 2,
	kEventPalFade = 3,
	// TRANSITION events
	kEventDissolve = 1,
	kEventDissolveBGMask = 2,
	// CUTAWAY events
	kEventClear = 1,
	kEventShowCutawayBg = 2
};

enum EventParams {
	kEvPNoSetPalette,
	kEvPSetPalette
};

struct Event {
	unsigned int type;
	unsigned int code; // Event operation category & flags
	int op;            // Event operation
	long param;        // Optional event parameter
	long param2;
	long param3;
	long param4;
	long param5;
	long param6;
	void *data;        // Optional event data
	long time;         // Elapsed time until event
	long duration;     // Duration of event
	long d_reserved;

	Event() {
		memset(this, 0, sizeof(*this));
	}
};

typedef Common::List<Event> EventColumns;

typedef Common::List<EventColumns> EventList;

#define EVENT_WARNINGCOUNT 1000
#define EVENT_MASK 0x00FF

enum EventStatusCode {
	kEvStInvalidCode = 0,
	kEvStDelete,
	kEvStContinue,
	kEvStBreak
};

class Events {
 public:
	Events(SagaEngine *vm);
	~Events();
	void handleEvents(long msec);
	void clearList(bool playQueuedMusic = true);
	void freeList();

	// Schedules an event in the event list; returns a pointer to the scheduled
	// event columns suitable for chaining if desired.
	EventColumns *queue(const Event &event) {
		return chain(NULL, event);
	}

	// Places a 'event' on the end of an event columns given by 'eventColumns'
	EventColumns *chain(EventColumns *eventColumns, const Event &event);

 private:
	int handleContinuous(Event *event);
	int handleOneShot(Event *event);
	int handleInterval(Event *event);
	int handleImmediate(Event *event);
	void processEventTime(long msec);
	void initializeEvent(Event &event);

 private:
	SagaEngine *_vm;

	EventList _eventList;
};

} // End of namespace Saga

#endif
