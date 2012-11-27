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

#include "audio/mods/protracker.h"
#include "audio/mods/paula.h"
#include "audio/mods/module.h"

#include "audio/audiostream.h"

#include "common/textconsole.h"

namespace Modules {

class ProtrackerStream : public ::Audio::Paula {
private:
	Module _module;

	int _tick;
	int _row;
	int _pos;

	int _speed;
	int _bpm;

	// For effect 0xB - Jump To Pattern;
	bool _hasJumpToPattern;
	int _jumpToPattern;

	// For effect 0xD - PatternBreak;
	bool _hasPatternBreak;
	int _skipRow;

	// For effect 0xE6 - Pattern Loop
	bool _hasPatternLoop;
	int _patternLoopCount;
	int _patternLoopRow;

	// For effect 0xEE - Pattern Delay
	byte _patternDelay;

	static const int16 sinetable[];

	struct {
		byte sample;
		byte lastSample;
		uint16 period;
		Offset offset;

		byte vol;
		byte finetune;

		// For effect 0x0 - Arpeggio
		bool arpeggio;
		byte arpeggioNotes[3];

		// For effect 0x3 - Porta to note
		uint16 portaToNote;
		byte portaToNoteSpeed;

		// For effect 0x4 - Vibrato
		int vibrato;
		byte vibratoPos;
		byte vibratoSpeed;
		byte vibratoDepth;

		// For effect 0xED - Delay sample
		byte delaySample;
		byte delaySampleTick;
	} _track[4];

public:
	ProtrackerStream(Common::SeekableReadStream *stream, int offs, int rate, bool stereo);

private:
	void interrupt();

	void doPorta(int track) {
		if (_track[track].portaToNote && _track[track].portaToNoteSpeed) {
			if (_track[track].period < _track[track].portaToNote) {
				_track[track].period += _track[track].portaToNoteSpeed;
				if (_track[track].period > _track[track].portaToNote)
					_track[track].period = _track[track].portaToNote;
			} else if (_track[track].period > _track[track].portaToNote) {
				_track[track].period -= _track[track].portaToNoteSpeed;
				if (_track[track].period < _track[track].portaToNote)
					_track[track].period = _track[track].portaToNote;
			}
		}
	}
	void doVibrato(int track) {
		_track[track].vibrato =
				(_track[track].vibratoDepth * sinetable[_track[track].vibratoPos]) / 128;
		_track[track].vibratoPos += _track[track].vibratoSpeed;
		_track[track].vibratoPos %= 64;
	}
	void doVolSlide(int track, byte ex, byte ey) {
		int vol = _track[track].vol;
		if (ex == 0)
			vol -= ey;
		else if (ey == 0)
			vol += ex;

		if (vol < 0)
			vol = 0;
		else if (vol > 64)
			vol = 64;

		_track[track].vol = vol;
	}

