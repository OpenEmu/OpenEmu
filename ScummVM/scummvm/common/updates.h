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

#ifndef BACKENDS_UPDATES_ABSTRACT_H
#define BACKENDS_UPDATES_ABSTRACT_H

#if defined(USE_UPDATES)

namespace Common {

/**
 * The UpdateManager allows configuring of the automatic update checking
 * for systems that support it:
 *  - using Sparkle on Mac OS X
 *  - using WinSparkle on Windows
 *
 * Most of the update checking is completely automated and this class only
 * gives access to basic settings. It is mostly used by the GUI to set
 * widgets state on the update page and for manually checking for updates
 *
 */
class UpdateManager {
public:
	enum UpdateState {
		kUpdateStateDisabled     = 0,
		kUpdateStateEnabled      = 1,
		kUpdateStateNotSupported = 2
	};

	enum UpdateInterval {
		kUpdateIntervalNotSupported = 0,
		kUpdateIntervalOneDay       = 86400,
		kUpdateIntervalOneWeek      = 604800,
		kUpdateIntervalOneMonth     = 2628000   // average seconds per month (60*60*24*365)/12
	};

	UpdateManager() {}
	virtual ~UpdateManager() {}

	/**
	 * Checks manually if an update is available, showing progress UI to the user.
	 *
	 * By default, update checks are done silently on start.
	 * This allows to manually start an update check.
	 */
	virtual void checkForUpdates() {}

	/**
	 * Sets the automatic update checking state
	 *
	 * @param  state    The state.
	 */
	virtual void setAutomaticallyChecksForUpdates(UpdateState state) {}

	/**
	 * Gets the automatic update checking state
	 *
	 * @return  kUpdateStateDisabled     if automatic update checking is disabled,
	 *          kUpdateStateEnabled      if automatic update checking is enabled,
	 *          kUpdateStateNotSupported if automatic update checking is not available
	 */
	virtual UpdateState getAutomaticallyChecksForUpdates() { return kUpdateStateNotSupported; }

	/**
	 * Sets the update checking interval.
	 *
	 * @param  interval    The interval.
	 */
	virtual void setUpdateCheckInterval(UpdateInterval interval) {}

	/**
	 * Gets the update check interval.
	 *
	 * @return  the update check interval.
	 */
	virtual UpdateInterval getUpdateCheckInterval() { return kUpdateIntervalNotSupported; }
};

}	// End of namespace Common

#endif

#endif // BACKENDS_UPDATES_ABSTRACT_H
