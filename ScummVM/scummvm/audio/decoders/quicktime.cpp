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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "common/util.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/decoders/codec.h"
#include "audio/decoders/quicktime.h"
#include "audio/decoders/quicktime_intern.h"

// Codecs
#include "audio/decoders/aac.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/qdm2.h"
#include "audio/decoders/raw.h"

namespace Audio {

/**
 * An AudioStream that just returns silent samples and runs infinitely.
 * Used to fill in the "empty edits" in the track queue which are just
 * supposed to be no sound playing.
 */
class SilentAudioStream : public AudioStream {
public:
	SilentAudioStream(int rate, bool stereo) : _rate(rate), _isStereo(stereo) {}

	int readBuffer(int16 *buffer, const int numSamples) {
		memset(buffer, 0, numSamples * 2);
		return numSamples;
	}

	bool endOfData() const { return false; } // it never ends!
	bool isStereo() const { return _isStereo; }
	int getRate() const { return _rate; }

private:
	int _rate;
	bool _isStereo;
};

/**
 * An AudioStream wrapper that forces audio to be played in mono.
 * It currently just ignores the right channel if stereo.
 */
class ForcedMonoAudioStream : public AudioStream {
public:
	ForcedMonoAudioStream(AudioStream *parentStream, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) :
			_parentStream(parentStream), _disposeAfterUse(disposeAfterUse) {}

	~ForcedMonoAudioStream() {
		if (_disposeAfterUse == DisposeAfterUse::YES)
				delete _parentStream;
	}

	int readBuffer(int16 *buffer, const int numSamples) {
		if (!_parentStream->isStereo())
			return _parentStream->readBuffer(buffer, numSamples);

		int16 temp[2];
		int samples = 0;

		while (samples < numSamples && !endOfData()) {
			_parentStream->readBuffer(temp, 2);
			*buffer++ = temp[0];
			samples++;
		}

		return samples;
	}

	bool endOfData() const { return _parentStream->endOfData(); }
	bool isStereo() const { return false; }
	int getRate() const { return _parentStream->getRate(); }

private:
	AudioStream *_parentStream;
	DisposeAfterUse::Flag _disposeAfterUse;
};

QuickTimeAudioDecoder::QuickTimeAudioDecoder() : Common::QuickTimeParser() {
}

QuickTimeAudioDecoder::~QuickTimeAudioDecoder() {
	for (uint32 i = 0; i < _audioTracks.size(); i++)
		delete _audioTracks[i];
}

bool QuickTimeAudioDecoder::loadAudioFile(const Common::String &filename) {
	if (!Common::QuickTimeParser::parseFile(filename))
		return false;

	init();
	return true;
}

bool QuickTimeAudioDecoder::loadAudioStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
	if (!Common::QuickTimeParser::parseStream(stream, disposeFileHandle))
		return false;

	init();
	return true;
}

void QuickTimeAudioDecoder::init() {
	Common::QuickTimeParser::init();

	// Initialize all the audio streams
	// But ignore any streams we don't support
	for (uint32 i = 0; i < _tracks.size(); i++)
		if (_tracks[i]->codecType == CODEC_TYPE_AUDIO && ((AudioSampleDesc *)_tracks[i]->sampleDescs[0])->isAudioCodecSupported())
			_audioTracks.push_back(new QuickTimeAudioTrack(this, _tracks[i]));
}

