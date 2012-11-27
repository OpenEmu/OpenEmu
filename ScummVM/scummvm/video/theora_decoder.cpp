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

/*
 * Source is based on the player example from libvorbis package,
 * available at: http://svn.xiph.org/trunk/theora/examples/player_example.c
 *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.
 *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2009
 * by the Xiph.Org Foundation and contributors http://www.xiph.org/
 *
 */

#include "video/theora_decoder.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/pixelformat.h"
#include "graphics/yuv_to_rgb.h"

namespace Video {

TheoraDecoder::TheoraDecoder(Audio::Mixer::SoundType soundType) : _soundType(soundType) {
	_fileStream = 0;

	_videoTrack = 0;
	_audioTrack = 0;
	_hasVideo = _hasAudio = false;
}

TheoraDecoder::~TheoraDecoder() {
	close();
}

bool TheoraDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_fileStream = stream;

	// start up Ogg stream synchronization layer
	ogg_sync_init(&_oggSync);

	// init supporting Vorbis structures needed in header parsing
	vorbis_info_init(&_vorbisInfo);
	vorbis_comment vorbisComment;
	vorbis_comment_init(&vorbisComment);

	// init supporting Theora structures needed in header parsing
	th_info theoraInfo;
	th_info_init(&theoraInfo);
	th_comment theoraComment;
	th_comment_init(&theoraComment);
	th_setup_info *theoraSetup = 0;

	uint theoraPackets = 0, vorbisPackets = 0;

	// Ogg file open; parse the headers
	// Only interested in Vorbis/Theora streams
	bool foundHeader = false;
	while (!foundHeader) {
		int ret = bufferData();

		if (ret == 0)
			break; // FIXME: Shouldn't this error out?

		while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0) {
			ogg_stream_state test;

			// is this a mandated initial header? If not, stop parsing
			if (!ogg_page_bos(&_oggPage)) {
				// don't leak the page; get it into the appropriate stream
				queuePage(&_oggPage);
				foundHeader = true;
				break;
			}

			ogg_stream_init(&test, ogg_page_serialno(&_oggPage));
			ogg_stream_pagein(&test, &_oggPage);
			ogg_stream_packetout(&test, &_oggPacket);

			// identify the codec: try theora
			if (theoraPackets == 0 && th_decode_headerin(&theoraInfo, &theoraComment, &theoraSetup, &_oggPacket) >= 0) {
				// it is theora
				memcpy(&_theoraOut, &test, sizeof(test));
				theoraPackets = 1;
				_hasVideo = true;
			} else if (vorbisPackets == 0 && vorbis_synthesis_headerin(&_vorbisInfo, &vorbisComment, &_oggPacket) >= 0) {
				// it is vorbis
				memcpy(&_vorbisOut, &test, sizeof(test));
				vorbisPackets = 1;
				_hasAudio = true;
			} else {
				// whatever it is, we don't care about it
				ogg_stream_clear(&test);
			}
		}
		// fall through to non-bos page parsing
	}

	// we're expecting more header packets.
	while ((theoraPackets && theoraPackets < 3) || (vorbisPackets && vorbisPackets < 3)) {
		int ret;

		// look for further theora headers
		while (theoraPackets && (theoraPackets < 3) && (ret = ogg_stream_packetout(&_theoraOut, &_oggPacket))) {
			if (ret < 0)
				error("Error parsing Theora stream headers; corrupt stream?");

			if (!th_decode_headerin(&theoraInfo, &theoraComment, &theoraSetup, &_oggPacket))
				error("Error parsing Theora stream headers; corrupt stream?");

			theoraPackets++;
		}

		// look for more vorbis header packets
		while (vorbisPackets && (vorbisPackets < 3) && (ret = ogg_stream_packetout(&_vorbisOut, &_oggPacket))) {
			if (ret < 0)
				error("Error parsing Vorbis stream headers; corrupt stream?");

			if (vorbis_synthesis_headerin(&_vorbisInfo, &vorbisComment, &_oggPacket))
				error("Error parsing Vorbis stream headers; corrupt stream?");

			vorbisPackets++;

			if (vorbisPackets == 3)
				break;
		}

		// The header pages/packets will arrive before anything else we
		// care about, or the stream is not obeying spec

		if (ogg_sync_pageout(&_oggSync, &_oggPage) > 0) {
			queuePage(&_oggPage); // demux into the appropriate stream
		} else {
			ret = bufferData(); // someone needs more data

			if (ret == 0)
				error("End of file while searching for codec headers.");
		}
	}

	// And now we have it all. Initialize decoders next
	if (_hasVideo) {
		_videoTrack = new TheoraVideoTrack(getDefaultHighColorFormat(), theoraInfo, theoraSetup);
		addTrack(_videoTrack);
	}

	th_info_clear(&theoraInfo);
	th_comment_clear(&theoraComment);
	th_setup_free(theoraSetup);

	if (_hasAudio) {
		_audioTrack = new VorbisAudioTrack(_soundType, _vorbisInfo);

		// Get enough audio data to start us off
		while (!_audioTrack->hasAudio()) {
			// Queue more data
			bufferData();
			while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0)
				queuePage(&_oggPage);

			queueAudio();
		}

		addTrack(_audioTrack);
	}

	vorbis_comment_clear(&vorbisComment);

	return true;
}

