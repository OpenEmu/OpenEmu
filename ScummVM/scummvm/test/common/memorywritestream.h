#include <cxxtest/TestSuite.h>

#include "common/memstream.h"

class MemoryWriteStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_err() {
		byte temp = 0;

		Common::MemoryWriteStream stream(&temp, 0);
		TS_ASSERT(!stream.err());

		// Make sure the error indicator gets set
		stream.write(&temp, 1);
		TS_ASSERT(stream.err());

		// Test whether the error indicator can be cleared
		stream.clearErr();
		TS_ASSERT(!stream.err());
	}

	void test_write() {
		byte buffer[7] = {};
		Common::MemoryWriteStream stream(buffer, sizeof(buffer));

		const byte data[7] = { 7, 4, 3, 0, 10, 12, 1 };
		stream.write(data, sizeof(data));
		TS_ASSERT(memcmp(buffer, data, sizeof(data)) == 0);
		TS_ASSERT(!stream.err());
	}
};
