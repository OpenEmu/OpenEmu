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

#include "audio/mods/paula.h"
#include "audio/null.h"

namespace Audio {

Paula::Paula(bool stereo, int rate, uint interruptFreq) :
		_stereo(stereo), _rate(rate), _periodScale((double)kPalPaulaClock / rate), _intFreq(interruptFreq) {

	clearVoices();
	_voice[0].panning = 191;
	_voice[1].panning = 63;
	_voice[2].panning = 63;
	_voice[3].panning = 191;

	if (_intFreq == 0)
		_intFreq = _rate;

	_curInt = 0;
	_timerBase = 1;
	_playing = false;
	_end = true;
}

Paula::~Paula() {
}

void Paula::clearVoice(byte voice) {
	assert(voice < NUM_VOICES);

	_voice[voice].data = 0;
	_voice[voice].dataRepeat = 0;
	_voice[voice].length = 0;
	_voice[voice].lengthRepeat = 0;
	_voice[voice].period = 0;
	_voice[voice].volume = 0;
	_voice[voice].offset = Offset(0);
	_voice[voice].dmaCount = 0;
}

int Paula::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	memset(buffer, 0, numSamples * 2);
	if (!_playing) {
		return numSamples;
	}

	if (_stereo)
		return readBufferIntern<true>(buffer, numSamples);
	else
		return readBufferIntern<false>(buffer, numSamples);
}


template<bool stereo>
inline int mixBuffer(int16 *&buf, const int8 *data, Paula::Offset &offset, frac_t rate, int neededSamples, uint bufSize, byte volume, byte panning) {
	int samples;
	for (samples = 0; samples < neededSamples && offset.int_off < bufSize; ++samples) {
		const int32 tmp = ((int32) data[offset.int_off]) * volume;
		if (stereo) {
			*buf++ += (tmp * (255 - panning)) >> 7;
			*buf++ += (tmp * (panning)) >> 7;
		} else
			*buf++ += tmp;

		// Step to next source sample
		offset.rem_off += rate;
		if (offset.rem_off >= (frac_t)FRAC_ONE) {
			offset.int_off += fracToInt(offset.rem_off);
			offset.rem_off &= FRAC_LO_MASK;
		}
	}

	return samples;
}

template<bool stereo>
int Paula::readBufferIntern(int16 *buffer, const int numSamples) {
	int samples = _stereo ? numSamples / 2 : numSamples;
	while (samples > 0) {

		// Handle 'interrupts'. This gives subclasses the chance to adjust the channel data
		// (e.g. insert new samples, do pitch bending, whatever).
		if (_curInt == 0) {
			_curInt = _intFreq;
			interrupt();
		}

		// Compute how many samples to generate: at most the requested number of samples,
		// of course, but we may stop earlier when an 'interrupt' is expected.
		const uint nSamples = MIN((uint)samples, _curInt);

		// Loop over the four channels of the emulated Paula chip
		for (int voice = 0; voice < NUM_VOICES; voice++) {
			// No data, or paused -> skip channel
			if (!_voice[voice].data || (_voice[voice].period <= 0))
				continue;

			// The Paula chip apparently run at 7.0937892 MHz in the PAL
			// version and at 7.1590905 MHz in the NTSC version. We divide this
			// by the requested the requested output sampling rate _rate
			// (typically 44.1 kHz or 22.05 kHz) obtaining the value _periodScale.
			// This is then divided by the "period" of the channel we are
			// processing, to obtain the correct output 'rate'.
			frac_t rate = doubleToFrac(_periodScale / _voice[voice].period);
			// Cap the volume
			_voice[voice].volume = MIN((byte) 0x40, _voice[voice].volume);


			Channel &ch = _voice[voice];
			int16 *p = buffer;
			int neededSamples = nSamples;
			assert(ch.offset.int_off < ch.length);

			// Mix the generated samples into the output buffer
			neededSamples -= mixBuffer<stereo>(p, ch.data, ch.offset, rate, neededSamples, ch.length, ch.volume, ch.panning);

			// Wrap around if necessary
			if (ch.offset.int_off >= ch.length) {
				// Important: Wrap around the offset *before* updating the voice length.
				// Otherwise, if length != lengthRepeat we would wrap incorrectly.
				// Note: If offset >= 2*len ever occurs, the following would be wrong;
				// instead of subtracting, we then should compute the modulus using "%=".
				// Since that requires a division and is slow, and shouldn't be necessary
				// in practice anyway, we only use subtraction.
				ch.offset.int_off -= ch.length;
				ch.dmaCount++;

				ch.data = ch.dataRepeat;
				ch.length = ch.lengthRepeat;
			}

			// If we have not yet generated enough samples, and looping is active: loop!
			if (neededSamples > 0 && ch.length > 2) {
				// Repeat as long as necessary.
				while (neededSamples > 0) {
					// Mix the generated samples into the output buffer
					neededSamples -= mixBuffer<stereo>(p, ch.data, ch.offset, rate, neededSamples, ch.length, ch.volume, ch.panning);

					if (ch.offset.int_off >= ch.length) {
						// Wrap around. See also the note above.
						ch.offset.int_off -= ch.length;
						ch.dmaCount++;
					}
				}
			}

		}
		buffer += _stereo ? nSamples * 2 : nSamples;
		_curInt -= nSamples;
		samples -= nSamples;
	}
	return numSamples;
}

} // End of namespace Audio


//	Plugin interface
//	(This can only create a null driver since apple II gs support seeems not to be implemented
//  and also is not part of the midi driver architecture. But we need the plugin for the options
//  menu in the launcher and for MidiDriver::detectDevice() which is more or less used by all engines.)

class AmigaMusicPlugin : public NullMusicPlugin {
public:
	const char *getName() const {
		return _s("Amiga Audio Emulator");
	}

	const char *getId() const {
		return "amiga";
	}

	MusicDevices getDevices() const;
};

MusicDevices AmigaMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_AMIGA));
	return devices;
}

//#if PLUGIN_ENABLED_DYNAMIC(AMIGA)
	//REGISTER_PLUGIN_DYNAMIC(AMIGA, PLUGIN_TYPE_MUSIC, AmigaMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(AMIGA, PLUGIN_TYPE_MUSIC, AmigaMusicPlugin);
//#endif
