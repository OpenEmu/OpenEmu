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

// Music class

#ifndef MADE_MUSIC_H
#define MADE_MUSIC_H

#include "audio/midiplayer.h"

namespace Made {

class GenericResource;

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 1
};

class MusicPlayer : public Audio::MidiPlayer {
public:
	MusicPlayer();

	void playXMIDI(GenericResource *midiResource, MusicFlags flags = MUSIC_NORMAL);
	void playSMF(GenericResource *midiResource, MusicFlags flags = MUSIC_NORMAL);
//	void stop();
	void pause();
	void resume();

	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);

protected:
	bool _isGM;
};

} // End of namespace Made

#endif
