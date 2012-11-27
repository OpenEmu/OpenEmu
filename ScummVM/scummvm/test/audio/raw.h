#include <cxxtest/TestSuite.h>

#include "audio/decoders/raw.h"

#include "helper.h"

class RawStreamTestSuite : public CxxTest::TestSuite
{
private:
	template<typename T>
	void readBufferTestTemplate(const int sampleRate, const int time, const bool le, const bool isStereo) {
		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, &sine, le, isStereo);

		const int totalSamples = sampleRate * time * (isStereo ? 2 : 1);
		int16 *buffer = new int16[totalSamples];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, totalSamples), totalSamples);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sizeof(int16) * totalSamples), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}

public:
	void test_read_buffer_8_bit_signed_mono() {
		readBufferTestTemplate<int8>(11025, 2, false, false);
	}

	void test_read_buffer_8_bit_signed_stereo() {
		readBufferTestTemplate<int8>(11025, 2, false, true);
	}

	void test_read_buffer_8_bit_unsigned_mono() {
		readBufferTestTemplate<uint8>(11025, 2, false, false);
	}

	void test_read_buffer_16_bit_signed_be_mono() {
		readBufferTestTemplate<int16>(11025, 2, false, false);
	}

	void test_read_buffer_16_bit_signed_be_stereo() {
		readBufferTestTemplate<int16>(11025, 2, false, true);
	}

	void test_read_buffer_16_bit_unsigned_be_mono() {
		readBufferTestTemplate<uint16>(11025, 2, false, false);
	}

	void test_read_buffer_16_bit_unsigned_be_stereo() {
		readBufferTestTemplate<uint16>(11025, 2, false, true);
	}

	void test_read_buffer_16_bit_signed_le_mono() {
		readBufferTestTemplate<int16>(11025, 2, true, false);
	}

	void test_read_buffer_16_bit_signed_le_stereo() {
		readBufferTestTemplate<int16>(11025, 2, true, true);
	}

	void test_read_buffer_16_bit_unsigned_le_mono() {
		readBufferTestTemplate<uint16>(11025, 2, true, false);
	}

	void test_read_buffer_16_bit_unsigned_le_stereo() {
		readBufferTestTemplate<uint16>(11025, 2, true, true);
	}

private:
	void partialReadTest() {
		const int sampleRate = 11025;
		const int time = 4;

		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, &sine, false, false);
		int16 *buffer = new int16[sampleRate * time];

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate), sampleRate);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sampleRate), 0);
		TS_ASSERT_EQUALS(s->endOfData(), false);

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * 2), sampleRate * 2);
		TS_ASSERT_EQUALS(memcmp(sine + sampleRate, buffer, sampleRate * 2), 0);
		TS_ASSERT_EQUALS(s->endOfData(), false);

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate), sampleRate);
		TS_ASSERT_EQUALS(memcmp(sine + sampleRate * 3, buffer, sampleRate), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}
public:
	void test_partial_read() {
		partialReadTest();
	}

private:
	void readAfterEndTest() {
		const int sampleRate = 11025;
		const int time = 1;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, 0, false, false);
		int16 *buffer = new int16[sampleRate * time];

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}

public:
	void test_read_after_end() {
		readAfterEndTest();
	}

private:
	void rewindTest() {
		const int sampleRate = 11025;
		const int time = 2;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, 0, false, false);
		int16 *buffer = new int16[sampleRate * time];

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		s->rewind();
		TS_ASSERT_EQUALS(s->endOfData(), false);

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}
public:
	void test_rewind() {
		rewindTest();
	}

