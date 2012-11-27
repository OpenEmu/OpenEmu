#ifndef __cxxtest__ValueTraits_h__
#define __cxxtest__ValueTraits_h__

//
// ValueTraits are used by CxxTest to convert arbitrary
// values used in TS_ASSERT_EQUALS() to a string representation.
//
// This header file contains value traits for builtin integral types.
// To declare value traits for new types you should instantiate the class
// ValueTraits<YourClass>.
//

#include <cxxtest/Flags.h>

#ifdef _CXXTEST_OLD_TEMPLATE_SYNTAX
#   define CXXTEST_TEMPLATE_INSTANTIATION
#else // !_CXXTEST_OLD_TEMPLATE_SYNTAX
#   define CXXTEST_TEMPLATE_INSTANTIATION template<>
#endif // _CXXTEST_OLD_TEMPLATE_SYNTAX

namespace CxxTest
{
    //
    // This is how we use the value traits
    //
#   define TS_AS_STRING(x) CxxTest::traits(x).asString()

    //
    // Char representation of a digit
    //
    char digitToChar( unsigned digit );

    //
    // Convert byte value to hex digits
    // Returns pointer to internal buffer
    //
    const char *byteToHex( unsigned char byte );

    //
    // Convert byte values to string
    // Returns one past the copied data
    //
    char *bytesToString( const unsigned char *bytes, unsigned numBytes, unsigned maxBytes, char *s );

    //
    // Copy a string.
    // Returns one past the end of the destination string
    // Remember -- we can't use the standard library!
    //
    char *copyString( char *dst, const char *src );

    //
    // Compare two strings.
    // Remember -- we can't use the standard library!
    //
    bool stringsEqual( const char *s1, const char *s2 );

    //
    // Represent a character value as a string
    // Returns one past the end of the string
    // This will be the actual char if printable or '\xXXXX' otherwise
    //
    char *charToString( unsigned long c, char *s );

    //
    // Prevent problems with negative (signed char)s
    //
    char *charToString( char c, char *s );

    //
    // The default ValueTraits class dumps up to 8 bytes as hex values
    //
    template<class T>
    class ValueTraits
    {
        enum { MAX_BYTES = 8 };
        char _asString[sizeof("{ ") + sizeof("XX ") * MAX_BYTES + sizeof("... }")];

    public:
        ValueTraits( const T &t ) { bytesToString( (const unsigned char *)&t, sizeof(T), MAX_BYTES, _asString ); }
        const char *asString( void ) const { return _asString; }
    };

    //
    // traits( T t )
    // Creates an object of type ValueTraits<T>
    //
    template<class T>
    inline ValueTraits<T> traits( T t )
    {
        return ValueTraits<T>( t );
    }

    //
    // You can duplicate the implementation of an existing ValueTraits
    //
#   define CXXTEST_COPY_TRAITS(CXXTEST_NEW_CLASS, CXXTEST_OLD_CLASS) \
    CXXTEST_TEMPLATE_INSTANTIATION \
    class ValueTraits< CXXTEST_NEW_CLASS > \
    { \
        ValueTraits< CXXTEST_OLD_CLASS > _old; \
    public: \
        ValueTraits( CXXTEST_NEW_CLASS n ) : _old( (CXXTEST_OLD_CLASS)n ) {} \
        const char *asString( void ) const { return _old.asString(); } \
    }

    //
    // Certain compilers need separate declarations for T and const T
    //
#   ifdef _CXXTEST_NO_COPY_CONST
#       define CXXTEST_COPY_CONST_TRAITS(CXXTEST_CLASS)
#   else // !_CXXTEST_NO_COPY_CONST
#       define CXXTEST_COPY_CONST_TRAITS(CXXTEST_CLASS) CXXTEST_COPY_TRAITS(CXXTEST_CLASS, const CXXTEST_CLASS)
#   endif // _CXXTEST_NO_COPY_CONST

    //
    // Avoid compiler warnings about unsigned types always >= 0
    //
    template<class N> inline bool negative( N n ) { return n < 0; }
    template<class N> inline N abs( N n ) { return negative(n) ? -n : n; }

#   define CXXTEST_NON_NEGATIVE(Type) \
    CXXTEST_TEMPLATE_INSTANTIATION \
    inline bool negative<Type>( Type ) { return false; } \
    CXXTEST_TEMPLATE_INSTANTIATION \
    inline Type abs<Type>( Type value ) { return value; }

