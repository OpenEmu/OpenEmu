#ifndef __cxxtest__TestSuite_h__
#define __cxxtest__TestSuite_h__

//
// class TestSuite is the base class for all test suites.
// To define a test suite, derive from this class and add
// member functions called void test*();
//

#include <cxxtest/Flags.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/Descriptions.h>
#include <cxxtest/ValueTraits.h>

#ifdef _CXXTEST_HAVE_STD
#   include <stdexcept>
#endif // _CXXTEST_HAVE_STD

namespace CxxTest
{
    class TestSuite
    {
    public:
        virtual ~TestSuite();
        virtual void setUp();
        virtual void tearDown();
    };

    class AbortTest {};
    void doAbortTest();
#   define TS_ABORT() CxxTest::doAbortTest()

    bool abortTestOnFail();
    void setAbortTestOnFail( bool value = CXXTEST_DEFAULT_ABORT );

    unsigned maxDumpSize();
    void setMaxDumpSize( unsigned value = CXXTEST_MAX_DUMP_SIZE );

    void doTrace( const char *file, unsigned line, const char *message );
    void doWarn( const char *file, unsigned line, const char *message );
    void doFailTest( const char *file, unsigned line, const char *message );
    void doFailAssert( const char *file, unsigned line, const char *expression, const char *message );

    template<class X, class Y>
    bool equals( X x, Y y )
    {
        return (x == y);
    }

    template<class X, class Y>
    void doAssertEquals( const char *file, unsigned line,
                         const char *xExpr, X x,
                         const char *yExpr, Y y,
                         const char *message )
    {
        if ( !equals( x, y ) ) {
            if ( message )
                tracker().failedTest( file, line, message );
            tracker().failedAssertEquals( file, line, xExpr, yExpr, TS_AS_STRING(x), TS_AS_STRING(y) );
            TS_ABORT();
        }
    }

    void doAssertSameData( const char *file, unsigned line,
                           const char *xExpr, const void *x,
                           const char *yExpr, const void *y,
                           const char *sizeExpr, unsigned size,
                           const char *message );

    template<class X, class Y>
    bool differs( X x, Y y )
    {
        return !(x == y);
    }

    template<class X, class Y>
    void doAssertDiffers( const char *file, unsigned line,
                          const char *xExpr, X x,
                          const char *yExpr, Y y,
                          const char *message )
    {
        if ( !differs( x, y ) ) {
            if ( message )
                tracker().failedTest( file, line, message );
            tracker().failedAssertDiffers( file, line, xExpr, yExpr, TS_AS_STRING(x) );
            TS_ABORT();
        }
    }

    template<class X, class Y>
    bool lessThan( X x, Y y )
    {
        return (x < y);
    }

    template<class X, class Y>
    void doAssertLessThan( const char *file, unsigned line,
                           const char *xExpr, X x,
                           const char *yExpr, Y y,
                           const char *message )
    {
        if ( !lessThan(x, y) ) {
            if ( message )
                tracker().failedTest( file, line, message );
            tracker().failedAssertLessThan( file, line, xExpr, yExpr, TS_AS_STRING(x), TS_AS_STRING(y) );
            TS_ABORT();
        }
    }

    template<class X, class Y>
    bool lessThanEquals( X x, Y y )
    {
        return (x <= y);
    }

    template<class X, class Y>
    void doAssertLessThanEquals( const char *file, unsigned line,
                                 const char *xExpr, X x,
                                 const char *yExpr, Y y,
                                 const char *message )
    {
        if ( !lessThanEquals( x, y ) ) {
            if ( message )
                tracker().failedTest( file, line, message );
            tracker().failedAssertLessThanEquals( file, line, xExpr, yExpr, TS_AS_STRING(x), TS_AS_STRING(y) );
            TS_ABORT();
        }
    }

    template<class X, class P>
    void doAssertPredicate( const char *file, unsigned line,
                            const char *pExpr, const P &p,
                            const char *xExpr, X x,
                            const char *message )
    {
        if ( !p( x ) ) {
            if ( message )
                tracker().failedTest( file, line, message );
            tracker().failedAssertPredicate( file, line, pExpr, xExpr, TS_AS_STRING(x) );
            TS_ABORT();
        }
    }

