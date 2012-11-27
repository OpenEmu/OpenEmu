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

// Sound resource class header

#ifndef SAGA_SNDRES_H
#define SAGA_SNDRES_H

#include "saga/itedata.h"
#include "saga/sound.h"

namespace Saga {

struct FxTable {
	int16 res;
	int16 vol;
};

class SndRes {
public:

	SndRes(SagaEngine *vm);

	void playSound(uint32 resourceId, int volume, bool loop);
	void playVoice(uint32 resourceId);
	int getVoiceLength(uint32 resourceId);
	void setVoiceBank(int serial);

	Common::Array<FxTable> _fxTable;

	Common::Array<int16> _fxTableIDs;

 private:
	bool load(ResourceContext *context, uint32 resourceId, SoundBuffer &buffer, bool onlyHeader);

	ResourceContext *_sfxContext;
	ResourceContext *_voiceContext;

	int _voiceSerial; // voice bank number

	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
