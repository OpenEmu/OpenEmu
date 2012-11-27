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

#ifndef TOUCHE_MIDI_H
#define TOUCHE_MIDI_H

#include "common/util.h"
#include "common/mutex.h"

#include "audio/midiplayer.h"

class MidiParser;

namespace Common {
class ReadStream;
}

namespace Touche {

class MidiPlayer : public Audio::MidiPlayer {
public:
	MidiPlayer();

	void play(Common::ReadStream &stream, int size, bool loop = false);
	void adjustVolume(int diff);
	void setVolume(int volume);

	// MidiDriver_BASE interface
	virtual void send(uint32 b);
};

} // namespace Touche

#endif
