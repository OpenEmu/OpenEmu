#ifndef __cxxtest__Mock_h__
#define __cxxtest__Mock_h__

//
// The default namespace is T::
//
#ifndef CXXTEST_MOCK_NAMESPACE
#   define CXXTEST_MOCK_NAMESPACE T
#endif // CXXTEST_MOCK_NAMESPACE

//
// MockTraits: What to return when no mock object has been created
//
#define __CXXTEST_MOCK__TRAITS \
    namespace CXXTEST_MOCK_NAMESPACE \
    { \
        template<class T> \
        class MockTraits \
        { \
        public: \
            static T defaultValue() { return 0; } \
        }; \
    };

//
// extern "C" when needed
//
#ifdef __cplusplus
#   define CXXTEST_EXTERN_C extern "C"
#else
#   define CXXTEST_EXTERN_C
#endif // __cplusplus

//
// Prototypes: For "normal" headers
//
#define __CXXTEST_MOCK__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    namespace CXXTEST_MOCK_NAMESPACE { TYPE NAME ARGS; }

#define __CXXTEST_MOCK_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__PROTOTYPE( MOCK, void, NAME, ARGS, REAL, CALL )

#define __CXXTEST_SUPPLY__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    TYPE REAL ARGS;

#define __CXXTEST_SUPPLY_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY__PROTOTYPE( MOCK, void, NAME, ARGS, REAL, CALL )

//
// Class declarations: For test files
//
#define __CXXTEST_MOCK__CLASS_DECLARATION( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    namespace CXXTEST_MOCK_NAMESPACE { \
        class Base_##MOCK : public CxxTest::Link \
        { \
        public: \
            Base_##MOCK(); \
            ~Base_##MOCK(); \
            bool setUp(); \
            bool tearDown(); \
         \
            static Base_##MOCK &current(); \
         \
            virtual TYPE NAME ARGS = 0; \
         \
        private: \
            static CxxTest::List _list; \
        }; \
         \
        class Real_##MOCK  : public Base_##MOCK \
        { \
        public: \
            TYPE NAME ARGS; \
        }; \
         \
        class _Unimplemented_##MOCK  : public Base_##MOCK \
        { \
        public: \
            TYPE NAME ARGS; \
        }; \
    }

#define __CXXTEST_MOCK_VOID__CLASS_DECLARATION( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__CLASS_DECLARATION( MOCK, void, NAME, ARGS, REAL, CALL )

#define __CXXTEST_SUPPLY__CLASS_DECLARATION( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    namespace CXXTEST_MOCK_NAMESPACE { \
        class Base_##MOCK : public CxxTest::Link \
        { \
        public: \
            Base_##MOCK(); \
            ~Base_##MOCK(); \
            bool setUp(); \
            bool tearDown(); \
         \
            static Base_##MOCK &current(); \
         \
            virtual TYPE NAME ARGS = 0; \
         \
        private: \
            static CxxTest::List _list; \
        }; \
         \
        class _Unimplemented_##MOCK  : public Base_##MOCK \
        { \
        public: \
            TYPE NAME ARGS; \
        }; \
    }

#define __CXXTEST_SUPPLY_VOID__CLASS_DECLARATION( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY__CLASS_DECLARATION( MOCK, void, NAME, ARGS, REAL, CALL )

