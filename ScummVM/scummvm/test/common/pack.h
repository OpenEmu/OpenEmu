#include <cxxtest/TestSuite.h>

#include "common/scummsys.h"
#include <stddef.h>

#include <common/pack-start.h>	// START STRUCT PACKING

struct TestStruct {
	uint32 x;
	byte y;
	uint16 z;
	uint32 a;
	byte b;
} PACKED_STRUCT;

#include <common/pack-end.h>	// END STRUCT PACKING

#define OFFS(type,item) (((ptrdiff_t)(&((type *)42)->type::item))-42)

class PackTestSuite : public CxxTest::TestSuite
{
	public:
	void test_packing() {
		TS_ASSERT_EQUALS(sizeof(TestStruct), size_t(4+1+2+4+1));
	}

	void test_offsets() {
		TS_ASSERT_EQUALS(OFFS(TestStruct, x), (ptrdiff_t)0);
		TS_ASSERT_EQUALS(OFFS(TestStruct, y), (ptrdiff_t)4);
		TS_ASSERT_EQUALS(OFFS(TestStruct, z), (ptrdiff_t)5);
		TS_ASSERT_EQUALS(OFFS(TestStruct, a), (ptrdiff_t)7);
		TS_ASSERT_EQUALS(OFFS(TestStruct, b), (ptrdiff_t)11);
	}
};
