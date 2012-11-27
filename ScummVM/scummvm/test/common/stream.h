#include <cxxtest/TestSuite.h>

#include "common/memstream.h"

class ReadLineStreamTestSuite : public CxxTest::TestSuite {
	public:
	void test_readline() {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c', '\n' };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		char buffer[100];

		TS_ASSERT_DIFFERS((char *)0, ms.readLine(buffer, sizeof(buffer)));
		TS_ASSERT_EQUALS(0, strcmp(buffer, "ab\n"));

		TS_ASSERT_DIFFERS((char *)0, ms.readLine(buffer, sizeof(buffer)));
		TS_ASSERT_EQUALS(0, strcmp(buffer, "\n"));

		TS_ASSERT_DIFFERS((char *)0, ms.readLine(buffer, sizeof(buffer)));
		TS_ASSERT_EQUALS(0, strcmp(buffer, "c\n"));

		TS_ASSERT(!ms.eos());

		TS_ASSERT_EQUALS((char *)0, ms.readLine(buffer, sizeof(buffer)));

		TS_ASSERT(ms.eos());
	}

	void test_readline2() {
		byte contents[] = { 'a', 'b', '\n', '\n', 'c' };
		Common::MemoryReadStream ms(contents, sizeof(contents));

		char buffer[100];

		TS_ASSERT_DIFFERS((char *)0, ms.readLine(buffer, sizeof(buffer)));
		TS_ASSERT_EQUALS(0, strcmp(buffer, "ab\n"));

		TS_ASSERT_DIFFERS((char *)0, ms.readLine(buffer, sizeof(buffer)));
		TS_ASSERT_EQUALS(0, strcmp(buffer, "\n"));

		TS_ASSERT_DIFFERS((char *)0, ms.readLine(buffer, sizeof(buffer)));
		TS_ASSERT_EQUALS(0, strcmp(buffer, "c"));

		TS_ASSERT(ms.eos());
	}
};
