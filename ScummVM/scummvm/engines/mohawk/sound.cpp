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

#include "common/debug.h"
#include "common/system.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "audio/midiparser.h"
#include "audio/musicplugin.h"
#include "audio/audiostream.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"

#include "mohawk/sound.h"

namespace Mohawk {

Sound::Sound(MohawkEngine* vm) : _vm(vm) {
	_midiDriver = NULL;
	_midiParser = NULL;
	_midiData = NULL;
	_mystBackgroundSound.type = kFreeHandle;
	initMidi();
}

Sound::~Sound() {
	stopSound();
	stopAllSLST();
	stopBackgroundMyst();

	if (_midiParser) {
		_midiParser->unloadMusic();
		delete _midiParser;
	}

	if (_midiDriver) {
		_midiDriver->close();
		delete _midiDriver;
	}

	if (_midiData)
		delete[] _midiData;
}

void Sound::initMidi() {
	if (!(_vm->getFeatures() & GF_HASMIDI))
		return;

	// Let's get our MIDI parser/driver
	_midiParser = MidiParser::createParser_SMF();
	_midiDriver = MidiDriver::createMidi(MidiDriver::detectDevice(MDT_ADLIB|MDT_MIDI));

	// Set up everything!
	_midiDriver->open();
	_midiParser->setMidiDriver(_midiDriver);
	_midiParser->setTimerRate(_midiDriver->getBaseTempo());
}

Audio::AudioStream *Sound::makeAudioStream(uint16 id, CueList *cueList) {
	Audio::AudioStream *audStream = NULL;

	switch (_vm->getGameType()) {
	case GType_MYST:
		if (_vm->getFeatures() & GF_ME)
			audStream = Audio::makeWAVStream(_vm->getResource(ID_MSND, convertMystID(id)), DisposeAfterUse::YES);
		else
			audStream = makeMohawkWaveStream(_vm->getResource(ID_MSND, id), cueList);
		break;
	case GType_ZOOMBINI:
		audStream = makeMohawkWaveStream(_vm->getResource(ID_SND, id));
		break;
	case GType_LIVINGBOOKSV1:
		audStream = makeLivingBooksWaveStream_v1(_vm->getResource(ID_WAV, id));
		break;
	case GType_LIVINGBOOKSV2:
		if (_vm->getPlatform() == Common::kPlatformMacintosh) {
			audStream = makeLivingBooksWaveStream_v1(_vm->getResource(ID_WAV, id));
			break;
		}
		// fall through
	default:
		audStream = makeMohawkWaveStream(_vm->getResource(ID_TWAV, id), cueList);
	}

	return audStream;
}

Audio::SoundHandle *Sound::playSound(uint16 id, byte volume, bool loop, CueList *cueList) {
	debug (0, "Playing sound %d", id);

	Audio::AudioStream *audStream = makeAudioStream(id, cueList);

	if (audStream) {
		SndHandle *handle = getHandle();
		handle->type = kUsedHandle;
		handle->id = id;
		handle->samplesPerSecond = audStream->getRate();

		// Set the stream to loop here if it's requested
		if (loop)
			audStream = Audio::makeLoopingAudioStream((Audio::RewindableAudioStream *)audStream, 0);

		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle->handle, audStream, -1, volume);
		return &handle->handle;
	}

	return NULL;
}

Audio::SoundHandle *Sound::replaceSoundMyst(uint16 id, byte volume, bool loop) {
	debug (0, "Replacing sound %d", id);

	// TODO: The original engine does fading

	Common::String name = _vm->getResourceName(ID_MSND, convertMystID(id));

	// Check if sound is already playing
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle
				&& _vm->_mixer->isSoundHandleActive(_handles[i].handle)
				&& name.equals(_vm->getResourceName(ID_MSND, convertMystID(_handles[i].id))))
			return &_handles[i].handle;

	// The original engine also forces looping for those sounds
	switch (id) {
	case 2205:
	case 2207:
	case 5378:
	case 7220:
	case 9119: 	// Elevator engine sound in mechanical age is looping.
	case 9120:
	case 9327:
		loop = true;
		break;
	}

	stopSound();
	return playSound(id, volume, loop);
}

void Sound::playSoundBlocking(uint16 id, byte volume) {
	Audio::SoundHandle *handle = playSound(id, volume);

	while (_vm->_mixer->isSoundHandleActive(*handle))
		_vm->_system->delayMillis(10);
}

