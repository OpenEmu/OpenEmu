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

#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/sound/midiparser_sci.h"
#include "sci/sound/drivers/mididriver.h"

namespace Sci {

static const int nMidiParams[] = { 2, 2, 2, 2, 1, 1, 2, 0 };

enum SciMidiCommands {
	kSetSignalLoop = 0x7F,
	kEndOfTrack = 0xFC,
	kSetReverb = 0x50,
	kMidiHold = 0x52,
	kUpdateCue = 0x60,
	kResetOnPause = 0x4C
};

//  MidiParser_SCI
//
MidiParser_SCI::MidiParser_SCI(SciVersion soundVersion, SciMusic *music) :
	MidiParser() {
	_soundVersion = soundVersion;
	_music = music;
	_mixedData = NULL;
	// mididata contains delta in 1/60th second
	// values of ppqn and tempo are found experimentally and may be wrong
	_ppqn = 1;
	setTempo(16667);

	_masterVolume = 15;
	_volume = 127;

	_signalSet = false;
	_signalToSet = 0;
	_dataincAdd = false;
	_dataincToAdd = 0;
	_resetOnPause = false;
	_pSnd = 0;
}

MidiParser_SCI::~MidiParser_SCI() {
	unloadMusic();
	// we do this, so that MidiParser won't be able to call his own ::allNotesOff()
	//  this one would affect all channels and we can't let that happen
	_driver = 0;
}

void MidiParser_SCI::mainThreadBegin() {
	_mainThreadCalled = true;
}

void MidiParser_SCI::mainThreadEnd() {
	_mainThreadCalled = false;
}

bool MidiParser_SCI::loadMusic(SoundResource::Track *track, MusicEntry *psnd, int channelFilterMask, SciVersion soundVersion) {
	unloadMusic();
	_track = track;
	_pSnd = psnd;
	_soundVersion = soundVersion;

	for (int i = 0; i < 16; i++) {
		_channelUsed[i] = false;
		_channelRemap[i] = -1;
		_channelMuted[i] = false;
		_channelVolume[i] = 127;
	}
	_channelRemap[9] = 9; // never map channel 9, because that's used for percussion
	_channelRemap[15] = 15; // never map channel 15, because thats used by sierra internally

	if (channelFilterMask) {
		// SCI0 only has 1 data stream, but we need to filter out channels depending on music hardware selection
		midiFilterChannels(channelFilterMask);
	} else {
		midiMixChannels();
	}

	_numTracks = 1;
	_tracks[0] = _mixedData;
	if (_pSnd)
		setTrack(0);
	_loopTick = 0;

	return true;
}

byte MidiParser_SCI::midiGetNextChannel(long ticker) {
	byte curr = 0xFF;
	long closest = ticker + 1000000, next = 0;

	for (int i = 0; i < _track->channelCount; i++) {
		if (_track->channels[i].time == -1) // channel ended
			continue;
		SoundResource::Channel *curChannel = &_track->channels[i];
		if (curChannel->curPos >= curChannel->size)
			continue;
		next = curChannel->data[curChannel->curPos]; // when the next event should occur
		if (next == 0xF8) // 0xF8 means 240 ticks delay
			next = 240;
		next += _track->channels[i].time;
		if (next < closest) {
			curr = i;
			closest = next;
		}
	}

	return curr;
}

byte *MidiParser_SCI::midiMixChannels() {
	int totalSize = 0;

	for (int i = 0; i < _track->channelCount; i++) {
		_track->channels[i].time = 0;
		_track->channels[i].prev = 0;
		_track->channels[i].curPos = 0;
		totalSize += _track->channels[i].size;
	}

	byte *outData = new byte[totalSize * 2]; // FIXME: creates overhead and still may be not enough to hold all data
	_mixedData = outData;
	long ticker = 0;
	byte channelNr, curDelta;
	byte midiCommand = 0, midiParam, globalPrev = 0;
	long newDelta;
	SoundResource::Channel *channel;

	while ((channelNr = midiGetNextChannel(ticker)) != 0xFF) { // there is still an active channel
		channel = &_track->channels[channelNr];
		curDelta = channel->data[channel->curPos++];
		channel->time += (curDelta == 0xF8 ? 240 : curDelta); // when the command is supposed to occur
		if (curDelta == 0xF8)
			continue;
		newDelta = channel->time - ticker;
		ticker += newDelta;

		midiCommand = channel->data[channel->curPos++];
		if (midiCommand != kEndOfTrack) {
			// Write delta
			while (newDelta > 240) {
				*outData++ = 0xF8;
				newDelta -= 240;
			}
			*outData++ = (byte)newDelta;
		}
		// Write command
		switch (midiCommand) {
		case 0xF0: // sysEx
			*outData++ = midiCommand;
			do {
				midiParam = channel->data[channel->curPos++];
				*outData++ = midiParam;
			} while (midiParam != 0xF7);
			break;
		case kEndOfTrack: // end of channel
			channel->time = -1;
			break;
		default: // MIDI command
			if (midiCommand & 0x80) {
				midiParam = channel->data[channel->curPos++];
			} else {// running status
				midiParam = midiCommand;
				midiCommand = channel->prev;
			}

			// remember which channel got used for channel remapping
			byte midiChannel = midiCommand & 0xF;
			_channelUsed[midiChannel] = true;

			if (midiCommand != globalPrev)
				*outData++ = midiCommand;
			*outData++ = midiParam;
			if (nMidiParams[(midiCommand >> 4) - 8] == 2)
				*outData++ = channel->data[channel->curPos++];
			channel->prev = midiCommand;
			globalPrev = midiCommand;
		}
	}

	// Insert stop event
	*outData++ = 0;    // Delta
	*outData++ = 0xFF; // Meta event
	*outData++ = 0x2F; // End of track (EOT)
	*outData++ = 0x00;
	*outData++ = 0x00;
	return _mixedData;
}

// This is used for SCI0 sound-data. SCI0 only has one stream that may
// contain several channels and according to output device we remove
// certain channels from that data.
byte *MidiParser_SCI::midiFilterChannels(int channelMask) {
	SoundResource::Channel *channel = &_track->channels[0];
	byte *channelData = channel->data;
	byte *channelDataEnd = channel->data + channel->size;
	byte *outData = new byte[channel->size + 5];
	byte curChannel = 15, curByte, curDelta;
	byte command = 0, lastCommand = 0;
	int delta = 0;
	int midiParamCount = 0;
	bool containsMidiData = false;

	_mixedData = outData;

	while (channelData < channelDataEnd) {
		curDelta = *channelData++;
		if (curDelta == 0xF8) {
			delta += 240;
			continue;
		}
		delta += curDelta;
		curByte = *channelData++;

		switch (curByte) {
		case 0xF0: // sysEx
		case kEndOfTrack: // end of channel
			command = curByte;
			curChannel = 15;
			break;
		default:
			if (curByte & 0x80) {
				command = curByte;
				curChannel = command & 0x0F;
				midiParamCount = nMidiParams[(command >> 4) - 8];
			}
		}
		if ((1 << curChannel) & channelMask) {
			if (curChannel != 0xF)
				containsMidiData = true;

			if (command != kEndOfTrack) {
				// Write delta
				while (delta > 240) {
					*outData++ = 0xF8;
					delta -= 240;
				}
				*outData++ = (byte)delta;
				delta = 0;
			}
			// Write command
			switch (command) {
			case 0xF0: // sysEx
				*outData++ = command;
				do {
					curByte = *channelData++;
					*outData++ = curByte; // out
				} while (curByte != 0xF7);
				lastCommand = command;
				break;

			case kEndOfTrack: // end of channel
				break;

			default: // MIDI command
				// remember which channel got used for channel remapping
				byte midiChannel = command & 0xF;
				_channelUsed[midiChannel] = true;

				if (lastCommand != command) {
					*outData++ = command;
					lastCommand = command;
				}
				if (midiParamCount > 0) {
					if (curByte & 0x80)
						*outData++ = *channelData++;
					else
						*outData++ = curByte;
				}
				if (midiParamCount > 1) {
					*outData++ = *channelData++;
				}
			}
		} else {
			if (curByte & 0x80)
				channelData += midiParamCount;
			else
				channelData += midiParamCount - 1;
		}
	}

	// Insert stop event
	*outData++ = 0;    // Delta
	*outData++ = 0xFF; // Meta event
	*outData++ = 0x2F; // End of track (EOT)
	*outData++ = 0x00;
	*outData++ = 0x00;

	// This occurs in the music tracks of LB1 Amiga, when using the MT-32
	// driver (bug #3297881)
	if (!containsMidiData)
		warning("MIDI parser: the requested SCI0 sound has no MIDI note data for the currently selected sound driver");

	return _mixedData;
}

// This will get called right before actual playing and will try to own the used channels
void MidiParser_SCI::tryToOwnChannels() {
	// We don't have SciMusic in case debug command show_instruments is used
	if (!_music)
		return;
	for (int curChannel = 0; curChannel < 15; curChannel++) {
		if (_channelUsed[curChannel]) {
			if (_channelRemap[curChannel] == -1) {
				_channelRemap[curChannel] = _music->tryToOwnChannel(_pSnd, curChannel);
			}
		}
	}
}

void MidiParser_SCI::lostChannels() {
	for (int curChannel = 0; curChannel < 15; curChannel++)
		if ((_channelUsed[curChannel]) && (curChannel != 9))
			_channelRemap[curChannel] = -1;
}

void MidiParser_SCI::sendInitCommands() {
	// reset our "global" volume and channel volumes
	_volume = 127;
	for (int i = 0; i < 16; i++)
		_channelVolume[i] = 127;

	// Set initial voice count
	if (_pSnd) {
		if (_soundVersion <= SCI_VERSION_0_LATE) {
			for (int i = 0; i < 15; ++i) {
				byte voiceCount = 0;
				if (_channelUsed[i]) {
					voiceCount = _pSnd->soundRes->getInitialVoiceCount(i);
					sendToDriver(0xB0 | i, 0x4B, voiceCount);
				}
			}
		}
	}

	// Reset all the parameters of the channels used by this song
	for (int i = 0; i < 16; ++i) {
		if (_channelUsed[i]) {
			sendToDriver(0xB0 | i, 0x07, 127);	// Reset volume to maximum
			sendToDriver(0xB0 | i, 0x0A, 64);	// Reset panning to center
			sendToDriver(0xB0 | i, 0x40, 0);	// Reset hold pedal to none
			sendToDriver(0xB0 | i, 0x4E, 0);	// Reset velocity to none
			sendToDriver(0xE0 | i,    0, 64);	// Reset pitch wheel to center
		}
	}
}

void MidiParser_SCI::unloadMusic() {
	if (_pSnd) {
		resetTracking();
		allNotesOff();
	}
	_numTracks = 0;
	_activeTrack = 255;
	_resetOnPause = false;

	if (_mixedData) {
		delete[] _mixedData;
		_mixedData = NULL;
	}
}

// this is used for scripts sending midi commands to us. we verify in that case that the channel is actually
//  used, so that channel remapping will work as well and then send them on
void MidiParser_SCI::sendFromScriptToDriver(uint32 midi) {
	byte midiChannel = midi & 0xf;

	if (!_channelUsed[midiChannel]) {
		// trying to send to an unused channel
		//  this happens for cmdSendMidi at least in sq1vga right at the start, it's a script issue
		return;
	}
	if (_channelRemap[midiChannel] == -1) {
		// trying to send to an unmapped channel
		//  this happens for cmdSendMidi at least in sq1vga right at the start, scripts are pausing the sound
		//  and then sending manually. it's a script issue
		return;
	}
	sendToDriver(midi);
}

void MidiParser_SCI::sendToDriver(uint32 midi) {
	byte midiChannel = midi & 0xf;

	if ((midi & 0xFFF0) == 0x4EB0) {
		// this is channel mute only for sci1
		// it's velocity control for sci0
		if (_soundVersion >= SCI_VERSION_1_EARLY) {
			_channelMuted[midiChannel] = midi & 0xFF0000 ? true : false;
			return; // don't send this to driver at all
		}
	}

	// Is channel muted? if so, don't send command
	if (_channelMuted[midiChannel])
		return;

	if ((midi & 0xFFF0) == 0x07B0) {
		// someone trying to set channel volume?
		int channelVolume = (midi >> 16) & 0xFF;
		// Remember, if we need to set it ourselves
		_channelVolume[midiChannel] = channelVolume;
		// Adjust volume accordingly to current local volume
		channelVolume = channelVolume * _volume / 127;
		midi = (midi & 0xFFF0) | ((channelVolume & 0xFF) << 16);
	}

	// Channel remapping
	int16 realChannel = _channelRemap[midiChannel];
	if (realChannel == -1)
		return;

	midi = (midi & 0xFFFFFFF0) | realChannel;
	if (_mainThreadCalled)
		_music->putMidiCommandInQueue(midi);
	else
		_driver->send(midi);
}

void MidiParser_SCI::parseNextEvent(EventInfo &info) {
	// Set signal AFTER waiting for delta, otherwise we would set signal too soon resulting in all sorts of bugs
	if (_dataincAdd) {
		_dataincAdd = false;
		_pSnd->dataInc += _dataincToAdd;
		_pSnd->signal = 0x7f + _pSnd->dataInc;
		debugC(4, kDebugLevelSound, "datainc %04x", _dataincToAdd);
	}
	if (_signalSet) {
		_signalSet = false;
		_pSnd->setSignal(_signalToSet);

		debugC(4, kDebugLevelSound, "signal %04x", _signalToSet);
	}

	info.start = _position._playPos;
	info.delta = 0;
	while (*_position._playPos == 0xF8) {
		info.delta += 240;
		_position._playPos++;
	}
	info.delta += *(_position._playPos++);

	// Process the next info.
	if ((_position._playPos[0] & 0xF0) >= 0x80)
		info.event = *(_position._playPos++);
	else
		info.event = _position._runningStatus;
	if (info.event < 0x80)
		return;

	_position._runningStatus = info.event;
	switch (info.command()) {
	case 0xC:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = 0;
		if (info.channel() == 0xF) {// SCI special case
			if (info.basic.param1 != kSetSignalLoop) {
				// At least in kq5/french&mac the first scene in the intro has
				// a song that sets signal to 4 immediately on tick 0. Signal
				// isn't set at that point by sierra sci and it would cause the
				// castle daventry text to get immediately removed, so we
				// currently filter it. Sierra SCI ignores them as well at that
				// time. However, this filtering should only be performed for
				// SCI1 and newer games. Signalling is done differently in SCI0
				// though, so ignoring these signals in SCI0 games will result
				// in glitches (e.g. the intro of LB1 Amiga gets stuck - bug
				// #3297883). Refer to MusicEntry::setSignal() in sound/music.cpp.
				if (_soundVersion <= SCI_VERSION_0_LATE ||
					_position._playTick || info.delta) {
					_signalSet = true;
					_signalToSet = info.basic.param1;
				}
			} else {
				_loopTick = _position._playTick + info.delta;
			}
		}
		break;
	case 0xD:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = 0;
		break;

	case 0xB:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = *(_position._playPos++);

		// Reference for some events:
		// http://wiki.scummvm.org/index.php/SCI/Specifications/Sound/SCI0_Resource_Format#Status_Reference
		// Handle common special events
		switch (info.basic.param1) {
		case kSetReverb:
			if (info.basic.param2 == 127)		// Set global reverb instead
				_pSnd->reverb = _music->getGlobalReverb();
			else
				_pSnd->reverb = info.basic.param2;

			((MidiPlayer *)_driver)->setReverb(_pSnd->reverb);
			break;
		default:
			break;
		}

		// Handle events sent to the SCI special channel (15)
		if (info.channel() == 0xF) {
			switch (info.basic.param1) {
			case kSetReverb:
				// Already handled above
				break;
			case kMidiHold:
				// Check if the hold ID marker is the same as the hold ID
				// marker set for that song by cmdSetSoundHold.
				// If it is, loop back, but don't stop notes when jumping.
				if (info.basic.param2 == _pSnd->hold)
					jumpToTick(_loopTick, false, false);
				break;
			case kUpdateCue:
				_dataincAdd = true;
				switch (_soundVersion) {
				case SCI_VERSION_0_EARLY:
				case SCI_VERSION_0_LATE:
					_dataincToAdd = info.basic.param2;
					break;
				case SCI_VERSION_1_EARLY:
				case SCI_VERSION_1_LATE:
				case SCI_VERSION_2_1:
					_dataincToAdd = 1;
					break;
				default:
					error("unsupported _soundVersion");
				}
				break;
			case kResetOnPause:
				_resetOnPause = info.basic.param2;
				break;
			// Unhandled SCI commands
			case 0x46: // LSL3 - binoculars
			case 0x61: // Iceman (AdLib?)
			case 0x73: // Hoyle
			case 0xD1: // KQ4, when riding the unicorn
				// Obscure SCI commands - ignored
				break;
			// Standard MIDI commands
			case 0x01:	// mod wheel
			case 0x04:	// foot controller
			case 0x07:	// channel volume
			case 0x0A:	// pan
			case 0x0B:	// expression
			case 0x40:	// sustain
			case 0x79:	// reset all
			case 0x7B:	// notes off
				// These are all handled by the music driver, so ignore them
				break;
			case 0x4B:	// voice mapping
				// TODO: is any support for this needed at the MIDI parser level?
				warning("Unhanded SCI MIDI command 0x%x - voice mapping (parameter %d)", info.basic.param1, info.basic.param2);
				break;
			default:
				warning("Unhandled SCI MIDI command 0x%x (parameter %d)", info.basic.param1, info.basic.param2);
				break;
			}
		}
		info.length = 0;
		break;

	case 0x8:
	case 0x9:
	case 0xA:
	case 0xE:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = *(_position._playPos++);
		if (info.command() == 0x9 && info.basic.param2 == 0)
			info.event = info.channel() | 0x80;
		info.length = 0;
		break;

	case 0xF: // System Common, Meta or SysEx event
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.basic.param1 = *(_position._playPos++);
			info.basic.param2 = *(_position._playPos++);
			break;

		case 0x3: // Song Select
			info.basic.param1 = *(_position._playPos++);
			info.basic.param2 = 0;
			break;

		case 0x6:
		case 0x8:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xE:
			info.basic.param1 = info.basic.param2 = 0;
			break;

		case 0x0: // SysEx
			info.length = readVLQ(_position._playPos);
			info.ext.data = _position._playPos;
			_position._playPos += info.length;
			break;

		case 0xF: // META event
			info.ext.type = *(_position._playPos++);
			info.length = readVLQ(_position._playPos);
			info.ext.data = _position._playPos;
			_position._playPos += info.length;
			if (info.ext.type == 0x2F) {// end of track reached
				if (_pSnd->loop)
					_pSnd->loop--;
				// QFG3 abuses the hold flag. Its scripts call kDoSoundSetHold,
				// but sometimes there's no hold marker in the associated songs
				// (e.g. song 110, during the intro). The original interpreter
				// treats this case as an infinite loop (bug #3311911).
				if (_pSnd->loop || _pSnd->hold > 0) {
					// We need to play it again...
					jumpToTick(_loopTick);
				} else {
					_pSnd->status = kSoundStopped;
					_pSnd->setSignal(SIGNAL_OFFSET);

					debugC(4, kDebugLevelSound, "signal EOT");
				}
			}
			break;
		default:
			warning(
					"MidiParser_SCI::parseNextEvent: Unsupported event code %x",
					info.event);
		} // // System Common, Meta or SysEx event
	}// switch (info.command())
}

