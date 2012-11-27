#ifndef __DELTATEST_H
#define __DELTATEST_H

#include <cxxtest/TestSuite.h>
#include <math.h>

class DeltaTest : public CxxTest::TestSuite
{
    double _pi, _delta;

public:
    void setUp()
    {
        _pi = 3.1415926535;
        _delta = 0.0001;
    }

    void testSine()
    {
        TS_ASSERT_DELTA( sin(0.0), 0.0, _delta );
        TS_ASSERT_DELTA( sin(_pi / 6), 0.5, _delta );
        TS_ASSERT_DELTA( sin(_pi / 2), 1.0, _delta );
        TS_ASSERT_DELTA( sin(_pi), 0.0, _delta );
    }
};

#endif // __DELTATEST_H
