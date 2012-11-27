#include <cxxtest/TestSuite.h>
#include "common/endian.h"

class EndianTestSuite : public CxxTest::TestSuite
{
	public:
	void test_MKTAG() {
		const char *str_tag = "ABCD";
		uint32 tag = READ_BE_UINT32(str_tag);
		TS_ASSERT_EQUALS(MKTAG('A','B','C','D'), tag);
	}

	void test_READ_BE_UINT32() {
		const char data[4] = { 0x12, 0x34, 0x56, 0x78 };
		uint32 value = READ_BE_UINT32(data);
		TS_ASSERT_EQUALS(value, 0x12345678UL);
	}

	void test_READ_LE_UINT32() {
		const char data[4] = { 0x12, 0x34, 0x56, 0x78 };
		uint32 value = READ_LE_UINT32(data);
		TS_ASSERT_EQUALS(value, 0x78563412UL);
	}

	void test_READ_BE_UINT16() {
		const char data[4] = { 0x12, 0x34, 0x56, 0x78 };
		uint32 value = READ_BE_UINT16(data);
		TS_ASSERT_EQUALS(value, 0x1234UL);
	}

	void test_READ_LE_UINT16() {
		const char data[4] = { 0x12, 0x34, 0x56, 0x78 };
		uint32 value = READ_LE_UINT16(data);
		TS_ASSERT_EQUALS(value, 0x3412UL);
	}
};
