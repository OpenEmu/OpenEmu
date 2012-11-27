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

// Disable printf override in common/forbidden.h to avoid
// clashes with pspdebug.h from the PSP SDK.
// That header file uses
//   __attribute__((format(printf,1,2)));
// which gets messed up by our override mechanism; this could
// be avoided by either changing the PSP SDK to use the equally
// legal and valid
//   __attribute__((format(__printf__,1,2)));
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the PSP port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/debug.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/singleton.h"
#include "common/mutex.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"

#include <pspaudiocodec.h>
#include <psputility_modules.h>
#include <pspthreadman.h>
#include <pspsysmem.h>
#include <pspmodulemgr.h>
#include <psputility_avmodules.h>
#include <mad.h>
#include "backends/platform/psp/mp3.h"

//#define DISABLE_PSP_MP3		// to make us use the regular MAD decoder instead

//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__
#include "backends/platform/psp/trace.h"

//#define PRINT_BUFFERS	/* to debug MP3 buffers */

namespace Audio {

class Mp3PspStream;

bool Mp3PspStream::_decoderInit = false;	// has the decoder been initialized
#ifdef DISABLE_PSP_MP3
bool Mp3PspStream::_decoderFail = true;		// pretend the decoder failed
#else
bool Mp3PspStream::_decoderFail = false;	// has the decoder failed to load
#endif

// Arranged in groups of 3 (layers), starting with MPEG-1 and ending with MPEG 2.5
static uint32 mp3SamplesPerFrame[9] = {384, 1152, 1152, 384, 1152, 576, 384, 1152, 576};

// The numbering below doesn't correspond to the way they are in the header
enum {
	MPEG_VER1 = 0,
	MPEG_VER1_HEADER = 0x3,
	MPEG_VER2 = 1,
	MPEG_VER2_HEADER = 0x2,
	MPEG_VER2_5 = 2,
	MPEG_VER2_5_HEADER = 0x0
};

#define HEADER_GET_MPEG_VERSION(x) ((((x)[1])>>3) & 0x3)

bool Mp3PspStream::initDecoder() {
	DEBUG_ENTER_FUNC();

	if (_decoderInit) {
		PSP_ERROR("Already initialized!");
		return true;
	}

	// Based on PSP firmware version, we need to do different things to do Media Engine processing
	uint32 firmware = sceKernelDevkitVersion();
	PSP_DEBUG_PRINT("Firmware version 0x%x\n", firmware);
    if (firmware == 0x01050001){
		if (!loadStartAudioModule((char *)(void *)"flash0:/kd/me_for_vsh.prx",
			PSP_MEMORY_PARTITION_KERNEL)) {
			PSP_ERROR("failed to load me_for_vsh.prx. ME cannot start.\n");
			_decoderFail = true;
			return false;
		}
        if (!loadStartAudioModule((char *)(void *)"flash0:/kd/audiocodec.prx", PSP_MEMORY_PARTITION_KERNEL)) {
			PSP_ERROR("failed to load audiocodec.prx. ME cannot start.\n");
			_decoderFail = true;
			return false;
		}
    } else {
        if (sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC) < 0) {
			PSP_ERROR("failed to load AVCODEC module. ME cannot start.\n");
			_decoderFail = true;
			return false;
		}
	}

	PSP_DEBUG_PRINT("Using PSP's ME for MP3\n");	// important to know this is happening

	_decoderInit = true;
	return true;
}

bool Mp3PspStream::stopDecoder() {
	DEBUG_ENTER_FUNC();

	if (!_decoderInit)
		return true;

	// Based on PSP firmware version, we need to do different things to do Media Engine processing
    if (sceKernelDevkitVersion() == 0x01050001){  // TODO: how do we unload?
/*      if (!unloadAudioModule("flash0:/kd/me_for_vsh.prx", PSP_MEMORY_PARTITION_KERNEL) ||
			!unloadAudioModule("flash0:/kd/audiocodec.prx", PSP_MEMORY_PARTITION_KERNEL) {
			PSP_ERROR("failed to unload audio module\n");
			return false;
		}
*/
    }else{
        if (sceUtilityUnloadModule(PSP_MODULE_AV_AVCODEC) < 0) {
			PSP_ERROR("failed to unload avcodec module\n");
			return false;
		}
	}

	_decoderInit = false;
	return true;
}