    template<class X, class Y, class R>
    void doAssertRelation( const char *file, unsigned line,
                           const char *rExpr, const R &r,
                           const char *xExpr, X x,
                           const char *yExpr, Y y,
                           const char *message )
    {
        if ( !r( x, y ) ) {
            if ( message )
                tracker().failedTest( file, line, message );
            tracker().failedAssertRelation( file, line, rExpr, xExpr, yExpr, TS_AS_STRING(x), TS_AS_STRING(y) );
            TS_ABORT();
        }
    }

    template<class X, class Y, class D>
    bool delta( X x, Y y, D d )
    {
        return ((y >= x - d) && (y <= x + d));
    }

    template<class X, class Y, class D>
    void doAssertDelta( const char *file, unsigned line,
                        const char *xExpr, X x,
                        const char *yExpr, Y y,
                        const char *dExpr, D d,
                        const char *message )
    {
        if ( !delta( x, y, d ) ) {
            if ( message )
                tracker().failedTest( file, line, message );

            tracker().failedAssertDelta( file, line, xExpr, yExpr, dExpr,
                                         TS_AS_STRING(x), TS_AS_STRING(y), TS_AS_STRING(d) );
            TS_ABORT();
        }
    }

    void doFailAssertThrows( const char *file, unsigned line,
                             const char *expr, const char *type,
                             bool otherThrown,
                             const char *message );

    void doFailAssertThrowsNot( const char *file, unsigned line,
                                const char *expression, const char *message );

#   ifdef _CXXTEST_HAVE_EH
#       define _TS_TRY try
#       define _TS_CATCH_TYPE(t, b) catch t b
#       define _TS_CATCH_ABORT(b) _TS_CATCH_TYPE( (const CxxTest::AbortTest &), b )
#       define _TS_LAST_CATCH(b) _TS_CATCH_TYPE( (...), b )
#       define _TSM_LAST_CATCH(f,l,m) _TS_LAST_CATCH( { (CxxTest::tracker()).failedTest(f,l,m); } )
#       ifdef _CXXTEST_HAVE_STD
#           define ___TSM_CATCH(f,l,m) \
                    catch(const std::exception &e) { (CxxTest::tracker()).failedTest(f,l,e.what()); } \
                    _TSM_LAST_CATCH(f,l,m)
#       else // !_CXXTEST_HAVE_STD
#           define ___TSM_CATCH(f,l,m) _TSM_LAST_CATCH(f,l,m)
#       endif // _CXXTEST_HAVE_STD
#       define __TSM_CATCH(f,l,m) \
                _TS_CATCH_ABORT( { throw; } ) \
                ___TSM_CATCH(f,l,m)
#       define __TS_CATCH(f,l) __TSM_CATCH(f,l,"Unhandled exception")
#       define _TS_CATCH __TS_CATCH(__FILE__,__LINE__)
#   else // !_CXXTEST_HAVE_EH
#       define _TS_TRY
#       define ___TSM_CATCH(f,l,m)
#       define __TSM_CATCH(f,l,m)
#       define __TS_CATCH(f,l)
#       define _TS_CATCH
#       define _TS_CATCH_TYPE(t, b)
#       define _TS_LAST_CATCH(b)
#       define _TS_CATCH_ABORT(b)
#   endif // _CXXTEST_HAVE_EH

    // TS_TRACE
#   define _TS_TRACE(f,l,e) CxxTest::doTrace( (f), (l), TS_AS_STRING(e) )
#   define TS_TRACE(e) _TS_TRACE( __FILE__, __LINE__, e )

    // TS_WARN
#   define _TS_WARN(f,l,e) CxxTest::doWarn( (f), (l), TS_AS_STRING(e) )
#   define TS_WARN(e) _TS_WARN( __FILE__, __LINE__, e )

    // TS_FAIL
#   define _TS_FAIL(f,l,e) CxxTest::doFailTest( (f), (l), TS_AS_STRING(e) )
#   define TS_FAIL(e) _TS_FAIL( __FILE__, __LINE__, e )

