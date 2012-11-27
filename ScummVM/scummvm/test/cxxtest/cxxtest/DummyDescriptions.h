#ifndef __cxxtest__DummyDescriptions_h__
#define __cxxtest__DummyDescriptions_h__

//
// DummyTestDescription, DummySuiteDescription and DummyWorldDescription
//

#include <cxxtest/Descriptions.h>

namespace CxxTest
{
    class DummyTestDescription : public TestDescription
    {
    public:
        DummyTestDescription();

        const char *file() const;
        unsigned line() const;
        const char *testName() const;
        const char *suiteName() const;
        bool setUp();
        void run();
        bool tearDown();

        TestDescription *next();
        const TestDescription *next() const;
    };

    class DummySuiteDescription : public SuiteDescription
    {
    public:
        DummySuiteDescription();

        const char *file() const;
        unsigned line() const;
        const char *suiteName() const;
        TestSuite *suite() const;
        unsigned numTests() const;
        const TestDescription &testDescription( unsigned ) const;
        SuiteDescription *next();
        TestDescription *firstTest();
        const SuiteDescription *next() const;
        const TestDescription *firstTest() const;
        void activateAllTests();
        bool leaveOnly( const char * /*testName*/ );

        bool setUp();
        bool tearDown();

    private:
        DummyTestDescription _test;
    };

    class DummyWorldDescription : public WorldDescription
    {
    public:
        DummyWorldDescription();

        unsigned numSuites( void ) const;
        unsigned numTotalTests( void ) const;
        const SuiteDescription &suiteDescription( unsigned ) const;
        SuiteDescription *firstSuite();
        const SuiteDescription *firstSuite() const;
        void activateAllTests();
        bool leaveOnly( const char * /*suiteName*/, const char * /*testName*/ = 0 );

        bool setUp();
        bool tearDown();

    private:
        DummySuiteDescription _suite;
    };
}

#endif // __cxxtest__DummyDescriptions_h__
