#include <cxxtest/TestSuite.h>

#include "common/memstream.h"
#include "common/bufferedstream.h"

class BufferedReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_traverse() {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		// Use a buffer size of 4 -- note that 10 % 4 != 0,
		// so we test what happens if the cache can't be completely
		// refilled.
		Common::ReadStream &srs = *Common::wrapBufferedReadStream(&ms, 4, DisposeAfterUse::NO);

		byte i, b;
		for (i = 0; i < 10; ++i) {
			TS_ASSERT(!srs.eos());

			b = srs.readByte();
			TS_ASSERT_EQUALS(i, b);
		}

		TS_ASSERT(!srs.eos());

		b = srs.readByte();

		TS_ASSERT(srs.eos());

		delete &srs;
	}

	void test_traverse2() {
		byte contents[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
		Common::MemoryReadStream ms(contents, 9);

		Common::ReadStream &brs = *Common::wrapBufferedReadStream(&ms, 4, DisposeAfterUse::NO);

		// Traverse the stream with reads of 2 bytes. The size is not
		// a multiple of 2, so we can test the final partial read.

		byte i, b[2];
		for (i = 0; i < 4; ++i) {
			TS_ASSERT(!brs.eos());

			int n = brs.read(b, 2);
			TS_ASSERT_EQUALS(n, 2);
		}

		TS_ASSERT(!brs.eos());

		int n = brs.read(b, 2);
		TS_ASSERT_EQUALS(n, 1);

		TS_ASSERT(brs.eos());

		delete &brs;
	}
};
