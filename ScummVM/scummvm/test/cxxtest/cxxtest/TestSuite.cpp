#ifndef __cxxtest__TestSuite_cpp__
#define __cxxtest__TestSuite_cpp__

#include <cxxtest/TestSuite.h>

namespace CxxTest
{
    //
    // TestSuite members
    //
    TestSuite::~TestSuite() {}
    void TestSuite::setUp() {}
    void TestSuite::tearDown() {}

    //
    // Test-aborting stuff
    //
    static bool currentAbortTestOnFail = false;

    bool abortTestOnFail()
    {
        return currentAbortTestOnFail;
    }

    void setAbortTestOnFail( bool value )
    {
        currentAbortTestOnFail = value;
    }

    void doAbortTest()
    {
#   if defined(_CXXTEST_HAVE_EH)
        if ( currentAbortTestOnFail )
            throw AbortTest();
#   endif // _CXXTEST_HAVE_EH
    }

    //
    // Max dump size
    //
    static unsigned currentMaxDumpSize = CXXTEST_MAX_DUMP_SIZE;

    unsigned maxDumpSize()
    {
        return currentMaxDumpSize;
    }

    void setMaxDumpSize( unsigned value )
    {
        currentMaxDumpSize = value;
    }

    //
    // Some non-template functions
    //
    void doTrace( const char *file, unsigned line, const char *message )
    {
        tracker().trace( file, line, message );
    }

    void doWarn( const char *file, unsigned line, const char *message )
    {
        tracker().warning( file, line, message );
    }

    void doFailTest( const char *file, unsigned line, const char *message )
    {
        tracker().failedTest( file, line, message );
        TS_ABORT();
    }

    void doFailAssert( const char *file, unsigned line,
                       const char *expression, const char *message )
    {
        if ( message )
            tracker().failedTest( file, line, message );
        tracker().failedAssert( file, line, expression );
        TS_ABORT();
    }

    bool sameData( const void *x, const void *y, unsigned size )
    {
        if ( size == 0 )
            return true;

        if ( x == y )
            return true;

        if ( !x || !y )
            return false;

        const char *cx = (const char *)x;
        const char *cy = (const char *)y;
        while ( size -- )
            if ( *cx++ != *cy++ )
                return false;

        return true;
    }

    void doAssertSameData( const char *file, unsigned line,
                           const char *xExpr, const void *x,
                           const char *yExpr, const void *y,
                           const char *sizeExpr, unsigned size,
                           const char *message )
    {
        if ( !sameData( x, y, size ) ) {
            if ( message )
                tracker().failedTest( file, line, message );
            tracker().failedAssertSameData( file, line, xExpr, yExpr, sizeExpr, x, y, size );
            TS_ABORT();
        }
    }

    void doFailAssertThrows( const char *file, unsigned line,
                             const char *expr, const char *type,
                             bool otherThrown,
                             const char *message )
    {
        if ( message )
            tracker().failedTest( file, line, message );

        tracker().failedAssertThrows( file, line, expr, type, otherThrown );
        TS_ABORT();
    }

    void doFailAssertThrowsNot( const char *file, unsigned line,
                                const char *expression, const char *message )
    {
        if ( message )
            tracker().failedTest( file, line, message );

        tracker().failedAssertThrowsNot( file, line, expression );
        TS_ABORT();
    }
}

#endif // __cxxtest__TestSuite_cpp__