void TheoraDecoder::close() {
	VideoDecoder::close();

	if (!_fileStream)
		return;

	if (_videoTrack) {
		ogg_stream_clear(&_theoraOut);
		_videoTrack = 0;
	}

	if (_audioTrack) {
		ogg_stream_clear(&_vorbisOut);
		_audioTrack = 0;
	}

	ogg_sync_clear(&_oggSync);
	vorbis_info_clear(&_vorbisInfo);

	delete _fileStream;
	_fileStream = 0;

	_hasVideo = _hasAudio = false;
}

void TheoraDecoder::readNextPacket() {
	// First, let's get our frame
	if (_hasVideo) {
		while (!_videoTrack->endOfTrack()) {
			// theora is one in, one out...
			if (ogg_stream_packetout(&_theoraOut, &_oggPacket) > 0) {
				if (_videoTrack->decodePacket(_oggPacket))
					break;
			} else if (_theoraOut.e_o_s || _fileStream->eos()) {
				// If we can't get any more frames, we're done.
				_videoTrack->setEndOfVideo();
			} else {
				// Queue more data
				bufferData();
				while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0)
					queuePage(&_oggPage);
			}

			// Update audio if we can
			queueAudio();
		}
	}

	// Then make sure we have enough audio buffered
	ensureAudioBufferSize();
}

TheoraDecoder::TheoraVideoTrack::TheoraVideoTrack(const Graphics::PixelFormat &format, th_info &theoraInfo, th_setup_info *theoraSetup) {
	_theoraDecode = th_decode_alloc(&theoraInfo, theoraSetup);

	if (theoraInfo.pixel_fmt != TH_PF_420)
		error("Only theora YUV420 is supported");

	int postProcessingMax;
	th_decode_ctl(_theoraDecode, TH_DECCTL_GET_PPLEVEL_MAX, &postProcessingMax, sizeof(postProcessingMax));
	th_decode_ctl(_theoraDecode, TH_DECCTL_SET_PPLEVEL, &postProcessingMax, sizeof(postProcessingMax));

	_surface.create(theoraInfo.frame_width, theoraInfo.frame_height, format);

	// Set up a display surface
	_displaySurface.pixels = _surface.getBasePtr(theoraInfo.pic_x, theoraInfo.pic_y);
	_displaySurface.w = theoraInfo.pic_width;
	_displaySurface.h = theoraInfo.pic_height;
	_displaySurface.format = format;
	_displaySurface.pitch = _surface.pitch;

	// Set the frame rate
	_frameRate = Common::Rational(theoraInfo.fps_numerator, theoraInfo.fps_denominator);

	_endOfVideo = false;
	_nextFrameStartTime = 0.0;
	_curFrame = -1;
}