Common::QuickTimeParser::SampleDesc *QuickTimeAudioDecoder::readSampleDesc(Track *track, uint32 format) {
	if (track->codecType == CODEC_TYPE_AUDIO) {
		debug(0, "Audio Codec FourCC: \'%s\'", tag2str(format));

		AudioSampleDesc *entry = new AudioSampleDesc(track, format);

		uint16 stsdVersion = _fd->readUint16BE();
		_fd->readUint16BE(); // revision level
		_fd->readUint32BE(); // vendor

		entry->_channels = _fd->readUint16BE();			 // channel count
		entry->_bitsPerSample = _fd->readUint16BE();	  // sample size

		_fd->readUint16BE(); // compression id = 0
		_fd->readUint16BE(); // packet size = 0

		entry->_sampleRate = (_fd->readUint32BE() >> 16);

		debug(0, "stsd version =%d", stsdVersion);
		if (stsdVersion == 0) {
			// Not used, except in special cases. See below.
			entry->_samplesPerFrame = entry->_bytesPerFrame = 0;
		} else if (stsdVersion == 1) {
			// Read QT version 1 fields. In version 0 these dont exist.
			entry->_samplesPerFrame = _fd->readUint32BE();
			debug(0, "stsd samples_per_frame =%d",entry->_samplesPerFrame);
			_fd->readUint32BE(); // bytes per packet
			entry->_bytesPerFrame = _fd->readUint32BE();
			debug(0, "stsd bytes_per_frame =%d", entry->_bytesPerFrame);
			_fd->readUint32BE(); // bytes per sample
		} else {
			warning("Unsupported QuickTime STSD audio version %d", stsdVersion);
			delete entry;
			return 0;
		}

		// Version 0 files don't have some variables set, so we'll do that here
		if (format == MKTAG('i', 'm', 'a', '4')) {
			entry->_samplesPerFrame = 64;
			entry->_bytesPerFrame = 34 * entry->_channels;
		}

		if (entry->_sampleRate == 0 && track->timeScale > 1)
			entry->_sampleRate = track->timeScale;

		return entry;
	}

	return 0;
}

QuickTimeAudioDecoder::QuickTimeAudioTrack::QuickTimeAudioTrack(QuickTimeAudioDecoder *decoder, Common::QuickTimeParser::Track *parentTrack) {
	_decoder = decoder;
	_parentTrack = parentTrack;
	_queue = createStream();
	_samplesQueued = 0;

	AudioSampleDesc *entry = (AudioSampleDesc *)_parentTrack->sampleDescs[0];

	if (entry->getCodecTag() == MKTAG('r', 'a', 'w', ' ') || entry->getCodecTag() == MKTAG('t', 'w', 'o', 's'))
		_parentTrack->sampleSize = (entry->_bitsPerSample / 8) * entry->_channels;

	// Initialize our edit parser too
	_curEdit = 0;
	enterNewEdit(Timestamp());

	// If the edit doesn't start on a nice boundary, set us up to skip some samples
	Timestamp editStartTime(0, _parentTrack->editList[_curEdit].mediaTime, _parentTrack->timeScale);
	Timestamp trackPosition = getCurrentTrackTime();
	if (_parentTrack->editList[_curEdit].mediaTime != -1 && trackPosition != editStartTime)
		_skipSamples = editStartTime.convertToFramerate(getRate()) - trackPosition;
}

QuickTimeAudioDecoder::QuickTimeAudioTrack::~QuickTimeAudioTrack() {
	delete _queue;
}

