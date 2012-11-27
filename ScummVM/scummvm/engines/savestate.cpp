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

#include "engines/savestate.h"
#include "graphics/surface.h"
#include "common/textconsole.h"

SaveStateDescriptor::SaveStateDescriptor()
	// FIXME: default to 0 (first slot) or to -1 (invalid slot) ?
	: _slot(-1), _description(), _isDeletable(true), _isWriteProtected(false),
	  _saveDate(), _saveTime(), _playTime(), _thumbnail() {
}

SaveStateDescriptor::SaveStateDescriptor(int s, const Common::String &d)
	: _slot(s), _description(d), _isDeletable(true), _isWriteProtected(false),
	  _saveDate(), _saveTime(), _playTime(), _thumbnail() {
}

void SaveStateDescriptor::setThumbnail(Graphics::Surface *t) {
	if (_thumbnail.get() == t)
		return;

	_thumbnail = Common::SharedPtr<Graphics::Surface>(t, Graphics::SharedPtrSurfaceDeleter());
}

void SaveStateDescriptor::setSaveDate(int year, int month, int day) {
	_saveDate = Common::String::format("%.2d.%.2d.%.4d", day, month, year);
}

void SaveStateDescriptor::setSaveTime(int hour, int min) {
	_saveTime = Common::String::format("%.2d:%.2d", hour, min);
}

void SaveStateDescriptor::setPlayTime(int hours, int minutes) {
	_playTime = Common::String::format("%.2d:%.2d", hours, minutes);
}

void SaveStateDescriptor::setPlayTime(uint32 msecs) {
	uint minutes = msecs / 60000;
	setPlayTime(minutes / 60, minutes % 60);
}