TheoraDecoder::TheoraVideoTrack::~TheoraVideoTrack() {
	th_decode_free(_theoraDecode);

	_surface.free();
	_displaySurface.pixels = 0;
}

bool TheoraDecoder::TheoraVideoTrack::decodePacket(ogg_packet &oggPacket) {
	if (th_decode_packetin(_theoraDecode, &oggPacket, 0) == 0) {
		_curFrame++;

		// Convert YUV data to RGB data
		th_ycbcr_buffer yuv;
		th_decode_ycbcr_out(_theoraDecode, yuv);
		translateYUVtoRGBA(yuv);

		double time = th_granule_time(_theoraDecode, oggPacket.granulepos);

		// We need to calculate when the next frame should be shown
		// This is all in floating point because that's what the Ogg code gives us
		// Ogg is a lossy container format, so it doesn't always list the time to the
		// next frame. In such cases, we need to calculate it ourselves.
		if (time == -1.0)
			_nextFrameStartTime += _frameRate.getInverse().toDouble();
		else
			_nextFrameStartTime = time;

		return true;
	}

	return false;
}

enum TheoraYUVBuffers {
	kBufferY = 0,
	kBufferU = 1,
	kBufferV = 2
};

void TheoraDecoder::TheoraVideoTrack::translateYUVtoRGBA(th_ycbcr_buffer &YUVBuffer) {
	// Width and height of all buffers have to be divisible by 2.
	assert((YUVBuffer[kBufferY].width & 1) == 0);
	assert((YUVBuffer[kBufferY].height & 1) == 0);
	assert((YUVBuffer[kBufferU].width & 1) == 0);
	assert((YUVBuffer[kBufferV].width & 1) == 0);

	// UV images have to have a quarter of the Y image resolution
	assert(YUVBuffer[kBufferU].width == YUVBuffer[kBufferY].width >> 1);
	assert(YUVBuffer[kBufferV].width == YUVBuffer[kBufferY].width >> 1);
	assert(YUVBuffer[kBufferU].height == YUVBuffer[kBufferY].height >> 1);
	assert(YUVBuffer[kBufferV].height == YUVBuffer[kBufferY].height >> 1);

	YUVToRGBMan.convert420(&_surface, Graphics::YUVToRGBManager::kScaleITU, YUVBuffer[kBufferY].data, YUVBuffer[kBufferU].data, YUVBuffer[kBufferV].data, YUVBuffer[kBufferY].width, YUVBuffer[kBufferY].height, YUVBuffer[kBufferY].stride, YUVBuffer[kBufferU].stride);
}

static vorbis_info *info = 0;

TheoraDecoder::VorbisAudioTrack::VorbisAudioTrack(Audio::Mixer::SoundType soundType, vorbis_info &vorbisInfo) : _soundType(soundType) {
	vorbis_synthesis_init(&_vorbisDSP, &vorbisInfo);
	vorbis_block_init(&_vorbisDSP, &_vorbisBlock);
	info = &vorbisInfo;

	_audStream = Audio::makeQueuingAudioStream(vorbisInfo.rate, vorbisInfo.channels);

	_audioBufferFill = 0;
	_audioBuffer = 0;
	_endOfAudio = false;
}

TheoraDecoder::VorbisAudioTrack::~VorbisAudioTrack() {
	vorbis_dsp_clear(&_vorbisDSP);
	vorbis_block_clear(&_vorbisBlock);
	delete _audStream;
	free(_audioBuffer);
}

Audio::AudioStream *TheoraDecoder::VorbisAudioTrack::getAudioStream() const {
	return _audStream;
}

#define AUDIOFD_FRAGSIZE 10240

static double rint(double v) {
	return floor(v + 0.5);
}