    // TS_ASSERT
#   define ___ETS_ASSERT(f,l,e,m) { if ( !(e) ) CxxTest::doFailAssert( (f), (l), #e, (m) ); }
#   define ___TS_ASSERT(f,l,e,m) { _TS_TRY { ___ETS_ASSERT(f,l,e,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT(f,l,e) ___ETS_ASSERT(f,l,e,0)
#   define _TS_ASSERT(f,l,e) ___TS_ASSERT(f,l,e,0)

#   define ETS_ASSERT(e) _ETS_ASSERT(__FILE__,__LINE__,e)
#   define TS_ASSERT(e) _TS_ASSERT(__FILE__,__LINE__,e)

#   define _ETSM_ASSERT(f,l,m,e) ___ETS_ASSERT(f,l,e,TS_AS_STRING(m) )
#   define _TSM_ASSERT(f,l,m,e) ___TS_ASSERT(f,l,e,TS_AS_STRING(m) )

#   define ETSM_ASSERT(m,e) _ETSM_ASSERT(__FILE__,__LINE__,m,e)
#   define TSM_ASSERT(m,e) _TSM_ASSERT(__FILE__,__LINE__,m,e)

    // TS_ASSERT_EQUALS
#   define ___ETS_ASSERT_EQUALS(f,l,x,y,m) CxxTest::doAssertEquals( (f), (l), #x, (x), #y, (y), (m) )
#   define ___TS_ASSERT_EQUALS(f,l,x,y,m) { _TS_TRY { ___ETS_ASSERT_EQUALS(f,l,x,y,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT_EQUALS(f,l,x,y) ___ETS_ASSERT_EQUALS(f,l,x,y,0)
#   define _TS_ASSERT_EQUALS(f,l,x,y) ___TS_ASSERT_EQUALS(f,l,x,y,0)

#   define ETS_ASSERT_EQUALS(x,y) _ETS_ASSERT_EQUALS(__FILE__,__LINE__,x,y)
#   define TS_ASSERT_EQUALS(x,y) _TS_ASSERT_EQUALS(__FILE__,__LINE__,x,y)

#   define _ETSM_ASSERT_EQUALS(f,l,m,x,y) ___ETS_ASSERT_EQUALS(f,l,x,y,TS_AS_STRING(m))
#   define _TSM_ASSERT_EQUALS(f,l,m,x,y) ___TS_ASSERT_EQUALS(f,l,x,y,TS_AS_STRING(m))

#   define ETSM_ASSERT_EQUALS(m,x,y) _ETSM_ASSERT_EQUALS(__FILE__,__LINE__,m,x,y)
#   define TSM_ASSERT_EQUALS(m,x,y) _TSM_ASSERT_EQUALS(__FILE__,__LINE__,m,x,y)

    // TS_ASSERT_SAME_DATA
#   define ___ETS_ASSERT_SAME_DATA(f,l,x,y,s,m) CxxTest::doAssertSameData( (f), (l), #x, (x), #y, (y), #s, (s), (m) )
#   define ___TS_ASSERT_SAME_DATA(f,l,x,y,s,m) { _TS_TRY { ___ETS_ASSERT_SAME_DATA(f,l,x,y,s,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT_SAME_DATA(f,l,x,y,s) ___ETS_ASSERT_SAME_DATA(f,l,x,y,s,0)
#   define _TS_ASSERT_SAME_DATA(f,l,x,y,s) ___TS_ASSERT_SAME_DATA(f,l,x,y,s,0)

#   define ETS_ASSERT_SAME_DATA(x,y,s) _ETS_ASSERT_SAME_DATA(__FILE__,__LINE__,x,y,s)
#   define TS_ASSERT_SAME_DATA(x,y,s) _TS_ASSERT_SAME_DATA(__FILE__,__LINE__,x,y,s)

#   define _ETSM_ASSERT_SAME_DATA(f,l,m,x,y,s) ___ETS_ASSERT_SAME_DATA(f,l,x,y,s,TS_AS_STRING(m))
#   define _TSM_ASSERT_SAME_DATA(f,l,m,x,y,s) ___TS_ASSERT_SAME_DATA(f,l,x,y,s,TS_AS_STRING(m))

#   define ETSM_ASSERT_SAME_DATA(m,x,y,s) _ETSM_ASSERT_SAME_DATA(__FILE__,__LINE__,m,x,y,s)
#   define TSM_ASSERT_SAME_DATA(m,x,y,s) _TSM_ASSERT_SAME_DATA(__FILE__,__LINE__,m,x,y,s)