void Sound::playMidi(uint16 id) {
	uint32 idTag;
	if (!(_vm->getFeatures() & GF_HASMIDI)) {
		warning ("Attempting to play MIDI in a game without MIDI");
		return;
	}

	assert(_midiDriver && _midiParser);

	_midiParser->unloadMusic();
	if (_midiData)
		delete[] _midiData;

	Common::SeekableReadStream *midi = _vm->getResource(ID_TMID, id);

	idTag = midi->readUint32BE();
	assert(idTag == ID_MHWK);
	midi->readUint32BE(); // Skip size
	idTag = midi->readUint32BE();
	assert(idTag == ID_MIDI);

	_midiData = new byte[midi->size() - 12]; // Enough to cover MThd/Prg#/MTrk

	// Read the MThd Data
	midi->read(_midiData, 14);

	// TODO: Load patches from the Prg# section... skip it for now.
	idTag = midi->readUint32BE();
	assert(idTag == ID_PRG);
	midi->skip(midi->readUint32BE());

	// Read the MTrk Data
	uint32 mtrkSize = midi->size() - midi->pos();
	midi->read(_midiData + 14, mtrkSize);

	delete midi;

	// Now, play it :)
	if (!_midiParser->loadMusic(_midiData, 14 + mtrkSize))
		error ("Could not play MIDI music from tMID %04x\n", id);

	_midiDriver->setTimerCallback(_midiParser, MidiParser::timerCallback);
}

void Sound::stopMidi() {
	_midiParser->unloadMusic();
}

byte Sound::convertRivenVolume(uint16 volume) {
	return (volume == 256) ? 255 : volume;
}

void Sound::playSLST(uint16 index, uint16 card) {
	Common::SeekableReadStream *slstStream = _vm->getResource(ID_SLST, card);
	SLSTRecord slstRecord;
	uint16 recordCount = slstStream->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		slstRecord.index = slstStream->readUint16BE();
		slstRecord.sound_count = slstStream->readUint16BE();
		slstRecord.sound_ids = new uint16[slstRecord.sound_count];

		for (uint16 j = 0; j < slstRecord.sound_count; j++)
			slstRecord.sound_ids[j] = slstStream->readUint16BE();

		slstRecord.fade_flags = slstStream->readUint16BE();
		slstRecord.loop = slstStream->readUint16BE();
		slstRecord.global_volume = slstStream->readUint16BE();
		slstRecord.u0 = slstStream->readUint16BE();			// Unknown

		if (slstRecord.u0 > 1)
			warning("slstRecord.u0: %d non-boolean", slstRecord.u0);

		slstRecord.u1 = slstStream->readUint16BE();			// Unknown

		if (slstRecord.u1 != 0)
			warning("slstRecord.u1: %d non-zero", slstRecord.u1);

		slstRecord.volumes = new uint16[slstRecord.sound_count];
		slstRecord.balances = new int16[slstRecord.sound_count];
		slstRecord.u2 = new uint16[slstRecord.sound_count];

		for (uint16 j = 0; j < slstRecord.sound_count; j++)
			slstRecord.volumes[j] = slstStream->readUint16BE();

		for (uint16 j = 0; j < slstRecord.sound_count; j++)
			slstRecord.balances[j] = slstStream->readSint16BE();	// negative = left, 0 = center, positive = right

		for (uint16 j = 0; j < slstRecord.sound_count; j++) {
			slstRecord.u2[j] = slstStream->readUint16BE();		// Unknown

			if (slstRecord.u2[j] != 255 && slstRecord.u2[j] != 256)
				warning("slstRecord.u2[%d]: %d not 255 or 256", j, slstRecord.u2[j]);
		}

		if (slstRecord.index == index) {
			playSLST(slstRecord);
			delete[] slstRecord.sound_ids;
			delete[] slstRecord.volumes;
			delete[] slstRecord.balances;
			delete[] slstRecord.u2;
			delete slstStream;
			return;
		}

		delete[] slstRecord.sound_ids;
		delete[] slstRecord.volumes;
		delete[] slstRecord.balances;
		delete[] slstRecord.u2;
	}

	delete slstStream;

	// If we have no matching entries, we do nothing and just let
	// the previous ambient sounds continue.
}

void Sound::playSLST(SLSTRecord slstRecord) {
	// End old sounds
	for (uint16 i = 0; i < _currentSLSTSounds.size(); i++) {
		bool noLongerPlay = true;
		for (uint16 j = 0; j < slstRecord.sound_count; j++)
			if (_currentSLSTSounds[i].id == slstRecord.sound_ids[j])
				noLongerPlay = false;
		if (noLongerPlay)
			stopSLSTSound(i, (slstRecord.fade_flags & 1) != 0);
	}

	// Start new sounds
	for (uint16 i = 0; i < slstRecord.sound_count; i++) {
		bool alreadyPlaying = false;
		for (uint16 j = 0; j < _currentSLSTSounds.size(); j++) {
			if (_currentSLSTSounds[j].id == slstRecord.sound_ids[i])
				alreadyPlaying = true;
		}
		if (!alreadyPlaying) {
			playSLSTSound(slstRecord.sound_ids[i],
						 (slstRecord.fade_flags & (1 << 1)) != 0,
						 slstRecord.loop != 0,
						 slstRecord.volumes[i],
						 slstRecord.balances[i]);
		}
	}
}