byte MidiParser_SCI::getSongReverb() {
	assert(_track);

	if (_soundVersion >= SCI_VERSION_1_EARLY) {
		for (int i = 0; i < _track->channelCount; i++) {
			SoundResource::Channel &channel = _track->channels[i];
			// Peek ahead in the control channel to get the default reverb setting
			if (channel.number == 15 && channel.size >= 7)
				return channel.data[6];
		}
	}

	return 127;
}

void MidiParser_SCI::allNotesOff() {
	if (!_driver)
		return;

	int i, j;

	// Turn off all active notes
	for (i = 0; i < 128; ++i) {
		for (j = 0; j < 16; ++j) {
			if ((_activeNotes[i] & (1 << j)) && (_channelRemap[j] != -1)){
				sendToDriver(0x80 | j, i, 0);
			}
		}
	}

	// Turn off all hanging notes
	for (i = 0; i < ARRAYSIZE(_hangingNotes); i++) {
		byte midiChannel = _hangingNotes[i].channel;
		if ((_hangingNotes[i].timeLeft) && (_channelRemap[midiChannel] != -1)) {
			sendToDriver(0x80 | midiChannel, _hangingNotes[i].note, 0);
			_hangingNotes[i].timeLeft = 0;
		}
	}
	_hangingNotesCount = 0;

	// To be sure, send an "All Note Off" event (but not all MIDI devices
	// support this...).

	for (i = 0; i < 16; ++i) {
		if (_channelRemap[i] != -1) {
			sendToDriver(0xB0 | i, 0x7b, 0); // All notes off
			sendToDriver(0xB0 | i, 0x40, 0); // Also send a sustain off event (bug #3116608)
		}
	}

	memset(_activeNotes, 0, sizeof(_activeNotes));
}

