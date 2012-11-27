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

// Code is based on:
//
// A very simple program, that converts an AGI-song into a MIDI-song.
// Feel free to use it for anything.
//
// The default instrument is "piano" for all the channels, what gives
// good results on most games. But I found, that some songs are interesting
// with other instruments. If you want to experiment, modify the "instr"
// array.
//
// Timing is not perfect, yet. It plays correct, when I use the
// Gravis-Midiplayer, but the songs are too fast when I use playmidi on
// Linux.
//
// Original program developed by Jens. Christian Restemeier
//

// MIDI and digital music class

#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "agi/agi.h"

#include "agi/sound.h"
#include "agi/sound_midi.h"

#define SPEED_FACTOR 6

namespace Agi {

static uint32 convertSND2MIDI(byte *snddata, byte **data);

MIDISound::MIDISound(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	_data = data; // Save the resource pointer
	_len  = len;  // Save the resource's length
	_type = READ_LE_UINT16(data); // Read sound resource's type
	_isValid = (_type == AGI_SOUND_4CHN) && (_data != NULL) && (_len >= 2);

	if (!_isValid) // Check for errors
		warning("Error creating MIDI sound from resource %d (Type %d, length %d)", resnum, _type, len);
}

SoundGenMIDI::SoundGenMIDI(AgiBase *vm, Audio::Mixer *pMixer) : SoundGen(vm, pMixer), _isGM(false) {
	MidiPlayer::createDriver(MDT_MIDI | MDT_ADLIB);

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		// FIXME: We need to cast "this" here due to the effects of
		// multiple inheritance. This hack can go away once this
		// setTimerCallback() has been moved inside Audio::MidiPlayer code.
		_driver->setTimerCallback(static_cast<Audio::MidiPlayer *>(this), &timerCallback);
	}
}

void SoundGenMIDI::send(uint32 b) {
	if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}

	Audio::MidiPlayer::send(b);
}

void SoundGenMIDI::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

void SoundGenMIDI::endOfTrack() {
	stop();
	_vm->_sound->soundIsFinished();
}

void SoundGenMIDI::play(int resnum) {
	MIDISound *track;

	stop();

	_isGM = true;

	track = (MIDISound *)_vm->_game.sounds[resnum];

	// Convert AGI Sound data to MIDI
	int midiMusicSize = convertSND2MIDI(track->_data, &_midiData);

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(_midiData, midiMusicSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		_isPlaying = true;
	} else {
		delete parser;
	}
}

/* channel / intrument setup: */

/* most songs are good with this: */
unsigned char instr[] = {0, 0, 0};

/* cool for sq2:
unsigned char instr[] = {50, 51, 19};
*/

static void writeDelta(Common::MemoryWriteStreamDynamic *st, int32 delta) {
	int32 i;

	i = delta >> 21; if (i > 0) st->writeByte((i & 127) | 128);
	i = delta >> 14; if (i > 0) st->writeByte((i & 127) | 128);
	i = delta >> 7;  if (i > 0) st->writeByte((i & 127) | 128);
	st->writeByte(delta & 127);
}

static uint32 convertSND2MIDI(byte *snddata, byte **data) {
	int32 lp, ep;
	int n;
	double ll;

	Common::MemoryWriteStreamDynamic st;

	ll = log10(pow(2.0, 1.0 / 12.0));

	/* Header */
	st.write("MThd", 4);
	st.writeUint32BE(6);
	st.writeUint16BE(1);    /* mode */
	st.writeUint16BE(3);    /* number of tracks */
	st.writeUint16BE(192);  /* ticks / quarter */

	for (n = 0; n < 3; n++) {
		uint16 start, end, pos;

		st.write("MTrk", 4);
		lp = st.pos();
		st.writeUint32BE(0);        /* chunklength */
		writeDelta(&st, 0);       /* set instrument */
		st.writeByte(0xc0 + n);
		st.writeByte(instr[n]);
		start = snddata[n * 2 + 0] | (snddata[n * 2 + 1] << 8);
		end = ((snddata[n * 2 + 2] | (snddata[n * 2 + 3] << 8))) - 5;

		for (pos = start; pos < end; pos += 5) {
			uint16 freq,  dur;
			dur = (snddata[pos + 0] | (snddata[pos + 1] << 8)) * SPEED_FACTOR;
			freq = ((snddata[pos + 2] & 0x3F)  <<  4)  +  (snddata[pos + 3] & 0x0F);
			if (snddata[pos + 2] > 0) {
				double fr;
				int note;
				/* I don't know,  what frequency equals midi note 0 ... */
				/* This moves the song 4 octaves down: */
				fr = (log10(111860.0 / (double)freq) / ll) - 48;
				note = (int)floor(fr + 0.5);
				if (note < 0) note = 0;
				if (note > 127) note = 127;
				/* note on */
				writeDelta(&st, 0);
				st.writeByte(144 + n);
				st.writeByte(note);
				st.writeByte(100);
				/* note off */
				writeDelta(&st, dur);
				st.writeByte(128 + n);
				st.writeByte(note);
				st.writeByte(0);
			} else {
				/* note on */
				writeDelta(&st, 0);
				st.writeByte(144 + n);
				st.writeByte(0);
				st.writeByte(0);
				/* note off */
				writeDelta(&st, dur);
				st.writeByte(128 + n);
				st.writeByte(0);
				st.writeByte(0);
			}
		}
		writeDelta(&st, 0);
		st.writeByte(0xff);
		st.writeByte(0x2f);
		st.writeByte(0x0);
		ep = st.pos();
		st.seek(lp, SEEK_SET);
		st.writeUint32BE((ep - lp) - 4);
		st.seek(ep, SEEK_SET);
	}

	*data = st.getData();

	return st.pos();
}

} // End of namespace Agi
