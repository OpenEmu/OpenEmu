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

#ifndef BACKENDS_BASE_BACKEND_H
#define BACKENDS_BASE_BACKEND_H

#include "common/system.h"
#include "common/events.h"

class BaseBackend : public OSystem {
protected:
	virtual Common::EventSource *getDefaultEventSource() = 0;
public:
	virtual void initBackend();

	virtual void displayMessageOnOSD(const char *msg);
	virtual void fillScreen(uint32 col);
};

class EventsBaseBackend : public BaseBackend, Common::EventSource {
protected:
	virtual Common::EventSource *getDefaultEventSource() { return this; }
public:
};


#endif
