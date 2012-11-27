//
// This is a test of CxxTest's ValueTraits for enumerations.
//
#include <cxxtest/TestSuite.h>

//
// First define your enumeration
//
enum Answer {
    Yes,
    No,
    Maybe,
    DontKnow,
    DontCare
};

//
// Now make CxxTest aware of it
//
CXXTEST_ENUM_TRAITS( Answer,
                     CXXTEST_ENUM_MEMBER( Yes )
                     CXXTEST_ENUM_MEMBER( No )
                     CXXTEST_ENUM_MEMBER( Maybe )
                     CXXTEST_ENUM_MEMBER( DontKnow )
                     CXXTEST_ENUM_MEMBER( DontCare ) );

class EnumTraits : public CxxTest::TestSuite
{
public:
    void test_Enum_traits()
    {
        TS_FAIL( Yes );
        TS_FAIL( No );
        TS_FAIL( Maybe );
        TS_FAIL( DontKnow );
        TS_FAIL( DontCare );
        TS_FAIL( (Answer)1000 );
    }
};
