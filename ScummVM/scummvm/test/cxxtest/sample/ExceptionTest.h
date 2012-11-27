#ifndef __EXCEPTIONTEST_H
#define __EXCEPTIONTEST_H

#include <cxxtest/TestSuite.h>

//
// This test suite demonstrates the use of TS_ASSERT_THROWS
//

class ExceptionTest : public CxxTest::TestSuite
{
public:
    void testAssertion( void )
    {
        // This assert passes, since throwThis() throws (Number)
        TS_ASSERT_THROWS( throwThis(3), const Number & );
        // This assert passes, since throwThis() throws something
        TS_ASSERT_THROWS_ANYTHING( throwThis(-30) );
        // This assert fails, since throwThis() doesn't throw char *
        TS_ASSERT_THROWS( throwThis(5), const char * );
        // This assert fails since goodFunction() throws nothing
        TS_ASSERT_THROWS_ANYTHING( goodFunction(1) );
        // The regular TS_ASSERT macros will catch unhandled exceptions
        TS_ASSERT_EQUALS( throwThis(3), 333 );
        // You can assert that a function throws nothing
        TS_ASSERT_THROWS_NOTHING( throwThis(-1) );
        // If you want to catch the exceptions yourself, use the ETS_ marcos
        try {
            ETS_ASSERT_EQUALS( throwThis(3), 333 );
        } catch( const Number & ) {
            TS_FAIL( "throwThis(3) failed" );
        }
    }

private:
    void goodFunction( int )
    {
    }

    class Number
    {
    public:
        Number( int ) {}
    };

    int throwThis( int i )
    {
        throw Number( i );
    }
};

#endif // __EXCEPTIONTEST_H