void QuickTimeAudioDecoder::QuickTimeAudioTrack::queueAudio(const Timestamp &length) {
	if (allDataRead() || (length.totalNumberOfFrames() != 0 && Timestamp(0, _samplesQueued, getRate()) >= length))
		return;

	do {
		Timestamp nextEditTime(0, _parentTrack->editList[_curEdit].timeOffset + _parentTrack->editList[_curEdit].trackDuration, _decoder->_timeScale);

		if (_parentTrack->editList[_curEdit].mediaTime == -1) {
			// We've got an empty edit, so fill it with silence
			Timestamp editLength(0, _parentTrack->editList[_curEdit].trackDuration, _decoder->_timeScale);

			// If we seek into the middle of an empty edit, we need to adjust
			if (_skipSamples != Timestamp()) {
				editLength = editLength - _skipSamples;
				_skipSamples = Timestamp();
			}

			queueStream(makeLimitingAudioStream(new SilentAudioStream(getRate(), isStereo()), editLength), editLength);
			_curEdit++;
			enterNewEdit(nextEditTime);
		} else {
			// Normal audio
			AudioStream *stream = readAudioChunk(_curChunk);
			Timestamp chunkLength = getChunkLength(_curChunk, _skipAACPrimer);
			_skipAACPrimer = false;
			_curChunk++;

			// If we have any samples that we need to skip (ie. we seeked into
			// the middle of a chunk), skip them here.
			if (_skipSamples != Timestamp()) {
				skipSamples(_skipSamples, stream);
				_curMediaPos = _curMediaPos + _skipSamples;
				chunkLength = chunkLength - _skipSamples;
				_skipSamples = Timestamp();
			}

			// Calculate our overall position within the media
			Timestamp trackPosition = getCurrentTrackTime() + chunkLength;

			// If we have reached the end of this edit (or have no more media to read),
			// we move on to the next edit
			if (trackPosition >= nextEditTime || _curChunk >= _parentTrack->chunkCount) {
				chunkLength = nextEditTime.convertToFramerate(getRate()) - getCurrentTrackTime();
				stream = makeLimitingAudioStream(stream, chunkLength);
				_curEdit++;
				enterNewEdit(nextEditTime);

				// Next time around, we'll know how much to skip
				trackPosition = getCurrentTrackTime();
				if (!allDataRead() && _parentTrack->editList[_curEdit].mediaTime != -1 && nextEditTime != trackPosition)
					_skipSamples = nextEditTime.convertToFramerate(getRate()) - trackPosition;
			} else {
				_curMediaPos = _curMediaPos + chunkLength.convertToFramerate(_curMediaPos.framerate());
			}

			queueStream(stream, chunkLength);
		}
	} while (!allDataRead() && Timestamp(0, _samplesQueued, getRate()) < length);
}

Timestamp QuickTimeAudioDecoder::QuickTimeAudioTrack::getCurrentTrackTime() const {
	if (allDataRead())
		return getLength().convertToFramerate(getRate());

	return Timestamp(0, _parentTrack->editList[_curEdit].timeOffset, _decoder->_timeScale).convertToFramerate(getRate())
			+ _curMediaPos - Timestamp(0, _parentTrack->editList[_curEdit].mediaTime, _parentTrack->timeScale).convertToFramerate(getRate());
}

void QuickTimeAudioDecoder::QuickTimeAudioTrack::queueRemainingAudio() {
	queueAudio(getLength());
}

int QuickTimeAudioDecoder::QuickTimeAudioTrack::readBuffer(int16 *buffer, const int numSamples) {
	int samplesRead = _queue->readBuffer(buffer, numSamples);
	_samplesQueued -= samplesRead / (isStereo() ? 2 : 1);
	return samplesRead;
}

bool QuickTimeAudioDecoder::QuickTimeAudioTrack::allDataRead() const {
	return _curEdit == _parentTrack->editCount;
}

bool QuickTimeAudioDecoder::QuickTimeAudioTrack::endOfData() const {
	return allDataRead() && _queue->endOfData();
}

bool QuickTimeAudioDecoder::QuickTimeAudioTrack::seek(const Timestamp &where) {
	// Recreate the queue
	delete _queue;
	_queue = createStream();
	_samplesQueued = 0;

	if (where >= getLength()) {
		// We're done
		_curEdit = _parentTrack->editCount;
		return true;
	}

	// Find where we are in the stream
	findEdit(where);

	// Now queue up some audio and skip whatever we need to skip
	Timestamp samplesToSkip = where.convertToFramerate(getRate()) - getCurrentTrackTime();
	queueAudio();
	if (_parentTrack->editList[_curEdit].mediaTime != -1)
		skipSamples(samplesToSkip, _queue);

	return true;
}

Timestamp QuickTimeAudioDecoder::QuickTimeAudioTrack::getLength() const {
	return Timestamp(0, _parentTrack->duration, _decoder->_timeScale);
}

