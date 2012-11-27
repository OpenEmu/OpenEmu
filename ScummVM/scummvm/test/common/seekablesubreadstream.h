#include <cxxtest/TestSuite.h>

#include "common/memstream.h"
#include "common/substream.h"

class SeekableSubReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_traverse() {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		int start = 2, end = 8;

		Common::SeekableSubReadStream ssrs(&ms, start, end);

		int i;
		byte b;
		for (i = start; i < end; ++i) {
			TS_ASSERT(!ssrs.eos());

			TS_ASSERT_EQUALS(i - start, ssrs.pos());

			ssrs.read(&b, 1);
			TS_ASSERT_EQUALS(i, b);
		}

		TS_ASSERT(!ssrs.eos());
		TS_ASSERT_EQUALS((uint)0, ssrs.read(&b, 1));
		TS_ASSERT(ssrs.eos());
	}

	void test_seek() {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		Common::SeekableSubReadStream ssrs(&ms, 1, 9);
		byte b;

		TS_ASSERT_EQUALS(ssrs.pos(), 0);

		ssrs.seek(1, SEEK_SET);
		TS_ASSERT_EQUALS(ssrs.pos(), 1);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 2);

		ssrs.seek(5, SEEK_CUR);
		TS_ASSERT_EQUALS(ssrs.pos(), 7);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 8);

		ssrs.seek(-3, SEEK_CUR);
		TS_ASSERT_EQUALS(ssrs.pos(), 5);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 6);

		ssrs.seek(0, SEEK_END);
		TS_ASSERT_EQUALS(ssrs.pos(), 8);
		TS_ASSERT(!ssrs.eos());
		b = ssrs.readByte();
		TS_ASSERT(ssrs.eos());

		ssrs.seek(-3, SEEK_END);
		TS_ASSERT(!ssrs.eos());
		TS_ASSERT_EQUALS(ssrs.pos(), 5);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 6);

		ssrs.seek(-8, SEEK_END);
		TS_ASSERT_EQUALS(ssrs.pos(), 0);
		b = ssrs.readByte();
		TS_ASSERT_EQUALS(b, 1);
	}
};