    CXXTEST_NON_NEGATIVE( bool )
    CXXTEST_NON_NEGATIVE( unsigned char )
    CXXTEST_NON_NEGATIVE( unsigned short int )
    CXXTEST_NON_NEGATIVE( unsigned int )
    CXXTEST_NON_NEGATIVE( unsigned long int )
#   ifdef _CXXTEST_LONGLONG
    CXXTEST_NON_NEGATIVE( unsigned _CXXTEST_LONGLONG )
#   endif // _CXXTEST_LONGLONG

    //
    // Represent (integral) number as a string
    // Returns one past the end of the string
    // Remember -- we can't use the standard library!
    //
    template<class N>
    char *numberToString( N n, char *s,
                          N base = 10,
                          unsigned skipDigits = 0,
                          unsigned maxDigits = (unsigned)-1 )
    {
        if ( negative(n) ) {
            *s++ = '-';
            n = abs(n);
        }

        N digit = 1;
        while ( digit <= (n / base) )
            digit *= base;
        N digitValue;
        for ( ; digit >= 1 && skipDigits; n -= digit * digitValue, digit /= base, -- skipDigits )
            digitValue = (unsigned)(n / digit);
        for ( ; digit >= 1 && maxDigits; n -= digit * digitValue, digit /= base, -- maxDigits )
            *s++ = digitToChar( (unsigned)(digitValue = (unsigned)(n / digit)) );

        *s = '\0';
        return s;
    }

    //
    // All the specific ValueTraits follow.
    // You can #define CXXTEST_USER_VALUE_TRAITS if you don't want them
    //

#ifndef CXXTEST_USER_VALUE_TRAITS
    //
    // ValueTraits: const char * const &
    // This is used for printing strings, as in TS_FAIL( "Message" )
    //
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const char * const &>
    {
        ValueTraits &operator=( const ValueTraits & );
        const char *_asString;

    public:
        ValueTraits( const char * const &value ) : _asString( value ) {}
        ValueTraits( const ValueTraits &other ) : _asString( other._asString ) {}
        const char *asString( void ) const { return _asString; }
    };

    CXXTEST_COPY_TRAITS( const char *, const char * const & );
    CXXTEST_COPY_TRAITS( char *, const char * const & );

    //
    // ValueTraits: bool
    //
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const bool>
    {
        bool _value;

    public:
        ValueTraits( const bool value ) : _value( value ) {}
        const char *asString( void ) const { return _value ? "true" : "false"; }
    };

    CXXTEST_COPY_CONST_TRAITS( bool );

#   ifdef _CXXTEST_LONGLONG
    //
    // ValueTraits: signed long long
    //
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const signed _CXXTEST_LONGLONG>
    {
        typedef _CXXTEST_LONGLONG T;
        char _asString[2 + 3 * sizeof(T)];
    public:
        ValueTraits( T t ) { numberToString<T>( t, _asString ); }
        const char *asString( void ) const { return _asString; }
    };

    CXXTEST_COPY_CONST_TRAITS( signed _CXXTEST_LONGLONG );

    //
    // ValueTraits: unsigned long long
    //
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const unsigned _CXXTEST_LONGLONG>
    {
        typedef unsigned _CXXTEST_LONGLONG T;
        char _asString[1 + 3 * sizeof(T)];
    public:
        ValueTraits( T t ) { numberToString<T>( t, _asString ); }
        const char *asString( void ) const { return _asString; }
    };

    CXXTEST_COPY_CONST_TRAITS( unsigned _CXXTEST_LONGLONG );
#   endif // _CXXTEST_LONGLONG

    //
    // ValueTraits: signed long
    //
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const signed long int>
    {
        typedef signed long int T;
        char _asString[2 + 3 * sizeof(T)];
    public:
        ValueTraits( T t ) { numberToString<T>( t, _asString ); }
        const char *asString( void ) const { return _asString; }
    };

