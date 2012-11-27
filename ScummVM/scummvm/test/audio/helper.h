#ifndef TEST_SOUND_HELPER_H
#define TEST_SOUND_HELPER_H

#include "audio/decoders/raw.h"

#include "common/stream.h"
#include "common/endian.h"

#include <math.h>
#include <limits>

template<typename T>
static T *createSine(const int sampleRate, const int time) {
	T *sine = (T *)malloc(sizeof(T) * time * sampleRate);

	const bool isUnsigned = !std::numeric_limits<T>::is_signed;
	const T xorMask = isUnsigned ? (1 << (std::numeric_limits<T>::digits - 1)) : 0;
	const T maxValue = std::numeric_limits<T>::max() ^ xorMask;

	for (int i = 0; i < time * sampleRate; ++i)
		sine[i] = ((T)(sin((double)i / sampleRate * 2 * M_PI) * maxValue)) ^ xorMask;

	return sine;
}

template<typename T>
static Audio::SeekableAudioStream *createSineStream(const int sampleRate, const int time, int16 **comp, bool le, bool isStereo) {
	T *sine = createSine<T>(sampleRate, time * (isStereo ? 2 : 1));

	const bool isUnsigned = !std::numeric_limits<T>::is_signed;
	const T xorMask = isUnsigned ? (1 << (std::numeric_limits<T>::digits - 1)) : 0;
	const bool is16Bits = (sizeof(T) == 2);
	assert(sizeof(T) == 2 || sizeof(T) == 1);

	const int samples = sampleRate * time * (isStereo ? 2 : 1);

	if (comp) {
		*comp = new int16[samples];
		for (int i = 0; i < samples; ++i) {
			if (is16Bits)
				(*comp)[i] = sine[i] ^ xorMask;
			else
				(*comp)[i] = (sine[i] ^ xorMask) << 8;
		}
	}

	if (is16Bits) {
		if (le) {
			for (int i = 0; i < samples; ++i)
				WRITE_LE_UINT16(&sine[i], sine[i]);
		} else {
			for (int i = 0; i < samples; ++i)
				WRITE_BE_UINT16(&sine[i], sine[i]);
		}
	}

	Audio::SeekableAudioStream *s = 0;
	Common::SeekableReadStream *sD = new Common::MemoryReadStream((const byte *)sine, sizeof(T) * samples, DisposeAfterUse::YES);
	s = Audio::makeRawStream(sD, sampleRate,
	                             (is16Bits ? Audio::FLAG_16BITS : 0)
	                             | (isUnsigned ? Audio::FLAG_UNSIGNED : 0)
	                             | (le ? Audio::FLAG_LITTLE_ENDIAN : 0)
	                             | (isStereo ? Audio::FLAG_STEREO : 0));

	return s;
}

#endif