QueuingAudioStream *QuickTimeAudioDecoder::QuickTimeAudioTrack::createStream() const {
	AudioSampleDesc *entry = (AudioSampleDesc *)_parentTrack->sampleDescs[0];
	return makeQueuingAudioStream(entry->_sampleRate, entry->_channels == 2);
}

bool QuickTimeAudioDecoder::QuickTimeAudioTrack::isOldDemuxing() const {
	return _parentTrack->timeToSampleCount == 1 && _parentTrack->timeToSample[0].duration == 1;
}

AudioStream *QuickTimeAudioDecoder::QuickTimeAudioTrack::readAudioChunk(uint chunk) {
	AudioSampleDesc *entry = (AudioSampleDesc *)_parentTrack->sampleDescs[0];
	Common::MemoryWriteStreamDynamic *wStream = new Common::MemoryWriteStreamDynamic();

	_decoder->_fd->seek(_parentTrack->chunkOffsets[chunk]);

	// First, we have to get the sample count
	uint32 sampleCount = getAudioChunkSampleCount(chunk);
	assert(sampleCount != 0);

	if (isOldDemuxing()) {
		// Old-style audio demuxing

		// Then calculate the right sizes
		while (sampleCount > 0) {
			uint32 samples = 0, size = 0;

			if (entry->_samplesPerFrame >= 160) {
				samples = entry->_samplesPerFrame;
				size = entry->_bytesPerFrame;
			} else if (entry->_samplesPerFrame > 1) {
				samples = MIN<uint32>((1024 / entry->_samplesPerFrame) * entry->_samplesPerFrame, sampleCount);
				size = (samples / entry->_samplesPerFrame) * entry->_bytesPerFrame;
			} else {
				samples = MIN<uint32>(1024, sampleCount);
				size = samples * _parentTrack->sampleSize;
			}

			// Now, we read in the data for this data and output it
			byte *data = (byte *)malloc(size);
			_decoder->_fd->read(data, size);
			wStream->write(data, size);
			free(data);
			sampleCount -= samples;
		}
	} else {
		// New-style audio demuxing

		// Find our starting sample
		uint32 startSample = 0;
		for (uint32 i = 0; i < chunk; i++)
			startSample += getAudioChunkSampleCount(i);

		for (uint32 i = 0; i < sampleCount; i++) {
			uint32 size = (_parentTrack->sampleSize != 0) ? _parentTrack->sampleSize : _parentTrack->sampleSizes[i + startSample];

			// Now, we read in the data for this data and output it
			byte *data = (byte *)malloc(size);
			_decoder->_fd->read(data, size);
			wStream->write(data, size);
			free(data);
		}
	}

	AudioStream *audioStream = entry->createAudioStream(new Common::MemoryReadStream(wStream->getData(), wStream->size(), DisposeAfterUse::YES));
	delete wStream;

	return audioStream;
}

void QuickTimeAudioDecoder::QuickTimeAudioTrack::skipSamples(const Timestamp &length, AudioStream *stream) {
	int32 sampleCount = length.convertToFramerate(getRate()).totalNumberOfFrames();

	if (sampleCount <= 0)
		return;

	if (isStereo())
		sampleCount *= 2;

	int16 *tempBuffer = new int16[sampleCount];
	uint32 result = stream->readBuffer(tempBuffer, sampleCount);
	delete[] tempBuffer;

	// If this is the queue, make sure we subtract this number from the
	// amount queued
	if (stream == _queue)
		_samplesQueued -= result / (isStereo() ? 2 : 1);
}

void QuickTimeAudioDecoder::QuickTimeAudioTrack::findEdit(const Timestamp &position) {
	for (_curEdit = 0; _curEdit < _parentTrack->editCount - 1 && position > Timestamp(0, _parentTrack->editList[_curEdit].timeOffset, _decoder->_timeScale); _curEdit++)
		;

	enterNewEdit(position);
}