//Load a PSP audio module
bool Mp3PspStream::loadStartAudioModule(const char *modname, int partition){
	DEBUG_ENTER_FUNC();

    SceKernelLMOption option;
    SceUID modid;

    memset(&option, 0, sizeof(option));
    option.size = sizeof(option);
    option.mpidtext = partition;
    option.mpiddata = partition;
    option.position = 0;
    option.access = 1;

    modid = sceKernelLoadModule(modname, 0, &option);
    if (modid < 0) {
		PSP_ERROR("Failed to load module %s. Got error 0x%x\n", modname, modid);
        return false;
	}

    int ret = sceKernelStartModule(modid, 0, NULL, NULL, NULL);
	if (ret < 0) {
		PSP_ERROR("Failed to start module %s. Got error 0x%x\n", modname, ret);
        return false;
	}
	return true;
}

// TODO: make parallel function for unloading the 1.50 modules

Mp3PspStream::Mp3PspStream(Common::SeekableReadStream *inStream, DisposeAfterUse::Flag dispose) :
	_inStream(inStream, dispose),
	_pcmLength(0),
	_posInFrame(0),
	_state(MP3_STATE_INIT),
	_length(0, 1000),
	_sampleRate(0),
	_totalTime(mad_timer_zero) {

	DEBUG_ENTER_FUNC();

	assert(_decoderInit);	// must be initialized by now

	// let's leave the buffer guard -- who knows, it may be good?
	memset(_buf, 0, sizeof(_buf));
	memset(_codecInBuffer, 0, sizeof(_codecInBuffer));

	initStream();	// init needed stuff for the stream

	findValidHeader();	// get a first header so we can read basic stuff

	_sampleRate = _header.samplerate;	// copy it before it gets destroyed
	_stereo = (MAD_NCHANNELS(&_header) == 2);

	while (_state != MP3_STATE_EOS)
		findValidHeader();	// get a first header so we can read basic stuff

	_length = Timestamp(mad_timer_count(_totalTime, MAD_UNITS_MILLISECONDS), getRate());

	deinitStream();

	_state = MP3_STATE_INIT;
}

int Mp3PspStream::initStream() {
	DEBUG_ENTER_FUNC();

	if (_state != MP3_STATE_INIT)
		deinitStream();

	// Init MAD
	mad_stream_init(&_stream);
	mad_header_init(&_header);

	// Reset the stream data
	_inStream->seek(0, SEEK_SET);
	_totalTime = mad_timer_zero;
	_posInFrame = 0;

	// Update state
	_state = MP3_STATE_READY;

	// Read the first few sample bytes into the buffer
	readMP3DataIntoBuffer();

	return true;
}

bool Mp3PspStream::initStreamME() {
	// The following will eventually go into the thread

	memset(_codecParams, 0, sizeof(_codecParams));

	// Init the MP3 hardware
	int ret = 0;
	ret = sceAudiocodecCheckNeedMem(_codecParams, 0x1002);
	if (ret < 0) {
		PSP_ERROR("failed to init MP3 ME module. sceAudiocodecCheckNeedMem returned 0x%x.\n", ret);
		return false;
	}
	PSP_DEBUG_PRINT("sceAudiocodecCheckNeedMem returned %d\n", ret);
	ret = sceAudiocodecGetEDRAM(_codecParams, 0x1002);
	if (ret < 0) {
		PSP_ERROR("failed to init MP3 ME module. sceAudiocodecGetEDRAM returned 0x%x.\n", ret);
		return false;
	}
	PSP_DEBUG_PRINT("sceAudioCodecGetEDRAM returned %d\n", ret);

	PSP_DEBUG_PRINT("samplerate[%d]\n", _sampleRate);
	_codecParams[10] = _sampleRate;

	ret = sceAudiocodecInit(_codecParams, 0x1002);
	if (ret < 0) {
		PSP_ERROR("failed to init MP3 ME module. sceAudiocodecInit returned 0x%x.\n", ret);
		return false;
	}

	return true;
}

Mp3PspStream::~Mp3PspStream() {
	DEBUG_ENTER_FUNC();

	deinitStream();
	releaseStreamME(); 	// free the memory used for this stream
}

