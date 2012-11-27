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

#ifndef COMMON_EVENTS_H
#define COMMON_EVENTS_H

#include "common/keyboard.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/noncopyable.h"

#include "common/list.h"
#include "common/singleton.h"

namespace Common {

/**
 * The types of events backends may generate.
 * @see Event
 *
 * @todo Merge EVENT_LBUTTONDOWN, EVENT_RBUTTONDOWN and EVENT_WHEELDOWN;
 *       likewise EVENT_LBUTTONUP, EVENT_RBUTTONUP, EVENT_WHEELUP.
 *       To do that, we just have to add a field to the Event which
 *       indicates which button was pressed.
 */
enum EventType {
	EVENT_INVALID = 0,
	/** A key was pressed, details in Event::kbd. */
	EVENT_KEYDOWN = 1,
	/** A key was released, details in Event::kbd. */
	EVENT_KEYUP = 2,
	/** The mouse moved, details in Event::mouse. */
	EVENT_MOUSEMOVE = 3,
	EVENT_LBUTTONDOWN = 4,
	EVENT_LBUTTONUP = 5,
	EVENT_RBUTTONDOWN = 6,
	EVENT_RBUTTONUP = 7,
	EVENT_WHEELUP = 8,
	EVENT_WHEELDOWN = 9,
	EVENT_MBUTTONDOWN = 13,
	EVENT_MBUTTONUP = 14,

	EVENT_MAINMENU = 15,
	EVENT_RTL = 16,
	EVENT_MUTE = 17,

	EVENT_QUIT = 10,
	EVENT_SCREEN_CHANGED = 11,
	/**
	 * The backend requests the agi engine's predictive dialog to be shown.
	 * TODO: Fingolfin suggests that it would be of better value to expand
	 * on this notion by generalizing its use. For example the backend could
	 * use events to ask for the save game dialog or to pause the engine.
	 * An associated enumerated type can accomplish this.
	 **/
	EVENT_PREDICTIVE_DIALOG = 12

#ifdef ENABLE_KEYMAPPER
	,
	// IMPORTANT NOTE: This is part of the WIP Keymapper. If you plan to use
	// this, please talk to tsoliman and/or LordHoto.
	EVENT_CUSTOM_BACKEND_ACTION = 18,
	EVENT_CUSTOM_BACKEND_HARDWARE = 21,
	EVENT_GUI_REMAP_COMPLETE_ACTION = 22,
	EVENT_KEYMAPPER_REMAP = 19
#endif
#ifdef ENABLE_VKEYBD
	,
	EVENT_VIRTUAL_KEYBOARD = 20
#endif
};

typedef uint32 CustomEventType;
/**
 * Data structure for an event. A pointer to an instance of Event
 * can be passed to pollEvent.
 */
struct Event {
	/** The type of the event. */
	EventType type;
	/** Flag to indicate if the event is real or synthetic. E.g. keyboard
	  * repeat events are synthetic.
	  */
	bool synthetic;
	/**
	  * Keyboard data; only valid for keyboard events (EVENT_KEYDOWN and
	  * EVENT_KEYUP). For all other event types, content is undefined.
	  */
	KeyState kbd;
	/**
	 * The mouse coordinates, in virtual screen coordinates. Only valid
	 * for mouse events.
	 * Virtual screen coordinates means: the coordinate system of the
	 * screen area as defined by the most recent call to initSize().
	 */
	Point mouse;

#ifdef ENABLE_KEYMAPPER
	// IMPORTANT NOTE: This is part of the WIP Keymapper. If you plan to use
	// this, please talk to tsoliman and/or LordHoto.
	CustomEventType customType;
#endif

	Event() : type(EVENT_INVALID), synthetic(false) {
#ifdef ENABLE_KEYMAPPER
		customType = 0;
#endif
	}
};

/**
 * A source of Events.
 *
 * An example for this is OSystem, it provides events created by the system
 * and or user.
 */
class EventSource {
public:
	virtual ~EventSource() {}

	/**
	 * Queries a event from the source.
	 *
	 * @param	event 	a reference to the event struct, where the event should be stored.
	 * @return	true if an event was polled, false otherwise.
	 */
	virtual bool pollEvent(Event &event) = 0;

