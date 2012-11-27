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

#ifndef BACKEND_WIN32_TASKBAR_H
#define BACKEND_WIN32_TASKBAR_H

#if defined(WIN32) && defined(USE_TASKBAR)

#include "common/str.h"
#include "common/taskbar.h"

struct ITaskbarList3;

class Win32TaskbarManager : public Common::TaskbarManager {
public:
	Win32TaskbarManager();
	virtual ~Win32TaskbarManager();

	virtual void setOverlayIcon(const Common::String &name, const Common::String &description);
	virtual void setProgressValue(int completed, int total);
	virtual void setProgressState(TaskbarProgressState state);
	virtual void setCount(int count);
	virtual void addRecent(const Common::String &name, const Common::String &description);
	virtual void notifyError();
	virtual void clearError();

private:
	ITaskbarList3 *_taskbar;

	// Count handling
	HICON _icon;
	int   _count;

	/**
	 * 	Get the path to an icon for the game
	 *
	 * @param	target	The game target
	 *
	 * @return	The icon path (or "" if no icon was found)
	 */
	Common::String getIconPath(Common::String target);

	// Helper functions
	bool isWin7OrLater();
	LPWSTR ansiToUnicode(const char *s);
	HWND getHwnd();
};

#endif

#endif // BACKEND_WIN32_TASKBAR_H
