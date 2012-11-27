#include <cxxtest/TestSuite.h>

#include "common/rect.h"

class RectTestSuite : public CxxTest::TestSuite
{
	public:
	void test_point_sqrDist() {
		Common::Point p0;
		Common::Point p11(1, 1);
		Common::Point p21(2, 1);
		Common::Point p23(2, 3);
		Common::Point p32(3, 2);
		TS_ASSERT_EQUALS(p0.sqrDist(p11), (uint) 2);
		TS_ASSERT_EQUALS(p0.sqrDist(p21), (uint) 5);
		TS_ASSERT_EQUALS(p0.sqrDist(p23), p0.sqrDist(p32));
		TS_ASSERT_EQUALS(p11.sqrDist(p11), (uint) 0);
		TS_ASSERT_EQUALS(p11.sqrDist(p23), (uint) 5);
	}

	void test_intersects() {
		TS_ASSERT(Common::Rect(0, 0, 2, 2).intersects(Common::Rect(0, 0, 1, 1)));
		TS_ASSERT(Common::Rect(0, 0, 2, 2).intersects(Common::Rect(1, 1, 2, 2)));
		TS_ASSERT(!Common::Rect(0, 0, 1, 1).intersects(Common::Rect(1, 1, 2, 2)));
	}

	void test_contains() {
		Common::Rect r0;
		Common::Rect r1(0, 0, 1, 1);
		Common::Rect r2(0, 0, 2, 2);
		TS_ASSERT(!r0.contains(r1));
		TS_ASSERT(!r0.contains(r2));
		TS_ASSERT(!r1.contains(r2));
		TS_ASSERT(r0.contains(r0));

		TS_ASSERT(r1.contains(r0));
		TS_ASSERT(r1.contains(r1));

		TS_ASSERT(r2.contains(r0));
		TS_ASSERT(r2.contains(r1));
		TS_ASSERT(r2.contains(r2));
	}

	void test_extend() {
		Common::Rect r0;
		Common::Rect r1(0, 0, 1, 1);
		Common::Rect r2(0, 0, 2, 2);
		TS_ASSERT(!r0.contains(r1));
		r0.extend(r1);
		TS_ASSERT(r0.contains(r1));
		TS_ASSERT_EQUALS(r0.top,    0);
		TS_ASSERT_EQUALS(r0.left,   0);
		TS_ASSERT_EQUALS(r0.bottom, 1);
		TS_ASSERT_EQUALS(r0.right,  1);
		r2.extend(r1);
		TS_ASSERT_EQUALS(r2.top,    0);
		TS_ASSERT_EQUALS(r2.left,   0);
		TS_ASSERT_EQUALS(r2.bottom, 2);
		TS_ASSERT_EQUALS(r2.right,  2);
	}

};
