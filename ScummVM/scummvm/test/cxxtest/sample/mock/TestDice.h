#include <cxxtest/TestSuite.h>
#include "Dice.h"
#include "MockStdlib.h"

class TestDice : public CxxTest::TestSuite
{
public:
    MockStdlib *stdlib;

    void setUp()
    {
        TS_ASSERT( stdlib = new MockStdlib );
    }

    void tearDown()
    {
        delete stdlib;
    }

    void test_Randomize_uses_time()
    {
        stdlib->nextTime = 12345;
        Dice dice;
        TS_ASSERT_EQUALS( stdlib->lastSeed, 12345 );
    }

    void test_Roll()
    {
        Dice dice;

        stdlib->nextRand = 0;
        TS_ASSERT_EQUALS( dice.roll(), 1 );

        stdlib->nextRand = 2;
        TS_ASSERT_EQUALS( dice.roll(), 3 );

        stdlib->nextRand = 5;
        TS_ASSERT_EQUALS( dice.roll(), 6 );

        stdlib->nextRand = 7;
        TS_ASSERT_EQUALS( dice.roll(), 2 );
    }

    void test_Temporary_override_of_one_mock_function()
    {
        Dice dice;

        stdlib->nextRand = 2;
        TS_ASSERT_EQUALS( dice.roll(), 3 );

        class Five : public T::Base_rand { int rand() { return 5; } };

        Five *five = new Five;
        TS_ASSERT_EQUALS( dice.roll(), 6 );
        TS_ASSERT_EQUALS( dice.roll(), 6 );
        TS_ASSERT_EQUALS( dice.roll(), 6 );
        delete five;

        stdlib->nextRand = 1;
        TS_ASSERT_EQUALS( dice.roll(), 2 );
    }
};
