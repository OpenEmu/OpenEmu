#include <cxxtest/TestSuite.h>

#ifdef _WIN32
#   include <windows.h>
#   define CXXTEST_SAMPLE_GUI_WAIT() Sleep( 1000 )
#else // !_WIN32
    extern "C" unsigned sleep( unsigned seconds );
#   define CXXTEST_SAMPLE_GUI_WAIT() sleep( 1 )
#endif // _WIN32

class GreenYellowRed : public CxxTest::TestSuite
{
public:
    void wait()
    {
        CXXTEST_SAMPLE_GUI_WAIT();
    }

    void test_Start_green()
    {
        wait();
    }

    void test_Green_again()
    {
        TS_TRACE( "Still green" );
        wait();
    }

    void test_Now_yellow()
    {
        TS_WARN( "Yellow" );
        wait();
    }

    void test_Cannot_go_back()
    {
        wait();
    }

    void test_Finally_red()
    {
        TS_FAIL( "Red" );
        wait();
    }

    void test_Cannot_go_back_to_yellow()
    {
        TS_WARN( "Yellow?" );
        wait();
    }

    void test_Cannot_go_back_to_green()
    {
        wait();
    }
};
