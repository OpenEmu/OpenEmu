#include <cxxtest/TestSuite.h>

#include "common/math.h"

class MathTestSuite : public CxxTest::TestSuite
{
	public:
	void test_intLog2() {
		// Test special case for 0
		TS_ASSERT_EQUALS(Common::intLog2(0), -1);

		// intLog2 should round the result towards 0
		TS_ASSERT_EQUALS(Common::intLog2(7), 2);

		// Some simple test for 2^10
		TS_ASSERT_EQUALS(Common::intLog2(1024), 10);
	}
};