void QuickTimeAudioDecoder::QuickTimeAudioTrack::enterNewEdit(const Timestamp &position) {
	_skipSamples = Timestamp(); // make sure our skip variable doesn't remain around

	// If we're at the end of the edit list, there's nothing else for us to do here
	if (allDataRead())
		return;

	// For an empty edit, we may need to adjust the start time
	if (_parentTrack->editList[_curEdit].mediaTime == -1) {
		// Just invalidate the current media position (and make sure the scale
		// is in terms of our rate so it simplifies things later)
		_curMediaPos = Timestamp(0, 0, getRate());

		// Also handle shortening of the empty edit if needed
		if (position != Timestamp())
			_skipSamples = position.convertToFramerate(_decoder->_timeScale) - Timestamp(0, _parentTrack->editList[_curEdit].timeOffset, _decoder->_timeScale);
		return;
	}

	// I really hope I never need to implement this :P
	// But, I'll throw in this error just to make sure I catch anything with this...
	if (_parentTrack->editList[_curEdit].mediaRate != 1)
		error("Unhandled QuickTime audio rate change");

	// Reinitialize the codec
	((AudioSampleDesc *)_parentTrack->sampleDescs[0])->initCodec();
	_skipAACPrimer = true;

	// First, we need to track down what audio sample we need
	// Convert our variables from the media time (position) and the edit time (based on position)
	// and the media time
	Timestamp curAudioTime = Timestamp(0, _parentTrack->editList[_curEdit].mediaTime, _parentTrack->timeScale)
		+ position.convertToFramerate(_parentTrack->timeScale)
		- Timestamp(0, _parentTrack->editList[_curEdit].timeOffset, _decoder->_timeScale).convertToFramerate(_parentTrack->timeScale);

	uint32 sample = curAudioTime.totalNumberOfFrames();
	uint32 seekSample = sample;

	if (!isOldDemuxing()) {
		// For MPEG-4 style demuxing, we need to track down the sample based on the time
		// The old style demuxing doesn't require this because each "sample"'s duration
		// is just 1
		uint32 curSample = 0;
		seekSample = 0;

		for (int32 i = 0; i < _parentTrack->timeToSampleCount; i++) {
			uint32 sampleCount = _parentTrack->timeToSample[i].count * _parentTrack->timeToSample[i].duration;

			if (sample < curSample + sampleCount) {
				seekSample += (sample - curSample) / _parentTrack->timeToSample[i].duration;
				break;
			}

			seekSample += _parentTrack->timeToSample[i].count;
			curSample += sampleCount;
		}
	}

	// Now to track down what chunk it's in
	uint32 totalSamples = 0;
	_curChunk = 0;
	for (uint32 i = 0; i < _parentTrack->chunkCount; i++, _curChunk++) {
		uint32 chunkSampleCount = getAudioChunkSampleCount(i);

		if (seekSample < totalSamples + chunkSampleCount)
			break;

		totalSamples += chunkSampleCount;
	}

	// Now we get to have fun and convert *back* to an actual time
	// We don't want the sample count to be modified at this point, though
	if (!isOldDemuxing())
		totalSamples = getAACSampleTime(totalSamples);

	_curMediaPos = Timestamp(0, totalSamples, getRate());
}

void QuickTimeAudioDecoder::QuickTimeAudioTrack::queueStream(AudioStream *stream, const Timestamp &length) {
	// If the samples are stereo and the container is mono, force the samples
	// to be mono.
	if (stream->isStereo() && !isStereo())
		_queue->queueAudioStream(new ForcedMonoAudioStream(stream, DisposeAfterUse::YES), DisposeAfterUse::YES);
	else
		_queue->queueAudioStream(stream, DisposeAfterUse::YES);

	_samplesQueued += length.convertToFramerate(getRate()).totalNumberOfFrames();
}

