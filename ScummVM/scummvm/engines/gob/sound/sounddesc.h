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

#ifndef GOB_SOUND_SOUNDDESC_H
#define GOB_SOUND_SOUNDDESC_H

#include "common/endian.h"

namespace Gob {

class Resource;

enum SoundType {
	SOUND_SND,
	SOUND_WAV,
	SOUND_ADL
};

class SoundDesc {
public:
	int16 _repCount;
	int16 _frequency;
	int16 _flag;
	int16 _id;
	byte _mixerFlags;

	SoundDesc();
	~SoundDesc();

	void swap(SoundDesc &desc);

	byte *getData() { return _dataPtr; }

	uint32    size()         const { return _size;     }
	bool      empty()        const { return !_dataPtr; }
	SoundType getType()      const { return _type;     }

	bool isId(int16 id) const { return _dataPtr && (_id == id); }

	void set(SoundType type, byte *data, uint32 dSize);
	void set(SoundType type, Resource *resource);
	bool load(SoundType type, byte *data, uint32 dSize);
	bool load(SoundType type, Resource *resource);

	void free();
	void convToSigned();

	// Which fade out length to use when the fade starts half-way through?
	int16 calcFadeOutLength(int16 frequency);
	uint32 calcLength(int16 repCount, int16 frequency, bool fade);

private:
	Resource *_resource;
	byte *_data;
	byte *_dataPtr;
	uint32 _size;

	SoundType _type;

	bool loadSND(byte *data, uint32 dSize);
	bool loadWAV(byte *data, uint32 dSize);
	bool loadADL(byte *data, uint32 dSize);
};

} // End of namespace Gob

#endif // GOB_SOUND_SOUNDDESC_H