void Sound::stopAllSLST(bool fade) {
	for (uint16 i = 0; i < _currentSLSTSounds.size(); i++) {
		// TODO: Fade out, if requested
		_vm->_mixer->stopHandle(*_currentSLSTSounds[i].handle);
		delete _currentSLSTSounds[i].handle;
	}

	_currentSLSTSounds.clear();
}

static int8 convertBalance(int16 balance) {
	return (int8)(balance >> 8);
}

void Sound::playSLSTSound(uint16 id, bool fade, bool loop, uint16 volume, int16 balance) {
	// WORKAROUND: Some Riven SLST entries have a volume of 0, so we just ignore them.
	if (volume == 0)
		return;

	SLSTSndHandle sndHandle;
	sndHandle.handle = new Audio::SoundHandle();
	sndHandle.id = id;
	_currentSLSTSounds.push_back(sndHandle);

	Audio::AudioStream *audStream = makeMohawkWaveStream(_vm->getResource(ID_TWAV, id));

	// Loop here if necessary
	if (loop)
		audStream = Audio::makeLoopingAudioStream((Audio::RewindableAudioStream *)audStream, 0);

	// TODO: Handle fading, possibly just raise the volume of the channel in increments?

	_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, sndHandle.handle, audStream, -1, convertRivenVolume(volume), convertBalance(balance));
}

void Sound::stopSLSTSound(uint16 index, bool fade) {
	// TODO: Fade out, if requested
	_vm->_mixer->stopHandle(*_currentSLSTSounds[index].handle);
	delete _currentSLSTSounds[index].handle;
	_currentSLSTSounds.remove_at(index);
}

void Sound::pauseSLST() {
	for (uint16 i = 0; i < _currentSLSTSounds.size(); i++)
		_vm->_mixer->pauseHandle(*_currentSLSTSounds[i].handle, true);
}

void Sound::resumeSLST() {
	for (uint16 i = 0; i < _currentSLSTSounds.size(); i++)
		_vm->_mixer->pauseHandle(*_currentSLSTSounds[i].handle, false);
}