uint32 QuickTimeAudioDecoder::QuickTimeAudioTrack::getAudioChunkSampleCount(uint chunk) const {
	uint32 sampleCount = 0;

	for (uint32 i = 0; i < _parentTrack->sampleToChunkCount; i++)
		if (chunk >= _parentTrack->sampleToChunk[i].first)
			sampleCount = _parentTrack->sampleToChunk[i].count;

	return sampleCount;
}

Timestamp QuickTimeAudioDecoder::QuickTimeAudioTrack::getChunkLength(uint chunk, bool skipAACPrimer) const {
	uint32 chunkSampleCount = getAudioChunkSampleCount(chunk);

	if (isOldDemuxing())
		return Timestamp(0, chunkSampleCount, getRate());

	// AAC needs some extra handling, of course
	return Timestamp(0, getAACSampleTime(chunkSampleCount, skipAACPrimer), getRate());
}

uint32 QuickTimeAudioDecoder::QuickTimeAudioTrack::getAACSampleTime(uint32 totalSampleCount, bool skipAACPrimer) const{
	uint32 curSample = 0;
	uint32 time = 0;

	for (int32 i = 0; i < _parentTrack->timeToSampleCount; i++) {
		uint32 sampleCount = _parentTrack->timeToSample[i].count;

		if (totalSampleCount < curSample + sampleCount) {
			time += (totalSampleCount - curSample) * _parentTrack->timeToSample[i].duration;
			break;
		}

		time += _parentTrack->timeToSample[i].count * _parentTrack->timeToSample[i].duration;
		curSample += sampleCount;
	}

	// The first chunk of AAC contains "duration" samples that are used as a primer
	// We need to subtract that number from the duration for the first chunk. See:
	// http://developer.apple.com/library/mac/#documentation/QuickTime/QTFF/QTFFAppenG/QTFFAppenG.html#//apple_ref/doc/uid/TP40000939-CH2-SW1
	// The skipping of both the primer and the remainder are handled by the AAC code,
	// whereas the timing of the remainder are handled by this time-to-sample chunk
	// code already.
	// We have to do this after each time we reinitialize the codec
	if (skipAACPrimer) {
		assert(_parentTrack->timeToSampleCount > 0);
		time -= _parentTrack->timeToSample[0].duration;
	}

	return time;
}

QuickTimeAudioDecoder::AudioSampleDesc::AudioSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag) : Common::QuickTimeParser::SampleDesc(parentTrack, codecTag) {
	_channels = 0;
	_sampleRate = 0;
	_samplesPerFrame = 0;
	_bytesPerFrame = 0;
	_bitsPerSample = 0;
	_codec = 0;
}

QuickTimeAudioDecoder::AudioSampleDesc::~AudioSampleDesc() {
	delete _codec;
}

bool QuickTimeAudioDecoder::AudioSampleDesc::isAudioCodecSupported() const {
	// Check if the codec is a supported codec
	if (_codecTag == MKTAG('t', 'w', 'o', 's') || _codecTag == MKTAG('r', 'a', 'w', ' ') || _codecTag == MKTAG('i', 'm', 'a', '4'))
		return true;

#ifdef AUDIO_QDM2_H
	if (_codecTag == MKTAG('Q', 'D', 'M', '2'))
		return true;
#endif

	if (_codecTag == MKTAG('m', 'p', '4', 'a')) {
		Common::String audioType;
		switch (_parentTrack->objectTypeMP4) {
		case 0x40: // AAC
#ifdef USE_FAAD
			return true;
#else
			audioType = "AAC";
			break;
#endif
		default:
			audioType = "Unknown";
			break;
		}
		warning("No MPEG-4 audio (%s) support", audioType.c_str());
	} else {
		warning("Audio Codec Not Supported: \'%s\'", tag2str(_codecTag));
	}

	return false;
}