    // TS_ASSERT_DIFFERS
#   define ___ETS_ASSERT_DIFFERS(f,l,x,y,m) CxxTest::doAssertDiffers( (f), (l), #x, (x), #y, (y), (m) )
#   define ___TS_ASSERT_DIFFERS(f,l,x,y,m) { _TS_TRY { ___ETS_ASSERT_DIFFERS(f,l,x,y,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT_DIFFERS(f,l,x,y) ___ETS_ASSERT_DIFFERS(f,l,x,y,0)
#   define _TS_ASSERT_DIFFERS(f,l,x,y) ___TS_ASSERT_DIFFERS(f,l,x,y,0)

#   define ETS_ASSERT_DIFFERS(x,y) _ETS_ASSERT_DIFFERS(__FILE__,__LINE__,x,y)
#   define TS_ASSERT_DIFFERS(x,y) _TS_ASSERT_DIFFERS(__FILE__,__LINE__,x,y)

#   define _ETSM_ASSERT_DIFFERS(f,l,m,x,y) ___ETS_ASSERT_DIFFERS(f,l,x,y,TS_AS_STRING(m))
#   define _TSM_ASSERT_DIFFERS(f,l,m,x,y) ___TS_ASSERT_DIFFERS(f,l,x,y,TS_AS_STRING(m))

#   define ETSM_ASSERT_DIFFERS(m,x,y) _ETSM_ASSERT_DIFFERS(__FILE__,__LINE__,m,x,y)
#   define TSM_ASSERT_DIFFERS(m,x,y) _TSM_ASSERT_DIFFERS(__FILE__,__LINE__,m,x,y)

    // TS_ASSERT_LESS_THAN
#   define ___ETS_ASSERT_LESS_THAN(f,l,x,y,m) CxxTest::doAssertLessThan( (f), (l), #x, (x), #y, (y), (m) )
#   define ___TS_ASSERT_LESS_THAN(f,l,x,y,m) { _TS_TRY { ___ETS_ASSERT_LESS_THAN(f,l,x,y,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT_LESS_THAN(f,l,x,y) ___ETS_ASSERT_LESS_THAN(f,l,x,y,0)
#   define _TS_ASSERT_LESS_THAN(f,l,x,y) ___TS_ASSERT_LESS_THAN(f,l,x,y,0)

#   define ETS_ASSERT_LESS_THAN(x,y) _ETS_ASSERT_LESS_THAN(__FILE__,__LINE__,x,y)
#   define TS_ASSERT_LESS_THAN(x,y) _TS_ASSERT_LESS_THAN(__FILE__,__LINE__,x,y)

#   define _ETSM_ASSERT_LESS_THAN(f,l,m,x,y) ___ETS_ASSERT_LESS_THAN(f,l,x,y,TS_AS_STRING(m))
#   define _TSM_ASSERT_LESS_THAN(f,l,m,x,y) ___TS_ASSERT_LESS_THAN(f,l,x,y,TS_AS_STRING(m))

#   define ETSM_ASSERT_LESS_THAN(m,x,y) _ETSM_ASSERT_LESS_THAN(__FILE__,__LINE__,m,x,y)
#   define TSM_ASSERT_LESS_THAN(m,x,y) _TSM_ASSERT_LESS_THAN(__FILE__,__LINE__,m,x,y)

