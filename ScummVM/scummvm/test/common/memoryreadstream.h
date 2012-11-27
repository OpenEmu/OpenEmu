#include <cxxtest/TestSuite.h>

#include "common/memstream.h"

class MemoryReadStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_seek_set() {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c', '\n' };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		ms.seek(0, SEEK_SET);
		TS_ASSERT_EQUALS(ms.pos(), 0);
		TS_ASSERT(!ms.eos());

		ms.seek(1, SEEK_SET);
		TS_ASSERT_EQUALS(ms.pos(), 1);
		TS_ASSERT(!ms.eos());

		ms.seek(5, SEEK_SET);
		TS_ASSERT_EQUALS(ms.pos(), 5);
		TS_ASSERT(!ms.eos());
	}

	void test_seek_cur() {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c' };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		ms.seek(3, SEEK_CUR);
		TS_ASSERT_EQUALS(ms.pos(), 3);
		TS_ASSERT(!ms.eos());

		ms.seek(-1, SEEK_CUR);
		TS_ASSERT_EQUALS(ms.pos(), 2);
		TS_ASSERT(!ms.eos());

		ms.seek(3, SEEK_CUR);
		TS_ASSERT_EQUALS(ms.pos(), 5);
		TS_ASSERT(!ms.eos());

		ms.seek(-1, SEEK_CUR);
		TS_ASSERT_EQUALS(ms.pos(), 4);
		TS_ASSERT(!ms.eos());
	}

	void test_seek_end() {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c' };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		ms.seek(0, SEEK_END);
		TS_ASSERT_EQUALS(ms.pos(), 5);
		TS_ASSERT(!ms.eos());

		ms.seek(-1, SEEK_END);
		TS_ASSERT_EQUALS(ms.pos(), 4);
		TS_ASSERT(!ms.eos());

		ms.seek(-5, SEEK_END);
		TS_ASSERT_EQUALS(ms.pos(), 0);
		TS_ASSERT(!ms.eos());
	}

	void test_seek_read_le() {
		byte contents[] = { 1, 2, 3, 4, 5, 6, 7 };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		TS_ASSERT_EQUALS(ms.readUint16LE(), 0x0201UL);
		TS_ASSERT_EQUALS(ms.pos(), 2);
		TS_ASSERT_EQUALS(ms.readUint32LE(), 0x06050403UL);
		TS_ASSERT_EQUALS(ms.pos(), 6);
		TS_ASSERT_EQUALS(ms.readByte(), 0x07);
		TS_ASSERT_EQUALS(ms.pos(), 7);
		TS_ASSERT(!ms.eos());
	}

	void test_seek_read_be() {
		byte contents[] = { 1, 2, 3, 4, 5, 6, 7 };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		TS_ASSERT_EQUALS(ms.readUint16BE(), 0x0102UL);
		TS_ASSERT_EQUALS(ms.pos(), 2);
		TS_ASSERT_EQUALS(ms.readUint32BE(), 0x03040506UL);
		TS_ASSERT_EQUALS(ms.pos(), 6);
		TS_ASSERT_EQUALS(ms.readByte(), 0x07);
		TS_ASSERT_EQUALS(ms.pos(), 7);
		TS_ASSERT(!ms.eos());
	}

	void test_eos() {
		byte contents[] = { 1, 2, 3, 4, 5, 6, 7 };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		// Read after the end of the stream
		for (int32 i = 0; i <= ms.size(); ++i)
			ms.readByte();

		// The eos flag should be set here
		TS_ASSERT(ms.eos());

		// Seeking should reset the eos flag
		ms.seek(0, SEEK_SET);
		TS_ASSERT(!ms.eos());
	}
};