void MidiParser_SCI::setMasterVolume(byte masterVolume) {
	assert(masterVolume <= MUSIC_MASTERVOLUME_MAX);
	_masterVolume = masterVolume;
	switch (_soundVersion) {
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE:
		// update driver master volume
		setVolume(_volume);
		break;

	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
	case SCI_VERSION_2_1:
		// directly set master volume (global volume is merged with channel volumes)
		((MidiPlayer *)_driver)->setVolume(masterVolume);
		break;

	default:
		error("MidiParser_SCI::setVolume: Unsupported soundVersion");
	}
}

void MidiParser_SCI::setVolume(byte volume) {
	assert(volume <= MUSIC_VOLUME_MAX);
	_volume = volume;

	switch (_soundVersion) {
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE: {
		// SCI0 adlib driver doesn't support channel volumes, so we need to go this way
		int16 globalVolume = _volume * _masterVolume / MUSIC_VOLUME_MAX;
		((MidiPlayer *)_driver)->setVolume(globalVolume);
		break;
	}

	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
	case SCI_VERSION_2_1:
		// Send previous channel volumes again to actually update the volume
		for (int i = 0; i < 15; i++)
			if (_channelRemap[i] != -1)
				sendToDriver(0xB0 + i, 7, _channelVolume[i]);
		break;

	default:
		error("MidiParser_SCI::setVolume: Unsupported soundVersion");
	}
}

} // End of namespace Sci