    // TS_ASSERT_LESS_THAN_EQUALS
#   define ___ETS_ASSERT_LESS_THAN_EQUALS(f,l,x,y,m) \
        CxxTest::doAssertLessThanEquals( (f), (l), #x, (x), #y, (y), (m) )
#   define ___TS_ASSERT_LESS_THAN_EQUALS(f,l,x,y,m) \
        { _TS_TRY { ___ETS_ASSERT_LESS_THAN_EQUALS(f,l,x,y,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT_LESS_THAN_EQUALS(f,l,x,y) ___ETS_ASSERT_LESS_THAN_EQUALS(f,l,x,y,0)
#   define _TS_ASSERT_LESS_THAN_EQUALS(f,l,x,y) ___TS_ASSERT_LESS_THAN_EQUALS(f,l,x,y,0)

#   define ETS_ASSERT_LESS_THAN_EQUALS(x,y) _ETS_ASSERT_LESS_THAN_EQUALS(__FILE__,__LINE__,x,y)
#   define TS_ASSERT_LESS_THAN_EQUALS(x,y) _TS_ASSERT_LESS_THAN_EQUALS(__FILE__,__LINE__,x,y)

#   define _ETSM_ASSERT_LESS_THAN_EQUALS(f,l,m,x,y) ___ETS_ASSERT_LESS_THAN_EQUALS(f,l,x,y,TS_AS_STRING(m))
#   define _TSM_ASSERT_LESS_THAN_EQUALS(f,l,m,x,y) ___TS_ASSERT_LESS_THAN_EQUALS(f,l,x,y,TS_AS_STRING(m))

#   define ETSM_ASSERT_LESS_THAN_EQUALS(m,x,y) _ETSM_ASSERT_LESS_THAN_EQUALS(__FILE__,__LINE__,m,x,y)
#   define TSM_ASSERT_LESS_THAN_EQUALS(m,x,y) _TSM_ASSERT_LESS_THAN_EQUALS(__FILE__,__LINE__,m,x,y)

    // TS_ASSERT_PREDICATE
#   define ___ETS_ASSERT_PREDICATE(f,l,p,x,m) \
        CxxTest::doAssertPredicate( (f), (l), #p, p(), #x, (x), (m) )
#   define ___TS_ASSERT_PREDICATE(f,l,p,x,m) \
        { _TS_TRY { ___ETS_ASSERT_PREDICATE(f,l,p,x,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT_PREDICATE(f,l,p,x) ___ETS_ASSERT_PREDICATE(f,l,p,x,0)
#   define _TS_ASSERT_PREDICATE(f,l,p,x) ___TS_ASSERT_PREDICATE(f,l,p,x,0)

#   define ETS_ASSERT_PREDICATE(p,x) _ETS_ASSERT_PREDICATE(__FILE__,__LINE__,p,x)
#   define TS_ASSERT_PREDICATE(p,x) _TS_ASSERT_PREDICATE(__FILE__,__LINE__,p,x)

#   define _ETSM_ASSERT_PREDICATE(f,l,m,p,x) ___ETS_ASSERT_PREDICATE(f,l,p,x,TS_AS_STRING(m))
#   define _TSM_ASSERT_PREDICATE(f,l,m,p,x) ___TS_ASSERT_PREDICATE(f,l,p,x,TS_AS_STRING(m))

#   define ETSM_ASSERT_PREDICATE(m,p,x) _ETSM_ASSERT_PREDICATE(__FILE__,__LINE__,m,p,x)
#   define TSM_ASSERT_PREDICATE(m,p,x) _TSM_ASSERT_PREDICATE(__FILE__,__LINE__,m,p,x)

    // TS_ASSERT_RELATION
#   define ___ETS_ASSERT_RELATION(f,l,r,x,y,m) \
        CxxTest::doAssertRelation( (f), (l), #r, r(), #x, (x), #y, (y), (m) )
#   define ___TS_ASSERT_RELATION(f,l,r,x,y,m) \
        { _TS_TRY { ___ETS_ASSERT_RELATION(f,l,r,x,y,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT_RELATION(f,l,r,x,y) ___ETS_ASSERT_RELATION(f,l,r,x,y,0)
#   define _TS_ASSERT_RELATION(f,l,r,x,y) ___TS_ASSERT_RELATION(f,l,r,x,y,0)

#   define ETS_ASSERT_RELATION(r,x,y) _ETS_ASSERT_RELATION(__FILE__,__LINE__,r,x,y)
#   define TS_ASSERT_RELATION(r,x,y) _TS_ASSERT_RELATION(__FILE__,__LINE__,r,x,y)

#   define _ETSM_ASSERT_RELATION(f,l,m,r,x,y) ___ETS_ASSERT_RELATION(f,l,r,x,y,TS_AS_STRING(m))
#   define _TSM_ASSERT_RELATION(f,l,m,r,x,y) ___TS_ASSERT_RELATION(f,l,r,x,y,TS_AS_STRING(m))

#   define ETSM_ASSERT_RELATION(m,r,x,y) _ETSM_ASSERT_RELATION(__FILE__,__LINE__,m,r,x,y)
#   define TSM_ASSERT_RELATION(m,r,x,y) _TSM_ASSERT_RELATION(__FILE__,__LINE__,m,r,x,y)

    // TS_ASSERT_DELTA
#   define ___ETS_ASSERT_DELTA(f,l,x,y,d,m) CxxTest::doAssertDelta( (f), (l), #x, (x), #y, (y), #d, (d), (m) )
#   define ___TS_ASSERT_DELTA(f,l,x,y,d,m) { _TS_TRY { ___ETS_ASSERT_DELTA(f,l,x,y,d,m); } __TS_CATCH(f,l) }

#   define _ETS_ASSERT_DELTA(f,l,x,y,d) ___ETS_ASSERT_DELTA(f,l,x,y,d,0)
#   define _TS_ASSERT_DELTA(f,l,x,y,d) ___TS_ASSERT_DELTA(f,l,x,y,d,0)

#   define ETS_ASSERT_DELTA(x,y,d) _ETS_ASSERT_DELTA(__FILE__,__LINE__,x,y,d)
#   define TS_ASSERT_DELTA(x,y,d) _TS_ASSERT_DELTA(__FILE__,__LINE__,x,y,d)

#   define _ETSM_ASSERT_DELTA(f,l,m,x,y,d) ___ETS_ASSERT_DELTA(f,l,x,y,d,TS_AS_STRING(m))
#   define _TSM_ASSERT_DELTA(f,l,m,x,y,d) ___TS_ASSERT_DELTA(f,l,x,y,d,TS_AS_STRING(m))

#   define ETSM_ASSERT_DELTA(m,x,y,d) _ETSM_ASSERT_DELTA(__FILE__,__LINE__,m,x,y,d)
#   define TSM_ASSERT_DELTA(m,x,y,d) _TSM_ASSERT_DELTA(__FILE__,__LINE__,m,x,y,d)

    // TS_ASSERT_THROWS
#   define ___TS_ASSERT_THROWS(f,l,e,t,m) { \
            bool _ts_threw_expected = false, _ts_threw_else = false; \
            _TS_TRY { e; } \
            _TS_CATCH_TYPE( (t), { _ts_threw_expected = true; } ) \
            _TS_CATCH_ABORT( { throw; } ) \
            _TS_LAST_CATCH( { _ts_threw_else = true; } ) \
            if ( !_ts_threw_expected ) { CxxTest::doFailAssertThrows( (f), (l), #e, #t, _ts_threw_else, (m) ); } }

#   define _TS_ASSERT_THROWS(f,l,e,t) ___TS_ASSERT_THROWS(f,l,e,t,0)
#   define TS_ASSERT_THROWS(e,t) _TS_ASSERT_THROWS(__FILE__,__LINE__,e,t)

#   define _TSM_ASSERT_THROWS(f,l,m,e,t) ___TS_ASSERT_THROWS(f,l,e,t,TS_AS_STRING(m))
#   define TSM_ASSERT_THROWS(m,e,t) _TSM_ASSERT_THROWS(__FILE__,__LINE__,m,e,t)

    // TS_ASSERT_THROWS_ASSERT
#   define ___TS_ASSERT_THROWS_ASSERT(f,l,e,t,a,m) { \
            bool _ts_threw_expected = false, _ts_threw_else = false; \
            _TS_TRY { e; } \
            _TS_CATCH_TYPE( (t), { a; _ts_threw_expected = true; } ) \
            _TS_CATCH_ABORT( { throw; } ) \
            _TS_LAST_CATCH( { _ts_threw_else = true; } ) \
            if ( !_ts_threw_expected ) { CxxTest::doFailAssertThrows( (f), (l), #e, #t, _ts_threw_else, (m) ); } }

#   define _TS_ASSERT_THROWS_ASSERT(f,l,e,t,a) ___TS_ASSERT_THROWS_ASSERT(f,l,e,t,a,0)
#   define TS_ASSERT_THROWS_ASSERT(e,t,a) _TS_ASSERT_THROWS_ASSERT(__FILE__,__LINE__,e,t,a)

#   define _TSM_ASSERT_THROWS_ASSERT(f,l,m,e,t,a) ___TS_ASSERT_THROWS_ASSERT(f,l,e,t,a,TS_AS_STRING(m))
#   define TSM_ASSERT_THROWS_ASSERT(m,e,t,a) _TSM_ASSERT_THROWS_ASSERT(__FILE__,__LINE__,m,e,t,a)

    // TS_ASSERT_THROWS_EQUALS
#   define TS_ASSERT_THROWS_EQUALS(e,t,x,y) TS_ASSERT_THROWS_ASSERT(e,t,TS_ASSERT_EQUALS(x,y))
#   define TSM_ASSERT_THROWS_EQUALS(m,e,t,x,y) TSM_ASSERT_THROWS_ASSERT(m,e,t,TSM_ASSERT_EQUALS(m,x,y))

    // TS_ASSERT_THROWS_DIFFERS
#   define TS_ASSERT_THROWS_DIFFERS(e,t,x,y) TS_ASSERT_THROWS_ASSERT(e,t,TS_ASSERT_DIFFERS(x,y))
#   define TSM_ASSERT_THROWS_DIFFERS(m,e,t,x,y) TSM_ASSERT_THROWS_ASSERT(m,e,t,TSM_ASSERT_DIFFERS(m,x,y))

    // TS_ASSERT_THROWS_DELTA
#   define TS_ASSERT_THROWS_DELTA(e,t,x,y,d) TS_ASSERT_THROWS_ASSERT(e,t,TS_ASSERT_DELTA(x,y,d))
#   define TSM_ASSERT_THROWS_DELTA(m,e,t,x,y,d) TSM_ASSERT_THROWS_ASSERT(m,e,t,TSM_ASSERT_DELTA(m,x,y,d))

    // TS_ASSERT_THROWS_SAME_DATA
#   define TS_ASSERT_THROWS_SAME_DATA(e,t,x,y,s) TS_ASSERT_THROWS_ASSERT(e,t,TS_ASSERT_SAME_DATA(x,y,s))
#   define TSM_ASSERT_THROWS_SAME_DATA(m,e,t,x,y,s) TSM_ASSERT_THROWS_ASSERT(m,e,t,TSM_ASSERT_SAME_DATA(m,x,y,s))

    // TS_ASSERT_THROWS_LESS_THAN
#   define TS_ASSERT_THROWS_LESS_THAN(e,t,x,y) TS_ASSERT_THROWS_ASSERT(e,t,TS_ASSERT_LESS_THAN(x,y))
#   define TSM_ASSERT_THROWS_LESS_THAN(m,e,t,x,y) TSM_ASSERT_THROWS_ASSERT(m,e,t,TSM_ASSERT_LESS_THAN(m,x,y))

    // TS_ASSERT_THROWS_LESS_THAN_EQUALS
#   define TS_ASSERT_THROWS_LESS_THAN_EQUALS(e,t,x,y) TS_ASSERT_THROWS_ASSERT(e,t,TS_ASSERT_LESS_THAN_EQUALS(x,y))
#   define TSM_ASSERT_THROWS_LESS_THAN_EQUALS(m,e,t,x,y) TSM_ASSERT_THROWS_ASSERT(m,e,t,TSM_ASSERT_LESS_THAN_EQUALS(m,x,y))

    // TS_ASSERT_THROWS_PREDICATE
#   define TS_ASSERT_THROWS_PREDICATE(e,t,p,v) TS_ASSERT_THROWS_ASSERT(e,t,TS_ASSERT_PREDICATE(p,v))
#   define TSM_ASSERT_THROWS_PREDICATE(m,e,t,p,v) TSM_ASSERT_THROWS_ASSERT(m,e,t,TSM_ASSERT_PREDICATE(m,p,v))

    // TS_ASSERT_THROWS_RELATION
#   define TS_ASSERT_THROWS_RELATION(e,t,r,x,y) TS_ASSERT_THROWS_ASSERT(e,t,TS_ASSERT_RELATION(r,x,y))
#   define TSM_ASSERT_THROWS_RELATION(m,e,t,r,x,y) TSM_ASSERT_THROWS_ASSERT(m,e,t,TSM_ASSERT_RELATION(m,r,x,y))

    // TS_ASSERT_THROWS_ANYTHING
#   define ___TS_ASSERT_THROWS_ANYTHING(f,l,e,m) { \
            bool _ts_threw = false; \
            _TS_TRY { e; } \
            _TS_LAST_CATCH( { _ts_threw = true; } ) \
            if ( !_ts_threw ) { CxxTest::doFailAssertThrows( (f), (l), #e, "...", false, (m) ); } }

#   define _TS_ASSERT_THROWS_ANYTHING(f,l,e) ___TS_ASSERT_THROWS_ANYTHING(f,l,e,0)
#   define TS_ASSERT_THROWS_ANYTHING(e) _TS_ASSERT_THROWS_ANYTHING(__FILE__, __LINE__, e)

#   define _TSM_ASSERT_THROWS_ANYTHING(f,l,m,e) ___TS_ASSERT_THROWS_ANYTHING(f,l,e,TS_AS_STRING(m))
#   define TSM_ASSERT_THROWS_ANYTHING(m,e) _TSM_ASSERT_THROWS_ANYTHING(__FILE__,__LINE__,m,e)

    // TS_ASSERT_THROWS_NOTHING
#   define ___TS_ASSERT_THROWS_NOTHING(f,l,e,m) { \
            _TS_TRY { e; } \
            _TS_CATCH_ABORT( { throw; } ) \
            _TS_LAST_CATCH( { CxxTest::doFailAssertThrowsNot( (f), (l), #e, (m) ); } ) }

#   define _TS_ASSERT_THROWS_NOTHING(f,l,e) ___TS_ASSERT_THROWS_NOTHING(f,l,e,0)
#   define TS_ASSERT_THROWS_NOTHING(e) _TS_ASSERT_THROWS_NOTHING(__FILE__,__LINE__,e)

#   define _TSM_ASSERT_THROWS_NOTHING(f,l,m,e) ___TS_ASSERT_THROWS_NOTHING(f,l,e,TS_AS_STRING(m))
#   define TSM_ASSERT_THROWS_NOTHING(m,e) _TSM_ASSERT_THROWS_NOTHING(__FILE__,__LINE__,m,e)


    //
    // This takes care of "signed <-> unsigned" warnings
    //
#   define CXXTEST_COMPARISONS(CXXTEST_X, CXXTEST_Y, CXXTEST_T) \
    inline bool equals( CXXTEST_X x, CXXTEST_Y y ) { return (((CXXTEST_T)x) == ((CXXTEST_T)y)); } \
    inline bool equals( CXXTEST_Y y, CXXTEST_X x ) { return (((CXXTEST_T)y) == ((CXXTEST_T)x)); } \
    inline bool differs( CXXTEST_X x, CXXTEST_Y y ) { return (((CXXTEST_T)x) != ((CXXTEST_T)y)); } \
    inline bool differs( CXXTEST_Y y, CXXTEST_X x ) { return (((CXXTEST_T)y) != ((CXXTEST_T)x)); } \
    inline bool lessThan( CXXTEST_X x, CXXTEST_Y y ) { return (((CXXTEST_T)x) < ((CXXTEST_T)y)); } \
    inline bool lessThan( CXXTEST_Y y, CXXTEST_X x ) { return (((CXXTEST_T)y) < ((CXXTEST_T)x)); } \
    inline bool lessThanEquals( CXXTEST_X x, CXXTEST_Y y ) { return (((CXXTEST_T)x) <= ((CXXTEST_T)y)); } \
    inline bool lessThanEquals( CXXTEST_Y y, CXXTEST_X x ) { return (((CXXTEST_T)y) <= ((CXXTEST_T)x)); }

#   define CXXTEST_INTEGRAL(CXXTEST_T) \
    CXXTEST_COMPARISONS( signed CXXTEST_T, unsigned CXXTEST_T, unsigned CXXTEST_T )

    CXXTEST_INTEGRAL( char )
    CXXTEST_INTEGRAL( short )
    CXXTEST_INTEGRAL( int )
    CXXTEST_INTEGRAL( long )
#   ifdef _CXXTEST_LONGLONG
    CXXTEST_INTEGRAL( _CXXTEST_LONGLONG )
#   endif // _CXXTEST_LONGLONG

#   define CXXTEST_SMALL_BIG(CXXTEST_SMALL, CXXTEST_BIG) \
    CXXTEST_COMPARISONS( signed CXXTEST_SMALL, unsigned CXXTEST_BIG, unsigned CXXTEST_BIG ) \
    CXXTEST_COMPARISONS( signed CXXTEST_BIG, unsigned CXXTEST_SMALL, unsigned CXXTEST_BIG )

    CXXTEST_SMALL_BIG( char, short )
    CXXTEST_SMALL_BIG( char, int )
    CXXTEST_SMALL_BIG( short, int )
    CXXTEST_SMALL_BIG( char, long )
    CXXTEST_SMALL_BIG( short, long )
    CXXTEST_SMALL_BIG( int, long )

#   ifdef _CXXTEST_LONGLONG
    CXXTEST_SMALL_BIG( char, _CXXTEST_LONGLONG )
    CXXTEST_SMALL_BIG( short, _CXXTEST_LONGLONG )
    CXXTEST_SMALL_BIG( int, _CXXTEST_LONGLONG )
    CXXTEST_SMALL_BIG( long, _CXXTEST_LONGLONG )
#   endif // _CXXTEST_LONGLONG
}

#endif // __cxxtest__TestSuite_h__