	/**
	 * Checks whether events from this source are allowed to be mapped.
	 *
	 * Possible event sources not allowing mapping are: the event recorder/player and/or
	 * the EventManager, which allows user events to be pushed.
	 *
	 * By default we allow mapping for every event source.
	 */
	virtual bool allowMapping() const { return true; }
};

/**
 * An artificial event source. This is class is used as an event source, which is
 * made up by client specific events.
 *
 * Example usage cases for this are the Keymapper or the DefaultEventManager.
 */
class ArtificialEventSource : public EventSource {
protected:
	Queue<Event> _artificialEventQueue;
public:
	void addEvent(const Event &ev) {
		_artificialEventQueue.push(ev);
	}

	bool pollEvent(Event &ev) {
	if (!_artificialEventQueue.empty()) {
			ev = _artificialEventQueue.pop();
			return true;
		} else {
			return false;
		}
	}

	/**
	 * By default an artificial event source prevents its events
	 * from being mapped.
	 */
	virtual bool allowMapping() const { return false; }
};

/**
 * Object which catches and processes Events.
 *
 * An example for this is the Engine object, it is catching events and processing them.
 */
class EventObserver {
public:
	virtual ~EventObserver() {}

	/**
	 * Notifies the observer of an incoming event.
	 *
	 * An observer is supposed to eat the event, with returning true, when
	 * it wants to prevent other observers from receiving the event.
	 * A usage example here is the keymapper:
	 * If it processes an Event, it should 'eat' it and create a new
	 * event, which the EventDispatcher will then catch.
	 *
	 * @param   event   the event, which is incoming.
	 * @return  true if the event should not be passed to other observers,
	 *          false otherwise.
	 */
	virtual bool notifyEvent(const Event &event) = 0;

	/**
	 * Notifies the observer of pollEvent() query.
	 *
	 * @return  true if the event should not be passed to other observers,
	 *          false otherwise.
	 */
	virtual bool notifyPoll() { return false; }
};

/**
 * A event mapper, which will map events to others.
 *
 * An example for this is the Keymapper.
 */
class EventMapper {
public:
	virtual ~EventMapper() {}

	/**
	 * Map an incoming event to one or more action events
	 */
	virtual List<Event> mapEvent(const Event &ev, EventSource *source) = 0;

	virtual List<Event> getDelayedEvents() = 0;
};

class DefaultEventMapper : public EventMapper {
public:
	DefaultEventMapper() : _delayedEvents(), _delayedEffectiveTime(0) {}
	// EventMapper interface
	virtual List<Event> mapEvent(const Event &ev, EventSource *source);
	virtual List<Event> getDelayedEvents();
protected:
	virtual void addDelayedEvent(uint32 millis, Event ev);

	struct DelayedEventsEntry {
		const uint32 timerOffset;
		const Event event;
		DelayedEventsEntry(const uint32 offset, const Event ev) : timerOffset(offset), event(ev) { }
	};

	Queue<DelayedEventsEntry> _delayedEvents;
	uint32 _delayedEffectiveTime;
};

/**
 * Dispatches events from various sources to various observers.
 *
 * EventDispatcher is using a priority based approach. Observers
 * with higher priority will be notified before observers with
 * lower priority. Because of the possibility that oberservers
 * might 'eat' events, not all observers might be notified.
 *
 * Another speciality is the support for a event mapper, which
 * will catch events and create new events out of them. This
 * mapper will be processed before an event is sent to the
 * observers.
 */
class EventDispatcher {
public:
	EventDispatcher();
	~EventDispatcher();

	/**
	 * Tries to catch events from the registered event
	 * sources and dispatch them to the observers.
	 *
	 * This dispatches *all* events the sources offer.
	 */
	void dispatch();

	/**
	 * Registers an event mapper with the dispatcher.
	 *
	 * The ownership of the "mapper" variable will pass
	 * to the EventDispatcher, thus it will be deleted
	 * with "delete", when EventDispatcher is destroyed.
	 *
	 * Note there is only one mapper per EventDispatcher
	 * possible, thus when this method is called twice,
	 * the former mapper will be destroied.
	 */
	void registerMapper(EventMapper *mapper);

