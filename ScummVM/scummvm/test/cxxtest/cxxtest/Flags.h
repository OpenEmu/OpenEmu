#ifndef __cxxtest__Flags_h__
#define __cxxtest__Flags_h__

//
// These are the flags that control CxxTest
//

#if !defined(CXXTEST_FLAGS)
#   define CXXTEST_FLAGS
#endif // !CXXTEST_FLAGS

#if defined(CXXTEST_HAVE_EH) && !defined(_CXXTEST_HAVE_EH)
#   define _CXXTEST_HAVE_EH
#endif // CXXTEST_HAVE_EH

#if defined(CXXTEST_HAVE_STD) && !defined(_CXXTEST_HAVE_STD)
#   define _CXXTEST_HAVE_STD
#endif // CXXTEST_HAVE_STD

#if defined(CXXTEST_OLD_TEMPLATE_SYNTAX) && !defined(_CXXTEST_OLD_TEMPLATE_SYNTAX)
#   define _CXXTEST_OLD_TEMPLATE_SYNTAX
#endif // CXXTEST_OLD_TEMPLATE_SYNTAX

#if defined(CXXTEST_OLD_STD) && !defined(_CXXTEST_OLD_STD)
#   define _CXXTEST_OLD_STD
#endif // CXXTEST_OLD_STD

#if defined(CXXTEST_ABORT_TEST_ON_FAIL) && !defined(_CXXTEST_ABORT_TEST_ON_FAIL)
#   define _CXXTEST_ABORT_TEST_ON_FAIL
#endif // CXXTEST_ABORT_TEST_ON_FAIL

#if defined(CXXTEST_NO_COPY_CONST) && !defined(_CXXTEST_NO_COPY_CONST)
#   define _CXXTEST_NO_COPY_CONST
#endif // CXXTEST_NO_COPY_CONST

#if defined(CXXTEST_FACTOR) && !defined(_CXXTEST_FACTOR)
#   define _CXXTEST_FACTOR
#endif // CXXTEST_FACTOR

#if defined(CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION) && !defined(_CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION)
#   define _CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION
#endif // CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION

#if defined(CXXTEST_LONGLONG)
#   if defined(_CXXTEST_LONGLONG)
#       undef _CXXTEST_LONGLONG
#   endif
#   define _CXXTEST_LONGLONG CXXTEST_LONGLONG
#endif // CXXTEST_LONGLONG

#ifndef CXXTEST_MAX_DUMP_SIZE
#   define CXXTEST_MAX_DUMP_SIZE 0
#endif // CXXTEST_MAX_DUMP_SIZE

#if defined(_CXXTEST_ABORT_TEST_ON_FAIL) && !defined(CXXTEST_DEFAULT_ABORT)
#   define CXXTEST_DEFAULT_ABORT true
#endif // _CXXTEST_ABORT_TEST_ON_FAIL && !CXXTEST_DEFAULT_ABORT

#if !defined(CXXTEST_DEFAULT_ABORT)
#   define CXXTEST_DEFAULT_ABORT false
#endif // !CXXTEST_DEFAULT_ABORT

#if defined(_CXXTEST_ABORT_TEST_ON_FAIL) && !defined(_CXXTEST_HAVE_EH)
#   warning "CXXTEST_ABORT_TEST_ON_FAIL is meaningless without CXXTEST_HAVE_EH"
#   undef _CXXTEST_ABORT_TEST_ON_FAIL
#endif // _CXXTEST_ABORT_TEST_ON_FAIL && !_CXXTEST_HAVE_EH

//
// Some minimal per-compiler configuration to allow us to compile
//

#ifdef __BORLANDC__
#   if __BORLANDC__ <= 0x520 // Borland C++ 5.2 or earlier
#       ifndef _CXXTEST_OLD_STD
#           define _CXXTEST_OLD_STD
#       endif
#       ifndef _CXXTEST_OLD_TEMPLATE_SYNTAX
#           define _CXXTEST_OLD_TEMPLATE_SYNTAX
#       endif
#   endif
#   if __BORLANDC__ >= 0x540 // C++ Builder 4.0 or later
#       ifndef _CXXTEST_NO_COPY_CONST
#           define _CXXTEST_NO_COPY_CONST
#       endif
#       ifndef _CXXTEST_LONGLONG
#           define _CXXTEST_LONGLONG __int64
#       endif
#   endif
#endif // __BORLANDC__

#ifdef _MSC_VER // Visual C++
#   ifndef _CXXTEST_LONGLONG
#       define _CXXTEST_LONGLONG __int64
#   endif
#   if (_MSC_VER >= 0x51E)
#       ifndef _CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION
#           define _CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION
#       endif
#   endif
#   pragma warning( disable : 4127 )
#   pragma warning( disable : 4290 )
#   pragma warning( disable : 4511 )
#   pragma warning( disable : 4512 )
#   pragma warning( disable : 4514 )
#endif // _MSC_VER

#ifdef __GNUC__
#   if (__GNUC__ > 2) || (__GNUC__ == 2 && __GNUC_MINOR__ >= 9)
#       ifndef _CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION
#           define _CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION
#       endif
#   endif
#endif // __GNUC__

#ifdef __DMC__ // Digital Mars
#   ifndef _CXXTEST_OLD_STD
#       define _CXXTEST_OLD_STD
#   endif
#endif

#endif // __cxxtest__Flags_h__
