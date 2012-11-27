#ifndef __cxxtest__TestListener_h__
#define __cxxtest__TestListener_h__

//
// TestListener is the base class for all "listeners",
// i.e. classes that receive notifications of the
// testing process.
//
// The names of the parameters are in comments to avoid
// "unused parameter" warnings.
//

#include <cxxtest/Descriptions.h>

namespace CxxTest
{
    class TestListener
    {
    public:
        TestListener() {}
        virtual ~TestListener() {}

        virtual void enterWorld( const WorldDescription & /*desc*/ ) {}
        virtual void enterSuite( const SuiteDescription & /*desc*/ ) {}
        virtual void enterTest( const TestDescription & /*desc*/ ) {}
        virtual void trace( const char * /*file*/, unsigned /*line*/,
                            const char * /*expression*/ ) {}
        virtual void warning( const char * /*file*/, unsigned /*line*/,
                              const char * /*expression*/ ) {}
        virtual void failedTest( const char * /*file*/, unsigned /*line*/,
                                 const char * /*expression*/ ) {}
        virtual void failedAssert( const char * /*file*/, unsigned /*line*/,
                                   const char * /*expression*/ ) {}
        virtual void failedAssertEquals( const char * /*file*/, unsigned /*line*/,
                                         const char * /*xStr*/, const char * /*yStr*/,
                                         const char * /*x*/, const char * /*y*/ ) {}
        virtual void failedAssertSameData( const char * /*file*/, unsigned /*line*/,
                                           const char * /*xStr*/, const char * /*yStr*/,
                                           const char * /*sizeStr*/, const void * /*x*/,
                                           const void * /*y*/, unsigned /*size*/ ) {}
        virtual void failedAssertDelta( const char * /*file*/, unsigned /*line*/,
                                        const char * /*xStr*/, const char * /*yStr*/,
                                        const char * /*dStr*/, const char * /*x*/,
                                        const char * /*y*/, const char * /*d*/ ) {}
        virtual void failedAssertDiffers( const char * /*file*/, unsigned /*line*/,
                                          const char * /*xStr*/, const char * /*yStr*/,
                                          const char * /*value*/ ) {}
        virtual void failedAssertLessThan( const char * /*file*/, unsigned /*line*/,
                                           const char * /*xStr*/, const char * /*yStr*/,
                                           const char * /*x*/, const char * /*y*/ ) {}
        virtual void failedAssertLessThanEquals( const char * /*file*/, unsigned /*line*/,
                                                 const char * /*xStr*/, const char * /*yStr*/,
                                                 const char * /*x*/, const char * /*y*/ ) {}
        virtual void failedAssertPredicate( const char * /*file*/, unsigned /*line*/,
                                            const char * /*predicate*/, const char * /*xStr*/, const char * /*x*/ ) {}
        virtual void failedAssertRelation( const char * /*file*/, unsigned /*line*/,
                                           const char * /*relation*/, const char * /*xStr*/, const char * /*yStr*/,
                                           const char * /*x*/, const char * /*y*/ ) {}
        virtual void failedAssertThrows( const char * /*file*/, unsigned /*line*/,
                                         const char * /*expression*/, const char * /*type*/,
                                         bool /*otherThrown*/ ) {}
        virtual void failedAssertThrowsNot( const char * /*file*/, unsigned /*line*/,
                                            const char * /*expression*/ ) {}
        virtual void leaveTest( const TestDescription & /*desc*/ ) {}
        virtual void leaveSuite( const SuiteDescription & /*desc*/ ) {}
        virtual void leaveWorld( const WorldDescription & /*desc*/ ) {}
    };
}

#endif // __cxxtest__TestListener_h__
