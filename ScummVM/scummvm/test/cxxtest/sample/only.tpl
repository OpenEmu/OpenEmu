// -*- C++ -*-
#include <cxxtest/StdioPrinter.h>
#include <stdio.h>

int main( int argc, char *argv[] )
{
    if ( argc < 2 || argc > 3 ) {
        fprintf( stderr, "Usage: only <suite name> [<test name>]\n\n" );
        fprintf( stderr, "Available tests:\n" );
        CxxTest::RealWorldDescription wd;
        for ( CxxTest::SuiteDescription *sd = wd.firstSuite(); sd; sd = sd->next() )
            for ( CxxTest::TestDescription *td = sd->firstTest(); td; td = td->next() )
                fprintf( stderr, " - %s::%s()\n", sd->suiteName(), td->testName() );
        return 1;
    }

    const char *suiteName = argv[1];
    const char *testName = (argc > 2) ? argv[2] : 0;
    if ( !CxxTest::leaveOnly( suiteName, testName ) ) {
        if ( testName )
            fprintf( stderr, "Cannot find %s::%s()\n", argv[1], argv[2] );
        else
            fprintf( stderr, "Cannot find class %s\n", argv[1] );
        return 2;
    }
    
    return CxxTest::StdioPrinter().run();
}


// The CxxTest "world"
<CxxTest world>