    CXXTEST_COPY_CONST_TRAITS( signed long int );

    //
    // ValueTraits: unsigned long
    //
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const unsigned long int>
    {
        typedef unsigned long int T;
        char _asString[1 + 3 * sizeof(T)];
    public:
        ValueTraits( T t ) { numberToString<T>( t, _asString ); }
        const char *asString( void ) const { return _asString; }
    };

    CXXTEST_COPY_CONST_TRAITS( unsigned long int );

    //
    // All decimals are the same as the long version
    //

    CXXTEST_COPY_TRAITS( const signed int, const signed long int );
    CXXTEST_COPY_TRAITS( const unsigned int, const unsigned long int );
    CXXTEST_COPY_TRAITS( const signed short int, const signed long int );
    CXXTEST_COPY_TRAITS( const unsigned short int, const unsigned long int );
    CXXTEST_COPY_TRAITS( const unsigned char, const unsigned long int );

    CXXTEST_COPY_CONST_TRAITS( signed int );
    CXXTEST_COPY_CONST_TRAITS( unsigned int );
    CXXTEST_COPY_CONST_TRAITS( signed short int );
    CXXTEST_COPY_CONST_TRAITS( unsigned short int );
    CXXTEST_COPY_CONST_TRAITS( unsigned char );

    //
    // ValueTraits: char
    // Returns 'x' for printable chars, '\x??' for others
    //
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const char>
    {
        char _asString[sizeof("'\\xXX'")];
    public:
        ValueTraits( char c ) { copyString( charToString( c, copyString( _asString, "'" ) ), "'" ); }
        const char *asString( void ) const { return _asString; }
    };

    CXXTEST_COPY_CONST_TRAITS( char );

    //
    // ValueTraits: signed char
    // Same as char, some compilers need it
    //
    CXXTEST_COPY_TRAITS( const signed char, const char );
    CXXTEST_COPY_CONST_TRAITS( signed char );

    //
    // ValueTraits: double
    //
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const double>
    {
    public:
        ValueTraits( double t )
        {
            ( requiredDigitsOnLeft( t ) > MAX_DIGITS_ON_LEFT ) ?
                hugeNumber( t ) :
                normalNumber( t );
        }

        const char *asString( void ) const { return _asString; }

    private:
        enum { MAX_DIGITS_ON_LEFT = 24, DIGITS_ON_RIGHT = 4, BASE = 10 };
        char _asString[1 + MAX_DIGITS_ON_LEFT + 1 + DIGITS_ON_RIGHT + 1];

        static unsigned requiredDigitsOnLeft( double t );
        char *doNegative( double &t );
        void hugeNumber( double t );
        void normalNumber( double t );
        char *doubleToString( double t, char *s, unsigned skip = 0, unsigned max = (unsigned)-1 );
    };

    CXXTEST_COPY_CONST_TRAITS( double );

    //
    // ValueTraits: float
    //
    CXXTEST_COPY_TRAITS( const float, const double );
    CXXTEST_COPY_CONST_TRAITS( float );
#endif // !CXXTEST_USER_VALUE_TRAITS
}

#ifdef _CXXTEST_HAVE_STD
#   include <cxxtest/StdValueTraits.h>
#endif // _CXXTEST_HAVE_STD

//
// CXXTEST_ENUM_TRAITS
//
#define CXXTEST_ENUM_TRAITS( TYPE, VALUES ) \
    namespace CxxTest \
    { \
        CXXTEST_TEMPLATE_INSTANTIATION \
        class ValueTraits<TYPE> \
        { \
            TYPE _value; \
            char _fallback[sizeof("(" #TYPE ")") + 3 * sizeof(TYPE)]; \
        public: \
            ValueTraits( TYPE value ) { \
                _value = value; \
                numberToString<unsigned long int>( _value, copyString( _fallback, "(" #TYPE ")" ) ); \
            } \
            const char *asString( void ) const \
            { \
                switch ( _value ) \
                { \
                    VALUES \
                    default: return _fallback; \
                } \
            } \
        }; \
    }

#define CXXTEST_ENUM_MEMBER( MEMBER ) \
    case MEMBER: return #MEMBER;

#endif // __cxxtest__ValueTraits_h__