	/**
	 * Queries the setup event mapper.
	 */
	EventMapper *queryMapper() const { return _mapper; }

	/**
	 * Registers a new EventSource with the Dispatcher.
	 */
	void registerSource(EventSource *source, bool autoFree);

	/**
	 * Unregisters a EventSource.
	 *
	 * This takes the "autoFree" flag passed to registerSource into account.
	 */
	void unregisterSource(EventSource *source);

	/**
	 * Registers a new EventObserver with the Dispatcher.
	 *
	 * @param listenPools if set, then all pollEvent() calls are passed to observer
	 *                    currently it is used by keyMapper
	 */
	void registerObserver(EventObserver *obs, uint priority, bool autoFree, bool listenPolls = false);

	/**
	 * Unregisters a EventObserver.
	 *
	 * This takes the "autoFree" flag passed to registerObserver into account.
	 */
	void unregisterObserver(EventObserver *obs);
private:
	EventMapper *_mapper;

	struct Entry {
		bool autoFree;
	};

	struct SourceEntry : public Entry {
		EventSource *source;
	};

	List<SourceEntry> _sources;

	struct ObserverEntry : public Entry {
		uint priority;
		EventObserver *observer;
		bool poll;
	};

	List<ObserverEntry> _observers;

	void dispatchEvent(const Event &event);
	void dispatchPoll();
};

class Keymapper;

/**
 * The EventManager provides user input events to the client code.
 * In addition, it keeps track of the state of various input devices,
 * like keys, mouse position and buttons.
 */
class EventManager : NonCopyable {
public:
	EventManager() {}
	virtual ~EventManager() {}

	enum {
		LBUTTON = 1 << 0,
		RBUTTON = 1 << 1
	};


	/**
	 * Initialize the event manager.
	 * @note	called after graphics system has been set up
	 */
	virtual void init() {}
	/**
	 * Get the next event in the event queue.
	 * @param event	point to an Event struct, which will be filled with the event data.
	 * @return true if an event was retrieved.
	 */
	virtual bool pollEvent(Event &event) = 0;

	/**
	 * Pushes a "fake" event into the event queue
	 */
	virtual void pushEvent(const Event &event) = 0;

	/** Return the current mouse position */
	virtual Point getMousePos() const = 0;

	/**
	 * Return a bitmask with the button states:
	 * - bit 0: left button up=0, down=1
	 * - bit 1: right button up=0, down=1
	 */
	virtual int getButtonState() const = 0;

	/** Get a bitmask with the current modifier state */
	virtual int getModifierState() const = 0;

	/**
	 * Should the application terminate? Set to true if we
	 * received an EVENT_QUIT.
	 */
	virtual int shouldQuit() const = 0;

	/**
	 * Should we return to the launcher?
	 */
	virtual int shouldRTL() const = 0;

	/**
	 * Reset the "return to launcher" flag (as returned shouldRTL()) to false.
	 * Used when we have returned to the launcher.
	 */
	virtual void resetRTL() = 0;
#ifdef FORCE_RTL
	virtual void resetQuit() = 0;
#endif
	// Optional: check whether a given key is currently pressed ????
	//virtual bool isKeyPressed(int keycode) = 0;

	// TODO: Keyboard repeat support?

	// TODO: Consider removing OSystem::getScreenChangeID and
	// replacing it by a generic getScreenChangeID method here
#ifdef ENABLE_KEYMAPPER
	virtual Keymapper *getKeymapper() = 0;
#endif

	enum {
		/**
		 * Priority of the event manager, for now it's lowest since it eats
		 * *all* events, we might to change that in the future though.
		 */
		kEventManPriority = 0,
		/**
		 * Priority of the event recorder. It has to go after event manager
		 * in order to record events generated by it
		 */
		kEventRecorderPriority = 1
	};

	/**
	 * Returns the underlying EventDispatcher.
	 */
	EventDispatcher *getEventDispatcher() { return &_dispatcher; }

protected:
	EventDispatcher _dispatcher;
};

} // End of namespace Common

#endif
