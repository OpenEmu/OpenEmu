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

#ifndef ENGINES_SAVESTATE_H
#define ENGINES_SAVESTATE_H

#include "common/array.h"
#include "common/str.h"
#include "common/ptr.h"


namespace Graphics {
struct Surface;
}

/**
 * Object describing a save state.
 *
 * This at least includes the save slot number and a human readable
 * description of the save state.
 *
 * Further possibilites are a thumbnail, play time, creation date,
 * creation time, delete protected, write protection.
 *
 * Saves are writable and deletable by default.
 */
class SaveStateDescriptor {
public:
	SaveStateDescriptor();
	SaveStateDescriptor(int s, const Common::String &d);

	/**
	 * @param slot The saveslot id, as it would be passed to the "-x" command line switch.
	 */
	void setSaveSlot(int slot) { _slot = slot; }

	/**
	 * @return The saveslot id, as it would be passed to the "-x" command line switch.
	 */
	int getSaveSlot() const { return _slot; }

	/**
	 * @param desc A human readable description of the save state.
	 */
	void setDescription(const Common::String &desc) { _description = desc; }

	/**
	 * @return A human readable description of the save state.
	 */
	const Common::String &getDescription() const { return _description; }

	/** Optional entries only included when querying via MetaEngine::querySaveMetaInfo */

	/**
	 * Defines whether the save state is allowed to be deleted.
	 */
	void setDeletableFlag(bool state) { _isDeletable = state; }

	/**
	 * Queries whether the save state is allowed to be deleted.
	 */
	bool getDeletableFlag() const { return _isDeletable; }

	/**
	 * Defines whether the save state is write protected.
	 */
	void setWriteProtectedFlag(bool state) { _isWriteProtected = state; }

	/**
	 * Queries whether the save state is write protected.
	 */
	bool getWriteProtectedFlag() const { return _isWriteProtected; }

	/**
	 * Return a thumbnail graphics surface representing the savestate visually.
	 * This is usually a scaled down version of the game graphics. The size
	 * should be either 160x100 or 160x120 pixels, depending on the aspect
	 * ratio of the game. If another ratio is required, contact the core team.
	 */
	const Graphics::Surface *getThumbnail() const { return _thumbnail.get(); }

	/**
	 * Set a thumbnail graphics surface representing the savestate visually.
	 * Ownership of the surface is transferred to the SaveStateDescriptor.
	 * Hence the caller must not delete the surface.
	 */
	void setThumbnail(Graphics::Surface *t);

	/**
	 * Sets the date the save state was created.
	 *
	 * @param year  Year of creation.
	 * @param month Month of creation.
	 * @param day   Day of creation.
	 */
	void setSaveDate(int year, int month, int day);

	/**
	 * Queries a human readable description of the date the save state was created.
	 *
	 * This will return an empty string in case the value is not set.
	 */
	const Common::String &getSaveDate() const { return _saveDate; }

	/**
	 * Sets the time the save state was created.
	 *
	 * @param hour  Hour of creation.
	 * @param min   Minute of creation.
	 */
	void setSaveTime(int hour, int min);

	/**
	 * Queries a human readable description of the time the save state was created.
	 *
	 * This will return an empty string in case the value is not set.
	 */
	const Common::String &getSaveTime() const { return _saveTime; }

	/**
	 * Sets the time the game was played before the save state was created.
	 *
	 * @param hours How many hours the user played the game so far.
	 * @param min   How many minutes the user played the game so far.
	 */
	void setPlayTime(int hours, int minutes);

	/**
	 * Sets the time the game was played before the save state was created.
	 *
	 * @param msecs How many milliseconds the user played the game so far.
	 */
	void setPlayTime(uint32 msecs);

	/**
	 * Queries a human readable description of the time the game was played
	 * before the save state was created.
	 *
	 * This will return an empty string in case the value is not set.
	 */
	const Common::String &getPlayTime() const { return _playTime; }

private:
	/**
	 * The saveslot id, as it would be passed to the "-x" command line switch.
	 */
	int _slot;

	/**
	 * A human readable description of the save state.
	 */
	Common::String _description;

	/**
	 * Whether the save state can be deleted.
	 */
	bool _isDeletable;

	/**
	 * Whether the save state is write protected.
	 */
	bool _isWriteProtected;

	/**
	 * Human readable description of the date the save state was created.
	 */
	Common::String _saveDate;

	/**
	 * Human readable description of the time the save state was created.
	 */
	Common::String _saveTime;

	/**
	 * Human readable description of the time the game was played till the
	 * save state was created.
	 */
	Common::String _playTime;

	/**
	 * The thumbnail of the save state.
	 */
	Common::SharedPtr<Graphics::Surface> _thumbnail;
};

/** List of savestates. */
typedef Common::Array<SaveStateDescriptor> SaveStateList;


#endif
