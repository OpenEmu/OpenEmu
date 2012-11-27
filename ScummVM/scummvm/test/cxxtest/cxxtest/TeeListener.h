#ifndef __cxxtest__TeeListener_h__
#define __cxxtest__TeeListener_h__

//
// A TeeListener notifies two "reular" TestListeners
//

#include <cxxtest/TestListener.h>
#include <cxxtest/TestListener.h>

namespace CxxTest
{
    class TeeListener : public TestListener
    {
    public:
        TeeListener()
        {
            setFirst( _dummy );
            setSecond( _dummy );
        }

        virtual ~TeeListener()
        {
        }

        void setFirst( TestListener &first )
        {
            _first = &first;
        }

        void setSecond( TestListener &second )
        {
            _second = &second;
        }

        void enterWorld( const WorldDescription &d )
        {
            _first->enterWorld( d );
            _second->enterWorld( d );
        }

        void enterSuite( const SuiteDescription &d )
        {
            _first->enterSuite( d );
            _second->enterSuite( d );
        }

        void enterTest( const TestDescription &d )
        {
            _first->enterTest( d );
            _second->enterTest( d );
        }

        void trace( const char *file, unsigned line, const char *expression )
        {
            _first->trace( file, line, expression );
            _second->trace( file, line, expression );
        }

        void warning( const char *file, unsigned line, const char *expression )
        {
            _first->warning( file, line, expression );
            _second->warning( file, line, expression );
        }

        void failedTest( const char *file, unsigned line, const char *expression )
        {
            _first->failedTest( file, line, expression );
            _second->failedTest( file, line, expression );
        }

        void failedAssert( const char *file, unsigned line, const char *expression )
        {
            _first->failedAssert( file, line, expression );
            _second->failedAssert( file, line, expression );
        }

        void failedAssertEquals( const char *file, unsigned line,
                                 const char *xStr, const char *yStr,
                                 const char *x, const char *y )
        {
            _first->failedAssertEquals( file, line, xStr, yStr, x, y );
            _second->failedAssertEquals( file, line, xStr, yStr, x, y );
        }

        void failedAssertSameData( const char *file, unsigned line,
                                   const char *xStr, const char *yStr,
                                   const char *sizeStr, const void *x,
                                   const void *y, unsigned size )
        {
            _first->failedAssertSameData( file, line, xStr, yStr, sizeStr, x, y, size );
            _second->failedAssertSameData( file, line, xStr, yStr, sizeStr, x, y, size );
        }

        void failedAssertDelta( const char *file, unsigned line,
                                const char *xStr, const char *yStr, const char *dStr,
                                const char *x, const char *y, const char *d )
        {
            _first->failedAssertDelta( file, line, xStr, yStr, dStr, x, y, d );
            _second->failedAssertDelta( file, line, xStr, yStr, dStr, x, y, d );
        }

        void failedAssertDiffers( const char *file, unsigned line,
                                  const char *xStr, const char *yStr,
                                  const char *value )
        {
            _first->failedAssertDiffers( file, line, xStr, yStr, value );
            _second->failedAssertDiffers( file, line, xStr, yStr, value );
        }

        void failedAssertLessThan( const char *file, unsigned line,
                                   const char *xStr, const char *yStr,
                                   const char *x, const char *y )
        {
            _first->failedAssertLessThan( file, line, xStr, yStr, x, y );
            _second->failedAssertLessThan( file, line, xStr, yStr, x, y );
        }

        void failedAssertLessThanEquals( const char *file, unsigned line,
                                         const char *xStr, const char *yStr,
                                         const char *x, const char *y )
        {
            _first->failedAssertLessThanEquals( file, line, xStr, yStr, x, y );
            _second->failedAssertLessThanEquals( file, line, xStr, yStr, x, y );
        }

        void failedAssertPredicate( const char *file, unsigned line,
                                    const char *predicate, const char *xStr, const char *x )
        {
            _first->failedAssertPredicate( file, line, predicate, xStr, x );
            _second->failedAssertPredicate( file, line, predicate, xStr, x );
        }

        void failedAssertRelation( const char *file, unsigned line,
                                   const char *relation, const char *xStr, const char *yStr,
                                   const char *x, const char *y )
        {
            _first->failedAssertRelation( file, line, relation, xStr, yStr, x, y );
            _second->failedAssertRelation( file, line, relation, xStr, yStr, x, y );
        }

        void failedAssertThrows( const char *file, unsigned line,
                                 const char *expression, const char *type,
                                 bool otherThrown )
        {
            _first->failedAssertThrows( file, line, expression, type, otherThrown );
            _second->failedAssertThrows( file, line, expression, type, otherThrown );
        }

        void failedAssertThrowsNot( const char *file, unsigned line,
                                    const char *expression )
        {
            _first->failedAssertThrowsNot( file, line, expression );
            _second->failedAssertThrowsNot( file, line, expression );
        }

        void leaveTest( const TestDescription &d )
        {
            _first->leaveTest(d);
            _second->leaveTest(d);
        }

        void leaveSuite( const SuiteDescription &d )
        {
            _first->leaveSuite(d);
            _second->leaveSuite(d);
        }

        void leaveWorld( const WorldDescription &d )
        {
            _first->leaveWorld(d);
            _second->leaveWorld(d);
        }

    private:
        TestListener *_first, *_second;
        TestListener _dummy;
    };
}

#endif // __cxxtest__TeeListener_h__