private:
	void lengthTest() {
		int sampleRate = 0;
		const int time = 4;

		Audio::SeekableAudioStream *s = 0;

		// 11025 Hz tests
		sampleRate = 11025;
		s = createSineStream<int8>(sampleRate, time, 0, false, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		s = createSineStream<uint16>(sampleRate, time, 0, false, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		// 48000 Hz tests
		sampleRate = 48000;
		s = createSineStream<int8>(sampleRate, time, 0, false, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		s = createSineStream<uint16>(sampleRate, time, 0, true, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		// 11840 Hz tests
		sampleRate = 11840;
		s = createSineStream<int8>(sampleRate, time, 0, false, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		s = createSineStream<uint16>(sampleRate, time, 0, false, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		// 11111 Hz tests
		sampleRate = 11111;
		s = createSineStream<int8>(sampleRate, time, 0, false, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		s = createSineStream<uint16>(sampleRate, time, 0, false, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		// 22050 Hz stereo test
		sampleRate = 22050;
		s = createSineStream<int8>(sampleRate, time, 0, false, true);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		s = createSineStream<uint16>(sampleRate, time, 0, true, true);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;
	}

public:
	void test_length() {
		lengthTest();
	}

private:
	void seekTest(const int sampleRate, const int time, const bool isStereo) {
		const int totalFrames = sampleRate * time * (isStereo ? 2 : 1);
		int readData = 0, offset = 0;

		int16 *buffer = new int16[totalFrames];
		Audio::SeekableAudioStream *s = 0;
		int16 *sine = 0;

		s = createSineStream<int8>(sampleRate, time, &sine, false, isStereo);

		// Seek to 500ms
		const Audio::Timestamp a(0, 1, 2);
		offset = Audio::convertTimeToStreamPos(a, sampleRate, isStereo).totalNumberOfFrames();
		readData = totalFrames - offset;

		TS_ASSERT_EQUALS(s->seek(a), true);
		TS_ASSERT_EQUALS(s->endOfData(), false);
		TS_ASSERT_EQUALS(s->readBuffer(buffer, readData), readData);
		TS_ASSERT_EQUALS(memcmp(buffer, sine + offset, readData * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		// Seek to 3/4 of a second
		const Audio::Timestamp b(0, 3, 4);
		offset = Audio::convertTimeToStreamPos(b, sampleRate, isStereo).totalNumberOfFrames();
		readData = totalFrames - offset;

		TS_ASSERT_EQUALS(s->seek(b), true);
		TS_ASSERT_EQUALS(s->endOfData(), false);
		TS_ASSERT_EQUALS(s->readBuffer(buffer, readData), readData);
		TS_ASSERT_EQUALS(memcmp(buffer, sine + offset, readData * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		// Seek to the start of the stream
		TS_ASSERT_EQUALS(s->seek(0), true);
		TS_ASSERT_EQUALS(s->endOfData(), false);

		// Seek to the mid of the stream
		const Audio::Timestamp c(time * 1000 / 2, 1000);
		offset = Audio::convertTimeToStreamPos(c, sampleRate, isStereo).totalNumberOfFrames();
		readData = totalFrames - offset;

		TS_ASSERT_EQUALS(s->seek(c), true);
		TS_ASSERT_EQUALS(s->endOfData(), false);
		TS_ASSERT_EQUALS(s->readBuffer(buffer, readData), readData);
		TS_ASSERT_EQUALS(memcmp(buffer, sine + offset, readData * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		// Seek to the 1/4th of the last second of the stream
		const Audio::Timestamp d(time - 1, 1, 4);
		offset = Audio::convertTimeToStreamPos(d, sampleRate, isStereo).totalNumberOfFrames();
		readData = totalFrames - offset;

		TS_ASSERT_EQUALS(s->seek(d), true);
		TS_ASSERT_EQUALS(s->endOfData(), false);
		TS_ASSERT_EQUALS(s->readBuffer(buffer, readData), readData);
		TS_ASSERT_EQUALS(memcmp(buffer, sine + offset, readData * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		// Try to seek after the end of the stream
		TS_ASSERT_EQUALS(s->seek(Audio::Timestamp(time, 1, 100000)), false);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		// Try to seek exactly at the end of the stream
		TS_ASSERT_EQUALS(s->seek(Audio::Timestamp(time * 1000, 1000)), true);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete s;
		delete[] buffer;
	}

public:
	void test_seek_mono() {
		seekTest(11025, 2, false);
	}

	void test_seek_stereo() {
		seekTest(11025, 2, true);
	}
};
