#ifndef __cxxtest__RealDescriptions_cpp__
#define __cxxtest__RealDescriptions_cpp__

//
// NOTE: If an error occur during world construction/deletion, CxxTest cannot
//       know where the error originated.
//

#include <cxxtest/RealDescriptions.h>

namespace CxxTest
{
    RealTestDescription::RealTestDescription()
    {
    }

    RealTestDescription::RealTestDescription( List &argList,
                                              SuiteDescription &argSuite,
                                              unsigned argLine,
                                              const char *argTestName )
    {
        initialize( argList, argSuite, argLine, argTestName );
    }

    void RealTestDescription::initialize( List &argList,
                                          SuiteDescription &argSuite,
                                          unsigned argLine,
                                          const char *argTestName )
    {
        _suite = &argSuite;
        _line = argLine;
        _testName = argTestName;
        attach( argList );
    }

    bool RealTestDescription::setUp()
    {
        if ( !suite() )
            return false;

        for ( GlobalFixture *gf = GlobalFixture::firstGlobalFixture(); gf != 0; gf = gf->nextGlobalFixture() ) {
            bool ok;
            _TS_TRY { ok = gf->setUp(); }
            _TS_LAST_CATCH( { ok = false; } );

            if ( !ok ) {
                doFailTest( file(), line(), "Error in GlobalFixture::setUp()" );
                return false;
            }
        }

        _TS_TRY {
            _TSM_ASSERT_THROWS_NOTHING( file(), line(), "Exception thrown from setUp()", suite()->setUp() );
        }
        _TS_CATCH_ABORT( { return false; } );

        return true;
    }

    bool RealTestDescription::tearDown()
    {
        if ( !suite() )
            return false;

        _TS_TRY {
            _TSM_ASSERT_THROWS_NOTHING( file(), line(), "Exception thrown from tearDown()", suite()->tearDown() );
        }
        _TS_CATCH_ABORT( { return false; } );

        for ( GlobalFixture *gf = GlobalFixture::lastGlobalFixture(); gf != 0; gf = gf->prevGlobalFixture() ) {
            bool ok;
            _TS_TRY { ok = gf->tearDown(); }
            _TS_LAST_CATCH( { ok = false; } );

            if ( !ok ) {
                doFailTest( file(), line(), "Error in GlobalFixture::tearDown()" );
                return false;
            }
        }

        return true;
    }

    const char *RealTestDescription::file() const { return _suite->file(); }
    unsigned RealTestDescription::line() const { return _line; }
    const char *RealTestDescription::testName() const { return _testName; }
    const char *RealTestDescription::suiteName() const { return _suite->suiteName(); }

    TestDescription *RealTestDescription::next() { return (RealTestDescription *)Link::next(); }
    const TestDescription *RealTestDescription::next() const { return (const RealTestDescription *)Link::next(); }

    TestSuite *RealTestDescription::suite() const { return _suite->suite(); }

    void RealTestDescription::run()
    {
        _TS_TRY { runTest(); }
        _TS_CATCH_ABORT( {} )
            ___TSM_CATCH( file(), line(), "Exception thrown from test" );
    }

    RealSuiteDescription::RealSuiteDescription() {}
    RealSuiteDescription::RealSuiteDescription( const char *argFile,
                                                unsigned argLine,
                                                const char *argSuiteName,
                                                List &argTests )
    {
        initialize( argFile, argLine, argSuiteName, argTests );
    }

    void RealSuiteDescription::initialize( const char *argFile,
                                           unsigned argLine,
                                           const char *argSuiteName,
                                           List &argTests )
    {
        _file = argFile;
        _line = argLine;
        _suiteName = argSuiteName;
        _tests = &argTests;

        attach( _suites );
    }

    const char *RealSuiteDescription::file() const { return _file; }
    unsigned RealSuiteDescription::line() const { return _line; }
    const char *RealSuiteDescription::suiteName() const { return _suiteName; }

    TestDescription *RealSuiteDescription::firstTest() { return (RealTestDescription *)_tests->head(); }
    const TestDescription *RealSuiteDescription::firstTest() const { return (const RealTestDescription *)_tests->head(); }
    SuiteDescription *RealSuiteDescription::next() { return (RealSuiteDescription *)Link::next(); }
    const SuiteDescription *RealSuiteDescription::next() const { return (const RealSuiteDescription *)Link::next(); }

    unsigned RealSuiteDescription::numTests() const { return _tests->size(); }

    const TestDescription &RealSuiteDescription::testDescription( unsigned i ) const
    {
        return *(RealTestDescription *)_tests->nth( i );
    }

    void RealSuiteDescription::activateAllTests()
    {
        _tests->activateAll();
    }

    bool RealSuiteDescription::leaveOnly( const char *testName )
    {
        for ( TestDescription *td = firstTest(); td != 0; td = td->next() ) {
            if ( stringsEqual( td->testName(), testName ) ) {
                _tests->leaveOnly( *td );
                return true;
            }
        }
        return false;
    }