AudioStream *QuickTimeAudioDecoder::AudioSampleDesc::createAudioStream(Common::SeekableReadStream *stream) const {
	if (!stream)
		return 0;

	if (_codec) {
		// If we've loaded a codec, make sure we use first
		AudioStream *audioStream = _codec->decodeFrame(*stream);
		delete stream;
		return audioStream;
	} else if (_codecTag == MKTAG('t', 'w', 'o', 's') || _codecTag == MKTAG('r', 'a', 'w', ' ')) {
		// Fortunately, most of the audio used in Myst videos is raw...
		uint16 flags = 0;
		if (_codecTag == MKTAG('r', 'a', 'w', ' '))
			flags |= FLAG_UNSIGNED;
		if (_channels == 2)
			flags |= FLAG_STEREO;
		if (_bitsPerSample == 16)
			flags |= FLAG_16BITS;
		uint32 dataSize = stream->size();
		byte *data = (byte *)malloc(dataSize);
		stream->read(data, dataSize);
		delete stream;
		return makeRawStream(data, dataSize, _sampleRate, flags);
	} else if (_codecTag == MKTAG('i', 'm', 'a', '4')) {
		// Riven uses this codec (as do some Myst ME videos)
		return makeADPCMStream(stream, DisposeAfterUse::YES, stream->size(), kADPCMApple, _sampleRate, _channels, 34);
	}

	error("Unsupported audio codec");
	return NULL;
}

void QuickTimeAudioDecoder::AudioSampleDesc::initCodec() {
	delete _codec; _codec = 0;

	switch (_codecTag) {
	case MKTAG('Q', 'D', 'M', '2'):
#ifdef AUDIO_QDM2_H
		_codec = makeQDM2Decoder(_parentTrack->extraData);
#endif
		break;
	case MKTAG('m', 'p', '4', 'a'):
#ifdef USE_FAAD
		if (_parentTrack->objectTypeMP4 == 0x40)
			_codec = makeAACDecoder(_parentTrack->extraData);
#endif
		break;
	default:
		break;
	}
}

/**
 * A wrapper around QuickTimeAudioDecoder that implements the SeekableAudioStream API
 */
class QuickTimeAudioStream : public SeekableAudioStream, public QuickTimeAudioDecoder {
public:
	QuickTimeAudioStream() {}
	~QuickTimeAudioStream() {}

	bool openFromFile(const Common::String &filename) {
		return QuickTimeAudioDecoder::loadAudioFile(filename) && !_audioTracks.empty();
	}

	bool openFromStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
		return QuickTimeAudioDecoder::loadAudioStream(stream, disposeFileHandle) && !_audioTracks.empty();
	}

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		int samples = 0;

		while (samples < numSamples && !endOfData()) {
			if (!_audioTracks[0]->hasDataInQueue())
				_audioTracks[0]->queueAudio();
			samples += _audioTracks[0]->readBuffer(buffer + samples, numSamples - samples);
		}

		return samples;
	}

	bool isStereo() const { return _audioTracks[0]->isStereo(); }
	int getRate() const { return _audioTracks[0]->getRate(); }
	bool endOfData() const { return _audioTracks[0]->endOfData(); }

	// SeekableAudioStream API
	bool seek(const Timestamp &where) { return _audioTracks[0]->seek(where); }
	Timestamp getLength() const { return _audioTracks[0]->getLength(); }
};

SeekableAudioStream *makeQuickTimeStream(const Common::String &filename) {
	QuickTimeAudioStream *audioStream = new QuickTimeAudioStream();

	if (!audioStream->openFromFile(filename)) {
		delete audioStream;
		return 0;
	}

	return audioStream;
}

SeekableAudioStream *makeQuickTimeStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	QuickTimeAudioStream *audioStream = new QuickTimeAudioStream();

	if (!audioStream->openFromStream(stream, disposeAfterUse)) {
		delete audioStream;
		return 0;
	}

	return audioStream;
}

} // End of namespace Audio