Audio::AudioStream *Sound::makeMohawkWaveStream(Common::SeekableReadStream *stream, CueList *cueList) {
	uint32 tag = 0;
	ADPCMStatus adpcmStatus;
	DataChunk dataChunk;
	uint32 dataSize = 0;

	memset(&dataChunk, 0, sizeof(DataChunk));

	if (stream->readUint32BE() != ID_MHWK) // MHWK tag again
		error ("Could not find tag 'MHWK'");

	stream->readUint32BE(); // Skip size

	if (stream->readUint32BE() != ID_WAVE)
		error ("Could not find tag 'WAVE'");

	while (!dataChunk.audioData) {
		tag = stream->readUint32BE();

		switch (tag) {
		case ID_ADPC:
			debug(2, "Found Tag ADPC");
			// ADPCM Sound Only
			//
			// This is useful for seeking in the stream, and is actually quite brilliant
			// considering some of the other things Broderbund did with the engine.
			// Only Riven and CSTime are known to use ADPCM audio and only CSTime
			// actually requires this for seeking. On the other hand, it may be interesting
			// to look at that one Riven sample that uses the cue points.
			//
			// Basically, the sample frame from the cue list is looked up here and then
			// sets the starting sample and step index at the point specified. Quite
			// an elegant/efficient system, really.

			adpcmStatus.size = stream->readUint32BE();
			adpcmStatus.itemCount = stream->readUint16BE();
			adpcmStatus.channels = stream->readUint16BE();
			adpcmStatus.statusItems = new ADPCMStatus::StatusItem[adpcmStatus.itemCount];

			assert(adpcmStatus.channels <= 2);

			for (uint16 i = 0; i < adpcmStatus.itemCount; i++) {
				adpcmStatus.statusItems[i].sampleFrame = stream->readUint32BE();

				for (uint16 j = 0; j < adpcmStatus.channels; j++) {
					adpcmStatus.statusItems[i].channelStatus[j].last = stream->readSint16BE();
					adpcmStatus.statusItems[i].channelStatus[j].stepIndex = stream->readUint16BE();
				}
			}

			// TODO: Actually use this chunk. For now, just delete the status items...
			delete[] adpcmStatus.statusItems;
			break;
		case ID_CUE:
			debug(2, "Found Tag Cue#");
			// Cues are used for animation sync. There are a couple in Myst and
			// Riven but are not used there at all.

			if (!cueList) {
				uint32 size = stream->readUint32BE();
				stream->skip(size);
				break;
			}

			cueList->size = stream->readUint32BE();
			cueList->pointCount = stream->readUint16BE();

			if (cueList->pointCount == 0)
				debug(2, "Cue# chunk found with no points!");
			else
				debug(2, "Cue# chunk found with %d point(s)!", cueList->pointCount);

			cueList->points.resize(cueList->pointCount);
			for (uint16 i = 0; i < cueList->pointCount; i++) {
				cueList->points[i].sampleFrame = stream->readUint32BE();

				byte nameLength = stream->readByte();
				cueList->points[i].name.clear();
				for (byte j = 0; j < nameLength; j++)
					cueList->points[i].name += stream->readByte();

				// Realign to an even boundary
				if (!(nameLength & 1))
					stream->readByte();

				debug (3, "Cue# chunk point %d (frame %d): %s", i, cueList->points[i].sampleFrame, cueList->points[i].name.c_str());
			}
			break;
		case ID_DATA:
			debug(2, "Found Tag DATA");
			// We subtract 20 from the actual chunk size, which is the total size
			// of the chunk's header
			dataSize = stream->readUint32BE() - 20;
			dataChunk.sampleRate = stream->readUint16BE();
			dataChunk.sampleCount = stream->readUint32BE();
			dataChunk.bitsPerSample = stream->readByte();
			dataChunk.channels = stream->readByte();
			dataChunk.encoding = stream->readUint16BE();
			dataChunk.loopCount = stream->readUint16BE();
			dataChunk.loopStart = stream->readUint32BE();
			dataChunk.loopEnd = stream->readUint32BE();

			// NOTE: We currently ignore all of the loop parameters here. Myst uses the
			// loopCount variable but the loopStart and loopEnd are always 0 and the size of
			// the sample. Myst ME doesn't use the Mohawk Sound format and just standard WAVE
			// files and therefore does not contain any of this metadata and we have to specify
			// whether or not to loop elsewhere.

			dataChunk.audioData = stream->readStream(dataSize);
			break;
		default:
			error ("Unknown tag found in 'tWAV' chunk -- '%s'", tag2str(tag));
		}
	}

	// makeMohawkWaveStream always takes control of the original stream
	delete stream;

	// The sound in Myst uses raw unsigned 8-bit data
	// The sound in the CD version of Riven is encoded in Intel DVI ADPCM
	// The sound in the DVD version of Riven is encoded in MPEG-2 Layer II or Intel DVI ADPCM
	if (dataChunk.encoding == kCodecRaw) {
		byte flags = Audio::FLAG_UNSIGNED;

		if (dataChunk.channels == 2)
			flags |= Audio::FLAG_STEREO;

		return Audio::makeRawStream(dataChunk.audioData, dataChunk.sampleRate, flags);
	} else if (dataChunk.encoding == kCodecADPCM) {
		uint32 blockAlign = dataChunk.channels * dataChunk.bitsPerSample / 8;
		return Audio::makeADPCMStream(dataChunk.audioData, DisposeAfterUse::YES, dataSize, Audio::kADPCMDVI, dataChunk.sampleRate, dataChunk.channels, blockAlign);
	} else if (dataChunk.encoding == kCodecMPEG2) {
#ifdef USE_MAD
		return Audio::makeMP3Stream(dataChunk.audioData, DisposeAfterUse::YES);
#else
		warning ("MAD library not included - unable to play MP2 audio");
#endif
	} else {
		error ("Unknown Mohawk WAVE encoding %d", dataChunk.encoding);
	}

	return NULL;
}

Audio::AudioStream *Sound::makeLivingBooksWaveStream_v1(Common::SeekableReadStream *stream) {
	uint16 header = stream->readUint16BE();
	uint16 rate = 0;
	uint32 size = 0;

	if (header == 'Wv') { // Big Endian
		rate = stream->readUint16BE();
		stream->skip(10); // Unknown
		size = stream->readUint32BE();
	} else if (header == 'vW') { // Little Endian
		stream->readUint16LE(); // Unknown
		rate = stream->readUint16LE();
		stream->skip(8); // Unknown
		size = stream->readUint32LE();
	} else
		error("Could not find Old Mohawk Sound header");

	Common::SeekableReadStream *dataStream = stream->readStream(size);
	delete stream;

	return Audio::makeRawStream(dataStream, rate, Audio::FLAG_UNSIGNED);
}

