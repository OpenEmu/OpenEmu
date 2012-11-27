#include <cxxtest/TestSuite.h>

#include "common/memstream.h"
#include "common/substream.h"

class SubReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_traverse() {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, 10);

		int end = 5;

		Common::SubReadStream srs(&ms, end);

		int i;
		byte b;
		for (i = 0; i < end; ++i) {
			TS_ASSERT(!srs.eos());

			b = srs.readByte();
			TS_ASSERT_EQUALS(i, b);
		}

		TS_ASSERT(!srs.eos());
		b = srs.readByte();
		TS_ASSERT(srs.eos());
	}

	void test_safe_eos() {
		byte contents[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		Common::SafeSeekableSubReadStream ssrs1(&ms, 0, sizeof(contents));
		Common::SafeSeekableSubReadStream ssrs2(&ms, 0, sizeof(contents));

		// Read after the end of the stream of the first sub stream
		for (int32 i = 0; i <= ssrs1.size(); ++i)
			ssrs1.readByte();

		// eos should be set for the first sub stream
		TS_ASSERT(ssrs1.eos());

		// eos should not be set for the second sub stream
		TS_ASSERT(!ssrs2.eos());
	}
};