void Mp3PspStream::deinitStream() {
	DEBUG_ENTER_FUNC();

	if (_state == MP3_STATE_INIT)
		return;

	// Deinit MAD
	mad_header_finish(&_header);
	mad_stream_finish(&_stream);

	_state = MP3_STATE_EOS;
}

void Mp3PspStream::releaseStreamME() {
	sceAudiocodecReleaseEDRAM(_codecParams);
}

void Mp3PspStream::decodeMP3Data() {
	DEBUG_ENTER_FUNC();

	do {
		if (_state == MP3_STATE_INIT) {
			initStream();
			initStreamME();
		}

		if (_state == MP3_STATE_EOS)
			return;

		findValidHeader();	// seach for next valid header

		while (_state == MP3_STATE_READY) {	// not a real 'while'. Just for easy flow
			_stream.error = MAD_ERROR_NONE;

			uint32 frame_size = _stream.next_frame - _stream.this_frame;

			updatePcmLength(); // Retrieve the number of PCM samples.
							   // We seem to change this, so it needs to be dynamic

			PSP_DEBUG_PRINT("MP3 frame size[%d]. pcmLength[%d]\n", frame_size, _pcmLength);

			memcpy(_codecInBuffer, _stream.this_frame, frame_size);	// we need it aligned

			// set up parameters for ME
			_codecParams[6] = (unsigned long)_codecInBuffer;
			_codecParams[8] = (unsigned long)_pcmSamples;
			_codecParams[7] = frame_size;
			_codecParams[9] = _pcmLength * 2;	// x2 for stereo, though this one's not so important

			// debug
#ifdef PRINT_BUFFERS
			PSP_DEBUG_PRINT("mp3 frame:\n");
			for (int i=0; i < (int)frame_size; i++) {
				PSP_DEBUG_PRINT_SAMELN("%x ", _codecInBuffer[i]);
			}
			PSP_DEBUG_PRINT("\n");
#endif
			// Decode the next frame
			// This function blocks. We'll want to put it in a thread
			int ret = sceAudiocodecDecode(_codecParams, 0x1002);
			if (ret < 0) {
				PSP_INFO_PRINT("failed to decode MP3 data in ME. sceAudiocodecDecode returned 0x%x\n", ret);
			}

#ifdef PRINT_BUFFERS
			PSP_DEBUG_PRINT("PCM frame:\n");
			for (int i=0; i < (int)_codecParams[9]; i+=2) {	// changed from i+=2
				PSP_DEBUG_PRINT_SAMELN("%d ", (int16)_pcmSamples[i]);
			}
			PSP_DEBUG_PRINT("\n");
#endif
			_posInFrame = 0;
			break;
		}
	} while (_state != MP3_STATE_EOS && _stream.error == MAD_ERROR_BUFLEN);

	if (_stream.error != MAD_ERROR_NONE)	// catch EOS
		_state = MP3_STATE_EOS;
}

inline void Mp3PspStream::updatePcmLength() {
	uint32 mpegVer = HEADER_GET_MPEG_VERSION(_stream.this_frame);	// sadly, MAD can't do this for us
	PSP_DEBUG_PRINT("mpeg ver[%x]\n", mpegVer);
	switch (mpegVer) {
		case MPEG_VER1_HEADER:
			mpegVer = MPEG_VER1;
			break;
		case MPEG_VER2_HEADER:
			mpegVer = MPEG_VER2;
			break;
		case MPEG_VER2_5_HEADER:
			mpegVer = MPEG_VER2_5;
			break;
		default:
			PSP_ERROR("Unknown MPEG version %x\n", mpegVer);
			break;
	}
	PSP_DEBUG_PRINT("layer[%d]\n", _header.layer);
	_pcmLength = mp3SamplesPerFrame[(mpegVer * 3) + _header.layer - 1];
}