	void updateRow();
	void updateEffects();

};

const int16 ProtrackerStream::sinetable[64] = {
		 0,   24,   49,   74,   97,  120,  141,  161,
	 180,  197,  212,  224,  235,  244,  250,  253,
	 255,  253,  250,  244,  235,  224,  212,  197,
	 180,  161,  141,  120,   97,   74,   49,   24,
		 0,  -24,  -49,  -74,  -97, -120, -141, -161,
	-180, -197, -212, -224, -235, -244, -250, -253,
	-255, -253, -250, -244, -235, -224, -212, -197,
	-180, -161, -141, -120,  -97,  -74,  -49,  -24
};

ProtrackerStream::ProtrackerStream(Common::SeekableReadStream *stream, int offs, int rate, bool stereo) :
		Paula(stereo, rate, rate/50) {
	bool result = _module.load(*stream, offs);
	assert(result);

	_tick = _row = _pos = 0;

	_speed = 6;
	_bpm = 125;

	_hasJumpToPattern = false;
	_jumpToPattern = 0;

	_hasPatternBreak = false;
	_skipRow = 0;

	_hasPatternLoop = false;
	_patternLoopCount = 0;
	_patternLoopRow = 0;

	_patternDelay = 0;

	memset(_track, 0, sizeof(_track));

	startPaula();
}

void ProtrackerStream::updateRow() {
	for (int track = 0; track < 4; track++) {
		_track[track].arpeggio = false;
		_track[track].vibrato = 0;
		_track[track].delaySampleTick = 0;
		const note_t note =
		    _module.pattern[_module.songpos[_pos]][_row][track];

		const int effect = note.effect >> 8;

		if (note.sample) {
			if (_track[track].sample != note.sample) {
				_track[track].vibratoPos = 0;
			}
			_track[track].sample = note.sample;
			_track[track].lastSample = note.sample;
			_track[track].finetune = _module.sample[note.sample - 1].finetune;
			_track[track].vol = _module.sample[note.sample - 1].vol;
		}

		if (note.period) {
			if (effect != 3 && effect != 5) {
				if (_track[track].finetune)
					_track[track].period = _module.noteToPeriod(note.note, _track[track].finetune);
				else
					_track[track].period = note.period;

				_track[track].offset = Offset(0);
				_track[track].sample = _track[track].lastSample;
			}
		}

		const byte exy = note.effect & 0xff;
		const byte ex = (note.effect >> 4) & 0xf;
		const byte ey = note.effect & 0xf;

		int vol;
		switch (effect) {
		case 0x0:
			if (exy) {
				_track[track].arpeggio = true;
				if (note.period) {
					_track[track].arpeggioNotes[0] = note.note;
					_track[track].arpeggioNotes[1] = note.note + ex;
					_track[track].arpeggioNotes[2] = note.note + ey;
				}
			}
			break;
		case 0x1:
			break;
		case 0x2:
			break;
		case 0x3:
			if (note.period)
				_track[track].portaToNote = note.period;
			if (exy)
				_track[track].portaToNoteSpeed = exy;
			break;
		case 0x4:
			if (exy) {
				_track[track].vibratoSpeed = ex;
				_track[track].vibratoDepth = ey;
			}
			break;
		case 0x5:
			doPorta(track);
			doVolSlide(track, ex, ey);
			break;
		case 0x6:
			doVibrato(track);
			doVolSlide(track, ex, ey);
			break;
		case 0x9: // Set sample offset
			if (exy) {
				_track[track].offset = Offset(exy * 256);
				setChannelOffset(track, _track[track].offset);
			}
			break;
		case 0xA:
			break;
		case 0xB:
			_hasJumpToPattern = true;
			_jumpToPattern = exy;
			break;
		case 0xC:
			_track[track].vol = exy;
			break;
		case 0xD:
			_hasPatternBreak = true;
			_skipRow = ex * 10 + ey;
			break;
		case 0xE:
			switch (ex) {
			case 0x0: // Switch filters off
				break;
			case 0x1: // Fine slide up
				_track[track].period -= exy;
				break;
			case 0x2: // Fine slide down
				_track[track].period += exy;
				break;
			case 0x5: // Set finetune
				_track[track].finetune = ey;
				_module.sample[_track[track].sample].finetune = ey;
				if (note.period) {
					if (ey)
						_track[track].period = _module.noteToPeriod(note.note, ey);
					else
						_track[track].period = note.period;
				}
				break;
			case 0x6:
				if (ey == 0) {
					_patternLoopRow = _row;
				} else {
					_patternLoopCount++;
					if (_patternLoopCount <= ey)
						_hasPatternLoop = true;
					else
						_patternLoopCount = 0;
				}
				break;
			case 0x9:
				break;	// Retrigger note
			case 0xA: // Fine volume slide up
				vol = _track[track].vol + ey;
				if (vol > 64)
					vol = 64;
				_track[track].vol = vol;
				break;
			case 0xB: // Fine volume slide down
				vol = _track[track].vol - ey;
				if (vol < 0)
					vol = 0;
				_track[track].vol = vol;
				break;
			case 0xD: // Delay sample
				_track[track].delaySampleTick = ey;
				_track[track].delaySample = _track[track].sample;
				_track[track].sample = 0;
				_track[track].vol = 0;
				break;
			case 0xE: // Pattern delay
				_patternDelay = ey;
				break;
			default:
				warning("Unimplemented effect %X", note.effect);
			}
			break;

		case 0xF:
			if (exy < 0x20) {
				_speed = exy;
			} else {
				_bpm = exy;
				setInterruptFreq((int) (getRate() / (_bpm * 0.4)));
			}
			break;
		default:
			warning("Unimplemented effect %X", note.effect);
		}
	}
}

void ProtrackerStream::updateEffects() {
	for (int track = 0; track < 4; track++) {
		_track[track].vibrato = 0;

		const note_t note =
		    _module.pattern[_module.songpos[_pos]][_row][track];

		const int effect = note.effect >> 8;

		const int exy = note.effect & 0xff;
		const int ex = (note.effect >> 4) & 0xf;
		const int ey = (note.effect) & 0xf;

		switch (effect) {
		case 0x0:
			if (exy) {
				const int idx = (_tick == 1) ? 0 : (_tick % 3);
				_track[track].period =
					_module.noteToPeriod(_track[track].arpeggioNotes[idx],
							_track[track].finetune);
			}
			break;
		case 0x1:
			_track[track].period -= exy;
			break;
		case 0x2:
			_track[track].period += exy;
			break;
		case 0x3:
			doPorta(track);
			break;
		case 0x4:
			doVibrato(track);
			break;
		case 0x5:
			doPorta(track);
			doVolSlide(track, ex, ey);
			break;
		case 0x6:
			doVibrato(track);
			doVolSlide(track, ex, ey);
			break;
		case 0xA:
			doVolSlide(track, ex, ey);
			break;
		case 0xE:
			switch (ex) {
			case 0x6:
				break;	// Pattern loop
			case 0x9:	// Retrigger note
				if (ey && (_tick % ey) == 0)
					_track[track].offset = Offset(0);
				break;
			case 0xD: // Delay sample
				if (_tick == _track[track].delaySampleTick) {
					_track[track].sample = _track[track].delaySample;
					_track[track].offset = Offset(0);
					if (_track[track].sample)
						_track[track].vol = _module.sample[_track[track].sample - 1].vol;
				}
				break;
			}
			break;
		}
	}
}

void ProtrackerStream::interrupt() {
	int track;

	for (track = 0; track < 4; track++) {
		_track[track].offset = getChannelOffset(track);
		if (_tick == 0 && _track[track].arpeggio) {
			_track[track].period = _module.noteToPeriod(_track[track].arpeggioNotes[0],
					_track[track].finetune);
		}
	}

	if (_tick == 0) {
		if (_hasJumpToPattern) {
			_hasJumpToPattern = false;
			_pos = _jumpToPattern;
			_row = 0;
		} else if (_hasPatternBreak) {
			_hasPatternBreak = false;
			_row = _skipRow;
			_pos = (_pos + 1) % _module.songlen;
			_patternLoopRow = 0;
		} else if (_hasPatternLoop) {
			_hasPatternLoop = false;
			_row = _patternLoopRow;
		}
		if (_row >= 64) {
			_row = 0;
			_pos = (_pos + 1) % _module.songlen;
			_patternLoopRow = 0;
		}

		updateRow();
	} else
		updateEffects();

	_tick = (_tick + 1) % (_speed + _patternDelay * _speed);
	if (_tick == 0) {
		_row++;
		_patternDelay = 0;
	}

	for (track = 0; track < 4; track++) {
		setChannelVolume(track, _track[track].vol);
		setChannelPeriod(track, _track[track].period + _track[track].vibrato);
		if (_track[track].sample) {
			sample_t &sample = _module.sample[_track[track].sample - 1];
			setChannelData(track,
			               sample.data,
			               sample.replen > 2 ? sample.data + sample.repeat : 0,
			               sample.len,
			               sample.replen);
			setChannelOffset(track, _track[track].offset);
			_track[track].sample = 0;
		}
	}
}

} // End of namespace Modules

namespace Audio {

AudioStream *makeProtrackerStream(Common::SeekableReadStream *stream, int offs, int rate, bool stereo) {
	return new Modules::ProtrackerStream(stream, offs, rate, stereo);
}

} // End of namespace Audio
