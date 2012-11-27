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

#ifndef COMMON_TASKBAR_MANAGER_H
#define COMMON_TASKBAR_MANAGER_H

#include "common/scummsys.h"
#include "common/str.h"

#if defined(USE_TASKBAR)

namespace Common {

/**
 * The TaskbarManager allows interaction with the ScummVM application icon:
 *  - in the taskbar on Windows 7 and later
 *  - in the launcher for Unity
 *  - in the dock on Mac OS X
 *  - ...
 *
 * This allows GUI code and engines to display a progress bar, an overlay icon and/or count
 * associated with the ScummVM icon as well as add the started engine to the recent items
 * list (so that the user can start the engine directly in one click).
 *
 * Examples of use:
 *  - Track search progress and found engines when running the Mass Add dialog
 *  - Add an entry to the recent items when starting an engine
 *  - Show the current running engine icon as an overlay
 *
 * @note functionality will vary between supported platforms (due to API limitations)
 *       and some of the methods will just be no-ops or approximate the functionality
 *       as best as possible
 */
class TaskbarManager {
public:
	/**
	 * Values representing the taskbar progress state
	 */
	enum TaskbarProgressState {
		kTaskbarNoProgress = 0,
		kTaskbarIndeterminate = 1,
		kTaskbarNormal = 2,
		kTaskbarError = 4,
		kTaskbarPaused = 8
	};

	TaskbarManager() {}
	virtual ~TaskbarManager() {}

	/**
	 * Sets an overlay icon on the taskbar icon
	 *
	 * When an empty name is given, no icon is shown
	 * and the current overlay icon (if any) is removed
	 *
	 * @param  name         Path to the icon
	 * @param  description  The description
	 *
	 * @note on Windows, the icon should be an ICO file
	 */
	virtual void setOverlayIcon(const String &name, const String &description) {}

	/**
	 * Sets a progress value on the taskbar icon
	 *
	 * @param  completed  The current progress value.
	 * @param  total      The maximum progress value.
	 */
	virtual void setProgressValue(int completed, int total) {}

	/**
	 * Sets the progress state on the taskbar icon
	 *
	 * State can be any of the following:
	 *   - NoProgress: disable display of progress state
	 *   - Indeterminate
	 *   - Normal
	 *   - Error
	 *   - Paused
	 *
	 * @param  state    The progress state
	 */
	virtual void setProgressState(TaskbarProgressState state) {}

	/**
	 * Sets the count number associated with the icon as an overlay
	 *
	 * @param  count    The count
	 *
	 * @note Setting a count of 0 will hide the count
	 */
	virtual void setCount(int count) {}

	/**
	 * Adds an engine to the recent items list
	 *
	 * Path is automatically set to the current executable path,
	 * an icon name is generated (with fallback to default icon)
	 * and the command line is set to start the engine on click.
	 *
	 * @param  name         The target name.
	 * @param  description  The description.
	 */
	virtual void addRecent(const String &name, const String &description) {}

	/**
	 * Notifies the user an error occured through the taskbar icon
	 *
	 * This will for example show the taskbar icon as red (using progress of 100% and an error state)
	 * on Windows, and set the launcher icon in the urgent state on Unity
	 */
	virtual void notifyError() {}

	/**
	 * Clears the error notification
	 */
	virtual void clearError() {}
};

}	// End of namespace Common

#endif

#endif // COMMON_TASKBAR_MANAGER_H