SndHandle *Sound::getHandle() {
	for (uint32 i = 0; i < _handles.size(); i++) {
		if (_handles[i].type == kFreeHandle)
			return &_handles[i];

		if (!_vm->_mixer->isSoundHandleActive(_handles[i].handle)) {
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
			return &_handles[i];
		}
	}

	// Let's add a new sound handle!
	SndHandle handle;
	handle.handle = Audio::SoundHandle();
	handle.type = kFreeHandle;
	handle.id = 0;
	_handles.push_back(handle);

	return &_handles[_handles.size() - 1];
}

void Sound::stopSound() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle) {
			_vm->_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
		}
}

void Sound::stopSound(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id) {
			_vm->_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
		}
}

void Sound::pauseSound() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle)
			_vm->_mixer->pauseHandle(_handles[i].handle, true);
}

void Sound::resumeSound() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle)
			_vm->_mixer->pauseHandle(_handles[i].handle, false);
}

bool Sound::isPlaying(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id)
			return _vm->_mixer->isSoundHandleActive(_handles[i].handle);

	return false;
}

bool Sound::isPlaying() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle)
			if (_vm->_mixer->isSoundHandleActive(_handles[i].handle))
				return true;

	return false;
}

uint Sound::getNumSamplesPlayed(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id) {
			return (_vm->_mixer->getSoundElapsedTime(_handles[i].handle) * _handles[i].samplesPerSecond) / 1000;
		}

	return 0;
}

uint16 Sound::convertMystID(uint16 id) {
	// Myst ME is a bit more efficient with sound storage than Myst
	// Myst has lots of sounds repeated. To overcome this, Myst ME
	// has MJMP resources which provide a link to the actual MSND
	// resource we're looking for. This saves a lot of space from
	// repeated data.
	if (_vm->hasResource(ID_MJMP, id)) {
		Common::SeekableReadStream *mjmpStream = _vm->getResource(ID_MJMP, id);
		id = mjmpStream->readUint16LE();
		delete mjmpStream;
	}

	return id;
}

Audio::SoundHandle *Sound::replaceBackgroundMyst(uint16 id, uint16 volume) {
	debug(0, "Replacing background sound with %d", id);

	// TODO: The original engine does fading

	Common::String name = _vm->getResourceName(ID_MSND, convertMystID(id));

	// Only the first eight characters need to be the same to have a match
	Common::String prefix;
	if (name.size() >= 8)
		prefix = Common::String(name.c_str(), name.c_str() + 8);
	else
		prefix = name;

	// Check if sound is already playing
	if (_mystBackgroundSound.type == kUsedHandle && _vm->_mixer->isSoundHandleActive(_mystBackgroundSound.handle)
			&& _vm->getResourceName(ID_MSND, convertMystID(_mystBackgroundSound.id)).hasPrefix(prefix))
		return &_mystBackgroundSound.handle;

	// Stop old background sound
	stopBackgroundMyst();

	// Play new sound
	Audio::AudioStream *audStream = makeAudioStream(id);

	if (audStream) {
		_mystBackgroundSound.type = kUsedHandle;
		_mystBackgroundSound.id = id;
		_mystBackgroundSound.samplesPerSecond = audStream->getRate();

		// Set the stream to loop
		audStream = Audio::makeLoopingAudioStream((Audio::RewindableAudioStream *)audStream, 0);

		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mystBackgroundSound.handle, audStream, -1, volume >> 8);
		return &_mystBackgroundSound.handle;
	}

	return NULL;
}

void Sound::stopBackgroundMyst() {
	if (_mystBackgroundSound.type == kUsedHandle) {
		_vm->_mixer->stopHandle(_mystBackgroundSound.handle);
		_mystBackgroundSound.type = kFreeHandle;
		_mystBackgroundSound.id = 0;
	}
}

void Sound::pauseBackgroundMyst() {
	if (_mystBackgroundSound.type == kUsedHandle)
		_vm->_mixer->pauseHandle(_mystBackgroundSound.handle, true);
}

void Sound::resumeBackgroundMyst() {
	if (_mystBackgroundSound.type == kUsedHandle)
		_vm->_mixer->pauseHandle(_mystBackgroundSound.handle, false);
}

void Sound::changeBackgroundVolumeMyst(uint16 vol) {
	if (_mystBackgroundSound.type == kUsedHandle)
		_vm->_mixer->setChannelVolume(_mystBackgroundSound.handle, vol >> 8);
}

} // End of namespace Mohawk