//
// Class implementation: For test source files
//
#define __CXXTEST_MOCK__COMMON_CLASS_IMPLEMENTATION( MOCK, NAME ) \
    namespace CXXTEST_MOCK_NAMESPACE { \
         \
        CxxTest::List Base_##MOCK::_list = { 0, 0 }; \
         \
        Base_##MOCK::Base_##MOCK() { attach( _list ); } \
        Base_##MOCK::~Base_##MOCK() { detach( _list ); } \
        bool Base_##MOCK::setUp() { return true; } \
        bool Base_##MOCK::tearDown() { return true; } \
         \
        Base_##MOCK &Base_##MOCK::current() \
        { \
            if ( _list.empty() ) \
                static _Unimplemented_##MOCK unimplemented; \
            return *(Base_##MOCK *)_list.tail(); \
        } \
    }

#define __CXXTEST_MOCK__CLASS_IMPLEMENTATION( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__COMMON_CLASS_IMPLEMENTATION( MOCK, NAME ) \
    namespace CXXTEST_MOCK_NAMESPACE { \
        TYPE Real_##MOCK::NAME ARGS \
        { \
            return REAL CALL; \
        } \
         \
        TYPE _Unimplemented_##MOCK::NAME ARGS \
        { \
            while ( false ) \
                return NAME CALL; \
            __CXXTEST_MOCK_UNIMPLEMENTED( NAME, ARGS ); \
            return MockTraits<TYPE>::defaultValue(); \
        } \
         \
        TYPE NAME ARGS \
        { \
            return Base_##MOCK::current().NAME CALL; \
        } \
    }

#define __CXXTEST_MOCK_VOID__CLASS_IMPLEMENTATION( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__COMMON_CLASS_IMPLEMENTATION( MOCK, NAME ) \
    namespace CXXTEST_MOCK_NAMESPACE { \
        void Real_##MOCK::NAME ARGS \
        { \
            REAL CALL; \
        } \
         \
        void _Unimplemented_##MOCK::NAME ARGS \
        { \
            while ( false ) \
                NAME CALL; \
            __CXXTEST_MOCK_UNIMPLEMENTED( NAME, ARGS ); \
        } \
         \
        void NAME ARGS \
        { \
            Base_##MOCK::current().NAME CALL; \
        } \
    }

#define __CXXTEST_SUPPLY__CLASS_IMPLEMENTATION( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__COMMON_CLASS_IMPLEMENTATION( MOCK, NAME ) \
    namespace CXXTEST_MOCK_NAMESPACE { \
        TYPE _Unimplemented_##MOCK::NAME ARGS \
        { \
            while ( false ) \
                return NAME CALL; \
            __CXXTEST_MOCK_UNIMPLEMENTED( NAME, ARGS ); \
            return MockTraits<TYPE>::defaultValue(); \
        } \
    } \
     \
    TYPE REAL ARGS \
    { \
        return CXXTEST_MOCK_NAMESPACE::Base_##MOCK::current().NAME CALL; \
    }

#define __CXXTEST_SUPPLY_VOID__CLASS_IMPLEMENTATION( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__COMMON_CLASS_IMPLEMENTATION( MOCK, NAME ) \
    namespace CXXTEST_MOCK_NAMESPACE { \
        void _Unimplemented_##MOCK::NAME ARGS \
        { \
            while ( false ) \
                NAME CALL; \
            __CXXTEST_MOCK_UNIMPLEMENTED( NAME, ARGS ); \
        } \
    } \
     \
    void REAL ARGS \
    { \
        CXXTEST_MOCK_NAMESPACE::Base_##MOCK::current().NAME CALL; \
    } \

//
// Error for calling mock function w/o object
//
#define __CXXTEST_MOCK_UNIMPLEMENTED( NAME, ARGS ) \
    TS_FAIL( CXXTEST_MOCK_NAMESPACE_STR #NAME #ARGS " called with no " \
             CXXTEST_MOCK_NAMESPACE_STR "Base_" #NAME " object" ); \

#define CXXTEST_MOCK_NAMESPACE_STR __CXXTEST_STR(CXXTEST_MOCK_NAMESPACE) "::"
#define __CXXTEST_STR(X) __CXXTEST_XSTR(X)
#define __CXXTEST_XSTR(X) #X

#if defined(CXXTEST_MOCK_TEST_SOURCE_FILE)
//
// Test source file: Prototypes, class declarations and implementation
//
#include <cxxtest/TestSuite.h>

__CXXTEST_MOCK__TRAITS;

#define CXXTEST_MOCK( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__CLASS_DECLARATION( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__CLASS_IMPLEMENTATION( MOCK, TYPE, NAME, ARGS, REAL, CALL )

#define CXXTEST_MOCK_VOID( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK_VOID__CLASS_DECLARATION( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK_VOID__CLASS_IMPLEMENTATION( MOCK, NAME, ARGS, REAL, CALL )

#define CXXTEST_SUPPLY( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY__CLASS_DECLARATION( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY__CLASS_IMPLEMENTATION( MOCK, TYPE, NAME, ARGS, REAL, CALL )

#define CXXTEST_SUPPLY_VOID( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY_VOID__CLASS_DECLARATION( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY_VOID__CLASS_IMPLEMENTATION( MOCK, NAME, ARGS, REAL, CALL )

#elif defined(CXXTEST_FLAGS) || defined(CXXTEST_RUNNING)
//
// Test file other than source: Prototypes and class declarations
//
#include <cxxtest/TestSuite.h>

__CXXTEST_MOCK__TRAITS;

#define CXXTEST_MOCK( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__CLASS_DECLARATION( MOCK, TYPE, NAME, ARGS, REAL, CALL )

#define CXXTEST_MOCK_VOID( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK_VOID__CLASS_DECLARATION( MOCK, NAME, ARGS, REAL, CALL )

#define CXXTEST_SUPPLY( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY__CLASS_DECLARATION( MOCK, TYPE, NAME, ARGS, REAL, CALL )

#define CXXTEST_SUPPLY_VOID( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY_VOID__CLASS_DECLARATION( MOCK, NAME, ARGS, REAL, CALL )

#elif defined(CXXTEST_MOCK_REAL_SOURCE_FILE)
//
// Real source file: "Real" implementations
//
#define CXXTEST_MOCK( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    namespace CXXTEST_MOCK_NAMESPACE { TYPE NAME ARGS { return REAL CALL; } }

#define CXXTEST_MOCK_VOID( MOCK, NAME, ARGS, REAL, CALL ) \
    namespace CXXTEST_MOCK_NAMESPACE { void NAME ARGS { REAL CALL; } }

#else
//
// Ordinary header file: Just prototypes
//

#define CXXTEST_MOCK( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL )

#define CXXTEST_MOCK_VOID( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_MOCK_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL )

#define CXXTEST_SUPPLY( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL )

#define CXXTEST_SUPPLY_VOID( MOCK, NAME, ARGS, REAL, CALL ) \
    __CXXTEST_SUPPLY_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL )

#endif // Ordinary header file

//
// How to supply extern "C" functions
//
#define CXXTEST_SUPPLY_C( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    CXXTEST_EXTERN_C __CXXTEST_SUPPLY__PROTOTYPE( MOCK, TYPE, NAME, ARGS, REAL, CALL ) \
    CXXTEST_SUPPLY( MOCK, TYPE, NAME, ARGS, REAL, CALL )

#define CXXTEST_SUPPLY_VOID_C( MOCK, NAME, ARGS, REAL, CALL ) \
    CXXTEST_EXTERN_C __CXXTEST_SUPPLY_VOID__PROTOTYPE( MOCK, NAME, ARGS, REAL, CALL ) \
    CXXTEST_SUPPLY_VOID( MOCK, NAME, ARGS, REAL, CALL )

//
// Usually we mean the global namespace
//
#define CXXTEST_MOCK_GLOBAL( TYPE, NAME, ARGS, CALL ) \
    CXXTEST_MOCK( NAME, TYPE, NAME, ARGS, ::NAME, CALL )

#define CXXTEST_MOCK_VOID_GLOBAL( NAME, ARGS, CALL ) \
    CXXTEST_MOCK_VOID( NAME, NAME, ARGS, ::NAME, CALL )

#define CXXTEST_SUPPLY_GLOBAL( TYPE, NAME, ARGS, CALL ) \
    CXXTEST_SUPPLY( NAME, TYPE, NAME, ARGS, NAME, CALL )

#define CXXTEST_SUPPLY_VOID_GLOBAL( NAME, ARGS, CALL ) \
    CXXTEST_SUPPLY_VOID( NAME, NAME, ARGS, NAME, CALL )

#define CXXTEST_SUPPLY_GLOBAL_C( TYPE, NAME, ARGS, CALL ) \
    CXXTEST_SUPPLY_C( NAME, TYPE, NAME, ARGS, NAME, CALL )

#define CXXTEST_SUPPLY_VOID_GLOBAL_C( NAME, ARGS, CALL ) \
    CXXTEST_SUPPLY_VOID_C( NAME, NAME, ARGS, NAME, CALL )

//
// What to return when no mock object has been created.
// The default value of 0 usually works, but some cases may need this.
//
#define CXXTEST_MOCK_DEFAULT_VALUE( TYPE, VALUE ) \
    namespace CXXTEST_MOCK_NAMESPACE \
    { \
        template<> \
        class MockTraits<TYPE> \
        { \
        public: \
            static TYPE defaultValue() { return VALUE; } \
        }; \
    }

#endif // __cxxtest__Mock_h__