    StaticSuiteDescription::StaticSuiteDescription() {}
    StaticSuiteDescription::StaticSuiteDescription( const char *argFile, unsigned argLine,
                                                    const char *argSuiteName, TestSuite &argSuite,
                                                    List &argTests ) :
        RealSuiteDescription( argFile, argLine, argSuiteName, argTests )
    {
        doInitialize( argSuite );
    }

    void StaticSuiteDescription::initialize( const char *argFile, unsigned argLine,
                                             const char *argSuiteName, TestSuite &argSuite,
                                             List &argTests )
    {
        RealSuiteDescription::initialize( argFile, argLine, argSuiteName, argTests );
        doInitialize( argSuite );
    }

    void StaticSuiteDescription::doInitialize( TestSuite &argSuite )
    {
        _suite = &argSuite;
    }

    TestSuite *StaticSuiteDescription::suite() const
    {
        return _suite;
    }

    bool StaticSuiteDescription::setUp() { return true; }
    bool StaticSuiteDescription::tearDown() { return true; }

    CommonDynamicSuiteDescription::CommonDynamicSuiteDescription() {}
    CommonDynamicSuiteDescription::CommonDynamicSuiteDescription( const char *argFile, unsigned argLine,
                                                                  const char *argSuiteName, List &argTests,
                                                                  unsigned argCreateLine, unsigned argDestroyLine ) :
        RealSuiteDescription( argFile, argLine, argSuiteName, argTests )
    {
        doInitialize( argCreateLine, argDestroyLine );
    }

    void CommonDynamicSuiteDescription::initialize( const char *argFile, unsigned argLine,
                                                    const char *argSuiteName, List &argTests,
                                                    unsigned argCreateLine, unsigned argDestroyLine )
    {
        RealSuiteDescription::initialize( argFile, argLine, argSuiteName, argTests );
        doInitialize( argCreateLine, argDestroyLine );
    }

    void CommonDynamicSuiteDescription::doInitialize( unsigned argCreateLine, unsigned argDestroyLine )
    {
        _createLine = argCreateLine;
        _destroyLine = argDestroyLine;
    }

    List &RealWorldDescription::suites()
    {
        return RealSuiteDescription::_suites;
    }

    unsigned RealWorldDescription::numSuites( void ) const
    {
        return suites().size();
    }

    unsigned RealWorldDescription::numTotalTests( void ) const
    {
        unsigned count = 0;
        for ( const SuiteDescription *sd = firstSuite(); sd != 0; sd = sd->next() )
            count += sd->numTests();
        return count;
    }

    SuiteDescription *RealWorldDescription::firstSuite()
    {
        return (RealSuiteDescription *)suites().head();
    }

    const SuiteDescription *RealWorldDescription::firstSuite() const
    {
        return (const RealSuiteDescription *)suites().head();
    }

    const SuiteDescription &RealWorldDescription::suiteDescription( unsigned i ) const
    {
        return *(const RealSuiteDescription *)suites().nth( i );
    }

    void RealWorldDescription::activateAllTests()
    {
        suites().activateAll();
        for ( SuiteDescription *sd = firstSuite(); sd != 0; sd = sd->next() )
            sd->activateAllTests();
    }

    bool RealWorldDescription::leaveOnly( const char *suiteName, const char *testName )
    {
        for ( SuiteDescription *sd = firstSuite(); sd != 0; sd = sd->next() ) {
            if ( stringsEqual( sd->suiteName(), suiteName ) ) {
                if ( testName )
                    if ( !sd->leaveOnly( testName ) )
                        return false;
                suites().leaveOnly( *sd );
                return true;
            }
        }
        return false;
    }

    bool RealWorldDescription::setUp()
    {
        for ( GlobalFixture *gf = GlobalFixture::firstGlobalFixture(); gf != 0; gf = gf->nextGlobalFixture() ) {
            bool ok;
            _TS_TRY { ok = gf->setUpWorld(); }
            _TS_LAST_CATCH( { ok = false; } );

            if ( !ok ) {
                reportError( "Error setting up world" );
                return false;
            }
        }

        return true;
    }

    bool RealWorldDescription::tearDown()
    {
        for ( GlobalFixture *gf = GlobalFixture::lastGlobalFixture(); gf != 0; gf = gf->prevGlobalFixture() ) {
            bool ok;
            _TS_TRY { ok = gf->tearDownWorld(); }
            _TS_LAST_CATCH( { ok = false; } );

            if ( !ok ) {
                reportError( "Error tearing down world" );
                return false;
            }
        }

        return true;
    }

    void RealWorldDescription::reportError( const char *message )
    {
        doWarn( __FILE__, 5, message );
    }

    void activateAllTests()
    {
        RealWorldDescription().activateAllTests();
    }

    bool leaveOnly( const char *suiteName, const char *testName )
    {
        return RealWorldDescription().leaveOnly( suiteName, testName );
    }
}

#endif // __cxxtest__RealDescriptions_cpp__
