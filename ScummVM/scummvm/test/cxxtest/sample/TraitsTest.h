#ifndef __TRAITSTEST_H
#define __TRAITSTEST_H

//
// This example shows how to use TS_ASSERT_EQUALS for your own classes
//
#include <cxxtest/TestSuite.h>
#include <cxxtest/ValueTraits.h>

//
// Define your class with operator==
//
#include <stdio.h>
#include <string.h>

class Pet
{
    char _name[128];
public:
    Pet( const char *petName ) { strcpy( _name, petName ); }

    const char *name() const { return _name; }

    bool operator== ( const Pet &other ) const
    {
        return !strcmp( name(), other.name() );
    }
};

//
// Instantiate CxxTest::ValueTraits<*your class*>
// Note: Most compilers do not require that you define both
//       ValueTraits<const T> and ValueTraits<T>, but some do.
//
namespace CxxTest
{
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const Pet>
    {
        char _asString[256];

    public:
        ValueTraits( const Pet &pet ) { sprintf( _asString, "Pet(\"%s\")", pet.name() ); }
        const char *asString() const { return _asString; }
    };

    CXXTEST_COPY_CONST_TRAITS( Pet );
}

//
// Here's how it works
//
class TestFunky : public CxxTest::TestSuite
{
public:
    void testPets()
    {
        Pet pet1("dog"), pet2("cat");
        TS_ASSERT_EQUALS( pet1, pet2 );
        Pet cat("cat"), gato("cat");
        TS_ASSERT_DIFFERS( cat, gato );
#ifdef _CXXTEST_HAVE_STD
        typedef CXXTEST_STD(string) String;
        TS_ASSERT_EQUALS( String("Hello"), String("World!") );
#endif // _CXXTEST_HAVE_STD
    }
};

#endif // __TRAITSTEST_H
