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

#ifndef LASTEXPRESS_EVENTHANDLER_H
#define LASTEXPRESS_EVENTHANDLER_H

#include "common/func.h"
#include "common/events.h"

namespace LastExpress {

#define SET_EVENT_HANDLERS(class, inst) \
	_engine->setEventHandlers(new EVENT_HANDLER(class, eventMouse, inst), new EVENT_HANDLER(class, eventTick, inst));

#define EVENT_HANDLER(class, name, inst) \
	Common::Functor1Mem<const Common::Event&, void, class>(inst, &class::name)

class EventHandler {
public:
	virtual ~EventHandler() {}

	// Function pointer for event handler
	typedef Common::Functor1<const Common::Event&, void> EventFunction;

	virtual void eventMouse(const Common::Event &ev) {}    // Event type 1
	virtual void eventTick(const Common::Event &ev) {}     // Event type 3
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_EVENTHANDLER_H
