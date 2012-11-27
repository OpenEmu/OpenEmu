#include <cxxtest/DummyDescriptions.h>

namespace CxxTest
{
    DummyTestDescription::DummyTestDescription() {}

    const char *DummyTestDescription::file() const { return "<no file>"; }
    unsigned DummyTestDescription::line() const { return 0; }
    const char *DummyTestDescription::testName() const { return "<no test>"; }
    const char *DummyTestDescription::suiteName() const { return "<no suite>"; }
    bool DummyTestDescription::setUp() { return true;}
    void DummyTestDescription::run() {}
    bool DummyTestDescription::tearDown() { return true;}

    TestDescription *DummyTestDescription::next() { return 0; }
    const TestDescription *DummyTestDescription::next() const { return 0; }

    DummySuiteDescription::DummySuiteDescription() : _test() {}

    const char *DummySuiteDescription::file() const { return "<no file>"; }
    unsigned DummySuiteDescription::line() const { return 0; }
    const char *DummySuiteDescription::suiteName() const { return "<no suite>"; }
    TestSuite *DummySuiteDescription::suite() const { return 0; }
    unsigned DummySuiteDescription::numTests() const { return 0; }
    const TestDescription &DummySuiteDescription::testDescription( unsigned ) const { return _test; }
    SuiteDescription *DummySuiteDescription::next() { return 0; }
    TestDescription *DummySuiteDescription::firstTest() { return 0; }
    const SuiteDescription *DummySuiteDescription::next() const { return 0; }
    const TestDescription *DummySuiteDescription::firstTest() const { return 0; }
    void DummySuiteDescription::activateAllTests() {}
    bool DummySuiteDescription::leaveOnly( const char * /*testName*/ ) { return false; }

    bool DummySuiteDescription::setUp() { return true;}
    bool DummySuiteDescription::tearDown() { return true;}

    DummyWorldDescription::DummyWorldDescription() : _suite() {}

    unsigned DummyWorldDescription::numSuites( void ) const { return 0; }
    unsigned DummyWorldDescription::numTotalTests( void ) const { return 0; }
    const SuiteDescription &DummyWorldDescription::suiteDescription( unsigned ) const { return _suite; }
    SuiteDescription *DummyWorldDescription::firstSuite() { return 0; }
    const SuiteDescription *DummyWorldDescription::firstSuite() const { return 0; }
    void DummyWorldDescription::activateAllTests() {}
    bool DummyWorldDescription::leaveOnly( const char * /*suiteName*/, const char * /*testName*/ ) { return false; }

    bool DummyWorldDescription::setUp() { return true;}
    bool DummyWorldDescription::tearDown() { return true;}
}
