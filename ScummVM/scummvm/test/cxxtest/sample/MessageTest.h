#ifndef __MESSAGETEST_H
#define __MESSAGETEST_H

#include <cxxtest/TestSuite.h>

//
// The [E]TSM_ macros can be used to print a specified message
// instead of the default one.
// This is useful when you refactor your tests, as shown below
//

class MessageTest : public CxxTest::TestSuite
{
public:
    void testValues()
    {
        checkValue( 0, "My hovercraft" );
        checkValue( 1, "is full" );
        checkValue( 2, "of eels" );
    }

    void checkValue( unsigned value, const char *message )
    {
        TSM_ASSERT( message, value != 0 );
        TSM_ASSERT_EQUALS( message, value, value * value );
    }
};


#endif // __MESSAGETEST_H
