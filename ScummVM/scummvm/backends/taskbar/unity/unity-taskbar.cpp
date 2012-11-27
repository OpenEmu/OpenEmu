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

#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#include "common/scummsys.h"

#if defined(POSIX) && defined(USE_TASKBAR) && defined(USE_TASKBAR_UNITY)

#include "backends/taskbar/unity/unity-taskbar.h"

#include "common/textconsole.h"

#include <unity.h>

UnityTaskbarManager::UnityTaskbarManager() {
	g_type_init();

	_loop = g_main_loop_new(NULL, FALSE);

	_launcher = unity_launcher_entry_get_for_desktop_id("scummvm.desktop");
}

UnityTaskbarManager::~UnityTaskbarManager() {
	g_main_loop_unref(_loop);
	_loop = NULL;
}

void UnityTaskbarManager::setProgressValue(int completed, int total) {
	if (_launcher == NULL)
		return;

	double percentage = (double)completed / (double)total;
	unity_launcher_entry_set_progress(_launcher, percentage);
	unity_launcher_entry_set_progress_visible(_launcher, TRUE);
}

void UnityTaskbarManager::setProgressState(TaskbarProgressState state) {
	if (_launcher == NULL)
		return;

	switch (state) {
	default:
		warning("[UnityTaskbarManager::setProgressState] Unknown state / Not implemented (%d)", state);
		// fallback to noprogress state

	case kTaskbarNoProgress:
		unity_launcher_entry_set_progress_visible(_launcher, FALSE);
		break;

	// Unity only support two progress states as of 3.0: visible or not visible
	// We show progress in all of those states
	case kTaskbarIndeterminate:
	case kTaskbarNormal:
	case kTaskbarError:
	case kTaskbarPaused:
		unity_launcher_entry_set_progress_visible(_launcher, TRUE);
		break;
	}
}

void UnityTaskbarManager::addRecent(const Common::String &name, const Common::String &description) {
	warning("[UnityTaskbarManager::addRecent] Not implemented");
}

void UnityTaskbarManager::setCount(int count) {
	if (_launcher == NULL)
		return;

	unity_launcher_entry_set_count(_launcher, count);

	unity_launcher_entry_set_count_visible(_launcher, (count == 0) ? FALSE : TRUE);
}

// Unity requires the glib event loop to the run to function properly
// as events are sent asynchronously
bool UnityTaskbarManager::pollEvent(Common::Event &event) {
	if (!_loop)
		return false;

	// Get context
	GMainContext *context = g_main_loop_get_context(_loop);
	if (!context)
		return false;

	// Dispatch events
	g_main_context_iteration(context, FALSE);

	return false;
}

#endif
