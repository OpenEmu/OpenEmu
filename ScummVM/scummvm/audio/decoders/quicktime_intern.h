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

/**
 * Internal interface to the QuickTime audio decoder.
 *
 * This is available so that the QuickTimeVideoDecoder can use
 * this directly.
 */

#ifndef AUDIO_QUICKTIME_INTERN_H
#define AUDIO_QUICKTIME_INTERN_H

#include "common/quicktime.h"
#include "common/scummsys.h"
#include "common/types.h"

#include "audio/audiostream.h"

namespace Common {
	class SeekableReadStream;
	class String;
}

namespace Audio {

class Codec;

class QuickTimeAudioDecoder : public Common::QuickTimeParser {
public:
	QuickTimeAudioDecoder();
	virtual ~QuickTimeAudioDecoder();

	/**
	 * Load a QuickTime audio file
	 * @param filename	the filename to load
	 */
	bool loadAudioFile(const Common::String &filename);

	/**
	 * Load a QuickTime audio file from a SeekableReadStream
	 * @param stream	the stream to load
	 */
	bool loadAudioStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle);

protected:
	class QuickTimeAudioTrack : public SeekableAudioStream {
	public:
		QuickTimeAudioTrack(QuickTimeAudioDecoder *decoder, Track *parentTrack);
		~QuickTimeAudioTrack();

		// AudioStream API
		int readBuffer(int16 *buffer, const int numSamples);
		bool isStereo() const { return _queue->isStereo(); }
		int getRate() const { return _queue->getRate(); }
		bool endOfData() const;

		// SeekableAudioStream API
		bool seek(const Timestamp &where);
		Timestamp getLength() const;

		// Queue *at least* "length" audio
		// If length is zero, it queues the next logical block of audio whether
		// that be a whole edit or just one chunk within an edit
		void queueAudio(const Timestamp &length = Timestamp());
		Track *getParent() const { return _parentTrack; }
		void queueRemainingAudio();
		bool hasDataInQueue() const { return _samplesQueued != 0; }

	private:
		QuickTimeAudioDecoder *_decoder;
		Track *_parentTrack;
		QueuingAudioStream *_queue;
		uint _curChunk;
		Timestamp _curMediaPos, _skipSamples;
		uint32 _curEdit, _samplesQueued;
		bool _skipAACPrimer;

		QueuingAudioStream *createStream() const;
		AudioStream *readAudioChunk(uint chunk);
		bool isOldDemuxing() const;
		void skipSamples(const Timestamp &length, AudioStream *stream);
		void findEdit(const Timestamp &position);
		bool allDataRead() const;
		void enterNewEdit(const Timestamp &position);
		void queueStream(AudioStream *stream, const Timestamp &length);
		uint32 getAudioChunkSampleCount(uint chunk) const;
		Timestamp getChunkLength(uint chunk, bool skipAACPrimer = false) const;
		uint32 getAACSampleTime(uint32 totalSampleCount, bool skipAACPrimer = false) const;
		Timestamp getCurrentTrackTime() const;
	};

	class AudioSampleDesc : public Common::QuickTimeParser::SampleDesc {
	public:
		AudioSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~AudioSampleDesc();

		bool isAudioCodecSupported() const;

		AudioStream *createAudioStream(Common::SeekableReadStream *stream) const;
		void initCodec();

		// TODO: Make private in the long run
		uint16 _bitsPerSample;
		uint16 _channels;
		uint32 _sampleRate;
		uint32 _samplesPerFrame;
		uint32 _bytesPerFrame;

	private:
		Codec *_codec;
	};

	// Common::QuickTimeParser API
	virtual Common::QuickTimeParser::SampleDesc *readSampleDesc(Track *track, uint32 format);

	void init();

	Common::Array<QuickTimeAudioTrack *> _audioTracks;
};

} // End of namespace Audio

#endif
