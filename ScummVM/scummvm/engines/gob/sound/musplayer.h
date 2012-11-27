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

#ifndef GOB_SOUND_MUSPLAYER_H
#define GOB_SOUND_MUSPLAYER_H

#include "common/str.h"
#include "common/array.h"

#include "gob/sound/adlib.h"

namespace Common {
	class SeekableReadStream;
}

namespace Gob {

/** A player for the AdLib MUS format, with the instrument information in SND files.
 *
 *  In the Gob engine, those files are usually named .MDY and .TBR instead.
 */
class MUSPlayer : public AdLib {
public:
	MUSPlayer(Audio::Mixer &mixer);
	~MUSPlayer();

	/** Load the instruments (.SND or .TBR) */
	bool loadSND(Common::SeekableReadStream &snd);
	/** Load the melody (.MUS or .MDY) */
	bool loadMUS(Common::SeekableReadStream &mus);

	void unload();

	uint32 getSongID() const;
	const Common::String &getSongName() const;

protected:
	// AdLib interface
	uint32 pollMusic(bool first);
	void rewind();

private:
	struct Timbre {
		Common::String name;

		uint16 params[kOperatorsPerVoice * kParamCount];
	};

	Common::Array<Timbre> _timbres;

	byte  *_songData;
	uint32 _songDataSize;

	const byte *_playPos;

	uint32 _songID;
	Common::String _songName;

	uint8 _ticksPerBeat;
	uint8 _beatsPerMeasure;

	uint8 _soundMode;
	uint8 _pitchBendRange;

	uint16 _baseTempo;

	uint16 _tempo;

	byte _lastCommand;


	void unloadSND();
	void unloadMUS();

	bool readSNDHeader (Common::SeekableReadStream &snd, int &timbreCount, int &timbrePos);
	bool readSNDTimbres(Common::SeekableReadStream &snd, int  timbreCount, int  timbrePos);

	bool readMUSHeader(Common::SeekableReadStream &mus);
	bool readMUSSong  (Common::SeekableReadStream &mus);

	uint32 getSampleDelay(uint16 delay) const;
	void setInstrument(uint8 voice, uint8 instrument);
	void skipToTiming();

	static bool readString(Common::SeekableReadStream &stream, Common::String &string, byte *buffer, uint size);
};

} // End of namespace Gob

#endif // GOB_SOUND_MUSPLAYER_H
