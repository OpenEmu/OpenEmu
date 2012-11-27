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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKEND_UNITY_TASKBAR_H
#define BACKEND_UNITY_TASKBAR_H

#if defined(POSIX) && defined(USE_TASKBAR) && defined(USE_TASKBAR_UNITY)

#include "common/events.h"
#include "common/str.h"
#include "common/taskbar.h"

typedef struct _GMainLoop GMainLoop;
typedef struct _UnityLauncherEntry UnityLauncherEntry;

class UnityTaskbarManager : public Common::TaskbarManager, public Common::EventSource {
public:
	UnityTaskbarManager();
	virtual ~UnityTaskbarManager();

	virtual void setProgressValue(int completed, int total);
	virtual void setProgressState(TaskbarProgressState state);
	virtual void addRecent(const Common::String &name, const Common::String &description);
	virtual void setCount(int count);

	// Implementation of the EventSource interface
        virtual bool pollEvent(Common::Event &event);

private:
	GMainLoop          *_loop;
	UnityLauncherEntry *_launcher;
};

#endif

#endif // BACKEND_UNITY_TASKBAR_H
