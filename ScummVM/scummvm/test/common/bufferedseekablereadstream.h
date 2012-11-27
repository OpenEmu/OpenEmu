#include <cxxtest/TestSuite.h>

#include "common/memstream.h"
#include "common/bufferedstream.h"

class BufferedSeekableReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_traverse() {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		Common::SeekableReadStream &ssrs
			= *Common::wrapBufferedSeekableReadStream(&ms, 4, DisposeAfterUse::NO);

		byte i, b;
		for (i = 0; i < 10; ++i) {
			TS_ASSERT(!ssrs.eos());

			TS_ASSERT_EQUALS(i, ssrs.pos());

			ssrs.read(&b, 1);
			TS_ASSERT_EQUALS(i, b);
		}

		TS_ASSERT(!ssrs.eos());

		TS_ASSERT_EQUALS((uint)0, ssrs.read(&b, 1));
		TS_ASSERT(ssrs.eos());

		delete &ssrs;
	}

	void test_seek() {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		Common::SeekableReadStream &ssrs
			= *Common::wrapBufferedSeekableReadStream(&ms, 4, DisposeAfterUse::NO);
		byte b;

		TS_ASSERT_EQUALS(ssrs.pos(), 0);

		ssrs.seek(1, SEEK_SET);
		TS_ASSERT_EQUALS(ssrs.pos(), 1);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 1);

		ssrs.seek(5, SEEK_CUR);
		TS_ASSERT_EQUALS(ssrs.pos(), 7);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 7);

		ssrs.seek(-3, SEEK_CUR);
		TS_ASSERT_EQUALS(ssrs.pos(), 5);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 5);

		ssrs.seek(0, SEEK_END);
		TS_ASSERT_EQUALS(ssrs.pos(), 10);
		TS_ASSERT(!ssrs.eos());
		b = ssrs.readByte();
		TS_ASSERT(ssrs.eos());

		ssrs.seek(-3, SEEK_END);
		TS_ASSERT(!ssrs.eos());
		TS_ASSERT_EQUALS(ssrs.pos(), 7);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 7);

		ssrs.seek(-8, SEEK_END);
		TS_ASSERT_EQUALS(ssrs.pos(), 2);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 2);

		delete &ssrs;
	}
};
