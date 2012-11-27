#ifndef __cxxtest__RealDescriptions_h__
#define __cxxtest__RealDescriptions_h__

//
// The "real" description classes
//

#include <cxxtest/Descriptions.h>
#include <cxxtest/TestSuite.h>
#include <cxxtest/GlobalFixture.h>

namespace CxxTest
{
    class RealTestDescription : public TestDescription
    {
    public:
        RealTestDescription();
        RealTestDescription( List &argList, SuiteDescription &argSuite, unsigned argLine, const char *argTestName );
        void initialize( List &argList, SuiteDescription &argSuite, unsigned argLine, const char *argTestName );

        const char *file() const;
        unsigned line() const;
        const char *testName() const;
        const char *suiteName() const;

        TestDescription *next();
        const TestDescription *next() const;

        TestSuite *suite() const;

        bool setUp();
        void run();
        bool tearDown();

    private:
        RealTestDescription( const RealTestDescription & );
        RealTestDescription &operator=( const RealTestDescription & );

        virtual void runTest() = 0;

        SuiteDescription *_suite;
        unsigned _line;
        const char *_testName;
    };

    class RealSuiteDescription : public SuiteDescription
    {
    public:
        RealSuiteDescription();
        RealSuiteDescription( const char *argFile, unsigned argLine, const char *argSuiteName, List &argTests );

        void initialize( const char *argFile, unsigned argLine, const char *argSuiteName, List &argTests );

        const char *file() const;
        unsigned line() const;
        const char *suiteName() const;

        TestDescription *firstTest();
        const TestDescription *firstTest() const;
        SuiteDescription *next();
        const SuiteDescription *next() const;

        unsigned numTests() const;
        const TestDescription &testDescription( unsigned i ) const;

        void activateAllTests();
        bool leaveOnly( const char *testName );

    private:
        RealSuiteDescription( const RealSuiteDescription & );
        RealSuiteDescription &operator=( const RealSuiteDescription & );

        const char *_file;
        unsigned _line;
        const char *_suiteName;
        List *_tests;

        static List _suites;
        friend class RealWorldDescription;
    };

    class StaticSuiteDescription : public RealSuiteDescription
    {
    public:
        StaticSuiteDescription();
        StaticSuiteDescription( const char *argFile, unsigned argLine,
                                const char *argSuiteName, TestSuite &argSuite,
                                List &argTests );

        void initialize( const char *argFile, unsigned argLine,
                         const char *argSuiteName, TestSuite &argSuite,
                         List &argTests );
        TestSuite *suite() const;

        bool setUp();
        bool tearDown();

    private:
        StaticSuiteDescription( const StaticSuiteDescription & );
        StaticSuiteDescription &operator=( const StaticSuiteDescription & );

        void doInitialize( TestSuite &argSuite );

        TestSuite *_suite;
    };

    class CommonDynamicSuiteDescription : public RealSuiteDescription
    {
    public:
        CommonDynamicSuiteDescription();
        CommonDynamicSuiteDescription( const char *argFile, unsigned argLine,
                                       const char *argSuiteName, List &argTests,
                                       unsigned argCreateLine, unsigned argDestroyLine );

        void initialize( const char *argFile, unsigned argLine,
                         const char *argSuiteName, List &argTests,
                         unsigned argCreateLine, unsigned argDestroyLine );

    protected:
        unsigned _createLine, _destroyLine;

    private:
        void doInitialize( unsigned argCreateLine, unsigned argDestroyLine );
    };

    template<class S>
    class DynamicSuiteDescription : public CommonDynamicSuiteDescription
    {
    public:
        DynamicSuiteDescription() {}
        DynamicSuiteDescription( const char *argFile, unsigned argLine,
                                 const char *argSuiteName, List &argTests,
                                 S *&argSuite, unsigned argCreateLine,
                                 unsigned argDestroyLine ) :
            CommonDynamicSuiteDescription( argFile, argLine, argSuiteName, argTests, argCreateLine, argDestroyLine )
        {
            _suite = &argSuite;
        }

        void initialize( const char *argFile, unsigned argLine,
                         const char *argSuiteName, List &argTests,
                         S *&argSuite, unsigned argCreateLine,
                         unsigned argDestroyLine )
        {
            CommonDynamicSuiteDescription::initialize( argFile, argLine,
                                                       argSuiteName, argTests,
                                                       argCreateLine, argDestroyLine );
            _suite = &argSuite;
        }

        TestSuite *suite() const { return realSuite(); }

        bool setUp();
        bool tearDown();

    private:
        S *realSuite() const { return *_suite; }
        void setSuite( S *s ) { *_suite = s; }

        void createSuite()
        {
            setSuite( S::createSuite() );
        }

        void destroySuite()
        {
            S *s = realSuite();
            setSuite( 0 );
            S::destroySuite( s );
        }

        S **_suite;
    };

    template<class S>
    bool DynamicSuiteDescription<S>::setUp()
    {
        _TS_TRY {
            _TSM_ASSERT_THROWS_NOTHING( file(), _createLine, "Exception thrown from createSuite()", createSuite() );
            _TSM_ASSERT( file(), _createLine, "createSuite() failed", suite() != 0 );
        }
        _TS_CATCH_ABORT( { return false; } );

        return (suite() != 0);
    }

    template<class S>
    bool DynamicSuiteDescription<S>::tearDown()
    {
        if ( !_suite )
            return true;

        _TS_TRY {
            _TSM_ASSERT_THROWS_NOTHING( file(), _destroyLine, "destroySuite() failed", destroySuite() );
        }
        _TS_CATCH_ABORT( { return false; } );

        return true;
    }

    class RealWorldDescription : public WorldDescription
    {
    public:
        static List &suites();
        unsigned numSuites( void ) const;
        unsigned numTotalTests( void ) const;
        SuiteDescription *firstSuite();
        const SuiteDescription *firstSuite() const;
        const SuiteDescription &suiteDescription( unsigned i ) const;
        void activateAllTests();
        bool leaveOnly( const char *suiteName, const char *testName = 0 );

        bool setUp();
        bool tearDown();
        static void reportError( const char *message );
    };

    void activateAllTests();
    bool leaveOnly( const char *suiteName, const char *testName = 0 );
}

#endif // __cxxtest__RealDescriptions_h__