bool TheoraDecoder::VorbisAudioTrack::decodeSamples() {
	float **pcm;

	// if there's pending, decoded audio, grab it
	int ret = vorbis_synthesis_pcmout(&_vorbisDSP, &pcm);

	if (ret > 0) {
		if (!_audioBuffer) {
			_audioBuffer = (ogg_int16_t *)malloc(AUDIOFD_FRAGSIZE * sizeof(ogg_int16_t));
			assert(_audioBuffer);
		}

		int channels = _audStream->isStereo() ? 2 : 1;
		int count = _audioBufferFill / 2;
		int maxsamples = ((AUDIOFD_FRAGSIZE - _audioBufferFill) / channels) >> 1;
		int i;

		for (i = 0; i < ret && i < maxsamples; i++) {
			for (int j = 0; j < channels; j++) {
				int val = CLIP((int)rint(pcm[j][i] * 32767.f), -32768, 32767);
				_audioBuffer[count++] = val;
			}
		}

		vorbis_synthesis_read(&_vorbisDSP, i);
		_audioBufferFill += (i * channels) << 1;

		if (_audioBufferFill == AUDIOFD_FRAGSIZE) {
			byte flags = Audio::FLAG_16BITS;

			if (_audStream->isStereo())
				flags |= Audio::FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
			flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif

			_audStream->queueBuffer((byte *)_audioBuffer, AUDIOFD_FRAGSIZE, DisposeAfterUse::YES, flags);

			// The audio mixer is now responsible for the old audio buffer.
			// We need to create a new one.
			_audioBuffer = 0;
			_audioBufferFill = 0;
		}

		return true;
	}

	return false;
}

bool TheoraDecoder::VorbisAudioTrack::hasAudio() const {
	return _audStream->numQueuedStreams() > 0;
}

bool TheoraDecoder::VorbisAudioTrack::needsAudio() const {
	// TODO: 5 is very arbitrary. We probably should do something like QuickTime does.
	return !_endOfAudio && _audStream->numQueuedStreams() < 5;
}

void TheoraDecoder::VorbisAudioTrack::synthesizePacket(ogg_packet &oggPacket) {
	if (vorbis_synthesis(&_vorbisBlock, &oggPacket) == 0) // test for success
		vorbis_synthesis_blockin(&_vorbisDSP, &_vorbisBlock);
}

void TheoraDecoder::queuePage(ogg_page *page) {
	if (_hasVideo)
		ogg_stream_pagein(&_theoraOut, page);

	if (_hasAudio)
		ogg_stream_pagein(&_vorbisOut, page);
}

int TheoraDecoder::bufferData() {
	char *buffer = ogg_sync_buffer(&_oggSync, 4096);
	int bytes = _fileStream->read(buffer, 4096);

	ogg_sync_wrote(&_oggSync, bytes);

	return bytes;
}

bool TheoraDecoder::queueAudio() {
	if (!_hasAudio)
		return false;

	bool queuedAudio = false;

	for (;;) {
		if (_audioTrack->decodeSamples()) {
			// we queued some pending audio
			queuedAudio = true;
		} else if (ogg_stream_packetout(&_vorbisOut, &_oggPacket) > 0) {
			// no pending audio; is there a pending packet to decode?
			_audioTrack->synthesizePacket(_oggPacket);
		} else {
			// we've buffered all we have, break out for now
			break;
		}
	}

	return queuedAudio;
}

void TheoraDecoder::ensureAudioBufferSize() {
	if (!_hasAudio)
		return;

	// Force at least some audio to be buffered
	while (_audioTrack->needsAudio()) {
		bufferData();
		while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0)
			queuePage(&_oggPage);

		bool queuedAudio = queueAudio();
		if ((_vorbisOut.e_o_s  || _fileStream->eos()) && !queuedAudio) {
			_audioTrack->setEndOfAudio();
			break;
		}
	}
}

} // End of namespace Video
