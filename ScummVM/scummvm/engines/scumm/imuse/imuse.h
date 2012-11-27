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

#ifndef SCUMM_IMUSE_H
#define SCUMM_IMUSE_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "scumm/music.h"

class MidiDriver;
class OSystem;

namespace Scumm {

class IMuseInternal;
class Player;
class ScummEngine;
class Serializer;

typedef void (*sysexfunc)(Player *, const byte *, uint16);

/**
 * iMuse implementation interface.
 * MusicEngine derivative for state-tracked, interactive,
 * persistent event-based music playback and control.
 * This class serves as an interface to actual implementations
 * so that client code is not exposed to the details of
 * any specific implementation.
 */
class IMuse : public MusicEngine {
public:
	enum {
		PROP_TEMPO_BASE,
		PROP_NATIVE_MT32,
		PROP_GS,
		PROP_LIMIT_PLAYERS,
		PROP_RECYCLE_PLAYERS,
		PROP_GAME_ID,
		PROP_PC_SPEAKER
	};

public:
	virtual void on_timer(MidiDriver *midi) = 0;
	virtual void pause(bool paused) = 0;
	virtual int save_or_load(Serializer *ser, ScummEngine *scumm) = 0;
	virtual bool get_sound_active(int sound) const = 0;
	virtual int32 doCommand(int numargs, int args[]) = 0;
	virtual int clear_queue() = 0;
	virtual uint32 property(int prop, uint32 value) = 0;
	virtual void addSysexHandler(byte mfgID, sysexfunc handler) = 0;

public:
	virtual void startSoundWithNoteOffset(int sound, int offset) = 0;

	// MusicEngine base class methods.
	// Not actually redefined here because none are implemented.

public:
	// Factory methods
	static IMuse *create(OSystem *syst, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver);
};

} // End of namespace Scumm

#endif
