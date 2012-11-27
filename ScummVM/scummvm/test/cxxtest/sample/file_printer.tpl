// -*- C++ -*-
// This is a sample of a custom test runner
// using CxxTest template files.
// This prints the output to a file given on the command line.
//

#include <cxxtest/StdioPrinter.h>
#include <stdio.h>

int main( int argc, char *argv[] )
{
    if ( argc != 2 ) {
        fprintf( stderr, "Usage: %s <output file name>\n", argv[0] );
        return -1;
    }

    return CxxTest::StdioPrinter( fopen( argv[1], "w" ) ).run();
}

// The CxxTest "world"
<CxxTest world>
