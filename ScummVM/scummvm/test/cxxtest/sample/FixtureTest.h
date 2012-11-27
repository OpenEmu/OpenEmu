#ifndef __FIXTURETEST_H
#define __FIXTURETEST_H

#include <cxxtest/TestSuite.h>
#include <string.h>

//
// This test suite shows how to use setUp() and tearDown()
// to initialize data common to all tests.
// setUp()/tearDown() will be called before and after each
// test.
//

class FixtureTest : public CxxTest::TestSuite
{
    char *_buffer;
public:
    void setUp()
    {
        _buffer = new char[1024];
    }

    void tearDown()
    {
        delete[] _buffer;
    }

    void test_strcpy()
    {
        strcpy( _buffer, "Hello, world!" );
        TS_ASSERT_EQUALS( _buffer[0], 'H' );
        TS_ASSERT_EQUALS( _buffer[1], 'E' );
    }
};


#endif // __FIXTURETEST_H