void Mp3PspStream::readMP3DataIntoBuffer() {
	DEBUG_ENTER_FUNC();

	uint32 remaining = 0;

	// Give up immediately if we already used up all data in the stream
	if (_inStream->eos()) {
		_state = MP3_STATE_EOS;
		return;
	}

	if (_stream.next_frame) {
		// If there is still data in the MAD stream, we need to preserve it.
		// Note that we use memmove, as we are reusing the same buffer,
		// and hence the data regions we copy from and to may overlap.
		remaining = _stream.bufend - _stream.next_frame;
		assert(remaining < BUFFER_SIZE);	// Paranoia check
		memmove(_buf, _stream.next_frame, remaining);	// TODO: may want another buffer
	}

	// Try to read the next block
	uint32 size = _inStream->read(_buf + remaining, BUFFER_SIZE - remaining);
	if (size <= 0) {
		_state = MP3_STATE_EOS;
		return;
	}

	// Feed the data we just read into the stream decoder
	_stream.error = MAD_ERROR_NONE;
	mad_stream_buffer(&_stream, _buf, size + remaining);	// just setup the pointers
}

bool Mp3PspStream::seek(const Timestamp &where) {
	DEBUG_ENTER_FUNC();

	if (where == _length) {
		_state = MP3_STATE_EOS;
		return true;
	} else if (where > _length) {
		return false;
	}

	const uint32 time = where.msecs();

	mad_timer_t destination;
	mad_timer_set(&destination, time / 1000, time % 1000, 1000);

	// Important to release and re-init the ME
	releaseStreamME();
	initStreamME();

	// Check if we need to rewind
	if (_state != MP3_STATE_READY || mad_timer_compare(destination, _totalTime) < 0) {
		initStream();
	}

	// Skip ahead
	while (mad_timer_compare(destination, _totalTime) > 0 && _state != MP3_STATE_EOS)
		findValidHeader();

	return (_state != MP3_STATE_EOS);
}

// Seek in the stream, finding the next valid header
void Mp3PspStream::findValidHeader() {
	DEBUG_ENTER_FUNC();

	if (_state != MP3_STATE_READY)
		return;

	// If necessary, load more data into the stream decoder
	if (_stream.error == MAD_ERROR_BUFLEN)
		readMP3DataIntoBuffer();

	while (_state != MP3_STATE_EOS) {
		_stream.error = MAD_ERROR_NONE;

		// Decode the next header.
		if (mad_header_decode(&_header, &_stream) == -1) {
			if (_stream.error == MAD_ERROR_BUFLEN) {
				readMP3DataIntoBuffer();  // Read more data
				continue;
			} else if (MAD_RECOVERABLE(_stream.error)) {
				debug(6, "MP3PSPStream: Recoverable error in mad_header_decode (%s)", mad_stream_errorstr(&_stream));
				continue;
			} else {
				warning("MP3PSPStream: Unrecoverable error in mad_header_decode (%s)", mad_stream_errorstr(&_stream));
				break;
			}
		}

		// Sum up the total playback time so far
		mad_timer_add(&_totalTime, _header.duration);
		break;
	}

	if (_stream.error != MAD_ERROR_NONE)
		_state = MP3_STATE_EOS;
}

int Mp3PspStream::readBuffer(int16 *buffer, const int numSamples) {
	DEBUG_ENTER_FUNC();

	int samples = 0;
#ifdef PRINT_BUFFERS
	int16 *debugBuffer = buffer;
#endif

	// Keep going as long as we have input available
	while (samples < numSamples && _state != MP3_STATE_EOS) {
		const int len = MIN(numSamples, samples + (int)(_pcmLength - _posInFrame) * MAD_NCHANNELS(&_header));

		while (samples < len) {
			*buffer++ = _pcmSamples[_posInFrame << 1];
			samples++;
			if (MAD_NCHANNELS(&_header) == 2) {
				*buffer++ = _pcmSamples[(_posInFrame << 1) + 1];
				samples++;
			}
			_posInFrame++;	// always skip an extra sample since ME always outputs stereo
		}

		if (_posInFrame >= _pcmLength) {
			// We used up all PCM data in the current frame -- read & decode more
			decodeMP3Data();
		}
	}

#ifdef PRINT_BUFFERS
		PSP_INFO_PRINT("buffer:\n");
		for (int i = 0; i<numSamples; i++)
			PSP_INFO_PRINT("%d ", debugBuffer[i]);
		PSP_INFO_PRINT("\n\n");
#endif

	return samples;
}

} // End of namespace Audio
